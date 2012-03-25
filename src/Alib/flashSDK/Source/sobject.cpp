/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright © Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 
****************************************************************************/
//	990325	mnk	"fixed1" -> "fixed_1"

#include "stdafx.h"

#include "sobject.h"

#include "stags.h"
#include "splay.h"
#include "sstroker.h"
#include "morphinter.h"
#include "splayer.h"
#include "edittext.h"

#ifdef EDITTEXT
BOOL PlayerIsFontAvailable(const char *fontName);

void ResolveFontName(char *result, const char *fontName)
{
	const char *ptr;
	if (!strcmp(fontName, SANS_NAME)) {
		ptr = SANS_DEVICE_NAME;
	} else if (!strcmp(fontName, SERIF_NAME)) {
		ptr = SERIF_DEVICE_NAME;
	} else if (!strcmp(fontName, TYPEWRITER_NAME)) {
		ptr = TYPEWRITER_DEVICE_NAME;
 	} else if (!strcmp(fontName, GOTHIC_NAME)) {
		if (PlayerIsFontAvailable(GOTHIC_DEVICE_NAME_1)) {
			ptr = GOTHIC_DEVICE_NAME_1;
		} else {
			ptr = GOTHIC_DEVICE_NAME_2;
		}
	} else if (!strcmp(fontName, GOTHIC_MONO_NAME)) {
		ptr = GOTHIC_MONO_DEVICE_NAME;
	} else if (!strcmp(fontName, MINCHO_NAME)) {
		ptr = MINCHO_DEVICE_NAME;
 	} else {
 		ptr = fontName;
 	}
 	strcpy(result, ptr);
}
#endif

void SCharacterParser::AddCurve(P_CURVE src)
{
	CURVE c;
	S32 dir;
	
	c = *src;

	// Sort the anchors
	if ( c.anchor1.y <= c.anchor2.y ) {
		dir = 1;
	} else {
		dir = -1;
		Swap(c.anchor1, c.anchor2, SPOINT);
	}
	
	if ( !c.isLine ) {
		if ( c.control.y < c.anchor1.y || c.control.y > c.anchor2.y ) {
			// Handle a minimum or maximum
		
			// If it is close, just adjust the curve
			if ( c.control.y < c.anchor1.y && c.anchor1.y - c.control.y < 3 ) {
				c.control.y = c.anchor1.y;
			
			} else if ( c.control.y > c.anchor2.y && c.control.y - c.anchor2.y < 3 ) {
				c.control.y = c.anchor2.y;   
			
			} else {
				SCOORD a, b;
				CURVE c1, c2;
				
				// Use the curve with the original order
				a = src->anchor1.y - 2*src->control.y + src->anchor2.y;
				b = src->anchor1.y - src->control.y;
			
				// See if b/a is in the range 0..1
				// a and b must have the same sign and abs(b) <= abs(a)
				FLASHASSERT(((a > 0 && b >= 0) || (a < 0 && b <= 0)) && Abs(b) <= Abs(a));
				
				CurveDivide(src, FixedDiv(b, a), &c1, &c2);
				if ( ++depth > 16 ) {
					FLASHASSERT(false);
					return;
				}
				AddCurve(&c1);
				AddCurve(&c2);
				depth--;
				return;
			}
		}

		// Split any curves that are too long
		if ( c.anchor2.y - c.anchor1.y > 256 ) {
			CURVE c1, c2;
			CurveDivide(src, &c1, &c2);
			if ( ++depth > 16 ) {
				FLASHASSERT(false);
				return;
			}
			AddCurve(&c1);
			AddCurve(&c2);
			depth--;
			return;
		}
	}

	if ( c.anchor1.y == c.anchor2.y ) 
		return;	// this is a horizontal edge, we can ignore it

	if ( preclip ) {
		SRECT b;

		// Check to clip
		CurveBounds(&c, &b);

		// If the curve is above or below, we can completely ignore it
		// We assumme that the largest image will be 4000x4000 pixels
		if ( b.ymax < -16000 || b.ymin > 16000 ) return;

		// Pre-clip to prevent overflow...
		if ( b.xmin < -32000 || b.xmax > 32000 || b.ymin < -32000 || b.ymax > 32000 ) {
			if ( b.xmax < -16000 ) {
				// The curve is completely off to the left, make a vertical line
				MakeVertLine(&c, -32000);
			} else if ( b.xmin > 16000 ) {
				// Curve is completely off to the right, make a vertical line
				MakeVertLine(&c, 32000);
			} else if ( depth < 16 ) {
				// Split the curve
				CURVE c1, c2;
				CurveDivide(src, &c1, &c2);
				depth++;
				AddCurve(&c1);
				AddCurve(&c2);
				depth--;
				return;
			}
		}
	}
	
	// Set up a new edge
	REdge* edge = display->CreateEdge();
	if ( !edge ) return;

    // Set up the curve
	edge->Set(&c);
	edge->dir = (S8)dir;
	
	// Track the colors
	edge->fillRule = fillRule;
	edge->color1 = color1;
	edge->color2 = color2;
	
	// Add to the edge list
	edge->nextObj = obj->edges;
	obj->edges = edge;
}

