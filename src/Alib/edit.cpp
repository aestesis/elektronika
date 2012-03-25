/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"export.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	EDIT.CPP					(c)	YoY'01						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<stdio.h>
#include						"edit.h"
#include						"interface.h"
#include						"resource.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Aedit::CI=ACI("Aedit", GUID(0xAE57E515,0x00001090), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define							ALLOCBLOC						128

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Aedit::Aedit(char *name, Aobject *L, int x, int y, int w, int h) : Aobject(name, L, x, y, w, h)
{
    colorBackground=0xffc0c0c0;
    colorBorder=0xff000000;
    colorText=0xff000000;
    colorCursor=0xffffff00;
    colorSelect=0xff008000;
    state|=stateOVERWRITE;
	fsel=-1;
	lsel=-1;
	xx=2;
	yy=2;
	xMouse=-1;
	sizeAlloc=ALLOCBLOC;
	caption=(char *)malloc(sizeAlloc);
	caption[0]=0;
	cursor=0;
	cursvis=false;
	haveFocus=false;
	font=alib.getFont(fontTERMINAL06);
	timer(400);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Aedit::~Aedit()
{
	free(caption);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void Aedit::pulse()
{
    if((state&stateFOCUS)!=0)
    {
        cursvis=!cursvis;
        haveFocus=true;
        repaint();
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void Aedit::paint(Abitmap *b)
{
	b->boxfa(0, 0, pos.w-1, pos.h-1, colorBackground, (float)(colorBackground>>24)/255.f);

	if((state&stateMULTILINE)==0)
	{
		Arectangle	r=font->getRectangle(0, 0, caption, 0, cursor);
		int         x=xx+r.w;
		int         y=yy+r.h-font->h;
		if(x<2)
		{
			xx-=(x-2);
			x=0;
		}
		else if(x>=pos.w)
		{
			xx-=(x-(pos.w-2));
			x=pos.w-2;
		}
		if(fsel!=-1)
		{
			if(fsel==lsel)
				fsel=lsel=-1;
			else
			{
				Arectangle	r0=font->getRectangle(xx, yy, caption, 0, fsel);
				Arectangle	r1=font->getRectangle(r0.x+r0.w, r0.y+r0.h-font->h, caption, fsel, lsel);
				b->boxfa(r1.x, r1.y, r1.x+r1.w-1, r1.y+r1.h-1, colorSelect, (float)(colorSelect>>24)/255.f);
			}
		}
		font->set(b, xx, yy, caption, colorText);
		if(cursvis)
			b->boxf(x-1, y, x, y+font->h, colorCursor);
	}
	else
	{
		Apoint   pc=font->getPosition(caption, cursor);
		pc.translat(xx, yy);
		if(pc.x<2)
			xx-=(pc.x-2);
		else if(pc.x>=pos.w)
			xx-=(pc.x-(pos.w-2));
		if(pc.y<0)
			yy-=pc.y;
		else if(pc.y>=(pos.h-font->h))
			yy-=(pc.y-(pos.h-font->h));
		if(fsel!=-1)
		{
			if(fsel==lsel)
				fsel=lsel=-1;
			else
			{
				Apoint	pff=font->getPosition(caption, fsel);
				Apoint	pf=pff;
				Apoint	p;
				bool	bt=false;
				int		i=fsel;
				while(i<lsel+1)
				{
					p=font->getPosition(caption, i);
					bt=true;
					if(p.y>pf.y)
					{
						b->boxfa(pff.x+xx, pff.y+yy, pf.x+xx, yy+pf.y+font->h, colorSelect, (float)(colorSelect>>24)/255.f);
						pff=p;
					}
					pf=p;
					i++;
                }
                if(bt)
                    b->boxfa(pff.x+xx, pff.y+yy, p.x+xx, yy+p.y+font->h, colorSelect, (float)(colorSelect>>24)/255.f);
            }
        }
        font->set(b, xx, yy, caption, colorText);
        if(cursvis)
        {
            Apoint   p=font->getPosition(caption, cursor);
            p.translat(xx, yy);
            b->boxfa(p.x-1, p.y, p.x, p.y+font->h, colorCursor, (float)(colorCursor>>24)/255.f);
        }
    }

	b->boxa(0, 0, pos.w-1, pos.h-1, colorBorder, (float)(colorBorder>>24)/255.f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Aedit::getMpos(int x, int y)
{
    if((state&stateMULTILINE)==0)
    {
        int xf=xx;
        int x0=xx;
        int y0=yy;
        int l=strlen(caption);
        int i;
        x+=(font->w>>1);
        if(x<=xx)
            return 0;
        for(i=0; i<l; i++)
        {
            Arectangle   r0=font->getRectangle(x0, y0, caption, i, i+1);
            x0+=r0.w;
            y0+=r0.h-(font->h+font->e);
            if((xf<x)&&(x0>=x))
                return i;
        }
        return i;
    }
    else
    {
        Apoint	pf;
        int		l=strlen(caption);
        int		i;
        x-=xx;
        y-=yy;
        x-=(font->w>>2);
        if(y<yy)
            return 0;
        int nl=y/(font->h+font->e);
        int bc=0;
        while((nl>0)&&(bc<l))
        {
            if(caption[bc]==13)
                nl--;
            bc++;
        }
        if(x<xx)
        {
            for(i=bc; i<l; i++)
            {
                Apoint   p0=font->getPosition(caption, i);
                if((y>p0.y)&&(y<p0.y+font->h+font->e))
                    return i;
                pf=p0;
            }
        }
        else
        {
            for(i=bc; i<l; i++)
            {
                Apoint   p0=font->getPosition(caption, i);
                if((pf.x<x)&&(p0.x>=x)&&(y>p0.y)&&(y<p0.y+font->h+font->e))
                    return i;
                else if(y<p0.y)
                    return ((i-1)<0)?0:(i-1);
                pf=p0;
            }
        }
        return i;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Aedit::mouse(int x, int y, int state, int event)
{
    switch(event)
    {
        case mouseENTER:
        //setCursor(new java.awt.Cursor(java.awt.Cursor.TEXT_CURSOR));
        break;

        case mouseLEAVE:
        xMouse=-1;
        //setCursor(new java.awt.Cursor(java.awt.Cursor.DEFAULT_CURSOR));
        break;

        case mouseLUP:
        if(xMouse!=-1)
        {
            int n=getMpos(x, y);
            if(xMouse<=n)
            {
                fsel=xMouse;
                lsel=n;
            }
            else
            {
                fsel=n;
                lsel=xMouse;
            }
            cursvis=true;
            xMouse=-1;
            repaint();
			mouseCapture(false);
        }
        return true;

        case mouseLDOWN:
		mouseCapture(true);
        {
            int n=getMpos(x, y);
            cursor=n;
            fsel=lsel=-1;
            xMouse=n;
            cursvis=true;
            repaint();
        }
        return true;

        case mouseNORMAL:
		Aobject::cursor(cursorTEXT);
        if(xMouse!=-1)
        {
            int n=getMpos(x, y);
            cursor=n;
            if(xMouse<=n)
            {
                fsel=xMouse;
                lsel=n;
            }
            else
            {
                fsel=n;
                lsel=xMouse;
            }
            cursvis=true;
            repaint();
        }
        return true;
    }
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


ADLL bool Aedit::keyboard(int event, int ascii, int scan, int state)
{
	byte	c=ascii;
	int		a=scan;	
    switch(event)
    {
        case keyboardGETFOCUS:
        haveFocus=false;
        cursvis=true;
        repaint();
        break;

        case keyboardLOSTFOCUS:
        haveFocus=true;
        cursvis=false;
		fsel=lsel=-1;
		xMouse=-1;
		father->notify(this, nyOK, 0);
        repaint();
        break;

        case keyboardDOWN:
        //System.out.println("touche: "+a);
        switch(c)
        {
            case 65:    // select all // ctrl+a
            if((state&keyboardCTRL)!=0)
            {
                fsel=0;
                lsel=strlen(caption)-1;
                if(lsel>0)
                {
                    cursvis=true;
                    repaint();
                }
                else
                    fsel=lsel=-1;
            }
            break;
            
            case 33:    // page up
            if((this->state&stateMULTILINE)!=0)
            {
                int     lcursor=cursor;
                Apoint   p=font->getPosition(caption, cursor);
                p.translat(xx, yy);
                cursor=getMpos(p.x+5, p.y-pos.h);
                if((state&keyboardSHIFT)!=0)
                {
                    if(fsel==-1)
                    {
                        lsel=lcursor;
                        fsel=cursor;
                    }
                    else
                    {
                        if(lcursor==fsel)
                            fsel=cursor;
                        else
                            lsel=cursor;
                    }
                    if(lsel<fsel)
                    {
                        int n=fsel;
                        fsel=lsel;
                        lsel=n;
                    }
                }
                else
                    fsel=lsel=-1;
                cursvis=true;
                repaint();
            }
            break;
            
            case 34:    // page down
            if((this->state&stateMULTILINE)!=0)
            {
				int     lcursor=cursor;
				Apoint   p=font->getPosition(caption, cursor);
				p.translat(xx, yy);
				cursor=getMpos(p.x, p.y+font->h+font->e+pos.h);
				if((state&keyboardSHIFT)!=0)
                {
                    if(fsel==-1)
                    {
                        fsel=lcursor;
                        lsel=cursor;
                    }
                    else
                    {
                        if(lcursor==fsel)
                            fsel=cursor;
                        else
                            lsel=cursor;
                    }
                    if(lsel<fsel)
                    {
                        int n=fsel;
                        fsel=lsel;
                        lsel=n;
                    }
                }
                else
                    fsel=lsel=-1;
                cursvis=true;
                repaint();
            }
            break;
            
            case 38:    // cursor up
            if((this->state&stateMULTILINE)!=0)
            {
				int		lcursor=cursor;
				Apoint	p=font->getPosition(caption, cursor);
				p.translat(xx, yy);
				cursor=getMpos(p.x, p.y-5);
				if((state&keyboardSHIFT)!=0)
                {
                    if(fsel==-1)
                    {
                        lsel=lcursor;
                        fsel=cursor;
                    }
                    else
                    {
                        if(lcursor==fsel)
                            fsel=cursor;
                        else
                            lsel=cursor;
                    }
                    if(lsel<fsel)
                    {
                        int n=fsel;
                        fsel=lsel;
                        lsel=n;
                    }
                }
                else
                    fsel=lsel=-1;
                cursvis=true;
                repaint();
            }
            break;

            case 40:    // cursor down
            if((this->state&stateMULTILINE)!=0)
            {
				int		lcursor=cursor;
				Apoint	p=font->getPosition(caption, cursor);
				p.translat(xx, yy);
				cursor=getMpos(p.x, p.y+font->h+5);
				if((state&keyboardSHIFT)!=0)
                {
                    if(fsel==-1)
                    {
                        fsel=lcursor;
                        lsel=cursor;
                    }
                    else
                    {
                        if(lcursor==fsel)
                            fsel=cursor;
                        else
                            lsel=cursor;
                    }
                    if(lsel<fsel)
                    {
                        int n=fsel;
                        fsel=lsel;
                        lsel=n;
                    }
                }
                else
                    fsel=lsel=-1;
                cursvis=true;
                repaint();
            }
            break;

            case 37:    // cursor left
			if((state&keyboardSHIFT)!=0)
			{
				if(cursor>0)
				{
					if(fsel==-1)
					{
						lsel=cursor;
						fsel=--cursor;
						cursvis=true;
						repaint();
					}
					else
					{
						if(cursor==fsel)
						{
							fsel=--cursor;
							cursvis=true;
							repaint();
						}
						else
						{
							lsel=--cursor;
							cursvis=true;
							repaint();
						}
					}
				}
			}
			else
			{
                if(fsel!=-1)
                {
                    cursor=fsel;
                    fsel=lsel=-1;
                    cursvis=true;
                    repaint();
                }
                else if(cursor>0) 
                {
                    fsel=lsel=-1;
                    cursor--;
                    cursvis=true;
                    repaint();
                }
            }
            break;

            case 39:    // cursor right
            if((state&keyboardSHIFT)!=0)
            {
                if(cursor<(int)strlen(caption))
                {
                    if(fsel==-1)
                    {
                        fsel=cursor;
                        lsel=++cursor;
                        cursvis=true;
                        repaint();
                    }
                    else
                    {
                        if(cursor==lsel)
                        {
                            lsel=++cursor;
                            cursvis=true;
                            repaint();
                        }
                        else
                        {
                            fsel=++cursor;
                            cursvis=true;
                            repaint();
                        }
                    }
                }
            }
            else
            {
                if(fsel!=-1)
                {
                    cursor=lsel;
                    fsel=lsel=-1;
                    cursvis=true;
                    repaint();
                }
                else if(cursor<(int)strlen(caption)) 
                {
                    fsel=lsel=-1;
                    cursor++;
                    cursvis=true;
                    repaint();
                }
            }
            break;

            case 36:    // home
            if(((this->state&stateMULTILINE)!=0)&&((state&keyboardCTRL)==0))
            {
				int	len=strlen(caption);
                if((fsel!=-1)&&(cursor!=fsel))
                    cursor=lsel=fsel;
                int lcursor=cursor;
                if(cursor>=len)
                    cursor=len-1;
                else if(caption[cursor]==13)
                    cursor--;
                if(cursor<0)
                    cursor=0;
                while((cursor>0)&&(caption[cursor]!=13))
                    cursor--;
                if((cursor<len)&&(caption[cursor]==13))
                    cursor++;
                if((state&keyboardSHIFT)!=0)
                {
                    if(fsel==-1)
                    {
                        lsel=lcursor;
                        fsel=cursor;
                    }
                    else
                    {
                        if(fsel==lcursor)
                            fsel=cursor;
                        else
                            lsel=cursor;
                    }
                }
                else
                    fsel=lsel=-1;
                cursvis=true;
                repaint();
            }
            else
            {
                if((state&keyboardSHIFT)!=0)
                {
                    if(fsel==-1)
                    {
                        lsel=cursor;
                        fsel=cursor=0;
                        cursvis=true;
                        repaint();
                    }
                    else
                    {
                        fsel=cursor=0;
                        cursvis=true;
                        repaint();
                    }
                }
                else
                {
                    cursor=0;
                    fsel=lsel=-1;
                    cursvis=true;
                    repaint();
                }
            }
            break;

            case 35:    // end
            if(((this->state&stateMULTILINE)!=0)&&((state&keyboardCTRL)==0))
            {
				int	len=strlen(caption);
                if((fsel!=-1)&&(cursor==fsel))
                    cursor=fsel=lsel;
                int lcursor=cursor;
                while((cursor<len)&&(caption[cursor]!=13))
                    cursor++;
                if((state&keyboardSHIFT)!=0)
                {
                    if(fsel==-1)
                    {
                        fsel=lcursor;
                        lsel=cursor;
                    }
                    else
                    {
                        if(lsel==lcursor)
                            lsel=cursor;
                        else
                            fsel=cursor;
                    }
                }
                else
                    fsel=lsel=-1;
                cursvis=true;
                repaint();
            }
            else
            {
                if((state&keyboardSHIFT)!=0)
                {
                    if(fsel==-1)
                    {
                        fsel=cursor;
                        lsel=cursor=strlen(caption);
                        cursvis=true;
                        repaint();
                    }
                    else
                    {
                        lsel=cursor=strlen(caption);
                        cursvis=true;
                        repaint();
                    }
                }
                else
                {
                    cursor=strlen(caption);
                    fsel=lsel=-1;
                    cursvis=true;
                    repaint();
                }
            }
            break;

            case 8:     // backspace
            if(fsel!=-1)
            {
				strcpy(caption+fsel, caption+lsel);
                cursor=fsel;
                fsel=lsel=-1;
                cursvis=true;
                repaint();
                father->notify(this, nyCHANGE, 0);
            }
            else if(cursor>0)
            {
				strcpy(caption+cursor-1, caption+cursor);
                cursor--;
                fsel=lsel=-1;
                cursvis=true;
                repaint();
                father->notify(this, nyCHANGE, 0);
            }
            break;

            case 67:    // copy // ctrl+c
            if(fsel!=-1)
            {
                if((state&keyboardCTRL)!=0)
				{
					Aclipboard::setText(caption, fsel, lsel);
				}
            }
            break;
            
            case 88:    // cut  // ctrl+x
            if((fsel!=-1)&&((state&keyboardCTRL)!=0))
            {
                Aclipboard::setText(caption, fsel, lsel);
				strcpy(caption+fsel, caption+lsel);
                cursor=fsel;
                fsel=lsel=-1;
                cursvis=true;
                repaint();
                father->notify(this, nyCHANGE, 0);
            }
            break;
            
            case 46:   // del
            if(fsel!=-1)
            {
                if((state&keyboardSHIFT)!=0)
                    Aclipboard::setText(caption, fsel, lsel);
				strcpy(caption+fsel, caption+lsel);
                cursor=fsel;
                fsel=lsel=-1;
                cursvis=true;
                repaint();
                father->notify(this, nyCHANGE, 0);
            }
            else if(cursor<(int)strlen(caption))
            {
				strcpy(caption+cursor, caption+cursor+1);
                cursvis=true;
                fsel=lsel=-1;
                repaint();
                father->notify(this, nyCHANGE, 0);
            }
            break;

            case 86:    // insert // ctrl+v
            if((state&keyboardCTRL)!=0)
            {
                char	*s=Aclipboard::getText();
                if(fsel!=-1)
                {
					strcpy(caption+fsel, caption+lsel);
					cursor=fsel;
                    fsel=lsel=-1;
                }
                if(s)
                {
					{
						int	lens=strlen(s);
						int	lenc=strlen(caption);
						if((lens+lenc+3)<sizeAlloc)
						{
							int	i;
							int	d=lens;
							for(i=lenc+1; i>=cursor; i--)
								caption[i+lens]=caption[i];
							memcpy(caption+cursor, s, lens);
						}
						else
						{
							sizeAlloc=(((lenc+lens)/ALLOCBLOC)+1)*ALLOCBLOC;
							char	*str=(char *)malloc(sizeAlloc);
							memcpy(str, caption, cursor);
							memcpy(str+cursor, s, lens);
							strcpy(str+cursor+lens, caption+cursor);
							free(caption);
							caption=str;
						}
						cursor+=lens;
						cursvis=true;
						repaint();
		                father->notify(this, nyCHANGE, 0);
					}
                }
            }
            break;
            
            case 45:       // insert
            if((state&keyboardSHIFT)!=0)
            {
                char	*s=Aclipboard::getText();
				if(fsel!=-1)
				{
					strcpy(caption+fsel, caption+lsel);
					cursor=fsel;
					fsel=lsel=-1;
				}
				if(s)
				{
					{
						int	lens=strlen(s);
						int	lenc=strlen(caption);
						if((lens+lenc+3)<sizeAlloc)
						{
							int	i;
							int	d=lens;
							for(i=lenc+1; i>=cursor; i--)
								caption[i+lens]=caption[i];
							memcpy(caption+cursor, s, lens);
						}
						else
						{
							sizeAlloc=(((lenc+lens)/ALLOCBLOC)+1)*ALLOCBLOC;
							char	*str=(char *)malloc(sizeAlloc);
							memcpy(str, caption, cursor);
							memcpy(str+cursor, s, lens);
							strcpy(str+cursor+lens, caption+cursor);
							free(caption);
							caption=str;
						}
						cursor+=lens;
						cursvis=true;
						repaint();
						father->notify(this, nyCHANGE, 0);
					}
				}
            }
            break;

        }
        break;

        case keyboardCHAR:
        if(filter(c))
        {
            switch(c)
            {
                case 13:    // return !!
                if((this->state&stateMULTILINE)==0)
					break;

                default:
				if(((byte)c>31)||((byte)c==13))
				{
					if((fsel!=-1)&&((this->state&stateOVERWRITE)!=0))
					{
						strcpy(caption+fsel, caption+lsel);
						cursor=fsel;
						fsel=lsel=-1;
					}
					{
						int	len=strlen(caption);
						if(len+3>sizeAlloc)
						{
							sizeAlloc+=ALLOCBLOC;
							char	*str=(char *)malloc(sizeAlloc);
							strncpy(str, caption, cursor);
							str[cursor]=c;
							strcpy(str+cursor+2, caption+cursor+1);
							free(caption);
							caption=str;
						}
						else
						{
							int	i;
							for(i=len+1; i>=cursor; i--)
								caption[i+1]=caption[i];
							caption[cursor]=c;
						}
						cursor++;
						cursvis=true;
						repaint();
						father->notify(this, nyCHANGE, 0);
					}
				}
				break;
            }
        }
        break;
        
        case keyboardUP:
        break;
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void Aedit::set(char *text)
{
	if(text)
	{
		sizeAlloc=((strlen(text)/ALLOCBLOC)+1)*ALLOCBLOC;
		char	*str=(char *)malloc(sizeAlloc);
		strcpy(str, text);
		free(caption);
		caption=str;
	}
	else
		caption[0]=0;
	repaint();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL int Aedit::getSize()
{
	return strlen(caption);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void Aedit::get(char *text, int size)
{
	strncpy(text, caption, size);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
