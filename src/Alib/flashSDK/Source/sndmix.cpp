/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright © Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 
****************************************************************************/

#ifdef SOUND

#include "sndmix.h"
#include NATIVE_SOUND
#include "splay.h"
#include NATIVE_UTIL

//
// The Low Level sound object
//

const S32 kRateTable[4] = {sndRate5K, sndRate11K, sndRate22K, sndRate44K};
const int kRateShiftTable[4] = {3, 2, 1, 0};

void CSound::Init()
{
	format = 0;
	nSamples = 0;
	samples = 0;
	dataLen = 0;
    delay = 0;
}

//
// The Sound Channel object
//

CSoundChannel::CSoundChannel()
{
	mixer = 0;
	next = 0;
	refCount = 0;
	remove = false;
	tag = 0;

	sound = 0;
	envelope.nPoints = 0;

	syncFlags = 0;
	loops = 0;
	inPoint44 = 0;
	outPoint44 = 0x7FFFFFFFL;

	streamProc = 0;
	refPtr = 0;

	samplesPlayed = 0;  //!!@xx look at all the instances of this var

	blockSamples = 0;
    seekSamples = 0;

    blockCanBeZero = false;
	fadeIn = true;
	silentBlock = false;
	streamStalled = false;
    decomp = 0;
}

CSoundChannel::~CSoundChannel()
{
	if ( mixer ) {
		AddRef(); // prevent a recursive delete...
		mixer->RemoveSound(this);
		Release();
	}
	if ( decomp )
		delete decomp;
}

void CSoundChannel::Stop() 
{
	if ( mixer ) 
		mixer->RemoveSound(this); 
}

S32 CSoundChannel::ElapsedFrames()
{
	if ( !mixer ) {
		FLASHASSERT(false);
		return 0;
	}

	// Get the two values we want as quickly as possible so it is unlikely that we get interupted here...
	// We might need a critical section here...
	int sp = samplesPlayed;
	S32 ct = bufferCompletionTime;

	// get an adjustment for the elapsed time into the current buffer
// 	S32 adj = mixer->Time() - ct;	
	S32 adj = GetTimeMSec() - ct;	
	if ( adj > 63 )
		adj = 63;
	adj *= 44;	// convert from msec to 44khz units
	//if ( adj*44 > mixer->SOUND_SAMPLES44 ) {
	//	adj = mixer->SOUND_SAMPLES44/44;
	//}

	if ( streamStalled ) adj = 0;

	// Calculate the elapsed frames
	return (sp + (adj>>sound->RateShift()) + sound->nSamples/2)/sound->nSamples;
}
 
//
// The Sound Mixer
//


void CSoundMix::Construct()
{
	mixFormat.Init();
// 	mixFormat.format = BEST_SOUND_FORMAT;

	firstPlay = true;

	firstChannel = 0;
	nBuffers = INITIAL_SOUND_BUFFERS;

	bufferDuration = FixedDiv(SOUND_SAMPLES44, FloatToFixed(44.1));

	nSilent = 0;

	bufferSamples = 0;
	{
		for ( int i = 0; i < MAX_SOUND_BUFFERS; i++ )
			buffer[i] = 0;
	}
	
	nextScratch = 0;
	{
		for ( int i = 0; i < MAX_SOUND_CHANNELS; i++ )
			scratchBuf[i] = 0;
	}

	NativeConstruct();
}

void CSoundMix::Destruct()
{
	RemoveAll();
	CloseDevice();
	FreeBuffers();
}

void CSoundMix::FreeBuffers()
{	
	{
		for ( int i = 0; i < MAX_SOUND_BUFFERS; i++ ) {
			delete buffer[i];
			buffer[i] = 0;
		}
	}
	{
		for ( int i = 0; i < MAX_SOUND_CHANNELS; i++ ) {
			delete scratchBuf[i];
			scratchBuf[i] = 0;
		}
	}
}

void CSoundMix::EnlargeBuffers()
{
	if ( nBuffers < MAX_SOUND_BUFFERS ) {
		if ( buffer[nBuffers] == 0 ) {
			// Allocate the buffer if not already allocated
			buffer[nBuffers] = new U8[bufferBytes];
			waveHdr[nBuffers].data		= (U8*)buffer[nBuffers];
			waveHdr[nBuffers].bufferId	= nBuffers;
		}

		nBuffers++;
	}
	BuffersUpdated();
}

