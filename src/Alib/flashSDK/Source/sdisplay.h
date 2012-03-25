/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright © Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 
****************************************************************************/

#ifndef SDISPLAY_INCLUDED
#define SDISPLAY_INCLUDED

#ifndef RASTER_INCLUDED
#include "raster.h"
#endif
#ifndef SOBJECT_INCLUDED
#include "sobject.h"
#endif
#ifndef STAGS_INCLUDED
#include "stags.h"
#endif
#ifndef NATIVE_SOUND_INCLUDED
#include NATIVE_SOUND
#endif

struct SObject;
struct SCharacter;
class ScriptPlayer;
class ScriptThread;
class EditText;

// #ifdef SPRITE_XTRA
// //  Display transform data
// //  All points should be in standard twips coordinates
// 
// typedef struct TDisplayXForm
// {
//     SFIXED			scale;
//     SFIXED          rotation;
//     SCOORD          originH;
//     SCOORD          originV;
//     SFIXED          viewScale;
//     SFIXED          viewRotation;
//     SCOORD          viewH;
//     SCOORD          viewV;
//     SRECT	        docRect;
// 	ColorTransform	cxform;
// 	SFIXED			skew;
// 	SCOORD			width;
// 	SCOORD			height;
// 	BOOL			bFlipX;
// 	BOOL			bFlipY;
// } TDisplayXForm, * PTDisplayXForm;
// 
// #endif // ..SPRITE_XTRA

struct PlaceInfo : STransform {
	int flags;
	SCharacter* character;
	U16 depth;
	U16 ratio;
    U16 clipDepth;
    U8 puppet;
	char* name;

    PlaceInfo()
    {
	    clipDepth = ratio = depth = flags = 0;
        puppet = 0;
	    character = 0;
	    name = 0;
    }
};

// Button states
enum { 
	bsNull = 0,		// the button is not initialized
	bsIdle,			// the button is idle
	bsOverUp,		// the mouse is up and over the button
	bsOverDown,		// the mouse is down and over the button
	bsOutDown		// the mouse is down and outside the button
};

class ActionList
{
public:
	// Current Action list
	enum { maxActionsLimit = 4096 };
    U32 maxActions; 
    U32 actionMask;
    
	U8 ** actionList;
	ScriptThread** actionThread;
	U32 actionBase;
	U32 actionTop;

	ActionList();
	~ActionList();
    void PushAction(U8 * a, ScriptThread* t);
	void RemoveThread(ScriptThread *thread);
};

struct ActionContext
{
	ActionList		*actionList;
	ScriptThread	*targetThread;
	BOOL			 useTarget;
};

#define MaxActionCallStack 256

class DisplayList {
public:
	// The Object List
	STransform camera;
	SObject root;

	// The Frame Buffer
	SRGB backgroundColor;
	int backgroundColorPriority;

	CBitBuffer* bits;		// the context for drawing into
	BOOL antialias;
	BOOL faster;
	BOOL useDeviceFont;
	BOOL hasDeviceEditText;

	BOOL dirty;
	SRECT devViewRect;		// the visible area of the bitmap in device coords
	SRECT screenDirtyRgn;	// the area of the bits that needs to be copied to the screen
	SRECT devDirtyRgn;		// the union of the dirty areas
	enum { maxDirtyAreas = 4 };
	int nDirty;
	S32 devDirtyArea[maxDirtyAreas];	// the portion of the bitmap that needs to be redrawn in device coords
	SRECT devDirtyRect[maxDirtyAreas];	// the portion of the bitmap that needs to be redrawn in device coords

	CRaster raster;

	SObject* holdList;
	SObject* holdParent;

	// The Thread List
	ScriptThread* threads;

	// Current button info
	SObject* button;
	int buttonState;			// these state fields are used to maintain a buttons state through a place/remove tag
	SRECT tabFocusRect;
	BOOL useFocusRect;
	SObject* buttonParent;
	SCharacter* buttonCharacter;
	U16 buttonDepth;
	U16 buttonRatio;

// #ifdef SPRITE_XTRA
// 	// Dither type to use in highquality mode		!!SPRITE_XTRA
// 	int	highQualityDitherType;
// 
// 	// Rect for render clipping						!!SPRITE_XTRA
// 	SRECT fClipRect;	
// 	
// 	BOOL fbImageEnabled;		
// #endif

#ifdef EDITTEXT
	// Text editing
	BOOL iBeam;
#endif

