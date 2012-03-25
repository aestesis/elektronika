/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright © Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 
****************************************************************************/
//	990325	mnk	"fixed1" -> "fixed_1"

#include "global.h"

#include <stddef.h>
#include <memory.h>
#include <math.h>
#include <stdio.h>

#include "splayer.h"

#include "stags.h"
#include "sobject.h"
#include "memcop.h"
#include "edittext.h"
#include "splay.h"

#include NATIVE_VERSION
#include NATIVE_UTIL
#include NATIVE_SOUND

char *ConvertIntegerToString(int value);

/* Button Semaphore (unused) */

#define ButtonLock()
#define ButtonUnlock()

/*-------------------------------------------------------------------------*/
/* Coefficients used in the pure random number generator. */
#define c3  15731L
#define c2  789221L
#define c1  1376312589L

/* The pure random number generator returns random numbers between zero
/  and this number.  This value is useful to know for scaling random
/  numbers to a desired range. */
#define kRandomPureMax  0x7FFFFFFFL

/* Data structure for state of fast random number generator. */
typedef struct
{
    /* Random result and seed for next random result. */
    U32  uValue;

    /* XOR mask for generating the next random value. */
    U32  uXorMask;

    /* This is the number of values which will be generated in the
    /  sequence given the initial value of n. */
    U32  uSequenceLength;

} TRandomFast, *pTRandomFast;

/* Read-only, XOR masks for random # generation. */
static U32 Random_Xor_Masks[31] =
{
    /* First mask is for generating sequences of 3 (2^2 - 1) random numbers,
    /  Second mask is for generating sequences of 7 (2^3 - 1) random numbers,
    /  etc.  Numbers to the right are number of bits of random number sequence.
    /  It generates 2^n - 1 numbers. */
    0x00000003L, 0x00000006L, 0x0000000CL, 0x00000014L,     /* 2,3,4,5 */
    0x00000030L, 0x00000060L, 0x000000B8L, 0x00000110L,     /* 6,7,8,9 */
    0x00000240L, 0x00000500L, 0x00000CA0L, 0x00001B00L,     /* 10,11,12,13 */
    0x00003500L, 0x00006000L, 0x0000B400L, 0x00012000L,     /* 14,15,16,17 */
    0x00020400L, 0x00072000L, 0x00090000L, 0x00140000L,     /* 18,19,20,21 */
    0x00300000L, 0x00400000L, 0x00D80000L, 0x01200000L,     /* 22,23,24,25 */
    0x03880000L, 0x07200000L, 0x09000000L, 0x14000000L,     /* 26,27,28,29 */
    0x32800000L, 0x48000000L, 0xA3000000L                   /* 30,31,32 */

};                              /* Randomizer Xor mask values  CRK 10-29-90 */
/*-------------------------------------------------------------------------*/

TRandomFast gRandomFast = { 0, 0, 0};

/*-*-------------------------------------------------------------------------
/ Function
/   RandomFastInit
/
/ Purpose
/   This initializes the fast random number generator.
/
/ Notes
/   The fast random number generator has some unique qualities:
/
/   1) It generates an exact and repeatable sequence of pseudorandom
/      integers between 1 and 2^n-1, inclusive.  Zero is not generated.
/   2) The same sequence repeats every 2^n-1 generations.
/   3) Each number can be generated extremely rapidly.
/
/   This returns the first random number in the sequence.
/
/ Entry
/   pRandomFast = Pointer to data structure for current state of a fast
/     pseudorandom number generator.
/   n = A value which sets the number of unique values to generate before
/     repeating.  This value can range from 2 to 32, inclusive.  If an
/     invalid value (such as zero) is passed in, 32 is assumed.
/--------------------------------------------*/
void RandomFastInit(pTRandomFast pRandomFast)
{
    S32 n = 32;

    /* The sequence always starts with 1. */
    //    pRandomFast->uValue = 1L;
    pRandomFast->uValue = GetTimeMSec();

    /* Figure out the sequence length (2^n - 1). */
    pRandomFast->uSequenceLength = (1L << n) - 1L;

    /* Gather the XOR mask value. */
    pRandomFast->uXorMask = Random_Xor_Masks[n - 2];
}
/*-------------------------------------------------------------------------*/

/*-*-------------------------------------------------------------------------
/ Function
/   imRandomFastNext
/
/ Purpose
/   This generates a new pseudorandom number using the fast random number
/   generator.
/
/ Notes
/   The fast random number generator must be initialized before use.
/
/   This is implemented as a macro for the best possible performance.
/
/ Entry
/   pRandomFast = Pointer to data structure for current state of a fast
/     pseudorandom number generator.
/
/ Exit
/   Returns the next pseudorandom number in the sequence.
/--------------------------------------------*/
#define RandomFastNext(_pRandomFast)                                                            \
(                                                                                               \
    ((_pRandomFast)->uValue & 1L)                                                               \
        ? ((_pRandomFast)->uValue = ((_pRandomFast)->uValue >> 1) ^ (_pRandomFast)->uXorMask)   \
        : ((_pRandomFast)->uValue = ((_pRandomFast)->uValue >> 1))                              \
)
/*-------------------------------------------------------------------------*/

/*-*-------------------------------------------------------------------------
/ Function
/   RandomPureHasher
/
/ Purpose
/   This generates a pseudorandom number from a given seed using the high
/   quality random number generator.
/
/ Notes
/   The caller must pass in a seed value.  This value is typically the
/   output of the fast random number generator multiplied by a prime
/   number, but can be any seed which was not derived from the output of
/   this function.
/
/   Please note that this random number generator is really as hasher.
/   It will not work well if you pass its own output in as the next input.
/
/   A given input seed always generates the same pseudorandom output result.
/
/   The feature of this hasher is that the value returned from one seed
/   to the next is highly uncorrelated to the seed value.  High quality
/   multidimensional random numbers can be generated by using a sum of
/   prime multiples of the seed values.
/
/   For example, to generate a vector given three seed values sx, sy and sz,
/   use something like the following:
/     result = imRandomPureHasher(59*sx + 67*sy + 71*sz);
/
/ Entry
/   iSeed = Starting seed value.
/
/ Exit
/   Returns the next pseudorandom value in the sequence.
/--------------------------------------------*/
S32 RandomPureHasher(S32 iSeed)
{
    S32   iResult;

    /* Adapted from "A Recursive Implementation of the Perlin Noise Function,"
    /  by Greg Ward, Graphics Gems II, p. 396. */

    /* First, hash s as a preventive measure.  This helps ensure the first
    /  few numbers are more random when used in conjunction with the fast
    /  random number generator, which starts off with the first 10 or so
    /  numbers all zero in the lowe bits. */
    iSeed = ((iSeed << 13) ^ iSeed) - (iSeed >> 21);

    /* Next, use a third order odd polynomial, better than linear. */
    iResult = (iSeed*(iSeed*iSeed*c3 + c2) + c1) & kRandomPureMax;

    /* DJ14dec94 -- The above wonderful expression always returns odd
    /  numbers, and this is easy to prove.  So we add the seed back to
    /  the result which again randomizes bit zero. */
    iResult += iSeed;

    /* DJ17nov95 -- The above always returns values that are NEVER divisible
    /  evenly by four, so do additional hashing. */
    iResult = ((iResult << 13) ^ iResult) - (iResult >> 21);

    return iResult;
}
/* ------------------------------------------------------------------------------ */
S32 Random(S32 range)
{
    long    aNum;

    /* Fill out gRandomFast if it is uninitialized.
    /  This means seed hasn't been set.  Sequence of numbers will be
    /  the same every time player is run. */
    if (gRandomFast.uValue == 0)
    {
        /* Initialize 32 bit pure random number generator. */
        RandomFastInit(&gRandomFast);
    }

    aNum = RandomFastNext(&gRandomFast);

    /* Use the pure random number generator to hash the result. */
    aNum = RandomPureHasher(aNum * 71L);

    if (range > 0)
    {
        aNum = ( (aNum&0x7FFFFFFF) % range);
    }
    else
        aNum = 0;

    return aNum;
}

/*-------------------------------------------------------------------------*/

#ifdef SOUND
	#ifdef ONE_SOUND_OBJECT
	// If there is only one sound object, initialize it.
	NativeSoundMix SPlayer::theSoundMix;
	#endif
#endif SOUND

SPlayer::SPlayer()
{
	#ifdef SOUND
		display.theSoundMix = &theSoundMix;
	#endif

	// Our Player
	player.SetDisplay(&display);	// the player draws into the display list
	player.splayer = this;
	display.SetBits(&bits);			// the display list draws into the bit buffer

    highQuality = true;
	autoQuality = false;
	bestDither = false;
	lastQualitySwitch = 1000;

    // set the version variable
    SetVersionVariable( &player );

	// Counters for auto quality settings
	nTooSlow = 0;
	nTooFast = 0;
	nTotal = 0;
	//starved = false;

	// # of seconds to prebuffer sound
	nSoundBufferTime = 5;

	// Offset for GetTimer
	mTimerOffset = GetTimeMSec();

	showMenu = true;

	//syncToClock = false;

#ifdef EDITTEXT
	// Text editing
	focus = 0;
	cursorBlink = FALSE;
	builtInFontsState = kBuiltInFontsNotLoaded;
#endif

	// View Control
	scaleMode = DisplayList::showAll;
	RectSetEmpty(&zoomRect);

	actionDepth = 0;

	actionOverflow = false;

	scrolling = false;

	// Play Control
	loaded = false;
	running = false;
	nextFrameTime = 0;

	mouseState = -1;

#ifdef ACTIONSCRIPT
	stack = NULL;
	stackPos = 0;
	stackSize = 0;
#endif

//#ifdef DRAGBUTTONS
#if defined(DRAGBUTTONS) && !defined(GENERATOR)
	dragObject = 0;
	dragStarted = false;
#endif

	// Button initialization
	buttonArray		= NULL;
	buttonIndex		= 0;
	arraySize		= 0;

	// streaming (netscape) initialization
	mode = 0;
	url = 0;
	urlBase = 0;
	name = 0;

	firstStream = true;
	destructing = false;

	#ifdef SOUND
	theSoundMix.Construct();
	#endif

	SetVersionVariable( &player );
}

SPlayer::~SPlayer()
{
	FLASHOUTPUT( "Deleting: SPlayer\n" );

	destructing = true;		// We cannon call NativePlayerWnd functions any more - since NPW has
							// been deleted. Set this flag so NPW functions can be wrapped in
							// if ( !destructing ) calls.
	ClearScript();

	FreeLayers();

#ifdef ACTIONSCRIPT
	// Delete the action stack
	while ( stackPos > 0 )
	{
		FreeStr(Pop());
	}
	delete [] stack;
#endif

	RemoveAllButtons();
	delete url;
	url = 0;

#ifdef SOUND
	theSoundMix.Destruct();
#endif
}


void SPlayer::SetVersionVariable( ScriptPlayer *player )
{
    char version[64];
	
	sprintf( version, "%s %s", PLATFORM_STRING, VERSION_STRING );
    SetVariable(player, "$version", version, false);
}


void SPlayer::DoButton(SPOINT* pt, BOOL mouseIsDown, BOOL updateScreen)
{
	SObject* hitBtn = display.HitButton(pt);

#ifdef EDITTEXT
	if (hitBtn && hitBtn->character->type == editTextChar) {
		if (mouseIsDown) {
			if (!IsSelecting()) {
				XSetCapture();
				if (focus != hitBtn) {
					if (focus) {
						cursorBlink = FALSE;
						focus->editText->ClearSelection();
						focus->Modify();
					}
					focus = hitBtn;
				}
				focus->EditTextMouse(pt, true);
				focus->Modify();
			}			
		} else {
			XReleaseCapture();
		}
		display.iBeam = ((hitBtn->editText->m_flags & seditTextFlagsNoSelect) == 0);
		hitBtn = 0;
	} else {
		display.iBeam = FALSE;
		if (mouseIsDown && focus && !focus->editText->m_selecting) {
			cursorBlink = FALSE;
			focus->editText->ClearSelection();
			focus->Modify();
			focus = NULL;
		}
	}

#endif

	if ( display.SetButton(hitBtn, mouseIsDown) ) {
		if (!RectIsEmpty(&display.tabFocusRect)) {
			display.InvalidateRect(&display.tabFocusRect);
			RectSetEmpty(&display.tabFocusRect);
		}

		DoActions();
		if ( updateScreen )
			UpdateScreen();
		UpdateCursor();

		if ( display.button && mouseIsDown ) {
			XSetCapture();
		} else {
			XReleaseCapture();
		}
	}

	// Update the cached mouse info
	SPOINT dpt = *pt;
	if ( display.antialias ) {
		dpt.x *= 4;
		dpt.y *= 4;
	}
	if ( !RectPointIn(&display.devViewRect, &dpt) ) {
		mouseState = -1;
	} else {
		mouseState = mouseIsDown;
		mousePt = *pt;
	}
}

//#ifdef DRAGBUTTONS
#if defined(DRAGBUTTONS) && !defined(GENERATOR)
void SPlayer::UpdateDragObject(SPOINT *pt)
{
	if (dragObject) {
		SObject *parentObject = dragObject->thread->rootObject;
		// 
		// Let's figure out where we need to position this
		// sprite.
		//
		MATRIX m, invMat;
		SObject *current;

		MatrixIdentity(&m);
		current = parentObject->parent;
		while ( current && (current != &display.root) ) {
			MatrixConcat( &m, &current->xform.mat, &m );
			current = current->parent;
		}
		MATRIX viewmat;
		if (display.antialias) {
			MatrixScale(fixed_1/4, fixed_1/4, &viewmat);
			MatrixConcat(&display.camera.mat, &viewmat, &viewmat);
		} else {
			viewmat = display.camera.mat;
		}
		MatrixConcat(&m, &viewmat, &m);

		MatrixInvert(&m, &invMat);

		SPOINT delta, xformedDelta, newPt;

		// If this drag is of the lock-to-center variety...
		if (dragObject->dragCenter) {
			MatrixTransformPoint(&invMat, pt, &newPt);
		} else {
			delta.x = dragPoint.x - pt->x;
			delta.y = dragPoint.y - pt->y;
			MatrixDeltaTransformPoint(&invMat, &delta, &xformedDelta);

			newPt.x = parentObject->xform.mat.tx - xformedDelta.x;
			newPt.y = parentObject->xform.mat.ty - xformedDelta.y;
		}

		SRECT* constraint = &parentObject->dragConstraint;
		if (!RectIsEmpty(constraint)) {
			if (newPt.x < constraint->xmin) {
				newPt.x = constraint->xmin;
			} else if (newPt.x > constraint->xmax) {
				newPt.x = constraint->xmax;
			}
			if (newPt.y < constraint->ymin) {
				newPt.y = constraint->ymin;
			} else if (newPt.y > constraint->ymax) {
				newPt.y = constraint->ymax;
			}
			
			if (dragObject->dragCenter) {
				MatrixTransformPoint(&m, &newPt, pt);
			} else {
				xformedDelta.x = parentObject->xform.mat.tx - newPt.x;
				xformedDelta.y = parentObject->xform.mat.ty - newPt.y;

				MatrixDeltaTransformPoint(&m, &xformedDelta, &delta);

				pt->x = dragPoint.x - delta.x;
				pt->y = dragPoint.y - delta.y;
			}
		}

		parentObject->xform.mat.tx = newPt.x;
		parentObject->xform.mat.ty = newPt.y;

		parentObject->Modify();

		dragPoint = *pt;

		UpdateDropTarget();
	}
}
#endif

void SPlayer::MouseMove( int x, int y, BOOL mouseIsDown)
{
	SPOINT pt;
	pt.x = x;
	pt.y = y;

	// If we're doing a partial first frame, don't accept mouse downs yet
	if (!loaded) {
		return;
	}

//#ifdef DRAGBUTTONS
#if defined(DRAGBUTTONS) && !defined(GENERATOR)
	if (dragObject) {
		UpdateDragObject(&pt);
	}
#endif

#ifdef EDITTEXT
	if (IsSelecting()) {
		focus->EditTextMouse(&pt, true);
		focus->Modify();
		return;
	}
#endif

	if ( scrolling ) {
		// Track scrolling when zoomed in
		FLASHASSERT(mouseIsDown);
		MATRIX m, invMat;
		if ( display.antialias ) {
			MatrixScale(fixed_1/4, fixed_1/4, &m);
			MatrixConcat(&display.camera.mat, &m ,&m);
		} else {
			m = display.camera.mat;
		}
		MatrixInvert(&m, &invMat);

		SRECT viewRect, docRect;
		ClientRect(&viewRect);
		MatrixTransformRect(&m, &player.frame, &docRect);

		// Calculate the scroll limits
		SRECT limit;
		limit.xmin = docRect.xmin - viewRect.xmin;
		if ( limit.xmin > 0 ) limit.xmin = 0;
		limit.xmax = docRect.xmax - viewRect.xmax;
		if ( limit.xmax < 0 ) limit.xmax = 0;
		limit.ymin = docRect.ymin - viewRect.ymin;
		if ( limit.ymin > 0 ) limit.ymin = 0;
		limit.ymax = docRect.ymax - viewRect.ymax;
		if ( limit.ymax < 0 ) limit.ymax = 0;

		SPOINT delta;
		delta.x = scrollPt.x - pt.x;
		delta.y = scrollPt.y - pt.y;
		if ( delta.x < limit.xmin )
			delta.x = limit.xmin;
		else if ( delta.x > limit.xmax ) 
			delta.x = limit.xmax;

		if ( delta.y < limit.ymin )
			delta.y = limit.ymin;
		else if ( delta.y > limit.ymax ) 
			delta.y = limit.ymax;

		// Show the updated view after a 1/2 second pause
		zoomRect = viewRect;
		RectOffset(delta.x, delta.y, &zoomRect);
		MatrixTransformRect(&invMat, &zoomRect, &zoomRect);
		SetCamera(updateNow);

		UpdateScreen();

		scrollPt = pt;
	} else {
		DoButton(&pt, mouseIsDown);
	}
}

