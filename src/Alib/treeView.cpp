/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"export.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	TREEVIEW.CPP				(c)	YoY'03						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"interface.h"
#include						"treeView.h"
#include						"resource.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL ACI						AtreeView::CI=ACI("AtreeView", GUID(0xAE57E515,0x00001110), &Aobject::CI, 0, NULL); // countof(properties), properties);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL AtreeView::AtreeView(char *name, Aobject *father, int x, int y, int w, int h, Aitem *items) : Aobject(name, father, x, y, w, h)
{
	colorText		= 0xff000000;
	colorFolder 	= 0xffffffff;
	colorBackground	= 0x80ffffff;
	colorSelect		= 0xff808040;
	colorOver		= 0xff408000;
	xtree=ytree=0;
	xftree=yftree=0;
    xmtree=ymtree=0;
    btree=false;
	this->items=items;
	current=NULL;
	font=alib.getFont(fontTERMINAL06);
	branchs=new Abitmap(&alibres.get(MAKEINTRESOURCE(PNG_TREEVIEW), "PNG"));
	nbinfo=0;
	calcInfo();		
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL AtreeView::~AtreeView()
{
	delete(branchs);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool AtreeView::size(int w, int h)
{
	Aobject::size(w, h);
	calcInfo();
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool AtreeView::mouse(int x, int y, int state, int event)
{
	switch(event)
	{
		case mouseNORMAL:
        if(btree)
        {
			cursor(cursorHAND);
            xtree=x-xftree;
            ytree=y-yftree;
            if(xmtree<pos.w)
                xtree=0;
            else
            {
                if(xtree>0)
                    xtree=0;
                else if(xtree<(pos.w-xmtree-50))
                    xtree=pos.w-xmtree-50;
            }
            if(ymtree<pos.h)
                ytree=0;
            else
            {
                if(ytree>0)
                    ytree=0;
                else if(ytree<(pos.h-ymtree-50))
                    ytree=pos.h-ymtree-50;
            }
            refresh();
        }
        else
        {
			cursor(cursorHANDSEL);
        }
		return true;

        case mouseLEAVE:
        case mouseLUP:
        if(btree)
			cursor(cursorHAND);
		else
			cursor(cursorHANDSEL);
        mouseCapture(false);
        btree=false;
        return true;
        
        case mouseWHEEL:
        {
			int	yf=ytree;
			ytree+=getWindow()->mouseW;
            if(ymtree<pos.h)
                ytree=0;
            else
            {
                if(ytree>0)
                    ytree=0;
                else if(ytree<(pos.h-ymtree-50))
                    ytree=pos.h-ymtree-50;
            }
            refresh();
        }
		return true;
			
        break;

		case mouseLDOWN:
		mouseCapture(true);
        xftree=x-xtree;
        yftree=y-ytree;
		{
			int	i;
			int	xx=x;
			int	yy=y;
			for(i=0; i<nbinfo; i++)
			{
				Arectangle	r=info[i].r;
				if(r.contains(xx, yy))
				{
                    Aitem  *n=info[i].item;
                    if(info[i].circ==-1)
                    {
                        if((n->state&Aitem::stateCOLLAPSED)==0)
                            n->state|=Aitem::stateCOLLAPSED;
                        else
                        {
                            n->state&=~Aitem::stateCOLLAPSED;
                            father->notify(this, nyEXTEND, (dword)n);
                        }
                        refresh();
						cursor(cursorHANDSEL);
                        return true;
                    }
                    else if(n!=null)
                    {
						if(state&(mouseSHIFT|mouseCTRL))
    					{
                            if((this->state&stateMULTISELECT)!=0)
                            {
    		    				if((n->state&Aitem::stateSELECTED)==0)
    		    				{
    			    				n->state|=Aitem::stateSELECTED;
    			    				notify(this, nyTOOLTIPS, (dword)n->help);
    			    			}
    				    		else
    					    		n->state&=~Aitem::stateSELECTED;
                                father->notify(this, nyCHANGE, (dword)n);
    						    repaint();
                            }
							cursor(cursorHANDSEL);
       						return true;
       					}
       					else
       					{
	    					deselect(items);
		    				n->state|=Aitem::stateSELECTED;
                            current=n;
 		    				notify(this, nyTOOLTIPS, (dword)n->help);
                            father->notify(this, nySELECT, (dword)n);
			    			repaint();
							cursor(cursorHANDSEL);
				    		return true;
					    }
                    }
                }
			}
		}
        //if((state&(mouseCTRL|mouseSHIFT|mouseALT))==0)
    	//	deselect(items);
		cursor(cursorHAND);
        btree=true;
		repaint();
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void AtreeView::align(Aitem *ns)
{
	Aitem	*nlevel[16];
	int		level=0;
	int		y=0;
	int		x=0;
	Aitem	*n=items;
	int		ni=0;
    int	    circ=0;
    
	while((n!=null)&&(n!=ns))
	{
        if(n->next==null)
            circ&=~(3<<((level-1)<<1));
        y+=16;
		if((n->state&Aitem::stateFOLDER)!=0)
        {
            circ|=(3<<(level<<1));
            if((n->state&Aitem::stateCOLLAPSED)==0)
            {
                nlevel[level++]=n;
                n=(Aitem *)n->fchild;
                if(n==null)
                    y+=16;
            }
            else
                n=(Aitem *)n->next;
        }
        else
            n=(Aitem *)n->next;
        while((n==null)&&(level>0))
        {
            circ&=~(3<<(level<<1));
            n=(Aitem *)nlevel[--level]->next;
        }
	}
    if(n!=null)
    {
        ytree=-y+20;
        refresh();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void AtreeView::deselect(Aitem *n)
{
	while(n!=null)
	{
		n->state&=~Aitem::stateSELECTED;
		if(n->fchild!=null)
			deselect((Aitem *)n->fchild);
		n=(Aitem *)n->next;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void AtreeView::collaps(Aitem *n)
{
	while(n!=null)
	{
		if(n->state&Aitem::stateFOLDER)
			n->state|=Aitem::stateCOLLAPSED;
		if(n->fchild!=null)
			collaps((Aitem *)n->fchild);
		n=(Aitem *)n->next;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void AtreeView::drawBranchs(Abitmap *b, int x, int y, int n)
{
	b->set(x, y, n*16, 0, 16, 16, branchs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void AtreeView::paint(Abitmap *b)
{
	int		i,ni;
	int		dh=(16-font->h)>>1;
	b->boxfa(0, 0, pos.w, pos.h, colorBackground, (float)((colorBackground>>24)&255)/255.f);
	for(ni=0; ni<nbinfo; ni++)
	{
		Aitem		*n=info[ni].item;
		int			level=info[ni].level;
		Arectangle	r=info[ni].r;
		int			circ=info[ni].circ;
		int			y=r.y;
        if(circ!=-1)
        {
    		for(i=0; i<level; i++)
    		{
                int v=(circ>>(i<<1))&3;
                if(v>0)
    				drawBranchs(b, xtree+i*16, y, 1+v);
    		}
            if(n==null)
        		font->set(b, r.x+2, r.y+dh, "...", colorText);
            else
            {
				dword	color;
    			if((n->state&Aitem::stateSELECTED)!=0)
    				b->boxf(r.x, r.y, r.x+r.w-1, r.y+r.h-1, colorSelect);
    			if(this->state&stateUSECOLORDATA)
    				color=n->data;
    			else
    				color=((n->state&Aitem::stateFOLDER)!=0)?colorFolder:colorText;
        		font->set(b, r.x+2, r.y+dh, n->name, color);
                if((n->state&Aitem::stateFOLDER)==0)
                {
					/*
                    if(n->icon!=null)
                        b->set(r.x-18, y, n->icon);
                    else */
        				drawBranchs(b, r.x-20, y, 5);
                }
            }
        }
        else
        {
    		drawBranchs(b, r.x, r.y, ((n->state&Aitem::stateCOLLAPSED)!=0)?0:1);
        }
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void AtreeView::refresh()
{
    calcInfo();
    repaint();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void AtreeView::calcInfo()
{
	Aitem	*nlevel[16];	//Anode	nlevel[]=new Anode[16];
	int		level=0;
	int		y=ytree;
	int		x=xtree;
	Aitem	*n=items;
	int		ni=0;
    int	    circ=0;
	int		dh=(16-font->h)>>1;
    xmtree=0;
    ymtree=0;

	while((n!=null)&&(y<-16))
	{
        if(n->next==null)
            circ&=~(3<<((level-1)<<1));
        y+=16;
		if((n->state&Aitem::stateFOLDER)!=0)
        {
            circ|=(3<<(level<<1));
            if((n->state&Aitem::stateCOLLAPSED)==0)
            {
                nlevel[level++]=n;
                n=(Aitem *)n->fchild;
                if(n==null)
                    y+=16;
            }
            else
                n=(Aitem *)n->next;
        }
        else
            n=(Aitem *)n->next;
        while((n==null)&&(level>0))
        {
            circ&=~(3<<(level<<1));
            n=(Aitem *)nlevel[--level]->next;
        }
	}

	while((n!=null)&&(y<pos.h)&&(ni<MAXTREEINFO))
	{
        info[ni].item=n;
        info[ni].circ=circ;
        info[ni].level=level;
        if((n->name!=null)&&(strlen(n->name)>0))
            info[ni].r=font->getRectangle(x+level*16+20, y, n->name);
        else
            info[ni].r=font->getRectangle(x+level*16+20, y, "...");
        info[ni].r.h=16;
        info[ni].r.w+=4;
        xmtree=maxi(xmtree, info[ni].r.x+info[ni].r.w-xtree);
        ymtree=maxi(ymtree, info[ni].r.y+info[ni].r.h-ytree);
        if(n->next==null)
        {
            circ&=~(3<<((level-1)<<1));
            info[ni].circ&=~(2<<((level-1)<<1));
        }
        else
            info[ni].circ&=~(1<<((level-1)<<1));
        ni++;
        y+=16;
		if((n->state&Aitem::stateFOLDER)!=0)
        {
            info[ni].item=n;
            info[ni].circ=-1;
            info[ni].level=level;
            info[ni].r=Arectangle(x+level*16, y-16, 16, 16);
            ni++;
            circ|=(3<<(level<<1));
            if((n->state&Aitem::stateCOLLAPSED)==0)
            {
                nlevel[level++]=n;
                n=(Aitem *)n->fchild;
                if(n==null)
                {
                    info[ni].item=null;
                    info[ni].circ=circ&(~(2<<((level-1)<<1)));
                    info[ni].level=level;
                    info[ni].r=font->getRectangle(x+level*16+4, y, "...");
                    ni++;
                    y+=16;
                }
            }
            else
                n=(Aitem *)n->next;
        }
        else
            n=(Aitem *)n->next;
        while((n==null)&&(level>0))
        {
            circ&=~(3<<(level<<1));
            n=(Aitem *)nlevel[--level]->next;
        }
	}
	nbinfo=ni;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void AtreeView::setCurrent(Aitem *i)
{
	deselect(items);
	collaps(items);
	i->state|=Aitem::stateSELECTED;
	current=i;
	view(i);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void AtreeView::view(Aitem	*i)
{
	{
		Aitem	*n=(Aitem *)i->father;
		while(n)
		{
			if((n->state&Aitem::stateFOLDER)!=0)
			{
				if((n->state&Aitem::stateCOLLAPSED)!=0)
					n->state&=~Aitem::stateCOLLAPSED;
			}
			n=(Aitem *)n->father;
		}
	}
	{
		Aitem	*nlevel[16];	//Anode	nlevel[]=new Anode[16];
		int		level=0;
		int		y=0;
		Aitem	*n=items;
		int		ni=0;
		int	    circ=0;
		int		yi=0;
		
		while(n!=null)
		{
			if(n==i)
				yi=y;
			if(n->next==null)
				circ&=~(3<<((level-1)<<1));
			y+=16;
			if((n->state&Aitem::stateFOLDER)!=0)
			{
				circ|=(3<<(level<<1));
				if((n->state&Aitem::stateCOLLAPSED)==0)
				{
					nlevel[level++]=n;
					n=(Aitem *)n->fchild;
					if(n==null)
						y+=16;
				}
				else
					n=(Aitem *)n->next;
			}
			else
				n=(Aitem *)n->next;
			while((n==null)&&(level>0))
			{
				circ&=~(3<<(level<<1));
				n=(Aitem *)nlevel[--level]->next;
			}
		}

		{
			ytree=pos.h/2-yi;
            if(ytree>0)
                ytree=0;
            else if(ytree<(pos.h-y-50))
                ytree=pos.h-y-50;
			refresh();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