void CSoundMix::AllocBuffers()
{
	S32 nSamples = SOUND_SAMPLES44>>mixFormat.RateShift();
	if ( buffer[0] && nSamples == bufferSamples ) 
		return;
	FreeBuffers();

	bufferDuration = FixedDiv(SOUND_SAMPLES44, FloatToFixed(44.1));

	// Allocate the buffers
	bufferSamples = nSamples;
	bufferBytes = mixFormat.BytesPerBlock()*nSamples;

	{
		for ( int i = 0; i < nBuffers; i++ )
			buffer[i] = new U8[bufferBytes];
	}

	// We allocate scratch buffers large enough to hold 44kHz 16 bit stereo sound
	//	so that we have space for any format conversion
	scratchBytes = SOUND_SAMPLES44*4;

	// Set up the wave headers
	for ( int i = 0; i < MAX_SOUND_BUFFERS; i++ ) {
		memset(waveHdr+i, 0, sizeof(WaveHeader));
		waveHdr[i].data			= (U8*)buffer[i];
		waveHdr[i].bufferId		= i;//(U32)this;
	}
	BuffersUpdated();
}

void* CSoundMix::GetScratch()
{
	FLASHASSERT(nextScratch<16);
	// The mac can't allocate these at interupt time, they better already be available
	if ( !scratchBuf[nextScratch] ) {
		scratchBuf[nextScratch] = new U8[scratchBytes];
	}
	return scratchBuf[nextScratch++];
}

// void CSoundMix::SetFormat(int f)
// {
// 	// Never mix at 5 khz
// 	//if ( (f & sndRateMask) < snd11K )
// 	//	f = (f & ~sndRateMask) | snd11K;
// 
// 	if ( f != mixFormat.format ) {
// 		CloseDevice();
// 		FreeBuffers();
// 		mixFormat.format = f;
// 
// 		if ( firstChannel )
// 			OpenDevice();
// 	}
// }

// void CSoundMix::SendBuffer(int i, BOOL silence)
// {
// 	if ( !buffer[i] ) return;
// 
// 	WaveHeader* hdr = waveHdr+i;
// 
// 	// Send the new data
// 	BuildAndWrite( hdr, silence );
// 
// 	//TRACE("write\n");
// // #endif
// }

void CSoundMix::OpenDevice()
{
	if ( IsDeviceOpen() ) return;

	EnterCritical();
	{
		GetNativeSoundCap( &mixFormat.format );
		int okay = OpenNativeDevice( &mixFormat );

		if ( !okay ) 
			goto Fail; // something is wrong...

		// We are ready to play sound...
// 		deviceOpen = true;
		nSilent = 0;

		AllocBuffers();
		if ( !buffer[0] || !buffer[1] ) {
			CloseNativeDevice();
			goto Fail;
		}

		// Prime the stream, send first two buffers as silence to prevent burps
		int nSilence = firstPlay ? 4 : 2;
		firstPlay = false;
		for ( int i = 0; i < nBuffers; i++ ) {
// 			SendBuffer(i, i < nSilence);	
			if ( buffer[i] )
			{
	 			BuildAndWrite( &waveHdr[i], i < nSilence );
			}
		}

		BuffersReady();
	}

Fail:
	//TRACE("open\n");
	LeaveCritical();

}

void CSoundMix::CloseDevice()
{
	CloseNativeDevice();
}