void SPlayer::MouseDown( int x, int y )
{
	SPOINT pt;
	pt.x = x;
	pt.y = y;

	// If we're doing a partial first frame, don't accept mouse downs yet
	if (!loaded) {
		return;
	}

	DoButton(&pt, true);
	if ( display.button ) {
		return;
	}
#ifdef EDITTEXT
	if (IsSelecting()) {
		return;
	}
#endif
		
	if ( !RectIsEmpty(&zoomRect) ) {
		// Use the hand to scroll
		scrollPt = pt;

		scrolling = true;
		XSetCapture();
		return;
	}

	// Just incase, release the capture
	XReleaseCapture();
}

#ifdef DRAGBUTTONS
void SPlayer::UpdateDropTarget()
{
	int saveVisible = dragObject->visible;

	dragObject->visible = false;	
	SObject *dropObject = dragObject->display->HitTest(&mousePt);
	dragObject->visible = saveVisible;	

	FreeStr(dragObject->dropTarget);

	if (dropObject) {
		dragObject->dropTarget = GetTargetPath(dropObject->thread->rootObject);
	} else {
		dragObject->dropTarget = NULL;
	}
}
#endif

void SPlayer::MouseUp( int x, int y )
{
	SPOINT pt;
	pt.x = x;
	pt.y = y;

	// If we're doing a partial first frame, don't accept mouse downs yet
	if (!loaded) {
		return;
	}

#ifdef EDITTEXT
	if (IsSelecting()) {
		focus->EditTextMouse(&pt, false);
		focus->Modify();
		return;
	}
#endif

	if ( scrolling ) {
		scrolling = false;
	} else {
		DoButton(&pt, false);
	}

	XReleaseCapture();
}

void SPlayer::CancelCapture(BOOL outsideWindow)
{
	// We lost the capture
	scrolling = false;
	if ( outsideWindow ) {
		// We lost the mouse entirely
		mouseState = -1;
		if ( display.SetButton(0, false) ) {
			DoActions();
			UpdateScreen();
		}
	} else {
		// We just lost the mousedown capture
		mouseState = 0;
	}
	UpdateCursor();
}

ScriptPlayer* FindLayer(ScriptPlayer* layer, int layerDepth)
{
	while ( layer ) {
		if ( layer->layerDepth == layerDepth )
			return layer;
		layer = layer->nextLayer;
	}
	return 0;
}

void SPlayer::ClearLayer(int layerDepth)
{
	if ( layerDepth == 0 ) {
		// For layer 0, reset everything
		ClearScript();
	} else {
		// For all other layers, delete the player
		ScriptPlayer** link = &player.nextLayer;
		while ( true ) {
			ScriptPlayer* layer = *link;
			if ( !layer ) break;
			if ( layer->layerDepth == layerDepth ) {
				*link = layer->nextLayer;
				delete layer;
				return;
			}
			link = &layer->nextLayer;
		}
	}
}

ScriptPlayer* SPlayer::OpenVariableLoader(int layer, const char *spriteName)
{
	ScriptPlayer *player = new ScriptPlayer;
	player->splayer = this;
	player->variableLoader = true;
	player->variableLayer = layer;
	player->variableSprite = CreateStr((char *)spriteName);
	return player;
}

/////////////////////////////////////////
// create a place holder for the 
// load external sprite call
//
ScriptPlayer* SPlayer::OpenSprite(ScriptThread *targetThread)
{
    ScriptPlayer    *sprite = 0;
    SCharacter*     ch = 0;
    PlaceInfo       info;
    BOOL            success = false;
    SObject         *rootObject = 0;
    SObject         *srcObject = 0;

	if (!targetThread)
        goto exit_gracefully;

    srcObject = targetThread->rootObject;

    sprite = new ScriptPlayer;
	sprite->splayer = this;

	if (!sprite)
        goto exit_gracefully;

    // don't use regular character alloc
    // on this. Use new intead. We treat this
    // ch differently since it's external
	ch = new SCharacter;
	if ( ch ) 
    {
        // create a unique character becuase
        // the player will be different than the
        // main movie's player
		ch->next = 0;;
		ch->player = sprite;
		ch->tag = ctagExternalThreadRoot;
		ch->tagCode = 0;
		ch->type = spriteExternalChar;
		ch->data = 0;
		ch->bounds = srcObject->character->bounds;
	}
    else
        goto exit_gracefully;

	info.flags = splaceCharacter | splaceMatrix;
	info.depth = srcObject->depth;
    info.mat = srcObject->xform.mat;
    info.puppet = srcObject->puppet;
	info.character = ch;

    if (srcObject->xform.cxform.HasTransform())
    {
        info.flags |= splaceColorTransform;
        info.cxform = srcObject->xform.cxform;
    }
    else
		info.cxform.Clear();

    // we inherit the ratio of the object we are trying
    // replace
    info.ratio = srcObject->ratio;

    info.name = CreateStr(srcObject->name);
    if (info.name)
        info.flags |= splaceName;

    if (srcObject->clipDepth > 0)
    {
        info.flags |= splaceDefineClip;
		info.clipDepth = srcObject->clipDepth;
    }
    else
		info.clipDepth = 0;

    rootObject = srcObject->parent;
    rootObject->thread->RemoveSprite(targetThread, false);
	sprite->rootObject = display.PlaceObject(rootObject, &info);
	sprite->rootObject->thread = sprite;
    
    success = true;
    
exit_gracefully:

    if (!success)
    {
        if (ch) {
	        sprite->characterAlloc.Free(ch);
		}

        delete sprite;
        sprite = 0;
    }
            
	return sprite;
}

ScriptPlayer* SPlayer::OpenLayer(int layerDepth, BOOL loadVars)
{
	if ( layerDepth == 0 && player.script && !loadVars) 
		ClearScript();			// For layer 0, reset everything

	// See if the layer already exists
	ScriptPlayer* layer = FindLayer(&player, layerDepth);
	if ( !layer ) {
		// Create a new layer
		FLASHASSERT(layerDepth != 0);
		layer = new ScriptPlayer;
		if ( !layer ) 
			return 0;
		layer->nextLayer = player.nextLayer;
		player.nextLayer = layer;
		layer->SetDisplay(&display);
		layer->splayer = this;
		layer->layerDepth = layerDepth;
	} else {
		// Empty an existing layer
		if ( (layer->script || layerDepth != 0) && !loadVars )
			layer->ClearScript();
	}

    // set the version variable
    SetVersionVariable( layer );

	return layer;
}

void SPlayer::Zoom(SPOINT size, SPOINT center) 
{
	if ( scaleMode == DisplayList::noScale )
		scaleMode = DisplayList::showAll;

	SRECT viewRect;
	ClientRect(&viewRect);

	// Never more than 1:1 enlargement
	if ( size.x < RectWidth(&viewRect) )
		size.x = RectWidth(&viewRect);

	if ( size.y < RectHeight(&viewRect) )
		size.y = RectHeight(&viewRect);

	zoomRect.xmin = center.x - size.x/2;
	zoomRect.ymin = center.y - size.y/2;
	zoomRect.xmax = center.x + size.x/2;
	zoomRect.ymax = center.y + size.y/2;

	SCOORD adjust = zoomRect.xmin - player.frame.xmin;
	if ( adjust < 0 ) {
		zoomRect.xmin -= adjust;
		zoomRect.xmax -= adjust;
	}

	adjust = zoomRect.xmax - player.frame.xmax;
	if ( adjust > 0 ) {
		zoomRect.xmin -= adjust;
		zoomRect.xmax -= adjust;
	}

	adjust = zoomRect.ymin - player.frame.ymin;
	if ( adjust < 0 ) {
		zoomRect.ymin -= adjust;
		zoomRect.ymax -= adjust;
	}

	adjust = zoomRect.ymax - player.frame.ymax;
	if ( adjust > 0 ) {
		zoomRect.ymin -= adjust;
		zoomRect.ymax -= adjust;
	}

	RectIntersect(&player.frame, &zoomRect, &zoomRect);
	if ( RectIsEmpty(&zoomRect) ||
		(size.x >= RectWidth(&player.frame) && 
		 size.y >= RectHeight(&player.frame)) ) {
		// We are zoomed out past the maximum
		Home();
	} else {
		//RectIntersect(&zoomRect, &player.frame, &zoomRect);
		SetCamera(updateLazy);
	}
}

void SPlayer::ZoomF(SFIXED factor, SPOINT* pt) 
{
	MATRIX m, invMat;
	if ( display.antialias ) {
		MatrixScale(fixed_1/4, fixed_1/4, &m);
		MatrixConcat(&display.camera.mat, &m ,&m);
	} else {
		m = display.camera.mat;
	}
	MatrixInvert(&m, &invMat);

	SRECT viewRect;
	ClientRect(&viewRect);
	MatrixTransformRect(&invMat, &viewRect, &zoomRect);

	SPOINT size, center;
	size.x = RectWidth(&zoomRect);
	size.y = RectHeight(&zoomRect);

	if ( pt ) {
		MatrixTransformPoint(&invMat, pt, &center);
	} else {
		center.x = (zoomRect.xmin + zoomRect.xmax)/2;
		center.y = (zoomRect.ymin + zoomRect.ymax)/2;
	}

	size.x = FixedMul(factor, size.x);
	size.y = FixedMul(factor, size.y);

	Zoom(size, center);
}

void SPlayer::Zoom(SRECT* r) 
{
	SPOINT size, center;
	size.x = RectWidth(r);
	size.y = RectHeight(r);

	center.x = (r->xmin + r->xmax)/2;
	center.y = (r->ymin + r->ymax)/2;

	Zoom(size, center);
}

void SPlayer::Zoom100()
{
	if ( scaleMode & DisplayList::zoom100Scroll ) {
		SRECT vr;
		ClientRect(&vr);
		zoomRect.xmin = zoomRect.ymin = 0;
		zoomRect.xmax = 20*RectWidth(&vr);
		zoomRect.ymax = 20*RectHeight(&vr);
	} else {
		RectSetEmpty(&zoomRect);
		scaleMode = DisplayList::noScale;
	}
	SetCamera(updateLazy);
}

void SPlayer::ZoomScale(SFIXED scale) 
{
	/*
	MATRIX m, invMat;
	if ( display.antialias ) {
		MatrixScale(fixed_1/4, fixed_1/4, &m);
		MatrixConcat(&display.camera.mat, &m ,&m);
	} else {
		m = display.camera.mat;
	}
	MatrixInvert(&m, &invMat);

	SRECT viewRect;
	ClientRect(&viewRect);
	MatrixTransformRect(&invMat, &viewRect, &zoomRect);
	*/
	/*
	SRECT viewRect;
	ClientRect(&viewRect);
	SPOINT size, center;
	size.x = 20*RectWidth(&viewRect);
	size.y = 20*RectHeight(&viewRect);

	center.x = (zoomRect.xmin + zoomRect.xmax)/2;
	center.y = (zoomRect.ymin + zoomRect.ymax)/2;

	size.x = FixedDiv(size.x, scale);
	size.y = FixedDiv(size.y, scale);

	Zoom(size, center);
	*/
}

void SPlayer::Home() 
{
	if ( scaleMode == DisplayList::noScale )
		scaleMode = DisplayList::showAll;

	RectSetEmpty(&zoomRect);
	SetCamera(updateLazy); 
}

void SPlayer::SetCamera(int update)
// Set the camera mat so that the picture frame fills the window
{
	SRECT vr;
	ClientRect(&vr);
	display.faster = player.numFrames > 1 && !bestDither;
	display.SetCamera(RectIsEmpty(&zoomRect) || scaleMode == DisplayList::noScale ? 
						&player.frame : &zoomRect, &vr, highQuality, scaleMode);
	switch ( update ) {
		case updateLazy:
			CheckUpdate();
			break;			
		case updateNow:
			UpdateScreen();
			break;
	}
}

void SPlayer::FreeLayers()
{
	// Note layer[0] is special and should not be deleted
	while (true) {
		ScriptPlayer* layer = player.nextLayer;
		if ( !layer ) 
			break;
		player.nextLayer = layer->nextLayer;
		delete layer;
	}

	player.ClearScript();
}

void SPlayer::ClearScript()
// Clear all of the layers in the movie
{
	Suspend();

	FreeLayers();

	RectSetEmpty(&zoomRect);
	loaded = false;

#ifdef DRAGBUTTONS
	dragObject = 0;
#endif
#ifdef EDITTEXT
	focus = 0;
	saveFocus.Clear();
#endif

	// This is a spagetti situation:
	// SetCamera will call 
	//		NativePlayerWnd::ClientRect(&vr)
	// which is already destroyed. So we set a 
	// flag to suppress this call.
	if ( !destructing )
	{
		SetCamera(updateLazy);
	}
}

void SPlayer::GotoFrame(int f)
{
	//FLASHOUTPUT("Goto %i\n", f);
	player.Seek(f);
	if ( actionDepth == 0 ) {
		DoActions();
		CheckUpdate();
	}
}

void SPlayer::FreeBuffer()
{
	bits.FreeBits();
	display.Invalidate();
}


ScriptThread* SPlayer::FindTarget(SObject* base, char* path)
// Path notation
//
// "/fred"			- the object named fred that is a top level object of the movie
// ".."				- the thread's parent
// "fred/bob"		- the object named fred that is a top level object of the movie
// "fred"			- the object named fred that is a child of this thread
// "../../fred"		- the object named fred that is a child of this threads parent's parent
// "" - self		- blank refers to the current thread
// "_flash1/bob"	- the object named bob at the top level of layer1
{
	FLASHASSERT(base);

	BOOL top = true;

	if ( path[0] == '/' ) {
		// Pop up to the root
		while ( base->parent && base->parent != &display.root ) {
			base = base->parent;
		}
		path++;
		top = false;
	}

	// Parse the path elements
	while ( true ) {
		char name[128];

		// We have parsed the path, return the object
		if ( *path == 0 ) 
			return base->thread;

		{// Pull off a name
			int i = 0;
			while ( *path && i < 127 ) {
				if ( *path == '/' ) {
					path++;
					break;
				}
				name[i++] = *path++;
			}
			name[i] = 0;
		}

		// See if they reference a layer directly (only check at the top element)
		if ( top ) {
			top = false;
			int layerNum = LayerNum(name);
			if ( layerNum >= 0 ) {
				ScriptPlayer* l = FindLayer(&player, layerNum);
				if ( l && l->rootObject ) {
					base = l->rootObject;
					continue;
				} else {
					return 0;
				}
			}
		}

		if ( StrEqual(name, "..") ) {
			// Visit the parent
			base = base->parent;
			if ( !base ) 
				return 0;
		} else {
			// Find an object that matches the name
			for ( SObject* obj = base->bottomChild; obj; obj = obj->above ) {
				// Does this object match?
				if ((obj->character->type == spriteChar || obj->character->type == spriteExternalChar) 
                    && obj->name && StrEqual(obj->name, name) ) {
					base = obj;
					goto found;
				}
			}
			return 0;

		  found:
			;
		}
	}
}

char* SPlayer::GetTargetPath(SObject* base)
{
	enum { MAX_TARGET_DEPTH = 256 };

	SObject* stack[MAX_TARGET_DEPTH];
	int stackPtr = 0;

	SObject* obj = base;

	// Find the path to the root
	while (obj && obj->character->type != rootChar) {
		stack[stackPtr++] = obj;
		if (stackPtr == MAX_TARGET_DEPTH) {
			break;
		}
		obj = obj->parent;
	}

	URLBuilder ub;

	int depth = obj->thread->player->layerDepth;
	if (depth) {
		ub.AppendString("_level");
		ub.AppendInt(depth);
	}

	// Copy the names of the target paths
	while (stackPtr) {
		char *name = stack[--stackPtr]->name;
		ub.AppendChar('/');
		ub.AppendString(name ? name : "?");
	}

	if (!ub.len) {
		ub.AppendChar('/');
	}

	return CreateStr(ub.buf);
}

