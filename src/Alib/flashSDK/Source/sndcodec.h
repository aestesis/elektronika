/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright © Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 
****************************************************************************/

#ifdef SOUND

#ifndef SNDCODEC_INCLUDED
#define SNDCODEC_INCLUDED

#ifndef FIXED_INCLUDED
#include "fixed.h"
#endif

class CSound;

#ifdef _CELANIMATOR

class Recorder;

class CSoundComp 
{
private:
	BOOL isStereo;
	BOOL is8Bit;
	int nBits;			// number of bits in each sample

	S32 nSamples;		// samples compressed so far

	S32 valpred[2];		// ADPCM state
	int index[2];

	// The Destination
	S32 len;			// default is to just calculate the size
	CArchive* archive;
	Recorder* recorder;

	// State for writing bits
	U32 bitBuf;
	int bitPos;


////////////////////////////////////////
// Methods
////////////////////////////////////////

public:
	CSoundComp(CSound* snd, S32 nb);
	virtual ~CSoundComp() { FLASHASSERT(bitPos == 0); }

	void Attach(Recorder* r) { recorder = r; }
	void Flush();
	void Compress(void* src, S32 n);

private:
	void Attach(CArchive* ar) { archive = ar; }
	void Compress16(S16* src, S32 n);

	// Write variable width bit fields
	void WriteBits();	// empty the buffer of whole bytes
	void PutBits(S32 v, int n) {
		FLASHASSERT(n<=16);
		if ( bitPos + n > 32 )
			WriteBits();
		bitBuf = (bitBuf<<n) | (v & ~(0xFFFFFFFFL<<n));
		bitPos += n;
	}
};

#endif


//////////////////////////////////////////////////////////////////////////////////
//
// all of our decompresors will inherit from this class
//

class CSoundDecomp 
{
public:
    virtual void Setup(CSound* snd, BOOL reset) {FLASHASSERT(0);};
    virtual void Decompress(S16* dst, S32 n) {FLASHASSERT(0);};	   // must decompress into the format specified by snd
    virtual ~CSoundDecomp() {};
};

// Create a decompressor object based on a compression format id
CSoundDecomp* CreateDecompressor(int compressFormat);

//////////////////////////////////////////////////////////////////////////////////
//
// pcm decompressor class - this is raw uncompressed sound
//
class CPCMDecomp : public CSoundDecomp {
public:
	S32     pos;
	S32     bytesPerBlock;
	void*   samples;
    BOOL    swapBytes;

public:
    virtual void Setup(CSound* snd, BOOL reset = false);
    void Decompress(S16* dst, S32 n);	// if dst == 0, just skip the requested samples
};


//////////////////////////////////////////////////////////////////////////////////
//
// adpcm decompressor class
//
class CAdpcmDecomp : public CSoundDecomp
{
private:
	// Destination format - note we always decompress to 16 bit
	BOOL stereo;
	int nBits;		// number of bits in each sample

	S32 valpred[2];	// Current state
	int index[2];

	S32 nSamples;	// number of samples decompressed so far

	// Parsing Info
	U8 * src;
//	S32 srcSize;
	U32 bitBuf;	// this should always contain at least 24 bits of data
	int bitPos;

	void FillBuffer();
	void SkipBits(S32 n);
	S32 GetBits(int n) {
		if ( bitPos < n )
			FillBuffer();
		FLASHASSERT(bitPos >= n);
		S32 v = ((U32)bitBuf << (32-bitPos)) >> (32-n);
		bitPos -= n;
		return v;
	}
	S32 GetSBits(int n) {
		if ( bitPos < n )
			FillBuffer();
		FLASHASSERT(bitPos >= n);
		S32 v = ((S32)bitBuf << (32-bitPos)) >> (32-n);
		bitPos -= n;
		return v;
	}

public:

	#ifdef _CELANIMATOR
	CArchive* archive;
	void Attach(CArchive* ar) { archive = ar; }
	#endif

    virtual void Setup(CSound* snd, BOOL reset = false);
	void Decompress(S16* dst, S32 n);	// return number of good samples
};

#endif

#endif
