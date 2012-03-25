/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright © Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 
****************************************************************************/

// #include "stdafx.h"
#include "memcop.h"

//--------------------------------------------------------------------------------
// defines/constants
#ifdef _MAC

const Size kTempMemSafetyMargin = 0x00080000; 	// 512 K
const Size kAppMemSafetyMargin 	= 50000; 		// between 48K and 49K

#endif													

//--------------------------------------------------------------------------------
// globals
#ifdef _MAC
BOOL allocFailed = false;
BOOL allocFailedOnce = false;

BOOL TempMemSeemsLow(Size size)
{
	BOOL seemsLow = TempFreeMem() < kTempMemSafetyMargin + size;

	if (seemsLow)
	{
		// Force a heap compaction, and look again.
		Size ignored = 0;
		TempMaxMem(&ignored);
		
		seemsLow = TempFreeMem() < kTempMemSafetyMargin + size;
	}

	return seemsLow;
}

BOOL AppMemSeemsLow(Size size)
{
	return  ( FreeMem() < kAppMemSafetyMargin + size ) ;
}

#endif                     

