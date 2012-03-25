/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright © Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 
****************************************************************************/

// #include "stdafx.h"

#ifdef SOUND

#include "memory.h"

#include "sndcodec.h"

#include "sndmix.h"
#include "mp3decoder.h"

#ifdef _CELANIMATOR
#include "srecord.h"
#endif

// 
// ADPCM tables
//

static const int indexTable2[2] = {
    -1, 2, 
};

// Is this ok?
static const int indexTable3[4] = {
    -1, -1, 2, 4,
};

static const int indexTable4[8] = {
    -1, -1, -1, -1, 2, 4, 6, 8,
};

static const int indexTable5[16] = {
	-1, -1, -1, -1, -1, -1, -1, -1, 1, 2, 4, 6, 8, 10, 13, 16, 
};

static const int* indexTables[] = {
	indexTable2,
	indexTable3,
	indexTable4,
	indexTable5 
};

static const int stepsizeTable[89] = {
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
    19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
    50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
    130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
    337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
    876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
    2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
    5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};


// swaps the byteorder of count words
void SwapBytes(void *dst, S32 count)
{
	U8* b = (U8*) dst;
	int n = count;
    U8  t;

    while ( n-- ) 
    {
		t = b[0];
		b[0] = b[1];
		b[1] = t;
		b += 2;
	}
}


#ifdef _CELANIMATOR

//
// The Compressor
//

CSoundComp::CSoundComp(CSound* snd, S32 nb)
{
//	FLASHASSERT(snd->CompressFormat() == sndCompressADPCM);
	isStereo = snd->Stereo();
	is8Bit = snd->Is8Bit();

	nBits = nb;
	nSamples = 0;

	len = 0;
	recorder = 0;
	archive = 0;

	bitBuf = 0;
	bitPos = 0;
}

void CSoundComp::WriteBits()
{
	if ( recorder ) {
		// Actually write the bits...
		while ( bitPos >= 8 ) {
			recorder->PutByte((U8)(bitBuf >> (bitPos-8)));
			bitPos -= 8;
			len++;
		}

	} else if ( archive ) {
		// Actually write the bits...
		while ( bitPos >= 8 ) {
			*archive << (U8)(bitBuf >> (bitPos-8));
			bitPos -= 8;
			len++;
		}

	} else {
		// Just counting...
		len += bitPos/8;
		bitPos &= 0x7;
	}
}

void CSoundComp::Flush()
{
	WriteBits();
	if ( bitPos > 0 ) {
		PutBits(0, 8-bitPos);
		WriteBits();
	}
}

void CSoundComp::Compress16(S16* src, S32 n)
{
	if ( nSamples == 0 ) {
		// Emit the compression settings
		PutBits(nBits-2, 2);
	}

	int sn = isStereo ? 2 : 1;
	const int* indexTable = indexTables[nBits-2];
	while ( n-- > 0 ) {
		nSamples++;
		if ( (nSamples & 0xfff) == 1 ) {
			// We emit a header every 4096 samples so we can seek quickly
			for ( int i = 0; i < sn; i++ ) {
				// Pick an initial index value
				S32 d = abs(src[sn] - src[0]);
				int k = 0;
				while ( k < 63 && stepsizeTable[k] < d )
					k++;

				PutBits(valpred[i] = *src++, 16);
				PutBits(index[i] = k, 6);
			}

		} else {
			// Generate a delta value
			for ( int i = 0; i < sn; i++ ) {
				/* Step 1 - compute difference with previous value */
				S32 diff = *src++ - valpred[i];	/* Difference between val and valprev */
				int sign;
				if ( diff < 0 ) {
					sign = 1<<(nBits-1);
					diff = -diff;
				} else {
					sign = 0;
				}

				/* Step 2 - Divide and clamp */
				/* Note:
				** This code *approximately* computes:
				**    delta = diff*4/step;
				**    vpdiff = (delta+0.5)*step/4;
				** but in shift step bits are dropped. The net result of this is
				** that even if you have fast mul/div hardware you cannot put it to
				** good use since the fixup would be too expensive.
				*/
				int step = stepsizeTable[index[i]];		/* Stepsize */
				S32 delta = 0;				/* Current adpcm output value */
				S32 vpdiff = 0;			/* Current change to valpred */
				
				int k = 1<<(nBits-2);
				do {
					if ( diff >= step ) {
						delta |= k;
						diff -= step;
						vpdiff += step;
					}
					step >>= 1;	
					k >>= 1;
				} while ( k );
				vpdiff += step;	// add the 0.5

				/* Step 3 - Update previous value */
				if ( sign )
					valpred[i] -= vpdiff;
				else
					valpred[i] += vpdiff;

				/* Step 4 - Clamp previous value to 16 bits */
				if ( valpred[i] != (S16)valpred[i] )
					valpred[i] = valpred[i] < 0 ? -32768 : 32767;
				FLASHASSERT(valpred[i] <=32767 && valpred[i] >= -32768);

				/* Step 5 - Assemble value, update index and step values */
				index[i] += indexTable[delta];
				if ( index[i] < 0 ) index[i] = 0;
				else if ( index[i] > 88 ) index[i] = 88;

				delta |= sign;
				
				/* Step 6 - Output value */
				PutBits(delta, nBits);
			}
		}
	}
}

