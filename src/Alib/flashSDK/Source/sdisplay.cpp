/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright © Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 
****************************************************************************/
//	990325	mnk	"fixed1" -> "fixed_1"

#include "splay.h"
#include "splayer.h"
#include NATIVE_SOUND

#ifdef EDITTEXT
#include "edittext.h"
#endif

#include "bitbuf.h"
#include "stags.h"
#include "sobject.h"

//
// Display List Methods
//

DisplayList::DisplayList() :
				edgeAlloc(sizeof(REdge), 600, true, 0x22222222),
				colorAlloc(sizeof(RColor), 64, true, 0x44444444),
				objectAlloc(sizeof(SObject), 32, true, 0x55555555)
{
	// Set up the root object
	root.display = this;
	root.parent = 0;
	root.above = 0;
	root.bottomChild = 0;
	root.character = 0;
	root.depth = 0;
	root.ratio = 0;
	root.name = 0;
	root.xform.Clear();
	RectSetEmpty(&root.devBounds);
	root.state = 0;
	root.drawn = false;
    root.puppet = false;
	
	root.edges = 0;
	root.colors = 0;
	root.thread = 0;
	root.visible = true;

	root.dragCenter = false;
	RectSetEmpty(&root.dragConstraint);
	root.dropTarget = 0;

#ifdef EDITTEXT
	root.editText = 0;
#endif

	holdList = 0;
	holdParent = 0;

	threads = 0;

	instanceNameCount = 0;

	camera.Clear();
	antialias = false;
	faster = false;
	useDeviceFont = false;
	bits = 0;
	dirty = false;
	RectSetEmpty(&screenDirtyRgn);
	RectSetEmpty(&devDirtyRgn);
	nDirty = 0;
	UpdateDevViewRect();

	backgroundColor.all = SRGBWhite;
	backgroundColorPriority = 0;

	// Button info
	buttonState = 0;
	button = 0;
	RectSetEmpty(&tabFocusRect);
	useFocusRect = true;
	buttonParent = 0;
	buttonCharacter = 0;
	buttonDepth = 0;
	buttonRatio = 0;
	actionURL[0] = 0;

#ifdef EDITTEXT
	// Text editing
	iBeam = FALSE;
#endif

	hasDeviceEditText = false;
	actionCallStackTop = 0;
}

DisplayList::~DisplayList()
{
	root.FreeChildren();

	// Zero out the thread list
	while ( threads ) {
		threads->display = 0;
		threads = threads->next;
	}
}

void DisplayList::AddThread(ScriptThread* thread) 
{
	FLASHASSERT(!thread->next); 
	thread->next = threads; 
	threads = thread; 
}

void DisplayList::RemoveThread(ScriptThread* thread)
{
	// Clear any pending actions on a thread that is going away
	actionList.RemoveThread(thread);

	// If we are inside a DoActions, remove pending actions on
	// the action context stack
	ActionContext* actionContext = actionCallStack;
	for (int i=0; i<actionCallStackTop; i++) {
		if (actionContext->targetThread == thread) {
			actionContext->targetThread = 0;
			actionContext->useTarget = false;
		}
		actionContext->actionList->RemoveThread(thread);
		actionContext++;
	}

	// Remove the thread from the list
	ScriptThread** link = &threads;
	while ( *link ) {
		if ( *link == thread ) {
			*link = thread->next;
			return;
		}
		link = &(*link)->next;
	}
	FLASHASSERT(false);
}

void DisplayList::Reset()
{
	if ( backgroundColorPriority <= 1 ) {
		backgroundColor.all = SRGBWhite;
		backgroundColorPriority = 0;
		Invalidate();
 	}
}

S32 RectArea(SRECT* area)
{
	return (area->xmax-area->xmin) * (area->ymax-area->ymin);
}

inline S32 UnionArea(SRECT* r1, SRECT* r2)
{
	SRECT area;
	RectUnion(r1, r2, &area);
	return RectArea(&area);
}

BOOL RectTestOverlap(P_SRECT r1, P_SRECT r2)
{
	return r1->xmin < r2->xmax && r2->xmin < r1->xmax &&
		   r1->ymin < r2->ymax && r2->ymin < r1->ymax;
}

BOOL DisplayList::MergeDirtyList(BOOL forceMerge)
// Merge the pair of rectangles that will cause the smallest increase in the total dirty area
{
	if ( nDirty > 1 ) {
		// Check to merge areas to reduce the number
		S32 bestDelta = forceMerge ? 0x7FFFFFFFL : 0;	// if there is no empty slot, we must merge
		int mergeA = 0;
		int mergeB = 0;
		for ( int i = 0; i < nDirty-1; i++ ) {
			for ( int j = i+1; j < nDirty; j++ ) {
				S32 delta = UnionArea(devDirtyRect+i, devDirtyRect+j) - devDirtyArea[i] - devDirtyArea[j];
				if ( bestDelta > delta ) {
					mergeA = i;
					mergeB = j;
					bestDelta = delta;
				}
			}
		}

		if ( mergeA != mergeB ) {
			RectUnion(devDirtyRect+mergeA, devDirtyRect+mergeB, devDirtyRect+mergeA);
			devDirtyArea[mergeA] = RectArea(devDirtyRect+mergeA);
			for ( int i = mergeB+1; i < nDirty; i++ ) {
				devDirtyRect[i-1] = devDirtyRect[i];
				devDirtyArea[i-1] = devDirtyArea[i];
			}
			nDirty--;
			return true;
		}
	}
	return false;
}

