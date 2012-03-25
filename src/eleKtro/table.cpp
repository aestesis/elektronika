/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"elektroexp.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	table.CPP					(c)	YoY'01						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<assert.h>
#include						<math.h>
#include						"resource.h"
#include						"table.h"
#include						"effect.h"
#include						"pin.h"
#include						"elektro.h"
#include						"sequencer.h"
#include						"acontrol.h"
#include						"tcpRemote.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL ACI						Atable::CI			= ACI("Atable",			GUID(0xE4EC7600,0x00000000), &Aobject::CI, 0, NULL);
EDLL ACI		 				AtableFront::CI		= ACI("AtableFront",	GUID(0xE4EC7600,0x00000010), &Aobject::CI, 0, NULL);
EDLL ACI						AtableBack::CI		= ACI("AtableBack",		GUID(0xE4EC7600,0x00000020), &Aobject::CI, 0, NULL);
EDLL ACI						Actbar::CI			= ACI("Actbar",			GUID(0xE4EC7600,0x00000030), &Aobject::CI, 0, NULL);
EDLL ACI						Aonoff::CI			= ACI("Aonoff",			GUID(0xE4EC7600,0x00000040), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL AinfoReso	Atable::infoReso[reso_MAXIMUM]=
{
	{ "80x60", "[80x60] very small", 80, 60},					
	{ "88x72", "[88x72]", 88, 72},								
	{ "128x96", "[128x96]", 128, 96},							
	{ "160x120", "[160x120] small", 160, 120 },					
	{ "176x144", "[176x144]", 176, 144},						
	{ "240x176", "[240x176]", 240, 176},						
	{ "240x180", "[240x180]", 240, 180},						
	{ "320x240", "[320x240] multimedia", 320, 240 },			
	{ "352x240", "[352x240] NTSC video CD", 352, 240},			
	{ "352x288", "[352x288] PAL video CD", 352, 288},			
	{ "384x288", "[384x288] DEMI PAL", 384, 288},				
	{ "400x300", "[400x300]", 400, 300},				
	{ "512x384", "[512x384] high", 512, 384 },					
	{ "640x240", "[640x240]", 640, 240},						
	{ "640x288", "[640x288]", 640, 288},						
	{ "640x480", "[640x480] VGA - NTSC", 640, 480},				
	{ "704x576", "[704x576]", 704, 576},						
	{ "720x240", "[720x240]", 720, 240},						
	{ "720x288", "[720x288]", 720, 288},						
	{ "720x480", "[720x480] NTSC DV", 720, 480},				
	{ "720x576", "[720x576] PAL DV", 720, 576},					
	{ "768x576", "[768x576] PAL", 768, 576},
	{ "800x600", "[800x600]", 800, 600},
	{ "1024x768", "[1024x768]", 1024, 768},
	{ "1280x720", "[1280x720] 720p", 1280, 720},
	{ "1920x1080", "[1920x1080] 1080p", 1920, 1080},
	{ "1200x400", "[1200x400] multi-display", 1200, 400},
};									

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QIID newqiid()
{
	QIID	qiid=0;
	while(!qiid)
		qiid=(QIID)rand()*(QIID)rand()*(QIID)rand()*(QIID)rand()*(QIID)rand();
	return qiid;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AtableSelect : public Aobject
{
	ELIBOBJ

								AtableSelect						(char *name, Aobject *l, int x, int y, int w, int h);
	virtual						~AtableSelect						();

	virtual void				paint								(Abitmap *b);
	virtual bool				notify								(Anode *o, int event, dword p=0);

	Abutton						*bFront;
	Abutton						*bBack;
	Abutton						*bPat;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL ACI						AtableSelect::CI	= ACI("AtableSelect",	GUID(0xE4EC7600,0x00000050), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AtableSelect::AtableSelect(char *name, Aobject *l, int x, int y, int w, int h) : Aobject(name, l, x, y, w, h)
{
	bFront=new Abutton("bFront", this, 2, 2, 100, h-4, "FRONT");
	bFront->setTooltips("select front panel");
	bFront->show(TRUE);
	bBack=new Abutton("bBack", this, 104, 2, 100, h-4, "BACK");
	bBack->setTooltips("select back panel");
	bBack->show(TRUE);
	bPat=new Abutton("bPat", this, 206, 2, 100, h-4, "SEQUENCER");
	bPat->setTooltips("select sequencer panel");
	bPat->show(TRUE);
}
	
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AtableSelect::~AtableSelect()
{
	delete(bFront);
	delete(bBack);
	delete(bPat);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AtableSelect::paint(Abitmap *b)
{
	b->boxfa(0, 0, pos.w, pos.h, 0xff000000, 0.8f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AtableSelect::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCLICK:
		if(o==bFront)
			father->notify(this, nyCHANGE, 0);
		else if(o==bBack)
			father->notify(this, nyCHANGE, 1);
		else if(o==bPat)
			father->notify(this, nyCHANGE, 2);
		break;
	}
	return Aobject::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AmidiCtrl
{
	public:
	
	int					nc;
	Acontrol			*control;
	AmidiCtrl			*next;

						AmidiCtrl			(int nc, Acontrol *control, AmidiCtrl *next);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AmidiCtrl::AmidiCtrl(int nc, Acontrol *control, AmidiCtrl *next)
{
	this->nc=nc;
	this->control=control;
	this->next=next;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AkeyCtrl
{
	public:

	int					nctrl;	
	int					kmode;
	int					nkey;
	Acontrol			*control;
	AkeyCtrl			*next;

						AkeyCtrl			(int nctrl, int kmode, int nkey, Acontrol *control, AkeyCtrl *next);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AkeyCtrl::AkeyCtrl(int nctrl, int kmode, int nkey, Acontrol *control, AkeyCtrl *next)
{
	this->nctrl=nctrl;
	this->kmode=kmode;
	this->nkey=nkey;
	this->control=control;
	this->next=next;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL ACI						Alayer::CI			= ACI("Alayer",			GUID(0xE4EC7600,0x00000005), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL Alayer::Alayer(char *name, Aobject *father, int x, int y, int w, int h) : Aobject(name, father, x, y, w, h)
{
	effectItems=new Aitem("add module", "add a new module", context, contextADDEFFECT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL Alayer::~Alayer()
{
	delete(effectItems);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL Atable * Atable::getTable(Aobject *o)
{
	while(o)
	{
		if(o->getCI()->guid==Atable::CI.guid)
			return (Atable *)o;
		o=(Aobject *)o->father;
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL Atable::Atable(char *name, Aobject *l, int x, int y, int w, int h) : Aobject(name, l, x, y, w, h)
{
	recalcPresetPos=false;
	GetModuleFileName(GetModuleHandle(null), rootdir, sizeof(rootdir));
	if(rootdir[0])
	{
		char	*s=strrchr(rootdir, '\\');
		if(s)
			*s=0;
	}
	memShowMap=false;
	tobeRepaint=false;
	running=FALSE;
	memset(midiIn, 0, sizeof(midiIn));
	midiOut=NULL;
	midiInDeviceF=0;
	midiOutDevice=-1;
	midiFilterType=0;
	midiFilterChannel=0;
	midiProgChangeChannel=-1;
	memset(midiMemoCtrl, 0, sizeof(midiMemoCtrl));
	clockType=clockINT;
	clockMidiTick=0;
	clockMidiBpm=0.f;
	clockMidiCount=0;
	clockMidiCountBpm=0;
	audioBPM=0.f;
	frameRate=50;
	render=false;
	vlooptime=8;
	highPriority=(getCPUcount()>1);
	threading=false;	// threaded rendering for future version
	nbEffectInfo=0;
	nbEffect=0;
	nbPreset=0;
	lastMouseY=0;
	memset(effectInfo, 0, sizeof(effectInfo));
	memset(midiCtrl, 0, sizeof(midiCtrl));
	memset(keyCtrl, 0, sizeof(keyCtrl));
	memset(preset, 0, sizeof(preset));
	memset(presetFile, 0, sizeof(presetFile));
	memset(presetPos, 0, sizeof(presetPos));
	dsoundsize=0.5f;
	
	bpmTracker=new AbpmTracker("bpmTracker");

	layer=new Alayer("layer", this, 0, 20, w, h-40);
	layer->show(true);

	front=new AtableFront("front table", layer, this, 0, 0, w, h-40);
	front->show(TRUE);
	back=new AtableBack("back table", layer, this, 0, 0, w, h-40);

	ctbar=new Actbar("ctbar", layer, this, w-26, 0, 15, h-40);
	ctbar->show(true);

	tselect=new AtableSelect("select", this, 0, 0, w, 20);
	tselect->show(TRUE);

	panelShowed=panelFRONT;

//	effectItems=new Aitem("add module", "add a new module");

	connects=new Aconnect("connect list", this, NULL, NULL);
	pins=NULL;

	mapping=NULL;

	bpmPhase=0;
	bpmFreq=120;
	recording=false;
	
	compressRamVidz=false;
	compressJpegQuality=1.f;

	nbControl=0;
	memset(control, 0, sizeof(control));

	sequencer=new Asequencer("sequencer", this, this, 0, h-20, w, 20);
	sequencer->setTooltips("sequencer");
	sequencer->show(true);

	plugz=new Aitem("plugz", "plugz");	// qqfois plugz->help a l'air d etre écrasé ????
	
#ifdef VSTUDIO
	midiPipeConnected=false;
	midiPipe=new Apipe("\\\\.\\pipe\\elekMidiPipe", Apipe::SERVER|Apipe::READ);
	
	remote=new AtcpRemote(this);
#endif

	oscBpm=new AoscNode("bpm", null, ",f", "bpm");
	addOscNode(oscBpm);
	oscPlay=new AoscNode("play", null, ",f", "play");
	addOscNode(oscPlay);
	oscPreset=new AoscNode("preset", null, ",i", "preset");
	addOscNode(oscPreset);
	oscSeqPattern=new AoscNode("pattern", null, ",i", "sequencer pattern");
	addOscNode(oscSeqPattern);

	timer(10);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL Atable::~Atable()
{
	midiClose();
#ifdef VSTUDIO
	delete(midiPipe);
	delete(remote);
#endif
	running=FALSE;
	synchronize.enter(__FILE__,__LINE__);
	{
		Aconnect	*c=(Aconnect *)connects->fchild;
		while(c)
		{
			Aconnect	*n=(Aconnect *)c->next;
			del(c);
			c=n;
		}
	}
	synchronize.leave();
	Athread::stop();
	{
		int	i;
		for(i=0; i<nbEffect; i++)
		{
			delete(effect[i]);
			delete(capsule[i]);
		}
	}
	delete(connects);
	delete(bpmTracker);
	delete(front);
	delete(back);
	delete(sequencer);
	{
		Aitem	*i=(Aitem *)plugz->fchild;
		while(i)
		{
			Aplugz	*p=(Aplugz *)i->link;
			if(p)
			{
				HINSTANCE	hDLL=p->windll;
				delete(p);
				if(hDLL)
					FreeLibrary(hDLL);
			}
			i=(Aitem *)i->next;
		}
	}
	delete(plugz);
	{
		int	i;
		for(i=0; i<MAXPRESET; i++)
			if(preset[i])
			{
				delete(preset[i]);
				preset[i]=null;
			}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Atable::idlePulse()
{
	if(running/*&&nseqpass*/)
	{
		sequencer->action(getBeat());
		/*nseqpass--;*/
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Atable::pulse()
{
	if(audioBPM!=0.f)
	{
		setBPM(audioBPM);
		audioBPM=0.f;
	}
	
	if(tobeRepaint)
	{
		repaint();
		tobeRepaint=false;
	}
	
#ifdef VSTUDIO
	if(midiPipe->isConnected())
	{
		int	avaible=midiPipe->avaible();
		midiPipeConnected=true;
		if(avaible)
		{
			dword		magic;
			if(midiPipe->read(&magic, sizeof(magic))==sizeof(magic))
			{
				switch(magic)
				{
					case magicMIDIPACKET:
					{
						MidiPacket	mp;
						if(midiPipe->read(&mp, sizeof(mp))==sizeof(mp))
						{
							int mclk=clockType;
							clockType=clockMIDI;
							midiMapping(mp.channel, mp.status, mp.p0, mp.p1, 0);
							clockType=mclk;
						}
						else goto packet_error;
					}
					break;
					
					case magicMIDISYSEX:
					{
						dword size;
						if(midiPipe->read(&size, sizeof(size))==sizeof(size))
						{
							Abuffer	*b=new Abuffer("midiSysex", size);
							dword	s=midiPipe->read(b->buffer, size);
							b->size=s;
							if(s==size)
							{
								midiSysex(b, 0);
							}
							else
							{
								delete(b);
								goto packet_error;
							}
						}
						else goto packet_error;
					}
					break;
					
					case magicELEKOFF:
					Aexit(0);
					break;
					
					case magicELEKSHOW:
					{
						bool	show;
						if(midiPipe->read(&show, sizeof(show))==sizeof(show))
							getWindow()->show(show);
						else goto packet_error;
					}
					break;
					
					default:
					assert(false); // lost in the stream
					break;
				}
			}
			else goto packet_error;
		}
	}
	else
	{
		if(midiPipeConnected)
		{
			midiPipeConnected=false;
			goto packet_error;
		}
	}
	return;
packet_error:
	delete(midiPipe);
	midiPipe=new Apipe("\\\\.\\pipe\\elekMidiPipe", Apipe::SERVER|Apipe::READ);
#endif	// VSTUDIO
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Atable::tcpSysex(char *mes)
{
	/*
	if(mes[5]==' ')
	{
		char	*s=&mes[6];
		Abuffer	*buf=new Abuffer("sysex", 8192);
		while(*s)
		{
			if(!strncmp(s, "GUID(0x", 7))
			{
				qword	q;
				dword	n0=0;
				dword	n1=0;
				sscanf(s+7, "%x", &n0);
				s=strchr(s, ',')+1;
				sscanf(s+2, "%x", &n1);
				s=strchr(s, ',')+1;
				q=GUID(n0, n1);
				buf->write(&q, sizeof(q));
				
			}
			else if(!strncmp(s, "0x", 2))
			{
				dword	n0=0;
				sscanf(s+2, "%x", &n0);
				s=strchr(s, ',')+1;
				buf->write(&n0, sizeof(n0));
			}
			else if(!strncmp(s, "TEXT:", 5))
			{
				buf->writeString(s+5);
				s=strchr(s, 0);
			}
			else
				s++;
		}
		midiSysex(buf, (int)getTime());
		delete(buf);
	}
	*/
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Atable::load(Afile *f)
{
	bool		oldversion=false;
	bool		missplugz=false;
	bool		misspresetpos=true;

	//byte		buffer[65536*2];
	//Afilemem	*fm=new Afilemem("filemem", buffer, 65536*2);		
	
	if(!magictest(f, magicTABLE))
		goto error;

	clear();
	
	{	// read/skip old midiInDevice
		int	dummy;
		if(!f->read(&dummy, sizeof(dummy)))
			goto error;
	}

	if(!f->read(&midiOutDevice, sizeof(midiOutDevice)))
		goto error;
	if(!f->read(&videoW, sizeof(videoW)))
		goto error;
	if(!f->read(&videoH, sizeof(videoH)))
		goto error;
	if(!f->read(&frameRate, sizeof(frameRate)))
		goto error;
	{
		bool dummy;
		if(!f->read(&dummy, sizeof(dummy)))	// old render mode info
			goto error;
		if(!f->read(&dummy, sizeof(dummy)))	// old high priority info
			goto error;
	}
	if(!f->read(&bpmPhase, sizeof(bpmPhase)))
		goto error;
	if(!f->read(&bpmFreq, sizeof(bpmFreq)))
		goto error;

	if(safemagictest(f, magicMIDIIN))
	{
		if(!f->read(&midiInDeviceF, sizeof(midiInDeviceF)))
			goto error;
	}

	if(safemagictest(f, magicDSOUNDSIZE))
	{
		if(!f->read(&dsoundsize, sizeof(dsoundsize)))
			goto error;
	}
		
	{
		sqword		oset=f->offset;
		if(magictest(f, magicPROGRAMCHANNEL))
			f->read(&midiProgChangeChannel, sizeof(midiProgChangeChannel));
		else
			f->seek(oset);
	}
		
	{
		bool	efcreated[MAXEFFECT];
		int		nb,i,j;
		
		memset(efcreated, 0, sizeof(efcreated));
		
		if(!magictest(f, magicEFFECTS))
			goto error;
			
		if(!f->read(&nb, sizeof(nb)))
			goto error;
			
		for(i=0; i<nb; i++)
		{
			char	ename[1024];
			qword	guid;
			QIID	qiid;
			dword	version;
			dword	size;
			sqword	skip;

			if(!magictest(f, magicEFFECT))
				goto error;
			if(!f->read(&guid, sizeof(guid)))
				goto error;
			if(!f->read(&version, sizeof(version)))
				goto error;
			if(!f->read(&qiid, sizeof(qiid)))
				goto error;
			if(safemagictest(f, magicEFFECTNAME))
				f->readString(ename);
			else
				ename[0]=0;
			if(!f->read(&size, sizeof(size)))
				goto error;
				
			skip=f->offset+size;
				
			effect[nbEffect]=NULL;
			capsule[nbEffect]=NULL;
			
			for(j=0; j<nbEffectInfo; j++)
			{
				AeffectInfo	*ei=effectInfo[j];
				if(ei->guid==guid)
				{
					AeffectInfo	*ei=effectInfo[j];
					if(!ename[0])
						genEffectName(ename, ei);
					Acapsule	*caps=capsule[nbEffect]=new Acapsule(ename, this);
					Aeffect		*e=effect[nbEffect]=ei->create(qiid, ename, caps);
					efcreated[i]=true;
					
					e->front->show(TRUE);
					e->back->show(TRUE);
					if(ei->version()==version)
					{
						if(!effect[nbEffect]->load(f))
							goto error;
					}
					else
					{
						f->seek(skip);
						oldversion=true;
					}
					ctbar->onoff[nbEffect]=new Aonoff(ename, ctbar, e, 0, 0, 16, 2);
					ctbar->onoff[nbEffect]->show(true);
					break;
				}
			}
			if(j==nbEffectInfo)
			{
				f->seek(skip);
				missplugz=true;
			}
			else
				nbEffect++;
		}

		if(!magictest(f, magicONOFF))
			goto error;
		{
			int		i;
			int		n=0;
			bool	onoff;
			for(i=0; i<nb; i++)
			{
				f->read(&onoff, sizeof(onoff));
				if(efcreated[i])
				{
					effect[n]->onoff=onoff;
					ctbar->onoff[n]->bon->setChecked(onoff);
					n++;
				}
			}
		}
	}

	if(!magictest(f, magicCONNECT))
		goto error;
	{
		int	nb,i;
		f->read(&nb, sizeof(nb));
		for(i=0; i<nb; i++)
		{
			qword	qiidin,qiidout;
			Apin	*pin=NULL;
			Apin	*pout=NULL;
			if(!f->read(&qiidin, sizeof(qiidin)))
				goto error;
			if(!f->read(&qiidout, sizeof(qiidout)))
				goto error;
			{
				Apin	*p=pins;
				while(p)
				{
					qword	qiid=p->qiid;
					if(qiid==qiidin)
					{
						pin=p;
						if(pout)
							break;
					}
					else if(qiid==qiidout)
					{
						pout=p;
						if(pin)
							break;
					}
					p=(Apin *)p->nextPin;
				}
			}
			if(pin&&pout)
				add(new Aconnect("connect", this, pin, pout));
			else
				oldversion=true;
		}
	}

	if(!magictest(f, magicCONTROLS))
		goto error;

	{
		int	nbCtrl;
		int	i;

		if(!f->read(&nbCtrl, sizeof(nbCtrl)))
			goto error;

		//if(nbCtrl!=nbControl)
		//	goto error;
		
		for(i=0; i<nbCtrl; i++)
		{
			QIID	q;
			int		j;
			dword	size;

			if(!magictest(f, magicCONTROL))
				goto error;
				
			if(!f->read(&q, sizeof(q)))
				goto error;

			if(!f->read(&size, sizeof(size)))
				goto error;
				

			for(j=0; j<nbControl; j++)
				if(control[j]->qiid==q)
					break;
				
			if(j==nbControl)
			{
				f->seek(f->offset+size);	// don't find control >> skip it
				oldversion=true;
			}
			else
				if(!control[j]->load(f))
					goto error;
		}
	}

	while(1)
	{
		sqword	offset=f->offset;
		if(magictest(f, magicPRESET))
		{
			int		i;
			dword	size;
			f->read(&i, sizeof(i));
			f->read(&size, sizeof(size));
			preset[i]=new Abuffer("preset", size);
			f->read(preset[i]->buffer, size);
		}
		else
		{
			f->seek(offset);
			break;
		}
	}

	memset(presetFile, 0, sizeof(presetFile));

	while(1)
	{
		sqword	offset=f->offset;
		if(magictest(f, magicPRESETFILENAME))
		{
			char	path[ALIB_PATH];
			char	abspath[ALIB_PATH];
			int		i;
			f->read(&i, sizeof(i));
			f->readString(path);
			if(f->absoluPath(abspath, path))
				strcpy(presetFile[i], abspath);
			else
				strcpy(presetFile[i], path);
		}
		else
		{
			f->seek(offset);
			break;
		}
	}

	memset(presetPos, 0, sizeof(presetPos));

	{
		sqword	offset=f->offset;
		if(magictest(f, magicPRESETPOS))
		{
			f->read(presetPos, sizeof(presetPos));
			misspresetpos=false;
		}
		else
		{
			f->seek(offset);
			misspresetpos=true;
		}
	}
	
	if(!magictest(f, magicCLOCKTYPE))
		goto error;
	if(!f->read(&clockType, sizeof(clockType)))
		goto error;
	switch(clockType)
	{
		case clockAUDIOPIN:
		{
			qword	q;
			Apin	*p=pins;
			f->read(&q, sizeof(q));
			while(p)
			{
				if(p&&(q==p->qiid))
					break;
				p=(Apin *)p->next;
			}
			if(p)
				clockType=(dword)p;
			else
				clockType=clockINT;
		}
		break;
	}

	if(safemagictest(f, magicLOOPTIME))
		f->read(&vlooptime, sizeof(vlooptime));
	else
		vlooptime=8;

	if(safemagictest(f, magicRAMCOMPRESS))
	{
		f->read(&compressRamVidz, sizeof(compressRamVidz));
		f->read(&compressJpegQuality, sizeof(compressJpegQuality));
	}
	else
	{
		compressRamVidz=false;
		compressJpegQuality=1.f;
	}

	if(!sequencer->load(f))
		goto error;
	
	arrange();
	order();
	
	midiClose();	// init midi for memValues() send midi state
	if(!midiInit())
		front->notify(this, nyERROR, (dword)"can't open one of the midi devices");
		
	memValues();
	repaint();
	settings(false);
	//delete(fm);
	if(missplugz)
		notify(this, nyERROR, (dword)"[warning] some plugins missing");
	else if(oldversion)
		notify(this, nyERROR, (dword)"[warning] old file version");

	if(misspresetpos)
	{
		int	i;
		notify(this, nyALERT, (dword)"[warning] old file version, presets fixed");
		recalcPresetPos=true;
		for(i=0; i<MAXPRESET; i++)
		{
			curPreset=i;
			loadPreset(i, true);
		}
		recalcPresetPos=false;
		misspresetpos=false;
	}

	return true;
error:
	clear();
	//delete(fm);
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Atable::save(Afile *f)
{
	dword		magic;
	bool		dummy=0;
	int			idummy=0;
	//byte		buffer[65536*2];
	int			i;
	//Afilemem	*fm=new Afilemem("filemem", buffer, 65536*2);		

	magic=magicTABLE;
	f->write(&magic, sizeof(magic));
	f->write(&idummy, sizeof(idummy));	// old midi in device
	f->write(&midiOutDevice, sizeof(midiOutDevice));
	f->write(&videoW, sizeof(videoW));
	f->write(&videoH, sizeof(videoH));
	f->write(&frameRate, sizeof(frameRate));
	f->write(&dummy, sizeof(dummy));	// old render
	f->write(&dummy, sizeof(dummy));	// old high priority
	f->write(&bpmPhase, sizeof(bpmPhase));
	f->write(&bpmFreq, sizeof(bpmFreq));

	magic=magicMIDIIN;
	f->write(&magic, sizeof(magic));
	f->write(&midiInDeviceF, sizeof(midiInDeviceF));

	magic=magicDSOUNDSIZE;
	f->write(&magic, sizeof(magic));
	f->write(&dsoundsize, sizeof(dsoundsize));
	
	magic=magicPROGRAMCHANNEL;
	f->write(&magic, sizeof(magic));
	f->write(&midiProgChangeChannel, sizeof(midiProgChangeChannel));

	magic=magicEFFECTS;
	f->write(&magic, sizeof(magic));
	f->write(&nbEffect, sizeof(nbEffect));
	{
		Abuffer	*buf=new Abuffer(f->name, 1024*1024);
		for(i=0; i<nbEffect; i++)
		{
			Aeffect	*e=effect[i];
			qword	guid=e->info->guid;
			qword	qiid=e->qiid;
			dword	version=0x00010500;
			dword	size;
			
			{
				int	j;
				for(j=0; j<this->nbEffectInfo; j++)
					if(effectInfo[j]->guid==guid)
						version=effectInfo[j]->version();
			}
			
			buf->clear();

			magic=magicEFFECT;
			f->write(&magic, sizeof(magic));
			f->write(&guid, sizeof(guid));
			f->write(&version, sizeof(version));
			f->write(&qiid, sizeof(qiid));
			magic=magicEFFECTNAME;
			f->write(&magic, sizeof(magic));
			f->writeString(e->name);
			
			effect[i]->save(buf);
			size=(int)buf->size;
			f->write(&size, sizeof(size));
			f->write(buf->buffer, size);
		}
		delete(buf);
	}
	{
		int	i;
		magic=magicONOFF;
		f->write(&magic, sizeof(magic));
		for(i=0; i<nbEffect; i++)
			f->write(&effect[i]->onoff, sizeof(effect[i]->onoff));
	}

	{
		int			nb=connects->count();
		Aconnect	*c=(Aconnect *)connects->fchild;
		magic=magicCONNECT;
		f->write(&magic, sizeof(magic));
		f->write(&nb, sizeof(nb));
		while(c)
		{
			qword	qiid=c->in->qiid;
			f->write(&qiid, sizeof(qiid));
			qiid=c->out->qiid;
			f->write(&qiid, sizeof(qiid));
			c=(Aconnect *)c->next;
		}
	}

	{
		magic=magicCONTROLS;
		f->write(&magic, sizeof(magic));
		if(!f->write(&nbControl, sizeof(nbControl)))
			goto error;

		{
			Abuffer		*buf=new Abuffer(f->name, 65536);
			dword		size;
			for(i=0; i<nbControl; i++)
			{
				buf->clear();
				magic=magicCONTROL;
				f->write(&magic, sizeof(magic));
				f->write(&control[i]->qiid, sizeof(control[i]->qiid));
				if(!control[i]->save(buf))
					goto error;
				size=(int)buf->size;
				f->write(&size, sizeof(size));
				f->write(buf->buffer, (int)buf->size);
			}
			delete(buf);
		}
	}
	
	{
		int	i;
		for(i=0; i<128; i++)
		{
			if(preset[i])
			{
				dword size=preset[i]->bufsize;
				magic=magicPRESET;
				f->write(&magic, sizeof(magic));
				f->write(&i, sizeof(i));
				f->write(&size, sizeof(size));
				f->write(preset[i]->buffer, size);
			}
		}
	}

	{
		int	i;
		for(i=0; i<MAXPRESETFILE; i++)
		{
			if(this->presetFile[i][0])
			{
				char	relpath[ALIB_PATH];
				magic=magicPRESETFILENAME;
				f->write(&magic, sizeof(magic));
				f->write(&i, sizeof(i));
				if(f->relativePath(relpath, presetFile[i]))
					f->writeString(relpath);
				else
					f->writeString(presetFile[i]);
			}
		}
	}

	{
		magic=magicPRESETPOS;
		f->write(&magic, sizeof(magic));
		f->write(presetPos, sizeof(presetPos));
	}

	{	
		magic=magicCLOCKTYPE;
		f->write(&magic, sizeof(magic));
		switch(clockType)
		{
			case clockINT:
			case clockMIDI:
			f->write(&clockType, sizeof(clockType));
			break;
			
			default:
			{
				dword	d=clockAUDIOPIN;
				qword	q=((Apin *)clockType)->qiid;
				f->write(&d, sizeof(d));
				f->write(&q, sizeof(q));
			}
			break;
		}
	}

	{	
		magic=magicLOOPTIME;
		f->write(&magic, sizeof(magic));
		f->write(&vlooptime, sizeof(vlooptime));
	}
	
	{	
		magic=magicRAMCOMPRESS;
		f->write(&magic, sizeof(magic));
		f->write(&compressRamVidz, sizeof(compressRamVidz));
		f->write(&compressJpegQuality, sizeof(compressJpegQuality));
	}
	

	sequencer->save(f);
	
	//delete(fm);
	return true;
error:
	//delete(fm);
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Atable::clear()
{
	sequencer->clear();
	seek(0.f);
	while(nbEffect)
		del(effect[0]);
	nbControl=0;	// somes control in the sequencer
	sequencer->clearInit();
	midiClose();
	midiInDeviceF=0;
	midiOutDevice=-1;
	videoW=320;
	videoH=240;
	running=FALSE;
	render=false;
	bpmPhase=0.f;
	bpmFreq=120.f;
	{
		int	i;
		for(i=0; i<MAXPRESET; i++)
			if(preset[i])
			{
				delete(preset[i]);
				preset[i]=null;
			}
	}
	memset(presetFile, 0, sizeof(presetFile));
	memset(presetPos, 0, sizeof(presetPos));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Atable::add(Aplugz *ei, bool view)
{
	if(ei&&ei->isCI(&Aplugz::CI))
	{
		new Aitem(ei->ename, ei->ehelp, plugz, ei);
		if(ei->isCI(&AeffectInfo::CI))
		{
			AeffectInfo	*inf=(AeffectInfo *)ei;
			int			cat=inf->getEffectClass();
			//Aitem		*i=(Aitem *)effectItems->fchild;
			{
				Aitem	*i=(Aitem *)layer->effectItems->fchild;
				while(i&&i->data!=cat)
					i=(Aitem *)i->next;
				if(!i)
					i=new Aitem(efClassName[cat], efClassName[cat], layer->effectItems, cat);
				if(view)
					new Aitem(ei->ename, ei->ehelp, i, contextADDEFFECT+nbEffectInfo, this);
			}
			
			effectInfo[nbEffectInfo++]=(AeffectInfo *)ei;
		}
		return TRUE;
	}
	return FALSE;
}

EDLL void Atable::sort()
{
	{
		Aitem	*i=(Aitem *)layer->effectItems->fchild;
		while(i)
		{
			i->sort();
			i=(Aitem *)i->next;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Atable::del(Aeffect *e)
{
	int			i,j;
	Acapsule	*caps=null;
	for(i=0; i<nbEffect; i++)
	{
		if(effect[i]==e)
			break;
	}
	assert(i<nbEffect);
	caps=capsule[i];
	{
		synchronize.enter(__FILE__,__LINE__);
		ctbar->onoff[i]->destroy();
		for(j=i; j<nbEffect-1; j++)
		{
			effect[j]=effect[j+1];
			capsule[j]=capsule[j+1];
			ctbar->onoff[j]=ctbar->onoff[j+1];
		}
		effect[nbEffect-1]=NULL;
		capsule[nbEffect-1]=NULL;
		ctbar->onoff[nbEffect-1]=NULL;
		{
			Apin	*p=pins;
			while(p)
			{
				Apin	*n=p->nextPin;
				if(p->effect==e)
					del(p);
				p=n;
			}
		}
		{
			Aconnect	*c=(Aconnect *)connects->fchild;
			while(c)
			{
				Aconnect	*n=(Aconnect *)c->next;
				if((c->in->effect==e)||(c->out->effect==e))
					del(c);
				c=n;
			}
		}
		nbEffect--;
		e->front->destroy();
		e->back->destroy();
		sequencer->del(e);
		mapping->del(e);
		delete(e);
		delete(caps);
		arrange();
		order();
		synchronize.leave();
		repaint();
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Atable::select(Acontrol *c)
{
	{
		int	i;
		for(i=0; i<nbEffect; i++)
			effect[i]->selected=(effect[i]==c->effect);
	}
	sequencer->select(c);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Atable::select(Aeffect *e)
{
	{
		int	i;
		for(i=0; i<nbEffect; i++)
			effect[i]->selected=(effect[i]==e);
	}
	sequencer->select(e);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Atable::randomize(Aeffect *e)
{
	int	i;
	for(i=0; i<nbControl; i++)
	{
		Acontrol	*c=control[i];
		if(c->effect==e)
			c->randomize();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL int Atable::addPresetFile(char *fn)
{
	int	i;
	for(i=0; i<MAXPRESETFILE; i++)
		if(!strcmp(presetFile[i], fn))
		{
			presetPos[i][curPreset]++;
			return i;
		}
	for(i=0; i<MAXPRESETFILE; i++)
		if(presetFile[i][0]==0)
		{
			strcpy(presetFile[i], fn);
			presetPos[i][curPreset]++;
			return i;
		}
	assert(false);
	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Atable::getPresetFile(int n, char *fn)
{
	assert(presetFile[n][0]!=0);
	strcpy(fn, presetFile[n]);
	if(recalcPresetPos)
		presetPos[n][curPreset]++;
	return (presetFile[n][0]!=0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Atable::clearPreset(int np)
{
	int	i;
	if(preset[np])
		delete(preset[np]);
	preset[np]=NULL;
	for(i=0; i<MAXPRESETFILE; i++)
	{
		if(presetPos[i][np])
		{
			int	j;
			presetPos[i][np]--;
			if(presetPos[i][np]==0)
			{
				for(j=0; j<128; j++)
					if(presetPos[i][j])
						break;
				if(j==128)
					presetFile[i][0]=0;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Atable::loadPreset(int np, bool all)
{
	if(preset[np])
	{
		bool	ctrlchanged[MAXCONTROL];
		memset(ctrlchanged, 0, sizeof(ctrlchanged));
		synchronize.enter(__FILE__,__LINE__);
		{
			int		efcreated[MAXEFFECT];
			int		nb=0;
			int		kk;
			Abuffer	*f=preset[np];
			f->offset=0;
			f->size=f->bufsize;
			for(kk=0; kk<MAXEFFECT; kk++)
				efcreated[kk]=-1;
			{
				dword	magic;
				int		i;
				{
					if(f->read(&magic, sizeof(magic))!=sizeof(magic))
						goto loadpreset_error;
					if(magic!=magicEFFECTS)
						goto loadpreset_error;
					if(f->read(&nb, sizeof(nb))!=sizeof(nb))
						goto loadpreset_error;
					for(i=0; i<nb; i++)
					{
						qword	guid;
						qword	qiid;
						dword	version;
						dword	size;
						sqword	offset;

						magic=magicEFFECT;
						if(f->read(&magic, sizeof(magic))!=sizeof(magic))
							goto loadpreset_error;
						if(f->read(&guid, sizeof(guid))!=sizeof(guid))
							goto loadpreset_error;
						if(f->read(&version, sizeof(version))!=sizeof(version))
							goto loadpreset_error;
						if(f->read(&qiid, sizeof(qiid))!=sizeof(qiid))
							goto loadpreset_error;
						if(f->read(&size, sizeof(size))!=sizeof(size))
							goto loadpreset_error;
							
						offset=f->offset;
						
						{
							int	j;
							for(j=0; j<nbEffect; j++)
							{
								if(effect[j]->qiid==qiid)
								{
									int	k;
									for(k=0; k<nbEffectInfo; k++)
									{
										AeffectInfo	*ei=effectInfo[k];
										if(ei->guid==guid)
										{
											if(ei->version()==version)
											{
												if(all||effect[j]->selected)
													efcreated[i]=j;
											}
										}
									}
									break;
								}
							}
						}
						
						if(efcreated[i]!=-1)
						{
							Aeffect	*e=effect[efcreated[i]];
							if(!e->loadPreset(f))
								f->seek(offset+size);	// repositionne (erreur de lecture)
						}
						else
							f->seek(offset+size);	// skip (effect not present)
					}
				}
			}

			if(!magictest(f, magicONOFF))
				goto loadpreset_error;
				
			{
				int		i;
				bool	onoff;
				for(i=0; i<nb; i++)
				{
					f->read(&onoff, sizeof(onoff));
					if(efcreated[i]!=-1)
					{
						int	n=efcreated[i];
						effect[n]->onoff=onoff;
						ctbar->onoff[n]->bon->setChecked(onoff);
					}
				}
			}
			
			if(!magictest(f, magicCONTROLS))
				goto loadpreset_error;

			{
				int	nbCtrl;
				int	i;

				if(!f->read(&nbCtrl, sizeof(nbCtrl)))
					goto loadpreset_error;

				//if(nbCtrl!=nbControl)
				//	goto error;
				
				for(i=0; i<nbCtrl; i++)
				{
					QIID	q;
					int		j;
					dword	size;

					if(!magictest(f, magicCONTROL))
						goto loadpreset_error;
						
					if(!f->read(&q, sizeof(q)))
						goto loadpreset_error;

					if(!f->read(&size, sizeof(size)))
						goto loadpreset_error;

					for(j=0; j<nbControl; j++)
						if((control[j]->qiid==q)&&(control[j]->effect&&(all||(control[j]->effect->selected))))
							break;
						
					if(j==nbControl)
						f->seek(f->offset+size);
					else
					{
						if(!control[j]->loadValues(f))
							goto loadpreset_error;
						else
							ctrlchanged[j]=true;
					}
				}
			}
		}
		
		{	// modified memValues();
			int	i;
			for(i=0; i<nbControl; i++)
			{
				if(control[i]&&ctrlchanged[i])
					control[i]->set2memValue();
			}
		}

		synchronize.leave();
		return true;
	loadpreset_error:
		synchronize.leave();
		return false;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Atable::savePreset(int np, bool all)
{
	synchronize.enter(__FILE__,__LINE__);
	{
		Abuffer	*f=new Abuffer("preset", 1024*1024);
		clearPreset(np);
		curPreset=np;
		if(f->isOK())
		{
			int		nEffect=0;
			dword	magic;
			int		i;
			
			if(all)
				nEffect=nbEffect;
			else
			{
				for(i=0; i<nbEffect; i++)
				{
					Aeffect	*e=effect[i];
					if(e->selected)
						nEffect++;
				}
			}
			
			{
				Abuffer	*buf=new Abuffer("effect", 1024*1024);
				
				magic=magicEFFECTS;
				f->write(&magic, sizeof(magic));
				f->write(&nEffect, sizeof(nEffect));
				
				for(i=0; i<nbEffect; i++)
				{
					Aeffect	*e=effect[i];
					if(all||e->selected)
					{
						qword	guid=e->info->guid;
						qword	qiid=e->qiid;
						dword	version=0x00010500;
						dword	size;
						
						buf->clear();
						
						e->savePreset(buf);
						size=(dword)buf->size;
						
						{
							int	j;
							for(j=0; j<this->nbEffectInfo; j++)
								if(effectInfo[j]->guid==guid)
									version=effectInfo[j]->version();
						}
						
						magic=magicEFFECT;
						f->write(&magic, sizeof(magic));
						f->write(&guid, sizeof(guid));
						f->write(&version, sizeof(version));
						f->write(&qiid, sizeof(qiid));
						f->write(&size, sizeof(size));
						f->write(buf->buffer, size);
					}
				}
				delete(buf);
			}
			{
				int	i;
				magic=magicONOFF;
				f->write(&magic, sizeof(magic));
				for(i=0; i<nbEffect; i++)
				{
					Aeffect	*e=effect[i];
					if(all||e->selected)
						f->write(&e->onoff, sizeof(e->onoff));
				}
			}
			{
				int	nControl=0;
				
				{
					for(i=0; i<nbControl; i++)
						if(control[i]->effect&&(all||control[i]->effect->selected))
							nControl++;
				}
				
				magic=magicCONTROLS;
				f->write(&magic, sizeof(magic));
				
				if(!f->write(&nControl, sizeof(nControl)))
					goto savepreset_error;

				{
					Abuffer		*buf=new Abuffer(f->name, 65536);
					dword		size;
					for(i=0; i<nbControl; i++)
					{
						if(control[i]->effect&&(all||control[i]->effect->selected))
						{
							buf->clear();
							magic=magicCONTROL;
							f->write(&magic, sizeof(magic));
							f->write(&control[i]->qiid, sizeof(control[i]->qiid));
							if(!control[i]->saveValues(buf))
							{
								delete(buf);
								goto savepreset_error;
							}
							size=(dword)buf->size;
							f->write(&size, sizeof(size));
							f->write(buf->buffer, (int)buf->size);
						}
					}
					delete(buf);
				}
			}
		}
		preset[np]=new Abuffer("preset", (int)f->size);
		preset[np]->write(f->buffer, (int)f->size);
		delete(f);
	}
	synchronize.leave();
	return true;
savepreset_error:
	synchronize.leave();
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Atable::genEffectName(char *ename, AeffectInfo *ei)
{	// generate the "name %d" (skip existing name)
	int	i;
	for(i=0; i<99; i++)
	{
		if(i==0)
			strcpy(ename, ei->ename);
		else
			sprintf(ename, "%s %d", ei->ename, i+1);
		{
			int		j;
			for(j=0; j<nbEffect; j++)
			{
				if(!strcmp(effect[j]->name, ename))
					break;
			}
			if(j==nbEffect)
				break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Atable::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyNOTIFY:
		//this->tcpMessage((char *)p);
		break;
		
		case nyCONTEXT:
		{
			int	d=((Aitem *)p)->data;
			if((d>=contextADDEFFECT)&&(d<=contextLASTEFFECT))
			{
				synchronize.enter(__FILE__,__LINE__);
				if(nbEffect<MAXEFFECT)
				{
					AeffectInfo	*ei=effectInfo[d-contextADDEFFECT];
					char		ename[1024];
					genEffectName(ename, ei);
					Acapsule	*caps=capsule[nbEffect]=new Acapsule(ename, this);
					Aeffect		*e=effect[nbEffect]=ei->create(newqiid(), ename, caps);
					if(e)
					{
						e->front->show(TRUE);
						e->back->show(TRUE);
						ctbar->onoff[nbEffect]=new Aonoff(ename, ctbar, e, 0, 0, 16, 2);
						ctbar->onoff[nbEffect]->show(true);
						if(nbEffect)
						{
							int	i;
							for(i=nbEffect-2; i>=0; i--)
								if(effect[i]->front->pos.y<lastMouseY)
									break;
							i++;
							if(i<nbEffect)
							{
								Acapsule	*caps=capsule[nbEffect];
								Aeffect		*e=effect[nbEffect];
								Aonoff		*oo=ctbar->onoff[nbEffect];
								int		j;
								for(j=nbEffect; j>i; j--)
								{
									effect[j]=effect[j-1];
									capsule[j]=capsule[j-1];
									ctbar->onoff[j]=ctbar->onoff[j-1];
								}
								effect[i]=e;
								capsule[i]=caps;
								ctbar->onoff[i]=oo;
							}
						}
						nbEffect++;
						arrange();
						order();
					}
					else
						notify(this, nyERROR, (dword)"can't instantiate module");
				}
				else
					notify(this, nyERROR, (dword)"can't add module, max modules reached");
				synchronize.leave();
				repaint();
				return TRUE;
			}
		}
		break;

		case nyCHANGE:
		switch(p)
		{
			case 0:
			panelShowed=panelFRONT;
			back->show(FALSE);
			front->show(TRUE);
			repaint();
			break;
			
			case 1:
			panelShowed=panelBACK;
			front->show(FALSE);
			back->show(TRUE);
			repaint();
			break;
			
			case 2:
			sequencer->notify(this, nySWITCH);
			break;
		}
		return TRUE;

		case nySELECT:
		if(o->isCI(&AcontrolObjMap::CI))
		{
			AcontrolObjMap *map=(AcontrolObjMap *)o;
			int	i;
			for(i=0; i<nbControl; i++)
			{
				Acontrol	*c=control[i];
				c->selectMap(map);
			}
			father->notify(map->ctrlobj->control, event, p);
		}
		break;
	}
	return Aobject::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Atable::paint(Abitmap *b)
{
	b->boxfa(0, 0, pos.w-1, pos.h-1, 0xff202020, 0.7f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Atable::size(int w, int h)
{
	bool	b=Aobject::size(w, h);
	{
		int	dh=(h-20)-sequencer->pos.h;
		if(dh<0)
		{
			sequencer->size(w, h-20);
			sequencer->move(0, 20);
			layer->size(w, 0);
		}
		else
		{
			sequencer->move(0, h-sequencer->pos.h);
			layer->size(w, dh);
		}
	}
	return b;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Atable::deplace(int my, int ly)
{
	int		meffect[MAXEFFECT];
	int		nmeffect=0;
	Aeffect	*e2=NULL;
	Aeffect	*e3=NULL;
	int		n2=0;
	int		i;
	int		s=my-ly;
	bool	ret=false;
	
	for(i=0; i<nbEffect; i++)
	{
		Aeffect	*ce=effect[i];
		if(my<=(ce->front->pos.y+ce->front->pos.h))
		{
			e2=ce;
			n2=i;
			break;
		}
	}

	for(i=0; i<nbEffect; i++)
	{
		Aeffect	*ce=effect[i];
		if(ly<=(ce->front->pos.y+ce->front->pos.h))
		{
			e3=ce;
			break;
		}
	}
	
	if(e3==e2)
		return false;
	
	if(s>0)
		n2++;

	if(e2)
	{
		for(i=0; i<n2; i++)
		{
			Aeffect	*e=effect[i];
			if(!e->selected)
				meffect[nmeffect++]=i;
		}

		for(i=0; i<nbEffect; i++)
		{
			Aeffect	*e=effect[i];
			if(e->selected)
				meffect[nmeffect++]=i;
		}
		
		for(i=n2; i<nbEffect; i++)
		{
			Aeffect	*e=effect[i];
			if(!e->selected)
				meffect[nmeffect++]=i;
		}

		{
			Aeffect		*ef[MAXEFFECT];
			Aonoff		*oo[MAXEFFECT];
			Acapsule	*cf[MAXEFFECT];
			
			for(i=0; i<nbEffect; i++)
			{
				int	n=meffect[i];
				if(n!=i)
					ret=true;
				ef[i]=effect[n];
				cf[i]=capsule[n];
				oo[i]=ctbar->onoff[n];
			}
			
			synchronize.enter(__FILE__,__LINE__);
			memcpy(effect, ef, sizeof(Aeffect *)*nbEffect);
			memcpy(capsule, cf, sizeof(Acapsule *)*nbEffect);
			memcpy(ctbar->onoff, oo, sizeof(Aonoff *)*nbEffect);
			synchronize.leave();
		}

		arrange();
		repaint();
	}
	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Atable::arrange()
{
	int	i;
	int	he=5;
	for(i=0; i<nbEffect; i++)
	{
		effect[i]->front->pos.y=he;
		effect[i]->back->pos.y=he;
		ctbar->onoff[i]->pos.y=he;
		ctbar->onoff[i]->pos.h=effect[i]->front->pos.h;
		he+=effect[i]->front->pos.h+10;
	}
	if(he<(pos.h-20))
	{
		front->pos.y=0;
		back->pos.y=0;
		ctbar->pos.y=0;
	}
	front->size(pos.w, he);
	back->size(pos.w, he);
	ctbar->size(ctbar->pos.w, he);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Atable::add(Apin *p)
{
	p->nextPin=pins;
	pins=p;
	father->notify(this, nyCHANGE, 0);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Atable::del(Apin *p)
{
	Apin	*ps=pins;
	Apin	*po=NULL;
	while(ps)
	{
		if(ps==p)
		{
			if(po)
				po->nextPin=p->nextPin;
			else
				pins=p->nextPin;
			father->notify(this, nyCHANGE, 0);
			return TRUE;
		}
		po=ps;
		ps=ps->nextPin;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Atable::add(Aconnect *c)
{
	connects->add(c);
	if(c->in)
		c->in->connect=c;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Atable::del(Aconnect *c)
{
	if(c->in)
		c->in->connect=NULL;
	delete(c);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Atable::testDown(Aeffect *e0, Aeffect *e1, bool *be, int level)
{
	Apin	*p=pins;
	bool	beffect[MAXEFFECT];

	memcpy(beffect, be, sizeof(beffect));

	while(p)
	{
		if((p->effect==e0)&&((p->type&pinDIR)==pinIN))
		{
			if(p->connect)
			{
				Apin	*out=p->connect->out;

				if(out->effect==e1)
					return level;

				if(beffect[out->effect->instantOrder])
					return 0;

				beffect[out->effect->instantOrder]=true;

				{
					int	n=testDown(out->effect, e1, beffect, level+1);
					if(n)
						return n;
				}
			}
		}
		p=p->nextPin;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Atable::testUp(Aeffect *e0, Aeffect *e1, bool *be, int level)
{
	Apin	*p=pins;
	bool	beffect[MAXEFFECT];

	memcpy(beffect, be, sizeof(beffect));

	while(p)
	{
		if((p->effect==e0)&&((p->type&pinDIR)==pinOUT))
		{
			Aconnect	*c=(Aconnect *)connects->fchild;
			while(c)
			{
				if(c->out==p)
				{
					Apin *in=c->in;

					if(in->effect==e1)
						return level;
					
					if(beffect[in->effect->instantOrder])
						return 0;

					beffect[in->effect->instantOrder]=true;

					{
						int	n=testUp(in->effect, e1, beffect, level+1);
						if(n)
							return n;
					}
				}
				c=(Aconnect *)c->next;
			}
		}
		p=p->nextPin;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Atable::order()
{
	synchronize.enter(__FILE__,__LINE__);
	memcpy(effectOrder, effect, sizeof(effect[0])*nbEffect);
	{
		int	i,j;
		for(i=0; i<nbEffect; i++)
		{
			Aeffect	*e=effectOrder[i];
			e->instantPriority=e->priority()*nbEffect*nbEffect;
			e->instantOrder=i;
			nEffectOrder[i]=i;
		}
		for(i=0; i<nbEffect; i++)
			for(j=0; j<nbEffect; j++)
			{
				if(j!=i)
				{
					bool	beffect[MAXEFFECT];
					int		d;
					memset(beffect, 0, sizeof(beffect));
					d=testDown(effectOrder[i], effectOrder[j], beffect, 0);
					memset(beffect, 0, sizeof(beffect));
					d-=testUp(effectOrder[i], effectOrder[j], beffect, 0);
					effectOrder[i]->instantPriority+=d;
				}
			}
		for(i=nbEffect-1; i>=0; i--)
			for(j=0; j<i; j++)
			{
				Aeffect	*e0=effectOrder[j];
				Aeffect	*e1=effectOrder[j+1];
				assert(e0!=e1);
				if(e1->instantPriority>e0->instantPriority)
				{
					effectOrder[j]=e1;
					effectOrder[j+1]=e0;
					{
						int	n=nEffectOrder[j];
						nEffectOrder[j]=nEffectOrder[j+1];
						nEffectOrder[j+1]=n;
					}
				}
			}
		for(i=0; i<nbEffect; i++)
			capsule[i]->calcDependency();
	}
	synchronize.leave();
	father->notify(this, nyCHANGE, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Atable::settings(bool emergency)
{
	int	i;
	for(i=0; i<nbEffect; i++)
		effect[i]->settings(emergency);
	{
		Apin	*p=pins;
		while(p)
		{
			p->settings(emergency);
			p=p->nextPin;
		}
	}
	midiClose();
	if(!midiInit())
		front->notify(this, nyERROR, (dword)"can't open one of the midi devices");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Atable::run()
{
	timeBeginPeriod(1);
	outrun=false;
	double	gtime=Athread::getTime();
	double	lasttime;
	if(render)
		timeCurrent=timeStart=lasttime=0;
	else
	{
		timeStart=(int)Athread::getTime();
		lasttime=getTime();
		if(highPriority)
			setPriority(Athread::prioritySPEED);
		else
			setPriority(Athread::priorityNORMAL);
	}
	instantWait=10;
	instantWaitReal=10;
	instantFPS=(float)frameRate;
	instantFrameTime=1000.f/instantFPS;
	instantFrameRenderTime=0;
	running=TRUE;

	CoInitialize(NULL);

//	seek(0.f);	// todo:

	synchronize.enter(__FILE__,__LINE__);
	{
		double time=getTime();
		int	i;
		for(i=0; i<nbEffect; i++)
		{
			if(effectOrder[i]->onoff)
				effectOrder[i]->actionStart(time);
		}
	}
	synchronize.leave();
	
	//nseqpass=0;
	beatTime=getTime();
	sequencer->start();

	while(running)
	{
		float	pow[MAXEFFECT];
		int		i;
		double	dtime;
		double	mtime=1000/frameRate;
		synchronize.enter(__FILE__,__LINE__);
		if(running)
		{
			double	time=getTime();
			double	dbeat;
			dtime=time-lasttime;
			lasttime=time;
			dbeat=(dtime*(double)bpmFreq/60000.0);
			double	et=Athread::getCpuTime();
			double	localbeat=(beat+=dbeat);
			beatTime=time;
			//nseqpass++;
			for(i=0; i<nbEffect; i++)
			{
				if(effectOrder[i]->onoff!=effectOrder[i]->monoff)
				{
					if(effectOrder[i]->onoff)
						effectOrder[i]->actionStart(time);
					else
						effectOrder[i]->actionStop();
					effectOrder[i]->monoff=effectOrder[i]->onoff;
				}
				if(threading)
				{
					Acapsule	*caps=effectOrder[i]->capsule;
					caps->set(time, dtime, localbeat, dbeat);
				}
				else
				{
					if(effectOrder[i]->onoff)
					{
						#ifndef _DEBUG
						try
						{
							effectOrder[i]->action(time, dtime, localbeat, dbeat);
						}
						catch(...)
						{
							effectOrder[i]->onoff=false;
						}
						#else
						effectOrder[i]->action(time, dtime, localbeat, dbeat);
						#endif
					}
					{
						double	v=Athread::getCpuTime();
						pow[nEffectOrder[i]]=(float)(v-et);
						et=v;
					}
				}
			}
			if(threading)
			{
				int	i;
				for(i=0; i<nbEffect; i++)
					capsule[i]->start();
				{
					bool running=true;
					while(running)
					{
						running=false;
						for(i=0; i<nbEffect; i++)
							if(capsule[i]->running)
								running=true;
						sleep(1);
					}
				}
				for(i=0; i<nbEffect; i++)
					pow[i]=(float)capsule[i]->cpu;
			}
		}
		{
			double	gt=Athread::getTime();
			double	s=gt-gtime;
			gtime=gt;
			if(s>0.f)
			{
				s=1.0/s;
				for(i=0; i<nbEffect; i++)
					ctbar->onoff[i]->power=(float)(ctbar->onoff[i]->power*0.9+pow[i]*s*0.1);
			}
		}
		computeAudioBPM();
		synchronize.leave();
		if(render)
		{
			instantFrameTime=mtime;
			instantFPS=(double)frameRate;
			timeCurrent+=mtime;
			sleep(100);
		}
		else
		{
			double dt=getTime()-lasttime;
			instantFrameRenderTime=dt;
			instantFrameTime=dtime;
			instantFPS=1000.0/instantFrameTime;
			int iw=(int)maxi((1000.0/(double)frameRate)-dt, 0.0);
			/*
			if(instantFPS>((double)frameRate+0.1))
				instantWait++;
			else if(instantFPS<((double)frameRate-0.1))
				instantWait--;
			*/
#ifdef _DEBUG
			instantWait=maxi(instantWait, 50);
#else
			instantWait=maxi(iw, 1);
#endif
			{
				double	d=getTime();
				sleep(instantWait);
				instantWaitReal=getTime()-d;
			}
		}
	}
	sequencer->stop();
	CoUninitialize();
	timeEndPeriod(1);
	outrun=true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Atable::computeAudioBPM()
{
	dword	clk=clockType;
	switch(clk)
	{
		case clockINT:
		case clockMIDI:
		break;
		
		default:
		{
			float	cBpm=bpmTracker->getBPM((Asample *)clk);
			if((cBpm!=0.f)&&(cBpm!=bpmFreq))
			{
				if(cBpm>500.f)
					;
				else if(cBpm>200.f)
					audioBPM=cBpm/2.f;
				else if(cBpm<40.f)
					;
				else if(cBpm<70.f)
					audioBPM=cBpm*2.f;
				else
					audioBPM=cBpm;
			}
		}
		break;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Atable::seek(double beat)
{
	if(beat==SEEKLAST)
	{
		// todo
		return true;
	}
	else
	{
		this->beat=beat;
		return sequencer->seek(beat);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Atable::start()
{
	bool b=Athread::start();
	if(b)
		setAffinity(1);	// only cpu0 allowed to execute render thread
	return b;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Atable::stop()
{
	running=false;
	while(!outrun)
		sleep(10);

	Athread::stop();
	
	tobeRepaint=true;
	
	synchronize.enter(__FILE__,__LINE__);
	{
		int	i;
		for(i=0; i<nbEffect; i++)
		{
			if(effectOrder[i]->onoff!=effectOrder[i]->monoff)
			{
				if(!effectOrder[i]->onoff)
					effectOrder[i]->actionStop();
				effectOrder[i]->monoff=effectOrder[i]->onoff;
			}
			else if(effectOrder[i]->onoff)
				effectOrder[i]->actionStop();
			ctbar->onoff[i]->power=0.f;
		}
	}
	synchronize.leave();

//	memValues();

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL double Atable::getBeat()
{
	if(running)
	{
		double dtime=getTime()-beatTime;
		return beat+(dtime*(double)bpmFreq/60000.0);
	}
	return beat;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL double Atable::getTime()
{
	if(render)
		return (timeCurrent-timeStart);
	else
		return (Athread::getTime()-timeStart);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Atable::showMap(bool value)
{
	int	i;
	memShowMap=value;
	for(i=0; i<nbControl; i++)
	{
		Acontrol *c=control[i];
		if(c->object)
		{
			c->object->showMap(value);
		}
	}
	repaint();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Atable::add(Acontrol *c)
{
	{
		int	i;
		for(i=0; i<nbControl; i++)
			if(control[i]->qiid==c->qiid)
			{
				char	mes[256];
				sprintf(mes, "[integrity error] same control QID used in module %s", c->effect->name);
				notify(this, nyERROR, (dword)mes);
				break;
			}
	}
	control[nbControl++]=c;
	c->object->showMap(memShowMap);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Atable::del(Acontrol *c)
{
	int	i;
	for(i=0; i<nbControl; i++)
		if(control[i]==c)
		{
			int	j;
			nbControl--;
			for(j=i; j<nbControl; j++)
				control[j]=control[j+1];
			//control[i]=NULL;
			break;
		}
	sequencer->del(c);
	if(mapping)
		mapping->del(c);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Atable::memValues()
{
	int	i;
	for(i=0; i<nbControl; i++)
	{
		if(control[i])
			control[i]->set2memValue();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Atable::addMidiCtrl(int nctrl, Acontrol *ctrl)
{
	smidiCtrl.enter(__FILE__,__LINE__);
	{
		Amidimap	*mm=&ctrl->midimap[nctrl];
		int			nc;
		
		switch(mm->status)
		{
			case midiNoteOn:
			nc=1;
			break;

			case midiControl:
			nc=2;
			break;

			default:
			nc=0;
			break;
		}
		if(nc)
		{
			if((ctrl->type>=Acontrol::KEY_01)&&(nc==1))
			{
				int	nb=1+ctrl->type-Acontrol::KEY_01;
				int	no=mm->data;
				int	i;
				for(i=0; i<nb; i++)
				{
					if(midiCtrl[nc-1][mm->channel-1][no])
						notify(this, nyALERT, (dword)"addMidiCtrl ERROR");
					midiCtrl[nc-1][mm->channel-1][no]=new AmidiCtrl(nctrl, ctrl, midiCtrl[nc-1][mm->channel-1][no]);
					no++;
				}
			}
			else
			{
				int	nbplus=(ctrl->midimap[0].modeCtrl>=Amidimap::modeABS_14)?32:0;
				int	data=(mm->data+nbplus)&127;
				midiCtrl[nc-1][mm->channel-1][data]=new AmidiCtrl(nctrl, ctrl, midiCtrl[nc-1][mm->channel-1][data]);
			}
		}
	}		
	smidiCtrl.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Atable::delMidiCtrl(int nctrl, Acontrol *ctrl)
{
	smidiCtrl.enter(__FILE__,__LINE__);
	{
		Amidimap	*mm=&ctrl->midimap[nctrl];
		int			nc;
		switch(mm->status)
		{
			case midiNoteOn:
			nc=1;
			break;

			case midiControl:
			nc=2;
			break;

			default:
			nc=0;
			break;
		}
		if(nc)
		{
			if((ctrl->type>=Acontrol::KEY_01)&&(nc==1))
			{
				int	nb=1+ctrl->type-Acontrol::KEY_01;
				int	no=mm->data;
				int	i;
				for(i=0; i<nb; i++)
				{
					AmidiCtrl	*mc=midiCtrl[nc-1][mm->channel-1][no];
					AmidiCtrl	*mo=NULL;
					while(mc)
					{
						if((mc->control==ctrl)&&(mc->nc==nctrl))
							break;
						mo=mc;
						mc=mc->next;
					}
					assert(mc);
					if(mo)
						mo->next=mc->next;
					else
						midiCtrl[nc-1][mm->channel-1][no]=mc->next;
					delete(mc);
					no++;
				}
				/*
					case 2:
					{
				AmidiCtrl	*mc=midiCtrl[nc-1][mm->channel-1][mm->data];
				AmidiCtrl	*mo=NULL;
				while(mc)
				{
					if((mc->control==ctrl)&&(mc->nc==nctrl))
						break;
					mo=mc;
					mc=mc->next;
				}
				assert(mc);
				if(mo)
					mo->next=mc->next;
				else
					midiCtrl[nc-1][mm->channel-1][mm->data]=mc->next;
				delete(mc);
					}
					break;
				*/
			}
			else
			{
				int			nbplus=(ctrl->midimap[0].modeCtrl>=Amidimap::modeABS_14)?32:0;
				int			data=(mm->data+nbplus)&127;
				AmidiCtrl	*mc=midiCtrl[nc-1][mm->channel-1][data];
				AmidiCtrl	*mo=NULL;
				while(mc)
				{
					if((mc->control==ctrl)&&(mc->nc==nctrl))
						break;
					mo=mc;
					mc=mc->next;
				}
				assert(mc);
				if(mo)
					mo->next=mc->next;
				else
					midiCtrl[nc-1][mm->channel-1][data]=mc->next;
				delete(mc);
			}
		}
	}		
	smidiCtrl.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Atable::addKeyCtrl(int nctrl, Acontrol *ctrl)
{
	ctrl->smidimap.enter(__FILE__,__LINE__);
	{
		int	kmode=ctrl->keymap[nctrl].kmode;
		if(kmode)
		{
			int	max=1;
			switch(kmode)
			{
				case keymodeSELECT:
				assert(ctrl->type>=Acontrol::KEY_01);
				max=1+ctrl->type-Acontrol::KEY_01;
				break;

				case keymodeCYCLE:
				case keymodePRESS:
				case keymodeSWITCH:
				break;

				default:
				assert(false);
				break;
			}
			{
				int	i;
				for(i=0; i<max; i++)
				{
					int	scan=ctrl->keymap[nctrl].key[i];
					if(scan)
					{
						smidiCtrl.enter(__FILE__,__LINE__);
						keyCtrl[scan]=new AkeyCtrl(nctrl, kmode, i, ctrl, keyCtrl[scan]);
						smidiCtrl.leave();
					}
				}
			}
		}
	}
	ctrl->smidimap.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Atable::delKeyCtrl(int nctrl, Acontrol *ctrl)
{
	ctrl->smidimap.enter(__FILE__,__LINE__);
	{
		int	kmode=ctrl->keymap[nctrl].kmode;
		if(kmode)
		{
			int	max=1;
			switch(kmode)
			{
				case keymodeSELECT:
				assert(ctrl->type>=Acontrol::KEY_01);
				max=1+ctrl->type-Acontrol::KEY_01;
				break;

				case keymodeCYCLE:
				case keymodePRESS:
				case keymodeSWITCH:
				break;

				default:
				assert(false);
				break;
			}
			{
				int	i;
				for(i=0; i<max; i++)
				{
					int	scan=ctrl->keymap[nctrl].key[i];
					if(scan)
					{
						ctrl->keymap[nctrl].key[i]=0;
						smidiCtrl.enter(__FILE__,__LINE__);
						{
							AkeyCtrl	*kc=keyCtrl[scan];
							AkeyCtrl	*ko=NULL;
							while(kc)
							{
								if((kc->control==ctrl)&&(kc->nkey==i))
									break;
								ko=kc;
								kc=kc->next;
							}
							assert(kc);
							if(ko)
								ko->next=kc->next;
							else
								keyCtrl[scan]=kc->next;
							delete(kc);
						}
						smidiCtrl.leave();
					}
				}
			}
		}
	}
	ctrl->smidimap.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Atable::command(int cmd)
{
	father->notify(this, nyCOMMAND, cmd);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int CALLBACK midiproc(HMIDIIN hmidin, UINT msg, DWORD instance, DWORD param1, DWORD param2)
{
	Atable	*table=(Atable *)instance;
	switch(msg)
	{
		case MIM_DATA:
		//safeGUI.enter(__FILE__,__LINE__);
		switch(param1&0xff)
		{
			case 0xf2:
			case 0xf3:
			case 0xf8:
			case 0xfa:
			case 0xfb:
			case 0xfc:
			table->midiMapping(0, param1&0xff, 0, 0, param2);
			break;

			default:
			table->midiMapping((param1&0x0f)+1, param1&0xf0, (param1>>8)&0x7f, (param1>>16)&0x7f, param2);
			break;
		}
		//safeGUI.leave();
		break;

		case MIM_LONGDATA:
		if(0)
		{
			MIDIHDR	*mhdr=(MIDIHDR *)param1;
			Abuffer	*b=new Abuffer("sysex", mhdr->dwBytesRecorded);
			// todo...
			//table->midiSysex(b, param2);
			delete(b);
		}
		break;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Atable::midiInit()
{
	MMRESULT	result;
	int			test=0;

	if(midiInDeviceF)
	{
		int	i;
		for(i=0; i<MAXMIDIIN; i++)
		{
			if(this->midiInDeviceF&(1i64<<(qword)i))
			{
				HMIDIIN	h;
				result=midiInOpen(&h, i, (unsigned int)midiproc, (DWORD)this, CALLBACK_FUNCTION);
				if(result==0)
				{
					result=midiInStart(h);
					if(!result)
					{	
						midiIn[i]=h;
						test|=1;
					}
					else
					{
						midiInStop(h);
						midiIn[i]=0;
					}
				}
			}
		}
	}
	else
		test|=1;
	if(midiOutDevice!=-1)
	{
		HMIDIOUT	h;
		result=midiOutOpen(&h, midiOutDevice, NULL, (DWORD)this, CALLBACK_NULL);
		if(result==MMSYSERR_NOERROR)
		{
			moh.lpData=(char *)mohdata;
			moh.dwBufferLength=sizeof(mohdata);
			moh.dwFlags=0;
			result=midiOutPrepareHeader(h, &moh, sizeof(moh));
			if(result!=MMSYSERR_NOERROR)
				midiOutClose(h);
			else
			{
				midiOut=h;
				test|=2;
			}
		}
	}
	else
		test|=2;
	return test==3;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Atable::midiClose()
{
	int	i;
	for(i=0; i<MAXMIDIIN; i++)
	{
		if(midiIn[i])
		{
			midiInStop(midiIn[i]);
			midiInClose(midiIn[i]);
			midiIn[i]=NULL;
		}
	}
	if(midiOut)
	{
		midiOutUnprepareHeader(midiOut, &moh, sizeof(moh));
		midiOutClose(midiOut);
		midiOut=NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Atable::midiSend(int channel, int status, int p0, int p1)
{
	if(midiOut)
		midiOutShortMsg(midiOut, ((channel-1)&0x0f) | (status&0x0f0) | ((p0&0x7f)<<8) | ((p1&0x7f)<<16));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Atable::midiSysex(Abuffer *b, dword time)
{
	qword	guid;
	int		i;
	b->seek(0);
	b->read(&guid, sizeof(guid));
	for(i=0; i<nbEffect; i++)
	{
		if(effect[i]->info->guid==guid)
		{
			b->seek(8);
			effect[i]->midiSysex(b, time);
		}
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Atable::midiMapping(int channel, int status, int p0, int p1, dword time)
{
	switch(status)	// remember values
	{
		case midiControl:	// control
		midiMemoCtrl[0][p0]=p1;
		midiMemoCtrl[channel][p0]=p1;
		break;
	}

	if(mapping&&mapping->isUsed())
		mapping->midiMapping(channel, status, p0, p1, time);

#ifdef _DEBUG
	{	// 4debug
		char	str[1024];
		char	*sta[]={ "NF", "NO", "AT", "CC", "PC", "CH", "PB", "SY" };
		sprintf(str, "midi event [chan: %2d - status: %s - d0: %3d - d1: %3d]\r\n", channel, sta[((status>>4)-8)&7], p0, p1);
		OutputDebugString(str);
	}
#endif

	smidiCtrl.enter(__FILE__,__LINE__);
	{
		int			nc;
		switch(status)
		{
			case midiNoteOn:
			nc=1;
			{
				AmidiCtrl *mc=midiCtrl[nc-1][channel-1][p0];
				while(mc)
				{
					mc->control->midiseq(mc->nc, status, p0, p1);
					mc=mc->next;
				}
			}
			break;

			case midiNoteOff:
			nc=1;
			{
				AmidiCtrl *mc=midiCtrl[nc-1][channel-1][p0];
				while(mc)
				{
					mc->control->midiseq(mc->nc, midiNoteOn, p0, 0);
					mc=mc->next;
				}
			}
			break;

			case midiControl:
			nc=2;
			{
				AmidiCtrl *mc=midiCtrl[nc-1][channel-1][p0];
				while(mc)
				{
					mc->control->midiseq(mc->nc, status, p0, p1);
					mc=mc->next;
				}
			}
			break;
			
			case midiProgram:
			if(((midiProgChangeChannel+1)==channel)||(midiProgChangeChannel==16))
				this->loadPreset(p0&127, true);
			break;

			case 0xfa:	// midi start
			if(clockType==clockMIDI)
			{
				clockMidiBpm=0.f;
				clockMidiCountBpm=0;
				command(Atable::START);
			}
			break;

			case 0xfb:	// pause/restart at position
			if(clockType==clockMIDI)
			{
				if(isRunning())
					command(Atable::STOP);
				else
					command(Atable::START);
			}
			break;

			case 0xfc:	// midi stop
			if(clockType==clockMIDI)
			{
				clockMidiCountBpm=0;
				clockMidiCount=0;
				clockMidiBpm=0.f;
				command(Atable::STOP);
				seek(0.f);
			}
			break;
			
			case 0xf2:	// SPP: song position pointeur
			if(clockType==clockMIDI)
			{
				int	spp=((p1&127)<<7)|(p0&127);
				clockMidiBpm=(float)spp/4.f;
				seek(clockMidiBpm);
				clockMidiCount=0;
				clockMidiCountBpm=0;
			}
			break;
			
			case 0xf8:	// midi tick (24 ticks pour une noire) (une noire == un beat) (une blanche == une mesure == 4 beats)
			if(clockType==clockMIDI)
			{
				int	tick=time;
				if(isRunning())
				{
					if(clockMidiCountBpm>=24)
					{
						clockMidiCountBpm=0;
						clockMidiBpm+=1.f;
						seek(clockMidiBpm);
					}
					clockMidiCountBpm++;
				}
				if(clockMidiCount>=24)
				{
					int	dt=tick-clockMidiTick;
					if(dt)
					{
						float cBpm=1000.f*60.f/(float)dt;
						setBPM(cBpm);
					}
					clockMidiCount=0;
				}
				if(!clockMidiCount)
					clockMidiTick=tick;
				clockMidiCount++;
			}
			break;
			
			case 0xf3:	// song select 
			// todo: pattern select on the sequencer 
			// todo add a midi tab to activate/desactivate use of midi events
			break;

			default:
			nc=0;
			break;
		}
	}
	smidiCtrl.leave();

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Atable::keyMapping(int event, int ascii, int scan, int state)
{
	switch(event)
	{
		case keyboardDOWN:
		smidiCtrl.enter(__FILE__,__LINE__);
		{
			AkeyCtrl	*kc=keyCtrl[scan];
			while(kc)
			{
				kc->control->keyseq(kc->nctrl, kc->kmode, kc->nkey, event);
				kc=kc->next;
			}
		}
		smidiCtrl.leave();
		break;

		case keyboardUP:
		smidiCtrl.enter(__FILE__,__LINE__);
		{
			AkeyCtrl	*kc=keyCtrl[scan];
			while(kc)
			{
				if(kc->kmode==keymodePRESS)
					kc->control->keyseq(kc->nctrl, kc->kmode, kc->nkey, event);
				kc=kc->next;
			}
		}
		smidiCtrl.leave();
		break;

	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Atable::setBPM(float bpm)
{
	bpmFreq=bpm;
	father->notify(this, nyBPMCHANGE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL float Atable::getBPM()
{
	return bpmFreq;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Atable::record(Acontrol *c, int nctrl, float value)
{
	if(recording)
	{
		sequencer->record(c, nctrl, value, beat);
		return true;
	}
	else
	{
		
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Atable::addOscNode(class AoscNode *n)
{
	n->next=oscNodes;
	oscNodes=n;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Atable::oscMessage(AoscStream *osc, AoscMessage *msg)
{
	AoscMessage *reply=new AoscMessage();
	bool		used=false;
	strcpy(reply->pattern, "\\reply");
	reply->add(new AoscString(msg->pattern));

	if(!strcmp(msg->pattern, "\\"))
	{
		int i;
		for(i=0; i<nbEffect; i++)
			reply->add(new AoscString(effect[i]->name));
		reply->add(new AoscString("master"));
		used=true;
	}
	else if(!strcmp(msg->pattern, oscSTRING[oscTYPESIGNATURE]))
	{
		reply->add(new AoscString(","));		// root signature, reply no params
		used=true;
	}
	else if(!strcmp(msg->pattern, oscSTRING[oscDOCUMENTATION]))
	{
		Awindow	*w=this->getWindow();
		if(w->tooltips)
		{
			reply->add(new AoscString(w->tooltips));
			used=true;
		}
	}
	else if(msg->pattern[0]=='\\')
	{
		char *slash=strchr(msg->pattern+1, '\\');
		if(slash)
		{
			char	name[1024];
			int		i;
			memset(name, 0, sizeof(name));
			strncpy(name, msg->pattern+1, (int)(slash-msg->pattern)-1);
			for(i=0; i<nbEffect; i++)
			{
				Aeffect	*e=effect[i];
				if(!strcmp(e->name, name))
				{
					used=e->NCoscMessage(reply, msg, slash);
					break;
				}
			}
			if(!strcmp("master", name))
			{
				used=oscMasterMessage(reply, msg, slash);
			}
		}
	}

	if(used)
		osc->reply(reply);
	else
		delete(reply);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Atable::oscMasterMessage(AoscMessage *reply, AoscMessage *msg, char *path)
{
	if(!strcmp(path, "\\"))
	{
		AoscNode	*n=oscNodes;
		while(n)
		{
			reply->add(new AoscString(n->name));
			n=(AoscNode *)n->next;
		}
		return true;
	}
	else if(!strcmp(path, oscSTRING[oscTYPESIGNATURE]))
	{
		reply->add(new AoscString(","));
		return true;
	}
	else if(!strcmp(path, oscSTRING[oscDOCUMENTATION]))
	{
		reply->add(new AoscString("master controls"));
		return true;
	}
	else if(path[0]=='\\')
	{
		char *slash=strchr(path+1, '\\');
		if(!slash)
			slash=path+strlen(path);
		{
			char	name[1024];
			memset(name, 0, sizeof(name));
			strncpy(name, path+1, (int)(slash-path)-1);
			{
				AoscNode	*n=oscNodes;
				while(n)
				{
					if(!strcmp(n->name, name))
						return oscMasterMessage(reply, msg, n, slash);
					n=(AoscNode *)n->next;
				}
			}
		}
	}	
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Atable::oscMasterMessage(class AoscMessage *reply, class AoscMessage *msg, class AoscNode *node, char *path)
{
	if(!path[0])	// set value
	{
		if(node==oscBpm)
		{
			if(msg->fchild&&msg->fchild->isCI(&AoscFloat::CI))
			{
				float v=((AoscFloat *)msg->fchild)->value;
				this->setBPM(v);
			}			
		}
		else if (node==oscPlay)
		{
			if(msg->fchild&&msg->fchild->isCI(&AoscFloat::CI))
			{
				float v=((AoscFloat *)msg->fchild)->value;
				if(v==1.f)
					command(Atable::START);
				else
					command(Atable::STOP);
			}			
		}
		else if (node==oscPreset)
		{
			if(msg->fchild&&msg->fchild->isCI(&AoscInteger::CI))
			{
				int v=((AoscInteger *)msg->fchild)->value;
				loadPreset(v, true);
			}			
		}
		else if (node==oscSeqPattern)
		{
			if(msg->fchild&&msg->fchild->isCI(&AoscInteger::CI))
			{
				int v=((AoscInteger *)msg->fchild)->value;
				sequencer->setPattern(v);
			}			
		}
		return false;	// no reply
	}
	else if(!strcmp(path, oscSTRING[oscGETVALUE]))
	{
		if(node==oscBpm)
		{
			reply->add(new AoscFloat(this->bpmFreq));
			return true;
		}
		else if (node==oscPlay)
		{
			reply->add(new AoscFloat(this->isRunning()?1.f:0.f));
			return true;
		}
		else if (node==oscPreset)
		{
			reply->add(new AoscInteger(0));
			return true;
		}
		else if (node==oscSeqPattern)
		{
			reply->add(new AoscInteger(0));	// todo: get the real value
			return true;
		}
	}
	else if(!strcmp(path, oscSTRING[oscGETMINVALUE]))
	{
		if(node==oscBpm)
		{
			reply->add(new AoscFloat(40.00f));
			return true;
		}
		else if (node==oscPlay)
		{
			reply->add(new AoscFloat(0.0f));
			return true;
		}
		else if (node==oscPreset)
		{
			reply->add(new AoscInteger(0));
			return true;
		}
		else if (node==oscSeqPattern)
		{
			reply->add(new AoscInteger(0));
			return true;
		}
	}
	else if(!strcmp(path, oscSTRING[oscGETMAXVALUE]))
	{
		if(node==oscBpm)
		{
			reply->add(new AoscFloat(499.99f));
			return true;
		}
		else if (node==oscPlay)
		{
			reply->add(new AoscFloat(1.0f));
			return true;
		}
		else if (node==oscPreset)
		{
			reply->add(new AoscInteger(127));
			return true;
		}
		else if (node==oscSeqPattern)
		{
			reply->add(new AoscInteger(31));
			return true;
		}
	}
	else if(!strcmp(path, oscSTRING[oscUISIGNATURE]))
	{
		if(node==oscBpm)
		{
			reply->add(new AoscString("bpm"));
			return true;
		}
		else if (node==oscPlay)
		{
			reply->add(new AoscString("2states"));
			return true;
		}
		else if (node==oscPreset)
		{
			return false;
		}
		else if (node==oscSeqPattern)
		{
			return false;
		}
	}
	else if(!strcmp(path, oscSTRING[oscTYPESIGNATURE]))
	{
		reply->add(new AoscString(node->format));
		return true;
	}
	else if(!strcmp(path, oscSTRING[oscDOCUMENTATION]))
	{
		reply->add(new AoscString(node->help));
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL AtableFront::AtableFront(char *name, Aobject *l, Atable *t, int x, int y, int w, int h) : Aobject(name, l, x, y, w, h)
{
	bac=FALSE;
	table=t;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL AtableFront::~AtableFront()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool AtableFront::notify(Anode *o, int event, dword p)
{
/*
	switch(event)
	{
		case nyCONTEXT:
		{
			int	d=((Aitem *)p)->data;
			if((d>=contextADDEFFECT)&&(d<=contextLASTEFFECT))
				return table->notify(o, event, p);
		}
		break;
	}
*/
	return Aobject::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool AtableFront::mouse(int x, int y, int state, int event)
{
	switch(event)
	{
		case mouseLDOWN:
		wy=pos.y;
		ly=pos.y+y;
		bac=TRUE;
		mouseCapture(TRUE);
		return TRUE;

		case mouseNORMAL:
		table->lastMouseY=y;
		if((state&mouseL)&&bac)
		{
			int	lmy=pos.y;
			int	ny=wy+(y+pos.y)-ly;
			int	ht=table->layer->pos.h;
			if(pos.h<=ht)
				ny=0;
			else
			{
				int	d=pos.h-ht;
				if(ny<-d)
					ny=-d;
				else if(ny>0)
					ny=0;
			}
			if(lmy!=ny)
			{
				move(0, ny);
				table->back->pos.y=ny;
				table->ctbar->move(table->ctbar->pos.x, ny);
			}
		}
		return TRUE;

		case mouseLUP:
		bac=FALSE;
		mouseCapture(FALSE);
		return TRUE;

		case mouseWHEEL:
		{
			int	dy=getWindow()->mouseW;
			int	lmy=pos.y;
			int	ny=pos.y+dy;
			int	ht=table->layer->pos.h;
			if(pos.h<=ht)
				ny=0;
			else
			{
				int	d=pos.h-ht;
				if(ny<-d)
					ny=-d;
				else if(ny>0)
					ny=0;
			}
			if(ny!=lmy)
			{
				move(0, ny);
				table->back->pos.y=ny;
				table->ctbar->move(table->ctbar->pos.x, ny);
			}
		}
		return TRUE;

		case mouseRDOWN:
		table->lastMouseY=y;
		return false; // false to be send to context menu
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ApinPreview : public Aobject
{
public:
	AtableBack	*back;
	Apin		*pin;
	Abitmap		*bitmap;
								
	ApinPreview(AtableBack * back, Apin *pin, int x, int y, int w, int h) : Aobject("pinPreview", back, x, y, w, h)
	{
		this->back=back;
		this->pin=pin;
		bitmap=new Abitmap(pos.w-6, (pos.w-6)*3/4);
		timer(40);
	}

	~ApinPreview()
	{
		timer(0);
		delete(bitmap);
	}

	virtual void pulse()
	{
		repaint();
	}

	virtual void paint(Abitmap *b)
	{
		b->boxfa(0, 0, pos.w, pos.h, 0x80000000);
		pin->enter(__FILE__,__LINE__);
		if(pin->isCI(&Avideo::CI))
		{
			Abitmap *img=((Avideo *)pin)->getBitmap();
			if(img)
				b->set(3, 3, pos.w-6, (pos.w-6)*3/4, 0, 0, img->w, img->h, img, bitmapNORMAL, bitmapNORMAL); 
			else
				b->boxf(3, 3, (pos.w-6)+2, ((pos.w-6)*3/4)+2, 0xFF000000);
		}
		else if(pin->isCI(&Asample::CI))
		{
			Apin *pc=((pin->type&pinDIR)==pinIN)?pin->getConnectedPin():pin;
			bitmap->boxf(0, 0, bitmap->w, bitmap->h, 0xFF000000);
			{
				int	v2=bitmap->h>>1;
				int	v4=bitmap->h>>2;
				int	v8=bitmap->h>>3;

				bitmap->line(0, v2, bitmap->w, v2, 0xffc0c0c0);
				bitmap->line(0, v4, bitmap->w, v4, 0xff808080);
				bitmap->line(0, v2+v4, bitmap->w, v2+v4, 0xff808080);
				bitmap->line(0, v8, bitmap->w, v8, 0xff404040);
				bitmap->line(0, v4+v8, bitmap->w, v4+v8, 0xff404040);
				bitmap->line(0, v2+v8, bitmap->w, v2+v8, 0xff404040);
				bitmap->line(0, v2+v4+v8, bitmap->w, v2+v4+v8, 0xff404040);
			}
			if(pc)
			{
				int		h=(bitmap->h>>1);
				int		w=(bitmap->w>>1);
				Asample	*s=(Asample *)pc;
				int		n=s->offset-w-32;
				sword	*sp=s->sample;
				sword	*bp=s->bass;
				sword	*mp=s->medium;
				sword	*tp=s->treble;
				int		i;
				int		vf0,vf1;
				int		bf0,bf1;
				int		mf0,mf1;
				int		tf0,tf1;

				if(n<0)
					n+=s->size;

				vf0=(((int)sp[n<<1]*h)>>15)+h;
				vf1=(((int)sp[(n<<1)+1]*h)>>15)+h;
				bf0=(((int)bp[n<<1]*h)>>15)+h;
				bf1=(((int)bp[(n<<1)+1]*h)>>15)+h;
				mf0=(((int)mp[n<<1]*h)>>15)+h;
				mf1=(((int)mp[(n<<1)+1]*h)>>15)+h;
				tf0=(((int)tp[n<<1]*h)>>15)+h;
				tf1=(((int)tp[(n<<1)+1]*h)>>15)+h;

				n++;
				if(n>s->size)
					n=0;

				for(i=1; i<w; i++)
				{
					int	x=i<<1;
					int	pos=n<<1;
					int	v0=(((int)sp[n<<1]*h)>>15)+h;
					int	v1=(((int)sp[(n<<1)+1]*h)>>15)+h;
					int	b0=(((int)bp[n<<1]*h)>>15)+h;
					int	b1=(((int)bp[(n<<1)+1]*h)>>15)+h;
					int	m0=(((int)mp[n<<1]*h)>>15)+h;
					int	m1=(((int)mp[(n<<1)+1]*h)>>15)+h;
					int	t0=(((int)tp[n<<1]*h)>>15)+h;
					int	t1=(((int)tp[(n<<1)+1]*h)>>15)+h;

					bitmap->boxfa(x-1, bitmap->h-bf1, x, b0, 0xff804040, 0.3f);
					bitmap->boxfa(x-1, bitmap->h-mf1, x, m0, 0xff408040, 0.3f);
					bitmap->boxfa(x-1, bitmap->h-tf1, x, t0, 0xff404080, 0.3f);
					bitmap->line(x-2, vf0, x, v0, 0xffffff00);
					bitmap->line(x-2, vf1, x, v1, 0xff00ff00);

					vf0=v0;
					vf1=v1;
					bf0=b0;
					bf1=b1;
					mf0=m0;
					mf1=m1;
					tf0=t0;
					tf1=t1;
					n++;
					if(n>s->size)
						n=0;
				}
			}
			b->set(3, 3, bitmap);
		}
		if((pin->type&pinDIR)==pinIN)
		{
			Apin *p=pin->getConnectedPin();
			if(p)
			{
				Afont *f=alib.getFont(fontTERMINAL05);
				f->set(b, 3, pos.h-9, p->effect->name, 0xFFC0C0C0);
			}
		}
		pin->leave();
	}

};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL AtableBack::AtableBack(char *name, Aobject *l, Atable *t, int x, int y, int w, int h) : Aobject(name, l, x, y, w, h)
{
	bac=FALSE;
	fromPin=NULL;
	toPin=NULL;
	hiPin=NULL;
	hiPinPreview=NULL;
	table=t;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL AtableBack::~AtableBack()
{
	if(hiPinPreview)
		delete(hiPinPreview);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool AtableBack::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		/*
		case nyCONTEXT:
		{
			int	d=((Aitem *)p)->data;
			if((d>=contextADDEFFECT)&&(d<=contextLASTEFFECT))
				return table->notify(o, event, p);
		}
		break;
		*/

		case nyPINENTER:
		hiPin=(Apin *)o;
		if(hiPinPreview)
		{
			hiPinPreview->destroy();
			hiPinPreview=null;
		}
		{
			int x,y;
			getPosition(hiPin, &x, &y);
			x-=16;
			y+=12;
			x=maxi(x, 14);
			x=mini(x, 458);
			hiPinPreview=new ApinPreview(this, hiPin, x ,y, 60, 56);
			hiPinPreview->show(true);
		}

		repaint();
		return true;

		case nyPINLEAVE:
		if(hiPinPreview)
		{
			hiPinPreview->destroy();
			hiPinPreview=NULL;
		}
		hiPin=null;
		repaint();
		return true;

		case nyPINCLICK:
		assert(o->isCI(&Apin::CI));
		fromPin=(Apin *)o;
		if(fromPin->getCountConnect()==1)
		{
			if(fromPin->connect)
			{
				toPin=fromPin;
				fromPin=fromPin->connect->out;
				table->synchronize.enter(__FILE__,__LINE__);
				table->del(toPin->connect);
				table->synchronize.leave();
			}
			else
			{
				Aconnect	*c=(Aconnect *)(table->connects->fchild);
				while(c)
				{
					Aconnect	*n=(Aconnect *)c->next;
					if(c->out==fromPin)
					{
						toPin=fromPin;
						fromPin=c->in;
						table->synchronize.enter(__FILE__,__LINE__);
						table->del(c);
						table->synchronize.leave();
						break;
					}
					c=n;
				}
				assert(c);
			}
			getPosition(fromPin, &fromX, &fromY);
			getPosition(toPin, &toX, &toY);
			fromX+=fromPin->pos.w>>1;
			fromY+=fromPin->pos.h>>1;
			toX+=toPin->pos.w>>1;
			toY+=toPin->pos.h>>1;
		}
		else
		{
			getPosition(fromPin, &fromX, &fromY);
			fromX+=fromPin->pos.w>>1;
			fromY+=fromPin->pos.h>>1;
			toX=fromX;
			toY=fromY;
		}
		mouseCapture(TRUE);
		return TRUE;
	}
	return Aobject::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool AtableBack::mouse(int x, int y, int state, int event)
{
	if(fromPin)
	{
		switch(event)
		{
			case mouseNORMAL:
			table->lastMouseY=y;
			{
				Aobject	*o=getObject(x, y);
				if(o->getCI()==fromPin->getCI())
				{
					Arectangle	r(toX, toY);
					toPin=(Apin *)o;
					if((toPin->type&pinDIR)!=(fromPin->type&pinDIR))
					{
						getPosition(toPin, &toX, &toY);
						toX+=toPin->pos.w>>1;
						toY+=toPin->pos.h>>1;
						r.add(toX, toY);
						r.add(fromX, fromY);
						repaint(r, true);
						return TRUE;
					}
				}
				{
					int	ht=table->layer->pos.h;
					int	ny=pos.y;
					if(pos.h<=ht)
						ny=0;
					else if((y+pos.y)<0)
					{
						ny=-y;
						if(ny>0)
							ny=0;
					}
					else if((y+pos.y)>=ht)
					{
						int	d=pos.h-ht;
						int	ony=ny;
						ny=pos.y-(y+pos.y-ht);
						if(ny<-d)
							ny=-d;
					}
					if(ny!=pos.y)
					{
						move(0, ny);
						table->front->pos.y=ny;
						table->ctbar->move(table->ctbar->pos.x, ny);
					}
					y+=ny-pos.y;
					{
						Arectangle	r(toX, toY);
						r.add(x, y);
						r.add(fromX, fromY);
						toX=x;
						toY=y;
						toPin=NULL;
						repaint(r, true);
					}
				}
			}
			return TRUE;

			case mouseLUP:
			{
				bool	rall=false;
				if(fromPin&&toPin)
				{
					Apin	*in=((fromPin->type&pinDIR)==pinIN)?fromPin:toPin;
					Apin	*out=((fromPin->type&pinDIR)==pinIN)?toPin:fromPin;
					table->synchronize.enter(__FILE__,__LINE__);
					if(in->connect)
						table->del(in->connect);
					table->add(new Aconnect("connect", table, in, out));
					table->order();
					table->synchronize.leave();
				}
				fromPin=FALSE;
				toPin=FALSE;
				mouseCapture(FALSE);
				repaint();
			}
			return TRUE;

			case mouseWHEEL:
			{
				int	dy=getWindow()->mouseW;
				int	lmy=pos.y;
				int	ny=pos.y+dy;
				int	ht=table->layer->pos.h;
				if(pos.h<=ht)
					ny=0;
				else
				{
					int	d=pos.h-ht;
					if(ny<-d)
						ny=-d;
					else if(ny>0)
						ny=0;
				}
				if(ny!=lmy)
				{
					move(0, ny);
					table->ctbar->move(table->ctbar->pos.x, ny);
					table->front->pos.y=ny;
				}
			}
			return TRUE;

			case mouseRDOWN:
			table->lastMouseY=y;
			return false; // false to be send to context menu
		}
	}
	else
	{
		switch(event)
		{
			case mouseLDOWN:
			wy=pos.y;
			ly=pos.y+y;
			bac=TRUE;
			mouseCapture(TRUE);
			return TRUE;

			case mouseNORMAL:
			table->lastMouseY=y;
			if((state&mouseL)&&bac)
			{
				int	lmy=pos.y;
				int	ny=wy+(y+pos.y)-ly;
				int	ht=table->layer->pos.h;
				if(pos.h<=ht)
					ny=0;
				else
				{
					int	d=pos.h-ht;
					if(ny<-d)
						ny=-d;
					else if(ny>0)
						ny=0;
				}
				if(ny!=lmy)
				{
					move(0, ny);
					table->front->pos.y=ny;
					table->ctbar->move(table->ctbar->pos.x, ny);
				}
			}
			return TRUE;

			case mouseLUP:
			bac=FALSE;
			mouseCapture(FALSE);
			return TRUE;

			case mouseWHEEL:
			{
				int	dy=getWindow()->mouseW;
				int	lmy=pos.y;
				int	ny=pos.y+dy;
				int	ht=table->layer->pos.h;
				if(pos.h<=ht)
					ny=0;
				else
				{
					int	d=pos.h-ht;
					if(ny<-d)
						ny=-d;
					else if(ny>0)
						ny=0;
				}
				if(ny!=lmy)
				{
					move(0, ny);
					table->front->pos.y=ny;
					table->ctbar->move(table->ctbar->pos.x, ny);
				}
			}
			return TRUE;
			
			case mouseRDOWN:
			table->lastMouseY=y;
			return false; // false to be send to context menu
		}
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AtableBack::update(Abitmap *b)
{
	setConnects();
	Aobject::update(b);
	drawConnects(b);
	if(fromPin)
		b->line(fromX, fromY, toX, toY, 0xff00ff00);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AtableBack::setConnects()
{
	{
		Apin	*p=table->pins;
		while(p)
		{
			p->type&=~pinHILIGHT;
			p=(Apin *)p->nextPin;
		}
	}
	{
		Aconnect	*c=(Aconnect *)(table->connects->fchild);
		while(c)
		{
			if((c->in==hiPin)||(c->out==hiPin))
			{
				c->in->type|=pinHILIGHT;
				c->out->type|=pinHILIGHT;
			}
			c=(Aconnect *)c->next;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AtableBack::drawConnects(Abitmap *b)
{
	Aconnect	*c=(Aconnect *)(table->connects->fchild);
	while(c)
	{
		int		x0,y0,x1,y1;
		dword	color,colorb,colorn;

		if(c->in->isCI(&Avideo::CI))
			color=0xffff0000;
		else if(c->in->isCI(&Asample::CI))
			color=0xffffaa00;
		else
			color=0xffffff00;

		if((c->in==hiPin)||(c->out==hiPin))
			color=0xff40ff40;

		colorb=colormix32(color, 0xffffffff, 0.4f);
		colorn=colormix32(color, 0xff000000, 0.4f);

		getPosition(c->in, &x0, &y0);
		getPosition(c->out, &x1, &y1);

		x0+=c->in->pos.w>>1;
		y0+=c->in->pos.h>>1;
		x1+=c->out->pos.w>>1;
		y1+=c->out->pos.h>>1;

		if(abs(x0-x1)<abs(y0-y1))
		{
			b->linea(x0-1, y0, x1-1, y1, colorb, 0.7f);
			b->linea(x0+1, y0, x1+1, y1, colorn, 0.7f);
		}
		else
		{
			b->linea(x0, y0-1, x1, y1-1, colorb, 0.7f);
			b->linea(x0, y0+1, x1, y1+1, colorn, 0.7f);
		}

		b->line(x0, y0, x1, y1, color);

		c=(Aconnect *)c->next;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL Actbar::Actbar(char *name, Aobject *father, Atable *table, int x, int y, int w, int h) : Aobject(name, father, x, y, w, h)
{
	this->table=table;
	memset(onoff, 0, sizeof(onoff));
	timer(200);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL Actbar::~Actbar()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Actbar::pulse()
{
	repaint();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Actbar::mouse(int x, int y, int state, int event)
{
	switch(event)
	{
		case mouseWHEEL:
		{
			int	dy=getWindow()->mouseW;
			int	lmy=pos.y;
			int	ny=pos.y+dy;
			int	ht=table->layer->pos.h;
			if(pos.h<=ht)
				ny=0;
			else
			{
				int	d=pos.h-ht;
				if(ny<-d)
					ny=-d;
				else if(ny>0)
					ny=0;
			}
			if(ny!=lmy)
			{
				move(table->ctbar->pos.x, ny);
				table->back->move(0, ny);
				table->front->move(0, ny);
			}
		}
		return TRUE;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL Aonoff::Aonoff(char *name, Actbar *ctbar, Aeffect *effect, int x, int y, int w, int h) : Aobject(name, ctbar, x, y, w, h)
{
	lastY=0;
	test=false;
	efsel=false;
	this->effect=effect;
	this->ctbar=ctbar;
	power=0.f;
	bon=new Abutton("bon", this, 3, 4, 9, 9, &resdll.get(MAKEINTRESOURCE(PNG_ONOFF), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	bon->setTooltips("switch on/off");
	bon->setChecked(true);
	bon->show(true);
	timer(500);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL Aonoff::~Aonoff()
{
	delete(bon);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Aonoff::pulse()
{
	if(bon->isChecked()!=effect->onoff)	// from effect action try..catch error
	{
		char	str[1024];
		sprintf(str, "module %s crashed", effect->name); 
		notify(this, nyERROR, (dword)str);
		bon->setChecked(effect->onoff);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Aonoff::paint(Abitmap *b)
{
	int		npow=1+(int)((float)(pos.w-3)*power);
	dword	color;
	dword	bc=effect->selected?0xff80ff80:(effect->onoff?0xffc0c0c0:0xffff8080);
	b->boxfa(0, 0, pos.w, 16, 0xffffffff, 0.8f);
	b->boxfa(0, 17, pos.w, pos.h, bc, 0.6f);
	b->boxfa(0, pos.h-5, pos.w, pos.h-1, 0xffffffff, 0.8f);
	//b->boxf(1, pos.h-4, pos.w-2, pos.h-2, 0xffffffff);
	if(power>0.8f)
		color=0xffff0000;
	else 
		color=0xff000000;
	b->boxf(1, pos.h-4, npow, pos.h-2, color);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aonoff::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCHANGE:
		if(o==bon)
		{
			effect->onoff=bon->isChecked();
		}
		return true;
	}
	return Aobject::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Aonoff::mouse(int x, int y, int state, int event)
{
	switch(event)
	{
		case mouseLDOWN:
		{
			int	xx;
			ctbar->mousePosition(&xx, &lastY);
		}
		test=true;
		mouseCapture(true);
		cursor(cursorHANDKEEP);
		
		efsel=false;
		if((state&mouseSHIFT)||(state&mouseCTRL))
			effect->selected=!effect->selected;
		else if(!effect->selected)
			effect->capsule->select();
		else
			efsel=true;
		return true;

		case mouseNORMAL:
		if(test)
		{
			int	x,y;
			int	ht=effect->capsule->table->layer->pos.h;
			int	ny=ctbar->pos.y;
			ctbar->mousePosition(&x, &y);
			if(ctbar->pos.h<=ht)
				ny=0;
			else if((y+ctbar->pos.y)<0)
			{
				ny=-y;
				if(ny>0)
					ny=0;
			}
			else if((y+ctbar->pos.y)>=ht)
			{
				int	d=ctbar->pos.h-ht;
				int	ony=ny;
				ny=ctbar->pos.y-(y+ctbar->pos.y-ht);
				if(ny<-d)
					ny=-d;
			}
			if(ny!=ctbar->pos.y)
			{
				ctbar->move(ctbar->pos.x, ny);
				effect->capsule->table->front->move(0, ny);
				effect->capsule->table->back->move(0, ny);
				//repaint();
			}
		}
		return true;

		case mouseLUP:
		case mouseLEAVE:
		if(test)
		{
			int	mx,my;
			test=false;
			mouseCapture(false);
			ctbar->mousePosition(&mx, &my);
			if(!effect->capsule->table->deplace(my, lastY)&&efsel)
				effect->capsule->table->select(effect);
		}
		return true;

	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
