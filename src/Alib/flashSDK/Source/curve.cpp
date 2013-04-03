/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright © Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 

  This file contains routines for working with quadratic bezier curve segments

***************************************************************************/

#include "curve.h"

//
// CURVE CONSTANTS
//

#define minAngleDelta 10


//
// Simple Utilities
//

SFIXED CurveLineCalcParam(P_CURVE c, P_SPOINT pt)
// Given a curve that is a line and a point on that line, returns the
//	parameter value
{
	return FixedDiv(PointDistance(&c->anchor1, pt), PointDistance(&c->anchor1, &c->anchor2));
}

SFIXED CurveParamInterpolate(SFIXED u, SFIXED iu, SFIXED lu)
{
	return u + FixedMul(lu-fixedHalf, 2*iu);
}


//
// CURVE FUNCTIONS
//

void CurveSet(P_SPOINT anchor1, P_SPOINT control, P_SPOINT anchor2, P_CURVE dst)
{
	dst->anchor1 = *anchor1;
	dst->control = *control;
	dst->anchor2 = *anchor2;
	dst->isLine = false;
}

void CurveSetLine(P_SPOINT pt1, P_SPOINT pt2, P_CURVE dst)
{
	dst->anchor1 = *pt1;
	PointAverage(pt1, pt2, &(dst->control));
	dst->anchor2 = *pt2;
	dst->isLine = true;
}

void CurveBounds(P_CURVE c, P_SRECT bounds)
{
	if ( c->anchor1.x < c->anchor2.x ) {
		bounds->xmin = c->anchor1.x;
		bounds->xmax = c->anchor2.x;
	} else {
		bounds->xmin = c->anchor2.x;
		bounds->xmax = c->anchor1.x;
	}
	if ( c->anchor1.y < c->anchor2.y ) {
		bounds->ymin = c->anchor1.y;
		bounds->ymax = c->anchor2.y;
	} else {
		bounds->ymin = c->anchor2.y;
		bounds->ymax = c->anchor1.y;
	}
	if ( !c->isLine ) {
		if ( c->control.x < bounds->xmin ) 
			bounds->xmin = c->control.x;
		else if ( c->control.x > bounds->xmax ) 
			bounds->xmax = c->control.x;

		if ( c->control.y < bounds->ymin ) 
			bounds->ymin = c->control.y;
		else if ( c->control.y > bounds->ymax ) 
			bounds->ymax = c->control.y;
	}
}

void CurveTightBounds(P_CURVE c, P_SRECT bounds)
// Calculate a smaller bounds rectangle for the curve - is slower
{
	RectSetPoint2(&c->anchor1, &c->anchor2, bounds);

	if ( !c->isLine ) {
		SPOINT extrema;

 		CurveExtrema(c, &extrema.x, &extrema.y);
		RectUnionPoint(&extrema, bounds);
	}
}

#define MinMax3(a, b, c, min, max) if ( a < b ) { \
				min = a;			\
				max = b;			\
			} else {				\
				min = b;			\
				max = a;			\
			}						\
			if ( c < min ) 			\
				min = c;			\
			else if ( c > max ) 	\
				max = c;

BOOL CurveBoundsSect(P_CURVE curveA, P_CURVE curveB, SCOORD expandA, SCOORD expandB, P_SCOORD sizeA, P_SCOORD sizeB)
{
	SCOORD minAx, maxAx, minBx, maxBx;
	SCOORD minAy, maxAy, minBy, maxBy;

	MinMax3(curveA->anchor1.x, curveA->anchor2.x, curveA->control.x, minAx, maxAx);
	MinMax3(curveB->anchor1.x, curveB->anchor2.x, curveB->control.x, minBx, maxBx);

	if ( maxBx+expandB < minAx-expandA || maxAx+expandA < minBx-expandB ) return false;

	MinMax3(curveA->anchor1.y, curveA->anchor2.y, curveA->control.y, minAy, maxAy);
	MinMax3(curveB->anchor1.y, curveB->anchor2.y, curveB->control.y, minBy, maxBy);

	if ( maxBy+expandB < minAy-expandA || maxAy+expandA < minBy-expandB ) return false;

	*sizeA = Max(maxAx - minAx, maxAy - minAy);
	*sizeB = Max(maxBx - minBx, maxBy - minBy);

	return true;
}

