/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright © Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 
****************************************************************************/
//	990325	mnk	"fixed1" -> "fixed_1"


#include <stddef.h>
#include <memory.h>
#include <math.h>

#include "splay.h"

#include "bitbuf.h"
#include "stags.h"
#include "sobject.h"
#include "splayer.h"
#include "morphinter.h"
#include "edittext.h"
#include "memcop.h"
#include NATIVE_SOUND 
#include NATIVE_UTIL

#include "zlib/zlib.h"

#ifdef USE_JPEG
#include "jpeg.h"
#endif

#include <stdio.h>

#include "win32/PlayerWnd.h"

//
// Simple Parser
//

int SParser::GetTag(S32 len)
{
	tagPos = pos;
	if ( len-pos < 2 ) 
		return -1;	// we need more data before we can process this tag

	tagCode = GetWord();
	S32 tagLen = tagCode & 0x3f;
	if ( tagLen == 0x3f ) {
		if ( len-pos < 4 ) {
			pos = tagPos;
			return -1;	// we need more data before we can process this tag
		}
		tagLen = GetDWord();
	}
	tagEnd = pos + tagLen;
	if ( tagEnd > len ) {
		pos = tagPos;
		return -1;	// we need more data before we can process this tag
	}
	tagCode = tagCode >> 6;
	return tagCode;
}

S32 SParser::GetData(void * data, S32 len)
// returns the actual number of bytes read could 
//	be less than len if we hit the end of a tag
{
 	S32 n = Min(len, tagEnd-pos);
// 	hmemcpy(data, script+pos, n);
	memcpy(data, script+pos, n);
	pos+=n;
	return n;
}

void SParser::GetColor(RGBI* color, BOOL alpha)
{
	color->red = GetByte();
	color->green = GetByte();
	color->blue = GetByte();
	if ( alpha ) {
		color->alpha = GetByte();
	} else {
		color->alpha = 255;
	}
}

SRGB SParser::GetColor(BOOL alpha)
{
	SRGB color;
	color.rgb.red = GetByte();
	color.rgb.green = GetByte();
	color.rgb.blue = GetByte();
	if ( alpha ) {
		color.rgb.transparency = GetByte();
	} else {
		color.rgb.transparency = 0xFF;
	}
	return color;
}

void SParser::GetRect(SRECT* r)
{
	InitBits();
	int nBits = (int)GetBits(5);
	r->xmin = GetSBits(nBits);
	r->xmax = GetSBits(nBits);
	r->ymin = GetSBits(nBits);
	r->ymax = GetSBits(nBits);
}

void SParser::GetMatrix(MATRIX* mat)
{
	InitBits();
	// Scale terms
	if ( GetBits(1) ) {
		int nBits = (int)GetBits(5);
		mat->a = GetSBits(nBits);
		mat->d = GetSBits(nBits);
	} else {
	 	mat->a = mat->d = fixed_1;
	}

	// Rotate/skew terms
	if ( GetBits(1) ) {
		int nBits = (int)GetBits(5);
		mat->b = GetSBits(nBits);
		mat->c = GetSBits(nBits);
	} else {
	 	mat->b = mat->c = 0;
	}

	{// Translate terms
		int nBits = (int)GetBits(5);
		mat->tx = GetSBits(nBits);
		mat->ty = GetSBits(nBits);
	}
}

void SParser::GetColorTransform(ColorTransform* cx, BOOL alpha)
{
	InitBits();
	cx->flags = (int)GetBits(2);
	int nBits = (int)GetBits(4);
	cx->aa = 256; cx->ab = 0;
	if ( cx->flags & ColorTransform::needA ) {
		cx->ra = (S16)GetSBits(nBits);
		cx->ga = (S16)GetSBits(nBits);
		cx->ba = (S16)GetSBits(nBits);
		if ( alpha )
			cx->aa = (S16)GetSBits(nBits);
	} else {
		cx->ra = cx->ga = cx->ba = 256;
	}
	if ( cx->flags & ColorTransform::needB ) {
		cx->rb = (S16)GetSBits(nBits);
		cx->gb = (S16)GetSBits(nBits);
		cx->bb = (S16)GetSBits(nBits);
		if ( alpha )
			cx->ab = (S16)GetSBits(nBits);
	} else {
		cx->rb = cx->gb = cx->bb = 0;
	}
}

char* SParser::GetString()
// caller must free memory with a delete operator
{
#if 1
	return CreateStr(GetStringP());
#else
	S32 len = 0;
	S32 savedPos = pos;
	while ( true ) {
		U8 ch = GetByte();
		len++;
		if ( !ch ) break;
	}

	char* ch = new char[len];
	if ( !ch ) 
		return 0;
		
	pos = savedPos;

	char* c = ch;
	while ( true ) {
		U8 ch = GetByte();
		*c++ = ch;
		if ( !ch ) break;
	}

	return ch;
#endif
}

char* SParser::GetStringP()
{
	char* str = (char*)script+pos;	// get the string address
	while ( GetByte() ) {}			// advance past the string
	return str;
}

#ifdef SOUND
void SParser::GetSoundInfo(CSoundChannel* c)
{
	int code = GetByte();
	if ( !c ) {
		// Skip the info
		if ( code & soundHasInPoint )
			SkipBytes(4);//GetDWord();
		if ( code & soundHasOutPoint )
			SkipBytes(4);//GetDWord();
		if ( code & soundHasLoops )
			SkipBytes(2);//GetWord();
		if ( code & soundHasEnvelope ) {
			int nPoints = GetByte();
			SkipBytes(nPoints*(4+2+2));
			//for ( int i = 0; i < nPoints; i++ ) {
			//	SkipBytes(4+2+2);//GetDWord();
			//	//GetWord();
			//	//GetWord();
			//}
		}
	} else {
		// Fill in the info
		c->syncFlags = code>>4;
		if ( code & soundHasInPoint )
			c->inPoint44 = GetDWord();
		if ( code & soundHasOutPoint )
			c->outPoint44 = GetDWord();
		if ( code & soundHasLoops )
			c->loops = GetWord();
		if ( code & soundHasEnvelope ) {
			c->envelope.nPoints = GetByte();
			for ( int i = 0; i < c->envelope.nPoints; i++ ) {
				c->envelope.points[i].mark44 = GetDWord();
				c->envelope.points[i].level0 = GetWord();
				c->envelope.points[i].level1 = GetWord();
			}
		}
	}
}
#endif

void SParser::InitBits()
{
	bitPos = 0;
	bitBuf = 0;
}

U32 SParser::GetBits(int n)
// get n bits from the stream
{
	U32 v = 0;
	for (;;) {
		int s = n-bitPos;
		if ( s > 0 ) {
			// Consume the entire buffer
			v |= bitBuf << s;
			n -= bitPos;

			// Get the next buffer
			bitBuf = GetByte();
			bitPos = 8;
		} else {
		 	// Consume a portion of the buffer
			v |= bitBuf >> -s;
			bitPos -= n;
			bitBuf &= 0xFF >> (8-bitPos);	// mask off the consumed bits
			return v;
		}
	}
}

S32 SParser::GetSBits(int n)
// Get bits w/ sign extension
{
 	S32 v = GetBits(n);
	if ( v & (1L<<(n-1)) ) {
		// Number is negative, extend the sign
		v |= -1L << n;
	}
	return v;
}


//
// The Shape Parser
//

SShapeParser::SShapeParser(ScriptPlayer* p, U8 * data, S32 start, MATRIX* m)
{
//	player = 0;
//	display = 0;
	raster = 0;
	localColors = 0;
	colorList = &localColors;
	mat = *m;
	ratio = 0;
	layer = 0;
	nLines = nFills = 0;

	// Init the default state
	line = fill[0] = fill[1] = 0;
	curPt.x = curPt.y = 0;
	MatrixTransformPoint(&mat, &curPt, &curPtX);

	cxform.Clear();	// the color transform

	// Attach the parser to the script
	player = p; 
	display = p->display;
	SParser::Attach(data, start); 
	FLASHASSERT(player && display);

	fillIndex = fillIndexMem;
	lineIndex = lineIndexMem;

	getAlpha = false;
}

SShapeParser::~SShapeParser()
{
	// Free the index mem if they are "large"
	if ( fillIndex != fillIndexMem ) 
		delete [] fillIndex;
 	if ( lineIndex != lineIndexMem ) 
		delete [] lineIndex;

	// Free the colors if we have a local list
	RColor* color = localColors;
 	while ( color ) {
	 	RColor* next = color->nextColor;
		display->FreeColor(color);
		color = next;
	}
}

typedef RColor *PRColor;

void SShapeParser::SetupColor(RColor* color)
// Set up the colors for the current CRaster
{
	if ( !raster ) return;
	   
	//color->raster = raster;
	switch ( color->colorType ) {
		case colorGradient: {
			// Set up the inverse matrix
			MATRIX gradMat, devMat;
			gradMat = color->grad.savedMat;
			if ( raster->antialias ) {
				MATRIX aaToDev;
				MatrixScale(fixed_1/4, fixed_1/4, &aaToDev);
				MatrixConcat(&mat, &aaToDev, &devMat);	// we don't want the supersampled coords
			} else {
				devMat = mat;
			}

			// We need to work with the translation components in fixed point
			devMat.tx <<= 8;	devMat.ty <<= 8;
			gradMat.tx <<= 8;	gradMat.ty <<= 8;
			MatrixConcat(&gradMat, &devMat, &gradMat);
			MatrixInvert(&gradMat, &color->grad.invMat);
		} break;

		case colorBitmap: {
			// Set up the transforms
			MATRIX bitsMat, devMat;
			if ( raster->antialias ) {
				MATRIX aaToDev;
				MatrixScale(fixed_1/4, fixed_1/4, &aaToDev);
				MatrixConcat(&mat, &aaToDev, &devMat);	// we don't want the supersampled coords
			} else {
				devMat = mat;
			}

			// We need to work with the translation components in fixed point
			bitsMat = color->bm.savedMat;
			bitsMat.tx <<= 16;	bitsMat.ty <<= 16;
			devMat.tx <<= 16;	devMat.ty <<= 16;
			MatrixConcat(&bitsMat, &devMat, &bitsMat);
			MatrixInvert(&bitsMat, &color->bm.invMat);

			SBitmapCore* bits = color->bm.bitmap;
			color->transparent = int( cxform.HasTransparency() ) | int( bits->transparent );
			if ( bits->HasBits() ) {
				color->BuildCache();	// this locks the bitmap so it cannot be purged...
			} else {
				color->colorType = colorSolid;
			}
		} break;
	}
	color->BuildCache();
}