void DecomposeRect(SRECT* r1, SRECT* r2)
// Restructure two overlaping rectangles to eliminate the intersecting area
//	while still covering the same area and perhaps more area
{
	FLASHASSERT(RectTestOverlap(r1, r2));

	// Build the 3 rect slabs on y-axis
	SRECT r[3];
	if ( r1->ymin < r2->ymin ) {
		r[0].ymin = r1->ymin;
		r[0].ymax = r2->ymin;
		r[0].xmin = r1->xmin;
		r[0].xmax = r1->xmax;
	} else {
		r[0].ymin = r2->ymin;
		r[0].ymax = r1->ymin;
		r[0].xmin = r2->xmin;
		r[0].xmax = r2->xmax;
	}
	if ( r1->ymax < r2->ymax ) {
		r[2].ymin = r1->ymax;
		r[2].ymax = r2->ymax;
		r[2].xmin = r2->xmin;
		r[2].xmax = r2->xmax;
	} else {
		r[2].ymin = r2->ymax;
		r[2].ymax = r1->ymax;
		r[2].xmin = r1->xmin;
		r[2].xmax = r1->xmax;
	}
	r[1].ymin = r[0].ymax;
	r[1].ymax = r[2].ymin;
	r[1].xmin = Min(r1->xmin, r2->xmin);
	r[1].xmax = Max(r1->xmax, r2->xmax);

	// Combine the middle slab with the slab that will generate the smallest area
	S32 a[3];
	for ( int i = 0; i < 3; i++ )
		a[i] = RectArea(r+i);

	SRECT u1, u2;
	RectUnion(&r[0], &r[1], &u1);
	RectUnion(&r[1], &r[2], &u2);

	S32 delta0 = a[0] + a[1] - RectArea(&u1);
	S32 delta1 = a[1] + a[2] - RectArea(&u2);

	if ( delta0 > delta1 ) {
		*r1 = u1;
		*r2 = r[2];
	} else {
		*r1 = r[0];
		*r2 = u2;
	}
}

void DisplayList::DecomposeDirtyList()
// Decompose any overlapping rectangles into non-overlapping rectangles
{
	// Merge any areas that would reduce the total area
	while ( MergeDirtyList(false) ) {}

	for ( int i = 0; i < nDirty-1; i++ ) {
		for ( int j = i+1; j < nDirty; j++ ) {
			if ( RectTestOverlap(devDirtyRect+i, devDirtyRect+j) ) {
				DecomposeRect(devDirtyRect+i, devDirtyRect+j);
			}
		}
	}
}

void DisplayList::InvalidateRect(SRECT* r)
{
	SRECT rect;
	rect.xmin = r->xmin;
	rect.xmax = r->xmax;
	rect.ymin = r->ymin;
	rect.ymax = r->ymax;

	RectInset(antialias ? -8 : -2, &rect);
	if ( RectTestIntersect(&devViewRect, &rect) ) {
		FLASHASSERT(nDirty < maxDirtyAreas);
		// Add to the list
		RectIntersect(&devViewRect, &rect, &devDirtyRect[nDirty]);
		RectUnion(&devDirtyRgn, &devDirtyRect[nDirty], &devDirtyRgn);	// add to the dirty region
		devDirtyArea[nDirty] = RectArea(devDirtyRect+nDirty);	// add to the list
		nDirty++;

		MergeDirtyList(nDirty == maxDirtyAreas);
	}
}

void FreeCache(SObject* parent)
{
	parent->FreeCache();
	for ( SObject* obj = parent->bottomChild; obj; obj = obj->above )
		FreeCache(obj);
}

void DisplayList::FreeCache()
// free the cached data to reduce memory load
{
	::FreeCache(&root);

	edgeAlloc.FreeEmpties();
	objectAlloc.FreeEmpties();
	colorAlloc.FreeEmpties();
	raster.FreeEmpties();
}

void DisplayList::SetBits(CBitBuffer* b)
{
	bits = b;
	UpdateDevViewRect();
	Invalidate();	
}

