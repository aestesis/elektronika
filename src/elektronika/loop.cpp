/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	LOOP.CPP					(c)	YoY'02						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include						"loop.h"
#include						"resource.h"
#include						<math.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__inline float frand()
{
	return (float)rand()/(float)RAND_MAX;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								AloopInfo::CI	= ACI("AloopInfo",	GUID(0x11111112,0x00000210), &AeffectInfo::CI, 0, NULL);
ACI								Aloop::CI		= ACI("Aloop",		GUID(0x11111112,0x00000211), &Aeffect::CI, 0, NULL);
ACI								AloopFront::CI	= ACI("AloopFront",	GUID(0x11111112,0x00000212), &AeffectFront::CI, 0, NULL);
ACI								AloopBack::CI	= ACI("AloopBack",	GUID(0x11111112,0x00000213), &AeffectBack::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AtimeView : public Aobject
{
	AOBJ

								AtimeView						(char *name, Aobject *e, int x, int y, int w, int h);
	virtual						~AtimeView						();

	void						paint							(Abitmap *b);
	void						pulse							();

	void						setStart						(float v);
	void						setLen							(float v);
	void						setCurrent						(float v);

	dword						colorBack;
	dword						colorBorder;
	dword						colorSel;
	dword						colorCurrent;
	dword						colorStart;

private:
	float						start;
	float						len;
	float						current;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								AtimeView::CI		= ACI("AtimeView",	GUID(0x11111112,0x00000215), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AtimeView::AtimeView(char *name, Aobject *e, int x, int y, int w, int h) : Aobject(name, e, x, y, w, h)
{
	colorBack=0xff9abc90;
	colorSel=0xffffffff;
	colorBorder=0xff5A5956;
	colorCurrent=0xffff0000;
	colorStart=0xffffffff;
	start=0.f;
	len=0.1f;
	current=0.f;
	timer(100);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AtimeView::~AtimeView()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AtimeView::paint(Abitmap *b)
{
	int		ww=pos.w-6;
	int		sta=mini((int)(start*(float)ww)+3, pos.w-4);
	int		cur=mini((int)(current*(float)ww)+3, pos.w-4);
	int		sto=(int)((start+len)*(float)ww)+3;
	int		hh=pos.h-4;

	b->boxf(0, 0, pos.w, pos.h, colorBack);
	
	b->linea(0, 0, 0, pos.h, 0xff000000, 0.4f);
	b->linea(1, 0, pos.w, 0, 0xff000000, 0.4f);

	b->linea(pos.w-1, 0, pos.w-1, pos.h, 0xffffffff, 0.4f);
	b->linea(1, pos.h-1, pos.w-2, pos.h-1, 0xffffffff, 0.4f);
	
	//b->box(0, 0, pos.w-1, pos.h-1, colorBorder);
	//b->box(1, 1, pos.w-2, pos.h-2, colorBorder);

	if(sto>=pos.w-3)
	{
		sto-=ww;
		b->boxfa(3, 3, sto, hh, colorSel, 0.3f);
		if(sta<pos.w-3)
			b->boxfa(sta, 3, pos.w-4, hh, colorSel, 0.3f);
	}
	else
	{
		b->boxfa(sta, 3, sto, hh, colorSel, 0.3f);
	}

	b->line(sta, 3, sta, hh, colorStart);

	b->pixel(cur, 2, colorCurrent);
	b->pixel(cur, 3, colorCurrent);
	b->pixel(cur, pos.h-3, colorCurrent);
	b->pixel(cur, pos.h-4, colorCurrent);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AtimeView::pulse()
{
	repaint();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AtimeView::setStart(float v)
{
	start=maxi(mini(v, 1.f), 0.f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AtimeView::setLen(float v)
{
	len=maxi(mini(v, 1.f), 0.f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AtimeView::setCurrent(float v)
{
	current=maxi(mini(v, 1.f), 0.f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

Aloop::Aloop(QIID qiid, char *name, AeffectInfo *info, Acapsule *capsule) : Aeffect(qiid, name, info, capsule)
{
	init();
	front=new AloopFront(qiid, "loop front", this, 80);
	front->setTooltips("loop module");
	back=new AloopBack(qiid, "loop back", this, 80);
	back->setTooltips("loop module");
	fps=getFrameRate();
	loopTime=getLoopTime();
	nbb=fps*loopTime;
	bitmaps=new Abitmap *[nbb];
	shape=new Abitmap(getVideoWidth(), getVideoHeight(), 32);
	{
		int	i;
		for(i=0; i<nbb; i++)
			bitmaps[i]=new Abitmap(getVideoWidth(), getVideoHeight(), 32);
	}
	ww=getVideoWidth();
	hh=getVideoHeight();
	recpos=0;
	aloop=0.f;
	awave[0]=awave[1]=0.f;
	mstart=0.f;
	oldbeat=0.f;
	settings(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aloop::~Aloop()
{
	int	i;
	for(i=0; i<nbb; i++)
		delete(bitmaps[i]);
	delete(shape);
	delete(bitmaps);
	end();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aloop::save(class Afile *f)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aloop::load(class Afile *f)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aloop::settings(bool emergency)
{
	if((loopTime!=getLoopTime())||(fps!=getFrameRate())||(getVideoWidth()!=ww)||(getVideoHeight()!=hh))
	{
		int	i;
		for(i=0; i<nbb; i++)
			delete(bitmaps[i]);
		delete(bitmaps);
		delete(shape);
		fps=getFrameRate();
		loopTime=getLoopTime();
		nbb=fps*loopTime;
		bitmaps=new Abitmap *[nbb];
		for(i=0; i<nbb; i++)
			bitmaps[i]=new Abitmap(getVideoWidth(), getVideoHeight(), 32);
		shape=new Abitmap(getVideoWidth(), getVideoHeight(), 32);
		ww=getVideoWidth();
		hh=getVideoHeight();
		recpos=0;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aloop::actionStart(double time)
{
	oldbeat=0.f;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aloop::action(double time, double dtime, double beat, double dbeat)
{
	int			w=getVideoWidth();
	int			h=getVideoHeight();
	int			mw=w>>1;
	int			mh=h>>1;
	AloopFront	*front=(AloopFront *)this->front;
	AloopBack	*back=(AloopBack *)this->back;
	Avideo		*out=back->out;
	Avideo		*in=back->in;
	Avideo		*in0=back->in0;
	int			mode=front->mode->get();
	float		fadesrc=front->fadesrc->get();
	float		fadeshp=front->fadeshp->get();
	bool		bshape=(mode==2);
	float		len=front->len->get();
	float		baslvl=front->baslvl->get();
	float		medlvl=front->medlvl->get();
	float		trelvl=front->trelvl->get();
	bool		brun=front->brun->get();
	bool		bloop=((mode==1)||(mode==2)||(mode==3));
	bool		bpong=front->bpong->get();
	float		speed=(float)pow((front->speed->get()-0.5f)*4, 3);
	//float		start=front->start->get();
	float		start=(front->start->get()+mstart*7.f)/8.f;
	float		bass=0.f;
	float		medium=0.f;
	float		treble=0.f;

	mstart=start;

	{
		float			mb=baslvl/16366.f;
		float			mm=medlvl/16366.f;
		float			mt=trelvl/16366.f;
		Asample			*snd=((AloopBack *)back)->sin;
		sword			smp[64*2];
		sword			bas[64*2];
		sword			med[64*2];
		sword			tre[64*2];
		int				nget=0;
		int				i;

		if(snd->isConnected())
		{
			snd->enter(__FILE__,__LINE__);
			snd->skip(maxi(snd->avaible()-64, 0));
			nget=snd->getBuffer(smp, bas, med, tre, 64);
			snd->leave();

			for(i=0; i<nget; i++)
			{
				int	i0=i<<1;
				int	i1=i0+1;
				bass=maxi(bass, (float)abs(bas[i0])*mb);
				bass=maxi(bass, (float)abs(bas[i1])*mb);
				medium=maxi(medium, (float)abs(med[i0])*mm);
				medium=maxi(medium, (float)abs(med[i1])*mm);
				treble=maxi(treble, (float)abs(tre[i0])*mt);
				treble=maxi(treble, (float)abs(tre[i1])*mt);
			}
		}
	}
	
	if(brun)
	{
		in->enter(__FILE__,__LINE__);
		{
			Abitmap	*bi=in->getBitmap();
			if(bi)
			{
				Abitmap	*b=this->bitmaps[recpos];
				b->set(0, 0, bi, bitmapNORMAL, bitmapNORMAL);
				if(fadesrc>0.f)
				{
					int		curpos=recpos-1;
					if(curpos<0)
						curpos+=nbb;
					bitmaps[curpos]->flags=bitmapGLOBALALPHA;
					bitmaps[curpos]->alpha=fadesrc;
					b->set(0, 0, bitmaps[curpos], bitmapDEFAULT, bitmapDEFAULT);
					bitmaps[curpos]->flags=0;
				}
				recpos=(recpos+1)%nbb;
			}
		}
		in->leave();
	}

	out->enter(__FILE__,__LINE__);
	{
		Abitmap	*bo=out->getBitmap();
		if(bo)
		{
			int		curpos=recpos-1;
			if(curpos<0)
				curpos+=nbb;
			in0->enter(__FILE__,__LINE__);
			{
				Abitmap	*bi0=in0->getBitmap();
				if(bi0)
				{
					if(fadeshp>0.f)
					{
						bi0->flags=bitmapGLOBALALPHA;
						bi0->alpha=1.f-fadeshp;
						shape->set(0, 0, bi0, bitmapDEFAULT, bitmapDEFAULT);
						bi0->flags=0;
					}
					else
						shape->set(0, 0, bi0, bitmapNORMAL, bitmapNORMAL);
				}
				switch(mode)
				{
					case 5:
					{
						int		x,y;
						int		ni=0;
						float	a0=awave[0]+=(float)(dbeat*(front->phase[0]->get()-0.5f)*10.f);
						float	a1=awave[1]+=(float)(dbeat*(front->phase[1]->get()-0.5f)*10.f);
						float	d0=10.f*(front->freq[0]->get()+0.01f)/(float)bo->h;
						float	d1=10.f*(front->freq[1]->get()+0.01f)/(float)bo->h;
						float	m1=front->mix->get()*0.25f;
						float	m0=0.25f-m1;
						int		ln=(int)(len*(float)(nbb-1));
						int		stapos=curpos-(int)(start*(float)(nbb-1));
						
						if(stapos<0)
							stapos+=nbb;

						dword	*d=&bo->body32[0];
						dword	*s=&shape->body32[0];

						a0-=(float)bo->h*d0*0.5f;
						a1-=(float)bo->h*d1*0.5f;

						for(y=0; y<h; y++)
						{
							float	v=(float)((sin(a0)+1.0)*m0+(sin(a1)+1.0)*m1);
							float	im=v*((float)ln);
							float	r=(im-(float)(int)im);
							int		n=stapos-(int)im;
							Abitmap	*im0,*im1;

							if(n<0)
								n+=nbb;
							im0=bitmaps[n];
							n--;
							if(n<0)
								n+=nbb;
							im1=bitmaps[n];

							for(x=0; x<w; x++)
							{
								*(d++)=colormix32(im0->body32[ni], im1->body32[ni], r);
								ni++;
								s++;
							}

							a0+=d0;
							a1+=d1;
						}
					}
					break;

					case 4:
					{
						Abitmap		*idx[384];
						Abitmap		*idx2[384];
						float		mix[384];
						{
							int		i;
							int		ln=(int)(len*(float)(nbb-1));
							int		stapos=curpos-(int)(start*(float)(nbb-1));
							if(stapos<0)
								stapos+=nbb;
							for(i=0; i<384; i++)
							{
								float	d=(((float)i*(float)ln)/384.f);
								int		n=stapos-(int)d;
								float	v=d-(float)(int)d;
								mix[i]=v;
								if(n<0)
									n+=nbb;
								idx[i]=bitmaps[n];
								n--;
								if(n<0)
									n+=nbb;
								idx2[i]=bitmaps[n];
							}
						}
						{
							int		x,y;
							int		n=0;

							dword	*d=&bo->body32[0];
							dword	*s=&shape->body32[0];

							for(y=0; y<h; y++)
							{
								for(x=0; x<w; x++)
								{
									int	v=((*s&255)+((*s>>8)&255)+((*s>>16)&255))>>1;
									*(d++)=colormix32(idx[v]->body32[n], idx2[v]->body32[n], mix[v]);
									n++;
									s++;
								}
							}
						}
					}
					break;
					
					case 3:
					{
						float	v=1.f-front->rndtime->get();
						if(v<1.f)
						{
							
							if((beat-oldbeat)>((float)pow(v*2.f, 4.f)))
							{
								oldbeat=beat;
								if(bpong)
									aloop=(bass+medium+treble)*len*2.f;
								else
									aloop=(bass+medium+treble)*len;
							}
						}
					}
					goto nextpart;

					case 2:
					{
						float	v=1.f-front->rndtime->get();
						if(v<1.f)
						{
							
							if((beat-oldbeat)>(pow(v*2.f, 4.f)))
							{
								oldbeat=beat;
								if(bpong)
									aloop=frand()*len*2.f;
								else
									aloop=frand()*len;
							}
						}
					}
					
				nextpart:
					case 0:
					case 1:
					{
						int	ns;
						if(bloop)
						{
							float	nsta=(start*(nbb-1));
							if(len>0.f)
							{
								if(bpong)
								{
									float	len2=len*2.f;
									while(aloop>=len2)
										aloop-=len2;
									while(aloop<0)
										aloop+=len2;
									{
										float al=aloop;
										if(al>len)
											al=len2-al;
										int	np=(int)(nsta+al*(float)nbb);
										if(np>=nbb)
											np-=nbb;
										ns=curpos-(int)mini(nbb-1, np);
									}
								}
								else
								{
									while(aloop>=len)
										aloop-=len;
									while(aloop<0)
										aloop+=len;
									int	np=(int)(nsta+aloop*(float)nbb);
									if(np>=nbb)
										np-=nbb;
									ns=curpos-(int)mini(nbb-1, np);
								}
								aloop+=(float)(speed*dtime/4000);
							}
							else
							{
								ns=curpos-(int)(start*(nbb-1));
								bloop=0;
							}
						}
						else
							ns=curpos-(int)(start*(nbb-1));
						if(ns<0)
							ns+=nbb;
						bo->set(0, 0, bitmaps[ns], bitmapNORMAL, bitmapNORMAL);
					}
					break;
/*					
					case 3:
					{
						float	nsta=(start*(nbb-1)); 
						int		ns;
						if(len>0.f)
						{
							if(bpong)
							{
								float	len2=len*2.f;
								aloop=aloop*0.8f+(bass+medium+treble)*len2*0.2f;
								while(aloop>=len2)
									aloop-=len2;
								while(aloop<0)
									aloop+=len2;
								{
									float al=aloop;
									if(al>len)
										al=len2-al;
									int	np=(int)(nsta+al*(float)nbb);
									if(np>=nbb)
										np-=nbb;
									ns=curpos-(int)mini(nbb-1, np);
								}
							}
							else
							{
								aloop=aloop*0.8f+(bass+medium+treble)*len*0.2f;
								while(aloop>=len)
									aloop-=len;
								while(aloop<0)
									aloop+=len;
								int	np=(int)(nsta+aloop*(float)nbb);
								if(np>=nbb)
									np-=nbb;
								ns=curpos-(int)mini(nbb-1, np);
							}
						}
						else
						{
							ns=curpos-(int)(start*(nbb-1));
							bloop=0;
						}
						if(ns<0)
							ns+=nbb;
						bo->set(0, 0, bitmaps[ns], false);
					}
					break;
*/
				}
			}
			in0->leave();
		}
	}
	out->leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AloopFront::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AloopFront::AloopFront(QIID qiid, char *name, Aloop *e, int h) : AeffectFront(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_LOOP), "PNG");
	int		i;
	back=new Abitmap(&o);

	fadesrc=new Apaddle(MKQIID(qiid, 0x3c80ebaaeb32d1e8), "fade source", this, 20, 38, paddleYOLI24);
	fadesrc->setTooltips("fade source");
	fadesrc->set(0.f);
	fadesrc->show(TRUE);

	fadeshp=new Apaddle(MKQIID(qiid, 0xc0df027785e02c40), "fade shape", this, 313, 53, paddleYOLI16);
	fadeshp->setTooltips("fade shape");
	fadeshp->set(0.f);
	fadeshp->show(TRUE);

	timeview=new AtimeView("loop view", this, 72, 16, 96, 32);
	timeview->setTooltips("loop view");
	timeview->show(TRUE);

	timeview->setStart(0.f);
	timeview->setCurrent(0.f);
	timeview->setLen(0.1f);

	start=new Apaddle(MKQIID(qiid, 0x0fa42f4ccbb3df80), "loop start position", this, 96, 52, paddleYOLI16);
	start->setTooltips("loop start postion");
	start->set(0.f);
	start->show(TRUE);

	len=new Apaddle(MKQIID(qiid, 0xb9c00409becc4ea0), "loop length", this, 125, 52, paddleYOLI16);
	len->setTooltips("loop length");
	len->set(0.1f);
	len->show(TRUE);

	speed=new Apaddle(MKQIID(qiid, 0x01398b43ec80a8e0), "loop speed", this, 192, 52, paddleYOLI16);
	speed->setTooltips("loop speed");
	speed->set(0.5f);
	speed->show(TRUE);

	brun=new ActrlButton(MKQIID(qiid, 0x32cb24624668d480), "activate in flow", this, 28, 19, 9, 9, &resource.get(MAKEINTRESOURCE(PNG_LOOP_SHAPE), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	brun->setTooltips("activate in flow");
	brun->set(true);
	brun->show(TRUE);

	bpong=new ActrlButton(MKQIID(qiid, 0x2656f215d43cd300), "activate ping-pong", this, 195, 37, 9, 9, &resource.get(MAKEINTRESOURCE(PNG_LOOP_SHAPE), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	bpong->setTooltips("activate ping-pong");
	bpong->set(false);
	bpong->show(TRUE);

	mode=new Aselect(MKQIID(qiid, 0xcb1dc6d19e679f20), "mode", this, 420, 20, 2, 3, &resource.get(MAKEINTRESOURCE(PNG_LOOP_SELECT), "PNG"), 12, 12);
	mode->setTooltips("mode selector");
	mode->set(0);
	mode->show(TRUE);

	mix=new Apaddle(MKQIID(qiid, 0x6150eeac649aede0), "mix waves", this, 364, 28, paddleYOLI24);
	mix->setTooltips("mix waves");
	mix->set(0.f);
	mix->show(TRUE);
	
	static QIID qiidfr[]={ 0x35fb446cb352da00, 0xcdb3882090824800};
	static QIID qiidph[]={ 0x71a71e93e955da80, 0x2d3081a48449f562};

	for(i=0; i<2; i++)
	{
		freq[i]=new Apaddle(MKQIID(qiid, qiidfr[i]), "wave frequency", this, 348, 12+i*40, paddleYOLI16);
		freq[i]->setTooltips("wave frequency");
		freq[i]->set(0.0f);
		freq[i]->show(TRUE);

		phase[i]=new Apaddle(MKQIID(qiid, qiidph[i]), "wave phase", this, 388, 12+i*40, paddleYOLI16);
		phase[i]->setTooltips("wave phase");
		phase[i]->set(0.5f);
		phase[i]->show(TRUE);
	}

	rndtime=new Apaddle(MKQIID(qiid, 0xde5a457017e40112), "rndtime", this, 232, 52, paddleYOLI16);
	rndtime->setTooltips("random jump time");
	rndtime->set(0.0f);
	rndtime->show(TRUE);

	baslvl=new Apaddle(MKQIID(qiid, 0xa1e2cfecb0acb4a0), "baslvl", this, 272, 52, paddleYOLI16);
	baslvl->setTooltips("bass audio level");
	baslvl->set(0.0f);
	baslvl->show(TRUE);

	medlvl=new Apaddle(MKQIID(qiid, 0x4b87747e98c538a0), "medlvl", this, 272, 32, paddleYOLI16);
	medlvl->setTooltips("medium audio level");
	medlvl->set(0.0f);
	medlvl->show(TRUE);

	trelvl=new Apaddle(MKQIID(qiid, 0xb875b1440b257cfc), "trelvl", this, 272, 12, paddleYOLI16);
	trelvl->setTooltips("treble audio level");
	trelvl->set(0.0f);
	trelvl->show(TRUE);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AloopFront::~AloopFront()
{
	delete(mix);
	delete(freq[0]);
	delete(freq[1]);
	delete(phase[0]);
	delete(phase[1]);
	delete(mode);
	delete(bpong);
	delete(brun);
	delete(timeview);
	delete(start);
	delete(len);
	delete(speed);
	delete(fadesrc);
	delete(fadeshp);
	delete(rndtime);
	delete(baslvl);
	delete(medlvl);
	delete(trelvl);
	delete(back);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AloopFront::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCHANGE:
		if(o==start)
		{
			float	sta=start->get();
			timeview->setStart(sta);
			timeview->setCurrent(sta);
			if(mode->get()!=1)
				((Aloop *)effect)->aloop=0.f;
		}
		else if(o==len)
		{
			timeview->setLen(len->get());
		}
		else if(o==mode)
		{
			int	m=mode->get();
			if(m!=1)
			{
				timeview->setCurrent(start->get());
				((Aloop *)effect)->aloop=0.f;
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

AloopBack::AloopBack(QIID qiid, char *name, Aloop *e, int h) : AeffectBack(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_LOOP2), "PNG");
	back=new Abitmap(&o);

	out=new Avideo(MKQIID(qiid, 0x7ff60d607c228080), "video out", this, pinOUT, pos.w-18, 10);
	out->setTooltips("video out");
	out->show(TRUE);

	in=new Avideo(MKQIID(qiid, 0xd7c3c5014dad9b78), "video in", this, pinIN, 10, 10);
	in->setTooltips("video in");
	in->show(TRUE);

	in0=new Avideo(MKQIID(qiid, 0x2f92aad923b8596e), "video shape in", this, pinIN, 30, 10);
	in0->setTooltips("video shape in");
	in0->show(TRUE);

	sin=new Asample(MKQIID(qiid, 0x29e8fa2474e2f128), "audio input", this, pinIN, 10, 30);
	sin->setTooltips("audio input");
	sin->show(TRUE);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AloopBack::~AloopBack()
{
	delete(back);
	delete(in);
	delete(in0);
	delete(out);
	delete(sin);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AloopBack::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aeffect * AloopInfo::create(QIID qiid, char *name, Acapsule *capsule)
{
	return new Aloop(qiid, name, this, capsule);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aplugz * loopGetInfo()
{
	return new AloopInfo("loopInfo", &Aloop::CI, "loop107", "loop107 module - video loop sampler");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