void CSoundMix::BuildBuffer(int iBuf)
{
	// Build a scratch buffer with each sound to be mixed
	int mixN = 0;
	void* mixBuf[MAX_SOUND_CHANNELS];

	BOOL silent = true;
	for ( CSoundChannel* c = firstChannel; c; c = c->next ) {
		if ( c->remove )
			continue;	// This channel is finished and waiting to be removed

		silent = false;

		// Set up the buffer
		void* buf = 0;
		if ( mixN < MAX_SOUND_CHANNELS ) {
			buf = GetScratch();
			if ( buf ) 
				mixBuf[mixN++] = buf;
		}

		// Get the sound from the source into the mixBuf
		CSound* s = c->sound;
		int rs = s->RateShift();
		int bytesPerBlock = s->BytesPerBlock();

		S32 n = SOUND_SAMPLES44>>rs;
		U8* b = (U8*)buf;

		if ( c->streamProc ) {
			// Get samples from a synchronized sound stream
			c->bufferCounts[iBuf] = 0;
			//FLASHASSERT(!s->Is8Bit());	// I think this is ok for uncompressed sounds...
			while ( n > 0 ) {
				S32 nc = Min(n, c->blockSamples);
				if ( nc > 0) {

					if ( c->silentBlock ) {
						// Note we are not advancing in the stream...
						if ( b )
							memset(b, 0x00, nc*bytesPerBlock);
					} else {
						c->decomp->Decompress((S16*)b, nc);
					}
					if ( !c->streamStalled )
						c->bufferCounts[iBuf] += nc;	// we're playing these samples from the source stream...

					if ( b )
						b += nc*bytesPerBlock;
					n -= nc;
					c->blockSamples -= nc;
				} else {
					// We are out of samples, get a new block
					c->streamStalled = !c->streamProc(c);

					if ( c->blockSamples <= 0) {
						// We did not get any data, create a silent block
                        if (c->blockCanBeZero)
                        {
                            c->blockCanBeZero = false;
                            if (c->seekSamples < 0)
                            {
                                // the steaming code is telling us
                                // to play some silence to sync to the
                                // futere sound frames
						        c->blockSamples = -c->seekSamples;
						        c->seekSamples = 0;
                            }
                        }
                        else
						    c->blockSamples = s->nSamples;

						c->silentBlock = true;
					} else {
						// Play back this block of data
						c->silentBlock = false;
					}
				}
			}

			// Fade in on the first block
			if ( c->fadeIn ) {
				c->fadeIn = false;
				SoundEnvelope fader;
				fader.nPoints = 2;
				fader.points[0].mark44 = 0;
				fader.points[0].level0 = fader.points[0].level1 = 0;
				fader.points[1].mark44 = Min(SOUND_SAMPLES44, sndRate44K/4);
				fader.points[1].level0 = fader.points[1].level1 = sndMaxLevel;
				if ( buf )
					ApplyEnvelope(c->sound->format, buf, SOUND_SAMPLES44, 0, &fader);
			}

		} else {
			// Get samples from an event sound
			while ( n > 0 ) {
				S32 nc = Min(n, c->outPos - c->pos);
				if ( nc <= 0 ) {
					// We are out of samples
					if ( --c->loops > 0 ) {
						// Loop the sound
						c->pos = c->inPoint44>>rs;
				
						// Setup should be the same as a rewind operation for the decompressor - Warning we cannot do memory allocations here on Mac...
						c->decomp->Setup(s, true);
						c->decomp->Decompress(0, c->pos);	// skip

					} else {
						// Remove the channel and pad w/ silence
						if ( b )
							memset(b, s->Is8Bit() ? 0x80 : 0x00, n*bytesPerBlock);
						c->remove = true;
						break;
					}
				}

				// Decompress the sound
				c->decomp->Decompress((S16*)b, nc);
					
				if ( b )
					b += nc*bytesPerBlock;
				n -= nc;
				c->pos += nc;
			}
		}

		// Convert the sound to the mixer format
		if ( buf ) {
			// Save the last sample from this buffer for interpolating with the next buffer
			S32 savedSample;
			memcpy(&savedSample, (U8*)buf+bytesPerBlock*((SOUND_SAMPLES44>>rs)-1), bytesPerBlock);

			if ( !c->prevSampleInited ) {
				// For the first buffer, just use the first sample of this buffer
				memcpy(&c->prevSample, buf, bytesPerBlock);
				c->prevSampleInited = true;
			}

			ConvertSound(buf, SOUND_SAMPLES44, s->format&~sndCompressMask, mixFormat.format, &c->prevSample);
			c->prevSample = savedSample;
		}

		// Apply a volume envelope
		if ( c->envelope.nPoints > 0 ) {
			if ( buf )
				ApplyEnvelope(mixFormat.format, buf, SOUND_SAMPLES44, c->envelopePos44, &c->envelope);
			c->envelopePos44 += SOUND_SAMPLES44;
		}
	}

	if ( silent )
		nSilent++;
	else
		nSilent = 0;

	// Add the sound channels together
	if ( mixFormat.Is8Bit() ) {
		// Combine the sounds into our buffer
		// Note this changes the mixBuf pointers
		U8* buf = (U8*)buffer[iBuf];
		if ( mixN == 0 ) {
			// Set the buffer to silence
			memset(buf, 0x80, bufferBytes);
			//memset(buf, 0x10, bufferBytes/2); // a test square wave...

		} else {
			if ( mixN == 1 ) {
				// Copy a single sound
				memcpy(buf, mixBuf[0], bufferBytes);
				
			} else if ( mixN == 2 ) {
				// Mix 2 sounds
				U8* src1 = (U8*)mixBuf[0];
				U8* src2 = (U8*)mixBuf[1];
				U8* dst = buf;
				S32 i = bufferSamples;
				if ( mixFormat.Stereo() ) i*=2;
				while ( i-- ) {
					int s = (int)*src1++ + *src2++ - 0x80;
					if ( s & 0xFFFF00 ) {
						// Overflow or underflow
						*dst++ = s < 0 ? 0 : 0xff;
					} else {
						*dst++ = (U8)s;
					}
				}

			} else {
				// Mix multiple sounds
				U8* src[MAX_SOUND_CHANNELS];
				memcpy(src, mixBuf, sizeof(void*)*mixN);

				U8* dst = buf;
				int offset = (mixN-1) * -0x80;
				S32 i = bufferSamples;
				if ( mixFormat.Stereo() ) i*=2;
				while ( i-- ) {
					int s = offset;
					for ( int j = mixN; j--; ) {
						s += *(src[j]++);
					}
					if ( s & 0xFFFF00 ) {
						// Overflow or underflow
						*dst++ = s < 0 ? 0 : 0xff;
					} else {
						*dst++ = (U8)s;
					}
				}
			}
		}
	} else {
		// Combine the sounds into our buffer
		// Note this changes the mixBuf pointers
		S16* buf = (S16*)buffer[iBuf];
		if ( mixN == 0 ) {
			// Set the buffer to silence
			memset(buf, 0, bufferBytes);
			//memset(buf, 0x10, bufferBytes/2); // a test square wave...
		} else {
			if ( mixN == 1 ) {
				// Copy a single sound
				memcpy(buf, mixBuf[0], bufferBytes);

			} else if ( mixN == 2 ) {
				// Mix 2 sounds
				S16* src1 = (S16*)mixBuf[0];
				S16* src2 = (S16*)mixBuf[1];
				S16* dst = buf;
				S32 i = bufferSamples;
				if ( mixFormat.Stereo() ) i*=2;
				while ( i-- ) {
					S32 s = (S32)*src1++ + *src2++;
					if ( (S16)s != s ) {
						// Overflow or underflow - if the high 16 bits are not all 0 or all 1
						*dst++ = s < 0 ? (int)-32768 : (int)32767;
					} else {
						*dst++ = (S16)s;
					}
				}

			} else {
				// Mix multiple sounds
				S16* src[MAX_SOUND_CHANNELS];
				memcpy(src, mixBuf, sizeof(void*)*mixN);

				S16* dst = buf;
				S32 i = bufferSamples;
				if ( mixFormat.Stereo() ) i*=2;
				while ( i-- ) {
					S32 s = 0;
					for ( int j = mixN; j--; ) {
						s += *(src[j]++);
					}
					if ( (S16)s != s ) {
						// Overflow or underflow - if the high 16 bits are not all 0 or all 1
						*dst++ = s < 0 ? (int)-32768 : (int)32767;
					} else {
						*dst++ = (S16)s;
					}
				}
			}
		}
	}

	ReleaseScratch();	// mark the scratch buffers as unused
}

