/*****************************************************************************
 * 
 * Sample rate transposer. Changes sample rate by using linear interpolation 
 * together with anti-alias filtering (first order interpolation with anti-
 * alias filtering should be quite adequate for this application)
 *
 * Author        : Copyright (c) Olli Parviainen
 * Author e-mail : oparviai @ iki.fi
 * File created  : 13-Jan-2002
 *
 * Last changed  : $Date: 2003/12/27 10:00:52 $
 * File revision : $Revision: 1.5 $
 *
 * $Id: RateTransposer.h,v 1.5 2003/12/27 10:00:52 Olli Exp $
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

#ifndef RateTransposer_H
#define RateTransposer_H

#include "AAFilter.h"
#include "FIFOSamplePipe.h"
#include "FIFOSampleBuffer.h"

#include "STTypes.h"

class RateTransposer : public FIFOProcessor
{
private:
    /// Anti-alias filter object
    AAFilter *pAAFilter;

    int iSlopeCount;
    uint uRate;
    uint uChannels;
    soundtouch::SAMPLETYPE sPrevSampleL, sPrevSampleR;

    /// Buffer for collecting samples to feed the anti-alias filter between
    /// two batches
    FIFOSampleBuffer storeBuffer;

    /// Buffer for keeping samples between transposing & anti-alias filter
    FIFOSampleBuffer tempBuffer;

    /// Output sample buffer
    FIFOSampleBuffer outputBuffer;

    BOOL bUseAAFilter;

    void resetRegisters();

    uint transposeStereo(soundtouch::SAMPLETYPE *dest, 
                         const soundtouch::SAMPLETYPE *src, 
                         uint numSamples);
    uint transposeMono(soundtouch::SAMPLETYPE *dest, 
                       const soundtouch::SAMPLETYPE *src, 
                       uint numSamples);
    uint transpose(soundtouch::SAMPLETYPE *dest, 
                   const soundtouch::SAMPLETYPE *src, 
                   uint numSamples);

    void flushStoreBuffer();

    void downsample(const soundtouch::SAMPLETYPE *src, 
                    uint numSamples);
    void upsample(const soundtouch::SAMPLETYPE *src, 
                 uint numSamples);

    /// Transposes sample rate by applying anti-alias filter to prevent folding. 
    /// Returns amount of samples returned in the "dest" buffer.
    /// The maximum amount of samples that can be returned at a time is set by
    /// the 'set_returnBuffer_size' function.
    void processSamples(const soundtouch::SAMPLETYPE *src, 
                        uint numSamples);

public:
    RateTransposer();
    virtual ~RateTransposer();

    /// Returns the output buffer object
    FIFOSamplePipe *getOutput() { return &outputBuffer; };

    /// Returns the store buffer object
    FIFOSamplePipe *getStore() { return &storeBuffer; };

    /// Return anti-alias filter object
    AAFilter *getAAFilter() const;

    /// Enables/disables the anti-alias filter. Zero to disable, nonzero to enable
    void enableAAFilter(BOOL newMode);

    /// Returns nonzero if anti-alias filter is enabled.
    BOOL isAAFilterEnabled() const;

    /// Sets new target rate. Normal rate = 1.0, smaller values represent slower 
    /// rate, larger faster rates.
    void setRate(float newRate);

    /// Sets the number of channels, 1 = mono, 2 = stereo
    void setChannels(uint channels);

    /// Adds 'numSamples' pcs of samples from the 'samples' memory position into
    /// the input of the object.
    virtual void putSamples(const soundtouch::SAMPLETYPE *samples, uint numSamples);

    /// Clears all the samples in the object
    virtual void clear();

    /// Returns nonzero if there aren't any samples available for outputting.
    virtual uint isEmpty();
};

#endif
