/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright © Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 
****************************************************************************/
//	990325	mnk	"fixed1" -> "fixed_1"


#include "geom.h"


//
// SPOINT Routines
//

void PointScale(P_SPOINT pt, SFIXED factor, P_SPOINT dst)
{
	dst->x = FixedMul(factor, pt->x);
	dst->y = FixedMul(factor, pt->y);
}

void PointShift(P_SPOINT src, int shift, P_SPOINT dst)
{
	if ( shift >= 0 ) {
		dst->x = src->x << shift;
		dst->y = src->y << shift;
	} else {
		SCOORD round;

		shift = -shift;
		round = 1 << (shift-1);
		dst->x = (src->x+round) >> shift;
		dst->y = (src->y+round) >> shift;
	}
}

void PointAverage(P_SPOINT pt1, P_SPOINT pt2, SFIXED f, P_SPOINT dst)
{
	//if ( f == fixedHalf ) {
	//	dst->x = (pt1->x + pt2->x)>>1;
	//	dst->y = (pt1->y + pt2->y)>>1;
	//} else {
		// Factor the equation to save a couple of multiplies
		dst->x = pt2->x + FixedMul(f, pt1->x - pt2->x);
		dst->y = pt2->y + FixedMul(f, pt1->y - pt2->y);
		//SFIXED f1;
		//f1 = fixed_1-f;
		//dst->x = FixedMul(f, pt1->x) + FixedMul(f1, pt2->x);
		//dst->y = FixedMul(f, pt1->y) + FixedMul(f1, pt2->y);
	//}
}

BOOL PointEqual(P_SPOINT pt1, P_SPOINT pt2, SCOORD err)
{
	return  FixedAbs(pt1->x - pt2->x) <= err && 
			FixedAbs(pt1->y - pt2->y) <= err;
}

SCOORD PointFastLength(P_SPOINT pt)
{
	SCOORD dx = FixedAbs(pt->x);
	SCOORD dy = FixedAbs(pt->y);
	return dx + dy - (FixedMin(dx, dy) >> 1);
}


// step = 0.015625;
// DistanceTable[i] = sqrt(1 + (i*step)^2)
#define AdjDE(x) ((x+0x200)>>10)
static const S32 gDistanceTable[] = {
	AdjDE(1073741824L), AdjDE(1073872888L), AdjDE(1074265984L), AdjDE(1074920825L), 
	AdjDE(1075836932L), AdjDE(1077013639L), AdjDE(1078450093L),	AdjDE(1080145258L), 
	AdjDE(1082097918L), AdjDE(1084306681L), AdjDE(1086769986L), AdjDE(1089486107L), 
	AdjDE(1092453157L), AdjDE(1095669100L),	AdjDE(1099131748L), AdjDE(1102838780L), 
	AdjDE(1106787739L), AdjDE(1110976045L), AdjDE(1115401003L), AdjDE(1120059807L), 
	AdjDE(1124949552L),	AdjDE(1130067241L), AdjDE(1135409791L), AdjDE(1140974043L), 
	AdjDE(1146756771L), AdjDE(1152754686L), AdjDE(1158964447L), AdjDE(1165382668L),
	AdjDE(1172005924L), AdjDE(1178830760L), AdjDE(1185853694L), AdjDE(1193071229L), 
	AdjDE(1200479854L), AdjDE(1208076055L), AdjDE(1215856315L),	AdjDE(1223817123L), 
	AdjDE(1231954981L), AdjDE(1240266402L), AdjDE(1248747921L), AdjDE(1257396097L), 
	AdjDE(1266207514L), AdjDE(1275178788L),	AdjDE(1284306569L), AdjDE(1293587545L), 
	AdjDE(1303018442L), AdjDE(1312596028L), AdjDE(1322317116L), AdjDE(1332178565L), 
	AdjDE(1342177280L),	AdjDE(1352310217L), AdjDE(1362574382L), AdjDE(1372966831L), 
	AdjDE(1383484673L), AdjDE(1394125071L), AdjDE(1404885240L), AdjDE(1415762448L),
	AdjDE(1426754019L), AdjDE(1437857331L), AdjDE(1449069814L), AdjDE(1460388955L), 
	AdjDE(1471812291L), AdjDE(1483337417L), AdjDE(1494961978L),	AdjDE(1506683672L), 
	AdjDE(1518500250L), AdjDE(1518500250L) };

