/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright © Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 
****************************************************************************/

#ifndef RASTER_INCLUDED
#define RASTER_INCLUDED
                   
#ifndef SHAPE_INCLUDED
#include "shape.h"
#endif
#ifndef BITBUF_INCLUDED
#include "bitbuf.h"
#endif
#ifndef SBITMAP_INCLUDED
#include "sbitmap.h"
#endif

#define SMOOTHBITS 1

class CBitBuffer;
class CRaster;
class SBitmapCore;

struct RActiveEdge;
struct RRun;
struct RColor;
struct RColorMap;
struct REdge;
struct BltInfo;

typedef void (*DrawSlabProc)(RColor* color, S32 xmin, S32 xmax);
typedef void (*CompositeSlabProc)(RColor* color, S32 xmin, S32 xmax, RGBI* buf);
typedef void (*CalcPixelProc)(RColor* color, S32 x, RGBI* buf);

typedef void (*GetBackgroundProc)(CRaster*, S32 xmin, S32 xmax, RGBI* pix);
typedef void (*DrawRGBSlabProc)(CRaster*, S32 xmin, S32 xmax, RGBI* pix);
typedef void (*DitherRGBSlabProc)(CRaster*, S32 xmin, S32 xmax, RGBI* pix, U8 * dst);

typedef void (*BltProc)(BltInfo bi, SPOINT& pt, int n, void* dst);

#ifdef ENABLE_MASK
typedef void (*GetBackProc)(void* data, int x, int y, RGBI* pix);
#endif

//
// The Scan Line Renderer
//

enum { solidPat = 0, disablePat = 1, highlightPat = 2, lineHighlightPat = 3 };
enum { fillEdgeRule = 0, fillEvenOddRule, fillWindingRule };
enum { noDither = 0, orderedDither = 1, errorDiffusionDither = 2 };

struct RGBErr;

class CRaster {
public:
	BOOL antialias;
	BOOL getBackground;		// set this if we might be painting on a non-white background...
	BOOL smoothBitmaps;		// turns on interpolation for scaled bitmaps
	int bitmapDither;		// dither type for bitmaps
	int solidDither;		// dither type for solid colors - note that if this is set to errorDiffusion dither, it simply uses the bitmapDither type

	BOOL needFlush;
	U32 layerDepth;
	
public:
	CBitBuffer* bits;
	SColorInfo* cinfo;
	SRECT edgeClip;	// The clip rect in higher res coords (for antialiasing)
	SRECT bitClip;	// The clip rect in bitmap coords
	SPOINT patAlign;

#ifdef ENABLE_MASK
public:
	void* getBackData;
	GetBackProc getBackProc;
	CBitBuffer* maskBits;

	char  * maskRowAddr;

	void SetMask(int xmin, int xmax);
#endif

private:
	// The active color list
	RColor *topColor;
	S32 topColorXleft;	// the left edge of the current slab being processed
	
	// Error diffusion context
	int ditherSize;
	U8* ditherData;
	int ditherErrSize;
	void* curDitherErr;
	void* nextDitherErr;

	// Context for Paint()
	RActiveEdge* firstActive;
	REdge** yindex;
	S32 yindexSize;
	S32 ylines;
	S32 y;
	
    // Antialiasling info
    RRun* firstRun;
    RRun* curRun;

private:
	// Fast Memory Allocators
	ChunkAlloc activeEdgeAlloc;
	ChunkAlloc runAlloc;
	ChunkAlloc rampAlloc;
	ChunkAlloc mapAlloc;

	RActiveEdge* CreateActiveEdge() { return (RActiveEdge*)(activeEdgeAlloc.Alloc()); }
	void FreeActiveEdge(RActiveEdge* a) { activeEdgeAlloc.Free(a); }
		
	RRun* CreateRun() { return (RRun*)(runAlloc.Alloc()); }
	void FreeRun(RRun* c) { runAlloc.Free(c); }

	RGB8* CreateRamp() { return (RGB8*)(rampAlloc.Alloc()); }
	void FreeRamp(RGB8* c) { rampAlloc.Free(c); }

	RColorMap* CreateMap() { return (RColorMap*)(mapAlloc.Alloc()); }
	void FreeMap(RColorMap* c) { mapAlloc.Free(c); }

 	friend struct RColor;
 	friend struct RRun;
 	friend struct RPixel;

	friend void DoEdgeEdgeRule(CRaster*, RActiveEdge*);
	friend void DoEdgeEvenOddRule(CRaster*, RActiveEdge*);
	friend void DoEdgeWindingRule(CRaster*, RActiveEdge*);

private:
	// The sub steps for Painting
	void AddActive();
	void SortActive();

	void PaintSlab(S32 xright);
	void ShowColor(RColor*, S32 x);
	void HideColor(RColor*, S32 x);

