/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"export.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SURFACE.CPP					(c)	YoY'99						WEB: search aestesis
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<assert.h>
#include						"surface.h"
#include						"window.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
static Aproperties				properties[]=	
							{	
								{	guidBITMAP,	(int)&(((Asurface *)NULL)->bitmap),		"bitmap",		"surface bitmap"	}
							};
*/
ADLL ACI						Asurface::CI=ACI("Asurface", GUID(0xAE57E515,0x00000005), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Asurface::Asurface(char *name, class Aobject *L, int x, int y, int w, int h) : Aobject(name, L, x, y, w, h)
{
	bitmap=new Abitmap(w, h);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Asurface::~Asurface()
{
	delete(bitmap);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void Asurface::paint(Abitmap *b)
{
	b->set(0, 0, bitmap, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Asurface::size(int w, int h)
{
	bool	b=FALSE;
	b=bitmap->size(w, h);
	assert((w==bitmap->w)&&(h==bitmap->h));
	Aobject::size(w, h);
	return b;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