void DisplayList::SetCamera(SRECT* frame, SRECT* viewRct, BOOL smooth, int scaleMode)
{
	// Set up the camera
	SRECT viewPort = *viewRct;
	if ( smooth ) {
	 	viewPort.xmin *= 4;
	 	viewPort.ymin *= 4;
	 	viewPort.xmax *= 4;
	 	viewPort.ymax *= 4;
	}

	// Calculate the mapping
	//	src = frame
	//	dst = viewPort
	MATRIX newMat;
	#define minMapSize 0x10L
	FLASHASSERT(!RectIsEmpty(&viewPort) && !RectIsEmpty(frame));
	newMat.a = FixedDiv(FixedMax(RectWidth(&viewPort), minMapSize), FixedMax(RectWidth(frame), minMapSize));
	newMat.d = FixedDiv(FixedMax(RectHeight(&viewPort), minMapSize), FixedMax(RectHeight(frame), minMapSize));
	newMat.b = newMat.c = 0;

	// Adjust the scaling to maintain the aspect ratio
	switch ( scaleMode & scaleMask ) {
		case showAll:
			newMat.a = newMat.d = FixedMin(newMat.a, newMat.d);
			break;
		case noBorder:
			newMat.a = newMat.d = FixedMax(newMat.a, newMat.d);
			break;
		case exactFit:
			//MatrixMap(frame, &viewPort, &newMat);
			break;
		case noScale:
			newMat.a = newMat.d = smooth ? 4*fixed_1/20 : fixed_1/20;
			break;
	}

	// Pick the alignment point
	SPOINT srcRef, dstRef;
	if ( scaleMode & alignLeft ) {
		srcRef.x = frame->xmin;
		dstRef.x = viewPort.xmin;
	} else if ( scaleMode & alignRight ) {
		srcRef.x = frame->xmax;
		dstRef.x = viewPort.xmax;
	} else {
		srcRef.x = FixedAverage(frame->xmin, frame->xmax);
		dstRef.x = FixedAverage(viewPort.xmin, viewPort.xmax);
	}
	if ( scaleMode & alignTop ) {
		srcRef.y = frame->ymin;
		dstRef.y = viewPort.ymin;
	} else if ( scaleMode & alignBottom ) {
		srcRef.y = frame->ymax;
		dstRef.y = viewPort.ymax;
	} else {
		srcRef.y = FixedAverage(frame->ymin, frame->ymax);
		dstRef.y = FixedAverage(viewPort.ymin, viewPort.ymax);
	}

	// Calculate the translation based on the alignment
	newMat.tx = dstRef.x - FixedMul(srcRef.x, newMat.a);
	newMat.ty = dstRef.y - FixedMul(srcRef.y, newMat.d);

	// Don't invalidate unless there was a change
	BOOL smoothBits = smooth && !faster;
	if ( newMat.a  != camera.mat.a  || newMat.b  != camera.mat.b ||
		 newMat.c  != camera.mat.c  || newMat.d  != camera.mat.d ||
		 newMat.tx != camera.mat.tx || newMat.ty != camera.mat.ty ||
		 antialias != smooth || raster.smoothBitmaps != smoothBits ) {
		antialias = smooth;
		camera.mat = newMat;
		raster.smoothBitmaps = smoothBits;
		raster.bitmapDither = antialias ? errorDiffusionDither : orderedDither;
		ModifyCamera();
	}
}

void DisplayList::UpdateDevViewRect()
// set the dirty area to the whole view
{
	devViewRect.xmin = devViewRect.ymin = 0;
	if ( bits && bits->BitsValid()) {
// 		devViewRect.xmax = bits->m_bufWidth;
// 		devViewRect.ymax = bits->m_bufHeight;
		devViewRect.xmax = bits->width();
		devViewRect.ymax = bits->height();

	} else {
		devViewRect.xmax = 2000;
		devViewRect.ymax = 2000;
	}
	if ( antialias ) {
		devViewRect.xmax *= 4;
		devViewRect.ymax *= 4;
	}
}

void DisplayList::ModifyCamera()
{
	// Free all of the transformed edges and update the object matricies
	UpdateDevViewRect();
	Invalidate();
	root.Modify();
}

void DisplayList::SetBackgroundColor(SRGB color, int priority)
{
	if ( priority > backgroundColorPriority ) {
		if ( backgroundColor.all != color.all ) {
			Invalidate();	
			backgroundColor = color;
		}
		backgroundColorPriority = priority;
	}
}

void DisplayList::CalcBitsDirty(SRECT* devDirty, SRECT* bitsDirtyRgn)
{
	*bitsDirtyRgn = *devDirty;
	if ( antialias && !RectIsEmpty(bitsDirtyRgn) ) {
		bitsDirtyRgn->xmin /= 4;
		bitsDirtyRgn->ymin /= 4;
		bitsDirtyRgn->xmax /= 4;
		bitsDirtyRgn->ymax /= 4;
	}
}

class SStroker;
void AddRect(P_SRECT rect,
			 P_MATRIX mat, RColor* color,
			 DisplayList *display, REdge** edgeList,
			 SStroker* stroker);

