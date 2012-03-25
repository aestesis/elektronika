/*****************************************************************************
 * 
 * Sampled sound tempo changer/time stretch algorithm. Changes the sound tempo 
 * while maintaining the original pitch by using a time domain WSOLA-like 
 * method with several performance-increasing tweaks.
 *
 * Note : MMX optimized functions reside in a separate, platform-specific 
 * file, e.g. 'mmx_win.cpp' or 'mmx_gcc.cpp'
 *
 * Author        : Copyright (c) Olli Parviainen 
 * Author e-mail : oparviai @ iki.fi
 * File created  : 13-Jan-2002
 *
 * Last changed  : $Date: 2003/12/27 10:00:52 $
 * File revision : $Revision: 1.14 $
 *
 * $Id: TDStretch.cpp,v 1.14 2003/12/27 10:00:52 Olli Exp $
 *
 * License :
 * 
 *  SoundTouch sound processing library
 *  Copyright (c) Olli Parviainen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *****************************************************************************/


#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include <limits.h>
#include <math.h>
#include <assert.h>

#include "STTypes.h"
#include "cpu_detect.h"
#include "TDStretch.h"

using namespace soundtouch;

#ifndef min
#define min(a,b) ((a > b) ? b : a)
#define max(a,b) ((a < b) ? b : a)
#endif


/*****************************************************************************
 *
 * Constant definitions
 *
 *****************************************************************************/


#define MAX_SCAN_DELTA      124

// Table for the hierarchical mixing position seeking algorithm
int scanOffsets[4][24]={
    { 124,  186,  248,  310,  372,  434,  496,  558,  620,  682,  744, 806, 
      868,  930,  992, 1054, 1116, 1178, 1240, 1302, 1364, 1426, 1488,   0}, 
    {-100,  -75,  -50,  -25,   25,   50,   75,  100,    0,    0,    0,   0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,   0},
    { -20,  -15,  -10,   -5,    5,   10,   15,   20,    0,    0,    0,   0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,   0},
    {  -4,   -3,   -2,   -1,    1,    2,    3,    4,    0,    0,    0,   0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,   0}};

/*****************************************************************************
 *
 * Implementation of the class 'TDStretch'
 *
 *****************************************************************************/


TDStretch::TDStretch() : FIFOProcessor(&outputBuffer)
{
    bQuickseek = FALSE;
    channels = 2;
    bMidBufferDirty = FALSE;

    setParameters(44100, DEFAULT_SEQUENCE_MS, DEFAULT_SEEKWINDOW_MS, DEFAULT_OVERLAP_MS);

    pMidBuffer = new SAMPLETYPE[2 * overlapLength];
    memset(pMidBuffer, 0, 2 * overlapLength);

    pRefMidBufferUnaligned = new SAMPLETYPE[2 * overlapLength + 16 / sizeof(SAMPLETYPE)];
    // ensure that 'pRefMidBuffer' is aligned to 16 byte boundary for efficiency
    pRefMidBuffer = (SAMPLETYPE *)((((ulong)pRefMidBufferUnaligned) + 15) & -16);

    setTempo(1.0f);
}




TDStretch::~TDStretch()
{
    delete[] pMidBuffer;
    delete[] pRefMidBufferUnaligned;
}


    
// Calculates the x having the closest 2^x value for the given value
static int _getClosest2Power(double value)
{
    return (int)(log((double)value) / log(2.0) + 0.5);
}



// Sets routine control parameters. These control are certain time constants
// defining how the sound is stretched to the desired duration.
//
// 'sampleRate' = sample rate of the sound
// 'sequenceMS' = one processing sequence length in milliseconds (default = 82 ms)
// 'seekwindowMS' = seeking window length for scanning the best overlapping 
//      position (default = 28 ms)
// 'overlapMS' = overlapping length (default = 12 ms)