RColor* CreateClipColor(DisplayList* display, RColor** colorList)
{
	RColor* color = display->CreateColor();
	if ( !color ) {
		//FLASHASSERT(false);
		return 0;
	}
	color->SetUp(&display->raster);

	color->nextColor = *colorList;
	*colorList = color;

	color->order = 0;

	// A solid color
	color->colorType = colorClip;

	color->transparent = true;

	// Build the pattern here also
	if ( display->raster.bits )
		color->BuildCache();

	return color;
}

void SCharacterParser::MakeClipper()
{
	// Create a clip color
	RColor* clipColor = CreateClipColor(display, colorList);

	{// Set all of the fills to our clip color
		for ( int i = 1; i <= nFills; i++ ) {
			fillIndex[i] = clipColor;
		}
	}

	{// We don't clip with lines
		for ( int i = 1; i <= nLines; i++ ) {
			lineIndex[i].color = 0;
		}
	}
}

void SCharacterParser::BuildEdges(BOOL getStyles, SRECT* devBounds)
{
	depth = 0;

	// Set up a stroker for any lines
	SStroker stroker(obj->display, &obj->edges);
	stroker.antialias = obj->display->antialias;

	stroker.preclip = preclip = devBounds && (
				devBounds->xmin < -32000 || devBounds->xmax > 32000 || 
				devBounds->ymin < -32000 || devBounds->ymax > 32000
			  );

	raster = &obj->display->raster;
	if ( !raster->bits ) 
		raster = 0;
	colorList = &obj->colors;	// we want to keep the colors
	if ( getStyles ) {
		if ( !GetStyles() ) 
			return;
		if ( obj->clipDepth )
			MakeClipper();
	} else {
		// See how large we need the fill and line tags to be
		InitBits();
		nFillBits = (int)GetBits(4);
		nLineBits = (int)GetBits(4);
	}
	
	// Get the edges
	BOOL hasFill, hasLine;

	color1 = color2 = 0;
	hasLine = hasFill = false;
	for (;;){
		CURVE c;
		int flags = GetEdge(&c);
		if ( flags ) {
			// Process a state change

			// Are we at the end
		 	if ( flags == eflagsEnd ) {
				if ( hasLine )
					stroker.EndStroke();
		 		break;
			}

			if ( flags & (eflagsFill0|eflagsFill1) ) {
				// Get new fill info
				color1 = fillIndex[fill[0]];
				color2 = fillIndex[fill[1]];

				if ( !color1 && color2 ) {
					color1 = color2;
					color2 = 0;
				}
				fillRule = color2 ? fillEdgeRule : (useWinding ? fillWindingRule : fillEvenOddRule);
				hasFill = color1 || color2;
			}

			// Check to start a new stroke
			if ( flags & (eflagsLine|eflagsMoveTo) ) {
				// Cap off the current line
				if ( hasLine )
					stroker.EndStroke();

				// Start a new stroke
				RColor* lineColor = lineIndex[line].color;
				if ( lineColor ) {
					stroker.BeginStroke(lineIndex[line].thickness, lineColor);
					hasLine = true;
				} else {
				 	hasLine = false;
				}
			}
		} else {
			// Add an edge
			if ( hasFill )
				AddCurve(&c);
			if ( hasLine )
				stroker.AddStrokeCurve(&c);
		}
	}
}