void DisplayList::UpdateRect(SRECT* clip)
{
	if ( bits->LockBits() ) {
		raster.Attach(bits, clip, antialias);
		raster.BeginPaint();

		// Add the background edges
		REdge bkEdges[2];
		RColor bkColor;
		if ( backgroundColor.all != 0 ) {
			// Set up the color
			bkColor.SetUp(&raster);
			bkColor.order = 0;
			bkColor.rgb.alpha = 255;
			bkColor.rgb.red   = backgroundColor.rgb.red;
			bkColor.rgb.green = backgroundColor.rgb.green;
			bkColor.rgb.blue  = backgroundColor.rgb.blue;
			bkColor.BuildCache();

			// Set up the edges
			bkEdges[0].nextObj = &bkEdges[1];
			bkEdges[1].nextObj = 0;
			
			SPOINT pt1, pt2;
			CURVE c;
			pt1.y = raster.edgeClip.ymin;
			pt2.y = raster.edgeClip.ymax;

			pt1.x = pt2.x = raster.edgeClip.xmin;
			CurveSetLine(&pt1, &pt2, &c);
			bkEdges[0].Set(&c);

			pt1.x = pt2.x = raster.edgeClip.xmax;
			CurveSetLine(&pt1, &pt2, &c);
			bkEdges[1].Set(&c);

			bkEdges[0].dir 		= bkEdges[1].dir      = 0;
			bkEdges[0].fillRule = bkEdges[1].fillRule = fillEvenOddRule;
			bkEdges[0].color1 	= bkEdges[1].color1   = &bkColor;
			bkEdges[0].color2 	= bkEdges[1].color2   = 0;

			raster.AddEdges(bkEdges, &bkColor);
		} else {
			// It's transparent...
			raster.getBackground = true;
		}

		{
			// Add the object edges
			root.Draw(&raster, camera);
		}

#ifndef DISABLE_TAB_FOCUS
		// Add button focus rectangle
		RColor yellow;
		REdge* edges = NULL;

		if (useFocusRect && !RectIsEmpty(&tabFocusRect)) {
			// Set up the color
			yellow.SetUp(&raster);
			yellow.order = 0;
			yellow.rgb.alpha = 255;
			yellow.rgb.red   = 255;
			yellow.rgb.green = 255;
			yellow.rgb.blue  = 0;
			yellow.BuildCache();

			SRECT bounds = tabFocusRect;
			MATRIX mat;
			MatrixIdentity(&mat);

			int border = 3;
			if (antialias) {
				border *= 4;
			}

			SRECT rect = bounds;
			rect.xmax = bounds.xmin + border;
			AddRect(&rect, &mat, &yellow, this, &edges, NULL);

			rect.xmin = bounds.xmax - border;
			rect.xmax = bounds.xmax;
			AddRect(&rect, &mat, &yellow, this, &edges, NULL);

			rect.xmin = bounds.xmin;
			rect.ymax = bounds.ymin + border;
			AddRect(&rect, &mat, &yellow, this, &edges, NULL);

			rect.ymin = bounds.ymax - border;
			rect.ymax = bounds.ymax;
			AddRect(&rect, &mat, &yellow, this, &edges, NULL);

			raster.AddEdges(edges, &yellow);
		}
#endif
			
		raster.PaintBits();

#ifndef DISABLE_TAB_FOCUS
		if (!RectIsEmpty(&tabFocusRect)) {
			// Free the edges
			REdge* e = edges;
			while ( e ) {
				REdge* next = e->nextObj;
				FreeEdge(e);
				e = next;
			}
			edges = 0;
			yellow.FreeCache();
		}
#endif

		if ( backgroundColor.all != 0 ) 
			bkColor.FreeCache();

		bits->UnlockBits();
	}
}

void DisplayList::CalcUpdate()
{
	if ( dirty ) {
		root.CalcUpdate(&camera.mat);
		dirty = false;
	}
}

void DisplayList::Update()
{
	CalcUpdate();
	// Use multiple dirty rectangles to minimize area
	if ( nDirty > 0 && bits ) {
		// Update the frame buffer
		DecomposeDirtyList();
		for ( int i = 0; i < nDirty; i++ ) {
			SRECT bitsDirtyRgn;
			CalcBitsDirty(devDirtyRect+i, &bitsDirtyRgn);
			RectUnion(&bitsDirtyRgn, &screenDirtyRgn, &screenDirtyRgn);
			UpdateRect(&bitsDirtyRgn);
		}
	}

	RectSetEmpty(&devDirtyRgn);
	nDirty = 0;
}

