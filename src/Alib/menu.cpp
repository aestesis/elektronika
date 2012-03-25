/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"export.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	MENU.CPP					(c)	YoY'00						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<stdio.h>
#include						"menu.h"
#include						"interface.h"
#include						"resource.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL ACI						Amenu::CI=ACI("Amenu", GUID(0xAE57E515,0x00001040), &Asurface::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Amenu::Amenu(char *name, Aobject *father, Aitem *item, Aobject *toNotify, int type, Aitem *current) : Aobject(name, father, 0, 0, 4, 4)
{
	font=alib.getFont(fontTERMINAL06);
	this->current=current;
	this->defcur=current;
	sub=NULL;
	subcurrent=NULL;
	firstDrawn=NULL;
	this->toNotify=toNotify?toNotify:father;
	if(toNotify->isCI(&Amenu::CI))
	{
	/*
		((Aobject *)father)->mousePosition(&mx, &my);
		Amenu	*mn=(Amenu *)toNotify;
		int		ww=((Aobject *)mn->father)->pos.w;
		if(mn->pos.x>(ww-mn->pos.w-mn->pos.x))
		{
			mx=mn->pos.x;
			dmx=-1;
		}
		else
		{
			mx=mn->pos.x+mn->pos.w;
			dmx=1;
		}
	*/
		Amenu	*mn=(Amenu *)toNotify;
		((Aobject *)father)->mousePosition(&mx, &my);
		mx=mn->pos.x;
		dmx=1;
	}
	else
	{
		((Aobject *)father)->mousePosition(&mx, &my);
		dmx=0;
	}
	dy=0;
	if(item==NULL)
	{
		Aitem		*tt=new	Aitem("context menu");
		Aobject		*os=toNotify;
		this->type=menuCONTEXT;
		while(os)
		{
			Aitem	*ts=os->context;
			if(ts&&ts->count())
			{
				Aitem	*e=(Aitem *)ts->fchild;
				while(e)
				{
					tt->copyAll(e);
					e=(Aitem *)e->next;
				}
			}
			os=(Aobject *)os->father;
		}
		items=tt;
		if(!items->count())
		{
			destroy();
			return;
		}
	}
	else
	{
		if(type==menuUNKNOWN)
			this->type=menuNORMAL;
		else
			this->type=type;
		items=item;
	}
	zorder(zorderTOP);
	NCcalculate();
	mouse(mx-pos.x, my-pos.y, 0, mouseNORMAL);
	show(TRUE);
	mouseCapture(TRUE);
	repaint();
	timer(200);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Amenu::~Amenu()
{
	if(sub)
		sub->destroy();
	if((type==menuCONTEXT)&&items)
		delete(items);
}
	
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Amenu::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCHANGE:
		//destroy();
		return TRUE;

		case nyCLOSE:
		mouseCapture(true);
		if(sub)
		{
			sub=NULL;
			subcurrent=NULL;
		}
		if(!p)
		{
			destroy();
			toNotify->notify(this, nyCLOSE, 0);
		}
		else
			repaint();
		return TRUE;
	}
	return Aobject::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void Amenu::paint(Abitmap *b)
{
	int		hf=font->h+4;
	dword	green=sub?0xff008000:0xff00ff00;
	b->boxfa(4, 4, pos.w-5, pos.h-5, 0xff202020, 0.9f);
	{
		int			n=4;
		Aitem		*e=firstDrawn;
		while(e&&(n<(pos.h-4)))
		{
			dword	ct=0xff80ff00;
			if(e==defcur)
			{
				b->boxfa(4, n, pos.w-5, n+hf-1, green, 0.5f);
				ct=0xffffff00;
			}
			if(e==current)
			{
				b->boxf(4, n, pos.w-5, n+hf-1, green);
				ct=0xff000000;
			}
			if(e->name)
			{
				char	str[1024];
				strcpy(str, e->name);
				strupr(str);
				font->set(b, 10, n+2, str, ct);
			}
			n+=hf;
			e=(Aitem *)e->next;
		}
/*
		b->box(0, 0, pos.w-4, pos.h-4,  0xc0ffffff);
		b->box(1, 1, pos.w-3, pos.h-3,  0xc0c0c0c0);
		b->box(2, 2, pos.w-2, pos.h-2,  0xc0808080);
		b->box(3, 3, pos.w-1, pos.h-1,  0xc0404040);
*/
		b->box(0, 0, pos.w-4, pos.h-4,  green);
		b->box(1, 1, pos.w-3, pos.h-3,  green);
		b->box(2, 2, pos.w-2, pos.h-2,  green);
		b->box(3, 3, pos.w-1, pos.h-1,  green);

		if(items->fchild!=firstDrawn)
		{
			int		m=pos.w/2;
			dword	c=0xff000000;
			b->pixel(m, 0, c);
			b->line(m-1, 1, m+1, 1, c);
			b->line(m-2, 2, m+2, 2, c);
			b->line(m-3, 3, m+3, 3, c);
		}
		if(e)
		{
			int		m=pos.w/2;
			dword	c=0xff000000;
			b->pixel(m, pos.h-1, c);
			b->line(m-1, pos.h-2, m+1, pos.h-2, c);
			b->line(m-2, pos.h-3, m+2, pos.h-3, c);
			b->line(m-3, pos.h-4, m+3, pos.h-4, c);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void Amenu::pulse()
{
	int	hf=font->h+4;
	if(dy)
	{
		Aitem	*mfirstDrawn=firstDrawn;
		Aitem	*mcurrent=current;
		if(dy<0)
		{
			int		n=-dy;
			while(current&&(n--))
				current=(Aitem *)current->prev;
			if(!current)
				current=(Aitem *)items->fchild;
			{
				Aitem	*e=(Aitem *)items->fchild;
				while(e&&(e!=current)&&(e!=firstDrawn))
					e=(Aitem *)e->next;
				if(e==current)
					firstDrawn=e;
			}
		}
		else if(dy>0)
		{
			int		n=dy;
			while(current&&(n--))
				current=(Aitem *)current->next;
			if(!current)
			{
				int	n=items->count()-(((pos.h-8)/hf));
				firstDrawn=(Aitem *)items->fchild;
				while(n--)
					firstDrawn=(Aitem *)firstDrawn->next;
				n=(((pos.h-8)/hf));
				current=firstDrawn;
				while(current&&(n--))
					current=(Aitem *)current->next;
				if(!current)
					current=(Aitem *)items->lchild;
			}
			else
			{
				int			n1=(((pos.h-8)/hf));
				int			n0=0;
				Aitem		*e=firstDrawn;
				while(e&&(e!=current))
				{
					e=(Aitem *)e->next;
					n0++;
				}
				while(n0>=n1)
				{
					firstDrawn=(Aitem *)firstDrawn->next;
					n0--;
				}
			}
		}
		if((mfirstDrawn!=firstDrawn)||(mcurrent!=current))
		{
			repaint();
			if(current&&current->help)
			{
				setTooltips(current->help);
				toNotify->notify(this, nyTOOLTIPS, (dword)current->help);
			}
			else
			{
				setTooltips("left click -> OK / right click -> CANCEL");
				toNotify->notify(this, nyTOOLTIPS, (dword)"left click -> OK / right click -> CANCEL");
			}
		}
	}
	else
	{
	/*
		if(nomove==2)
		{
			if(subcurrent!=current)
			{
				if(sub)
				{
					sub->destroy();
					sub=NULL;
					subcurrent=NULL;
					repaint();
				}
				if(current&&current->fchild)
				{
					sub=new Amenu("submenu", (Aobject *)father, current, this, menuSUBCONTEXT);
					subcurrent=current;
					repaint();
				}
			}
		}
		nomove++;
	*/
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Amenu::mouse(int x, int y, int state, int event)
{
/*
	if((event!=mouseRDOWN)&&((x<0)||(x>=pos.w)||(y<0)||(y>=pos.h)))
	{
		if(toNotify&&toNotify->isCI(&Amenu::CI))
		{
			Amenu	*mn=((Amenu *)toNotify);
			int	mx,my;
			mn->mousePosition(&mx, &my);
			if((mx>=0)||(mx<mn->pos.w)||(y>=0)||(y<mn->pos.h))
			{
				nomove=5;
				return mn->mouse(mx, my, state|mouseFLAG, event);
			}
		}
		if(state&mouseFLAG)
			return true;
	}
*/
	switch(event)
	{
		case mouseLDOWN:
		return TRUE;

		case mouseLUP:
		if((x<0)||(x>=pos.w)||(y<0)||(y>=pos.h))
		{
			mouseCapture(FALSE);
			destroy();
			toNotify->notify(this, nyCLOSE);
			return TRUE;
		}
		mouse(x, y, 0, mouseNORMAL);
		//if(current&&current->fchild)
		if(current)
		{
			if(subcurrent!=current)
			{
				if(sub)
					sub->destroy();
				sub=NULL;
				subcurrent=NULL;
				if(current->fchild)
				{
					sub=new Amenu("submenu", (Aobject *)father, current, this, menuSUBCONTEXT);
					subcurrent=current;
					repaint();
					return TRUE;
				}
				repaint();
			}
			else if(current->fchild)
				return true;
		}
		if(current)
		{
			mouseCapture(FALSE);
			destroy();
			switch(type)
			{
				case menuCONTEXT:
				case menuSUBCONTEXT:
				if(current->link)
					current->link->notify(this, nyCONTEXT, (dword)current);
				else if(toNotify)
					toNotify->notify(this, nyCONTEXT, (dword)current);
				break;

				case menuNORMAL:
				toNotify->notify(this, nyCHANGE, (dword)current);
				break;
			}
			if(toNotify)
				toNotify->notify(this, nyCLOSE);
		}
		return TRUE;

		case mouseRDOWN:
		{
			destroy();
			if((x<0)||(x>=pos.w)||(y<0)||(y>=pos.h))
			{
				mouseCapture(FALSE);
				toNotify->notify(this, nyCLOSE);
			}
			else
				toNotify->notify(this, nyCLOSE, 1);
			return TRUE;
		}
		/*
		tree[level--]->restoreCurrents();
		if(level<0)
		{
			mouseCapture(FALSE);
			switch(type)
			{
				case menuNORMAL:
				object->notify(this, nyCANCEL);
				break;
			}
			delete(this);
			return TRUE;
		}
		NCinit();
		NCdisplay();
		father->update();
		*/
		return TRUE;

		case mouseNORMAL:
		nomove=0;
		{
			int	hf=font->h+4;
			if(y<6)
			{
				dy=-(1+((6-y)/2));
			}
			else if(y>=(((pos.h-4)/hf)*hf))
			{
				dy=1+((y-(((pos.h-4)/hf)*hf))/2);
			}
			else
			{
				Aitem	*mcurrent=current;
				int		yy=hf;
				Aitem	*e=firstDrawn;
				dy=0;
				while(e&&(yy<y))
				{
					yy+=hf;
					e=(Aitem *)e->next;
				}
				if(e)
					current=e;
				if(mcurrent!=current)
				{
					int	myy=0;
					int	y0,y1;
					{
						Aitem	*e=firstDrawn;
						while(e)
						{
							if(e==mcurrent)
								break;
							myy+=hf;
							e=(Aitem *)e->next;
						}
					}
					
					y0=mini(myy, yy-hf);
					y1=maxi(myy+hf+4, yy+4);
					
					Arectangle	r;
					r.w=pos.w;
					r.y=y0;
					r.h=y1-y0;
					repaint(r);
					
					if(current&&current->help)
					{
						setTooltips(current->help);
						toNotify->notify(this, nyTOOLTIPS, (dword)current->help);
					}
					else
					{
						setTooltips("left click -> OK / right click -> CANCEL");
						toNotify->notify(this, nyTOOLTIPS, (dword)"left click -> OK / right click -> CANCEL");
					}
				}
			}
		}
		return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Amenu::NCcalculate()
{
	int	hf=font->h+4;
	int	wmax=100;
	int	hmax=8;
	int	cur=0;
	int	nn=0;
	{
		Aitem		*e=(Aitem *)items->fchild;
		while(e)
		{
			if(e==current)
				cur=nn;
			if(e->name)
			{
				char	str[1024];
				strcpy(str, e->name);
				strupr(str);
				wmax=maxi(font->getWidth(str), wmax);
				hmax+=hf;
			}
			e=(Aitem *)e->next;
			nn++;
		}
	}
	wmax=mini(wmax+20, ((Aobject *)father)->pos.w);
	mx+=dmx*(wmax>>1);
	if(hmax<((Aobject *)father)->pos.h)
	{
		int	x=mx-wmax/2;
		int	y=my-hmax/2;
		pos.w=wmax;
		pos.h=hmax;
		if(!((x+pos.w)<((Aobject *)father)->pos.w))
			x=((Aobject *)father)->pos.w-pos.w;
		if(x<0)
			x=0;
		if(!((y+pos.h)<((Aobject *)father)->pos.h))
			y=((Aobject *)father)->pos.h-pos.h;
		if(y<0)
			y=0;
		move(x, y);
		firstDrawn=(Aitem *)items->fchild;
	}
	else
	{
		int	x=mx-wmax/2;
		pos.w=wmax;
		pos.h=((Aobject *)father)->pos.h;
		if(!((x+pos.w)<((Aobject *)father)->pos.w))
			x=((Aobject *)father)->pos.w-pos.w;
		if(x<0)
			x=0;
		move(x, 0);
		{
			int	ni=cur-((pos.h/hf)/2);
			ni=mini(maxi(ni, 0), nn-pos.h/hf);
			{
				Aitem	*e=(Aitem *)items->fchild;
				int		i;
				for(i=0; i<(ni+1); i++)
				{
					if(e)
					{
						if(e==current)
							break;
						e=(Aitem *)e->next;
					}
				}
				if(e)
					firstDrawn=e;
				else
					firstDrawn=(Aitem *)items->fchild;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