/*! AddSound takes a pointer to a CSoundChannel and prepares the CSoundChannel for playing. 
	It then enters a critical section, and adds the sound to the list of sounds being played.
	If the sound device is not open, AddSound will open it.
 */
void CSoundMix::AddSound(CSoundChannel* c)
{
	FLASHASSERT(!c->mixer);

	// Create the decompression object for the sound data
	FLASHASSERT(!c->decomp);	// sound channels are never reused...
	c->decomp = CreateDecompressor(c->sound->CompressFormat());
	if ( !c->decomp ) 
		return;		// we failed to create a decompressor, don't even start the sound channel

	if ( c->streamProc ) {
		// A stream channel
		for ( int i = 0; i < MAX_SOUND_BUFFERS; i++ )
			c->bufferCounts[i] = 0;

		c->bufferCompletionTime = GetTimeMSec();

	} else {
		// An event channel
		// Calculate the start and stop times
		CSound* s = c->sound;

		if ( c->syncFlags & syncStop ) {
			// Stop the sound
			FreeSound(s);
			return;
		} else if ( (c->syncFlags & syncNoMultiple) && SoundPlaying(s) ) {
			// Don't start the sound since it is already playing
			return;
		}

		int rs = s->RateShift();
		c->pos = c->inPoint44>>rs;
		c->outPos = Min(c->outPoint44>>rs, s->nSamples);
		c->envelopePos44 = 0;
		if ( c->pos >= c->outPos ) // there is no sound to play, ignore the channel
			return;

		c->decomp->Setup(c->sound, true);
		c->decomp->Decompress(0, c->pos);	// Skip to the start point
	}

	// Init prevSample in case we need to upsample...
	c->prevSampleInited = false;

	EnterCritical();

	PurgeList();	// Remove any old channels...

	c->AddRef();

	// Add to the mixer's channel list
	c->mixer = this;
	// Add at the end...
	CSoundChannel** link = &firstChannel;
	while ( *link ) 
		link = &(*link)->next;
	*link = c;
	//c->next = firstChannel;
	//firstChannel = c;

	// Open the device when we first need it...
	if ( !IsDeviceOpen() ) {
		OpenDevice();
		if ( !IsDeviceOpen() )
			// The device failed to open
			RemoveAll();
	}

	LeaveCritical();
}