// ConvertStringToInteger: Converts an ASCII string to an integer
//

BOOL ConvertStringToInteger( const char *s, int *value)
{
	BOOL negate = false;
	int result = 0;

	// Check sign
	if (*s == '-') {
		negate = true;
		s++;
	} else if (*s == '+') {
		s++;
	}

	// Make sure there actually are digits
	if (!*s) {
		return false;
	}

	// Read the digits
	while (*s) {
		if (*s >= '0' && *s <= '9') {
			result = result * 10 + *s - '0';
		} else {
			return false;
		}
		s++;
	}

	if (negate) {
		result = -result;
	}
	*value = result;
	return true;
}

// convertdoubletostring: Converts a double-precision floating-point number
// to its ASCII representation
//

#define LOG2_10 0.301029995663981

static int NextDigit(double *value)
{
  int digit;
  digit = (int) *value;
  *value -= digit;
  *value *= 10;
  return digit;
}

static int RoundInt(double x)
{
	if (x < 0) {
		return (int) (x - 0.5);
	} else {
		return (int) (x + 0.5);
	}
}

double PowerOfTen(int exponent)
{
	double base = 10.0;

	if (exponent < 0) {
		base = 0.1;
		exponent = -exponent;
	}

	double result = 1.0;

	while (exponent) {
		if (exponent & 1) {
			result *= base;
		}
		exponent >>= 1;
		base *= base;
	}

	return result;
}

char *ConvertDoubleToString(double value)
{
	int i;
	int prec = 15; // base 10 precision

	char buffer[256];
	char *s = buffer;

	// Deal with negative numbers
	if (value < 0) {
		value = -value;
		*s++ = '-';
	}

	// Determine base 2 exponent
	int exp2;
	frexp(value, &exp2);
	exp2--;

	// Convert to base 10 exponent
	int exp10 = (int) RoundInt((double)exp2 * LOG2_10);

	// Obtain base 10 mantissa
	double pow10 = PowerOfTen(exp10);
	double f = value / pow10;

	// Adjust for round-off error on exponent
	if ((int) f == 0) {
		exp10--;
		pow10 = PowerOfTen(exp10);
		f = value / pow10;
	}

	// Sentinel is used for rounding
	char *sentinel = s;

	if (exp10 < 0 && exp10 > -6) {
		// Number is of form 0.######
		if (exp10 < -prec) {
			exp10 = -prec-1;
		}
		sentinel = s;
		*s++ = '0'; // Sentinel
		*s++ = '0';
		*s++ = '.';

		// Write out leading zeros
		for (i=exp10; i<-1; i++) {
			*s++ = '0';
		}
		// Write out significand
		for (i=0; i<prec; i++) {
			*s++ = NextDigit(&f) + '0';
		}
		exp10 = 0;
	} else if (exp10 >= prec) {
		*s++ = NextDigit(&f) + '0';
		*s++ = '.';
		for (i=0; i<prec-1; i++) {
			*s++ = NextDigit(&f) + '0';
		}
	} else {
		int digits = 0;
		sentinel = s;
		*s++ = '0';
		i = NextDigit(&f);
		if (i) {
			*s++ = i + '0';
		}
		while (exp10 > 0) {
			*s++ = NextDigit(&f) + '0';
			exp10--;
			digits++;
		}
		*s++ = '.';
		for (; digits < prec-1; digits++) {
			*s++ = NextDigit(&f) + '0';
		}
    }

	// Rounding
	i = NextDigit(&f);
	if (i > 4) {
		char *ptr = s-1;
		while (ptr >= buffer) {
			if (*ptr < '0') {
				ptr--;
				continue;
			}
			(*ptr)++;
			if (*ptr != 0x3A) {
				break;
			}
			*ptr-- = '0';
		}
    }

	/* Remove trailing zeros */
	while (*(s-1) == '0') {
		s--;
	}
	if (*(s-1) == '.') {
		s--;
	}

	if(exp10) {
		*s++ = 'e';
		char *expstr = ConvertIntegerToString(exp10);
		char *t = expstr;
		while (*t) { *s++ = *t++; }
		FreeStr(expstr);
	}
  
	*s = '\0';

	if (sentinel && sentinel[0] == '0' && sentinel[1] != '.') {
		char *s = sentinel;
		char *t = sentinel+1;
		while ((*s++ = *t++) != 0);
	}

	char *result = new char[strlen(buffer) + 1];
	strcpy(result, buffer);
	return result;
}



// ConvertStringToDouble: Converts an ASCII string in the form
//     [+-]######.######e[+-]###
// to a double-precision floating-point number
//

BOOL ConvertStringToDouble( const char *s, double *value)
{
	BOOL negate = false;
	int exp10 = 0;
	double result = 0;

	// Handle sign
	if (*s == '-') {
		negate = true;
		s++;
	} else if (*s == '+') {
		s++;
	}
	// Make sure it's not - and + and nothing else
	if (!*s) {
		return false;
	}

	// Read digits to left of decimal point
	while (*s) {
		if (*s >= '0' && *s <= '9') {
			result = result * 10 + *s++ - '0';
		} else {
			break;
		}
	}

	// If decimal point encountered, read
	// digits to right of decimal point
	if (*s == '.') {
		while (*++s) {
			if (*s >= '0' && *s <= '9') {
				result = result * 10 + *s - '0';
				exp10--;
			} else {
				break;
			}
		}
	}

	// Handle exponent
	if (*s == 'e' || *s == 'E') {
		int num;
		if (!ConvertStringToInteger(++s, &num)) {
			return false;
		}
		exp10 += num;
	} else if (*s) {
		// Unknown character encountered
		return false;
	}

	if (negate) {
		result = -result;
	}
	*value = result * PowerOfTen(exp10);
	return true;
}

BOOL IsTrue( const char* c)
{
	double numval;
	if ( ConvertStringToDouble(c, &numval) )
	{
		return numval != 0;
	}
	return StrEqual(c, "true") || StrEqual(c, "yes");
}
// ConvertIntegerToString: Converts a signed integer to its ASCII representation

char *ConvertIntegerToString(int value)
{
	URLBuilder ub;
	if (value < 0) {
		ub.AppendChar('-');
		value = -value;
	}
	ub.AppendInt(value);
	return CreateStr(ub.buf);
}

#ifdef _CELANIMATOR
// Generate an error message
void ShowTargetError(SPlayer* player, SObject*, char*);

// Send a string to the trace window
void TraceString(const char *message);
void ExecutionLimitError();
void CallLimitError();
#endif



////////////////////////////////////////////////////////////
// Clone
//      Create a new sprite that is a copy of the current one.
//      NOTE: State information (current frame, variables, etc.)
//              is not cloned.
//
// inputs:
//      thread  - the sprite we want to clone
//      name    - the name of the new sprite that we are about to 
//                  create. space for this name has been allocated
//                  no need to copy.
//      depth   - what layer we want to place the cloned sprite in
void CloneSprite(ScriptThread *thread, char *name, int depth)
{
    if (thread)
    {
        // don't allow duplication of main timeline
        if (thread == thread->player)
            return;
        
        thread->rootObject->parent->thread->CloneSprite(thread, name, depth);
    }
}

void SPlayer::RemoveSprite(ScriptThread *thread, BOOL createPlaceholder)
{
    if (thread)
    {
        if (thread->rootObject->parent == &display.root)
            ClearLayer(thread->layerDepth);
        else
            thread->rootObject->parent->thread->RemoveSprite(thread, createPlaceholder);
    }
}


void SPlayer::SetProperty(ScriptThread *thread, int propType, double value)
{
    if (thread == 0)
        return;

    SObject *object = thread->rootObject;

    switch (propType)
    {
		// Sprite properties
        case kSpritePosX: {
				SCOORD tx = (SCOORD) (value * 20); // turn into twips
				if ( object->xform.mat.tx != tx ) {
					object->xform.mat.tx = tx;
					object->Modify();
				}			
				object->puppet = true;
			} break;
        case kSpritePosY: {
				SCOORD ty = (SCOORD) (value * 20); // turn into twips
				if ( object->xform.mat.ty != ty ) {
					object->xform.mat.ty = ty;
					object->Modify();
				}			
				object->puppet = true;
            } break;
        case kSpriteScaleX: {
				SFIXED scalex = DoubleToFixed(value/100.0);
				SPOINT pt;
				pt.x = object->xform.mat.a;
				pt.y = object->xform.mat.b;
				SFIXED curScalex = PointLength(&pt);
				if ( !FixedEqual(scalex, curScalex, 0x10) ) {
					SFIXED rotX = FixedAtan2(object->xform.mat.b, object->xform.mat.a);
					object->xform.mat.a =  FixedMul(scalex, FixedCos(rotX));
					object->xform.mat.b =  FixedMul(scalex, FixedSin(rotX));
					object->Modify();
				}
				object->puppet = true;
            } break;
        case kSpriteScaleY:{
				SFIXED scaley = DoubleToFixed(value/100.0);
				SPOINT pt;
				pt.x = object->xform.mat.c;
				pt.y = object->xform.mat.d;
				SFIXED curScaley = PointLength(&pt);
				if ( !FixedEqual(scaley, curScaley, 0x10) ) {
					SFIXED rotY = FixedAtan2(-object->xform.mat.c, object->xform.mat.d);
					object->xform.mat.c = -FixedMul(scaley, FixedSin(rotY));
					object->xform.mat.d =  FixedMul(scaley, FixedCos(rotY));
					object->Modify();
				}
				object->puppet = true;
            } break;
        case kSpriteAlpha: {
				// (a value between 0 and 100 %)
				S16 aa = (S16) (value * (256.0 / 100.0));
				if ( object->xform.cxform.aa != aa ) {
					object->xform.cxform.aa = aa;
					object->xform.cxform.CalcFlags();
					object->Modify();
				}
				object->puppet = true;
            } break;
        case kSpriteVisible: {
				// (if zero this means we don't hit test the object)
				int vis = (value != 0.0f) ? 1 : 0;
				if ( object->visible != vis ) {
					object->visible = vis;
					object->Modify();
				}
			} break;
        case kSpriteRotate:
            {
				MATRIX mat = object->xform.mat;
				SPOINT vec;
				vec.x = mat.a;
				vec.y = mat.b;
				SFIXED scalex = PointLength(&vec);

				vec.x = mat.c;
				vec.y = mat.d;
				SFIXED scaley = PointLength(&vec);

				SFIXED rotX = FixedAtan2(mat.b, mat.a);
				SFIXED rotY = FixedAtan2(-mat.c, mat.d);
				SFIXED newRot = DoubleToFixed(value);

				mat.a =  FixedMul(scalex, FixedCos(newRot));
				mat.b =  FixedMul(scalex, FixedSin(newRot));
				mat.c = -FixedMul(scaley, FixedSin(rotY-rotX + newRot));
				mat.d =  FixedMul(scaley, FixedCos(rotY-rotX + newRot));

				if ( !MatrixEqual(&mat, &object->xform.mat, 0x10) ) {	// note that we don't change the tx or ty values ever...
					object->xform.mat = mat;
					object->Modify();
				} else {
					FLASHOUTPUT("skip\n");
				}

                object->puppet = true;
            }
            break;

		// Global properties
		case kSpriteHighQuality:
			highQuality = (value != 0.0);
			bestDither = (value > 1.5);	// really should be 2.0 but be flexible...
			autoQuality = false;
			SetCamera(updateLazy);
			break;
		case kSpriteFocusRect:
			display.useFocusRect = (value != 0.0);
			if (!RectIsEmpty(&display.tabFocusRect)) {
				display.InvalidateRect(&display.tabFocusRect);
			}
			break;
		case kSpriteSoundBufferTime:
			nSoundBufferTime = (int) value;
			break;

        default:
            // we should never get here
            FLASHASSERT(0);
    }
}

void SPlayer::SetPropertyStr(ScriptThread *thread, int propType, char *value)
{
    if (thread == 0)
        return;

    SObject *object = thread->rootObject;

	switch (propType) {
	case kSpriteName:
		FreeStr(object->name);
		object->name = CreateStr(value);
		break;
	case kSpriteTarget:
	case kSpriteURL:
	case kSpriteDropTarget:
		// (can only get, not set)
		break;
	default:
		{
			double num;
			if (ConvertStringToDouble(value, &num)) {
				SetProperty(thread, propType, num);
			}
		}
		break;
	}
}

ScriptThread* SPlayer::ResolveFrameNum(char *name, ScriptThread *thread, int *frameNum)
{
	if (!thread) {
		// Use the root
		thread = &player;
	}

	char *funcName = NULL;
	ScriptThread *targetThread = ResolveVariable(name, thread, &funcName);
	if (targetThread) {
		// Convert function name/number to frame number
		if (ConvertStringToInteger(funcName, frameNum)) {
			(*frameNum)--;
		} else {
			*frameNum = targetThread->FindLabel(funcName);
		}
		// Fail if label not found, or if frame number is negative
		if (*frameNum < 0) {
			return NULL;
		}
	}
	return targetThread;
}

ScriptThread *SPlayer::ResolveVariable(char *name, ScriptThread *thread, char **varName)
{
	if (!thread) {
		// Use the root
		thread = &player;
	}

	char *colon = strchr(name, ':');
	if (colon) {
		char *targetName = CreateStr(name, colon-name);

		ScriptThread* result = FindTarget(thread->rootObject, targetName);
		FreeStr(targetName);

		*varName = colon+1;
		return result;
	} else {
		*varName = name;
		return thread;
	}
}

char* SPlayer::GetVariable(ScriptThread *thread, char *name)
{
	char *result = NULL;

	char *varName = NULL;
	ScriptThread* targetThread = ResolveVariable(name, thread, &varName);
	if (targetThread) {
#ifdef EDITTEXT
		char *dot = strchr(varName, '.');
		if (dot) {
			// Suffix encountered.  It must be scroll or maxscroll.
			int operation;
			if (StrEqual(dot, ".scroll")) {
				operation = editTextGetScroll;
			} else if (StrEqual(dot, ".maxscroll")) {
				operation = editTextGetMaxScroll;
			} else {
				// Invalid suffix, return nothing.
				return NULL;
			}
			// Construct name without suffix
			char *editTextName = CreateStr(varName, dot-varName);
			if (editTextName) {
				int intval;
				if (display.root.ScrollEditText(targetThread, editTextName, operation, intval)) {
					result = ConvertIntegerToString(intval);
				}
				FreeStr(editTextName);
			}
		} else
#endif
		{
			ScriptVariable* var = targetThread->FindVariable(varName);
			if (var) {
				result = CreateStr(var->value);
			}
		}
	}

	return result;
}

void SPlayer::SetVariable(ScriptThread *thread, char *name, char *value, BOOL updateFocus)
{
	char *varName;
	ScriptThread* targetThread = ResolveVariable(name, thread, &varName);
	if (targetThread) {
#ifdef EDITTEXT
		char *dot = strchr(varName, '.');
		if (dot) {
			// Suffix encountered.  Must be .scroll, otherwise do nothing
			if (StrEqual(dot, ".scroll")) {
				char *editTextName = CreateStr(varName, dot-varName);
				if (editTextName) {
					int intval;
					if (ConvertStringToInteger(value, &intval)) {
						display.root.ScrollEditText(targetThread, editTextName, editTextSetScroll, intval);
					}
					FreeStr(editTextName);
				}
			}
		} else
#endif
		{
			targetThread->SetVariable(varName, value, updateFocus);
		}
	}
}

void GetBoundingBox(SObject* obj, P_SRECT bounds);

