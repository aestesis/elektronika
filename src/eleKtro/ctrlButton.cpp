/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"elektroexp.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CTRLBUTTON.CPP				(c)	YoY'01						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<stdio.h>
#include						<assert.h>
#include						"ctrlButton.h"
#include						"interface.h"
#include						"resource.h"
#include						"effect.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL ACI						ActrlButton::CI		= ACI("ActrlButton",	GUID(0xE4EC7600,0x00010090), &AcontrolObj::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL ActrlButton::ActrlButton(QIID qiid, char *name, Aobject *L, int x, int y, int w, int h, Aresobj *o, int type) : AcontrolObj(name, L, x, y, w, h)
{
	control=new Acontrol(qiid, name, Aeffect::getEffect(L), this, Acontrol::CONTROLER_01);
	control->oscType=((type&Abutton::bt2STATES)!=0)?Acontrol::osc2STATES:Acontrol::oscPRESS;
	button=new Abutton(name, this, 0, 0, w, h, o, type);
	button->show(true);
	set(false);
	ctrlPaint=false;
	timer(CTRLTIMEREPAINT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL ActrlButton::ActrlButton(QIID qiid, char *name, Aobject *L, int x, int y, int w, int h, Abitmap *b, int type) : AcontrolObj(name, L, x, y, w, h)
{
	control=new Acontrol(qiid, name, Aeffect::getEffect(L), this, Acontrol::CONTROLER_01);
	control->oscType=(type&Abutton::bt2STATES)?Acontrol::osc2STATES:Acontrol::oscPRESS;
	button=new Abutton(name, this, 0, 0, w, h, b, type);
	button->show(true);
	set(false);
	ctrlPaint=false;
	timer(CTRLTIMEREPAINT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL ActrlButton::~ActrlButton()
{
	delete(control);
	delete(button);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void ActrlButton::pulse()
{
	if(ctrlPaint)
	{
		button->mouse(0,0,0,mouseLEAVE);
		ctrlPaint=false;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void ActrlButton::paint(Abitmap *b)
{
	controlPaint(b, pos.w, pos.h);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool ActrlButton::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCHANGE:
		assert(button->type&Abutton::bt2STATES);
		control->set(Acontrol::CONTROLER_01, button->checked?1.f:0.f);
		father->notify(this, nyCHANGE, button->checked);
		return true;
		
		case nyPRESS:
		control->select();
		if(!(button->type&Abutton::bt2STATES))
		{
			control->set(Acontrol::CONTROLER_01, 1.f);
			father->notify(this, nyPRESS, 0);
		}
		return true;

		case nyCLICK:
		/*	old system
		assert((button->type&Abutton::bt2STATES)==0);
		control->set(Acontrol::CONTROLER_01, 1.f);
		father->notify(this, nyCLICK, 0);
		*/
		return true;
		
		
		case nyRELEASE:
		if(!(button->type&Abutton::bt2STATES))
		{
			control->set(Acontrol::CONTROLER_01, 0.f);
			father->notify(this, nyRELEASE, 0);
		}
		return true;
	}
	return AcontrolObj::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool ActrlButton::sequence(int nctrl, float value)
{
	if(button->type&Abutton::bt2STATES)
	{
		button->checked=(value>=0.5f);
		father->notify(this, nyCHANGE, button->checked);
	}
	else
	{
		if(value>0.f)
		{
			button->checked=true;
			father->notify(this, nyPRESS, 0);
		}
		else
		{
			button->checked=false;
			father->notify(this, nyRELEASE, 0);
			father->notify(this, nyCLICK, 0);
		}
	}
	ctrlPaint=true;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool ActrlButton::get()
{
	return button->checked;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void ActrlButton::set(bool b)
{
	button->setChecked(b);
	control->set(Acontrol::CONTROLER_01, button->checked?1.f:0.f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
