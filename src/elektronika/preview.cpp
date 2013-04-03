/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	PREVIEW.CPP					(c)	YoY'00						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"main.h"
#include						"config.h"
#include						"preview.h"
#include						"global.h"
#include						"resource.h"
#include						"presets.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Apreview::CI=ACI("Apreview", GUID(0x11111111,0x00000002), &Aobject::CI, 0, NULL);
ACI								Amonitor::CI=ACI("Amonitor", GUID(0x11111111,0x00000007), &Asurface::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define							FOLLOW						((Apin *)1)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Apreview::Apreview(char *name, Aobject *l, int x, int y, int w, int h) : Aobject(name, l, x, y, w, h)
{
	int		ha=((pos.w-20)*240)/320;
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_MIRE_NB_220), "PNG");

	bNewClockItem=false;

	mire=new Abitmap(&o);
	display[0]=new Amonitor("monitor one", this, 10, 10, w-20, ha);
	display[0]->setTooltips("monitor one");
	display[0]->show(TRUE);
	display[1]=new Amonitor("monitor two", this, 10, w-20, w-20, ha);
	display[1]->setTooltips("monitor two");
	display[1]->show(TRUE);

	dispitems=new Aitem("video out", "video out");
	new Aitem("[none]", "no input selected", dispitems, 0);
	new Aitem("[auto follow]", "follow mouse", dispitems, 1);

	displist[0]=new Alist("monitor 1 list", this, 10, ha+20, w-20, 20, dispitems);
	displist[0]->setTooltips("monitor one input selector");
	displist[0]->show(TRUE);

	displist[1]=new Alist("monitor 2 list", this, 10, w+ha-10, w-20, 20, dispitems);
	displist[1]->setTooltips("monitor two input selector");
	displist[1]->show(TRUE);

	buttonNew=new Abutton("buttonNew", this, 10, h-26, 16, 16, &resource.get(MAKEINTRESOURCE(PNG_ICONNEW), "PNG"));
	buttonNew->setTooltips("new project");
	buttonNew->show(TRUE);

	buttonOpen=new Abutton("buttonOpen", this, 30, h-26, 16, 16, &resource.get(MAKEINTRESOURCE(PNG_ICONOPEN), "PNG"));
	buttonOpen->setTooltips("open project");
	buttonOpen->show(TRUE);

	buttonSave=new Abutton("buttonSave", this, 50, h-26, 16, 16, &resource.get(MAKEINTRESOURCE(PNG_ICONSAVE), "PNG"));
	buttonSave->setTooltips("save current project");
	buttonSave->show(TRUE);

	buttonSaveAs=new Abutton("buttonSaveAs", this, 70, h-26, 16, 16, &resource.get(MAKEINTRESOURCE(PNG_ICONSAVEAS), "PNG"));
	buttonSaveAs->setTooltips("save current project as ... ");
	buttonSaveAs->show(TRUE);

	buttonProp=new Abutton("properties", this, 90, h-26, 16, 16, &resource.get(MAKEINTRESOURCE(PNG_BUTTONPROP), "PNG"));
	buttonProp->setTooltips("properties dialog");
	buttonProp->show(TRUE);

	buttonHelp=new Abutton("help", this, 110, h-26, 16, 16, &resource.get(MAKEINTRESOURCE(PNG_ICONHELP), "PNG"));
	buttonHelp->setTooltips("help");
	buttonHelp->show(TRUE);

	buttonStop=new Abutton("stop", this, 10, h-64, 16, 16, &resource.get(MAKEINTRESOURCE(PNG_BUTTONSTOP), "PNG"));
	buttonStop->setTooltips("stop");
	buttonStop->show(TRUE);
	buttonPlay=new Abutton("play", this, 90, h-64, 16, 16, &resource.get(MAKEINTRESOURCE(PNG_BUTTONPLAY), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	buttonPlay->setTooltips("play");
	buttonPlay->show(TRUE);

	buttonFirst=new Abutton("start", this, 30, h-64, 16, 16, &resource.get(MAKEINTRESOURCE(PNG_BUTTONFIRST), "PNG"));
	buttonFirst->setTooltips("start");
	buttonRew=new Abutton("rewind", this, 50, h-64, 16, 16, &resource.get(MAKEINTRESOURCE(PNG_BUTTONREWIND), "PNG"));
	buttonRew->setTooltips("rewind");
	buttonRec=new Abutton("record", this, 70, h-64, 16, 16, &resource.get(MAKEINTRESOURCE(PNG_BUTTONREC), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	buttonRec->setTooltips("record");
	buttonFew=new Abutton("few", this, 110, h-64, 16, 16, &resource.get(MAKEINTRESOURCE(PNG_BUTTONFWRD), "PNG"));
	buttonFew->setTooltips("forward");
	buttonLast=new Abutton("end", this, 130, h-64, 16, 16, &resource.get(MAKEINTRESOURCE(PNG_BUTTONLAST), "PNG"));
	buttonLast->setTooltips("end");

	buttonFirst->show(TRUE);
	buttonRew->show(TRUE);
	buttonRec->show(TRUE);
	buttonFew->show(TRUE);
	buttonLast->show(TRUE);

	bpm=new Asegment("bpm", this, 180, h-63, 4, 1, 4999, alib.getFont(fontSEGMENT10), 0xffffff00, 0.05f, 1);
	bpm->set(1200);
	bpm->setTooltips("bpm display");
	bpm->show(true);
	
	clockItems=new Aitem("clock", "clock type", NULL, 0);
	new Aitem("int clock", "internal clock", clockItems, clockINT, this);
	new Aitem("midi clock", "external midi clock", clockItems, clockMIDI, this);

	clock=new Alist("clock", this, 160, h-88, w-160-2, 17, clockItems);
	clock->setCurrentByData(clockINT);
	clock->setTooltips("clock type");
	clock->show(TRUE);

	presets=new Apresets("presets", this, 0, 0, w, h-90);
	presets->setTooltips("presets");

	mapping=new Amapping("mapping", this, 0, 0, w, h-90);
	mapping->setTooltips("mapping");

	tap=new Abutton("tap", this, 160, h-43, w-160, 43, "TAP");
	tap->setTooltips("tap clock");
	tap->show(true);

	toolbar=new AselButton("toolbar", this, 6, h-88, 3, 1, &resource.get(MAKEINTRESOURCE(PNG_DISP_TOOLBAR), "PNG"), 24, 16);
	toolbar->setTooltips("tab selection");
	toolbar->show(true);

	timer(500);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Apreview::~Apreview()
{
	delete(mapping);

	delete(presets);

	delete(toolbar);
	delete(tap);
	
	delete(buttonStop);
	delete(buttonPlay);
	delete(buttonFirst);
	delete(buttonRew);
	delete(buttonRec);
	delete(buttonFew);
	delete(buttonLast);

	delete(bpm);

	delete(clock);
	delete(clockItems);

	delete(buttonNew);
	delete(buttonOpen);
	delete(buttonSave);
	delete(buttonSaveAs);
	delete(buttonProp);
	delete(buttonHelp);
	delete(display[0]);
	delete(display[1]);
	delete(displist[0]);
	delete(displist[1]);
	delete(mire);
	delete(dispitems);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Apreview::setTable(Atable *table)
{
	this->table=table;
	table->mapping=mapping;
	mapping->setTable(table);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Apreview::clear()
{
	presets->clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Apreview::load(Afile *f)
{
	int	i;
	for(i=0; i<2; i++)
	{
		QIID	q;
		f->read(&q, sizeof(q));
		if(q==1)	// [auto follow]
		{
			display[i]->setPin(FOLLOW);
			displist[i]->setCurrentByData(1);
		}
		else if(q)	// pin
		{
			Aitem	*it=(Aitem *)dispitems->fchild;
			while(it)
			{
				Apin	*p=(Apin *)it->data;
				if(p&&(q==p->qiid))
					break;
				it=(Aitem *)it->next;
			}
			if(it)
			{
				display[i]->setPin((Apin *)it->data);
				displist[i]->setCurrentByData(it->data);
			}
			else
			{
				display[i]->setPin(0);
				displist[i]->setCurrentByData(0);
			}
		}
		else	// [none]
		{
			display[i]->setPin(0);
			displist[i]->setCurrentByData(0);
		}
	}

	presets->load(f);
	
	bpm->set((int)(((MYwindow *)getWindow())->table->getBPM()*10.f));
	
	refreshClockType(((MYwindow *)getWindow())->table);
	clock->setCurrentByData(((MYwindow *)getWindow())->table->clockType);
	refreshClockType(((MYwindow *)getWindow())->table);
	
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Apreview::save(Afile *f)
{
	int	i;
	for(i=0; i<2; i++)
	{
		QIID	q=0;
		if(display[i]->pin)
		{
			if(display[i]->pin==FOLLOW)
				q=1;
			else
				q=display[i]->pin->qiid;
		}
		f->write(&q, sizeof(q));
	}
	
	presets->save(f);
	
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Apreview::size(int w, int h)
{
	if(Aobject::size(w, h))
	{
		buttonNew->pos.y=h-26;
		buttonOpen->pos.y=h-26;
		buttonSave->pos.y=h-26;
		buttonSaveAs->pos.y=h-26;
		buttonProp->pos.y=h-26;
		buttonHelp->pos.y=h-26;
		buttonStop->pos.y=h-64;
		buttonPlay->pos.y=h-64;
		buttonFirst->pos.y=h-64;
		buttonRew->pos.y=h-64;
		buttonRec->pos.y=h-64;
		buttonFew->pos.y=h-64;
		buttonLast->pos.y=h-64;
		tap->pos.y=h-43;
		presets->size(presets->pos.w, h-90);
		mapping->size(mapping->pos.w, h-90);
		bpm->pos.y=h-63;
		clock->pos.y=h-88;
		toolbar->pos.y=h-88;
		return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Apreview::pulse()
{
	if(bNewClockItem)
	{
		refreshClockType(((MYwindow *)getWindow())->table);
		bNewClockItem=false;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Apreview::paint(Abitmap *b)
{
	b->boxfa(0, 0, pos.w-1, pos.h-1, 0xffffffff, 0.5f);
	b->boxfa(0, pos.h-70, pos.w-1, pos.h-44, 0xff000000, 0.5f);
	b->boxfa(160, pos.h-70, pos.w-1, pos.h-44, 0xff000000, 0.4f);
	b->boxfa(0, pos.h-90, pos.w-1, pos.h-71, 0xff404040, 0.5f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Apreview::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyBPMCHANGE:
		if(!(bpm->state&stateENABLE))
		{
			bpm->set((int)(((MYwindow *)getWindow())->table->getBPM()*10.f));
		}
		break;

		case nySELECT:
		if(o->isCI(&Acontrol::CI))
			mapping->setControl((Acontrol *)o);
		break;
		
		case nyCHANGE:
		if(o==bpm)
		{
			Atable		*table=((MYwindow *)getWindow())->table;
			table->synchronize.enter(__FILE__,__LINE__);
			table->setBPM((float)p/10.f);
			table->synchronize.leave();
		}
		else if(o==clock)
		{
			Aitem		*i=(Aitem *)p;
			Atable		*table=((MYwindow *)getWindow())->table;
			table->clockMidiCount=0;
			table->clockType=i->data;
			refreshClockType(table);
			tap->setCaption("TAP");
		}
		else if(o==displist[0])
		{
			display[0]->setPin((Apin *)((Aitem *)p)->data);
		}
		else if(o==displist[1])
		{
			display[1]->setPin((Apin *)((Aitem *)p)->data);
		}
		else if(o->isCI(&Apin::CI))
		{
			displist[0]->setCurrentByData((dword)o);
			display[0]->setPin((Apin *)o);
		}
		else if(o->isCI(&Atable::CI))
		{
			dword data0=displist[0]->getCurrentData();
			dword data1=displist[1]->getCurrentData();
			dispitems->clear();
			new Aitem("[none]", "no input selected", dispitems, 0);
			new Aitem("[auto follow]", "follow mouse", dispitems, 1);
			{
				char	nm[256];
				Atable	*t=(Atable *)o;
				int		nbe=t->nbEffect;
				int		ie;
				for(ie=0; ie<nbe; ie++)
				{
					Aeffect *effect=t->effect[nbe-ie-1];
					Apin	*p=t->pins;
					while(p)
					{
						if(p->effect==effect)
						{
							if((p->type&pinDIR)==pinOUT)
							{
								sprintf(nm, "%s / %s", p->effect->name, p->name);
								new Aitem(nm, nm, dispitems, (dword)p);
							}
						}
						p=p->nextPin;
					}
				}
			}
			displist[0]->setCurrentByData(data0);
			if(displist[0]->getCurrentData()!=-1)
				display[0]->setPin((Apin *)displist[0]->getCurrentData());
			else
				display[0]->setPin(NULL);
			displist[1]->setCurrentByData(data1);
			if(displist[1]->getCurrentData()!=-1)
				display[1]->setPin((Apin *)displist[1]->getCurrentData());
			else
				display[1]->setPin(NULL);
			refreshClockType((Atable *)o);
			((Atable *)o)->clockType=clock->getCurrentData();
			bNewClockItem=true;
		}
		else if(o==buttonPlay)
		{
			if(((MYwindow *)father)->config)
			{
				notify(this, nyALERT, (dword)"can't play when properties dialog is displayed.");
				buttonPlay->setChecked(false);
			}
			else
			{
				if(buttonPlay->isChecked())
				{
					if(!((MYwindow *)father)->table->start())
						buttonPlay->setChecked(false);
				}
				else
					((MYwindow *)father)->table->stop();
			}
		}
		else if(o==buttonRec)
		{
			if(buttonRec->isChecked())
				if(((MYwindow *)father)->table->render)
					buttonRec->setChecked(false);
			((MYwindow *)father)->table->recording=buttonRec->isChecked();
		}
		else if(o==toolbar)
		{
			switch(toolbar->get())
			{
				case 0:
				mapping->setControl(null);
				mapping->show(false);
				presets->show(false);
				display[0]->show(true);
				display[1]->show(true);
				displist[0]->show(true);
				displist[1]->show(true);
				((MYwindow *)father)->table->showMap(false);
				break;

				case 1:
				mapping->setControl(null);
				mapping->show(false);
				display[0]->show(false);
				display[1]->show(false);
				displist[0]->show(false);
				displist[1]->show(false);
				presets->show(true);
				((MYwindow *)father)->table->showMap(false);
				break;

				case 2:
				mapping->show(true);
				presets->show(false);
				display[0]->show(false);
				display[1]->show(false);
				displist[0]->show(false);
				displist[1]->show(false);
				((MYwindow *)father)->table->showMap(true);
				break;
			}
			repaint();
		}
		break;

		case nyCLICK:
		if(o==buttonNew)
		{
			if(buttonPlay->isChecked())
				((MYwindow *)father)->table->stop();
			buttonPlay->setChecked(false);
			((MYwindow *)father)->clear();
			strcpy(((MYwindow *)father)->filename, "newproject.eka");
			((MYwindow *)father)->repaint();
		}
		else if(o==buttonOpen)
		{
			if(buttonPlay->isChecked())
				((MYwindow *)father)->table->stop();
			buttonPlay->setChecked(false);
			{
				MYwindow	*w=(MYwindow *)father;
				AfileDlg	fdlg=AfileDlg("loadDlg", this, "elektronika file\0*.eka\0", w->filename, AfileDlg::LOAD);
				if(fdlg.getFile(w->filename))
					w->load(); 
			}
		}
		else if(o==buttonSave)
		{
			MYwindow	*w=(MYwindow *)father;
			Aregistry	*reg=w->regpwd;
			char		sname[256]="";
			char		spwd[256]="";

			reg->read("name", sname, sizeof(sname));
			reg->read("pwd", spwd, sizeof(spwd));

			if(1/*VerifyPassword(sname, spwd)*/)
			{
				if(buttonPlay->isChecked())
					w->table->stop();
				buttonPlay->setChecked(false);

				if(w->filename[0])
					w->save();
				else
				{
					AfileDlg	fdlg=AfileDlg("saveDlg", this, "elektronika file\0*.eka\0", w->filename, AfileDlg::SAVE);
					if(fdlg.getFile(w->filename))
						w->save();
				}
			}
			else
				notify(this, nyALERT, (dword)"demo version, save not activated");
		}
		else if(o==buttonSaveAs)
		{
			MYwindow	*w=(MYwindow *)father;
			Aregistry	*reg=w->regpwd;
			char		sname[256]="";
			char		spwd[256]="";

			reg->read("name", sname, sizeof(sname));
			reg->read("pwd", spwd, sizeof(spwd));

			if(1/*VerifyPassword(sname, spwd)*/)
			{
				if(buttonPlay->isChecked())
					w->table->stop();
				buttonPlay->setChecked(false);

				{
					AfileDlg	fdlg=AfileDlg("saveDlg", this, "elektronika file\0*.eka\0", w->filename, AfileDlg::SAVE);
					if(fdlg.getFile(w->filename))
						w->save();
				}
			}
			else
				notify(this, nyALERT, (dword)"demo version, save not activated");
		}
		else if(o==buttonProp)
		{
			MYwindow	*w=(MYwindow *)father;
			if(!w->config)
			{
				if(buttonPlay->isChecked())
					((MYwindow *)father)->table->stop();
				buttonPlay->setChecked(false);
				w->config=new Aconfig("config", w);
				w->config->show(true);
				w->config->repaint();
			}
		}
		else if(o==buttonHelp)
		{
			char	help[1024];
			GetModuleFileName(GetModuleHandle(null), help, sizeof(help));
			if(help[0])
			{
				char	*s=strrchr(help, '\\');
				if(s)
					*s=0;
			}
			strcat(help, "\\help\\elektronika.chm");
			ShellExecute(getWindow()->hw, "open", help, NULL, NULL, SW_SHOWNORMAL);
		}
		else if(o==buttonStop)
		{
			if(buttonPlay->isChecked())
				((MYwindow *)father)->table->stop();
			buttonPlay->setChecked(false);
		}
		else if(o==buttonFirst)
		{
			((MYwindow *)father)->table->seek(0.f);
		}
		else if(o==buttonRew)
		{
			double	b=((MYwindow *)father)->table->beat-4.0;
			if(b<0.f)
				b=0.f;
			((MYwindow *)father)->table->seek(b);
		}
		else if(o==buttonFew)
		{
			double	b=((MYwindow *)father)->table->beat+4.0;
			((MYwindow *)father)->table->seek(b);
		}
		else if(o==buttonLast)
		{
			((MYwindow *)father)->table->seek(SEEKLAST);
		}
		else if(o==tap)
		{
			if(clock->getCurrentData()==clockINT)
			{
				if(!strcmp(tap->getCaption(), "TAP"))
				{
					tapTime=(int)Athread::getTickCount();
					tap->setCaption("2");
				}
				else
				{
					int	n=atoi(tap->getCaption());
					if(n>=4)
					{
						int		time=(int)Athread::getTickCount()-tapTime;
						float	cbpm=1000.f*60.f*3.f/(float)time;
						((MYwindow *)getWindow())->table->setBPM(cbpm);
						bpm->set((int)(cbpm*10.f));
						tap->setCaption("TAP");
					}
					else
					{
						char	str[256];
						sprintf(str, "%d", n+1);
						tap->setCaption(str);
					}
				}
			}
		}
		break;
	}
	return Aobject::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Apreview::refreshClockType(Atable *table)
{
	dword data=this->clock->getCurrentData();
	clockItems->clear();
	new Aitem("int clock", "internal clock", clockItems, clockINT, this);
	new Aitem("midi clock", "external midi clock", clockItems, clockMIDI, this);
	{
		char	nm[256];
		Apin	*p=table->pins;
		while(p)
		{
			if((p->type&pinDIR)==pinOUT)
			{
				if(p->isGUID(Asample::CI.guid))
				{
					sprintf(nm, "%s / %s", p->effect->name, p->name);
					new Aitem(nm, nm, clockItems, (dword)p);
				}
			}
			p=p->nextPin;
		}
	}
	clock->setCurrentByData(data);
	if(clock->getCurrentData()==-1)
		clock->setCurrentByData(clockINT);
	data=clock->getCurrentData();
	if(data==clockINT)
	{
		bpm->state|=stateENABLE;
		bpm->color=0xffffff00;
		bpm->repaint();
	}
	else
	{
		bpm->state&=~stateENABLE;
		bpm->color=0xffff8000;
		bpm->repaint();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Amonitor::Amonitor(char *name, Aobject *l, int x, int y, int w, int h) : Asurface(name, l, x, y, w, h)
{
	bitmap->set(0, 0, ((Apreview *)l)->mire, bitmapDEFAULT, bitmapDEFAULT);
	pin=NULL;
	timer(40);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Amonitor::~Amonitor()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Amonitor::setPin(Apin *p)
{
	pin=p;
	if(!p)
		bitmap->set(0, 0, ((Apreview *)father)->mire, bitmapDEFAULT, bitmapDEFAULT);
	repaint();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Amonitor::pulse()
{
	bool	ok=false;
	if(pin)
	{
		Apin	*pin=this->pin;
		if(pin==FOLLOW)
		{
			Atable	*table=((MYwindow *)getWindow())->table;
			pin=null;
			{
				int		nb=table->nbEffect;
				int		i;
				for(i=0; i<nb; i++)
				{
					Aeffect	*e=table->effect[i];
					if(e->selected)
					{
						Apin	*p=table->pins;
						int		nin=0;
						int		nout=0;
						while(p)
						{
							if(p->effect==e)
							{
								switch(p->type&pinDIR)
								{
									case pinOUT:
									if(!pin)
										pin=p;
									if(p->isCI(&Avideo::CI))
										nout++;
									break;
									
									case pinIN:
									if(p->isCI(&Avideo::CI))
										nin++;
									break;
								}
							}
							p=p->nextPin;
						}
						if(nin||nout)
						{
							int	nb=nin+nout;
							int	nmax=1;
							int	nc;
							int	i;
							for(i=1; i<5; i++)
							{
								nmax=i*i;
								if(nmax>=nb)
								{
									nc=i;
									break;
								}
							}
							bitmap->boxf(0, 0, bitmap->w, bitmap->h, 0xff000000);
							{
								Afont	*f=alib.getFont(fontTERMINAL06);
								Apin	*p=table->pins;
								int		n=0;
								int		nw=bitmap->w/nc;
								int		nh=bitmap->h/nc;
								int		maxc=(nw/f->w)-2;
								while(p)
								{
									if(p->effect==e)
									{
										if(p->isCI(&Avideo::CI))
										{
											Avideo	*in=(Avideo *)p;
											int		nr=nb-n-1;
											int		nx=(nr/nc)*nw;
											int		ny=(nr%nc)*nh;
											in->enter(__FILE__,__LINE__);
											{
												Abitmap	*bi=in->getBitmap();
												if(bi)
													bitmap->set(nx, ny, nw, nh, 0, 0, bi->w, bi->h, bi, bitmapNORMAL, bitmapNORMAL);
											}
											{
												char	str[1024];
												int		plus=maxi(strlen(p->name)-maxc, 0);
												strcpy(str, p->name);
												strupr(str);
												f->set(bitmap, nx+2, ny+2, &str[plus], 0xff00ff00, 0xff404040);
											}
											in->leave();
											n++;
										}
									}
									p=p->nextPin;
								}
							}
							{
								Afont	*f=alib.getFont(fontARMY10);
								int	ww=f->getWidth(e->name);
								f->set(bitmap, (bitmap->w-ww)>>1, bitmap->h-f->h-2, e->name, 0xffffff00, 0xff404040);
							}
							pin=null;
							ok=true;
							repaint();
							break;
						}
					}
					if(pin)
						break;
				}
			}
		}
		if(pin)
		{
			if(pin->isCI(&Asample::CI))
			{
				pin->enter(__FILE__,__LINE__);
				bitmap->boxfa(0, 0, bitmap->w-1, bitmap->h-1, 0xff000000, 0.2f);
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
				{
					int		h=(bitmap->h>>1);
					int		w=(bitmap->w>>1);
					Asample	*s=(Asample *)pin;
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
				pin->leave();
				ok=true;
				repaint();
			}
			else if(pin->isCI(&Avideo::CI))
			{
				pin->enter(__FILE__,__LINE__);
				{
					Avideo	*vid=(Avideo *)pin;
					bitmap->set(0, 0, bitmap->w, bitmap->h, 0, 0, vid->image->w, vid->image->h, vid->image, bitmapNORMAL, bitmapNORMAL);
				}
				pin->leave();
				ok=true;
				repaint();
			}
		}
		if(!ok)
		{
			bitmap->boxf(0, 0, bitmap->w, bitmap->h, 0xff000000);
			repaint();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
