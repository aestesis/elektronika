/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright © Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 
****************************************************************************/

#ifndef SSTROKER_INCLUDED
#define SSTROKER_INCLUDED

#ifndef CURVE_INCLUDED
#include "curve.h"
#endif
#ifndef SPLAY_INCLUDED
#include "splay.h"
#endif

class SStroker {
public:
	BOOL antialias;
	BOOL preclip;

	DisplayList* display;
	REdge** edgeList;
	
	// Context for adding a stroke run
	// Cap/join context
	RColor* color;
	BOOL isThick;
	S32 lineThickness;
	BOOL strokeInited;
	int strokeDepth;
	SPOINT lStartPt, startOrigin, rStartPt;
	SPOINT lCurPt, curOrigin, rCurPt;
	SPOINT curPt;
	
	// Internal routines for building the edges
	void AddEdge(SPOINT* pt1, SPOINT* pt2);
 	void AddCurve(CURVE*);
 	
	void StrokeJoin(P_SPOINT pt1, P_SPOINT pt2, P_SPOINT origin);
	void StrokeThickCurve(P_CURVE c);
	void StrokeThinCurve(P_CURVE c);
	void StrokeThinLine(P_CURVE c);
 	
public:
	SStroker(DisplayList* p, REdge** list);

	void BeginStroke(S32 thickness, RColor*);
	void AddStrokeCurve(CURVE*);
	void EndStroke();
};

// This is a private helper...
void MakeVertLine(CURVE* c, SCOORD x);

#endif