void SCharacterParser::BuildMorphEdges(SRECT* devBounds)
{
	depth = 0;

	// Set up a stroker for any lines
	SStroker stroker(obj->display, &obj->edges);
	stroker.antialias = obj->display->antialias;

	stroker.preclip = preclip = devBounds && (
				devBounds->xmin < -32000 || devBounds->xmax > 32000 || 
				devBounds->ymin < -32000 || devBounds->ymax > 32000 );
	
	raster = &obj->display->raster;
	if ( !raster->bits )
	{
		raster = 0;
	}
	colorList = &obj->colors;	// we want to keep the colors

	// Set up the second shape parser
	S32 offset = GetDWord();
	
	// Note that we always ignore the styles from the second set of edges, they should always be zero
	SShapeParser edges2(player, script, pos+offset, &mat);
	edges2.InitBits();
	edges2.nFillBits = edges2.GetBits(4);
	edges2.nLineBits = edges2.GetBits(4);

	// build the interpolated fill and line styles for this morph shape
	if ( !GetMorphStyles() ) 
		return;
	
	if ( obj->clipDepth )
		MakeClipper();

	InitBits();
	nFillBits = (int)GetBits(4);
	nLineBits = (int)GetBits(4);

	// Get the edges
	BOOL hasFill, hasLine;
	
	color1 = color2 = 0;
	hasLine = hasFill = false;
	for (;;)
	{
		CURVE c1, c2;
		int flags = GetEdge(&c1);

		// Process a state change
		if ( flags )
		{
			// Are we at the end
		 	if ( flags == eflagsEnd )
			{
				if ( hasLine )
				{
					stroker.EndStroke();
				}
		 		break;
			}

			// Check to start a new stroke
			if ( flags & (eflagsLine|eflagsMoveTo) )
			{
				// Cap off the current line
				if ( hasLine )
				{
					stroker.EndStroke();
				}

				// Start a new stroke
				RColor* lineColor = lineIndex[line].color;
				if ( lineColor && lineIndex[line].thickness > 0)
				{
					stroker.BeginStroke(lineIndex[line].thickness, lineColor);
					hasLine = true;
				}
				else
				{
				 	hasLine = false;
				}
			}
			if ( flags & (eflagsFill0 | eflagsFill1) )
			{
				// Get new fill info
				color1 = fillIndex[fill[0]];
				color2 = fillIndex[fill[1]];
				if ( color1 == color2 ) // This does not make sense but it did happen with a beta build so make it keep working
					color2 = 0;
				fillRule = color2 ? fillEdgeRule : (useWinding ? fillWindingRule : fillEvenOddRule);
				hasFill = color1 || color2;
			}
		}
		else
		{
			if ( edges2.GetEdge(&c2) )
				edges2.GetEdge(&c2);	// if there are flags, ignore them

			// do the linear interpolation of the points
			CURVE	curve;
			Interpolate(&c1.anchor1, &c2.anchor1, ratio, &curve.anchor1);
			Interpolate(&c1.control, &c2.control, ratio, &curve.control);
			Interpolate(&c1.anchor2, &c2.anchor2, ratio, &curve.anchor2);
			curve.isLine = (c1.isLine && c2.isLine);

			// Add an edge
			if ( hasFill )
			{				
				AddCurve(&curve);
			}
			if ( hasLine )
			{
				stroker.AddStrokeCurve(&curve);
			}
		}
	}
	return;
}


//
// The Object Methods
//

void SObject::FreeCache()
{
	{// Free the edges
		REdge* e = edges;
		while ( e ) {
			REdge* next = e->nextObj;
			display->FreeEdge(e);
			e = next;
		}
		edges = 0;
	}

	{// Free the colors
		RColor* c = colors;
		while ( c ) {
			RColor* next = c->nextColor;
			display->FreeColor(c);
			c = next;
		}
		colors = 0;
	}
}

