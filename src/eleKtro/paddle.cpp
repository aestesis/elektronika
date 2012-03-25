/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"elektroexp.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	PADDLE.CPP					(c)	YoY'01						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<stdio.h>
#include						<math.h>
#include						<assert.h>

#include						"paddle.h"
#include						"interface.h"
#include						"../alib/resource.h"
#include						"effect.h"
#include						"resource.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL ACI						Apaddle::CI=ACI("Apaddle", GUID(0xE4EC7600,0x00010050), &AcontrolObj::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL Apaddle::Apaddle(QIID qiid, char *name, Aobject *L, int x, int y, int w, int h, Aresobj *o, int ow, int oh, int mode) : AcontrolObj(name, L, x, y, w, h)
{
	control=new Acontrol(qiid, name, Aeffect::getEffect(L), this, (mode==paddleXY)?Acontrol::CONTROLER_02:Acontrol::CONTROLER_01);
	this->mode=mode;
	bmp=new Abitmap(o);
	xx=0.5f;
	yy=0.5f;
	set(0.f, 0.f);
	defxx=-1.f;
	defyy=-1.f;
	test=FALSE;
	this->ow=ow;
	this->oh=oh;
	this->type=paddleNOTDEFINED;
	ctrlPaint=false;
	timer(CTRLTIMEREPAINT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL Apaddle::Apaddle(QIID qiid, char *name, Aobject *L, int x, int y, int w, int h, Abitmap *b, int ow, int oh, int mode) : AcontrolObj(name, L, x, y, w, h)
{
	control=new Acontrol(qiid, name, Aeffect::getEffect(L), this, (mode==paddleXY)?Acontrol::CONTROLER_02:Acontrol::CONTROLER_01);
	this->mode=mode;
	bmp=b;
	xx=0.5f;
	yy=0.5f;
	set(0.f, 0.f);
	defxx=-1.f;
	defyy=-1.f;
	test=FALSE;
	this->ow=ow;
	this->oh=oh;
	this->type=paddleNOTDEFINED;
	ctrlPaint=false;
	timer(CTRLTIMEREPAINT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL Apaddle::Apaddle(QIID qiid, char *name, Aobject *L, int x, int y, int type) : AcontrolObj(name, L, x, y, 8, 8)
{
	control=new Acontrol(qiid, name, Aeffect::getEffect(L), this, Acontrol::CONTROLER_01);
	this->type=type;
	switch(type)
	{
		case paddleBUTTON01:
		{
			Aresobj	o=alibres.get(MAKEINTRESOURCE(PNG_PADDLE001), "PNG");
			bmp=new Abitmap(&o);
			ow=32;
			oh=32;
			size(32, 32);
			mode=paddleY;
		}
		break;

		case paddleBUTTON02:
		{
			Aresobj	o=alibres.get(MAKEINTRESOURCE(PNG_PADDLE002), "PNG");
			bmp=new Abitmap(&o);
			ow=32;
			oh=32;
			size(32, 32);
			mode=paddleY;
		}
		break;

		case paddleBUTTON03:
		{
			Aresobj	o=alibres.get(MAKEINTRESOURCE(PNG_PADDLE003), "PNG");
			bmp=new Abitmap(&o);
			ow=64;
			oh=64;
			size(64, 64);
			mode=paddleY;
		}
		break;

		case paddleBUTTON04:
		{
			Aresobj	o=alibres.get(MAKEINTRESOURCE(PNG_PADDLE004), "PNG");
			bmp=new Abitmap(&o);
			ow=32;
			oh=32;
			size(32, 32);
			mode=paddleY;
		}
		break;

		case paddleBUTTON05:
		{
			Aresobj	o=alibres.get(MAKEINTRESOURCE(PNG_PADDLE005), "PNG");
			bmp=new Abitmap(&o);
			ow=32;
			oh=32;
			size(32, 32);
			mode=paddleY;
		}
		break;

		case paddleBUTTON06:
		{
			Aresobj	o=alibres.get(MAKEINTRESOURCE(PNG_PADDLE006), "PNG");
			bmp=new Abitmap(&o);
			ow=32;
			oh=32;
			size(32, 32);
			mode=paddleY;
		}
		break;

		case paddleBUTTON07:
		{
			Aresobj	o=alibres.get(MAKEINTRESOURCE(PNG_PADDLE007), "PNG");
			bmp=new Abitmap(&o);
			ow=16;
			oh=32;
			size(ow, oh);
			mode=paddleY;
		}
		break;

		case paddleELEK32:
		{
			Aresobj	o=resdll.get(MAKEINTRESOURCE(IDR_PNG1), "PNG");
			bmp=new Abitmap(&o);
			ow=32;
			oh=32;
			size(ow, oh);
			mode=paddleY;
		}
		break;

		case paddleELEK24:
		{
			Aresobj	o=resdll.get(MAKEINTRESOURCE(IDR_PNG2), "PNG");
			bmp=new Abitmap(&o);
			ow=24;
			oh=24;
			size(24, 24);
			mode=paddleY;
		}
		break;

		case paddleELEK16:
		{
			Aresobj	o=resdll.get(MAKEINTRESOURCE(IDR_PNG3), "PNG");
			bmp=new Abitmap(&o);
			ow=16;
			oh=16;
			size(16, 16);
			mode=paddleY;
		}
		break;

		case paddleELEK12:
		{
			Aresobj	o=resdll.get(MAKEINTRESOURCE(IDR_PNG4), "PNG");
			bmp=new Abitmap(&o);
			ow=12;
			oh=12;
			size(12, 12);
			mode=paddleY;
		}
		break;
		
		case paddleYOLI16:
		{
			Aresobj	o=resdll.get(MAKEINTRESOURCE(PNG_YOLI16), "PNG");
			bmp=new Abitmap(&o);
			ow=16;
			oh=16;
			size(ow, oh);
			mode=paddleY;
		}
		break;

		case paddleYOLI24:
		{
			Aresobj	o=resdll.get(MAKEINTRESOURCE(PNG_YOLI24), "PNG");
			bmp=new Abitmap(&o);
			ow=24;
			oh=24;
			size(ow, oh);
			mode=paddleY;
		}
		break;

		case paddleYOLI32:
		{
			Aresobj	o=resdll.get(MAKEINTRESOURCE(PNG_YOLI32), "PNG");
			bmp=new Abitmap(&o);
			ow=32;
			oh=32;
			size(ow, oh);
			mode=paddleY;
		}
		break;

		case paddleYOLI64:
		{
			Aresobj	o=resdll.get(MAKEINTRESOURCE(PNG_YOLI64), "PNG");
			bmp=new Abitmap(&o);
			ow=64;
			oh=64;
			size(ow, oh);
			mode=paddleY;
		}
		break;

		default:
		bmp=NULL;
		break;
	}
	set(0.f, 0.f);
	defxx=-1.f;
	defyy=-1.f;
	test=FALSE;
	ctrlPaint=false;
	timer(CTRLTIMEREPAINT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL Apaddle::Apaddle(QIID qiid, char *name, Aobject *L, int x, int y, int size, dword color) : AcontrolObj(name, L, x-size, y-size, (size<<1)+1, (size<<1)+1)
{
	control=new Acontrol(qiid, name, Aeffect::getEffect(L), this, Acontrol::CONTROLER_01);
	bmp=NULL;
	lineColor=color;
	type=paddleLINE;
	mode=paddleY;
	xx=0.5f;
	yy=0.5f;
	set(0.f, 0.f);
	defxx=-1.f;
	defyy=-1.f;
	test=false;
	ctrlPaint=false;
	timer(CTRLTIMEREPAINT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL Apaddle::~Apaddle()
{
	delete(control);
	if(bmp)
		delete(bmp);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Apaddle::keyboard(int event, int ascii, int scan, int state)
{
	switch(event)
	{
		case keyboardDOWN:
		if(state&keyboardALT)
		{
			if((defxx!=-1)&&(defyy!=-1))
				set(defxx, defyy);
			else if(defyy!=-1)
				set(0.f, defyy);
			else if(defxx!=-1)
				set(defxx, 0.f);
			else
				set(0.f, 0.f);
			father->notify(this, nyCHANGE);
			return true;
		}
		break;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Apaddle::mouse(int x, int y, int state, int event)
{
	switch(event)
	{
		case mouseLDOWN:
		control->select();
		case mouseNORMAL:
		case mouseLUP:
		cursor(cursorHANDSEL);
		focus(this);
		{
			if(getKeyboardState()&keyboardLALT)
			{
				if((defxx!=-1)&&(defyy!=-1))
					set(defxx, defyy);
				else if(defyy!=-1)
					set(0.f, defyy);
				else if(defxx!=-1)
					set(defxx, 0.f);
				else
					set(0.f, 0.f);
				father->notify(this, nyCHANGE);
				return true;
			}
		}
		switch(event)
		{
			case mouseLDOWN:
			mouseCapture(true);
			test=TRUE;
			x0=x;
			y0=y;
			xx0=xx;
			yy0=yy;
			break;
		}
		if(test&&(state&mouseL))
		{
			switch(mode)
			{
				case paddleX:
				{
					float	dy=0.05f*alib.mouseSensibility*(float)(y-y0);
					xx=mini(maxi(-dy+xx0+alib.mouseSensibility*(float)(x-x0), 0.f), 1.f);
					control->set(Acontrol::CONTROLER_01, xx);
					{
						char	str[256];
						sprintf(str, "%s [%1.2f]", tooltips?tooltips:"", xx);
						notify(this, nyTOOLTIPS, (dword)str);
					}
				}
				break;

				case paddleY:
				{
					int	dxi=x-x0;
					int	dyi=y-y0;
					if(abs(dxi)>=abs(dyi))
						dyi=0;
					else
						dxi=0;
					float	dx=0.05f*alib.mouseSensibility*(float)(dxi);
					float	dy=alib.mouseSensibility*(float)(dyi);
					yy=mini(maxi(yy-dy+dx, 0.f), 1.f);
					control->set(Acontrol::CONTROLER_01, yy);
					{
						char	str[256];
						sprintf(str, "%s [%1.2f]", tooltips?tooltips:"", yy);
						notify(this, nyTOOLTIPS, (dword)str);
					}
					x0=x;
					y0=y;
				}
				break;

				case paddleXY:
				xx=mini(maxi(xx0+alib.mouseSensibility*(float)(x-x0), 0.f), 1.f);
				yy=mini(maxi(yy0-alib.mouseSensibility*(float)(y-y0), 0.f), 1.f);
				control->set(Acontrol::CONTROLER_01, xx);
				control->set(Acontrol::CONTROLER_02, yy);
				{
					char	str[256];
					sprintf(str, "%s [%1.2f,%f1.2]", tooltips?tooltips:"", xx, yy);
					notify(this, nyTOOLTIPS, (dword)str);
				}
				break;
			}
			repaint();
			father->notify(this, nyCHANGE);
		}
		switch(event)
		{
			case mouseLUP:
			mouseCapture(FALSE);
			test=FALSE;
			break;
		}
		return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Apaddle::set(float v)
{
	switch(mode)
	{
		case paddleX:
		case paddleXY:
		{
			float	ox=xx;
			xx=maxi(mini(v, 1.f), 0.f);
			if(defxx==-1)
				defxx=xx;
			if(ox!=xx)
			{
				control->set(Acontrol::CONTROLER_01, xx);
				repaint();
			}
		}
		break;

		case paddleY:
		{
			float	oy=yy;
			yy=maxi(mini(v, 1.f), 0.f);
			if(defyy==-1)
				defyy=yy;
			if(oy!=yy)
			{
				control->set(Acontrol::CONTROLER_01, yy);
				repaint();
			}
		}
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Apaddle::set(float x, float y)
{
	xx=maxi(mini(x, 1.f), 0.f);
	yy=maxi(mini(y, 1.f), 0.f);
	switch(mode)
	{
		case paddleX:
		if(defxx==-1)
			defxx=xx;
		control->set(Acontrol::CONTROLER_01, xx);
		break;

		case paddleY:
		if(defyy==-1)
			defyy=yy;
		control->set(Acontrol::CONTROLER_01, yy);
		break;

		case paddleXY:
		if(defxx==-1)
			defxx=xx;
		if(defyy==-1)
			defyy=yy;
		control->set(Acontrol::CONTROLER_01, xx);
		control->set(Acontrol::CONTROLER_02, yy);
		break;
	}
	repaint();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL float Apaddle::get(int type)
{
	switch(type)
	{
		case paddleCURRENT:
		switch(mode)
		{
			case paddleX:
			return xx;

			case paddleY:
			return yy;
		}
		break;

		case paddleX:
		return xx;

		case paddleY:
		return yy;
	}
	return -666.f;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Apaddle::sequence(int nctrl, float value)
{
	assert((value>=0.f)&&(value<=1.f));

	switch(mode)
	{
		case paddleX:
		assert(nctrl==Acontrol::CONTROLER_01);
		xx=maxi(mini(value, 1.f), 0.f);
		break;

		case paddleY:
		assert(nctrl==Acontrol::CONTROLER_01);
		yy=maxi(mini(value, 1.f), 0.f);
		break;

		case paddleXY:
		switch(nctrl)
		{
			case Acontrol::CONTROLER_01:
			xx=maxi(mini(value, 1.f), 0.f);
			break;

			case Acontrol::CONTROLER_02:
			yy=maxi(mini(value, 1.f), 0.f);
			break;

			default:
			assert(FALSE);
			break;
		}
		break;
	}
	father->notify(this, nyCHANGE);
	ctrlPaint=true;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Apaddle::pulse()
{
	if(ctrlPaint)
	{
		repaint();
		ctrlPaint=false;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Apaddle::paint(Abitmap *bitmap)
{
	switch(type)
	{
		case paddleLINE:
		{
			dword	cf=((lineColor>>1)&0x007f7f7f)|0xff000000;
			int		s=pos.w>>1;
			double	a=PI*1.4-yy*PI*1.7;
			int		px=(int)(cos(a)*s+s);
			int		py=(int)(-sin(a)*s+s);
			bitmap->linea((int)(cos(a+PI*0.5)+s), (int)(-sin(a+PI*0.5)+s), px, py, cf, 0.8f);
			bitmap->linea((int)(cos(a-PI*0.5)+s), (int)(-sin(a-PI*0.5)+s), px, py, cf, 0.8f);
			bitmap->line(s, s, px, py, lineColor);
		}
		break;

		default:
		if(bmp)
		{
			switch(mode)
			{
				case paddleX:
				{
					int	nx=bmp->w/ow;
					int	ny=bmp->h/oh;
					int	nb=nx*ny;
					int	n=mini((int)(xx*(float)nb), nb-1);
					bitmap->set(0, 0, (n%nx)*ow, (n/nx)*oh, ow, oh, bmp, bitmapDEFAULT, bitmapDEFAULT);
				}
				break;

				case paddleY:
				{
					int	nx=bmp->w/ow;
					int	ny=bmp->h/oh;
					int	nb=nx*ny;
					int	n=mini((int)(yy*(float)nb), nb-1);
					bitmap->set(0, 0, (n%nx)*ow, (n/nx)*oh, ow, oh, bmp, bitmapDEFAULT, bitmapDEFAULT);
				}
				break;

				case paddleXY:
				{
					int	nbx=bmp->w/ow;
					int	nby=bmp->h/oh;
					int	nx=mini((int)(xx*(float)nbx), nbx-1);
					int	ny=mini((int)(yy*(float)nby), nby-1);
					bitmap->set(0, 0, nx*ow, ny*oh, ow, oh, bmp, bitmapDEFAULT, bitmapDEFAULT);
				}
				break;
			}
		}
		break;
	}
	controlPaint(bitmap, pos.w, pos.h);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
