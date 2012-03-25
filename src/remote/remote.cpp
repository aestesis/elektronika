/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// remote.cpp : Defines the entry point for the application.
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include						<assert.h>
#include						<windows.h>
#include						<libxml/tree.h>
#include						<string.h>
#include						"remote.h"
#include						"interface.h"
#include						"elektro.h"
#include						"audiovideo.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef RESELLER
#define							REGISTRYPATH2		"SOFTWARE\\sisetsea\\elektro20"
#else
#define							REGISTRYPATH2		"SOFTWARE\\sisetsea\\resell20"
#endif

static qword					GUIDPUB=GUID(0x11111112,0x00000421);

//

typedef struct
{
	char						name[1024];
	char						snap[1024];
	int							neffect;
	struct
	{
		char					name[1024];
		char					snap[1024];
		int						program[10];
		int						nprogram;
	}							effect[9];
} Ttype;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
	char						port[256];
	int							baud;
	int							bits;
	int							stop;
	int							parity;
} RS232;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
	int			x;
	int			y;
	float		a;
} Tpos;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Tpos	bpos[6]=
{
	{ 0, 0, 0.5f },
	{ 42, 0, 0.5f },
	{ 86, 2, 1.f },
	{ 2, 34, 1.f },
	{ 44, 34, 1.f },
	{ 84, 32, 0.5f }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool abspath(char *apath, char *path, char *root)
{
	char	root_drive[ALIB_PATH];
	char	root_dir[ALIB_PATH];
	char	root_file[ALIB_PATH];
	char	root_ext[ALIB_PATH];
	char	path_drive[ALIB_PATH];
	char	path_dir[ALIB_PATH];
	char	path_file[ALIB_PATH];
	char	path_ext[ALIB_PATH];
	
	strlwr(root);
	strlwr(path);
	_splitpath(root, root_drive, root_dir, root_file, root_ext);
	_splitpath(path, path_drive, path_dir, path_file, path_ext);
	
	if(path_drive[0])
		strcpy(apath, path);
	else
	{
		strcpy(apath, root_drive);
		strcat(apath, root_dir);
		strcat(apath, path);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MYwindow : public Awindow, public Athread
{
	AOBJ

	enum
	{
								NONE=0,
								CLIENT,
								SERVER
	};

								MYwindow						(char *name, int x, int y, int w, int h);
								~MYwindow						();

	virtual void				paint							(Abitmap *b);
	virtual bool				notify							(Anode *o, int event, dword p=0);
	virtual bool				mouse							(int x, int y, int state, int event);
//	virtual bool				keyboard						(int event, int ascii, int scan, int state);
	bool						dragdrop						(int x, int y, int state, int event, void *data=NULL);

	void						pulse							();
	// mouse move 

	void						run								();
	void						change							(int mode, int port);
	
	void						parseXML						();
	void						makeButtons						();
	void						typeChange						();
	void						effectChange					();

	void						drop							(char *s);
	void						send							();
		
	void						midiStart						();
	void						midiStop						();
	
	void						midi							(byte status, byte p0, byte p1);
	void						midiSysex						(qword guid, Abuffer *b);
	
	
	int							bac,lx,ly,wx,wy;
	Abitmap						*back;
	
	Ttype						types[9];
	int							ntypes;
	
	RS232						rs232;
	Aserial						*serial;
	
	Apipe						*pipe;
	
	Abutton						*btype[9];
	Abutton						*beffect[9];
	
	int							curType;
	int							curEffect;

	Abutton						*close;
	
	Abutton						*start;
	Abutton						*stop;
	Abutton						*showEKA;
	
	Abitmap						*snap;
	Astatic						*filename;
	char						fileImage[ALIB_PATH];
	Aedit						*text;
	Abutton						*dispText;
	Abutton						*dispImage;
	Abutton						*loadImage;
	AselButton					*textPos;
	
	Asegment					*rmtype;
	Asegment					*rstype;
	Asegment					*rmeffect;
	Asegment					*rseffect;
	
	Abutton						*randType;
	Abutton						*randEffect;
	bool						brType;
	bool						brEffect;
	int							timeType;
	int							timeEffect;
	int							lenType;
	int							lenEffect;
	
	char						hostname[ALIB_PATH];
	Apipe						*viewpipe;
	Abuffer						*audio;
	Abitmap						*image;
	Asection					simage;
	bool						bbout;
	bool						bbrun;
	
	bool						brun;
	bool						showELEK;
	char						curdir[ALIB_PATH];
	char						eka[ALIB_PATH];
	
	int							time;	// beta
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								MYwindow::CI=ACI("MYwindow", GUID(0x11181111,0x00000001), &Awindow::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MYwindow::MYwindow(char *name, int x, int y, int w, int h) : Awindow(name, x, y, w, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_INTERFACE), "PNG");
	back=new Abitmap(&o);
	pipe=NULL;
	serial=NULL;
	showELEK=false;
	
	memset(eka, 0, sizeof(eka));
	GetCurrentDirectory(MAX_PATH, curdir);
	strcat(curdir, "\\");

	time=GetTickCount();	// beta
	
	{
		WSADATA wd;
		if(WSAStartup(MAKEWORD(2, 2), &wd)==0)
		{
			memset(hostname, 0, sizeof(hostname));
			gethostname(hostname, 128);
		}
	}

	bbrun=false;
	bbout=true;
	viewpipe=NULL;
	
	
	audio=new Abuffer("audio", 44100*4);
	image=new Abitmap(32, 24);
	
	//

	brun=false;

	memset(fileImage, 0, sizeof(fileImage));	
	brType=false;
	brEffect=false;
	timeType=0;
	timeEffect=0;
	lenType=10*60*1000;
	lenEffect=60*1000;
	
	snap=new Abitmap(100, 75);

	parseXML();
	
	{	
		HKEY	hk;
		if(RegOpenKey(HKEY_LOCAL_MACHINE, REGISTRYPATH2, &hk)==ERROR_SUCCESS)
		{
			char	curpath[ALIB_PATH];
			char	elekpath[ALIB_PATH];
			char	apath[ALIB_PATH];
			DWORD	t=REG_SZ;
			DWORD	s=MAX_PATH;
			RegQueryValueEx(hk, "", 0, &t, (byte *)elekpath, &s);
			RegCloseKey(hk);
			
			GetCurrentDirectory(sizeof(curpath), curpath);
			SetCurrentDirectory(elekpath);
			apath[0]=0;
			
			if(eka[0])
				abspath(apath, eka, curdir);
			
			strcat(elekpath, "\\elektronika.exe /showoff");
			if(apath[0])
			{
				strcat(elekpath, " \"");
				strcat(elekpath, apath);
				strcat(elekpath, "\"");
			}
			exec(elekpath);
			SetCurrentDirectory(curpath);
		}
	}
	
	
	makeButtons();

	start=new Abutton("start", this, 62, 326, 105, 46, &resource.get(MAKEINTRESOURCE(PNG_START), "PNG"), Abutton::btBITMAP|Abutton::bt2STATES);
	start->show(true);

	stop=new Abutton("stop", this, 62, 403, 105, 46, &resource.get(MAKEINTRESOURCE(PNG_STOP), "PNG"));
	stop->show(true);
	
	showEKA=new Abutton("stop", this, 62, 379, 105, 17, &resource.get(MAKEINTRESOURCE(PNG_SHOW), "PNG"), Abutton::btBITMAP|Abutton::bt2STATES);
	if(showELEK)
		showEKA->show(true);
	
	rmtype=new Asegment("rmtype", this, 366, 232, 2, 0, 59, alib.getFont(fontSEGMENT20), 0xffffffff, 0.3f);
	rmtype->set(10);
	rmtype->show(true);

	rstype=new Asegment("rstype", this, 416, 232, 2, 0, 59, alib.getFont(fontSEGMENT20), 0xffffffff, 0.3f);
	rstype->show(true);

	rmeffect=new Asegment("rmeffect", this, 600, 232, 2, 0, 59, alib.getFont(fontSEGMENT20), 0xffffffff, 0.3f);
	rmeffect->set(1);
	rmeffect->show(true);

	rseffect=new Asegment("rseffect", this, 650, 232, 2, 0, 59, alib.getFont(fontSEGMENT20), 0xffffffff, 0.3f);
	rseffect->show(true);

	randType=new Abutton("randType", this, 266, 236, 81, 23, &resource.get(MAKEINTRESOURCE(PNG_RANDOM), "PNG"), Abutton::btBITMAP|Abutton::bt2STATES);
	randType->show(true);

	randEffect=new Abutton("randEffect", this, 496, 236, 81, 23, &resource.get(MAKEINTRESOURCE(PNG_RANDOM), "PNG"), Abutton::btBITMAP|Abutton::bt2STATES);
	randEffect->show(true);

	close=new Abutton("close", this, 210, 1, 16, 16, &resource.get(MAKEINTRESOURCE(PNG_CLOSE), "PNG"));
	close->show(true);
	
	filename=new Astatic("filename", this, 290, 425, 140, 12);
	filename->show(true);
	
	text=new Aedit("text", this, 503, 323, 179, 105);
	text->colorBackground=0x00000000;
	text->colorBorder=0x00000000;
	text->state|=Aedit::stateMULTILINE;
	text->show(true);

	dispImage=new Abutton("dispImage", this, 322, 440, 81, 23, &resource.get(MAKEINTRESOURCE(PNG_DISPLAY), "PNG"), Abutton::btBITMAP|Abutton::bt2STATES);
	dispImage->show(true);

	dispText=new Abutton("randEffect", this, 522, 440, 81, 23, &resource.get(MAKEINTRESOURCE(PNG_DISPLAY), "PNG"), Abutton::btBITMAP|Abutton::bt2STATES);
	dispText->show(true);

	loadImage=new Abutton("loadImage", this, 438, 412, 27, 34, &resource.get(MAKEINTRESOURCE(PNG_LOAD), "PNG"), Abutton::btBITMAP);
	loadImage->show(true);

	textPos=new AselButton("textPos", this, 620, 446, 6, 1, &resource.get(MAKEINTRESOURCE(PNG_TEXTPOS), "PNG"), 8, 8);
	textPos->setTooltips("text position");
	textPos->show(true);

	pipe=new Apipe("\\\\localhost\\\\pipe\\elekMidiPipe", Apipe::CLIENT|Apipe::WRITE);
	if(!pipe->isOK())
	{
		delete(pipe);
		pipe=NULL;
	}
	
	if(rs232.port[0])
	{
		serial=new Aserial(rs232.port);
		if(serial->isOK())
		{
			int	st=(rs232.stop==2)?2:0;
			serial->configure(rs232.baud, rs232.bits, rs232.parity, st);
			serial->timeouts(50, 50, 50, 0, 0);
		}
		else
		{
			delete(serial);
			serial=NULL;
		}
	}
	
	timer(100);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MYwindow::~MYwindow()
{
	int	i;
	delete(back);
	
	change(NONE, 0);
	
	if(serial)
		delete(serial);
	
	if(pipe)
		delete(pipe);
	
	if(viewpipe)
		delete(viewpipe);
		
	for(i=0; i<9; i++)
	{
		delete(btype[i]);
		delete(beffect[i]);
	}
	
	delete(start);
	delete(stop);
	delete(rmtype);
	delete(rstype);
	delete(rmeffect);
	delete(rseffect);
	delete(randType);
	delete(randEffect);
	delete(close);
	delete(snap);
	delete(filename);
	delete(text);
	delete(dispText);
	delete(dispImage);
	delete(loadImage);
	delete(audio);
	delete(image);
	delete(showEKA);
	delete(textPos);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MYwindow::paint(Abitmap *b)
{
	Afont	*font=alib.getFont(fontTERMINAL06);
	b->set(0, 0, back, bitmapNORMAL, bitmapNORMAL);
	if(pipe)
		font->set(b, 8, pos.h-16, "CONNECTED TO ELEKTRONIKA SERVER", 0xff202020);
	else
		font->set(b, 8, pos.h-16, "NOT CONNECTED TO ELEKTRONIKA SERVER", 0xffff0000);
	if(rs232.port[0])
	{
		if(serial)
			font->set(b, 8, pos.h-24, "SERIAL COMMUNICATION OK", 0xff202020);
		else
			font->set(b, 8, pos.h-24, "SERIAL COMMUNICATION ERROR", 0xffff0000);
	}
	b->set(312, 323, snap, bitmapNORMAL, bitmapNORMAL);
	simage.enter(__FILE__,__LINE__);
	b->set(52, 77, 126, 96, 0, 0, image->w, image->h, image, bitmapDEFAULT, bitmapDEFAULT);
	b->boxf(52, 240, 52+126, 240+36, 0xff000000);
	if(audio->getSize()>(126*4))
	{
		int		size=(int)audio->getSize();
		short	snd[126*2];
		int		i;
		audio->seek(size-126*4);
		audio->read(snd, 126*4);
		for(i=1; i<126; i++)
		{
			int	n0=(i-1)<<1;
			int	n=i<<1;
			int	x0=i+51;
			int	x=i+52;
			b->line(x0, 258+(((int)snd[n0]*36)>>16), x, 258+(((int)snd[n]*36)>>16), 0xffffff00);
			b->line(x0, 258+(((int)snd[n0+1]*36)>>16), x, 258+(((int)snd[n+1]*36)>>16), 0xffff0000);
		}
	}
	simage.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MYwindow::mouse(int x, int y, int state, int event)
{
	switch(event)
	{
		case mouseLDOWN:
		{
			cursor(cursorSIZEALL);
			wx=pos.x;
			wy=pos.y;
			lx=pos.x+x;
			ly=pos.y+y;
			bac=1;
		}
		mouseCapture(TRUE);
		return TRUE;

		case mouseNORMAL:
		if((state&mouseL)&&bac)
			move(wx+(x+pos.x)-lx, wy+(y+pos.y)-ly);
		return TRUE;
		
		case mouseLUP:
		bac=0;
		mouseCapture(FALSE);
		return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MYwindow::notify(Anode *o, int event, dword p)
{
	int	i;
	switch(event)
	{
		case nyCHANGE:
		if((o==dispText)||(o==dispImage))
			send();
		else if((o==rmtype)||(o==rstype))
		{
			lenType=(rmtype->get()*60+rstype->get())*1000;
		}
		else if((o==rmeffect)||(o==rseffect))
		{
			lenEffect=(rmeffect->get()*60+rseffect->get())*1000;
		}
		else if(o==randType)
		{
			brType=randType->isChecked();
			if(brType)
				timeType=GetTickCount();
		}
		else if(o==randEffect)
		{
			brEffect=randEffect->isChecked();
			if(brEffect)
				timeEffect=GetTickCount();
		}
		else if(o==start)
		{
			if(start->isChecked())
			{
				if(pipe)
				{
					midiStart();
					effectChange();
				}
				else
					start->setChecked(false);
			}
			else
				midiStop();
			return true;
		}
		else if(o==showEKA)
		{
			bool	b=showEKA->isChecked();
			if(pipe)
			{
				dword		mg=magicELEKSHOW;
				pipe->write(&mg, sizeof(mg));
				pipe->write(&b, sizeof(b));
			}
			return true;
		}
		else
		{
			for(i=0; i<9; i++)
			{
				if(o==btype[i])
				{
					if(i==curType)
						btype[i]->setChecked(true);
					else
					{
						curType=i;
						typeChange();
					}
					return true;
				}
				else if(o==beffect[i])
				{
					if(i==curEffect)
						beffect[i]->setChecked(true);
					else
					{
						curEffect=i;
						effectChange();
					}
					return true;
				}
			}
		}
		break;
		
		case nyCLICK:
		if(o==stop)
		{
			start->setChecked(false);
			midiStop();
			return true;
		}
		else if(o==close)
		{
			if(pipe)
			{
				dword		mg=magicELEKOFF;
				pipe->write(&mg, sizeof(mg));
			}
			Aexit(0);
		}
		else if(o==loadImage)
		{
			char		filename[1024];
			AfileDlg	fdlg=AfileDlg("loadDlg", this, "all pictures files\0*.png;*.jpg;*.jpeg;\0picture file PNG\0*.png\0picture file JPEG\0*.jpg;*.jpeg\0", NULL, AfileDlg::LOAD);
			if(fdlg.getFile(filename))
				drop(filename);
		}
		break;
	};
	return Awindow::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MYwindow::dragdrop(int x, int y, int state, int event, void *data)
{
	switch(event)
	{
		case dragdropENTER:
		case dragdropOVER:
		if(state&dragdropFILE)	
			return true;
		break;
		
		case dragdropDROP:
		if(state&dragdropFILE)
		{
			char	*s=(char *)data;
			while(*s)
			{
				drop(s);
				s+=strlen(s)+1;
			}
			return true;
		}
		break;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MYwindow::pulse()
{
	int	time=GetTickCount();
	if(!pipe)
	{
		pipe=new Apipe("\\\\.\\pipe\\elekMidiPipe", Apipe::CLIENT|Apipe::WRITE);
		if(!pipe->isOK())
		{
			delete(pipe);
			pipe=NULL;
		}
		else
			repaint();
	}
	if(brun&&brType)
	{
		if(time-(timeType+lenType)>0)
		{
			int	n=rand()%ntypes;
			timeType=time;
			if(n!=curType)
			{
				curType=n;
				typeChange();
			}
		}
	}
	if(brun&&brEffect)
	{
		if(time-(timeEffect+lenEffect)>0)
		{
			int	n=rand()%types[curType].neffect;
			timeEffect=time;
			if(n!=curEffect)
			{
				curEffect=n;
				effectChange();
			}
		}
	}
	if(serial)
	{
		char	mes;
		if(serial->read(&mes, sizeof(mes)))
		{
			switch(mes)
			{
				case 's':
				case 'S':
				{
					char	data[3];
					if(serial->read(&data, 2)==2)
					{
						int	s=0;
						data[2]=0;
						curType=atoi(data);
						typeChange();
					}
				}
				break;

				case 'e':
				case 'E':
				{
					char	data[3];
					if(serial->read(&data, 2)==2)
					{
						int	s=0;
						data[2]=0;
						curEffect=atoi(data);
						effectChange();
					}
				}
				break;

				case 'd':
				case 'D':
				{
					char	tp;
					if(serial->read(&tp, sizeof(tp)))
					{
						char	b;
						if(serial->read(&b, sizeof(b)))
						{
							switch(tp)
							{
								case 't':
								case 'T':
								dispText->setChecked((b=='1')?true:false);
								break;
								
								case 'p':
								case 'P':
								dispImage->setChecked((b=='1')?true:false);
								break;
							}
							send();
						}
					}
				}
				break;
				
				case 'g':
				case 'G':
				{
					char	tp;
					if(serial->read(&tp, sizeof(tp)))
					{
						switch(tp)
						{
							case 'o':
							case 'O':
							{
								char	b;
								if(serial->read(&b, sizeof(b)))
								{
									switch(b)
									{
										case '1':
										start->setChecked(true);
										if(pipe)
										{
											midiStart();
											effectChange();
										}
										else
											start->setChecked(false);
										break;
										
										case '0':
										start->setChecked(false);
										midiStop();
										break;
									}
								}
							}
							break;
						}
					}
				}				
				break;
				
				case 't':
				case 'T':
				{
					char	tp;
					if(serial->read(&tp, sizeof(tp)))
					{
						switch(tp)
						{
							case 't':
							case 'T':
							{
								char	b;
								if(serial->read(&b, sizeof(b)))
								{
									int	n=b-'0';
									if((n>=0)&&(n<6))
									{
										textPos->set(n);
										send();
									}
								}
							}
							break;
						}
					}
				}				
				break;
			}
		}
	}
	if(!viewpipe)
		change(CLIENT, 1);
	else
	{
		Arectangle	r;
		r.x=52;
		r.y=77;
		r.w=126;
		r.h=194;
		repaint(r);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MYwindow::drop(char *s)
{
	int			l=(int)strlen(s);
	char		*st=s+l-4;
	strlwr(s);
	if(l>5)
	{
		if(!strcmp(st, ".txt"))
		{
			FILE	*fic=fopen(s, "r");
			if(fic)
			{
				int		lf=0;
				char	c;
				while(fread(&c, 1, 1, fic))
					lf++;
				fseek(fic, 0, SEEK_SET);
				char *tt=(char *)malloc(lf+10);
				fread(tt, lf, 1, fic);
				text->set(tt);
				free(tt);
				fclose(fic);
				text->repaint();
			}
		}
		else if((!strcmp(st, ".png"))||(!strcmp(st, ".jpg"))||(!strcmp(st-1, ".jpeg")))
		{
			Abitmap	*image=NULL;
			image=new Abitmap(s);
			snap->set(0, 0, snap->w, snap->h, 0, 0, image->w, image->h, image, bitmapNORMAL, bitmapNORMAL);
			strcpy(fileImage, s);
			filename->set(s, 0xff202020, alib.getFont(fontTERMINAL06), Astatic::LEFT);
			delete(image);
			repaint();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MYwindow::send()
{
	bool	dtext=dispText->isChecked();
	bool	dtype=false;
	bool	dimage=dispImage->isChecked();
	int		v=(dimage?1:0)|(dtext?2:0)|(dtype?4:0);
	
	if(dtext)
	{
		Abuffer	*b=new Abuffer("sysex", 65536*1024);
		char	*s=text->caption;
		dword	mode=0x01020304;
		
		b->write(&mode, sizeof(mode));
		b->writeString(s);
		
		midiSysex(GUIDPUB, b);
		delete(b);
		
	}
	if(dimage)
	{
		Abuffer	*b=new Abuffer("sysex", 65536);
		dword	mode=0x02030405;
		
		b->write(&mode, sizeof(mode));
		b->writeString(fileImage);
		
		midiSysex(GUIDPUB, b);
		delete(b);
	}
	
	{
		Abuffer	*b=new Abuffer("sysex", 65536); 
		dword	mode=0x05010302	;
		int		p=textPos->get();
		
		b->write(&mode, sizeof(mode));
		b->write(&p, sizeof(p));
		b->write(&v, sizeof(v));
		
		midiSysex(GUIDPUB, b);
		delete(b);
	}
/*
	{
		Abuffer	*b=new Abuffer("sysex", 65536); 
		dword	mode=0x05040302;
		int		v=textPos->get();
		
		b->write(&mode, sizeof(mode));
		b->write(&v, sizeof(v));
		
		midiSysex(GUIDPUB, b);
		delete(b);
	}
	{
		Abuffer	*b=new Abuffer("sysex", 65536); 
		dword	mode=0x03040506;
		
		b->write(&mode, sizeof(mode));
		b->write(&v, sizeof(v));
		
		midiSysex(GUIDPUB, b);
		delete(b);
	}
*/
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool parseStr(xmlDocPtr doc, xmlNodePtr cur, char *name, char *str)
{
	if(!strcmp((char *)cur->name, name))
	{
		xmlChar	*key;
		key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		if(key)
		{
			if(strlen((char *)key)<1023)
				strcpy(str, (char *)key);
			xmlFree(key);
			return true;
		}
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool parseInt(xmlDocPtr doc, xmlNodePtr cur, char *name, int *i)
{
	if(!strcmp((char *)cur->name, name))
	{
		xmlChar	*key;
		key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		if(key)
		{
			*i=atoi((char *)key);
			xmlFree(key);
			return true;
		}
	}
	return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MYwindow::parseXML()
{
	xmlDocPtr	doc;
	
	memset(types, 0, sizeof(types));
	ntypes=0;

	memset(&rs232, 0, sizeof(rs232));

	doc = xmlParseFile("init.xml");
	if(doc)
	{
		xmlNodePtr	cur;
		cur = xmlDocGetRootElement(doc);
		
		while(cur)
		{
			if(!strcmp((char *)cur->name, "remoteinit"))
			{
				xmlNodePtr	c=cur->xmlChildrenNode;
				while(c)
				{
					if(!strcmp((char *)c->name, "type"))
					{
						if(ntypes<9)
						{
							xmlNodePtr	c0=c->xmlChildrenNode;
							while(c0)
							{
								parseStr(doc, c0, "name", types[ntypes].name);
								parseStr(doc, c0, "snap", types[ntypes].snap);
								if(!strcmp((char *)c0->name, "effect"))
								{
									xmlNodePtr	c1=c0->xmlChildrenNode;
									if(types[ntypes].neffect<9)
									{
										types[ntypes].effect[types[ntypes].neffect].nprogram=0;
										while(c1)
										{
											parseStr(doc, c1, "name", types[ntypes].effect[types[ntypes].neffect].name);
											parseStr(doc, c1, "snap", types[ntypes].effect[types[ntypes].neffect].snap);
											if(types[ntypes].effect[types[ntypes].neffect].nprogram<10)
											{
												if(parseInt(doc, c1, "program", &types[ntypes].effect[types[ntypes].neffect].program[types[ntypes].effect[types[ntypes].neffect].nprogram]))
													types[ntypes].effect[types[ntypes].neffect].nprogram++;
											}
											c1=c1->next;
										}
										types[ntypes].neffect++;
									}
								}
								c0=c0->next;
							}
							ntypes++;
						}
					}
					else if(!strcmp((char *)c->name, "rs232"))
					{
						xmlNodePtr	c0=c->xmlChildrenNode;
						while(c0)
						{
							parseStr(doc, c0, "port", rs232.port);
							parseInt(doc, c0, "baud", &rs232.baud);
							parseInt(doc, c0, "bits", &rs232.bits);
							parseInt(doc, c0, "stop", &rs232.stop);
							parseInt(doc, c0, "parity", &rs232.parity);
							c0=c0->next;
						}
					}
					else
					{
						char	master[1024];
						if(parseStr(doc, c, "master", master))
						{
							strlwr(master);
							if(strstr(master, "extend"))
								showELEK=true;
						} 
						parseStr(doc, c, "file", eka);
					}
					c=c->next;
				}
			}
			cur=cur->next;
		}
		xmlFreeDoc(doc);
	}
	else
	{
		MessageBox(null, "problem with init.xml, not found or error in the XML format", "remote XML error", MB_OK);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MYwindow::midiStart()
{
	brun=true;
	midi(0xfa, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MYwindow::midiStop()
{
	brun=false;
	midi(0xfc, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MYwindow::midi(byte status, byte p0, byte p1)
{
	if(pipe)
	{
		MidiPacket	mp;
		dword		magic=magicMIDIPACKET;
	
		if(pipe->write(&magic, sizeof(magic))!=sizeof(magic))
		{
			delete(pipe);
			pipe=NULL;
			return;
		}

		mp.channel=0;
		mp.status=status;
		mp.p0=p0;
		mp.p1=p1;
	
		if(pipe->write(&mp, sizeof(mp))!=sizeof(mp))
		{
			delete(pipe);
			pipe=NULL;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MYwindow::midiSysex(qword guid, Abuffer *b)
{
	if(pipe)
	{
		dword		mg=magicMIDISYSEX;
		dword		size=(int)b->size+sizeof(guid);
	
		if(pipe->write(&mg, sizeof(mg))!=sizeof(mg))
		{
			delete(pipe);
			pipe=NULL;
			return;
		}

		if(pipe->write(&size, sizeof(size))!=sizeof(size))
		{
			delete(pipe);
			pipe=NULL;
			return;
		}

		if(pipe->write(&guid, sizeof(guid))!=sizeof(guid))
		{
			delete(pipe);
			pipe=NULL;
			return;
		}

		b->seek(0);
		if(pipe->write(b->buffer, (int)b->size)!=(int)b->size)
		{
			delete(pipe);
			pipe=NULL;
			return;
		}
		
		pipe->flush();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MYwindow::change(int mode, int port)
{
	bbrun=false;
	while(!bbout)
		sleep(10);
	Athread::stop();
	if(mode!=NONE)
		Athread::start();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MYwindow::run()
{
	bbout=false;
	bbrun=true;

	{
		char	name[256];
		aePipeName(name, 1, hostname);
		viewpipe=new Apipe(name, Apipe::READ|Apipe::CLIENT);
	}

	if(viewpipe&&viewpipe->isOK())
	{
		int		frame=0;
		bool	bmagic=false;
		
		viewpipe->timeout=1000;
		
		while(bbrun)
		{
			while(bbrun)
			{
				TAEAV_CHUNK	chunk;
				qword		magic;

				if(bmagic||((viewpipe->read(&magic, sizeof(magic))==sizeof(magic))&&(magic==AEAV_MAGIC_CHUNK)))
				{
					bmagic=false;
					if(viewpipe->read(&chunk, sizeof(chunk))==sizeof(TAEAV_CHUNK))
					{
						switch(chunk.magic)
						{
							case AEAV_MAGIC_ENDSTREAM:
							brun=false;
							break;

							case AEAV_MAGIC_PING:
							break;

							case AEAV_MAGIC_FRAME:
							{
								TAEAV_FRAME	f;
								if(viewpipe->read(&f, sizeof(f))==sizeof(f))
									frame=f.frame;	// 4 nothing 	// todo: error catch
							}
							break;

							case AEAV_MAGIC_AUDIOPCM:
							{
								TAEAV_AUDIOPCM	a;
								if(viewpipe->read(&a, sizeof(a))==sizeof(a))
								{
									byte	smp[16384];
									int		size=(a.nbsamples*a.nbchannel*a.nbbits)>>3;
									while(size)
									{
										int	tr=mini(size, sizeof(smp));
										if(viewpipe->read(smp, tr)!=tr)
										{
											viewpipe->timeout=10;
											bbrun=false;
											break;											
										}
										simage.enter(__FILE__,__LINE__);
										if(!audio->write(smp, tr))
										{
											audio->clear();
											audio->write(smp, tr);
										}
										simage.leave();
										size-=tr;
									}
								}
							}
							break;

							case AEAV_MAGIC_VIDEOJPEG:
							simage.enter(__FILE__,__LINE__);
							{
								Abuffer		*fm=new Abuffer("fm", chunk.size+1024);
								int			z,a=0,v,o=(int)viewpipe->offset;
								if(viewpipe->read(fm->buffer, chunk.size)==chunk.size)
								{
									fm->size=chunk.size+512;
									image->load(fm, bitmapJPG);
									
									v=(int)viewpipe->offset;
									a=(int)viewpipe->offset-o;
									z=chunk.size-a;
								}
								delete(fm);
							}
							simage.leave();
							break;

							default:
							//assert(false);
							break;
						}
					}
				}
				else
				{
					bool	r=true;
					int		ndbg=0;
					while(r)
					{
						int		n=0;
						int		i;
						byte	b;
						qword	magic=AEAV_MAGIC_CHUNK;
						for(i=0; i<8; i++)
						{
							if(viewpipe->read(&b, sizeof(b))==sizeof(b))
							{
								byte	v=(byte)(magic>>n);
								ndbg++;
								if(b==v)
									n+=8;
								else
									break;
							}
							else
							{
								viewpipe->timeout=10;
								r=false;
								bbrun=false;
								break;											
							}
						}
						if(i==8)
						{
							bmagic=true;
							break;
						}
					}
				}
			}
			sleep(30);
		}
	}
	else
		notify(this, nyERROR, (dword)(char *)"can't open network connection");

	if(viewpipe)
	{
		delete(viewpipe);
		viewpipe=NULL;
	}
	
	bbrun=false;
	bbout=true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MYwindow::typeChange()
{
	Aresobj		o=resource.get(MAKEINTRESOURCE(PNG_SELECT), "PNG");
	Abitmap		*b0=new Abitmap(&o);
	Abitmap		*b=new Abitmap(b0->w, b0->h);
	Ttype		*t=&types[curType];
	int			i;

	for(i=0; i<9; i++)
		btype[i]->setChecked(i==curType);

	for(i=0; i<9; i++)
	{
		int		j;

		b->set(0, 0, b0, bitmapNORMAL, bitmapNORMAL);
		b->state=b0->state;
		b->flags=b0->flags;
		
		if(i<t->neffect)
		{
			Abitmap	*bs=new Abitmap(t->effect[i].snap);
			for(j=0; j<6; j++)
			{
				bs->flags=bitmapGLOBALALPHA;
				bs->alpha=bpos[j].a;
				b->set(bpos[j].x, bpos[j].y, 0, 0, 40, 30, bs, bitmapDEFAULT, bitmapDEFAULT);
			}
			delete(bs);
		}
		beffect[i]->change(b);
		beffect[i]->show(i<t->neffect);
	}

	delete(b0);
	delete(b);
	curEffect=0;
	effectChange();
	repaint();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MYwindow::effectChange()
{
	int	j;
	for(j=0; j<9; j++)
		beffect[j]->setChecked(j==curEffect);
	if(pipe)
	{
		Ttype		*t=&types[curType];
		for(j=0; j<t->effect[curEffect].nprogram; j++)
			midi(midiProgram, t->effect[curEffect].program[j], 0);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void MYwindow::makeButtons()
{
	Aresobj		o=resource.get(MAKEINTRESOURCE(PNG_SELECT), "PNG");
	Abitmap		*b0=new Abitmap(&o);
	int			i;

	curType=0;
	curEffect=0;
	
	for(i=0; i<9; i++)
	{
		Abitmap	*b=new Abitmap(b0->w, b0->h);
		int		x=60*(i%3);
		int		y=50*(i/3);
		
		b->set(0, 0, b0, bitmapNORMAL, bitmapNORMAL);
		b->state=b0->state;
		b->flags=b0->flags;
		
		{
			Ttype	*t=&types[i];
			Abitmap	*bs=new Abitmap(t->snap);
			int		j;
			for(j=0; j<6; j++)
			{
				bs->flags=bitmapGLOBALALPHA;
				bs->alpha=bpos[j].a;
				b->set(bpos[j].x, bpos[j].y, 0, 0, 40, 30, bs, bitmapDEFAULT, bitmapDEFAULT);
			}
			delete(bs);
		}
		
		btype[i]=new Abutton("btype[]", this, x+280, y+50, 42, 32, b, Abutton::btBITMAP|Abutton::bt2STATES);
		btype[i]->show(i<ntypes);
		
		b->set(0, 0, b0, bitmapNORMAL, bitmapNORMAL);
		b->state=b0->state;
		b->flags=b0->flags;
		beffect[i]=new Abutton("btype[]", this, x+510, y+50, 42, 32, b, Abutton::btBITMAP|Abutton::bt2STATES);
		
		delete(b);
	}
	delete(b0);
	
	btype[0]->setChecked(true);
	beffect[0]->setChecked(true);

	typeChange();		
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	int		ra;
	srand(GetTickCount());
	initAlib();
	{
		char	cpath[ALIB_PATH];
		GetModuleFileName(GetModuleHandle(null), cpath, sizeof(cpath));
		if(cpath[0])
		{
			char	*s=strrchr(cpath, '\\');
			if(s)
				*s=0;
		}
		SetCurrentDirectory(cpath);
	}
	{
		bool	b=false;
		MYwindow	w("remote 1.0", 100, 100, 720, 486);
		w.show(TRUE);
		ra=runAlib();;
	}
	freeAlib();
	return ra;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
