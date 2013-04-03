/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright © Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 
****************************************************************************/

#include "sstroker.h"

#include "sobject.h"

//
// Thick line stroking code
//

SStroker::SStroker(DisplayList* d, REdge** list)
{
	display = d;
	edgeList = list;
	strokeDepth = 0;
	antialias = false;
	preclip = false;
}

void MakeVertLine(CURVE* c, SCOORD x)
{
	c->anchor1.x = c->control.x = c->anchor2.x = x;
	if ( c->anchor1.y < -32000 ) c->anchor1.y = -32000;
	if ( c->anchor2.y >  32000 ) c->anchor2.y = 32000;
	c->control.y = (c->anchor1.y + c->anchor2.y)/2;
	c->isLine = true;
}

void SStroker::AddCurve(CURVE* src)
{
	CURVE c;
	int dir;
	
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
				if ( ++strokeDepth > 16 ) {
					FLASHASSERT(false);
					return;
				}
				AddCurve(&c1);
				AddCurve(&c2);
				strokeDepth--;
				return;
			}
		}

		// Split any curves that are too long
		if ( c.anchor2.y - c.anchor1.y > 256 ) {
			CURVE c1, c2;
			CurveDivide(src, &c1, &c2);
			if ( ++strokeDepth > 16 ) {
				FLASHASSERT(false);
				return;
			}
			AddCurve(&c1);
			AddCurve(&c2);
			strokeDepth--;
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
			} else if ( strokeDepth < 16 ) {
				// Split the curve
				CURVE c1, c2;
				CurveDivide(src, &c1, &c2);
				strokeDepth++;
				AddCurve(&c1);
				AddCurve(&c2);
				strokeDepth--;
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
	edge->fillRule = fillWindingRule;
	edge->color1 = color;
	edge->color2 = 0;//color2;
	
	// Add to the edge list
	edge->nextObj = *edgeList;
	*edgeList = edge;
}

void SStroker::AddEdge(SPOINT* pt1, SPOINT* pt2)
{
	CURVE c;
	CurveSetLine(pt1, pt2, &c);
	AddCurve(&c);
}

void SStroker::StrokeJoin(P_SPOINT pt1, P_SPOINT pt2, P_SPOINT origin)
{
	SCOORD d = PointFastDistance(pt1, pt2);
	if ( d > 3 ) {
		// Do a round join
		S32 perpAng1 = PointAngle(pt1, origin);
		S32 perpAng2 = PointAngle(pt2, origin);
		while ( perpAng1 < perpAng2 ) perpAng1 += FC(360);
		
		S32 deltaAngle = perpAng1 - perpAng2;
		if ( FC(1) < deltaAngle && deltaAngle <= FC(180) ) {
			int n = (deltaAngle + FC(44))/FC(45);
			if ( n < 1 ) n = 1;
			SCOORD stepAngle = -deltaAngle/n;
			SCOORD controlLen = FixedDiv(lineThickness/2, FixedCos(stepAngle>>1));
			
			// Calculate the control points
			SPOINT controls[8];
			SCOORD angle = perpAng1+(stepAngle>>1);
			int i;
			for ( i = 0; i < n; i++, angle += stepAngle ) 
				PointPolar(controlLen, angle, origin, controls+i);

			// Add the curve segments
			CURVE c;
			c.isLine = false;
			c.anchor2 = *pt1;
			for ( i = 0; i < n; i++ ) {
				c.anchor1 = c.anchor2;
				c.control = controls[i];

				if ( i == n-1 )
					c.anchor2 = *pt2;
				else
					PointAverage(&c.control, &controls[i+1], &c.anchor2);

				AddCurve(&c);
			}
		} else {
			// The join is flat or inside the curve...
			AddEdge(pt1, pt2);
		}
	
	} else {
		// Just do a flat line join
		AddEdge(pt1, pt2);
	}
}

