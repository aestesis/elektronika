/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	AUTOMIX.CPP					(c)	YoY'03						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<math.h>
#include						<assert.h>
#include						"resource.h"
#include						"automix.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								AautomixInfo::CI	= ACI("AautomixInfo",	GUID(0x11111112,0x00000360), &AeffectInfo::CI, 0, NULL);
ACI								Aautomix::CI		= ACI("Aautomix",		GUID(0x11111112,0x00000361), &Aeffect::CI, 0, NULL);
ACI								AautomixFront::CI	= ACI("AautomixFront",	GUID(0x11111112,0x00000362), &AeffectFront::CI, 0, NULL);
ACI								AautomixBack::CI	= ACI("AautomixBack",	GUID(0x11111112,0x00000363), &AeffectBack::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aautomix::Aautomix(QIID qiid, char *name, AeffectInfo *info, Acapsule *capsule) : Aeffect(qiid, name, info, capsule)
{
	image[0]=new Abitmap(getVideoWidth(), getVideoHeight());
	image[1]=new Abitmap(getVideoWidth(), getVideoHeight());
	front=new AautomixFront(qiid, "automix front", this, 48);
	front->setTooltips("automix");
	back=new AautomixBack(qiid, "automix back", this, 48);
	back->setTooltips("automix");
	settings(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aautomix::~Aautomix()
{
	delete(image[0]);
	delete(image[1]);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aautomix::load(class Afile *f)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aautomix::save(class Afile *f)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aautomix::settings(bool emergency)
{
	int	i;
	for(i=0; i<2; i++)
		image[i]->size(getVideoWidth(), getVideoHeight());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aautomix::action(double time, double dtime, double beat, double dbeat)
{
	AautomixFront	*front=(AautomixFront *)this->front;
	AautomixBack	*back=(AautomixBack *)this->back;
	Abitmap			*bb=back->out->getBitmap();
	if(bb)
	{
		Abitmap	*bmix=back->in[front->selmix->get()]->getBitmap();
		Abitmap	*bin[2];
		bin[0]=back->in[front->sel[0]->get()]->getBitmap();
		bin[1]=back->in[front->sel[1]->get()]->getBitmap();
		
		{
			int	i;
			for(i=0; i<2; i++)
			{
				if(bin[i])
				{
					int		x,y;
					int		w=bin[i]->w;
					int		h=bin[i]->h;
					dword	*s=bin[i]->body32;
					dword	*d=image[i]->body32;
					dword	color=front->color[i]->get();
					int		boost=front->boost[i]->get()?2:1;
					byte	fr,fg,fb;

					colorRGB(&fr, &fg, &fb, color);
					{
						int		mr=(int)fr*boost;
						int		mg=(int)fg*boost;
						int		mb=(int)fb*boost;

						for(y=0; y<h; y++)
							for(x=0; x<w; x++)
							{
								byte	r,g,b,a;
								colorRGBA(&r, &g, &b, &a, *(s++));
								(*d++)=color32(mini(((int)r*mr)>>8, 255), mini(((int)g*mg)>>8, 255), mini(((int)b*mb)>>8, 255), a);
							}
					}
				}
			}
		}

		if(bin[0])
			bin[0]=image[0];
		if(bin[1])
			bin[1]=image[1];

		if(front->inv[0]->get())
		{
			Abitmap	*b=bin[0];
			bin[0]=bin[1];
			bin[1]=b;
		}

		if(bmix)
		{
			int		tab[256];
			int		colormix=front->colormix->get();
			{
				float	fade=front->powermix->get();
				float	seuil=front->fademix->get();
				
				if(bin[1]&&(!bin[0]))	// que la source droite -> inverser
					seuil=1.f-seuil;

				float	dseuil=(float)fabs(seuil-0.5f)*2.f;
				float	idseuil=1.f-dseuil;
				int		i;
				
				for(i=0; i<256; i++)
				{
					float	fi=(float)i/255.f;
					float	ff=(seuil*(1.f+fade))-(fade*0.5f);
					float	a=((fi-seuil)*(fade*fade*100.f+1.f)+0.5f)*idseuil+((1.f-seuil)*dseuil);
					tab[i]=maxi(mini((int)(a*256.f), 256), 0);
				}

				if(front->inv[1]->get())
				{
					int	i;
					for(i=0; i<128; i++)
					{
						int	n=tab[i];
						tab[i]=tab[255-i];
						tab[255-i]=n;
					}
				}
			}
			
			if(bin[0]&&bin[1])
			{
				int	w=bb->w;
				int	h=bb->h;
				int	x,y;
				for(y=0; y<h; y++)
				{
					int		xx=0;
					int		adr=bb->adr[y];
					dword	*ss=&bmix->body32[adr];
					dword	*s0=&bin[0]->body32[adr];
					dword	*s1=&bin[1]->body32[adr];
					dword	*dd=&bb->body32[adr];

					switch(colormix)
					{
						case 0:	// red
						for(x=0; x<w; x++)
						{
							int		v=tab[(*(ss++)>>16)&255];
							int		v0=256-v;
							dword	c0=*(s0++);
							dword	c1=*(s1++);
							byte	rs0,gs0,bs0;
							byte	rs1,gs1,bs1;
							colorRGB(&rs0, &gs0, &bs0, c0);
							colorRGB(&rs1, &gs1, &bs1, c1);
							*(dd++)=color32(((int)rs0*v+(int)rs1*v0)>>8, ((int)gs0*v+(int)gs1*v0)>>8, ((int)bs0*v+(int)bs1*v0)>>8);
						}
						break;

						case 1:	// green
						for(x=0; x<w; x++)
						{
							int		v=tab[(*(ss++)>>8)&255];
							int		v0=256-v;
							dword	c0=*(s0++);
							dword	c1=*(s1++);
							byte	rs0,gs0,bs0;
							byte	rs1,gs1,bs1;
							colorRGB(&rs0, &gs0, &bs0, c0);
							colorRGB(&rs1, &gs1, &bs1, c1);
							*(dd++)=color32(((int)rs0*v+(int)rs1*v0)>>8, ((int)gs0*v+(int)gs1*v0)>>8, ((int)bs0*v+(int)bs1*v0)>>8);
						}
						break;

						case 2:	// blue
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
						break;
					}
				}
			}
			else if(bin[0]||bin[1])
			{
				int		n=bin[0]?0:1;
				Abitmap	*b=bin[n];
				int		w=bb->w;
				int		h=bb->h;
				int		x,y;
				
				for(y=0; y<h; y++)
				{
					int		xx=0;
					int		adr=bb->adr[y];
					dword	*ss=&bmix->body32[adr];
					dword	*s0=&b->body32[adr];
					dword	*dd=&bb->body32[adr];

					switch(colormix)
					{
						case 0:	// red
						for(x=0; x<w; x++)
						{
							int		v=tab[(*(ss++)>>16)&255];
							int		v0=256-v;
							dword	c0=*(s0++);
							byte	rs0,gs0,bs0;
							colorRGB(&rs0, &gs0, &bs0, c0);
							*(dd++)=color32(((int)rs0*v)>>8, ((int)gs0*v)>>8, ((int)bs0*v)>>8);
						}
						break;

						case 1:	// green
						for(x=0; x<w; x++)
						{
							int		v=tab[(*(ss++)>>8)&255];
							int		v0=256-v;
							dword	c0=*(s0++);
							byte	rs0,gs0,bs0;
							colorRGB(&rs0, &gs0, &bs0, c0);
							*(dd++)=color32(((int)rs0*v)>>8, ((int)gs0*v)>>8, ((int)bs0*v)>>8);
						}
						break;

						case 2:	// blue
						for(x=0; x<w; x++)
						{
							int		v=tab[*(ss++)&255];
							int		v0=256-v;
							dword	c0=*(s0++);
							byte	rs0,gs0,bs0;
							colorRGB(&rs0, &gs0, &bs0, c0);
							*(dd++)=color32(((int)rs0*v)>>8, ((int)gs0*v)>>8, ((int)bs0*v)>>8);
						}
						break;
					}
				}
			}
			else
				bb->boxf(0, 0, bb->w, bb->h, 0xff000000);
		}
		else
		{
			float	v[2];
			int		i;
			v[1]=front->fademix->get();
			v[0]=1.f-v[1];
			bb->boxf(0, 0, bb->w, bb->h, 0xff000000);
			for(i=0; i<2; i++)
			{
				if(bin[i])
				{
					dword	state=bin[i]->state;
					dword	flags=bin[i]->flags;
					float	alpha=bin[i]->alpha;
					bin[i]->state=bitmapNORMAL;
					bin[i]->flags=bitmapGLOBALALPHA;
					bin[i]->alpha=v[i];
					bb->set(0, 0, bin[i], bitmapDEFAULT, bitmapDEFAULT);
					bin[i]->state=state;
					bin[i]->flags=flags;
					bin[i]->alpha=alpha;
				}
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
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AautomixFront::AautomixFront(QIID qiid, char *name, Aautomix *e, int h) : AeffectFront(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_AUTOMIX_FRONT), "PNG");
	back=new Abitmap(&o);

	selmix=new Aselect(MKQIID(qiid, 0xcca8967366438350), "source mixer", this, 220, 10, 4, 1, &resource.get(MAKEINTRESOURCE(PNG_VMIX_SOURCES), "PNG"), 16, 16);
	selmix->setTooltips("source mixer");
	selmix->set(1);
	selmix->show(TRUE);

	colormix=new Aselect(MKQIID(qiid, 0xa0f67698be078000), "source channel", this, 241, 32, 3, 1, &resource.get(MAKEINTRESOURCE(PNG_AUTOMIX_COLOR), "PNG"), 8, 8);
	colormix->setTooltips("source channel");
	colormix->set(1);
	colormix->show(TRUE);

	sel[0]=new Aselect(MKQIID(qiid, 0x6b0115d646e21500), "[1]/source", this, 40, 10, 4, 1, &resource.get(MAKEINTRESOURCE(PNG_VMIX_SOURCES), "PNG"), 16, 16);
	sel[0]->setTooltips("source 1");
	sel[0]->set(0);
	sel[0]->show(TRUE);
	
	color[0]=new Aselcolor(MKQIID(qiid, 0x8e02fb6a5ff2e4c0), "[1]/color", this, 120, 10);	
	color[0]->setTooltips("color 1");
	color[0]->set(0xffffffff);
	color[0]->show(true);

	boost[0]=new ActrlButton(MKQIID(qiid, 0x3c97e89a54e7c900), "[1]/color boost", this, 144, 10, 8, 16, &resource.get(MAKEINTRESOURCE(PNG_AUTOMIX_BUTTON2), "PNG"), Abutton::btBITMAP|Abutton::bt2STATES);
	boost[0]->setTooltips("color boost 1");
	boost[0]->show(TRUE);
	
	sel[1]=new Aselect(MKQIID(qiid, 0x2e43d2f4c9b44800), "[2]/source", this, 402, 10, 4, 1, &resource.get(MAKEINTRESOURCE(PNG_VMIX_SOURCES), "PNG"), 16, 16);
	sel[1]->setTooltips("source 2");
	sel[1]->set(2);
	sel[1]->show(TRUE);

	color[1]=new Aselcolor(MKQIID(qiid, 0xfd20251901f01420), "[2]/color", this, 376, 10);
	color[1]->setTooltips("color 2");
	color[1]->set(0xffffffff);
	color[1]->show(true);

	boost[1]=new ActrlButton(MKQIID(qiid, 0xf7d5111bf4c1ba04), "[2]/color boost", this, 360, 10, 8, 16, &resource.get(MAKEINTRESOURCE(PNG_AUTOMIX_BUTTON2), "PNG"), Abutton::btBITMAP|Abutton::bt2STATES);
	boost[1]->setTooltips("color boost 2");
	boost[1]->show(TRUE);
	
	fademix=new Apaddle(MKQIID(qiid, 0x658141850e6ba822), "fade mix", this, 172, 4, paddleYOLI32);
	fademix->setTooltips("fade mix");
	fademix->set(0.5f);
	fademix->show(TRUE);

	powermix=new Apaddle(MKQIID(qiid, 0xec15c007f02a1b00), "power mix", this, 204, 28, paddleYOLI16);
	powermix->setTooltips("power mix");
	powermix->set(0.5f);
	powermix->show(TRUE);
	
	inv[0]=new ActrlButton(MKQIID(qiid, 0x5adeca6ddd847300), "source switch", this, 302, 10, 8, 16, &resource.get(MAKEINTRESOURCE(PNG_AUTOMIX_BUTTON2), "PNG"), Abutton::btBITMAP|Abutton::bt2STATES);
	inv[0]->setTooltips("source switch");
	inv[0]->show(TRUE);
	
	inv[1]=new ActrlButton(MKQIID(qiid, 0x496947d63f5fd49c), "effect switch", this, 316, 10, 8, 16, &resource.get(MAKEINTRESOURCE(PNG_AUTOMIX_BUTTON), "PNG"), Abutton::btBITMAP|Abutton::bt2STATES);
	inv[1]->setTooltips("effect switch");
	inv[1]->show(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AautomixFront::~AautomixFront()
{
	delete(colormix);
	delete(fademix);
	delete(powermix);
	delete(selmix);
	delete(sel[0]);
	delete(sel[1]);
	delete(color[0]);
	delete(color[1]);
	delete(boost[0]);
	delete(boost[1]);
	delete(inv[0]);
	delete(inv[1]);
	delete(back);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AautomixFront::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AautomixFront::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCHANGE:
		break;
	}
	return AeffectFront::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AautomixBack::AautomixBack(QIID qiid, char *name, Aautomix *e, int h) : AeffectBack(qiid, name, e, h)
{
	static QIID qiidin[]={ 0x244d64e5877aee40, 0x32c20d794c0653c0, 0x94be5637e625b2d0, 0x6f612bc4c1beb610};
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_AUTOMIX_BACK), "PNG");
	int		i;
	back=new Abitmap(&o);

	for(i=0; i<4; i++)
	{
		char	str[128];
		sprintf(str, "video in #%d", i);
		in[i]=new Avideo(MKQIID(qiid, qiidin[i]), str, this, pinIN, 10+20*i, 10);
		in[i]->setTooltips(str);
		in[i]->show(TRUE);
	}
	
	out=new Avideo(MKQIID(qiid, 0xd053d1ef3e3fddc0), "video out", this, pinOUT, pos.w-18, 10);
	out->setTooltips("video out");
	out->show(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AautomixBack::~AautomixBack()
{
	int	i;
	delete(back);
	for(i=0; i<4; i++)
		delete(in[i]);
	delete(out);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AautomixBack::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aeffect * AautomixInfo::create(QIID qiid, char *name, Acapsule *capsule)
{
	return new Aautomix(qiid, name, this, capsule);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aplugz * automixGetInfo()
{
	return new AautomixInfo("automixInfo", &Aautomix::CI, "automix", "automix");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
