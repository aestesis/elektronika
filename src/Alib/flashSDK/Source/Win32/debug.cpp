/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright � Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 
****************************************************************************/

/****************************************************************************
  Porting Version of Flash 4 SDK
  Middlesoft, Inc. October 1999
  Release 10/22/1999 Lee Thomason, lee@middlesoft.com 
****************************************************************************/

#include <windows.h>
#include "debug.h"

void debugbreak()
{
	DebugBreak();
}

void dprintf( const char* format, ... )
{
    va_list     va;
    char		buffer[1024];

    //
    //  format and output the message..
    //
    va_start( va, format );
    wvsprintf( buffer, format, va );
    va_end( va );

    OutputDebugString( buffer );
}
