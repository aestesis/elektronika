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
****************************************************************************/

#ifndef FLASH_DEBUG_H_
#define FLASH_DEBUG_H_

/*!
 *	\page DEBUG Debugging macros.
 *	The debugging macros are #defined in debug.h.<p>
 *	Flash native code needs to define two macros that are used for 
 *	debugging the program.<br>
 *	FLASHASSERT is a standard assert function with a long name to avoid
 *  name space collisions.<br>
 *	FLASHOUTPUT is a "printf" which sends output to a debugging window.
 */
#ifdef DEBUG

	#if PURIFY
		#define FLASHASSERT(f)	((void)0)
		inline void FLASHOUTPUT( const char* f, ... ) {}
	#else
		void debugbreak();
		void dprintf( const char* format, ... );

		#define FLASHASSERT(f)	{ if ( !(f) ) debugbreak(); }
		#define FLASHOUTPUT		dprintf
	#endif /* Purify */

#else

	#define FLASHASSERT(f)     ((void)0)
	inline void FLASHOUTPUT( const char* f, ... ) {}

#endif	// DEBUG

#endif	// _DEBUG_H_