SObject* DisplayList::PlaceObject(SObject* parent, PlaceInfo* info)
{
	// Find the insertion point
	SObject** link = &parent->bottomChild;
	for (;;) {
		SObject* obj = *link;
		if ( !obj ) break;
	 	if ( obj->depth >= info->depth ) {
			FLASHASSERT(obj->depth != info->depth);
			break;
		}
		link = &obj->above;
	}

	// Create a new object
	SObject* obj = CreateObject();
	if ( !obj ) return 0;

	// Add to list
	obj->parent = parent;
	obj->above = *link;
	*link = obj;

	// Set up object
	obj->character = info->character;
	obj->xform = *info;
	obj->depth = info->depth;
	obj->ratio = info->ratio;
    obj->puppet = info->puppet;
	obj->drawn = false;
    obj->visible = true;
	obj->dragCenter = false;
	RectSetEmpty(&obj->dragConstraint);
	obj->dropTarget = NULL;
	obj->display = this;
	obj->bottomChild = 0;
	obj->edges = 0;
	obj->colors = 0;
	obj->state = 0;
#ifdef EDITTEXT
	obj->editText = 0;
#endif

	// If this is a movie clip and it has no name,
	// generate a default name
	if (obj->character->type == spriteChar && !info->name) {
		URLBuilder ub;
		ub.AppendString("instance");
		ub.AppendInt(++instanceNameCount);
		obj->name = CreateStr(ub.buf);
	} else {
		// Regular processing: copy name if present
		obj->name = CreateStr(info->name);
	}
	obj->thread = parent ? parent->thread : 0;
	obj->clipDepth = info->flags & splaceDefineClip ? info->clipDepth : 0;//(info->flags & splaceDefineClip) != 0;
	obj->Modify();

	switch ( obj->character->type ) {
		case buttonChar: {
			// Set up the button state
			int state = bsIdle;
			if ( obj->depth == buttonDepth && obj->parent == buttonParent && 
				 buttonCharacter == obj->character && obj->ratio == buttonRatio ) {
				// The button is currently being tracked
				FLASHASSERT(obj->character->type == buttonChar);
				button = obj;
				state = buttonState;
			}
			UpdateButton(obj, state);
		} break;

#ifdef EDITTEXT
		case editTextChar:
			{
				obj->editText = new EditText(obj);
				if (holdParent != obj->parent) {
					// Not doing a hold... update value immediately
					obj->editText->UpdateFromVariable();
				}
				// If there is a holdParent, the edit text's value will
				// be updated later.
			}
			break;
#endif

		case spriteExternalChar:
        {
			ScriptThread    *thread = 0;
            ScriptPlayer    *player = obj->character->player;
            int             startPos = 0;
            int             len = 0;
            int             numFrames = 0;

            if (info->flags & splaceCloneExternalSprite)
            {
                // means we are cloning an existing
                // external sprite
                if (player->len >= player->scriptLen)
                {
                    // we only clone the external
                    // sprite if it has been completely loaded
                    // allocate full player instead of thread
                    // it is easier to delete the external
                    // sprite if it has its own player
                    thread = new ScriptPlayer;
                    startPos = player->startPos;
                    len = player->len;
                    numFrames = player->numFrames;
                    thread->script = player->script;
                    ((ScriptPlayer *) thread)->numFramesComplete = player->numFramesComplete;
                    player = (ScriptPlayer *) thread;
                    player->gotHeader = true;
                    
                    // add a ref count to the script
                    // we don't want to duplicate it
                    // so we want to make sure it's
                    // protected from deletion for
                    // cloned external sprites
                    player->scriptRefCount = obj->character->player->scriptRefCount;
                    player->ScriptAddRefCount();
                }
            }
            else
            {
                // note that it's ok to have some of the
                // initial values of the len, etc. set
                // to zero. they will be set during pushdata
                thread = player;

                // add a ref count to the script
                // we don't want to duplicate it
                // so we want to make sure it's
                // protected from deletion for
                // cloned external sprites
                player->ScriptAddRefCount();
            }
			
            if ( thread ) 
            {
				thread->ClearState();
				obj->thread = thread;
				thread->player = player;
				thread->layerDepth = obj->depth;
				thread->rootObject = obj;
				thread->SetDisplay(obj->display);
				thread->Attach(obj->character->player->script, startPos);
				thread->len = len;
				thread->startPos = startPos;
				thread->numFrames = numFrames;
                
                // for non cloned sprite this won't draw the frame
                // untill we push data into the script (this is ok)
				thread->DrawFrame(0, false);
			}
		} break;

  		case spriteChar: {
			// Create a sprite thread
			ScriptThread* thread = new ScriptThread();
			if ( thread ) {
				thread->ClearState();
				obj->thread = thread;
				thread->player = obj->character->player;
				thread->layerDepth = obj->depth;
				thread->rootObject = obj;
				thread->SetDisplay(obj->display);
				thread->Attach(obj->character->data, 0);
				thread->len = obj->character->sprite.length;
				thread->startPos = 0;
				thread->numFrames = obj->character->sprite.numFrames;
				thread->DrawFrame(0, false);
			}
		} break;
	}

	return obj;
}

SObject* DisplayList::MoveObject(SObject* parent, PlaceInfo* info)
{
	// See if there is already an object at this depth or find the insertion point
	SObject* obj = parent->bottomChild;
	while ( obj ) {
	 	if ( obj->depth >= info->depth ) 
			break;
		obj = obj->above;
	}

	if ( obj ) {
		// We found an object at this depth
		if ( obj->depth != info->depth ) {
			FLASHASSERT(false);
			return 0;		// don't move the wrong object
		}

        if (obj->puppet)
            return 0;       // don't move the object if it's puppeted

		// Move the proper fields of the object
		obj->Modify();
		if ( info->flags & splaceCharacter ) {
			// The type of sprite or button objects must never change
			if ( obj->character->type != info->character->type && 
				 (obj->character->type == spriteChar || info->character->type == spriteChar ||
				  obj->character->type == buttonChar || info->character->type == buttonChar) ) {
				FLASHASSERT(false);		// this should never happen, there was a beta build where we did export like this, so prevent the crash
			} else {
				obj->character = info->character;
			}
		}
		if ( info->flags & splaceMatrix )
			obj->xform.mat = info->mat;
		if ( info->flags & splaceColorTransform )
			obj->xform.cxform = info->cxform;
		if ( info->flags & splaceRatio )
			obj->ratio = info->ratio;
	}
	return obj;
}

