/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	TRANS-SLICE.CPP				(c)	YoY'01						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"trans-slice.h"
#include						"resource.h"
#include						<io.h>
#include						<math.h>
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								AtranssliceInfo::CI	= ACI("AtranssliceInfo",	GUID(0x11111112,0xA0000020), &AtransitionInfo::CI, 0, NULL);
ACI								Atransslice::CI		= ACI("Atransslice",		GUID(0x11111112,0xA0000021), &Atransition::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define							X								2
#define							Y								2

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int						count=0;
static Abitmap					*slice;
static char						dir[128][ALIB_PATH];
static int						ndir=0;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void init(char *path)
{
	if(!count)
	{
		slice=new Abitmap(&resource.get(MAKEINTRESOURCE(PNG_SLICE_AESTESIS), "PNG"));
		
		if(ndir==0)
		{
			struct _finddata_t		fd;
			char					rpath[ALIB_PATH];
			strcpy(rpath, path);
			strcat(rpath, "\\wipes\\*");
			long					h=_findfirst(rpath, &fd);
			if(h!=-1)
			{
				int		r=0;
				while(r!=-1)
				{
					char		str[ALIB_PATH];
					sprintf(str, "%s\\wipes\\%s", path, fd.name);
					if(fd.attrib&_A_SUBDIR)
					{
						if((ndir<128)&&strcmp(fd.name, ".")&&strcmp(fd.name, ".."))
							strcpy(dir[ndir++], str);
					}
					r=_findnext(h, &fd);
				}
				_findclose(h);
			}
		}
	}
	count++;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void end()
{
	count--;
	if(count==0)
	{
		delete(slice);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Asnap : public AcontrolObj
{
public:
	AOBJ

								Asnap							(QIID qiid, char *name, Aobject *L, int x, int y, int w, int h);
	virtual						~Asnap							();

	virtual void				pulse							();
	virtual bool				mouse							(int x, int y, int state, int event);
	virtual bool				keyboard						(int event, int ascii, int scan, int state);

	virtual bool				sequence						(int nctrl, float value);
	
	virtual void				setBitmap						(int n, Abitmap *b);

	virtual void				set								(int p);
	virtual int					get								();

	virtual void				paint							(Abitmap *b);


private:
	Abitmap						*bmp[16];
	int							ipos;
	int							defpos;
	
	bool						ctrlPaint;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Asnap::CI		= ACI("Asnap",	GUID(0x11111112,0xA0000022), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Asnap::Asnap(QIID qiid, char *name, Aobject *L, int x, int y, int w, int h) : AcontrolObj(name, L, x, y, w, h)
{
	int	i;
	for(i=0; i<16; i++)
		bmp[i]=new Abitmap(24, 14);
	control=new Acontrol(qiid, name, Aeffect::getEffect(L), this, Acontrol::KEY_16);
	ctrlPaint=false;
	set(0);
	defpos=-1;
	timer(100);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Asnap::~Asnap()
{
	int	i;
	delete(control);
	for(i=0; i<16; i++)
		delete(bmp[i]);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Asnap::keyboard(int event, int ascii, int scan, int state)
{
	switch(event)
	{
		case keyboardDOWN:
		if(state&keyboardALT)
		{
			if(defpos!=-1)
				set(defpos);
			else
				set(0);
			father->notify(this, nyCHANGE);
			return true;
		}
		break;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Asnap::mouse(int x, int y, int state, int event)
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
				if(defpos!=-1)
					set(defpos);
				else
					set(0);
				father->notify(this, nyCHANGE);
				return true;
			}
		}
		switch(event)
		{
			case mouseLDOWN:
			mouseCapture(true);
			return true;
			
			case mouseLUP:
			{
				int	xx=(x-X)/28;
				int	yy=(y-Y)/18;
				if((xx>=0)&&(xx<4)&&(yy>=0)&&(yy<4))
				{
					set(yy*4+xx);
					father->notify(this, nyCHANGE);
					repaint();
				}
				mouseCapture(false);
				return TRUE;
			}
			break;
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Asnap::set(int v)
{
	ipos=maxi(mini(v, 15), 0);
	if(defpos==-1)
		defpos=ipos;
	control->set(Acontrol::KEY, (float)ipos);
	repaint();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Asnap::get()
{
	return ipos;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Asnap::sequence(int nctrl, float value)
{
	ipos=(int)value;
	ipos=maxi(mini(ipos, 15), 0);
	ctrlPaint=true;
	father->notify(this, nyCHANGE);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Asnap::setBitmap(int n, Abitmap *b)
{
	bmp[n]->set(0, 0, bmp[n]->w, bmp[n]->h, 0, 0, b->w, b->h, b, bitmapNORMAL, bitmapNORMAL);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Asnap::pulse()
{
	if(ctrlPaint)
	{
		repaint();
		ctrlPaint=false;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Asnap::paint(Abitmap *b)
{
	{
		int	n=0;
		int	x,y;
		for(y=0; y<4; y++)
		{
			int	yy=Y+y*18;
			for(x=0; x<4; x++)
			{
				int	xx=X+x*28;
				b->set(xx, yy, bmp[n], bitmapNORMAL, bitmapNORMAL);
				n++;
			}
		}
	}
	{
		int	x=X+(ipos&3)*28;
		int	y=Y+(ipos>>2)*18;
		b->boxfa(x, y, x+23, y+13, 0xffffff00, 0.3f);
		b->box(x-1, y-1, x+24, y+14, 0xffffff00);
		b->box(x-2, y-2, x+25, y+15, 0xff808000);
	}
	controlPaint(b, pos.w, pos.h);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Atransslice::Atransslice(QIID qiid, char *name, Aobject *f, int x, int y) : Atransition(qiid, name, f, x, y)
{
	Atable *table=Atable::getTable(this);
	char	str[1024];
	int		i;
	init(table->rootdir);
	
	{
		Aeffect *e=Aeffect::getEffect(this);
		int		w=e->getVideoWidth();
		int		h=e->getVideoHeight();
		sources=new Abitmap(w*2, h);
		wipe=new Abitmap(w, h);
		transform=new Abitmap(w, h);
	}

	bChange=false;
	
	for(i=0; i<16; i++)
		bmp[i]=new Abitmap(4, 4);
	
	nwipe=-1;
	
	items=new Aitem("items", "items");
	{
		int	i;
		for(i=0; i<ndir; i++)
		{
			char *s=strrchr(dir[i], '\\')+1;
			if(s)
				new Aitem(s, s, items, i);
		}
	}
	
	list=new Alist("wipes", this, 4, 78, pos.w-42, 14, items);
	list->setTooltips("wipes");
	list->show(true);
	list->setCurrentByData(0);
	
	sprintf(str, "%s/flip X", name);
	flipx=new ActrlButton(MKQIID(qiid, 0x8f0456a12c039bcc), str, this, 86, 78, 14, 14, &resource.get(MAKEINTRESOURCE(PNG_VMIX_FLIPX), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	flipx->setTooltips("flip X");
	flipx->set(false);
	flipx->show(true);
	
	sprintf(str, "%s/flip Y", name);
	flipy=new ActrlButton(MKQIID(qiid, 0x8f0456a12c039bad), str, this, 102, 78, 14, 14, &resource.get(MAKEINTRESOURCE(PNG_VMIX_FLIPY), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	flipy->setTooltips("flip Y");
	flipy->set(false);
	flipy->show(true);
	
	sprintf(str, "%s/mask", name);
	snap=new Asnap(MKQIID(qiid, 0x5d286623dbee24c0), str, this, 4, 4, pos.w-8, 72);
	snap->setTooltips("mask select");
	snap->set(0);
	snap->show(TRUE);
	
	change();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Atransslice::~Atransslice()
{
	int	i;
	for(i=0; i<16; i++)
		delete(bmp[i]);
	delete(list);
	delete(items);
	delete(wipe);
	delete(transform);
	delete(snap);
	delete(flipx);
	delete(flipy);
	delete(sources);
	end();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Atransslice::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCHANGE:
		if(o==list)
		{
			change();
			snap->repaint();
		}
		else if((o==flipx)||(o==flipy))
		{
			bChange=true;
		}
		break;
	}
	return Aobject::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Atransslice::paint(Abitmap *b)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Atransslice::mouse(int x, int y, int state, int event)
{
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Atransslice::save(class Afile *f)
{
	int		n=list->getCurrentData();
	char	*s=strrchr(dir[n], '\\');
	f->writeString(s);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Atransslice::load(class Afile *f)
{
	char	file[ALIB_PATH];
	int		i;
	int		c=0;
	f->readString(file);
	for(i=0; i<ndir; i++)
	{
		char	*s=strrchr(dir[i], '\\');
		if(!strcmp(s, file))
		{
			c=i;
			break;
		}
	}
	list->setCurrentByData(c);
	change();
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Atransslice::settings(bool emergency)
{
	{
		Aeffect *e=Aeffect::getEffect(this);
		int		w=e->getVideoWidth();
		int		h=e->getVideoHeight();
		sources->size(w*2, h);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Atransslice::change()
{
	int		n=list->getCurrentData();
	char	*path=dir[n];
	{
		struct _finddata_t		fd;
		char					rpath[ALIB_PATH];
		strcpy(rpath, path);
		strcat(rpath, "\\*.png");
		long					h=_findfirst(rpath, &fd);
		if(h!=-1)
		{
			int		r=0;
			int		nb=0;
			while(r!=-1)
			{
				char		str[ALIB_PATH];
				sprintf(str, "%s\\%s", path, fd.name);
				if(nb<16)
				{
					section.enter(__FILE__,__LINE__);
					bmp[nb]->load(str);
					snap->setBitmap(nb, bmp[nb]);
					nb++;
					section.leave();
				}
				r=_findnext(h, &fd);
			}
			_findclose(h);

			section.enter(__FILE__,__LINE__);
			while(nb<16)
			{
				Abitmap	*b=slice;
				bmp[nb]->size(b->w, b->h);
				bmp[nb]->set(0, 0, b, bitmapNORMAL, bitmapNORMAL);
				snap->setBitmap(nb, bmp[nb]);
				nb++;
			}
			section.leave();
		}
	}
	bChange=true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Atransslice::action(Abitmap *out, Abitmap *b0, Abitmap *b1, float cross, int src, float power, Abitmap *in[8])
{
	int		sslice=snap->get();
	int		w=out->w;
	int		h=out->h;
	int		tab[256];
	int		i;
	
	if((wipe->w!=w)||(wipe->h!=h)||(sslice!=nwipe)||bChange)
	{
		bool	flipx=this->flipx->get();
		bool	flipy=this->flipy->get();
		Abitmap	*t=bmp[sslice];
		section.enter(__FILE__,__LINE__);
		wipe->size(w, h);
		if((wipe->w!=t->w)||(wipe->h!=t->h))
		{
			transform->size(t->w, t->h);
			transform->set(0, 0, t, bitmapNORMAL, bitmapNORMAL);
			wipe->stretch(transform);
		}
		else
			wipe->set(0, 0, t, bitmapNORMAL, bitmapNORMAL);
		if(flipx)
			wipe->flipX();
		if(flipy)
			wipe->flipY();
		section.leave();
		nwipe=sslice;
		bChange=false;
	}
	
	if(power<0.5f)
	{
		bool	bInv=(src!=0);
		float	fade=power*2.f;
		float	seuil=cross;
		float	dseuil=(float)fabs(seuil-0.5f)*2.f;
		float	idseuil=1.f-dseuil;
		float	corec=(float)256-(float)fade;
		
		for(i=0; i<256; i++)
		{
			float	fi=(float)i/255.f;
			float	ff=(seuil*(1.f+fade))-(fade*0.5f);
			float	a=((fi-seuil)*(fade*fade*100.f+1.f)+0.5f)*idseuil+((1.f-seuil)*dseuil);
			tab[i]=maxi(mini((int)(a*256.f), 256), 0);
		}
		
		if(bInv)
		{
			int	i;
			for(i=0; i<128; i++)
			{
				int	n=tab[i];
				tab[i]=tab[255-i];
				tab[255-i]=n;
			}
		}

		if(b0&&b1)
		{
			int	x,y;
			for(y=0; y<h; y++)
			{
				dword	*ss=&wipe->body32[wipe->adr[y]];
				dword	*s0=&b0->body32[b0->adr[y]];
				dword	*s1=&b1->body32[b1->adr[y]];
				dword	*dd=&out->body32[out->adr[y]];

				for(x=0; x<w; x++)
				{
					int		v=tab[*(ss++)&255];
					int		v0=256-v;
					dword	c0=*(s0++);
					dword	c1=*(s1++);
					byte	rs0,gs0,bs0;
					byte	rs1,gs1,bs1;
					colorRGB(&rs0, &gs0, &bs0, c0);
					colorRGB(&rs1, &gs1, &bs1, c1);
					*(dd++)=color32(((int)rs0*v+(int)rs1*v0)>>8, ((int)gs0*v+(int)gs1*v0)>>8, ((int)bs0*v+(int)bs1*v0)>>8);
				}
			}
		}
		else if(b0)
		{
			int	x,y;
			for(y=0; y<h; y++)
			{
				dword	*ss=&wipe->body32[wipe->adr[y]];
				dword	*s0=&b0->body32[b0->adr[y]];
				dword	*dd=&out->body32[out->adr[y]];

				for(x=0; x<w; x++)
				{
					int		v=tab[*(ss++)&255];
					int		v0=256-v;
					dword	c0=*(s0++);
					byte	rs0,gs0,bs0;
					colorRGB(&rs0, &gs0, &bs0, c0);
					*(dd++)=color32(((int)rs0*v)>>8, ((int)gs0*v)>>8, ((int)bs0*v)>>8);
				}
			}
		}
		else if(b1)
		{
			int	x,y;
			for(y=0; y<h; y++)
			{
				dword	*ss=&wipe->body32[wipe->adr[y]];
				dword	*s1=&b1->body32[b1->adr[y]];
				dword	*dd=&out->body32[out->adr[y]];

				for(x=0; x<w; x++)
				{
					int		v=tab[*(ss++)&255];
					int		v0=256-v;
					dword	c1=*(s1++);
					byte	rs1,gs1,bs1;
					colorRGB(&rs1, &gs1, &bs1, c1);
					*(dd++)=color32(((int)rs1*v0)>>8, ((int)gs1*v0)>>8, ((int)bs1*v0)>>8);
				}
			}
		}
		else
			out->boxf(0, 0, w, h, 0xff000000);
	}
	else
	{
		bool	bInv=(src!=0);
		float	fade=1.f-((power-0.5f)*2.f);
		float	seuil=bInv?(1.f-cross):cross;
		float	dseuil=(float)fabs(seuil-0.5f)*2.f;
		float	idseuil=1.f-dseuil;
		float	corec=(float)256-(float)fade;
		Abitmap	*b=sources;
		int		x,y;
		int		x0,x1;
		
		if(bInv)
		{
			x0=0;
			x1=out->w;
		}
		else
		{
			x0=out->w;
			x1=0;
		}
		
		for(i=0; i<256; i++)
		{
			float	fi=(float)i/255.f;
			float	ff=(seuil*(1.f+fade))-(fade*0.5f);
			float	a=((fi-seuil)*(fade*fade*100.f+1.f)+0.5f)*idseuil+((1.f-seuil)*dseuil);
			tab[i]=maxi(mini((int)(a*(float)out->w), out->w), 0);
		}
		
		if(b0)
			b->set(x0, 0, b0, bitmapNORMAL, bitmapNORMAL);
		else
			b->boxf(x0, 0, out->w, out->h, 0xff000000);
		if(b1)
			b->set(x1, 0, b1, bitmapNORMAL, bitmapNORMAL);
		else
			b->boxf(x1, 0, out->w*2, out->h, 0xff000000);
		
		for(y=0; y<h; y++)
		{
			dword	*ss=&wipe->body32[wipe->adr[y]];
			dword	*s0=&b->body32[b->adr[y]];
			dword	*dd=&out->body32[out->adr[y]];

			for(x=0; x<w; x++)
			{
				int		v=tab[*(ss++)&255];
				*(dd++)=s0[x+v];
			}
		}
	}
	

/* morph	
		case 1:
		for(i=0; i<256; i++)
		{
			float	fi=(float)i/255.f;
			float	ff=(seuil*(1.f+fade))-(fade*0.5f);
			float	a=((fi-seuil)*(fade*fade*100.f+1.f)+0.5f)*idseuil+((1.f-seuil)*dseuil);
			tab[i]=maxi(mini((int)(a*(float)out->w), out->w), 0);
		}
		if(b0&&b1)
		{
			int	x,y;
			Abitmap	b=new Abitmap(out->w*2, out->h);
			b->set(0, 0, b0, bitmapNORMAL, bitmapNORMAL);
			b->set(out->w, 0, b1, bitmapNORMAL, bitmapNORMAL);
			for(y=0; y<h; y++)
			{
				dword	*ss=&wipe->body32[wipe->adr[y]];
				dword	*s0=&b->body32[b->adr[y]];
				dword	*dd=&out->body32[out->adr[y]];

				for(x=0; x<w; x++)
				{
					int		v=tab[*(ss++)&255];
					*(dd++)=s0[x+v];
				}
			}
			delete(b);
		}
		break;
	}
*/
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Atransition * AtranssliceInfo::create(QIID qiid, char *name, Aobject *father, int x, int y)
{
	return new Atransslice(qiid, name, father, x, y);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aplugz * transsliceGetInfo()
{
	return new AtranssliceInfo("wipe", &Atransslice::CI, "wipe", "wipe");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
