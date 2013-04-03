/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	EXPORT.CPP					(c)	YoY'05						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<math.h>
#include						<assert.h>
#include						"resource.h"
#include						"export.h"
#include						"main.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"../sourceFilter/sharedmem.cpp"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								AexportInfo::CI		= ACI("AexportInfo",		GUID(0x11111112,0x00000720), &AeffectInfo::CI, 0, NULL);
ACI								Aexport::CI			= ACI("Aexport",			GUID(0x11111112,0x00000721), &Aeffect::CI, 0, NULL);
ACI								AexportFront::CI	= ACI("AexportFront",		GUID(0x11111112,0x00000722), &AeffectFront::CI, 0, NULL);
ACI								AexportBack::CI		= ACI("AexportBack",		GUID(0x11111112,0x00000723), &AeffectBack::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void convTimeFormat(char *str, float n)
{
	int	h=(int)(n/3600.f);
	n-=(float)(h*3600);
	int	m=(int)(n/60.f);
	n-=(float)(m*60);
	sprintf(str, "%2d:%2d:%4.1f", h, m, n);
	{
		char	*s=str;
		while(*s)
		{
			if(*s==32)
				*s='0';
			s++;
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

Aexport::Aexport(QIID qiid, char *name, AeffectInfo *info, Acapsule *capsule) : Aeffect(qiid, name, info, capsule)
{
	image=new Abitmap(320, 240);
	bitshare=null;
	notregistred=null;
	avi=null;
	vleft=0;
	vright=0;
	sharedmem=new Asharedmem();
	if(sharedmem->thefirst)
	{
		Abitmap	*b=new Abitmap(&resource.get(MAKEINTRESOURCE(PNG_EXPORT_AEST), "PNG"));
		if(sharedmem->data)
		{
			byte	*s=b->body8;
			byte	*d=sharedmem->data->body;
			int		n=320*240;
			int		i;
			for(i=0; i<n; i++)
			{
				*(d++)=*(s++);
				*(d++)=*(s++);
				*(d++)=*(s++);
				s++;
			}
		}
		delete(b);
	}
	{
		MYwindow	*w=(MYwindow *)getTable()->getWindow();
		Aregistry	*reg=w->regpwd;
		char		sname[256]="";
		char		spwd[256]="";

		reg->read("name", sname, sizeof(sname));
		reg->read("pwd", spwd, sizeof(spwd));

		//if(!VerifyPassword(sname, spwd))
		//	notregistred=new Abitmap(&resource.get(MAKEINTRESOURCE(PNG_EXPORT_AESTALPHA), "PNG"));
	}
	memset(filename, 0, sizeof(filename));
	strcpy(filename, "c:\\elek-export.avi");
	front=new AexportFront(qiid, "export front", this, 48);
	front->setTooltips("export module");
	back=new AexportBack(qiid, "export back", this, 48);
	back->setTooltips("export module");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aexport::~Aexport()
{
	delete(image);
	if(sharedmem)
	{
		delete(sharedmem);
		sharedmem=null;
	}
	if(bitshare)
	{
		delete(bitshare);
		bitshare=null;
	}
	if(notregistred)
		delete(notregistred);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aexport::load(class Afile *f)
{
	AexportFront	*front=(AexportFront *)this->front;
	{
		bool	b;
		f->read(&b, sizeof(b));
		if(b)
		{
			char	path[ALIB_PATH];
			char	abspath[ALIB_PATH];
			f->readString(path);
			if(f->absoluPath(abspath, path))
				strcpy(filename, abspath);
			else
				strcpy(filename, path);

		}
	}
	front->updateFilename();
	{
		bool b=false;
		f->read(&b, sizeof(b));
		front->compress->setChecked(b);
	}
	{
		bool b=false;
		f->read(&b, sizeof(b));
		front->flip->setChecked(b);
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aexport::save(class Afile *f)
{
	AexportFront	*front=(AexportFront *)this->front;
	if(filename[0])
	{
		char	relpath[ALIB_PATH];
		bool	b=true;
		f->write(&b, sizeof(b));
		if(f->relativePath(relpath, filename))
			f->writeString(relpath);
		else
			f->writeString(filename);
	}
	else
	{
		bool	b=false;
		f->write(&b, sizeof(b));
	}
	{
		bool b=front->compress->isChecked();
		f->write(&b, sizeof(b));
	}
	{
		bool b=front->flip->isChecked();
		f->write(&b, sizeof(b));
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aexport::loadPreset(class Afile *f)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aexport::savePreset(class Afile *f)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aexport::settings(bool emergency)
{
	image->size(getVideoWidth(), getVideoHeight());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aexport::nextFile()
{
	section.enter(__FILE__,__LINE__);
	if(avi)
	{
		delete(avi);
		avi=null;
	}
	{
		ULARGE_INTEGER	freeSpace;
		ULARGE_INTEGER	total;
		ULARGE_INTEGER	totalFree;
		char			dir[ALIB_PATH];
		strcpy(dir, filename);
		dir[3]=0;
		if(GetDiskFreeSpaceEx(dir, &freeSpace, &total, &totalFree))
		{
			if(freeSpace.QuadPart>1024*1024*10)
			{
				double t=m_time;	
				actionStart(0);
				m_time=t;			// dont init start recording time
			}
			else
				front->asyncNotify(this, nyALERT, (dword)"[EXPORT] error, not enought space on disk, record stopped");
		}
	}
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aexport::actionStart(double time)
{
	AexportFront	*front=(AexportFront *)this->front;
	section.enter(__FILE__,__LINE__);
	assert(avi==null);
	m_time=0;
	starttime=Athread::getTime();
	if(avi)
	{
		delete(avi);
		avi=null;
	}
	if(front->record->isChecked())
	{
		ULARGE_INTEGER	freeSpace;
		ULARGE_INTEGER	total;
		ULARGE_INTEGER	totalFree;
		char			dir[ALIB_PATH];
		strcpy(dir, filename);
		dir[3]=0;
		if(GetDiskFreeSpaceEx(dir, &freeSpace, &total, &totalFree))
		{
			if(freeSpace.QuadPart>((qword)1<<30))
				maxsize=(int)(((qword)1<<30)-(60*60*25*16));	// - index table size (1 hour)
			else
				maxsize=(int)(freeSpace.QuadPart-(60*60*25*16));	// - index table size (1 hour)
			if(maxsize>1024*1024*10)
			{
				char	fname[ALIB_PATH];
				int		nn=1;
				strcpy(fname, filename);

				while(Afilehd::exist(fname))
				{
					strcpy(fname, filename);
					{
						char	*s=strrchr(fname, '.');
						if(!s)
							break;
						sprintf(s, "-%4d.avi", nn++);
						while(*s)
						{
							if(*s==' ')
								*s='0';
							s++;
						}
					}
				}
	
				avi=new AaviWrite(fname, getVideoWidth(), getVideoHeight(), (float)front->fps->get(), front->compress->isChecked()); 

				if(!avi->isOK)
				{
					delete(avi);
					avi=null;
					front->asyncNotify(this, nyERROR, (dword)"[EXPORT] error, can't open file");
				}
			}
			else 
				front->asyncNotify(this, nyERROR, (dword)"[EXPORT] error, not enought space on disk");
		}
		else
		{
			front->asyncNotify(this, nyERROR, (dword)"[EXPORT] error, can't access disk");
		}
	}
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aexport::actionStop()
{
	AexportFront	*front=(AexportFront *)this->front;
	section.enter(__FILE__,__LINE__);
	if(avi)
	{
		delete(avi);
		avi=null;
	}
	section.leave();
	vleft=0.f;
	vright=0.f;
	front->asyncNotify(this, nyCHANGE, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aexport::action(double time, double dtime, double beat, double dbeat)
{
	AexportFront	*front=(AexportFront *)this->front;
	AexportBack		*back=(AexportBack *)this->back;
	Asample			*audio=back->audio;
	Avideo			*video=back->video;
	bool			flip=front->flip->isChecked();

	section.enter(__FILE__,__LINE__);
	//try
	{
		bool	ok=true;
		if(avi)
		{
			if(!front->record->isChecked())
			{
				actionStop();
			}
			else if(audio->getCountConnect())
			{
				audio->enter(__FILE__,__LINE__);
				{
					int nsmp=audio->avaible();
					int	need=(int)(44100.f/avi->fps);
					while(nsmp>need)
					{
						Abitmap	*b=image;
						Abitmap	*bi=video->getBitmap();

						{
							MYwindow	*w=(MYwindow *)getTable()->getWindow();
							Aregistry	*reg=w->regpwd;
							char		sname[256]="";
							char		spwd[256]="";

							reg->read("name", sname, sizeof(sname));
							reg->read("pwd", spwd, sizeof(spwd));

							//if(VerifyPassword(sname, spwd))
							{
								b=bi;
							}
							//else
							//{
							//	b->set(0, 0, bi, bitmapNORMAL, bitmapNORMAL);
							//	if(notregistred)
							//		b->set(0, 0, b->w, b->h, 0, 0, notregistred->w, notregistred->h, notregistred);
							//}
						}

						sword buf[10000][2];
						int na=audio->getBuffer((sword *)buf, need);

						{
							int	mL=0;
							int	mR=0;
							int	i;
							for(i=0; i<na; i++)
							{
								mL=maxi(mL, abs(buf[i][0]));
								mR=maxi(mR, abs(buf[i][1]));
							}
							vleft=(float)mL/32768.f;
							vright=(float)mR/32768.f;
						}

						avi->setSample(buf, na*sizeof(sword)*2);
						if(b)
						{
							avi->setBitmap(b);
						}
						if(!avi->writeFrame())
							front->asyncNotify(this, nyERROR, (dword)"[EXPORT] write error - record stopped");
						if(avi->file->offset>maxsize)
						{
							nextFile();
							break;
						}
						nsmp-=na;
					}
				}
				audio->leave();
			}
			else
			{
				Abitmap	*b=image;
				Abitmap	*bi=video->getBitmap();
				if(bi)
				{
					{
						MYwindow	*w=(MYwindow *)getTable()->getWindow();
						Aregistry	*reg=w->regpwd;
						char		sname[256]="";
						char		spwd[256]="";

						reg->read("name", sname, sizeof(sname));
						reg->read("pwd", spwd, sizeof(spwd));

						//if(VerifyPassword(sname, spwd))
						{
							b=bi;
						}
						//else
						//{
						//	b->set(0, 0, bi, bitmapNORMAL, bitmapNORMAL);
						//	b->set(0, 0, b->w, b->h, 0, 0, notregistred->w, notregistred->h, notregistred);
						//}
					}
				}
				{
					double	ft=1000.0/(double)avi->fps;
					m_time+=dtime;
					while(m_time>ft)
					{
						if(b)
							avi->setBitmap(b);
						if(!avi->writeFrame())
							front->asyncNotify(this, nyERROR, (dword)"[EXPORT] write error - record stopped");
						if(avi->file->offset>maxsize)
						{
							nextFile();
							break;
						}
						m_time-=ft;
					}
				}
				vleft=0;
				vright=0;
			}
		}
		else if(front->record->isChecked())
		{
			vleft=0;
			vright=0;
			if(audio->getCountConnect())
			{
				int	a=audio->avaible();
				if(a)
					audio->skip(a);
			}
			actionStart(time);
		}
		else
		{
			vleft=0;
			vright=0;
		}
	}
	/*
	catch(...)
	{
		// let AVI in memory if crashed
		avi=null;
		front->asyncNotify(this, nyERROR, (dword)"[EXPORT] major error - record stopped");
	}
	*/
	if(front->camera->isChecked())
	{
		Abitmap	*bi=video->getBitmap();
		if(bi&&sharedmem->data)
		{
			Abitmap	*b=bi;
			
			if((!((bi->w==320)&&(bi->h==240)))||flip)
			{
				if(!bitshare)
					bitshare=new Abitmap(320, 240);
				bitshare->set(0, 0, 320, 240, 0, 0, bi->w, bi->h, bi, bitmapNORMAL, bitmapNORMAL);
				b=bitshare;
			}

			if(notregistred)	// bitmap notregistred exist
			{
				MYwindow	*w=(MYwindow *)getTable()->getWindow();
				Aregistry	*reg=w->regpwd;
				char		sname[256]="";
				char		spwd[256]="";

				reg->read("name", sname, sizeof(sname));
				reg->read("pwd", spwd, sizeof(spwd));

				if(0/*!VerifyPassword(sname, spwd)*/)
				{
					if(b!=bitshare)
					{
						if(!bitshare)
							bitshare=new Abitmap(320, 240);
						bitshare->set(0, 0, bi, bitmapNORMAL, bitmapNORMAL);
						b=bitshare;
					}
					bitshare->set(0, 0, notregistred);
				}
			}

			if(flip)
				b->flipY();

			if(sharedmem->data->locked)
				Sleep(1);

			if(!sharedmem->data->locked)
			{
				sharedmem->data->locked=true;

				{
					byte	*s=b->body8;
					byte	*d=sharedmem->data->body;
					int		n=320*240;
					int		i;
					for(i=0; i<n; i++)
					{
						*(d++)=*(s++);
						*(d++)=*(s++);
						*(d++)=*(s++);
						s++;
					}
				}

				sharedmem->data->frame++;
				sharedmem->data->locked=false;
			}
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

AexportFront::AexportFront(QIID qiid, char *name, Aexport *e, int h) : AeffectFront(qiid, name, e, h)
{
	int		hb=(h-32)>>1;
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_EXPORT_FRONT), "PNG");
	back=new Abitmap(&o);
	
	left=new Adisplay("left vue meter", this, 367, hb, displayVM0002);
	left->setTooltips("left vue meter");
	left->show(TRUE);

	right=new Adisplay("right vue meter", this, 387, hb, displayVM0002);
	right->setTooltips("right vue meter");
	right->show(TRUE);

	m_vleft=0.f;
	m_vright=0.f;

	record=new Abutton("record", this, 454, ((h-16)>>1)-1, 16, 16, &resource.get(MAKEINTRESOURCE(PNG_BUTTONREC), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	record->setTooltips("record to disk");
	record->show(true);

	open=new Abutton("open", this, 480, (h-28)>>1, 19, 28, &resource.get(MAKEINTRESOURCE(PNG_EXPORT_OPEN), "PNG"));
	open->setTooltips("select file");
	open->show(TRUE);

	camera=new Abutton("camera", this, 300, 27, 19, 12, &resource.get(MAKEINTRESOURCE(PNG_EXPORTCAM), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	camera->setTooltips("export to \"elektronika source\" virtual capture driver");
	camera->show(TRUE);

	flip=new Abutton("flip", this, 326, 26, 9, 14, &resource.get(MAKEINTRESOURCE(PNG_EXPORTFLIP), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	flip->setTooltips("vertical flip");
	flip->show(TRUE);

	compress=new Abutton("compress", this, 348, 25, 18, 18, &resource.get(MAKEINTRESOURCE(PNG_EXPORTCOMPPRESS), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	compress->setTooltips("use compressed format (only to virtual dub)");
	compress->show(TRUE);

	fps=new Asegment("fps", this, 64, 25, 2, 5, 30, alib.getFont(fontSEGMENT10), 0xffCFD3D3, 0.5f);
	fps->setTooltips("fps");
	fps->set(25);
	fps->show(true);

	filename=new Astatic("filename", this, 12, 11, 360, 10);
	updateFilename();
	filename->setTooltips("export file");
	filename->show(true);

	convTimeFormat(ttime, 0.f);

	timer(40);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AexportFront::~AexportFront()
{
	delete(back);
	delete(left);
	delete(right);
	delete(record);
	delete(open);
	delete(camera);
	delete(compress);
	delete(flip);
	delete(fps);
	delete(filename);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AexportFront::paint(Abitmap *b)
{
	b->set(0, 0, back);

	{
		Aexport	*si=(Aexport *)effect;
		Afont	*fs=alib.getFont(fontSEGMENT10);
		if(record->isChecked())
		{
			double	time=Athread::getTime()-si->starttime;
			if(!si->getTable()->isRunning())
				time=0.f;
			convTimeFormat(ttime, (float)(time/1000.0));
		}
		fs->setFixed(b, 180, 25, ttime, 0xffCFD3D3);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AexportFront::pulse()
{
	Aexport	*si=(Aexport *)effect;
	m_vleft=(m_vleft+si->vleft)*0.5f;
	m_vright=(m_vright+si->vright)*0.5f;
	left->set(m_vleft);
	right->set(m_vright);
	{
		Arectangle	r=Arectangle(180, 25, 100, 13);
		repaint(r);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AexportFront::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCLICK:
		if(o==open)
		{
			Aexport	*si=(Aexport *)effect;
			AfileDlg	fdlg=AfileDlg("saveDlg", this, "movie file AVI\0*.avi\0", si->filename, AfileDlg::SAVE);
			if(fdlg.getFile(si->filename))
			{
				updateFilename();
				record->setChecked(false);
			}
		}
		return true;

		case nyCHANGE:
		if(o==fps)
			record->setChecked(false);
		else if(o==effect)
			record->setChecked(false);
		return true;

		case nyALERT:
		case nyERROR:
		if(o==effect)
			record->setChecked(false);
		break;

	}
	return AeffectFront::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AexportFront::updateFilename()
{
	Aexport	*si=(Aexport *)effect;
	char	str[1024];
	strcpy(str, si->filename);
	strupr(str);
	filename->set(str, 0xffCFD3D3, alib.getFont(fontTERMINAL06), Astatic::LEFT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AexportBack::AexportBack(QIID qiid, char *name, Aexport *e, int h) : AeffectBack(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_EXPORT_BACK), "PNG");
	back=new Abitmap(&o);

	video=new Avideo(MKQIID(qiid,0x184a84a54d8e8d94), "video in", this, pinIN, 10, 10);
	video->setTooltips("video in");
	video->show(TRUE);
	
	audio=new Asample(MKQIID(qiid,0xd845d8e45c5ab03a), "audio in", this, pinIN, 10, 30);
	audio->setTooltips("audio in");
	audio->show(TRUE);
}

///////////////////////////////////////////////////////////////lo//////////////////////////////////////////////////////////

AexportBack::~AexportBack()
{
	delete(back);
	delete(video);
	delete(audio);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AexportBack::paint(Abitmap *b)
{
	b->set(0, 0, back);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aeffect * AexportInfo::create(QIID qiid, char *name, Acapsule *capsule)
{
	return new Aexport(qiid, name, this, capsule);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aplugz * exportGetInfo()
{
	return new AexportInfo("exportInfo", &Aexport::CI, "export", "export module");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
