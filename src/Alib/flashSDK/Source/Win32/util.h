/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright © Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 
****************************************************************************/

/****************************************************************************
  Porting Version of Flash 4 SDK
  Middlesoft, Inc. October 1999
  Release 10/22/1999 Lee Thomason, lee@middlesoft.com 
  
  Version system added 3/15/2000 lee@middelsoft.com
****************************************************************************/

#ifndef UTIL_H_
#define UTIL_H_

#include "../stdafx.h"
#include "../global.h"

/*! \Page UTIL Flash Utility Functions.
 *	Located in util.h, at this point there is only one utility function:<br>
 *  GetTimeMSec which returns the time in milliseconds, from some arbitrary 
 *	point. (Program start, system start, number of milliseconds since HAL went on-line,
 *	whatever floats your boat.)<br>
 *	CAVEAT: Though a signed 32bit int is returned, it must always
 *			be positive.
 */
inline S32 GetTimeMSec()		{ return timeGetTime() & 0x7FFFFFFF; }
		   	  
#endif


