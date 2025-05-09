/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"elektroexp.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SLIDER.CPP					(c)	YoY'01						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<stdio.h>
#include						<math.h>
#include						"slider.h"
#include						"interface.h"
#include						"../alib/resource.h"
#include						"effect.h"
#include						"resource.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL ACI						Aslider::CI=ACI("Apaddle", GUID(0xE4EC7600,0x00010080), &AcontrolObj::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL Aslider::Aslider(QIID qiid, char *name, Aobject *L, int x, int y, int w, int h, int mode, Aresobj *o) : AcontrolObj(name, L, x, y, w, h)
{
	control=new Acontrol(qiid, name, Aeffect::getEffect(L), this, Acontrol::CONTROLER_01);
	stick=new Abitmap(o);
	this->mode=mode;
	switch(mode)
	{
		case sliderX:
		dp=stick->w/2;
		break;
		case sliderY:
		dp=stick->h/2;
		break;
	}
	test=FALSE;
	ctrlPaint=false;
	set(0.f);
	timer(CTRLTIMEREPAINT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


EDLL Aslider::Aslider(QIID qiid, char *name, Aobject *L, int x, int y, int w, int h, int mode) : AcontrolObj(name, L, x, y, w, h)
{
	control=new Acontrol(qiid, name, Aeffect::getEffect(L), this, Acontrol::CONTROLER_01);
	stick=null;
	this->mode=mode;
	switch(mode)
	{
		case sliderX:
		dp=h>>3;
		break;
		case sliderY:
		dp=w>>3;
		break;
	}
	test=FALSE;
	ctrlPaint=false;
	set(0.f);
	timer(CTRLTIMEREPAINT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL Aslider::~Aslider()
{
	if(stick)
		delete(stick);
	delete(control);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Aslider::mouse(int x, int y, int state, int event)
{
	switch(event)
	{
		case mouseLDOWN:
		control->select();
		mouseCapture(true);
		test=true;

		case mouseNORMAL:
		if(test)
		{
			switch(mode)
			{
				case sliderX:
				{
					int		w=pos.w-dp*2;
					float	v=(float)maxi(mini(x-dp, w), 0)/(float)w;
					set(v);
					repaint();
				}
				break;

				case sliderY:
				{
					int		h=pos.h-dp*2;
					float	v=(float)maxi(mini(y-dp, h), 0)/(float)h;
					set(v);
					repaint();
				}
				break;
			}
		}
		cursor(cursorHANDSEL);
		return TRUE;

		case mouseLUP:
		mouseCapture(false);
		test=false;
		return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Aslider::set(float v)
{
	v=mini(maxi(0.f, v), 1.f);
	control->set(Acontrol::CONTROLER_01, v);
	value=v;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL float Aslider::get()
{
	return value;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Aslider::sequence(int nctlr, float value)
{
	this->value=value;
	ctrlPaint=true;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Aslider::pulse()
{
	if(ctrlPaint)
	{
		repaint();
		ctrlPaint=false;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Aslider::paint(Abitmap *b)
{
	switch(mode)
	{
		case sliderX:
		if(stick)
		{
			int	y=(pos.h-stick->h)>>1;
			int	x=(int)((pos.w-stick->w-1)*value);
			b->set(x, y, stick);
		}
		else
		{
			int d=pos.h>>1;
			int	w=pos.h>>2;
			int	p=(int)((pos.w-w-1)*value);
			b->linea(0, d, pos.w, d, 0xff000000, 0.5f);
			b->linea(0, d+1, pos.w, d+1, 0xffffffff, 0.5f);
			b->boxf(p, 0, p+w, pos.h, 0xffc0c0c0);
		}
		break;

		case sliderY:
		if(stick)
		{
			int	x=(pos.w-stick->w)>>1;
			int	y=(int)((pos.h-stick->h-1)*value);
			b->set(x, y, stick);
		}
		else
		{
			int d=pos.w>>1;
			int	h=pos.w>>2;
			int	p=(int)((pos.h-h-1)*value);
			b->linea(d, 0, d, pos.h, 0xff000000, 0.5f);
			b->linea(d+1, 0, d+1, pos.h, 0xffffffff, 0.5f);
			b->boxf(0, p, pos.w, p+h, 0xffc0c0c0);
		}
		break;
	}
	controlPaint(b, pos.w, pos.h);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
