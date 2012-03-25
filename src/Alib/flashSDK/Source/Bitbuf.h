/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright © Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 
****************************************************************************/

#ifndef BITBUF_INCLUDED
#define BITBUF_INCLUDED

#ifndef SBITMAP_INCLUDED
#include "sbitmap.h"
#endif

#include NATIVE_PALETTE
#include NATIVE_DISPLAY

#define CalcITabIndex(r, g, b) (((int)(r)&0xF0) << 4 | ((int)(g)&0xF0) | ((int)(b)&0xF0) >> 4)
#define CalcITabIndexRGB(rgb) (((rgb)->red&0xF0) << 4 | ((rgb)->green&0xF0) | ((rgb)->blue&0xF0) >> 4)


class NativePlayerWnd;

const U8 colorRamp6[] = { 0, 0x33, 0x66, 0x99, 0xCC, 0xff };

/*!	The bitmap formats supported by Flash. 
 */
enum PixelFormat { 
	pix1, //!< Indexed, black and white
	pix2, //!< Indexed, 4 color
	pix4, //!< Indexed, 16 color
	pix8, //!< Indexed, 256 color

	pix16, 	//!< Direct color, 555 RGB
	pix16A, //!< Direct color, 565 RGB
	pix24, 	//!< Direct color, BGR 
	pix32, 	//!< Direct color, 888 RGB
	pix32A 	//!< Direct color, 8888 ARGB
};


/*!	Flash palette object. 
 */
struct SColorInfo {
	// ---------------- Platform independant -----------------
	SColorInfo* next;	// Pointer to the next SColorInfo.
	int refCount;

	SColorTable ctab;	// This is the 256 Palette entries.
	U8 itab[16*16*16];	// The inverse color table - a mapping of RGB to indexes.

	// ---------------- Platform dependant -------------------
	/*! After the SColorInfo object is valid, CreateOSPalette gets
	 *	called, allowing OS dependant code to be inserted.
	 */
	void CreateOSPalette();
	/*! DestroyOSPalette gets called before the SColorInfo structure is destroyed. */
	void DestroyOSPalette();
};

class CBitBuffer {
  public:
	CBitBuffer();
	~CBitBuffer();

	BOOL inverted()		{ return ( m_inverted != 0 ); }
	int  pixelFormat()	{ FLASHASSERT( bitmap ); return bitmap->PixelFormat();  }
	int  depth()		{ FLASHASSERT( bitmap ); return bitmap->Depth();  }
	char* baseAddess()	{ FLASHASSERT( bitmap ); return (char*) bitmap->Bits();  }
	int  scanLine()		{ FLASHASSERT( bitmap ); return bitmap->ScanLine();  }
	int  width()		{ FLASHASSERT( bitmap ); return bitmap->Width();   }
	int  height()		{ FLASHASSERT( bitmap ); return bitmap->Height();  }

	void invalidate();	// Deletes the current palette and sets the entire buffer to
						// be invalid. (Be careful when you call this.)
	
	int xorg()					{ return m_xorg; }
  	SColorInfo* getSColorInfo() { return m_cinfo; }
	void setAllowPurge( BOOL );
	BOOL allowPurge()			{ return ( m_allowPurge != 0 ); }

	void bltToScreen(	NativePlayerWnd*,				// where to go get platform dependant info
						int x, int y,				// upper left target on screen
						int cx, int cy,
						int xSource,	int ySource	// where to start drawing from
					);	
	void clearScreen( NativePlayerWnd* );

	BOOL CreateScreenBits( NativePlayerWnd* native, SRECT* frame );	// pass frame in global screen coordinates

	BOOL LockBits() { return (bitmap && ((bitmap->Bits()) != 0)); }
	void UnlockBits() {}

	void FreeBits();

	BOOL BitsValid();

	void Flush()		{ if ( bitmap ) bitmap->Flush();  }

	class NativeBitmap* bitmap;
  private:

	// The bitmap info
	BOOL m_inverted;	// set if we are using the bottom up DIB format

	int m_xorg;			// an offset for aligning the offscreen pixels to the screen pixels

	// Color table info
	SColorInfo* m_cinfo;

	// Flags the determine whether the destructor should free these objects
	BOOL m_allowPurge;

	int m_bmiSize;
};


SColorInfo* CreateColorInfo(SColorTable*);	// create a new color info
SColorInfo* GetColorInfo(SColorTable*);		// search the shared color infos for a match, otherwise create a new one

inline void AttachColorInfo(SColorInfo* cinfo) {cinfo->refCount++;}
void ReleaseColorInfo(SColorInfo*);

BOOL ColorTableEqual(SColorTable* c1, SColorTable* c2);
void ColorTableCopy(SColorTable* src, SColorTable* dst);

void SetStandardColors(SColorTable*);

extern const U8 colorRamp6[6];	 // the standard color ramp for our 8-bit color table

#endif
