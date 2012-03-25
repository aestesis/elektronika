
/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright © Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 
****************************************************************************/

#include <memory.h>	   // lee@middlesoft - thanks E. Hyche, RealNetworks, for providing header corrections.
#include "raster.h"
#include "bitbuf.h"
#include "sbitmap.h"

#include NATIVE_SOUND
#include "sndmix.h"

enum { RGBSlabChunkSize=256 };

// Rendering tables

CoverEntry PixCoverage[8][8];
BOOL renderTablesBuilt = false;

enum { dd = 0x7 };
const U8 ditherNoise[8*8] = {
		dd,00,00,dd,00,dd,dd,00,
		00,dd,00,dd,00,dd,00,dd,
		00,dd,dd,00,dd,00,dd,dd,
		00,dd,dd,00,dd,00,dd,dd,
		dd,00,dd,00,00,dd,00,dd,
		00,dd,00,dd,dd,00,dd,00,
		dd,dd,00,dd,00,dd,00,00,
		00,dd,dd,00,dd,00,dd,dd,
	};

const U8 ditherNoiseX[8*8] = {
		0,1,2,3,1,2,1,0,
		2,3,0,1,0,3,2,3,
		3,1,2,0,2,1,2,0,
		1,0,3,2,1,0,1,3,
		2,3,1,0,3,2,3,2,
		0,0,3,1,2,3,1,3,
		3,2,1,0,0,2,0,1,
		2,3,0,1,0,3,2,3,
	};

// A dither matrix for reducing 24bit to 16 bit color
const U8 ditherNoise16[4*4] = {
		7,0,5,1,	// Random dither
		2,6,4,3,
		4,3,7,2,
		0,6,1,5,
	};

// Tables for our ordered dither
const U8 ditherNoise8[8*8] = {	// 0..31
		 1/2, 17/2,  0/2, 16/2,  2/2, 18/2, 30/2, 15/2,
		21/2, 13/2,  5/2, 28/2, 22/2, 14/2,  6/2, 27/2,
		 9/2, 29/2, 20/2,  4/2, 10/2, 31/2, 23/2,  3/2,
		25/2,  8/2, 24/2, 12/2, 26/2,  7/2, 11/2, 19/2,

		 3/2, 19/2, 30/2, 14/2,  4/2, 20/2,  0/2, 13/2,
		23/2, 15/2,  7/2, 26/2, 24/2, 16/2,  8/2, 25/2,
		11/2, 31/2, 18/2,  2/2, 12/2, 29/2, 21/2,  1/2,
		27/2,  6/2, 22/2, 10/2, 28/2,  5/2,  9/2, 17/2,
	};

// Tables for the the 16 bit dither
#define p16pR(x) (x<<10)
#define p16pG(x) (x<<5)
#define p16pB(x) (x<<0)
const U16 pix16PackR[33] = {
				p16pR(0x00), p16pR(0x01), p16pR(0x02), p16pR(0x03), 
				p16pR(0x04), p16pR(0x05), p16pR(0x06), p16pR(0x07), 
				p16pR(0x08), p16pR(0x09), p16pR(0x0A), p16pR(0x0B), 
				p16pR(0x0C), p16pR(0x0D), p16pR(0x0E), p16pR(0x0F), 

				p16pR(0x10), p16pR(0x11), p16pR(0x12), p16pR(0x13), 
				p16pR(0x14), p16pR(0x15), p16pR(0x16), p16pR(0x17), 
				p16pR(0x18), p16pR(0x19), p16pR(0x1A), p16pR(0x1B), 
				p16pR(0x1C), p16pR(0x1D), p16pR(0x1E), p16pR(0x1F), p16pR(0x1F)
			};
const U16 pix16PackG[33] = {
				p16pG(0x00), p16pG(0x01), p16pG(0x02), p16pG(0x03), 
				p16pG(0x04), p16pG(0x05), p16pG(0x06), p16pG(0x07), 
				p16pG(0x08), p16pG(0x09), p16pG(0x0A), p16pG(0x0B), 
				p16pG(0x0C), p16pG(0x0D), p16pG(0x0E), p16pG(0x0F), 

				p16pG(0x10), p16pG(0x11), p16pG(0x12), p16pG(0x13), 
				p16pG(0x14), p16pG(0x15), p16pG(0x16), p16pG(0x17), 
				p16pG(0x18), p16pG(0x19), p16pG(0x1A), p16pG(0x1B), 
				p16pG(0x1C), p16pG(0x1D), p16pG(0x1E), p16pG(0x1F), p16pG(0x1F)
			};
const U8 pix16PackB[33] = {
				p16pB(0x00), p16pB(0x01), p16pB(0x02), p16pB(0x03), 
				p16pB(0x04), p16pB(0x05), p16pB(0x06), p16pB(0x07), 
				p16pB(0x08), p16pB(0x09), p16pB(0x0A), p16pB(0x0B), 
				p16pB(0x0C), p16pB(0x0D), p16pB(0x0E), p16pB(0x0F), 

				p16pB(0x10), p16pB(0x11), p16pB(0x12), p16pB(0x13), 
				p16pB(0x14), p16pB(0x15), p16pB(0x16), p16pB(0x17), 
				p16pB(0x18), p16pB(0x19), p16pB(0x1A), p16pB(0x1B), 
				p16pB(0x1C), p16pB(0x1D), p16pB(0x1E), p16pB(0x1F), p16pB(0x1F)
			};

// Tables for extracting pixels from bytes
const U8 shiftTab1[8] = {7,6,5,4,3,2,1,0};
const U8 shiftTab2[4] = {6,4,2,0};
const U8 shiftTab4[2] = {4,0};
const U8 shiftTab8[1] = {0};

							// bm1bit, bm2bit, bm4bit, bm8bit
const U8* shiftTabs[4] = {shiftTab1, shiftTab2, shiftTab4, shiftTab8};
const U8  rbShifts[4]  = {  3,   2,   1,   0 };
const U8  pxMasks[4]   = {0x1, 0x3, 0xF, 0xFF};
const U8  stMasks[4]   = {0x7, 0x3, 0x1, 0x0 };

#define dr(x) ((x)<<8)
#define dg(x) ((x)<<4)
#define db(x) ((x)<<0)
const U16 ditherTabR[]={dr(0), dr(1), dr(2), dr(3), dr(4), dr(5), dr(6), dr(7), dr(8), 
				  dr(9), dr(10), dr(11), dr(12), dr(13), dr(14), dr(15), dr(15), dr(15), dr(15)};
const U8 ditherTabG[] ={dg(0), dg(1), dg(2), dg(3), dg(4), dg(5), dg(6), dg(7), dg(8), 
				  dg(9), dg(10), dg(11), dg(12), dg(13), dg(14), dg(15), dg(15), dg(15), dg(15)};
const U8 ditherTabB[] ={db(0), db(1), db(2), db(3), db(4), db(5), db(6), db(7), db(8), 
				  db(9), db(10), db(11), db(12), db(13), db(14), db(15), db(15), db(15), db(15)};


static U16 Sq256[258];

void BuildRenderTables()
{
	renderTablesBuilt = true;

	// Build the coverage table
 	//FLASHOUTPUT("Coverage Table\n");
 	for ( int xf = 0; xf < 8; xf++ ) {
		for ( int yf = 0; yf < 8; yf++ ) {
			int* ce = PixCoverage[xf][yf].ce;
			ce[0] = (8-xf)*(8-yf);
			ce[1] = xf*(8-yf);
			ce[2] = (8-xf)*yf;
			ce[3] = xf*yf;

			// Normalize the sum of coverages to 8
			int big = 0;	// add the error to the largest weight
			int sum = 0;
			for ( int j = 0; j <= 3; j++ ) {
				ce[j] = (ce[j]+4)/8;
				sum += ce[j];
				if ( ce[j] > ce[big] ) big = j;
			}
			int err = 8-sum;
			ce[big] += err;
			//FLASHOUTPUT("%i, ", ce[0]);
		}
		//FLASHOUTPUT("\n");
	}

	// Build a table of the first 256 squares for radial fills
	for ( U32 j = 0; j < 258; j++ )
		Sq256[j] = (U16)Min(0xFFFF, j*j);
}

static const U16 grayPatList[] = {
		0xFFFF, //0
		0x7FFF, 
		0x7FDF, 
		0x5FDF, 
		0x5F5F, //4
		0x5B5F,
		0x5B5E,
		0x5A5E,
		0x5A5A, //8
		0x1A5A,
		0x1A4A,
		0x0A4A,
		0x0A0A, //12
		0x020A,
		0x0208,
		0x0008,
		0x0000  //16
	};

// Use more distinctive patterns on PC for higher dpi screens
#define linePatBits 	0x39C6 // diag lines
#define fillPatBits 	0x33CC // 2x2 blocks

#define solidPatBits 	0xFFFF // solid
#define disablePatBits 	0xA5A5 // 50% 

// #endif


//
// Proc Tables
//

const DitherRGBSlabProc DitherRGBSlabProcs[3] = {
	DitherRGBSlabNone,
	DitherRGBSlabOrdered,
	DitherRGBSlabErrorDiffusion
};

const DrawRGBSlabProc DrawRGBSlabProcs[9] = {
	DrawRGBSlab1,
	DrawRGBSlab2,
	DrawRGBSlab4,
	DrawRGBSlab8,
	DrawRGBSlab16,
	DrawRGBSlab16A,
	DrawRGBSlab24,
	DrawRGBSlab32,
	DrawRGBSlab32A
};

const DrawRGBSlabProc GetBackgroundProcs[9] = {
	GetBackground1,
	GetBackground2,
	GetBackground4,
	GetBackground8,
	GetBackground16,
	GetBackground16A,
	GetBackground24,
	GetBackground32,
	GetBackground32
};



//
// Doubly Linked List
//

// head must be the pointer to the list start
// elem must have a next and prev field

// Add to the elem to the front of the list
#define DListAdd(head, elem, type) { \
		if ( head ) \
			head->prev = elem; \
		elem->next = head; \
		elem->prev = 0; \
		head = elem; \
	}

// Add elem after node
#define DListInsertAfter(node, elem, type) { \
		type *tmp; \
		if ( (tmp = node->next) != 0 ) \
			tmp->prev = elem; \
		elem->next = tmp; \
		elem->prev = node; \
		node->next = elem; \
	}

// Remove elem from the list
#define DListRemove(head, elem, type) \
	{ \
		type *tmp; \
		if ( (tmp = elem->prev) != 0 ) \
			tmp->next = elem->next; \
		else if ( head == elem ) \
			head = elem->next; \
		if ( (tmp = elem->next) != 0 ) \
			tmp->prev = elem->prev; \
		elem->next = elem->prev = 0; \
	}

// Swap two elements, first must be immediately before second
#define DListSwap(head, first, second, type) \
	{ \
		DListRemove(head, first, type); \
		DListInsertAfter(second, first, type); \
	}


//
// Helpers for wrapping texture coordinates

inline S32 LimitAbs(S32 v, S32 range)
// An absolute wrapper
{
	S32 i = v/range;
	if ( v < 0 ) 
		i--;
	return v - i*range;
}

inline int LimitAbsI(int v, S32 range)
// An absolute wrapper
{
	int i = (int)(v/range);
	if ( v < 0 ) 
		i--;
	return (int)(v - i*range);
}

inline void CalcLimit(int& n, S32& x, S32 dx, S32 limit)
{
	int lim;
	if ( dx > 0 ) {
		while ( x >= limit )
			x -= limit;
		lim = (int)((limit-x+dx-1)/dx);
		if ( n > lim )
			n = lim;
	} else if ( dx < 0 ) {
		while ( x <= 0 )
			x += limit;
		lim = (int)((x-dx-1)/-dx);
		if ( n > lim )
			n = lim;
	}
}


void ApplyRGBPattern(int pattern, int y, int xmin, int xmax, RGBI* pix);

//
// Low-Level Compositing Functions
//

inline void CompositeRGBPixel(RGBI* src, RGBI* dst)
{
	int a1 = 256-src->alpha;
	((RGBIL*)dst)->c0 = (((RGBIL*)src)->c0 + (((RGBIL*)dst)->c0*a1>>8)) & 0x00FF00FF;
	((RGBIL*)dst)->c1 = (((RGBIL*)src)->c1 + (((RGBIL*)dst)->c1*a1>>8)) & 0x00FF00FF;
}

void CompositeRGB(RGBI* src, RGBI* dst, int n)
{
	while ( n-- > 0 ) {
		int a1 = 256-src->alpha;
		((RGBIL*)dst)->c0 = (((RGBIL*)src)->c0 + (((RGBIL*)dst)->c0*a1>>8)) & 0x00FF00FF;
		((RGBIL*)dst)->c1 = (((RGBIL*)src)->c1 + (((RGBIL*)dst)->c1*a1>>8)) & 0x00FF00FF;
		src++; dst++;
	}
}

void CompositeRGBSolid(RGBI* src, RGBI* dst, int n)
{
	while ( n-- > 0 ) {
		int a1 = 256-src->alpha;
		((RGBIL*)dst)->c0 = (((RGBIL*)src)->c0 + (((RGBIL*)dst)->c0*a1>>8)) & 0x00FF00FF;
		((RGBIL*)dst)->c1 = (((RGBIL*)src)->c1 + (((RGBIL*)dst)->c1*a1>>8)) & 0x00FF00FF;
		dst++;
	}
}

//
// Raster Inline functions
//

inline void CRaster::IndexToRGB(int index, RGBI* color)
// Find the color that corresponds to an index into the color table
{
	FLASHASSERT(cinfo && index < cinfo->ctab.n);
	ExpandColor(cinfo->ctab.colors[index], color);
}

inline int CRaster::RGBToIndex(RGBI* color)
// Search the inverse table for an entry that most closely matches the color
{
	FLASHASSERT( cinfo );
	FLASHASSERT(cinfo->itab);
	return cinfo->itab[CalcITabIndexRGB(color)];
}

inline void CRaster::DrawRGBAPixel(S32 x, RGBI* rgb)
{
	if ( rgb->alpha == 255 ) {
		drawRGBSlab(this, x, x+1, rgb);
	} else {
		RGBI pix;
		doGetBackground(this, x, x+1, &pix);
		CompositeRGBPixel(rgb, &pix);
		drawRGBSlab(this, x, x+1, &pix);
	}
}

void CRaster::DrawRGBASlab(S32 xleft, S32 xright, RGBI* rgb)
{
	FLASHASSERT(xright - xleft <= RGBSlabChunkSize);

	RGBI pixBuf[RGBSlabChunkSize];
	doGetBackground(this, xleft, xright, pixBuf);
	CompositeRGB(rgb, pixBuf, xright - xleft);
	drawRGBSlab(this, xleft, xright, pixBuf);
}


//
// The Color Object
// 

void RColor::SetUp(CRaster* r)
{
	raster = r;
	nextColor = nextActive = 0;
	visible = 0;
	transparent = false;
	colorType = colorSolid;
	cacheValid = false;
	clippedBy = 0;
	//priority = 0;
	//order = 0;
	//color = c;
	//rgb.red = green = blue = 0;
	pattern = 0;
}

void RColor::FreeCache()
{
	if ( cacheValid ) {
		cacheValid = false;
		switch ( colorType ) {
			case colorBitmap:
				bm.bitmap->UnlockBits();
				if ( bm.colorMap ) {
					raster->FreeMap(bm.colorMap);
					bm.colorMap = 0;	
				}
				break;
			case colorGradient:
				if ( grad.colorRamp ) {
					raster->FreeRamp(grad.colorRamp);
					grad.colorRamp = 0;	
				}
				break;
		}
	}
}


//
// Helpers for building dither patterns
//

inline int PinColor(int c)
{
	if ( c < 0 ) c = 0;
	if ( c > 255 ) c = 255;
	return c;
}

void ExpandPat(int src, int xPatAlign, U32 px1, U32 px2, int pixelFormat, int rowLongs, U32* dst)
{
	for ( int y = 0; y < 4; y++, dst += rowLongs ) {
		U32* d = dst;

		int s = (src >> (3-y)*4) & 0xF;	// get the 4 bits for this row from the pattern word
		
		// Align the pattern on the x axis
		s <<= xPatAlign&3;
		s = (s | (s >> 4)) & 0xF;	// handle the wrap around for alignment

		switch ( pixelFormat ) {
			case pix1: {
				// Create 4 copies of the 1 byte pattern
				U8* db = (U8*)d;
				s |= s<<4;
				if ( px1 && px2 ) s = 0xFF;
				else if ( !px1 && !px2 ) s = 0x00;
				else if ( !px1 && px2 ) s = ~s;
				db[0] = (U8)s;
				db[1] = (U8)s;
				db[2] = (U8)s;
				db[3] = (U8)s;
			} break;

			case pix2: {
				// the pat expands to 1 byte which we replicate to 4 byte
				U8* db = (U8*)d;
				U8 p = 0;
				int shift = 6;
				for ( int x = 4; x--; s <<= 1, shift -= 2 )
					p |= (U8)(s & 0x8 ? px1 : px2) << shift;
				
				db[0] = p;
				db[1] = p;
				db[2] = p;
				db[3] = p;
			} break;

			case pix4: {
				// the pat expands to 2 bytes which we replicate to 4 bytes
				U8* db = (U8*)d;
				for ( int x = 2; x--; db++, s <<= 2 ) {
					*db = ((U8)(s & 0x8 ? px1 : px2) << 4) |
						  ((U8)(s & 0x4 ? px1 : px2) 	 );
				}
				db = (U8*)d;
				db[2] = db[0];
				db[3] = db[1];
			} break;

			case pix8: {
				// the pat expands to 4 byte rows
				U8* db = (U8*)d;
				for ( int x = 4; x--; db++, s <<= 1 )
					*db = (U8)(s & 0x8 ? px1 : px2);
			} break;

			case pix16: 
			case pix16A: {
				// the pat expands to 8 byte rows
				U16* dw = (U16*)d;
				for ( int x = 4; x--; dw++, s <<= 1 )
					*dw = (U16)(s & 0x8 ? px1 : px2);
			} break;

			case pix24: {
				// the pat expands to 12 byte rows
				U8* db = (U8*)d;
				for ( int x = 4; x--; db+=3, s <<= 1 ) {
					U32 px = (s & 0x8 ? px1 : px2);
					db[2] = (U8)(px >> 16);	// red
					db[1] = (U8)(px >> 8);	// green
					db[0] = (U8)(px);		// blue
				}
			} break;

			case pix32:
			case pix32A: {
				// the pat expands to 16 byte rows
				for ( int x = 4; x--; d++, s <<= 1 )
					*d = (s & 0x8 ? px1 : px2);
			} break;
		}
	}
}

static const DrawSlabProc solidSlabProcTable[] = {
		DrawSolidSlab1,
		DrawSolidSlab2,
		DrawSolidSlab4,
		DrawSolidSlab8,
		DrawSolidSlab16,
		DrawSolidSlab16,
		DrawSolidSlab24,
		DrawSolidSlab32,
		DrawSolidSlab32
	};

static const BltProc FastBltProcs8D[] = {	// index by dest pixel format
		0,	//bm1Bit
		0,	//bm2Bit
		0,	//bm4Bit
		(BltProc)Blt8to8D,	//bm8Bit
		(BltProc)Blt16to8D,	//bm16Bit
		(BltProc)Blt32to8D	//bm32Bit
	};

static const BltProc FastBltProcs16[] = {	// index by dest pixel format
		0,	//bm1Bit
		0,	//bm2Bit
		0,	//bm4Bit
		(BltProc)Blt8to16,	//bm8Bit
		(BltProc)Blt16to16,	//bm16Bit
		(BltProc)Blt32to16	//bm32Bit
	};

static const BltProc FastBltProcs16A[] = {	// index by dest pixel format
		0,	//bm1Bit
		0,	//bm2Bit
		0,	//bm4Bit
		(BltProc)Blt8to16A,	//bm8Bit
		(BltProc)Blt16to16A,	//bm16Bit
		(BltProc)Blt32to16A	//bm32Bit
	};

static const BltProc FastBltProcs32[] = {	// index by dest pixel format
		0,	//bm1Bit
		0,	//bm2Bit
		0,	//bm4Bit
		(BltProc)Blt8to32,	//bm8Bit
		(BltProc)Blt16to32,	//bm16Bit
		(BltProc)Blt32to32	//bm32Bit
	};

static const BltProc* FastBltProcs[] = {	// index by source bitmap format
		0,		// pix1
		0,		// pix2
		0,		// pix4
		FastBltProcs8D,
		FastBltProcs16,
		FastBltProcs16A,		// pix16A
		0,		// pix24
		FastBltProcs32,
		0		// pix32A
	};

static const BltProc GeneralBltProcs[] = {	// index by bitmap format
		(BltProc)BltXtoI,
		(BltProc)BltXtoI,
		(BltProc)BltXtoI,
		(BltProc)Blt8toI,
		(BltProc)Blt16toI,
		(BltProc)Blt32toI
	};