void SObject::Free()
{
	FreeCache();

	#ifdef FSSERVER
	if ( character )
		character->player->ReleaseData(name);
	#endif

#ifdef EDITTEXT
	if (character && character->type == editTextChar) {
		SPlayer* splayer = character->player->splayer;
		if (splayer && splayer->focus == this) {
			splayer->focus = NULL;
		}
		delete editText;
		editText = 0;
	}
#endif

#ifdef DRAGBUTTONS
	if (character && character->player) {
		SPlayer* splayer = character->player->splayer;
		if (splayer && splayer->dragObject == this) {
			splayer->dragObject = NULL;
			splayer->dragStarted = false;
		}
	}
#endif

	if (character && character->type == buttonChar) {
		SPlayer* splayer = character->player->splayer;
		if (splayer) {
			if (splayer->currentButton.button == this) {
				splayer->currentButton.button = NULL;
				if (display) {
					display->InvalidateRect(&display->tabFocusRect);
					RectSetEmpty(&display->tabFocusRect);				
				}
			}
			if (splayer->m_lastButton.button == this) {
				splayer->m_lastButton.button = NULL;
			}
		}
	}		

    if ( character->type == spriteChar && thread ) {
		delete thread;
		thread = 0;
	}
    else if (character->type == spriteExternalChar && thread)
    {
        // we are dealing with a sprite that was 
        // loaded externally so delete the whole
        // scriptplayer
        //
        // note: since player inherits from thread
        //          the delete below deletes
        //          ->player and thread and the character 
        //          dictionary for the scriptplayer

        // avoid recursion, because when we try to
        // delete the player below it will try to clear
        // the script and that in turn will try to delete
        // the rootObject which one and the same as this
        // object
        thread->rootObject = 0; 
        
        delete thread->player;
        thread = 0;
    }
    
    if (character->tag == ctagExternalThreadRoot)
    {
        // this character was created by us and so is not in
        // the char dictionary so we must delete it ourselves
        // also it was created using new so use delete to 
        // destroy it
        delete character;
        character = NULL;
    }
    else if (character->tag == ctagSpritePlaceholder)
    {
        // this character was created by us and so is not in
        // the char dictionary so we must delete it ourselves
        character->player->characterAlloc.Free(character);
        character = NULL;
    }

	FreeStr(dropTarget);
	dropTarget = NULL;

	FreeStr(name);
	name = NULL;
}

void SObject::CalcDevBounds(MATRIX* mat)
{
	// Calculate the device matrix
	if ( !character ) {
		RectSetEmpty(&devBounds);
	} else {
		if ( character->type == morphShapeChar ) {
			SRECT b;
			Interpolate(&character->bounds, &character->morph.bounds2, ratio, &b);
			MatrixTransformRect(mat, &b, &devBounds);
		} else {
			MatrixTransformRect(mat, &character->bounds, &devBounds);
		}
	}
}

void SObject::BuildEdges(STransform* x)
{
	ScriptPlayer* player = character->player;
	if ( edges || colors || player->scriptErr ) return;

	switch ( character->type ) {
		case shapeChar: {
			SCharacterParser parser(player, character->data, 0, &x->mat);
			parser.getAlpha = character->tagCode == stagDefineShape3;
			parser.cxform = x->cxform;
			parser.obj = this;
			parser.BuildEdges(true, &devBounds);
		} break;

		case textChar: {
			SParser parser;
			parser.Attach(character->data, 0);
			BOOL getAlpha = character->tagCode == stagDefineText2;

			MATRIX mat;
			parser.GetMatrix(&mat);
			MatrixConcat(&mat, &x->mat, &mat);

			int nGlyphBits = parser.GetByte();
			int nAdvanceBits = parser.GetByte();

			RColor* color = 0;
			if ( clipDepth ) 
				color = CreateClipColor(display, &colors);

			U32 layer = 0;
			int n = 0;
			MATRIX charMat;
			MatrixIdentity(&charMat);
			SRGB rgb;
			SCharacter* font = 0;
			while ( true ) {
				if ( n == 0 ) {
					// Get a new run
					int code = parser.GetByte();
					if ( !code ) break;

					if ( code & tflagsFont ) {
						font = player->FindCharacter(parser.GetWord());
					}
					if ( code & tflagsColor ) {
						// Set up a new color
						rgb = parser.GetColor(getAlpha);

						if ( !clipDepth ) {
							color = display->CreateColor();
							if ( !color ) {
								//FLASHASSERT(false);
								return;
							}
							color->SetUp(&display->raster);

							color->nextColor = colors;
							colors = color;

							color->order = layer;
							FLASHASSERT(color->order < 0x10000);

							// A solid color
							color->rgb.alpha = rgb.rgb.transparency;
							color->rgb.red = rgb.rgb.red;
							color->rgb.green = rgb.rgb.green;
							color->rgb.blue = rgb.rgb.blue;

							if ( x->cxform.HasTransform() )
								x->cxform.Apply(&color->rgb);

							color->transparent = PreMulAlpha(&color->rgb);

							// Build the pattern here also
							if ( display->raster.bits )
								color->BuildCache();

							layer++;
						}
					}
					if ( code & tflagsX ) {
						charMat.tx = (S16)parser.GetWord();
					}
					if ( code & tflagsY ) {
						charMat.ty = (S16)parser.GetWord();
					}
					if ( code & tflagsHeight ) {
						charMat.a = charMat.d = parser.GetWord() * (fixed_1/1024);
					}

					n = parser.GetByte();
					parser.InitBits();
				}

				{// Handle a glyph
					int g = (int)parser.GetBits(nGlyphBits);
					S32 advance = parser.GetSBits(nAdvanceBits);

					if ( font ) {
						U8* s;
						MATRIX m;
						MatrixConcat(&charMat, &mat, &m);
						S32 glyphOffset;

						if(font->font.flags & sfontFlagsWideOffsets) {
							s = font->data + 4*g;
							glyphOffset = (U32)s[0] | ((U32)s[1]<<8) | ((U32)s[2]<<16) | ((U32)s[3]<<24);
						} else {
							s = font->data + 2*g;
							glyphOffset = (U16)s[0] | ((U16)s[1]<<8);
						}
						SCharacterParser parser(player, font->data, glyphOffset, &m);
						parser.useWinding = true;
						parser.obj = this;

						parser.fillIndex[0] = 0;
						parser.lineIndex[0].color = 0;
						parser.lineIndex[0].thickness = 0;
						parser.fillIndex[1] = color;
						parser.nFills = 1;
						parser.BuildEdges(false, &devBounds);
					}
					charMat.tx += advance;
				}
				n--;
			}
		} break;

		case morphShapeChar: {
			// Build the edges based on the "ratio"
			SCharacterParser parser(player, character->data, 0, &x->mat);
			parser.getAlpha = character->tagCode == stagDefineMorphShape;
			parser.cxform = x->cxform;
			parser.ratio = ratio;
			parser.obj = this;
			parser.BuildMorphEdges(&devBounds);
		} break;

#ifdef EDITTEXT
		case editTextChar: {
			editText->Draw(x, TRUE);
		} 
		break;
#endif

		//case bitsChar: {
		//} break;

		//case buttonChar: {
		//} break;
	}
}