void DisplayList::DoRemove(SObject** link)
{
	SObject* obj = *link;
	FLASHASSERT(obj);

	// Remove the contents of the old object
	if ( obj->drawn )
		InvalidateRect(&obj->devBounds);

	obj->FreeChildren();
	obj->Free();

	
	// Remove from list
	if ( obj == button )
		button = 0;
	*link = obj->above;
	FreeObject(obj);
}

void DisplayList::RemoveObject(SObject* parent, U16 depth)
{
	// See if there is already an object at this depth or find the insertion point
	SObject* obj;
	SObject** link = &parent->bottomChild;
	for (;;) {
		obj = *link;
		if ( !obj ) break;
	 	if ( obj->depth >= depth ) 
			break;
		link = &obj->above;
	}

	if ( obj && obj->depth == depth )
		DoRemove(link);
}

void DisplayList::BeginHold(SObject* parent)
// This is used only for backwards seeks because removes all of the children
{
	// Save the old state for a later compare
	holdParent = parent;
	holdList = holdParent->bottomChild;
	holdParent->bottomChild = 0;
}

void DisplayList::FinishHold()
// Compare the old and the new list
// If an old object matches a new object, replace the new object with the old object
{
	FLASHASSERT(holdParent);

	SObject** oldLink = &holdList;
	SObject** newLink = &holdParent->bottomChild;

    for (;;) {
		SObject* oldObj = *oldLink;
		SObject* newObj = *newLink;

		if ( !oldObj )
			break;	// we are done

		if ( !newObj || oldObj->depth < newObj->depth ) {
			if ( oldObj->depth >= 0x4000 ) {
				// This is a cloned object, it always gets copied to the new list
				FLASHASSERT(!newObj);	// none of the new obj's should have a depth > 4000
				*oldLink = oldObj->above;	// remove from old list
				oldObj->above = *newLink;	// place on new list
				*newLink = oldObj;

			} else {
				// Get rid of the old object
				DoRemove(oldLink);
			}

		} else if ( oldObj->depth == newObj->depth ) {
			// Compare these objects
            BOOL remove = true;
			if (oldObj->character == newObj->character && oldObj->ratio == newObj->ratio && oldObj->clipDepth == newObj->clipDepth) 
            {
				// Check the matrix and cxform and ratio
				BOOL moved = !Equal(&oldObj->xform, &newObj->xform);
				if (!moved || oldObj->character->type == spriteChar) 
                {
					if ( moved && !oldObj->puppet) 
                    {
						oldObj->Modify();
						oldObj->xform = newObj->xform;
						//oldObj->ratio = newObj->ratio;
					}

					if ( button == newObj ) 
                    { 
						FLASHASSERT(oldObj->state == buttonState);
						button = oldObj;
						//UpdateButton(obj, state);
					}
                    remove = false;
				}
			}
            else if (
                oldObj->ratio == newObj->ratio && 
                (oldObj->character->type == spriteExternalChar || oldObj->character->type == spriteChar) && 
                (newObj->character->type == spriteChar || newObj->character->type == spriteExternalChar))
            {
                // we are dealing with externally loaded sprite
                // or an empty placeholder. It inherits the 
                // sprite ratio of the sprite we are replacing. 
                // This means that it goes away when the same 
                // time the replaced sprite should have gone away
				
                if (!Equal(&oldObj->xform, &newObj->xform))
                {
                        if (!oldObj->puppet)
                        {
						    oldObj->Modify();
						    oldObj->xform = newObj->xform;
                        }
			    }
                remove = false;
            }
            
            if (!remove)
            {
				DoRemove(newLink);			// delete the new obj
				*oldLink = oldObj->above;	// remove from old list
				oldObj->above = *newLink;	// place on new list
				*newLink = oldObj;
				continue;
            }
			
			newLink = &newObj->above;
			DoRemove(oldLink);

		} else {
			// Just look at the next new object
			FLASHASSERT(oldObj->depth > newObj->depth);
			newLink = &newObj->above;
		}
	}

#ifdef EDITTEXT
	// Update edit texts on hold list
	SObject* obj = holdParent->bottomChild;
	while (obj) {
		if (obj->character && obj->character->type == editTextChar) {
			obj->editText->UpdateFromVariable();
		}
		obj = obj->above;
	}
#endif

    holdParent = 0;
}


//
// Button and mouse target handling
//

const U8 btnStateMap[] = {
	0,				//bsNull,		// the button is not initialized
	sbtnUpState,	//bsIdle,		// the button is idle
	sbtnOverState,	//bsOverUp,		// the mouse is up and over the button
	sbtnDownState,	//bsOverDown,	// the mouse is down and over the button
	sbtnOverState,	//bsOutDown		// the mouse is down and outside the button
};

