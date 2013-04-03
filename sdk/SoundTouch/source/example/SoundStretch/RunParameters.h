/******************************************************************************
 *
 * A class for parsing the 'soundstretch' application command line parameters
 *
 * Author        : Copyright (c) Olli Parviainen
 * Author e-mail : oparviai @ iki.fi
 * File created  : 13-Jan-2002
 *
 * Last changed  : $Date: 2003/09/21 10:50:40 $
 * File revision : $Revision: 1.4 $
 *
 * $Id: RunParameters.h,v 1.4 2003/09/21 10:50:40 Olli Exp $
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

#ifndef RUNPARAMETERS_H
#define RUNPARAMETERS_H

#include "STTypes.h"
#include <string>

using namespace std;

/// Parses command line parameters into program parameters
class RunParameters
{
private:
    void throwIllegalParamExp(const string &str) const;
    void throwLicense() const;
    void parseSwitchParam(const string &str);
    void checkLimits();
    float parseSwitchValue(const string &str) const;

public:
    char  *inFileName;
    char  *outFileName;
    float tempoDelta;
    float pitchDelta;
    float rateDelta;
    int   quick;
    int   noAntiAlias;
    float goalBPM;
    BOOL  detectBPM;

    RunParameters(const int nParams, const char *paramStr[]);
};

#endif