#ifdef EDITTEXT
void SObject::EditTextMouse(SPOINT* pt, BOOL mouseIsDown)
{
	editText->DoMouse(pt, mouseIsDown);
}

BOOL SObject::DrawEditText(STransform* x)
{
	editText->Draw(x, FALSE);

	return true;
}

BOOL SObject::ScrollEditText(ScriptThread *thread, char *name, int operation, int& value)
{
	if (character && character->type == editTextChar) {
		SPlayer* splayer = character->player->splayer;

		// Get the ScriptThread enclosing this edit text object
		ScriptThread* editTextThread = this->thread->rootObject->thread;

		char *varName;
		ScriptThread* targetThread = splayer->ResolveVariable(editText->m_variable,
															  editTextThread, &varName);

		if (targetThread == thread && StrEqual(varName, name)) {
			// Found it!
			switch (operation) {
			case editTextGetScroll:
				value = editText->m_vscroll + 1;
				break;
			case editTextSetScroll:
				{
					int maxScroll = editText->CalcMaxVScroll();
					value--;
					if (value < 0) {
						value = 0;
					} else if (value > maxScroll) {
						value = maxScroll;
					}
					editText->m_vscroll = value;
					Modify();
				}
				break;
			case editTextGetMaxScroll:
				value = editText->CalcMaxVScroll() + 1;
				break;
			}
			return TRUE;
		}
	}

	for ( SObject* obj = bottomChild; obj; ) {
		if (obj->ScrollEditText(thread, name, operation, value)) {
			return TRUE;
		}
		obj = obj->above;
	}
	return FALSE;
}

void SObject::UpdateEditText(ScriptThread *thread, char *name, char *value, BOOL updateFocus)
{
	if (character) {
		if (character->type == editTextChar) {

			if (updateFocus || character->player->splayer->focus != this) {
				SPlayer* splayer = character->player->splayer;

				// Get the ScriptThread enclosing this edit text object
				ScriptThread* editTextThread = this->thread->rootObject->thread;

				char *varName;
				ScriptThread* targetThread = splayer->ResolveVariable(editText->m_variable,
																	  editTextThread, &varName);

				if (targetThread == thread && !stricmp(varName, name)) {
					editText->SetBuffer(value);
					Modify();
				}
			}
		}
	}

	for ( SObject* obj = bottomChild; obj; ) {
		obj->UpdateEditText(thread, name, value, updateFocus);
		obj = obj->above;
	}
}

#endif