double SPlayer::GetProperty(ScriptThread *thread, int propType)
{
    double result = 0.0;
    SObject *object = 0;

    if (thread == 0)
        goto exit_gracefully;
        
    object = thread->rootObject;

    switch (propType)
    {
        case kSpritePosX:
            result = (double) object->xform.mat.tx;
            result /= 20; // convet twips into pixels.
            break;
        case kSpritePosY:
            result = (double) object->xform.mat.ty;
            result /= 20; // convet twips into pixels.
            break;
        case kSpriteScaleX: {
			SPOINT vec;
			vec.x = object->xform.mat.a;
			vec.y = object->xform.mat.b;
			result = FixedToDouble(PointLength(&vec)*100);
			} break;
        case kSpriteScaleY: {
			SPOINT vec;
			vec.x = object->xform.mat.c;
			vec.y = object->xform.mat.d;
			result = FixedToDouble(PointLength(&vec)*100);
			} break;
        case kSpriteCurFrame:
            result = (double) (thread->curFrame+1);
            break;
        case kSpriteTotalframes:     
            result = (double) thread->numFrames;
            break;
        case kSpriteAlpha:
            result = (double) (object->xform.cxform.aa * 100) / 256;
            break;
        case kSpriteVisible:
            result = object->visible;
            break;
        case kSpriteWidth:
			{
				SRECT bounds;
				GetBoundingBox(object, &bounds);
				result = (double) (bounds.xmax - bounds.xmin) / 20;
			}
            break;
        case kSpriteHeight:           
			{
				SRECT bounds;
				GetBoundingBox(object, &bounds);
				result = (double) (bounds.ymax - bounds.ymin) / 20;
			}
            break;
        case kSpriteLastFrameLoaded:
            {
                if (thread == thread->player) {
                    // this is the main movie
					if (thread->player->numFramesComplete < thread->numFrames) {
						result = (double) thread->player->numFramesComplete;
					} else {
						result = (double) thread->numFrames;
					}
				} else {
                    // movie sprites are alwayes fully loaded 
                    // before they start playing
                    result = (double) thread->numFrames;
				}
            }
            break;
        case kSpriteRotate:
            {
				SFIXED rotX = FixedAtan2(object->xform.mat.b, object->xform.mat.a);
				//if ( rotX < 0 )
				//	rotX += FC(360);
				result = FixedToDouble(rotX);
            }
            break;
		case kSpriteHighQuality:
			if (highQuality) {
				result = bestDither ? 2.0 : 1.0;
			} else {
				result = 0.0;
			}
			break;
		case kSpriteFocusRect:
			result = display.useFocusRect ? 1.0 : 0.0;
			break;
		case kSpriteSoundBufferTime:
			result = (double) nSoundBufferTime;
			break;
        default:
            // we should never get here
            FLASHASSERT(0);
    }

exit_gracefully:
    return result;
}

char* SPlayer::GetPropertyStr(ScriptThread *thread, int propType)
{
	BOOL makeCopy = true;
    char *result = NULL;
    SObject *object = 0;

    if (thread == 0)
        goto exit_gracefully;
        
    object = thread->rootObject;

    switch (propType)
    {
		case kSpriteDropTarget:
			result = object->dropTarget;
			break;
		case kSpriteTarget:
			result = GetTargetPath(object);
			makeCopy = false;
			break;
		case kSpriteName:
			result = object->name;
			break;
		case kSpriteURL:
			result = object->character->player->url;
			break;
        default:
			{
				double value = GetProperty(thread, propType);
				result = ConvertDoubleToString(value);
				makeCopy = false;
			}
			break;
    }
	
	if (makeCopy) {
		result = CreateStr(result ? result : "");
	}

exit_gracefully:
    return result;
}

BOOL IsMachineNameURL(const char *url)
{
	// An URL that contains only a machine name looks like
	//    protocol://machinename

	// Search for :// sequence
	const char *s = url;
	while (*s) {
		if (s[0] == ':' && s[1] == '/' && s[2] == '/')  {
			// Found it!
			// The remaining text must contain no slashes.
			s += 3;
			while (*s) {
				if (*s == '/') {
					return false;
				}
				s++;
			}
			return true;
		}
		s++;
	}
	return false;
}

//
// Multibyte support
//
// WARNING: The multibyte support here will only work for Japanese on the Macintosh.
//

// #ifdef _MAC
// static int gDoubleByteSystem = -1;
// static char gDoubleByteParseTable[256];
// #elif defined(_WINDOWS)
// static UINT gACPCache = 0;
// static BOOL gIsMultiByte = false;
// #endif

// PlayerIsMultiByte returns TRUE if the system is multibyte
BOOL PlayerIsMultiByte()
{
// #ifdef _MAC
// 	if (gDoubleByteSystem == -1) {
// 		gDoubleByteSystem = (::GetEnvirons(smDoubleByte) != 0);
// 		if (gDoubleByteSystem) {
// 			// Initialize the parse table
// 			gDoubleByteSystem = FillParseTable(gDoubleByteParseTable, smSystemScript);
// 		}
// 	}
// 	return gDoubleByteSystem;
// #elif defined(_WINDOWS)
// 	UINT acp = GetACP();
// 	if (acp != gACPCache) {
// 		CPINFO cpinfo;
// 		GetCPInfo(acp, &cpinfo);
// 		gACPCache = acp;
// 		gIsMultiByte = cpinfo.MaxCharSize > 0;
// 	}
// 	return gIsMultiByte;
// #else
	// Multibyte support only available for Windows and Mac
	return FALSE;
// #endif
}

// PlayerIsLeadByte returns TRUE if ch is a lead byte
BOOL PlayerIsLeadByte(U8 ch)
{
// #ifdef _MAC
// 	if (!PlayerIsMultiByte()) {
// 		return false;
// 	}
// 	return gDoubleByteParseTable[ch] != 0;
// #elif defined(_WINDOWS)
// 	if (!PlayerIsMultiByte()) {
// 		return FALSE;
// 	}
// 	return IsDBCSLeadByte(ch);
// #else
	return FALSE;
// #endif
}

//
// String Length
//

int PlayerStringLength(char *string, BOOL doMultiByte)
{
	if (!doMultiByte || !PlayerIsMultiByte()) {
		return strlen(string);
	} else {
		int count = 0;
		while (*string) {
			count++;
			if (PlayerIsLeadByte(*string)) {
				string++;
				if (!*string) { break; }
			}
			string++;
		}
		return count;
	}
}

//
// Helper function for StringExtract
// This function is for multibyte use only!
// Returns pointer to n'th character in a multibyte string
// n is assumed to be 0 <= n <= StringLength(string)
//

char *PlayerStringIndex(char *string, int index)
{
	while (index--) {
		if (PlayerIsLeadByte(*string)) {
			string++;
			if (!*string) { break; }
		}
		string++;
	}
	return string;
}

//
// String Extract
//

char *PlayerStringExtract(char *string, int index, int count, BOOL doMultiByte)
{
	int len = PlayerStringLength(string, doMultiByte);
	if (index < 0) {
		index = 0;
	} else if (index > len) {
		index = len;
	}
	if (count < 0 || count > len-index) {
		count = len-index;
	}
	if (!doMultiByte || !PlayerIsMultiByte()) {
		return CreateStr(string+index, count);
	} else {
		char *start = PlayerStringIndex(string, index);
		char *end   = PlayerStringIndex(start, count);
		return CreateStr(start, end-start);
	}
}

void SPlayer::DoActions()
{
	// If we're doing a partial first frame, don't execute actions yet.
	if (!loaded) {
		return;
	}

	// If we've overflowed, clear the action stack and do nothing
	if (actionOverflow) {
		display.actionList.actionBase = display.actionList.actionTop = 0;
		return;
	}

	// Always let the outermost loop process the actions
	if ( actionDepth > 0 )
		return;

	actionDepth++;

	// Build a list of GetURLs so we can defer loading to avoid reentrancy problems
	enum { maxURLs = 32 };
	int urlCount = 0;
	char* actionURLs[maxURLs];
	char* actionTargets[maxURLs];
	char* actionPostData[maxURLs];
	int   actionVars[maxURLs];

	display.actionCallStackTop = 1;
	display.actionCallStack[0].actionList = &display.actionList;

	BOOL enteringCall = false, exitingCall = false;
	int actionCount = 0;

	while (display.actionCallStackTop) {

		enteringCall = false;

		ActionContext* context = &display.actionCallStack[display.actionCallStackTop-1];
		ActionList *actionList = context->actionList;

	while ( actionList->actionBase < actionList->actionTop ) {
		// Do the action
		int i = actionList->actionBase & actionList->actionMask;
		if ( actionList->actionList[i] ) {

			SParser parser;
			parser.Attach(actionList->actionList[i], 0);

			if (!exitingCall) {
				context->useTarget = false;
				context->targetThread = 0;
			} else {
				exitingCall = false;
			}

			int skipCount = 0;
			for (;;) {
				// Handle the action
				int actionCode = parser.GetByte();
				if ( !actionCode )
					break;

				// Get the source of the action - set this everytime since the thread could be deleted
				ScriptThread* baseThread = actionList->actionThread[i];

				int len = 0;
				if ( actionCode & sactionHasLength ) 
					len = parser.GetWord();
				S32 pos = parser.pos + len;
				if ( skipCount > 0 ) {
					// Skip an action because of a wait for frame
					skipCount--;
				} else {
					ScriptThread* thread = context->useTarget ? context->targetThread : baseThread;
					if ( thread ) {
						// Handle actions that use a target
						switch ( actionCode ) {
							// Handle timeline actions
							case sactionGotoFrame: {
 								thread->Seek(parser.GetWord()); 
							} break;

							case sactionGotoLabel: {
								int frameNum = thread->FindLabel((char*)(parser.script+parser.pos));
								if ( frameNum >= 0 ) 
									thread->Seek(frameNum);
							} break;
							case sactionNextFrame: {
								thread->Seek(thread->GetFrame()+1);
							} break;
							case sactionPrevFrame: {
									thread->Seek(thread->GetFrame()-1);
							} break;
							case sactionPlay: {
								thread->Play();
							} break;
							case sactionStop: {
								thread->StopPlay();
							} break;

							case sactionWaitForFrame: {
								// Skip n actions if the frame is not yet loaded
								int frame = parser.GetWord();
								if ( !thread->player->FrameComplete(frame) )
									skipCount = parser.GetByte();
							} break;
						}
					}

					// Handle actions that do not need a target
					switch ( actionCode ) {
						case sactionGetURL: {// Get an URL
							if ( urlCount < maxURLs ) {
								actionURLs[urlCount] = parser.GetString();
								actionTargets[urlCount] = parser.GetString();
								actionPostData[urlCount] = 0;
								actionVars[urlCount] = 0;
								urlCount++;
							}
						} break;

						case sactionSetTarget: {
							char* targetName = (char*)parser.script+parser.pos;
							if ( *targetName ) {
								// Set a new target
								context->useTarget = true;

								if ( baseThread ) {
									// Check to set a target
									context->targetThread = FindTarget(baseThread->rootObject, targetName);

									#ifdef _CELANIMATOR
									// Generate an error message
									if ( !context->targetThread ) 
										ShowTargetError(this, baseThread->rootObject, targetName);
									#endif
								} else {
									// Setting a target after the base object has been removed is an error
									context->targetThread = 0;

									#ifdef _CELANIMATOR
									// Generate an error where the base object is missing
									ShowTargetError(this, 0, targetName);
									#endif
								}
							} else {
								// Clear the set target
								context->useTarget = false;
								context->targetThread = 0;
							}
						} break;

						// Global actions
						#ifdef SOUND
						case sactionStopSounds: {
							theSoundMix.RemoveAll();
						} break;
						#endif

						case sactionToggleQuality: {
							highQuality = !highQuality;
							autoQuality = false;
							SetCamera(updateLazy);
						} break;

						// Flash 4 Actions
#ifdef ACTIONSCRIPT
						case sactionAdd:
							PushNum(PopNum() + PopNum());
							break;

						case sactionSubtract:
							{
								double valueA = PopNum();
								double valueB = PopNum();
								PushNum(valueB - valueA);
							}
							break;

						case sactionMultiply:
							PushNum(PopNum() * PopNum());
							break;

						case sactionDivide:
							{
								double valueA = PopNum();
								double valueB = PopNum();
								if (valueA) {
									PushNum(valueB / valueA);
								} else {
									Push("#ERROR#");
								}
							}
							break;

						case sactionEquals:
							Push((PopNum() == PopNum()) ? "1" : "0");
							break;

						case sactionLess:
							{
								double valueA = PopNum();
								double valueB = PopNum();
								Push((valueB < valueA) ? "1" : "0");
							}
							break;

						case sactionAnd:
							{
								int valueA = (int)PopNum();
								int valueB = (int)PopNum();
								Push((valueA && valueB) ? "1" : "0");
							}
							break;

						case sactionOr:
							{
								int valueA = (int)PopNum();
								int valueB = (int)PopNum();
								Push((valueA || valueB) ? "1" : "0");
							}
							break;

						case sactionNot:
							Push(PopNum() ? "0" : "1");
							break;

						case sactionStringEquals:
							{
								char *A = Pop();
								char *B = Pop();
								Push(!strcmp(A, B) ? "1" : "0");
								FreeStr(A);
								FreeStr(B);
							}
							break;

						case sactionStringLess:
							{
								char *A = Pop();
								char *B = Pop();
								Push((strcmp(B, A) < 0) ? "1" : "0");
								FreeStr(A);
								FreeStr(B);
							}
							break;

						case sactionStringLength:
						case sactionMBStringLength:
							{
								BOOL doMultiByte = (actionCode == sactionMBStringLength);
								char *value = Pop();
								char *result = ConvertIntegerToString(PlayerStringLength(value, doMultiByte));
								Push(result);
								FreeStr(result);
								FreeStr(value);
							}
							break;

						case sactionStringAdd:
							{
								char *A = Pop();
								char *B = Pop();
								char *result = new char[strlen(A)+strlen(B)+1];
								strcpy(result, B);
								strcat(result, A);
								Push(result);
								delete [] result;
								FreeStr(A);
								FreeStr(B);
							}
							break;

						case sactionGetTime:
							PushNum((double) (GetTimeMSec() - mTimerOffset));
							break;

						case sactionRandomNumber:
							{
								double value = PopNum();
								PushNum((double) Random((S32) value));
							}
							break;

						case sactionStringExtract:
						case sactionMBStringExtract:
							{
								BOOL doMultiByte = (actionCode == sactionMBStringExtract);
								int count = (int)PopNum();
								int index = (int)PopNum()-1;
								char *string = Pop();
								char *result = PlayerStringExtract(string, index, count, doMultiByte);
								Push(result);
								FreeStr(result);
								FreeStr(string);
							}
							break;

						case sactionPush:
							{
								int type = parser.GetByte();
								if (type == 0) {
									// String literal
									Push(parser.GetStringP());
								} else if (type == 1) {
									// Floating-point literal
									union {
										U32 dw;
										float f;
									} u;
									u.dw = parser.GetDWord();
									PushNum(u.f);
								}
							}
							break;

						case sactionPop:
							{
								char *result = Pop();
								FreeStr(result);
							}
							break;

						case sactionToInteger:
							{
								double value = PopNum();
								PushNum((float) ((int) value));
							}
							break;

						case sactionJump:
							{
								short offset = (short) parser.GetWord();
								pos += offset;
							}
							break;

						case sactionIf:
							{
								double value = PopNum();
								short offset = (short) parser.GetWord();
								if (value) {
									pos += offset;
								}
							}
							break;

						case sactionWaitForFrame2:
							{
								char *str = Pop();
								int frameNum;
								ScriptThread *targetThread = ResolveFrameNum(str, thread, &frameNum);
								if (targetThread) {
									// Skip n actions if the frame is not yet loaded
									if (!thread->player->FrameComplete(frameNum)) {
										skipCount = parser.GetByte();
									}
								}
								FreeStr(str);
							}
							break;

						case sactionGotoFrame2:
							{
								int gotoPlay = parser.GetByte();

								char *str = Pop();
								int frameNum;
								ScriptThread* targetThread = ResolveFrameNum(str, thread, &frameNum);
								if (targetThread) {
									targetThread->Seek(frameNum);
									if (gotoPlay) {
										targetThread->Play();
									}
								}
								FreeStr(str);
							}
							break;

						case sactionCall:
							{
								char *name = Pop();
								if (display.actionCallStackTop < MaxActionCallStack) {
									int frameNum;
									ScriptThread* targetThread = ResolveFrameNum(name, thread, &frameNum);
									if (targetThread) {									
										// Save position in old action list
										i = actionList->actionBase & actionList->actionMask;
										actionList->actionList[i] += pos;

										SParser parser;
										parser.Attach(targetThread->script,
													  targetThread->startPos);
										int code;
										int curFrame = 0;
										int len = targetThread->len;
										// Seek to the frame
										if (frameNum) {
											while ((code = parser.GetTag(len)) != -1) {
												parser.pos = parser.tagEnd;
												if (code == stagShowFrame) {
													if (++curFrame == frameNum) {
														break;
													}
												}
											}
										}
										ActionList* newActions = new ActionList;
										// Scan for DoAction tags
										while ((code = parser.GetTag(len)) != -1) {
											if (code == stagDoAction) {
												newActions->PushAction(parser.script+parser.pos, targetThread);
											}
											if (code == stagShowFrame) {
												break;
											}
											parser.pos = parser.tagEnd;
										}
										// Push this new action list onto context stack
										display.actionCallStack[display.actionCallStackTop++].actionList = newActions;
										enteringCall = true;
									}
								}
#ifdef _CELANIMATOR
								else {
									CallLimitError();
								}
#endif
								FreeStr(name);
							}

							break;

						case sactionGetVariable:
							{
								char *name = Pop();
								char *value = GetVariable(thread, name);
								if (value) {
									Push(value);
									FreeStr(value);
								} else {
									Push("");
								}
								FreeStr(name);
							}
							break;

						case sactionSetVariable:
							{
								char *value = Pop();
								char *name = Pop();
								SetVariable(thread, name, value, true);
								FreeStr(name);
								FreeStr(value);
							}
							break;

						case sactionSetTarget2: {
							char* targetName = Pop();
							if ( *targetName ) {
								// Set a new target
								context->useTarget = true;

								if ( baseThread ) {
									// Check to set a target
									context->targetThread = FindTarget(baseThread->rootObject, targetName);

									#ifdef _CELANIMATOR
									// Generate an error message
									if ( !context->targetThread ) 
										ShowTargetError(this, baseThread->rootObject, targetName);
									#endif
								} else {
									// Setting a target after the base object has been removed is an error
									context->targetThread = 0;

									#ifdef _CELANIMATOR
									// Generate an error where the base object is missing
									ShowTargetError(this, 0, targetName);
									#endif
								}
							} else {
								// Clear the set target
								context->useTarget = false;
								context->targetThread = 0;
							}
							FreeStr(targetName);
						} break;

						case sactionGetURL2:
							{
								U8 method = parser.GetByte();

								actionVars[urlCount] = method;

								if (method & kHttpLoadVariables)
									method &= ~kHttpLoadVariables;

								char *target = actionTargets[urlCount] = Pop();
								char *baseURL = Pop();

                                if (method & kHttpLoadTarget) 
                                {
                                    //+
                                    // we are loading into a target
                                    //-

									method &= ~kHttpLoadTarget;

                                    // see if target resolves to a _levelXX
                                    char            *targetTmp = target;
                                    BOOL            loadTarget = true;
    						        ScriptThread*   targetThread = thread;							

                                    if (target) 
                                    {
                                        // find the target we want
									    targetThread = FindTarget(thread ? thread->rootObject : player.rootObject, target);
								    }

                                    // see if target resolves to a _levelXX
                                    if (LayerNum(targetTmp) >= 0) 
                                    {
                                        // skip past the _levelxx or _levelxx/ string
		                                for (; *targetTmp != 0; targetTmp += 1)
                                        {
                                            if (*targetTmp == '/')
                                            {
                                                targetTmp += 1;
                                                break;
                                            }
                                        }

					                    if (*targetTmp == 0)
                                        {
                                            // we are dealing _level not target
                							actionVars[urlCount] &= ~kHttpLoadTarget;
                                            loadTarget = false;
                                        }
                                    }
                                    else if (target && target[0] == '/' && target[1] == 0)
                                    {
                                        // we are dealing _level not target
                						actionVars[urlCount] &= ~kHttpLoadTarget;
                                        loadTarget = false;
                                        
                                        if (targetThread && targetThread->layerDepth > 0)
                                        {
                                            // will resolve to "_levelXX/" or "/"
                                            char *fullPath = GetTargetPath(targetThread->rootObject);

                                            if (fullPath)
                                            {
                                                FreeStr(actionTargets[urlCount]);
                                                actionTargets[urlCount] = fullPath;
                                            }
                                        }
                                        else
                                        {
                                            // special case: root of main movie
                                            FreeStr(actionTargets[urlCount]);
                                            actionTargets[urlCount] = CreateStr("_level0");
                                        }
                                    }

                                    if (loadTarget)
                                    {
                                        // we are dealing with target not _levelXX
                                        if (targetThread)
                                        {
                                            // find the full path of the target
                                            char *fullPath = GetTargetPath(targetThread->rootObject);

                                            if (fullPath)
                                            {
                                                if (fullPath[0] == '/' && fullPath[1] == 0)
                                                {
                                                    // special case: root of main movie
                							        actionVars[urlCount] &= ~kHttpLoadTarget;
                                                    FreeStr(fullPath);
                                                    fullPath = CreateStr("_level0");
                                                } 
                                                else if (LayerNum(fullPath) >= 0)
                                                {
                                                    // skip past the _levelxx or _levelxx/ string
		                                            for (targetTmp = fullPath; *targetTmp != 0; targetTmp += 1)
                                                    {
                                                        if (*targetTmp == '/')
                                                        {
                                                            targetTmp += 1;
                                                            break;
                                                        }
                                                    }

					                                if (*targetTmp == 0)
                							            actionVars[urlCount] &= ~kHttpLoadTarget;
                                                }

                                                FreeStr(actionTargets[urlCount]);
                                                actionTargets[urlCount] = fullPath;
                                            }
                                        }
                                        else
                                        {
                                            // could not find target don't do get url
								            FreeStr(baseURL);
                                            break;
                                        }
                                    }
                                }
                                
                                if (method == kHttpSendUseGet) {
									 
                                        // Send the variables with a GET
										URLBuilder ub;
										ub.AppendString(baseURL);

                                        // thread could have been deleted
										if ( thread && thread->firstVariable ) {
											if (IsMachineNameURL(baseURL)) {
												ub.AppendChar('/');
											}
											ub.AppendChar('?');
											ub.EncodeVariables(thread->firstVariable);
										}

										actionURLs[urlCount] = CreateStr(ub.buf);

                                        actionPostData[urlCount] = 0;
								} else if (method == kHttpSendUsePost) {

                                    // Send the variables with a POST
									actionURLs[urlCount] = CreateStr(baseURL);

                                    // thread could have been deleted
                                    if (thread) {
										URLBuilder ub;
										ub.EncodeVariables(thread->firstVariable);

										actionPostData[urlCount] = CreateStr(ub.buf);
                                    }
    								else	
                                        actionPostData[urlCount] = 0;
								} else {
									/* normal fetch */
									actionURLs[urlCount] = CreateStr(baseURL);
									actionPostData[urlCount] = 0;
								}

								urlCount++;
								FreeStr(baseURL);
							}
							break;

						case sactionGetProperty:
							{
								int index = (int) PopNum();
								char *target = Pop();
								ScriptThread* targetThread = thread;
								
                                if (target[0]) {
									targetThread = FindTarget(thread ? thread->rootObject : player.rootObject, target);
								}
								if (targetThread) {
									char *value = GetPropertyStr(targetThread, index);
									Push(value);
									FreeStr(value);
								} else {
									Push("");
								}

								FreeStr(target);
							}
							break;

#ifdef DRAGBUTTONS
						case sactionStartDrag:
							{
								char *target = Pop();
								int center = (int)PopNum();
								int constrainType = (int)PopNum();
								
								int x1, y1, x2, y2;
								if (constrainType == sdragRectConstraint) {
									// Pop off rectangle constraint
									y2 = (int)PopNum();
									x2 = (int)PopNum();
									y1 = (int)PopNum();
									x1 = (int)PopNum();
								}

								ScriptThread* targetThread = thread;							
                                if (target[0]) {
									targetThread = FindTarget(thread ? thread->rootObject : player.rootObject, target);
								}
								if (targetThread) {
									dragObject = targetThread->rootObject;
									dragStarted = TRUE;
                                    dragObject->puppet = true;
									dragPoint = mousePt;
									dragObject->dragCenter = (center == sdragFromCenter);
									if (constrainType == sdragRectConstraint) {
										RectSet(x1*20, y1*20, x2*20, y2*20, &dragObject->dragConstraint);
									} else {
										RectSetEmpty(&dragObject->dragConstraint);
									}
								}
								FreeStr(target);
							}
							break;

						case sactionEndDrag:
							if (dragObject) {
								UpdateDropTarget();
								dragObject = NULL;
								dragStarted = FALSE;
							}
							break;
#endif

						case sactionSetProperty:
							{
								char *value = Pop();
								int index = (int) PopNum();
								char *target = Pop();
								ScriptThread* targetThread = thread;
								
                                if (target[0]) {
									targetThread = FindTarget(thread ? thread->rootObject : player.rootObject, target);
								}
								SetPropertyStr(targetThread, index, value);

								FreeStr(target);
								FreeStr(value);
							}
							break;

						case sactionTrace:
							{
								char			*value = Pop();
#ifdef _CELANIMATOR
								TraceString(value);
#endif
								FreeStr(value);
							}
							break;

                        case sactionCloneSprite:
                            {
                                int             depth = (int) PopNum();
                                char            *name = Pop();
								char            *target = Pop();
								ScriptThread    *targetThread = thread;

                                // neg levels are not allowed
                                if (depth < 0) {
                                    break;
								}
                                if (target[0]) {
									targetThread = FindTarget(thread ? thread->rootObject : player.rootObject, target);
								}
                                CloneSprite(targetThread, name, depth);

								FreeStr(target);
								FreeStr(name);
                            }
                            break;

                        case sactionRemoveSprite:
                            {
								char            *target = Pop();
								ScriptThread    *targetThread = thread;

                                if (target[0]) {
									targetThread = FindTarget(thread ? thread->rootObject : player.rootObject, target);
								}
                                
                                if (targetThread && targetThread->rootObject->depth >= 0x4000)
                                    // remove only duplicate sprites
                                    RemoveSprite(targetThread, false);

								FreeStr(target);
                            }
                            break;

						case sactionAsciiToChar:
						case sactionMBAsciiToChar:
							{
								BOOL doMultiByte = (actionCode == sactionMBAsciiToChar);
								int asciiCode = (int)PopNum();
								char str[3];
								if (doMultiByte && asciiCode > 0xff) {
									str[0] = (char) (asciiCode>>8);
									str[1] = (char) (asciiCode&0xff);
									str[2] = 0;
								} else {
									str[0] = (char)asciiCode;
									str[1] = 0;
								}
								Push(str);
							}
							break;

						case sactionCharToAscii:
						case sactionMBCharToAscii:
							{
								BOOL doMultiByte = (actionCode == sactionMBCharToAscii);
								char *str = Pop();
								U16 ch;
								if (doMultiByte && PlayerIsLeadByte(*str)) {
									ch = ((U8)str[0]<<8) | (U8)str[1];
								} else {
									ch = (U8)*str;
								}
								char *result = ConvertIntegerToString((int)ch);
								Push(result);
								FreeStr(result);
								FreeStr(str);
							}
							break;
#endif
					}
				}

				actionCount++;

				if (actionCount > MaxExecActions) {
					// So many actions have been executed, something must be
					// wrong... stop all actions processing.
#ifdef _CELANIMATOR
					ExecutionLimitError();
#endif
					actionOverflow = true;
				}

				if (enteringCall || actionOverflow) {
					break;
				}

				parser.pos = pos;
			}
		}

		if (enteringCall || actionOverflow) {
			break;
		}

		actionList->actionBase++;
	}

	if (!enteringCall || actionOverflow) {
		// Pop context
		// Delete action list for old stack context
		if (--display.actionCallStackTop) {
			exitingCall = true;
			delete display.actionCallStack[display.actionCallStackTop].actionList;
		}
	}

	}

	actionDepth--;

	// Load the URLs now
	for ( int i = 0; i < urlCount; i++ ) {
		if ( actionURLs[i] && actionTargets[i] ) {
			GetURL(actionURLs[i],
				   actionTargets[i],
				   actionPostData[i],
				   actionVars[i]);
		}

		FreeStr(actionURLs[i]);
		FreeStr(actionTargets[i]);
		FreeStr(actionPostData[i]);
	}
}

