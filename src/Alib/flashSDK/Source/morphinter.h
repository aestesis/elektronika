/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright © Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 
****************************************************************************/

#ifndef MORPHINTER_INCLUDED
#define MORPHINTER_INCLUDED

// Methods used for calculating the tween shapes
void Interpolate(SRECT* rect1, SRECT *rect2, SFIXED ratio, SRECT *rect);
void Interpolate(P_MATRIX pStart, P_MATRIX pEnd, SFIXED ratio, P_MATRIX pDestMatrix);
void Interpolate(RGBI *rgb1, RGBI *rgb2, SFIXED ratio, RGBI *rgb);
void Interpolate(SRGB *color1, SRGB *color2, SFIXED ratio, SRGB *color);

inline S32 Interpolate(S32 value1, S32 value2, SFIXED ratio)
{
	return value1 + FixedMul(ratio, value2 - value1);
}

inline void Interpolate(SPOINT *pptBegin, SPOINT *pptEnd, SFIXED ratio, SPOINT *pptAverage)
{
	pptAverage->x = pptBegin->x + FixedMul(ratio, pptEnd->x - pptBegin->x);
	pptAverage->y = pptBegin->y + FixedMul(ratio, pptEnd->y - pptBegin->y);
}


// #ifndef PLAYER
// 
// void Interpolate(S32 *value1, S32 *value2, SFIXED ratio, S32 *value);
// void Interpolate(U16 *value1, U16 *value2, SFIXED ratio, U16 *value);
// void Interpolate(U8 *color1, U8 *color2, SFIXED ratio, U8 *color);
// 
// void Interpolate(P_SHAPE_LINE line1, P_SHAPE_LINE line2, SFIXED ratio, P_SHAPE_LINE pLine);
// void Interpolate(P_SHAPE_FILL fill1, P_SHAPE_FILL fill2, SFIXED ratio, P_SHAPE_FILL pFill);
// #endif


#endif