void SStroker::StrokeThickCurve(P_CURVE c)
{
	// Check to subdivide the curve...
	if ( CurveIsExtreme(c) && strokeDepth < 5 ) {
		// Subdivide the curve
		CURVE a, b;
		CurveDivide(c, &a, &b);
		strokeDepth++;
		StrokeThickCurve(&a);
		StrokeThickCurve(&b);
		strokeDepth--;
		return;
	}

	SPOINT perp1, perp2;
	SPOINT l1, l2, r1, r2;
	
	// Handle a 4 pixel or thicker line
	S32 t = lineThickness/2;
    
	// Calculate the perpendicular at anchor1
	perp1.x = c->control.y - c->anchor1.y;
	perp1.y = c->anchor1.x - c->control.x;
	if ( perp1.x == 0 && perp1.y == 0 ) {
		perp1.x = c->anchor2.y - c->anchor1.y;
		perp1.y = c->anchor1.x - c->anchor2.x;
	}
 	PointNormalize(&perp1, t);

	if ( c->isLine ) {
		// The perpendiculars are the same for a line
		perp2 = perp1;
	} else {
		// Calculate the perpendicular at anchor2
		perp2.x = c->anchor2.y - c->control.y;
		perp2.y = c->control.x - c->anchor2.x;
		if ( perp2.x == 0 && perp2.y == 0 ) {
			perp2.x = c->anchor2.y - c->anchor1.y;
			perp2.y = c->anchor1.x - c->anchor2.x;
		}
		PointNormalize(&perp2, t);
	}				
	
	// Offset the anchor points
	PointAdd(&c->anchor1, &perp1, &l1);
	PointAdd(&c->anchor2, &perp2, &l2);
			
	PointSub(&c->anchor1, &perp1, &r1);
	PointSub(&c->anchor2, &perp2, &r2);

	if ( c->isLine ) {
		// Handle a straight line
		AddEdge(&l2, &l1);
		AddEdge(&r1, &r2);
				
	} else {
		// Handle a curved line
		CURVE c1, c2;
				
		CurveAdjust(c, &l1, &l2, &c1);
		CurveReverse(&c1, &c1);
		AddCurve(&c1);
		
		CurveAdjust(c, &r1, &r2, &c2);
		AddCurve(&c2);
	}
		
	if ( !strokeInited ) {
		// Set up the start
		lStartPt = l1;
		startOrigin = c->anchor1;
		rStartPt = r1;
		strokeInited = true;
				
	} else {
		// Add the joins
		StrokeJoin(&l1, &lCurPt, &curOrigin);
		StrokeJoin(&rCurPt, &r1, &curOrigin);
	}
			
	// Track the joins
	lCurPt = l2;
	curOrigin = c->anchor2;
	rCurPt = r2;
}


//
// Thin line stroking code
//

void SStroker::StrokeThinLine(CURVE* c)
{
	SPOINT perp;
	SPOINT l1, l2, r1, r2;
		
	// Calculate the perpendicular at anchor1
	perp.x = c->anchor2.y - c->anchor1.y;
	perp.y = c->anchor1.x - c->anchor2.x;
		
	l1 = r1 = c->anchor1;
	l2 = r2 = c->anchor2;
		
	BOOL mostlyV = Abs(perp.x) > Abs(perp.y);
	switch ( lineThickness ) {
		case 1:
			if ( mostlyV ) { // mostly vertical line
				int d = Sign(perp.x);
				if ( d > 0 ) {
					l1.x += d;	l2.x += d;
				} else {
					r1.x -= d;	r2.x -= d;
				}
			} else {
				int d = Sign(perp.y);
				if ( d > 0 ) {
					l1.y += d;	l2.y += d;
				} else {
					r1.y -= d;	r2.y -= d;
				}
			}
			break;
			
		case 2: 
			if ( mostlyV ) { // mostly vertical line
				int d = Sign(perp.x);
				l1.x += d;	l2.x += d;
				r1.x -= d;	r2.x -= d;
			} else {
				int d = Sign(perp.y);
				l1.y += d;	l2.y += d;
				r1.y -= d;	r2.y -= d;
			}
			break;
				
		case 3: 
			if ( mostlyV ) { // mostly vertical line
				int d = Sign(perp.x);
				l1.x += d;	l2.x += d;
				d *= 2;
				r1.x -= d;	r2.x -= d;
			} else {
				int d = Sign(perp.y);
				l1.y += d;	l2.y += d;
				d *= 2;
				r1.y -= d;	r2.y -= d;
			}
			break;
	}

	AddEdge(&l2, &l1);
	AddEdge(&r1, &r2);

	if ( !strokeInited ) {
		// Set up the start
		lStartPt = l1;
		startOrigin = c->anchor1;
		rStartPt = r1;
		strokeInited = true;
				
	} else {
		// Add the joins
		AddEdge(&l1, &lCurPt);
		AddEdge(&rCurPt, &r1);
	}
	
	// Track the joins
	lCurPt = l2;
	curOrigin = c->anchor2;
	rCurPt = r2;
}