void SPlayer::DoPlay(BOOL wait)
{
	// If the first frame isn't loaded yet, we must not perform DoPlay, because movie clips
	// may start animating before we're ready to execute actions.
	if (!loaded) {
		return;
	}

    // Note that DoActions() can cause this to get called again (when the browser is loading an URL) which is not properly supported
	if ( actionDepth ) return;

	S32 time = GetTimeMSec();
	int nFramesToAdvance = 1;

	#ifdef SOUND
	ScriptThread* syncThread = 0;
	{// Look for a thread with streaming sound...
		for ( ScriptThread* t = display.threads; t; t = t->next ) {
			CSoundChannel* c = t->sndChannel;
			if ( c && c->IsPlaying() && !c->streamStalled ) {
				// Use the streaming sound as the clock...
				nFramesToAdvance = c->ElapsedFrames() - t->GetFrame();
				syncThread = t;
				if ( nFramesToAdvance <= 0 ) 
					return;
				goto Advance;
			}
		}
	}
	#endif

	// Is it time to show the next frame yet???
	if ( wait && time < nextFrameTime )
		return;

//  Advance:
	// Always try to maintain the frame delay and slow down if we can't keep up
	nextFrameTime = time + player.frameDelay;

	#ifdef DEBUG
	if ( nFramesToAdvance > 1 )
		FLASHOUTPUT("Skipped %i frames\n", nFramesToAdvance);
	#endif

	while ( nFramesToAdvance-- ) {
		// Process the timeline for all threads
		for ( ScriptThread* t = display.threads; t; t = t->next ) {
			#ifdef SOUND
			if ( t->sndChannel && t != syncThread && t->sndChannel->IsPlaying() ) {
				if ( t->sndChannel->ElapsedFrames() >= t->GetFrame() || t->sndAtEnd ) {
					// Don't get ahead of a sound channel
					t->DoFrame();
				}
			} else 
			#endif
			{
				// Advance a single frame
				t->DoFrame();
			}
		}

		// Handle the action list
		DoActions();
	}
// NOTE: commented out for windows
// #if defined(DRAGBUTTONS) && defined(_WINDOWS) && !defined(GENERATOR)
// 	// When a drag is started on Windows, the object doesn't get moved to
// 	// the current mouse position until the first WM_MOUSEMOVE.
// 	// This fixes that.
// 	if (dragStarted) {
// 		UpdateDragObject(&mousePt);
// 		dragStarted = FALSE;
// 	}
// #endif

  #ifdef EDITTEXT
	if ( !display.iBeam ) {
  #endif
		// Update the buttons in case a button has moved under the mouse
		if ( !scrolling && mouseState >= 0 )
			DoButton(&mousePt, ( mouseState != 0), false);
  #ifdef EDITTEXT
	}
  #endif

	UpdateScreen();

	// See if the quality setting is appropriate for this movie
	if ( autoQuality ) {
		S32 elapsed = GetTimeMSec() - time;
		if ( elapsed > player.frameDelay || nFramesToAdvance > 1 )
			nTooSlow++;
		else if ( elapsed < player.frameDelay/4 )
			nTooFast++;
		nTotal++;
		if ( nTotal > 4 && lastQualitySwitch > 10 ) {
			if ( highQuality ) {
				if ( nTooSlow*3 > nTotal ) {
					highQuality = false;
					lastQualitySwitch = 0;
					//autoQuality = false;	// never switch more than once
					SetCamera(updateLazy);
				}
			} else {
				if ( nTooFast*3 > nTotal ) {
					highQuality = true;
					lastQualitySwitch = 0;
					//autoQuality = false;
					SetCamera(updateLazy);
				}
			}
			//if ( nTotal > 10 )
			//	// If we have not switched yet, don't switch
			//	autoQuality = false;
		}
		lastQualitySwitch++;
	}
}


void SPlayer::LoadMovie( char* path )
{
	ClearScript();			       //SPlayer

	// Set this as the first movie again - lee@middlesoft
	// Should this be in the ClearScript call?
	firstStream = true;

	GetURL( path );
}

void SPlayer::GetMenuState( MenuStatus* menuStatus )
{
	BOOL hasScript = player.ParseHeader();

	menuStatus->highQChecked	= highQuality;

	menuStatus->zoomOut			= hasScript && !RectIsEmpty(&zoomRect);
	menuStatus->zoomIn			= hasScript;

	menuStatus->view100percent	= hasScript;
	menuStatus->view100Check    = ( scaleMode == DisplayList::noScale );
 
	menuStatus->viewAllCheck	= ( hasScript && RectIsEmpty(&zoomRect) && scaleMode != DisplayList::noScale );
	menuStatus->viewAll			= hasScript;

	menuStatus->play			= hasScript && player.numFrames > 1;
	menuStatus->playCheck		= player.playing;

	menuStatus->loop			= hasScript && player.numFrames > 1;
	menuStatus->loopCheck		= hasScript && player.loop;

	BOOL goBackEnable = hasScript && player.GetFrame() > 0;
	menuStatus->rewind			= goBackEnable;
	menuStatus->back			= goBackEnable;
	menuStatus->forward			= hasScript && !player.AtEnd();
}

#ifdef EDITTEXT
void SPlayer::GetEditMenuState( EditMenuStatus* editMenuStatus )
{
	int flags = 0;
	memset( editMenuStatus, 0, sizeof( EditMenuStatus ) );

	if (focus) 
	{
		focus->editText->GetEnabledCommands(flags);
	}

	editMenuStatus->cut			= flags & EditText::kCutEnabled			? true : false;
	editMenuStatus->copy		= flags & EditText::kCopyEnabled		? true : false;
	editMenuStatus->paste		= flags & EditText::kPasteEnabled		? true : false;
	editMenuStatus->clear		= flags & EditText::kClearEnabled		? true : false;
	editMenuStatus->selectAll	= flags & EditText::kSelectAllEnabled	? true : false;
}
#endif

void SPlayer::SetMenuState()
{
	MenuStatus menuStatus;
	GetMenuState( &menuStatus );
	EnableMenus( menuStatus );
}