void TDStretch::setParameters(uint sampleRate, uint sequenceMS, 
                              uint seekWindowMS, uint overlapMS)
{
    this->sampleRate = sampleRate;
    this->sequenceMs = sequenceMS;
    this->seekWindowMs = seekWindowMS;
    this->overlapMs = overlapMs;

    seekLength = (sampleRate * seekWindowMS) / 1000;
    seekWindowLength = (sampleRate * sequenceMS) / 1000;
    overlapDividerBits = _getClosest2Power((sampleRate * overlapMS) / 1000.0);

    if (overlapDividerBits > 9) overlapDividerBits = 9;
    if (overlapDividerBits < 4) overlapDividerBits = 4;

    overlapLength = (uint)pow(2.0, (double)overlapDividerBits);

    slopingDividerBits = 2 * (overlapDividerBits - 1) - 1;
    slopingDivider = (uint)pow(2.0, (double)slopingDividerBits);

    maxOffset = seekLength;

}



/// Get routine control parameters, see setParameters() function.
/// Any of the parameters to this function can be NULL, in such case corresponding parameter
/// value isn't returned.
void TDStretch::getParameters(uint *pSampleRate, uint *pSequenceMs, uint *pSeekWindowMs, uint *pOverlapMs)
{
    if (pSampleRate)
    {
        *pSampleRate = sampleRate;
    }

    if (pSequenceMs)
    {
        *pSequenceMs = sequenceMs;
    }

    if (pSeekWindowMs)
    {
        *pSeekWindowMs = seekWindowMs;
    }

    if (pOverlapMs)
    {
        *pOverlapMs = overlapMs;
    }
}


// Overlaps samples in 'midBuffer' with the samples in 'input'
void TDStretch::overlapMono(SAMPLETYPE *output, const SAMPLETYPE *input) const
{
    int i, itemp;

    for (i = 0; i < (int)overlapLength ; i ++) 
    {
        itemp = overlapLength - i;
        output[i] = (input[i] * i + pMidBuffer[i] * itemp ) / overlapLength;    // >> overlapDividerBits;
    }
}



// Overlaps samples in 'midBuffer' with the samples in 'input'. The 'Stereo' 
// version of the routine.
void TDStretch::overlapStereo(SAMPLETYPE *output, const SAMPLETYPE *input) const
{
    int i;
    SAMPLETYPE temp;
    uint cnt2;

    for (i = 0; i < (int)overlapLength ; i ++) 
    {
        temp = (SAMPLETYPE)(overlapLength - i);
        cnt2 = 2 * i;
        output[cnt2] = (input[cnt2] * i + pMidBuffer[cnt2] * temp )  / overlapLength; //>> overlapDividerBits;
        output[cnt2 + 1] = (input[cnt2 + 1] * i + pMidBuffer[cnt2 + 1] * temp ) / overlapLength; //>> overlapDividerBits;
    }
}



// Slopes the amplitude of the 'midBuffer' samples
void TDStretch::slopeReferenceSamplesStereo()
{
    int i, cnt2;
    LONG_SAMPLETYPE temp1, temp2;

    for (i=0 ; i < (int)overlapLength ;i ++) 
    {
        temp1 = (LONG_SAMPLETYPE)i * (overlapLength - i);
        cnt2 = i * 2;
        temp2 = (pMidBuffer[cnt2] * temp1) / slopingDivider;   // >> slopingDividerBits;
        pRefMidBuffer[cnt2] = (SAMPLETYPE)temp2;
        temp2 = (pMidBuffer[cnt2 + 1] * temp1) / slopingDivider;   // >> slopingDividerBits;
        pRefMidBuffer[cnt2 + 1] = (SAMPLETYPE)temp2;
    }
}



void TDStretch::clearMidBuffer()
{
    if (bMidBufferDirty) 
    {
        memset(pMidBuffer, 0, 2 * sizeof(SAMPLETYPE) * overlapLength);
        bMidBufferDirty = FALSE;
    }
}


void TDStretch::clearInput()
{
    inputBuffer.clear();
    clearMidBuffer();
}


// Clears the sample buffers
void TDStretch::clear()
{
    outputBuffer.clear();
    inputBuffer.clear();
    clearMidBuffer();
}



