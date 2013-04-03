/******************************************************************************
 *
 * SoundStretch main routine.
 *
 * Author        : Copyright (c) Olli Parviainen
 * Author e-mail : oparviai @ iki.fi
 * File created  : 13-Jan-2002
 *
 * Last changed  : $Date: 2003/12/27 11:59:22 $
 * File revision : $Revision: 1.16 $
 *
 * $Id: main.cpp,v 1.16 2003/12/27 11:59:22 Olli Exp $
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

#include <stdexcept>
#include <stdio.h>
#include "RunParameters.h"
#include "WavFile.h"
#include "SoundTouch.h"

#include "BPMDetect.h"

using namespace soundtouch;
using namespace std;

#define BUFF_SIZE       2048
#define BUFF_SIZE_BYTES (2 * BUFF_SIZE)

static const char _helloText[] = 
    "\n"
    "   SoundStretch v%s -  Written by Olli Parviainen 2001 - 2003\n"
    "==================================================================\n"
    "author e-mail: <oparviai@iki.fi> - WWW: http://www.iki.fi/oparviai/soundtouch\n"
    "\n"
    "This program is subject to (L)GPL license. Run \"soundstretch -license\" for\n"
    "more information.\n"
    "\n";

static void openFiles(WavInFile **inFile, WavOutFile **outFile, const RunParameters *params)
{
    int bits, samplerate, channels;

    // open input file...
    *inFile = new WavInFile(params->inFileName);

    // ... open output file with same sound parameters
    bits = (*inFile)->getNumBits();
    samplerate = (*inFile)->getSampleRate();
    channels = (*inFile)->getNumChannels();

    if (params->outFileName)
    {
        *outFile = new WavOutFile(params->outFileName, samplerate, bits, channels);
    }
    else
    {
        *outFile = NULL;
    }
}



// Sets the 'SoundTouch' object up according to input file sound format & 
// command line parameters
static void setup(SoundTouch *pSoundTouch, const WavInFile *inFile, const RunParameters *params)
{
    int sampleRate;
    int channels;

    sampleRate = inFile->getSampleRate();
    channels = inFile->getNumChannels();
    pSoundTouch->setSampleRate(sampleRate);
    pSoundTouch->setChannels(channels);

    pSoundTouch->setTempoChange(params->tempoDelta);
    pSoundTouch->setPitchSemiTones(params->pitchDelta);
    pSoundTouch->setRateChange(params->rateDelta);

    pSoundTouch->setSetting(SETTING_USE_QUICKSEEK, params->quick);
    pSoundTouch->setSetting(SETTING_USE_AA_FILTER, !params->noAntiAlias);

    // print processing information
    if (params->outFileName)
    {
#ifdef INTEGER_SAMPLES
        printf("Uses 16bit integer sample type in processing.\n\n");
#else
    #ifndef FLOAT_SAMPLES
        #error "Sampletype not defined"
    #endif
        printf("Uses 32bit floating point sample type in processing.\n\n");
#endif
        // print processing information only if outFileName given i.e. some processing will happen
        printf("Processing the file with the following changes:\n");
        printf("  tempo change = %+g %%\n", params->tempoDelta);
        printf("  pitch change = %+g semitones\n", params->pitchDelta);
        printf("  rate change  = %+g %%\n\n", params->rateDelta);
        printf("Working...");
    }
    else
    {
        // outFileName not given
        printf("Warning: output file name missing, won't output anything.\n\n");
    }

    fflush(stdout);
}


/// Convert samples in SAMPLETYPE format to 16bit integer
///
/// Note: This conversion function is inefficient for floating point
/// sample format, because the standard C way of converting floats to
/// integers is slow. If you're concerned about performance, you'd 
/// better remake this function with some CPU-specific stuff, but it's 
/// here only as an example.
static void _sampleType2Short(short *pDest, const SAMPLETYPE *pSrc, uint uNum)
{
    uint i;

    for (i = 0; i < uNum; i ++)
    {
        int temp;

        // saturate result between -32768..32767
        temp = (int)pSrc[i];
        if (temp > 32767)
        {
            temp = 32767;
        } 
        else if (temp < -32768)
        {
            temp = -32768;
        }
        pDest[i] = temp;
    }
}


/// Convert 16bit integer samples to SAMPLETYPE format.
static void _short2SampleType(SAMPLETYPE *pDest, const short *pSrc, uint uNum)
{
    uint i;

    for (i = 0; i < uNum; i ++)
    {
        pDest[i] = pSrc[i];
    }
}


// Processes the sound
static void process(SoundTouch *pSoundTouch, WavInFile *inFile, WavOutFile *outFile)
{
    int nBytes;
    int nSamples;
    int bytesPerSample;
    int buffSizeSamples;
    short intBuffer[BUFF_SIZE];
    SAMPLETYPE sampleBuffer[BUFF_SIZE];

    if ((inFile == NULL) || (outFile == NULL)) return;  // nothing to do.

    bytesPerSample = inFile->getBytesPerSample();
    buffSizeSamples = BUFF_SIZE_BYTES / bytesPerSample;

    while (inFile->eof() == 0)
    {
        nBytes = inFile->read(intBuffer, BUFF_SIZE_BYTES);
        _short2SampleType(sampleBuffer, intBuffer, nBytes / sizeof(short));

        nSamples = nBytes / bytesPerSample;
        pSoundTouch->putSamples(sampleBuffer, nSamples);

        do 
        {
            nSamples = pSoundTouch->receiveSamples(sampleBuffer, buffSizeSamples);
            nBytes = nSamples * bytesPerSample;
            assert(nBytes <= BUFF_SIZE_BYTES);

            _sampleType2Short(intBuffer, sampleBuffer, nBytes / sizeof(short));
            outFile->write(intBuffer, nBytes);
        } while (nSamples != 0);
    }

    pSoundTouch->flush();
    do 
    {
        nSamples = pSoundTouch->receiveSamples(sampleBuffer, buffSizeSamples);
        nBytes = nSamples * bytesPerSample;
        _sampleType2Short(intBuffer, sampleBuffer, nBytes / sizeof(short));
        outFile->write(intBuffer, nBytes);
    } while (nSamples != 0);
}



// Detect BPM rate of inFile and adjust tempo setting accordingly if necessary
static void detectBPM(WavInFile *inFile, RunParameters *params)
{
    BPMDetect bpm(inFile->getNumChannels(), inFile->getSampleRate());
    float bpmValue;
    int bytesPerSample;
    short intBuffer[BUFF_SIZE];
    SAMPLETYPE sampleBuffer[BUFF_SIZE];

    // detect bpm rate
    printf("Detecting BPM rate...");
    fflush(stdout);

    bytesPerSample = inFile->getBytesPerSample();

    // Process the 'inFile' in small blocks, repeat until whole file has 
    // been processed
    while (inFile->eof() == 0)
    {
        int bytes, samples;

        // Read sample data from input file
        bytes = inFile->read(intBuffer, BUFF_SIZE_BYTES);
        // Convert to locally used sample type
        _short2SampleType(sampleBuffer, intBuffer, bytes / sizeof(short));

        // Enter the new samples to the bpm analyzer class
        samples = bytes / bytesPerSample;
        bpm.inputSamples(sampleBuffer, samples);
    }

    // Now the whole song data has been analyzed. Read the resulting bpm.
    bpmValue = bpm.getBpm();
    printf("Done!\n");

    // rewind the file after bpm detection
    inFile->rewind();

    if (bpmValue > 0)
    {
        printf("Detected BPM rate %.1f\n\n", bpmValue);
    }
    else
    {
        printf("Couldn't detect BPM rate.\n\n");
        return;
    }

    if (params->goalBPM > 0)
    {
        // adjust tempo to given bpm
        params->tempoDelta = (params->goalBPM / bpmValue - 1.0f) * 100.0f;
        printf("The file will be converted to %.1f BPM\n\n", params->goalBPM);
    }
}



int main(const int nParams, const char *paramStr[])
{
    WavInFile *inFile;
    WavOutFile *outFile;
    SoundTouch SoundTouch;
    RunParameters *params;

    printf(_helloText, SoundTouch::getVersionString());

    try 
    {
        // Parse command line parameters
        params = new RunParameters(nParams, paramStr);

        // Open input & output files
        openFiles(&inFile, &outFile, params);

        if (params->detectBPM == TRUE)
        {
            // detect sound BPM (and adjust processing parameters
            //  accordingly if necessary)
            detectBPM(inFile, params);
        }

        // Setup the 'SoundTouch' object for processing the sound
        setup(&SoundTouch, inFile, params);

        // Process the sound
        process(&SoundTouch, inFile, outFile);

        // Close WAV file handles & dispose of the objects
        delete inFile;
        delete outFile;
        delete params;

        printf("Done!\n");
    } 
    catch (runtime_error &e) 
    {
        // An exception occurred during processing, display an error message
        printf("%s\n", e.what());
        return -1;
    }

    return 0;
}