SCOORD PointLength(P_SPOINT vec)
{
	SCOORD minC, maxC;
	SFIXED slope, ratio;
	S32* f;
	
	minC = FixedAbs(vec->x);
	maxC = FixedAbs(vec->y);
	if ( minC > maxC ) {
		SCOORD tmp = minC;
		minC = maxC;
		maxC = tmp;
	}

	if ( maxC == 0 ) return 0;
	
	slope = FixedDiv(minC, maxC);	// will always be between zero and one
	f = (P_S32)gDistanceTable+(slope >> 10);
	ratio = slope & 0x03FFL;	// (modulus step)/step
	return FixedMul(maxC, ((0x400 - ratio)*f[0] + ratio*f[1])>>14);
	//return FixedMul(maxC, ((0x3ff - ratio)*f[0] + ratio*f[1])>>14);
}

SCOORD PointDistance(P_SPOINT pt1, P_SPOINT pt2)
{
	SPOINT vec;
	PointSub(pt1, pt2, &vec);
	return PointLength(&vec);
}

SCOORD PointFastDistance(P_SPOINT pt1, P_SPOINT pt2)
{
	SCOORD dx = pt1->x - pt2->x;
	SCOORD dy = pt1->y - pt2->y;
	if ( dx < 0 ) dx = -dx;
	if ( dy < 0 ) dy = -dy;
	return ( dx + dy - Min(dx, dy) )>>1;
}

SFIXED PointAngle(P_SPOINT pt, P_SPOINT origin)
{
	if ( !origin )
		return FixedAtan2(pt->y, pt->x);
	else
		return FixedAtan2(pt->y - origin->y, pt->x - origin->x);
}

void PointPolar(SCOORD length, SFIXED angle, P_SPOINT origin, P_SPOINT dst)
{
	dst->x = FixedMul(length, FixedCos(angle));
	dst->y = FixedMul(length, FixedSin(angle));
	if ( origin ) {
	 	dst->x += origin->x;
	 	dst->y += origin->y;
	}
}

void PointPerpNearestLine(P_SPOINT l1, P_SPOINT l2, P_SPOINT pt, P_SPOINT nearestPt)
// Return the approximate nearest SPOINT to pt on the line segment l1,l2
//	the answer is exact for vertical or horizontal lines
{
	SPOINT d;
	SPOINT lMin, lMax;

	PointSub(l2, l1, &d);
	
	if ( FixedAbs(d.y) > FixedAbs(d.x) ) {
	  // A mostly vertical line
		if ( l1->y <= l2->y ) {
			lMin = *l1;
			lMax = *l2;
		} else {
			lMin = *l2;
			lMax = *l1;
		}
			
		if ( pt->y < lMin.y ) {
			*nearestPt = lMin;
		} else if ( pt->y > lMax.y ) {
			*nearestPt = lMax;
		} else {
			nearestPt->x = FixedMul(pt->y - lMin.y, FixedDiv(d.x, d.y)) + lMin.x;
			nearestPt->y = pt->y;
		}
		
	} else {
	  // A mostly horizontal line
		if ( l1->x <= l2->x ) {
			lMin = *l1;
			lMax = *l2;
		} else {
			lMin = *l2;
			lMax = *l1;
		}

		if ( pt->x < lMin.x ) {
			*nearestPt = lMin;
		} else if ( pt->x > lMax.x ) {
			*nearestPt = lMax;
		} else {
			nearestPt->x = pt->x;
			nearestPt->y = FixedMul(pt->x - lMin.x, FixedDiv(d.y, d.x)) + lMin.y;
		}
	}
}