inline void CurveOffsetX(P_CURVE c, S32 d)
{
	c->anchor1.x += d;
	c->control.x += d;
	c->anchor2.x += d;
}

inline void CurveOffsetY(P_CURVE c, S32 d)
{
	c->anchor1.y += d;
	c->control.y += d;
	c->anchor2.y += d;
}

inline BOOL SameSign(SCOORD a, SCOORD b)
{
	if ( a == 0 )
		return true;
 	else if ( a > 0 )
 		return b >= 0;
 	else
 		return b <= 0;
}

void SStroker::StrokeThinCurve(P_CURVE c)
{
	SPOINT perp;
	CURVE cl, cr;

    if ( CurveFlatness(c) > 2  ) {
	    SPOINT perp1, perp2;
	    BOOL mostlyV1, mostlyV2;
	    
		perp1.x = c->control.y - c->anchor1.y;
		perp1.y = c->anchor1.x - c->control.x;
	    
		perp2.x = c->anchor2.y - c->control.y;
		perp2.y = c->control.x - c->anchor2.x;
		
		mostlyV1 = Abs(perp1.x) > Abs(perp1.y);
		mostlyV2 = Abs(perp2.x) > Abs(perp2.y);
		
		if ( mostlyV1 != mostlyV2 || !SameSign(perp1.x, perp2.x) || !SameSign(perp1.y, perp2.y) ) {
			// Subdivide the curve
			CURVE a, b;
			CurveDivide(c, &a, &b);
			StrokeThinCurve(&a);
			StrokeThinCurve(&b);
			return;
		}
	}
	
	// Calculate the average perpendicular
	perp.x = c->anchor2.y - c->anchor1.y;
	perp.y = c->anchor1.x - c->anchor2.x;

	cl = *c;
	cr = *c;
		
	BOOL mostlyV = Abs(perp.x) > Abs(perp.y);
	switch ( lineThickness ) {
		case 1:
			if ( mostlyV ) { // mostly vertical line
				int d = Sign(perp.x);
				if ( d > 0 ) {
					CurveOffsetX(&cl, d);
				} else {
					CurveOffsetX(&cr, -d);
				}
			} else {
				int d = Sign(perp.y);
				if ( d > 0 ) {
					CurveOffsetY(&cl, d);
				} else {
					CurveOffsetY(&cr, -d);
				}
			}
			break;
			
		case 2: 
			if ( mostlyV ) { // mostly vertical line
				int d = Sign(perp.x);
				CurveOffsetX(&cl, d);
				CurveOffsetX(&cr, -d);
			} else {
				int d = Sign(perp.y);
				CurveOffsetY(&cl, d);
				CurveOffsetY(&cr, -d);
			}
			break;
				
		case 3: 
			if ( mostlyV ) { // mostly vertical line
				int d = Sign(perp.x);
				CurveOffsetX(&cl, d);
				CurveOffsetX(&cr, -2*d);
			} else {
				int d = Sign(perp.y);
				CurveOffsetY(&cl, d);
				CurveOffsetY(&cr, -2*d);
			}
			break;
	}

	CURVE tmp;
	CurveReverse(&cl, &tmp);	// reverse the left curve
	AddCurve(&tmp);
	AddCurve(&cr);

	if ( !strokeInited ) {
		// Set up the start
		lStartPt = cl.anchor1;
		startOrigin = c->anchor1;
		rStartPt = cr.anchor1;
		strokeInited = true;
				
	} else {
		// Add the joins
		AddEdge(&cl.anchor1, &lCurPt);
		AddEdge(&rCurPt, &cr.anchor1);
	}
	
	// Track the joins
	lCurPt = cl.anchor2;
	curOrigin = c->anchor2;
	rCurPt = cr.anchor2;
}


