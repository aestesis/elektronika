/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	COLOR.CPP					(c)	YoY'03						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<math.h>
#include						<assert.h>
#include						"resource.h"
#include						"color.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								AcolorInfo::CI	= ACI("AcolorInfo",		GUID(0x11111112,0x00000370), &AeffectInfo::CI, 0, NULL);
ACI								Acolor::CI		= ACI("Acolor",			GUID(0x11111112,0x00000371), &Aeffect::CI, 0, NULL);
ACI								AcolorFront::CI	= ACI("AcolorFront",	GUID(0x11111112,0x00000372), &AeffectFront::CI, 0, NULL);
ACI								AcolorBack::CI	= ACI("AcolorBack",		GUID(0x11111112,0x00000373), &AeffectBack::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static dword stacol[6][5]={
						{0xff000000, 0xff003000, 0xffff0000, 0xffffff00, 0xffffffff},
						{0xff100000, 0xff504040, 0xff908080, 0xffd0c0c0, 0xffffffff},
						{0xff000000, 0xff404000, 0xff008000, 0xffc01080, 0xffffffff},
						{0xffffffff, 0xff8000c0, 0xff400080, 0xff000040, 0xff000000},
						{0xffffffff, 0xffc0c0c0, 0xff808080, 0xff404040, 0xff000000},
						{0xffff0000, 0xffc0c000, 0xff808080, 0xff400040, 0xff000000} };

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Acolor::Acolor(QIID qiid, char *name, AeffectInfo *info, Acapsule *capsule) : Aeffect(qiid, name, info, capsule)
{
	{
		int	i,j;
		for(i=0; i<6; i++)
			for(j=0; j<5; j++)
				colors[i][j]=stacol[i][j];
	}
	image=new Abitmap(getVideoWidth(), getVideoHeight());
	front=new AcolorFront(qiid, "color front", this, 48);
	front->setTooltips("color module");
	back=new AcolorBack(qiid, "color back", this, 48);
	back->setTooltips("color module");
	settings(false);
	calcPal();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Acolor::~Acolor()
{
	delete(image);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Acolor::load(class Afile *f)
{
	f->read(colors, sizeof(colors));
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Acolor::save(class Afile *f)
{
	f->write(colors, sizeof(colors));
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Acolor::loadPreset(class Afile *f)
{
	return load(f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Acolor::savePreset(class Afile *f)
{
	return save(f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Acolor::settings(bool emergency)
{
	image->size(getVideoWidth(), getVideoHeight());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Acolor::action(double time, double dtime, double beat, double dbeat)
{
	AcolorFront		*front=(AcolorFront *)this->front;
	AcolorBack		*back=(AcolorBack *)this->back;
	Abitmap			*bb=back->out->getBitmap();
	Asample			*audio=back->audio;
	float			palpow=front->palpow->get();
	bool			paletize=(palpow>0.f);
	float			psound=front->sound->get();;
	float			signal[3];
	float			ipsound;
	
	memset(signal, 0, sizeof(signal));
	
	ipsound=1.f-mini(psound*2.f, 1.f);
	psound=mini(psound*2.f, 1.f)+maxi(0.f, psound-0.5f)*20.f;
	
	if(audio->isConnected())
	{
		sword	smp[1024*2];
		sword	bas[1024*2];
		sword	med[1024*2];
		sword	tre[1024*2];
		int		mbas=0;
		int		mmed=0;
		int		mtre=0;
		int		avaible;
		int		sget;
		int		i;

		audio->enter(__FILE__,__LINE__);
		avaible=audio->avaible();
		avaible-=audio->skip(maxi((avaible-4096)>>1, 0));
		sget=audio->getBuffer(smp, bas, med, tre, 1024);
		audio->leave();

		for(i=0; i<(sget<<1); i++)
		{
			mbas=max(mbas, abs((int)bas[i]));
			mmed=max(mmed, abs((int)med[i]));
			mtre=max(mtre, abs((int)tre[i]));
		}
		
		signal[0]=psound*(float)mbas/32768.f;
		signal[1]=psound*(float)mmed/32768.f;
		signal[2]=psound*(float)mtre/32768.f;
	}
	
	if(bb)
	{
		Abitmap	*bin=back->in->getBitmap();
		if(bin)
		{
			{	// contrast // luminosity // color // audio
				Abitmap	*buf=paletize?image:bb;
				float	contrast=front->contrast->get();
				float	luminosity;
				int		i;
				int		n=buf->w*buf->h;
				dword	*s=bin->body32;
				dword	*d=buf->body32;
				int		mr,mg,mb;
				int		icontrast,iluminosity;
				int		iipsound=(int)(ipsound*256.f);
				int		isignal[3];
				float	gamma=front->levels[2]->get();
				float	start=front->levels[0]->get();
				float	end=front->levels[1]->get();
				gamma=(gamma<0.5f)?(gamma*2.f):(1.f+(gamma-0.5f)*6.f);
				int		gam=(int)(maxi(-65536.f, mini(65536.f, 255.f*gamma/(end-start))));
				int		sta=(int)(start*65536.f);
				
				isignal[0]=(int)(signal[0]*256.f);
				isignal[1]=(int)(signal[1]*256.f);
				isignal[2]=(int)(signal[2]*256.f);
				
				contrast=(contrast<0.5f)?contrast*2.f:((contrast-0.5f)*6.f+1.f);
				icontrast=(int)(contrast*256.f);
				luminosity=(front->luminosity->get()-0.5f)*(contrast+1.f);
				iluminosity=(int)(luminosity*65536.f);
				
				{
					byte	r,g,b;
					colorRGB(&r, &g, &b, front->color->get());
					mr=(int)r+1;
					mg=(int)g+1;
					mb=(int)b+1;
				}


				
				for(i=0; i<n; i++)
				{
					byte	r,g,b;
					colorRGB(&r, &g, &b, *(s++));
					int		fr=(int)r;
					int		fg=(int)g;
					int		fb=(int)b;
					
					fr=((fr-128)*icontrast)+32768;	// 1<<8==1.f
					fg=((fg-128)*icontrast)+32768;
					fb=((fb-128)*icontrast)+32768;

					fr+=iluminosity;
					fg+=iluminosity;
					fb+=iluminosity;

					fr=(fr*mr)>>8;	// 1<<16==1.f
					fg=(fg*mg)>>8;
					fb=(fb*mb)>>8;
					
					fr=(fr*iipsound)+(fr*isignal[0]);	// 1<<24==1.f
					fg=(fg*iipsound)+(fg*isignal[1]);
					fb=(fb*iipsound)+(fb*isignal[2]);

					fr>>=8;	// 1<<16==1.f
					fg>>=8;
					fb>>=8;
					
					fr=(fr-sta)*gam;
					fg=(fg-sta)*gam;
					fb=(fb-sta)*gam;
					
					r=(byte)maxi(mini(fr>>16, 255), 0);
					g=(byte)maxi(mini(fg>>16, 255), 0);
					b=(byte)maxi(mini(fb>>16, 255), 0);
					*(d++)=color32(r, g, b);
				}
			}	
	
			if(paletize)
			{
				int	chan=front->channel->get();
				section.enter(__FILE__,__LINE__);
				{
					dword	*s=image->body32;
					dword	*d=bb->body32;
					int		n=bb->w*bb->h;
					int		i;
					switch(chan)
					{
						case 0:	// red
						for(i=0; i<n; i++)
							*(d++)=pal[(*(s++)>>16)&0xff];
						break;

						case 1:	// green
						for(i=0; i<n; i++)
							*(d++)=pal[(*(s++)>>8)&0xff];
						break;

						case 2:	// blue
						for(i=0; i<n; i++)
							*(d++)=pal[*(s++)&0xff];
						break;

						case 3: // all
						for(i=0; i<n; i++)
						{
							int	v=( (*s&0xff) + ((*s>>8)&0xff)*2 +((*s>>16)&0xff) )>>2;
							s++;
							*(d++)=pal[v];
						}
						break;
					}
				}
				if(palpow<1.f)
				{
					float	a=image->alpha;
					image->alpha=1.f-palpow;
					bb->set(0, 0, image, bitmapNORMAL, bitmapGLOBALALPHA);
					image->alpha=a;
				}
				section.leave();
			}
		}
		else
			bb->boxf(0, 0, bb->w, bb->h, 0xff000000);
	}	
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Acolor::calcPal()
{
	AcolorFront		*front=(AcolorFront *)this->front;
	dword			co[5];
	int				i;
	
	for(i=0; i<5; i++)
		co[i]=front->colpal[i]->get();
		
	section.enter(__FILE__,__LINE__);
	for(i=0; i<4; i++)
	{
		int		pd=255*i/4;
		int		pf=255*(i+1)/4;
		int		pn=1+pf-pd;
		int		r,g,b;
		int		dr,dg,db;
		int		j;
		
		{
			byte	br,bg,bb;
			colorRGB(&br, &bg, &bb, co[i]);
			r=br<<16;
			g=bg<<16;
			b=bb<<16;
			colorRGB(&br, &bg, &bb, co[i+1]);
			dr=br<<16;
			dg=bg<<16;
			db=bb<<16;
			dr=(dr-r)/pn;
			dg=(dg-g)/pn;
			db=(db-b)/pn;
		}
		
		for(j=pd; j<pf+1; j++)
		{
			pal[j]=color32((byte)(r>>16), (byte)(g>>16), (byte)(b>>16));
			r+=dr;
			g+=dg;
			b+=db;
		}
	}
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AcolorFront::AcolorFront(QIID qiid, char *name, Acolor *e, int h) : AeffectFront(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_COLOR_FRONT), "PNG");
	int		i;
	back=new Abitmap(&o);

	contrast=new Apaddle(MKQIID(qiid, 0xbfbca4e7ad156640), "contrast", this, 10, 14, paddleYOLI24);
	contrast->setTooltips("contrast");
	contrast->set(0.5f);
	contrast->show(TRUE);
	
	luminosity=new Apaddle(MKQIID(qiid, 0x155f56ad69a655d7), "luminosity", this, 36, 26, paddleYOLI16);
	luminosity->setTooltips("luminosity");
	luminosity->set(0.5f);
	luminosity->show(TRUE);
	
	color=new Aselcolor(MKQIID(qiid, 0x8b619442ed8e86f8), "color", this, 66, 18);
	color->setTooltips("color");
	color->border=0xff343856;
	color->set(0xffffffff);
	color->show(true);

	sound=new Apaddle(MKQIID(qiid, 0x002a53469a6c98d8), "audio level", this, 94, 14, paddleYOLI24);
	sound->setTooltips("audio level");
	sound->set(0.f);
	sound->show(TRUE);

	static char *lvln[3]={ "start", "end", "gamma" };
	static float lvl[3]={ 0.f, 1.f, 0.5f };
	for(i=0; i<3; i++)
	{
		levels[i]=new Apaddle(MKQIID(qiid, 0xbfbc52651e156640+i*0x516515), lvln[i], this, 148+i*30+(i&6)*8, 14, paddleYOLI24);
		levels[i]->setTooltips(lvln[i]);
		levels[i]->set(lvl[i]);
		levels[i]->show(TRUE);
	}

	palpow=new Apaddle(MKQIID(qiid, 0x255a1c52e8660b40), "palette level", this, 284, 14, paddleYOLI24);
	palpow->setTooltips("palette level");
	palpow->set(0.0f);
	palpow->show(TRUE);
	
	channel=new Aselect(MKQIID(qiid, 0x999b3e1af6a1adf8), "source channel", this, 280, 4, 4, 1, &resource.get(MAKEINTRESOURCE(PNG_AUTOMIX_COLOR), "PNG"), 8, 8);
	channel->setTooltips("source channel");
	channel->set(1);
	channel->show(TRUE);

	bank=new Aselect(MKQIID(qiid, 0x9ac0e0d0f6a1adf8), "bank", this, 446, 8, 3, 2, &resource.get(MAKEINTRESOURCE(PNG_COLOR_PAL), "PNG"), 16, 16);
	bank->setTooltips("bank");
	bank->set(0);
	bank->show(TRUE);
	

	static QIID qiidcp[]={	0xad3c9c08b8be751c, 0x81dee26e6e390b58, 0xd3255e10f6a36000,
							0x45d7ddd7a816d550, 0xc58eee46293f69a0};
	
	for(i=0; i<5; i++)
	{
		int		x=328+i*20;
		char	str[256];
		char	stt[256];
		sprintf(str, "palette/color %d", i+1);
		sprintf(stt, "palette color %d", i+1);
		colpal[i]=new Aselcolor(MKQIID(qiid, qiidcp[i]), str, this, x, 18);
		colpal[i]->setTooltips(stt);
		colpal[i]->border=0xff343856;
		colpal[i]->set(e->colors[0][i]);
		colpal[i]->show(true);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AcolorFront::~AcolorFront()
{
	int	i;
	for(i=0; i<5; i++)
		delete(colpal[i]);
	delete(channel);
	delete(palpow);
	for(i=0; i<3; i++)
		delete(levels[i]);
	delete(sound);
	delete(contrast);
	delete(luminosity);
	delete(color);
	delete(bank);
	delete(back);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AcolorFront::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AcolorFront::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCHANGE:
		if(o==bank)
		{	
			Acolor	*color=(Acolor *)effect;
			int		i;
			for(i=0; i<5; i++)
				colpal[i]->set(color->colors[p][i]);
			color->calcPal();
			return true;
		}
		else
		{
			int	i;
			for(i=0; i<5; i++)
			{
				if(o==colpal[i])
				{
					Acolor	*color=(Acolor *)effect;
					color->colors[bank->get()][i]=colpal[i]->get();
					color->calcPal();
					return true;
				}
			}
		}
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

AcolorBack::AcolorBack(QIID qiid, char *name, Acolor *e, int h) : AeffectBack(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_COLOR_BACK), "PNG");
	back=new Abitmap(&o);

	in=new Avideo(MKQIID(qiid, 0x5b4c83080ee4da30), "video in", this, pinIN, 10, 10);
	in->setTooltips("video in");
	in->show(TRUE);
	
	audio=new Asample(MKQIID(qiid, 0x397d052752fa2f37), "audio in", this, pinIN, 30, 10);
	audio->setTooltips("audio in");
	audio->show(TRUE);
	
	out=new Avideo(MKQIID(qiid, 0x528e30f7dd2ec938), "video out", this, pinOUT, pos.w-18, 10);
	out->setTooltips("video out");
	out->show(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AcolorBack::~AcolorBack()
{
	delete(back);
	delete(in);
	delete(audio);
	delete(out);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AcolorBack::paint(Abitmap *b)
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

Aeffect * AcolorInfo::create(QIID qiid, char *name, Acapsule *capsule)
{
	return new Acolor(qiid, name, this, capsule);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aplugz * colorGetInfo()
{
	return new AcolorInfo("colorInfo", &Acolor::CI, "color", "color module");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