#ifdef SMOOTHBITS
static const BltProc SmoothBltProcs[] = {	// index by bitmap format
		(BltProc)BltXtoI,
		(BltProc)BltXtoI,
		(BltProc)BltXtoI,
		(BltProc)Blt8toIS,
		(BltProc)Blt16toIS,
		(BltProc)Blt32toIS
	};

static const BltProc SmoothBltProcsA[] = {	// index by bitmap format
		(BltProc)BltXtoI,
		(BltProc)BltXtoI,
		(BltProc)BltXtoI,
		(BltProc)Blt8toISA,
		(BltProc)Blt16toISA,
		(BltProc)Blt32toISA
	};

#endif


//
// Color Transform helpers for bitmaps
//

void BuildMapChannel(int a, int b, U8* map)
{
	S32 c = (S32)b << 8;
	int n = 256;
	while ( n-- ) {
		if ( (c & 0xFFFF0000) == 0 )
			*map++ = (U8)(c>>8);
		else if ( c > 0 ) 
			*map++ = 255;
		else
			*map++ = 0;
		c += a;
	}
}

// Applying a color transform to bitmap values with alpha is messy...
// 1) Do an inverse pre-multiply on the the pixel
// 2) Apply the color transform
// 3) Do the pre-multiply on the transformed pixel
void ApplyColorMap(RColorMap* map, RGBI* c)
{
	FLASHASSERT(map);
	FLASHASSERT(c->red <= c->alpha && c->green <= c->alpha && c->blue <= c->alpha);
	if ( map->hasAlpha ) {
		FLASHASSERT(c->red <= c->alpha && c->green <= c->alpha && c->blue <= c->alpha);
		int a = map->alpha[c->alpha];
		if ( a == 0 ) {
			c->alpha = c->red = c->green = c->blue = 0;
		} else {
			int k = (256*255L)/c->alpha;
			c->alpha = a;
			a++;
			c->red   = a*map->red  [k*c->red  >>8] >> 8;
			c->green = a*map->green[k*c->green>>8] >> 8;
			c->blue  = a*map->blue [k*c->blue >>8] >> 8;
			FLASHASSERT((unsigned)c->red < 256 && (unsigned)c->green < 256 && (unsigned)c->blue < 256);
		}
	} else {
		c->red   = map->red  [c->red  ];
		c->green = map->green[c->green];
		c->blue  = map->blue [c->blue ];
		c->alpha = map->alpha[c->alpha];
	}
}

void ApplyColorMap(RColorMap* map, RGBI* c, int n)
{
	FLASHASSERT(map);
	if ( map->hasAlpha ) {
		while ( n-- ) {
			FLASHASSERT(c->red <= c->alpha && c->green <= c->alpha && c->blue <= c->alpha);
			int a = map->alpha[c->alpha];
			if ( a == 0 ) {
				c->alpha = c->red = c->green = c->blue = 0;
			} else {
				int k = (256*255L)/c->alpha;
				c->alpha = a;
				a++;
				FLASHASSERT((k*c->red>>8) < 256 && (k*c->green>>8) < 256 && (k*c->blue>>8) < 256);
				c->red   = a*map->red  [k*c->red  >>8] >> 8;
				c->green = a*map->green[k*c->green>>8] >> 8;
				c->blue  = a*map->blue [k*c->blue >>8] >> 8;
				FLASHASSERT((unsigned)c->red < 256 && (unsigned)c->green < 256 && (unsigned)c->blue < 256);
			}
			c++;
		}
	} else {
		while ( n-- ) {
			c->red   = map->red  [c->red  ];
			c->green = map->green[c->green];
			c->blue  = map->blue [c->blue ];
			c->alpha = map->alpha[c->alpha];
			c++;
		}
	}
}

void RColor::BuildCache()
{
	if ( cacheValid ) return;

	switch ( colorType ) {
		case colorSolid: {
			// Pick a pattern and a background RGB color
			drawSlabProc = solidSlabProcTable[raster->pixelFormat];
			compositeSlabProc = CompositeSolidSlab;

			RGBI bkColor;
			U16 patSrc;
			switch ( pattern ) {
		 	 	case solidPat:
					if ( raster->solidDither == errorDiffusionDither ) {
					 	drawSlabProc = DrawDitherSlab;
						return;
					}

					bkColor = rgb;
					patSrc = solidPatBits;
		 	 		break;
 	
		 	 	case disablePat: {
					patSrc = disablePatBits;
					bkColor.red = bkColor.green = bkColor.blue = 255;
		 	 	} break;
 	
		 	 	case highlightPat: 
		 	 	case lineHighlightPat: {
					patSrc = pattern == lineHighlightPat ? linePatBits : fillPatBits;

					bkColor.red   = rgb.red ^ 0xFF;
					bkColor.green = rgb.green ^ 0xFF;
					bkColor.blue  = rgb.blue ^ 0xFF;

		 	 	} break;
			}

			// Pick two pixel values
			U32 px1, px2;
			switch ( raster->pixelFormat ) {
				case pix1: 
					if ( pattern == solidPat ) {
						// create a gray dither pattern
						px1 = raster->cinfo->itab[CalcITabIndex(0,0,0)];
						if ( raster->solidDither == noDither ) {
							px2 = px1;
						} else {
							px2 = raster->cinfo->itab[CalcITabIndex(0xff,0xff,0xff)];
							int gray = (rgb.red + rgb.green + rgb.blue + 3) / 48;
							patSrc = grayPatList[gray];
						}
					} else {
						// Use whatever colors they chose
						px1 = raster->RGBToIndex(&rgb);
						px2 = raster->RGBToIndex(&bkColor);
					}
					break;
			  
				case pix2:
				case pix4:
				case pix8:
				{
					px1 = raster->RGBToIndex(&rgb);

					if ( pattern == solidPat ) {
						if ( raster->solidDither == noDither ) {
							px2 = px1;
						} else {
							// Implement a simple dither here...
							RGBI c1, c2;

							raster->IndexToRGB((int)px1, &c1);
							int redErr1   = c1.red   - rgb.red;
							int greenErr1 = c1.green - rgb.green;
							int blueErr1  = c1.blue  - rgb.blue;
							int err1 = Abs(redErr1) + Abs(greenErr1) + Abs(blueErr1);
							if ( err1 < 0x08 ) {
								px2 = px1;
							} else {
								c2.red   = PinColor(rgb.red   - redErr1);
								c2.green = PinColor(rgb.green - greenErr1);
								c2.blue  = PinColor(rgb.blue  - blueErr1);
								px2 = raster->RGBToIndex(&c2);
								raster->IndexToRGB((int)px2, &c2);
								int redErr2   = c2.red   - rgb.red;
								int greenErr2 = c2.green - rgb.green;
								int blueErr2  = c2.blue  - rgb.blue;

								// Pick a pattern based on the relative error
								int err2 = Abs(redErr2) + Abs(greenErr2) + Abs(blueErr2);
								int gray = (16*err1) / (err1 + err2);
								if ( gray > 15 ) gray = 15;
								patSrc = grayPatList[gray];
							}
						}

					} else {
						px2 = raster->RGBToIndex(&bkColor);
					}
				} break;

				case pix16: {
					px1 = PackPix16(&rgb);
					px2 = PackPix16(&bkColor);
				} break;

				case pix16A: {
					px1 = PackPix16A(&rgb);
					px2 = PackPix16A(&bkColor);
				} break;

				case pix24:
				case pix32: {
					px1 = PackPix32(&rgb);
					px2 = PackPix32(&bkColor);
				} break;

				case pix32A: {
					px1 = PackPix32A(&rgb, 0xFF);
					px2 = PackPix32A(&bkColor, 0xFF);
				} break;
			}
			
			FLASHASSERT(raster->bits);
			ExpandPat(patSrc, (int)raster->patAlign.x & 0x3, px1, px2, raster->pixelFormat, 4, pat);
		} break;

		case colorBitmap: {
			if ( !bm.bitmap || !bm.bitmap->HasBits() ) {
				colorType = colorSolid;
			 	drawSlabProc = DrawDitherSlab;
				compositeSlabProc = CompositeSolidSlab;

			} else {
				drawSlabProc = DrawBitmapSlab;
				compositeSlabProc = transparent ? CompositeBitmapSlab : BuildBitmapSlab;

				bm.bitmap->LockBits();	// the Mac puts bits in handles

				// Set up the blt info
				bm.bi.dx = bm.invMat.a;
				bm.bi.dy = bm.invMat.b;

				bm.bi.color = this;
				bm.bi.baseAddr = (U8*)bm.bitmap->baseAddr;
				bm.bi.rowBytes = bm.bitmap->rowBytes;
				bm.bi.width = bm.bitmap->width;
				bm.bi.height = bm.bitmap->height;
				bm.bi.colors = bm.bitmap->cTab ? bm.bitmap->cTab->colors : 0;
				bm.bi.itab = raster->cinfo ? raster->cinfo->itab : 0;

				if ( bm.cxform.HasTransform() ) {
					// build a color map
					RColorMap* cm = bm.colorMap = raster->CreateMap();
					if ( cm ) {
						cm->hasAlpha = bm.cxform.aa < 256 || bm.cxform.ab < 0 || transparent;
						BuildMapChannel(bm.cxform.ra, bm.cxform.rb, cm->red);
						BuildMapChannel(bm.cxform.ga, bm.cxform.gb, cm->green);
						BuildMapChannel(bm.cxform.ba, bm.cxform.bb, cm->blue);
						BuildMapChannel(bm.cxform.aa, bm.cxform.ab, cm->alpha);
						cm->alpha[0] = 0;	// this is required for the map transform
					}

				} else {
					bm.colorMap = 0;
				}

				bm.fastBltProc = 0;

				#ifdef SMOOTHBITS
				if ( raster->smoothBitmaps && !(bm.bitsStyle&fillBitsNoSmooth) && 
							( ((Abs(bm.invMat.a)+0xF)&~0x1F) != fixed_1 || 
							  ((Abs(bm.invMat.b)+0xF)&~0x1F) != 0 || 
							  bm.invMat.b != 0 || bm.invMat.c != 0) ) {
					// Smooth the bitmap
					bm.smooth = true;

					if ( transparent )
						bm.bltProc = SmoothBltProcsA[bm.bitmap->bmFormat];	// preserve the alpha
					else
						bm.bltProc = SmoothBltProcs[bm.bitmap->bmFormat];	// assume opaque alpha
				} else
				#endif
				{// Don't smooth the bitmap, so check for fast blt procs
					bm.smooth = false;
					bm.bltProc = GeneralBltProcs[bm.bitmap->bmFormat];
					if ( pattern == 0 && !bm.colorMap &&
						 (raster->pixelFormat != pix8 || raster->bitmapDither < errorDiffusionDither) ) {
						const BltProc* procs = FastBltProcs[raster->pixelFormat];
						if ( procs ) 
							bm.fastBltProc = procs[bm.bitmap->bmFormat];
					}
				}
			}
		} break;

		case colorGradient: {
			drawSlabProc = DrawGradientSlab;
			compositeSlabProc = transparent ?

								CompositeGradientSlab : 
								BuildGradientSlab;

			// Create the color ramp
			RGB8* c = grad.colorRamp = raster->CreateRamp();
			if ( !c ) {
			 	drawSlabProc = DrawDitherSlab;
				compositeSlabProc = CompositeSolidSlab;
				return;
			}
			SRGB c1, c2;
			S32 cr1, cr2;
			S32 w1, w2, wT;

			cr1 = 0;
			cr2 = grad.ramp.colorRatio[0];
			c1 = c2 = grad.ramp.color[0];
			int ci = 1;

			for ( int i = 0; i <= 256; i++, c++ ) {
				if ( i > cr2 ) {
					cr1 = cr2;
					c1 = c2;
				 	if ( ci < grad.ramp.nColors ) {
						// Get the next color in the ramp
						cr2 = grad.ramp.colorRatio[ci];
						c2 = grad.ramp.color[ci];
						ci++;
					} else {
						// We went past the end
						cr2 = 256;
					}
				}
				w1 = cr2 - i;
				w2 = i - cr1;
				wT = w1+w2;
				if ( wT > 0 ) {
					// change div to fixed mul
					c->alpha = (U8)((w1*c1.rgb.transparency + w2*c2.rgb.transparency)/wT);
					c->red   = (U8)((w1*c1.rgb.red   + w2*c2.rgb.red  )/wT);
					c->green = (U8)((w1*c1.rgb.green + w2*c2.rgb.green)/wT);
					c->blue  = (U8)((w1*c1.rgb.blue  + w2*c2.rgb.blue )/wT);
				} else {
					c->alpha = c1.rgb.transparency;
					c->red   = c1.rgb.red;
					c->green = c1.rgb.green;
					c->blue  = c1.rgb.blue;
				}

				{// Pre-multiply the RGB value
					int a = c->alpha;
					if ( a < 255 ) {
						if ( a == 0 ) {
							c->red = c->green = c->blue = 0;
						} else {
							a++;
							c->red   = (c->red   * a)>>8;
							c->green = (c->green * a)>>8;
							c->blue  = (c->blue  * a)>>8;
						}
					}
				}
			}
		} break;
	}
	cacheValid = true;
	rgbPoint = 0x80008000L;
}


//
// The Edge structures
//

typedef void (*DoEdgeProc)(CRaster*, RActiveEdge*);
typedef void (*StepProc)(RActiveEdge*, S32 yline);

struct RActiveEdge {
	RActiveEdge *next, *prev;
	S32 x;
	S32 ymax;

	SPOINT d, d2;		// difference stored as 10.22
	SCOORD xl, yl;		// location stored as 16.16
	S32 stepLimit;		// countdown to zero so we know when t == 1
	
	RColor *color1, *color2;
	int dir;

	DoEdgeProc doEdgeProc;
	StepProc stepProc;
	
	void SetUp(REdge* e, S32 y, BOOL interpolate);
};

//
// Difference Equation
//
//	P = A*u^2 + B*u + C
//
//	A = anchor1 - 2*control + anchor2
//	B = 2*control - 2*anchor1
//	C = anchor1
//
//	D(P, u) = 2*u*A + B
//
//	D2(P, u) = 2*A
//

static const DoEdgeProc edgeProcTable[] = {
		DoEdgeEdgeRule,
		DoEdgeEvenOddRule,
		DoEdgeWindingRule
	};

void StepLine(RActiveEdge* edge, S32 yline)
{
 	// Step down a line
	edge->xl += edge->d.x;
	edge->x = (edge->xl + 0x8000L) >> 16;
}

void StepCurve(RActiveEdge* edge, S32 yline)
{
	// Step down a curve
	S32 ymin = yline << 16;
	while ( edge->yl < ymin && edge->stepLimit >= 0 ) {
		// Step
		edge->yl += edge->d.y >> 8;
		edge->d.y += edge->d2.y;
			
		edge->xl += edge->d.x >> 8;
		edge->d.x += edge->d2.x;
		
		edge->stepLimit--;
	}
	
	edge->x = (edge->xl + 0x8000L) >> 16;
}

void StepCurveInterpolate(RActiveEdge* edge, S32 yline)
{
	// Step down a curve
	S32 ymin = yline << 16;
	S32 prevX = edge->xl;
	S32 prevY = edge->yl;
	
	while ( edge->yl < ymin && edge->stepLimit >= 0 ) {
		// Step
		edge->yl += edge->d.y >> 8;
		edge->d.y += edge->d2.y;
			
		edge->xl += edge->d.x >> 8;
		edge->d.x += edge->d2.x;
		
		edge->stepLimit--;
	}
	
	// Calculate a precise x position
	SCOORD dy= edge->yl - prevY;
	if ( dy != 0 ) {
		// Calculate a position along the line
		SCOORD dx = edge->xl - prevX;
		SCOORD ky = ymin - edge->yl;
		edge->x = (edge->xl + FixedDiv(FixedMul(ky, dx), dy) + 0x8000L) >> 16;
	} else {
		edge->x = (edge->xl + 0x8000L) >> 16;
	}
}

void RActiveEdge::SetUp(REdge* e, S32 y, BOOL interpolate)
{
	// Set up the difference equation
	if ( e->isLine ) {
		// Do a straight line
		stepProc = StepLine;

	 	SCOORD dx = (SCOORD)e->anchor2x - (SCOORD)e->anchor1x;
	 	SCOORD dy = (SCOORD)e->anchor2y - (SCOORD)e->anchor1y;
			
		// Set up the difference equation
		d.x = (dx << 16)/dy;
		xl = (SCOORD)e->anchor1x << 16;
			
		// Adjust for missed lines
		dy = y - (SCOORD)e->anchor1y;
		if ( dy != 0 )
			xl += d.x*dy;
		
		x = (xl+0x8000L)>>16;
		
	} else {
		SPOINT a, b;
		SCOORD dt, d2t;
		
		// Set up for a curve
		stepProc = interpolate ? StepCurveInterpolate : StepCurve;
		
		// compute quadratic coeffs w/ 16 bits of fraction
		a.x = (SCOORD)e->anchor1x - ((SCOORD)e->controlx<<1) + (SCOORD)e->anchor2x;
		b.x = ((SCOORD)e->controlx - (SCOORD)e->anchor1x)<<1;
	
		a.y = (SCOORD)e->anchor1y - ((SCOORD)e->controly<<1) + (SCOORD)e->anchor2y;
		b.y = ((SCOORD)e->controly - (SCOORD)e->anchor1y)<<1;
	
		stepLimit = ((SCOORD)e->anchor2y - (SCOORD)e->anchor1y)<<1;
		dt = (1L<<24)/stepLimit;	// Guess at two steps per scanline
		d2t = _FPMul(dt, dt, 24);
		
		// Calc step values
		d.x = b.x * dt;
		d2.x = (a.x<<1) * d2t;
	
		d.y = b.y * dt;
		d2.y = (a.y<<1) * d2t;
		
		FLASHASSERT(d.y > 0 || d2.y > 0);
	
		// Setup the location
		xl = (SCOORD)e->anchor1x << 16;
		yl = (SCOORD)e->anchor1y << 16;
		
		x = e->anchor1x;
		
		// Handle starting in the middle of a curve
		if ( y > e->anchor1y ) 
		 	stepProc(this, y);
	}
	
	ymax = e->anchor2y;
	
	color1 = e->color1;
	color2 = e->color2;
	doEdgeProc = edgeProcTable[e->fillRule];
	dir = e->dir;
}


//
// Pixel Color Calculation
//

const CalcPixelProc CalcPixelProcs[3] = {
	CalcSolidPixel,		// colorSolid
	CalcBitmapPixel,	// colorBitmap
	CalcGradientPixel,	// colorGradient
};


//
// The Antialiasing run object
//

enum { maxRunStack = 12 };

struct RRunStack {
	RColor* colors[maxRunStack];
};

struct RRunCore {// these fields are always copied on a Split()
	S32 xmin, xmax;
	RRun* next;
	int nColors;
	U8 isComplex;	// there are non-solid colors in this run
	U8 isPure;		// all color stacks are the same
	U8 transparent;	// there is a transparent color in this run
	U8 stackN[4];
};

struct RRun : public RRunCore {
	RRunStack stacks[4];
	
	void Init(S32 l, S32 r);
	void AddColor(RColor*);
	void CalcColor(S32 x, S32 y, RGBI* rgb);
	RRun* Split(CRaster*, S32 x);
};

inline void RRun::Init(S32 l, S32 r)
{
	next = 0;
	xmin = l;
	xmax = r;
	nColors = 0;
	isPure = true;
	isComplex = false;
	transparent = false;
}

inline void RRun::AddColor(RColor* rc)
{
	//FLASHASSERT(nColors < 4);
	FLASHASSERT(rc);

	// Add a new color entry
	RColor** stack = stacks[nColors].colors;
	int n = 0;
	while ( true ) {
		// Skip this color if it is clipped out
		if ( rc->clippedBy ) {
			if ( rc->colorType == colorClip ) 
				goto SkipColor; // this is a clipping color
			RColor* cc = rc->clippedBy;
			do {
				if ( !cc->visible ) 
					goto SkipColor;
				cc = cc->clippedBy;
				FLASHASSERT(cc);
			} while ( cc != (void*)1 );
		}

		stack[n] = rc;
		if ( stacks[0].colors[n] != rc ) 
			isPure = false;
		isComplex |= rc->colorType;	// NOTE: assumes colorSolid == 0, and the isComplex is only tested for true or false
		//if ( rc->colorType != colorSolid )
		//	isComplex = true;
		n++;
		if ( !rc->transparent ) 
			break;
		transparent = true;

	SkipColor:
		rc = rc->nextActive;
		if ( !rc ) break;
		if ( n == maxRunStack ) // if we overflow the stack always keep the bottom color
			n--;
	}
	if ( n > 0 ) {
		stackN[nColors] = n;
		if ( n != stackN[0] )
			isPure = false;
		nColors++;
	}
}

