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

#include <windows.h>
#include "..\\bitbuf.h"
#include "palette.h"
#include "playerwnd.h"

// The palette code herin is based on the "Identity Palette" idea and implementation from the
// article at www.compuphase.com/palette.htm (Good reading about Windows palettes.)
// 
// Another reference is at www.geocities.com/Silicon Valley/2151/dibs.h, for general windows
// bitmap and palette code.


struct LOGPALETTE256 {
    WORD			palVersion;
    WORD			palNumEntries;
    PALETTEENTRY	palEntry[256];
};


void SColorInfo::CreateOSPalette()
{
}


void SColorInfo::DestroyOSPalette()
{
}


SColorInfo* NativePalette::screenColorInfo = 0;

// SColorInfo* NativePalette::StoreScreenPalette()
// {
// 	// delete the existing palette information
// 	if ( screenColorInfo )
// 	{
// 		delete screenColorInfo;
//         screenColorInfo = 0;
// 	}
// 
// 	int depth = NativeDisplayTester::currentScreenBitDepth();
// 	U32 nColors = NativeDisplayTester::currentScreenColors();
// 
// 	FLASHASSERT( depth <= 8 );
// 	int colors = ( 1 << depth );
// 	SColorTable ctab;
// 
// 	GetScreenPalette( &ctab );
// 	screenColorInfo = CreateColorInfo( &ctab );
// 	return screenColorInfo;
// }

void NativePalette::GetScreenPalette( SColorTable* ctab )
{
	int depth = NativeDisplayTester::CurrentScreenBitDepth();
	U32 nColors = NativeDisplayTester::CurrentScreenColors();

	// Get the system palette entries
	HDC dc = GetDC(0);
	LOGPALETTE256 sysPal;

	sysPal.palVersion = 0x300;
	sysPal.palNumEntries = GetSystemPaletteEntries(dc, 0, nColors, sysPal.palEntry);
	ReleaseDC(0, dc);

	for ( U32 i = 0; i<nColors; i++ )
	{
		ctab->colors[i].red   = sysPal.palEntry[i].peRed;
		ctab->colors[i].green = sysPal.palEntry[i].peGreen;
		ctab->colors[i].blue  = sysPal.palEntry[i].peBlue;
		ctab->colors[i].alpha = 255;
	}
	ctab->n = nColors;

	ReleaseDC( 0, dc );
}


// void NativePalette::ClearScreenPalette()
// {
// 	if ( screenColorInfo )
// 	{
// 		delete screenColorInfo;
//         screenColorInfo = 0;
// 	}
// }


// BOOL NativePalette::IsPaletteCurrent()
// {
// 	if ( screenColorInfo )
// 	{
// 		SColorTable ctab;
// 
// 		int depth;
// 		U32 nColors;
// 		NativeDisplayTester::currentScreenBitDepth( &depth, &nColors );
// 
// 		if ( int( nColors ) != screenColorInfo->ctab.n )
// 			return false;
// 
// 		GetScreenPalette( &ctab );
// 
// 		// check if they are the same
// 		for( int i = 0; i< int(nColors); i++ )
// 		{
// 			if (    ctab.colors[i].red   != screenColorInfo->ctab.colors[i].red 
// 				 || ctab.colors[i].green != creenColorInfo->ctab.colors[i].green
//  				 || ctab.colors[i].blue  != screenColorInfo->ctab.colors[i].blue
// 			   )
// 			{
// 				return false;
// 			}
// 		}
// 		return true;
// 	}
// 	return false;
// }
// 
// void NativePalette::SetPaletteToScreen( NativePlayerWnd* native, const SColorTable* table )
// {
// 	HPALETTE palette;
// 	LOGPALETTE256 pal;
// 
// 	pal.palNumEntries = table->n;
//     pal.palVersion = 0x300;
// 
// 	PALETTEENTRY* pe = pal.palPalEntry;
// 	const RGB8*	  c  = table->colors;
// 	for ( int i = 0; i < pal.palNumEntries; i++, pe++, c++ ) {
// 		pe->peRed = c->red;
// 		pe->peGreen = c->green;
// 		pe->peBlue = c->blue;
// 		pe->peFlags = 0;
// 	}
// 
// 	palette = CreatePalette((LOGPALETTE*)&pal);
// 
// 	HDC hdc = native->GetWindowDC();
// 	SelectPalette( hdc, palette, true );
// 	RealizePalette( hdc );
// 
// 	native->ReleaseWindowDC();
// 	DeleteObject( palette );
// }


// See the compuphase reference.
// Creates and identity palette - one which does not need OS level color conversion.

void NativePalette::ClearSystemPalette( HWND hwnd )
{
	struct {
		WORD palVersion;
		WORD palNumEntries;
		PALETTEENTRY palEntry[256];
	} logpal = { 0x300,	256	};

	HPALETTE hpal;
	HDC hdc;
	int i;

	/* Reset everything in the system palette to black */
	memset(logpal.palEntry, 0, sizeof( logpal.palEntry) );

	for (i = 0; i < 256; i++)
	{
		logpal.palEntry[i].peFlags = PC_NOCOLLAPSE;
	}

	/* Create, select, realize, deselect, and delete the palette */
	hpal = CreatePalette((LOGPALETTE *)&logpal);
	if (hpal) {
		hdc = GetDC(hwnd);
		hpal = SelectPalette(hdc,hpal,FALSE);
		RealizePalette(hdc);
		hpal = SelectPalette(hdc,hpal,FALSE);
		ReleaseDC(hwnd,hdc);
		DeleteObject(hpal);
	}
}

HPALETTE NativePalette::CreateIdentityPalette( HWND hwnd )
{
	LOGPALETTE256 logpal = { 0x300, 256	};
	int i, n, nStatCols;

	HDC hdc = GetDC( NULL );

	/* The number of static colours should be 20, but inquire it anyway */
	nStatCols = GetDeviceCaps(hdc, NUMRESERVED);

	/* Copy the entire system palette, though we only need the first 10
	* and the last 10 entries */
	GetSystemPaletteEntries(hdc, 0, 256, logpal.palEntry);

	/* Clear the peFlags of the lower and upper static colours */
	for (i = 0; i < (nStatCols / 2); i++)
		logpal.palEntry[i].peFlags = 0;
	for (i = 256 - (nStatCols / 2); i < 256; i++)
		logpal.palEntry[i].peFlags = 0;

	// Set the palette entries to the DIB colours.
	// This is a well behaved function, but we are really relying
	// on using 6*6*6 = 216 colors. 
	// The rest is given to grey scale (20 colors) and the OS (20 colors.)
	for (i = (nStatCols / 2), n = 0 ; i < 256 - (nStatCols / 2); i++, n++ ) 
	{
		if ( i < 6*6*6 )
		{
			logpal.palEntry[i].peRed   = colorRamp6[ (n/36) % 6 ];
			logpal.palEntry[i].peGreen = colorRamp6[ (n/6)  % 6 ];
			logpal.palEntry[i].peBlue  = colorRamp6[ (n)    % 6 ];
			logpal.palEntry[i].peFlags = PC_NOCOLLAPSE;
		}
		else
		{
			logpal.palEntry[i].peRed   = ( n - 6*6*6 ) * 12 + 10;
			logpal.palEntry[i].peGreen = ( n - 6*6*6 ) * 12 + 10;
			logpal.palEntry[i].peBlue  = ( n - 6*6*6 ) * 12 + 10; 
			logpal.palEntry[i].peFlags = PC_NOCOLLAPSE;
		}
	}

	ReleaseDC( 0, hdc );
	return CreatePalette((LOGPALETTE *)&logpal);
}