SCOORD CurveCalcExtreme(SCOORD anchor1, SCOORD control, SFIXED anchor2)
{
	SCOORD a, b;

	a = anchor1 - 2*control + anchor2;
	b = anchor1 - control;

	// See if b/a is in the range 0..1
	// a and b must have the same sign and abs(b) <= abs(b)
	if ( ((a > 0 && b >= 0) || (a < 0 && b <= 0)) && Abs(b) <= Abs(a) ) {
		SFIXED u;
		u = FixedDiv(b, a);
		return FixedMul(FixedMul(u, u), a) - 2*FixedMul(u, b) + anchor1;
	}

	return anchor1;
}

void CurveExtrema(P_CURVE c, P_SCOORD x, P_SCOORD y)
// calculates the maxima or minima of the x and y coordinates
//
//	P = A*u^2 - 2*B*u + C
//
//	A = anchor1 - 2*control + anchor2
//	B = anchor1 - control
//	C = anchor1
//
//	dy/du = 2*a*u - 2*b
//
{
	if ( x )
		*x = CurveCalcExtreme(c->anchor1.x, c->control.x, c->anchor2.x);
	if ( y )
		*y = CurveCalcExtreme(c->anchor1.y, c->control.y, c->anchor2.y);
}

SCOORD CurveFlatness(P_CURVE c)
{
	SCOORD dx, dy; 

	if ( c->isLine ) return 0;
	
	dx = ((c->anchor1.x + c->anchor2.x)>>1) - c->control.x;
	dy = ((c->anchor1.y + c->anchor2.y)>>1) - c->control.y;
	
	if ( dx < 0 ) dx = -dx;
	if ( dy < 0 ) dy = -dy;
	
	return dx + dy - (Min(dx, dy)>>1);

//	SPOINT midPoint;
//
//	if ( c->isLine ) return 0;
//	
//	PointAverage(&c->anchor1, &c->anchor2, fixedHalf, &midPoint);
//	return PointFastDistance(&c->control, &midPoint);
}

void CurveDivide(P_CURVE c, P_CURVE dst1, P_CURVE dst2)
{
	SPOINT control1, control2, newAnchor;
	
	PointAverage(&c->control, &c->anchor1, &control1);
	PointAverage(&c->anchor2, &c->control, &control2);
	PointAverage(&control2, &control1, &newAnchor);
	
	dst1->anchor1 = c->anchor1;
	dst1->control = control1;
	dst1->anchor2 = 
	dst2->anchor1 = newAnchor;
	dst2->control = control2;
	dst2->anchor2 = c->anchor2;

	dst1->isLine = dst2->isLine = c->isLine;
}

void CurveDivide(P_CURVE c, SFIXED u, P_CURVE dst1, P_CURVE dst2)
{
	SPOINT control1, control2, newAnchor;
	
	PointAverage(&c->control, &c->anchor1, u, &control1);
	PointAverage(&c->anchor2, &c->control, u, &control2);
	PointAverage(&control2, &control1, u, &newAnchor);
	
	dst1->anchor1 = c->anchor1;
	dst1->control = control1;
	dst1->anchor2 = 
	dst2->anchor1 = newAnchor;
	dst2->control = control2;
	dst2->anchor2 = c->anchor2;

	dst1->isLine = dst2->isLine = c->isLine;
}

void CurveDivideTwo(P_CURVE c, SFIXED u1, SFIXED u2, P_CURVE dst1, P_CURVE dst2, P_CURVE dst3)
{
	CURVE tmp;
	SPOINT pt;
	SFIXED u;

	// Sort the divide points
	if ( u1 > u2 ) 
		Swap(u1, u2, SFIXED);
	
	CurveDivide(c, u1, dst1, &tmp);
	CurveCalcPoint(c, u2, &pt);
	u = CurveNearestPoint(&tmp, &pt, 1000, 0);
	CurveDivide(&tmp, u, dst2, dst3);
}