BOOL SShapeParser::GetStyles()
{
	FLASHASSERT(!*colorList || (*colorList)->colorType != colorClip);
	{// Get the fills
		nFills = GetByte();
		if ( nFills == 255 ) {
			// We have a "large number"
			nFills = GetWord();
			if ( fillIndex != fillIndexMem ) 
				delete [] fillIndex;
			fillIndex = new PRColor[nFills+1];
			if ( !fillIndex ) {
				fillIndex = fillIndexMem;
				return false;
			}
		}

		fillIndex[0] = 0;
		for ( int i = 1; i <= nFills; i++ ) {
			RColor* color = display->CreateColor();
			if ( !color ) 
				return false;

			color->SetUp(raster);

			color->nextColor = *colorList;
			*colorList = color;
			fillIndex[i] = color;

			color->order = layer + i;
			FLASHASSERT(color->order < 0x10000);

			int fillStyle = GetByte();
			if ( fillStyle & fillGradient ) {
// 				#if defined(GENERATOR) || defined(_CELANIMATOR)
// 				if (!player->solidGradients) {
// 				#endif
				// Gradient fill
				color->colorType = colorGradient;
				color->grad.gradStyle = fillStyle;
 				color->grad.colorRamp = 0;

				GetMatrix(&color->grad.savedMat);

				color->grad.ramp.nColors = GetByte();
				for ( int j = 0; j < color->grad.ramp.nColors; j++ ) {
					color->grad.ramp.colorRatio[j] = GetByte();
					SRGB c = GetColor(getAlpha);
					if ( c.rgb.transparency < 255 )
						color->transparent = true;
					color->grad.ramp.color[j] = c;
				}

				// merge fix
				if ( cxform.HasTransform() )
				{
					cxform.Apply(&color->grad.ramp);
					if ( cxform.HasTransparency() )
						color->transparent = true;
				}
// 				#if defined(GENERATOR) || defined(_CELANIMATOR)
// 				} else {
// 					// Skip the gradient matrix.
// 					MATRIX matrix;
// 					GetMatrix(&matrix);
// 
// 					// Get the number of colors.
// 					int nColors = GetByte();
// 					for (int j = 0; j < nColors; j++) {
// 						// Is this the first color?
// 						if (!j) {
// 							// Handle the first color as a solid color.
// 							GetByte();
// 							GetColor(&color->rgb, getAlpha);
// 							if (cxform.HasTransform()) cxform.Apply(&color->rgb);
// 							color->transparent = PreMulAlpha(&color->rgb);
// 						} else {
// 							// Skip colors other than the first color.
// 							GetByte();
// 							GetColor(getAlpha);
// 						}
// 					}
// 				}
// 				#endif // GENERATOR || _CELANIMATOR
            } else if ( fillStyle & fillBits ) {
				// A bitmap fill
				U16 tag = GetWord();		// the bitmap tag
				GetMatrix(&color->bm.savedMat);	// the bitmap matrix
				SCharacter* ch = player->FindCharacter(tag);

				// Default to red if we can't find the bits
				color->rgb.alpha = 255;
				color->rgb.red = 255;
				color->rgb.green = color->rgb.blue = 0;

				if ( ch && ch->type == bitsChar ) {
					// We found the bits, set up the color
					color->colorType = colorBitmap;
					color->bm.bitmap = &ch->bits;
					color->bm.bitsStyle = fillStyle;
					color->bm.cxform = cxform;
				//	color->bm.rowIndex = 0;
					player->BuildBits(ch);	// be sure the bits are decompressed and available
				}

			} else {
				// A solid color
				GetColor(&color->rgb, getAlpha);
				if ( cxform.HasTransform() )
					cxform.Apply(&color->rgb);
				color->transparent = PreMulAlpha(&color->rgb);
			}
			SetupColor(color);		// the buildcache in this function must be called immediately after BuildBits()
		}
	}
	
	{// Get the lines
		nLines = GetByte();
		if ( nLines == 255 ) {
			// We have a "large number"
			nLines = GetWord();
			if ( lineIndex != lineIndexMem ) 
				delete [] lineIndex;
			lineIndex = new SLine[nLines+1];
			if ( !lineIndex ) {
				lineIndex = lineIndexMem;
				return false;
			}
		}

		lineIndex[0].color = 0;
		lineIndex[0].thickness = 0;
		for ( int i = 1; i <= nLines; i++ ) {
			RColor* color = display->CreateColor();
			if ( !color ) 
				return false;

			color->SetUp(raster);

			color->nextColor = *colorList;
			*colorList = color;
			lineIndex[i].color = color;
			lineIndex[i].thickness = (int)MatrixTransformThickness(&mat, GetWord());

			color->order = layer + (i+nFills);
			FLASHASSERT(color->order < 0x10000);

			GetColor(&color->rgb, getAlpha);
			if ( cxform.HasTransform() )
				cxform.Apply(&color->rgb);
			color->transparent = PreMulAlpha(&color->rgb);
			SetupColor(color);
		}
	}

	InitBits();
	nFillBits = (int)GetBits(4);
	nLineBits = (int)GetBits(4);
	return true;
}

BOOL SShapeParser::GetMorphStyles()
{
	MATRIX mat1, mat2;
	int i;

	// Get the fills
	nFills = GetByte();
	if ( nFills >= 255 )
	{
		// we have a lot of fills to alloc for
		nFills = GetWord();
		if ( fillIndex != fillIndexMem ) 
			delete fillIndex;
		fillIndex = new PRColor[nFills+1];
		if ( !fillIndex )
		{
			fillIndex = fillIndexMem;
			return false;
		}
	}

	fillIndex[0] = 0;
	for ( i = 1; i <= nFills; i++ )
	{
		RColor* color = display->CreateColor();
		if ( !color ) 
			return false;

		color->SetUp(raster);

		color->nextColor = *colorList;
		*colorList = color;
		fillIndex[i] = color;

		color->order = layer + i;
		FLASHASSERT(color->order < 0x10000);

		int fillStyle = GetByte();
		if ( fillStyle & fillGradient )
		{
// 			#if defined(GENERATOR) || defined(_CELANIMATOR)
// 			if (!player->solidGradients) {
// 			#endif
			// Gradient fill
			color->colorType = colorGradient;
			color->grad.gradStyle = fillStyle;
			color->grad.colorRamp = 0;

			GetMatrix(&mat1);
			GetMatrix(&mat2);
			Interpolate(&mat1, &mat2, ratio, &color->grad.savedMat);

			color->grad.ramp.nColors = GetByte();
			for ( int j = 0; j < color->grad.ramp.nColors; j++ )
			{
				U8 r1, r2;
				SRGB c1, c2;

				r1 = GetByte();
				c1 = GetColor(getAlpha);
				r2 = GetByte();
				c2 = GetColor(getAlpha);
				color->grad.ramp.colorRatio[j] = (U8)Interpolate(r1, r2, ratio);
				Interpolate(&c1, &c2, ratio, &color->grad.ramp.color[j]);

				if ( c1.rgb.transparency < 255 || c2.rgb.transparency < 255)
				{
					color->transparent = true;
				}
			}

			if ( cxform.HasTransform() )
			{
				cxform.Apply(&color->grad.ramp);
				if ( cxform.HasTransparency() )
				{
					color->transparent = true;
				}
			}										
// 			#if defined(GENERATOR) || defined(_CELANIMATOR)
// 			} else {
// 				// Skip the morph gradient matrices.
// 				MATRIX matrix;
// 				GetMatrix(&matrix);
// 				GetMatrix(&matrix);
// 
// 				// Get the number of colors.
// 				int nColors = GetByte();
// 				for (int j = 0; j < nColors; j++) {
// 					// Is this the first color?
// 					if (!j) {
// 						RGBI rgb1, rgb2;
// 
// 						// Handle the first color as a solid color.
// 						GetByte();
// 						GetColor(&rgb1, getAlpha);
// 						GetByte();
// 						GetColor(&rgb2, getAlpha);
// 
// 						Interpolate(&rgb1, &rgb2, ratio, &color->rgb);
// 						if ( cxform.HasTransform() )
// 						{
// 							cxform.Apply(&color->rgb);
// 						}
// 						color->transparent = PreMulAlpha(&color->rgb);
// 					} else {
// 						// Skip colors other than the first color.
// 						GetByte();
// 						GetColor(getAlpha);
// 						GetByte();
// 						GetColor(getAlpha);
// 					}
// 				}
// 			}
// 			#endif // GENERATOR || _CELANIMATOR
		}
		else if ( fillStyle & fillBits )
		{
			// A bitmap fill
			U16 tag = GetWord();		// the bitmap tag

			GetMatrix(&mat1);
			GetMatrix(&mat2);
//			Interpolate(&mat1, &mat2, 0, &color->bm.savedMat);	// The original line is missing the "ratio"
																// Bug from David Michie (Middlesoft) and
																// Jeff Martin. Changed lee@middlesoft
			Interpolate(&mat1, &mat2, ratio, &color->bm.savedMat);
			SCharacter* ch = player->FindCharacter(tag);

			// Default to red if we can't find the bits
			color->rgb.alpha = 255;
			color->rgb.red = 255;
			color->rgb.green = color->rgb.blue = 0;

			if ( ch && ch->type == bitsChar )
			{
				// We found the bits, set up the color
				color->colorType = colorBitmap;
				color->bm.bitmap = &ch->bits;
				color->bm.bitsStyle = fillStyle;
				color->bm.cxform = cxform;
				//	color->bm.rowIndex = 0;
				player->BuildBits(ch);	// be sure the bits are decompressed and available
			}
		}
		else
		{
			RGBI rgb1, rgb2;
			// A solid color
			GetColor(&rgb1, getAlpha);
			GetColor(&rgb2, getAlpha);

			Interpolate(&rgb1, &rgb2, ratio, &color->rgb);
			if ( cxform.HasTransform() )
			{
				cxform.Apply(&color->rgb);
			}
			color->transparent = PreMulAlpha(&color->rgb);
		}
		SetupColor(color);		// the buildcache in this function must be called immediately after BuildBits()
	}
	
	// Get the lines
	nLines = GetByte();
	if ( nLines >= 255 )
	{
		// We have a "large number"
		nLines = GetWord();
		if ( lineIndex != lineIndexMem ) 
			delete lineIndex;
		lineIndex = new SLine[nLines+1];
		if ( !lineIndex )
		{
			lineIndex = lineIndexMem;
			return false;
		}
	}

	lineIndex[0].color = 0;
	lineIndex[0].thickness = 0;
	for (i = 1; i <= nLines; i++ )
	{
		U16		thickness, thick1,	thick2;
		RGBI	rgb1, rgb2;
			
		RColor* color = display->CreateColor();
		if ( !color ) 
			return false;

		color->SetUp(raster);

		color->nextColor = *colorList;
		*colorList = color;
		lineIndex[i].color = color;

		// get the thickness
		thick1 = GetWord();
		thick2 = GetWord();
		thickness = (U16)Interpolate(thick1, thick2, ratio);
		
		lineIndex[i].thickness = (int)MatrixTransformThickness(&mat, thickness);

		color->order = layer + (i+nFills);
		FLASHASSERT(color->order < 0x10000);

		// get the color
		GetColor(&rgb1, getAlpha);
		GetColor(&rgb2, getAlpha);
		
		// interpolate the color value
		Interpolate(&rgb1, &rgb2, ratio, &color->rgb);
		
		if ( cxform.HasTransform() )
			cxform.Apply(&color->rgb);
		color->transparent = PreMulAlpha(&color->rgb);
		SetupColor(color);
	}
	return true;
}

int SShapeParser::GetEdge(CURVE* c)
{
	BOOL isEdge = (int)GetBits(1);
	if ( !isEdge ) {
		// Handle a state change
		int flags = (int)GetBits(5);

		if ( flags == 0 ) 
			// at end, do nothing
			return eflagsEnd;

		// Process a state change

		// Process a MoveTo
		if ( flags & eflagsMoveTo ) {
			int nBits = (int)GetBits(5);
			curPt.x = GetSBits(nBits);
			curPt.y = GetSBits(nBits);
			MatrixTransformPoint(&mat, &curPt, &curPtX);
		}

		// Get new fill info
		if ( flags & eflagsFill0 ) 
			fill[0] = (int)GetBits(nFillBits);
		if ( flags & eflagsFill1 ) 
			fill[1] = (int)GetBits(nFillBits);

		// Get new line info
		if ( flags & eflagsLine )
			line = (int)GetBits(nLineBits);

		// Check to get a new set of styles for a new shape layer
		if ( flags & eflagsNewStyles ) {
		 	layer += nFills+nLines;
			GetStyles();
		}

		FLASHASSERT(fill[0]<=nFills && fill[1]<=nFills && line<=nLines);	// be sure the styles are loaded

		return flags;

	} else {
		// Create an edge
		c->anchor1 = curPtX;

		c->isLine = (int) GetBits(1);
		if ( c->isLine ) {
			// Handle a line
			int nBits = (int)GetBits(4)+2;	// nBits is biased by 2

			// Save the deltas
			BOOL generalLine = (int)GetBits(1);
			if ( generalLine ) {
				// Handle a general line
				curPt.x += GetSBits(nBits);
				curPt.y += GetSBits(nBits);
			} else {
				// Handle a vert or horiz line
				BOOL vertical = (int)GetBits(1);
				if ( vertical ) {
					// Vertical line
					curPt.y += GetSBits(nBits);
				} else {
					// Horizontal line
					curPt.x += GetSBits(nBits);
				}
			}

			MatrixTransformPoint(&mat, &curPt, &c->anchor2);
			PointAverage(&c->anchor1, &c->anchor2, &c->control);

		} else {
		 	// Handle a curve
			int nBits = (int)GetBits(4)+2;	// nBits is biased by 2

			// Get the control
			curPt.x += GetSBits(nBits);
			curPt.y += GetSBits(nBits);

			MatrixTransformPoint(&mat, &curPt, &c->control);

			// Get the anchor
			curPt.x += GetSBits(nBits);
			curPt.y += GetSBits(nBits);

			MatrixTransformPoint(&mat, &curPt, &c->anchor2);
		}

		curPtX = c->anchor2;
		// FLASHOUTPUT("Edge-f1=%i, f2=%i, l=%i, a2.x=%i, a2.y=%i\n", fill[0], fill[1], line, pt.x, pt.y);
		return 0;
	}
}


//
// Script Player
//

ScriptThread::ScriptThread()
{
	script = 0;
	display = 0;
	rootObject = 0;
	layerDepth = 0;

	player = 0;
	next = 0;
    len = 0;
    startPos = 0;

	firstVariable = 0;
	lastVariable = 0;

    justSeeked = true;
    justSeekedNegative = false;

	#ifdef SOUND
	needBuffer = true;
	sndChannel = 0;
    snd.Init();
	#endif

	ClearState();
}

ScriptThread::~ScriptThread()
{
	#ifdef SOUND
	StopStream();
	#endif

	if ( display ) 
		display->RemoveThread(this);

	// Clear all the variables
	ClearVariables();
}