	void PaintActive();
	void PaintAARuns();

private:
	// Our local cached bitmap information
	BOOL inverted;
	char  * baseAddr;
	S32 rowBytes;		// should always be a multiple of 4 bytes
	int pixelFormat;
	S32 bitHeight;
	S32 xorg;			// x pixel alignment

	// Current line context
	char  * rowAddr;
	S32 bitY;

	// Draw procs
	DrawRGBSlabProc drawRGBSlab;
	DitherRGBSlabProc ditherRGBSlab;
	GetBackgroundProc doGetBackground;

public:
	friend void DrawSolidSlab1	(RColor*, S32 xmin, S32 xmax);
	friend void DrawSolidSlab2	(RColor*, S32 xmin, S32 xmax);
	friend void DrawSolidSlab4	(RColor*, S32 xmin, S32 xmax);
	friend void DrawSolidSlab8	(RColor*, S32 xmin, S32 xmax);
	friend void DrawSolidSlab16	(RColor*, S32 xmin, S32 xmax);
	friend void DrawSolidSlab24	(RColor*, S32 xmin, S32 xmax);
	friend void DrawSolidSlab32	(RColor*, S32 xmin, S32 xmax);
	friend void DrawDitherSlab	(RColor*, S32 xmin, S32 xmax);

	friend void BuildSolidSlab  (RColor*, S32 xmin, S32 xmax, RGBI* pixBuf);
	friend void BuildBitmapSlab (RColor*, S32 xmin, S32 xmax, RGBI* pixBuf);
	friend void BuildGradientSlab(RColor*, S32 xmin, S32 xmax, RGBI* pixBuf);
	friend void BuildLinearGradientSlab(RColor*, S32 xmin, S32 xmax, RGBI* pixBuf);
	friend void BuildRadialGradientSlab(RColor*, S32 xmin, S32 xmax, RGBI* pixBuf);

	friend void CalcSolidPixel   (RColor*, S32 x, RGBI* pixBuf);
	friend void CalcBitmapPixel  (RColor*, S32 x, RGBI* pixBuf);
	friend void CalcGradientPixel(RColor*, S32 x, RGBI* pixBuf);

	friend void DrawBitmapSlab	(RColor*, S32 xmin, S32 xmax);
	friend void DrawGradientSlab(RColor*, S32 xmin, S32 xmax);

	friend void DrawRGBSlab1(CRaster*, S32 xmin, S32 xmax, RGBI* pix);
	friend void DrawRGBSlab2(CRaster*, S32 xmin, S32 xmax, RGBI* pix);
	friend void DrawRGBSlab4(CRaster*, S32 xmin, S32 xmax, RGBI* pix);
	friend void DrawRGBSlab8(CRaster*, S32 xmin, S32 xmax, RGBI* pix);
	friend void DrawRGBSlab16(CRaster*, S32 xmin, S32 xmax, RGBI* pix);
	friend void DrawRGBSlab16A(CRaster*, S32 xmin, S32 xmax, RGBI* pix);
	friend void DrawRGBSlab24(CRaster*, S32 xmin, S32 xmax, RGBI* pix);
	friend void DrawRGBSlab32(CRaster*, S32 xmin, S32 xmax, RGBI* pix);
	friend void DrawRGBSlab32A(CRaster*, S32 xmin, S32 xmax, RGBI* pix);

	friend void DitherRGBSlabNone(CRaster*, S32 xmin, S32 xmax, RGBI* pix, U8 * dst);
	friend void DitherRGBSlabOrdered(CRaster*, S32 xmin, S32 xmax, RGBI* pix, U8 * dst);
	friend void DitherRGBSlabErrorDiffusion(CRaster*, S32 xmin, S32 xmax, RGBI* pix, U8 * dst);

	friend void CompositeSolidSlab(RColor*, S32 xmin, S32 xmax, RGBI* buf);
	friend void CompositeGradientSlab(RColor*, S32 xmin, S32 xmax, RGBI* buf);
	friend void CompositeBitmapSlab(RColor*, S32 xmin, S32 xmax, RGBI* buf);

	friend void GetBackground1(CRaster* r, S32 xmin, S32 xmax, RGBI* pix);
	friend void GetBackground2(CRaster* r, S32 xmin, S32 xmax, RGBI* pix);
	friend void GetBackground4(CRaster* r, S32 xmin, S32 xmax, RGBI* pix);
	friend void GetBackground8(CRaster* r, S32 xmin, S32 xmax, RGBI* pix);
	friend void GetBackground16(CRaster* r, S32 xmin, S32 xmax, RGBI* pix);
	friend void GetBackground16A(CRaster* r, S32 xmin, S32 xmax, RGBI* pix);
	friend void GetBackground24(CRaster* r, S32 xmin, S32 xmax, RGBI* pix);
	friend void GetBackground32(CRaster* r, S32 xmin, S32 xmax, RGBI* pix);
	friend void GetBackgroundWhite(CRaster* r, S32 xmin, S32 xmax, RGBI* pix);

