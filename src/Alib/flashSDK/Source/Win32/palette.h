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

#ifndef _FLASHPALETTE_H_
#define _FLASHPALETTE_H_

#include <windows.h>
struct SColorInfo;
class NativePlayerWnd;

/*!
 *	NativePalette has only one method: GetScreenPalette, which is well defined.
 *	For palettized displays, the GetScreenPalette method will be called
 *	every time a bitmap is constructed, which is every time the Flash window
 *	changes size, depth, or palette.
 */
 
class NativePalette
{
  public:
	/*! Fetches the current palette being used by the screen and writes
	 *	it into SColorTable.
	 */
	static void GetScreenPalette( SColorTable* ctab );

	// ------- The following are Windows only methods -----------------
	static void		ClearSystemPalette( HWND hwnd );
	static HPALETTE CreateIdentityPalette( HWND hwnd );

  private:

	static SColorInfo* screenColorInfo;		// the current palette information
};

#endif
