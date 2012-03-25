/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright © Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 
****************************************************************************/

// The object is responsible for generating edges and colors in a format 
//	that the scan converter can use.

#ifndef SOBJECT_INCLUDED
#define SOBJECT_INCLUDED

#ifndef RASTER_INCLUDED
#include "raster.h"	// just for RGB16
#endif
#ifndef SNDMIX_INCLUDED
#include "sndmix.h"
#endif

class DisplayList;
class ScriptPlayer;
class ScriptThread;
struct SCharacter;

//
// The Character Maintains the unmapped edges of an object
//

struct ShapeCharInfo {
};

//struct BitsCharInfo {
//	SBitmapCore bm;
//};

struct ButtonCharInfo {
	U8 * soundData;
	U8 * cxformData;
	U8 trackAsMenu;		// set if we should track as a menu instead of as a push button
};

enum { textUnknown, textDeviceOK, textNoDevice };
struct FontCharInfo {
	U8 * infoData;
	int deviceState;
	U16 nGlyphs;
	U16 flags;
// [gsg] nDataOffset now used by player code (edit text) as well as authoring tool
//#ifdef _CELANIMATOR
	S32  nDataOffset;			// used for font2 and import
//#endif	
};

struct TextCharInfo {
	int deviceState;
};

struct SpriteCharInfo {
	int numFrames;
	U32 length;
};

struct MorphCharInfo {
	SRECT bounds2;
};

#ifdef EDITTEXT
class EditText;
#endif

enum { 
	shapeChar		    = 0, 
	bitsChar		    = 1, 
	buttonChar		    = 2, 
	fontChar		    = 3, 
	textChar		    = 4, 
	soundChar		    = 5, 
	spriteChar		    = 6,
	morphShapeChar	    = 7,
	commandChar		    = 8,			// used for the Flash Generator
	editTextChar	    = 9,
	videoChar			= 10,

    spriteExternalChar  = 97,
	rootChar		    = 98, 
	lostChar		    = 99 
};

struct SCharacter {
	SCharacter* next;
	ScriptPlayer* player;
	U16 tag;
	U8 type;
	U8 tagCode;			// the stag code from the define tag
	U8 * data;	// this must be first
	SRECT bounds;
	union {
		ShapeCharInfo shape;
		ButtonCharInfo button;
		SBitmapCore bits;
		FontCharInfo font;
		TextCharInfo text;
		SpriteCharInfo sprite;
		MorphCharInfo morph;
		#ifdef SOUND
		CSound sound;
		#endif
	};
};


//
// The Object keeps track of a set of colors and edges
// 	That have been mapped to the render device
//

struct STransform {
	MATRIX mat;
	ColorTransform cxform;

	void Concat(STransform* child) {
		MatrixConcat(&child->mat, &mat, &mat);
		cxform.Concat(&child->cxform);
	}
	void Clear() {
		MatrixIdentity(&mat);
		cxform.Clear();
	}
};

inline BOOL Equal(STransform* a, STransform* b) 
{
	return MatrixEqual(&a->mat, &b->mat, 0) && Equal(&a->cxform, &b->cxform);
}


//
// Display list update model
//
//  place - put the object on the display, mark as dirty
//	move - change transform, free cache mark as dirty
//	remove - if drawn, add inval rect
//	update - if dirty, calc devMat and devBounds, inval rect
//	freeCache - free cache

struct SObject {
	DisplayList* display;	// the display tree
	SObject* parent;
	SObject* above;
	SObject* bottomChild;

	SCharacter* character;
	U16 depth;			// the z order where this object should be drawn
	U16 ratio;
	char* name;

	ScriptThread* thread;	// the thread that owns this object
	STransform xform;

	SRECT devBounds;

    U8 puppet;              // set to true if the user controlls the xform
	U8 state;				// the button state for a button
	U8 drawn;				// true if this object has been drawn and needs to be erased
	U8 dirty;
    U8 visible;             // if set to zero we dont draw or hit test the object
	U16 clipDepth;

	// Drag and drop
	U8 dragCenter;
	SRECT dragConstraint;
	char *dropTarget;
	
	REdge* edges;
	RColor* colors;

#ifdef EDITTEXT
	EditText* editText;
#endif

	void CalcDevBounds(MATRIX*);
	void CalcButtonBounds(P_SRECT bounds);
	void BuildEdges(STransform*);
	void FreeCache();
	void Free();

	SObject* DrawClipBracket(CRaster* raster, STransform x, RColor* clipColor);
	SObject* HitClipper(STransform x, SPOINT* pt);

	void FreeChildren();		// free all the children of the object
	void Modify();				// mark the object as need draw
	void CalcUpdate(MATRIX* m, BOOL forceDirty=false);	// calculate the update region based on the changes

	void Draw(CRaster* raster, STransform x, RColor* clipColor = 0);

#ifdef EDITTEXT
	void EditTextMouse(SPOINT* pt, int mouseIsDown);
//	#ifdef _MAC
	BOOL DrawEditText(STransform* x);
//	#elif defined(_WINDOWS)
// 	BOOL DrawEditText(STransform* x, HDC dc);
//	#endif
	BOOL ScrollEditText(ScriptThread *thread, char *name, int operation, int& value);
	void UpdateEditText(ScriptThread *thread, char *name, char *value, BOOL updateFocus);
#endif

	BOOL HitTestOne(STransform* x, SPOINT* pt); // the point should be in device coordinates

	void HitTest(SObject*& hitObj, STransform x, SPOINT* pt);
	void HitButton(SObject*& hitObj, STransform x, SPOINT* pt);
};


#endif