inline void Filter8to16(U8* src, S16* dst, S32 n)
// Can work in place
{
	src += n;
	dst += n;
	while ( n-- )
		*(--dst) = ((S16)*(--src) - 128) << 8;
}

void CSoundComp::Compress(void* src, S32 n)
{
	if ( is8Bit ) {
		S16 buf[4096];
		U8* s = (U8*)src;
		while ( n > 0 ) {
			// Expand to 16 bit and compress
			S32 nb = min((S32)4096, n);
			Filter8to16(s, buf, nb);
			Compress16(buf, nb);
			n -= nb;
			s += nb;
		}

	} else {
		Compress16((S16*)src, n);
	}
}

#endif


// 
// The Decompressor
//

void CAdpcmDecomp::Setup(CSound* snd, BOOL reset)
{
	FLASHASSERT(snd->CompressFormat() == sndCompressADPCM);

	stereo = snd->Stereo();
	src = (U8 *)snd->samples;
/*
	{// Calculate the compressed size
		S32 nblocks = snd->nSamples >> 12;
		S32 k = nblocks * ((16 + 6) + nBits * 0xFFF);
		int nFrac = snd->nSamples & 0xFFF;
		if ( nFrac )
			k += (16 + 6) + nBits * nFrac;
		if ( stereo ) k *= 2;
		srcSize = (k+7)/8;
	}
*/
	nBits = 0;	// flag that it is not inited
	nSamples = 0;

	bitPos = 0; 
	bitBuf = 0; 

#ifdef _CELANIMATOR
	archive = 0;
#endif
}

void CAdpcmDecomp::FillBuffer()
{
#ifdef _CELANIMATOR
	if ( archive ) {
		while ( bitPos <= 24 ) {
			U8 s;
			*archive >> s;
			bitBuf = (bitBuf<<8) | s;
			bitPos += 8;
		}
	} else
#endif
	{
		while ( bitPos <= 24 /*&& srcSize > 0*/ ) {
			bitBuf = (bitBuf<<8) | *src++;
			bitPos += 8;
			//srcSize--;
		}
	}
}

void CAdpcmDecomp::SkipBits(S32 n)
{
	if ( n <= 32 ) {
		// Skip a small chunk
		while ( n > 0 ) {
			int k = (int)Min(16,n);
			GetBits(k);
			n -= k;
		}
	} else {
		// Skip a larger chunk

		// Skip the current buffer
		n -= bitPos;
		bitPos = 0;

		// Skip whole bytes
		S32 k = n/8;
		src += k;
		//srcSize -= k;

		// Skip the trailing fraction
		GetBits((int)n & 0x7);
	}
}