SCOORD PointNearestLine(P_SPOINT l1, P_SPOINT l2, P_SPOINT pt, P_SPOINT nearestPt)
// Return the nearest SPOINT on the line segment l1,l2 to pt
{
	SPOINT d, dAbs;
	SPOINT memNearestPt;

	if ( !nearestPt ) nearestPt = &memNearestPt;

	PointSub(l2, l1, &d);

	dAbs.x = Abs(d.x);
	dAbs.y = Abs(d.y);
	
	if ( dAbs.x == 0 || dAbs.y == 0 || (dAbs.x >> 5) > dAbs.y || (dAbs.y >> 5) > dAbs.x ) {
//	if ( FixedEqual(d.x, 0, fixedStdErr) || FixedEqual(d.y, 0, fixedStdErr) ) {
	  // Special case vertical and horizontal lines
		PointPerpNearestLine(l1, l2, pt, nearestPt);

	} else {
	  // A sloped line
	  	SFIXED m, im;
		SFIXED b, c;
		P_SPOINT minPt, maxPt;
	  
	  	m = FixedDiv(d.y, d.x);		// slope of line
		im = FixedDiv(d.x, d.y);	// inverse of slope of line, -im is normal
		
	  // Find out where normal hits the line of interest
		b = l1->y - FixedMul(m, l1->x);
		c = pt->y + FixedMul(im, pt->x);
		nearestPt->x = FixedDiv(c - b, m + im);
		nearestPt->y = FixedMul(-im, nearestPt->x) + c;
		
	  // Be sure that the SPOINT is on the segment
		if ( l1->x < l2->x ) {
			minPt = l1;
			maxPt = l2;
		} else {
			minPt = l2;
			maxPt = l1;
		}
		if ( nearestPt->x < minPt->x )
			*nearestPt = *minPt;
		else if ( nearestPt->x > maxPt->x )
			*nearestPt = *maxPt;
	}

	return PointDistance(nearestPt, pt);
}


void PointNormalize(SPOINT* v, SCOORD thickness)
{
	SCOORD invD = PointLength(v);
	if ( invD > 0 ) {
		invD = FixedDiv(thickness, invD);
		v->x = FixedMul(invD, v->x);
		v->y = FixedMul(invD, v->y);
	}
}


//
// SRECT Routines
//

void RectSet(SCOORD xmin, SCOORD ymin, SCOORD xmax, SCOORD ymax, P_SRECT dst)
{	
	if ( xmin < xmax ) {
		dst->xmin = xmin;
		dst->xmax = xmax;
	} else {
		dst->xmin = xmax;
		dst->xmax = xmin;
	}
	if ( ymin < ymax ) {
		dst->ymin = ymin;
		dst->ymax = ymax;
	} else {
		dst->ymin = ymax;
		dst->ymax = ymin;
	}
}

void RectSetPoint(P_SPOINT pt, P_SRECT dst)
{
	dst->xmin = dst->xmax = pt->x;
	dst->ymin = dst->ymax = pt->y;
}

void RectSetPoint2(P_SPOINT pt1, P_SPOINT pt2, P_SRECT dst)
{
	if ( pt1->x < pt2->x ) {
		dst->xmin = pt1->x;
		dst->xmax = pt2->x;
	} else {
		dst->xmin = pt2->x;
		dst->xmax = pt1->x;
	}
	if ( pt1->y < pt2->y ) {
		dst->ymin = pt1->y;
		dst->ymax = pt2->y;
	} else {
		dst->ymin = pt2->y;
		dst->ymax = pt1->y;
	}
}

void RectSetEmpty(P_SRECT dst)
{
	dst->xmin = dst->xmax = dst->ymin = dst->ymax = rectEmptyFlag;
}

void RectSetHuge(P_SRECT dst)
{
	dst->xmin = dst->ymin = -maxS32>>9;
	dst->xmax = dst->ymax =  maxS32>>9;
}