// Enables/disables the quick position seeking algorithm. Zero to disable, nonzero
// to enable
void TDStretch::enableQuickSeek(BOOL enable)
{
    bQuickseek = enable;
}


// Returns nonzero if the quick seeking algorithm is enabled.
BOOL TDStretch::isQuickSeekEnabled() const
{
    return bQuickseek;
}


// Seeks for the optimal overlap-mixing position.
uint TDStretch::seekBestOverlapPosition(const SAMPLETYPE *refPos)
{
    if (channels == 2) 
    {
        // stereo sound
        if (bQuickseek) 
        {
            return seekBestOverlapPositionStereoQuick(refPos);
        } 
        else 
        {
            return seekBestOverlapPositionStereo(refPos);
        }
    } 
    else 
    {
        // mono sound
        if (bQuickseek) 
        {
            return seekBestOverlapPositionMonoQuick(refPos);
        } 
        else 
        {
            return seekBestOverlapPositionMono(refPos);
        }
    }
}




// Overlaps samples in 'midBuffer' with the samples in 'inputBuffer' at position
// of 'ovlPos'.
inline void TDStretch::overlap(SAMPLETYPE *output, const SAMPLETYPE *input, uint ovlPos) const
{
    if (channels == 2) 
    {
        // stereo sound
        overlapStereo(output, input + 2 * ovlPos);
    } else {
        // mono sound.
        overlapMono(output, input + ovlPos);
    }
}



// Seeks for the optimal overlap-mixing position. The 'stereo' version of the
// routine
//
// The best position is determined as the position where the two overlapped
// sample sequences are 'most alike', in terms of the highest cross-correlation
// value over the overlapping period
uint TDStretch::seekBestOverlapPositionStereo(const SAMPLETYPE *refPos) 
{
    uint i;
    uint bestOffs;
    LONG_SAMPLETYPE bestCorr, corr;
    uint tempOffset;
    const SAMPLETYPE *compare;

    // Slopes the amplitudes of the 'midBuffer' samples
    slopeReferenceSamplesStereo();

    bestCorr = INT_MIN;
    bestOffs = 0;

    // Scans for the best correlation value by testing each possible position
    // over the permitted range.
    for (tempOffset = 0; tempOffset < seekLength; tempOffset ++) 
    {
        compare = refPos + 2 * tempOffset;

        // Calculates the cross-correlation value for the mixing position 
        // corresponding to 'tempOffset'
        corr = 0;
        for (i = 2; i < 2 * overlapLength; i += 2) 
        {
            LONG_SAMPLETYPE temp1, temp2;

            temp1 = pRefMidBuffer[i] * compare[i];
            temp2 = pRefMidBuffer[i + 1] * compare[i + 1];
#ifdef INTEGER_SAMPLES

            // integer samples, scale the internal result to prevent overflowing
            corr += (temp1 + temp2) >> overlapDividerBits;

#else   // INTEGER_SAMPLES
     
            // floating point samples, no need to scale internal result
            corr += (temp1 + temp2);

#endif  // INTEGER_SAMPLES
        }

        // Checks for the highest correlation value
        if (corr > bestCorr) 
        {
            bestCorr = corr;
            bestOffs = tempOffset;
        }
    }
    return bestOffs;
}