int SPlayer::GetPopupMenuState()
{
	int id = MENU_NONE;

	#ifdef EDITTEXT
	if (display.iBeam) 
	{
		id = MENU_EDIT;
		FLASHOUTPUT( "Menu: Edit\n" );
	} 
	else
	#endif
	if ( !player.ParseHeader() ) 
	{
		id = MENU_NO_MOVIE;
		FLASHOUTPUT( "Menu: No Movie\n" );
	}
	else if ( !showMenu ) 
	{
		id = MENU_NONE;
		FLASHOUTPUT( "Menu: None\n" );
	}
	else if (player.numFrames > 1) 
	{
		id = MENU_NORMAL;
		FLASHOUTPUT( "Menu: Normal\n" );
	}
	else
	{
		id = MENU_STATIC;
		FLASHOUTPUT( "Menu: Static\n" );
	}	
	return id;
}


void SPlayer::ControlOpen( char* filename )
{
	if ( filename && *filename )
	{
		display.Reset();
		LoadMovie( filename );

		if ( player.ParseHeader() )
		{
			int width = (int)RectWidth( &player.frame ) / 20;
			int height = (int)RectHeight( &player.frame ) / 20;
			AdjustWindow( width, height );
		}
	}
	SetMenuState();
}

void SPlayer::ControlOpen(Aresobj *o)
{
	ClearScript();
	firstStream = true;
	StreamGetResourceNotify(o, NULL);
	if(player.ParseHeader())
	{
		int width = (int)RectWidth( &player.frame ) / 20;
		int height = (int)RectHeight( &player.frame ) / 20;
		AdjustWindow( width, height );
	}
}

void SPlayer::ControlClose()
{
	ClearScript();	
	SetMenuState();
}

void SPlayer::ControlPlay()
{
	if ( !player.playing ) 
		Play();
	else
		StopPlay();
	SetMenuState();
}

void SPlayer::ControlRewind()
{
	Rewind();
	SetMenuState();
}

void SPlayer::ControlForward()
{
	Forward();
	SetMenuState();
}
void SPlayer::ControlBack()
{
	Back();
	SetMenuState();
}

void SPlayer::ControlLoop()
{
	player.loop = !player.loop;
	SetMenuState();
}

void SPlayer::Control100()
{
	Zoom100();
	SetMenuState();
}

void SPlayer::ControlViewAll()
{
	Home();
	SetMenuState();
}

void SPlayer::ControlZoomIn()
{
	ZoomF(fixed_1/2);
	SetMenuState();
}

void SPlayer::ControlZoomContext( int x, int y )
{
	SPOINT pt;
	
	pt.x = x;
	pt.y = y;	

	ZoomF( fixed_1/2, &pt );
	SetMenuState();
}

void SPlayer::ControlZoomOut()
{
	ZoomF(fixed_1*2);
	SetMenuState();
}

void SPlayer::ControlHighQuality()
{
	highQuality = !highQuality;
	autoQuality = false;
	SetCamera(updateLazy);
	SetMenuState();
}

void SPlayer::Run()
{
	const int CursorBlinkTime = 400;

	int delay = player.frameDelay / 10;
    if ( delay < 0 )
        delay = 1;

	if ( !running && StartTimer( delay, CursorBlinkTime ) )
	{
		// Start the timer
		running = true;
		nextFrameTime = GetTimeMSec();
		//syncToClock = false;
	}
}

void SPlayer::Suspend()
{
	if ( running )
	{
		// Nothing should be running
		if ( !destructing )
		{
			StopTimer();		// once again, this is a native call - we can't call this in
								// a destructor.
		}
		running = false;

		#ifdef SOUND
		for ( ScriptThread* t = display.threads; t; t = t->next )
		{
			t->StopStream();
			theSoundMix.RemoveTagged((U32)t);	// stop all the sounds when we go away
		}
		#endif
	}
}


//
// Scripting API
//

void SPlayer::Pan(long x, long y, int mode)
{
	if ( RectIsEmpty(&zoomRect) ) return;

	// Adjust the scroll distance based on the mode
	switch ( mode ) {
		case panPixels: // absolute pixels
			break;
		case panPercent:{// percent of the window size
			SRECT viewRect;
			ClientRect(&viewRect);
			x = (RectWidth(&viewRect)*x)/100;
			y = (RectHeight(&viewRect)*y)/100;
		} break;
	}

	{// scroll x number of pixels
		MATRIX m, invMat;
		if ( display.antialias ) {
			MatrixScale(fixed_1/4, fixed_1/4, &m);
			MatrixConcat(&display.camera.mat, &m ,&m);
		} else {
			m = display.camera.mat;
		}
		MatrixInvert(&m, &invMat);

		SRECT viewRect, docRect;
		ClientRect(&viewRect);
		MatrixTransformRect(&m, &player.frame, &docRect);

		// Calculate the scroll limits
		SRECT limit;
		limit.xmin = docRect.xmin - viewRect.xmin;
		if ( limit.xmin > 0 ) limit.xmin = 0;
		limit.xmax = docRect.xmax - viewRect.xmax;
		if ( limit.xmax < 0 ) limit.xmax = 0;
		limit.ymin = docRect.ymin - viewRect.ymin;
		if ( limit.ymin > 0 ) limit.ymin = 0;
		limit.ymax = docRect.ymax - viewRect.ymax;
		if ( limit.ymax < 0 ) limit.ymax = 0;

		SPOINT delta;
		delta.x = x;
		delta.y = y;
		if ( delta.x < limit.xmin )
			delta.x = limit.xmin;
		else if ( delta.x > limit.xmax ) 
			delta.x = limit.xmax;

		if ( delta.y < limit.ymin )
			delta.y = limit.ymin;
		else if ( delta.y > limit.ymax ) 
			delta.y = limit.ymax;

		// Do the scroll
		RectOffset(delta.x, delta.y, &viewRect);
		MatrixTransformRect(&invMat, &viewRect, &zoomRect);
		SetCamera(updateLazy);
	}
}

void SPlayer::Zoom(int factor)
{
	if ( factor == 0 ) {
		Home();
	} else {
		ZoomF(FC(factor)/100);
	}
}

void SPlayer::SetZoomRect(long left, long top, long right, long bottom)
{
	if ( left >= right || top >= bottom ) {
		Home();
	} else {
		zoomRect.xmin = left;
		zoomRect.ymin = top;
		zoomRect.xmax = right;
		zoomRect.ymax = bottom;
		SetCamera(SPlayer::updateLazy);
	}
}

//////////////////////////////////////////////////////////////////////////
// Internet download routines for the standalone Flash Player

// void SPlayer::LoadFile( URLWrapper* url, 
// 						int layer, 
// 						char *dstSprite, 
// 						char *postData, 
// 						BOOL loadVars 
// 					  )
// {
//     ScriptThread    *thread = 0;
//     
//     // see if we are loading a sprite instead
//     // of a movie layer. If so find the target
//     // thead that is going to get the new script
//     if (dstSprite)
//     {
// 		if (!FindTarget(player.rootObject, dstSprite)) {
// 			// if can't find the target thread
// 			// just return.
//             return;
// 		}
//     }
// 
// 	// Put the data into the script
// 	DownloadURL( url, postData, dstSprite, layer, loadVars );
// }
// 

// String Utils
//

int LayerNum(char* s)
{
	const char* layer = SkipPrefix(s, "_flash");  // this is obsolete but we include it for compatibility with a beta version
	if ( !layer ) 
		layer = SkipPrefix(s, "_level");	// allow this also since it is easier for users
	if ( layer ) 
		return ToInt(layer);
	else
		return -1;
}


//
// A Helper Object that combines variables into a form suitable for URLs or a POST operation
//

void URLBuilder::AppendChar(char ch)
{
	if ( len >= size-1 ) {
		// enlarge the buffer
		size += 256;
		char* x = new char[size];
		if ( !x ) return;
		if ( buf )
			memcpy(x, buf, len+1);
		FreeStr(buf);
		buf = x;
	}

	buf[len++] = ch;
	buf[len] = 0;	// keep the string null terminated
}

void URLBuilder::AppendInt(int i)
{
	if (!i) {
		AppendChar('0');
	} else {
		int n = 0;
		char buf[10];
		while (i > 0) {
			buf[n++] = (i % 10)+'0';
			i /= 10;
		}
		char* c = buf+n;
		while ( n-- ) {
			AppendChar(*--c);
		}
	}
}

int HexChar(int v)
{
	v &= 0xF;
	return v < 10 ? v + '0' : v + 'A' - 10;
}

