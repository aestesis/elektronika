/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright © Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 
****************************************************************************/

#ifndef SHAPE_INCLUDED
#define SHAPE_INCLUDED

#ifndef CURVE_INCLUDED
#include "curve.h"
#endif

typedef union 
{
	U32 all;
	struct {
		#if defined(BIG_ENDIAN)
		// Mac 68K, Mac PPC & Sparc CPUs
		U8	transparency,
			blue,
			green,
			red;
		#else
		// Intel CPUs
		U8	red,
			green,
			blue,
			transparency;
		#endif
	} rgb;
} SRGB, *P_SRGB;

#define  MakeSRGB(r, g, b)	 MakeU32(MakeU16(r,g),MakeU16(b,255))

#define SRGBBlack		(MakeSRGB(0, 0, 0))
#define SRGBLtGray		(MakeSRGB(85, 85, 85))
#define SRGBWhite		(MakeSRGB(255, 255, 255))


enum { 	// Fill Styles
		fillSolid 				= 0, 

		// Gradient fills
		fillGradient			= 0x10, // if this bit is set, must be a gradient fill
		fillLinearGradient 		= 0x10, 
		fillRadialGradient 		= 0x12, 

		fillMaxGradientColors 	= 8,

		// Vector Pattern fills
		fillVectorPattern		= 0x20,	// if this bit is set, must be a vector pattern
		fillRaggedCrossHatch 	= 0x20,
		fillDiagonalLines 		= 0x21,
		fillCrossHatchLines 	= 0x22,
		fillStipple 			= 0x23,

		// Texture/bitmap fills - note these are currently only used by our player
		fillBits				= 0x40,	// if this bit is set, must be a bitmap pattern
		fillBitsClip			= 0x01,	// flag that a bitmap should be clipped to its edges, otherwise a repeating texture
		fillBitsNoSmooth		= 0x02,	// set if a bitmap should never be smoothed

		// Fill bit Flags
		fillLooseClip			= 0x1,
		fillOverlapClip			= 0x2
	};

struct GradColorRamp {
	U8 nColors;
	U8 colorRatio[fillMaxGradientColors];
	SRGB color[fillMaxGradientColors];
};

#endif