void RectCenter(P_SRECT rect, P_SPOINT pt)
{
	if (RectIsEmpty(rect))
	{
		pt->x = pt->y = 0;
	}
	else
	{
		pt->x = rect->xmin + (rect->xmax - rect->xmin) / 2;
		pt->y = rect->ymin + (rect->ymax - rect->ymin) / 2;
	}
}

void RectOffset(SCOORD dx, SCOORD dy, P_SRECT dst)
{
	if ( !RectIsEmpty(dst) ) {
		dst->xmin += dx;
		dst->xmax += dx;
		dst->ymin += dy;
		dst->ymax += dy;
	}
}

void RectInset(SCOORD dist, P_SRECT dst)
{
	if ( !RectIsEmpty(dst) ) {
		dst->xmin += dist;
		dst->xmax -= dist;
		dst->ymin += dist;
		dst->ymax -= dist;
		RectValidate(dst);
	}
}

void RectShift(P_SRECT src, int shift, P_SRECT dst)
{
	if ( RectIsEmpty(src) ) {
		*dst = *src;
	} else {
		if ( shift >= 0 ) {
			dst->xmin = src->xmin << shift;
			dst->xmax = src->xmax << shift;
			dst->ymin = src->ymin << shift;
			dst->ymax = src->ymax << shift;
		} else {
			SCOORD round;

			shift = -shift;
			round = 1 << (shift-1);
			dst->xmin = (src->xmin+round) >> shift;
			dst->xmax = (src->xmax+round) >> shift;
			dst->ymin = (src->ymin+round) >> shift;
			dst->ymax = (src->ymax+round) >> shift;
		}
	}
}

void RectValidate(P_SRECT r)
{
	if ( r->xmin > r->xmax || r->ymin > r->ymax )
		RectSetEmpty(r);
}

int RectOppositePoint(int ref)
{
	switch ( ref ) {
		case rectTopLeft: 		return rectBottomRight;
		case rectTopRight: 		return rectBottomLeft;
		case rectBottomLeft: 	return rectTopRight;
		case rectBottomRight: 	return rectTopLeft;

		case rectTopCenter: 	return rectBottomCenter;
		case rectBottomCenter: 	return rectTopCenter;
		case rectCenterRight: 	return rectCenterLeft;
		case rectCenterLeft: 	return rectCenterRight;

		default:				return rectCenter;
	}
}

void RectGetPoint(P_SRECT r, int ref, P_SPOINT pt)
{
	switch ( ref ) {
		case rectTopLeft: 
			pt->x = r->xmin;
			pt->y = r->ymax;
			break;
		case rectTopRight:
			pt->x = r->xmax;
			pt->y = r->ymax;
			break;
		case rectBottomRight:
			pt->x = r->xmax;
			pt->y = r->ymin;
			break;
		case rectBottomLeft:
			pt->x = r->xmin;
			pt->y = r->ymin;
			break;
		case rectTopCenter:
			pt->x = (r->xmin+r->xmax)>>1;
			pt->y = r->ymax;
			break;
		case rectCenterRight: 
			pt->x = r->xmax;
			pt->y = (r->ymin+r->ymax)>>1;
			break;
		case rectBottomCenter:
			pt->x = (r->xmin+r->xmax)>>1;
			pt->y = r->ymin;
			break;
		case rectCenterLeft:
			pt->x = r->xmin;
			pt->y = (r->ymin+r->ymax)>>1;
			break;
		default:
		case rectCenter:
			pt->x = (r->xmin+r->xmax)>>1;
			pt->y = (r->ymin+r->ymax)>>1;
			break;
	}
}