void ScriptThread::ClearVariables()
{
	while ( firstVariable ) {
		ScriptVariable* v = firstVariable;
		firstVariable = v->next;

		FreeStr(v->name);
		FreeStr(v->value);
		delete v;
	}
	lastVariable = 0;
}

void ScriptThread::ClearState()
{
	// Reset the script attributes
	curFrame = -1;
	currentLabel = 0;
	atEnd = false;
	seeking = false;

	len = 0;
	pos = 0;
	script = 0;

	#ifdef SOUND
	StopStream();
	mixFormat = snd22K16Stereo;
	#endif

	numFrames = 0;
	scriptErr = 0;

// 	#ifdef _CELANIMATOR
// 	protect = false;
// 	pathsArePostScript = false;
// 	#endif

	playing = true;
	loop = true;
}


//
// Handle Specific Tags
//

void ScriptThread::PlaceObject()
{
	FLASHASSERT(rootObject);

	PlaceInfo info;
	info.flags = splaceCharacter|splaceMatrix;
	U16 tag = GetWord();
	info.character = player->FindCharacter(tag);
	
	info.depth = GetWord();

	//FLASHOUTPUT("Place tag=%i depth=%i\n", tag, depth);
	
	#ifdef FSSERVER
	if ( !info.character )
		info.character = lostCh;
	#endif
	if ( !info.character ) return;

	// Get the position
	GetMatrix(&info.mat);

	// Get a color transform
	if ( pos < tagEnd ) {
		GetColorTransform(&info.cxform);
		info.flags |= splaceColorTransform;
	} else {
		info.cxform.Clear();
	}

	info.ratio = 0;
	info.name = 0;

	display->PlaceObject(rootObject, &info);
}

void ScriptThread::PlaceObject2()
{
	FLASHASSERT(rootObject);

	PlaceInfo info;
	info.flags = GetByte();
	info.depth = GetWord();

	if ( info.flags & splaceCharacter ) {
		info.character = player->FindCharacter(GetWord());

		#ifdef FSSERVER
		if ( !info.character )
			info.character = lostCh;
		#endif
		if ( !info.character ) 
			return;
	} else
		info.character = 0;
	
	if ( info.flags & splaceMatrix )
		GetMatrix(&info.mat);
	else
		MatrixIdentity(&info.mat);

	if ( info.flags & splaceColorTransform )
		GetColorTransform(&info.cxform, true);
	else
		info.cxform.Clear();

	if ( info.flags & splaceRatio )
		info.ratio = GetWord();
	else
		info.ratio = 0;

	if ( info.flags & splaceName )
		info.name = (char*)AttachData(pos, tagEnd);//script+pos;
	else
		info.name = 0;

	if ( info.flags & splaceDefineClip )
		info.clipDepth = GetWord();
	else
		info.clipDepth = 0;

	if ( info.flags & splaceMove )
		display->MoveObject(rootObject, &info);
	else
		display->PlaceObject(rootObject, &info);
}

////////////////////////////////////////////////////////////
// CloneSprite
//      Create a new sprite that is a copy of the current one.
//      NOTE: State information (current frame, variables, etc.)
//              is not cloned.
//
// inputs:
//      thread  - the sprite we want to clone
//      name    - the name of the new sprite that we are about to 
//                  create. space for this name has been allocated
//                  no need to copy.
//      depth   - what layer we want to place the cloned sprite.
//
//  note: this ptr should be thread->rootObject->parent->thread
//
void ScriptThread::CloneSprite(ScriptThread *thread, char *name, int depth)
{
	FLASHASSERT(rootObject);
    FLASHASSERT(this == thread->rootObject->parent->thread);
    
    PlaceInfo info;

	info.flags = splaceCharacter | splaceMatrix;
	info.depth = depth;

	if (thread->rootObject->character->type == spriteExternalChar)
    {
        // don't use regular character alloc
        // on this. Use new intead. We treat this
        // ch differently since it's external
    	SCharacter *ch = new SCharacter;
	    if (!ch) 
            goto exit_gracefully;
        
        *ch = *thread->rootObject->character;
        info.character = ch;
    }
    else
        info.character = thread->rootObject->character;

    info.mat = thread->rootObject->xform.mat;

    if (thread->rootObject->xform.cxform.HasTransform())
    {
        info.flags |= splaceColorTransform;
        info.cxform = thread->rootObject->xform.cxform;
    }
    else
		info.cxform.Clear();

    // cloned sprites don't have a ratio
    // they always stay on untill their
    // parent gets removed
    info.ratio = (unsigned short)-1;

    info.name = name;
    if (name)
        info.flags |= splaceName;

    if (thread->rootObject->clipDepth > 0)
    {
        info.flags |= splaceDefineClip;
		info.clipDepth = thread->rootObject->clipDepth;
    }
    else
		info.clipDepth = 0;

    if (info.character->type == spriteExternalChar)
    {
        // We never set the character data ptr when
        // we initially loaded the sprite. But now
        // we must during the cloning, because the
        // cloned movie must know where the script
        // is and it gets it from the character
        // data ptr.
        //
        // note: we could not set the data ptr when we
        //          originally created the loaded sprite
        //          because the data was read in after the
        //          sprite structures where created
        info.character->data = thread->script;

		info.flags |= splaceCloneExternalSprite;
    }

	display->RemoveObject(rootObject, depth);
    display->PlaceObject(rootObject, &info);

exit_gracefully:
    return;
}
///////////////////////////////////////////////////////////////
//
//  note: this ptr should be thread->rootObject->parent->thread
void ScriptThread::RemoveSprite(ScriptThread *thread, BOOL createPlaceholder)
{
    PlaceInfo   info;
	SCharacter  *ch = 0;
    SObject     *oldObject = 0;

    FLASHASSERT(this == thread->rootObject->parent->thread);
    FLASHASSERT(thread->rootObject);

    if (!createPlaceholder)
    {
	    display->RemoveObject(rootObject, thread->rootObject->depth);
        goto exit_gracefully;
    }

    // clear out the old sprite and put an empty placeholder
    // in its place which has the same name, depth and position
	ch = player->CreateCharacter();
    oldObject = thread->rootObject;
	if (!ch) 
        goto exit_gracefully;

	ch->next = 0;
	ch->player = player;
	ch->tag = ctagSpritePlaceholder;
	ch->tagCode = 0;
	ch->type = spriteChar;
	ch->data = script;
    ch->sprite.length = 0;
    ch->sprite.numFrames = 0;
    ch->bounds = oldObject->character->bounds;
    
    info.flags = splaceCharacter | splaceMatrix;
	info.depth = oldObject->depth;
    info.mat = oldObject->xform.mat;
    info.puppet = oldObject->puppet;
	info.character = ch;

    if (oldObject->xform.cxform.HasTransform())
    {
        info.flags |= splaceColorTransform;
        info.cxform = thread->rootObject->xform.cxform;
    }
    else
		info.cxform.Clear();

    // make sure we olways replace the empty object during end hold
    info.ratio = thread->rootObject->ratio;
    
    info.clipDepth = 0;
    info.name = CreateStr(oldObject->name);
    if (info.name)
        info.flags |= splaceName;

    // remove old object
	display->RemoveObject(rootObject, thread->rootObject->depth);
    
    // place the empty placeholder
    display->PlaceObject(rootObject, &info);
    
exit_gracefully:
    return;
}

void ScriptThread::RemoveObject()
{
	if ( tagCode == stagRemoveObject ) // the old style remove has the unneed tag
		SkipBytes(2);//GetWord();		//U16 tag = 
	U16 depth = GetWord();

	display->RemoveObject(rootObject, depth);
	//FLASHOUTPUT("Remove tag=%i depth=%i\n", tag, depth);
}

void ScriptThread::DefineShape()
{
	U16 tag = GetWord();

	SCharacter* ch = player->CreateCharacter(tag);
	if ( !ch ) return;

	// Set up object
	ch->type = shapeChar;
	ch->tagCode = tagCode;

	// Get the bounds
	GetRect(&ch->bounds);

	// Simply save a pointer to the original data
	ch->data = AttachData(pos, tagEnd);//script+pos;
}

void ScriptThread::DefineMorphShape()
{
	U16 tag = GetWord();

	SCharacter* ch = player->CreateCharacter(tag);
	if ( !ch ) return;

	// Set up object
	ch->type = morphShapeChar;
	ch->tagCode = tagCode;

	// Get the bounds
	GetRect(&ch->bounds);			// start bounds
	GetRect(&ch->morph.bounds2);	// end bounds

	// Interpolate(&rectBeg, &rectEnd, ratio, &ch->bounds);

	// Simply save a pointer to the original data
	ch->data = AttachData(pos, tagEnd);  //script+pos;
}

void ScriptThread::DefineButton()	
{
	U16 tag = GetWord();

	SCharacter* ch = player->CreateCharacter(tag);
	if ( !ch ) return;

	// Set up object
	ch->type = buttonChar;
	ch->tagCode = tagCode;

	// Simply save a pointer to the original data
	ch->button.soundData = 0;
	ch->button.cxformData = 0;

	if ( tagCode == stagDefineButton2 ) {
		ch->button.trackAsMenu = GetByte();
	} else {
		ch->button.trackAsMenu = false;
	}

	ch->data = AttachData(pos, tagEnd);//script+pos;
}

void ScriptThread::DefineButtonExtra(int i)
// Attach a sound definition to a button
{
	U16 tag = GetWord();

	SCharacter* ch = player->FindCharacter(tag);
	if ( !ch || ch->type != buttonChar ) return;

	// Save a pointer to the info
	if ( i == 0 ) {
		if ( !ch->button.soundData )
			ch->button.soundData = AttachData(pos, tagEnd);
	} else {
		if ( !ch->button.cxformData )
			ch->button.cxformData = AttachData(pos, tagEnd);
	}
}

void ScriptThread::DefineSprite()
// Attach a sound definition to a button
{
	U16 tag = GetWord();

	SCharacter* ch = player->CreateCharacter(tag);
	if ( !ch ) return;

	// Set up object
	ch->type = spriteChar;

	// Simply save a pointer to the original data
	ch->sprite.numFrames = GetWord();
	ch->sprite.length = tagEnd - pos;
	ch->data = AttachData(pos, tagEnd);
}

#ifdef USE_VIDEO
void ScriptThread::DefineVideo()
// Attach a sound definition to a button
{
	U16 tag = GetWord();

	SCharacter* ch = player->CreateCharacter(tag);
	if ( !ch ) return;

	// Set up object
	ch->type = videoChar;

	// Simply save a pointer to the original data
	ch->data = AttachData(pos, tagEnd);
}
#endif

#ifdef USE_JPEG

//
// JPEG Data Source
//

#define JPEGBufferSize 512
struct play_source_mgr {
	struct jpeg_source_mgr pub;	/* public fields */
	SParser* player;
	JOCTET buffer[JPEGBufferSize];
	boolean start_of_file;	/* have we gotten any data yet? */
};

struct JPEGPlayState {
	jpeg_decompress_struct cinfo;
	jpeg_error_mgr jerr;
	play_source_mgr src;
};

// #if defined(_UNIX) && defined(NETSCAPE)
// #undef METHODDEF
// #define METHODDEF
// #endif

METHODDEF void
play_init_source (j_decompress_ptr cinfo)
{
}

METHODDEF boolean
play_fill_input_buffer (j_decompress_ptr cinfo)
{
	play_source_mgr* src = (play_source_mgr*)cinfo->src;
	
	FLASHASSERT(JPEGBufferSize<16000);
	int nbytes = (int)src->player->GetData(src->buffer, JPEGBufferSize);
	if ( nbytes <= 0 ) {
		FLASHASSERT(false);	// the jpeg code wanted more data but we did not have any...
		// Insert a fake EOI marker
		src->buffer[0] = (JOCTET) 0xFF;
		src->buffer[1] = (JOCTET) JPEG_EOI;
		nbytes = 2;
	}

	src->pub.next_input_byte = src->buffer;
	src->pub.bytes_in_buffer = nbytes;
	return TRUE;
}

METHODDEF void
play_skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
	play_source_mgr* src = (play_source_mgr*) cinfo->src;

	/* Just a dumb implementation for now.  Could use fseek() except
	* it doesn't work on pipes.  Not clear that being smart is worth
	* any trouble anyway --- large skips are infrequent.
	*/
	while ( num_bytes > (long)src->pub.bytes_in_buffer ) {
		num_bytes -= (long)src->pub.bytes_in_buffer;
		play_fill_input_buffer(cinfo);
	}
	src->pub.next_input_byte += (size_t) num_bytes;
	src->pub.bytes_in_buffer -= (size_t) num_bytes;
}

