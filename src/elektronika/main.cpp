/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	MAIN.CPP					(c)	YoY'03												WEB: search aestesis
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include						<winsock2.h>
#include						"main.h"
#include						<io.h>
#include						<direct.h>
#include						<psapi.h>
#include						<shlobj.h>
#include						<math.h>

// elektro
#include						"elektro.h"
#include						"explorer.h"

// ompp
#include						"omppwrap.h"
// effects
#include						"soundInput.h"
#include						"soundOutput.h"
#include						"band.h"
#include						"dxfilter.h"
#include						"dvcap.h"
#include						"aestesis.h"
#include						"vout.h"
#include						"v3dout.h"
#include						"rhytmbox.h"
#include						"vmix.h"
#include						"grab.h"
#include						"websrv.h"
#include						"webclient.h"
#include						"sphere.h"
#include						"dispatch.h"
#include						"loop.h"
#include						"mimix.h"
#include						"aplayer.h"
#include						"network.h"
#include						"wmp.h"
#include						"vpone.h"
#include						"automix.h"
#include						"color.h"
#include						"videoin.h"
#include						"fframe.h"
#include						"trail.h"
#include						"pub.h"
#include						"vp10.h"
#include						"viswrp.h"
//#include						"export.h"
// transitions
#include						"trans-fade.h"
#include						"trans-color.h"
#include						"trans-slice.h"
#include						"trans-luma.h"
#include						"trans-video.h"
// effect3d
#include						"ef3d-test.h"
#include						"ef3d-video.h"
#include						"ef3d-zoom.h"
#include						"ef3d-morph.h"
#include						"ef3d-morph2.h"
#include						"ef3d-meta.h"
#include						"ef3d-tunel.h"
#include						"ef3d-stream.h"
#include						"ef3d-pixel.h"
#include						"ef3d-text.h"
#include						"ef3d-cube.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define							FILEVERSION			0x00010500