void RectUnion(P_SRECT r1, P_SRECT r2, P_SRECT dst)
{
	if ( RectIsEmpty(r1) ) {
		*dst = *r2;
	} else if ( RectIsEmpty(r2) ) {
		*dst = *r1;
	} else {
		dst->xmin = Min(r1->xmin, r2->xmin);
		dst->xmax = Max(r1->xmax, r2->xmax);
		dst->ymin = Min(r1->ymin, r2->ymin);
		dst->ymax = Max(r1->ymax, r2->ymax);
	}
}

void RectUnionPoint(P_SPOINT pt, P_SRECT dst)
{
	if ( RectIsEmpty(dst) ) {
		RectSetPoint(pt, dst);
	} else {
		if ( pt->x < dst->xmin ) dst->xmin = pt->x;
		else if ( pt->x > dst->xmax ) dst->xmax = pt->x;

		if ( pt->y < dst->ymin ) dst->ymin = pt->y;
		else if ( pt->y > dst->ymax ) dst->ymax = pt->y;
	}
}

void RectIntersect(P_SRECT r1, P_SRECT r2, P_SRECT dst)
{
	if ( RectIsEmpty(r1) || RectIsEmpty(r2) ) {
		RectSetEmpty(dst);
	} else {
		dst->xmin = FixedMax(r1->xmin, r2->xmin);
		dst->xmax = FixedMin(r1->xmax, r2->xmax);
		dst->ymin = FixedMax(r1->ymin, r2->ymin);
		dst->ymax = FixedMin(r1->ymax, r2->ymax);
		
		RectValidate(dst);
	}
}

BOOL RectTestIntersect(P_SRECT r1, P_SRECT r2)
{
	return !RectIsEmpty(r1) &&	// if one or both rect is empty, they cannot intersect
		   r1->xmin <= r2->xmax && r2->xmin <= r1->xmax &&
		   r1->ymin <= r2->ymax && r2->ymin <= r1->ymax;
}

BOOL RectPointIn(P_SRECT r, P_SPOINT pt)
{
	return	r->xmin <= pt->x && pt->x <= r->xmax &&
			r->ymin <= pt->y && pt->y <= r->ymax;
}

/*
BOOL RectContains(P_SRECT big, P_SRECT small)
{
	return	big->xmin <= small->xmin && 
			big->xmax >= small->xmax &&
			big->ymin <= small->ymin && 
			big->ymax >= small->ymax;
}
*/

BOOL RectEqual(P_SRECT r1, P_SRECT r2, SCOORD err)
{
	if ( !RectIsEmpty(r1) && !RectIsEmpty(r2) ) {
	  // Neither SRECT is empty
		return 	FixedEqual(r1->xmin, r2->xmin, err) && 
				FixedEqual(r1->xmax, r2->xmax, err) && 
				FixedEqual(r1->ymin, r2->ymin, err) && 
				FixedEqual(r1->ymax, r2->ymax, err);
	} else {
	  // See if both rects are empty
		return RectIsEmpty(r1) && RectIsEmpty(r2);
	}
}

SCOORD RectSize(P_SRECT r)
{
	SPOINT tmp;
	tmp.x = RectWidth(r);
	tmp.y = RectHeight(r);
	return PointLength(&tmp);
}

void  RectSizePoint(P_SRECT r, P_SPOINT pt)
{
	pt->x = RectWidth(r);
	pt->y = RectHeight(r);
}

SCOORD RectNearDistance(P_SRECT r, P_SPOINT pt)
// Find the distance from pt to the nearest point contained in r
{
	SPOINT nearPt;

 	if ( RectPointIn(r, pt) ) return 0;

	if ( pt->x < r->xmin ) 		nearPt.x = r->xmin;
	else if ( pt->x > r->xmax ) nearPt.x = r->xmax;
	else 						nearPt.x = pt->x;
	
	if ( pt->y < r->ymin ) 		nearPt.y = r->ymin;
	else if ( pt->y > r->ymax ) nearPt.y = r->ymax;
	else 						nearPt.y = pt->y;
	
	return PointDistance(&nearPt, pt);
}