void RRun::CalcColor(S32 x, S32 y, RGBI* rgb)
// Update the color of this run based on the given device pixel location
{
	U32 rgbPoint = (y<<16) | (x&0xFFFF);
	((RGBIL*)rgb)->c0 = ((RGBIL*)rgb)->c1 = 0;
	if ( transparent ) {
		// Handle the most general case of transparent colors
		for ( int i = nColors; i--; ) {
			int n = stackN[i];
			RColor** c = stacks[i].colors+n-1;
			RGBI pix;
			BOOL bottom = true;
			while ( n-- > 0 ) {
				RColor* rc = *c;
				if ( rc->rgbPoint != rgbPoint ) {
					if ( rc->colorType != colorSolid )
						(CalcPixelProcs[rc->colorType])(rc, x, &rc->rgb);
					rc->rgbPoint = rgbPoint;
				}
				if ( bottom ) {
					pix = rc->rgb;
					bottom = false;
				} else {
					CompositeRGBPixel(&rc->rgb, &pix);
				}
				c--;
			}

			((RGBIL*)rgb)->c0 += ((RGBIL*)&pix)->c0;
			((RGBIL*)rgb)->c1 += ((RGBIL*)&pix)->c1;
			if ( isPure ) {
				((RGBIL*)rgb)->c0 *= nColors;
				((RGBIL*)rgb)->c1 *= nColors;
				break;
			}
		}
	} else {
		// The colors are all opaque
		for ( int i = nColors; i--; ) {
			RColor* rc = stacks[i].colors[0];
			if ( rc->rgbPoint != rgbPoint ) {
				if ( rc->colorType != colorSolid ) {
					(CalcPixelProcs[rc->colorType])(rc, x, &rc->rgb);
					rc->rgb.alpha = 255;
				}
				rc->rgbPoint = rgbPoint;
			}
			((RGBIL*)rgb)->c0 += ((RGBIL*)&rc->rgb)->c0;
			((RGBIL*)rgb)->c1 += ((RGBIL*)&rc->rgb)->c1;
			if ( isPure ) {
				((RGBIL*)rgb)->c0 *= nColors;
				((RGBIL*)rgb)->c1 *= nColors;
				break;
			}
		}
	}
}

inline RRun* RRun::Split(CRaster* render, S32 x)
{
	// Split the run
	RRun *n = render->CreateRun();
	if ( !n ) return 0;
	if ( transparent ) {
		// Copy the entire stacks
		*n = *this;
	} else {
		// Copy just the tops of the stacks if there are not transparent colors
		*(RRunCore*)n = *(RRunCore*)this;
		n->stacks[0].colors[0] = stacks[0].colors[0];
		n->stacks[1].colors[0] = stacks[1].colors[0];
		n->stacks[2].colors[0] = stacks[2].colors[0];
		n->stacks[3].colors[0] = stacks[3].colors[0];
	}
	xmax = n->xmin = x;
	next = n;
	return n;
}


//
// The pixel object used to render partially covered pixels
//

struct RPixel {
	CRaster* raster;
	RGBI rgb;
	S32 x, y;
	
	void Init(CRaster* r, S32 py);
	void Add(RGBI* rgb, int yw, S32 px, int xw);
	void Paint();
};

inline void RPixel::Init(CRaster* r, S32 py)
{
	((RGBIL*)&rgb)->c0 = 0;
	((RGBIL*)&rgb)->c1 = 0;
	x = -32000;
	y = py;
	raster = r;
}

inline void RPixel::Paint()
{
	if ( rgb.alpha ) {
		// Divide by 16
		((RGBIL*)&rgb)->c0 = (((RGBIL*)&rgb)->c0 >> 4) & 0x00FF00FFL;
		((RGBIL*)&rgb)->c1 = (((RGBIL*)&rgb)->c1 >> 4) & 0x00FF00FFL;
		raster->DrawRGBAPixel(x, &rgb);

		#ifdef ENABLE_MASK
		if ( raster->maskBits )
			raster->SetMask(x, x+1);
		#endif
	}
	// Reset
	((RGBIL*)&rgb)->c0 = 0;
	((RGBIL*)&rgb)->c1 = 0;
}

inline void RPixel::Add(RGBI* rc, int yw, S32 px, int xw)
// xw is the x coverage it is multiplied by the y coverage in r to get the area coverage
{
	if ( x != px ) {
		Paint();
	 	x = px;
	}

	((RGBIL*)&rgb)->c0 += xw*((RGBIL*)rc)->c0;
	((RGBIL*)&rgb)->c1 += xw*((RGBIL*)rc)->c1;
}


//
// The Renderer Object
//
	
CRaster::CRaster() :
				activeEdgeAlloc(sizeof(RActiveEdge), 64, true, 0x33333333),
				runAlloc(sizeof(RRun), 128, true, 0x55555555),
				rampAlloc(sizeof(RGB8)*257, 16, true, 0xEEEEEEEE),
				mapAlloc(sizeof(RColorMap), 8, true)
{
	if ( !renderTablesBuilt )
		BuildRenderTables();

	antialias = false;
	getBackground = false;
	smoothBitmaps = false;
	solidDither = orderedDither;
	bitmapDither = orderedDither;

	patAlign.x = patAlign.y = 0;

	//activeColors = 0;
	topColor = 0;

	firstActive = 0;
	yindex = 0;
	yindexSize = 0;
	
	firstRun = 0;

	ditherData = 0;

	bits = 0;
	cinfo = 0;
	baseAddr = 0;
	
	topColorXleft = 0;

#ifdef ENABLE_MASK
	maskBits = 0;
	getBackProc = 0;
#endif
}

CRaster::~CRaster() 
{
	delete [] yindex;
	delete [] ditherData;
}

void CRaster::FreeEmpties()
{
	activeEdgeAlloc.FreeEmpties();
	runAlloc.FreeEmpties();
	rampAlloc.FreeEmpties();
}

//
// Painting code
//

BOOL CRaster::BitsValid()
{
	return bits && bits->BitsValid();
}

void CRaster::Attach(CBitBuffer* b, SRECT* c, BOOL doAntialias)
{
	if ( b ) {
		bits = b;
		cinfo = bits->getSColorInfo();	//m_cinfo;
		inverted = bits->inverted();	//m_inverted;
		bitHeight = bits->height();		//m_bufHeight;
		baseAddr = bits->baseAddess();	//b(char*)bits->m_baseAddr;
		rowBytes = bits->scanLine();	//m_rowBytes;

		xorg = bits->xorg();
		pixelFormat = bits->pixelFormat();

		doGetBackground = GetBackgroundProcs[pixelFormat];

		{
			drawRGBSlab = DrawRGBSlabProcs[pixelFormat];
			ditherRGBSlab = DitherRGBSlabProcs[bitmapDither];
		}

		//if ( alphaChannel && pixelFormat == pix32 ) 
		//	pixelFormat = pix32A;
	} else {
	 	bits = 0;
		cinfo = 0;
		baseAddr = 0;
	}

	antialias = doAntialias;

// 	RectSet(0, 0, bits->m_bufWidth, bits->m_bufHeight, &bitClip);
	RectSet(0, 0, bits->width(), bits->height(), &bitClip);
	if ( c ) 
		RectIntersect(c, &bitClip, &bitClip);

	edgeClip = bitClip;
	if ( antialias ) {
		edgeClip.xmin *= 4;
		edgeClip.ymin *= 4;
		edgeClip.xmax *= 4;
		edgeClip.ymax *= 4;
	}

	ylines = edgeClip.ymax - edgeClip.ymin + 1;
	if ( ylines <= 0 ) {
		ylines = 0;	    
		FLASHASSERT(false);
	}
}

void CRaster::UseAlphaChannel() 
{ 
	if ( pixelFormat == pix32 ) 
		pixelFormat = pix32A; 
}

void CRaster::FillBackground(SRGB backgroundColor)
{
	BeginPaint();

	// Add the background edges
	int savedPixelFormat = pixelFormat;
	if ( pixelFormat == pix32A ) 
		pixelFormat = pix32;

	// Set up the color
	RColor bkColor;
	bkColor.SetUp(this);
	bkColor.order = 0;
	bkColor.rgb.alpha = backgroundColor.rgb.transparency;
	bkColor.rgb.red   = backgroundColor.rgb.red;
	bkColor.rgb.green = backgroundColor.rgb.green;
	bkColor.rgb.blue  = backgroundColor.rgb.blue;
	bkColor.BuildCache();

	// Set up the edges
	REdge bkEdges[2];
	bkEdges[0].nextObj = &bkEdges[1];
	bkEdges[1].nextObj = 0;
	
	SPOINT pt1, pt2;
	CURVE c;
	pt1.y = edgeClip.ymin;
	pt2.y = edgeClip.ymax;

	pt1.x = pt2.x = edgeClip.xmin;
	CurveSetLine(&pt1, &pt2, &c);
	bkEdges[0].Set(&c);

	pt1.x = pt2.x = edgeClip.xmax;
	CurveSetLine(&pt1, &pt2, &c);
	bkEdges[1].Set(&c);

	bkEdges[0].dir 		= bkEdges[1].dir      = 0;
	bkEdges[0].fillRule = bkEdges[1].fillRule = fillEvenOddRule;
	bkEdges[0].color1 	= bkEdges[1].color1   = &bkColor;
	bkEdges[0].color2 	= bkEdges[1].color2   = 0;

	AddEdges(bkEdges);

	PaintBits();

	bkColor.FreeCache();

	pixelFormat = savedPixelFormat;

	getBackground = backgroundColor.all != SRGBWhite;
}

typedef REdge* PREdge;

void CRaster::BeginPaint()
{
	FLASHASSERT(baseAddr);

	// Setup the y index
	S32 newSize = ylines;
	if ( newSize > yindexSize ) {
		delete [] yindex;//free(yindex);
		yindex = new PREdge[newSize];
		if ( !yindex ) {
			yindexSize = 0;
			return;
		}
		yindexSize = newSize;
	}

	memset(yindex, 0, (int)(newSize * (sizeof(PREdge))));

	firstActive = 0;
	topColor = 0;

	needFlush = false;
	layerDepth = 0;
}

void CRaster::AddEdges(REdge* edge)
{
	if ( !edge ) return;
	while ( edge ) {
		if ( edge->anchor1y <= edgeClip.ymax && edge->anchor2y > edgeClip.ymin ) {
			// Insert this edge at the proper scanline
			S32 i = edge->anchor1y - edgeClip.ymin;
			if ( i < 0 ) i = 0;
			edge->nextActive = yindex[i];
			yindex[i] = edge;
		}
		edge = edge->nextObj;
	}
	needFlush = true;
}

void CRaster::AddEdges(REdge* edge, RColor* colors, RColor* clipColor)
// Add the edges and adjust the color depths for the current layering
{
	AddEdges(edge);
	while ( colors ) {
		colors->order = (colors->order & 0xFFFF) | layerDepth;
		colors->clippedBy = clipColor;
		colors = colors->nextColor;
	}
	layerDepth += 0x10000;
}

void CRaster::AddActive()
{// Add new active edge entries
	REdge *e, *lastE;
	RActiveEdge *a, *pos;
	REdge** index;
	S32 x;
	
	pos = firstActive;
	index = yindex + y-edgeClip.ymin;
	for ( e = *index; e; e = e->nextActive ) {
		a = CreateActiveEdge();
		if ( !a ) return;
		
		a->SetUp(e, y, !antialias);
		
		// Insert into the active edge list
		if ( pos ) {
			// Find the proper position
			x = a->x;
			if ( pos->x < x ) {
				while ( pos->x < x && pos->next )
					pos = pos->next;
			} else if ( pos->x > x ) {
				while ( pos->x > x && pos->prev )
					pos = pos->prev;
			}
			
			DListInsertAfter(pos, a, RActiveEdge);
			
		} else {
			// Insert into an empty list
			DListAdd(firstActive, a, RActiveEdge);
		}
		pos = a;
		
		lastE = e;	// remember the tail of the list
	}
}

void CRaster::SortActive()
{// Sort the active edges
	RActiveEdge *a, *b, *stop;
	BOOL swapped;
	
	stop = 0;
	do {
		swapped = false;
		a = firstActive;
		
		while ( a != 0 && (b = a->next) != 0 ) {
			if ( a->x > b->x ) {
				// Swap
			 	DListSwap(firstActive, a, b, RActiveEdge);
			 	if ( !swapped && b->prev && b->prev->x > b->x )
			 		swapped = true;
			} else {
				// Just keep going
				a = b;
			}
		}
	} while ( swapped );
}


//
// Get Background Procs
//

void GetBackground1(CRaster* r, S32 xmin, S32 xmax, RGBI* pix)
{
	xmin += r->xorg;
	xmax += r->xorg;
	RGB8* ctab = r->cinfo->ctab.colors;
	FLASHASSERT( r->cinfo->refCount );
	U8 * src = (U8 *)r->rowAddr + xmin/8;
	int shift = 7 - ((int)xmin & 0x07);
	for ( int i = (int)(xmax-xmin); i--; pix++ ) {
		ExpandColor(ctab[(*src >> shift)&1], pix);
		if ( --shift < 0 ) {
			shift = 7;
			src++;
		}
	}
}

void GetBackground2(CRaster* r, S32 xmin, S32 xmax, RGBI* pix)
{
	xmin += r->xorg;
	xmax += r->xorg;
	RGB8* ctab = r->cinfo->ctab.colors;
	FLASHASSERT( r->cinfo->refCount );
	U8 * src = (U8 *)r->rowAddr + (xmin>>2);
	int shift = (3 - ((int)xmin & 0x03))<<1;
	for ( int i = (int)(xmax-xmin); i--; pix++ ) {
		ExpandColor(ctab[(*src>>shift)&0x3], pix);
		shift-=2;
		if ( shift < 0 ) {
			shift = 6;
			src++;
		}
	}
}

void GetBackground4(CRaster* r, S32 xmin, S32 xmax, RGBI* pix)
{
	xmin += r->xorg;
	xmax += r->xorg;
	RGB8* ctab = r->cinfo->ctab.colors;
	FLASHASSERT( r->cinfo->refCount );
	U8 * src = (U8 *)r->rowAddr + (xmin>>1);
	int shift = (1 - ((int)xmin & 0x01))<<2;
	for ( int i = (int)(xmax-xmin); i--; pix++ ) {
		ExpandColor(ctab[(*src>>shift)&0xF], pix);
		shift -= 4;
		if ( shift < 0 ) {
			shift = 4;
			src++;
		}
	}
}

void GetBackground8(CRaster* r, S32 xmin, S32 xmax, RGBI* pix)
{
	RGB8* ctab = r->cinfo->ctab.colors;
	FLASHASSERT( r->cinfo->refCount );
	U8 * src = (U8 *)r->rowAddr + (xmin + r->xorg);
	for ( int i = (int)(xmax-xmin); i--; pix++, src++ )
		ExpandColor(ctab[*src], pix);
}

void GetBackground16(CRaster* r, S32 xmin, S32 xmax, RGBI* pix)
{
	U16 * src = (U16 *)r->rowAddr + (xmin + r->xorg);
	for ( int i = xmax-xmin; i--; pix++, src++ )
		UnpackPix16(*src, pix);
}

void GetBackground16A(CRaster* r, S32 xmin, S32 xmax, RGBI* pix)
{
	U16 * src = (U16 *)r->rowAddr + (xmin + r->xorg);
	for ( int i = xmax-xmin; i--; pix++, src++ )
		UnpackPix16A(*src, pix);
}

void GetBackground24(CRaster* r, S32 xmin, S32 xmax, RGBI* pix)
{
	U8 * src = (U8 *)r->rowAddr + 3*(xmin + r->xorg);
	for ( S32 i = xmax-xmin; i--; pix++, src+=3 ) {
		pix->red   = src[2];
		pix->green = src[1];
		pix->blue  = src[0];
		pix->alpha = 255;
	}
}

void GetBackground32(CRaster* r, S32 xmin, S32 xmax, RGBI* pix)
{
	U32 * src = (U32 *)r->rowAddr + (xmin + r->xorg);
	for ( S32 i = xmax-xmin; i--; pix++, src++ ) {
		UnpackPix32(*src, pix);
	}
}

void GetBackgroundWhite(CRaster* r, S32 xmin, S32 xmax, RGBI* pix)
{
	// Set a white background
	RGBI white;
	white.alpha = 0;
	white.red = white.green = white.blue = 255;
	int n = xmax - xmin;
	while ( n-- > 0 ) 
		*pix++ = white;
}


//
// Drawing with an alpha channel
//

void CRaster::CompositeSlab(S32 xleft, S32 xright, RColor** stack, int n)
{
	// Apply the colors
	while ( xleft < xright ) {
		RGBI pixBuf[RGBSlabChunkSize];
		S32 limit = Min(xright, xleft+RGBSlabChunkSize);

		RColor** c = stack+n-1;
		int i = n;

		if ( (*c)->transparent ) { // if the bottom visible color is transparent, get the bits
			doGetBackground(this, xleft, limit, pixBuf);
		} else {
			if ( (*c)->pattern != 0 ) {
				// Note that patterned fills should never be transparent
				((*c)->compositeSlabProc)(*c, xleft, limit, pixBuf);
				ApplyRGBPattern((*c)->pattern, bitY, xleft, limit, pixBuf);
				c--;i--;
			} else if ( (*c)->colorType == colorSolid && (*c)->pattern == 0 ) {
				// For the lower solid color layers, step through the color stack and 
				//	composite a single pixel to find the pixel value for the entire run
				RGBI rgb = (*c)->rgb;
				c--;i--;
				while ( i > 0 && (*c)->colorType == colorSolid && (*c)->pattern == 0 ) {
					CompositeRGBPixel(&(*c)->rgb, &rgb);
					c--;i--;
				}

				int n = limit-xleft;
				RGBI* p = pixBuf;
				while ( n-- )
					*p++ = rgb;
			}
		}

		while ( i > 0 ) {
			// Composite the slab to the buffer
			((*c)->compositeSlabProc)(*c, xleft, limit, pixBuf);
			c--;i--;
		}
		drawRGBSlab(this, xleft, limit, pixBuf);
		xleft = limit;
	}
}


//
// Paint a slab of color
//

void CRaster::PaintSlab(S32 xright)
// Apply a slab to the current run using the topColor
{
	S32 xleft = topColorXleft;
	topColorXleft = xright;

	if ( topColor ) {
		if ( antialias ) {
			register RRun* r = curRun;
			
			if ( !r || r->xmin >= xright ) return; // we are outside the visible region
		 				
			// Skip over blank sections
			while ( r->xmax < xleft ) {
				if ( !(r = r->next) ) {
					// We went past the right edge of the clip rect
		 			curRun = 0;
					return;
				}
			}
		 				
			// Split the run if needed
			if ( r->xmin < xleft )
				r = r->Split(this, xleft);
			 					
			// Paint the covered run sections
			while ( r && r->xmin < xright ) {
				if ( r->xmax > xright ) {
					//r->Split(this, xright);
					curRun = r->Split(this, xright);
					r->AddColor(topColor);
					return;
				}
		 						
				r->AddColor(topColor);
				r = r->next;
			}
		 				
			curRun = r;

		} else {
			// Clip to the bitmap
			if ( xleft < bitClip.xmin )
				xleft = bitClip.xmin;
			if ( xright > bitClip.xmax )
				xright = bitClip.xmax;

			if ( xright > xleft ) {
				// Build a bottom up list of visible colors
				RColor* stack[255];
				int n = 0;

				{// Build a stack of colors
					RColor* c = topColor;
					while ( c ) {
						// Skip this color if it is clipped out
						if ( c->clippedBy ) {
							if ( c->colorType == colorClip ) 
								goto SkipColor; // this is a clipping color
							RColor* cc = c->clippedBy;
							do {
								if ( !cc->visible ) 
									goto SkipColor;
								cc = cc->clippedBy;
								FLASHASSERT(cc);
							} while ( cc != (void*)1 );
						}

						if ( n < 255 ) // we always want to keep the bottom color since it is usually not tranparent
							n++;

						stack[n-1] = c;
						if ( !c->transparent ) break;
					SkipColor:
						c = c->nextActive;
					}
				}

				if ( n > 0 ) {
					if ( !stack[0]->transparent )
						(stack[0]->drawSlabProc)(stack[0], xleft, xright);	// The top color is opaque, just draw it
					else
						CompositeSlab(xleft, xright, stack, n);		// composite the entire stack
				}
			}

			#ifdef ENABLE_MASK
			if ( maskBits )
				SetMask(xleft, xright);
			#endif
		}
	}
}


//
// Manage the color transitions
//

inline void CRaster::ShowColor(RColor* c, S32 x)
{
	// Find where to insert the color
	BOOL paint = true;
	RColor** link = &topColor;
	while ( true ) {
		RColor* tc = *link;
		if ( !tc ) break;
		if ( c->order > tc->order ) {
			break;
		} else {
			if ( !tc->transparent && !tc->clippedBy )
				paint = false;
			link = &tc->nextActive;
		}
	}

	// Draw a slab if the new color is visible (not under an opaque color)
	if ( paint )
		PaintSlab(x);

	// Insert the color
	c->nextActive = *link;
	*link = c;
}