// Seeks for the optimal overlap-mixing position. The 'mono' version of the
// routine
//
// The best position is determined as the position where the two overlapped
// sample sequences are 'most alike', in terms of the highest cross-correlation
// value over the overlapping period
uint TDStretch::seekBestOverlapPositionMono(const SAMPLETYPE *refPos) 
{
    uint i;
    uint bestOffs;
    LONG_SAMPLETYPE bestCorr, corr;
    uint tempOffset;
    const SAMPLETYPE *compare;

    // Slopes the amplitude of the 'midBuffer' samples
    for (i = 0; i < overlapLength ;i ++) 
    {
        LONG_SAMPLETYPE temp1, temp2;

        // Note : 'i' is unsigned, this step's required to converts the result to signed!
        temp1 = (LONG_SAMPLETYPE)(i * (overlapLength - i));
        temp2 = (pMidBuffer[i] * temp1) / slopingDivider;  // >> slopingDividerBits;
        pRefMidBuffer[i] = (SAMPLETYPE)temp2;
    }

    bestCorr = INT_MIN;
    bestOffs = 0;


    // Scans for the best correlation value by testing each possible position
    // over the permitted range.
    for (tempOffset = 0; tempOffset < seekLength; tempOffset ++) 
    {
        compare = refPos + tempOffset;

        // Calculates correlation value for the mixing position corresponding
        // to 'tempOffset'
        corr = 0;
        for (i = 1; i < overlapLength; i ++) 
        {
            LONG_SAMPLETYPE temp;

            temp = pRefMidBuffer[i] * compare[i];

#ifdef INTEGER_SAMPLES

            // integer samples, scale the internal result to prevent overflowing
            corr += temp >> overlapDividerBits;

#else   // INTEGER_SAMPLES

            // floating point samples, no need to scale the internal result
            corr += temp;

#endif  // INTEGER_SAMPLES
        }

        // Checks for the highest correlation value
        if (corr > bestCorr) 
        {
            bestCorr = corr;
            bestOffs = tempOffset;
        }
    }
    return bestOffs;
}




// Seeks for the optimal overlap-mixing position. The 'stereo' version of the
// routine
//
// The best position is determined as the position where the two overlapped
// sample sequences are 'most alike', in terms of the highest cross-correlation
// value over the overlapping period
uint TDStretch::seekBestOverlapPositionStereoQuick(const SAMPLETYPE *refPos) 
{
    uint j,i;
    uint bestOffs;
    LONG_SAMPLETYPE bestCorr, corr;
    uint scanCount, corrOffset, tempOffset;
    const SAMPLETYPE *compare;

    // Slopes the amplitude of the 'midBuffer' samples
    slopeReferenceSamplesStereo();

    bestCorr = INT_MIN;
    bestOffs = 0;
    corrOffset = 0;
    tempOffset = 0;

    // Scans for the best correlation value using four-pass hierarchical search.
    //
    // The look-up table 'scans' has hierarchical position adjusting steps.
    // In first pass the routine searhes for the highest correlation with 
    // relatively coarse steps, then rescans the neighbourhood of the highest
    // correlation with better resolution and so on.
    for (scanCount = 0;scanCount < 4; scanCount ++) 
    {
        j = 0;
        while (scanOffsets[scanCount][j]) 
        {
            tempOffset = corrOffset + scanOffsets[scanCount][j];
            if (tempOffset >= seekLength) break;
            compare = refPos + 2 * tempOffset;

            /* Calculates correlation value for the mixing position corresponding
             * to 'tempOffset' */
            corr = 0;
            for (i = 2; i < 2 * overlapLength; i += 2) 
            {
                LONG_SAMPLETYPE temp1, temp2;

                temp1 = pRefMidBuffer[i] * compare[i];
                temp2 = pRefMidBuffer[i + 1] * compare[i + 1];

#ifdef INTEGER_SAMPLES

                // integer samples, scale the internal result to prevent overflowing
                corr += (temp1 + temp2) >> overlapDividerBits;

#else   // INTEGER_SAMPLES

                // floating point samples, no need to scale the internal result
                corr += (temp1 + temp2);

#endif  // INTEGER_SAMPLES

            }

            /* Checks for the highest correlation value */
            if (corr > bestCorr) 
            {
                bestCorr = corr;
                bestOffs = tempOffset;
            }
            j ++;
        }
        corrOffset = bestOffs;
    }
    return bestOffs;
}