void DisplayList::UpdateButton(SObject* obj, int newState)
// Add or remove child characters so that the button matches newState
{
	SCharacter* ch = obj->character;
	if ( ch->type != buttonChar )
		return;
	
	int oldState = obj->state;
	if ( oldState == newState ) return;

	U8 * cxData = ch->button.cxformData;

	ScriptPlayer* player = ch->player;
	for ( int pass = 0; pass < 2; pass++ ) {
		SParser parser;
		parser.Attach(ch->data, 0);

		if ( ch->tagCode == stagDefineButton2 ) 
			parser.GetWord();	// skip the action offset

		SParser cxParser;
		if ( cxData )
			cxParser.Attach(cxData, 0);

		for (;;) {
			U8 stateFlags = parser.GetByte();
			if ( !stateFlags ) break;

			PlaceInfo info;
			info.flags = splaceCharacter|splaceMatrix|splaceColorTransform;
			U16 tag = parser.GetWord();
			info.depth = obj->depth + parser.GetWord();
			parser.GetMatrix(&info.mat);
			if ( ch->tagCode == stagDefineButton2 ) 
				parser.GetColorTransform(&info.cxform, true);	// new style button
			else if ( cxData )
				cxParser.GetColorTransform(&info.cxform, false);// separate cxfrom data
			else
				info.cxform.Clear();							// no cxform data

			// Remove the objects first and then add in case we have conflicting layer/id combos
			BOOL newVis = stateFlags & btnStateMap[newState];
			BOOL oldVis = stateFlags & btnStateMap[oldState];
			if ( pass == 0 && !newVis && oldVis ) {
				// Remove the child
				RemoveObject(obj, info.depth);
			}
			if ( pass == 1 && newVis && !oldVis ) {
				// Add the child
				info.character = player->FindCharacter(tag);
				info.name = 0;
				if ( info.character )
					PlaceObject(obj, &info);
			}
		}
	}
	obj->state = newState;
}

void DisplayList::DoButtonAction(SObject* target, int transition)
{
	SCharacter* ch = target->character;
	FLASHASSERT(ch->type == buttonChar);

	if ( ch->tagCode == stagDefineButton2 ) {
		// Handle the new style button
		SParser parser;
		parser.Attach(ch->data, 0);
		BOOL first = true;
		for (;;) {
			S32 link = parser.pos;
			int offset = parser.GetWord();
			if ( !first ) {
				int code = parser.GetWord();
				if ( code & (1<<transition) ) {
					PushAction(parser.script+parser.pos, target->thread);
				}
			}
			if ( !offset ) break;
			parser.pos = link + offset;
			first = false;
		}

	} else {
		// Handle the old style button
		if ( transition == bsOverDownToOverUp ) {
			// Do the button action on the mouse up in
			SParser parser;
			parser.Attach(ch->data, 0);

			// Skip over the child data
			for (;;) {
				U8 stateFlags = parser.GetByte();
				if ( !stateFlags ) break;
				MATRIX m;
				parser.SkipBytes(4);//GetWord();
				//parser.GetWord();
				parser.GetMatrix(&m);
			}

			// Handle the action
			PushAction(parser.script+parser.pos, target->thread);
		}
	}

#ifdef SOUND
	{// Play the sound
		int state = 0;
		switch ( transition ) {
			case bsIdleToOverDown:
			case bsIdleToOverUp:
				state = sbtnOverState;
				break;

			case bsOverUpToOverDown:
				state = sbtnDownState;
				break;

			case bsOverDownToOverUp:
				state = sbtnHitTestState;
				break;

			case bsOverUpToIdle:
			case bsOutDownToIdle:
			case bsOverDownToIdle:
				state = sbtnUpState;
				break;
		}

		if ( state && ch->button.soundData ) {
			SParser parser;
			parser.Attach(ch->button.soundData, 0);

			// Skip the sounds
			for ( int i = 1; i < state; i <<= 1 ) {
				U16 tag = parser.GetWord();
				if ( tag )
					parser.GetSoundInfo(0);// skip the data
			}

			// Get the sound we want
			U16 tag = parser.GetWord();
			if ( tag ) {
				SCharacter* sound = ch->player->FindCharacter(tag);
				if ( !sound || sound->type != soundChar ) return;

				CSoundChannel* channel = new CSoundChannel();
				if ( channel ) {
					channel->AddRef();
					channel->sound = &sound->sound;
					channel->tag = (U32)this;
					parser.GetSoundInfo(channel);	// set up envelope, inPoint, loops, etc...

					theSoundMix->AddSound(channel);
					channel->Release();
				}
			}
		}
	}
#endif
}

void DisplayList::DoButtonStateTransition(SObject* newButton, int transition)
{
	FLASHASSERT(newButton);
	switch ( transition ) {
		case bsIdleToOverUp:
		case bsOverDownToOverUp:
			buttonState = bsOverUp;
			break;
		case bsOverUpToIdle:
		case bsOutDownToIdle:
		case bsOverDownToIdle:
			buttonState = bsIdle;
			break;
		case bsOverUpToOverDown:
		case bsOutDownToOverDown:
		case bsIdleToOverDown:
			buttonState = bsOverDown;
			break;
		case bsOverDownToOutDown:
			buttonState = bsOutDown;
			break;
		default:
			FLASHASSERT(false);
	}

	UpdateButton(newButton, buttonState);
	DoButtonAction(newButton, transition);

	if ( buttonState == bsIdle ) {
		button = 0;
		buttonParent = 0;
		buttonCharacter = 0;
		buttonDepth = 0;
		buttonRatio = 0;
	} else {
		button = newButton;
		buttonParent = newButton->parent;
		buttonCharacter = newButton->character;
		buttonDepth = newButton->depth;
		buttonRatio = newButton->ratio;
	}
}

