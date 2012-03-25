/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright © Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 
****************************************************************************/

#ifndef SBITMAP_INCLUDED
#define SBITMAP_INCLUDED

#ifndef GEOM_INCLUDED
#include "geom.h"
#endif

// Note on alpha values:
// With SRGB, the rgb values are not pre-multiplied by the alpha value
// With RGB8, RGBI and rgb bitmap values the RGB values should be pre-multiplied by the RGB values
//	(against a black background)

struct RGB8 {
	// Note the color order is determined by what the MMX code likes
	U8 blue;
	U8 green;
	U8 red;
	U8 alpha;
};

struct RGBI {	// an RGB struct that uses 16 integers for intermediate calculations like compositing
	// Note the color order is determined by what the MMX code likes
	U16 blue;
	U16 green;
	U16 red;
	U16 alpha;
};

struct RGBIL {	// a struct that can be overlaid over an RGBI for compositing operations
	U32 c0;
	U32 c1;
};

union RGBIU {	// an RGB struct that uses 16 integers for intermedia calculations like compositing
	RGBI i;
	RGBIL l;
};


struct SColorTable {
	int n;
	RGB8 colors[256];
};

enum { bm1Bit, bm2Bit, bm4Bit, bm8Bit, bm16Bit, bm32Bit };	// the four bitmap formats we support

class SBitmapCore {
public:
	// Out bitmap structure
	char* baseAddr;
	SColorTable* cTab;

	int bmFormat;
	int width, height;
	S32 rowBytes;
	int lockCount;
	int transparent;	// set if this bitmap has any transparent alpha values, this is a hint that should be kept up to date

public:
	void GetRGBPixel(int x, int y, RGBI* c);
	
	void GetSSRGBPixel(SFIXED x, SFIXED y, RGBI* c);

	BOOL HasBits() { return baseAddr != 0; }
	void LockBits() { lockCount++; }
	void UnlockBits() { lockCount--; }

	// Create a platform independent bitmap
	void PIInit();
	BOOL PICreate(int format, int w, int h, SColorTable* c, BOOL allowPurge);
	void PIFree();
};

int SBitmapCalcRowbytes(int bmFormat, int width);
int SBitmapBitsPerPixel(int bmFormat);

#ifdef BIG_ENDIAN
inline void SBitmapSwapBits(void* data, S32 bytes, int bmFormat) {}
#else
void SBitmapSwapBits(void* data, S32 bytes, int bmFormat);
#endif


//
// Pixel packing and unpacking routines
//

inline void ExpandColor(RGB8 src, RGBI* dst)
{
	dst->alpha = src.alpha;
	dst->red   = src.red;
	dst->green = src.green;
	dst->blue  = src.blue;
}

inline void PackColor(RGBI* src, RGB8* dst)
{
	dst->alpha = (U8)src->alpha;
	dst->red   = (U8)src->red;
	dst->green = (U8)src->green;
	dst->blue  = (U8)src->blue;
}

inline U16 PackPix16(RGBI* c)
{
	return 	(c->red   & 0xF8) << 7  | 
			(c->green & 0xF8) << 2  | 
			(c->blue  /*& 0xF8*/) >> 3;
}

inline U16 PackPix16A(RGBI* c)
{
	return 	(c->red   & 0xF8) << 8  | 
			(c->green & 0xFC) << 3  | 
			(c->blue  /*& 0xF8*/) >> 3;
}

#if defined(_UNIX) && !defined(GENERATOR) 

extern int g_redShift, g_greenShift, g_blueShift;

inline U32 PackPix32(RGBI* c)
{
  return (U32) ((c->blue  << g_blueShift) |
		(c->green << g_greenShift) |
		(c->red   << g_redShift));
}

inline U32 PackPix32A(RGBI* c, int alpha)
{
  return (U32) ((c->blue  << g_blueShift) |
		(c->green << g_greenShift) |
		(c->red   << g_redShift));
}

#else

inline U32 PackPix32(RGBI* c)
{
	return 	(U32)(c->alpha /*& 0xFF*/) << 24 |
			(U32)(c->red   /*& 0xFF*/) << 16 | 
			(U32)(c->green /*& 0xFF*/) << 8  | 
				 (c->blue  /*& 0xFF*/);
}

inline U32 PackPix32A(RGBI* c, int alpha)
{
	return 	(U32)(c->alpha /*& 0xFF*/) << 24 |
			(U32)(c->red   /*& 0xFF*/) << 16 | 
			(U32)(c->green /*& 0xFF*/) << 8  | 
				 (c->blue  /*& 0xFF*/);
}

#endif

extern const U32 pix16ExpandR[32];
extern const U32 pix16ExpandG[32];
extern const int pix16Expand[32];	// use this for blue
inline void UnpackPix16(U32 pix, RGBI* c)
{
	c->alpha = 255;
	c->red   = pix16Expand[(pix>>10)&0x1f];
	c->green = pix16Expand[(pix>> 5)&0x1f];
	c->blue  = pix16Expand[(pix    )&0x1f];
}

#if defined(_UNIX) && !defined(GENERATOR)
inline U32 Pix16To32(int pix)
{
	return (pix16Expand[(pix>>10)&0x1f]<<g_redShift) |
		   (pix16Expand[(pix>> 5)&0x1f]<<g_greenShift) |
		   (pix16Expand[(pix    )&0x1f]<<g_blueShift);
}
#else
inline U32 Pix16To32(int pix)
{
	return pix16ExpandR[(pix>>10)&0x1f] |
		   pix16ExpandG[(pix>> 5)&0x1f] |
		   pix16Expand [(pix    )&0x1f];
}
#endif

inline void UnpackPix16A(U32 pix, RGBI* c)
{
	c->alpha = 255;
	c->red   = pix16Expand[(pix>>11)&0x1f];
	//c->green = pix16Expand[(pix>> 6)&0x1f];	// note we throw away the extra bit for simplicity
	int i = ((int)pix & 0x07E0) >> 3;
	c->green = i == 0 ? 0 : i | 0x03;
	c->blue  = pix16Expand[(pix    )&0x1f];
}

inline void UnpackPix32(U32 pix, RGBI* c)
{
	c->alpha = (U8)(pix >> 24);
	c->red   = (U8)(pix >> 16);//&0xFF;//((U8*)&pix)[1];
	c->green = (U8)(pix >>  8);//&0xFF;//((U8*)&pix)[2];
	c->blue  = (U8) pix       ;//&0xFF;//((U8*)&pix)[3];
}

inline void UnpackPix32A(U32 pix, RGBI* c, int* alpha)
{
	*alpha   = (U8)(pix >> 24);//&0xFF;//((U8*)&pix)[0];
	c->red   = (U8)(pix >> 16);//&0xFF;//((U8*)&pix)[1];
	c->green = (U8)(pix >>  8);//&0xFF;//((U8*)&pix)[2];
	c->blue  = (U8) pix		  ;//&0xFF;//((U8*)&pix)[3];
}

inline BOOL PreMulAlpha(RGBI* color)
{
	int alpha = color->alpha;
	if ( alpha < 255 ) {
		// Pre-multiply the color values
		alpha++;
		color->red   = alpha*color->red   >> 8;
		color->green = alpha*color->green >> 8;
		color->blue  = alpha*color->blue  >> 8;
		return true;
	} else {
		return false;
	}
}


#endif