void CAdpcmDecomp::Decompress(S16* dst, S32 n)
{
	if ( nBits == 0 ) {
		// Get the compression header
		nBits = (int)GetBits(2)+2;
	}

	if ( !dst ) {
		// Skip samples
		S32 nInBlock = nSamples & ~0xFFF;
		if ( nInBlock > 0 && n > nInBlock+nSamples ) {
			// Skip all of the rest of the current block
			nSamples += nInBlock;
			n -= nInBlock;
			S32 k = nInBlock*nBits;
			if ( stereo ) k *= 2;
			SkipBits(k);
		}

		{// Skip whole blocks
			S32 nblocks = n >> 12;
			// The first sample is 
			S32 k = nblocks * ((16 + 6) + nBits * 0xFFF);
			if ( stereo ) k *= 2;
			SkipBits(k);
			n = n & 0xFFF;
		}

		{// Skip a portion of the current block
			S16 buf[1024];
			S32 limit = stereo ? 512 : 1024;
			while ( n > 0 ) {
				S32 k = Min(limit, n);
				Decompress(buf, k);
				n -= k;
			}
		}
	} else {
		// Decompress the samples
		const int* indexTable = indexTables[nBits-2];
		int k0 = 1 << (nBits-2);
		int signmask = 1 << (nBits-1);
		if ( !stereo ) {
			// Optimize for mono
			S32 vp = valpred[0];	// maybe these can get into registers...
			int ind = index[0];
			S32 ns = nSamples;
			while ( n-- > 0 ) {
				ns++;
				if ( (ns & 0xFFF) == 1 ) {
					// Get a new block header
					*dst++ = (S16)(vp = GetSBits(16));
					ind = (int)GetBits(6); // The first sample in a block does not have a delta
				} else {
					// Process a delta value
					int delta = (int)GetBits(nBits);

					// Compute difference and new predicted value 
					// Computes 'vpdiff = (delta+0.5)*step/4'
					int step = stepsizeTable[ind];
					S32 vpdiff = 0;
					int k = k0;
					do {
						if ( delta & k )
							vpdiff += step;
						step >>= 1;
						k >>= 1;
					} while ( k );
					vpdiff += step;	// add 0.5

					if ( delta & signmask ) // the sign bit
						vp -= vpdiff;
					else
						vp += vpdiff;

					// Find new index value 
					ind += indexTable[delta&(~signmask)];
					if ( ind < 0 ) ind = 0;
					else if ( ind > 88 ) ind = 88;

					// clamp output value 
					if ( vp != (S16)vp )
						vp = vp < 0 ? -32768 : 32767;
					FLASHASSERT(vp <=32767 && vp >= -32768);

					/* Step 7 - Output value */
					*dst++ = (S16)vp;
				}
			}
			valpred[0] = vp;
			index[0] = ind;
			nSamples = ns;
		} else {
			int sn = stereo ? 2 : 1;
			// Stereo
			while ( n-- > 0 ) {
				nSamples++;
				if ( (nSamples & 0xFFF) == 1 ) {
					// Get a new block header
					for ( int i = 0; i < sn; i++ ) {
						*dst++ = (S16)(valpred[i] = GetSBits(16));
						index[i] = (int)GetBits(6); // The first sample in a block does not have a delta
					}
				} else {
					// Process a delta value
					for ( int i = 0; i < sn; i++ ) {
						int delta = (int)GetBits(nBits);

						// Compute difference and new predicted value 
						// Computes 'vpdiff = (delta+0.5)*step/4'
						int step = stepsizeTable[index[i]];
						S32 vpdiff = 0;
						int k = k0;
						do {
							if ( delta & k )
								vpdiff += step;
							step >>= 1;
							k >>= 1;
						} while ( k );
						vpdiff += step;	// add 0.5

						if ( delta & signmask ) // the sign bit
							valpred[i] -= vpdiff;
						else
							valpred[i] += vpdiff;

						// Find new index value 
						index[i] += indexTable[delta&(~signmask)];
						if ( index[i] < 0 ) index[i] = 0;
						else if ( index[i] > 88 ) index[i] = 88;

						// clamp output value 
						if ( valpred[i] != (S16)valpred[i] )
							valpred[i] = valpred[i] < 0 ? -32768 : 32767;
						FLASHASSERT(valpred[i] <=32767 && valpred[i] >= -32768);

						/* Step 7 - Output value */
						*dst++ = (S16)valpred[i];
					}
				}
			}
		}
	}
}

#endif

#ifdef SOUND

#if !defined(GENERATOR)

//
// A "stub" decompressor for uncomressed PCM sound
//

void CPCMDecomp::Setup(CSound* snd, BOOL reset)
{
	samples = snd->samples;
	bytesPerBlock = snd->BytesPerBlock();
	pos = 0;
    #ifdef _MAC
    swapBytes = (snd->CompressFormat() == sndCompressNoneI) && !snd->Is8Bit();
	#else
	swapBytes = false;
	#endif
}

void CPCMDecomp::Decompress(S16* dst, S32 n)
{
	if ( dst ) {
// 		hmemcpy(dst, (U8 *)samples + pos*bytesPerBlock, n*bytesPerBlock);
		memcpy(dst, (U8 *)samples + pos*bytesPerBlock, n*bytesPerBlock);

      #ifdef _MAC
        if ( swapBytes )
            SwapBytes(dst, n*bytesPerBlock / 2);
	  #endif
	}

	pos += n;
}


//
// Create a decompressor object based on a compression format id
//

CSoundDecomp* CreateDecompressor(int compressFormat)
{
	switch ( compressFormat ) {
		case sndCompressNone:
        case sndCompressNoneI:
			return new CPCMDecomp();
		case sndCompressADPCM:
			return new CAdpcmDecomp();
		#ifdef FLASHMP3
		case sndCompressMP3:
			return new CMp3Decomp();
		#endif
	}
	return 0;
}

#endif	// !defined(GENERATOR)
#endif