SCOORD RectDistance(P_SRECT a, P_SRECT b)
{
	SPOINT err;

    if ( a->xmin > b->xmax )  		err.x = a->xmin - b->xmax;
	else if ( b->xmin > a->xmax )	err.x = b->xmin - a->xmax;
	else							err.x = 0;

	if ( a->ymin > b->ymax )  		err.y = a->ymin - b->ymax;
	else if ( b->ymin > a->ymax ) 	err.y = b->ymin - a->ymax;
	else							err.y = 0;

	return PointLength(&err);
}


//
// MATRIX Routines
//

void MatrixTransformPoint(P_MATRIX m, P_SPOINT p, P_SPOINT dst)
// Note that we try to optomize the common case of no rotation or slant
{
	SCOORD x, y;
	
	x = FixedMul(m->a, p->x) + m->tx;
	if ( m->c != 0 ) 
		x += FixedMul(m->c, p->y);
		
	y = FixedMul(m->d, p->y) + m->ty;
	if ( m->b != 0 ) 
		y += FixedMul(m->b, p->x);	
		
	dst->x = x;
	dst->y = y;
}

void MatrixDeltaTransformPoint(P_MATRIX m, P_SPOINT p, P_SPOINT dst)
{
	SPOINT result;
	result.x = FixedMul(m->a, p->x);
	if ( m->c != 0 ) 
		result.x += FixedMul(m->c, p->y);
	result.y = FixedMul(m->d, p->y);
	if ( m->b != 0 ) 
		result.y += FixedMul(m->b, p->x);
	*dst = result;
}

void MatrixTransformRect(P_MATRIX m, P_SRECT r, P_SRECT dst)
{
	if ( RectIsEmpty(r) ) {
		RectSetEmpty(dst);
	} else {
	  // Make the destination SRECT the union of the transformed corners
	  //	of the source SRECT
		SPOINT pt;
		SRECT result;
		int i;
		
		RectSetEmpty(&result);
		for ( i = 0; i < 4; i++ ) {
			RectGetPoint(r, i, &pt);
			MatrixTransformPoint(m, &pt, &pt);
			RectUnionPoint(&pt, &result);
		}
		
		*dst = result;
	}
}

SCOORD MatrixTransformThickness(P_MATRIX m, SCOORD t)
{
	SPOINT pt;
	pt.x = pt.y = t;
	MatrixDeltaTransformPoint(m, &pt, &pt);
	SCOORD thick = FixedMul(fixedSqrt2/2, PointLength(&pt));
	if ( t > 0 ) thick = Max(1, thick);
	return thick;
}

void MatrixConcat(P_MATRIX m1, P_MATRIX m2, P_MATRIX dst)
{
	MATRIX result;
	
  // Multiply the common terms
	result.a  = FixedMul(m1->a,  m2->a);
	result.d  = FixedMul(m1->d,  m2->d);
	result.b  = result.c = 0;
	result.tx = FixedMul(m1->tx, m2->a) + m2->tx;
	result.ty = FixedMul(m1->ty, m2->d) + m2->ty;
	
  // Include the less common terms
	if ( m1->b != 0 || m1->c != 0 || m2->b != 0 || m2->c != 0 ) {
		result.a  += FixedMul(m1->b,  m2->c);
		result.d  += FixedMul(m1->c,  m2->b);
		result.b  += FixedMul(m1->a,  m2->b) + FixedMul(m1->b,  m2->d);
		result.c  += FixedMul(m1->c,  m2->a) + FixedMul(m1->d,  m2->c);
		result.tx += FixedMul(m1->ty, m2->c);
		result.ty += FixedMul(m1->tx, m2->b);
	}
	
	*dst = result;
}

inline SFIXED IShift(SFIXED x, int s)
{
	if ( s >= 0 ) {
		return x << s;
	} else {
		return x >> s;
	}
}

