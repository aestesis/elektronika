/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	ABOUT.CPP					(c)	YoY'00						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<math.h>
#include						"about.h"
#include						"main.h"
#include						"resource.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Aabout::CI=ACI("Aabout", GUID(0x11111111,0x00000103), &Awindow::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AaboutMenu : public Aobject
{
	AOBJ

								AaboutMenu						(char *name, class Aobject *L, int x, int y, int w, int h);
	virtual						~AaboutMenu						();

	virtual void				paint							(Abitmap *b);
	virtual bool				mouse							(int x, int y, int state, int event);

	void						calculText						();

	int							position;
	Afont						*font;
	char						str[8192][128];
	int							nbstr;
};

ACI								AaboutMenu::CI=ACI("AaboutMenu", GUID(0x11111111,0x00000114), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AaboutMenu::AaboutMenu(char *name, Aobject *L, int x, int y, int w, int h) : Aobject(name, L, x, y, w, h)
{
	state|=stateNOCONTEXT;
	position=0;
	font=alib.getFont(fontTERMINAL09);
	calculText();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AaboutMenu::~AaboutMenu()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AaboutMenu::mouse(int x, int y, int state, int event)
{ 
	switch(event)
	{
		case mouseWHEEL:
		position=maxi(mini(position-(getWindow()->mouseW>>5), nbstr-5), 0);
		return true;
	}
	return ((Aobject *)father)->mouse(x+pos.x, y+pos.y, state, event); 
}

void AaboutMenu::calculText()
{
	{
		Aresobj *text=resource.getObj(MAKEINTRESOURCE(TXT_LICENSEDEMO), "TXT");
		char	*txt=(char *)text->lock();
		int		size=text->getSize();
		int		p=0;

		nbstr=0;

		while(p<size)
		{
			int		d=p;
			int		w=4;
			char	t[2];
			int		l=size-p;
			int		pn=p;
			char	s=0;
			int		last;
			t[1]=0;
			while((p<size)&&(w<this->pos.w))
			{
				last=p;
				s=t[0]=txt[p++];
				switch(s)
				{
					case ' ':
					l=p-d;
					pn=p;
					w+=font->w>>1;
					break;

					case '\n':
					d=p;
					break;

					case '\r':
					l=p-d;
					pn=p;
					w=this->pos.w;
					break;

					default:
					w+=font->getWidth(t);
					break;
				}
			}
			if(!(p>=size))
				p=pn;
			else
				l=p-d;
			if(l>0)
			{
				strncpy(str[nbstr], txt+d, l);
				str[nbstr++][l]=0;
			}
			else
				str[nbstr++][0]=0;
		}

		text->unlock();
		delete(text);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AaboutMenu::paint(Abitmap *b)
{
	b->boxfa(0, 0, pos.w-1, pos.h-1, 0xff000000, 0.9f);
	{
		int	y=4;
		int	p=position;
		int	h=font->getHeight("A");
		while(((y+h)<pos.h)&&(p<nbstr))
		{
			font->set(b, 4, y, str[p], 0xffffaa00);
			p++;
			y+=h;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aabout::Aabout(char *name, int x, int y, class MYwindow *myw) : Awindow(name, x, y, 400, 300)
{
	this->myw=myw;
	bac=false;
	zorder(zorderTOP);
	back=new Abitmap(pos.w, pos.h);
	title=new Astatic("title", this, 0, 0, &resource.get(MAKEINTRESOURCE(PNG_ELEKTRONIKA), "PNG"));
	title->show(TRUE);

	buttonClose=new Abutton("close", this, 378, 6, 16, 16, &alibres.get(MAKEINTRESOURCE(117), "PNG"));
	buttonClose->setTooltips("close button");
	buttonClose->show(TRUE);

	pal.ar=frand()*100.f;
	pal.ag=frand()*100.f;
	pal.ab=frand()*100.f;
	pal.dr=(frand()+0.1f)*0.1f;
	pal.dg=(frand()+0.1f)*0.1f;
	pal.db=(frand()+0.1f)*0.1f;
	pal.ar0=frand()*100.f;
	pal.ag0=frand()*100.f;
	pal.ab0=frand()*100.f;
	pal.dr0=(frand()+0.1f)*0.09f;
	pal.dg0=(frand()+0.1f)*0.09f;
	pal.db0=(frand()+0.1f)*0.09f;
	pal.ar1=frand()*100.f;
	pal.ag1=frand()*100.f;
	pal.ab1=frand()*100.f;
	pal.dr1=(frand()+0.1f)*0.03f;
	pal.dg1=(frand()+0.1f)*0.03f;
	pal.db1=(frand()+0.1f)*0.03f;
	pal.ar10=frand()*100.f;
	pal.ag10=frand()*100.f;
	pal.ab10=frand()*100.f;
	pal.dr10=(frand()+0.1f)*0.01f;
	pal.dg10=(frand()+0.1f)*0.01f;
	pal.db10=(frand()+0.1f)*0.01f;
	pal.n=1.f/(frand()*512.f+16.f);

	zyg.zz1=5.f/(frand()*pos.w*10.f+(float)pos.w);
	zyg.zz2=5.f/(frand()*pos.w*10.f+(float)pos.w);
	zyg.zz3=5.f/(frand()*pos.w*10.f+(float)pos.w);

	zyg.ax1=frand()*100.f;
	zyg.ay1=frand()*100.f;
	zyg.ax2=frand()*100.f;
	zyg.ay2=frand()*100.f;
	zyg.ax3=frand()*100.f;
	zyg.ay3=frand()*100.f;

	zyg.dx1=(frand()+0.1f)*0.03f;
	zyg.dy1=(frand()+0.1f)*0.03f;
	zyg.dx2=(frand()+0.1f)*0.03f;
	zyg.dy2=(frand()+0.1f)*0.03f;
	zyg.dx3=(frand()+0.1f)*0.03f;
	zyg.dy3=(frand()+0.1f)*0.03f;

	menu=new AaboutMenu("about menu", this, 10, pos.h-110, pos.w-20, 100);
	menu->show(TRUE);

	hlp=new Abutton("help", this, 306, 120, 80, 16, "HELP");
	hlp->setTooltips("help file");
	hlp->show(true);

	web=new Abutton("web", this, 306, 140, 80, 16, "WEB");
	web->setTooltips("aestesis web site");
	web->show(true);

	reg=new Abutton("reg", this, 306, 160, 80, 16, "DONATE");
	reg->setTooltips("donate");
	reg->show(true);

	timer(100);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aabout::~Aabout()
{
	timer(0);
	delete(hlp);
	delete(web);
	delete(reg);
	delete(back);
	delete(title);
	delete(buttonClose);
	delete(menu);
	back=NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aabout::mouse(int x, int y, int state, int event)
{
	switch(event)
	{
		case mouseLDOWN:
		wx=pos.x;
		wy=pos.y;
		lx=pos.x+x;
		ly=pos.y+y;
		bac=TRUE;
		mouseCapture(TRUE);
		return TRUE;

		case mouseNORMAL:
		if((state&mouseL)&&bac)
			move(wx+(x+pos.x)-lx, wy+(y+pos.y)-ly);
		return TRUE;

		case mouseLUP:
		bac=FALSE;
		mouseCapture(FALSE);
		return TRUE;

		case mouseRDOWN:
		destroy();
		return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aabout::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCLICK:
		if(o==buttonClose)
		{
			destroy();
		}
		else if(o==hlp)
		{
			char	help[1024];
			GetModuleFileName(GetModuleHandle(null), help, sizeof(help));
			if(help[0])
			{
				char	*s=strrchr(help, '\\');
				if(s)
					*s=0;
			}
			strcat(help, "\\help\\elektronika.chm");
			ShellExecute(getWindow()->hw, "open", help, NULL, NULL, SW_SHOWNORMAL);
		}
		else if(o==web)
		{
			ShellExecute(getWindow()->hw, "open", "http://www.aestesis.eu/", NULL, NULL, SW_SHOWNORMAL);
			//httpto("http://www.aestesis.org/");
		}
		else if(o==reg)
		{
			ShellExecute(getWindow()->hw, "open", "http://aestesis.eu/", NULL, NULL, SW_SHOWNORMAL);
			//httpto("http://www.aestesis.org/aestesis/register.php");
		}
		break;
	}
	return Aobject::notify(o, event , p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aabout::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aabout::pulse()
{
	if(back)
	{
		int		x,y,i;
		dword	palette[8192];
		{
			float	ar=pal.ar;
			float	ag=pal.ag;
			float	ab=pal.ab;
			float	dr=pal.dr*0.1f;
			float	dg=pal.dg*0.1f;
			float	db=pal.db*0.1f;
			float	ar0=pal.ar0;
			float	ag0=pal.ag0;
			float	ab0=pal.ab0;
			float	dr0=pal.dr0*0.1f;
			float	dg0=pal.dg0*0.1f;
			float	db0=pal.db0*0.1f;
			float	ar1=pal.ar1;
			float	ag1=pal.ag1;
			float	ab1=pal.ab1;
			float	dr1=pal.dr1*0.1f;
			float	dg1=pal.dg1*0.1f;
			float	db1=pal.db1*0.1f;
			float	ar10=pal.ar10;
			float	ag10=pal.ag10;
			float	ab10=pal.ab10;
			float	dr10=pal.dr10*0.1f;
			float	dg10=pal.dg10*0.1f;
			float	db10=pal.db10*0.1f;
			float	n=pal.n*0.1f;
			for(i=0; i<8192; i++)
			{
				float	m=(float)sin(PI*(float)i*n)*0.4999f+0.5f;
				float	vb=(float)(sin(PI*(float)i*0.01f)*0.5f+0.5f);
				byte	r=(byte)(vb*((1.f-m)*((float)sin(ar1)*sin(ar10)*127.9f+128.f)+m*((float)sin(ar)*sin(ar0)*127.9f+128.f)))&255;
				byte	g=(byte)(vb*((1.f-m)*((float)sin(ag1)*sin(ag10)*127.9f+128.f)+m*((float)sin(ag)*sin(ag0)*127.9f+128.f)))&255;
				byte	b=0;//(byte)(vb*((1.f-m)*((float)sin(ab1)*sin(ab10)*127.9f+128.f)+m*((float)sin(ab)*sin(ab0)*127.9f+128.f)))&255;
				palette[i]=color32(r, g, b, 255);
				ar+=dr;
				ag+=dg;
				ab+=db;
				ar0+=dr0;
				ag0+=dg0;
				ab0+=db0;
				ar1+=dr1;
				ag1+=dg1;
				ab1+=db1;
				ar10+=dr10;
				ag10+=dg10;
				ab10+=db10;
			}
			pal.ar+=pal.dr*10.f;
			pal.ag+=pal.dg*10.f;
			pal.ab+=pal.db*10.f;
			pal.ar0+=pal.dr0*10.f;
			pal.ag0+=pal.dg0*10.f;
			pal.ab0+=pal.db0*10.f;
			pal.ar1+=pal.dr1*10.f;
			pal.ag1+=pal.dg1*10.f;
			pal.ab1+=pal.db1*10.f;
			pal.ar10+=pal.dr10*10.f;
			pal.ag10+=pal.dg10*10.f;
			pal.ab10+=pal.db10*10.f;
		}
		{
			dword	*d=back->body32;

			float	xx1=(float)sin(zyg.ax1)*pos.w;
			float	yy1=(float)sin(zyg.ay1)*pos.h;
			float	xx2=(float)sin(zyg.ax2)*pos.w;
			float	yy2=(float)sin(zyg.ay2)*pos.h;
			float	xx3=(float)sin(zyg.ax2)*pos.w;
			float	yy3=(float)sin(zyg.ay2)*pos.h;

			zyg.ax1+=zyg.dx1;
			zyg.ay1+=zyg.dy1;
			zyg.ax2+=zyg.dx2;
			zyg.ay2+=zyg.dy2;
			zyg.ax3+=zyg.dx3;
			zyg.ay3+=zyg.dy3;

			for(y=0; y<pos.h; y++)
			{
				float	dy1=(float)y-yy1;
				float	dy2=(float)y-yy2;
				float	dy3=(float)y-yy3;
				dy1*=dy1;
				dy2*=dy2;
				dy3*=dy3;
				for(x=0; x<pos.w; x++)
				{
					float	dx1=(float)x-xx1;
					float	dx2=(float)x-xx2;
					float	dx3=(float)x-xx3;
					*(d++)=palette[(int)(((float)sin(sqrt(dx1*dx1+dy1)*zyg.zz1) + (float)sin(sqrt(dx2*dx2+dy2)*zyg.zz2) + (float)sin(sqrt(dx2*dx2+dy2)*zyg.zz2) ) * (4095.f/3.f)+ 4096.f )];
				}
			}
		}
		back->boxfa(0,0,pos.w-1,pos.h-1,0xffffaa00,0.5f);
		back->boxa(0,0,pos.w-1, pos.h-1,0xff000000,0.8f);
		back->boxa(1,1,pos.w-2, pos.h-2,0xff000000,0.8f);
		back->boxa(2,2,pos.w-3, pos.h-3,0xff000000,0.8f);
		back->boxa(3,3,pos.w-4, pos.h-4,0xff000000,0.8f);
		back->boxa(4,4,pos.w-5, pos.h-5,0xff000000,0.8f);
		back->boxa(5,5,pos.w-6, pos.h-6,0xff000000,0.8f);
		{
			Afont	*font=alib.getFont(fontCONFIDENTIAL14);
			font->set(back, 10, 70, "live version " VERSION, 0xffffffff);
			font->set(back, 10, pos.h-136, "aestesis 1991,2009", 0xffffffff);
			font->set(back, 10, pos.h-176, "software developer YoY", 0xff000000);
			font->set(back, 10, pos.h-156, "(CC) RENAN JEGOUZO", 0xff808080);
		}
		repaint();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
