/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"export.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	OBJECT.CPP					(c)	YoY'99						WEB: search aestesis
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<stdio.h>
#include						"object.h"
#include						"interface.h"
#include						"window.h"
#include						"menu.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char							*mouseEVENTS[]=
{
								"mouseNORMAL",
								"mouseLDOWN",
								"mouseLUP",
								"mouseLDBLCLK",
								"mouseMDOWN",
								"mouseMUP",
								"mouseMDBLCLK",
								"mouseRDOWN",
								"mouseRUP",
								"mouseRDBLCLK",
								"mouseLOSTCAPTURE",
								"mouseWHEEL",
								"mouseSTAY",
								"mouseENTER",
								"mouseLEAVE"
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
static Aproperties				properties[]=	
							{	
								{	guidDWORD,	(int)&(((Aobject *)NULL)->state),		"state",		"object states"					},
								{	guidSDWORD,	(int)&(((Aobject *)NULL)->x),			"x",			"x position"					},
								{	guidSDWORD,	(int)&(((Aobject *)NULL)->y),			"y",			"y position"					},
								{	guidSDWORD,	(int)&(((Aobject *)NULL)->w),			"w",			"bitmap width"					},
								{	guidSDWORD,	(int)&(((Aobject *)NULL)->h),			"h",			"bitmap height"					} 
							};
*/

ADLL ACI						Aobject::CI=ACI("Aobject", GUID(0xAE57E515,0x00000002), &Anode::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Aobject::Aobject(char *name, Aobject *l, int x, int y, int w, int h) : Anode(name, l)
{
	pos.x=x;
	pos.y=y;
	pos.w=w;
	pos.h=h;
	tooltips=NULL;
	context=new Aitem(name, NULL, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Aobject::~Aobject()
{
	Awindow	*w=getWindow();
	if(w->mouseCaptureOBJ==this)
		w->mouseCaptureOBJ=NULL;
	if(w->objectTrack==this)
		w->objectTrack=NULL;
	if(w->keyboardFocusOBJ==this)
	{
		state&=~stateFOCUS;
		w->keyboardFocusOBJ=w;
		w->state|=stateFOCUS;
	}
	if(tooltips)
		free(tooltips);
	delete(context);
	timer();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void Aobject::update(Abitmap *b)
{
	paint(b);
	{
		Aobject		*o=(Aobject *)lchild;
		Arectangle	mclip=b->clip;
		Arectangle	mview=b->view;
		while(o)
		{
			if(o->state&stateVISIBLE)
			{
				b->view.translat(o->pos.x, o->pos.y);
				b->view.w=o->pos.w;
				b->view.h=o->pos.h;
				if(b->clip.intersect(b->view))
					o->update(b);
			}
			b->clip=mclip;
			b->view=mview;
			o=(Aobject *)o->prev;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void Aobject::repaint(bool now)
{
	if(state&stateVISIBLE)
		((Aobject *)father)->repaint(pos, now);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void Aobject::repaint(Arectangle r, bool now)
{
	if(state&stateVISIBLE)
	{
		r.x+=pos.x;
		r.y+=pos.y;
		((Aobject *)father)->repaint(r, now);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Aobject::mouseCapture(bool b, Aobject *o)
{
	if(o==NULL)
		return ((Aobject *)father)->mouseCapture(b, this);
	else
		return ((Aobject *)father)->mouseCapture(b, o);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Aobject::zorder(int action)
{
	bool	b=((Aobject *)father)->NCzorder(this, action);
	if(state&stateVISIBLE)
		repaint();
	return b;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Aobject::cursor(int cursor)
{
	return Awindow::NCcursor(cursor);
}
					
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL int Aobject::makeCursor(Aresobj *res, int hx, int hy)
{
	return Awindow::NCmakeCursor(res, hx, hy);
}
					
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Aobject::move(int x, int y)
{
	Arectangle	r=Arectangle(x, y, pos.w, pos.h);
	r.add(pos);
	pos.x=x;
	pos.y=y;
	if(state&stateVISIBLE)
	{
		Arectangle r0;
		r0.w=((Aobject *)father)->pos.w;
		r0.h=((Aobject *)father)->pos.h;
		r.intersect(r0);
		((Aobject *)father)->repaint(r);
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Aobject::show(bool visible, bool brepaint)
{
	if(visible)
		state|=stateVISIBLE;
	else
		state&=~stateVISIBLE;
	if(brepaint&&father)
		((Aobject *)father)->repaint(pos);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Aobject::focus(Aobject *o)
{
	if(o)
		return ((Aobject *)father)->focus(o);
	else
		return ((Aobject *)father)->focus(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Aobject::timer(int t)
{
	return NCglobalTimer(this, t);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Awindow * Aobject::getWindow()
{
	Anode	*o=this;
	while(o->father&&(!o->isGUID(guidWINDOW)))
		o=o->father;
	return (Awindow *)o;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Aobject * Aobject::getObject(int x, int y)
{
	Aobject	*o=(Aobject *)fchild;
	while(o&&((!(o->state&stateVISIBLE))||(x>=(o->pos.x+o->pos.w))||(x<o->pos.x)||(y>=(o->pos.y+o->pos.h))||(y<o->pos.y)))
		o=(Aobject *)o->next;
	if(o)
		return o->getObject(x-o->pos.x, y-o->pos.y);
	return this;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Aobject::getPosition(Aobject *o, int *x, int *y)
{
	*x=0;
	*y=0;
	while(o&&(o!=this))
	{
		*x+=o->pos.x;
		*y+=o->pos.y;
		o=(Aobject *)o->father;
	}
	return o!=NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void Aobject::mousePosition(int *mx, int *my)
{
	Awindow	*w=getWindow();
	Anode	*o=this;
	*mx=w->mouseX;
	*my=w->mouseY;
	while(o->father&&(!o->isGUID(guidWINDOW)))
	{
		*mx-=((Aobject *)o)->pos.x;
		*my-=((Aobject *)o)->pos.y;
		o=o->father;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL int Aobject::getKeyboardState()
{
	int		n=0;
	byte	ks[256];
	if(GetKeyboardState(ks))
	{
		if(ks[VK_CONTROL]&128)
			n|=keyboardCTRL;
		if(ks[VK_SHIFT]&128)
			n|=keyboardSHIFT;
		//if(ks[VK_ALT]&128)
		//	n|=keyboardALT;
		if(ks[VK_MENU]&128)
			n|=keyboardALT;
		if(ks[VK_LCONTROL]&128)
			n|=keyboardLCTRL;
		if(ks[VK_LSHIFT]&128)
			n|=keyboardLSHIFT;
		//if(ks[VK_]&128)
		//	n|=keyboardLALT;
		if(ks[VK_LMENU]&128)
			n|=keyboardLALT;
		if(ks[VK_RCONTROL]&128)
			n|=keyboardRCTRL;
		if(ks[VK_RSHIFT]&128)
			n|=keyboardRSHIFT;
		//if(ks[VK_]&128)
		//	n|=keyboardRALT;
		if(ks[VK_RMENU]&128)
			n|=keyboardRALT;
		if(ks[VK_LEFT]&128)
			n|=keyboardCURSORLEFT;
		if(ks[VK_RIGHT]&128)
			n|=keyboardCURSORRIGHT;
		if(ks[VK_HOME]&128)
			n|=keyboardHOME;
		if(ks[VK_END]&128)
			n|=keyboardEND;
		if(ks[VK_INSERT]&128)
			n|=keyboardINSERT;
		if(ks[VK_DELETE]&128)
			n|=keyboardDELETE;
		if(ks[VK_CLEAR]&128)
			n|=keyboardCLEAR;
		if(ks[VK_BACK]&128)
			n|=keyboardBACKSPACE;
		if(ks[VK_RETURN]&128)
			n|=keyboardENTER;
		if(ks[VK_UP]&128)
			n|=keyboardCURSORUP;
		if(ks[VK_DOWN]&128)
			n|=keyboardCURSORDOWN;
		if(ks[VK_NEXT]&128)
			n|=keyboardPAGEUP;
		if(ks[VK_PRIOR]&128)
			n|=keyboardPAGEDOWN;
		if(ks[VK_TAB]&128)
			n|=keyboardTAB;
		if(ks[VK_ESCAPE]&128)
			n|=keyboardESCAPE;
	}
	return n;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Aobject::NCdragdrop(int x, int y, int state, int event, void *data)
{
	Aobject	*o=(Aobject *)fchild;
	if(!(state&mouseCAPTURE))
	{
		while(o&&((!(o->state&stateVISIBLE))||(x>=(o->pos.x+o->pos.w))||(x<o->pos.x)||(y>=(o->pos.y+o->pos.h))||(y<o->pos.y)))
			o=(Aobject *)o->next;
		if(o&&o->NCdragdrop(x-o->pos.x, y-o->pos.y, state, event, data))
			return true;
	}
	if(this->state&stateENABLE)
	{
		Awindow	*w=getWindow();
		if((event!=dragdropLEAVE)&&(w->objectTrack!=this)&&(!o))
		{
			if(w->objectTrack)
				w->objectTrack->dragdrop(0, 0, 0, dragdropLEAVE, data);
			w->objectTrack=this;
			notify(this, nyTOOLTIPS, (dword)tooltips);
			dragdrop(x, y, state, dragdropENTER, data);
		}
		if(dragdrop(x, y, state, event, data))
			return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Aobject::NCmouse(int x, int y, int state, int event)
{
	Aobject	*o=(Aobject *)fchild;
	if(!(state&mouseCAPTURE))
	{
		while(o&&((!(o->state&stateVISIBLE))||(x>=(o->pos.x+o->pos.w))||(x<o->pos.x)||(y>=(o->pos.y+o->pos.h))||(y<o->pos.y)))
			o=(Aobject *)o->next;
		if(o&&o->NCmouse(x-o->pos.x, y-o->pos.y, state, event))
			return true;
	}
	//if(1)
	{
		Awindow	*w=getWindow();
		if((event!=mouseLEAVE)&&(w->objectTrack!=this)&&(!o))
		{
			if(w->objectTrack)
				w->objectTrack->mouse(0, 0, 0, mouseLEAVE);
			w->objectTrack=this;
			if(tooltips)
				notify(this, nyTOOLTIPS, (dword)tooltips);
			else
			{
				Aobject	*oo=(Aobject *)father;
				while(oo)
				{
					if(oo->tooltips)
					{
						notify(oo, nyTOOLTIPS, (dword)oo->tooltips);
						break;
					}
					oo=(Aobject *)oo->father;
				}
			}
			mouse(x, y, state, mouseENTER);
		}
		if(this->state&stateENABLE)
		{
			if(!o&&(event==mouseLDOWN))
				focus();
			if(mouse(x, y, state, event))
				return true;
		}
		if(!(this->state&stateNOCONTEXT))
		{
			switch(event)
			{
				case mouseRDOWN:
				new Amenu("context menu", w, NULL, this);
				return TRUE;
			}
		}
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aobject::NCzorder(Anode *o, int action)
{
	if(o->prev)
		o->prev->next=o->next;
	else
		fchild=o->next;
	if(o->next)
		o->next->prev=o->prev;
	else
		lchild=o->prev;
	o->next=NULL;
	o->prev=NULL;
	switch(action)
	{
		case zorderFRONT:
		o->state&=~stateTOP;
		{
			Anode	*os=fchild;
			while(os)
			{
				if(!(os->state&stateTOP))
					break;
				os=os->next;
			}
			if(os)
			{
				if(os->next)
				{
					o->next=os->next;
					o->prev=os;
					os->next->prev=o;
					os->next=o;
				}
				else
				{
					o->prev=os;
					o->next=NULL;
					os->next=o;
					lchild=o;
				}
			}
			else
			{
				if(lchild)
				{
					o->prev=lchild;
					o->next=NULL;
					lchild->next=o;
					lchild=o;
				}
				else
				{
					o->prev=NULL;
					o->next=NULL;
					fchild=lchild=o;
				}
			}
		}
		break;

		case zorderBACK:
		o->state&=~stateTOP;
		o->next=NULL;
		o->prev=lchild;
		if(lchild)
			lchild->next=o;
		else
			fchild=o;
		lchild=o;
		break;

		case zorderTOP:
		o->state|=stateTOP;
		o->prev=NULL;
		o->next=fchild;
		if(fchild)
			fchild->prev=o;
		else
			lchild=o;
		fchild=o;
		break;
	}
	return TRUE;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void Aobject::dropText(char *text)
{
	Awindow::dropText(text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void Aobject::destroy()
{
	timer();
	show(FALSE);
	if(father)
		((Aobject *)father)->repaint(pos);
	NCdestroy(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