inline void CRaster::HideColor(RColor* c, S32 x)
{
	// Find the color
	BOOL paint = true;
	RColor** link = &topColor;
	while ( true ) {
		RColor* tc = *link;
// #ifndef SPRITE_XTRA
		FLASHASSERT(tc);	// tc should never be null since the color c must be in the list
		if ( tc == c ) {
// #else
// 		if ( !tc || tc == c ) {
// #endif
			break;
		} else {
			if ( !tc->transparent && !tc->clippedBy )
				paint = false;
			link = &tc->nextActive;
		}
	}

	// Draw a slab if the old color was visible needed
	if ( paint )
		PaintSlab(x);

	// Remove the color
	*link = c->nextActive;
}

//
// Support the Edge rules
//

void DoEdgeEdgeRule(CRaster* r, RActiveEdge* a)
{
	// Handle color1
	RColor* c = a->color1;
	if ( c->visible ) {
		r->HideColor(c, a->x);
	 	c->visible = false;
	} else {
		r->ShowColor(c, a->x);
	 	c->visible = true;
	}
	
	// Handle color2
	c = a->color2;
	if ( c->visible ) {
		r->HideColor(c, a->x);
	 	c->visible = false;
	} else {
		r->ShowColor(c, a->x);
	 	c->visible = true;
	}
}

void DoEdgeEvenOddRule(CRaster* r, RActiveEdge* a)
{
	// Handle color1
	RColor* c = a->color1;
	if ( c->visible ) {
		r->HideColor(c, a->x);
	 	c->visible = false;
	} else {
		r->ShowColor(c, a->x);
	 	c->visible = true;
	}
}

void DoEdgeWindingRule(CRaster* r, RActiveEdge* a)
{
	// Handle color1
	RColor* c = a->color1;
	if ( c->visible == 0 ) {
		// The color is becoming active
		r->ShowColor(c, a->x);
		c->visible += a->dir;
	} else {
		int vis = c->visible + a->dir;
		if ( vis == 0 ) {
			// The color disappeared
			r->HideColor(c, a->x);
		}
		c->visible = vis;
	}
}

void CRaster::PaintActive()
// Draw the slabs
{
	S32 ynext = y+1;

	// Create a run object
    if ( antialias ) {
		if ( !firstRun ) {
		 	// Create an empty run
		 	firstRun = CreateRun();
		 	if ( !firstRun ) return;
		 	firstRun->Init(edgeClip.xmin, edgeClip.xmax);
		}
		curRun = firstRun;
    } else {
		SetYCoord(y);
	}

	{// This should never happen but we can minimize the effect
		//FLASHASSERT(!topColor);
		for ( RColor* c = topColor; c; c = c->nextActive )
		{
			c->visible = 0;
// #ifdef SPRITE_XTRA
// 			if (c == c->nextActive)
// 				break;
// #endif
		}

		topColor = 0;
	}
    
	register RActiveEdge* a = firstActive;
    while ( a ) {
		// Track the active colors
		(a->doEdgeProc)(this, a);
    	
		// Remove out of date active edge entries
		// And calculate new x coordinates for the ones we keep
		if ( a->ymax > ynext ) {
			// Keep this edge
		 	a->stepProc(a, ynext);
		 	a = a->next;
		} else {
			// Remove this edge
    		RActiveEdge* next = a->next;
			DListRemove(firstActive, a, RActiveEdge);
			FreeActiveEdge(a);
			a = next;
		}
    }
}


//
// Antialiased Painting
//

// Merging runs does not seem to be an effective optimization
//#define MERGE_RUNS

#ifdef MERGE_RUNS
BOOL RunEqual(RRun* a, RRun* b)
{
	int i = a->nColors;
	if ( i != b->nColors ) return false;
	while ( i-- ) {
		int j = a->stackN[i];
		if ( j != b->stackN[i] ) return false;
		while ( j-- ) {
			if ( a->stacks[i].colors[j] != b->stacks[i].colors[j] )
				return false;
		}
	}
	return true;
}

int nRuns = 0;
int nPureRuns = 0;
int nPureMerged = 0;
int nMerged = 0;
#endif

void CRaster::PaintAARuns()
// Paint the runs
{
	S32 xl, xr;
	int xlf, xrf;
	S32 yd;
	RPixel pixel;
	RRun *r, *next;
    	
	yd = y>>2;
	SetYCoord(yd);
    	
	pixel.Init(this, yd);
		
	r = firstRun;
	firstRun = 0;
	while ( r ) {
 		if ( r->nColors ) {
			#ifdef MERGE_RUNS
    		// Check to merge adjacent identical runs
    		if ( r->isPure && r->nColors == 4 ) {
				nPureRuns++;
    			while ( (next = r->next) != 0 ) {
					if ( r->stacks[0].colors[0] != next->stacks[0].colors[0] ) goto MergeDone;	// this should cause failure most of the time
					if ( !next->isPure ) goto MergeDone;
					if ( next->nColors != 4 ) goto MergeDone;
					int i = r->stackN[0];
					if ( i != next->stackN[0] ) goto MergeDone;
					RColor** ra = r->stacks[0].colors+1;
					RColor** rb = next->stacks[0].colors+1;
					i--;
					while ( i-- > 0) {
						if ( *ra++ != *rb++ ) 
							goto MergeDone;
					}
    				r->xmax = next->xmax;
    				r->next = next->next;
    				FreeRun(next);
					nPureMerged++;
    			}
			  MergeDone:
				;
    		}
    		{
    			while ( (next = r->next) && RunEqual(r, next) ) {
    				r->xmax = next->xmax;
    				r->next = next->next;
    				FreeRun(next);
					nMerged++;
    			}
    		}
			nRuns++;
			#endif
	    		
    		// Draw a run
	 		xl = r->xmin>>2;
	 		xlf = (int)r->xmin & 3;
	 		xr = r->xmax>>2;
	 		xrf = (int)r->xmax & 3;
		 		
			RGBI runRGB;
			BOOL runRGBValid = false;

	 		if ( xl == xr ) {
	 			// The run is completely within the current pixel
				if ( !runRGBValid )	{
					r->CalcColor(xl, yd, &runRGB);
					runRGBValid = true;
				}
	 			pixel.Add(&runRGB, r->nColors, xl, xrf - xlf);
		 		
	 		} else {
	 			// The run covers more than a single pixel
		 		if ( xlf ) {
		 			// Handle the leading fraction
					if ( !runRGBValid )	{
						r->CalcColor(xl, yd, &runRGB);
						runRGBValid = true;
					}
		 			pixel.Add(&runRGB, r->nColors, xl, 4-xlf);
			 		xl++;	// round up
		 		}
			 		
		 		if ( xl < xr ) {
		 			// Handle the solid run
		 			if ( r->isPure && r->nColors == 4 ) {
		 				// This is a fully covered run
						if ( r->transparent ) {
							CompositeSlab(xl, xr, r->stacks[0].colors, r->stackN[0]);
						} else {
							(r->stacks[0].colors[0]->drawSlabProc)(r->stacks[0].colors[0], xl, xr);
						}

		 			} else {
						// Handle a non-pure or partially covered run
						int x = xl;
						do {
							RGBI pixBuf[RGBSlabChunkSize];
							int limit = Min(xr, x+RGBSlabChunkSize);

							if ( r->isComplex ) {
								RGBI* pix = pixBuf;
								for ( S32 j = x; j < limit; j++, pix++ ) {
									r->CalcColor(j, yd, pix);
									((RGBIL*)pix)->c0 = (((RGBIL*)pix)->c0 >> 2) & 0x00FF00FFL;
									((RGBIL*)pix)->c1 = (((RGBIL*)pix)->c1 >> 2) & 0x00FF00FFL;
								}

							} else {
								if ( !runRGBValid )	{
									r->CalcColor(xl, yd, &runRGB);
									runRGBValid = true;
								}

								((RGBIL*)pixBuf)->c0 = (((RGBIL*)&runRGB)->c0 >> 2) & 0x00FF00FFL;
								((RGBIL*)pixBuf)->c1 = (((RGBIL*)&runRGB)->c1 >> 2) & 0x00FF00FFL;

								RGBI* pix = pixBuf+1;
								for ( S32 j = x+1; j < limit; j++, pix++ )
									*pix = pixBuf[0];
							}

							if ( r->nColors == 4 && !r->transparent ) 
								drawRGBSlab(this, x, limit, pixBuf);// there is no transparency - fast case
							else
								DrawRGBASlab(x, limit, pixBuf);

							x = limit;
						} while ( x < xr );
		 			}

					#ifdef ENABLE_MASK
					if ( maskBits )
						SetMask(xl, xr);
					#endif
		 		}
		 		if ( xrf ) {
		 			// Handle the trailing fraction
					if ( r->isComplex || !runRGBValid )
						r->CalcColor(xr, yd, &runRGB);
		 			pixel.Add(&runRGB, r->nColors, xr, xrf);
		 		}
		 	}
		}
		
		// Advance
		next = r->next;
		FreeRun(r);
		r = next;
	}
	pixel.Paint();	// Flush the pixel...
}


//
// Main Paint routine
//

void CRaster::PaintBits()
{
	bits->Flush();

	if ( getBackground ) {
		doGetBackground = GetBackgroundProcs[pixelFormat];
	} else {
		doGetBackground = GetBackgroundWhite;
	}
	
	// Paint the scanlines
	for ( y = edgeClip.ymin; y < edgeClip.ymax; y++ ) {
		AddActive();
		SortActive();
		PaintActive();
		if ( antialias && (y & 3) == 3 )
	    	PaintAARuns();

		if ( (y & 0x1f) == 0 ) {
			Spin();
// 			#if defined(USE_MMX) || defined(USE_KATMAI)
// 			if ( gUseMMX || gUseKatmai ) {
// 				_asm EMMS
// 			}
// 			#endif
			PollSound();
		}
	}

	{// Free any excess active edges
		for ( RActiveEdge* a = firstActive; a; ) {
			RActiveEdge* next = a->next;
			FreeActiveEdge(a);
			a = next;
		}
		firstActive = 0;
	}
	delete [] ditherData;
	ditherData = 0;

// 	#if defined(USE_MMX) || defined(USE_KATMAI)
// 	if ( gUseMMX || gUseKatmai ) {
// 		_asm EMMS
// 	}
// 	#endif
}

void CRaster::Flush()
{
	if ( needFlush ) {
		PaintBits();
		BeginPaint();
		getBackground = true;
	}
}


//
// Methods for setting the actual bits
//

void CRaster::SetYCoord(S32 yCoord)
{
//	FLASHASSERT(y >= bitClip.ymin && y < bitClip.ymax);

	bitY = yCoord;

	if ( inverted ) {	
		// The rows are inverted for a Windows DIB
		rowAddr = baseAddr + (bitHeight-(bitY+1))*rowBytes;
	} else {
		// We are using a standard row order
		rowAddr = baseAddr + bitY*rowBytes;
	}

	#ifdef ENABLE_MASK
	if ( maskBits ) {
		if ( maskBits->m_inverted ) {	
			// The rows are inverted for a Windows DIB
			maskRowAddr = (char  *)maskBits->m_baseAddr + (bitHeight-(bitY+1))*maskBits->m_rowBytes;
		} else {
			// We are using a standard row order
			maskRowAddr = (char  *)maskBits->m_baseAddr + bitY*maskBits->m_rowBytes;
		}
	}
	#endif

	if ( ditherData ) {
		Swap(curDitherErr, nextDitherErr, void*);
		memset(nextDitherErr, 0, ditherErrSize);
	}
}


//
// Dither Methods
//

void DitherRGBSlabNone(CRaster* r, S32 xmin, S32 xmax, RGBI* pix, U8 * dst)
{
	FLASHASSERT( r->cinfo->refCount );
	U8* itab = r->cinfo->itab;
	for ( S32 i = xmax-xmin; i--; pix++, dst++ ) {
		*dst = itab[CalcITabIndexRGB(pix)];
	}
}

void DitherRGBSlabOrdered(CRaster* r, S32 xmin, S32 xmax, RGBI* pix, U8 * dst)
{
	FLASHASSERT( r->cinfo->refCount );
	U8* itab = r->cinfo->itab;
	const U8* noise = ditherNoise8+(r->bitY&0x7)*8;
	for ( S32 x = xmin; x < xmax; x++, pix++, dst++ ) {
		int d = noise[x&0x7];
		*dst = itab[ditherTabR[(pix->red  +d)>>4] | 
					ditherTabG[(pix->green+d)>>4] |
					ditherTabB[(pix->blue +d)>>4] ];
	}
}

struct RGBErr {	// an RGB struct that uses large integers for better code generation
	int red;
	int green;
	int blue;
};

void DitherRGBSlabErrorDiffusion(CRaster* r, S32 xmin, S32 xmax, RGBI* pix, U8 * dst)
{
	// Set up the dither context
	if ( !r->ditherData ) {
		// Note that the non MMX code uses 4*3 bytes per pixel error while the MMX code uses 4*2 bytes per error
// 		r->ditherErrSize = sizeof(RGBErr)*(int)(r->bits->m_bufWidth+Abs(r->xorg)+2);
		r->ditherErrSize = sizeof(RGBErr)*(int)(r->bits->width() + Abs(r->xorg)+2);
		r->ditherSize = 2*r->ditherErrSize;
		r->ditherData = new U8[r->ditherSize];
		if ( !r->ditherData ) {
			FLASHASSERT(false);
			return;
		}
		memset(r->ditherData, 0, r->ditherSize);
		r->curDitherErr = r->ditherData;
		r->nextDitherErr = r->ditherData+r->ditherErrSize;
	}

	// Error difusion dither
	U8* itab = r->cinfo->itab;
	FLASHASSERT( r->cinfo->refCount );
	const U8* noiseTab = ditherNoise+(8*(r->bitY&7));
	RGB8* ctab = r->cinfo->ctab.colors;
	RGB8 c;
	RGBErr* curErr = (RGBErr*)r->curDitherErr + xmin+1;	// we bias the error arrays by 1 pixel so the -1 offset does not run off the end
	RGBErr* nextErr = (RGBErr*)r->nextDitherErr + xmin+1;

	for ( S32 x = xmin; x < xmax; x++ ) {
		// Calc the desired color
		int r, g, b;
		r = pix->red  + curErr->red;
		g = pix->green+ curErr->green;
		b = pix->blue + curErr->blue;
	
		{// Set the pixel
			// Check to limit the color and add a bit of noise
			int noise = noiseTab[x&7];
 
			int ri = (r+noise)>>4;
			if ( ri & ~0xf ) ri = ri < 0 ? 0 : 0xf;
			int gi = (g+noise)>>4;
			if ( gi & ~0xf ) gi = gi < 0 ? 0 : 0xf;
			int bi = (b+noise)>>4;
			if ( bi & ~0xf ) bi = bi < 0 ? 0 : 0xf;

			U8 dp = itab[(ri<<8) | (gi<<4) | bi];

			// Set the destination pixel
			*dst = dp;
			c = ctab[dp];	// get the actual color
		}
	
		{// Calc the errors
			// RED
			int err = r - c.red;
			curErr[1].red += err>>1;	// 1/2 the error to the next pixel on this line
			nextErr[-1].red += (err >>= 2);
			nextErr[0].red += err;

			// GREEN
			err = g - c.green;
			curErr[1].green += err>>1;
			nextErr[-1].green += (err >>= 2);
			nextErr[0].green += err;

			// BLUE
			err = b - c.blue;
			curErr[1].blue += err>>1;
			nextErr[-1].blue += (err >>= 2);
			nextErr[0].blue += err;
		}

		// Advance
		pix++;
		dst++;
		curErr++;
		nextErr++;
	}
}

// #ifdef USE_MMX
// #pragma warning( disable : 4799 )  // Disable "no EMMS instruction" warning messages
// void DitherRGBSlabErrorDiffusion_MMX(CRaster* r, S32 xmin, S32 xmax, RGBI* pix, U8 * dst)
// {
// 	// Set up the dither context
// 	if ( !r->ditherData ) {
// 		// Note that the non MMX code uses 4*3 bytes per pixel error while the MMX code uses 4*2 bytes per error
// 		r->ditherErrSize = sizeof(RGBI)*(int)(r->bits->m_bufWidth+Abs(r->xorg)+2);
// 		r->ditherSize = 2*r->ditherErrSize;
// 		r->ditherData = new U8[r->ditherSize];
// 		if ( !r->ditherData ) {
// 			FLASHASSERT(false);
// 			return;
// 		}
// 		memset(r->ditherData, 0, r->ditherSize);
// 		r->curDitherErr = r->ditherData;
// 		r->nextDitherErr = r->ditherData+r->ditherErrSize;
// 	}
// 
// 	// Error difusion dither
// 	FLASHASSERT( r->cinfo->refCount );
// 	U8* itab = r->cinfo->itab;
// 	const U8* noiseTab = ditherNoise+(8*(r->bitY&7));
// 	FLASHASSERT( r->cinfo->refCount );
// 	RGB8* ctab = r->cinfo->ctab.colors;
// 	RGBI* curErr = (RGBI*)r->curDitherErr+xmin+1;	// we bias the error arrays by 1 pixel so the -1 offset does not run off the end
// 	RGBI* nextErr = (RGBI*)r->nextDitherErr+xmin+1;
// 
// 	//RGB8 c;			// the color that we got for the current pixel
// 	//U8 dp;
// 	for ( S32 x = xmin; x < xmax; x++ ) {
// 		int noise = noiseTab[x&7];
// 
// 		_asm {
// 			// Calc the desired color
// 			//  color = pix + curErr
// 			mov			eax, dword ptr [curErr]
// 			movq		mm0, qword ptr [eax]
// 			mov			eax, dword ptr [pix]
// 			paddsw		mm0, qword ptr [eax]
// 			
// 			// Add some noise and calc the pixel RGB
// 			movd		mm2, dword ptr[noise]	// add a bit of noise
// 			punpcklwd	mm2, mm2				// replicate the noise value across the color components
// 			punpckldq	mm2, mm2
//  			paddsw		mm2, mm0
// 
// 			packuswb	mm2,mm2					// Pack the color elements to limit them to 0..0xff
// 
// 		//	mm0 is the color we want for this pixel
// 		//	mm2 is the color we matched in the color table
// 
// 			// int i = CalcITabIndex(c.red, c.green, c.blue);
// 			movd        eax, mm2
// 			mov			ebx, eax		// extract the red
// 			shr			ebx, 12
// 			and			ebx, 0F00h
// 			mov			ecx, eax		// extract the green
// 			shr			ecx, 8
// 			and			ecx, 0F0h
// 			or			ebx, ecx
// 			shr			eax, 4			// extract the blue
// 			and			eax, 0Fh
// 			or			eax, ebx
// 
// 			// U8 dp = itab[i];
// 			mov         ecx,dword ptr [itab]	// do the inverse lookup
// 			mov         dl,byte ptr [ecx+eax]
// 
// 			// *dst = dp;
// 			mov         eax,dword ptr [dst]		// set the pixel
// 			mov         byte ptr [eax],dl
// 
// 			// c = ctab[dp];   // get the actual color
// 			and         edx,0FFh
// 			mov         eax,dword ptr [ctab]
// 			movd		mm2,dword ptr [eax+edx*4]
// 
// 			// Propagate the errors
// 			pandn		mm1, mm1				// expand the color from the color table to 64 bits
// 			punpcklbw	mm2, mm1
// 
// 			// Calc the error
// 			psubsw		mm0, mm2
// 
// 			// Propagte 1/2 the error to the right pixel
// 			psraw		mm0, 1		// divide the error by 2 to get 1/2
// 			mov			eax, dword ptr [curErr]
// 			add			eax, 8
// 			movq		mm3, qword ptr [eax]
// 			paddsw		mm3, mm0
// 			movq		qword ptr [eax], mm3
// 
// 			// Propagte 1/4 the error to the pixel below and left
// 			psraw		mm0, 1		// divide the error by 2 again to get 1/4
// 			mov			eax, dword ptr [nextErr]
// 			movq		mm3, qword ptr [eax]
// 			paddsw		mm3, mm0
// 			movq		qword ptr [eax], mm3
// 
// 			// Propagte 1/4 the error to the pixel below and left
// 			sub			eax, 8			// go to nextErr-1
// 			movq		mm3, qword ptr [eax]
// 			paddsw		mm3, mm0
// 			movq		qword ptr [eax], mm3
// 		}
// 
// 		// Advance
// 		pix++;
// 		dst++;
// 		curErr++;
// 		nextErr++;
// 	}
// }
// #pragma warning( default : 4799 )  // Reenable "no EMMS instruction" warning messages
// #endif

