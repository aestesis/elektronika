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

#ifndef __DISPLAY_H_
#define __DISPLAY_H_

#include "stdafx.h"

#include "bitmap.h"


class NativePlayerWnd;

/*! 
 *	NativeDisplayTester:
 *  1) queries the current screen bit depth,
 *  2) queries the current screen color count, and
 *  3) tests the display for the best format for a given depth.
 */
class NativeDisplayTester
{
  public:
	/*! */
	NativeDisplayTester();
	/*! */
	~NativeDisplayTester();

	/*!
	 *	Given the depth, computes the best PixelFormat. BestDetectedFormat and
	 *  BestDetectedInvert should be valid after this call. Example: given a 
	 *	bit depth of 16, this could determine whether to use pix16A or pix16.
	 *  For format choices, see the #PixelFormat structure.
	 */
	void TestDepth( int depth );

	int	BestDetectedFormat()	{ return bestFormat; }	//!< The PixelFormat, valid after a call to TestDepth.

	/*! "invert" refers to the orientation of display memory relative to the
	 *	screen. If you display is drawn upside down, change this value.
	 */
	BOOL BestDetectedInvert()	{ return false; }		

	/*	Relation of the depth to the number of colors:
	 *			depth	nColors
	 *			1		2
	 *			2		4
	 *			4		16
	 *			8		256
	 *			16		64k		(k=1024)
	 *			24		16 million
	 *			32		16 million	-- Note the cheat. We can't return a number that big.
	 */
	/*!
	 *	CurrentScreenBitDepth returns the current depth of the display.
	 */
	static int CurrentScreenBitDepth();
	static U32 CurrentScreenColors();	//!< The number of colors the screen can display.
	
	// ----- Platform independant - don't change these functions ------
	// These are conversion functions from format (say pix8) to depth (256)
	static int PixelFormatToDepth( int format )				{ return mapPixelFormatToDepth[ format ]; }
	static int DepthToSimplestPixelFormat( int depth );

  private:
	void	test( int pixFormat, BOOL inverted );

	static const int mapPixelFormatToDepth[9];

	int  bestFormat;
	BOOL bestInvert;
};

/*!
 *	The NativeBitmap is a wrapper around a block of memory treated as a raster device.
 *	The "defining function" of this class is the BitBlt - which moves the bits in 
 *	memory to the screen. Many OSs have a rather specific idea of what a bitmap is. NativeBitmap 
 *  wraps this OS dependent object and presents an interface to the main Flash code. The closer 
 *  the OS bitmap is to a block of bits in memory (as opposed to an abstract object) the 
 *  easier it will be to port, and the better the results will be. Flash will create and 
 *  destroy the NativeBitmap in order that it has the same dimensions as the player window.
 */
class NativeBitmap : public Abitmap
{
	ALIBOBJ

  public:
	/*! Fully contstructs a bitmap - after construction, it can be read or written at any time.
	 *	\param native	Pointer back to the native code, so that OS dependant states can be queried.
	 *	\param ctab		For Indexed modes, this cotains the palette.
	 *	\param width	Not guaranteed to be word aligned.
 	 *	\param height
	 *	\param depth	The number of bits per pixel.
	 *  \param pixelFormat	How to interpret pixel data. See PixelFormat.
	 */ 
	NativeBitmap(	NativePlayerWnd* native,
					const SColorTable* ctab,
					int width, int height, int depth, int pixelFormat );

	/*! */
	~NativeBitmap();

	int Height()		{ return h; }				//!< Height, in pixels
	int Width()			{ return w; }				//!< Width, , in pixels
	U8* Bits()			{ return (U8*)body; }			//!< Pointer to the base address of bitmap memory.
	int ScanLine()		{ return bytesPerLine; }	//!< Bytes of memory per row of pixels.
	int PixelFormat()	{ return format; }			//!< Valid formats are: pix1, pix4, pix8, pix16, pix16A, pix24, pix32, pix32A
	int Depth()			{ return nbBits; }			//!< The number of pixels per byte.

	/*! Flush is only meaningful if you have to worry about graphics commands being queued on your OS. 
	 *	Win and Mac, mainly. Mac code looks like:
	 *  while ( !QDDone((GrafPort*)bits->m_gWorld) ) ;
	 */
	void Flush()		{ GdiFlush(); }		

	/*!  BltNativeToScreen transfers a block of pixels from memory to the display. 
	 *   All parameters are input.
	 *	\param native   Since the information needed to make a screen blit is dependent on the OS, 
	 *				    no assumption is made about what info you might need. 
	 *					Therefore, a BltToScreen() passes (as input) a pointer to the NativePlayerWnd 
	 *					so it can be queried for things like the current window handle, 
	 *					graphics context, etc. These query functions in the NativePlayerWnd are up 
	 *					to the implementor.
	 *	\param xDest	X value of upper left target on screen.
	 *	\param yDest	Y value of upper left target on screen.
	 *	\param cx		X size of pixels to blt.
	 *	\param cy		Y size of pixels to blt.
	 *	\param xSource	X coordinate of source pixels (in this object).
	 *	\param ySource	Y coordinate of source pixels (in this object).
	 */
	void BltNativeToScreen( NativePlayerWnd* native,
							  int xDest,	int yDest,		// upper left target on screen
							  int cx,		int cy,			// size to blt
							  int xSource,	int ySource		// where to start drawing from
							);
	/*! Clears the screen when nothing is going on.
	 */
	void ClearScreen( NativePlayerWnd* );

  private:
  	struct BITMAPINFO256 
	{
	   BITMAPINFOHEADER bmiHeader; 
	   RGBQUAD          bmiColors[256]; 
	};

	int		format;
	//int		depth;
	int		bytesPerLine;

	//HBITMAP hBitmap;
	//BITMAPINFO256 info;
	//HPALETTE palette;
};

#endif