BOOL DisplayList::SetButton(SObject* newButton, BOOL mouseIsDown)
{
	FLASHASSERT(!newButton || newButton->character->type == buttonChar);

	if ( !button ) {				// just in case the button was removed
		buttonState = bsIdle;
		buttonParent = 0;
		buttonCharacter = 0;
		buttonDepth = 0;
		buttonRatio = 0;
	}

	BOOL changed = false;
	switch ( buttonState ) {
		case bsIdle: {
			FLASHASSERT(!button);
			if ( newButton ) {
				if ( !mouseIsDown ) {
					// The mouse has come over the button
					DoButtonStateTransition(newButton, bsIdleToOverUp);
					changed = true;
				} else if ( newButton->character->button.trackAsMenu ) {
					// Enter a button while the mouse is down
					DoButtonStateTransition(newButton, bsIdleToOverDown);
					changed = true;
				}
			}
		} break;

		case bsOverUp: {
			FLASHASSERT(button);
			if ( button == newButton ) {
				if ( mouseIsDown ) {
					// The mouse clicked on the button
					DoButtonStateTransition(button, bsOverUpToOverDown);
					changed = true;
				}
			} else {
				// The mouse is over a different button
				DoButtonStateTransition(button, bsOverUpToIdle);
				changed = true;
			}
		} break;

		case bsOverDown: {
			FLASHASSERT(button);
			if ( button == newButton ) {
				if ( !mouseIsDown ) {
					// The mouse was released over the button
					DoButtonStateTransition(button, bsOverDownToOverUp);
					changed = true;
				}
			} else {
				if ( button->character->button.trackAsMenu ) {
					// Exit the button
					DoButtonStateTransition(button, bsOverDownToIdle);
					changed = true;
				} else {
					// The mouse is over a different button
					DoButtonStateTransition(button, bsOverDownToOutDown);
					changed = true;
				}
			}
		} break;

		case bsOutDown: {
			FLASHASSERT(button);
			if ( button == newButton ) {
				// The mouse went back over the button
				DoButtonStateTransition(button, bsOutDownToOverDown);
				changed = true;
			} else if ( !mouseIsDown ) {
				// The mouse was released outside the button
				DoButtonStateTransition(button, bsOutDownToIdle);
				changed = true;
			}
		} break;
	}

	// If we deactivated a button, we may need to activate a new button
	if ( changed && !button && newButton )
		SetButton(newButton, mouseIsDown);	// call again 

	return changed;
}

SObject* DisplayList::HitTest(SPOINT* src)
// Find which object is hit by pt
{
	SPOINT pt = *src;
	if ( antialias ) {
	 	pt.x *= 4;
	 	pt.y *= 4;
	}
	SObject* hitObj = 0;
	root.HitTest(hitObj, camera, &pt);

	return hitObj;
}

SObject* DisplayList::HitButton(SPOINT* src)
// Find which button is hit by pt if any
{
	SPOINT pt = *src;
	if ( antialias ) {
	 	pt.x *= 4;
	 	pt.y *= 4;
	}

	if ( !RectPointIn(&devViewRect, &pt) )
		return 0;

	SObject* hitObj = 0;
	root.HitButton(hitObj, camera, &pt);

	return hitObj;
}

// ActionList

ActionList::ActionList()
{
    maxActions = 32;
    actionMask = 31;
	actionBase = actionTop = 0;
    actionThread = (ScriptThread**) new U8[maxActions*4];
    actionList = (U8 **) new U8[maxActions*4];
}

ActionList::~ActionList()
{
    delete []actionThread;
    delete []actionList;
}

void ActionList::RemoveThread(ScriptThread *thread)
{
	for ( U32 i = actionBase; i < actionTop; i++ ) {
		int j = i & actionMask;
 		if ( actionThread[j] == thread ) {
 			actionList[j] = 0;
 			actionThread[j] = 0;
		}
	}
}

// Save the action on the action list
void ActionList::PushAction(U8 * a, ScriptThread* t) 
{
	U32	actionsDelta = actionTop - actionBase;

    if (actionsDelta >= maxActions && (maxActions * 2) <= maxActionsLimit)
    {
        ScriptThread** tmpThread = (ScriptThread**) new U8[maxActions*4*2];
        U8 **    tmpList = (U8 **) new U8[maxActions*4*2];
        
        if (tmpThread == 0 || tmpList == 0)
        {
            delete []tmpThread;
            delete []tmpList;
        }
        else
        {
			int srcMask = actionMask;
			int dstMask = maxActions * 2 - 1;
			for (U32 i = actionBase; i < actionTop; i++)
			{
				tmpThread[i & dstMask] = actionThread[i & srcMask];
				tmpList[i & dstMask] = actionList[i & srcMask];
			}

            delete []actionThread;
            delete []actionList;
            actionThread = tmpThread;
            actionList = tmpList;
            maxActions = dstMask + 1;
            actionMask = dstMask;
        }
        
    
    }
    
    if (actionsDelta < maxActions) 
    {
        int i = actionTop++ & actionMask;

	    actionList[i] = a;
	    actionThread[i] = t;
	}
}