void CurveCalcPoint(P_CURVE c, SFIXED u, P_SPOINT dst)
//
// pt = (1-u)^2*anchor1 + 2*u*(1-u)*control + u^2*anchor2
//
{
	if ( u == fixedHalf ) {
		// Special case for speed...
		dst->x = (2*c->control.x + c->anchor1.x + c->anchor2.x)/4; 
		dst->y = (2*c->control.y + c->anchor1.y + c->anchor2.y)/4; 

	} else {
		SCOORD a, b;
		SFIXED u2;

		u2 = FixedMul(u, u);

		a = c->anchor1.x - 2*c->control.x + c->anchor2.x;
		b = c->anchor1.x - c->control.x;
		dst->x = FixedMul(u2, a) - 2*FixedMul(u, b) + c->anchor1.x;

		a = c->anchor1.y - 2*c->control.y + c->anchor2.y;
		b = c->anchor1.y - c->control.y;
		dst->y = FixedMul(u2, a) - 2*FixedMul(u, b) + c->anchor1.y;
	}
}

void CurveTransform(P_CURVE c, MATRIX* m, P_CURVE dst)
{
	MatrixTransformPoint(m, &c->anchor1, &dst->anchor1);
	MatrixTransformPoint(m, &c->control, &dst->control);
	MatrixTransformPoint(m, &c->anchor2, &dst->anchor2);
	dst->isLine = c->isLine;
}

SFIXED CurveTangentAngle(P_CURVE c, SFIXED u)
{
	if ( u == 0 ) {
		if ( PointFastDistance(&c->control, &c->anchor1) < minAngleDelta )
			return PointAngle(&c->anchor2, &c->anchor1);
		else
			return PointAngle(&c->control, &c->anchor1);
		
	} else if ( u == fixed_1 ) {
		if ( PointFastDistance(&c->anchor2, &c->control) < minAngleDelta )
			return PointAngle(&c->anchor2, &c->anchor1);
		else
			return PointAngle(&c->anchor2, &c->control);

	} else {
		CURVE A,B;
		
		CurveDivide(c, u, &A, &B);
		if ( u < fixedHalf )
			return CurveTangentAngle(&A, 0);
		else
			return CurveTangentAngle(&B, fixed_1);
	}
}

SCOORD CurveLength(P_CURVE c)
{
	if ( CurveIsLine(c) ) {
		return PointDistance(&c->anchor1, &c->anchor2);
		
	} else {
	  // Set up the difference equation
	  	CURVE_STEPPER stepper;
		SCOORD length = 0;
		
  	  // Calculate points along the curve
		//CurveInitStepper(c, 32, &stepper);
		CurveInitStepper(c, 16, &stepper);
		while ( CurveStep(&stepper) ) {
			length += PointDistance(&stepper.prevPt, &stepper.pt);
		}
		return length;
	}
}

SFIXED CurveFindLength(P_CURVE c, SCOORD len)
{
	SFIXED u;
	
	if ( CurveIsLine(c) ) {
		u = FixedDiv(len, PointDistance(&c->anchor1, &c->anchor2));
		
	} else {
	  	CURVE_STEPPER stepper;
		SCOORD length = 0;
		SCOORD dlength = 0;
		
  	  // Calculate points along the curve
		//CurveInitStepper(c, 32, &stepper);
		CurveInitStepper(c, 16, &stepper);

	  // Set up the difference equation
		while ( length < len && CurveStep(&stepper) ) {
			dlength = PointDistance(&stepper.prevPt, &stepper.pt);
			length += dlength;
		}
	
		u = stepper.u;
		if ( dlength > 0 ) u -= FixedMul(stepper.du, FixedDiv(length - len, dlength));
	}
	
	if ( u < 0 ) return 0;
	if ( u > fixed_1 ) return fixed_1;
	return u;
}