METHODDEF void
play_term_source (j_decompress_ptr cinfo)
{
  /* no work necessary here */
}

void ReadJPEGTables(JPEGPlayState* jpegInfo, SParser* player)
{
	jpegInfo->cinfo.err = jpeg_afx_error(&jpegInfo->jerr);
	jpeg_create_decompress(&jpegInfo->cinfo);

	// Specify data source (eg, a file)
	jpegInfo->cinfo.src = &jpegInfo->src.pub;
	jpegInfo->src.pub.init_source = play_init_source;
	jpegInfo->src.pub.fill_input_buffer = play_fill_input_buffer;
	jpegInfo->src.pub.skip_input_data = play_skip_input_data;
	jpegInfo->src.pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
	jpegInfo->src.pub.term_source = play_term_source;
	jpegInfo->src.player = player;
	jpegInfo->src.pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
	jpegInfo->src.pub.next_input_byte = NULL; /* until buffer loaded */

	// Read the tables
	jpeg_read_header(&jpegInfo->cinfo, FALSE);
}

void ReadJPEGImage(JPEGPlayState* jpegInfo, SCharacter* ch, BOOL needAlpha)
{
	needAlpha = TRUE;

	jpeg_read_header(&jpegInfo->cinfo, TRUE);
	jpeg_start_decompress(&jpegInfo->cinfo);

	ch->bits.PICreate(needAlpha ? bm32Bit : bm16Bit, jpegInfo->cinfo.output_width, jpegInfo->cinfo.output_height, 0, true);

	if ( !ch->bits.HasBits() ) 
		goto Abort;

	{
		JSAMPLE* buffer = new JSAMPLE[jpegInfo->cinfo.output_width * jpegInfo->cinfo.output_components];
		if ( !buffer ) {
			ch->bits.PIFree();
			goto Abort;
		}
		ch->bits.LockBits();
		while (jpegInfo->cinfo.output_scanline < jpegInfo->cinfo.output_height) {
			// Get a scanline
			jpeg_read_scanlines(&jpegInfo->cinfo, &buffer, 1);

			// Copy the row to our bitmap
			JSAMPLE* src = buffer;
			if ( needAlpha ) {
				// 32 bit pixels
				U32 * dstPix = (U32 *)(ch->bits.baseAddr + ch->bits.rowBytes*(jpegInfo->cinfo.output_scanline-1));
				for ( int i = ch->bits.width; i--; ) {
// #if defined(SOLARIS) && defined(NETSCAPE) && !defined(GENERATOR)
// 					Make sure the the alpha gets set to FF
// 					xxx
// 					*dstPix = ((U32)src[0]<<g_redShift)|
// 							  ((U32)src[1]<<g_greenShift)|
// 							  ((U32)src[2]<<g_blueShift);
// #else
					*dstPix = 0xFF000000L |
							  ((U32)src[0] << 16) | 
							  ((U32)src[1] <<  8) | 
								    src[2];
// #endif
					dstPix++;
					src += 3;
				}
			} else {
				// 16 bit pixels
				U16 * dstPix = (U16 *)(ch->bits.baseAddr + ch->bits.rowBytes*(jpegInfo->cinfo.output_scanline-1));
				for ( int i = ch->bits.width; i--; ) {
					*dstPix = (U16)(src[0] & 0xF8) << 7  | 
								   (src[1] & 0xF8) << 2  | 
							       (src[2] /*& 0xF8*/) >> 3;
					dstPix++;
					src += 3;
				}
			}
		}
		delete [] buffer;
		ch->bits.UnlockBits();
	}

Abort:
	jpeg_finish_decompress(&jpegInfo->cinfo);
}
#endif

void ScriptThread::DefineJPEGTables()
// Createa a JPEG decompressor object and set up the shared tables
{
  #ifdef USE_JPEG
	if ( !player->jpegInfo ) {
		player->jpegInfo = new JPEGPlayState;
		if ( !player->jpegInfo ) return;
		ReadJPEGTables(player->jpegInfo, this);
	}
  #endif
}

void ScriptThread::BuildBits(SCharacter* ch)
{
	if ( !ch->bits.HasBits() ) {
		// Decompress the bits of the bitmap
		ch->bits.PIFree();

		SParser parser;
		parser.Attach(ch->data, 0);
		int code = parser.GetTag();
		parser.GetWord();	// skip character tag

		switch ( code ) {
  #ifdef USE_JPEG
			case stagDefineBits: {
				// Read JPEG info
				if ( player->jpegInfo ) {
					player->jpegInfo->src.player = &parser;
					ReadJPEGImage(player->jpegInfo, ch, false);
				}
			} break;

			case stagDefineBitsJPEG2: 
			case stagDefineBitsJPEG3: {
				// Read a JPEG using custom tables
				S32 savedEnd = parser.tagEnd;
				if ( code == stagDefineBitsJPEG3 ) {
					// Mark the end of the JPEG data
					S32 length = parser.GetDWord();
					parser.tagEnd = parser.pos + length;
				}

				JPEGPlayState jpeg;
				ReadJPEGTables(&jpeg, &parser);
				ReadJPEGImage(&jpeg, ch, code == stagDefineBitsJPEG3);
				jpeg_destroy_decompress(&jpeg.cinfo);

				parser.tagEnd = savedEnd;	// restore our tag end

				if ( code == stagDefineBitsJPEG3 ) {

					// Get the alpha channel
					int err;
					z_stream d_stream; /* decompression stream */

					d_stream.zalloc = (alloc_func)0;
					d_stream.zfree = (free_func)0;
					d_stream.opaque = (voidpf)0;

					err = inflateInit(&d_stream);
					if ( err != 0 ) goto AbortL;

					enum { bufSize = 2048 };
					U8 buf[bufSize];

					ch->bits.transparent = true;

					S32 size = (S32) ch->bits.width* (S32) ch->bits.height;
					ch->bits.LockBits();

// 					#ifdef WIN16
// 						//	deal with the fact that zlib can't
// 						// 	go beyond 64k on win16
// 
// 						U8  *	bits = (U8  *)ch->bits.baseAddr;
// 						U8			intermediate[2048];
// 						U8  *	srcData = parser.script + parser.pos;
// 						S32			avail_in = parser.tagEnd-parser.pos;
// 						S32			tmp = Min(2048, avail_in);
// 
// 						d_stream.avail_in = (int) tmp;
// 						d_stream.total_in = 0;
// // 						hmemcpy(intermediate, srcData, d_stream.avail_in);
// 						memcpy(intermediate, srcData, d_stream.avail_in);
// 						srcData += d_stream.avail_in;
// 						d_stream.next_in = (Bytef*) intermediate;
// 
// 						while ( true ) {
// 							if (d_stream.total_in >= 2048)
// 							{
// 								d_stream.total_in = 0;
// 								avail_in -= 2048;
// 								tmp = Min(2048, avail_in);
// 								d_stream.avail_in = (int) tmp;
// // 								hmemcpy(intermediate, srcData, d_stream.avail_in);
// 								memcpy(intermediate, srcData, d_stream.avail_in);
// 								srcData += d_stream.avail_in;
// 								d_stream.next_in = (Bytef*) intermediate;
// 							}
// 
// 							d_stream.next_out = (Bytef*)buf; 
// 							tmp = Min(bufSize, size);
// 							d_stream.avail_out = (int) tmp;
// 
// 							err = inflate(&d_stream, Z_NO_FLUSH);
// 							U8* b = buf;
// 							
// 							while ( (U8*)d_stream.next_out > b ) {
// 								int a = *b++;
// 								if ( bits[0] > a ) bits[0] = a;
// 								if ( bits[1] > a ) bits[1] = a;
// 								if ( bits[2] > a ) bits[2] = a;
// 								bits[3] = a;
// 								bits += 4;
// 							}
// 							if ( err || size == 0 ) 
// 								break;
// 						}
// 					#else
						d_stream.next_in = (Bytef*)parser.script + parser.pos;
						d_stream.avail_in = parser.tagEnd-parser.pos;

						U8* bits = (U8*)ch->bits.baseAddr;
						while ( true ) {
							d_stream.next_out = (Bytef*)buf;
							d_stream.avail_out = Min(bufSize, size);
							err = inflate(&d_stream, Z_NO_FLUSH);
							U8* b = buf;
							while ( (U8*)d_stream.next_out > b ) {
								int a = *b++;
								#ifdef BIG_ENDIAN
								// Mac 68K, Mac PPC & Sparc CPUs
								if ( bits[3] > a ) bits[3] = a;
								if ( bits[2] > a ) bits[2] = a;
								if ( bits[1] > a ) bits[1] = a;
								bits[0] = a;
								#else
								// Intel CPUs
								if ( bits[0] > a ) bits[0] = a;
								if ( bits[1] > a ) bits[1] = a;
								if ( bits[2] > a ) bits[2] = a;
								bits[3] = a;
								#endif
								bits += 4;
							}
							if ( err || size == 0 ) 
								break;
						}
// 					#endif
					ch->bits.UnlockBits();
				}
			} break;
#endif

			case stagDefineBitsLossless:
			case stagDefineBitsLossless2: {
				// Decompress lossless data
				int err;
				z_stream d_stream; /* decompression stream */

				d_stream.zalloc = (alloc_func)0;
				d_stream.zfree = (free_func)0;
				d_stream.opaque = (voidpf)0;

				{
					int format = parser.GetByte();
					int width = parser.GetWord();
					int height = parser.GetWord();
					BOOL readAlpha = code == stagDefineBitsLossless2;

					// Get the color table size
					int nColors = format <= bm8Bit ? parser.GetByte()+1 : 0;

// 					#ifdef WIN16
// 					// We have to use intermediate buffers for WIN16 because zlib does not handle segment boundaries
// 					{
// 						Byte buf_in[1024];
// 						
// 						err = inflateInit(&d_stream);
// 						if ( err != 0 ) goto AbortL;
// 
// 						d_stream.next_in = buf_in;
// 						d_stream.avail_in = (int)Min(parser.tagEnd-parser.pos, 1024);
// // 						hmemcpy(buf_in, parser.script + parser.pos, d_stream.avail_in);
//  						memcpy(buf_in, parser.script + parser.pos, d_stream.avail_in);
// 						parser.pos += d_stream.avail_in;
// 
// 						// Get the color table
// 						SColorTable ctab;
// 						if ( format <= bm8Bit ) {
// 							U8 buf[4];
// 							buf[3] = 255;	// use an opaque alpha when none is stored
// 							RGB8* c = ctab.colors;
// 							ctab.n = nColors;
// 							for ( int i = nColors; i--; c++ ) {
// 								d_stream.next_out = buf;
// 								d_stream.avail_out = readAlpha ? 4 : 3;
// 								while ( d_stream.avail_out > 0 ) {
// 									err = inflate(&d_stream, Z_NO_FLUSH);
// 									FLASHASSERT(err == 0);
// 								}
// 
// 								c->red   = buf[0];
// 								c->green = buf[1];
// 								c->blue  = buf[2];
// 								c->alpha = buf[3];
// 							}
// 						}
// 
// 						// Set up the bitmap
// 						ch->bits.PICreate(format, width, height, format <= bm8Bit ? &ctab : 0, true);
// 						ch->bits.transparent = readAlpha;
// 
// 						if ( !ch->bits.HasBits() ) 
// 							goto AbortL;
// 
// 						Byte buf_out[2048];
// 						
// 						ch->bits.LockBits();
// 						S32 size = height*ch->bits.rowBytes;
// 						char * bits = ch->bits.baseAddr;
// 						while ( true ) {
// 							// Fill the input buffer
// 							if ( d_stream.avail_in == 0 ) {
// 								d_stream.next_in  = buf_in;
// 								d_stream.avail_in = (int)Min(parser.tagEnd-parser.pos, 1024);
// // 								hmemcpy(buf_in, parser.script+parser.pos, d_stream.avail_in);
// 								memcpy(buf_in, parser.script+parser.pos, d_stream.avail_in);
// 								parser.pos += d_stream.avail_in;
// 							}
// 							
// 							// Decompress
// 							d_stream.next_out  = buf_out;
// 							d_stream.avail_out = 2048;
// 							err = inflate(&d_stream, Z_NO_FLUSH);
// 										 
// 							// Copy to bitmap
// 							int n = 2048-d_stream.avail_out;
// // 							hmemcpy(bits, buf_out, n);
// 							memcpy(bits, buf_out, n);
// 							bits += n;
// 							
// 							if ( err != Z_OK ) break;
// 						}
// 						FLASHASSERT(err == Z_STREAM_END);
// 						FLASHASSERT(bits == ch->bits.baseAddr + size);
// 						SBitmapSwapBits(ch->bits.baseAddr, size, ch->bits.bmFormat);
// 						ch->bits.UnlockBits();
// 					}
// 					#else
					{
						err = inflateInit(&d_stream);
						if ( err != 0 ) goto AbortL;

						d_stream.next_in = (Bytef*)parser.script + parser.pos;
						d_stream.avail_in = parser.tagEnd-parser.pos;

						// Get the color table
						SColorTable ctab;
						if ( format <= bm8Bit ) {
							U8 buf[4];
							buf[3] = 255;	// use an opaque alpha when none is stored
							RGB8* c = ctab.colors;
							ctab.n = nColors;
							for ( int i = nColors; i--; c++ ) {
								d_stream.next_out = buf;
								d_stream.avail_out = readAlpha ? 4 : 3;
								while ( d_stream.avail_out > 0 ) {
									err = inflate(&d_stream, Z_NO_FLUSH);
									FLASHASSERT(err == 0);
								}

								c->red   = buf[0];
								c->green = buf[1];
								c->blue  = buf[2];
								c->alpha = buf[3];
							}
						}

						// Set up the bitmap
						ch->bits.PICreate(format, width, height, format <= bm8Bit ? &ctab : 0, true);
						ch->bits.transparent = readAlpha;

						if ( !ch->bits.HasBits() ) 
							goto AbortL;

						ch->bits.LockBits();
						S32 size = height*ch->bits.rowBytes;
						d_stream.next_out  = (Bytef*)ch->bits.baseAddr;
						d_stream.avail_out = size;
						while ( true ) {
							err = inflate(&d_stream, Z_NO_FLUSH);
							if ( err != Z_OK ) break;
						}
						FLASHASSERT(err==Z_STREAM_END);
						SBitmapSwapBits(ch->bits.baseAddr, size, ch->bits.bmFormat);
						ch->bits.UnlockBits();
					}
// 					#endif
				}

			AbortL:
				err = inflateEnd(&d_stream);
				FLASHASSERT(err==0);
			} break;
		}
	}
}

