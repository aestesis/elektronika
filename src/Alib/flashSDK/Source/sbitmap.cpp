/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright © Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 
****************************************************************************/

#include <memory.h>
#include <malloc.h>
#include "sbitmap.h"

#include "memcop.h"
#include "raster.h"

#include "zlib/zlib.h"

//
// Utility
//

int SBitmapBitsPerPixel(int bmFormat)
{
	switch ( bmFormat ) {
		case bm1Bit: return 1;
		case bm2Bit: return 2;
		case bm4Bit: return 4;
		case bm8Bit: return 8;
		case bm16Bit: return 16;
		case bm32Bit: return 32;
	}
	FLASHASSERT(false);
	return 0;
}

int SBitmapCalcRowbytes(int bmFormat, int width)
{
	switch ( bmFormat ) {
		case bm1Bit: 
			return ((width + 31) >> 3) & ~3;

		case bm2Bit: 
			return ((width + 15) >> 2) & ~3;

		case bm4Bit: 
			return ((width + 7) >> 1) & ~3;

		case bm8Bit:
			return (width + 3) & ~3;

		case bm16Bit:
			return (2*width + 3) & ~3;

		case bm32Bit:
			return 4*width;
	}
	FLASHASSERT(false);
	return 0;
}


//
// SBitmapCore
//

void SBitmapCore::GetRGBPixel(int x, int y, RGBI* c)
{
	FLASHASSERT(baseAddr);

	// Clip to the edges
	if ( x < 0 ) x = 0;
	if ( y < 0 ) y = 0;
	if ( y >= height ) y = height-1;
	if ( x >= width ) x = width-1;

	U8 * rowAddr = (U8 *)baseAddr + y*rowBytes;
	switch ( bmFormat ) {
		case bm1Bit: {
			U8  *bits = rowAddr + (x>>3);
			int shift = 7 - (x & 0x07);
			ExpandColor(cTab->colors[(*bits >> shift) & 1], c);
		} return;

		case bm2Bit: {
			U8  *bits = rowAddr + (x>>2);
			int shift = (3 -(x & 0x03))<<1;
			ExpandColor(cTab->colors[(*bits >> shift) & 0x3], c);
		} return;

		case bm4Bit: {
			U8  *bits = rowAddr + (x>>1);
			int shift = (1 - (x & 0x01))<<2;
			ExpandColor(cTab->colors[(*bits >> shift) & 0xF], c);
		} return;

		case bm8Bit: {
			ExpandColor(cTab->colors[rowAddr[x]], c);
		} return;

		case bm16Bit: {
			U16 pix = *((U16 *)rowAddr + x);
			UnpackPix16(pix, c);
		} return;

		case bm32Bit: {
			U32 pix = *((U32 *)rowAddr + x);
			UnpackPix32(pix, c);
		} return;
	}
}