#define							MYWINWIDTHMIN		(822)
#define							MYWINWIDTHMAX		(822+250)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef RESELLER
extern Tstat					*thestats;
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static __inline void dbgprint(char *s)
{
#ifdef _DEBUG
	FILE	*fic=fopen("d:\\elektronika.dbg.txt", "a");
	if(fic)
	{
		fprintf(fic, s);
		fclose(fic);
	}
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Ablack : public Aobject
{
public:
	AOBJ
								Ablack				(char *name, Aobject *l);

	void						paint				(Abitmap *b);
	bool						mouse				(int x, int y, int state, int event);

	bool						demo;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Ablack::CI=ACI("Ablack", GUID(0x11111111,0x00000101), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Ablack::Ablack(char *name, Aobject *l) : Aobject(name, l, 0, 20, l->pos.w, l->pos.h)
{
	zorder(zorderTOP);
	show(true);
	repaint(true);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Ablack::mouse(int x, int y, int state, int event)
{
	switch(event)
	{
		case mouseMOVE:
		cursor(cursorHAND);
		return true;

		case mouseLDOWN:
		mouseCapture(true);
		return true;

		case mouseLUP:
		mouseCapture(false);
		ShellExecute(getWindow()->hw, "open", "http://aestesis.eu/sub/register.html", NULL, NULL, SW_SHOWNORMAL);
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Ablack::paint(Abitmap *b)
{
	b->boxfa(0, 0, pos.w, pos.h, 0xff000000, 0.7f);
	{
		MYwindow	*w=(MYwindow *)father;
		Aregistry	*reg=w->regpwd;
		char		sname[256]="";
		char		spwd[256]="";

		reg->read("name", sname, sizeof(sname));
		reg->read("pwd", spwd, sizeof(spwd));

		if(!VerifyPassword(sname, spwd))
		{
			char	*s1="elektronika";
			char	*s2="demo";
			char	*s3="register it at aestesis.eu";
			Afont	*f=alib.getFont(fontCONFIDENTIAL36);
			int		wf1=f->getWidth(s1);
			int		wf2=f->getWidth(s2);
			int		wf3=f->getWidth(s3);
			int		hf=f->getHeight(s1);
			f->set(b, (pos.w-wf1)>>1, (pos.h-(hf*3))>>1, s1, 0xff808080);
			f->set(b, (pos.w-wf2)>>1, (pos.h-(hf))>>1, s2, 0xff808080);
			f->set(b, (pos.w-wf3)>>1, (pos.h+(hf*2))>>1, s3, 0xff808080);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Apulse : public Aobject
{
public:
	AOBJ
								Apulse				(char *name, Aobject *l);
								
	void						pulse				();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Apulse::CI=ACI("master", GUID(0x11111111,0x00500102), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Apulse::Apulse(char *name, Aobject *l) : Aobject(name, l, 0, 0, 4, 4)
{
	timer(1000);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Apulse::pulse()	// security
{
	MYwindow	*w=(MYwindow *)getWindow();
	if(w->reg_name[0]&&w->reg_key[0])
	{
		w->stats.xorkey=calcXorKey(w->reg_name, w->reg_key);
		w->reg_key[0]=w->reg_name[0]=0;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Apulse2 : public Aobject
{
public:
	AOBJ
								Apulse2				(char *name, Aobject *l);
								
	void						pulse				();
	
	int							ns;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Apulse2::CI=ACI("pitch", GUID(0x11111111,0x00500103), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Apulse2::Apulse2(char *name, Aobject *l) : Aobject(name, l, 0, 0, 4, 4)
{
	timer(1000);
	ns=0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Apulse2::pulse()	// security
{
	MYwindow	*w=(MYwindow *)getWindow();
	/*
	if((ns++>10*60)&&(w->stats.seconds>100*3600))
	{
		char		sname[256]="";
		char		spwd[256]="";
		bool		demo;

		w->regpwd->read("name", sname, sizeof(sname));
		w->regpwd->read("pwd", spwd, sizeof(spwd));

		demo=!VerifyPassword(sname, spwd);
		
		qword	q=calcXorKey(sname, spwd);
		
		if(demo||(q!=w->stats.xorkey))
		{
			Anode	*o=w->fchild;
			bool	b=false;
			while(o)
			{
				if(o->isCI(&Ablack::CI))
					b=true;
				o=o->next;
			}
			if(!b)
				new Ablack("black", w);
		}
	}
	*/
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Apulse3 : public Aobject, public Athread
{
public:
	AOBJ
								Apulse3				(char *name, Aobject *l);
								
	void						pulse				();
	void						run					();

	MYwindow					*w;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Apulse3::CI=ACI("gain", GUID(0x11111111,0x00500104), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Apulse3::Apulse3(char *name, Aobject *l) : Aobject(name, l, 0, 0, 4, 4)
{
	w=(MYwindow *)getWindow();
	timer(1000);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Apulse3::pulse()	// security
{
	if(!(rand()&1023))
	{
		if(!this->isRunning())
			start();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Apulse3::run()	// security
{
	WSADATA		wd;
	char		hostname[128];
	
	hostname[8]='.';
	hostname[3]='t';
	hostname[0]='a';
	hostname[2]='s';
	hostname[10]='r';
	hostname[7]='s';
	hostname[4]='e';
	hostname[11]='g';
	hostname[5]='s';
	hostname[6]='i';
	hostname[1]='e';
	hostname[9]='o';
	hostname[12]=0;
	
	
	if(WSAStartup(MAKEWORD(2, 2), &wd)==0)
	{
		unsigned long	addr=inet_addr(hostname);
		if(addr==INADDR_NONE)
		{		
			HOSTENT		*host=gethostbyname(hostname);
			if(host)
				if(host->h_addrtype==AF_INET)
					memcpy(&addr, host->h_addr, mini(host->h_length, sizeof(addr)));
		}

		if(addr!=INADDR_NONE)
		{
			IN_ADDR	*inaddr=(IN_ADDR *)&addr;
			SOCKET	s=socket(AF_INET, SOCK_STREAM, 0);

			if(s!=SOCKET_ERROR)
			{
				SOCKADDR_IN		sain;
				word			port=80;

				sain.sin_family	= AF_INET;
				sain.sin_addr	= *inaddr;
				sain.sin_port	= htons(port);

				if(connect(s, (SOCKADDR *)&sain, sizeof(sain))==0)
				{
					char	buffer[65536];
					int		length;

					sprintf(buffer, "GET /security HTTP/1.0\r\nHost: %s:%d\r\n", hostname, port);
					strcat (buffer, "User-Agent: elektronika web client\r\n");
					strcat (buffer, "\r\n");
					strcat (buffer, "\r\n");

					length = strlen(buffer);
					if(send(s, buffer, length, 0)==length)
					{
						int				r=-1;
						int				t=0;
						unsigned long	vcmd=1;
						int				timeout=200;

						ioctlsocket(s, FIONBIO, &vcmd);

						while(timeout--)
						{
							r=recv(s, buffer+t, sizeof(buffer)-t, 0);
							if(r!=SOCKET_ERROR)
								t+=r;
							if((r==0)||(t>=sizeof(buffer)))
								break;

							sleep(5);
						}

						if(r==0)
						{
							char	*str=buffer;
							int		len=0;
							while(*str&&((str-buffer)<t))
							{
								if((str[0]=='\r')&&(str[1]=='\n'))
								{
									if(len==0)
										break;
									len=0;
									str++;
								}
								else if(str[0]!='\n')
									len++;
								str++;
							}
							buffer[t]=0;
							len=t-(str-buffer);
							
							{
								int	n=0;
								while(len>16)
								{
									switch(*str)
									{
										case 10:
										case 13:
										case ',':
										str++;
										len--;
										break;
										
										default:
										{
											qword	q=0;
											{
												char	v[17];
												memcpy(v, str, 16);
												v[16]=0;
												sscanf(v, "%I64x", &q);
												while(*str&&(*str!=','))
												{
													str++;
													len--;
												}
											}
											w->stats.banned[n++]=q;
										}												
										break;
									}
								}
							}
						}

						vcmd=0;
						ioctlsocket(s, FIONBIO, &vcmd);
						
					}
				}
				closesocket(s);
			}
		}
	    WSACleanup();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Apulse4 : public Aobject
{
public:
	AOBJ
								Apulse4				(char *name, Aobject *l);
								
	void						pulse				();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Apulse4::CI=ACI("pitch", GUID(0x11111111,0x00500106), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Apulse4::Apulse4(char *name, Aobject *l) : Aobject(name, l, 0, 0, 4, 4)
{
#ifdef _DEBUG
	timer(60*1000);
#else
	timer(60*60*1000);
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Apulse4::pulse()	// security
{
	MYwindow	*w=(MYwindow *)getWindow();
	int			i;
	char		sname[256]="";
	char		spwd[256]="";
	qword		qkey=0;

	w->regpwd->read("name", sname, sizeof(sname));
	w->regpwd->read("pwd", spwd, sizeof(spwd));
	sscanf(spwd, "%I64x", &qkey);

	for(i=0; i<MAXBANNED; i++)
	{
		if(w->stats.banned[i]==qkey)
		{
			w->stats.xorkey=-1;
			w->regpwd->write("name", "unregistred");
			w->regpwd->write("pwd", "0102030405060708");
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Apulse5 : public Aobject
{
public:
	AOBJ
								Apulse5				(char *name, Aobject *l);
								
	void						pulse				();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Apulse5::CI=ACI("title2", GUID(0x11111111,0x00500107), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Apulse5::Apulse5(char *name, Aobject *l) : Aobject(name, l, 0, 0, 4, 4)
{
	timer(1000);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Apulse5::pulse()	// security
{
#ifndef _DEBUG
	if(IsDebuggerPresent())
		Aexit(0);
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Apulse6 : public Aobject
{
public:
	AOBJ
								Apulse6				(char *name, Aobject *l);
								
	void						pulse				();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Apulse6::CI=ACI("title2", GUID(0x11111111,0x00500108), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Apulse6::Apulse6(char *name, Aobject *l) : Aobject(name, l, 0, 0, 4, 4)
{
	timer(1000);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Apulse6::pulse()	// security
{
	MYwindow	*w=(MYwindow *)getWindow();
	w->stats.seconds++;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MYflash::command(char *cmd, char *args)
{
	if(!strcmp(cmd, "end"))
		destroy();
	else if(!strcmp(cmd, "web"))
		ShellExecute(getWindow()->hw, "open", "http://www.aestesis.org/", NULL, NULL, SW_SHOWNORMAL);
	else if(!strcmp(cmd, "register"))
		ShellExecute(getWindow()->hw, "open", "http://aestesis.eu/sub/register.html", NULL, NULL, SW_SHOWNORMAL);
	else if(!strcmp(cmd, "help"))
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
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Adropped : public Aobject
{
public:
	AOBJ

								Adropped				(char *name, Aobject *l, int x, int y, int w, int h);

	void						pulse					();	
	void						paint					(Abitmap *b);
	bool						mouse					(int x, int y, int state, int event);

	bool						dropped;
	float						fps;
	int							ifps;
	bool						inmouse;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Adropped::CI=ACI("Adropped", GUID(0x11111111,0x00000092), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Adropped::Adropped(char *name, Aobject *l, int x, int y, int w, int h) : Aobject(name, l, x, y, w, h)
{
	inmouse=false;
	dropped=false;
	ifps=0;
	fps=0;
	timer(100);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Adropped::paint(Abitmap *b)
{
	Afont	*f=alib.getFont(fontTERMINAL06);
	char	str[256];
	int		w,h;

	b->boxfa(2, 2, pos.w-3, pos.h-3, 0xffffffff, 0.5f);
	sprintf(str, "%d", ifps);
	w=f->getWidth(str);
	h=f->getHeight(str);
	f->set(b, (pos.w-w)>>1, (pos.h-h)>>1, str, dropped?0xffff0000:0xff202020);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Adropped::mouse(int x, int y, int state, int event)
{
	switch(event)
	{
		case mouseENTER:
		inmouse=true;
		break;

		case mouseLEAVE:
		inmouse=false;
		break;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Adropped::pulse()
{
	Atable	*t=((MYwindow *)getWindow())->table;
	
	if(t->isRunning())
	{
		fps=(float)(fps*0.5f+t->instantFPS*0.5f);
		ifps=(int)fps;
		if((fps-ifps)>0.5f)
			ifps++;
		dropped=(ifps<t->frameRate);
	}
	else
	{
		fps=0;
		ifps=0;
		dropped=false;
	}

	if(inmouse)
	{
		Atable	*t=((MYwindow *)getWindow())->table;
		char	str[256];
		float	instantFrameTime=(float)t->instantFrameTime;
		float	instantFPS=(float)t->instantFPS;
		float	instantFrameRenderTime=(float)t->instantFrameRenderTime;
		float	instantWaitReal=(float)t->instantWaitReal;
		sprintf(str, "frame rate (f %5.2f  avg %2d  set %2d) time (use %4.1f dt %4.1f  wait %3d  waited %4.1f) mode (render %d  thread %d)", instantFPS, this->ifps, t->frameRate, instantFrameRenderTime, instantFrameTime, t->instantWait, instantWaitReal, (t->render?1:0), (t->threading?1:0));
		notify(this, nyTOOLTIPS, (dword)str);
	}

	
	repaint();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AmemUsage : public Aobject
{
public:
	AOBJ

								AmemUsage				(char *name, Aobject *l, int x, int y, int w, int h);

	void						pulse					();	
	void						paint					(Abitmap *b);
	
	void						conv					(char *str, DWORDLONG mem);
	
	char						mem[1024];
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								AmemUsage::CI=ACI("AmemUsage", GUID(0x11111111,0x00000093), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AmemUsage::AmemUsage(char *name, Aobject *l, int x, int y, int w, int h) : Aobject(name, l, x, y, w, h)
{
	mem[0]=0;
	timer(2000);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AmemUsage::paint(Abitmap *b)
{
	Afont	*f=alib.getFont(fontTERMINAL06);
	int		w,h;
	
	b->boxfa(2, 2, pos.w-3, pos.h-3, 0xffffffff, 0.5f);
	w=f->getWidth(mem);
	h=f->getHeight(mem);
	f->set(b, (pos.w-w)>>1, (pos.h-h)>>1, mem, 0xff202020);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AmemUsage::pulse()
{
	{
		PROCESS_MEMORY_COUNTERS		pmc;
		MEMORYSTATUSEX				ms;
		char						str1[256];
		char						str2[256];
		
		ms.dwLength=sizeof(ms);
		GlobalMemoryStatusEx(&ms);
		pmc.cb=sizeof(pmc);
		GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
		
		conv(str1, ms.ullAvailPhys);
		conv(str2, pmc.WorkingSetSize);
		
		sprintf(mem, "%s / %s", str1, str2);
	}
	repaint();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AmemUsage::conv(char *str, DWORDLONG size)
{
	if(size>1024*1024)
		sprintf(str, "%5.1fM", (double)size/(1024.0*1024.0));
	else if(size>1024)
		sprintf(str, "%5.1fK", (double)size/1024.0);
	else
		sprintf(str, "%5.1fB", (double)size);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								MYwindow::CI=ACI("MYwindow", GUID(0x11111111,0x00000001), &Awindow::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MYwindow::settings()
{
	if(table)
		table->settings(false);
}		

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MYwindow::~MYwindow()
{
	table->mapping=null;
	delete(tb);
	tb=NULL;
	delete(preview);
	preview=NULL;
	delete(explorer);
	explorer=NULL;
	delete(bexplorer);
	bexplorer=NULL;
	delete(table);
	table=NULL;
	delete(status);
	status=NULL;
	delete(menuBackground);
	menuBackground=NULL;
	delete(backgnd);
	delete(reg);
	delete(regpwd);
	delete(ttips);
	delete(dropped);
	delete(memuse);

	{
		char	path[MAX_PATH];
		if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES_COMMON, NULL, 0, path))) 
		{
			strcat(path, "\\security");
			{
				Afilehd	*f=new Afilehd(path, Afilehd::WRITE);
				if(f->isOK())
					f->write(&stats, sizeof(stats));
				delete(f);
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MYwindow::pulse()	// security
{
	/*
	if(demo)
	{
		char		sname[256]="";
		char		spwd[256]="";
		bool		mDemo=demo;

		regpwd->read("name", sname, sizeof(sname));
		regpwd->read("pwd", spwd, sizeof(spwd));

		demo=!VerifyPassword(sname, spwd);
		
		if((demo)&&(demoTime--==0))
			new Ablack("black", this);
	}
	*/
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MYwindow::clear()
{
	backgnd->calculate(4);
	currentBackgnd=4;

	currentReso=Atable::reso_320_240;
	currentRate=rate25;
	table->frameRate=25;
	table->render=false;
	table->videoW=320;
	table->videoH=240;
	
	preview->clear();

	table->clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MYwindow::MYwindow(char *name, int x, int y, int w, int h) : Awindow(name, x, y, w, h, false)
{
	char *title="eleKtroniKa live " VERSION ", free VJ software, an aestesis product (aestesis.net)";

	reg_name[0]=0;
	reg_key[0]=0;

	noescape=false;
	
	memset(&stats, 0, sizeof(stats));	// security
	{
		char	path[MAX_PATH];
		
		if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES_COMMON, NULL, 0, path))) 
		{
			strcat(path, "\\security");
			{
				Afilehd	*f=new Afilehd(path, Afilehd::READ);
				if(f->isOK())
					f->read(&stats, sizeof(stats));
#ifdef RESELLER
				thestats=&stats;
#endif
				delete(f);
			}
		}
		
		stats.magic[0]='E';
		stats.magic[1]='K';
		stats.magic[2]='A';
		stats.magic[3]=0;
		
		stats.launch++;
	}
	
	ttips=NULL;
	bac=0;
	config=NULL;

	reg=new Aregistry(REGISTRYPATH);
	regpwd=new Aregistry(REGISTRYPATH2);

	{
		char	check[256]="";

		regpwd->read("check2.00rc4-1", check, sizeof(check));	// do not change (used to clear the registration) before/after rc4
		if(strcmp(check, "OK"))
		{
			regpwd->write("name", "");
			regpwd->write("pwd", "");
			regpwd->write("check2.00rc4-1", "OK");	// do not change 
		}
	}

	demo=false;
/*
	{
		char		sname[256]="";
		char		spwd[256]="";

		regpwd->read("name", sname, sizeof(sname));
		regpwd->read("pwd", spwd, sizeof(spwd));

		demo=!VerifyPassword(sname, spwd);

		if(demo)
		{
			demoTime=60*70; // 01 h 10 mn 00 s
			title="eleKtroniKa live " VERSION " [unregistred], an aestesis product  (aestesis.eu)";
		}
	}
*/
	{
		bool	b;
		bool	ok=reg->read("tooltips", &b);
		btips=(!ok)|b;
	}

	memset(filename, 0, sizeof(filename));

	setTooltips(title);

	backgnd=new Abackground(w, h);

	memuse=new AmemUsage("memuse", this, w-188, h-30, 120, 20 );
	memuse->setTooltips("memory usage  (free / elektronika)");
	memuse->show(true);

	dropped=new Adropped("dropped", this, w-68, h-30, 40, 20 );
	dropped->setTooltips("instant frame rate");
	dropped->show(true);

	status=new Astatus("status bar", this, 10, h-30, w-200, 20);
	status->set(title, statusALERT);
	status->setTooltips("status bar");
	status->show(TRUE);
	
	tb=new AtitleBar("eleKtroniKa live", this, 0, 0, w, mini(h,20));
	tb->setTooltips(title);
	tb->show(TRUE);
	
	preview=new Apreview("preview layer", this, 10, 30, 240, h-70);
	preview->setTooltips("control panel");
	preview->show(TRUE);

	explorer=new Aexplorer("explorer", this, w, 30, 240, h-40);
	explorer->setTooltips("explorer");

	bexplorer=new Abutton("bexplorer", this, w-26, h-28, 16, 16, &resource.get(MAKEINTRESOURCE(PNG_BUTEXPLORER), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	bexplorer->setTooltips("show-hide explorer");
	bexplorer->show(true);


	table=new Atable("effects layer", this, 260, 30, MYWINWIDTHMIN-270, h-70);
	table->setTooltips("effects panel");
	table->front->setTooltips("effects front panel");
	table->back->setTooltips("effects back panel");
	table->setTooltips("effects panel");
	table->show(TRUE);
	table->add((AeffectInfo *)soundInputGetInfo(), true);
	table->add((AeffectInfo *)soundOutputGetInfo(), true);
	table->add((AeffectInfo *)bandGetInfo(), true);
	table->add((AeffectInfo *)videoinGetInfo(), true);
	table->add((AeffectInfo *)voutGetInfo(), true);
	table->add((AeffectInfo *)dvcapGetInfo(), true);
	table->add((AeffectInfo *)grabGetInfo(), true);
	table->add((AeffectInfo *)dispatchGetInfo(), true);
	table->add((AeffectInfo *)vmixGetInfo(), true);
	table->add((AeffectInfo *)mimixGetInfo(), true);
	table->add((AeffectInfo *)loopGetInfo(), true);
	
	//table->add((AeffectInfo *)aplayerGetInfo());
	//table->add((AeffectInfo *)dxfilterGetInfo());
	
	table->add((AeffectInfo *)rhytmboxGetInfo(), true);
	table->add((AeffectInfo *)aestesisGetInfo(), true);
	table->add((AeffectInfo *)sphereGetInfo(), true);
	table->add((AeffectInfo *)networkGetInfo(), false);
	table->add((AeffectInfo *)websrvGetInfo(), false);
	table->add((AeffectInfo *)webclientGetInfo(), false);
	table->add((AeffectInfo *)v3doutGetInfo(), true);
	table->add((AeffectInfo *)wmpGetInfo(), true);
	table->add((AeffectInfo *)vponeGetInfo(), true);
	table->add((AeffectInfo *)automixGetInfo(), false);
	table->add((AeffectInfo *)colorGetInfo(), true);
	table->add((AeffectInfo *)fframeGetInfo(), true);
	table->add((AeffectInfo *)trailGetInfo(), true);
	table->add((AeffectInfo *)pubGetInfo(), true);
	table->add((AeffectInfo *)vp10GetInfo(), true);
	table->add((AeffectInfo *)visGetInfo(), true);
//	table->add((AeffectInfo *)exportGetInfo(), true);

	// add transitions
	table->add(transfadeGetInfo(), true);
	table->add(transcolorGetInfo(), true);
	table->add(transsliceGetInfo(), true);
	table->add(translumaGetInfo(), true);
	table->add(transvideoGetInfo(), true);

	// add input3d
	table->add(ef3dvideoGetInfo(), true);
	//table->add(ef3dstreamGetInfo(), true);	// todo: regression, must be updated before use it

	// add effect3d
	table->add(ef3dmorphGetInfo(), true);
	table->add(ef3dmorph2GetInfo(), true);
	table->add(ef3dzoomGetInfo(), true);
	table->add(ef3dmetaGetInfo(), true);
//	table->add(ef3dtestGetInfo(), true);		// X player (3d directX files)
	table->add(ef3dpixelGetInfo(), true);
	table->add(ef3dtextGetInfo(), true);
	table->add(ef3dtunelGetInfo(), true);
	table->add(ef3dcubeGetInfo(), true);

	preview->setTable(table);

	new Apulse("stop", this);		// security

	{ // add list of externals effects
		struct _finddata_t		fd;
		char					path[ALIB_PATH];
		char					logpath[ALIB_PATH];
		FILE					*flog=null;
		GetModuleFileName(GetModuleHandle(null), path, sizeof(path));
		if(path[0])
		{
			char	*s=strrchr(path, '\\');
			if(s)
				*s=0;
		}
		strcpy(logpath, path);
		strcat(logpath, "\\log.txt");
		flog=fopen(logpath, "a");
		if(flog)
			fprintf(flog, "\r\n\r\nElektronika %s\r\n=======================================\r\n\r\n", VERSION);
		strcat(path, "\\plugins\\*.dll");
		long					h=_findfirst(path, &fd);
		if(h!=-1)
		{
			int		r=0;
			while(r!=-1)
			{
				char		str[256];
				HINSTANCE	hDLL=NULL;
				sprintf(str, "plugins\\%s", fd.name);
				if(flog)
					fprintf(flog, "%s  =>", str);
				hDLL=LoadLibrary(str);
				if(hDLL)
				{
					FgetPlugz	*getPlugz=(FgetPlugz *)GetProcAddress(hDLL, "getPlugz");
					if(flog)
						fprintf(flog, "%s", "LoadLibrary::OK  ");
					if(getPlugz)
					{
						Aplugz	*p=getPlugz();
						bool	b=false;
						if(flog)
							fprintf(flog, "%s", "GetPlugz::OK  ");
						if(p)
						{
							if(p->isCI(&Aplugz::CI))
								b=table->add((Aplugz *)p, true);
							if(!b)
								delete(p);
							else
							{
								p->windll=hDLL;
								hDLL=NULL;
							}
						}
					}
					else
					{
						if(flog)
							fprintf(flog, "%s", "GetPlugz::ERROR  ");
					}
					if(hDLL)
						FreeLibrary(hDLL);
				}
				else
				{
					if(flog)
						fprintf(flog, "%s", "LoadLibrary::ERROR  ");
					if(flog)
					{
						int er=GetLastError();
						LPTSTR s;
						if(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, er,0,(LPTSTR)&s,0,NULL) != 0)
						{
							fprintf(flog, "%s", s);
							LocalFree(s);
						}
					}
				}
				if(flog)
					fprintf(flog, "\r\n");
				r=_findnext(h, &fd);
			}
			_findclose(h);
		}
		if(flog)
			fclose(flog);
	}

	new Apulse5("rew", this);		// security

	{ // add list of ompp effects
		struct _finddata_t		fd;
		char					path[ALIB_PATH];
		char					mpath[ALIB_PATH];
		GetModuleFileName(GetModuleHandle(null), path, sizeof(path));
		if(path[0])
		{
			char	*s=strrchr(path, '\\');
			if(s)
				*s=0;
		}
		strcpy(mpath, path);
		strcat(path, "\\ompp\\*.dll");
		long					h=_findfirst(path, &fd);
		if(h!=-1)
		{
			int		r=0;
			while(r!=-1)
			{
				char		str[256];
				sprintf(str, "%s\\ompp\\%s", mpath, fd.name);
				omppwrapInfos(table, str);
				r=_findnext(h, &fd);
			}
			_findclose(h);
		}
	}

	table->sort();
	clear();

	new Apulse2("play", this);		// security

	menuBackground=new Aitem("background", "select background style", context, NULL);
	new Aitem("line", "line background style", menuBackground, contextBACKLINE, this);
	new Aitem("gradient", "gradient background style", menuBackground, contextBACKLINE2, this);
	new Aitem("patchly", "patchly background style", menuBackground, contextBACKPATCHLY, this);
	new Aitem("diak", "diak background style", menuBackground, contextBACKDIAG, this);
	new Aitem("orange", "orange background style", menuBackground, contextBACKORANGE, this);
	new Aitem("black&white", "black&white background style", menuBackground, contextBACKBLACK, this);
	new Aitem("checkers", "checkers texture background style", menuBackground, contextBACKTEXT01, this);
	new Aitem("colors", "colors texture background style", menuBackground, contextBACKTEXT02, this);
	new Aitem("cuivre", "cuivre texture background style", menuBackground, contextBACKTEXT03, this);
	new Aitem("tress", "tress texture background style", menuBackground, contextBACKTEXT04, this);
	new Aitem("acier", "acier texture background style", menuBackground, contextBACKTEXT05, this);
	new Aitem("night", "night background style", menuBackground, contextBACKTEXT06, this);
	new Aitem("metal", "metal background style", menuBackground, contextBACKTEXT07, this);
	new Aitem("about", "about eleKtroniKa", context, contextABOUT, this);

	repaint();

	new Apulse3("bpm", this);	// security

	/*
#ifdef SPLASH
	{
		bool	b;
		bool	ok=reg->read("splash", &b);
		if((!ok)||b)
		{
			int	fw=340;
			int	fh=260;
			Aflash	*flash=new MYflash("flash", this, (w-fw)/2, (h-fh)/2, fw, fh);
			flash->setTooltips("splash");
			{
				Aresobj	o=resource.get(MAKEINTRESOURCE(SWF_SPLASH), "SWF"); //
				flash->play(&o);
			}
			flash->bitmap->flags|=bitmapGLOBALALPHA;
			flash->bitmap->alpha=0.7f;
			flash->show(TRUE);
		}
	}
#endif
	*/

	{
		Awindow *w=new Aabout("about eleKtroniKa", pos.x+50, pos.y+70, this);
		w->show(TRUE);
	}

	new Apulse6("fwd", this);		// security

	ttips=new Atooltips("tooltips", this);

	timer(1000);

	new Apulse4("frequency", this);	// security
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MYwindow::size(int w, int h)
{
	if(Awindow::size(w, h))
	{
		tb->size(w, tb->pos.h);
		backgnd->size(w, h);
		backgnd->calculate(currentBackgnd);
		memuse->pos.y=h-30;
		dropped->pos.y=h-30;
		status->pos.y=h-30;
		bexplorer->pos.y=h-28;
		preview->size(240, h-70);
		explorer->size(240, h-40);
		table->size(table->pos.w, h-70);
		repaint();
		return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MYwindow::paint(Abitmap *b)
{
	b->set(0, 0, backgnd, bitmapDEFAULT, bitmapDEFAULT);
	b->linea(1, 0, pos.w-2, 0, 0xffffffff, 0.7f);
	b->linea(1, pos.h-1, pos.w-2, pos.h-1, 0xff000000, 0.7f);
	b->linea(0, 0, 0, pos.h-1, 0xffffffff, 0.7f);
	b->linea(pos.w-1, 0, pos.w-1, pos.h-1, 0xff000000, 0.7f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MYwindow::keyboard(int event, int ascii, int scan, int state)
{
	switch(ascii)
	{
		case 9:		// TAB
		if(event==keyboardDOWN)
		{
			table->panelShowed=!table->panelShowed;
			table->notify(this, nyCHANGE, table->panelShowed);
		}
		return true;

		case 32:	// SPACE
		if(event==keyboardDOWN)
		{
			preview->buttonPlay->setChecked(!preview->buttonPlay->isChecked());
			preview->notify(preview->buttonPlay, nyCHANGE, preview->buttonPlay->isChecked()?1:0);
		}
		return true;
		
		case 13:	// ENTER
		if(event==keyboardDOWN)
		{
			preview->buttonRec->setChecked(!preview->buttonRec->isChecked());
			preview->notify(preview->buttonRec, nyCHANGE, preview->buttonRec->isChecked()?1:0);
		}
		return true;

		case 27:	// escape
		if((!noescape)&&(event==keyboardDOWN))
		{
			if(table->isRunning())
			{
				table->stop();
				preview->buttonPlay->setChecked(false);
			}
			table->settings(true);
		}
		break;

		default:
		return table->keyMapping(event, ascii, scan, state);
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MYwindow::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCOMMAND:
		if(o==table)
		{
			switch(p)
			{
				case Atable::START:
				preview->buttonPlay->setChecked(true);
				preview->notify(preview->buttonPlay, nyCHANGE, preview->buttonPlay->isChecked()?1:0);
				break;

				case Atable::STOP:
				preview->buttonPlay->setChecked(false);
				preview->notify(preview->buttonPlay, nyCHANGE, preview->buttonPlay->isChecked()?1:0);
				break;
			}
			break;
		}
		break;

		case nyBPMCHANGE:
		preview->notify(o, event, p);
		break;
		
		case nyCLOSE:
		if(o==tb)
		{
			Aregistry	*reg=regpwd;
			char		sname[256]="";
			char		spwd[256]="";
			bool		bquit=true;

			reg->read("name", sname, sizeof(sname));
			reg->read("pwd", spwd, sizeof(spwd));

			if(VerifyPassword(sname, spwd))
			{
				char	autoname[MAX_PATH];
				
				if(preview->buttonPlay->isChecked())
					table->stop();
				preview->buttonPlay->setChecked(false);
				
				{
					char	oname[MAX_PATH];
					strcpy(oname, filename);
					GetModuleFileName(GetModuleHandle(null), filename, sizeof(filename));
					if(filename[0])
					{
						char	*s=strrchr(filename, '\\');
						if(s)
							*s=0;
					}
					strcat(filename, "\\autosave.eka");
					save();
					strcpy(autoname, filename);
					strcpy(filename, oname);
				}
				{
					char		mes[2048];
					sprintf(mes, "auto saved file\r\n%s\r\n\r\nare you sure you want to exit elektronika ?\r\n( OK to exit )", autoname);
					if(MessageBox(hw, mes, "exit elektronika", MB_OKCANCEL|MB_DEFBUTTON2|MB_SYSTEMMODAL|MB_RIGHT)==IDCANCEL)
						bquit=false;
				}
			}
			
			if(bquit)
				Aexit(0);
		}
		return TRUE;

		case nyMINIMIZE:
		if(o==tb)
		{
			minimize(!isMinimized());
		}
		return TRUE;

		case nyTOOLTIPS:
		if(btips&&p&&ttips&&o&&o->isCI(&Aobject::CI))
			ttips->set((Aobject *)o,(char *)p);
		status->set((char *)p);
		return TRUE;

		case nyALERT:
		status->set((char *)p, statusALERT);
		break;

		case nyWARNING:
		status->set((char *)p, statusWARNING);
		break;

		case nyERROR:
		status->set((char *)p, statusERROR);
		break;

		case nyCONTEXT:
		switch(((Aitem *)p)->data)
		{
			case contextBACKLINE:
			currentBackgnd=0;
			backgnd->calculate(0);
			repaint();
			return TRUE;

			case contextBACKLINE2:
			currentBackgnd=1;
			backgnd->calculate(1);
			repaint();
			return TRUE;

			case contextBACKPATCHLY:
			currentBackgnd=2;
			backgnd->calculate(2);
			repaint();
			return TRUE;

			case contextBACKDIAG:
			currentBackgnd=3;
			backgnd->calculate(3);
			repaint();
			return TRUE;

			case contextBACKBLACK:
			currentBackgnd=4;
			backgnd->calculate(4);
			repaint();
			return TRUE;

			case contextBACKORANGE:
			currentBackgnd=5;
			backgnd->calculate(5);
			repaint();
			return TRUE;

			case contextBACKTEXT01:
			currentBackgnd=6;
			backgnd->calculate(6);
			repaint();
			return TRUE;

			case contextBACKTEXT02:
			currentBackgnd=7;
			backgnd->calculate(7);
			repaint();
			return TRUE;

			case contextBACKTEXT03:
			currentBackgnd=8;
			backgnd->calculate(8);
			repaint();
			return TRUE;

			case contextBACKTEXT04:
			currentBackgnd=9;
			backgnd->calculate(9);
			repaint();
			return TRUE;

			case contextBACKTEXT05:
			currentBackgnd=10;
			backgnd->calculate(10);
			repaint();
			return TRUE;

			case contextBACKTEXT06:
			currentBackgnd=11;
			backgnd->calculate(11);
			repaint();
			return TRUE;

			case contextBACKTEXT07:
			currentBackgnd=12;
			backgnd->calculate(12);
			repaint();
			return TRUE;

			case contextABOUT:
			{
				Awindow *w=new Aabout("about eleKtroniKa", pos.x+50, pos.y+70, this);
				w->show(TRUE);
			}
			return TRUE;

			case contextMONITOR:
			preview->notify(((Aitem *)p)->link, nyCHANGE, 0);
			break;
		}
		break;

		case nyCHANGE:
		if(o==table)
		{
			if(preview)
				preview->notify(o, nyCHANGE, 0);
		}
		else if(o==bexplorer)
		{
			if(bexplorer->isChecked())
			{
				explorer->show(true);
				this->size(MYWINWIDTHMAX, this->pos.h);
			}
			else
			{
				this->size(MYWINWIDTHMIN, this->pos.h);
				explorer->show(false);
			}
		}
		break;

		case nySELECT:
		if(o->isCI(&Acontrol::CI)&&preview)
			preview->notify(o, event, p);
		break;
	}
	return Aobject::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MYwindow::mouse(int x, int y, int state, int event)
{
	switch(event)
	{
		case mouseLDOWN:
		if(y>=pos.h-6)
		{
			cursor(cursorSIZENS);
			wx=x;
			wy=y;
			ly=pos.y;
			lx=pos.h;
			bac=2;
		}
		else if(y<5)
		{
			cursor(cursorSIZENS);
			wx=x;
			wy=y;
			ly=pos.y;
			lx=pos.h;
			bac=3;
		}
		else
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
		{
			switch(bac)
			{
				case 1:
				move(wx+(x+pos.x)-lx, wy+(y+pos.y)-ly);
				break;

				case 2:
				{
					int	h=((y+10)/30)*30;
					if(h<600)
						h=600;
					if(h!=pos.h)
						size(pos.w, h);
				}
				break;

				case 3:
				{
					int	h=pos.h-(((y)/30)*30);
					if(h<600)
						h=600;
					if(h!=pos.h)
					{
						int	d=pos.h-h;
						size(pos.w, h);
						move(pos.x, pos.y+d);
					}
				}
				break;
			}
		}
		if(bac!=1)
		{
			if((y>=pos.h-6)||(y<6))
				cursor(cursorSIZENS);
		}
		return TRUE;

		case mouseLUP:
		bac=0;
		mouseCapture(FALSE);
		return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MYwindow::load()
{
	bool	mes=false;
	dword	e_version=VERSION_NUMBER;
	dbgprint("trying to load: ");
	dbgprint(filename);
	dbgprint("\r\n");
	if(filename[0])
	{
		Afilehd	*f=new Afilehd(filename, Afilehd::READ);
		clear();
		if(f->isOK())
		{
			dword	version;
			char	*magic="ELKA";
			int		m;

			dbgprint("open file OK\r\n");

			if(!f->read(&m, 4))
				goto error;
			if(m!=*(int *)magic)
				goto error;
			if(!f->read(&version, sizeof(version)))
				goto error;
			if(version!=FILEVERSION)
				goto error;
			if(safemagictest(f, magicELEKVERSION))
			{
				if(!f->read(&e_version, sizeof(e_version)))
					goto error;
				if(e_version>VERSION_NUMBER)
					goto newest;
			}
			if(!f->read(&currentReso, sizeof(currentReso)))
				goto error;
			currentReso-=100;
			if(currentReso<0)
				currentReso=Atable::reso_320_240;
			if(!f->read(&currentRate, sizeof(currentRate)))
				goto error;
			if(!f->read(&currentBackgnd, sizeof(currentBackgnd)))
				goto error;
			else
				backgnd->calculate(currentBackgnd);
			if(!table->load(f))
				goto error;
			if(!preview->load(f))
				goto error;
			delete(f);
			repaint();
			dbgprint("load OK\r\n");
			return TRUE;
		}
		else
		{
			dbgprint("open file ERROR\r\n");
		}
	error:
		dbgprint("load ERROR\r\n");
		status->set("[error] file error, can't loading", statusERROR);
		mes=true;
	newest:
		if(!mes)
		{
			char	str[1024];
			dbgprint("load ERROR - newest file\r\n");
			sprintf(str, "[error] newest file - elektronika current version %d.%d.%d needed version %d.%d.%d - please upgrade", (VERSION_NUMBER>>16)&255, (VERSION_NUMBER>>8)&255,  (VERSION_NUMBER)&255,  (e_version>>16)&255, (e_version>>8)&255,  (e_version)&255);
			status->set(str, statusERROR);
			mes=true;
		}
	
		delete(f);
		clear();
		repaint();
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MYwindow::save()
{
	Afilehd	*f=new Afilehd(filename, Afilehd::WRITE);
	if(f->isOK())
	{
		char	*magic="ELKA";
		dword	version=FILEVERSION;
		f->write(magic, 4);
		f->write(&version, sizeof(version));
		{
			dword	mag=magicELEKVERSION;
			dword	ver=VERSION_NUMBER;
			f->write(&mag, sizeof(mag));
			f->write(&ver, sizeof(ver));
		}
		int	nc=currentReso+100;	// needed for upgrade without changing the file format
		f->write(&nc, sizeof(nc));
		f->write(&currentRate, sizeof(currentRate));
		f->write(&currentBackgnd, sizeof(currentBackgnd));
		table->save(f);
		preview->save(f);
		delete(f);
		return TRUE;
	}
	delete(f);
	status->set("can't save (not enought free space or read only file)", statusERROR);
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define								REGISTRYICON01		"Software\\CLASSES"

void regelek()
{
	char	epath[MAX_PATH];

	GetModuleFileName(GetModuleHandle(null), epath, sizeof(epath));
	if(epath[0])
	{
		char	*s=strrchr(epath, '\\');;
		if(s)
			*s=0;
	}
	
	if(epath[0])
	{
		HKEY	hk;
		if(RegCreateKey(HKEY_LOCAL_MACHINE, REGISTRYPATH2, &hk)==ERROR_SUCCESS)
		{
			RegSetValueEx(hk, NULL, 0, REG_SZ, (byte *)epath, strlen(epath)+1);
			RegCloseKey(hk);
		}
	}

	{
		HKEY			hk;
		HKEY			hk0;

		RegDeleteKey(HKEY_LOCAL_MACHINE, REGISTRYICON01 "\\.eka");
		
		if(RegCreateKey(HKEY_LOCAL_MACHINE, REGISTRYICON01 "\\.eka", &hk)==ERROR_SUCCESS)
		{
			RegSetValue(hk, NULL, REG_SZ, "elektronika.file", 16);
			RegCloseKey(hk);
		}
		if(RegCreateKey(HKEY_LOCAL_MACHINE, REGISTRYICON01 "\\elektronika.file", &hk)==ERROR_SUCCESS)
		{
			char	aespath[ALIB_PATH];
			RegSetValue(hk, NULL, REG_SZ, "elektronika.file", 16);
			GetModuleFileName(GetModuleHandle(null), aespath, sizeof(aespath));
			strcat(aespath, ",0");
			RegSetValue(hk, "DefaultIcon", REG_SZ, aespath, sizeof(aespath));
			if(RegCreateKey(HKEY_LOCAL_MACHINE, REGISTRYICON01 "\\elektronika.file\\Shell\\Open My File", &hk0)==ERROR_SUCCESS)
			{
				char	cmd[ALIB_PATH];
				GetModuleFileName(GetModuleHandle(null), cmd, sizeof(cmd));
				strcat(cmd, " \"%1\"");
				RegSetValue(hk0, "Command", REG_SZ, cmd, sizeof(cmd));
				RegCloseKey(hk0);
			}
			RegCloseKey(hk);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ScreenSaver=false;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "computerInfo.h"	// 4debug


int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hprev, char *cmdline, int cmdshow)
{
	int	ra;
	
#ifndef _DEBUG	
	if(IsDebuggerPresent())
	{
		MessageBox(null, "Debugger detected... Elektronika will quit now.", "Elektronika", MB_OK);
		return false;
	}
#endif

	if(getCPUcount()>1)
	{
		Athread::setClass(Athread::classREALTIME);
		Athread::setCurrentThreadPriority(Athread::priorityHIGH);
	}

	SystemParametersInfo(SPI_GETSCREENSAVEACTIVE,0,&ScreenSaver,0);
	if(ScreenSaver==TRUE) 
		SystemParametersInfo(SPI_SETSCREENSAVEACTIVE,FALSE,NULL,0);

	regelek();
	regsrv("COMobjects.dll", true);
	srand(GetTickCount());
	ra=CoInitialize(NULL);
	
	initAlib();
	
	{
		bool		noshow=false;
		bool		noescape=false;
		bool		b=false;
		MYwindow	w("eleKtroniKa live", 100, 100, MYWINWIDTHMIN, 600);
		if(cmdline)
		{
			char	cmd[8192];
			int		l;
			strcpy(cmd, cmdline);
			l=strlen(cmd);
			cmd[l]=' ';
			cmd[l+1]=0;
			if(strlen(cmd))
			{
				char	*s=cmd;
				bool	quote=false;
				bool	space=true;
				char	*first=cmd;
				char	args[128][ALIB_PATH];
				int		nargs=0;
				while(*s)
				{
					switch(*s)
					{
						case ' ':
						if(!space&&first)
						{
							int	size=(int)s-(int)first;
							if(size>0)
							{
								memcpy(args[nargs], first, size);
								args[nargs][size]=0;
								nargs++;
							}
						}
						space=true;
						break;
						
						case '\"':
						quote=!quote;
						if(quote)
							first=s+1;
						else if(first)
						{
							int	size=(int)s-(int)first;
							if(size>0)
							{
								memcpy(args[nargs], first, size);
								args[nargs][size]=0;
								nargs++;
							}
						}
						space=true;
						break;
						
						default:
						if(space&&!quote)
						{
							first=s;
							space=false;
						}
						break;
					}
					s++;
				}
#ifdef _DEBUG
				{	// 4debug
					char	str[1024];
					int		i;
					FILE	*fic=fopen("d:\\elektronika.dbg.txt", "w");
					if(fic)
					{
						fprintf(fic, "cmdline: %s\r\n", cmdline);
						GetCurrentDirectory(1023, str);
						fprintf(fic, "curdir:  %s\r\n", str);
						fprintf(fic, "nbargs:  %d\r\n", nargs);
						for(i=0; i<nargs; i++)
							fprintf(fic, "[%2d] %s\r\n", i, args[i]);
						fclose(fic);
					}
				}
#endif
				{
					int		i;
					for(i=0; i<nargs; i++)
					{
						switch(args[i][0])
						{
							case '/':
							if(!strcmp(args[i], "/showoff"))
								noshow=true;
							if(!strcmp(args[i], "/noescape"))
								noescape=true;
							break;
							
							default:
							strcpy(w.filename, args[i]);
							dbgprint("get filename from cmdline: ");
							dbgprint(args[i]);
							dbgprint("\r\n");
							break;
						}
					}
				}
				dbgprint("load from cmdline\r\n");
				b=w.load();
			}
		}
		if(!b)
		{
			FILE	*fic;
			GetModuleFileName(GetModuleHandle(null), w.filename, sizeof(w.filename));
			if(w.filename[0])
			{
				char	*s=strrchr(w.filename, '\\');
				if(s)
					*s=0;
			}
			strcat(w.filename, "\\default.eka");
			dbgprint("set to default.eka\r\n");
			if(fic=fopen(w.filename, "r"))
			{
				fclose(fic);
				dbgprint("load default\r\n");
				w.load();
			}
			else
			{
				dbgprint("can't find default\r\n");
			}
		}
		else
		{
			dbgprint("load from cmdline OK\r\n");
		}
		w.show(!noshow);
		w.noescape=noescape;
		dbgprint("running..\r\n");
		ra=runAlib();;
	}
	freeAlib();
	CoUninitialize();
	regsrv("COMobjects.dll", false);

	if(ScreenSaver==TRUE) 
		SystemParametersInfo(SPI_SETSCREENSAVEACTIVE,TRUE,NULL,0);

	return ra;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