void ScriptThread::DefineBits()
// Create a bitmap from JPEG data
{
	U16 tag = GetWord();

	SCharacter* ch = player->CreateCharacter(tag);
	if ( !ch ) return;

	// Set up object
	ch->type = bitsChar;
	ch->data = AttachData(pos-8, tagEnd);//script+pos-8;

	ch->bits.PIInit();
}

// #ifdef _CELANIMATOR
// void ScriptThread::DefineBitsPtr()
// // Create a bitmap that references another bitmap that has already been allocated
// // A performance optimization for the editor only
// {
// 	U16 tag = GetWord();
// 
// 	SCharacter* ch = player->CreateCharacter(tag);
// 	if ( !ch ) return;
// 
// 	// Set up object
// 	ch->type = bitsChar;
// 	ch->data = 0;
// 
// 	SBitmapCore* bm = (SBitmapCore*)GetDWord();
// 	ch->bits = *bm;
// }
// #endif

void ScriptThread::DefineFont()
{
	U16 tag = GetWord();

	SCharacter* ch = player->CreateCharacter(tag);
	if ( !ch ) return;

	// Set up object
	ch->type = fontChar;
	ch->tagCode = tagCode;
	ch->font.infoData = 0;
	ch->font.deviceState = textUnknown;
	ch->font.nGlyphs = 0;
	ch->font.flags = 0;
// [gsg] nDataOffset now used in Player (edit text)
//#ifdef _CELANIMATOR
	ch->font.nDataOffset = 0;
//#endif

	// Simply save a pointer to the original data
	ch->data = AttachData(pos, tagEnd);//script+pos;
}

void ScriptThread::DefineFont2()
{
	U16 tag = GetWord();

	SCharacter* ch = player->CreateCharacter(tag);
	if ( !ch ) return;

	// Set up object
	ch->type = fontChar;
	ch->tagCode = tagCode;
	ch->font.infoData = 0;
	ch->font.deviceState = textUnknown;
	ch->font.flags = GetWord();
// [gsg] nDataOffset now used in Player (edit text)
//#ifdef _CELANIMATOR
	ch->font.nDataOffset = pos;
//#endif
	
	// Skip the font name
	SkipBytes(GetByte());		// save a bit of code... (JLG)
	//U8 fontNameLen = GetByte();
	//for (U8 i = 0; i < fontNameLen; ++i) GetByte();
	
	// Get the number of glyphs.
	ch->font.nGlyphs = GetWord();
//#ifdef _CELANIMATOR
	ch->font.nDataOffset = pos - ch->font.nDataOffset;
//#endif

	// Simply save a pointer to the original data
	ch->data = AttachData(pos, tagEnd);//script+pos;
}


void ScriptThread::DefineFontInfo()
{
	U16 tag = GetWord();

	SCharacter* ch = player->FindCharacter(tag);
	if ( !ch || ch->type != fontChar ) return;

	// Save a pointer to the info
	if ( !ch->font.infoData )
		ch->font.infoData = AttachData(pos, tagEnd);//script+pos;
}

void ScriptThread::DefineText()
{
	U16 tag = GetWord();

	SCharacter* ch = player->CreateCharacter(tag);
	if ( !ch ) return;

	// Set up object
	ch->type = textChar;
	ch->tagCode = tagCode;
	ch->text.deviceState = textUnknown;

	// Get the bounds
	GetRect(&ch->bounds);

	// Simply save a pointer to the original data
	ch->data = AttachData(pos, tagEnd);//script+pos;
}

#ifdef EDITTEXT
void ScriptThread::DefineEditText()
{
	U16 tag = GetWord();

	SCharacter* ch = player->CreateCharacter(tag);
	if ( !ch ) return;

	// Set up object
	ch->type = editTextChar;
	ch->tagCode = tagCode;

	// Get the bounds
	GetRect(&ch->bounds);

	// Simply save a pointer to the original data
	ch->data = AttachData(pos, tagEnd);//script+pos;

	U16 flags = GetWord();
	if (!(flags & seditTextFlagsUseOutlines)) {
		display->hasDeviceEditText = TRUE;
	}
}
#endif

#ifdef SOUND
void ScriptThread::DefineSound()
{                              
	U16 tag = GetWord();

	SCharacter* ch = player->CreateCharacter(tag);
	if ( !ch ) return;

	// Set up object
	ch->type = soundChar;

	// Get the bounds
	ch->sound.format = GetByte();
	ch->sound.nSamples = GetDWord();

	switch (ch->sound.CompressFormat())
    {
        case sndCompressNone:
        case sndCompressNoneI:
        case sndCompressADPCM:
            ch->sound.delay = 0;
            break;

        case sndCompressMP3:
            // get the compression delay in samples
            ch->sound.delay = GetWord();
            break;

		// We don't support the compression format
        default:
		    player->FreeCharacter(tag);
            goto exit_gracefully;
            break;
	}

    // Save a pointer to the s data
	ch->data = AttachData(pos, tagEnd); //script+pos;
	ch->sound.samples = ch->data;	    // this may be problem if we muck with the script pointer someday
    ch->sound.dataLen = tagEnd - pos;

exit_gracefully:;
}

void ScriptThread::StartSound()
{
	U16 tag = GetWord();

	SCharacter* ch = player->FindCharacter(tag);
	if ( !ch || ch->type != soundChar || player->mute ) return;

// #ifdef SPRITE_XTRA
// 	CSoundChannel* channel = CSoundMix::Mixer()->NewChannel();
// #else
	CSoundChannel* channel = new CSoundChannel();
// #endif
	if ( !channel ) return;
	channel->AddRef();
	channel->sound = &ch->sound;
	channel->tag = (U32)this;

	GetSoundInfo(channel);

// #ifdef SPRITE_XTRA
// 	CSoundMix::Mixer()->AddSound(channel);
// #else
	splayer->theSoundMix.AddSound(channel);
// #endif
	channel->Release();
}

BOOL ScriptStreamProc(CSoundChannel* channel)
// WARNING: this is called at interupt time on the Mac, don't do any memory allocations...
{
	ScriptThread* thread = (ScriptThread*)(channel->refPtr);
	FLASHASSERT(thread->sndChannel == channel);
	return thread->DoSoundFrame();
}

void ScriptThread::StopStream()
{
	if ( sndChannel ) {
		sndChannel->Stop();
		sndChannel->Release();
		sndChannel = 0;
	}
	needBuffer = (player == this);//true;	// don't start the sound until we have a decent amount loaded (for the streaming thread)
}

void ScriptThread::AdjustMp3Streaming()
{
    if (justSeeked)
    {
        
        justSeeked = false;
        if (sndChannel->blockSamples > 0)
        {
            FLASHASSERT(sndChannel->seekSamples >= 0);
            justSeekedNegative = true;
            AdjustMp3Streaming(); // recurse
        }
        else if (sndChannel->blockSamples == 0 && sndChannel->seekSamples == 0)
        {
            // do nothing here
            return;
        }
        else if (sndChannel->seekSamples < 0)
        {
            justSeekedNegative = true;
            AdjustMp3Streaming(); // recurse
        }
        else
        {
            FLASHASSERT(0);
        }
        
    }
    else
    {
        if (justSeekedNegative && sndChannel->blockSamples > 0)
        {
            FLASHASSERT(sndChannel->seekSamples >= 0);
            justSeekedNegative = false;
            sndChannel->blockSamples -= sndChannel->seekSamples;
            
            
            // throw away the samples that don't belong in this
            // frame. These samples actually belong in the previous
            // frame. If we read ahead one frame we could actually
            // use it (!!@ should we read ahead one frame). But
            // this really should not be noticable by an average
            // human. If you are above average come and talk to me
            // SBL
            sndChannel->decomp->Decompress(0, sndChannel->seekSamples);
        }
            
        if (sndChannel->seekSamples < 0)
            justSeekedNegative = true;
    }
}

BOOL ScriptThread::DoSoundFrame()
// Process the sound commands for the next frame
// WARNING: this is called at interupt time on the Mac, don't do any memory allocations...
{
#ifndef FSSERVER
	FLASHASSERT(sndChannel);

	if ( sndAtEnd ) 
		return false;

	// Check to prebuffer 5 seconds of sound
	// The sound prebuffering amount can be customized through the
	// _soundbuftime property.
	if ( needBuffer ) {
		FLASHASSERT(player == this);	// this should only happen on the main thread

		int soundBufferTime = 5;
		if (player->splayer) {
			soundBufferTime = player->splayer->nSoundBufferTime;
		}
		S32 count = player->frameDelay > 0 ? (soundBufferTime*1000)/player->frameDelay : 100;
		if ( !player->FrameComplete(sndFrame+count) ) {
			return false;
		}
		needBuffer = false;
	}

	SParser savedState = *(SParser*)this;
	while ( true ) {
		pos = sndPos;
		int code = GetTag(len);
		if ( code < 0 ) {
			needBuffer = true;			// we ran out so rebuffer the sound
			*(SParser*)this = savedState;
			FLASHASSERT(player == this);	// this should only happen on the main thread
			return player->ScriptComplete();	// return true at the end of the script
		}
		sndPos = tagEnd;	// start at the next pos

		switch ( code ) {
			case stagEnd:
				sndAtEnd = true;
				goto Done;
			case stagShowFrame:
				sndFrame++;
				goto Done;
			case stagDefineSound:
				DefineSound();
				break;
			case stagStartSound:
				// Start a sound
// 				#ifdef SPRITE_XTRA
// 				if ( CSoundMix::Mixer()->MixAhead() )
// 				#endif
					StartSound();
// 				break;
			//case stagSoundStreamHead:
			//	SoundStreamHead();
			//	break;
			case stagSoundStreamBlock: {
				// Set up the decompression for the block
                if (snd.CompressFormat() == sndCompressMP3)
                {
				    sndChannel->blockSamples = GetWord();
                    sndChannel->seekSamples = (S16) GetWord();
                    sndChannel->blockCanBeZero = true;
                }
                else
                {
				    sndChannel->blockSamples = snd.nSamples;
                }
				snd.samples = script+pos;
                snd.dataLen = tagEnd - pos;

                if ( sndChannel->decomp )
                {
                    // make sure setup does not skip past delay
                    // with streaming sound we do that manually
                    int delaySav = snd.delay;
                    snd.delay = 0;
					sndChannel->decomp->Setup(&snd, justSeeked);
                    snd.delay = delaySav;
                    
                    // adjust mp3 channel according to the
                    // hind data the outoring has provided
                    if (snd.CompressFormat() == sndCompressMP3)
                        AdjustMp3Streaming();
                }
				else
					sndChannel->pos = 0;
			} break;
		}
	}

Done:
	*(SParser*)this = savedState;
#endif
	return true;	// we got the block
}

void ScriptThread::SoundStreamHead()
// Set up the stream formats
{
// #ifndef FSSERVER
	mixFormat = GetByte();

	// The stream settings
    snd.Init();
	snd.format = GetByte();
	snd.nSamples = GetWord();


	if (snd.CompressFormat() == sndCompressMP3)
    {
        // get the compression delay in samples
        snd.delay = GetWord();
    }
// #endif
}

