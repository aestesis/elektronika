/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SOUNDINPUT.CPP				(c)	YoY'00						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<dsound.h>
#include						<assert.h>

#include						"soundinput.h"
//#include						"leffect.h"
#include						"resource.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								AsoundInputInfo::CI	= ACI("AsoundInputInfo",	GUID(0x11111112,0x00000000), &AeffectInfo::CI, 0, NULL);
ACI								AsoundInput::CI		= ACI("AsoundInput",		GUID(0x11111112,0x00000001), &Aeffect::CI, 0, NULL);
ACI								AsoundInputFront::CI= ACI("AsoundInputFront",	GUID(0x11111112,0x00000002), &AeffectFront::CI, 0, NULL);
ACI								AsoundInputBack::CI	= ACI("AsoundInputBack",	GUID(0x11111112,0x00000003), &AeffectBack::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct ASdevice
{
	GUID						guid;
	char						*name;
	char						*module;
} ATdevice;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AinfoSC
{
public:

	LPDIRECTSOUNDCAPTURE		lpDirectSoundCapture;
	ATdevice					dev[MAXDSOUNDDEVICE];
	int							ndev;
	DSCBUFFERDESC				dbd;
 	LPDIRECTSOUNDCAPTUREBUFFER  lpDirectBuffer;
	LPDIRECTSOUNDNOTIFY         lpDirectNotify;
	HANDLE                      rghEvent[NUMCAPTUREEVENTS];
	HANDLE						hThread;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AsoundInput::AsoundInput(QIID qiid, char *name, AeffectInfo *info, Acapsule *capsule) : Aeffect(qiid, name, info, capsule)
{
	bOK=FALSE;
	scinfo=new AinfoSC();
	
	scinfo->lpDirectSoundCapture=NULL;
	memset(scinfo->dev, 0, sizeof(scinfo->dev));
	scinfo->ndev=0;
	memset(&scinfo->dbd, 0, sizeof(scinfo->dbd));
 	scinfo->lpDirectBuffer=NULL;
	scinfo->lpDirectNotify=NULL;
	memset(scinfo->rghEvent, 0, sizeof(scinfo->rghEvent));
//	opos=0;
	scinfo->hThread=NULL;
	rulez=FALSE;

	memset(buffer, 0, sizeof(buffer));
	breste=bread=blen=0;

	bOK=dsoundInit();
	if(!bOK)
		getTable()->notify(getTable(), nyERROR, (dword)"can't initialize audio input");
	front=new AsoundInputFront(qiid, "sound input front", this, 50);
	front->setTooltips("audio input module");
	back=new AsoundInputBack(qiid, "sound input back", this, 50);
	back->setTooltips("audio input module");
}

AsoundInput::~AsoundInput()
{
	if(bOK)
	{
		dsoundStop();
		dsoundClose();
	}
	delete(scinfo);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AsoundInput::settings(bool emergency)
{
	AsoundInputFront	*front=(AsoundInputFront *)this->front;
	front->notify(front->drivers, nyCHANGE, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AsoundInput::load(class Afile *f)
{
	return ((AsoundInputFront *)front)->load(f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AsoundInput::save(class Afile *f)
{
	return ((AsoundInputFront *)front)->save(f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AsoundInput::action(double time, double dtime, double beat, double dbeat)
{
	AsoundInputFront	*front=(AsoundInputFront *)this->front;
	bool				monoL=front->monoL->isChecked();
	bool				monoR=front->monoR->isChecked();

	bsection.enter(__FILE__,__LINE__);
	{
		Asample	*pin=(Asample *)((AsoundInputBack *)back)->out;
		sqword	nsmp=((sqword)dtime*44100/1000);
		sqword	v;
		breste+=(sqword)(dtime*44100-nsmp*1000);
		v=breste/1000;
		nsmp+=v;
		breste-=v*1000;
		
		if(blen>=nsmp)
		{
			pin->enter(__FILE__,__LINE__);
			if(bread+nsmp>MAXINTBUF)
			{
				if(monoL||monoR)
				{
					int	i;
					if(monoL)
					{
						int	n=MAXINTBUF-bread;
						int	s=bread;
						
						for(i=0; i<n; i++)
						{
							int	v=(s++)<<1;
							buffer[v+1]=buffer[v];
						}
						
						n=(int)(nsmp-(MAXINTBUF-bread));
						for(i=0; i<n; i++)
						{
							int	v=i<<1;
							buffer[v+1]=buffer[v];
						}
					}
					else
					{
						int	n=MAXINTBUF-bread;
						int	s=bread;
						
						for(i=0; i<n; i++)
						{
							int	v=(s++)<<1;
							buffer[v]=buffer[v+1];
						}
						
						n=(int)(nsmp-(MAXINTBUF-bread));
						for(i=0; i<n; i++)
						{
							int	v=i<<1;
							buffer[v]=buffer[v+1];
						}
					}
				}
				pin->addBuffer(&buffer[bread<<1], (int)(MAXINTBUF-bread));
				pin->addBuffer(&buffer[0], (int)(nsmp-(MAXINTBUF-bread)));
			}
			else
			{
				if(monoL||monoR)
				{
					int	i;
					if(monoL)
					{
						int	s=bread;
						for(i=0; i<nsmp; i++)
						{
							int	v=(s++)<<1;
							buffer[v+1]=buffer[v];
						}
					}
					else
					{
						int	s=bread;
						for(i=0; i<nsmp; i++)
						{
							int	v=(s++)<<1;
							buffer[v]=buffer[v+1];
						}
					}
				}
				pin->addBuffer(&buffer[bread<<1], (int)nsmp);
			}
			vleft=mini(1.f, pin->maxL*0.8f);
			vright=mini(1.f, pin->maxR*0.8f);
			pin->leave();
 			bread=(bread+(int)nsmp)%MAXINTBUF;
			blen-=(int)nsmp;
		}
		else
			breste=0;
	}
	bsection.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AsoundInputFront::load(class Afile *f)
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
		AsoundInput	*si=(AsoundInput *)effect;
		drivers->setCurrentByData(i->data);
		if(si->bOK)
		{
			si->dsoundStop();
			memset(((AsoundInputBack *)si->back)->out->sample, 0, ((AsoundInputBack *)si->back)->out->size*4);
			if(!si->dsoundStart(i->data))
				drivers->setCurrentByData(666);
		}
		else
			drivers->setCurrentByData(666);
	}
	else
	{
		AsoundInput	*si=(AsoundInput *)effect;
		drivers->setCurrentByData(666);
		if(si->bOK)
			si->dsoundStop();
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AsoundInputFront::save(class Afile *f)
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

AsoundInputFront::AsoundInputFront(QIID qiid, char *name, AsoundInput *e, int h) : AeffectFront(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_SOUNDINPUT), "PNG");
	back=new Abitmap(&o);

	int	hb=(50-32)>>1;
	
	volume=new Apaddle(MKQIID(qiid, 0x04a9a7a9e5ff01b0), "master", this, 399, 12, paddleYOLI24);
	volume->setTooltips("master level");
	volume->set(0.5f);
	volume->show(TRUE);

	bass=new Apaddle(MKQIID(qiid, 0xa5ef6f7313ca82c0), "eq/bass", this, 255, 10, paddleBUTTON02);
	bass->setTooltips("bass level");
	bass->set(0.5f);
	bass->show(TRUE);

	medium=new Apaddle(MKQIID(qiid, 0x00f0c3df912e1a64), "eq/medium", this, 297, 10, paddleBUTTON02);
	medium->setTooltips("medium level");
	medium->set(0.5f);
	medium->show(TRUE);

	treble=new Apaddle(MKQIID(qiid, 0x7fb2e64af06ffc40), "eq/treble", this, 339, 10, paddleBUTTON02);
	treble->setTooltips("treble level");
	treble->set(0.5f);
	treble->show(TRUE);

	left=new Adisplay("left vue meter", this, 450, hb, displayVM0001);
	left->setTooltips("left vue meter");
	left->show(TRUE);

	right=new Adisplay("right vue meter", this, 470, hb, displayVM0001);
	right->setTooltips("right vue meter");
	right->show(TRUE);

	driverItems=new Aitem("directX audio input", "directX audio input drivers");
	if(e->bOK)
	{
		int	i;
		for(i=0; i<e->scinfo->ndev; i++)
			new Aitem(e->scinfo->dev[i].name, e->scinfo->dev[i].module, driverItems, i);
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

	monoL=new Abutton("monoL", this, 379, 10, 12, 12, &resource.get(MAKEINTRESOURCE(PNG_SOUNDINPUT_LEFT), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	monoL->setTooltips("set mono from left channel");
	monoL->show(true);

	monoR=new Abutton("monoR", this, 379, 25, 12, 12, &resource.get(MAKEINTRESOURCE(PNG_SOUNDINPUT_RIGHT), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	monoR->setTooltips("set mono from right channel");
	monoR->show(true);

	m_vleft=0.f;
	m_vright=0.f;

	timer(40);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AsoundInputFront::~AsoundInputFront()
{
	delete(mixer);
	delete(volume);
	delete(bass);
	delete(medium);
	delete(treble);
	delete(back);
	delete(left);
	delete(right);
	delete(drivers);
	delete(driverItems);
	delete(monoL);
	delete(monoR);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AsoundInputFront::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AsoundInputFront::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCLICK:
		exec("sndvol32 /record");
		break;
		
		case nyCHANGE:
		{
			AsoundInput	*si=(AsoundInput *)effect;
			if(si->bOK)
			{
				AsoundInputBack	*sib=(AsoundInputBack *)si->back;
				if(o==drivers)
				{
					si->dsoundStop();
					memset(sib->out->sample, 0, sib->out->size*4);
					if(!si->dsoundStart(drivers->getCurrentData()))
					{
						notify(this, nyERROR, (dword)"can't start audio input [select another device or close other audio software]");
						drivers->setCurrentByData(666);
					}
				}
				else if(o==volume)
				{
					Asample	*s=sib->out;
					s->enter(__FILE__,__LINE__);
					s->vmaster=volume->get()*2.f;
					s->leave();
				}
				else if(o==bass)
				{
					Asample	*s=sib->out;
					s->enter(__FILE__,__LINE__);
					s->vbass=bass->get()*2.f;
					s->leave();
				}
				else if(o==medium)
				{
					Asample	*s=sib->out;
					s->enter(__FILE__,__LINE__);
					s->vmedium=medium->get()*2.f;
					s->leave();
				}
				else if(o==treble)
				{
					Asample	*s=sib->out;
					s->enter(__FILE__,__LINE__);
					s->vtreeble=treble->get()*2.f;
					s->leave();
				}
				else if(o==monoL)
				{
					monoR->setChecked(false);
				}
				else if(o==monoR)
				{
					monoL->setChecked(false);
				}
			}
		}
		return TRUE;
	}
	return AeffectFront::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AsoundInputFront::pulse()
{
	AsoundInput	*si=(AsoundInput *)effect;
	m_vleft=(m_vleft+si->vleft)*0.5f;
	m_vright=(m_vright+si->vright)*0.5f;
	left->set(m_vleft);
	right->set(m_vright);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AsoundInputBack::AsoundInputBack(QIID qiid, char *name, AsoundInput *e, int h) : AeffectBack(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_SOUNDINPUT2), "PNG");
	back=new Abitmap(&o);

	out=new Asample(MKQIID(qiid, 0x0233b31dcd216c00), "audio out", this, pinOUT, pos.w-18, 10);
	out->setTooltips("audio output");
	out->show(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AsoundInputBack::~AsoundInputBack()
{
	delete(back);
	delete(out);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AsoundInputBack::paint(Abitmap *b)
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

static BOOL CALLBACK dsoundEnum(LPGUID lpGuid, LPCTSTR lpName, LPCTSTR lpModule, void *lpContext)
{
	HRESULT	r;
	{
		LPDIRECTSOUNDCAPTURE		lpDirectSoundCapture;
		DSCCAPS						dc;
		r=DirectSoundCaptureCreate(lpGuid, &lpDirectSoundCapture, NULL);
		if(r!=DS_OK)
			return TRUE;
		dc.dwSize=sizeof(dc);
		r=lpDirectSoundCapture->GetCaps(&dc);
		//lpDirectSoundCapture->Release();
		if(r!=DS_OK)
			return TRUE;
		if(!(dc.dwFormats&WAVE_FORMAT_4S16))
			return TRUE;
	}
	{
		AsoundInput	*si=(AsoundInput *)lpContext;
		ATdevice	*d=&si->scinfo->dev[si->scinfo->ndev++];
		if(lpGuid)
			d->guid=*lpGuid;
		d->name=strdup(lpName);
		d->module=strdup(lpModule);
		if(si->scinfo->ndev>=MAXDSOUNDDEVICE)
			return FALSE;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AsoundInput::dsoundInit()
{
	scinfo->lpDirectSoundCapture=NULL;
	scinfo->lpDirectBuffer=NULL;
	scinfo->lpDirectNotify=NULL;
	scinfo->ndev=0;
	memset(scinfo->dev, 0, sizeof(scinfo->dev));
	if(DirectSoundCaptureEnumerate(dsoundEnum, this)!=DS_OK)
	{
		return FALSE;
	}
 	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AsoundInput::dsoundClose()
{
	{
		int	i;
		for(i=0; i<scinfo->ndev; i++)
		{
			if(scinfo->dev[i].name)
				free(scinfo->dev[i].name);
			if(scinfo->dev[i].module)
				free(scinfo->dev[i].module);
		}
		memset(scinfo->dev, 0, sizeof(scinfo->dev));
		scinfo->ndev=0;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static DWORD __stdcall waitEvent(void *p)
{
	AsoundInput	*si=(AsoundInput *)p;
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
	while(si->rulez)
	{
		dword e=MsgWaitForMultipleObjects(NUMCAPTUREEVENTS, si->scinfo->rghEvent, FALSE, 100, QS_ALLINPUT)-WAIT_OBJECT_0;
		if(e<NUMCAPTUREEVENTS)
		{
			if(si->rulez)
				si->dsoundRulez(e);
		}
		else if(e==NUMCAPTUREEVENTS)
		{
			assert(NULL);
		/*
			MSG msg;
			while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		*/
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AsoundInput::dsoundRulez(dword e)
{
	if(dsoundSection.enter(__FILE__,__LINE__))
	{
		if(scinfo->lpDirectBuffer)
		{
			HRESULT	r;
			DWORD	size;
			DWORD	s1,s2;
			void	*p1,*p2;
			size=scinfo->dbd.dwBufferBytes>>1;
			r=scinfo->lpDirectBuffer->Lock((e==0)?size:0, size, &p1, &s1, &p2, &s2, 0);
			if(r==DS_OK)
			{
				int		oblen=blen;		// 4debug
				sword	*p0=(sword *)p1;
				int		s=(int)(s1>>2);

				bsection.enter(__FILE__,__LINE__);
				if((blen+s)>MAXINTBUF)
				{
					memcpy(buffer, p0, s1);
					bread=0;
					blen=s;
				}
				else
				{
					int	bwrite=(bread+blen)%MAXINTBUF;
					if((bwrite+s)>MAXINTBUF)
					{
						int	fp=MAXINTBUF-bwrite;
						memcpy(&buffer[bwrite<<1], p0, fp<<2);
						memcpy(&buffer[0], &p0[fp<<1], (s-fp)<<2);
					}
					else
						memcpy(&buffer[bwrite<<1], p0, s1);
					blen+=s;
				}
				/*
				{	// 4debug
					FILE	*f=fopen("c:\\soundtest.txt", "a");
					fprintf(f, "oblen %10d - blen %10d\r\n", oblen, blen);
					fclose(f);
				}
				*/
				bsection.leave();

				//analyse(p1, s1);
				scinfo->lpDirectBuffer->Unlock(p1, s1, p2, s2);


			}
			else
			{
				#define DIRECT_SOUND_BUFFER_LOCK_ERROR 0
				assert(DIRECT_SOUND_BUFFER_LOCK_ERROR);
			}
		}
		dsoundSection.leave();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static WAVEFORMATEX	wfx={WAVE_FORMAT_PCM, 2, 44100, 176400, 4, 16, 0};

bool AsoundInput::dsoundStart(int device)
{
	if(dsoundSection.enter(__FILE__,__LINE__))
	{
		if(device<scinfo->ndev)
		{
			HRESULT			r;
//			opos=0;
			r=DirectSoundCaptureCreate(&scinfo->dev[device].guid, &scinfo->lpDirectSoundCapture, NULL);
			if(r!=DS_OK)
				return FALSE;
			{
				char	*err=NULL;
				int		dsize=((int)(getTable()->dsoundsize*(float)(DSOUNDSIZE-DSOUNDSIZEMIN))&(~1))+DSOUNDSIZEMIN;
				memset(&scinfo->dbd, 0, sizeof(scinfo->dbd));
				scinfo->dbd.dwSize=sizeof(scinfo->dbd);
				scinfo->dbd.dwFlags=0;
				scinfo->dbd.dwBufferBytes=4*dsize;
				scinfo->dbd.dwReserved=0;
				scinfo->dbd.lpwfxFormat=&wfx;
				r=scinfo->lpDirectSoundCapture->CreateCaptureBuffer(&scinfo->dbd, &scinfo->lpDirectBuffer, NULL);
				switch(r)
				{
					case DSERR_INVALIDPARAM:
					err="create capture buffer, invalid param";
					break;
					case DSERR_BADFORMAT:
					err="create capture buffer, bad format";
					break;
					case DSERR_GENERIC:
					err="create capture buffer, generic";
					break;
					case DSERR_NODRIVER:
					err="create capture buffer, no driver";
					break;
					case DSERR_OUTOFMEMORY:
					err="create capture buffer, out of memory";
					break;
					case DSERR_UNINITIALIZED:
					err="create capture buffer, uninitialized";
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
				for(i=0; i<NUMCAPTUREEVENTS; i++)
				{
					scinfo->rghEvent[i]=CreateEvent(NULL, FALSE, FALSE, NULL);
					if(!scinfo->rghEvent[i])
					{
						dsoundStop();
						dsoundSection.leave();
						return FALSE;
					}
				}
			}
			r=scinfo->lpDirectBuffer->QueryInterface(IID_IDirectSoundNotify, (void **)&scinfo->lpDirectNotify);
			if(r!=DS_OK)
			{
				dsoundStop();
				dsoundSection.leave();
				return FALSE;
			}
			{
				DSBPOSITIONNOTIFY	dny[NUMCAPTUREEVENTS];
				dny[0].dwOffset=0;
				dny[0].hEventNotify=scinfo->rghEvent[0];
				dny[1].dwOffset=scinfo->dbd.dwBufferBytes/2;
				dny[1].hEventNotify=scinfo->rghEvent[1];
				r=scinfo->lpDirectNotify->SetNotificationPositions(NUMCAPTUREEVENTS, dny);
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
				scinfo->hThread=CreateThread(NULL, 0, waitEvent, this, 0, &id);
				if(!scinfo->hThread)
				{
					dsoundSection.leave();
					dsoundStop();
					return FALSE;
				}
				SetThreadPriority(scinfo->hThread, THREAD_PRIORITY_TIME_CRITICAL);
			}
			r=scinfo->lpDirectBuffer->Start(DSCBSTART_LOOPING);
			if(r!=DS_OK)
			{
				dsoundSection.leave();
				dsoundStop();
				return FALSE;
			}
		}
		dsoundSection.leave();
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AsoundInput::dsoundStop()
{
	rulez=FALSE;
	if(dsoundSection.enter(__FILE__,__LINE__))
	{
		Sleep(150);
		if(scinfo->lpDirectNotify)
			scinfo->lpDirectNotify->Release();
		scinfo->lpDirectNotify=NULL;
		if(scinfo->lpDirectBuffer)
			scinfo->lpDirectBuffer->Release();
		scinfo->lpDirectBuffer=NULL;
		if(scinfo->lpDirectSoundCapture)
			scinfo->lpDirectSoundCapture->Release();
		scinfo->lpDirectSoundCapture=NULL;
		dsoundSection.leave();
	}
	vleft=0.f;
	vright=0.f;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AsoundInput::analyse(void *p, dword s)
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

Aeffect * AsoundInputInfo::create(QIID qiid, char *name, Acapsule *capsule)
{
	return new AsoundInput(qiid, name, this, capsule);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aplugz * soundInputGetInfo()
{
	return new AsoundInputInfo("soundInputInfo", &AsoundInput::CI, "audio in", "audio in module");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
