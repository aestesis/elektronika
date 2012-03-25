/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright © Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 
****************************************************************************/

// #include "stdafx.h"

#ifdef SOUND
#ifdef FLASHMP3

#ifndef min
#define min(a, b) ((a)<(b)?(a):(b))
#endif

#include "stdafx.h"
#include "mp3decoder.h"
#include "sndmix.h"
#include "mpegbitstream.h"

///////////////////////////////////////////
//
// mp3 decoder used by flash
//
CMp3Decomp::CMp3Decomp()
{
    src = 0;            // mpeg src data
    bufLength = 0;      // number of bytes last filled with
    bufIndex = 0;       // index into the out buffer
    srcIndex = 0;
    snd = 0;
    mp3Handle = 0;

    //
    // open decoder instance
    //
#ifdef WINDOWS
    if (!SSC_SUCCESS(mp3decOpen(&mp3Handle, 0, 0, 0, 1)))
#else
    if (!SSC_SUCCESS(mp3decOpen(&mp3Handle, 0, 0, 0, 0)))
#endif
        mp3Handle = 0;
}

CMp3Decomp::~CMp3Decomp()
{
    if (mp3Handle)
        mp3decClose(mp3Handle);

    mp3Handle = 0;
}

void CMp3Decomp::Setup(CSound* snd, BOOL reset)
{
    FLASHASSERT(snd->CompressFormat() == sndCompressMP3);

	src = (U8 *) snd->samples;
    bufLength = 0;      // number of bytes last filled with
    bufIndex = 0;       // index into the out buffer
    srcIndex = 0;
    this->snd = snd;

    if (reset)
        mp3decReset(mp3Handle);

    // skip past the mp3 compression delay
    Decompress(0, snd->delay);
}

//////////////////////////////////////////////////
//
// Get buffered data. If return value == input n
// then we had enough data in the buffer
//
long CMp3Decomp::GetBufferedData(S8 *dst, S32 n)
{
    int bytesToCopy = n < (bufLength - bufIndex) ? n : (bufLength - bufIndex);

    // dst can bee 0 during seeking
    if (dst)
        memcpy(dst, &pcmBuf[bufIndex], bytesToCopy);

    // adjust the callers index and the current buff pointer
    bufIndex += bytesToCopy;

    if (bufIndex >= bufLength)
        bufIndex = bufLength = 0;

    return bytesToCopy;
}

void CMp3Decomp::Decompress(S16* dstWord, S32 nSamples) 
{
    long            bytesCleared;
    int             outBytes;
    S32             nBytes  = nSamples * snd->BytesPerBlock();
    S8              *dstByte = (S8 *) dstWord;
    SSC             ssc;    // status code (returned by decoder interface functions)
    BOOL            seeking = (!dstWord) ? (true) : false;

    // check first if there is any buffered data
    if ((bytesCleared = GetBufferedData(dstByte, nBytes)) == nBytes)
        goto exit_gracefully;
    else if (!mp3Handle)
    {
        // fill with silence
        if (dstByte)
            memset(dstByte, 0x00, nBytes);

        goto exit_gracefully;
    }
    else
    {
        nBytes -= bytesCleared;

        // dst byte can be zero during seeking
        if (dstByte)
            dstByte += bytesCleared;
    }

    // Loop until error occurs or bytes are saved for next pass
	while(nBytes > 0)
	{
        outBytes = 0;
        ssc = mp3decDecode(mp3Handle, pcmBuf, kBufSize, &outBytes);
        bufLength = outBytes;
        
        switch (ssc)
		{
			case SSC_W_MPGA_SYNCEOF:
                // are we actually ever going to get here ?
                mp3decSetInputEof(mp3Handle);

                // fill with silence
                if (dstByte)
                    memset(dstByte, 0x00, nBytes - outBytes);

                goto exit_gracefully;
				break;

            // informational message
            case SSC_I_MPGA:
            case SSC_I_MPGA_CRCERROR:
            case SSC_I_MPGA_NOMAINDATA:
			case SSC_OK:
			{
				// Everythings just fine...
				// check for outputted data
				if( outBytes > 0 )
				{
                    // this should work out for us. If there is more buffered data
                    // then we need, nBytes will be <= 0 else the GetBufferedData()
                    // will clear out the bufOut
                    bytesCleared = GetBufferedData(dstByte, nBytes);
                    nBytes -= bytesCleared;
    
                    // dst byte can be zero during seeking
                    if (dstByte)
                        dstByte += bytesCleared;
                }
                else
                {
                    mp3decSetInputEof(mp3Handle);
                    goto exit_gracefully;
                }
				break;
			}

            // Fill the decoder input buffer
            case SSC_W_MPGA_SYNCSEARCHED:
			case SSC_W_MPGA_SYNCNEEDDATA:
			{
				int acceptCount; 
                int bytesRefilled;
                int samplesToSkip = nBytes / snd->BytesPerBlock();

                // see if we are seeking and data has at least four bytes left
                if (seeking && samplesToSkip > 0)
                {
                    CMpegHeader mp3Hdr;
                    
                    // skip full frames of sound
                    while (true)
                    {
                        // make sure we can read the frame header
                        if ((snd->dataLen - srcIndex) < 4)
                            break;
                      
                        // read the frame header
                        unsigned long hdr = src[srcIndex]     << 24 |
                                            src[srcIndex + 1] << 16 |
                                            src[srcIndex + 2] <<  8 |
                                            src[srcIndex + 3];

                        //  init header variables
                        mp3Hdr.FromInt(hdr);

                        int frameLen = mp3Hdr.GetFrameLen();
                        int samplesPerFrame = mp3Hdr.GetSamplesPerFrame();

                        // make sure we have a min of delay samples to skip
                        //
                        // NOTE:
                        // during seeking start decompression at least delay samples
                        // before the destination point. This allows the mp3
                        // decoder to start interpolating before we get to the
                        // actual data to be played.
                        if (snd->delay + samplesPerFrame >= samplesToSkip)
                            break;

                        // make sure we don't run out of our data
                        if ((srcIndex + frameLen / 8) > snd->dataLen)
                        {
                            FLASHASSERT(0);
                            break;
                        }

                        // skip the frame
                        samplesToSkip -= samplesPerFrame;
                        srcIndex += (frameLen / 8);
                        nBytes -=  (samplesPerFrame * snd->BytesPerBlock()); 
                    }
                }

                mp3decGetInputFree(mp3Handle, &acceptCount);
				if (acceptCount > 0)
				{
                    /* refill input buffer */
                    acceptCount = min((long)(snd->dataLen - srcIndex), (long) acceptCount);
					mp3decFill(mp3Handle, &src[srcIndex], acceptCount, &bytesRefilled);

                    // we ran out of source data. We are probably sound streaming
                    // so we might get the data in the next stream block
                    if (bytesRefilled == 0)
                        goto exit_gracefully;

                    srcIndex += bytesRefilled;
                }
                else
                {
                    mp3decSetInputEof(mp3Handle);
                    goto exit_gracefully;
                }
			}
            break;

            case SSC_W_MPGA_SYNCLOST:
	            FLASHASSERT(0);
                mp3decSetInputEof(mp3Handle);
                goto exit_gracefully;

            default:
	            FLASHASSERT(0);
                mp3decClose(mp3Handle);
                mp3Handle = 0;
                goto exit_gracefully;
				break;
        }
	}

exit_gracefully:
    ;
}

#endif
#endif