void ScriptThread::SoundStreamBlock()
{
// #ifndef FSSERVER
	FLASHASSERT(!sndChannel);
	FLASHASSERT(player);
	if ( !playing ) return;
	if ( player->mute ) return;	// the player is muted
	if ( player->splayer && !player->splayer->running ) return; // the splayer is not running
	if ( snd.CompressFormat() > sndCompressNoneI ) return;	// we don't support this compression format

	// Create a new channel
// #ifdef SPRITE_XTRA
// 	sndChannel = CSoundMix::Mixer()->NewChannel();
// #else
	sndChannel = new CSoundChannel();
// #endif
	if ( sndChannel ) {
		sndChannel->AddRef();
		sndChannel->sound = &snd;
		sndChannel->samplesPlayed = curFrame > 0 ? snd.nSamples*curFrame : 0;	// adjust the number of samples for the starting frame number
		sndChannel->refPtr = this;
		sndChannel->streamProc = ScriptStreamProc;

		sndAtEnd = false;
		sndFrame = curFrame;
		sndPos = pos-6;	// the start of the tag

// #ifdef SPRITE_XTRA
// 		CSoundMix::Mixer()->AddSound(sndChannel);
// #else
		splayer->theSoundMix.AddSound(sndChannel);
// #endif
	}
// #endif
}
#endif

int ScriptThread::DoTag()
// Process a single tag
{
	S32 savedPos = pos;

	{
		// DrawFrame checks these...
		FLASHASSERT(script && !scriptErr);

		if ( atEnd ) 
			return playAtEnd;

		int code = GetTag(len);
		if ( code < 0 )
			return playNeedData;	// we need more data before we can process this tag

		switch ( code ) {
			case stagEnd: 
				atEnd = true;
				break;
			case stagShowFrame:
				curFrame++;
				break;
			case stagDefineShape: 
			case stagDefineShape2:
			case stagDefineShape3:
				DefineShape(); 
				break;
			case stagDefineMorphShape:
				DefineMorphShape(); 
				break;
			case stagFreeCharacter: {
				U16 tag = GetWord();
				player->FreeCharacter(tag);
			} break;
			case stagPlaceObject:
 				PlaceObject();
				break;
			case stagPlaceObject2:
				PlaceObject2();
				break;
			case stagRemoveObject:
			case stagRemoveObject2:
				RemoveObject();
				break;
			case stagDefineBits:
			case stagDefineBitsLossless:
			case stagDefineBitsJPEG2:
			case stagDefineBitsLossless2:
			case stagDefineBitsJPEG3:
				DefineBits();
				break;
			case stagJPEGTables:
				DefineJPEGTables();
				break;
			case stagDefineButton:
			case stagDefineButton2:
				DefineButton();
				break;
			case stagDefineSprite:
				DefineSprite();
				break;
			case stagDefineFont:
				DefineFont();
				break;
			case stagDefineFontInfo:
				DefineFontInfo();
				break;
			case stagDefineFont2:
				DefineFont2();
				break;
			case stagDefineText:
			case stagDefineText2:
				DefineText();
				break;
#ifdef USE_VIDEO
			case stagDefineVideo:
				DefineVideo();
				break;
#endif
#ifdef EDITTEXT
			case stagDefineEditText:
				DefineEditText();
				break;
#endif
			case stagSetBackgroundColor:
				display->SetBackgroundColor(GetColor());
				break;
			case stagDoAction:
				if ( !seeking )
					display->PushAction(script+pos, this);
				break;
			#ifdef SOUND
			case stagDefineSound:
				DefineSound();
				break;
			case stagStartSound:
// 				#ifdef SPRITE_XTRA
// 				if ( CSoundMix::Mixer()->MixAhead() )
// 				#endif
				{
					if ( !sndChannel && !seeking )
						StartSound();
				}
// 				#ifdef SPRITE_XTRA
// 				else if (!seeking )
// 					StartSound();
// 				#endif
				break;
			//case stagStopSound:
			//	StopSound();
			//	break;
			case stagDefineButtonSound:
				DefineButtonExtra(0);
				break;
			case stagSoundStreamHead:
			case stagSoundStreamHead2:
				SoundStreamHead();
				break;
			case stagSoundStreamBlock:
				if ( !sndChannel && !seeking )
					SoundStreamBlock();
				break;
			#endif

			case stagDefineButtonCxform:
				DefineButtonExtra(1);
				break;

// 			#ifdef _CELANIMATOR
// 			case stagDefineBitsPtr:
// 				DefineBitsPtr();
// 				break;
// 
// 			case stagProtect:
// 				protect = true;
// 				break;
// 
// 			case stagPathsArePostScript:
// 				pathsArePostScript = true;
// 				break;
// 			#endif

			case stagFrameLabel:
				currentLabel = (char*)script+pos;
				break;

			default:
				//FLASHASSERT(false);
				//FLASHOUTPUT("Unrecognized op code.\n");
				break;
		}
		pos = tagEnd;

		return scriptErr;
	}
}

int ScriptThread::DoTags(int fnum)
{
	int res = playOK;
	while ( curFrame < fnum && res == playOK ) {
		res = DoTag();
	}
	return res;
}

int ScriptThread::DrawFrame(int fnum, BOOL seek)
{
	if ( scriptErr ) 
		return scriptErr;

	if ( !player->gotHeader ) 
		return playNeedData;

	if ( !rootObject ) {
		// Create a root object on the display for this player
		SCharacter* ch = player->CreateCharacter(ctagThreadRoot);
		if ( ch ) {
			// Set up object
			ch->type = rootChar;
		} else {
			ch = player->FindCharacter(ctagThreadRoot);
		}

		if ( ch ) {
			PlaceInfo info;
			info.flags = splaceCharacter;
			MatrixIdentity(&info.mat);	
			info.cxform.Clear();
			info.depth = layerDepth;
			info.character = ch;
			info.ratio = 0;
			info.name = 0;
			rootObject = display->PlaceObject(&display->root, &info);
		}
		if ( !rootObject )
			return noMemErr;

		rootObject->thread = this;
	}

#ifdef EDITTEXT
	SPlayer* splayer = player->splayer;
	if ( splayer )
		splayer->saveFocus.Save(splayer);
#endif

	int res = playOK;
	if ( seek ) {
		if ( curFrame > fnum ) {
			// Seek backwards

			// Rewind the movie...
			pos = startPos;		// seek to the beginning
			curFrame = -1;
			currentLabel = 0;
			atEnd = false;
		
			// Marks all objects to be removed unless they are added again
		 	//rootObject->FreeChildren();
			display->BeginHold(rootObject);	

			// Skip over frames
			seeking = true;
            justSeeked = true;      // tell mp3 sound stream to adjust itself
			res = DoTags(fnum-1);
			seeking = false;

			// Do the destination frame
			res = DoTags(fnum);

			display->FinishHold();

		} else {
			// Seek forwards

			// Skip over frames
			seeking = true;
            justSeeked = true;      // tell mp3 sound stream to adjust itself
			res = DoTags(fnum-1);
			seeking = false;

			// Do the destination frame
			res = DoTags(fnum);
		}
	} else {
		// Build the frame
		res = DoTags(fnum);
	}

#ifdef EDITTEXT
	if ( splayer )
		splayer->saveFocus.Clear();
#endif

	return res;
}

// delta & bForceRebuild are optional parameters added for the
// flash asset xtra. they are never used by the flash player, 
// so in flash, delta = 1 and bForceRebuild = false.
void ScriptThread::DoFrame(int delta, BOOL bForceRebuild)
{
// #ifdef SPRITE_XTRA
// 	// If we're the sprite xtra, we still want to render
// 	// even if paused - so here we just ignore the frameDelta
// 	// and hang on the current frame
// 	if ( !playing )
// 		delta = 0;
// #endif

	if (!playing && !bForceRebuild)
		return;

	int thisFrame = GetFrame();				// !!sprite_xtra
	int frameNum =  thisFrame + delta;

	// Check to stall if the next frame is not loaded
	if ( this == player && frameNum > player->numFramesComplete ) {
		frameNum = player->numFramesComplete;
	}

	int res;
	
	if (bForceRebuild)
	{
		// we do not advance in the forceRebuild case either.
		curFrame = thisFrame + 1;   // hack to get it to rewind, forcing rebuild of frame
 		res = DrawFrame(thisFrame /* + delta */, true);
	}
	else
		res = DrawFrame(frameNum, false);

	if ( res == ScriptPlayer::playNeedData ) {
// #ifdef SPRITE_XTRA
// 		// If we're the sprite xtra, we need to draw something
// 		// no matter what - so redraw the current frame.
// 		curFrame = thisFrame + 1;			// hack to get it to rewind
// 		res = DrawFrame(thisFrame, true);
// 		if (res == ScriptPlayer::playNeedData)
// 		{
// 			// !!@ - should just re-blit what was last in the buffer
// 			// (only good if not using transparency, though)
// 			return;
// 		}
// #else
		return;
// #endif // SPRITE_XTRA
	} else if ( res < ScriptPlayer::noErr ) {
		// We got an error, let the thread terminate
		#ifdef SOUND
		StopStream();
		#endif
		playing = false;
		return;
	} else if ( res == ScriptPlayer::playAtEnd ) {
		#ifdef SOUND
		StopStream();
		#endif
		if ( loop && GetFrame() != 0 ) {
			// Never loop a single frame...
			res = DrawFrame(0, true);
		} else {
			playing = false;
		}
	}

#if FRAMESPERSEC
    // enable this section in order to see the frame rate in the debug window
    {
        static  long startTime = GetTimeMSec();
        static  long frameCount = 0;

        // look at only root thread
        if (rootObject == player->splayer->player.rootObject)
        {
            frameCount += 1;
            // report framerate every nth frame
//             if (!(frameCount & 0xf))
            if ( frameCount == 12 )
            {
                long currTime = GetTimeMSec();
                long fps;
                
                fps = (frameCount * 1000) / (currTime - startTime + 1);	// avoid divide by 0 for slow system clocks

				#ifdef DEBUG
					FLASHOUTPUT( "fps = %3d\n", fps );
				#else
					printf( "fps = %3d\n", fps );
				#endif
					
				frameCount = 0;
				startTime  = GetTimeMSec();
            }
        }
    }
#endif

}

// Start and stop the main layer
void ScriptThread::Play(BOOL rewind)
{
    // tell mp3 sound stream to adjust itself
    justSeeked = true;

	// Rewind if they are at the end
	if ( rewind && !loop && curFrame >= numFrames-1 )
		Seek(0);

	playing = true;
}

void ScriptThread::StopPlay()
{
	playing = false;

	#ifdef SOUND
	StopStream();
	#endif
}

void ScriptThread::Seek(int f)
{
	StopPlay();
	if ( f < 0 ) 
		f = 0;
	DrawFrame(f, true);
}

int ScriptThread::FindLabel(char* label)
{
	BOOL found = false;
	int frameNum = 0;

	S32 savedPos = pos;
	pos = startPos;
	while ( true ) {
		int code = GetTag(len);
		if ( code == stagShowFrame ) {
			// Advance to the next frame
			frameNum++;

		} else if ( code == stagFrameLabel ) {
			// See if we found the label
			if ( StrEqual(label, (char*)(script+pos)) ) {
				found = true;
				break;
			}
		} else if ( code < 0 ) {
			// We hit the end and did not find the label
			break;
		}

		pos = tagEnd;	// advance to the next tag
	}
	pos = savedPos;

	return found ? frameNum : -1;
}

// #ifdef SPRITE_XTRA
// // !!SPRITE_XTRA - added function
// // Get frame label for specified frame
// // Returns TRUE if label actually exists on that frame
// 
// BOOL ScriptThread::GetFrameLabel(int frameNum, char* label)
// {
// 	BOOL found = false;
// 	int testFrame = 0;
// 
// 	S32 savedPos = pos;
// 	pos = startPos;
// 
// 	// assume no label
// 	*label = 0;
// 
// 	while ( true ) {
// 		int code = GetTag(len);
// 		if ( code == stagShowFrame ) {
// 			// Advance to the next frame
// 			testFrame++;
// 
// 		} else if ( code == stagFrameLabel ) {
// 			// See if we found the label
// 			if (testFrame == frameNum)
// 			{
// 				// found label - copy to caller's buffer
// 				strcpy( label, (char*)(script+pos) );
// 				// null terminate
// 				label[ strlen( (char*)(script+pos) ) ] = 0;
// 				found = true;
// 				break;
// 			}
// 		} else if ( code < 0 ) {
// 			// We hit the end and did not find the label
// 			break;
// 		}
// 
// 		// passed the target frame?
// 		if (testFrame > frameNum)
// 			break;
// 
// 		pos = tagEnd;	// advance to the next tag
// 	}
// 	pos = savedPos;
// 
// 	return found;
// }
// #endif