// Seeks for the optimal overlap-mixing position. The 'mono' version of the
// routine
//
// The best position is determined as the position where the two overlapped
// sample sequences are 'most alike', in terms of the highest cross-correlation
// value over the overlapping period
uint TDStretch::seekBestOverlapPositionMonoQuick(const SAMPLETYPE *refPos) 
{
    uint j,i;
    uint bestOffs;
    LONG_SAMPLETYPE bestCorr, corr;
    uint scanCount, corrOffset, tempOffset;
    const SAMPLETYPE *compare;

    // Slopes the amplitude of the 'midBuffer' samples
    for (i=0 ; i < overlapLength ;i ++) 
    {
        LONG_SAMPLETYPE temp1, temp2;

        // Note : 'i' is unsigned, this step's required to converts the result to signed!
        temp1 = (LONG_SAMPLETYPE)(i * (overlapLength - i));
        temp2 = (pMidBuffer[i] * temp1) / slopingDivider;  // >> slopingDividerBits;
        pRefMidBuffer[i] = (SAMPLETYPE)temp2;
    }

    bestCorr = INT_MIN;
    bestOffs = 0;
    corrOffset = 0;
    tempOffset = 0;

    // Scans for the best correlation value using four-pass hierarchical search.
    //
    // The look-up table 'scans' has hierarchical position adjusting steps.
    // In first pass the routine searhes for the highest correlation with 
    // relatively coarse steps, then rescans the neighbourhood of the highest
    // correlation with better resolution and so on.
    for (scanCount = 0;scanCount < 4; scanCount ++) 
    {
        j = 0;
        while (scanOffsets[scanCount][j]) 
        {
            tempOffset = corrOffset + scanOffsets[scanCount][j];
            if (tempOffset >= seekLength) break;

            compare = refPos + tempOffset;

            // Calculates correlation value for the mixing position corresponding
            // to 'tempOffset'
            corr = 0;
            for (i = 1; i < overlapLength; i ++) 
            {
                LONG_SAMPLETYPE temp;

                temp = pRefMidBuffer[i] * compare[i];

#ifdef INTEGER_SAMPLES

                // integer samples, scale the internal result to prevent overflowing
                corr += temp >> overlapDividerBits;

#else   // INTEGER_SAMPLES

                // floating point samples, no need to scale the internal result
                corr += temp;

#endif  // INTEGER_SAMPLES

            }

            // Checks for the highest correlation value
            if (corr > bestCorr) 
            {
                bestCorr = corr;
                bestOffs = tempOffset;
            }
            j ++;
        }
        corrOffset = bestOffs;
    }
    return bestOffs;
}


// Sets new target tempo. Normal tempo = 'SCALE', smaller values represent slower 
// tempo, larger faster tempo.
void TDStretch::setTempo(float newTempo)
{
    uint optimSkip;

    tempo = newTempo;

    // Calculate ideal skip length (according to tempo value) and how many 
    // samples are needed in the 'inputBuffer' to process a batch of samples
    optimSkip = (int)(tempo * (seekWindowLength - overlapLength) + 0.5f);
    sampleReq = max(optimSkip + overlapLength, seekWindowLength) + maxOffset;
}



// Sets the number of channels, 1 = mono, 2 = stereo
void TDStretch::setChannels(uint numChannels)
{
    if (channels == numChannels) return;
    assert(numChannels == 1 || numChannels == 2);

    channels = numChannels;
    inputBuffer.setChannels(channels);
    outputBuffer.setChannels(channels);
}


// nominal tempo, no need for processing, just pass the samples through
// to outputBuffer
void TDStretch::processNominalTempo()
{
    assert(tempo == 1.0f);

    if (bMidBufferDirty) 
    {
        // If there are samples in pMidBuffer waiting for overlapping,
        // do a single sliding overlapping with them in order to prevent a 
        // clicking distortion in the output sound
        if (inputBuffer.numSamples() < overlapLength) 
        {
            // wait until we've got overlapLength input samples
            return;
        }
        // Mix the samples in the beginning of 'inputBuffer' with the 
        // samples in 'midBuffer' using sliding overlapping 
        overlap(outputBuffer.ptrEnd(overlapLength), inputBuffer.ptrBegin(), 0);
        outputBuffer.putSamples(overlapLength);
        inputBuffer.receiveSamples(overlapLength);
        clearMidBuffer();
        // now we've caught the nominal sample flow and may switch to
        // bypass mode
    }

    // Simply bypass samples from input to output
    outputBuffer.moveSamples(inputBuffer);
}


