/******************************************************************************
 *
 * The routine detects highest value on an array of values and calculates the 
 * precise peak location as a mass-center of the 'hump' around the peak value.
 *
 * Author        : Copyright (c) Olli Parviainen
 * Author e-mail : oparviai @ iki.fi
 * File created  : 11-Jan-2003
 *
 * Last changed  : $Date: 2003/11/17 18:18:39 $
 * File revision : $Revision: 1.1 $
 *
 * $Id: PeakFinder.h,v 1.1 2003/11/17 18:18:39 Olli Exp $
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

#ifndef _PeakFinder_H_
#define _PeakFinder_H_

class PeakFinder
{
protected:
    /// Min, max allowed peak positions within the data vector
    int minPos, maxPos;

    /// Calculates the mass center between given vector items.
    float calcMassCenter(const float *data, ///< Data vector.
                         int firstPos,      ///< Index of first vector item beloging to the peak.
                         int lastPos        ///< Index of last vector item beloging to the peak.
                         ) const;

    /// Finds the data vector index where the monotoniously decreasing signal crosses the
    /// given level.
    int   findCrossingLevel(const float *data,  ///< Data vector.
                            float level,        ///< Goal crossing level.
                            int peakpos,        ///< Peak position index within the data vector.
                            int direction       /// Direction where to proceed from the peak: 1 = right, -1 = left.
                            ) const;

    /// Finds the 'ground' level, i.e. smallest level between two neighbouring peaks, to right- 
    /// or left-hand side of the given peak position.
    int   findGround(const float *data,     /// Data vector.
                     int peakpos,           /// Peak position index within the data vector.
                     int direction          /// Direction where to proceed from the peak: 1 = right, -1 = left.
                     ) const;

public:
    /// Constructor. 
	PeakFinder();

    /// Detect exact peak position of the data vector by finding the largest peak 'hump'
    /// and calculating the mass-center location of the peak hump. 
    ///
    /// \return The exact mass-center location of the largest peak hump.
	float detectPeak(const float *data, /// Data vector to be analyzed. The data vector has
                                        /// to be at least 'maxPos' items long.
                     int minPos,        ///< Min allowed peak location within the vector data.
                     int maxPos         ///< Max allowed peak location within the vector data.
                     );
};

#endif // _PeakFinder_H_