//
// Script Player
//

ScriptPlayer::ScriptPlayer() : 
				characterAlloc(sizeof(SCharacter), 32, true, 0x66666666)
{
	player = this;

	memset(charIndex, 0, sizeof(charIndex));
	jpegInfo = 0;
	version = 0;

// 	#ifdef WIN16
// 	scriptHand = 0;
// 	#endif

// 	#ifdef FSSERVER
// 	lostCharacter = false;
// 	lostCharProc = 0;
// 
// 	lostCh = CreateCharacter();
// 	lostCh->next = 0;
// 	lostCh->player = this;
// 	lostCh->tag = 0;
// 	lostCh->type = 99;
// 	lostCh->data = 0;
// 	RectSetEmpty(&lostCh->bounds);
// 	#endif

	RectSet(0,0,oneInch, oneInch, &frame);

	splayer = 0;
	nextLayer = 0;
	stream = 0;
    scriptRefCount = 0;
    
	url = 0;

	#ifdef SOUND
	mute = false;
	#endif

	variableLoader = false;
	variableSprite = 0;
	variableLayer = -1;
	
	startPos = 0;
	len = 0;
// #ifdef SPRITE_XTRA
// 	naturalFrameDelay = 0;	// natural frame delay associated with movie	!!sprite_xtra
// 	S32 hostFrameDelay = 0;
// #endif
// 	#if defined(GENERATOR) || defined(_CELANIMATOR)
// 	solidGradients = false;
// 	#endif

	ClearScript();
}

ScriptPlayer::~ScriptPlayer()
{
	ClearScript();

	FreeStr(url);
	FreeStr(variableSprite);
}


void ScriptPlayer::FreeCharacter(SCharacter* c)
{
	ReleaseData(c->data);
	switch ( c->type ) {
		//case shapeChar: {
		//} break;

		//case morphShapeChar: {
		//} break;

		case bitsChar: {
			// Free the uncompressed bitmap
// 			#ifdef _CELANIMATOR
// 			if ( c->data != 0 )	// don't free pointer bitmaps
// 			#endif

			c->bits.PIFree();
		} break;

		case buttonChar: {
			ReleaseData(c->button.soundData);
			ReleaseData(c->button.cxformData);
		} break;

		case fontChar: {
			ReleaseData(c->font.infoData);
		} break;

		//case textChar: {
		//} break;

		#ifdef SOUND
		case soundChar: {
			// Stop the sound if it is playing
// #ifdef SPRITE_XTRA
// 			CSoundMix::Mixer()->FreeSound(&c->sound);
// #else
			splayer->theSoundMix.FreeSound(&c->sound);
// #endif
		} break;
		#endif
	}

	characterAlloc.Free(c); 
}

void ScriptPlayer::FreeCache()
{
	{// Free the characters
		SCharacter** index = charIndex;
		for ( int i = 0; i < charIndexSize; i++, index++ ) {
			SCharacter* ch = *index;
			while ( ch ) {
				SCharacter* nextChar = ch->next;
				if ( ch->type == bitsChar && ch->bits.lockCount == 0 ) {
					// Free the uncompressed bitmap
					ch->bits.PIFree();
				}
				ch = nextChar;
			}
		}
	}

	characterAlloc.FreeEmpties();
}

void ScriptPlayer::FreeAll()
// Free the current script context
{
	if ( display && rootObject ) {
	 	//display->RemoveList(rootObject->FreeChildren();
		display->RemoveObject(&display->root, layerDepth);	// remove the RootObject
		rootObject = 0;
	}

	{// Free the characters
		SCharacter** index = charIndex;
		for ( int i = 0; i < charIndexSize; i++, index++ ) {
			SCharacter* ch = *index;
			while ( ch ) {
				SCharacter* nextChar = ch->next;
				FreeCharacter(ch);
				ch = nextChar;
			}
			*index = 0;
		}
	}

// 	#ifdef FSSERVER
// 
// 	#else
	// Reset the script attributes
	pos = startPos;
	curFrame = -1;
	currentLabel = 0;
	atEnd = false;
// 	#endif

#ifdef USE_JPEG
	// Free the JPEG decompression info
	if ( jpegInfo ) {
		jpeg_destroy_decompress(&jpegInfo->cinfo);
		delete jpegInfo;//free(jpegInfo);
		jpegInfo = 0;
	}
#endif
}

void ScriptPlayer::ClearScript()
{
	// Cancel a streaming operation into this player
	if ( splayer && stream )
		splayer->CloseStream(stream);
// 		splayer->CloseStream(this);

	// Clear an existing script
	FreeAll();

// 	#ifdef FSSERVER
	// Handle a server based script

// 	#else
// 	#ifndef SPRITE_XTRA
	// For sprite xtra, the xtra owns the script handle
	// as it may be shared by several players simultaneously.

	// Handle a local script
	if ( script ) {
		#ifdef WIN16
		GlobalUnlock(scriptHand);
		GlobalFree(scriptHand);
		scriptHand = 0;
		#else
        if (ScriptSubRefCount() == 0) {
		    delete [] script;//free(script);
		}
		#endif
		//script = 0;
	}
// 	#endif // SPRITE_XTRA

	ClearState();	// this must happen after the script is deleted

	ClearVariables();

	// Reset the script attributes
	gotHeader = false;
	atEnd = false;

	headerLen = 0;
	scriptLen = -1;

	numFramesComplete = -1;
	numFramesCompletePos = 0;
// 	#endif

	stream = 0;
}


//
// Manage the Character List
//

SCharacter* ScriptPlayer::FindCharacter(U16 tag)
{
 	SCharacter* ch = charIndex[tag & charIndexMask];
	while ( ch && ch->tag != tag )
		ch = ch->next;
	
	#ifdef FSSERVER
	if ( !ch && lostCharProc && tag != 65535 ) {
		lostCharacter = true;
		lostCharProc(lostCharData, tag);
	}
	#endif

	return ch;
}

SCharacter* ScriptPlayer::CreateCharacter(U16 tag)
// Create a character, add it to the list and set the tag
{
	{// Look to see if the character is already defined
		SCharacter* ch = charIndex[tag & charIndexMask];
		while ( ch && ch->tag != tag )
			ch = ch->next;
		if ( ch ) 
			return 0;	// the character is alrady defined
	}

	SCharacter* ch = CreateCharacter();
	if ( ch ) {
		// Add to list
		SCharacter** link = &charIndex[tag & charIndexMask];
		ch->next = *link;
		*link = ch;

		// Set up object
		ch->player = this;
		ch->tag = tag;
		ch->tagCode = 0;
		ch->data = 0;
		RectSetEmpty(&ch->bounds);
	}
	return ch;
}

void ScriptPlayer::FreeCharacter(U16 tag)
{
	SCharacter** chP = &charIndex[tag & charIndexMask];
	for (;;) {
		SCharacter* ch = *chP;
		if ( !ch ) break;
	 	if ( ch->tag == tag ) {
			*chP = ch->next;
			FreeCharacter(ch);
		}
        else
            chP = &ch->next;
	}
}


#ifdef FSSERVER
U8 * ScriptThread::AttachData(S32 start, S32 end)
{
	S32 len = end-start;
	U8 * data = new U8[len];
	if ( data ) {
		memcpy(data, script+start, len);
	}
	return data;
}

void ScriptThread::ReleaseData(U8 *& data)
{
	delete [] data;
	data = 0;
}

void ScriptPlayer::SetHeader(void* data, int len)
// Parse the header block
{
	script = (U8*)data;

	// get the script len
	if ( script[0] != 'F' || script[1] != 'W' || script[2] != 'S' ) {
		FLASHASSERT(false);
		//scriptErr = badHeaderErr;
		return;
	}
	version = script[3];

	S32 scriptLen = (U32)script[4] | ((U32)script[5]<<8) | 
				((U32)script[6]<<16) | ((U32)script[7]<<24);
	//scriptLen -= 8; // note we subtract off the tag and length data

	FLASHASSERT(scriptLen < (3000000));

	pos = 8;
	
	GetRect(&frame);
	frameRate = (SFIXED)GetWord()<<8;
	frameDelay = FC(1000)/frameRate;
	numFrames = GetWord();
}

void ScriptPlayer::SetObject(void* data, int len)
// Add a character definition
{
	script = (U8*)data;
	pos = 0;

	while ( pos < len ) {
		int code = GetTag();

		switch ( code ) {
			case stagDefineShape: 
			case stagDefineShape2:
			case stagDefineShape3:
				DefineShape(); 
				break;
			case stagDefineBits:
			case stagDefineBitsLossless:
			case stagDefineBitsJPEG2:
			case stagDefineBitsLossless2:
			case stagDefineBitsJPEG3:
				DefineBits();
				break;
			case stagJPEGTables:
				DefineJPEGTables();
				break;
			case stagDefineButton:
			case stagDefineButton2:
				DefineButton();
				break;
			case stagDefineSprite:
				DefineSprite();
				break;
			case stagDefineFont:
				DefineFont();
				break;
			case stagDefineFont2:
				DefineFont2();
				break;
			case stagDefineFontInfo:
				DefineFontInfo();
				break;
			case stagDefineText:
			case stagDefineText2:
				DefineText();
				break;
#ifdef EDITTEXT
			case stagDefineEditText:
				DefineEditText();
				break;
#endif
			#ifdef SOUND
			case stagDefineSound:
				DefineSound();
				break;
			case stagDefineButtonSound:
				DefineButtonExtra(0);
				break;
			#endif

			case stagDefineButtonCxform:
			case stagDefineButtonCxform2:
				DefineButtonExtra(1);
				break;

			default:
				//FLASHASSERT(false);
				//FLASHOUTPUT("Unrecognized op code.\n");
		}

		pos = tagEnd;
	}
	FLASHASSERT(pos == len);
}

void ScriptPlayer::RecoverLost()
{
	if ( lostCharacter ) {
		lostCharacter = false;

		display->FreeCache();

		// Build a list of all lost objects
		SObject list[20];
		int n = 0;
		for ( SObject* obj = display->objects; obj; obj = obj->next ) {
			if ( obj->character->type == lostChar ) {
				list[n++] = *obj;
				if ( n >= 20 ) break;
			}
		}

		// Remove the lost objects
		for ( int i = 0; i < n; i++ ) {
			display->RemoveObject(list[i].id);
		}

		lostCharacter = false;

		// Place the objects again
		for ( int j = 0; j < n; j++ ) {
			SCharacter* ch = FindCharacter((U16)(list[j].id>>16));
			#ifdef FSSERVER
			if ( !ch )
				ch = lostCh;
			#endif
			display->PlaceObject(ch, list[j].id, &list[j].mat, &list[j].cxform);
		}
	}
}

void ScriptPlayer::SetFrame(void* data, int len, BOOL seek)
// Parse the commands to create a frame
{
	script = (U8*)data;
	pos = 0;

	BOOL syncFrame = false;

	while ( pos < len ) {
		int code = GetTag();

		switch ( code ) {
			case stagEnd: 
				//atEnd = true;
				break;
			case stagShowFrame:
				return;
				//curFrame++;
				//break;
			case stagPlaceObject:
				if ( syncFrame ) {
					U16 tag = GetWord();
					U16 depth = GetWord() + layerDepth;

					// Get the position
					MATRIX mat;
					GetMatrix(&mat);

					// Get a color transform
					ColorTransform cx;
					if ( pos < tagEnd ) {
						GetColorTransform(&cx);
					} else {
						cx.Clear();
					}

					U32 id = ((U32)tag <<16) | depth;
					SObject* obj = display->FindObject(id);

					SCharacter* ch = FindCharacter(tag);
					#ifdef FSSERVER
					if ( !ch )
						ch = lostCh;
					#endif
					if ( ch ) {
						if ( obj && MatrixEqual(&obj->mat, &mat, 0) && Equal(&obj->cxform, &cx) && obj->character == ch ) {
							// Just leave the object in place
							obj->syncFlag = 2;
						} else {
							// This is a new object
							display->PlaceObject(ch, id, &mat, &cx);
						}
					}
				} else {
					PlaceObject();
				}
				break;
			case stagSyncFrame: {
				syncFrame = true;
				for ( SObject* obj = display->objects; obj; obj = obj->next ) {
					obj->syncFlag = 1;
					if ( obj->buttonState != 0 ) // clear out any button objects
						display->UpdateButton(obj, 0);
				}
			} break;
			case stagFreeCharacter: {
				U16 tag = GetWord();
				FreeCharacter(tag);
			} break;
			case stagRemoveObject:
			case stagRemoveObject2:
				RemoveObject();
				break;
			case stagFreeAll:
				FreeAll();
				break;
			case stagSetBackgroundColor:
				display->SetBackgroundColor(GetColor());
				break;
			case stagDoAction:
				if ( !seek )
					PushAction(script+pos);
				break;
			#ifdef SOUND
			case stagStartSound:
				if ( !seek )
					StartSound();
				break;
			//case stagStopSound:
			//	StopSound();
			//	break;
			#endif

			default:
				//FLASHASSERT(false);
				//FLASHOUTPUT("Unrecognized op code.\n");
		}

		pos = tagEnd;
	}

	if ( syncFrame ) {
		SObject** objP = &display->objects;
		for (;;) {
			SObject* obj = *objP;
			if ( !obj ) break;
			if ( obj->syncFlag == 1 ) {
				// Remove from display list
				// Remove the object
				if ( display->button == obj )
					display->button = 0;
				if ( obj->drawn )
					RectUnion(&display->devDirtyRgn, &obj->devBounds, &display->devDirtyRgn);	// add to the dirty region
				*objP = obj->next;
				obj->Free();
				display->FreeObject(obj);
			} else {
				objP = &obj->next;
			}
		}
		for ( SObject* obj = display->objects; obj; obj = obj->next ) {
			if ( obj->syncFlag == 2 && 
					obj->character->type == buttonChar ) {
				int state = upState;
				if ( obj->id == display->buttonId ) {
					// The button is currently being tracked
					display->button = obj;
					state = display->buttonState;
				}
				display->UpdateButton(obj, state);
			}
		}
	}
}

