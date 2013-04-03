/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright © Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 
****************************************************************************/

#ifndef GEOM_INCLUDED
#define GEOM_INCLUDED

#ifndef FIXED_INCLUDED
#include "fixed.h"
#endif

//
// TYPES
//

#define oneInch 1440L
#define onePoint 20L
#define oneMillimeter 57L
#define oneCentimeter 570L

typedef S32 SCOORD, *P_SCOORD;

typedef struct SPOINT {
	SCOORD x;
	SCOORD y;
} SPOINT, *P_SPOINT;

#define rectEmptyFlag 0x80000000L
typedef struct SRECT {	// Note that a rectangle is considered empty if xmin == rectEmptyFlag
	SCOORD xmin;
	SCOORD xmax;
	SCOORD ymin;
	SCOORD ymax;
} SRECT, *P_SRECT;

enum { // reference points on a rectangle
		rectTopLeft = 0, rectTopRight, rectBottomRight, rectBottomLeft, 
		rectTopCenter, rectCenterRight, rectBottomCenter, rectCenterLeft,
		rectCenter
	};

typedef struct MATRIX {
	SFIXED a;
	SFIXED b;
	SFIXED c;
	SFIXED d;
	SCOORD tx;
	SCOORD ty;
} MATRIX, *P_MATRIX;

//
// SPOINT FUNCTIONS
//

void PointScale(P_SPOINT pt, SFIXED factor, P_SPOINT dst);
void PointShift(P_SPOINT src, int shift, P_SPOINT dst);
void PointAverage(P_SPOINT pt1, P_SPOINT pt2, SFIXED f, P_SPOINT dst);

//
// Inline Functions
//

inline void PointAdd(P_SPOINT pt1, P_SPOINT pt2, P_SPOINT dst)
{
	dst->x = pt1->x + pt2->x;
	dst->y = pt1->y + pt2->y;
}

inline void PointSub(P_SPOINT pt1, P_SPOINT pt2, P_SPOINT dst)
{
	dst->x = pt1->x - pt2->x;
	dst->y = pt1->y - pt2->y;
}

inline void PointAverage(P_SPOINT pt1, P_SPOINT pt2, P_SPOINT dst)
{
	dst->x = (pt1->x + pt2->x)>>1;
	dst->y = (pt1->y + pt2->y)>>1;
}

BOOL PointEqual(P_SPOINT pt1, P_SPOINT pt2, SCOORD err);

inline BOOL PointEqual(P_SPOINT pt1, P_SPOINT pt2) 
{ 
	return pt1->x == pt2->x && pt1->y == pt2->y; 
}

SFIXED PointAngle(P_SPOINT pt, P_SPOINT origin);
// Return the angle of the vector from origin to pt
// Origin can be nil as a shorthand for (0,0)

void PointPolar(SCOORD length, SFIXED angle, P_SPOINT origin, P_SPOINT dst);
// Generate a polar from the polar representaion

SCOORD PointNearestLine(P_SPOINT l1, P_SPOINT l2, P_SPOINT pt, P_SPOINT nearestPt);
// Find the point nearest to pt in line (l1, l2)
// If nearestPt is not null, return the
// Returns the distance from pt to nearestPt

BOOL LineIntersection(P_SPOINT a1, P_SPOINT a2, P_SPOINT b1, P_SPOINT b2, SFIXED* uA, SFIXED* uB);
// Calculate the intersection of line (a1,a2) and line (b1,b2)
// If both uA and uB are nil, simply tests to see if the lines intersect

SCOORD PointFastLength(P_SPOINT pt);
SCOORD PointFastDistance(P_SPOINT pt1, P_SPOINT pt2);
// Calculate a very fast approximation of the distance between pt1 amd pt2

SCOORD PointLength(P_SPOINT pt);
SCOORD PointDistance(P_SPOINT pt1, P_SPOINT pt2);
// Calculate the distance between pt1 and pt2