void CSoundMix::RemoveSound(CSoundChannel* c)
{
	c->remove = true;
	PurgeList();
}

void CSoundMix::PurgeList()
{
	EnterCritical();
	CSoundChannel** l = &firstChannel;
	while ( *l ) {
		CSoundChannel* c = *l;
		if ( c->remove ) {
			// Remove the channel
			*l = c->next;
			c->remove = false;
			c->next = 0;
			c->mixer = 0;
			c->Release();
		} else {
			// Advance to the next channel
			l = &c->next;
		}
	}
	LeaveCritical();
}

void CSoundMix::RemoveAll()
{
	for ( CSoundChannel* c = firstChannel; c; c = c->next )
		c->remove = true;
		
	PurgeList();
}

void CSoundMix::RemoveTagged(U32 tag)
{
	for ( CSoundChannel* c = firstChannel; c; c = c->next ) {
		if ( c->tag == tag )
			c->remove = true;
	}

	PurgeList();
}

void CSoundMix::FreeSound(CSound* s)
// Remove the channel if a CSound is going away
{
	for ( CSoundChannel* c = firstChannel; c; c = c->next ) {
		if ( c->sound == s )
			c->remove = true;
	}

	PurgeList();
}

BOOL CSoundMix::SoundPlaying(CSound* s)
// return true if the sound is playing
{
	BOOL playing = false;
	EnterCritical();
	for ( CSoundChannel* c = firstChannel; c; c = c->next ) {
		if ( c->sound == s && !c->remove ) {
			playing = true;
			break;
		}
	}
	LeaveCritical();
	return playing;
}

void CSoundMix::MarkBufferComplete(int i)
{
	S32 bufferCompletionTime = GetTimeMSec();

	for ( CSoundChannel* c = firstChannel; c; c = c->next ) {
		c->samplesPlayed += c->bufferCounts[i];
		c->bufferCounts[i] = 0;
		c->bufferCompletionTime = bufferCompletionTime;
	}
}

//
// Sample Conversion Code
//

void Cvt8RateDiv2(U8* samples, S32 n, BOOL stereo)
// n is the original number of samples
// Need to filter out high frequencies
{
	U8* src = samples;
	U8* dst = samples;
	if ( stereo ) {
		while ( n > 0 ) {
			dst[0] = (U8)(((int)src[0]+src[2])/2);
			dst[1] = (U8)(((int)src[1]+src[3])/2);
			src+=4;
			dst+=2;
			n-=2;
		}
	} else {
		while ( n > 0 ) {
			dst[0] = src[0];
			src+=2;
			dst++;
			n-=2;
		}
	}
}

