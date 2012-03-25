/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright © Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 
****************************************************************************/

// 980910 mnk deleted comma after last member of an enum to avoid warning
// 971003 mnk use CONST int instead of enum to avoid warnings


#ifndef SNDMIX_INCLUDED
#define SNDMIX_INCLUDED

#ifdef SOUND

#ifndef FIXED_INCLUDED
#include "fixed.h"
#endif
#ifndef SNDCODEC_INCLUDED
#include "sndcodec.h"
#endif

enum {// Sound format types
		snd5K8Mono = 0,		snd5K8Stereo, 
		snd5K16Mono,		snd5K16Stereo, 
		snd11K8Mono,		snd11K8Stereo, 
		snd11K16Mono,		snd11K16Stereo, 
		snd22K8Mono,		snd22K8Stereo, 
		snd22K16Mono,		snd22K16Stereo, 
		snd44K8Mono,		snd44K8Stereo, 
		snd44K16Mono,		snd44K16Stereo 
	  };


const int MAX_SOUND_BUFFERS			= 32;
const int MAX_SOUND_CHANNELS		= 8;
const int SOUND_SAMPLES44			= 2048;	// ~50 msec
const int INITIAL_SOUND_BUFFERS		= 2;
// const int BEST_SOUND_FORMAT			= snd22K16Stereo;

// Our supported sample rates
// Note that sndRate5K is really 5512.5khz
// Use defines instead of an enum since these must be 32 bits on all platforms
#define sndRate5K	5512L
#define sndRate11K	11025L
#define sndRate22K	22050L
#define sndRate44K	44100L

const int sndMono			= 0x0;
const int sndStereo			= 0x1;

const int snd8Bit			= 0x0;
const int snd16Bit			= 0x2;

const int snd5K				= 0<<2;
const int snd11K			= 1<<2;
const int snd22K			= 2<<2;
const int snd44K			= 3<<2;

const int sndCompressNone	= 0x00; // we could add 14 more compression types here...
const int sndCompressADPCM	= 0x10;
const int sndCompressMP3	= 0x20;
const int sndCompressNoneI	= 0x30; // save out in intel byte order
const int sndRateMask		= 0x3 << 2;
const int sndCompressMask	= 0xF0;

struct WaveHeader {
  void* data;
  S32 bufferId;
  U32 userFlags;
  U32 userData;
};

#define WAVEHEADER_PREPARED		0x01
#define WAVEHEADER_DONE			0x02

extern const S32 kRateTable[4];
extern const int kRateShiftTable[4];

// This object defines a sound sample
class CSound {
public:
	S32			format;		
	S32			nSamples; 	// the number of samples - duration = nSamples/Rate()
	void*		samples;  	// this should probably be a handle on Mac
	S32			dataLen;   	// length in bytes of samples - set only if needed
    S32         delay;      // MP3 compression has a delay before the real sound data
	
public:
	void Init();

	S32 Rate() { return kRateTable[(format >> 2)&0x3]; }
	int RateShift() { return kRateShiftTable[(format >> 2)&0x3]; }
	BOOL Stereo() { return (format & sndStereo) != 0; }
	int NChannels() { return (format & sndStereo) ? 2 : 1; }
	BOOL Is8Bit() { return (format & snd16Bit) == 0; }
	int BitsPerSample() { return (format & snd16Bit) ? 16 : 8; }
	int BytesPerSample() { return (format & snd16Bit) ? 2 : 1; }
	int CompressFormat() { return format & sndCompressMask; }
	BOOL Compressed() { return (format & sndCompressMask) != 0; }

	// Manage the duration in 44kHz units
	S32 GetDuration44() { return nSamples<<RateShift(); }
	void SetDuration44(S32 d) { nSamples = d>>RateShift(); }

	int BytesPerBlock() { return BytesPerSample()*NChannels(); }
	S32 SizeBytes() { return nSamples*BytesPerBlock(); }
};


// A sound Envelope consists of at least 2 marks 
// The first mark must be zero and the last mark must be "soundMaxMark"

