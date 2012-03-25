/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright © Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 
****************************************************************************/


#ifdef SOUND

#ifndef MP3CODEC_INCLUDED
#define MP3CODEC_INCLUDED

#include "sndcodec.h"
#include "mp3decifc.h"

///////////////////////////////////////////
//
// mp3 decoder used by flash
//


// destination data buffer (pcm)
// the largest possible output for one mpeg frame
// is 1152 16-bit stereo samples
const int kBufSize = 1024*8;

class CMp3Decomp : public CSoundDecomp
{
private:
    U8              *src;               // mpeg src data
    int             srcIndex;           // index into the source data to read
    U8              pcmBuf[kBufSize];   // buffered output data 
    MP3DEC_HANDLE   mp3Handle;          // handle, identifying the decoder instance
    int             bufLength;          // number of bytes last filled with
    int             bufIndex;           // index into the out buffer
    CSound          *snd;

    // get buffered data in bufOut. Return number of bytes written
    long GetBufferedData(S8 *dst, S32 n);

             
public:
	CMp3Decomp(void);
    ~CMp3Decomp(void);

    virtual void Setup(CSound* snd, BOOL reset = false);
    void Decompress(S16* dst, S32 nSamples);    // return number of good samples
};

#endif
#endif