#else

// #ifdef SPRITE_XTRA
// // separate out header reading so we can call it
// // directly from the flash asset code
// BOOL ScriptPlayer::ReadHeader()
// {
// 	if ( scriptErr ) return false;
// 
// 	if ( gotHeader ) 
// 		return true;	// we already have the header
// 
// 	if ( len < 17+2+2 ) 	// we don't have enough data
// 		return false;
// 
// 	// Get the Frame - NOTE be sure to adjust the len above if we add any header data
// 	GetRect(&frame);
// 	frameRate = (SFIXED)GetWord()<<8;
// 	naturalFrameDelay = FC(1000)/frameRate;		// !!sprite_xtra   was 'frameDelay'
// 	frameDelay = (hostFrameDelay > 0) ? hostFrameDelay : naturalFrameDelay;
// 	numFrames = GetWord();
// 
// 	numFramesCompletePos = startPos = pos;
// 	curFrame = -1;
// 	gotHeader = true;
// 
// 	return true;
// }
// #endif // SPRITE_XTRA

void DecodeQueryString(ScriptThread *thread, char *str);

void ScriptPlayer::SetDataComplete()
{
	if (variableLoader) {
		if (script) {
			script[len] = 0;

			// This stream is for a Load Sprite or a Load Variable action.
			ScriptThread *thread = 0;

			if (variableSprite) {
				// Loaded to a target path
				thread = splayer->FindTarget(splayer->display.root.bottomChild->bottomChild, variableSprite);
				if (!thread) {
					int layerNum = LayerNum(variableSprite);
					if (layerNum >= 0) {
						thread = splayer->OpenLayer(layerNum, TRUE);
					}
				}
			} else {
				// Loaded to a layer number
				ScriptPlayer *p = splayer->OpenLayer(variableLayer, TRUE);
				thread = p;

				// If the variables were loaded to a non-existent layer,
				// we need to create a root object for the layer.
				if ( !p->rootObject ) {
					// Create a root object on the display for this player
					SCharacter* ch = p->CreateCharacter(ctagThreadRoot);
					if (ch) {
						// Set up object
						ch->type = rootChar;
					} else {
						ch = p->FindCharacter(ctagThreadRoot);
					}
					if ( ch ) {
						PlaceInfo info;
						info.flags = splaceCharacter;
						MatrixIdentity(&info.mat);	
						info.cxform.Clear();
						info.depth = variableLayer;
						info.character = ch;
						info.ratio = 0;
						info.name = 0;
						p->rootObject = splayer->display.PlaceObject(&splayer->display.root, &info);
					}
					if ( p->rootObject ) {
						p->rootObject->thread = thread;
					}
				}
			}

			// cleanup
			FreeStr(variableSprite);
			variableSprite = 0;

			if (thread) {
				DecodeQueryString(thread, (char*)script);
			}
			delete [] script;
			script = 0;
			len = scriptLen = 0;
		}
		delete this;
	}
}

void ScriptPlayer::PushDataBuf(U8* data, S32 chunkLen)
// return number of bytes pushed
{
	if (variableLoader) {
		if (chunkLen + len > scriptLen) {
			scriptLen = chunkLen + len + VAR_INCR + 1;
			U8 *newBuffer = new U8[scriptLen];
			if (script) {
				memcpy(newBuffer, script, len);
				delete [] script;
			}
			script = newBuffer;
		}
		memcpy(script + len, data, chunkLen);
		len += chunkLen;
		return;
	}

	if ( scriptErr ) return;

	// Get the total script length from the header info
	if ( scriptLen < 0 ) {
		// Copy the first 8 bytes to our temp buffer
		S32 n = Min(8-headerLen,chunkLen);
		FLASHASSERT(n>0);
// 		hmemcpy(headerBuf+headerLen, data, n);
		memcpy(headerBuf+headerLen, data, n);
		data+=n;
		chunkLen -= n;
		headerLen += n;

		if ( headerLen == 8 ) {
			// get the script len
			if ( headerBuf[0] != 'F' || headerBuf[1] != 'W' || headerBuf[2] != 'S' ) {
				//FLASHASSERT(false);
				scriptErr = badHeaderErr;
				return;
			}
			version = headerBuf[3];

			scriptLen = (U32)headerBuf[4] | ((U32)headerBuf[5]<<8) | 
						((U32)headerBuf[6]<<16) | ((U32)headerBuf[7]<<24);
			//scriptLen -= 8; // note we subtract off the tag and length data

			if ( scriptLen < 8 )  {
				scriptErr = badHeaderErr;
				return;
			}

// 			#ifdef WIN16
// 			scriptHand = GlobalAlloc(GMEM_FIXED, scriptLen);
// 			if ( scriptHand )
// 				script = (U8 *)GlobalLock(scriptHand);
// 			#else
			script = new U8[scriptLen];
// 			#endif
			if ( !script ) {
				scriptErr = noMemErr;
				return;
			}
			memcpy(script, headerBuf, 8);
			len = 8;
			pos = 8;
		} else {
			// Wait for more data
			return;
		}
	}

	if ( len+chunkLen > scriptLen ) {
		// Too much data is a forgiveable error
		FLASHASSERT(false);
		chunkLen = Min(chunkLen, scriptLen-len);
	}

// 	hmemcpy(script+len, data, chunkLen);
	memcpy(script+len, data, chunkLen);
	len += chunkLen;

	// Check to parse the header
// #ifdef SPRITE_XTRA
// 	ReadHeader();
// #else
	if ( !gotHeader && len >= 17+2+2 ) {
		// Get the Frame - NOTE be sure to adjust the len above if we add any header data
		GetRect(&frame);
		frameRate = (SFIXED)GetWord()<<8;
		frameDelay = FC(1000)/frameRate;
		numFrames = GetWord();

		numFramesCompletePos = startPos = pos;
		curFrame = -1;
		currentLabel = 0;
		gotHeader = true;
	}
// #endif

	// See how many frames have been loaded
	if ( gotHeader ) {
		if ( len >= scriptLen ) {
			// The script is all here
			numFramesComplete = 16000;
		} else {
			// Scan for show frame tags
			SParser parser;
			parser.Attach(script, numFramesCompletePos);
			while ( true ) {
				int code = parser.GetTag(len);
				if ( code < 0 ) 
					break;	// we need more data before we can process this tag
				
				numFramesCompletePos = parser.pos = parser.tagEnd;

				if ( code == stagShowFrame )
					numFramesComplete++;
			}
		}
	}
}

void ScriptPlayer::PushDataComplete()
{
	if ( splayer ) {
		// See if we can draw the frame for the primary layer
		if ( this == &splayer->player && !splayer->loaded ) {
			// Keep trying on until we have a complete first frame
			if ( DrawFrame(0, true) == ScriptPlayer::playOK || 
				  ScriptComplete() ) {
				splayer->loaded = true;

				if ( numFrames > 1 ) {
					//Run();	// start the player thread now...
				} else {
					if ( splayer->autoQuality )
						splayer->highQuality = true; // Always use high-quality for single frames
				}

				splayer->Run();	// start the player thread now...
				splayer->DoActions();	// do any actions once the first frame is complete...
			}
			splayer->SetCamera(SPlayer::updateLazy); 
		}
	}
}

#endif

//
// String, mem and float Utils
//

double MyLdExp(double x, int n)
{
    if (n == 0)
        return x;

    if (n == 1)
        return x + x;

    if (n > 0)
	    return x * (1 <<  n);
    else
        return x / (1 << n);
}

double CubeRoot(double x)
{
	const double CBRT2 = 1.2599210498948731648; /* 2^(1/3) */
	const double SQR_CBRT2 = 1.5874010519681994748; /* 2^(2/3) */

	static const double factor[5] =
	{
	  1.0 / SQR_CBRT2,
	  1.0 / CBRT2,
	  1.0,
	  CBRT2,
	  SQR_CBRT2
	};

	double xm, ym, u, t2;
	int xe;

	/* Reduce X.  XM now is an range 1.0 to 0.5.  */
	xm = frexp (fabs (x), &xe);

	/* If X is not finite or is null return it (with raising exceptions
	 if necessary.  */
	if (xe == 0 && (x == 0.0 /*|| isnan (x) || isinf (x)*/))
	return x + x;

	u = (0.354895765043919860
	  + ((1.50819193781584896
	 + ((-2.11499494167371287
		+ ((2.44693122563534430
		   + ((-1.83469277483613086
		  + (0.784932344976639262 - 0.145263899385486377 * xm) * xm)
		  * xm))
		   * xm))
		* xm))
	 * xm));

	t2 = u * u * u;

	ym = u * (t2 + 2.0 * xm) / (2.0 * t2 + xm) * factor[2 + xe % 3];

	return MyLdExp(x > 0.0 ? ym : -ym, xe / 3);
}

char* SkipPrefix( char* str, char* pre )
// If str begins with pre, return the first char after in str
{
	while ( true ) {
		// Map to uppercase
		unsigned char s = *str;
		if ( s >= 'a' && s <= 'z' )
			s -= 'a' - 'A';
		unsigned char p = *pre;
		if ( p >= 'a' && p <= 'z' )
			p -= 'a' - 'A';

		// See if the characters are not equal or we hit the end of the strings
		if ( s != p || !s || !p ) 
			break;

		*pre++; *str++;
	}
	return *pre == 0 ? str : 0;
}

BOOL StrEqual( const char* a, const char* b) 
{
	if ( !a || !b ) 
		return false;
	char* c = SkipPrefix( (char*)a, (char*)b); 
	return c && *c == 0; 
}

int ToInt(const char* c)
{
	int v = 0;
	while ( *c >= '0' && *c <= '9' ) {
		v = v*10 + *c-'0';
		c++;
	}
	return v;
}


void FreeStr(char* s)
{
	delete [] s;
}

char* CreateStr( const char* s)
{
	if (!s) {
		return 0;
	}
	int len = strlen(s)+1;
	char* c = new char[len];
	if (c) {
		memcpy(c, s, len);
	}
	return c;
}

char *CreateStr( const char *s, int count)
{
	if (!s) {
		return 0;
	}
	char *c = new char[count+1];
	if (c) {
		memcpy(c, s, count);
		c[count] = '\0';
	}
	return c;
}

//
// Support for Variables
//

ScriptVariable* ScriptThread::FindVariable(char* name)
{
	for ( ScriptVariable* v = firstVariable; v; v = v->next ) {
		if ( StrEqual(name, v->name) ) 
			return v;
	}
	return 0;
}

void ScriptThread::SetVariable(char* name, char* value, BOOL updateFocus)
{
	// Find the variable
	ScriptVariable* v = FindVariable(name);

	if ( !v ) {
		// Create a new variable
		v = new ScriptVariable;
		if ( !v ) return;

		v->next = 0;
		v->name = CreateStr(name);
		v->value = 0;

		if (lastVariable) {
			lastVariable->next = v;
		} else {
			firstVariable = v;
		}
		lastVariable = v;
	}

	// Set the value
	FreeStr(v->value);
	v->value = CreateStr(value);

#ifdef EDITTEXT
	// We need to search the display for any edit text items that are mapped to this variable and update them
	display->root.UpdateEditText(this, name, value, updateFocus);
#endif
	if(((NativePlayerWnd *)splayer)->flashOK==666)
		((NativePlayerWnd *)splayer)->flash->varChange(name, value);
}