//enum { solidPat = 0, disablePat = 1, highlightPat = 2, lineHighlightPat = 3 };
void ApplyRGBPattern(int pattern, int y, int xmin, int xmax, RGBI* pix)
{
	switch ( pattern ) {
		case disablePat:{
			RGBI* p = pix;
			for ( int i = xmax-xmin; i--; p++ ) {
				p->alpha = 255;//p->red/2 + 128;
				p->red   = (p->red>>1) + 128;
				p->green = (p->green>>1) + 128;
				p->blue  = (p->blue>>1) + 128;
			}
		} break;

		/*case highlightPat:{
			int j = (y & 0x2) + xmin;
			RGBI* p = pix;
			for ( int i = xmax-xmin; i--; p++, j++ ) {
				if ( j & 0x2 ) {
					p->alpha = 255;//p->red/2 + 128;
					p->red   = p->red ^ 0xFF;
					p->green = p->green ^ 0xFF;
					p->blue  = p->blue ^ 0xFF;
				}
			}
		} break;*/

		case highlightPat:
		case lineHighlightPat:{
			int mask = (pattern == highlightPat ? fillPatBits : linePatBits) >> 4*(y&0x3);
			RGBI* p = pix;
			for ( int i = xmin; i < xmax; p++, i++ ) {
				if ( (1 << (i&3)) & mask ) {
					// Pick the "opposite" color
					p->alpha = 255;//p->red/2 + 128;
					p->red   = p->red ^ 0xFF;
					p->green = p->green ^ 0xFF;
					p->blue  = p->blue ^ 0xFF;
				}
			}
		} break;
	}
}


//
// DrawRGBSlab Procs
//

void DrawRGBSlab1(CRaster* r, S32 xmin, S32 xmax, RGBI* pix)
{
	xmin += r->xorg;
	xmax += r->xorg;

	U8 buf[RGBSlabChunkSize];
	r->ditherRGBSlab(r, xmin, xmax, pix, buf);

	U8* src = buf;
	U8 * dst = (U8 *)r->rowAddr + xmin/8;
	int shift = 7 - ((int)xmin & 0x07);
	for ( int i = (int)(xmax-xmin); i--; src++ ) {
		*dst = (*dst & ~(0x01 << shift)) | (*src << shift);
		if ( --shift < 0 ) {
			shift = 7;
			dst++;
		}
	}
}

void DrawRGBSlab2(CRaster* r, S32 xmin, S32 xmax, RGBI* pix)
{
	xmin += r->xorg;
	xmax += r->xorg;

	U8 buf[RGBSlabChunkSize];
	r->ditherRGBSlab(r, xmin, xmax, pix, buf);

	U8* src = buf;
	U8 * dst = (U8 *)r->rowAddr + (xmin>>2);
	int shift = (3 - ((int)xmin & 0x03))<<1;
	for ( int i = (int)(xmax-xmin); i--; src++ ) {
		*dst = (*dst & ~(0x03 << shift)) | (*src << shift);
		shift-=2;
		if ( shift < 0 ) {
			shift = 6;
			dst++;
		}
	}
}

void DrawRGBSlab4(CRaster* r, S32 xmin, S32 xmax, RGBI* pix)
{
	xmin += r->xorg;
	xmax += r->xorg;

	U8 buf[RGBSlabChunkSize];
	r->ditherRGBSlab(r, xmin, xmax, pix, buf);

	U8* src = buf;
	U8 * dst = (U8 *)r->rowAddr + (xmin>>1);
	int shift = (1 - ((int)xmin & 0x01))<<2;
	for ( int i = (int)(xmax-xmin); i--; src++ ) {
		*dst = (*dst & ~(0x0F << shift)) | (*src << shift);
		shift -= 4;
		if ( shift < 0 ) {
			shift = 4;
			dst++;
		}
	}
}

void DrawRGBSlab8(CRaster* r, S32 xmin, S32 xmax, RGBI* pix)
{
	r->ditherRGBSlab(r, xmin, xmax, pix, (U8 *)r->rowAddr + xmin + r->xorg);
}

void DrawRGBSlab16(CRaster* r, S32 xmin, S32 xmax, RGBI* pix)
{
	U16 * dst = (U16 *)r->rowAddr + xmin  + r->xorg;
	if ( r->bitmapDither != noDither ) {
		// Add a little dither noise for smoother gradients
		const U8* noise = ditherNoise16+(r->bitY&0x3)*4;
		for ( S32 x = xmin; x < xmax; pix++, dst++, x++ ) {
			U16 d = noise[x&0x3];
			*dst = pix16PackR[(pix->red  +d)>>3] | 
				   pix16PackG[(pix->green+d)>>3] | 
				   pix16PackB[(pix->blue +d)>>3];
			//c = pix->red + d;
			//p = c < 0xff ? (c&0xf8)<<7 : 0xF8<<7;
			//c = pix->green + d;
			//p |= c < 0xff ? (c&0xf8)<<2 : 0xF8<<2;
			//c = pix->blue + d;
			//*dst = p | (c < 0xff ? (c/*&0xf8*/)>>3 : 0xF8>>3);
		}
	} else {
		for ( S32 i = xmax-xmin; i--; pix++, dst++ ) {
			*dst = PackPix16(pix);
		}
	}
}

// #ifdef USE_MMX
// 						//    aa  rr  gg  bb
// const __int64 cvt16Mask = 0x000000f800f800f8;
// 
// #pragma warning( disable : 4799 )  // Disable "no EMMS instruction" warning messages
// void DrawRGBSlab16_MMX(CRaster* r, S32 xmin, S32 xmax, RGBI* pix)
// {
// 	U16 * dst = (U16 *)r->rowAddr + xmin + r->xorg;
// 	const __int64* noise = ditherNoise16_MMX+(r->bitY&0x3)*4;
// 
// 	for ( S32 x = xmin; x < xmax; pix++, dst++, x++ ) {
// 		// 1) load the source pixel
// 		// 2) add the noise value w/ saturation (byte add)
// 		// 3) convert to a 16 bit pixel
// 		// 4) store the pixel
// 		const __int64* n = noise+(x&3);
// 
// 		_asm {
// 			mov         eax, dword ptr [pix]		// load the source pixel
// 			movq		mm0, qword ptr [eax]
// 			mov         eax, dword ptr [n]			// add the noise value w/ saturation (byte add)
// 			paddusb		mm0, qword ptr [eax]		
// 
// 			// convert to a 16 bit pixel
//  			pand		mm0, qword ptr [cvt16Mask]	// mask out all but 5 msbs of each color
// 			movq		mm1, mm0
// 			movq		mm2, mm0
//  			psrlq		mm0, 25						// set up the red bits (40-15)
// 			psrlq		mm1, 14						// set up the green bits (24-10)
//  			psrlq		mm2, 3						// set up the blue bits	(8-5)
//   			por			mm0, mm1
// 			por			mm0, mm2
// 
// 			mov         eax, dword ptr [dst]		// save final pixel
// 			movd        ebx, mm0
// 			mov			word ptr [eax], bx
// 		}
// 	}
// }
// #pragma warning( default : 4799 )  // Reenable "no EMMS instruction" warning messages
// #endif

void DrawRGBSlab16A(CRaster* r, S32 xmin, S32 xmax, RGBI* pix)
{
	U16 * dst = (U16 *)r->rowAddr + xmin + r->xorg;
	if ( r->bitmapDither != noDither ) {
		// Add a little dither noise for smoother gradients
		const U8* noise = ditherNoise16+(r->bitY&0x3)*4;
		for ( S32 x = xmin; x < xmax; pix++, dst++, x++ ) {	
			U16 d = noise[x&0x3];
			//U16 c = pix->green + d/2;
			//*dst = pix16PackR[(pix->red  +d)>>3]<<1 | 
			//	   c < 0xff ? (c&0xFC)<<3 : 0xFC<<3 | 
			//	   pix16PackB[(pix->blue +d)>>3];

			U16 c = pix->red + d;
			U16 p = (U16)(c < 0xff ? (c&0xF8)<<8 : 0xF8<<8);
			c = pix->green + (d>>1);
			p |= c < 0xff ? (c&0xFC)<<3 : 0xFC<<3;
			c = pix->blue + d;
			*dst = (U16 )(p | (c < 0xFF ? (c/*&0xf8*/)>>3 : 0xF8>>3));
		}
	} else {
		for ( S32 i = xmax-xmin; i--; pix++, dst++ ) {
			*dst = PackPix16A(pix);
		}
	}
}

// #ifdef USE_MMX
// 						 //    aa  rr  gg  bb
// const __int64 cvt16AMask = 0x000000f800fc00f8;
// 
// #pragma warning( disable : 4799 )  // Disable "no EMMS instruction" warning messages
// void DrawRGBSlab16A_MMX(CRaster* r, S32 xmin, S32 xmax, RGBI* pix)
// {
// 	U16 * dst = (U16 *)r->rowAddr + xmin + r->xorg;
// 	const __int64* noise = ditherNoise16_MMX+(r->bitY&0x3)*4;
// 
// 	for ( S32 x = xmin; x < xmax; pix++, dst++, x++ ) {
// 		// 1) load the source pixel
// 		// 2) add the noise value w/ saturation (byte add)
// 		// 3) convert to a 16 bit pixel
// 		// 4) store the pixel
// 		const __int64* n = noise+(x&3);
// 
// 		__int64 z;
// 
// 		_asm {
// 			mov         eax, dword ptr [pix]		// load the source pixel
// 			movq		mm0, qword ptr [eax]
// 			mov         eax, dword ptr [n]			// add the noise value w/ saturation (byte add)
// 			paddusb		mm0, qword ptr [eax]		
// 
// 			// convert to a 16 bit pixel
//  			pand		mm0, qword ptr [cvt16AMask]	// mask out all but 5 msbs of each color
// 			movq		mm1, mm0
// 			movq		mm2, mm0
//  			psrlq		mm0, 24						// set up the red bits (40-16)
// 			psrlq		mm1, 13						// set up the green bits (24-11)
//  			psrlw		mm2, 3						// set up the blue bits	(8-5) - note this is word shift so that the green does not leak over
// 			movq		qword ptr [z], mm0
// 			movq		qword ptr [z], mm1
// 			movq		qword ptr [z], mm2
//   			por			mm0, mm1
// 			por			mm0, mm2
// 
// 			mov         eax, dword ptr [dst]		// save final pixel
// 			movd        ebx, mm0					
// 			mov			word ptr [eax], bx
// 		}
// 	}
// }
// #pragma warning( default : 4799 )  // Reenable "no EMMS instruction" warning messages
// #endif

void DrawRGBSlab24(CRaster* r, S32 xmin, S32 xmax, RGBI* pix)
{
	U8 * dst = (U8 *)r->rowAddr + 3*(xmin + r->xorg);
	for ( S32 i = xmax-xmin; i--; pix++, dst+=3 ) {
		dst[2] = (U8)pix->red;
		dst[1] = (U8)pix->green;
		dst[0] = (U8)pix->blue;
	}
}

// #if !defined(SPRITE_XTRA) || !defined(_MAC)
// #pragma warning( disable : 4799 )  // Disable "no EMMS instruction" warning messages
// #endif
void DrawRGBSlab32(CRaster* r, S32 xmin, S32 xmax, RGBI* pix)
{
	U32 * dst = (U32 *)r->rowAddr + xmin + r->xorg;
	for ( S32 i = xmax-xmin; i--; pix++, dst++ ) {
		*dst = PackPix32(pix);
	}
}
// #if !defined(SPRITE_XTRA) || !defined(_MAC) 
// #pragma warning( default : 4799 )  // reenable "no EMMS instruction" warning messages
// #endif
// 
// #ifdef USE_MMX
// #pragma warning( disable : 4799 )  // Disable "no EMMS instruction" warning messages
// void DrawRGBSlab32_MMX(CRaster* r, S32 xmin, S32 xmax, RGBI* pix)
// {
// 	U32 * dst = (U32 *)r->rowAddr + xmin + r->xorg;
// 	int n = xmax-xmin;
// 	_asm {
// 		// Set up the loop variables
// 		mov         eax,dword ptr [n]
// 		mov         ebx,dword ptr [pix]
// 		mov         ecx,dword ptr [dst]
// 
// loop1:		
// 		test        eax,eax
// 		jle         exit1
// 
// 		// Pack the pixels
// 		movq		mm0, qword ptr [ebx]		// src
// 		packuswb	mm0,mm0
// 		movd		edx,mm0
// 		mov			dword ptr [ecx], edx
// 
// 		// Increment the loop
// 		sub			eax,1						// decrement n
// 		add         ebx,8						// increment src
// 		add         ecx,4						// increment dst
// 
// 		jmp			loop1
// exit1:
// 	}
// }
// #pragma warning( default : 4799 )  // Reenable "no EMMS instruction" warning messages
// #endif

void DrawRGBSlab32A(CRaster* r, S32 xmin, S32 xmax, RGBI* pix)
{
	U32 * dst = (U32 *)r->rowAddr + xmin + r->xorg;
	for ( S32 i = xmax-xmin; i--; pix++, dst++ ) {
		*dst = PackPix32A(pix, 0xFF);
	}
}


//
// Fast Blts
//

// Src == 8 bits

// #if 0
// void Blt8to8(BltInfo bi, SPOINT& pt, int n, U8 * dst)
// {
// 	if ( bi.dy == 0 ) {
// 		U8 * src = bi.baseAddr + (pt.y>>16)*bi.rowBytes;
// 		while ( n-- ) {
// 			RGB8 c = bi.colors[*(src + (pt.x>>16))];
// 			*dst++ = bi.itab[CalcITabIndex(c.red, c.green, c.blue)];
// 			pt.x += bi.dx;
// 		}
// 
// 	} else {
// 		while ( n-- ) {
// 			RGB8 c = bi.colors[*(bi.baseAddr + (pt.y>>16)*bi.rowBytes + (pt.x>>16))];
// 			*dst++ = bi.itab[CalcITabIndex(c.red, c.green, c.blue)];
// 			pt.x += bi.dx;
// 			pt.y += bi.dy;
// 		}
// 	}
// }
// #endif

void Blt8to8D(BltInfo bi, SPOINT& pt, int n, U8 * dst)
{
	CRaster* r = bi.color->raster;
	const U8* noise = ditherNoise8+(r->bitY&0x7)*8;
	int x = (int)(dst - (U8 *)(r->rowAddr));
	if ( bi.dy == 0 ) {
		U8 * src = bi.baseAddr + (pt.y>>16)*bi.rowBytes;
		while ( n-- ) {
			RGB8 c = bi.colors[*(src + (pt.x>>16))];
			int d = noise[x++&0x7];
			*dst++ = bi.itab[ditherTabR[(c.red  +d)>>4] | 
							 ditherTabG[(c.green+d)>>4] |
							 ditherTabB[(c.blue +d)>>4] ];
			pt.x += bi.dx;
		}

	} else {
		while ( n-- ) {
			RGB8 c = bi.colors[*(bi.baseAddr + (pt.y>>16)*bi.rowBytes + (pt.x>>16))];
			int d = noise[x++&0x7];
			*dst++ = bi.itab[ditherTabR[(c.red  +d)>>4] | 
						     ditherTabG[(c.green+d)>>4] |
						     ditherTabB[(c.blue +d)>>4] ];
			pt.x += bi.dx;
			pt.y += bi.dy;
		}
	}
}

void Blt8to16(BltInfo bi, SPOINT& pt, int n, U16 * dst)
{
	if ( bi.dy == 0 ) {
		U8 * src = bi.baseAddr + (pt.y>>16)*bi.rowBytes;
		while ( n-- ) {
			RGB8 c = bi.colors[*(src + (pt.x>>16))];
			*dst++ = ((int)c.red   & 0xF8) << 7  | 
					 ((int)c.green & 0xF8) << 2  | 
					 ((int)c.blue  /*& 0xF8*/) >> 3;
			pt.x += bi.dx;
		}

	} else {
		while ( n-- ) {
			RGB8 c = bi.colors[*(bi.baseAddr + (pt.y>>16)*bi.rowBytes + (pt.x>>16))];
			*dst++ = ((int)c.red   & 0xF8) << 7  | 
					 ((int)c.green & 0xF8) << 2  | 
					 ((int)c.blue  /*& 0xF8*/) >> 3;
			pt.x += bi.dx;
			pt.y += bi.dy;
		}
	}
}

void Blt8to16A(BltInfo bi, SPOINT& pt, int n, U16 * dst)
{
	if ( bi.dy == 0 ) {
		U8 * src = bi.baseAddr + (pt.y>>16)*bi.rowBytes;
		while ( n-- ) {
			RGB8 c = bi.colors[*(src + (pt.x>>16))];
			*dst++ = ((int)c.red   & 0xF8) << 8  | 
					 ((int)c.green & 0xFC) << 3  | 
					 ((int)c.blue  /*& 0xF8*/) >> 3;
			pt.x += bi.dx;
		}

	} else {
		while ( n-- ) {
			RGB8 c = bi.colors[*(bi.baseAddr + (pt.y>>16)*bi.rowBytes + (pt.x>>16))];
			*dst++ = ((int)c.red   & 0xF8) << 8  | 
					 ((int)c.green & 0xFC) << 3  | 
					 ((int)c.blue  /*& 0xF8*/) >> 3;
			pt.x += bi.dx;
			pt.y += bi.dy;
		}
	}
}

// #if defined(_UNIX) && !defined(GENERATOR)
// inline U32 MakePix32(U8 r, U8 g, U8 b)
// {
//   return ((U32)r<<g_redShift)|((U32)g<<g_greenShift)|((U32)b<<g_blueShift);
// }
// #endif

void Blt8to32(BltInfo bi, SPOINT& pt, int n, U32 * dst)
{
	if ( bi.dy == 0 ) {
		U8 * src = bi.baseAddr + (pt.y>>16)*bi.rowBytes;
		while ( n-- ) {
			RGB8 c = bi.colors[*(src + (pt.x>>16))];
// #if defined(_UNIX)  && !defined(GENERATOR)
// 			*dst++ = MakePix32(c.red,c.green,c.blue);
// #else
			*dst++ = (U32)c.red   << 16 | 
					 (U32)c.green << 8  | 
						  c.blue ;
// #endif
			pt.x += bi.dx;
		}

	} else {
		while ( n-- ) {
			RGB8 c = bi.colors[*(bi.baseAddr + (pt.y>>16)*bi.rowBytes + (pt.x>>16))];
// #if defined(_UNIX) && !defined(GENERATOR)
// 			*dst++ = MakePix32(c.red,c.green,c.blue);
// #else
			*dst++ = (U32)c.red   << 16 | 
					 (U32)c.green << 8  | 
						  c.blue ;
// #endif
			pt.x += bi.dx;
			pt.y += bi.dy;
		}
	}
}

// Src == 16 bits

// #if 0
// void Blt16to8(BltInfo bi, SPOINT& pt, int n, U8 * dst)
// {
// 	if ( bi.dy == 0 ) {
// 		U16 * src = (U16 *)(bi.baseAddr + (pt.y>>16)*bi.rowBytes);
// 		while ( n-- ) {
// 			int pix = src[pt.x >> 16];
// 			*dst++ = bi.itab[CalcITabIndex(pix16Expand[(pix>>10)&0x1f], 
// 										   pix16Expand[(pix>> 5)&0x1f], 
// 										   pix16Expand[(pix    )&0x1f])];
// 			pt.x += bi.dx;
// 		}
// 
// 	} else {
// 		while ( n-- ) {
// 			int pix = *((U16 *)(bi.baseAddr + (pt.y>>16)*bi.rowBytes) + (pt.x>>16));
// 			*dst++ = bi.itab[CalcITabIndex(pix16Expand[(pix>>10)&0x1f], 
// 										   pix16Expand[(pix>> 5)&0x1f], 
// 										   pix16Expand[(pix    )&0x1f])];
// 			pt.x += bi.dx;
// 			pt.y += bi.dy;
// 		}
// 	}
// }
// #endif

void Blt16to8D(BltInfo bi, SPOINT& pt, int n, U8 * dst)
{
	CRaster* r = bi.color->raster;
	const U8* noise = ditherNoise8+(r->bitY&0x7)*8;
	int x = (int)(dst - (U8 *)(r->rowAddr));
	if ( bi.dy == 0 ) {
		U16 * src = (U16 *)(bi.baseAddr + (pt.y>>16)*bi.rowBytes);
		while ( n-- ) {
			int pix = src[pt.x >> 16];
			int d = noise[x++&0x7];
			*dst++ = bi.itab[ditherTabR[(((pix>> 7)&0xF8)+d)>>4] | 
							 ditherTabG[(((pix>> 2)&0xF8)+d)>>4] |
							 ditherTabB[(((pix<< 3)&0xF8)+d)>>4] ];
			pt.x += bi.dx;
		}

	} else {
		while ( n-- ) {
			int pix = *((U16 *)(bi.baseAddr + (pt.y>>16)*bi.rowBytes) + (pt.x>>16));
			int d = noise[x++&0x7];
			*dst++ = bi.itab[ditherTabR[(((pix>> 7)&0xF8)+d)>>4] | 
							 ditherTabG[(((pix>> 2)&0xF8)+d)>>4] |
							 ditherTabB[(((pix<< 3)&0xF8)+d)>>4] ];
			pt.x += bi.dx;
			pt.y += bi.dy;
		}
	}
}