//
// Curve Nearest Point
//

typedef struct {
	SPOINT pt;
	SFIXED bestU;
	SCOORD bestErr;
} FIT_INFO, *P_FIT_INFO;

void DoCurveNearestPoint(P_CURVE c, P_FIT_INFO info, SFIXED u, SFIXED iU)
{
	SRECT bounds;

	CurveBounds(c, &bounds);

 	if ( RectNearDistance(&bounds, &info->pt) > info->bestErr )
		return;

	if ( CurveFlatness(c) > 4 && iU > 2 ) {
		CURVE a, b;

		CurveDivide(c, &a, &b);
		iU >>= 1;

		if ( PointFastDistance(&a.control, &info->pt) < 
			 PointFastDistance(&b.control, &info->pt) ) {
			DoCurveNearestPoint(&a, info, u-iU, iU);
			DoCurveNearestPoint(&b, info, u+iU, iU);
		} else {
			DoCurveNearestPoint(&b, info, u+iU, iU);
			DoCurveNearestPoint(&a, info, u-iU, iU);
		}

	} else {
	 	// We have approximately a line
		SCOORD err;
		SPOINT nearPt;

		err = PointNearestLine(&c->anchor1, &c->anchor2, &info->pt, &nearPt);
		if ( err < info->bestErr ) {
			info->bestU = CurveParamInterpolate(u, iU, CurveLineCalcParam(c, &nearPt));
			info->bestErr = err;
		}
	}
}

SFIXED CurveNearestPoint(P_CURVE c, P_SPOINT pt, SCOORD dist, P_SPOINT dst)
// returns -1 if pt is not within dist
{
	if ( c->isLine ) {
		SPOINT nearPt;
		if ( PointNearestLine(&c->anchor1, &c->anchor2, pt, &nearPt) < dist ) {
			if ( dst ) 
				*dst = nearPt;
			return CurveLineCalcParam(c, &nearPt);
		} else 
			return -fixed_1;

	} else {
		FIT_INFO info;

		info.pt = *pt;
		info.bestErr = dist;
		info.bestU = -fixed_1;

		DoCurveNearestPoint(c, &info, fixedHalf, fixedHalf);

		if ( dst && info.bestU >= 0 )
			CurveCalcPoint(c, info.bestU, dst);

		return info.bestU;
	}
}

void CurveReverse(P_CURVE c, P_CURVE dst)
{
	if ( c == dst ) {
		Swap(c->anchor1, c->anchor2, SPOINT);
	} else {
		dst->anchor1 = c->anchor2;
		dst->control = c->control;
		dst->anchor2 = c->anchor1;
		dst->isLine = c->isLine;
	}
}


U16 CurveXRaySect(P_CURVE c, P_SPOINT pt, U16 depth)
// Count the number of intersections of a horizontal ray starting
//	at pt with the curve c. Will count end point intersections 
//	for connected paths only once.
// Always pass zero for depth.
{
	SRECT bounds;

	CurveBounds(c, &bounds);

	// We only want intersections on one side of the point
	if ( bounds.xmax < pt->x ||
		 bounds.ymin > pt->y ||
		 bounds.ymax <= pt->y ) return 0;

 	if ( RectPointIn(&bounds, pt) && depth < 12 && RectSize(&bounds) > 4 ) {
	 	CURVE a, b;

		CurveDivide(c, &a, &b);
		depth++;

		return CurveXRaySect(&a, pt, depth) + CurveXRaySect(&b, pt, depth);

	} else {
		SCOORD yMin, yMax;

		#if defined(DEBUG) && !defined(UNDER_CE)
	 	if ( RectPointIn(&bounds, pt) && RectSize(&bounds) > 4 ) {
		 	FLASHOUTPUT("XRay point in, depth=%i, size=%i", depth, RectSize(&bounds));
		}
		#endif

		yMin = c->anchor1.y;
		yMax = c->anchor2.y;

		// Horizontal lines don't count
		if ( yMin == yMax ) return 0;

		if ( yMin > yMax ) 
			Swap(yMin, yMax, SCOORD);
		
		if ( pt->y >= yMin && pt->y < yMax ) 
			return 1;
	}

	return 0;
}