// #ifndef WIN16
void SBitmapCore::GetSSRGBPixel(SFIXED xH, SFIXED yH, RGBI* c)
{
	FLASHASSERT(baseAddr);

	int x = (int)(xH >> 16);
	int y = (int)(yH >> 16);
	int xf = (U16)xH >> 13;
	int yf = (U16)yH >> 13;

	CoverEntry cv = PixCoverage[xf][yf];

	// Clip to the edges
	if ( x < 0 ) {
		x = 0;
		cv.ce[0]+=cv.ce[1]; cv.ce[1]=0; // shift the coverage to the leftmost column
		cv.ce[2]+=cv.ce[3]; cv.ce[3]=0;
	} else if ( x >= width-1 ) {
		x = width-2;
		cv.ce[1]+=cv.ce[0]; cv.ce[0]=0; // shift the coverage to the rightmost column
		cv.ce[3]+=cv.ce[2]; cv.ce[2]=0;
	}
	if ( y < 0 ) {
		y = 0;
		cv.ce[0]+=cv.ce[2]; cv.ce[2]=0; // shift the coverage to the top row
		cv.ce[1]+=cv.ce[3]; cv.ce[3]=0;
	} else if ( y >= height-1 ) {
		y = height-2;
		cv.ce[2]+=cv.ce[0]; cv.ce[0]=0; // shift the coverage to the bottom row
		cv.ce[3]+=cv.ce[1]; cv.ce[1]=0;
	}

// #if 1
	RGBIU pix;
	RGBIU p;
	U8 * rowAddr = (U8 *)baseAddr + y*rowBytes;
	switch ( bmFormat ) {
		case bm1Bit: 
		case bm2Bit: 
		case bm4Bit: 
		case bm8Bit: {
			const U8* shiftTab = shiftTabs[bmFormat];	// table of shift values
			int pxMask   = pxMasks[bmFormat];		// mask the same width as a pixel
			int rbShift  = rbShifts[bmFormat];	// shift count to calc rowbytes
			int stMask   = stMasks[bmFormat];		// mask to extract index for shiftTab

			// Get the first column
			U8 * bits = rowAddr + (x>>rbShift);
			int shift = shiftTab[x&stMask];

			ExpandColor(cTab->colors[(bits[0] >> shift) & pxMask], &pix.i);
			pix.l.c0 *= cv.ce[0];
			pix.l.c1 *= cv.ce[0];
			ExpandColor(cTab->colors[(bits[rowBytes] >> shift) & pxMask], &p.i);
			pix.l.c0 += p.l.c0*cv.ce[2];
			pix.l.c1 += p.l.c1*cv.ce[2];

			// Get the second column
			x++;
			bits = rowAddr + (x>>rbShift);
			shift = shiftTab[x&stMask];
			ExpandColor(cTab->colors[(bits[0] >> shift) & pxMask], &p.i);
			pix.l.c0 += p.l.c0*cv.ce[1];
			pix.l.c1 += p.l.c1*cv.ce[1];
			ExpandColor(cTab->colors[(bits[rowBytes] >> shift) & pxMask], &p.i);
			pix.l.c0 += p.l.c0*cv.ce[3];
			pix.l.c1 += p.l.c1*cv.ce[3];
		} break;

		case bm16Bit: {
			U16 * bits = (U16 *)rowAddr + x;
			UnpackPix16(bits[0], &pix.i);
			pix.l.c0 *= cv.ce[0];
			pix.l.c1 *= cv.ce[0];
			UnpackPix16(bits[1], &p.i);
			pix.l.c0 += p.l.c0*cv.ce[1];
			pix.l.c1 += p.l.c1*cv.ce[1];

			// Get the second row
			bits = (U16 *)((U8 *)bits + rowBytes);
			UnpackPix16(bits[0], &p.i);
			pix.l.c0 += p.l.c0*cv.ce[2];
			pix.l.c1 += p.l.c1*cv.ce[2];
			UnpackPix16(bits[1], &p.i);
			pix.l.c0 += p.l.c0*cv.ce[3];
			pix.l.c1 += p.l.c1*cv.ce[3];
		} break;

		case bm32Bit: {
			U32 * bits = (U32 *)rowAddr + x;
			UnpackPix32(bits[0], &pix.i);
			pix.l.c0 *= cv.ce[0];
			pix.l.c1 *= cv.ce[0];
			UnpackPix32(bits[1], &p.i);
			pix.l.c0 += p.l.c0*cv.ce[1];
			pix.l.c1 += p.l.c1*cv.ce[1];

			// Get the second row
			bits = (U32 *)((U8 *)bits + rowBytes);
			UnpackPix32(bits[0], &p.i);
			pix.l.c0 += p.l.c0*cv.ce[2];
			pix.l.c1 += p.l.c1*cv.ce[2];
			UnpackPix32(bits[1], &p.i);
			pix.l.c0 += p.l.c0*cv.ce[3];
			pix.l.c1 += p.l.c1*cv.ce[3];
		} break;
	}

	((RGBIL*)c)->c0 = (pix.l.c0>>3) & 0x00FF00FF;
	((RGBIL*)c)->c1 = (pix.l.c1>>3) & 0x00FF00FF;
}

void SBitmapCore::PIInit()
{
	baseAddr = 0;
	cTab = 0;
	lockCount = 0;
	transparent = false;
}

BOOL HasTransparent(SColorTable* ctab)
{
	if ( !ctab )
		return false;
	RGB8* c = ctab->colors;
	for ( int i = ctab->n; i--; c++ ) {
		if ( c->alpha < 255 )
			return true;
	}
	return false;
}

BOOL SBitmapCore::PICreate(int format, int w, int h, SColorTable* c, BOOL allowPurge)
{
	baseAddr = 0;
	cTab = 0;
	lockCount = 0;
	transparent = ( HasTransparent(c) != 0 );

	bmFormat = format;
	width = w;
	height = h;
	rowBytes = SBitmapCalcRowbytes(bmFormat, width);

	if ( bmFormat <= bm8Bit ) {
		FLASHASSERT(c);

		// Create our color table
		int ctabSize = sizeof(SColorTable) - (256-c->n)*sizeof(RGB8);
		cTab = new SColorTable;
		if ( !cTab ) 
			return false;
		memcpy(cTab, c, ctabSize);
	}

	S32 size = (S32)rowBytes*height;

	baseAddr = (char*)malloc(size);
	return baseAddr != 0;
}