enum { 
	   sndMinLevel = 0,
	   sndMaxLevel = 0x8000,

	   sndEnvelopeMaxPoints = 8	// 980910 mnk deleted comma after last member to avoid warning 
	};
	
struct SoundEnvelopePoint { 
	S32 mark44;
	U16 level0;	// left channel - level is a 1.15 unsigned fixed point number (0..0x8000)
	U16 level1;	// right channel
};

struct SoundEnvelope {
	int nPoints;
	SoundEnvelopePoint points[sndEnvelopeMaxPoints];
};

class CSoundMix;
class ScriptPlayer;

class CSoundChannel;

typedef BOOL (*StreamBlockProc)(CSoundChannel*);

enum {
	syncNoMultiple		= 0x1,	// don't start the sound if already playing
	syncStop			= 0x2	// stop the sound
};

/*! CSoundChannel controls the playing of sounds. Is is not a sound sample (it only has a 
 *  pointer to CSound) but it is all the wrapping info: when to start and
 *  stop the sound, the sound envelope, etc.
 */
class CSoundChannel {
public:
	// These are set is the channel is currently playing
	CSoundMix* mixer;
	CSoundChannel* next;
	int refCount;
	BOOL remove;		// set if this channel is finished playing and needs to be removed
	U32 tag;

	CSound* sound;		// the samples that define this sound

	// The user can set these values for event sounds
	SoundEnvelope envelope;
	int syncFlags;		// borrowed from stags.h can contain, soundNoMultiple or soundStop
	int loops;			// loop this sound
	S32	inPoint44;		// the first sample to play
	S32	outPoint44;		// the last sample to play

	// This is for streaming sounds
	StreamBlockProc streamProc;
	void* refPtr;

	// Support for checking the amount of the stream that has been played
	S32 bufferCounts[MAX_SOUND_BUFFERS];
	S32 samplesPlayed;				// the sound samples that have been played from this stream
	S32 bufferCompletionTime;		// the time the most recent mixer buffer completed playing for this stream

	S32 blockSamples;	// samples left in the current block
    S32 seekSamples;    // used for mp3 streaming sound
    U8 blockCanBeZero;  // used for mp3 streaming sound
	U8 fadeIn;
	U8 silentBlock;		// set if the stream is playing a silent block of data
	U8 streamStalled;	// set if the stream is waiting for data from the network...

public:
	// These are setup by AddSound
	S32 envelopePos44;	// the current position in the envelope
	S32 pos;			// the current position in the channel
	S32 outPos;			// the sample to stop at

	BOOL prevSampleInited;	// set if prevSample has been inited...
	S32 prevSample;		    // save the previous sample from the unconverted sound so we can do correct interpolation when upsampling need 4 bytes to store a single 16 bit stereo sample

	CSoundDecomp *decomp;   // The current decompression state

public:
	CSoundChannel();
	~CSoundChannel();

	void Stop();

	BOOL IsPlaying() { return mixer != 0 && !remove; }

	S32 ElapsedFrames();	// return the number of frames that have been played in this stream

	void AddRef() { refCount++; }
	void Release() { FLASHASSERT(refCount>0); if ( --refCount == 0 ) delete this; }
};

/*! Class CSoundMix continually mixes the sound output stream. It is the parent
 *	class of NativeSoundMix which actually talks to the hardware device.
 */
class CSoundMix {
public:
	CSound mixFormat;

	// The sound clock...
	int period;

	CSoundChannel* firstChannel;

	// Sound Buffer
	S32 bufferDuration;	// the length of the sound buffer - set from bufferSamples44
	int nSilent;		// the number of consecutive silent buffers sent

// 	S32 bufferSamples44;// number of samples in a buffer for a 44khz rate -this controls other settings
 	int nBuffers;
	BOOL firstPlay;		// set to indicate that we have not yet measures the buffer counts on this machine
	S32 bufferSamples;	// number of samples in a buffer
	S32 bufferBytes;
	void* buffer[MAX_SOUND_BUFFERS];// the sound buffers that we fill