void Blt16to16(BltInfo bi, SPOINT& pt, int n, U16 * dst)
{
	if ( bi.dy == 0 ) {
		U16 * src = (U16 *)(bi.baseAddr + (pt.y>>16)*bi.rowBytes);
		if ( Abs(bi.dx-fixed_1) < 0x100 ) {
			src += (pt.x>>16);
			pt.x += n*bi.dx;
			while ( n-- )
				*dst++ = *src++;
		} else {
			while ( n-- ) {
				*dst++ = src[pt.x>>16];
				pt.x += bi.dx;
			}
		}

	} else {
		while ( n-- ) {
			*dst++ = *((U16 *)(bi.baseAddr + (pt.y>>16)*bi.rowBytes) + (pt.x>>16));
			pt.x += bi.dx;
			pt.y += bi.dy;
		}
	}
}

void Blt16to16A(BltInfo bi, SPOINT& pt, int n, U16 * dst)
{
	if ( bi.dy == 0 ) {
		U16 * src = (U16 *)(bi.baseAddr + (pt.y>>16)*bi.rowBytes);
		while ( n-- ) {
			int pix = src[pt.x>>16];
			*dst++ = ((pix<<1)&0xFFE0) | (pix&0x3F);
			pt.x += bi.dx;
		}

	} else {
		while ( n-- ) {
			int pix = *((U16 *)(bi.baseAddr + (pt.y>>16)*bi.rowBytes) + (pt.x>>16));
			*dst++ = ((pix<<1)&0xFFE0) | (pix&0x3F);
			pt.x += bi.dx;
			pt.y += bi.dy;
		}
	}
}

void Blt16to32(BltInfo bi, SPOINT& pt, int n, U32 * dst)
{
	if ( bi.dy == 0 ) {
		U16 * src = (U16 *)(bi.baseAddr + (pt.y>>16)*bi.rowBytes);
		while ( n-- ) {
			int pix = *(src + (pt.x>>16));
			*dst++ = Pix16To32(pix);
			pt.x += bi.dx;
		}

	} else {
		while ( n-- ) {
			int pix = *((U16 *)(bi.baseAddr + (pt.y>>16)*bi.rowBytes) + (pt.x>>16));
			*dst++ = Pix16To32(pix);
			pt.x += bi.dx;
			pt.y += bi.dy;
		}
	}
}

// Src == 32

// #if 0
// void Blt32to8(BltInfo bi, SPOINT& pt, int n, U8 * dst)
// {
// 	if ( bi.dy == 0 ) {
// 		U32 * src = (U32 *)(bi.baseAddr + (pt.y>>16)*bi.rowBytes);
// 		while ( n-- ) {
// 			U32 pix = src[pt.x >> 16];
// 			*dst++ = bi.itab[CalcITabIndex(pix >> 16, 
// 										   pix >>  8, 
// 										   pix)];
// 			pt.x += bi.dx;
// 		}
// 
// 	} else {
// 		while ( n-- ) {
// 			U32 pix = *((U32 *)(bi.baseAddr + (pt.y>>16)*bi.rowBytes) + (pt.x>>16));
// 			*dst++ = bi.itab[CalcITabIndex(pix >> 16, 
// 										   pix >>  8, 
// 										   pix)];
// 			pt.x += bi.dx;
// 			pt.y += bi.dy;
// 		}
// 	}
// }
// #endif

void Blt32to8D(BltInfo bi, SPOINT& pt, int n, U8 * dst)
{
	CRaster* r = bi.color->raster;
	const U8* noise = ditherNoise8+(r->bitY&0x7)*8;
	int x = (int)(dst - (U8 *)(r->rowAddr));
	if ( bi.dy == 0 ) {
		U32 * src = (U32 *)(bi.baseAddr + (pt.y>>16)*bi.rowBytes);
		while ( n-- ) {
			U32 pix = src[pt.x >> 16];
			int d = noise[x++&0x7];
			*dst++ = bi.itab[ditherTabR[(((pix >> 16)&0xFF)+d)>>4] | 
							 ditherTabG[(((pix >>  8)&0xFF)+d)>>4] |
							 ditherTabB[(((pix      )&0xFF)+d)>>4] ];
			pt.x += bi.dx;
		}

	} else {
		while ( n-- ) {
			U32 pix = *((U32 *)(bi.baseAddr + (pt.y>>16)*bi.rowBytes) + (pt.x>>16));
			int d = noise[x++&0x7];
			*dst++ = bi.itab[ditherTabR[(((pix >> 16)&0xFF)+d)>>4] | 
							 ditherTabG[(((pix >>  8)&0xFF)+d)>>4] |
							 ditherTabB[(((pix      )&0xFF)+d)>>4] ];
			pt.x += bi.dx;
			pt.y += bi.dy;
		}
	}
}

void Blt32to16(BltInfo bi, SPOINT& pt, int n, U16 * dst)
{
	if ( bi.dy == 0 ) {
		U32 * src = (U32 *)(bi.baseAddr + (pt.y>>16)*bi.rowBytes);
		while ( n-- ) {
			U32 pix = src[pt.x >> 16];
			*dst++ = (U16)(((pix >> (24-15)) & 0x7C00) | 
						   ((pix >> (16-10)) & 0x03E0) | 
						   ((pix >> ( 8- 5)) & 0x001F));
			pt.x += bi.dx;
		}

	} else {
		while ( n-- ) {
			U32 pix = *((U32 *)(bi.baseAddr + (pt.y>>16)*bi.rowBytes) + (pt.x>>16));
			*dst++ = (U16)(((pix >> (24-15)) & 0x7C00) | 
						   ((pix >> (16-10)) & 0x03E0) | 
						   ((pix >> ( 8- 5)) & 0x001F));
			pt.x += bi.dx;
			pt.y += bi.dy;
		}
	}
}

void Blt32to16A(BltInfo bi, SPOINT& pt, int n, U16 * dst)
{
	if ( bi.dy == 0 ) {
		U32 * src = (U32 *)(bi.baseAddr + (pt.y>>16)*bi.rowBytes);
		while ( n-- ) {
			U32 pix = src[pt.x >> 16];
			*dst++ = (U16)(((pix >> (24-16)) & 0xF800) | 
						   ((pix >> (16-11)) & 0x07E0) | 
						   ((pix >> ( 8- 5)) & 0x001F));
			pt.x += bi.dx;
		}

	} else {
		while ( n-- ) {
			U32 pix = *((U32 *)(bi.baseAddr + (pt.y>>16)*bi.rowBytes) + (pt.x>>16));
			*dst++ = (U16)(((pix >> (24-16)) & 0xF800) | 
						   ((pix >> (16-11)) & 0x07E0) | 
						   ((pix >> ( 8- 5)) & 0x001F));
			pt.x += bi.dx;
			pt.y += bi.dy;
		}
	}
}

void Blt32to32(BltInfo bi, SPOINT& pt, int n, U32 * dst)
{
	if ( bi.dy == 0 ) {
		U32 * src = (U32 *)(bi.baseAddr + (pt.y>>16)*bi.rowBytes);
		if ( Abs(bi.dx-fixed_1) < 0x100 ) {
			src += (pt.x>>16);
			pt.x += n*bi.dx;
			while ( n-- )
				*dst++ = *src++;
		} else {
			while ( n-- ) {
				*dst++ = src[pt.x >> 16];
				pt.x += bi.dx;
			}
		}
	} else {
		while ( n-- ) {
			*dst++ = *((U32 *)(bi.baseAddr + (pt.y>>16)*bi.rowBytes) + (pt.x>>16));
			pt.x += bi.dx;
			pt.y += bi.dy;
		}
	}
}


//
// General Blts
//

#ifdef SMOOTHBITS

// A wide pixel is a specially scaled pixel used for blending pixels when 
//	transforming bitmaps - the top 3 bits are reserved for scaling
// 8+3 bits are reserved for red   - bits 21..31
// 8+3 bits are reserved for green - bits 10..20
// 7+3 bits are reserved for blue  - bits 0..9
#define wideColorMask = ((0x1ffL<<22) | (0x1ffL<<10) | 0xff)
inline U32 MakeWideColor(RGB8 c)
{
	return ((U32)c.red << (21)) | ((U32)c.green << 10) | ((U32)c.blue>>1);
}

inline U32 Pix16ToWide(U32 pix)
{
	return ((U32)pix16Expand[(pix>>10)&0x1f] << 21) |
		   ((U32)pix16Expand[(pix>> 5)&0x1f] << 10) |
		   (pix16Expand[(pix    )&0x1f] >> 1 );
}

inline U32 Pix32ToWide(U32 pix)
{
	return ((pix & 0xFF0000L)<<5) |
		   ((pix &   0xFF00L)<<2) |
		   ((pix &     0xFFL)>>1);
}

inline void WideToRGBI(U32 pixW, RGBI* pix)
{
	pix->alpha = 255;
	pix->red   = (int)(pixW >> 24);
	pix->green = (U8)(pixW >> 13);
	pix->blue  = (U8)(pixW >> 2 );
}

void Blt8toIS(BltInfo bi, SPOINT& pt, int n, RGBI* pix)
{
	while ( n-- ) {
		// Get the first row
		int* cv = PixCoverage[(U16)pt.x>>13][(U16)pt.y>>13].ce;
			
		int xi = pt.x >> 16;
		int yi = pt.y >> 16;
		S32 xInc = xi < bi.width-1 ? 1 : 1-bi.width;
		U32 pixW;
		U8 * bits = bi.baseAddr + yi*bi.rowBytes + xi;
		pixW  = MakeWideColor(bi.colors[bits[0]]) * cv[0];
		pixW += MakeWideColor(bi.colors[bits[xInc]]) * cv[1];

		// Get the second row
		bits += yi < bi.height-1 ? bi.rowBytes : bi.rowBytes*(1-bi.height);
		pixW += MakeWideColor(bi.colors[bits[0]]) * cv[2];
		pixW += MakeWideColor(bi.colors[bits[xInc]]) * cv[3];
		
		WideToRGBI(pixW, pix);
		pix++;

		// Advance
		pt.x += bi.dx;
		pt.y += bi.dy;
	}
}

void Blt16toIS(BltInfo bi, SPOINT& pt, int n, RGBI* pix)
{				
	while ( n-- ) {
		// Get the first row
		int* cv = PixCoverage[(U16)pt.x>>13][(U16)pt.y>>13].ce;
			
		int x = pt.x >> 16;
		int y = pt.y >> 16;
		S32 xInc = x < bi.width-1 ? 1 : 1-bi.width;
		U32 pixW;
		U16 * bits = (U16 *)(bi.baseAddr + (pt.y>>16)*bi.rowBytes) + x;
		pixW  = Pix16ToWide(bits[0]) * cv[0];
		pixW += Pix16ToWide(bits[xInc]) * cv[1];

		// Get the second row
		bits = (U16*)((U8*)bits + (y < bi.height-1 ? bi.rowBytes : bi.rowBytes*(1-bi.height)));
		pixW += Pix16ToWide(bits[0]) * cv[2];
		pixW += Pix16ToWide(bits[xInc]) * cv[3];

		WideToRGBI(pixW, pix);
		pix++;

		// Advance
		pt.x += bi.dx;
		pt.y += bi.dy;
	}
}
				
void Blt32toIS(BltInfo bi, SPOINT& pt, int n, RGBI* pix)
{
	while ( n-- ) {
		// Get the first row
		int* cv = PixCoverage[(U16)pt.x>>13][(U16)pt.y>>13].ce;
			
		int x = pt.x >> 16;
		int y = pt.y >> 16;
		S32 xInc = x < bi.width-1 ? 1 : 1-bi.width;
		U32 pixW;
		U32 * bits = (U32 *)(bi.baseAddr + (pt.y>>16)*bi.rowBytes) + x;
		pixW  = Pix32ToWide(bits[0]) * cv[0];
		pixW += Pix32ToWide(bits[xInc]) * cv[1];

		// Get the second row
		bits = (U32 *)((U8 *)bits + (y < bi.height-1 ? bi.rowBytes : bi.rowBytes*(1-bi.height)));
		pixW += Pix32ToWide(bits[0]) * cv[2];
		pixW += Pix32ToWide(bits[xInc]) * cv[3];

		WideToRGBI(pixW, pix);
		pix++;

		// Advance
		pt.x += bi.dx;
		pt.y += bi.dy;
	}
}

void Blt8toISA(BltInfo bi, SPOINT& pt, int n, RGBI* pix)
{
	while ( n-- ) {
		// Get the first row
		int* cv = PixCoverage[(U16)pt.x>>13][(U16)pt.y>>13].ce;
			
		int xi = pt.x >> 16;
		int yi = pt.y >> 16;
		S32 xInc = xi < bi.width-1 ? 1 : 1-bi.width;
		RGBIU p;
		RGBIU pp;
		U8 * bits = bi.baseAddr + yi*bi.rowBytes + xi;
		ExpandColor(bi.colors[bits[0]], &p.i);
		p.l.c0 *= cv[0];
		p.l.c1 *= cv[0];
		ExpandColor(bi.colors[bits[xInc]], &pp.i);
		p.l.c0 += pp.l.c0*cv[1];
		p.l.c1 += pp.l.c1*cv[1];

		// Get the second row
		bits += yi < bi.height-1 ? bi.rowBytes : bi.rowBytes*(1-bi.height);
		ExpandColor(bi.colors[bits[0]], &pp.i);
		p.l.c0 += pp.l.c0*cv[2];
		p.l.c1 += pp.l.c1*cv[2];
		ExpandColor(bi.colors[bits[xInc]], &pp.i);
		p.l.c0 += pp.l.c0*cv[3];
		p.l.c1 += pp.l.c1*cv[3];

		((RGBIL*)pix)->c0 = (p.l.c0>>3) & 0x00FF00FF;
		((RGBIL*)pix)->c1 = (p.l.c1>>3) & 0x00FF00FF;
		pix++;

		// Advance
		pt.x += bi.dx;
		pt.y += bi.dy;
	}
}

void Blt16toISA(BltInfo bi, SPOINT& pt, int n, RGBI* pix)
{				
	while ( n-- ) {
		// Get the first row
		int* cv = PixCoverage[(U16)pt.x>>13][(U16)pt.y>>13].ce;
			
		int x = pt.x >> 16;
		int y = pt.y >> 16;
		S32 xInc = x < bi.width-1 ? 1 : 1-bi.width;
		RGBIU p;
		RGBIU pp;
		U16 * bits = (U16 *)(bi.baseAddr + (pt.y>>16)*bi.rowBytes) + x;
		UnpackPix16(bits[0], &p.i);
		p.l.c0 *= cv[0];
		p.l.c1 *= cv[0];
		UnpackPix16(bits[xInc], &pp.i);
		p.l.c0 += pp.l.c0*cv[1];
		p.l.c1 += pp.l.c1*cv[1];

		// Get the second row
		bits = (U16*)((U8*)bits + (y < bi.height-1 ? bi.rowBytes : bi.rowBytes*(1-bi.height)));
		UnpackPix16(bits[0], &pp.i);
		p.l.c0 += pp.l.c0*cv[2];
		p.l.c1 += pp.l.c1*cv[2];
		UnpackPix16(bits[xInc], &pp.i);
		p.l.c0 += pp.l.c0*cv[3];
		p.l.c1 += pp.l.c1*cv[3];

		((RGBIL*)pix)->c0 = (p.l.c0>>3) & 0x00FF00FF;
		((RGBIL*)pix)->c1 = (p.l.c1>>3) & 0x00FF00FF;
		pix++;

		// Advance
		pt.x += bi.dx;
		pt.y += bi.dy;
	}
}
				
void Blt32toISA(BltInfo bi, SPOINT& pt, int n, RGBI* pix)
{
	while ( n-- ) {
		// Get the first row
		int* cv = PixCoverage[(U16)pt.x>>13][(U16)pt.y>>13].ce;
			
		int x = pt.x >> 16;
		int y = pt.y >> 16;
		S32 xInc = x < bi.width-1 ? 1 : 1-bi.width;
		RGBIU p;
		RGBIU pp;
		U32 * bits = (U32 *)(bi.baseAddr + (pt.y>>16)*bi.rowBytes) + x;
		UnpackPix32(bits[0], &p.i);
		p.l.c0 *= cv[0];
		p.l.c1 *= cv[0];
		UnpackPix32(bits[xInc], &pp.i);
		p.l.c0 += pp.l.c0*cv[1];
		p.l.c1 += pp.l.c1*cv[1];

		// Get the second row
		bits = (U32 *)((U8 *)bits + (y < bi.height-1 ? bi.rowBytes : bi.rowBytes*(1-bi.height)));
		UnpackPix32(bits[0], &pp.i);
		p.l.c0 += pp.l.c0*cv[2];
		p.l.c1 += pp.l.c1*cv[2];
		UnpackPix32(bits[xInc], &pp.i);
		p.l.c0 += pp.l.c0*cv[3];
		p.l.c1 += pp.l.c1*cv[3];

		((RGBIL*)pix)->c0 = (p.l.c0>>3) & 0x00FF00FF;
		((RGBIL*)pix)->c1 = (p.l.c1>>3) & 0x00FF00FF;
		pix++;

		// Advance
		pt.x += bi.dx;
		pt.y += bi.dy;
	}
}

// #if defined(USE_MMX) || defined(USE_KATMAI)
// 
// const __int64 rgbMask = 0x00ff00ff00ff00ff;
// 
// #pragma warning( disable : 4799 )  // Disable "no EMMS instruction" warning messages
// void ApplyCoverage_MMX(void* p, int* cv, RGBI* pix)
// {
// 	_asm {
// 		// Expand the source pixels into mmx registers
// 		pandn		mm0, mm0				// make a zero reg
// 
// 		mov			eax, dword ptr[p]
// 		mov			ebx, dword ptr[cv]
// 
// 		movd		mm1, dword ptr [eax]	// load the source pixel
// 		punpcklbw	mm1, mm0				// expand to 64 bits
// 		movd		mm2, dword ptr [ebx]	// load the source coverage
// 		punpcklwd	mm2,mm2					// replicate the coverage value across the pixels
// 		punpckldq	mm2,mm2
// 		pmullw		mm1,mm2					// apply the coverage
// 
// 		movd		mm3, dword ptr [eax+4]	// load the source pixel
// 		punpcklbw	mm3, mm0				// expand to 64 bits
// 		movd		mm2, dword ptr [ebx+4]	// load the source coverage
// 		punpcklwd	mm2, mm2				// replicate the coverage value across the pixels
// 		punpckldq	mm2, mm2
// 		pmullw		mm3, mm2				// apply the coverage
// 		paddusw		mm1, mm3				// calc the sum
// 
// 		movd		mm3, dword ptr [eax+8]	// load the source pixel
// 		punpcklbw	mm3, mm0				// expand to 64 bits
// 		movd		mm2, dword ptr [ebx+8]	// load the source coverage
// 		punpcklwd	mm2, mm2				// replicate the coverage value across the pixels
// 		punpckldq	mm2, mm2
// 		pmullw		mm3, mm2				// apply the coverage
// 		paddusw		mm1, mm3				// calc the sum
// 
// 		movd		mm3, dword ptr [eax+12]	// load the source pixel
// 		punpcklbw	mm3, mm0				// expand to 64 bits
// 		movd		mm2, dword ptr [ebx+12]	// load the source coverage
// 		punpcklwd	mm2, mm2				// replicate the coverage value across the pixels
// 		punpckldq	mm2, mm2
// 		pmullw		mm3, mm2				// apply the coverage
// 		paddusw		mm1, mm3				// calc the sum
// 
// 		psrlw		mm1, 3					// divide by 8
// 		pand		mm1, dword ptr [rgbMask]// mask	- should not be needed but just in case...
// 
// 		mov			eax, dword ptr [pix]	// store the result
// 		movq		qword ptr [eax], mm1	
// 	}
// }
// #pragma warning( default : 4799 )  // Reenable "no EMMS instruction" warning messages
// 
// void Blt8toISA_MMX(BltInfo bi, SPOINT& pt, int n, RGBI* pix)
// {
// 	RGB8 p[4];
// 	while ( n-- ) {
// 		// Get the pixels
// 		int xi = pt.x >> 16;
// 		int yi = pt.y >> 16;
// 		S32 xInc = xi < bi.width-1 ? 1 : 1-bi.width;
// 		U8 * bits = bi.baseAddr + yi*bi.rowBytes + xi;
// 		p[0] = bi.colors[bits[0]];
// 		p[1] = bi.colors[bits[xInc]];
// 		
// 		bits += yi < bi.height-1 ? bi.rowBytes : bi.rowBytes*(1-bi.height);
//  		p[2] = bi.colors[bits[0]];
// 		p[3] = bi.colors[bits[xInc]];
// 
// 		// Combine the pixels
// 		ApplyCoverage_MMX(p, PixCoverage[(U16)pt.x>>13][(U16)pt.y>>13].ce, pix);
// 
// 		// Advance
// 		pix++;
// 		pt.x += bi.dx;
// 		pt.y += bi.dy;
// 	}
// }
// 
// #endif