void MatrixInvert(P_MATRIX m, P_MATRIX dst)
{
	if ( m->b == 0 && m->c == 0 ) {
	  // Invert a simple matrix
		dst->a  = FixedDiv(fixed_1, m->a);
		dst->d  = FixedDiv(fixed_1, m->d);
		dst->b  = dst->c = 0; 
		dst->tx = -FixedMul(dst->a, m->tx);
		dst->ty = -FixedMul(dst->d, m->ty);
		
	} else {
	  // Invert a general matrix
// 		#ifdef PLAYER
		// We don't want a dependency on floating point so use the less accurate
		//	fixed point invert for the player
		SPOINT pt;

		// Check to normalize the matrix terms
		SFIXED bits = Abs(m->a) | Abs(m->b) | Abs(m->c) | Abs(m->d);
		if ( bits == 0 ) {
			FLASHASSERT(false);
			MatrixIdentity(dst);
			return;
		}
		int nbits = 0;
		while ( (bits & 0xFF000000) == 0 ) {
			bits <<= 6;
			nbits += 6;
		}
		while ( (bits & 0xE0000000) == 0 ) {// allow a sign bit...
			bits <<= 1;
			nbits += 1;
		}

		// bp should be the smallest value possible that will not overflow the 32 bit result
		int bp = 2*(32-nbits)-32;
		SFIXED det;
		if ( bp <= 0 ) {
			bp = 0;
			det = m->a*m->d - m->b*m->c;
		} else {
			det = _FPMul(m->a,m->d,bp) - _FPMul(m->b,m->c,bp);
		}

		if ( det == 0 ) {
			//FLASHASSERT(false);
			//FLASHOUTPUT("Singular matrix in invert.");
			MatrixIdentity(dst);
			return;
		}

		int s = 16-bp;
		dst->a =  FixedDiv(IShift(m->d,s), det);
		dst->b = -FixedDiv(IShift(m->b,s), det);
		dst->c = -FixedDiv(IShift(m->c,s), det);
		dst->d =  FixedDiv(IShift(m->a,s), det);
/*	
		SFIXED det = FixedMul(m->a, m->d) - FixedMul(m->b, m->c);
		if ( det == 0 ) {
			FLASHOUTPUT("Singular matrix in invert.");
			MatrixIdentity(dst);
			return;
		}

		dst->a =  FixedDiv(m->d, det);
		dst->b = -FixedDiv(m->b, det);
		dst->c = -FixedDiv(m->c, det);
		dst->d =  FixedDiv(m->a, det);
		*/
		pt.x = m->tx;
		pt.y = m->ty;
		MatrixDeltaTransformPoint(dst, &pt, &pt);
		dst->tx = -pt.x;
		dst->ty = -pt.y;
// 		#else
// 		double a0, a1, a2, a3, det;
// 		SPOINT pt;
// 	
// 		a0 = FixedToFloat(m->a);
// 		a1 = FixedToFloat(m->b);
// 		a2 = FixedToFloat(m->c);
// 		a3 = FixedToFloat(m->d);
// 	
// 		det = a0*a3 - a1*a2;
// 		if ( det == 0.0 ) {
// 			FLASHOUTPUT("Singular matrix in invert.");
// 			MatrixIdentity(dst);
// 			return;
// 		}
// 		det = 1.0 / det;
// 		
// 		dst->a =  FloatToFixed(a3 * det);
// 		dst->b = -FloatToFixed(a1 * det);
// 		dst->c = -FloatToFixed(a2 * det);
// 		dst->d =  FloatToFixed(a0 * det);
// 		pt.x = m->tx;
// 		pt.y = m->ty;
// 		MatrixDeltaTransformPoint(dst, &pt, &pt);
// 		dst->tx = -pt.x;
// 		dst->ty = -pt.y;
// 		#endif
	}
}

BOOL MatrixEqual(P_MATRIX m1, P_MATRIX m2, SCOORD err)
{
	return 	FixedEqual(m1->a,  m2->a,  err) &&
			FixedEqual(m1->b,  m2->b,  err) &&
			FixedEqual(m1->c,  m2->c,  err) &&
			FixedEqual(m1->d,  m2->d,  err) &&
			FixedEqual(m1->tx, m2->tx, err) &&
			FixedEqual(m1->ty, m2->ty, err);
}