	// Bitmap Drawing prototypes

	// General Blts
	#ifdef SMOOTHBITS
	friend void BltXtoIS(BltInfo bi, SPOINT& pt, int n, RGBI* pix);
	friend void Blt8toIS(BltInfo bi, SPOINT& pt, int n, RGBI* pix);
	friend void Blt16toIS(BltInfo bi, SPOINT& pt, int n, RGBI* pix);
	friend void Blt32toIS(BltInfo bi, SPOINT& pt, int n, RGBI* pix);

	friend void BltXtoISA(BltInfo bi, SPOINT& pt, int n, RGBI* pix);
	friend void Blt8toISA(BltInfo bi, SPOINT& pt, int n, RGBI* pix);
	friend void Blt16toISA(BltInfo bi, SPOINT& pt, int n, RGBI* pix);
	friend void Blt32toISA(BltInfo bi, SPOINT& pt, int n, RGBI* pix);

	#endif

	friend void BltXtoI(BltInfo bi, SPOINT& pt, int n, RGBI* pix);
	friend void Blt8toI(BltInfo bi, SPOINT& pt, int n, RGBI* pix);
	friend void Blt16toI(BltInfo bi, SPOINT& pt, int n, RGBI* pix);
	friend void Blt32toI(BltInfo bi, SPOINT& pt, int n, RGBI* pix);

	// Fast Blts
	friend void Blt8to8(BltInfo bi, SPOINT& pt, int n, U8 * dst);
	friend void Blt8to8D(BltInfo bi, SPOINT& pt, int n, U8 * dst);
	friend void Blt8to16(BltInfo bi, SPOINT& pt, int n, U16 * dst);
	friend void Blt8to16A(BltInfo bi, SPOINT& pt, int n, U16 * dst);
	friend void Blt8to32(BltInfo bi, SPOINT& pt, int n, U32 * dst);

	friend void Blt16to8(BltInfo bi, SPOINT& pt, int n, U8 * dst);
	friend void Blt16to8D(BltInfo bi, SPOINT& pt, int n, U8 * dst);
	friend void Blt16to16(BltInfo bi, SPOINT& pt, int n, U16 * dst);
	friend void Blt16to16A(BltInfo bi, SPOINT& pt, int n, U16 * dst);
	friend void Blt16to32(BltInfo bi, SPOINT& pt, int n, U32 * dst);

	friend void Blt32to8(BltInfo bi, SPOINT& pt, int n, U8 * dst);
	friend void Blt32to8D(BltInfo bi, SPOINT& pt, int n, U8 * dst);
	friend void Blt32to16(BltInfo bi, SPOINT& pt, int n, U16 * dst);
	friend void Blt32to16A(BltInfo bi, SPOINT& pt, int n, U16 * dst);
	friend void Blt32to32(BltInfo bi, SPOINT& pt, int n, U32 * dst);

private:
	void IndexToRGB(int index, RGBI* color);
	int RGBToIndex(RGBI* color);
	void DrawSubPixel(RGBI* color, int coverage, U8 * bits, int mask, int shift);

	void SetYCoord(S32 y);
	void DitherRGBSlab(S32 xmin, S32 xmax, RGBI* pix, U8 * dst);

	void DrawRGBAPixel(S32 x, RGBI* rgb);
	void DrawRGBASlab(S32 xleft, S32 xright, RGBI*);
	void CompositeSlab(S32 xleft, S32 xright, RColor** stack, int n);
	void GetBackground(S32 xleft, S32 xright, RGBI* pix);

public:
	CRaster();
	~CRaster();
	void FreeEmpties();		// free unused memory chunks

	void Attach(CBitBuffer*, SRECT* clip, BOOL doAntialias);
	BOOL BitsValid();
	void SetPatAlign(P_SPOINT pt) { patAlign = *pt; }
	void UseAlphaChannel();

	void FillBackground(SRGB); // fill a solid background with a transparent alpha value

	// Painting
	void BeginPaint();
	void AddEdges(REdge*, RColor* colors, RColor* clipColor=0);	// also adjust the colors for depth
	void AddEdges(REdge*);
	void PaintBits();
	void Flush();
};


//
// The Color Transform
//

class ColorTransform {
public:
	int flags;
	enum { 
		needA=0x1,	// set if we need the multiply terms
		needB=0x2	// set if we need the constant terms
	};
	S16 aa, ab;	// a is multiply factor, b is addition factor
	S16 ra, rb;
	S16 ga, gb;
	S16 ba, bb;

public:
	void Clear();
	BOOL HasTransform() { return flags != 0; }
	BOOL HasTransparency() { return flags != 0 && (aa < 256 || ab < 0); }	// return true if this can add transparency
	BOOL HasAlpha() { return flags != 0 && (aa != 256 || ab != 0); }	// return true if this has any alpha info