void Cvt8RateMul2(U8* samples, S32 n, BOOL stereo, U8* prevSample)
// n is the original number of samples
{
	FLASHASSERT(n>0);
	if ( stereo ) {
		U8* src = samples+n*2;
		U8* dst = samples+n*4;

		// Interpolate samples
		while ( true ) {
			dst-=4;
			src-=2;
			n--;
			dst[2] = src[0];
			dst[3] = src[1];
			if ( n == 0 ) {
				// For the first sample, interpolate with the saved previous sample
				dst[0] = (U8)(((int)prevSample[0]+src[0])/2);
				dst[1] = (U8)(((int)prevSample[1]+src[1])/2);
				break;
			}
			dst[0] = (U8)(((int)src[-2]+src[0])/2);
			dst[1] = (U8)(((int)src[-1]+src[1])/2);
		}
	} else {
		U8* src = samples+n;
		U8* dst = samples+n*2;

		// Interpolate samples
		while ( true ) {
			dst-=2;
			src--;
			n--;
			dst[1] = src[0];
			if ( n == 0 ) {
				// For the first sample, interpolate with the saved previous sample
				dst[0] = (U8)(((int)prevSample[0]+src[0])/2);
				break;
			}
			dst[0] = (U8)(((int)src[-1]+src[0])/2);
		}
	}
}

void Cvt16RateDiv2(S16* samples, S32 n, BOOL stereo)
// n is the original number of samples
// Need to filter out high frequencies
{
	S16* src = samples;
	S16* dst = samples;
	if ( stereo ) {
		while ( n > 0 ) {
			dst[0] = (S16)(((S32)src[0]+src[2])/2);
			dst[1] = (S16)(((S32)src[1]+src[3])/2);
			src+=4;
			dst+=2;
			n-=2;
		}
	} else {
		while ( n > 0 ) {
			dst[0] = src[0];
			src+=2;
			dst++;
			n-=2;
		}
	}
}

void Cvt16RateMul2(S16* samples, S32 n, BOOL stereo, S16* prevSample)
// n is the original number of samples
{
	FLASHASSERT(n>0);
	if ( stereo ) {
		S16* src = samples+n*2;
		S16* dst = samples+n*4;

		// Interpolate samples
		while ( true ) {
			dst-=4;
			src-=2;
			n--;
			dst[2] = src[0];
			dst[3] = src[1];
			if ( n == 0 ) {
				// For the first sample, interpolate with the saved previous sample
				dst[0] = (S16)(((S32)prevSample[0]+src[0])/2);
				dst[1] = (S16)(((S32)prevSample[1]+src[1])/2);
				break;
			}
			dst[0] = (S16)(((S32)src[-2]+src[0])/2);
			dst[1] = (S16)(((S32)src[-1]+src[1])/2);
		}
	} else {
		S16* src = samples+n;
		S16* dst = samples+n*2;

		// Interpolate samples
		while ( true ) {
			dst-=2;
			src--;
			n--;
			dst[1] = src[0];
			if ( n == 0 ) {
				// For the first sample, interpolate with the saved previous sample
				dst[0] = (S16)(((S32)prevSample[0]+src[0])/2);
				break;
			}
			dst[0] = (S16)(((S32)src[-1]+src[0])/2);
		}
	}
}

inline void Cvt8to16(U8* samples, S32 n, BOOL stereo)
{
	if ( stereo ) n*=2;
	U8* src = samples+n;
	S16* dst = (S16*)samples+n;
	while ( n > 0 ) {
		src--;
		dst--;
		n--;
		*dst = ((S16)*src - 128)<<8;
	}
}

inline void Cvt16to8(S16* samples, S32 n, BOOL stereo)
{
	if ( stereo ) n*=2;
	S16* src = samples;
	U8* dst = (U8*)samples;
	while ( n > 0 ) {
		*dst = (U8)((*src>>8)+128);
		src++;
		dst++;
		n--;
	}
}