BOOL SObject::HitTestOne(STransform* x, SPOINT* pt)
{
#ifdef EDITTEXT
	// If this is a text-edit object and it hit, focus on the
	// text-edit.
	if (character && character->type == editTextChar) {
		if (!(editText->m_flags & seditTextFlagsNoSelect)) {
			MATRIX invmat;
			SPOINT localpt;

			MatrixInvert(&x->mat, &invmat);
			MatrixTransformPoint(&invmat, pt, &localpt);
			if (RectPointIn(&character->bounds, &localpt)) {
				return TRUE;
			}
		}
		return FALSE;
	}
#endif

	BOOL hit = false;
	if ( RectPointIn(&devBounds, pt) ) {
		if ( !edges ) 
			BuildEdges(x);

		BOOL colorsInited = false;
		for ( REdge* edge = edges; edge; edge = edge->nextObj ) {
			// Intersect the horizontal ray with this edge
			if ( edge->anchor1y <= pt->y && pt->y < edge->anchor2y ) {
				CURVE c;
				edge->Get(&c);
				if ( CurveXRaySect(&c, pt, 0) ) {
					colorsInited = true;
					switch ( edge->fillRule ) {
						case fillEdgeRule:
							// Handle color1
							edge->color1->visible ^= 1;
							edge->color2->visible ^= 1;
							break;
					
						case fillEvenOddRule:
							// Note that winding and even/odd edges must have color1 set and color2 is ignored
							edge->color1->visible ^= 1;
							break;

						case fillWindingRule:
							// Note that winding and even/odd edges must have color1 set and color2 is ignored
							edge->color1->visible += edge->dir;
							break;
					}
				}
			}
		}

		if ( colorsInited ) {
			// Look for a hit
			for ( RColor* color = colors; color; color = color->nextColor ) {
				if ( color->visible != 0 )
					hit = true;
				color->visible = 0;
			}
		}
	}
	return hit;
}

void SObject::FreeChildren()
{
	SObject* obj = bottomChild;
	bottomChild = 0;
	while ( obj ) {
		SObject* next = obj->above;

		// Remove any children
		obj->FreeChildren();

		// Free the object
 		if ( obj->drawn )
			display->InvalidateRect(&obj->devBounds);

		if ( obj == display->button )
			display->button = 0;

		obj->Free();
		display->FreeObject(obj);

		obj = next;
	}
	
}

void SObject::Modify()
{
	display->dirty = true;
	dirty = true;
}

void SObject::CalcUpdate(MATRIX* m, BOOL forceDirty)
{
	MATRIX mat;
	MatrixConcat(&xform.mat, m, &mat);
	forceDirty |= dirty;
	if ( forceDirty ) {
		// The cache is no longer valid
		FreeCache();

		if ( drawn ) {
			display->InvalidateRect(&devBounds);
			drawn = false;
		}

		CalcDevBounds(&mat);

		display->InvalidateRect(&devBounds);
		dirty = false;
	}
	for ( SObject* obj = bottomChild; obj; obj = obj->above )
		obj->CalcUpdate(&mat, forceDirty);
}

SObject* SObject::DrawClipBracket(CRaster* raster, STransform x, RColor* clipColor)
{
	{// Create a clip color and build the edges
		STransform xx = x;
		xx.Concat(&xform);
		BuildEdges(&xx);
	}
	FLASHASSERT(colors && colors->colorType == colorClip);
	if ( !colors ) 
		return above;

	drawn = true;

	// Draw the in between objects
	SObject* obj = above;
	while ( obj && obj->depth <= clipDepth ) {
		// Draw an object
		if ( obj->clipDepth ) {
			obj = obj->DrawClipBracket(raster, x, colors);
		} else {
			obj->Draw(raster, x, colors);
			obj = obj->above;
		}
	}

	// Add the clip color to the raster above the clipped colors
	raster->AddEdges(edges, colors, clipColor ? clipColor : (RColor*)1);

	return obj;
}

SObject* SObject::HitClipper(STransform x, SPOINT* pt)
{
	x.Concat(&xform);
	if ( HitTestOne(&x, pt) ) {
		// It hit the clipper, so check the children
		return above;
	} else {
		// We missed the clipper so skip the children
		SObject* obj = this; 
		while ( true ) {
			obj = obj->above;
			if ( !obj || obj->depth > clipDepth )
				return obj;	// close the bracket
		}
	}
	return 0;
}

void SObject::Draw(CRaster* raster, STransform x, RColor* clipColor)
{
    if (!visible)
        return;

	x.Concat(&xform);
	if ( RectTestIntersect(&devBounds, &raster->edgeClip) ) {
		{
			BuildEdges(&x);
			raster->AddEdges(edges, colors, clipColor);
		}
		drawn = true;
	}

	for ( SObject* obj = bottomChild; obj; ) {
		if ( obj->clipDepth ) {
			obj = obj->DrawClipBracket(raster, x, clipColor);
		} else {
			obj->Draw(raster, x, clipColor);
			obj = obj->above;
		}
	}
}