// #ifdef USE_MMX
// 
// void Blt16toISA_MMX(BltInfo bi, SPOINT& pt, int n, RGBI* pix)
// {				
// 	U32 p[4];
// 	while ( n-- ) {
// 		// Get the pixels
// 		int x = pt.x >> 16;
// 		int y = pt.y >> 16;
// 		S32 xInc = x < bi.width-1 ? 1 : 1-bi.width;
// 		U16 * bits = (U16 *)(bi.baseAddr + (pt.y>>16)*bi.rowBytes) + x;
// 		p[0] = Pix16To32(bits[0]);
// 		p[1] = Pix16To32(bits[xInc]);
// 
// 		bits = (U16*)((U8*)bits + (y < bi.height-1 ? bi.rowBytes : bi.rowBytes*(1-bi.height)));
// 		p[2] = Pix16To32(bits[0]);
// 		p[3] = Pix16To32(bits[xInc]);
// 
// 		// Combine the pixels
// 		ApplyCoverage_MMX(p, PixCoverage[(U16)pt.x>>13][(U16)pt.y>>13].ce, pix);
// 
// 		// Advance
// 		pix++;
// 		pt.x += bi.dx;
// 		pt.y += bi.dy;
// 	}
// }
// 				
// void Blt32toISA_MMX(BltInfo bi, SPOINT& pt, int n, RGBI* pix)
// {
// 	U32 p[4];
// 	while ( n-- ) {
// 		// Get the pixels
// 		int x = pt.x >> 16;
// 		int y = pt.y >> 16;
// 		S32 xInc = x < bi.width-1 ? 1 : 1-bi.width;
// 		U32 * bits = (U32 *)(bi.baseAddr + (pt.y>>16)*bi.rowBytes) + x;
// 		p[0] = bits[0];
// 		p[1] = bits[xInc];
// 
// 		bits = (U32 *)((U8 *)bits + (y < bi.height-1 ? bi.rowBytes : bi.rowBytes*(1-bi.height)));
// 		p[2] = bits[0];
// 		p[3] = bits[xInc];
// 
// 		// Combine the pixels
// 		ApplyCoverage_MMX(p, PixCoverage[(U16)pt.x>>13][(U16)pt.y>>13].ce, pix);
// 
// 		// Advance
// 		pix++;
// 		pt.x += bi.dx;
// 		pt.y += bi.dy;
// 	}
// }
// #endif
// 
#endif			//SMOOTHBITS

void BltXtoI(BltInfo bi, SPOINT& pt, int n, RGBI* pix)
{
	FLASHASSERT(false);
}

void Blt8toI(BltInfo bi, SPOINT& pt, int n, RGBI* pix)
// src = bm8Bit dst = RGBI
{
	if ( bi.dy == 0 ) {
		U8 * rowAddr = bi.baseAddr + (pt.y>>16)*bi.rowBytes;
		while ( n-- ) {
			ExpandColor(bi.colors[rowAddr[pt.x>>16]], pix);
			pix++;
			pt.x += bi.dx;
		}
	} else {
		while ( n-- ) {
			ExpandColor(bi.colors[*(bi.baseAddr + (pt.y>>16)*bi.rowBytes + (pt.x>>16))], pix);
			pix++;
			pt.x += bi.dx;
			pt.y += bi.dy;
		}
	}
}

void Blt16toI(BltInfo bi, SPOINT& pt, int n, RGBI* pix)
{				
	if ( bi.dy == 0 ) {
		U16 * rowAddr = (U16 *)(bi.baseAddr + (pt.y>>16)*bi.rowBytes);
		while ( n-- ) {
			UnpackPix16(rowAddr[pt.x>>16], pix);
			pix++;
			pt.x += bi.dx;
		}
	} else {
		while ( n-- ) {
			UnpackPix16(*((U16 *)(bi.baseAddr + (pt.y>>16)*bi.rowBytes) + (pt.x>>16)), pix);
			pix++;
			pt.x += bi.dx;
			pt.y += bi.dy;
		}
	}
}

void Blt32toI(BltInfo bi, SPOINT& pt, int n, RGBI* pix)
{				
	if ( bi.dy == 0 ) {
		U32 * rowAddr = (U32 *)(bi.baseAddr + (pt.y>>16)*bi.rowBytes);
		for ( int i = n; i--; ) {
			UnpackPix32(rowAddr[pt.x>>16], pix);
			pix++;
			pt.x += bi.dx;
		}
	} else {
		for ( int i = n; i--; ) {
			UnpackPix32(*((U32 *)(bi.baseAddr + (pt.y>>16)*bi.rowBytes) + (pt.x>>16)), pix);
			pix++;
			pt.x += bi.dx;
			pt.y += bi.dy;
		}
	}
}

void DrawBitmapSlab(RColor* color, S32 xmin, S32 xmax)
{
	CRaster* r = color->raster;

	// Calc the start point in bitmap src coordinates
	SPOINT pt;
	pt.x = xmin << 16;
	pt.y = r->bitY << 16;
	MatrixTransformPoint(&color->bm.invMat, &pt, &pt);

	// Set up the slab start point
	S32 wLimit = (S32)color->bm.bi.width<<16;
	S32 hLimit = (S32)color->bm.bi.height<<16;
	if ( color->bm.bitsStyle & fillBitsClip ) {
		// Treat the edges of a cliped bitmap special to prevent rounding problems
		// Check for out of range values
		RGBI pix;
		int x,y;

		SBitmapCore* bitmap = color->bm.bitmap;
		int w = bitmap->width;
		int h = bitmap->height;
		if ( color->bm.smooth ) {
			h--; w--;		// allow for the extra row and column when getting 4 samples
		}

		SPOINT tail;
		S32 count = xmax-xmin;
		tail.x = pt.x + color->bm.bi.dx*count;
		tail.y = pt.y + color->bm.bi.dy*count;

		// Adjust the head
		for (;;) {
			x = (int)(pt.x>>16);
			y = (int)(pt.y>>16);
			if ( x >= 0 && y >= 0 && x < w && y < h ) break;
			if ( xmin >= xmax ) break;
			#ifdef SMOOTHBITS
			if ( color->bm.smooth )
				bitmap->GetSSRGBPixel(pt.x, pt.y, &pix);
			else
			#endif
				bitmap->GetRGBPixel(x, y, &pix);

			if ( color->bm.colorMap )
				ApplyColorMap(color->bm.colorMap, &pix);
			r->drawRGBSlab(r, xmin, xmin+1, &pix);

			pt.x += color->bm.bi.dx;
			pt.y += color->bm.bi.dy;
			xmin++;
		}

		// Adjust the tail
		for (;;) {
			x = (int)(tail.x>>16);
			y = (int)(tail.y>>16);
			if ( x >= 0 && y >= 0 && x < w && y < h ) break;
			if ( xmin >= xmax ) break;
			#ifdef SMOOTHBITS
			if ( color->bm.smooth )
				bitmap->GetSSRGBPixel(tail.x, tail.y, &pix);
			else
			#endif
				bitmap->GetRGBPixel(x, y, &pix);
			xmax--;
			if ( color->bm.colorMap )
				ApplyColorMap(color->bm.colorMap, &pix);
			r->drawRGBSlab(r, xmax, xmax+1, &pix);

			tail.x -= color->bm.bi.dx;
			tail.y -= color->bm.bi.dy;
		}
	} else {
		// Do an initial coordinate wrap for textures
		pt.x = LimitAbs(pt.x, wLimit);
		pt.y = LimitAbs(pt.y, hLimit);
	}

	if ( color->bm.fastBltProc ) {
		// Draw the slab for the fast case
// 		int pixSize = mapPixelFormatToDepth[r->pixelFormat]/8;
		int pixSize = NativeDisplayTester::PixelFormatToDepth( r->pixelFormat ) / 8;
		FLASHASSERT(pixSize > 0);

		xmin += r->xorg;
		xmax += r->xorg;

		while ( xmin < xmax ) {
			// Expand the pixels to RGB values
			int n = Min((int)(xmax-xmin), RGBSlabChunkSize);
			CalcLimit(n, pt.x, color->bm.bi.dx, wLimit);	// limit the slab size so we don't overwrap textures
			CalcLimit(n, pt.y, color->bm.bi.dy, hLimit);
			color->bm.fastBltProc(color->bm.bi, pt, n, r->rowAddr + xmin*pixSize);
			xmin += n;
		}
	} else {
		// Draw the slab for the general case
		while ( xmin < xmax ) {
			RGBI pixBuf[RGBSlabChunkSize];

			// Expand the pixels to RGB values
			int n = Min((int)(xmax-xmin), RGBSlabChunkSize);
			CalcLimit(n, pt.x, color->bm.bi.dx, wLimit);	// limit the slab size so we don't overwrap textures
			CalcLimit(n, pt.y, color->bm.bi.dy, hLimit);

			color->bm.bltProc(color->bm.bi, pt, n, pixBuf);

			// Apply a color transform
			if ( color->bm.colorMap )
				ApplyColorMap(color->bm.colorMap, pixBuf, n);

			// Apply a highlight pattern...
			if ( color->pattern )
				ApplyRGBPattern(color->pattern, r->bitY, xmin, xmin+n, pixBuf);

			// Draw the RGB values
			r->drawRGBSlab(r, xmin, xmin+n, pixBuf);
			xmin += n;
		}
	}
}

void BuildBitmapSlab(RColor* color, S32 xmin, S32 xmax, RGBI* buf)
{
	// Calc the start point in bitmap src coordinates
	SPOINT pt;
	pt.x = xmin << 16;
	pt.y = color->raster->bitY << 16;
	MatrixTransformPoint(&color->bm.invMat, &pt, &pt);

	// Set up the slab start point
	S32 wLimit = (S32)color->bm.bi.width<<16;
	S32 hLimit = (S32)color->bm.bi.height<<16;
	if ( color->bm.bitsStyle & fillBitsClip ) {
		// Treat the edges of a clipped bitmap special to prevent rounding problems
		// Check for out of range values
		int x,y;

		SBitmapCore* bitmap = color->bm.bitmap;
		int w = bitmap->width;
		int h = bitmap->height;
		if ( color->bm.smooth ) {
			h--; w--;		// allow for the extra row and column when getting 4 samples
		}

		SPOINT tail;
		S32 count = xmax-xmin;
		tail.x = pt.x + color->bm.bi.dx*count;
		tail.y = pt.y + color->bm.bi.dy*count;

		// Adjust the head
		for (;;) {
			x = (int)(pt.x>>16);
			y = (int)(pt.y>>16);
			if ( x >= 0 && y >= 0 && x < w && y < h ) break;
			if ( xmin >= xmax ) break;
			#ifdef SMOOTHBITS
			if ( color->bm.smooth )
				bitmap->GetSSRGBPixel(pt.x, pt.y, buf);
			else
			#endif
				bitmap->GetRGBPixel(x, y, buf);

			if ( color->bm.colorMap )
				ApplyColorMap(color->bm.colorMap, buf);

			pt.x += color->bm.bi.dx;
			pt.y += color->bm.bi.dy;
			xmin++;
			buf++;
		}

		// Adjust the tail
		for (;;) {
			x = (int)(tail.x>>16);
			y = (int)(tail.y>>16);
			if ( x >= 0 && y >= 0 && x < w && y < h ) break;
			if ( xmin >= xmax ) break;
			RGBI* pix = buf+xmax-xmin-1;
			#ifdef SMOOTHBITS
			if ( color->bm.smooth )
				bitmap->GetSSRGBPixel(tail.x, tail.y, pix);
			else
			#endif
				bitmap->GetRGBPixel(x, y, pix);
			xmax--;
			if ( color->bm.colorMap )
				ApplyColorMap(color->bm.colorMap, pix);

			tail.x -= color->bm.bi.dx;
			tail.y -= color->bm.bi.dy;
		}
	} else {
		// Do an initial coordinate wrap for textures
		pt.x = LimitAbs(pt.x, wLimit);
		pt.y = LimitAbs(pt.y, hLimit);
	}

	// Draw the slab for the general case
	while ( xmin < xmax ) {
		// Expand the pixels to RGB values
		//int n = Min((int)(xmax-xmin), RGBSlabChunkSize);
		int n = (int)(xmax-xmin);
		CalcLimit(n, pt.x, color->bm.bi.dx, wLimit);	// limit the slab size so we don't overwrap textures
		CalcLimit(n, pt.y, color->bm.bi.dy, hLimit);

		color->bm.bltProc(color->bm.bi, pt, n, buf);

		// Apply a color transform
		if ( color->bm.colorMap )
			ApplyColorMap(color->bm.colorMap, buf, n);

		xmin += n;
		buf += n;
	}
}

void CalcBitmapPixel(RColor* color, S32 x, RGBI* buf)
{
	// Calc the start point in bitmap src coordinates
	SPOINT pt;
	pt.x = x << 16;
	pt.y = color->raster->bitY << 16;
	MatrixTransformPoint(&color->bm.invMat, &pt, &pt);

	// Set up the slab start point
	if ( color->bm.bitsStyle & fillBitsClip ) {
		// Let GetRGBPixel handle the edge conditions
		#ifdef SMOOTHBITS
		if ( color->bm.smooth )
			color->bm.bitmap->GetSSRGBPixel(pt.x, pt.y, buf);
		else
		#endif
			color->bm.bitmap->GetRGBPixel(pt.x>>16, pt.y>>16, buf);
	} else {
		// Do an initial coordinate wrap for textures
		pt.x = LimitAbs(pt.x, (S32)color->bm.bi.width<<16);
		pt.y = LimitAbs(pt.y, (S32)color->bm.bi.height<<16);

		color->bm.bltProc(color->bm.bi, pt, 1, buf);
	}

	// Apply a color transform
	if ( color->bm.colorMap )
		ApplyColorMap(color->bm.colorMap, buf);
}

void CalcSolidPixel(RColor* color, S32 x, RGBI* pix)
{
	*pix = color->rgb;
}

void BuildSolidSlab(RColor* color, S32 xmin, S32 xmax, RGBI* pix)
{
	int n = xmax-xmin;
	while ( n-- > 0 ) {
		*pix++ = color->rgb;
	}
}

void CalcGradientPixel(RColor* color, S32 x, RGBI* pix)
{
	SPOINT pt;
	pt.x = x<<8;
	pt.y = color->raster->bitY<<8;
	MatrixTransformPoint(&color->grad.invMat, &pt, &pt);

	RGB8* ramp = color->grad.colorRamp;
	if ( !ramp ) return;

	S32 intensity;
	if ( color->grad.gradStyle == fillRadialGradient ) {
		// Radial gradient
		intensity = PointLength(&pt) >> (6+8);
		if ( intensity > 256 ) 
			intensity = 256;
		else if ( intensity < 0 )  // this can happen in overflow conditions
			intensity = 0;
	} else {
		// Linear gradient
		intensity = (pt.x >> (6+8+1)) + 128;
		if ( intensity > 256 ) 
			intensity = 256;
		else if ( intensity < 0 )
			intensity = 0;
	}

	ExpandColor(ramp[intensity], pix);
}

void BuildLinearGradientSlab(RColor* color, S32 xmin, S32 xmax, RGBI* pix)
{
	RGB8* ramp = color->grad.colorRamp;
	if ( !ramp ) return;

	SPOINT pt;
	pt.x = xmin<<8;
	pt.y = color->raster->bitY<<8;
	MatrixTransformPoint(&color->grad.invMat, &pt, &pt);
	
	// Linear gradient
	SFIXED dx = color->grad.invMat.a>>8;
	S32		n = xmax-xmin;

	while ( n-- ) {
		S32 intensity = (pt.x >> (6+8+1)) + 128;
		if ( intensity > 256 ) 
			intensity = 256;
		else if ( intensity < 0 )
			intensity = 0;

		// Convert from RGB8 to RGBI
		//
		pix->alpha = ramp[intensity].alpha;
		pix->red   = ramp[intensity].red;
		pix->green = ramp[intensity].green;
		pix->blue  = ramp[intensity].blue;

		// Advance
		pt.x += dx;
		pix++;
	}
}

void BuildRadialGradientSlab(RColor* color, S32 xmin, S32 xmax, RGBI* pix)
{
	S32 n = xmax-xmin;

	SPOINT pt;
	pt.x = xmin<<8;
	pt.y = color->raster->bitY<<8;
	MatrixTransformPoint(&color->grad.invMat, &pt, &pt);

	RGB8* ramp = color->grad.colorRamp;
	if ( !ramp ) return;

	SFIXED dx = color->grad.invMat.a>>8;
	SFIXED dy = color->grad.invMat.b>>8;
	
	// Draw the slab
	// Expand the pixels to RGB values
	// Radial gradient
	// i = sqrt(x^2 + y^2)
	S32 t, t2, i2;
	S32 i, previ;
	U16* sq;
	S32 step = 0;
	i = PointLength(&pt) >> (6+8);	// get a good start
	if ( i > 256 ) 
		i = 256;

	while ( n-- ) {
		t = pt.x>>(22-8);	// get the coords into the -256..256 range
		i2 = t*t;
		t2 = pt.y>>(22-8);
		i2 += t2*t2;

		// Find the square root of i2
		// Increment by the same number of entries as the previous step
		// I think this search is almost guarenteed to find a pixel in less than two cycles
		//	it averages about 1.5 searches per pixel
		previ = i;
		i += step;
		if ( i < 0 ) i = 0;
		else if ( i > 256 ) i = 256;
		sq = Sq256+i;
		for (;;) {
			if ( (U32)i2 < sq[0] ) {// Cast to unsigned to handle overflows more gracefully
				// Move down an entry
				i--;
				sq--;
			} else if ( (U32)i2 > sq[1] ) {
				// Move up an entry
				if ( i >= 256 ) 
					break;
				i++;
				sq++;
			} else {
				// We are bracketed
				break;
			}
		}
		step = i-previ;
		ExpandColor(ramp[i], pix);
		//i = PointLength(&pt) >> (6+8);
		//*pix = ramp[i < 256 ? i : 256];

		// Advance
		pt.x += dx;
		pt.y += dy;
		pix++;
	}		
}


void BuildGradientSlab(RColor* color, S32 xmin, S32 xmax, RGBI* pix)
{

	if(color->grad.gradStyle == fillRadialGradient) {
		BuildRadialGradientSlab(color, xmin, xmax, pix);
	} else {
		BuildLinearGradientSlab(color, xmin, xmax, pix);
	}

}

void DrawGradientSlab(RColor* color, S32 xmin, S32 xmax)
{
	CRaster* r = color->raster;

	// Draw the slab
	while ( xmin < xmax ) {
		RGBI pixBuf[RGBSlabChunkSize];

		S32 limit = Min(xmax, xmin+RGBSlabChunkSize);

// #ifdef USE_KATMAI
// 		if(gUseKatmai)
// 			BuildGradientSlab_Katmai(color, xmin, limit, pixBuf);
// 		else
// #endif
		BuildGradientSlab(color, xmin, limit, pixBuf);

		// Apply a highlight pattern...
		if ( color->pattern )
			ApplyRGBPattern(color->pattern, r->bitY, xmin, limit, pixBuf);

		r->drawRGBSlab(r, xmin, limit, pixBuf);
		xmin = limit;
	}
}


//
// Composite Draw Procs
//

void CompositeSolidSlab(RColor* color, S32 xmin, S32 xmax, RGBI* buf)
{
	FLASHASSERT(xmax-xmin<=RGBSlabChunkSize);
	CompositeRGBSolid(&color->rgb, buf, xmax-xmin);
}

void CompositeGradientSlab(RColor* color, S32 xmin, S32 xmax, RGBI* buf)
{
	FLASHASSERT(xmax-xmin<=RGBSlabChunkSize);
	RGBI pixBuf[RGBSlabChunkSize];

	BuildGradientSlab(color, xmin, xmax, pixBuf);
	CompositeRGB(pixBuf, buf, xmax-xmin);
}

void CompositeBitmapSlab(RColor* color, S32 xmin, S32 xmax, RGBI* buf)
{
	FLASHASSERT(xmax-xmin<=RGBSlabChunkSize);
	RGBI pixBuf[RGBSlabChunkSize];

	BuildBitmapSlab(color, xmin, xmax, pixBuf);
	CompositeRGB(pixBuf, buf, xmax-xmin);
}


// DrawSlab helpers
inline void DrawSolidSlab(U32 * bits, U32 pix, S32 n)
{
	// This seems to optimize very well w/ intel processor
	// For other processors, it may be worth trying other approaches.
	if ( n ) {
	 	do {
			*bits++ = pix;
		} while (--n);
	}

}