// Processes as many processing frames of the samples 'inputBuffer', store
// the result into 'outputBuffer'
void TDStretch::processSamples()
{
    uint ovlSkip, offset;
    int temp;

    if (tempo == 1.0f) 
    {
        // tempo not changed from the original, so bypass the processing
        processNominalTempo();
        return;
    }

    if (bMidBufferDirty == FALSE) 
    {
        // if midBuffer is empty, move the first samples of the input stream 
        // into it
        if (inputBuffer.numSamples() < overlapLength) 
        {
            // wait until we've got overlapLength samples
            return;
        }
        memcpy(pMidBuffer, inputBuffer.ptrBegin(), channels * overlapLength * sizeof(SAMPLETYPE));
        inputBuffer.receiveSamples(overlapLength);
        bMidBufferDirty = TRUE;
    }

    // Process samples as long as there are enough samples in 'inputBuffer'
    // to form a processing frame.
    while (inputBuffer.numSamples() >= sampleReq) 
    {
        // If tempo differs from the normal ('SCALE'), scan for the best overlapping
        // position
        offset = seekBestOverlapPosition(inputBuffer.ptrBegin());

        // Mix the samples in the 'inputBuffer' at position of 'offset' with the 
        // samples in 'midBuffer' using sliding overlapping
        // ... first partially overlap with the end of the previous sequence
        // (that's in 'midBuffer')
        overlap(outputBuffer.ptrEnd(overlapLength), inputBuffer.ptrBegin(), offset);
        outputBuffer.putSamples(overlapLength);

        // ... then copy sequence samples from 'inputBuffer' to output
        temp = (seekWindowLength - 2 * overlapLength);// & 0xfffffffe;
        if (temp > 0)
        {
            outputBuffer.putSamples(inputBuffer.ptrBegin() + channels * (offset + overlapLength), temp);
        }

        // Copies the end of the current sequence from 'inputBuffer' to 
        // 'midBuffer' for being mixed with the beginning of the next 
        // processing sequence and so on
        assert(offset + seekWindowLength <= inputBuffer.numSamples());
        memcpy(pMidBuffer, inputBuffer.ptrBegin() + channels * (offset + seekWindowLength - overlapLength), 
            channels * sizeof(SAMPLETYPE) * overlapLength);
        bMidBufferDirty = TRUE;

        // Remove the processed samples from the input buffer
        ovlSkip = (int)(tempo * (seekWindowLength - overlapLength) + 0.5f);
        inputBuffer.receiveSamples(ovlSkip);
    }
}


// Adds 'numsamples' pcs of samples from the 'samples' memory position into
// the input of the object.
void TDStretch::putSamples(const SAMPLETYPE *samples, uint numSamples)
{
    // Add the samples into the input buffer
    inputBuffer.putSamples(samples, numSamples);
    // Process the samples in input buffer
    processSamples();
}


// Operator 'new' is overloaded so that it automatically creates a suitable instance 
// depending on if we've a MMX-capable CPU available or not.
void * TDStretch::operator new(size_t s)
{
    // Notice! don't use "new TDStretch" directly, use "newInstance" to create a new instance instead!
    assert(FALSE);  
    return NULL;
}


TDStretch * TDStretch::newInstance()
{
    uint uExtensions;

    uExtensions = detectCPUextensions();

    // Check if MMX/SSE/3DNow! instruction set extensions supported by CPU

#ifdef ALLOW_MMX
    // MMX routines available only with integer sample types
    if (uExtensions & SUPPORT_MMX)
    {
        return ::new TDStretchMMX;
    }
    else
#endif // ALLOW_MMX


#ifdef ALLOW_SSE
    if (uExtensions & SUPPORT_SSE)
    {
        // SSE support
        return ::new TDStretchSSE;
    }
    else
#endif // ALLOW_SSE


#ifdef ALLOW_3DNOW
    if (uExtensions & SUPPORT_3DNOW)
    {
        // 3DNow! support
        return ::new TDStretch3DNow;
    }
    else
#endif // ALLOW_3DNOW

    {
        // ISA optimizations not supported, use plain C version
        return ::new TDStretch;
    }
}