	char actionURL[512];

	int instanceNameCount;

#ifdef SOUND
	// Initialized be the Splayer if we are using sound.
	NativeSoundMix* theSoundMix;
#endif

public:	
	// Memory Allocators
	ChunkAlloc edgeAlloc;
	ChunkAlloc objectAlloc;
	ChunkAlloc colorAlloc;

	inline RColor* CreateColor() { return (RColor*)(colorAlloc.Alloc()); }
	inline void FreeColor(RColor* c) { c->FreeCache(); colorAlloc.Free(c); }

	inline REdge* CreateEdge() { return (REdge*)(edgeAlloc.Alloc()); }
	inline void FreeEdge(REdge* e) { edgeAlloc.Free(e); }

	inline SObject* CreateObject() { return (SObject*)(objectAlloc.Alloc()); }
	inline void FreeObject(SObject* o) { objectAlloc.Free(o); }

	BOOL MergeDirtyList(BOOL forceMerge);
	void DecomposeDirtyList();

	void UpdateList(SObject* parent, BOOL matChanged);
	
	void AddThread(ScriptThread* thread);
	void RemoveThread(ScriptThread* thread);

	void UpdateButton(SObject* obj, int newState);
	void DoButtonAction(SObject* target, int transition);
	void DoButtonStateTransition(SObject* newButton, int transition);

	void DoRemove(SObject**);

public:
	ActionList actionList;

	ActionContext actionCallStack[MaxActionCallStack];
	int actionCallStackTop;

public:
	DisplayList();
	~DisplayList();

	void Reset();
	void Invalidate() { InvalidateRect(&devViewRect); } // set the dirty area to the whole view
	void InvalidateRect(SRECT*);

	void FreeCache();	// free the cached data to reduce memory load

	void SetBits(CBitBuffer* b);
	enum { showAll=0, noBorder=1, exactFit=2, noScale=3, scaleMask = 0x0F,
			alignLeft = 0x10, alignRight = 0x20, alignTop = 0x40, alignBottom = 0x80, 
			zoom100Scroll = 0x1000 // this is a special mode used by the editor only
		};

// #ifdef SPRITE_XTRA		// !!sprite_xtra
// 	void SetCamera(SRECT* frame, SRECT* viewRct, BOOL smooth, int scaleMode=showAll, PTDisplayXForm pXform = NULL);
// #else
	void SetCamera(SRECT* frame, SRECT* viewRct, BOOL smooth, int scaleMode=showAll);
// #endif
	void ModifyCamera();
	void UpdateDevViewRect();
	void SetBackgroundColor(SRGB, int priority = 1);

	SObject* PlaceObject(SObject* parent, PlaceInfo* info);
	void RemoveObject(SObject* parent, U16 depth);
	SObject* MoveObject(SObject* parent, PlaceInfo* info);

	void BeginHold(SObject* parent);	// hold the place/removes until we finish seeking
	void FinishHold();

	void CalcUpdate();
	void UpdateRect(SRECT*);	// rebuild a specific area of the offscreen buffer
	void Update();	// rebuild the offscreen buffer
	void CalcBitsDirty(SRECT* devDirty, SRECT* bitsDirtyRgn);	// convert a dev area to a bits area
	void CalcBitsDirty(SRECT* r) { CalcUpdate(); CalcBitsDirty(&devDirtyRgn, r); }	// calc the offsreen area that needs to be rebuilt

	SObject* HitButton(SPOINT*);
	BOOL SetButton(SObject* newButton, BOOL mouseIsDown);

	SObject* HitTest(SPOINT*);
    void PushAction(U8 * a, ScriptThread* t) { actionList.PushAction(a, t); }

// #ifdef SPRITE_XTRA
//    void SetImageEnabled(BOOL bEnabled) { fbImageEnabled = bEnabled; }
// #endif
};

#endif
