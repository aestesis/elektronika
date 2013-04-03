/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CONFIG.CPP					(c)	YoY'00						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<math.h>
#include						"config.h"
#include						"main.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Aconfig::CI=ACI("Aconfig", GUID(0x11111111,0x00000110), &Aobject::CI, 0, NULL);
ACI								Apass::CI=ACI("Apass", GUID(0x11111111,0x00000115), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aconfig::Aconfig(char *name, Aobject *father) : Aobject(name, father, (father->pos.w-400)>>1, (father->pos.h-400)>>1, 400, 400)
{
	//zorder(zorderTOP);
	MYwindow	*w=(MYwindow *)getWindow();
	setTooltips("properties dialog");

	close=new Abutton("close", this, pos.w-100, pos.h-30, 90, 20, "CLOSE");
	close->setTooltips("close the properties dialog");
	close->show(TRUE);

	/*
	regist=new Abutton("register", this, 10, pos.h-30, 90, 20, "regist");
	regist->setTooltips("register information");
	{
		MYwindow	*w=(MYwindow *)father;
		Aregistry	*reg=w->regpwd;
		char		sname[256]="";
		char		spwd[256]="";

		reg->read("name", sname, sizeof(sname));
		reg->read("pwd", spwd, sizeof(spwd));

		regist->show(!VerifyPassword(sname, spwd));
	}
	*/

	resItems=new Aitem("resolutions", "resolutions", NULL, 0);
	
	{
		int	i;
		for(i=0; i<Atable::reso_MAXIMUM; i++)
			new Aitem(Atable::infoReso[i].help, Atable::infoReso[i].help, resItems, i, this);
	}

	resList=new Alist("video resolution", this, 100, 40, pos.w-120, 20, resItems);
	resList->setCurrentByData(((MYwindow *)father)->currentReso);
	resList->setTooltips("video resolution selector");
	resList->show(TRUE);
	
	rateItems=new Aitem("rate", "rate", NULL, 0);
	new Aitem("8 f/s", "8 frames per seconde", rateItems, rate8, this);
	new Aitem("12 f/s", "12 frames per seconde", rateItems, rate12, this);
	new Aitem("20 f/s", "20 frames per seconde", rateItems, rate20, this);
	new Aitem("24 f/s", "24 frames per seconde", rateItems, rate24, this);
	new Aitem("25 f/s", "25 frames per seconde", rateItems, rate25, this);
	new Aitem("30 f/s", "30 frames per seconde", rateItems, rate30, this);
	new Aitem("50 f/s", "50 frames per seconde", rateItems, rate50, this);
	new Aitem("60 f/s", "60 frames per seconde", rateItems, rate60, this);

	rateList=new Alist("video frame rate", this, 100, 70, pos.w-120, 20, rateItems);
	rateList->setCurrentByData(((MYwindow *)father)->currentRate);
	rateList->setTooltips("video frame rate selector");
	rateList->show(TRUE);

	timeItems=new Aitem("time", "time", NULL, 0);
	new Aitem("4 s", "4 secondes", timeItems, 4, this);
	new Aitem("8 s", "8 secondes", timeItems, 8, this);
	new Aitem("12 s", "12 secondes", timeItems, 12, this);
	new Aitem("16 s", "16 secondes", timeItems, 16, this);
	new Aitem("20 s", "20 secondes", timeItems, 20, this);

	timeList=new Alist("timeList", this, 100, 100, pos.w-270, 20, timeItems);
	timeList->setCurrentByData(((MYwindow *)father)->table->vlooptime);
	timeList->setTooltips("video buffer time (eg: rnbox or loop107)");
	timeList->show(TRUE);

	cmpItems=new Aitem("cmp", "cmp", NULL, 0);
	new Aitem("[none]", "no compression, raw format", cmpItems, compressNONE, this);
	new Aitem("JPEG 100%", "compression, jpeg - quality 100%", cmpItems, compressJPEG100, this);
	new Aitem("JPEG 80%", "compression, jpeg - quality 80%", cmpItems, compressJPEG80, this);
	new Aitem("JPEG 50%", "compression, jpeg - quality 50%", cmpItems, compressJPEG50, this);
	
	cmpList=new Alist("cmpList", this, pos.w-160, 100, 140, 20, cmpItems);
	if(((MYwindow *)father)->table->compressRamVidz)
	{
		if(((MYwindow *)father)->table->compressJpegQuality>0.8f)
			cmpList->setCurrentByData(compressJPEG100);
		else if(((MYwindow *)father)->table->compressJpegQuality>0.5f)
			cmpList->setCurrentByData(compressJPEG80);
		else
			cmpList->setCurrentByData(compressJPEG50);
	}
	else
			cmpList->setCurrentByData(compressNONE);
	cmpList->setTooltips("video loop compression in ram (eg: rnbox)");
	cmpList->show(TRUE);
	
/*
	render=new Abutton("render", this, 100, 100, Abutton::btCHECK);
	render->setTooltips("activate threaded rendering [multi-processor]");
	render->setChecked(w->table->threading);
	render->show(TRUE);
*/
/*
	priority=new Abutton("priority", this, 100, 120, Abutton::btCHECK);
	priority->setTooltips("activate high priority level");
	priority->show(TRUE);
	priority->setChecked(w->table->highPriority);
*/
	/*
	splash=new Abutton("splash", this, 100, 320, Abutton::btCHECK);
	splash->show(TRUE);
	splash->setTooltips("activate splash screen");
	{
		bool		b;
		bool		ok=w->reg->read("splash", &b);
		splash->setChecked((!ok)|b);
	}
	*/

	tips=new Abutton("tooltips", this, 100, 320, Abutton::btCHECK);
	tips->show(TRUE);
	tips->setTooltips("activate tooltips");
	{
		bool		b;
		bool		ok=w->reg->read("tooltips", &b);
		tips->setChecked((!ok)|b);
	}

	midiInItems=new Aitem("midi in", "midi in devices", NULL, 0);
	{
		MIDIINCAPS	mic;
		int			n=midiInGetNumDevs();
		int			i;
		for(i=0; i<n; i++)
		{
			if(midiInGetDevCaps(i, &mic, sizeof(mic))==MMSYSERR_NOERROR)
			{
				Aitem *it=new Aitem(mic.szPname, mic.szPname, midiInItems, i, this);
				if(((MYwindow *)father)->table->midiInDeviceF&(1i64<<i))
					it->state|=Aitem::stateSELECTED;
			}
		}
	}

	midiOutItems=new Aitem("midi out", "midi out devices", NULL, 0);
	new Aitem("[none]", "no midi out device selected", midiOutItems, -1, this);
	{
		MIDIOUTCAPS	moc;
		int			n=midiOutGetNumDevs();
		int			i;
		for(i=0; i<n; i++)
		{
			if(midiOutGetDevCaps(i, &moc, sizeof(moc))==MMSYSERR_NOERROR)
				new Aitem(moc.szPname, moc.szPname, midiOutItems, i, this);
		}
	}

	psetItems=new Aitem("pset", "presets channel", NULL, -1);
	new Aitem("[none]", "no preset / program change", psetItems, -1, this);
	{
		int	i;
		for(i=0; i<16; i++)
		{
			char	str[256];
			char	st2[256];
			sprintf(str, "%d", i+1);
			sprintf(st2, "midi channel #%d", i+1);
			new Aitem(str, st2, psetItems, i, this);
		}
	}
	new Aitem("[all]", "all channels", psetItems, 16, this);

	midiIn=new Amlist("midi in devices", this, 100, 160, pos.w-120, 50, midiInItems);
	midiIn->setTooltips("midi in devices");
	midiIn->show(TRUE);

	midiOut=new Alist("midi out devices", this, 100, 220, pos.w-120, 20, midiOutItems);
	midiOut->setCurrentByData(((MYwindow *)father)->table->midiOutDevice);
	midiOut->setTooltips("midi out devices");
	midiOut->show(TRUE);

	pset=new Alist("pset", this, 100, 250, pos.w-120, 20, psetItems);
	pset->setCurrentByData(((MYwindow *)father)->table->midiProgChangeChannel);
	pset->setTooltips("presets - program change event midi channel");
	pset->show(TRUE);

	sndsize=new Aselsize("sndsize", this, 100, 280, pos.w-120, 20);
	sndsize->set(((MYwindow *)father)->table->dsoundsize);
	{
		char	str[256];
		float	v=(sndsize->get()*(float)(DSOUNDSIZE-DSOUNDSIZEMIN)+(float)DSOUNDSIZEMIN)/(44100.f*2.f);	// 2x >> double event buffer
		sprintf(str, "%5.2f ms", v*1000.f);
		sndsize->set(str);
	}
	sndsize->setTooltips("direct sound buffer latency");
	sndsize->show(true);

	timer(500);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aconfig::~Aconfig()
{
	((MYwindow *)father)->config=NULL;
	delete(close);
	//delete(regist);
	delete(resList);
	delete(resItems);
	delete(rateList);
	delete(rateItems);
	delete(timeList);
	delete(timeItems);
	delete(cmpList);
	delete(cmpItems);
	//delete(render);
	//delete(priority);
	delete(midiIn);
	delete(midiInItems);
	delete(midiOut);
	delete(midiOutItems);
	delete(pset);
	delete(psetItems);
	//delete(splash);
	delete(tips);
	((MYwindow *)father)->settings();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aconfig::mouse(int x, int y, int state, int event)
{
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aconfig::notify(Anode *o, int event, dword p)
{
	MYwindow	*w=(MYwindow *)father;

	switch(event)
	{
		case nyCLICK:
		if(o==close)
		{
			destroy();
			return TRUE;
		}
		else if(o==regist)
		{
			Aobject *o=new Apass("register", (Aobject *)father);
			o->show(true);
			o->repaint();
		}
		break;

		case nyCHANGE:
		if(o==tips)
		{
			bool		b=tips->isChecked();
			w->reg->write("tooltips", b);
			w->btips=b;
		}
		else if(o==resList)
		{
			Aitem		*i=(Aitem *)p;
			w->currentReso=i->data;
			w->table->videoW=Atable::infoReso[w->currentReso].width;
			w->table->videoH=Atable::infoReso[w->currentReso].height;
		}
		else if(o==rateList)
		{
			Aitem		*i=(Aitem *)p;
			w->currentRate=i->data;
			switch(i->data)
			{
				case rate8:
				w->table->frameRate=8;
				break;
				case rate12:
				w->table->frameRate=12;
				break;
				case rate20:
				w->table->frameRate=20;
				break;
				case rate24:
				w->table->frameRate=24;
				break;
				case rate25:
				w->table->frameRate=25;
				break;
				case rate30:
				w->table->frameRate=30;
				break;
				case rate50:
				w->table->frameRate=50;
				break;
				case rate60:
				w->table->frameRate=60;
				break;
			}
		}
		/*
		else if(o==render)
		{
			w->table->threading=render->isChecked();
		}
		else if(o==priority)
		{
			w->table->highPriority=priority->isChecked();
		}
		*/
		else if(o==timeList)
		{
			Aitem		*i=(Aitem *)p;
			w->table->vlooptime=i->data;
		}
		else if(o==cmpList)
		{
			Aitem		*i=(Aitem *)p;
			switch(i->data)
			{
				case compressNONE:
				w->table->compressRamVidz=false;
				w->table->compressJpegQuality=1.f;
				break;
				case compressJPEG100:
				w->table->compressRamVidz=true;
				w->table->compressJpegQuality=1.f;
				break;
				case compressJPEG80:
				w->table->compressRamVidz=true;
				w->table->compressJpegQuality=0.8f;
				break;
				case compressJPEG50:
				w->table->compressRamVidz=true;
				w->table->compressJpegQuality=0.5f;
				break;
			}
		}
		else if(o==midiIn)
		{
			Aitem		*i=(Aitem *)p;
			if(i->state&Aitem::stateSELECTED)
				w->table->midiInDeviceF|=(1i64<<i->data);
			else
				w->table->midiInDeviceF&=~(1i64<<i->data);
		}
		else if(o==midiOut)
		{
			Aitem		*i=(Aitem *)p;
			w->table->midiOutDevice=i->data;
		}
		else if(o==pset)
		{
			Aitem		*i=(Aitem *)p;
			w->table->midiProgChangeChannel=i->data;
		}
		else if(o==sndsize)
		{
			char	str[256];
			float	v=(sndsize->get()*(float)(DSOUNDSIZE-DSOUNDSIZEMIN)+(float)DSOUNDSIZEMIN)/(44100.f*2.f);	// 2x >> double event buffer
			sprintf(str, "%5.2f ms", v*1000.f);
			sndsize->set(str);
			w->table->dsoundsize=sndsize->get();
		}
		return true;
	}
	return Aobject::notify(o, event , p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aconfig::paint(Abitmap *b)
{
	Afont	*ft=alib.getFont(fontCONFIDENTIAL14);
	Afont	*f=alib.getFont(fontTERMINAL06);
	b->boxfa(0, 0, pos.w, pos.h, 0xff000000, 0.8f);
	b->box(0, 0, pos.w-1, pos.h-1, 0xff000000);
	b->box(1, 1, pos.w-2, pos.h-2, 0xffffffff);
	ft->set(b, 8, 8, "Properties", 0xff00ff00);
	f->set(b, 32, 46, "VIDEO", 0xffffffff);
	f->set(b, 32, 76, "FRAMES", 0xffffffff);
	f->set(b, 32, 106, "LOOP", 0xffffffff);
	//f->set(b, 130, 104, "THREADED RENDERING", 0xffffffff);
	//f->set(b, 130, 124, "HIGHT PRIORITY MODE", 0xffffffff);
	f->set(b, 32, 166, "MIDI IN", 0xffffffff);
	f->set(b, 32, 226, "MIDI OUT", 0xffffffff);
	f->set(b, 32, 256, "PRESET", 0xffffffff);
	f->set(b, 32, 286, "DX-SOUND", 0xffffffff);
	//f->set(b, 130, 324, "SPLASH SCREEN", 0xffffffff);
	f->set(b, 130, 324, "TOOLTIPS", 0xffffffff);
	/*
	if(!(regist->state&stateVISIBLE))
	{
		MYwindow	*w=(MYwindow *)father;
		Aregistry	*reg=w->regpwd;
		char		sname[256]="";
		reg->read("name", sname, sizeof(sname));
		strupr(sname);
		f->set(b, 10, pos.h-30, "REGISTERED TO", 0xffffffff);
		f->set(b, 10, pos.h-20, sname, 0xff00ff00);
	}
	*/
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aconfig::pulse()
{
	/*
	{
		MYwindow	*w=(MYwindow *)father;
		Aregistry	*reg=w->regpwd;
		char		sname[256]="";
		char		spwd[256]="";

		reg->read("name", sname, sizeof(sname));
		reg->read("pwd", spwd, sizeof(spwd));

		regist->show(!VerifyPassword(sname, spwd));
	}
	*/
	repaint();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Apass::Apass(char *name, Aobject *father) : Aobject(name, father, (father->pos.w-400)>>1, (father->pos.h-400)>>1, 400, 400)
{
	MYwindow	*w=(MYwindow *)father;
	Aregistry	*reg=w->regpwd;
	char		sname[256]="";
	char		spwd[256]="";

	reg->read("name", sname, sizeof(sname));
	reg->read("pwd", spwd, sizeof(spwd));

	//zorder(zorderTOP);
	setTooltips("registration dialog");

	close=new Abutton("OK", this, pos.w-100, pos.h-30, 90, 20, "OK");
	close->setTooltips("close the properties dialog");
	close->show(TRUE);

	ename=new Aedit("ename", this, 100, 60, 200, 14);
	ename->setTooltips("register name");
	ename->set(sname);
	ename->show(true);

	epwd=new Aedit("epwd", this, 100, 90, 200, 14);
	epwd->setTooltips("register pass");
	epwd->set(spwd);
	epwd->show(true);

	web=new Abutton("web", this, 100, 150, 200, 20, "GET A KEY");
	web->setTooltips("go the aestesis.org web site");
	web->show(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Apass::~Apass()
{
	delete(close);
	delete(ename);
	delete(epwd);
	delete(web);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Apass::mouse(int x, int y, int state, int event)
{
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Apass::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCLICK:
		if(o==close)
		{
			destroy();
			return TRUE;
		}
		else if(o==web)
		{
			ShellExecute(getWindow()->hw, "open", "http://aestesis.org/", NULL, NULL, SW_SHOWNORMAL);
			//httpto("http://www.aestesis.org/aestesis/register.php");
		}
		break;

		case nyCHANGE:
		if((o==ename)||(o==epwd))
		{
			MYwindow	*w=(MYwindow *)father;
			Aregistry	*reg=w->regpwd;
			char		sname[256]="";
			char		spwd[256]="";
			ename->get(sname, sizeof(sname));
			epwd->get(spwd, sizeof(spwd));
			reg->write("name", sname);
			reg->write("pwd", spwd);
			strcpy(w->reg_name, sname);
			strcpy(w->reg_key, spwd);
		}
		break;
	}
	return Aobject::notify(o, event , p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Apass::paint(Abitmap *b)
{
	Afont	*ft=alib.getFont(fontCONFIDENTIAL14);
	Afont	*f=alib.getFont(fontTERMINAL06);
	b->boxf(0, 0, pos.w, pos.h, 0xffffffff);
	b->box(0, 0, pos.w-1, pos.h-1, 0xff000000);
	b->box(1, 1, pos.w-2, pos.h-2, 0xff808080);
	ft->set(b, 8, 8, "Registration Information", 0xff404040);
	f->set(b, 32, 63, "NAME", 0xff404040);
	f->set(b, 32, 93, "PWD", 0xff404040);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Apass::pulse()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