void URLBuilder::EncodeString(char* ch)
{
	while ( true ) {
		char c = *ch;
		if ( !c ) break;
		if ( (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ) {
			AppendChar(c);		// leave alpha numeric characters alone
		} else if ( c == ' ' ) {
			AppendChar('+');	// convert a space to a "+"
		} else {
			// Encode all other characters
			AppendChar('%');
			AppendChar(HexChar(c>>4));
			AppendChar(HexChar(c));
		}

		ch++;
	}
}

void URLBuilder::AppendString(char* ch)
{
	while ( true ) {
		char c = *ch;
		if ( !c ) break;
		AppendChar(c);
		ch++;
	}
}

void URLBuilder::EncodeVariables(ScriptVariable* first)
{
	ScriptVariable* var = first;
	while ( var ) {
		if ( var->name && var->value ) {
			if ( var != first )
				AppendChar('&');
			EncodeString(var->name);
			AppendChar('=');
			EncodeString(var->value);
		}

		var = var->next;
	}
}


//
// Extract variable values from a URL
//

int ParseHexChar(char c)
{
	if ( '0' <= c && c <= '9' )
		return c-'0';
	if ( 'A' <= c && c <= 'F' )
		return c-'A'+10;
	if ( 'a' <= c && c <= 'f' )
		return c-'a'+10;
	return -1;
}

char* ExtractCharacter(char* ch, URLBuilder* ub)
{
	FLASHASSERT(ch && *ch);
	char c = *ch;
	if ( c == '%' ) {
		// Parse out a hex code
		ch++;	// skip the %

		int v = ParseHexChar(*ch++);	// get first digit
		if ( v < 0 ) return ch;
		int v2 = ParseHexChar(*ch++);	// get second digit
		if ( v2 < 0 ) return ch;

		ub->AppendChar((v << 4) | v2);

	} else if ( c == '+' ) {
		// Map a '+' to a ' '
		ub->AppendChar(' ');
		ch++;

	} else {
		// Handle raw characters
		ub->AppendChar(c);
		ch++;
	}
	return ch;
}

void DecodeQueryString(ScriptThread* thread, char *str)
{
	char *ch = str;

	// Iterate through the variable pairs...
	while ( true ) {
		URLBuilder name;
		URLBuilder value;

		// Pull off the name
		while ( true ) {
			char c = *ch;
			if ( !c ) break;
			if ( c == '=' ) {
				ch++;
				break;
			}
			if ( c == '&' ) {
				break;
			}
			ch = ExtractCharacter(ch, &name);
		}

		// Pull off the value
		while ( true ) {
			char c = *ch;
			if ( !c ) break;
			if ( c == '&' ) {
				ch++;
				break;
			}
			ch = ExtractCharacter(ch, &value);
		}

		if ( name.buf )
			thread->SetVariable(name.buf, value.buf ? value.buf : "");

		if ( *ch == 0 ) break;
	}
}

void ExtractVariables(ScriptThread* thread, char* url)
{
	char* ch = url;

	// Find the '?' delimiter
	while ( true ) {
		if ( !*ch ) return;	// there are no variables...
		if ( *ch++ == '?' ) break;
	}

	DecodeQueryString(thread, ch);
}


#ifdef EDITTEXT

void SPlayer::BlinkCursor()
{
	if (!focus) {
		return;
	}

	cursorBlink = !cursorBlink;

	focus->Modify();
}

BOOL SPlayer::TabKeyDown(int key, int modifiers)
{
	if (key == Keyboard::ID_KEY_TAB) {
		// If there is no current button, we must reset the
		// current button to the last button visited so that
		// navigation can continue where it left off.
		if (!currentButton.button) {
			currentButton = &m_lastButton;
		}
		// Save current button
		m_lastButton = &currentButton;
		// Do the tab
		if (modifiers & Keyboard::ID_KEY_SHIFT) {
			ButtonTabPrevious();
		} else {
			ButtonTabNext();
		}
		// Notify the container that we handled the navigation
		// only if we found a valid button to navigate to.
		return currentButton.button != 0;
	}

	// The arrow keys and Enter are only handled if there
	// is already a current button AND if there is no edit text
	// focused.
	if (!currentButton.button || focus != 0 || RectIsEmpty(&display.tabFocusRect)) {
		return FALSE;
	}

	// Save current button
	m_lastButton = &currentButton;

	switch (key) {
    case Keyboard::ID_KEY_RIGHT:
        ButtonGoDirection(RIGHT);
        break;
    case Keyboard::ID_KEY_LEFT:
        ButtonGoDirection(LEFT);
        break;
    case Keyboard::ID_KEY_UP:
        ButtonGoDirection(UP);
        break;
    case Keyboard::ID_KEY_DOWN:
        ButtonGoDirection(DOWN);
        break;
    case Keyboard::ID_KEY_ENTER:
	case ' ':
        // process an action here if required
		ButtonEnter();

		// Check if the button has changed/moved
		SButton* button = &currentButton;
		SButton  newButton;
		ButtonFind(button, &newButton);

		if ( button->button != newButton.button ) {
			button = &newButton;
			if (!newButton.button) {
				ButtonTabNext();
			}
		}
		// Return that we handled this
		return TRUE;
	}

	// Notify the container that we handled the navigation
	// only if we found a valid button to navigate to.
	return currentButton.button != 0;
}

BOOL SPlayer::HandleKeyDown(int key, int modifiers)
{
	{
		ButtonLock();

		RemoveAllButtons();

		int iNumButtons = NumButtons(&display.root);

		// Check for any buttons that match this key
		for (int i=0; i<iNumButtons; i++) {
			SObject* obj = buttonArray[i];
			SCharacter* ch = obj->character;

			// Does this button have a keypress state?
			if ( ch->tagCode == stagDefineButton2 ) {
				SParser parser;
				parser.Attach(ch->data, 0);
				BOOL first = true;
				for (;;) {
					S32 link = parser.pos;
					int offset = parser.GetWord();
					if ( !first ) {
						int code = parser.GetWord();
						code = (code >> 9) & 0x7F;

						if (code != 0 && code == key) {
							ButtonUnlock();

							// Execute the actions for the key condition
							display.PushAction(parser.script+parser.pos, obj->thread);

							DoActions();
							UpdateScreen();

							return TRUE;
						}
					}
					if ( !offset ) break;
					parser.pos = link + offset;
					first = false;
				}
			}
		}

		ButtonUnlock();
	}

	if (TabKeyDown(key, modifiers)) {
		return TRUE;
	}

	if (focus) {
		ScriptPlayer* player = focus->character->player;

		EditText* editText = focus->editText;
		// BACKCAST
		editText->HandleKeyDown( key, modifiers, (NativePlayerWnd*)this );

		// Did the edit text's value actually change?
		char* varName  = editText->m_variable;
		if (varName[0]) {
			char* newValue = editText->GetBuffer();
			if (newValue) {
				// oldValue may legitimately be NULL if variable isn't set
				char* oldValue = GetVariable(focus->thread, varName);
				BOOL changed = FALSE;
				if (oldValue) {
					changed = (strcmp(oldValue, newValue) != 0);
					FreeStr(oldValue);
				} else {
					changed = (newValue[0] != 0);
				}
				if (changed) {
					// Yes, it did change.
					SetVariable(focus->thread, varName, newValue, FALSE);

					// Call the changed handler.
					const char *changedSuffix = "_changed";
					char *handlerName = new char[strlen(changedSuffix)+strlen(varName)+1];
					if (handlerName) {
						strcpy(handlerName, varName);
						strcat(handlerName, changedSuffix);

						int frameNum;
						ScriptThread* targetThread = ResolveFrameNum(handlerName, focus->thread, &frameNum);
						if (targetThread) {
							CallFrame(targetThread, frameNum);
						}
						delete [] handlerName;
					}
				}
				delete [] newValue;
			}
		}

		focus->Modify();
		return TRUE;
	}

	return FALSE;
}

void SPlayer::HandleKeyUp(int key, int modifiers)
{
}

BOOL SPlayer::IsSelecting()
{
	return focus && focus->editText->m_selecting;
}

#endif

#ifdef ACTIONSCRIPT

void SPlayer::Push(char *value)
{
	if (stackPos >= stackSize) {
		if (stackSize) {
			stackSize *= 2;
		} else {
			stackSize = 128;
		}
		char **newStack = new char* [stackSize];
		memcpy(newStack, stack, sizeof(char*) * stackPos);
		delete[] stack;
		stack = newStack;
	}
	stack[stackPos++] = CreateStr(value);
}

char *SPlayer::Pop()
{
	if (stackPos == 0) {
		// Stack underflow
		return CreateStr("");
	}
	char *value = stack[--stackPos];
	return value;
}

double SPlayer::PopNum()
{
	char *str = Pop();
	double value;
	if (!ConvertStringToDouble(str, &value)) {
		value = 0;
	}
	FreeStr(str);
	return value;
}

void SPlayer::PushNum(double value)
{
	char *s = ConvertDoubleToString(value);
	Push(s);
	FreeStr(s);
}

BOOL SPlayer::CallFrame(ScriptThread *thread, int frameNum)
{
	if (frameNum < 0) {
		return FALSE;
	}

	if (thread) {
		// Seek to the frame
		SParser parser;
		parser.Attach(thread->script, thread->startPos);
		int code, framesLeft = frameNum, len = thread->len;
		if (framesLeft) {
			while ((code = parser.GetTag(len)) != -1) {
				parser.pos = parser.tagEnd;
				if (code == stagShowFrame) {
					if (!--framesLeft) {
						break;
					}
				}
			}
			if (framesLeft) {
				// No such frame
				return FALSE;
			}
		}

		// Scan for DoAction tags in the frame
		while ((code = parser.GetTag(len)) != -1) {
			if (code == stagDoAction) {
				thread->display->PushAction(parser.script+parser.pos, thread);
			}
			if (code == stagShowFrame) {
				break;
			}
			parser.pos = parser.tagEnd;
		}

		// Do the actions!
		DoActions();
		UpdateScreen();

		return TRUE;
	}

	return FALSE;
}

#endif


//
// Button support
//

// 
// Button interface methods
//

void SPlayer::ButtonGoDirection(int dir)
{
	ButtonLock();

	SButton oldButton = currentButton;

	// Remove focus from current button
	ButtonFocusRemove(currentButton.button);

	// Construct button ordering around current button
	buttonOrder.Reset();
	ButtonOrdering(&oldButton, &buttonOrder);

	if (buttonOrder.button[dir]) {
		// Set button
		currentButton.button	   = buttonOrder.button[dir];
		currentButton.buttonBounds = buttonOrder.bounds[dir];

		// Set focus to new button
		ButtonFocusSet(currentButton.button);
	} else {
		// Restore old focus
		currentButton = oldButton;
		ButtonFocusSet(oldButton.button);
	}

	ButtonUnlock();
}

void SPlayer::ButtonEnter()
{
	SButton* button = &currentButton;

	if (button->button &&
		button->button->character &&
		button->button->character->type == buttonChar)
	{
		// Take action on button

		// Simulate mouse down
		DoButton(button->button, true); 

		// Simulate mouse up
		DoButton(button->button, false); 
	}

	//XReleaseCapture();
}

void SPlayer::ButtonTabNext()
{
	ButtonLock();

	SButton* button = &currentButton;

#ifdef EDITTEXT
	if (focus) {
		button->button = focus;
		focus->CalcButtonBounds(&button->buttonBounds);
	}
#endif

	int weight = -1;
	if (button->button) {
		weight = CalcButtonWeight(&button->buttonBounds);
	}

	// Remove focus from current button
	ButtonFocusRemove(button->button);

	ButtonWeight* buttonTabArray = BuildButtonTabMap();
	if (!buttonTabArray) {
		button->button = NULL;
		ButtonUnlock();
		return;
	}

	// Find the first weight greater than this one
	int index = 0;
	while (index < buttonIndex) {
		if (buttonTabArray[index].weight > weight) {
			break;
		}
		index++;
	}
	if (index >= buttonIndex) {
		index = 0;
	}

	button->button		 = buttonTabArray[index].button;
	button->buttonBounds = buttonTabArray[index].buttonBounds;

	// Set focus to new button
	ButtonFocusSet(button->button);

	delete [] buttonTabArray;

	ButtonUnlock();
}

void SPlayer::ButtonTabPrevious()
{
	ButtonLock();

	SButton* button = &currentButton;

#ifdef EDITTEXT
	if (focus) {
		button->button = focus;
		focus->CalcButtonBounds(&button->buttonBounds);
	}
#endif

	int weight = -1;
	if (button->button) {
		weight = CalcButtonWeight(&button->buttonBounds);
	}

	// Remove focus from current button
	ButtonFocusRemove(button->button);

	ButtonWeight* buttonTabArray = BuildButtonTabMap();
	if (!buttonTabArray) {
		button->button = NULL;
		ButtonUnlock();
		return;
	}

	// Find the first button weight less than this one
	int index = buttonIndex - 1;
	if (weight != -1) {
		while (index >= 0) {
			if (buttonTabArray[index].weight < weight) {
				break;
			}
			index--;
		}
		if (index < 0) {
			index = buttonIndex - 1;
		}
	}

	button->button		 = buttonTabArray[index].button;
	button->buttonBounds = buttonTabArray[index].buttonBounds;

	// Set focus to new button
	ButtonFocusSet(button->button);

	delete [] buttonTabArray;

	ButtonUnlock();
}

// 
// Button implementation
// 

void SPlayer::ButtonOrdering(SButton* button, ButtonOrder* order)
{
	FLASHASSERT(button);
	FLASHASSERT(order);

	RemoveAllButtons();

	FLASHASSERT(&display.root);
	int iNumButtons = NumButtons(&display.root);
	if ( iNumButtons == 0 )
		return;	// no buttons in this control

	SObject* dummyButton = NULL;
	SObject* buttonObj   = NULL;
	SRECT    buttonBounds;

	if ( button->button ) {
		// Use current button and previously calculated boundary
		buttonObj	 = button->button;
		buttonBounds = button->buttonBounds;
		//buttonObj->CalcButtonBounds(&buttonBounds);
	}
	else {
		dummyButton = display.CreateObject();
		FLASHASSERT(dummyButton);
	
		SRECT originRect = {0, 0, 0, 0};
		buttonObj		 = dummyButton;
		buttonBounds	 = originRect;
	}

	// Arrange button ordering
	for ( int i=0; i<iNumButtons; i++ ) {
		SObject* buttoni = buttonArray[i];
		SRECT    buttoniBounds;
		
		if ( buttoni == buttonObj ) {
			// Skip current button
			continue;
		}

		buttoni->CalcButtonBounds(&buttoniBounds);

		SObject* buttonChosen;

		if ( IsButtonUp(&buttonBounds, &buttoniBounds) ) {
			buttonChosen = ButtonChoose(buttonObj, &buttonBounds, UP, buttoni, &buttoniBounds, order->button[UP], &order->bounds[UP]);
		
			if ( buttonChosen == buttoni ) {
				order->button[UP] = buttoni;
				order->bounds[UP] = buttoniBounds;
			}
		}

		if ( IsButtonDown(&buttonBounds, &buttoniBounds) ) {
			buttonChosen = ButtonChoose(buttonObj, &buttonBounds, DOWN, buttoni, &buttoniBounds, order->button[DOWN], &order->bounds[DOWN]);
		
			if ( buttonChosen == buttoni ) {
				order->button[DOWN] = buttoni;
				order->bounds[DOWN] = buttoniBounds;
			}
		}

		if ( IsButtonRight(&buttonBounds, &buttoniBounds) ) {
			buttonChosen = ButtonChoose(buttonObj, &buttonBounds, RIGHT, buttoni, &buttoniBounds, order->button[RIGHT], &order->bounds[RIGHT]);
		
			if ( buttonChosen == buttoni ) {
				order->button[RIGHT] = buttoni;
				order->bounds[RIGHT] = buttoniBounds;
			}
		}

		if ( IsButtonLeft(&buttonBounds, &buttoniBounds) ) {
			buttonChosen = ButtonChoose(buttonObj, &buttonBounds, LEFT, buttoni, &buttoniBounds, order->button[LEFT], &order->bounds[LEFT]);
		
			if ( buttonChosen == buttoni ) {
				order->button[LEFT] = buttoni;
				order->bounds[LEFT] = buttoniBounds;
			}
		}
	}

	if ( dummyButton ) {
		display.FreeObject(dummyButton);
	}
}

SObject* SPlayer::ButtonChoose(
						SObject* button, 
						P_SRECT  buttonBounds,
						int		 direction, 
						SObject* button1, 
						P_SRECT  button1Bounds,
						SObject* button2,
						P_SRECT  button2Bounds)
// Choose which button is closest to the current button in the direction specified
{
	FLASHASSERT(button);
	FLASHASSERT(button1 || button2);

	if ( !button1 ) {
		return(button2);
	}
	else if ( !button2 ) {
		return(button1);
	}

	FLASHASSERT(button1 && button2);

	BOOL bStraight1 = IsButtonAligned(buttonBounds, direction, button1Bounds);
	BOOL bStraight2 = IsButtonAligned(buttonBounds, direction, button2Bounds);

	if ( bStraight1 && bStraight2 ) {
		// Both buttons are aligned in the specified direction,
		// so take the closer one.
		P_SRECT buttonCloser =  ButtonCloser(buttonBounds, button1Bounds, button2Bounds);
		return ( buttonCloser == button1Bounds ) ? button1 : button2;
	}
	else if ( bStraight1 ) {
		return button1;
	}
	else if ( bStraight2 ) {
		return button2;
	}

	// Neither of the buttons are aligned with the current button


	BOOL bOverlap1  = IsButtonOverlapped(buttonBounds, direction, button1Bounds);
	BOOL bOverlap2  = IsButtonOverlapped(buttonBounds, direction, button2Bounds);

	if ( bOverlap1 && bOverlap2 ) {
		// Both buttons overlap, so take the closer one
		P_SRECT buttonCloser =  ButtonCloser(buttonBounds, button1Bounds, button2Bounds);
		return ( buttonCloser == button1Bounds ) ? button1 : button2;
	}
	else if ( bOverlap1 ) {
		return button2;
	}
	else if ( bOverlap2 ) {
		return button1;
	}
	else {
		// Neither button overlaps, so take the closer one
		P_SRECT buttonCloser =  ButtonCloser(buttonBounds, button1Bounds, button2Bounds);
		return ( buttonCloser == button1Bounds ) ? button1 : button2;
	}

}

BOOL SPlayer::IsButtonUp(P_SRECT origin, P_SRECT button)
{
	return (button->ymax < origin->ymin || button->ymax < origin->ymax);
}

BOOL SPlayer::IsButtonDown(P_SRECT origin, P_SRECT button)
{
	return (button->ymin > origin->ymax || button->ymax > origin->ymax);
}

BOOL SPlayer::IsButtonRight(P_SRECT origin, P_SRECT button)
{
	return (button->xmin > origin->xmax || button->xmax > origin->xmax);
}

BOOL SPlayer::IsButtonLeft(P_SRECT origin, P_SRECT button)
{
	return (button->xmax < origin->xmin || button->xmin < origin->xmin);
}

BOOL SPlayer::IsButtonAligned(P_SRECT origin, int direction, P_SRECT button)
{
	FLASHASSERT(origin);
	FLASHASSERT(button);

	if(direction == DOWN || direction == UP)
	{
		if( ( origin->xmax < button->xmin ) ||
		    ( origin->xmin > button->xmax ) ) {
			return false;
		}
		else {
			return true;
		}
	}
	else if ( direction == RIGHT || direction == LEFT ) {
		if( ( origin->ymax < button->ymin ) ||
		    ( origin->ymin > button->ymax ) ) {
			return false;
		}
		else {
			return true;
		}
	}

	FLASHASSERT(0);
	return false;
}

BOOL SPlayer::IsButtonOverlapped(P_SRECT origin, int direction, P_SRECT button)
{
	FLASHASSERT(origin);
	FLASHASSERT(button);

	if ( direction == DOWN ) {
		FLASHASSERT(IsButtonDown(origin, button));
		return ( button->ymin > origin->ymax );
	}

	return false;
}

P_SRECT SPlayer::ButtonCloser(P_SRECT origin, P_SRECT button1, P_SRECT button2)
{
	FLASHASSERT(origin);
	FLASHASSERT(button1);
	FLASHASSERT(button2);

	SCOORD d1 = RectDistance(origin, button1);
	SCOORD d2 = RectDistance(origin, button2);

	return (d1 < d2) ? button1 : button2;
}

BOOL SPlayer::ButtonFind(SButton* removed, SButton* current)
// Find the current button that (most closely) matches the removed button. 
{
	ButtonLock();

	FLASHASSERT(removed);
	FLASHASSERT(current);

	RemoveAllButtons();

	int iNumButtons = NumButtons(&display.root);
	if ( iNumButtons == 0 ) {
		ButtonUnlock();
		return false;
	}

	for ( int i=0; i<iNumButtons; i++ ) {
		SObject* buttoni = buttonArray[i];
		FLASHASSERT(buttoni);

		SRECT buttoniBounds;
		buttoni->CalcButtonBounds(&buttoniBounds);

		// Check if button sizes are equal
		if ( RectEqual(&buttoniBounds, &removed->buttonBounds, 0) ) {
			current->button		  = buttoni;
			current->buttonBounds = buttoniBounds;

			ButtonUnlock();
			return true;
		}
	}

	ButtonUnlock();
	return false;
}

void SPlayer::DoButton(SObject* hitBtn, BOOL mouseIsDown, BOOL updateScreen)
{
#ifdef EDITTEXT
	if (hitBtn) {
		if (hitBtn->character->type == editTextChar) {
			if (focus != hitBtn) {
				if (focus) {
					cursorBlink = false;
					focus->editText->ClearSelection();
					focus->Modify();
				}
				focus = hitBtn;
				EditText* editText = focus->editText;
				editText->m_selectionStart = 0;
				editText->m_selectionEnd = editText->m_length;
				focus->Modify();
				display.iBeam = true;
			}
			return;
		}
	}
	display.iBeam = false;
	if (mouseIsDown && focus && !focus->editText->m_selecting) {
		cursorBlink = false;
		focus->editText->ClearSelection();
		focus->Modify();
		focus = 0;
	}
#endif

	if ( hitBtn ) {
		// Check object is really a button
		//FLASHASSERT(hitBtn->character->type == buttonChar);
		if ( !hitBtn->character ) {
			FLASHASSERT(hitBtn->character);
			return;
		}
		
		if ( hitBtn->character->type != buttonChar ) {
			FLASHASSERT(hitBtn->character->type == buttonChar);
			return;
		}

	}

	FLASHASSERT(!hitBtn || hitBtn->character->type == buttonChar);
	if ( display.SetButton(hitBtn, mouseIsDown) ) {
		if (!RectIsEmpty(&display.tabFocusRect)) {
			display.InvalidateRect(&display.tabFocusRect);
		}
		if (hitBtn) {
			hitBtn->CalcButtonBounds(&display.tabFocusRect);
			display.InvalidateRect(&display.tabFocusRect);
		} else {
			RectSetEmpty(&display.tabFocusRect);
		}

		DoActions();
		if ( updateScreen )
			UpdateScreen();
		UpdateCursor();

		//if ( display.button && mouseIsDown ) {
		//	XSetCapture();
		//} else {
		//	XReleaseCapture();
		//}
	}

	mouseState = mouseIsDown;
}

int SPlayer::NumButtons(SObject* obj)
{
	FLASHASSERT(obj);

	int iNumButtons = 0;

	obj = obj->bottomChild;
	while ( obj ) {

		BOOL isButton = false;

		if (obj->character) {

	#ifdef EDITTEXT
			if (obj->character->type == editTextChar &&
				!(obj->editText->m_flags & seditTextFlagsReadOnly)) {
				isButton = true;
			}
	#endif
			if (obj->character->type == buttonChar) {
				isButton = true;
			}
		}

		if (isButton) {
			iNumButtons += 1;
			AddButton(obj);
		}	
		
		iNumButtons += NumButtons(obj);
		obj = obj->above;
	}

	return iNumButtons;
}

void SPlayer::ButtonFocusSet(SObject* button)
{
#ifdef EDITTEXT
	// Clear the edit text focus
	if (focus) {
		focus->editText->ClearSelection();
		focus->Modify();
		focus = NULL;
	}
#endif

	DoButton(button, false);

	// Set mouseState to -1 so the current button won't
	// change until the mouse is moved.
	mouseState = -1;
}

void SPlayer::ButtonFocusRemove(SObject* button)
{
	DoButton((SObject*)NULL, false);
}

//
// Methods used to help enumerate buttons
//

static int		ADDITIONAL_BUTTONS = 50;

void SPlayer::AddButton(SObject* obj)
{
	FLASHASSERT(obj);

	if ( buttonIndex == arraySize ) {
		// Dynamically resize array
		SObject** newArray = new SObject*[arraySize + ADDITIONAL_BUTTONS];
		FLASHASSERT(newArray);

		// Copy old array into new array
		memcpy(newArray, buttonArray, arraySize * sizeof(SObject*));
	
		// Delete old array
		delete [] buttonArray;

		// Initialize new array
		buttonArray = newArray;
		arraySize  += ADDITIONAL_BUTTONS; 
	}
	
	FLASHASSERT(buttonIndex < arraySize);

	// Add button to end of button array.
	FLASHASSERT(buttonArray);
	buttonArray[buttonIndex++] = obj;
}

void SPlayer::RemoveAllButtons()
{
	buttonIndex = 0;
	delete [] buttonArray;
	buttonArray = NULL;
	arraySize   = 0;
}

//
// Tabbing implementation
//

SPlayer::ButtonWeight* SPlayer::BuildButtonTabMap()
{
	ButtonWeight* buttonTabArray = NULL;

	RemoveAllButtons();

	int iNumButtons = NumButtons(&display.root);

	if ( iNumButtons == 0 ) {
		return NULL;
	}

	buttonTabArray = new ButtonWeight[iNumButtons+1];
	FLASHASSERT(buttonTabArray);

	SRECT originRect = {0, 0, 0, 0};

	for ( int i=0; i<iNumButtons; i++ ) {
		SObject* button = buttonArray[i];
		FLASHASSERT(button);

		buttonTabArray[i].button = button;
		button->CalcButtonBounds(&buttonTabArray[i].buttonBounds);

		// Use an algorithm to weight each button.
		buttonTabArray[i].weight = CalcButtonWeight(&buttonTabArray[i].buttonBounds);
	}

	// Sort buttons based on weight
	ButtonSort(buttonTabArray, iNumButtons-1);

	return buttonTabArray;
}

S32 SPlayer::CalcButtonWeight(P_SRECT bounds)
// Weight button by distance from origin, giving more weight to
// vertical direction than horizontal.
{
	// Weight factors
	const int xWeightFactor = 3;
	const int yWeightFactor = 2;
	return (bounds->xmin/xWeightFactor) + (bounds->ymin*yWeightFactor);
}

void SPlayer::ButtonSort(ButtonWeight* array, int size)
// Sort buttons based on distance
{
	int i,j;
	ButtonWeight t;

	for ( i=size; i>=0; i-- ) {
		for ( j=1; j<=i; j++ ) {
			if( array[j-1].weight > array[j].weight ) {
				t = array[j-1];
				array[j-1] = array[j];
				array[j]   = t;
			}
		}
	}
}

void SPlayer::CreateIdealPalette( SColorTable* ctab )
{
	const U8 colorRamp6[] = { 0, 0x33, 0x66, 0x99, 0xCC, 0xff };

	// Set up the a Palette of colors that we would like to be available.
	// Fill in the color ramp - 6 levels of each color
	int i = 0;
	for ( int b = 0; b <= 5; b++ ) {
		for ( int g = 0; g <= 5; g++ ) {
			for ( int r = 0; r <= 5; r++ ) {
				if ( r == g && g == b ) 
					continue;// don't do any grays here
				ctab->colors[i].red   = colorRamp6[r];
				ctab->colors[i].green = colorRamp6[g];
				ctab->colors[i].blue  = colorRamp6[b];
				ctab->colors[i].alpha = 0;
				i++;
			}
		}
	}
	ctab->n = i;
}

// update the buffer and blt to the screen given a DC
void SPlayer::OnDraw()
{
	// BACKCAST
	NativePlayerWnd* native = (NativePlayerWnd*)(this);

	if ( UpdateBuffer() && bits.LockBits() ) 
	{
		if ( !RectIsEmpty( &display.screenDirtyRgn ) ) 
		{
			// simply blt the buffer to the screen
			bits.bltToScreen(	native,
								display.screenDirtyRgn.xmin, display.screenDirtyRgn.ymin,
 								RectWidth(&display.screenDirtyRgn), RectHeight(&display.screenDirtyRgn),
								display.screenDirtyRgn.xmin, display.screenDirtyRgn.ymin
							);
		}

		UnlockBuffer();

		bits.UnlockBits();

	} else {
		bits.clearScreen( native );
	}
}

void SPlayer::Repaint( SRECT* rect )
{
	SRECT dev = *rect;

	if ( display.antialias ) {
		dev.xmin *= 4;
		dev.ymin *= 4;
		dev.xmax *= 4;
		dev.ymax *= 4;
	}

	display.InvalidateRect( &dev );
	OnDraw();
	RectSetEmpty(&display.screenDirtyRgn);
}

void SPlayer::UpdateScreen()
{
	// Copy to the screen
	SRECT dirty;

	display.CalcBitsDirty(&dirty);
	RectUnion(&dirty, &display.screenDirtyRgn, &dirty);

 	if ( !RectIsEmpty(&dirty) )
	{
		OnDraw();
	}
	RectSetEmpty(&display.screenDirtyRgn);
}

BOOL SPlayer::UpdateBuffer(BOOL render)
{
	// Check to create the offscreen bitmap
	if ( !bits.LockBits() ) {
		
 		SRECT cr;		
		ClientRect( &cr );

		if ( cr.xmin == cr.xmax || cr.ymin == cr.ymax ) {
			// The Window is empty, just exit
			return true;
		}
		bits.setAllowPurge( true );

		// Back-casting.
		NativePlayerWnd* native = (NativePlayerWnd*)( this );
		bits.CreateScreenBits( native, &cr );
		if ( !bits.LockBits() )
			return false;
		SetCamera(updateNone);
		display.ModifyCamera();	// be sure cached colors get flushed and display gets rebuilt...
        InvalidateScreenArea( &cr );
	}

	if ( render ) {
		// Be sure the bits are up to date
		display.Update();
	}

	return true;
}

void SPlayer::CheckUpdate()
// generate an InvalidateRect if the screen needs redrawn
{	
	SRECT dirty;
	display.CalcBitsDirty(&dirty);
	RectUnion(&dirty, &display.screenDirtyRgn, &dirty);
 	if ( !RectIsEmpty(&dirty) ) 
	{
        InvalidateScreenArea( &dirty );
		RectSetEmpty(&display.screenDirtyRgn);
	}
}

int SPlayer::GetCursorType()
{
	int type = CURSOR_TYPE_ARROW;
	if ( display.button )
	{
		type = CURSOR_TYPE_BUTTON;
	}
	else if ( !RectIsEmpty(&zoomRect) ) 
	{
		type = CURSOR_TYPE_HAND;
	}

	#ifdef EDITTEXT
		if ( display.iBeam ) 
		{
			type = CURSOR_TYPE_BEAM;
		}
	#endif
	
	return type;
}

char* SPlayer::BuildRelative(char* base, char* url)
{
	int maxLen = strlen(base)+strlen(url)+10;
	char* urlBuf = new char[maxLen];
	if ( !urlBuf ) return 0;

	char* c = url;
	while ( *c ) {
		if ( *c == '/' ) break;
		if ( *c == ':' ) {
			// It's already an absolute URL
			strcpy(urlBuf, url);
			return urlBuf;
		}
		c++;
	}

	// Build a relative URL
	strcpy(urlBuf, base);	// 

	// Extract just the machine name from the base
	char* m = urlBuf;
	while ( true ) {
		if ( !*m ) {
			m = urlBuf;	// we did not find a machine name
			goto NoMachine;
		}
		if ( m[0] == '/' && m[1] == '/' ) {
			m += 2;
			break;
		}
		m++;
	}
	while ( true ) {
		if ( !*m ) {
			// Add a trailing / to the machine name
			*m++ = '/';
			*m = 0;
			break;
		} 
		if ( m[0] == '/' ) {
			m++;
			break;
		}
		m++;
	}

	// m should be pointing to the first path 
NoMachine:
	if ( url[0] == '/' ) {
		// A root (machine) relative url
		*m = 0;	// strip any path info

		strcat(urlBuf, url+1);
		return urlBuf;

	} else {
		// Remove "../" from the url
		do {
			// Remove a directory (or file on first pass) from the base URL
			char* tail = urlBuf + strlen(urlBuf)-1;
			if ( *tail == '/' && tail >= m ) *tail-- = 0;
			while ( *tail && tail >= m ) {
				if ( *tail == '/' ) break;
				*tail-- = 0;
			}

			char* u = SkipPrefix(url, "../");
			if ( !u ) break;
			url = u; // Strip the prefix
		} while ( true );

		strcat(urlBuf, url);
		return urlBuf;
	}
}

char* SPlayer::NSResolveURL(char* actionURL)
{
	if ( urlBase ) {
		// Generate URLs relative to a given path
		char* ub = urlBase;
		if ( urlBase[0] == '.' && urlBase[1] == 0 ) {
			// Relative to the .spl file
			ub = url;
		}
		return BuildRelative(ub, actionURL);
	}
	// Let Netscape resolve the URL
	return actionURL;
}

void SPlayer::NSGetURL(char* actionURL, char* window, char* postData)
{
	// Check for the special FSCommand URL
	if ( window[0] == 0 )
		window = "_self";

	char* resolvedURL = NSResolveURL(actionURL);
	if ( resolvedURL ) 
	{
		NetscapeDoURL(resolvedURL, window, postData, 0 );
		if ( resolvedURL != actionURL )
		{
			delete resolvedURL;
		}
	}
}

void SPlayer::NetscapeDoURL(char* url, char* window, char* postData, LoadInfo* info )
// Handle a URL operation for Netscape based on whether we want a notify and or a POST operation
// Note that this routine will do a Notify if the window is NULL, otherwise, it will do a 
{
  if ( postData && *postData ) 
  {
		// Build up the headers that Navigator wants for a POST...
		URLBuilder ub;
		ub.AppendString("Content-type: application/x-www-form-urlencoded\n"
						"Content-length: ");
		ub.AppendInt(strlen(postData));
		ub.AppendString("\n\n");
		ub.AppendString(postData);
		ub.AppendString("\n");

// 		notifyData = notify;	
		StreamPostURLNotify( url, window, ub.len+1, ub.buf, (void*)info ); 
	}
	else 
	{
		// Just do a GET
// 		notifyData = notify;
		StreamGetURLNotify( url, window, (void*)info ); 
	}
}

void SPlayer::LoadLayer(  char* url,
						  int layer,
						  char* postData,
						  char *target,
						  int loadMethod)
{
    BOOL            loadVars = loadMethod & kHttpLoadVariables;
    BOOL            loadTarget = loadMethod & kHttpLoadTarget;
    ScriptThread    *thread = 0;

    // find target
    if (loadTarget)
    {
        thread = SPlayer::FindTarget(player.rootObject, target);

        if (!thread)
            return;

        // unload original sprite before anything else
        if (!loadVars)
            RemoveSprite(thread, true);
    }
    
    if ( *url == 0 && !loadVars && !loadTarget && layer >= 0) 
    {
        ClearLayer(layer);
		return;
	}

    LoadInfo* loadInfo = new LoadInfo;
    S32 notify = (S32) loadInfo;

    if (!loadInfo) {
      return;
    }
    
    // see if we are loading a sprite instead
    // of a movie layer. If so find the target
    // thead that is going to get the new script
    if (loadTarget)
    {
		loadInfo->layer = -1;
		loadInfo->spriteName = CreateStr(target);
		loadInfo->loadVars = loadVars;
    }
    else 
	{
		// If this is a Load Variable action, use the LoadInfo
		// structure as the notifyData so NPSHELL will open the
		// layer with loadVars set.
		if (loadVars) 
		{
			loadInfo->layer = layer;
			loadInfo->spriteName = NULL;
			loadInfo->loadVars = true;
		}
		else 
		{
			ClearLayer(layer); 	// clear out the old contents of the layer immediately??
			loadInfo->layer = layer;
			loadInfo->spriteName = NULL;
			loadInfo->loadVars = false;
		}
	}

    // NOTE: we don't remove sprites if we can't resolve the url.
    //          The user can use remove sprite action instead

	// Start loading a new stream
	char* resolvedURL = NSResolveURL(url);
	if ( resolvedURL ) 
	{
		NetscapeDoURL( resolvedURL, 0, postData, loadInfo );
		if ( resolvedURL != url )
		{
			delete resolvedURL;
		}
	}
}


void SPlayer::GetURL( char* actionURL, char* window, char* postData, int loadMethod )
{
	char* cmd = SkipPrefix(actionURL, "FSCommand:");
	if ( cmd )
	{
		ProcessFSCommand( cmd, window );
	}
	else 
	{    
        // Check to load a layer
        char *target = window;
		int layerNum = LayerNum(target);
        if (layerNum >= 0 || (loadMethod & kHttpLoadTarget))
        {
            // see if we are loading into a target
            if (!(loadMethod & kHttpLoadTarget)) target = 0;
			// LoadLayer moved from native to core.
			LoadLayer(actionURL, layerNum, postData, target, loadMethod);
			return;
        }
		else
		{
			// NSGetURL moved from native to core
			NSGetURL(actionURL, window, postData);
		}
	}
}

void SPlayer::StreamInNew( StreamData* streamData, char* streamUrl, void* notifyData )
{
	FLASHOUTPUT( "StreamInNew %x\n", streamData );

	// copy the url to stream data, since the client should not modify streamData
	streamData->url = CreateStr( streamUrl );

	// Open up a layer for the the stream
	S32 layer;
	if ( firstStream )
	{
		layer = 0;				// force the first stream to load to layer 0
		streamData->resize = true;
	}
   	else
	{
		layer = (S32)notifyData;		// If layer is -1, then this is just a notice from the browser.
										// If it is positive, then it is actually a pointer to a LoadInfo struct
		streamData->resize = false;
	}
	
	if ( layer >= 0 ) 
	{
        ScriptPlayer* p = 0;
		LoadInfo* loadInfo = (LoadInfo*) layer;
        
        if (layer == 0)
        {
		    p = OpenLayer(0);
        }
        else
        {
			// This stream is for a Load Sprite or a Load Variable action.

			if (loadInfo->loadVars) 
			{
				p = OpenVariableLoader(loadInfo->layer, loadInfo->spriteName);
			}
			else 
			{
				if (loadInfo->spriteName) {
					// Loaded to a target path
					ScriptThread *thread = FindTarget( display.root.bottomChild->bottomChild, 
													   loadInfo->spriteName);
					if (!thread) 
					{
						goto exit_gracefully;
					}
					p = OpenSprite(thread);
				} else {
					// Loaded to a layer number
					p = OpenLayer(loadInfo->layer);
				}
			}

            // cleanup
			FreeStr(loadInfo->spriteName);
            delete loadInfo;
        }

//         *stype = NP_ASFILE;	// this forces the data to be cached even if the file is large
		//*stype = NP_NORMAL;

		// Parse the variables from the URL and add to the base thread
		if (!p->variableLoader) {
			ExtractVariables( p, streamData->url);
		}

		p->stream = streamData;	// link the objects
		streamData->scriptPlayer = p;

		p->url = CreateStr(streamData->url);

		// Do special stuff for the first stream we get
		if ( firstStream ) {
			firstStream = false;

			// Save the base URL
			FreeStr(url);
			url = CreateStr(streamData->url);
		}
	} else {
		// If layer < 0, this is just a notify from a browser navigate operation and we don't care
	}
exit_gracefully:
	return;
}

int SPlayer::StreamInWrite( StreamData*	streamData,
							void* buffer,
							int	  length )
{
// 	FLASHOUTPUT( "StreamInWrite %x\n", streamData );
	ScriptPlayer* p = streamData->scriptPlayer;
	if ( p ) {
		p->PushData((U8*)buffer, length);

		// see if we can size the window - added lee@middlesoft
		if ( streamData->resize && p->gotHeader )
		{
			int width = (int)RectWidth( &p->frame ) / 20;
			int height = (int)RectHeight( &p->frame ) / 20;
			AdjustWindow( width, height );
			streamData->resize = false;
		}

		return length;
	}
	
	return -1;	// we want to abort the stream...
}

void SPlayer::StreamInDestroy( StreamData* streamData )
{
	FLASHOUTPUT( "StreamInDestroy %x\n", streamData );
    ScriptPlayer* p = streamData->scriptPlayer;
	if ( p ) {
		p->stream = 0;	// disconnect the stream

		// Resize the window if it has not happened yet.
		if ( streamData->resize )
		{
			int width = (int)RectWidth( &p->frame ) / 20;
			int height = (int)RectHeight( &p->frame ) / 20;
			AdjustWindow( width, height );
		}
		p->SetDataComplete();
	}

	// clean up the url memory
	if ( streamData->url )
	{
		FreeStr( streamData->url );
		streamData->url = 0;
	}
}

int GetUnixFontData(char **data);

#ifdef EDITTEXT
BOOL SPlayer::LoadBuiltInFonts()
{
  if (builtInFontsState != kBuiltInFontsNotLoaded) {
    return (builtInFontsState == kBuiltInFontsLoaded);
  }
  
  char *fontScript = 0;
  int fontScriptLen = GetUnixFontData(&fontScript);
  if (fontScriptLen <= 0) {
    builtInFontsState = kBuiltInFontsError;
    return false;
  }

  builtInFontsPlayer.SetDisplay(&display);
  builtInFontsPlayer.splayer = this;
  builtInFontsPlayer.layerDepth = 16000;

  builtInFontsPlayer.PushData((U8*) fontScript, fontScriptLen);

  builtInFontsPlayer.DoTags(0);

  delete [] fontScript;

  builtInFontsState = kBuiltInFontsLoaded;
  return true;
}
#endif