	void Apply(RGBI*, int n=1);
	void Apply(GradColorRamp*);

	void Concat(ColorTransform*);
	void CalcFlags();

	SRGB Apply(SRGB);
};

BOOL Equal(ColorTransform* a, ColorTransform* b);

//
// The Edge Structure
//
				
struct REdge {
	REdge *nextObj;		// The next edge belonging to this object
	REdge *nextActive;	// The next edge that becomes active on this scanline
	RColor *color1, *color2;
	S16 anchor1x;		// store the CURVE as 16 bit ints to save RAM
	S16 anchor1y;
	S16 controlx;
	S16 controly;
	S16 anchor2x;
	S16 anchor2y;
	int isLine;
	U8 fillRule;
	S8 dir;

	inline void Set(CURVE* c) {
		anchor1x = (S16)c->anchor1.x;
		anchor1y = (S16)c->anchor1.y;
		controlx = (S16)c->control.x;
		controly = (S16)c->control.y;
		anchor2x = (S16)c->anchor2.x;
		anchor2y = (S16)c->anchor2.y;
		isLine   = CurveFlatness(c) <= 1;
	}

	inline void Get(CURVE* c) {
		c->anchor1.x = anchor1x;
		c->anchor1.y = anchor1y;
		c->control.x = controlx;
		c->control.y = controly;
		c->anchor2.x = anchor2x;
		c->anchor2.y = anchor2y;
		c->isLine = isLine;
	}
};


//
// The Color Structure
//

struct BltInfo {
	RColor* color;
	SFIXED dx;		// delta for the next sample point (moving left one dest pixel)
	SFIXED dy;

	U8 * baseAddr;	// source bitmap description
	S32 rowBytes;	
	RGB8* colors;
	int width;
	int height;

	U8* itab;		// inverse table for destination pixels
};

struct RColorMap {
	U8 red[256];
	U8 green[256];
	U8 blue[256];
	U8 alpha[256];

	U8 hasAlpha;
};

struct RColorBMInfo {
	U16 bitsStyle;
	U8 smooth;
	SBitmapCore* bitmap;
	MATRIX savedMat;
	MATRIX invMat;

	ColorTransform cxform;
	RColorMap* colorMap;

	BltProc bltProc;
	BltProc fastBltProc;
	BltInfo bi;		// cached blting info
};

struct RColorGradInfo {
	int gradStyle;
  	MATRIX savedMat;
	MATRIX invMat;
	GradColorRamp ramp;
	RGB8* colorRamp;
};

enum {
		colorSolid, 	// a solid RGB color
		colorBitmap, 	// a bitmap 
		colorGradient, 	// a gradient ramp
		colorClip 	// a clip color
	};

struct RColor {
	CRaster* raster;		// The render object that owns this color
	RColor* nextActive;		// The next color in the active color list
	RColor* nextColor;		// The next entry in the color list
	U32 order;				// the stacking order
	RColor* clippedBy;		// A list of colors that clip (hide) this color

	U8 transparent;			// true if this color could have partial transparency
	S8 visible;				// 0 if not visible, != 0 if visible
	U8 colorType;			// solid, bitmap, gradient
	U8 cacheValid;

	DrawSlabProc drawSlabProc; // draw the bits direct to the device bits
	CompositeSlabProc compositeSlabProc; // draw or composite to an RGBI buffer

	RGBI rgb;				// the expanded color components
	int pattern;			// The pattern of the object
	U32 rgbPoint;			// the x,y location that rgb currently represents in device pixels, rgbPoint = (y<<16) | x
	
	union {
		U32 pat[16];		// for solid colors, the expanded pattern
		RColorBMInfo bm;	// for bitmaps, the source bitmap info
		RColorGradInfo grad;// for gradients, the gradient info
	};
	
	void SetUp(CRaster* r);

	void BuildCache();
	void FreeCache();
};


//
// Helper Functions
//

//
// These are helper for SBitmap::GetSSPixel()
//

// Tables for extracting pixels from bytes
extern const U8 shiftTab1[8];
extern const U8 shiftTab2[4];
extern const U8 shiftTab4[2];
extern const U8 shiftTab8[1];

							// bm1bit, bm2bit, bm4bit, bm8bit
extern const U8* shiftTabs[4];
extern const U8  rbShifts[4];
extern const U8  pxMasks[4];
extern const U8  stMasks[4];

struct CoverEntry {
  	int ce[4];
};
extern CoverEntry PixCoverage[8][8];

// JLH: Added for kraster.cpp
void CompositeRGB(RGBI* src, RGBI* dst, int n);

#endif
