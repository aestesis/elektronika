/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright © Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 
****************************************************************************/
//	990325	mnk	"fixed1" -> "fixed_1"

#include "geom.h"
#include "sbitmap.h"
#include "raster.h"
#include "morphinter.h"

//*********************************************************************
// Function -
//	 Interpolate
//*********************************************************************
void Interpolate(SRECT* rect1, SRECT *rect2, SFIXED ratio, SRECT *rect)
{
	rect->xmin = Interpolate(rect1->xmin, rect2->xmin, ratio);
	rect->xmax = Interpolate(rect1->xmax, rect2->xmax, ratio);
	rect->ymin = Interpolate(rect1->ymin, rect2->ymin, ratio);
	rect->ymax = Interpolate(rect1->ymax, rect2->ymax, ratio);
	return;
}

//*********************************************************************
// Function -
//	 Interpolate
//
// Description -
//	Determines the current tweened information from the begining matrix
//	transformation to the ending transformation
//*********************************************************************
void Interpolate(P_MATRIX matStart, P_MATRIX matEnd, SFIXED ratio, P_MATRIX pDestMatrix)
{
	pDestMatrix->a = Interpolate(matStart->a, matEnd->a, ratio);
	pDestMatrix->b = Interpolate(matStart->b, matEnd->b, ratio);
	pDestMatrix->c = Interpolate(matStart->c, matEnd->c, ratio);
	pDestMatrix->d = Interpolate(matStart->d, matEnd->d, ratio);
	pDestMatrix->tx = Interpolate(matStart->tx, matEnd->tx, ratio);
	pDestMatrix->ty = Interpolate(matStart->ty, matEnd->ty, ratio);
	return;
}

//*********************************************************************
// Interpolate
//*********************************************************************
void Interpolate(RGBI *rgb1, RGBI *rgb2, SFIXED ratio, RGBI *rgb)
{
	rgb->alpha = (U16)Interpolate(rgb1->alpha, rgb2->alpha, ratio);
	rgb->red = (U16)Interpolate(rgb1->red,   rgb2->red,   ratio);
	rgb->green = (U16)Interpolate(rgb1->green, rgb2->green, ratio);
	rgb->blue = (U16)Interpolate(rgb1->blue,  rgb2->blue,  ratio);
	return;
}

//*********************************************************************
// Interpolate
//*********************************************************************
void Interpolate(SRGB *color1, SRGB *color2, SFIXED ratio, SRGB *color)
{
	if ( ratio <= 0 )
	{
		*color = *color1;
	}
	else if (ratio >= fixed_1)
	{
		*color = *color2;
	}
	else
	{
		color->rgb.transparency = (U8)Interpolate(color1->rgb.transparency, color2->rgb.transparency, ratio);
		color->rgb.red			= (U8)Interpolate(color1->rgb.red,   color2->rgb.red,   ratio);
		color->rgb.green		= (U8)Interpolate(color1->rgb.green, color2->rgb.green, ratio);
		color->rgb.blue			= (U8)Interpolate(color1->rgb.blue,  color2->rgb.blue,  ratio);
	}
	return;
}

