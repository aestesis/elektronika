/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	VMIX.CPP					(c)	YoY'01						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"vmix.h"
#include						"resource.h"
#include						<math.h>
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								AvmixInfo::CI		= ACI("AvmixInfo",	GUID(0x11111112,0x00000040), &AeffectInfo::CI, 0, NULL);
ACI								Avmix::CI			= ACI("Avmix",		GUID(0x11111112,0x00000041), &Aeffect::CI, 0, NULL);
ACI								AvmixFront::CI		= ACI("AvmixFront",	GUID(0x11111112,0x00000042), &AeffectFront::CI, 0, NULL);
ACI								AvmixBack::CI		= ACI("AvmixBack",	GUID(0x11111112,0x00000043), &AeffectBack::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static double					autospeedfreq[7]	= { 0.125, 0.25, 0.5, 1, 2, 4, 8 };

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int						count=0;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void init()
{
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

static QIID	qiidtrans[]={ 0x36959b81e2b41448, 0x7f0f7ad9eecaf9d4, 0xc2f4f093659c925c, 0x9deefebe273e6554};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Avmix::Avmix(QIID qiid, char *name, AeffectInfo *info, Acapsule *capsule) : Aeffect(qiid, name, info, capsule)
{
	image[0]=null;
	image[1]=null;
	front=new AvmixFront(qiid, "vmix front", this, 128);
	front->setTooltips("vmix module");
	back=new AvmixBack(qiid, "vmix back", this, 128);
	back->setTooltips("vmix module");
	m_cross=0.5f;
	mixette=0.5f;
	init();
	settings(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Avmix::~Avmix()
{
	if(image[0])
		delete(image[0]);
	if(image[1])
		delete(image[1]);
	safeGUI.enter(__FILE__,__LINE__);
//	front->timer(0);	// when delete vmix pulse is on in vmixFront...
	{
		AvmixFront *f=(AvmixFront *)front;	// do it here because of the freelibrary DLL by Atable destructor
		delete(f->tr[0]);
		delete(f->tr[1]);
		delete(f->tr[2]);
		delete(f->tr[3]);
	}
	safeGUI.leave();
	end();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avmix::save(class Afile *fi)
{
	AvmixFront *f=(AvmixFront *)front;
	int			i;
	for(i=0; i<4; i++)
	{
		ACI *ci=f->tr[i]->getCI();
		fi->write(&ci->guid, sizeof(ci->guid));
	}
	for(i=0; i<4; i++)
	{
		if(!f->tr[i]->save(fi))
			return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avmix::load(class Afile *fi)
{
	AvmixFront *f=(AvmixFront *)front;
	int			i;
	for(i=0; i<4; i++)
	{
		qword	guid;
		Aitem	*it=(Aitem *)f->trans[i]->items->fchild;
		fi->read(&guid, sizeof(guid));

		if(f->tr[i]->getCI()->guid!=guid)
		{
			while(it)
			{
				Aplugz	*p=(Aplugz *)it->link;
				if(p->guid==guid)
				{
					char	str[1024];
					delete(f->tr[i]);
					sprintf(str, "[%d] TRANS %s", i+1, ((AtransitionInfo *)(p))->name);
					f->tr[i]=((AtransitionInfo *)(p))->create(MKQIID(getQIID(), qiidtrans[i]), str, f, 352, 16);
					f->tr[i]->show(false);
					f->trans[i]->setCurrent(it);
					break;
				}
				it=(Aitem *)it->next;
			}
			if(!it)
				return false;
		}
	}
	for(i=0; i<4; i++)
	{
		if(!f->tr[i]->load(fi))
			return false;
	}
	f->active=f->tr[f->seltrans->get()];
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avmix::loadPreset(Afile *f)
{
	section.enter(__FILE__,__LINE__);
	bool b=load(f);
	section.leave();
	return b;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
bool Avmix::savePreset(Afile *f)
{
	section.enter(__FILE__,__LINE__);
	bool b=save(f);
	section.leave();
	return b;
}
	
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avmix::settings(bool emergency)
{
	int	w=getVideoWidth();
	int	h=getVideoHeight();
	int	i;
	for(i=0; i<2; i++)
	{
		if(image[i])
			image[i]->size(w, h);
		else
			image[i]=new Abitmap(w, h);
	}
	for(i=0; i<4; i++)
		((AvmixFront *)front)->tr[i]->settings();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avmix::action(double time, double dtime, double beat, double dbeat)
{
	int	w=getVideoWidth();
	int	h=getVideoHeight();

	section.enter(__FILE__,__LINE__);
	{
		int		s1=((AvmixFront *)front)->lsource->get();
		int		s2=((AvmixFront *)front)->rsource->get();
		float	c=(mixette=(mixette*0.2f+(((AvmixFront *)front)->cross->get())*0.8f));
		int		src=((AvmixFront *)front)->absrc->get()?1:0;
		float	power=((AvmixFront *)front)->epow->get();
		Avideo	*in0=((AvmixBack *)back)->in[s1];
		Avideo	*in1=((AvmixBack *)back)->in[s2];
		Avideo	*out=((AvmixBack *)back)->out;
		Avideo	*outleft=((AvmixBack *)back)->outleft;
		Avideo	*outright=((AvmixBack *)back)->outright;
		Avideo	*rec0=((AvmixBack *)back)->inleft;
		Avideo	*rec1=((AvmixBack *)back)->inright;
		Asample	*audio=((AvmixBack *)back)->audio;
		int		automix=((AvmixFront *)front)->automix->get();
		int		autospeed=((AvmixFront *)front)->autospeed->get();
		bool	autoswitch=((AvmixFront *)front)->autoswitch->get();
		
		switch(automix)
		{
			case 0:
			{
				double	freq=autospeedfreq[autospeed];
				double	a=PI*freq*beat-PI/2.0;
				c=(float)(sin(a)*0.5+0.5);
				if(autoswitch)
				{
					sqword	n=(sqword)((a-PI/2.0)/PI);
					if(!(n&1))
						src=!src;
				}
			}
			break;
			
			case 1:
			if(autoswitch)
			{
				float v=((AvmixFront *)front)->cross->get();
				if((v==1.f)||(v==0.f))
				{
					c=v;
					m_cross=v;	
				}
				if(m_cross==1.f)
					src=!src;
			}
			else
				m_cross=0.5f;
			break;
			
			case 2:
			{
				int	value=0;
				audio->enter(__FILE__,__LINE__);
				if(audio->isConnected())
				{
					int	 avaible=audio->avaible();
					while(avaible)
					{
						sword	buffer[8192];
						int		n=audio->getBuffer(buffer, null, null, null, mini(4096, avaible));
						int		i;
						sword	*v=buffer;
						for(i=0; i<n; i++)
						{
							value=maxi(value, abs((int)*(v++)));
							value=maxi(value, abs((int)*(v++)));
						}
						avaible-=n;
					}
				}
				audio->leave();
				c=(float)value/32768.f;
			}
			break;
		}

		if(((AvmixFront *)front)->lpush->get())
			c=0.f;
		if(((AvmixFront *)front)->rpush->get())
			c=1.f;
			
		audioProcess(dtime, c);
	
		in0->enter(__FILE__,__LINE__);
		in1->enter(__FILE__,__LINE__);
		out->enter(__FILE__,__LINE__);
		
		{
			Abitmap	*b0=in0->getBitmap();
			Abitmap	*b1=in1->getBitmap();
			Abitmap	*br0=rec0->getBitmap();
			Abitmap	*br1=rec1->getBitmap();
			float	ret0=((AvmixFront *)front)->ret[0]->get();
			float	ret1=((AvmixFront *)front)->ret[1]->get();
			if(b0)
				image[0]->set(0, 0, b0, bitmapNORMAL, bitmapNORMAL);
			else
				image[0]->boxf(0, 0, w, h, 0xff000000);
			if(b1)
				image[1]->set(0, 0, b1, bitmapNORMAL, bitmapNORMAL);
			else
				image[1]->boxf(0, 0, w, h, 0xff000000);
			if((br0)&&(ret0>0.f))
			{
				br0->state=bitmapNORMAL;
				br0->flags=bitmapGLOBALALPHA;
				br0->alpha=ret0;
				image[0]->set(0, 0, br0);
				br0->flags=0;
			}
			if((br1)&&(ret1>0.f))
			{
				br1->state=bitmapNORMAL;
				br1->flags=bitmapGLOBALALPHA;
				br1->alpha=ret1;
				image[1]->set(0, 0, br1);
				br1->flags=0;
			}
		}
		
		
		{
			Abitmap	*in[8];
			int		i;
			for(i=0; i<8; i++)
				in[i]=((AvmixBack *)back)->in[i]->getBitmap();
			
			c=maxi(mini(c*1.001f-0.0005f, 1.f), 0.f);
			((AvmixFront *)front)->active->action(out->getBitmap(), image[0], image[1], c, src, power, in);
		}
		out->leave();

		outleft->enter(__FILE__,__LINE__);
		{
			Abitmap	*bs=in0->getBitmap();
			Abitmap	*bo=outleft->getBitmap();
			if(bs)
				bo->set(0, 0, bs, bitmapNORMAL, bitmapNORMAL);
			else
				bo->boxf(0, 0, w, h, 0xff000000);
		}
		outleft->leave();

		outright->enter(__FILE__,__LINE__);
		{
			Abitmap	*bs=in1->getBitmap();
			Abitmap	*bo=outright->getBitmap();
			if(bs)
				bo->set(0, 0, bs, bitmapNORMAL, bitmapNORMAL);
			else
				bo->boxf(0, 0, w, h, 0xff000000);
		}
		outright->leave();
		
		in1->leave();
		in0->leave();
	}
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avmix::audioProcess(double dtime, float mix)
{
	AvmixBack	*back=(AvmixBack *)this->back;
	int			s1=((AvmixFront *)front)->lsource->get();
	int			s2=((AvmixFront *)front)->rsource->get();
	int			i;

	for(i=0; i<8; i++)
	{
		if((i!=s1)&&(i!=s2))
		{
			back->ain[i]->enter(__FILE__,__LINE__);
			back->ain[i]->skip(back->ain[i]->avaible());
			back->ain[i]->leave();
		}
	}

	Asample	*ain1=back->ain[s1];
	Asample	*ain2=back->ain[s2];
	Asample	*aout=back->aout;

	if(s1==s2)
	{
		aout->enter(__FILE__,__LINE__);
		ain1->enter(__FILE__,__LINE__);
		{
			sword	buffer[8192*2];
			int		avaible=ain1->avaible();
			while(avaible)
			{
				int	a=mini(avaible, 8192);
				ain1->getBuffer(buffer, a);
				aout->addBuffer(buffer, a);
				avaible-=a;
			}
		}
		ain1->leave();
		aout->leave();
	}
	else
	{
		aout->enter(__FILE__,__LINE__);
		ain1->enter(__FILE__,__LINE__);
		ain2->enter(__FILE__,__LINE__);
		{
			sword	buf1[8192*2];
			sword	buf2[8192*2];
			sword	bufo[8192*2];
			int		a1=ain1->avaible();
			int		a2=ain2->avaible();
			int		avaible=mini(a1, a2);
			int		amax=maxi(a1, a2);
			if(!avaible)	// one of the stream is connected and not running
			{
				avaible=amax;
				memset(buf1, 0, sizeof(buf1));
				memset(buf2, 0, sizeof(buf2));
			}
			int		dif=amax-avaible;
			if(dif)
			{
				if(a1>a2)
					ain1->skip(dif);
				else
					ain2->skip(dif);
			}
			while(avaible)
			{
				int	a=mini(avaible, 8192);
				ain1->getBuffer(buf1, a);
				ain2->getBuffer(buf2, a);
				{
					int		nb=a*2;
					int		m1=(int)((1.f-mix)*1024.f);
					int		m2=1024-m1;
					sword	*s1=buf1;
					sword	*s2=buf2;
					sword	*d=bufo;
					for(i=0; i<nb; i++)
						*(d++)=(short)((*(s1++)*m1+*(s2++)*m2)>>10);
				}
				aout->addBuffer(bufo, a);
				avaible-=a;
			}
		}
		ain2->leave();
		ain1->leave();
		aout->leave();
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

void AvmixFront::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
	//b->set(10, 30, limage);
	//b->set(360, 30, rimage);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AvmixFront::AvmixFront(QIID qiid, char *name, Avmix *e, int h) : AeffectFront(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_VMIX), "PNG");
	back=new Abitmap(&o);

	lsource=new Aselect(MKQIID(qiid, 0xdb1525ae3a3821f0), "left source", this, 18, 16, 2, 4, &resource.get(MAKEINTRESOURCE(PNG_VMIX_SOURCES), "PNG"), 16, 16);
	lsource->setTooltips("left source selector");
	lsource->set(0);
	lsource->show(TRUE);

	rsource=new Aselect(MKQIID(qiid, 0x61149ced5ef016ef), "right source", this, 140, 16, 2, 4, &resource.get(MAKEINTRESOURCE(PNG_VMIX_SOURCES), "PNG"), 16, 16);
	rsource->setTooltips("right source selector");
	rsource->set(1);
	rsource->show(TRUE);
	
	lpush=new ActrlButton(MKQIID(qiid, 0x614c5ced5ef016ef), "left pulse [A]", this, 18, 92, 24, 24, &resource.get(MAKEINTRESOURCE(PNG_VMIX_PA), "PNG"), Abutton::btBITMAP);
	lpush->setTooltips("pulse mix, left source [A]");
	lpush->show(true);

	rpush=new ActrlButton(MKQIID(qiid, 0x614695c511a016ef), "right pulse [B]", this, 150, 92, 24, 24, &resource.get(MAKEINTRESOURCE(PNG_VMIX_PB), "PNG"), Abutton::btBITMAP);
	rpush->setTooltips("pulse mix, right source [B]");
	rpush->show(true);

	lauto=new ActrlButton(MKQIID(qiid, 0x61485d8e45d016ef), "left auto [A]", this, 56, 16, 8, 70, &resource.get(MAKEINTRESOURCE(PNG_VMIX_LAUTO), "PNG"), Abutton::btBITMAP);
	lauto->setTooltips("left auto transition [A]");
	lauto->show(true);

	rauto=new ActrlButton(MKQIID(qiid, 0x614548d54a4516ef), "right auto [B]", this, 128, 16, 8, 70, &resource.get(MAKEINTRESOURCE(PNG_VMIX_RAUTO), "PNG"), Abutton::btBITMAP);
	rauto->setTooltips("right auto transition [B]");
	rauto->show(true);

	itrans=new Aitem("transition", "transition");

	trans[0]=new Alist("transition 1", this, 200, 17, 112, 14, itrans);
	trans[0]->setTooltips("transition 1");
	trans[0]->show(true);
	trans[1]=new Alist("transition 2", this, 200, 35, 112, 14, itrans);
	trans[1]->setTooltips("transition 2");
	trans[1]->show(true);
	trans[2]=new Alist("transition 3", this, 200, 53, 112, 14, itrans);
	trans[2]->setTooltips("transition 3");
	trans[2]->show(true);
	trans[3]=new Alist("transition 4", this, 200, 71, 112, 14, itrans);
	trans[3]->setTooltips("transition 4");
	trans[3]->show(true);

	{
		Aitem	*pi=(Aitem *)e->getTable()->plugz->fchild;
		Aitem	*sel=NULL;
		char	str[1024];
		while(pi)
		{
			Aplugz	*p=(Aplugz *)pi->link;
			if(p->isCI(&AtransitionInfo::CI))
			{
				Aitem	*i=new Aitem(p->ename, p->ehelp, itrans, p);
				sel=i;
			}
			pi=(Aitem *)pi->next;
		}
		{
			Aitem	*i=sel;
			int		j;
			for(j=0; j<4; j++)
			{
				trans[j]->setCurrent(i);
				sprintf(str, "[%d] TRANS %s", j+1, ((AtransitionInfo *)(i->link))->name);
				tr[j]=((AtransitionInfo *)(i->link))->create(MKQIID(qiid, qiidtrans[j]), str, this, 352, 16);
				if(j==0)
					tr[j]->show(true);
				if(i->next)
					i=(Aitem *)i->next;
				else
					i=sel;
			}
		}
		active=tr[0];
	}

	seltrans=new Aselect(MKQIID(qiid, 0x8fcb1cd804239bcc), "transition selection", this, 324, 16, 1, 4, &resource.get(MAKEINTRESOURCE(PNG_VMIX_TRANS), "PNG"), 16, 16);
	seltrans->setTooltips("transition selection");
	seltrans->set(0);
	seltrans->show(TRUE);
	
	automix=new Aselect(MKQIID(qiid, 0x8fa0c0ed04239bcc), "mix mode", this, 230, 96, 3, 1, &resource.get(MAKEINTRESOURCE(PNG_VMIX_AUTO), "PNG"), 16, 16);
	automix->setTooltips("mix mode [auto/manual/sound]");
	automix->set(1);
	automix->show(true);

	autospeed=new Aselpad(MKQIID(qiid, 0x8fa956a260c39bcc), "auto mix speed", this, 200, 92, 7, paddleYOLI24);
	autospeed->setTooltips("auto mix speed");
	autospeed->set(3);
	autospeed->show(true);

	autoswitch=new ActrlButton(MKQIID(qiid, 0x8fa9960cd05a0d4c), "auto switch", this, 296, 96, 16, 16, &resource.get(MAKEINTRESOURCE(PNG_VMIX_SWITCH), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	autoswitch->setTooltips("auto switch effect source");
	autoswitch->set(false);
	autoswitch->show(true);
	
	absrc=new ActrlButton(MKQIID(qiid, 0x98d80673a0cfd2b0), "effect source [A-B]", this, 324, 96, 16, 16, &resource.get(MAKEINTRESOURCE(PNG_TRANSCOLOR_AB), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	absrc->setTooltips("effect source [A-B]");
	absrc->set(true);
	absrc->show(TRUE);

	epow=new Apaddle(MKQIID(qiid, 0x98d580560d54c2b0), "transition control", this, 80, 86, paddleYOLI32);
	epow->setTooltips("transition control");
	epow->set(0.f);
	epow->show(true);
	
	ret[0]=new Apaddle(MKQIID(qiid, 0x980590560d54c2b0), "effect A", this, 48, 92, paddleYOLI24);
	ret[0]->setTooltips("effect A");
	ret[0]->set(0.f);
	ret[0]->show(true);

	ret[1]=new Apaddle(MKQIID(qiid, 0x9809a0560d54c2b0), "effect B", this, 120, 92, paddleYOLI24);
	ret[1]->setTooltips("effect A");
	ret[1]->set(0.f);
	ret[1]->show(true);

	cross=new Aslider(MKQIID(qiid, 0xfa44747fa4a8ec10), "cross fader", this, 69, 8, 54, 78, sliderY, &resource.get(MAKEINTRESOURCE(PNG_VMIX_STICK), "PNG"));
	cross->setTooltips("cross fader");
	cross->set(0.5f);
	cross->show(TRUE);

	dirauto=0;
	timer(40);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AvmixFront::~AvmixFront()
{
	delete(autoswitch);
	delete(automix);
	delete(autospeed);
	delete(cross);
	delete(seltrans);
	delete(trans[0]);
	delete(trans[1]);
	delete(trans[2]);
	delete(trans[3]);
	delete(itrans);
	delete(lsource);
	delete(rsource);
	delete(lpush);
	delete(rpush);
	delete(lauto);
	delete(rauto);
	delete(absrc);
	delete(epow);
	delete(ret[0]);
	delete(ret[1]);
	delete(back);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AvmixFront::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCHANGE:
		if(o==seltrans)
		{
			((Avmix *)effect)->section.enter(__FILE__,__LINE__);
			active->show(false);
			active=tr[p];
			active->show(true);
			((Avmix *)effect)->section.leave();
			active->repaint();
		}
		else 
		{
			int	i;
			for(i=0; i<4; i++)
			{
				if(o==trans[i])
				{
					bool	b=(active==tr[i]);
					((Avmix *)effect)->section.enter(__FILE__,__LINE__);
					{
						Aitem			*it=(Aitem *)p;
						AtransitionInfo	*ti=(AtransitionInfo *)(it->link);
						char			str[1024];
						delete(tr[i]);
						//tr[i]->destroy();
						sprintf(str, "[%d] TRANS %s", i+1, ti->name);
						tr[i]=ti->create(MKQIID(effect->getQIID(), qiidtrans[i]), str, this, 352, 16);
					}
					if(b)
					{
						active=tr[i];
						active->show(true);
					}
					((Avmix *)effect)->section.leave();
					if(b)
						active->repaint();
					break;
				}
			}
		}
		break;

		case nyPRESS:
		if((o==lauto)||(o==rauto))
		{
			if(((Avmix *)effect)->getTable()->isRunning())
			{
				dirauto=(o==lauto)?-1:1;
				beatauto=effect->getBeat();
			}
		}
		break;
	}
	return AeffectFront::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AvmixFront::pulse()
{
	if(dirauto)
	{
		double freq=autospeedfreq[autospeed->get()];
		double beat=effect->getBeat();
		double dbeat=beat-beatauto;
		float  v=cross->get();
		v+=(float)(freq*dbeat*(double)dirauto);
		if(v<0.f)
		{
			v=0.f;
			dirauto=0;
		}
		else if(v>1.f)
		{
			v=1.f;
			dirauto=0;
		}
		this->cross->set(v);
		beatauto=beat;
		cross->repaint();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static QIID qiidin[]={ 0x664de60dc03c8e48, 0xff84624e118c4860, 0x100dd0250d55a000, 0xa8580984afb91ce4,
						0x4d831f06aeef0800, 0xa24dd3ad3afc4660, 0xa5f9b82f41fb9970, 0x66ab7a9a54e95518};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AvmixBack::AvmixBack(QIID qiid, char *name, Avmix *e, int h) : AeffectBack(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_VMIX2), "PNG");
	back=new Abitmap(&o);
	{
		int	i;
		for(i=0; i<8; i++)
		{
			char	str[128];

			sprintf(str, "video input %d", i+1);
			in[i]=new Avideo(MKQIID(qiid, qiidin[i]), str, this, pinIN, 10+20*i, 10);
			in[i]->setTooltips(str);
			in[i]->show(TRUE);

			sprintf(str, "audio input %d", i+1);
			ain[i]=new Asample(MKQIID(qiid, qiidin[i]^0xa2841cd5e62da060), str, this, pinIN, 10+20*i, 30);
			ain[i]->setTooltips(str);
			ain[i]->show(TRUE);
		}
	}

	out=new Avideo(MKQIID(qiid, 0xe0a80c54060fc440), "video output", this, pinOUT, pos.w-18, 10);
	out->setTooltips("video output");
	out->show(TRUE);

	aout=new Asample(MKQIID(qiid, 0xe0854a2cd5e6c440), "audio output", this, pinOUT, pos.w-18, 30);
	aout->setTooltips("audio output");
	aout->show(TRUE);

	outleft=new Avideo(MKQIID(qiid, 0xc4a842884d77b200), "send A", this, pinOUT, pos.w-238, 10);
	outleft->setTooltips("send A");
	outleft->show(TRUE);

	outright=new Avideo(MKQIID(qiid, 0xb36f0ed30cf90240), "send B", this, pinOUT, pos.w-238, 30);
	outright->setTooltips("send B");
	outright->show(TRUE);

	inleft=new Avideo(MKQIID(qiid, 0xc4a8428953c7b200), "receive A", this, pinIN, 230, 10);
	inleft->setTooltips("receive A");
	inleft->show(TRUE);

	inright=new Avideo(MKQIID(qiid, 0xb36f0e96200a0240), "receive B", this, pinIN, 230, 30);
	inright->setTooltips("receive B");
	inright->show(TRUE);

	audio=new Asample(MKQIID(qiid, 0xb360c0a0e00a0240), "audio", this, pinIN, 10, 50);
	audio->setTooltips("audio 2 video effect");
	audio->show(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AvmixBack::~AvmixBack()
{
	int	i;
	for(i=0; i<8; i++)
	{
		delete(in[i]);
		delete(ain[i]);
	}
	delete(out);
	delete(aout);
	delete(outleft);
	delete(outright);
	delete(inleft);
	delete(inright);
	delete(audio);
	delete(back);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AvmixBack::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aeffect * AvmixInfo::create(QIID qiid, char *name, Acapsule *capsule)
{
	return new Avmix(qiid, name, this, capsule);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aplugz * vmixGetInfo()
{
	return new AvmixInfo("vmixInfo", &Avmix::CI, "vmix", "vmix module");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