	S32 scratchBytes;
	S32 nextScratch;
	void* scratchBuf[MAX_SOUND_CHANNELS];	// buffers for holding sound before mixing

	WaveHeader waveHdr[MAX_SOUND_BUFFERS];

	// We should add a single MIDI channel here...

public:
	//! Deletes all the sound buffers.
	void FreeBuffers();
	/*! Allocates 'nBuffers' buffers, each of size bufferBytes. Sets up the waveHdr structure
		associated with the buffers.
	 */
	void AllocBuffers();
	/*!	Increases the buffer count by one, allocates the memory for that buffer, and sets up the
	    waveHdr structure. */
	void EnlargeBuffers();

	/*!	The scratch is a block of memory used for mixing sound channels. */
	void* GetScratch();
	/*! */
	void ReleaseScratch() { nextScratch = 0; }

	/*	SendBuffer is called either on callback time or when the device in opened. In either case
		it needs to be protected by a critical section. 
	 */
// 	void SendBuffer(int i, BOOL silence = false);

	/*!	BuildBuffer mixes and prepares the specified buffer. The next chunk of sound that is
	 *	waiting to be played is mixed to the buffer specified by bufferId. After this call, 
	 *	the buffer should be sent to be sent to the sound card.
	 */
	void BuildBuffer( int bufferId );

	/*!	Open device goes through the following sequence of events:<br>
	 *	1) Enters a critical section<br>
	 *	2) Gets the device capabilities<br>
	 *	3) Calls NativeSoundMix::OpenNativeDevice<br>
	 *  4) Allocates and buffers calls BuildAndWrite<br>
	 *	5) Calls NativeSoundMix::BuffersReady<br>
	 */
	void OpenDevice();

	/*! Calls through to NativeSoundMix::CloseNativeDevice.
	 */
	void CloseDevice();

public:
	//! Has the same functionality as a constructor - done so global does not need _atexit
	void Construct();	
	//! Same functionality as a destructor.
	void Destruct();

// 	void SetFormat(int);

	/*! AddSound takes a pointer to a CSoundChannel and prepares the CSoundChannel for playing. 
		It then enters a critical section, and adds the sound to the list of sounds being played.
		If the sound device is not open, AddSound will open it.
	 */
	void AddSound(CSoundChannel*);

	void RemoveSound(CSoundChannel*);
	void RemoveTagged(U32);
	void RemoveAll();
	void PurgeList();

	BOOL SoundPlaying(CSound*);	// return true if the sound is playing
	void FreeSound(CSound*);	// stop any instances of this sound
	
	void FreeCache() { if ( !IsDeviceOpen() ) FreeBuffers(); }

	/*!	Given a buffer index, marks that buffer as completed playing.
	 */
	void MarkBufferComplete(int i);			// on callback time

	virtual void NativeConstruct()= 0;
	virtual void NativeDestruct() = 0;
	virtual void EnterCritical() = 0;
	virtual void LeaveCritical() = 0;
	virtual void BuildAndWrite( WaveHeader* hdr, BOOL silence ) = 0;
	virtual void GetNativeSoundCap( S32* format ) = 0;
	virtual BOOL OpenNativeDevice( CSound* format ) = 0;
	virtual void BuffersReady() = 0;
	virtual void CloseNativeDevice() = 0;
	virtual BOOL IsDeviceOpen() = 0;
	virtual void BuffersUpdated() = 0;
};

void ConvertSound(void* samples, S32 nSamples44, int srcFormat, int dstFormat, void* prevSample);
inline void ConvertSound(void* samples, S32 nSamples44, int srcFormat, int dstFormat) 
{// If they don't specify a prevSample, just use the first sample
	ConvertSound(samples, nSamples44, srcFormat, dstFormat, samples);
}

void ApplyEnvelope(int format, void* samples, S32 nSamples44, S32 time44, SoundEnvelope* envelope);


#else
	// No sound support
	inline void PollSound() {}
#endif

#endif