void SObject::HitTest(SObject*& hitObj, STransform x, SPOINT* pt)
{
    if (!visible)
        return;

	x.Concat(&xform);
	if ( HitTestOne(&x, pt) )
		hitObj = this;

	for ( SObject* obj = bottomChild; obj;  ) {
		if ( obj->clipDepth ) {
			obj = obj->HitClipper(x, pt);
		} else {
			obj->HitTest(hitObj, x, pt);
			obj = obj->above;
		}
	}
}

void SObject::HitButton(SObject*& hitObj, STransform x, SPOINT* pt)
{
    if (!visible)
        return;

	x.Concat(&xform);

#ifdef EDITTEXT
	// If this is a text-edit object and it hit, focus on the
	// text-edit.
	if ( character && character->type == editTextChar &&
		 !(editText->m_flags & seditTextFlagsNoSelect) ) {
		MATRIX invmat;
		SPOINT localpt;

		MatrixInvert(&x.mat, &invmat);
		MatrixTransformPoint(&invmat, pt, &localpt);
		if (RectPointIn(&character->bounds, &localpt)) {
			hitObj = this;
			return;
		}
	}
#endif

	if ( character && character->type == buttonChar ) {
		// Check the hit area of the button character
		ScriptPlayer* player = character->player;
		SParser parser;
		parser.Attach(character->data, 0);

		if ( character->tagCode == stagDefineButton2 ) 
			parser.GetWord();	// skip the action offset

		for (;;) {
			U8 stateFlags = parser.GetByte();
			if ( !stateFlags ) break;

			MATRIX m;
			U16 tag = parser.GetWord();
			U16 d = depth + parser.GetWord();
			parser.GetMatrix(&m);

			ColorTransform cx;
			if ( character->tagCode == stagDefineButton2 )
				parser.GetColorTransform(&cx, true);

			if ( stateFlags & sbtnHitTestState ) {
				// Check the child
				SCharacter* ch = player->FindCharacter(tag);
				if ( ch ) {
					SObject child;

					// Set up object
					memset(&child, 0, sizeof(child));
					child.display = display;
					//child.parent = 0;
					//child.above = 0;
					//child.bottomChild = 0;
					child.character = ch;
					child.depth = d;
					//child.ratio = 0;
					//child.name = 0;
					//child.thread = 0;
					child.xform.mat = m;
					child.xform.cxform.Clear();
					//child.state = 0;
					//child.drawn = 0;
					//child.dirty = 0;
					//child.clipDepth = 0;
					//child.edges = 0;
					//child.colors = 0;
#if defined(EDITTEXT)
					if (ch->type == editTextChar) {
						child.editText = new EditText(&child);
					}
#endif

					// Calculate the device matrix and bounds
					STransform cx = x;
					MatrixConcat(&child.xform.mat, &cx.mat, &cx.mat);
					child.CalcDevBounds(&cx.mat);

					BOOL hit = child.HitTestOne(&cx, pt);
					child.Free();
					if ( hit ) {
						hitObj = this;
						break;
					}
				}
			}
		}
	}
	for ( SObject* obj = bottomChild; obj;  ) {
		if ( obj->clipDepth ) {
			obj = obj->HitClipper(x, pt);
		} else {
			obj->HitButton(hitObj, x, pt);
			obj = obj->above;
		}
	}
}