void MatrixIdentity(P_MATRIX m)
{
	m->a = m->d = fixed_1;
	m->b = m->c = 0;
	m->tx = m->ty = 0;
}

void MatrixRotate(SFIXED degrees, P_MATRIX m)
{
	m->a = FixedCos(degrees);
	m->b = FixedSin(degrees);
	m->c = -m->b;
	m->d = m->a;
	m->tx = m->ty = 0;
}

void MatrixTranslate(SCOORD dx, SCOORD dy, P_MATRIX m)
{
	m->a = m->d = fixed_1;
	m->b = m->c = 0;
	m->tx = dx;
	m->ty = dy;
}

void MatrixScale(SFIXED sx, SFIXED sy, P_MATRIX m)
{
	m->a = sx;
	m->d = sy;
	m->b = m->c = 0;
	m->tx = m->ty = 0;
}

void MatrixMap(P_SRECT src, P_SRECT dst, P_MATRIX m)
{
	FLASHASSERT(!RectIsEmpty(src) && !RectIsEmpty(dst));
	//m->a = FixedDiv(FixedMax(RectWidth(dst), minMapSize), FixedMax(RectWidth(src), minMapSize));
	//m->d = FixedDiv(FixedMax(RectHeight(dst), minMapSize), FixedMax(RectHeight(src), minMapSize));
	m->a = FixedDiv(RectWidth(dst), FixedMax(RectWidth(src), 1));
	m->d = FixedDiv(RectHeight(dst), FixedMax(RectHeight(src), 1));
	m->b = m->c = 0;
	m->tx = dst->xmin - FixedMul(src->xmin, m->a);
	m->ty = dst->ymax - FixedMul(src->ymax, m->d);
}

void MatrixMapAspect(P_SRECT src, P_SRECT dst, P_MATRIX m)
// Map rects with a 1:1 aspect ratio
{
	FLASHASSERT(!RectIsEmpty(src) && !RectIsEmpty(dst));
	//m->a = FixedDiv(FixedMax(RectWidth(dst), minMapSize), FixedMax(RectWidth(src), minMapSize));
	//m->d = FixedDiv(FixedMax(RectHeight(dst), minMapSize), FixedMax(RectHeight(src), minMapSize));
	m->a = FixedDiv(RectWidth(dst), FixedMax(RectWidth(src), 1));
	m->d = FixedDiv(RectHeight(dst), FixedMax(RectHeight(src), 1));
	m->a = m->d = FixedMin(m->a, m->d);
	m->b = m->c = 0;
	m->tx = FixedAverage(dst->xmin, dst->xmax) - FixedMul(FixedAverage(src->xmin, src->xmax), m->a);
	m->ty = FixedAverage(dst->ymin, dst->ymax) - FixedMul(FixedAverage(src->ymin, src->ymax), m->d);
}

void MatrixDecompose(MATRIX* mat, SPOINT* scale, SFIXED* rot)
{
	SPOINT vec;
	vec.x = mat->a;
	vec.y = mat->b;
	scale->x = PointLength(&vec);

	vec.x = mat->c;
	vec.y = mat->d;
	scale->y = PointLength(&vec);

	*rot = FixedAtan2(mat->b, mat->a);
}


BOOL MatrixIsScaleOnly(P_MATRIX m)
{
	return FixedEqual(m->b, 0, 0x10) && FixedEqual(m->c, 0, 0x10) && 
  			 m->a > 0 && m->d > 0;
}

BOOL MatrixIsIdentity(P_MATRIX m)
{
	return 	m->tx == 0 		&& m->ty == 0 	   &&
			m->a  == fixed_1 && m->d  == fixed_1 && 
			m->b  == 0 		&& m->c  == 0	   ;
}