//
// Public Stroking Methods
//

void SStroker::BeginStroke(S32 thickness, RColor* c)
{
	// Set up for the stroke
	strokeInited = false;

	// Set up the context
	lineThickness = Max(antialias ? 4 : 1, thickness);
 	isThick = lineThickness > 3;
	color = c;
	curPt.x = curPt.y = 0x80000000L;
}

void SStroker::AddStrokeCurve(CURVE* c)
{
	if ( c->anchor1.x == c->anchor2.x && c->anchor1.y == c->anchor2.y &&
		 c->anchor1.x == c->control.x && c->anchor1.y == c->control.y ) return;	// ignore a zero length curve

	if ( !isThick ) {
		if ( c->isLine )
			StrokeThinLine(c);
		else
			StrokeThinCurve(c);
	} else {
		// Check to grid the line
		if ( antialias && c->isLine ) {
			CURVE c2 = *c;
			if ( lineThickness == 4 || lineThickness == 12 ) {
				if ( c2.anchor1.x == c2.anchor2.x && Abs(c2.anchor1.y - c2.anchor2.y) > 4*3 ) {
					// Snap a vertical line to the pixel grid, note that we don't need to close the gaps since the
					//	lines to close would be horizontal and have no effect
					c2.anchor1.x = c2.anchor2.x = (c2.anchor1.x & ~3) + 2;

				} else if ( c2.anchor1.y == c2.anchor2.y && Abs(c2.anchor1.x - c2.anchor2.x) > 4*3 ) {
					// Snap a horizontal line to the pixel grid
					c2.anchor1.y = c2.anchor2.y = (c2.anchor1.y & ~3) + 2;
				}
			} else if ( lineThickness == 8 ) {
				if ( c2.anchor1.x == c2.anchor2.x && Abs(c2.anchor1.y - c2.anchor2.y) > 4*3 ) {
					// Snap a vertical line to the pixel grid, note that we don't need to close the gaps since the
					//	lines to close would be horizontal and have no effect
					c2.anchor1.x = c2.anchor2.x = (c2.anchor1.x + 2) & ~3;

				} else if ( c2.anchor1.y == c2.anchor2.y && Abs(c2.anchor1.x - c2.anchor2.x) > 4*3 ) {
					// Snap a horizontal line to the pixel grid
					c2.anchor1.y = c2.anchor2.y = (c2.anchor1.y + 2) & ~3;
				}
			}
			StrokeThickCurve(&c2);
		} else {
			StrokeThickCurve(c);
		}
	}
	
	curPt = c->anchor2;
}

void SStroker::EndStroke()
{
	if ( strokeInited ) {
		if ( PointEqual(&startOrigin, &curOrigin, 0) ) {
			// Handle a closed run
				
			// Add the final joins
			if ( !isThick ) {
				AddEdge(&lStartPt, &lCurPt);
				AddEdge(&rCurPt, &rStartPt);
			} else {
				StrokeJoin(&lStartPt, &lCurPt, &curOrigin);
				StrokeJoin(&rCurPt, &rStartPt, &curOrigin);
			}
					
		} else {
			// Handle an open run 
					
			// Add the caps
			if ( !isThick ) {
				AddEdge(&lStartPt, &rStartPt);
				AddEdge(&rCurPt, &lCurPt);
			} else {
				StrokeJoin(&lStartPt, &rStartPt, &startOrigin);
				StrokeJoin(&rCurPt, &lCurPt, &curOrigin);
			}
		}
		
	} else if ( curPt.x != 0x80000000L ) {
	 	// Draw a dot
		SPOINT pt1, pt2;
		S32 t = lineThickness/2;
		pt1 = pt2 = curPt;
		pt1.y -= t;
		pt2.y += lineThickness-t;

		if ( !isThick ) {
			// Handle a small dot
			pt1.x -= t;
			pt2.x -= t;
			AddEdge(&pt1, &pt2);
		
			pt1.x += lineThickness-t;
			pt2.x += lineThickness-t;
			AddEdge(&pt2, &pt1);

		} else {
			// Handle a large dot
			StrokeJoin(&pt1, &pt2, &curPt);
			StrokeJoin(&pt2, &pt1, &curPt);
		}
	}
}