// Adjust for plaform specific byte order
#ifdef BIG_ENDIAN
// Mac 68K, Mac PPC & Sparc CPUs
#define OrderPixels(x) (x)
#else
// Intel CPUs
#define OrderPixels(x) (((x&0xFFL)<<24) | ((x&0xFF00L)<<8) | ((x&0xFF0000L)>>8) | ((x&0xFF000000L)>>24))
#endif

const U32 pixMaskTab[32] = {
	OrderPixels(0xFFFFFFFF),//1
	OrderPixels(0x7FFFFFFF),
	OrderPixels(0x3FFFFFFF),
	OrderPixels(0x1FFFFFFF),

	OrderPixels(0x0FFFFFFF),//2
	OrderPixels(0x07FFFFFF),
	OrderPixels(0x03FFFFFF),
	OrderPixels(0x01FFFFFF),

	OrderPixels(0x00FFFFFF),//3
	OrderPixels(0x007FFFFF),
	OrderPixels(0x003FFFFF),
	OrderPixels(0x001FFFFF),

	OrderPixels(0x000FFFFF),//4
	OrderPixels(0x0007FFFF),
	OrderPixels(0x0003FFFF),
	OrderPixels(0x0001FFFF),

	OrderPixels(0x0000FFFF),//5
	OrderPixels(0x00007FFF),
	OrderPixels(0x00003FFF),
	OrderPixels(0x00001FFF),

	OrderPixels(0x00000FFF),//6
	OrderPixels(0x000007FF),
	OrderPixels(0x000003FF),
	OrderPixels(0x000001FF),

	OrderPixels(0x000000FF),//7
	OrderPixels(0x0000007F),
	OrderPixels(0x0000003F),
	OrderPixels(0x0000001F),

	OrderPixels(0x0000000F),//8
	OrderPixels(0x00000007),
	OrderPixels(0x00000003),
	OrderPixels(0x00000001)
};

static void DrawSubSlab(U32 pix, U32 * rowAddr, S32 minWord, S32 minFrac, S32 maxWord, S32 maxFrac)
// minWord - word to start on
// minFrac - number of bits to skip over in rowAddr[minWord]
// maxWord - the last word to change
// maxFrac - the number of bits to set in rowAddr[minWord]
{
	U32 * bits = rowAddr + minWord;
	S32 n = maxWord - minWord;
	if ( !n ) {
		// Change a single word
		U32 mask = pixMaskTab[minFrac] & ~pixMaskTab[maxFrac];
		*bits = (*bits & ~mask) | (pix & mask);
	} else {
		// Handle multiple words

		// Handle the leading fraction
		if ( minFrac ) {
			U32 mask = pixMaskTab[minFrac];
			*bits = (*bits & ~mask) | (pix & mask);
			bits++;
			n--;
		}

		// Handle the middle
		DrawSolidSlab(bits, pix, n);

		// Handle the trailing fraction
		if ( maxFrac ) {
			U32 mask = ~pixMaskTab[maxFrac];
			bits = (U32 *)rowAddr + maxWord;
			*bits = (*bits & ~mask) | (pix & mask);
		}
	}
}

void DrawSolidSlab1(RColor* color, S32 xmin, S32 xmax)
{
	CRaster* r = color->raster;
	xmin += r->xorg;
	xmax += r->xorg;

	if ( xmin >= xmax ) return;
	DrawSubSlab(color->pat[(r->bitY+r->patAlign.y & 3)*4],
				(U32 *)r->rowAddr, xmin>>5, xmin & 0x1F, xmax>>5, xmax & 0x1F);
}

void DrawSolidSlab2(RColor* color, S32 xmin, S32 xmax)
{
	CRaster* r = color->raster;
	xmin += r->xorg;
	xmax += r->xorg;

	if ( xmin >= xmax ) return;
	DrawSubSlab(color->pat[(r->bitY+r->patAlign.y & 3)*4],
				(U32 *)r->rowAddr, xmin>>4, (xmin & 0xF)<<1, xmax>>4, (xmax & 0xF)<<1);
}

void DrawSolidSlab4(RColor* color, S32 xmin, S32 xmax)
{
	CRaster* r = color->raster;
	xmin += r->xorg;
	xmax += r->xorg;

	if ( xmin >= xmax ) return;
	DrawSubSlab(color->pat[(r->bitY+r->patAlign.y & 3)*4],
				(U32 *)r->rowAddr, xmin>>3, (xmin & 0x7)<<2, xmax>>3, (xmax & 0x7)<<2);
}

void DrawSolidSlab8(RColor* color, S32 xmin, S32 xmax)
{
	CRaster* r = color->raster;
	xmin += r->xorg;
	xmax += r->xorg;

	if ( xmin >= xmax ) return;
	DrawSubSlab(color->pat[(r->bitY+r->patAlign.y & 3)*4],
				(U32 *)r->rowAddr, xmin>>2, (xmin & 0x3)<<3, xmax>>2, (xmax & 0x3)<<3);
}

void DrawSolidSlab16(RColor* color, S32 xmin, S32 xmax)
{
	CRaster* r = color->raster;
	xmin += r->xorg;
	xmax += r->xorg;

	S32 n = xmax-xmin;
	if ( n <= 0 ) return;
	U32* pat = color->pat + ((r->bitY+r->patAlign.y & 3)*4);	// calculate the pattern row...
	U16 * bits = (U16 *)r->rowAddr + xmin;
	if ( color->pattern == solidPat ) {
		// If we know we have a solid pattern, unroll the loop
	 	U32 pix = *pat;

		// Handle a leading fraction
		if ( xmin & 1 ) {
			*bits++ = (U16)pix;
			n--;
		}

		DrawSolidSlab((U32*)bits, pix, n>>1);

		// Handle the trailing fraction
		if ( xmax & 1 ) {
			((U16 *)r->rowAddr)[xmax-1] = (U16)pix;
		}
	} else {
		// Handle a pattern
		U16 *p = (U16*)pat + (xmin & 0x3);
		U16 *patEnd = (U16*)pat + 4;

		S32 i = xmax-xmin;
		while ( i-- ) {
			*bits++ = *p++;
			if ( p == patEnd ) p = (U16*)pat;
		}
	}
}

void DrawSolidSlab24(RColor* color, S32 xmin, S32 xmax)
{
	CRaster* r = color->raster;
	xmin += r->xorg;
	xmax += r->xorg;

	S32 n = xmax-xmin;
	if ( n < 0 ) return;
	U32 *pat = color->pat + ((r->bitY+r->patAlign.y & 3)*4);	// calculate the pattern row...
	U8  *bits = (U8 *)r->rowAddr + 3*xmin;
	/*if ( color->pattern == solidPat ) {
		// If we know we have a solid pattern, ignore the pattern
		U8* p = (U8*)pat;
		while ( n-- ) {
			bits[0] = p[0];
			bits[1] = p[1];
			bits[2] = p[2];
			bits+=3;
		}

	} else*/ {
		// Handle the pattern properly
		U8* p = (U8*)pat + 3*(xmin & 0x3);
		U8 *patEnd = (U8*)pat + 3*4;

		while ( n-- ) {
			bits[0] = p[0];
			bits[1] = p[1];
			bits[2] = p[2];
			bits+=3;
			p+=3;
			if ( p == patEnd ) p = (U8*)pat;
		}
	}
}

void DrawSolidSlab32(RColor* color, S32 xmin, S32 xmax)
{
	CRaster* r = color->raster;
	xmin += r->xorg;
	xmax += r->xorg;

	S32 n = xmax-xmin;
	if ( n < 0 ) return;
	U32 *pat = color->pat + ((r->bitY+r->patAlign.y & 3)*4);	// calculate the pattern row...
	U32  *bits = (U32 *)r->rowAddr + xmin;
	if ( color->pattern == solidPat ) {
		// If we know we have a solid pattern, unroll the loop
		DrawSolidSlab(bits, *pat, n);

	} else {
		// Handle the pattern properly
		U32 *p = (U32*)pat + (xmin & 0x3);
		U32 *patEnd = (U32*)pat + 4;

		while ( n-- ) {
			*bits++ = *p++;
			if ( p == patEnd ) p = (U32*)pat;
		}
	}
}

void DrawDitherSlab(RColor* color, S32 xmin, S32 xmax)
{
	CRaster* r = color->raster;

	// Draw the slab
	while ( xmin < xmax ) {
		RGBI pixBuf[RGBSlabChunkSize];

		// Expand the pixels to RGB values
		S32 limit = Min(xmax, xmin+RGBSlabChunkSize);
		RGBI* pix = pixBuf;
		for ( S32 j = limit-xmin; j--; pix++ ) {
			*pix = color->rgb;
		}

		// Apply a highlight pattern...
		if ( color->pattern )
			ApplyRGBPattern(color->pattern, r->bitY, xmin, limit, pixBuf);

		r->drawRGBSlab(r, xmin, limit, pixBuf);
		xmin = limit;
	}
}

#ifdef ENABLE_MASK
void CRaster::SetMask(int xmin, int xmax)
{
	if ( xmin >= xmax ) return;
	DrawSubSlab(0xffffffff, (U32 *)maskRowAddr, xmin>>5, xmin & 0x1F, xmax>>5, xmax & 0x1F);
}
#endif


/*
//
// Blend Pixels
//

static void BlendRGB(RGBI* c1, RGBI* c2, int coverage)
// Note that this assumes that c1 is already scaled by coverage
{
	coverage = 16 - coverage;
	#ifdef WIN16
	c2->red   = (c1->red   + c2->red  *coverage) >> 4;
	c2->green = (c1->green + c2->green*coverage) >> 4;
	c2->blue  = (c1->blue  + c2->blue *coverage) >> 4;
	#else
	((RGBIL*)c2)->c0 = ((((RGBIL*)&c1)->c0 + ((RGBIL*)c2)->c0*coverage)>>4) & 0x00FF00FF;
	((RGBIL*)c2)->c1 = ((((RGBIL*)&c1)->c1 + ((RGBIL*)c2)->c1*coverage)>>4) & 0x00FF00FF;
	#endif
}

static void Blend1RGB(RGBI* c1, RGBI* c2, int coverage)
// Note that this assumes that c1 is already scaled by coverage
// This is used when we can assume a white background
{
	int white = (16-coverage)*255;
	c2->red   = (c1->red   + white) >> 4;
	c2->green = (c1->green + white) >> 4;
	c2->blue  = (c1->blue  + white) >> 4;
}


//
// Draw Pixel Procs
//

void CRaster::DrawSubPixel(RGBI* color, int coverage, U8 * bits, int mask, int shift)
{
	RGBI back;
	mask <<= shift;
	if ( getBackground && coverage < 16 ) {
		// Calculate a blended pixel value
		//getBackProc(getBackData, x, bitY, &back);
		IndexToRGB((*bits & mask) >> shift, &back);
		BlendRGB(color, &back, coverage);
	} else {
		// Get the precalculated pixel value
		Blend1RGB(color, &back, coverage);
	}
	*bits = (*bits & ~mask) | ((U8)RGBToIndex(&back) << shift);
}

void DrawPixel1(CRaster* r, S32 x, RGBI* color, int coverage)
{
	x += r->xorg;
	r->DrawSubPixel(color, coverage, (U8 *)r->rowAddr + (x>>3), 0x01, 7 - ((int)x & 0x07));
}

void DrawPixel2(CRaster* r, S32 x, RGBI* color, int coverage)
{
	x += r->xorg;
	r->DrawSubPixel(color, coverage, (U8 *)r->rowAddr + (x>>2), 0x03, (3 -((int)x & 0x03))<<1);
}

void DrawPixel4(CRaster* r, S32 x, RGBI* color, int coverage)
{
	x += r->xorg;
	r->DrawSubPixel(color, coverage, (U8 *)r->rowAddr + (x>>1), 0x0F, (1 - ((int)x & 0x01))<<2);
}

void DrawPixel8(CRaster* r, S32 x, RGBI* color, int coverage)
{
	RGBI back;
	U8  *bits = (U8 *)r->rowAddr + x + r->xorg;

	if ( r->getBackground && coverage < 16 ) {
		// Calculate a blended pixel value
		#ifdef ENABLE_MASK
		if ( r->getBackProc ) {
			r->getBackProc(r->getBackData, x, r->bitY, &back);
		} else
		#endif
			r->IndexToRGB(*bits, &back);
		BlendRGB(color, &back, coverage);
	} else {
		// Get the precalculated pixel value
		Blend1RGB(color, &back, coverage);
	}
	*bits = (U8)r->RGBToIndex(&back);
}

void DrawPixel16(CRaster* r, S32 x, RGBI* color, int coverage)
{
	U16  *bits = (U16 *)r->rowAddr + x + r->xorg;
	RGBI back;

	if ( r->getBackground && coverage < 16 ) {
		// Calculate a blended pixel value
		#ifdef ENABLE_MASK
		if ( r->getBackProc ) {
			r->getBackProc(r->getBackData, x, r->bitY, &back);
		} else
		#endif
			UnpackPix16(*bits, &back);
		BlendRGB(color, &back, coverage);
	} else {
		// Get the precalculated pixel value
		Blend1RGB(color, &back, coverage);
	}
	*bits = PackPix16(&back);
}

void DrawPixel16A(CRaster* r, S32 x, RGBI* color, int coverage)
{
	U16  *bits = (U16 *)r->rowAddr + x + r->xorg;
	RGBI back;

	if ( r->getBackground && coverage < 16 ) {
		// Calculate a blended pixel value
		#ifdef ENABLE_MASK
		if ( r->getBackProc ) {
			r->getBackProc(r->getBackData, x, r->bitY, &back);
		} else
		#endif
			UnpackPix16A(*bits, &back);
		BlendRGB(color, &back, coverage);
	} else {
		// Get the precalculated pixel value
		Blend1RGB(color, &back, coverage);
	}
	*bits = PackPix16A(&back);
}

void DrawPixel24(CRaster* r, S32 x, RGBI* color, int coverage)
{
	U8  *bits = (U8 *)r->rowAddr + 3*(x+r->xorg);
	RGBI back;

	if ( r->getBackground && coverage < 16 ) {
		// Calculate a blended pixel value
		#ifdef ENABLE_MASK
		if ( r->getBackProc ) {
			r->getBackProc(r->getBackData, x, r->bitY, &back);
		} else 
		#endif
		{
			back.red   = bits[2];
			back.green = bits[1];
			back.blue  = bits[0];
		}
		BlendRGB(color, &back, coverage);
	} else {
		// Get the precalculated pixel value
		Blend1RGB(color, &back, coverage);
	}
	bits[2] = (U8)back.red;
	bits[1] = (U8)back.green;
	bits[0] = (U8)back.blue;
}

void DrawPixel32(CRaster* r, S32 x, RGBI* color, int coverage)
{
	U32  *bits = (U32 *)r->rowAddr + x + r->xorg;
	RGBI back;

	if ( r->getBackground && coverage < 16 ) {
		// Calculate a blended pixel value
		#ifdef ENABLE_MASK
		if ( r->getBackProc ) {
			r->getBackProc(r->getBackData, x, r->bitY, &back);
		} else
		#endif
			UnpackPix32(*bits, &back);
		BlendRGB(color, &back, coverage);
	} else {
		// Get the precalculated pixel value
		Blend1RGB(color, &back, coverage);
	}
	*bits = PackPix32(&back);
}

void DrawPixel32A(CRaster* r, S32 x, RGBI* color, int coverage)
{
	U32  *bits = (U32 *)r->rowAddr + x + r->xorg;
	RGBI back;
	int alpha;

	if ( r->getBackground && coverage < 16 ) {
		// Calculate a blended pixel value
		#ifdef ENABLE_MASK
		if ( r->getBackProc ) {
			r->getBackProc(r->getBackData, x, r->bitY, &back);
		} else
		#endif
			UnpackPix32A(*bits, &back, &alpha);
		BlendRGB(color, &back, coverage);
		alpha = (coverage*0xFF + alpha*(16-coverage)) >> 4;
	} else {
		// Get the precalculated pixel value
		Blend1RGB(color, &back, coverage);
		alpha = (coverage*0xFF) >> 4;
	}
	*bits = PackPix32A(&back, alpha);
}
*/


//
// The Color Transform
//

void ColorTransform::Clear()
{
	flags = 0;
	aa = ra = ga = ba = 256;
	ab = rb = gb = bb = 0;
}

int ApplyChannel(int x, int a, int b)
{
#ifdef WIN16
	x = ((x*(a>>1))>>7)+b;     // avoid overflow
#else
	x = ((x*a)>>8)+b;
#endif
	if ( (x & 0xFF00) == 0 )
		return x;
	else if ( x > 0 ) 
		return 255;
	else
		return 0;
}

int ApplyChannelB(int x, int a, int b)
{
	x = x+b;
	if ( (x & 0xFF00) == 0 )
		return x;
	else if ( x > 0 ) 
		return 255;
	else
		return 0;
}

void ColorTransform::Apply(RGBI* c, int n)
{
	if ( flags == 0 ) return;
	if ( flags & needA ) {
		while ( n-- ) {
			c->alpha = ApplyChannel(c->alpha, aa, ab);
			c->red   = ApplyChannel(c->red, ra, rb);
			c->green = ApplyChannel(c->green, ga, gb);
			c->blue  = ApplyChannel(c->blue, ba, bb);
			c++;
		}
	} else {
		// We don't need the multiply
		while ( n-- ) {
			c->alpha = ApplyChannelB(c->alpha, aa, ab);
			c->red   = ApplyChannelB(c->red, ra, rb);
			c->green = ApplyChannelB(c->green, ga, gb);
			c->blue  = ApplyChannelB(c->blue, ba, bb);
			c++;
		}
	}
}

void ColorTransform::Apply(GradColorRamp* ramp)
{
	if ( flags == 0 ) return;
	FLASHASSERT(ramp->nColors<=fillMaxGradientColors);
	SRGB* c = ramp->color;
	for ( int i = ramp->nColors; i--; c++ ) {
		c->rgb.transparency = (U8)ApplyChannel(c->rgb.transparency, aa, ab);
		c->rgb.red   = (U8)ApplyChannel(c->rgb.red  , ra, rb);
		c->rgb.green = (U8)ApplyChannel(c->rgb.green, ga, gb);
		c->rgb.blue  = (U8)ApplyChannel(c->rgb.blue , ba, bb);
	}
}

void ColorTransform::CalcFlags()
{
	flags = 0;
	if ( aa != 256 || ra != 256 || ga != 256 || ba != 256 ) 
		flags |= needA;
	if ( ab || rb || gb || bb ) 
		flags |= needB;
}

void ColorTransform::Concat(ColorTransform* cx)
{
	if ( cx->flags == 0 ) return;
	if ( flags == 0 ) {
		*this = *cx;
		return;
	}

	/*
		x' = a*x + b
		x'' = a1*x' + b1
		x'' = a1*(a0*x + b0) + b1 = (a1*a0)*x + (a1*b0+b1)
	*/

	ab += (S16)(((S32)aa*cx->ab)>>8);// alpha
	aa  = (S16)(((S32)aa*cx->aa)>>8);

	rb += (S16)(((S32)ra*cx->rb)>>8);// red
	ra  = (S16)(((S32)ra*cx->ra)>>8);

	gb += (S16)(((S32)ga*cx->gb)>>8);// green
	ga  = (S16)(((S32)ga*cx->ga)>>8);

	bb += (S16)(((S32)ba*cx->bb)>>8);// blue
	ba  = (S16)(((S32)ba*cx->ba)>>8);

	CalcFlags();
}

SRGB ColorTransform::Apply(SRGB c)
{
	if ( flags == 0 ) return c;
	SRGB res;
	res.rgb.transparency = (U8)ApplyChannel(c.rgb.transparency, aa, ab);
	res.rgb.red   = (U8)ApplyChannel(c.rgb.red  , ra, rb);
	res.rgb.green = (U8)ApplyChannel(c.rgb.green, ga, gb);
	res.rgb.blue  = (U8)ApplyChannel(c.rgb.blue , ba, bb);
	return res;
}

// #ifndef PLAYER
// RGB8 ColorTransform::Apply(RGB8 c)
// {
// 	if ( flags == 0 ) return c;
// 	RGB8 res;
// 	res.alpha = (U8)ApplyChannel(c.alpha, aa, ab);
// 	res.red   = (U8)ApplyChannel(c.red, ra, rb);
// 	res.green = (U8)ApplyChannel(c.green, ga, gb);
// 	res.blue  = (U8)ApplyChannel(c.blue, ba, bb);
// 	return res;
// }
// 
// #endif

BOOL Equal(ColorTransform* a, ColorTransform* b)
{
	return	a->flags == b->flags &&
			a->aa == b->aa && a->ab == b->ab &&
			a->ra == b->ra && a->rb == b->rb &&
			a->ga == b->ga && a->gb == b->gb &&
			a->ba == b->ba && a->bb == b->bb;
}