inline void Cvt8StereoToMono(U8* samples, S32 n)
{
	U8* src = samples;
	U8* dst = samples;
	while ( n > 0 ) {
		*dst = (U8)(((int)src[0] + (int)src[1])/2);
		src+=2;
		dst++;
		n--;
	}
}

inline void Cvt8MonoToStereo(U8* samples, S32 n)
{
	U8* src = samples+n;
	U8* dst = samples+2*n;
	while ( n > 0 ) {
		n--;
		src--;
		dst-=2;
		dst[0] = dst[1] = *src;
	}
}

inline void Cvt16StereoToMono(S16* samples, S32 n)
// n is the number of samples
{
	S16* src = samples;
	S16* dst = samples;
	while ( n > 0 ) {
		*dst = (S16)(((S32)src[0] + (S32)src[1])/2);
		src+=2;
		dst++;
		n--;
	}
}

inline void Cvt16MonoToStereo(S16* samples, S32 n)
{
	S16* src = samples+n;
	S16* dst = samples+2*n;
	while ( n > 0 ) {
		n--;
		src--;
		dst-=2;
		dst[0] = dst[1] = *src;
	}
}

void ConvertSound(void* samples, S32 nSamples44, int srcFormat, int dstFormat, void* prevSample)
// The memory pointed to by src->samples must be large enough to
//	hold the largest amount of data required by the conversion.
// The result will be placed in this memory
// returns the new number of samples - this is different only w/ a rate change...
{
	if ( srcFormat == dstFormat || nSamples44 == 0 ) return;
	CSound src, dst;
	src.format = srcFormat;
	dst.format = dstFormat;

	// We should always convert a multiple of 8 samples
	FLASHASSERT((nSamples44 & 7) == 0);

	// We don't do compression
	FLASHASSERT(!dst.Compressed()&&!src.Compressed());	

	S32 nSamples = nSamples44>>src.RateShift();

	{// Handle a rate change
		int srcR = src.RateShift();
		int dstR = dst.RateShift();
		// Decrease the src rate
		while ( srcR < dstR ) {
			if ( src.Is8Bit() )
				Cvt8RateDiv2((U8*)samples, nSamples, src.Stereo());
			else 
				Cvt16RateDiv2((S16*)samples, nSamples, src.Stereo());
			nSamples /= 2;
			srcR++;
		}
		// Increase the src rate
		while ( srcR > dstR ) {
			if ( src.Is8Bit() ) 
				Cvt8RateMul2((U8*)samples, nSamples, src.Stereo(), (U8*)prevSample);
			else
				Cvt16RateMul2((S16*)samples, nSamples, src.Stereo(), (S16*)prevSample);
			nSamples *= 2;
			srcR--;
		}
	}

	// Sample Size change
	if ( !dst.Is8Bit() && src.Is8Bit() ) {
		// Map 8->16
		Cvt8to16((U8*)samples, nSamples, src.Stereo());

	} else if ( dst.Is8Bit() && !src.Is8Bit() ) {
		// Map 16->8
		Cvt16to8((S16*)samples, nSamples, src.Stereo());
	}

	// Stereo/Mono conversion
	if ( !dst.Stereo() && src.Stereo() ) {
		// Stereo -> Mono conversion
		if ( dst.Is8Bit() ) 
			Cvt8StereoToMono((U8*)samples, nSamples);
		else
			Cvt16StereoToMono((S16*)samples, nSamples);

	} else if ( dst.Stereo() && !src.Stereo() ) {
		// Mono -> Stereo conversion
		if ( dst.Is8Bit() ) 
			Cvt8MonoToStereo((U8*)samples, nSamples);
		else
			Cvt16MonoToStereo((S16*)samples, nSamples);
	}
}


//
// Enveloping Code
//

void ApplyRamp8(U8* s, S32 n, int stride, S32 level, S32 dLevel)
{
	if ( dLevel == 0 ) {
		// A constant level
		if ( level == 0 ) {
			// Silence
			while ( n-- ) {
				*s = 128;
				s+=stride;
			}

		} else if ( level == sndMaxLevel ) {
			// Nothing to do...

		} else {
			// A constant level
			level = level >> 15;
			while ( n-- ) {
				*s = (U8)((( ((int)*s-128) * level) >> 15) + 128);
				s+=stride;
			}
		}
	} else {
		// A ramped level
		while ( n-- ) {
			*s = (U8)((( ((int)*s-128) * (level>>15)) >> 15) + 128);
			level += dLevel;
			s+=stride;
		}
	}
}

