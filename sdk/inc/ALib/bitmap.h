/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	BITMAP.H					(c)	YoY'99						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef							_BITMAP_H_
#define							_BITMAP_H_
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<windows.h>
#include						"types.h"
#include						"node.h"
#include						"rectangle.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//	If you use directly the variables in the class API, it's at your own risks. Prefer the methods...   

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define							guidBITMAP						(Abitmap::CI.guid)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define							bitmapMAXSIZE					8192

//! linea mode
enum
{
								lineaNORMAL,
								lineaADD,
								lineaSUB,
								lineaXOR,
								lineaOLDALPHA
};

#define							lineaSATURATION					bitmapSATURATION


//!	bitmap states
enum
{
								bitmapNORMAL=0,
								bitmapOR,
								bitmapXOR,
								bitmapAND,
								bitmapADD,
								bitmapSUB,
								bitmapMUL,
								bitmapDEFAULT=8
};

//! bitmap flags
#define							bitmapSATURATION				(1<<31)
#define							bitmapGLOBALALPHA				(1<<30)
#define							bitmapALPHA						(1<<29)
#define							bitmapCOLORKEY					(1<<28)
#define							bitmapANTIALIAS					(1<<27)

//! bitmap colors 32
#define							bitmapBLACK						0xff000000
#define							bitmapRED						0xffff0000
#define							bitmapGREEN						0xff00ff00
#define							bitmapBLUE						0xff0000ff
#define							bitmapYELLOW					0xffffff00
#define							bitmapCYAN						0xff00ffff
#define							bitmapMAGENTA					0xffff00ff
#define							bitmapWHITE						0xffffffff
#define							bitmapGRAY						0xff808080
#define							bitmapDARKGRAY					0xff404040

//! bitmap flags for overlay
#define							bltmodeSRC						0
#define							bltmodeXOR						1
#define							bltmodeOR						2
#define							bltmodeAND						3
#define							bltmodeANTIALIAS				4

//! bitmap format for saving 
#define							bitmapBMP						0
#define							bitmapPNG						1
#define							bitmapJPG						2


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! a class memory bitmap that manage all bitmap operation
class Abitmap : public Anode
{
public:
	ADLL static ACI				CI;
	virtual ACI					*getCI							()						{ return &CI; }

	//! the states, (ex:bitmapNORMAL, bitmapOR, ...)
	dword						state;
	//! the flags, (ex: bitmapALPHA, bitmapSATURATION, ...)
	dword						flags;
	//! colorkey value for opacity mode, flags bitmapCOLORKEY must be set to be used
	dword						colorkey;
	//! alpha value, flag bitmapGLOBALALPHA must be set to be used
	float						alpha;

	int							w;
	int							h;
	int							nbBits;
	int							bodysize;

	union
	{
		void					*body;
		byte					*body8;
		word					*body16;
		dword					*body32;
	};
	dword						*palette;

	Arectangle					clip;
	Arectangle					view;

								Abitmap							(const class Abitmap &b);
								Abitmap							(class Abitmap *b);
	ADLL						Abitmap							(int w, int h, int nbBits=32);
	ADLL						Abitmap							(char *filename);
	ADLL						Abitmap							(class Aresobj *o);
	ADLL virtual				~Abitmap						();
	
	void						setState						(Abitmap *b);

	//! resize the bitmap
	ADLL bool					size							(int w, int h);
	//! internal function 
	ADLL bool					set								(HDC hdc, int x=0, int y=0, int mblt=bltmodeSRC);
	ADLL bool					set								(HDC hdc, int x, int y, int w, int h, int mblt=bltmodeSRC);