void PointNormalize(SPOINT* v, SCOORD thickness);


//
// SRECT FUNCTIONS
//

void RectSet(SCOORD xmin, SCOORD ymin, SCOORD xmax, SCOORD ymax, P_SRECT dst);
void RectSetPoint(P_SPOINT pt, P_SRECT dst);
void RectSetPoint2(P_SPOINT, P_SPOINT, P_SRECT dst);
void RectSetEmpty(P_SRECT dst);
void RectSetHuge(P_SRECT dst);
void RectCenter(P_SRECT r, P_SPOINT pt);
void RectValidate(P_SRECT dst);
void RectOffset(SCOORD dx, SCOORD dy, P_SRECT dst);
void RectInset(SCOORD dist, P_SRECT dst);
void RectShift(P_SRECT src, int shift, P_SRECT dst);

#define RectIsEmpty(r) ((r)->xmin == rectEmptyFlag)
void RectGetPoint(P_SRECT r, int ref, P_SPOINT pt);
int RectOppositePoint(int);
void RectUnion(P_SRECT r1, P_SRECT r2, P_SRECT dst);
void RectUnionPoint(P_SPOINT pt, P_SRECT dst);
void RectIntersect(P_SRECT r1, P_SRECT r2, P_SRECT dst);
BOOL RectTestIntersect(P_SRECT r1, P_SRECT r2);
BOOL RectEqual(P_SRECT r1, P_SRECT r2, SCOORD err);
BOOL RectPointIn(P_SRECT r, P_SPOINT pt);
//BOOL RectContains(P_SRECT big, P_SRECT small);
SCOORD RectSize(P_SRECT);
void RectSizePoint(P_SRECT, P_SPOINT);
SCOORD RectNearDistance(P_SRECT, P_SPOINT);
SCOORD RectDistance(P_SRECT, P_SRECT);
#define RectWidth(r) ((r)->xmax - (r)->xmin)
#define RectHeight(r) ((r)->ymax - (r)->ymin)


//
// MATRIX FUNCTIONS
//

void MatrixTransformPoint(P_MATRIX m, P_SPOINT p, P_SPOINT dst);
void MatrixDeltaTransformPoint(P_MATRIX m, P_SPOINT p, P_SPOINT dst);
void MatrixTransformRect(P_MATRIX m, P_SRECT r, P_SRECT dst);
SCOORD MatrixTransformThickness(P_MATRIX m, SCOORD);

void MatrixConcat(P_MATRIX m1, P_MATRIX m2, P_MATRIX dst);
// Given m1 that maps from A to B and m2 that maps from B to C
// Generates dst that maps from A to C

void MatrixInvert(P_MATRIX m, P_MATRIX dst);
// Given m that maps from A to B
// Generates dst that maps from B to A

BOOL MatrixEqual(P_MATRIX m1, P_MATRIX m2, SFIXED err);

void MatrixIdentity(P_MATRIX m);
void MatrixRotate(SFIXED degrees, P_MATRIX m);
void MatrixTranslate(SCOORD dx, SCOORD dy, P_MATRIX m);
void MatrixScale(SCOORD sx, SCOORD sy, P_MATRIX m);

// #ifdef SPRITE_XTRA
// void MatrixFlipX(P_MATRIX m);
// void MatrixFlipY(P_MATRIX m);
// void MatrixSkew(SFIXED degrees, P_MATRIX m);
// #endif // SPRITE_XTRA

void MatrixMap(P_SRECT src, P_SRECT dst, P_MATRIX m);
// Generates m that will map any point in src to the coresponding point in dst

void MatrixMapAspect(P_SRECT src, P_SRECT dst, P_MATRIX m);	
// Map rects while maintaining a constant aspect ratio


void MatrixDecompose(MATRIX* mat, SPOINT* scale, SFIXED* rot);
BOOL MatrixIsScaleOnly(P_MATRIX m);
BOOL MatrixIsIdentity(P_MATRIX m);

#endif
