/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SOUNDOUTPUT.CPP				(c)	YoY'01						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<windows.h>
#include						<dsound.h>
#include						<assert.h>
#include						<math.h>

#include						"soundoutput.h"
//#include						"leffect.h"
#include						"resource.h"

#include						"aasio.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								AsoundOutputInfo::CI	= ACI("AsoundOutputInfo",	GUID(0x11111112,0x00000080), &AeffectInfo::CI, 0, NULL);
ACI								AsoundOutput::CI		= ACI("AsoundOutput",		GUID(0x11111112,0x00000081), &Aeffect::CI, 0, NULL);
ACI								AsoundOutputFront::CI	= ACI("AsoundOutputFront",	GUID(0x11111112,0x00000082), &AeffectFront::CI, 0, NULL);
ACI								AsoundOutputBack::CI	= ACI("AsoundOutputBack",	GUID(0x11111112,0x00000083), &AeffectBack::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------------------------------------------------
#ifdef SOUNDDEBUG
//-----------------------------------------------------------------------------------------------------------------------

#define MAXMEMSOUND				(8192)
#define MAXNBSOUND				(25)

static int posy=10;

//-----------------------------------------------------------------------------------------------------------------------

class AsoWin : public Awindow
{
public:
	AOBJ

								AsoWin							(char *name, int x, int y, int w, int h);
	virtual						~AsoWin							();

	void						paint							(Abitmap *b);
	void						pulse							();

	void						add								(short *buffer, int nsmp);
	void						setText							(int index, char *text);

	Asection					section;

	sword						buffer[MAXNBSOUND][MAXMEMSOUND*2];
	int							size[MAXNBSOUND];
	int							abuf;
	char						values[10][256];
};

//-----------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------

ACI								AsoWin::CI	= ACI("AsoWin",	GUID(0x11111112,0x00000084), &Awindow::CI, 0, NULL);

//-----------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------

AsoWin::AsoWin(char *name, int x, int y, int w, int h) : Awindow(name, x, y, w, h)
{
	abuf=0;
	memset(buffer, 0, sizeof(buffer));
	memset(size, 0, sizeof(size));
	memset(values, 0, sizeof(values));
	timer(40);
}

//-----------------------------------------------------------------------------------------------------------------------

AsoWin::~AsoWin()
{

}

//-----------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------

void AsoWin::pulse()
{
	repaint();
}

//-----------------------------------------------------------------------------------------------------------------------

void AsoWin::paint(Abitmap *b)
{
	b->boxf(0, 0, pos.w, pos.h, 0xff000000);
	b->line(32, 0, 32, pos.h, 0xff0000ff);
	section.enter(__FILE__,__LINE__);
	{
		int max=mini(pos.w, size[abuf]);
		int	i;
		for(i=0; i<max; i++)
		{
			int n=i<<1;
			int y=((int)buffer[abuf][n]*pos.h/32768)+(pos.h>>1);
			b->line(i,  y, i, pos.h>>1, 0xffffff00);
			y=((int)buffer[abuf][n+1]*pos.h/32768)+(pos.h>>1);
			b->pixel(i,  y, 0xffff0000);
		}
	}
	{
		Afont	*f=alib.getFont(fontTERMINAL06);
		int	i;
		for(i=0; i<10; i++)
		{
			f->set(b, 10, 10+i*10, values[i], 0xffffffff);
		}
	}
	section.leave();
}

//-----------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------

void AsoWin::add(short *buffer, int nsmp)
{
	int size=mini((int)nsmp*sizeof(sword)*2, (int)(MAXMEMSOUND*sizeof(sword)*2));
	section.enter(__FILE__,__LINE__);
	abuf=(abuf+1)%MAXNBSOUND;
	memcpy(&this->buffer[abuf][0], buffer, size);
	this->size[abuf]=size;
	section.leave();
}

//-----------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------

void AsoWin::setText(int index, char *text)
{
	section.enter(__FILE__,__LINE__);
	strcpy(values[index], text);
	section.leave();
}

//-----------------------------------------------------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AsoundOutput::AsoundOutput(QIID qiid, char *name, AeffectInfo *info, Acapsule *capsule) : Aeffect(qiid, name, info, capsule)
{
	bOK=FALSE;
	lpDirectSound=NULL;
	memset(dev, 0, sizeof(dev));
	ndev=0;
	memset(&dbd, 0, sizeof(dbd));
 	lpDirectBuffer=NULL;
 	lpDirectPrimaryBuffer=NULL;
	lpDirectNotify=NULL;
	memset(rghEvent, 0, sizeof(rghEvent));
//	opos=0;
	hThread=NULL;
	rulez=FALSE;
	vleft=vright=vlatency=0;

	asioDevice=-1;
	
	buffer=new Abuffer("audio", 44100*4);
	
	start=false;
	stop=true;
	mconnected=false;

	bOK=dsoundInit();
	if(!bOK)
		getTable()->notify(getTable(), nyERROR, (dword)"can't initialize audio input");
	front=new AsoundOutputFront(qiid, "sound input front", this, 50);
	front->setTooltips("audio output module");
	back=new AsoundOutputBack(qiid, "sound input back", this, 50);
	back->setTooltips("audio output module");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AsoundOutput::~AsoundOutput()
{
	if(bOK)
	{
		dsoundStop();
		dsoundClose();
	}
	delete(buffer);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AsoundOutput::settings(bool emergency)
{
	AsoundOutputFront	*front=(AsoundOutputFront *)this->front;
	front->notify(front->drivers, nyCHANGE, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AsoundOutput::load(class Afile *f)
{
	return ((AsoundOutputFront *)front)->load(f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AsoundOutput::save(class Afile *f)
{
	return ((AsoundOutputFront *)front)->save(f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AsoundOutput::actionStart(double time)
{
	start=true;
	stop=false;
	mconnected=false;
	vleft=vright=vlatency=0;
}
	
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AsoundOutput::actionStop()
{
	stop=true;
	mconnected=false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AsoundOutput::action(double time, double dtime, double beat, double dbeat)
{
	AsoundOutputFront *front=(AsoundOutputFront*)this->front; 
	Asample	*pin=(Asample *)((AsoundOutputBack *)back)->in;
	bool connected=pin->isConnected();
	pin->enter(__FILE__,__LINE__);
	if(dsoundSection.enter(__FILE__,__LINE__))
	{
		if(connected&&!mconnected)
		{
			buffer->clear();
			int		avaible=pin->avaible();
			pin->skip(avaible);
		}
		dsoundSection.leave();
		mconnected=connected;
	}
	if(dsoundSection.enter(__FILE__,__LINE__))
	{
		int		avaible=pin->avaible();
		int		buffree=(int)buffer->getFreeSpace();
		int		size=mini(avaible, buffree/(sizeof(sword)*2));
		dword	data[32768];
		int		used=0;
		int		tused=0;

#ifdef SOUNDDEBUG
		front->swin.enter(__FILE__,__LINE__);
		if(front->sowin)
		{
			char str[256];
			int	filled=(int)((asioDevice==-1)?buffer->getSize():asioGetBufferSize());
			sprintf(str, "avaible %d", avaible);
			front->sowin->setText(0, str);
			sprintf(str, "filled %d", filled);
			front->sowin->setText(1, str);
			sprintf(str, "free %d", buffree);
			front->sowin->setText(2, str);
		}
		front->swin.leave();
#endif

		while(size)
		{
			used=mini(size, sizeof(data));
			tused+=used;
			pin->getBuffer((sword *)data, used);
			if(asioDevice==-1)	// DSOUND
			{
				buffer->write(data, used*4);
				sqword	s=buffer->getSize();
				vlatency=mini((float)s/(float)buffer->bufsize, 1.f);
			}
			else	// ASIO
			{
				asioAddBuffer((sword *)data, used);
				vlatency=0.1f;
			}

#ifdef SOUNDDEBUG
			if(size==used)
			{
				int	s=mini(size, MAXMEMSOUND);
				front->swin.enter(__FILE__,__LINE__);
				if(front->sowin)
					front->sowin->add((short *)data, s);
				front->swin.leave();
			}
#endif

			size-=used;
		}
		if(used)
			analyse(data, used); 
		dsoundSection.leave();
	}
	pin->leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float AsoundOutput::getStreamQuality(class Apin *pin)
{
	assert((pin->type&pinDIR)==pinIN);
	return 0.f;
	//return quality;
}
	
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AsoundOutputFront::load(class Afile *f)
{
	char	drv[1024];
	dword	data=666;
	Aitem	*i=NULL;
	if(!f->readString(drv))
		return false;
	if(!f->read(&data, sizeof(data)))
		return false;
	if(drivers->setCurrentByData(data))
		i=drivers->getCurrent();
	if((!i)||strcmp(i->name, drv))
	{
		i=(Aitem *)driverItems->fchild;
		while(i)
		{
			if(!strcmp(i->name, drv))
				break;
			i=(Aitem *)i->next;
		}
	}
	if(i)
	{
		AsoundOutput	*si=(AsoundOutput *)effect;
		drivers->setCurrentByData(i->data);
		if(si->bOK)
		{
			si->dsoundStop();
			//memset(((AsoundInputBack *)si->back)->out->sample, 0, ((AsoundInputBack *)si->back)->out->size*4);
			if(!si->dsoundStart(i->data))
				drivers->setCurrentByData(666);
		}
		else
			drivers->setCurrentByData(666);
	}
	else
	{
		AsoundOutput	*si=(AsoundOutput *)effect;
		drivers->setCurrentByData(666);
		if(si->bOK)
			si->dsoundStop();
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AsoundOutputFront::save(class Afile *f)
{
	Aitem	*i=drivers->getCurrent();
	if(i)
	{
		dword	data=i->data;
		if(!f->writeString(i->name))
			return false;
		if(!f->write(&data, sizeof(data)))
			return false;
	}
	else
	{
		dword	data=666;
		if(!f->writeString("[none]"))
			return false;
		if(!f->write(&data, sizeof(data)))
			return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AsoundOutputFront::AsoundOutputFront(QIID qiid, char *name, AsoundOutput *e, int h) : AeffectFront(qiid, name, e, h)
{
#ifdef SOUNDDEBUG
	sowin=null;
	bwin=null;
#endif

	AsoundOutput	*soundOutput=e;
	Aresobj			o=resource.get(MAKEINTRESOURCE(PNG_SOUNDOUTPUT), "PNG");
	back=new Abitmap(&o);

	int	hb=(50-32)>>1;

	left=new Adisplay("left vue meter", this, 450, hb, displayVM0001);
	left->setTooltips("left vue meter");
	left->show(TRUE);

	right=new Adisplay("right vue meter", this, 470, hb, displayVM0001);
	right->setTooltips("right vue meter");
	right->show(TRUE);

	latency=new Aprogress("latency", this, 320, (50-16)>>1);
	latency->setTooltips("latency");
	latency->show(true);

	clear=new Abutton("clear", this, 260, (50-16)>>1, 50, 16, "CLEAR");
	clear->setTooltips("clear buffer");
	clear->show(true);

	driverItems=new Aitem("directX audio input", "directX audio input drivers");
	if(e->bOK)
	{
		int	i;
		for(i=0; i<e->ndev; i++)
			new Aitem(e->dev[i].name, e->dev[i].module, driverItems, i);

		{
			int	nb=soundOutput->asioGetNumDev();
			for(i=0; i<nb; i++)
			{
				char	name[256];
				soundOutput->asioGetDriverName(i, name, sizeof(name));
				new Aitem(name, name, driverItems, i+100);
			}
		}

		new Aitem("[none]", "no input", driverItems, 666);
	}

	mixer=new Abutton("mixer", this, 16, 15, 20, 20, &resource.get(MAKEINTRESOURCE(PNG_SNDMIX), "PNG"));
	mixer->setTooltips("display volume control");
	mixer->show(true);

	drivers=new Alist("audio driver", this, 16+22, 15, 250-32-22, 20, driverItems);
	if(e->bOK)
		drivers->setCurrentByData(666);
	drivers->setTooltips("audio driver selector");
	drivers->show(TRUE);


#ifdef SOUNDDEBUG
	bwin=new Abutton("bwin", this, 260, 2, 50, 14, "WIN DBG");
	bwin->setTooltips("show audio debug window");
	bwin->show(true);
#endif

	m_vleft=0.f;
	m_vright=0.f;
	m_vlatency=0.f;

	timer(40);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AsoundOutputFront::~AsoundOutputFront()
{
	delete(mixer);
	delete(back);
	delete(left);
	delete(right);
	delete(drivers);
	delete(driverItems);
	delete(clear);
#ifdef SOUNDDEBUG
	if(bwin)
		delete(bwin);
	if(sowin)
		delete(sowin);
	sowin=null;
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AsoundOutputFront::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AsoundOutputFront::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCLICK:
		if(o==mixer)
		{
			if(((AsoundOutput *)effect)->asioDevice==-1)
				exec("sndvol32 /play");
			else
				((AsoundOutput *)effect)->asioControlPanel();
		}
		else if(o==clear)
		{
			//_CrtDumpMemoryLeaks();
			if(((AsoundOutput *)effect)->dsoundSection.enter(__FILE__,__LINE__))
			{
				((AsoundOutput *)effect)->buffer->clear();
				((AsoundOutput *)effect)->dsoundSection.leave();
			}
		}
#ifdef SOUNDDEBUG
		else if(o==bwin)
		{
			swin.enter(__FILE__,__LINE__);
			if(sowin)
			{
				delete(sowin);
				sowin=null;
			}
			else
			{
				sowin=new AsoWin("sowin", 10, posy, 800, 200);
				posy+=210;
				sowin->show(true);
			}
			swin.leave();
		}
#endif
		break;
		
		case nyCHANGE:
		{
			AsoundOutput	*si=(AsoundOutput *)effect;
			if(si->bOK)
			{
				AsoundOutputBack	*sib=(AsoundOutputBack *)si->back;
				if(o==drivers)
				{
					si->dsoundStop();
					//memset(sib->out->sample, 0, sib->out->size*4);
					if(!si->dsoundStart(drivers->getCurrentData()))
					{
						notify(this, nyERROR, (dword)"can't start audio output [select another device or close other audio software]");
						drivers->setCurrentByData(666);
					}
				}
			}
		}
		return TRUE;
	}
	return AeffectFront::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AsoundOutputFront::pulse()
{
	AsoundOutput	*si=(AsoundOutput *)effect;
	m_vleft=(m_vleft+si->vleft)*0.5f;
	m_vright=(m_vright+si->vright)*0.5f;
	m_vlatency=(si->vlatency+m_vlatency)*0.5f;
	left->set(m_vleft);
	right->set(m_vright);
	latency->set(m_vlatency);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AsoundOutputBack::AsoundOutputBack(QIID qiid, char *name, AsoundOutput *e, int h) : AeffectBack(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_SOUNDOUTPUT2), "PNG");
	back=new Abitmap(&o);

	in=new Asample(MKQIID(qiid, 0x2012cf406a076540), "audio in", this, pinIN, 10, 10);
	in->setTooltips("audio intput");
	in->show(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AsoundOutputBack::~AsoundOutputBack()
{
	delete(back);
	delete(in);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AsoundOutputBack::paint(Abitmap *b)
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

__inline bool test(dword a, dword b) { return (a&b)==b; }

static BOOL CALLBACK dsoundEnum(LPGUID lpGuid, LPCTSTR lpName, LPCTSTR lpModule, void *lpContext)
{
	HRESULT	r;
	{
		LPDIRECTSOUND				lpDirectSound;
		DSCAPS						dc;
		r=DirectSoundCreate(lpGuid, &lpDirectSound, NULL);
		if(r!=DS_OK)
			return TRUE;
		dc.dwSize=sizeof(dc);
		lpDirectSound->GetCaps(&dc);
		lpDirectSound->Release();
		if(!(test(dc.dwFlags ,DSCAPS_PRIMARYSTEREO|DSCAPS_PRIMARY16BIT)&&(dc.dwMinSecondarySampleRate<=44100)&&(dc.dwMaxSecondarySampleRate>=44100)))
			return true;
	}
	{
		AsoundOutput	*si=(AsoundOutput *)lpContext;
		ATOdevice		*d=&si->dev[si->ndev++];
		if(lpGuid)
			d->guid=*lpGuid;
		d->name=strdup(lpName);
		d->module=strdup(lpModule);
		if(si->ndev>=MAXDSOUNDDEVICE)
			return FALSE;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AsoundOutput::dsoundInit()
{
	lpDirectSound=NULL;
	lpDirectBuffer=NULL;
	lpDirectNotify=NULL;
	ndev=0;
	memset(dev, 0, sizeof(dev));
	if(DirectSoundEnumerate(dsoundEnum, this)!=DS_OK)
	{
		return FALSE;
	}
 	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AsoundOutput::dsoundClose()
{
	{
		int	i;
		for(i=0; i<ndev; i++)
		{
			if(dev[i].name)
				free(dev[i].name);
			if(dev[i].module)
				free(dev[i].module);
		}
		memset(dev, 0, sizeof(dev));
		ndev=0;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static DWORD __stdcall waitEvent(void *p)
{
	AsoundOutput	*si=(AsoundOutput *)p;

	si->hThread=GetCurrentThread();

	if(!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL))
		si->front->asyncNotify(si, nyERROR, (dword)"can't set the audio out as time critical, some glitch can occure");

	while(si->rulez)
	{
		dword e=MsgWaitForMultipleObjects(NUMEVENTS, si->rghEvent, FALSE, 10, 0)-WAIT_OBJECT_0;
		if(e<NUMEVENTS)
		{
			if(si->rulez)
				si->dsoundRulez(e);
		}
		else if(e==NUMEVENTS)
		{
			//assert(NULL);
		}
	}

	si->hThread=null;
	
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AsoundOutput::dsoundRulez(dword e)
{
	AsoundOutputFront *front=(AsoundOutputFront*)this->front; 
	dsoundSection.enter(__FILE__,__LINE__);
	{
		if(lpDirectBuffer)
		{
			HRESULT	r;
			DWORD	size;
			DWORD	s1,s2;
			void	*p1,*p2;
			size=dbd.dwBufferBytes>>1;
			r=lpDirectBuffer->Lock((e==0)?size:0, size, &p1, &s1, &p2, &s2, 0);
			if(r==DS_OK)
			{
				dword	avaible=(dword)buffer->getSize();
				bool	bstart=false;
				if(start)
				{
					float	vfps=(float)getTable()->frameRate;
					float	sfps=(44100.f/((float)s1*0.25f));
					dword	min=(int)maxi((float)s1*1.5f, (float)s1*vfps/sfps);
					min=mini(min, buffer->bufsize/2)&~3;
					if(avaible>min)
					{
						start=false;
						bstart=true;
					}
				}
				if((!stop)&&(!start))
				{
					if(avaible>s1)
					{
						buffer->read(p1, s1);
						avaible-=s1;
						if(bstart)
						{
							short	*pv=(short *)p1;
							int		i;
							for(i=0; i<16; i++)
							{
								int	n0=(i<<1);
								pv[n0]=(short)((i*(int)pv[n0])>>4);
								pv[n0+1]=(short)((i*(int)pv[n0+1])>>4);
							}
						}
					}
					else
					{
						assert(avaible==((avaible/4)*4));	// 4 byte audio align
						buffer->read(p1, avaible);
						{
							int		i;
							int		n=mini(16*4, avaible)/4;
							short	*pv=(short *)((byte *)p1+avaible);
							short	*pve=pv;
							pv-=n*2;
							for(i=0; i<n; i++)
							{
								int	i0=n-i;
								int	n0=(i<<1);
								pv[n0]=(short)(i0*(int)pv[n0]/n);
								pv[n0+1]=(short)(i0*(int)pv[n0+1]/n);
							}
							memset(pve, 0, s1-avaible);
						}
						start=true;	// error
						avaible=0;
					}
				}
				else
				{
					memset(p1, 0, s1);
					vleft=0.f;
					vright=0.f;
#ifdef SOUNDDEBUG
					front->swin.enter(__FILE__,__LINE__);
					if(front->sowin)
					{
						front->sowin->setText(3, "Dsound send ERROR");
					}
					front->swin.leave();
#endif
				}
				lpDirectBuffer->Unlock(p1, s1, p2, s2);

#ifdef SOUNDDEBUG
				front->swin.enter(__FILE__,__LINE__);
				if(front->sowin)
				{
					char	str[256];
					sprintf(str, "framerate %d", getTable()->frameRate);
					front->sowin->setText(4, str);
					sprintf(str, "sound fps %8.2f", (44100.f/((float)s1*0.25f)));
					front->sowin->setText(5, str);
				}
				front->swin.leave();
#endif
			}
			else
			{
				#define DIRECT_SOUND_BUFFER_LOCK_ERROR 0
				assert(DIRECT_SOUND_BUFFER_LOCK_ERROR);
			}
		}
	}
	dsoundSection.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static WAVEFORMATEX	wfx={WAVE_FORMAT_PCM, 2, 44100, 176400, 4, 16, 0};

bool AsoundOutput::dsoundStart(int device)
{
	char	*err=NULL;
	dsoundSection.enter(__FILE__,__LINE__);
	{
		if(device<ndev)
		{
			HRESULT			r;
			r=DirectSoundCreate(&dev[device].guid, &lpDirectSound, NULL);
			if(r!=DS_OK)
				return FALSE;
			r=lpDirectSound->SetCooperativeLevel(GetDesktopWindow(), DSSCL_PRIORITY);
			if(r!=DS_OK)
				return FALSE;
			{
				memset(&dbd, 0, sizeof(dbd));
				dbd.dwSize=sizeof(dbd);
				dbd.dwFlags=DSBCAPS_PRIMARYBUFFER;
				r=lpDirectSound->CreateSoundBuffer(&dbd, &lpDirectPrimaryBuffer, NULL);
				switch(r)
				{
					case DSERR_INVALIDPARAM:
					err="create buffer, invalid param";
					break;
					case DSERR_BADFORMAT:
					err="create buffer, bad format";
					break;
					case DSERR_GENERIC:
					err="create buffer, generic";
					break;
					case DSERR_NODRIVER:
					err="create buffer, no driver";
					break;
					case DSERR_OUTOFMEMORY:
					err="create buffer, out of memory";
					break;
					case DSERR_UNINITIALIZED:
					err="create buffer, uninitialized";
					break;
				}
				if(r!=DS_OK)
				{
					dsoundStop();
					dsoundSection.leave();
					return FALSE;
				}
			}
			r=lpDirectPrimaryBuffer->SetFormat(&wfx);
			switch(r)
			{
				case DSERR_BADFORMAT:
				err="SetFormat: bad format";
				break;
				case DSERR_INVALIDCALL:
				err="SetFormat: invalid call";
				break;
				case DSERR_INVALIDPARAM:
				err="SetFormat: invalid format";
				break;
				case DSERR_OUTOFMEMORY:
				err="SetFormat: out of memory";
				break;
				case DSERR_PRIOLEVELNEEDED:
				err="SetFormat: priority level needed";
				break;
				case DSERR_UNSUPPORTED:
				err="SetFormat: unsurported";
				break;
			}
			if(r!=DS_OK)
			{
				dsoundStop();
				dsoundSection.leave();
				return FALSE;
			}
			r=lpDirectPrimaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
			if(r!=DS_OK)
			{
				dsoundSection.leave();
				dsoundStop();
				return FALSE;
			}
			{
				dsize=((int)(getTable()->dsoundsize*(float)(DSOUNDSIZE-DSOUNDSIZEMIN))&(~1))+DSOUNDSIZEMIN;
			}
			{	// create seconday buffer (primary can have events)
				memset(&dbd, 0, sizeof(dbd));
				dbd.dwSize=sizeof(dbd);
				dbd.dwFlags=DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_STICKYFOCUS | DSBCAPS_GLOBALFOCUS; 
				dbd.dwBufferBytes=dsize*4;
				dbd.dwReserved=0;
				dbd.lpwfxFormat=&wfx;
				r=lpDirectSound->CreateSoundBuffer(&dbd, &lpDirectBuffer, NULL);
				switch(r)
				{
					case DSERR_INVALIDPARAM:
					err="create buffer, invalid param";
					break;
					case DSERR_BADFORMAT:
					err="create buffer, bad format";
					break;
					case DSERR_GENERIC:
					err="create buffer, generic";
					break;
					case DSERR_NODRIVER:
					err="create buffer, no driver";
					break;
					case DSERR_OUTOFMEMORY:
					err="create buffer, out of memory";
					break;
					case DSERR_UNINITIALIZED:
					err="create buffer, uninitialized";
					break;
				}
				if(r!=DS_OK)
				{
					dsoundStop();
					dsoundSection.leave();
					return FALSE;
				}
			}
			{
				int	i;
				for(i=0; i<NUMEVENTS; i++)
				{
					rghEvent[i]=CreateEvent(NULL, FALSE, FALSE, NULL);
					if(!rghEvent[i])
					{
						dsoundStop();
						dsoundSection.leave();
						return FALSE;
					}
				}
			}
			r=lpDirectBuffer->QueryInterface(IID_IDirectSoundNotify, (void **)&lpDirectNotify);
			if(r!=DS_OK)
			{
				dsoundStop();
				dsoundSection.leave();
				return FALSE;
			}
			{
				DSBPOSITIONNOTIFY	dny[NUMEVENTS];
				dny[0].dwOffset=0;
				dny[0].hEventNotify=rghEvent[0];
				dny[1].dwOffset=dbd.dwBufferBytes/2;
				dny[1].hEventNotify=rghEvent[1];
				r=lpDirectNotify->SetNotificationPositions(NUMEVENTS, dny);
				if(r!=DS_OK)
				{
					dsoundSection.leave();
					dsoundStop();
					return FALSE;
				}
			}
			{
				DWORD	id;
				rulez=TRUE;
				hThread=CreateThread(NULL, 0, waitEvent, this, 0, &id);
				if(!hThread)
				{
					dsoundSection.leave();
					dsoundStop();
					return FALSE;
				}
			}
			/*
			r=lpDirectBuffer->SetVolume(DSBVOLUME_MAX);
			if(r!=DS_OK)
			{
				dsoundSection.leave();
				dsoundStop();
				return FALSE;
			*/
			r=lpDirectBuffer->Play(0, 0, DSBPLAY_LOOPING);
			if(r!=DS_OK)
			{
				dsoundSection.leave();
				dsoundStop();
				return FALSE;
			}
		}
		else if((device>=100)&&(device<666))
		{
			if(!asioStart(device-100))
			{
				dsoundSection.leave();
				dsoundStop();
				return FALSE;
			}
		}
	}
	dsoundSection.leave();
	start=true;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AsoundOutput::dsoundStop()
{
	rulez=FALSE;

	while(hThread)
		Athread::sleep(1);

	dsoundSection.enter(__FILE__,__LINE__);
	{
		if(lpDirectNotify)
			lpDirectNotify->Release();
		lpDirectNotify=NULL;
		if(lpDirectBuffer)
		{
			lpDirectBuffer->Stop();
			lpDirectBuffer->Release();
		}
		lpDirectBuffer=NULL;
		if(lpDirectPrimaryBuffer)
		{
			lpDirectPrimaryBuffer->Stop();
			lpDirectPrimaryBuffer->Release();
		}
		lpDirectPrimaryBuffer=NULL;
		if(lpDirectSound)
			lpDirectSound->Release();
		lpDirectSound=NULL;

		asioStop();

		buffer->clear();
	}
	{
		int	i;
		for(i=0; i<NUMEVENTS; i++)
		{
			if(rghEvent[i])
			{
				CloseHandle(rghEvent[i]);
				rghEvent[i]=NULL;
			}
		}
	}
	dsoundSection.leave();
	vleft=0.f;
	vright=0.f;
	vlatency=0.f;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AsoundOutput::asioStart(int device)
{
	bool	b=false;
	if(asioCreate(device))
	{
		if(asioGetNbOutput()>=2)
		{
			int	n=asioGetFirstOutput();
			asioUseChannel(n, n+1);
			asioDevice=device;
		}
		else
			asioRelease();
		b=true;
	}
	return b;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AsoundOutput::asioStop()
{
	asioRelease();
	asioDevice=-1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AsoundOutput::analyse(void *p, dword s)
{
	int		n=s/4;
	int		i;
	sword	*p0=(sword *)p;
	sqword	sl=0;
	sqword	sr=0;
	for(i=0; i<n; i++)
	{
		sl+=abs(*(p0++));
		sr+=abs(*(p0++));
	}
	vleft=(float)sl/((float)n*8192.f);
	vright=(float)sr/((float)n*8192.f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aeffect * AsoundOutputInfo::create(QIID qiid, char *name, Acapsule *capsule)
{
	return new AsoundOutput(qiid, name, this, capsule);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aplugz * soundOutputGetInfo()
{
	return new AsoundOutputInfo("soundOutputInfo", &AsoundOutput::CI, "audio out", "audio out module");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