void SObject::CalcButtonBounds(P_SRECT bounds)
{
	MATRIX mat;
	MatrixIdentity(&mat);
	SObject* current = this;
	while (current && current != &display->root) {
		MatrixConcat(&mat, &current->xform.mat, &mat);
		current = current->parent;
	}
	MatrixConcat(&mat, &display->camera.mat, &mat);

	// Initialize button boundary
	RectSetEmpty(bounds);

	if (character && character->type == editTextChar) {
		MatrixTransformRect(&mat, &character->bounds, bounds);
		return;
	}

	FLASHASSERT(character && character->type == buttonChar);
	if ( character && character->type == buttonChar ) {
		// Check the hit area of the button character
		ScriptPlayer* player = character->player;
		SParser parser;
		parser.Attach(character->data, 0);

		if ( character->tagCode == stagDefineButton2 ) 
			parser.GetWord();	// skip the action offset

		// Calculate union of all children's device boundaries.
		for (;;) {
			U8 stateFlags = parser.GetByte();
			if ( !stateFlags ) break;

			MATRIX m;
			U16 tag = parser.GetWord();
			U16 d = depth + parser.GetWord();
			parser.GetMatrix(&m);

			ColorTransform cx;
			if ( character->tagCode == stagDefineButton2 )
				parser.GetColorTransform(&cx, true);

			if ( stateFlags & sbtnHitTestState ) {
				// Check the child
				SCharacter* ch = player->FindCharacter(tag);
				if ( ch ) {
					SObject child;

					// Set up object
					memset(&child, 0, sizeof(child));
					child.display = display;
					//child.parent = 0;
					//child.above = 0;
					//child.bottomChild = 0;
					child.character = ch;
					child.depth = d;
					//child.ratio = 0;
					//child.name = 0;
					//child.thread = 0;
					child.xform.mat = m;
					child.xform.cxform.Clear();
					//child.state = 0;
					//child.drawn = 0;
					//child.dirty = 0;
					//child.clipDepth = 0;
					//child.edges = 0;
					//child.colors = 0;

					// Calculate the device matrix and bounds
					MATRIX childmat;
					MatrixConcat(&m, &mat, &childmat);
					child.CalcDevBounds(&childmat);

					// Calculate union of child's boundary with
					// boundary of button object so far
					RectUnion(&child.devBounds, bounds, bounds);
				}
			}
		}
	}
}

void GetBoundingBox(SObject* obj, P_MATRIX parentMatrix, P_SRECT bounds)
{
	MATRIX mat;
	MatrixConcat(&obj->xform.mat, parentMatrix, &mat);

	MatrixTransformRect(&mat, &obj->character->bounds, bounds);

	for (SObject* child = obj->bottomChild; child; child = child->above) {
		SRECT childBounds;
		GetBoundingBox(child, &mat, &childBounds);
		RectUnion(bounds, &childBounds, bounds);
	}


}

void GetBoundingBox(SObject* obj, P_SRECT bounds)
{
	MATRIX mat;
	MatrixIdentity(&mat);
	GetBoundingBox(obj, &mat, bounds);
}

// Helper for AddRect
void AddRectEdge(P_SPOINT a, P_SPOINT b,
			     RColor* color,
				 DisplayList* display, REdge** edgeList,
				 SStroker* stroker)
{
	if (stroker) {
		CURVE c;
		CurveSetLine(a, b, &c);
		stroker->AddStrokeCurve(&c);
		return;
	}

	if (a->y == b->y) {
		// Ignore horizontal lines
		return;
	}

	CURVE c;
	CurveSetLine(a, b, &c);

	int dir;
	if (c.anchor1.y <= c.anchor2.y) {
		dir = 1;
	} else {
		dir = -1;
		Swap(c.anchor1, c.anchor2, SPOINT);
	}

	REdge* edge = display->CreateEdge();
	if (!edge) {
		return;
	}

	// Set up the curve
	edge->Set(&c);
	edge->dir = (S8)dir;

	// Track the colors
	edge->fillRule = fillWindingRule;
	edge->color1 = color;
	edge->color2 = 0;

	// Add to the edge list
	edge->nextObj = *edgeList;
	*edgeList = edge;
}

// AddRect: Adds a transformed rectangle to the display list
void AddRect(P_SRECT rect,
			 P_MATRIX mat, RColor* color,
			 DisplayList *display, REdge** edgeList,
			 SStroker* stroker)
{
	// Corners in world space
	SPOINT TL, TR, BL, BR;

	TL.x = rect->xmin;
	TL.y = rect->ymin;
	MatrixTransformPoint(mat, &TL, &TL);

	TR.x = rect->xmax;
	TR.y = rect->ymin;
	MatrixTransformPoint(mat, &TR, &TR);

	BL.x = rect->xmin;
	BL.y = rect->ymax;
	MatrixTransformPoint(mat, &BL, &BL);

	BR.x = rect->xmax;
	BR.y = rect->ymax;
	MatrixTransformPoint(mat, &BR, &BR);

	if (stroker) {
		stroker->BeginStroke(1, color);
	}

	AddRectEdge(&TL, &BL, color, display, edgeList, stroker);
	AddRectEdge(&BL, &BR, color, display, edgeList, stroker);
	AddRectEdge(&BR, &TR, color, display, edgeList, stroker);
	AddRectEdge(&TR, &TL, color, display, edgeList, stroker);

	if (stroker) {
		stroker->EndStroke();
	}
}

