/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	MIMIX.CPP					(c)	YoY'02						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include						"mimix.h"
#include						"resource.h"
#include						<math.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								AmimixInfo::CI	= ACI("AmimixInfo",		GUID(0x11111112,0x00000230), &AeffectInfo::CI, 0, NULL);
ACI								Amimix::CI		= ACI("Amimix",			GUID(0x11111112,0x00000231), &Aeffect::CI, 0, NULL);
ACI								AmimixFront::CI	= ACI("AmimixFront",	GUID(0x11111112,0x00000232), &AeffectFront::CI, 0, NULL);
ACI								AmimixBack::CI	= ACI("AmimixBack",		GUID(0x11111112,0x00000233), &AeffectBack::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int						count=0;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void init()
{
	if(!count)
	{
	}
	count++;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void end()
{
	count--;
	if(count==0)
	{
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Amimix::Amimix(QIID qiid, char *name, AeffectInfo *info, Acapsule *capsule) : Aeffect(qiid, name, info, capsule)
{
	init();
	front=new AmimixFront(qiid, "mimix front", this, 80);
	front->setTooltips("mimix module");
	back=new AmimixBack(qiid, "mimix back", this, 80);
	back->setTooltips("mimix module");
	image=new Abitmap(getVideoWidth(), getVideoHeight());
	settings(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Amimix::~Amimix()
{
	delete(image);
	end();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Amimix::save(class Afile *f)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Amimix::load(class Afile *f)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Amimix::settings(bool emergency)
{
	if(image)
		image->size(getVideoWidth(), getVideoHeight());
	else		
		image=new Abitmap(getVideoWidth(), getVideoHeight());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Amimix::action(double time, double dtime, double beat, double dbeat)
{
	int			w=getVideoWidth();
	int			h=getVideoHeight();
	int			mw=w>>1;
	int			mh=h>>1;
	AmimixFront	*front=(AmimixFront *)this->front;
	AmimixBack	*back=(AmimixBack *)this->back;
	Avideo		*out=back->out;
	Avideo		**in=back->in;
	float		master=front->master->get();
	float		vlevel[4];
	int			i;

	for(i=0; i<4; i++)
		vlevel[i]=front->level[i]->get()*master;

	processAudio(vlevel);

	out->enter(__FILE__,__LINE__);
	{
		Abitmap	*bout=out->getBitmap();
		if(bout)
		{
			float	zoomlevel=front->zoomLevel->get();
			float	zoomsize=front->zoomSize->get();
			float	lvlblur=front->blurLevel->get();
			float	szblur=front->blurSize->get();
			int		i;

			if(zoomlevel>0.f)	// zoom
			{
				int		zx=(zoomsize>=0.5)?((int)(w*(zoomsize-0.5))):((int)((zoomsize-0.5)*w));
				int		zy=(zoomsize>=0.5)?((int)(h*(zoomsize-0.5))):((int)((zoomsize-0.5)*h));
				image->set(0, 0, bout, bitmapNORMAL, bitmapNORMAL);
				image->flags=bitmapGLOBALALPHA|bitmapSATURATION;
				image->alpha=zoomlevel;
				image->state=bitmapNORMAL;
				bout->set(-zx, -zy, w+zx*2, h+zy*2, 0, 0, w, h, image, bitmapDEFAULT, bitmapDEFAULT);
			}
			if(lvlblur>0.f)	// blur
			{
				image->blur(bout, w*szblur*0.2f, h*szblur*0.2f);
				image->state=bitmapNORMAL;
				image->flags=bitmapGLOBALALPHA;
				image->alpha=lvlblur;
				bout->set(0, 0, image, bitmapDEFAULT, bitmapDEFAULT);
			}
			{	// fade
				float	level=front->fadeLevel->get();
				if(level>0.f)
				{
					dword	color=front->fadeColor->get();
					bout->boxfa(0, 0, bout->w-1, bout->h-1, color, level);
				}
			}

			for(i=0; i<4; i++)
			{
				Abitmap	*bin=in[i]->getBitmap();
				if(bin)
				{
					double	zz=pow((front->zoom[i]->get()-0.5)*3, 3);
					int		zx=(int)((zz<0)?(zz*w*0.2963):(zz*w));
					int		zy=(int)((zz<0)?(zz*h*0.2963):(zz*h));
					float	level=vlevel[i];
					float	s1=front->lum[i]->get();
					float	s2=front->hue[i]->get();
					float	pan=front->sat[i]->get();
					int		filter=front->filter[i]->get();

					if(level>0.f)
					{
						if((s1<0.48f)||(s1>0.52f)||(s2<0.48f)||(s2>0.52f)||(pan<0.48f)||(pan>0.52f))
						{
							int		tt[3][3][256];
							double	cc[256];
							image->set(0, 0, bin, bitmapNORMAL, bitmapNORMAL);
							{
								int	i;
								for(i=128; i<256; i++)
									cc[i]=0;
								for(i=0; i<170; i++)
									cc[(i-21)&255]=sin(i*PI/170);
							}
							{
								int		i,j,n;
								for(i=0; i<3; i++)
								{
									for(j=0; j<3; j++)
									{
										int		d=(i-j);
										if(d==2)
											d=-1;
										else if(d==-2)
											d=1;
										double	s=cc[(int)(64+d*85+(s2-0.5)*256)&255];
										if(pan>=0.5f)
										{
											float	a0=(pan-0.5f)*2.f;
											float	a1=1.f-a0;
											s=s*a1+sqrt(s)*a0*2.f;
										}
										else
										{
											float	a0=(0.5f-pan)*2.f;
											float	a1=1.f-a0;
											s=0.33333f*a0+s*a1;
										}
										int	lum=(int)(s1*511-256);
										for(n=0; n<256; n++)
										{
											tt[i][j][n]=(int)(n*s)+lum;
										}
									}
								}
							}
							{
								int		i;
								int		m=w*h;
								dword	*p=image->body32;
								for(i=0; i<m; i++)
								{
									byte	r,g,b;
									byte	r2,g2,b2;
									colorRGB(&r, &g, &b, *p);
									r2=mini(maxi(tt[0][0][r]+tt[0][1][g]+tt[0][2][b], 0), 255);
									g2=mini(maxi(tt[1][0][r]+tt[1][1][g]+tt[1][2][b], 0), 255);
									b2=mini(maxi(tt[2][0][r]+tt[2][1][g]+tt[2][2][b], 0), 255);
									*(p++)=color32(r2, g2, b2);
								}
							}
							switch(filter)
							{
								case 0:
								image->state=bitmapNORMAL;
								image->flags=bitmapGLOBALALPHA;
								break;

								case 3:
								image->state=bitmapMUL;
								image->flags=bitmapGLOBALALPHA;
								break;

								case 1:
								image->state=bitmapADD;
								image->flags=bitmapGLOBALALPHA|bitmapSATURATION;
								break;

								case 2:
								image->state=bitmapSUB;
								image->flags=bitmapGLOBALALPHA|bitmapSATURATION;
								break;

								case 4:
								image->state=bitmapADD;
								image->flags=bitmapGLOBALALPHA;
								break;

								case 5:
								image->state=bitmapSUB;
								image->flags=bitmapGLOBALALPHA;
								break;
							}
							image->alpha=level;
							bout->set(-zx, -zy, w+zx*2, h+zy*2, 0, 0, w, h, image, bitmapDEFAULT, bitmapDEFAULT);
							image->state=bitmapNORMAL;
							image->flags=0;
							image->alpha=0.f;
						}
						else
						{
							switch(filter)
							{
								case 0:
								bin->state=bitmapNORMAL;
								bin->flags=bitmapGLOBALALPHA;
								break;

								case 3:
								bin->state=bitmapMUL;
								bin->flags=bitmapGLOBALALPHA;
								break;

								case 1:
								bin->state=bitmapADD;
								bin->flags=bitmapGLOBALALPHA|bitmapSATURATION;
								break;

								case 2:
								bin->state=bitmapSUB;
								bin->flags=bitmapGLOBALALPHA|bitmapSATURATION;
								break;

								case 4:
								bin->state=bitmapADD;
								bin->flags=bitmapGLOBALALPHA;
								break;

								case 5:
								bin->state=bitmapSUB;
								bin->flags=bitmapGLOBALALPHA;
								break;
							}
							bin->alpha=level;
							bout->set(-zx, -zy, w+zx*2, h+zy*2, 0, 0, w, h, bin, bitmapDEFAULT, bitmapDEFAULT);
							bin->state=bitmapNORMAL;
							bin->flags=0;
							bin->alpha=0.f;
							/*
							switch(filter)
							{
								case 0:
								bout->
								m->get(bout, -zx, -zy, w+zx*2, h+zy*2, 0, 0, m->w, m->h, bitmapNORMAL, bitmapGLOBALALPHA, level, 0, f);
								break;

								case 2:
								m->get(bout, -zx, -zy, w+zx*2, h+zy*2, 0, 0, m->w, m->h, bitmapADD, bitmapGLOBALALPHA|(v->sature?bitmapSATURATION:0), level, 0, f);
								break;

								case 3:
								m->get(bout, -zx, -zy, w+zx*2, h+zy*2, 0, 0, m->w, m->h, bitmapSUB, bitmapGLOBALALPHA|(v->sature?bitmapSATURATION:0), level, 0, f);
								break;

								case 1:
								m->get(bout, -zx, -zy, w+zx*2, h+zy*2, 0, 0, m->w, m->h, bitmapMUL, bitmapGLOBALALPHA, level, 0, f);
								break;
							}
							*/
						}
					}
				}
			}
		}
	}
	out->leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Amimix::processAudio(float vlevel[4])
{
	AmimixBack	*back=(AmimixBack *)this->back;
	Asample		*aout=back->aout;
	Asample		**ain=back->ain;
	int			i;
	int			ilevel[4]={0, 0, 0, 0};
	int			nok=0;

	aout->enter(__FILE__,__LINE__);
	for(i=0; i<4; i++)
		ain[i]->enter(__FILE__,__LINE__);

	{
		int	avaible=44100;
		for(i=0; i<4; i++)
		{
			int		a=ain[i]->avaible();
			if(a)
			{
				int	lvl=(int)(vlevel[i]*1024.f);
				if(lvl)
				{
					avaible=mini(avaible, a);
					ilevel[i]=lvl;
					nok++;
				}
				else
					ain[i]->skip(a);
			}
		}
		for(i=0; i<4; i++)
		{
			int	a=ain[i]->avaible();
			if(a>avaible)
				ain[i]->skip(a-avaible);
		}
		if(nok)
		{
			short	buffer[8192*2];
			int		bufint[8192*2];

			while(avaible)
			{
				int	a=mini(avaible, 8192);
				memset(bufint, 0, sizeof(bufint));
				for(i=0; i<4; i++)
				{
					if(ilevel[i])
					{
						ain[i]->getBuffer(buffer, a);
						{
							sword	*s=buffer;
							int		*d=bufint;
							int		m=ilevel[i];
							int		n=a*2;
							int		j;
							for(j=0; j<n; j++)
								*(d++)+=(int)*(s++)*m;
						}
					}
				}
				{
					int		*s=bufint;
					sword	*d=buffer;
					int		n=a*2;
					int		j;
					for(j=0; j<n; j++)
						*(d++)=(short)maxi(mini((*(s++)>>10), 32767), -32767);
				}
				aout->addBuffer(buffer, a);
				avaible-=a;
			}
		}
	}

	for(i=3; i>=0; i--)
		ain[i]->leave();
	aout->leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AmimixFront::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AmimixFront::AmimixFront(QIID qiid, char *name, Amimix *e, int h) : AeffectFront(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_MIMIX), "PNG");
	back=new Abitmap(&o);

	fadeLevel=new Apaddle(MKQIID(qiid, 0x969b155e0559e998), "fade level", this, 4, 20, paddleYOLI24);
	fadeLevel->setTooltips("fade level");
	fadeLevel->set(0.0f);
	fadeLevel->show(TRUE);

	fadeColor=new Aselcolor(MKQIID(qiid, 0x06c824e402bc37e0), "fade color", this, 8, 48);
	fadeColor->setTooltips("fade color");
	fadeColor->set(0xff000000);
	fadeColor->show(true);

	zoomLevel=new Apaddle(MKQIID(qiid, 0xc76fbfab23048088), "zoom level", this, 32, 20, paddleYOLI24);
	zoomLevel->setTooltips("zoom level");
	zoomLevel->set(0.0f);
	zoomLevel->show(TRUE);

	zoomSize=new Apaddle(MKQIID(qiid, 0x47fc2d7222929710), "zoom size", this, 36, 48, paddleYOLI16);
	zoomSize->setTooltips("zoom size");
	zoomSize->set(0.5f);
	zoomSize->show(TRUE);

	blurLevel=new Apaddle(MKQIID(qiid, 0x5d69a92447131c80), "blur level", this, 60, 20, paddleYOLI24);
	blurLevel->setTooltips("blur level");
	blurLevel->set(0.0f);
	blurLevel->show(TRUE);

	blurSize=new Apaddle(MKQIID(qiid, 0xc1a72b483e10b000), "blur size", this, 64, 48, paddleYOLI16);
	blurSize->setTooltips("blur size");
	blurSize->set(0.0f);
	blurSize->show(TRUE);

	master=new Apaddle(MKQIID(qiid, 0xa75fbbb3233a790c), "master level", this, 452, 12, paddleYOLI32);
	master->setTooltips("master level");
	master->set(1.0f);
	master->show(TRUE);

	{
		static QIID qiidlvl[]={ 0x8bab9cf269cf10e0, 0x2c670a15ec4fc140, 0xbbad7a5b1811ec2a, 0x2c31c5409e35f441};
		static QIID qiidzm[]={	0x59f8b72d7605a880, 0xe5940e320357d508, 0xd99a0f3dbf7d7380, 0x458775076690ae00};
		static QIID qiidlum[]={	0x2a1fa643a42be5f8, 0x8dbd010139c0aa40, 0xc0ff3bc62435e5d2, 0xcbf7212b3c5e4c68};
		static QIID qiidsat[]={	0x92feb4de98144340, 0x2e5e9a9ce16bcdc0, 0x7950c7ed2aa572b0, 0x197e116c707241c5};
		static QIID qiidhue[]={ 0x00863bfe1ac7a800, 0xc367ea7f3df51a08, 0x74d210ea28e64490, 0x87f5f02eb19fbcbc};
		static QIID qiidflt[]={	0xd7f7df1c3b62e618, 0x354205f2bf2c8034, 0x896b004b49cca800, 0xd2aa3aaca3840b00};
		int	i;
		for(i=0; i<4; i++)
		{
			char	str[256];
			sprintf(str, "source [%d]/level", i+1);
			level[i]=new Apaddle(MKQIID(qiid, qiidlvl[i]), str, this, 110+i*80, 53, paddleYOLI24);
			level[i]->setTooltips("level");
			level[i]->set(0.0f);
			level[i]->show(TRUE);

			sprintf(str, "source [%d]/zoom", i+1);
			zoom[i]=new Apaddle(MKQIID(qiid, qiidzm[i]), str, this, 140+i*80, 60, paddleYOLI16);
			zoom[i]->setTooltips("zoom");
			zoom[i]->set(0.5f);
			zoom[i]->show(TRUE);

			sprintf(str, "source [%d]/luminosity", i+1);
			lum[i]=new Apaddle(MKQIID(qiid, qiidlum[i]), str, this, 108+i*80, 4, paddleYOLI16);
			lum[i]->setTooltips("luminosity");
			lum[i]->set(0.5f);
			lum[i]->show(TRUE);

			sprintf(str, "source [%d]/saturation", i+1);
			sat[i]=new Apaddle(MKQIID(qiid, qiidsat[i]), str, this, 128+i*80, 4, paddleYOLI16);
			sat[i]->setTooltips("saturation");
			sat[i]->set(0.5f);
			sat[i]->show(TRUE);

			sprintf(str, "source [%d]/hue", i+1);
			hue[i]=new Apaddle(MKQIID(qiid, qiidhue[i]), str, this, 148+i*80, 4, paddleYOLI16);
			hue[i]->setTooltips("hue");
			hue[i]->set(0.5f);
			hue[i]->show(TRUE);

			sprintf(str, "source [%d]/blend", i+1);
			filter[i]=new Aselect(MKQIID(qiid, qiidflt[i]), str, this, 116+i*80, 24, 3, 2, &resource.get(MAKEINTRESOURCE(PNG_MIMIX_BLEND), "PNG"), 12, 12);
			filter[i]->setTooltips("blend filter");
			filter[i]->set(0);
			filter[i]->show(TRUE);
		}
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AmimixFront::~AmimixFront()
{
	int	i;
	for(i=0; i<4; i++)
	{
		delete(level[i]);
		delete(zoom[i]);
		delete(lum[i]);
		delete(sat[i]);
		delete(hue[i]);
		delete(filter[i]);
	}
	delete(fadeLevel);
	delete(fadeColor);
	delete(zoomLevel);
	delete(zoomSize);
	delete(blurLevel);
	delete(blurSize);
	delete(master);
	delete(back);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AmimixFront::notify(Anode *o, int event, dword p)
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

AmimixBack::AmimixBack(QIID qiid, char *name, Amimix *e, int h) : AeffectBack(qiid, name, e, h)
{
	static QIID qiidin[]={ 0x418ad066b1f37400, 0x98fc6023c184ba60, 0xf050dda3ff00d300, 0x50c249904d2b2d98};
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_MIMIX2), "PNG");
	back=new Abitmap(&o);

	out=new Avideo(MKQIID(qiid, 0xed2f0c2c3aa3f100), "video out", this, pinOUT, pos.w-18, 10);
	out->setTooltips("video out");
	out->show(TRUE);

	aout=new Asample(MKQIID(qiid, 0xe85e45a45ca3f100), "audio out", this, pinOUT, pos.w-18, 30);
	aout->setTooltips("audio out");
	aout->show(TRUE);

	int	i;
	for(i=0; i<4; i++)
	{
		char	str[256];

		sprintf(str, "video in #%d", i+1);
		in[i]=new Avideo(MKQIID(qiid, qiidin[i]), str, this, pinIN, 10+i*20, 10);
		in[i]->setTooltips(str);
		in[i]->show(TRUE);

		sprintf(str, "audio in #%d", i+1);
		ain[i]=new Asample(MKQIID(qiid, qiidin[i]^0xea54d5e15ca3f1aa), str, this, pinIN, 10+i*20, 30);
		ain[i]->setTooltips(str);
		ain[i]->show(TRUE);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AmimixBack::~AmimixBack()
{
	delete(back);
	delete(in[0]);
	delete(in[1]);
	delete(in[2]);
	delete(in[3]);
	delete(out);
	delete(ain[0]);
	delete(ain[1]);
	delete(ain[2]);
	delete(ain[3]);
	delete(aout);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AmimixBack::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aeffect * AmimixInfo::create(QIID qiid, char *name, Acapsule *capsule)
{
	return new Amimix(qiid, name, this, capsule);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aplugz * mimixGetInfo()
{
	return new AmimixInfo("mimixInfo", &Amimix::CI, "mimix", "mimix module- video mixer effect");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