// CURVE STEPPER FUNCTIONS
//

#define PointShift(src, n, dst) (dst)->x = (src)->x >> n; (dst)->y = (src)->y >> n;

#define nGuardBits 4 //10

void CurveInitStepper(P_CURVE c, U16 n, CURVE_STEPPER* stepper)
//
//	P = A*u^2 + B*u + C
//
//	A = anchor1 - 2*control + anchor2
//	B = 2*control - 2*anchor1
//	C = anchor1
//
//	D(P, u) = 2*u*A + B
//
//	D2(P, u) = 2*A
//
{
	SPOINT A, B;
   
	// Set up the difference equation

	// compute quadratic coeffs
	PointScale(&c->control, fixed2, &A);
	PointSub(&c->anchor1, &A, &A);
	PointAdd(&c->anchor2, &A, &A);
	
	PointSub(&c->control, &c->anchor1, &B);
	PointScale(&B, fixed2, &B);
	
	stepper->du = FixedDiv(1, n);

	PointScale(&B, stepper->du << nGuardBits, &stepper->dxH);
	PointScale(&A, FixedMul(stepper->du, stepper->du) << (1+nGuardBits), &stepper->dvH);
	
	stepper->nLeft = n;
	
	// Init stepping values
	stepper->u = 0;
	stepper->ptH.x = c->anchor1.x << nGuardBits;
	stepper->ptH.y = c->anchor1.y << nGuardBits;
	stepper->prevPt = stepper->pt = c->anchor1;
}

BOOL CurveStep(CURVE_STEPPER* stepper)
{
	if ( stepper->nLeft <= 0 ) return false;
	
	PointAdd(&stepper->dxH, &stepper->ptH, &stepper->ptH);
	PointAdd(&stepper->dxH, &stepper->dvH, &stepper->dxH);

	stepper->u  += stepper->du;
	stepper->nLeft--;

	stepper->prevPt = stepper->pt;
	PointShift(&stepper->ptH, nGuardBits, &stepper->pt);
	
	return true;
}

//
// Cubic Curve Support
//

BOOL CurveIsExtreme(P_CURVE c)
{
	SPOINT midPoint;

	//return false;
	
	if ( c->isLine ) return false;

	PointAverage(&c->anchor1, &c->anchor2, &midPoint);
	SCOORD dc = PointFastDistance(&c->control, &midPoint);
	if ( dc <= 6 ) return false;
	SCOORD da = PointFastDistance(&c->anchor1, &c->anchor2);
	return 2*dc > da;

//	if ( dc < 2 ) return false;
//	SCOORD da = PointFastDistance(&c->anchor1, &c->anchor2);
//	return 3*dc > da;
//	return dc > da/2;
}

void CurveAdjust(P_CURVE src, P_SPOINT pt1, P_SPOINT pt2, P_CURVE dst)
{
	SPOINT v1, v2;
	SPOINT control;
	SFIXED ratio;
	SCOORD srcDist;

	srcDist = PointDistance(&src->anchor1, &src->anchor2);
	if ( srcDist > 0 ) 
		ratio = FixedDiv(PointDistance(pt1, pt2), srcDist);
	else
		ratio = fixed_1;
	
	PointSub(&src->control, &src->anchor1, &v1);
	PointSub(&src->control, &src->anchor2, &v2);
	if ( PointFastLength(&v1) > PointFastLength(&v2) ) {
		PointScale(&v1, ratio, &v1);
		PointAdd(&v1, pt1, &control);
	} else {
		PointScale(&v2, ratio, &v2);
		PointAdd(&v2, pt2, &control);
	}
	
	CurveSet(pt1, &control, pt2, dst);
}