void SBitmapCore::PIFree()
{
	delete [] cTab;//free(cTab);
	cTab = 0;

	if ( baseAddr ) {
		free(baseAddr);
		baseAddr = 0;
	}
}

#ifdef BIG_ENDIAN
// Big-endian system: Defined as an empty inline function.
#else
// Little-endian system: Swap the bits within this function. 
void SBitmapSwapBits(void* data, S32 bytes, int bmFormat)
{
	if ( bmFormat == bm16Bit ) {
		U8 * w = (U8*)data;
		U8 tmp;
		for ( S32 i = bytes/4; i--; ) {
			tmp = w[0];
			w[0] = w[1];
			w[1] = tmp;
			w += 2;

			tmp = w[0];
			w[0] = w[1];
			w[1] = tmp;
			w += 2;
		}
	} else if ( bmFormat == bm32Bit ) {
		U8 * w = (U8*)data;
		U8 tmp;
		for ( S32 i = bytes/4; i--; ) {
			tmp = w[0];
			w[0] = w[3];
			w[3] = tmp;
			tmp = w[1];
			w[1] = w[2];
			w[2] = tmp;

			w += 4;
		}
	}
}
#endif

//PLAYER

//
// Pixel Packing support
//

#define Epnd16(x)  (((x)<<3)|0x7)
const int pix16Expand[32] = {	
		0, 			  Epnd16(0x01), Epnd16(0x02), Epnd16(0x03), 
		Epnd16(0x04), Epnd16(0x05), Epnd16(0x06), Epnd16(0x07), 
		Epnd16(0x08), Epnd16(0x09), Epnd16(0x0A), Epnd16(0x0B), 
		Epnd16(0x0B), Epnd16(0x0D), Epnd16(0x0E), Epnd16(0x0F),
		Epnd16(0x10), Epnd16(0x11), Epnd16(0x12), Epnd16(0x13), 
		Epnd16(0x14), Epnd16(0x15), Epnd16(0x16), Epnd16(0x17), 
		Epnd16(0x18), Epnd16(0x19), Epnd16(0x1A), Epnd16(0x1B), 
		Epnd16(0x1B), Epnd16(0x1D), Epnd16(0x1E), Epnd16(0x1F),
	};

#define Epnd16R(x) ((Epnd16(x)<<16)|0xff000000L)	// add the alpha value
const U32 pix16ExpandR[32] = {	
		0xFF000000L,   Epnd16R(0x01), Epnd16R(0x02), Epnd16R(0x03), 
		Epnd16R(0x04), Epnd16R(0x05), Epnd16R(0x06), Epnd16R(0x07), 
		Epnd16R(0x08), Epnd16R(0x09), Epnd16R(0x0A), Epnd16R(0x0B), 
		Epnd16R(0x0B), Epnd16R(0x0D), Epnd16R(0x0E), Epnd16R(0x0F),
		Epnd16R(0x10), Epnd16R(0x11), Epnd16R(0x12), Epnd16R(0x13), 
		Epnd16R(0x14), Epnd16R(0x15), Epnd16R(0x16), Epnd16R(0x17), 
		Epnd16R(0x18), Epnd16R(0x19), Epnd16R(0x1A), Epnd16R(0x1B), 
		Epnd16R(0x1B), Epnd16R(0x1D), Epnd16R(0x1E), Epnd16R(0x1F),
	};

#define Epnd16G(x)  (Epnd16(x)<< 8)
const U32 pix16ExpandG[32] = {	
		0, 			   Epnd16G(0x01), Epnd16G(0x02), Epnd16G(0x03), 
		Epnd16G(0x04), Epnd16G(0x05), Epnd16G(0x06), Epnd16G(0x07), 
		Epnd16G(0x08), Epnd16G(0x09), Epnd16G(0x0A), Epnd16G(0x0B), 
		Epnd16G(0x0B), Epnd16G(0x0D), Epnd16G(0x0E), Epnd16G(0x0F),
		Epnd16G(0x10), Epnd16G(0x11), Epnd16G(0x12), Epnd16G(0x13), 
		Epnd16G(0x14), Epnd16G(0x15), Epnd16G(0x16), Epnd16G(0x17), 
		Epnd16G(0x18), Epnd16G(0x19), Epnd16G(0x1A), Epnd16G(0x1B), 
		Epnd16G(0x1B), Epnd16G(0x1D), Epnd16G(0x1E), Epnd16G(0x1F),
	};