void ApplyRamp16(S16* s, S32 n, int stride, S32 level, S32 dLevel)
{
	if ( dLevel == 0 ) {
		// A constant level
		if ( level == 0 ) {
			// Silence
			while ( n-- ) {
				*s = 0;
				s+=stride;
			}
		} else if ( level == sndMaxLevel ) {
			// Nothing to do...

		} else {
			// A constant level
			level = level >> 15;
			while ( n-- ) {
				*s = (S16)((*s * level)>>15);
				s+=stride;
			}
		}
	} else {
		// A ramped level
		while ( n-- ) {
			*s = (S16)((*s * (level>>15))>>15);
			level += dLevel;
			s+=stride;
		}
	}
}

void ApplyEnvelope(int format, void* samples, S32 nSamples44, S32 time44, SoundEnvelope* envelope)
// The time should be relative to the start of the envelope
{
	if ( envelope->nPoints == 0 ) 
		return;	// the envelope is empty

	CSound snd;
	snd.format = format;
	int rateShift = snd.RateShift();
	S32 n = nSamples44 >> rateShift;

	int bytesPerBlock = snd.BytesPerBlock();

	int ie = 0;
	while ( n > 0 ) {
		// Find the proper segment of the envelope
		while ( ie < envelope->nPoints ) {
			if ( envelope->points[ie].mark44 > time44 ) break;
			ie++;
		}

		// Set up a new envelope segment
		// Note: The level difference equation is 2.30 fixed point numbers
		S32 nSeg = n;
		S32 dLevel0, dLevel1, level0, level1;
		if ( ie == 0 ) {
			// Before the start-should never happen...
			dLevel0 = dLevel1 = 0;
			level0 = (S32)envelope->points[0].level0 << 15;
			level1 = (S32)envelope->points[0].level1 << 15;
		} else if ( ie == envelope->nPoints ) {
			// After the end
			dLevel0 = dLevel1 = 0;
			level0 = (S32)envelope->points[ie-1].level0 << 15;
			level1 = (S32)envelope->points[ie-1].level1 << 15;
		} else {
			// In a ramp
			SoundEnvelopePoint* pt = envelope->points+ie-1;
			FLASHASSERT(pt[0].mark44 <= time44 && pt[1].mark44 > time44);

			S32 time = time44 >> rateShift;
			S32 mark0 = pt[0].mark44 >> rateShift;
			S32 dmark = (pt[1].mark44 - pt[0].mark44) >> rateShift;

			dLevel0 = ((S32)(pt[1].level0 - pt[0].level0)<<15) / dmark;
			level0  = ((S32)pt[0].level0<<15) + dLevel0*(time-mark0);

			dLevel1 = ((S32)(pt[1].level1 - pt[0].level1)<<15) / dmark;
			level1  = ((S32)pt[0].level1<<15) + dLevel1*(time-mark0);

			nSeg = Min(n, (pt[1].mark44-time44) >> rateShift);
		}

		// Apply this segment of the envelope
		n -= nSeg;
		if ( snd.Stereo() ) {
			if ( snd.Is8Bit() ) {
				ApplyRamp8((U8*)samples, nSeg, 2, level0, dLevel0);
				ApplyRamp8((U8*)samples+1, nSeg, 2, level1, dLevel1);
			} else {
				ApplyRamp16((S16*)samples, nSeg, 2, level0, dLevel0);
				ApplyRamp16((S16*)samples+1, nSeg, 2, level1, dLevel1);
			}
		} else {
			// Merge the two envelopes for a mono envelope
			level0 = (level0 + level1)/2;
			dLevel0 = (dLevel0 + dLevel1)/2;

			if ( snd.Is8Bit() ) {
				ApplyRamp8((U8*)samples, nSeg, 1, level0, dLevel0);
			} else {
				ApplyRamp16((S16*)samples, nSeg, 1, level0, dLevel0);
			}
		}
		samples = (char*)samples + bytesPerBlock*nSeg;
	}
}

#endif