	//! set the bitmap b in this bitmap at x,y position
	bool						set								(int x, int y, class Abitmap *b, int action=bitmapDEFAULT, int flags=bitmapDEFAULT);
	//! set the bitmap b in this bitmap at xd,yd position from xs, ys position in the source bitmap b to w, h width and height
	ADLL bool					set								(int xd, int yd, int xs, int ys, int w, int h, class Abitmap *b, int action=bitmapDEFAULT, int flags=bitmapDEFAULT);
	//! stretch part of the bitmap source(xs, ys, ws, hs) into part of the destination bitmap(xd, yd, wd, hd)
	ADLL bool					set								(int xd, int yd, int wd, int hd, int xs, int ys, int ws, int hs, class Abitmap *b, int action=bitmapDEFAULT, int flags=bitmapDEFAULT);

	//! set a pixel color c, at x,y position
	bool						pixel							(int x, int y, dword c);
	//! set a pixel with blending operation
	ADLL bool					pixela							(int x, int y, dword c, float alpha, dword mode=lineaNORMAL);
	//! trace a line
	ADLL bool					line							(int x0, int y0, int x1, int y1, dword c);
	//! trace a box
	ADLL bool					box								(int x0, int y0, int x1, int y1, dword c);
	//! fill a box
	ADLL bool					boxf							(int x0, int y0, int x1, int y1, dword c);

	//! trace a line with blending operation
	ADLL bool					linea							(int x0, int y0, int x1, int y1, dword c, float alpha=-1.f, dword mode=lineaNORMAL);
	//! trace a box with blending operation
	ADLL bool					boxa							(int x0, int y0, int x1, int y1, dword c, float alpha=-1.f, dword mode=lineaNORMAL);
	//! fill a box with blending operation
	ADLL bool					boxfa							(int x0, int y0, int x1, int y1, dword c, float alpha=-1.f, dword mode=lineaNORMAL);

	ADLL bool					fillAlpha						(float alpha);

	ADLL bool					flipX							();
	ADLL bool					flipY							();
	//! blur the bitmap
	ADLL bool					blur							(Abitmap *b, float coefx, float coefy);
	//! blur the alpha src channel and copy the src colors
	ADLL bool					ablur							(Abitmap *b, float coefx, float coefy);

	//! fill the entire surface by the stretched input bitmap
	ADLL bool					stretch							(Abitmap *b);

	ADLL static Abitmap *		getDesktop						();
	ADLL bool					getDesktop						(int x, int y);

	//! load the filename file
	/*! load a portable network graphic file (.PNG)*/
	ADLL bool					load							(char *filename);
	ADLL bool					load							(class Afile *f, int type=bitmapJPG);

	//! save the filename file (.png)
	/*! save a portable network graphic file (.PNG)*/
	ADLL bool					save							(char *filename, int quality=100);
	ADLL bool					save							(class Afile *f, int type=bitmapJPG, int quality=100);

	ADLL void					operator=						(const Abitmap& b);

	// private

	dword						*adr;	/// private, y adresse table

private:

	ADLL bool					NCsetMMX						(int xd, int yd, int xs, int ys, int w, int h, class Abitmap *b, int action=bitmapDEFAULT, int flags=bitmapDEFAULT);
	ADLL bool					NCsetMMX						(int xd, int yd, int wd, int hd, int xs, int ys, int ws, int hs, class Abitmap *b, int action=bitmapDEFAULT, int flags=bitmapDEFAULT);
	ADLL bool					NCcreate						();
	ADLL bool					NCfree							();
	ADLL bool					NCcopy							(const Abitmap *b);
	ADLL bool					NCreadPNG						(struct png_struct_def *png_ptr, struct png_info_struct *info_ptr);
	ADLL bool					NCreadJPG						(Afile *file);
	ADLL bool					NCreadBMP						(Afile *file);
	ADLL bool					NCwriteJPG						(Afile *file, int quality);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__inline Abitmap::Abitmap(const Abitmap &b) : Anode()
{
	adr=NULL;
	palette=NULL;
	body=NULL;
	NCcopy(&b);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__inline Abitmap::Abitmap(Abitmap *b) : Anode()
{
	adr=NULL;
	palette=NULL;
	body=NULL;
	NCcopy(b);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__inline void Abitmap::setState(Abitmap *b)
{
	state=b->state;
	flags=b->flags;
	colorkey=b->colorkey;
	alpha=b->alpha;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__inline bool Abitmap::pixel(int x, int y, dword c)
{
	x+=view.x;
	y+=view.y;
	if(clip.contains(x, y))
	{
		switch(nbBits)
		{
			case 8:
			body8[adr[y]+x]=(byte)c;
			return TRUE;

			case 16:
			body16[adr[y]+x]=(word)c;
			return TRUE;

			case 32:
			body32[adr[y]+x]=c;
			return TRUE;
		}
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__inline bool Abitmap::set(int x, int y, class Abitmap *b, int action, int flags)
{
	return set(x, y, 0, 0, b->w, b->h, b, action, flags);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__inline word color32to16(dword color)
{
	return (word)(((color&0x0000ff)>>3)|((color&0x00ff00)>>6)|((color&0xff0000)>>9));
}

__inline dword color16to32(word color)
{
	return (((dword)color&0x001f)<<3)|(((dword)color&0x03e0)<<6)|(((dword)color&0x7c00)<<9);
}

__inline void colorRGB(byte *r, byte *g, byte *b, dword color)
{
	*r=(byte)(color>>16);
	*g=(byte)(color>>8);
	*b=(byte)(color);
}

__inline void colorRGBA(byte *r, byte *g, byte *b, byte *a, dword color)
{
	*a=(byte)(color>>24);
	*r=(byte)(color>>16);
	*g=(byte)(color>>8);
	*b=(byte)(color);
}

__inline void colorRGB(byte *r, byte *g, byte *b, word color)
{
	*r=(byte)((color&0x07c0)>>7);
	*g=(byte)((color&0x03e0)>>2);
	*b=(byte)((color&0x001f)<<3);
}

__inline dword color32(byte r, byte g, byte b)
{
	return 0xff000000|((dword)r<<16)|((dword)g<<8)|(dword)b;
}

__inline dword color32(byte r, byte g, byte b, byte a)
{
	return ((dword)a<<24)|((dword)r<<16)|((dword)g<<8)|(dword)b;
}

__inline word color16(byte r, byte g, byte b)
{
	return (word)((((dword)r&0xf8)<<7)|(((dword)g&0xf8)<<2)|((dword)b>>3));
}

__inline dword colormix32(dword c0, dword c1, float mix)
{
	byte	a0,r0,g0,b0;
	byte	a1,r1,g1,b1;
	float	imix=1.f-mix;
	colorRGBA(&r0, &g0, &b0, &a0, c0);
	colorRGBA(&r1, &g1, &b1, &a1, c1);
	return color32((int)(r0*imix+r1*mix), (int)(g0*imix+g1*mix), (int)(b0*imix+b1*mix), (int)(a0*imix+a1*mix));
}

__inline dword colormix32(dword c0, dword c1)	// 50%+50%
{
	byte	a0,r0,g0,b0;
	byte	a1,r1,g1,b1;
	colorRGBA(&r0, &g0, &b0, &a0, c0);
	colorRGBA(&r1, &g1, &b1, &a1, c1);
	return color32(((int)r0+(int)r1)>>1, ((int)g0+(int)g1)>>1, ((int)b0+(int)b1)>>1, ((int)a0+(int)a1)>>1);
}

__inline int iabs(int x)
{
	int	v=x>>31;
	return (x^v)-v;
}

__inline byte revers8(int v)
{
//	return (byte)(255-(iabs((v&511)-256)));
	if(v<0)
		return -v;
	if(v>255)
		return 511-v;

	return v;
}

__inline dword revers32(int r, int g, int b, int a=255)
{
	return color32(revers8(r), revers8(g), revers8(b), revers8(a));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	HLS conversion
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__inline void rgb2hls(float *h, float *l, float *s, byte r0, byte g0, byte b0)
{
	float	r=(float)r0/255;
	float	g=(float)g0/255;
	float	b=(float)b0/255;
	float	max=maxi(maxi(r, g), b);
	float	min=mini(mini(r, g), b);
	float	d=max-min;
	*l=(max+min)/2;
	if(((d<0)?(-d):d)<0.001f)
	{
		*h=0;
		*s=0;
	}
	else
	{
		if(*l<0.5)
			*s=d/(max+min);
		else
			*s=d/(2-max-min);

		if(max==r)
			*h=(g-b)/d;
		else if(max==g)
			*h=2+(b-r)/d;
		else if(max==b)
			*h=4+(r-g)/d ;

		*h=*h*60;
        if(*h<0)
			*h=*h+360;
	}
}

__inline float hls2rgb_rgb(float p1, float p2, float h)	// call by hls2rgb
{
	if(h>360)
		h-=360;
	else if(h<0)
		h=h+360;
	if(h<60)
		return mini(1.f, maxi(0.f, p1+(p2+p1)*h/60));
	else if(h<180)
		return mini(1.f, maxi(0.f, p2));
	else if(h<=240)
		return mini(1.f, maxi(0.f,p1+(p2-p1)*(240-h)/60));
	return mini(1.f, maxi(0.f, p1));
}
	                     
__inline void hls2rgb(float h, float l, float s, byte *r, byte *g, byte *b)
{
	float	p1,p2;
	if(h<0)
		h+=360;
	else if(h>=360)
		h-=360;
	if(l<0.5)
		p2=l*(1+s);
	else
		p2=l+s-(l*s);
	p1=2*l-p2;
	if(s==0)
	{
		*r=*g=*b=(byte)(l*255.99);
	}
	else
	{
		*r=(byte)(hls2rgb_rgb(p1, p2, h+120)*255.99);
		*g=(byte)(hls2rgb_rgb(p1, p2, h)*255.99);
		*b=(byte)(hls2rgb_rgb(p1, p2, h-120)*255.99);
	}
}


// h = [0,360], s = [0,1], v = [0,1]
//		if s == 0, then h = -1 (undefined)

/*
__inline void rgb2hsv(byte r0, byte g0, byte b0, float *h, float *s, float *v)
{
	float	r=(float)r0/255;
	float	g=(float)g0/255;
	float	b=(float)b0/255;
	float	min, max, delta;
	min = mini(mini(r, g), b);
	max = maxi(maxi(r, g), b);
	*v = max;				// v
	delta = max - min;
	if( max != 0 )
		*s = delta / max;		// s
	else {
		// r = g = b = 0		// s = 0, v is undefined
		*s = 0;
		*h = -1;
		return;
	}
	if( r == max )
		*h = ( g - b ) / delta;		// between yellow & magenta
	else if( g == max )
		*h = 2 + ( b - r ) / delta;	// between cyan & yellow
	else
		*h = 4 + ( r - g ) / delta;	// between magenta & cyan
	*h *= 60;				// degrees
	if( *h < 0 )
		*h += 360;
}

__inline void hsv2rgb(byte *r, byte *g, byte *b, float h, float s, float v)
{
	int i;
	float f, p, q, t;
	if(s==0) 
	{
		// achromatic (grey)
		*r = *g = *b = v;
		return;
	}
	h /= 60;			// sector 0 to 5
	i = floor( h );
	f = h - i;			// factorial part of h
	p = v * ( 1 - s );
	q = v * ( 1 - s * f );
	t = v * ( 1 - s * ( 1 - f ) );
	switch( i ) 
	{
		case 0:
			*r = v;
			*g = t;
			*b = p;
			break;
		case 1:
			*r = q;
			*g = v;
			*b = p;
			break;
		case 2
			*r = p;
			*g = v;
			*b = t;
			break;
		case 3:
			*r = p;
			*g = q;
			*b = v;
			break;
		case 4:
			*r = t;
			*g = p;
			*b = v;
			break;
		default:		// case 5:
			*r = v;
			*g = p;
			*b = q;
			break;
	}
}
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif							//_BITMAP_H_
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
