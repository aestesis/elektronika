// flashPlayer.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "flashPlayer.h"
#include "shockwaveflash.h"
#include "apihijack.h"
#include <assert.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

/////////////////////////////////////////////////////////////////////////////
// CFlashPlayerApp

BEGIN_MESSAGE_MAP(CFlashPlayerApp, CWinApp)
	//{{AFX_MSG_MAP(CFlashPlayerApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFlashPlayerApp construction

CFlashPlayerApp::CFlashPlayerApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CFlashPlayerApp object

// CFlashPlayerApp theApp;

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	MAIN.CPP					(c)	YoY'02						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<math.h>
#include						"flashp.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// elektronika SDK effect sample
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// if you want to use MFC/AFX
// add the MFC/AFX includes
// select "MFC  shared dll" in general tab of the project settings
// and remove the _USRDLL define in the project settings
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								AflashInfo::CI		= ACI("AflashInfo",		GUID(0x50000000,0x00000850), &AeffectInfo::CI, 0, NULL);
ACI								Aflashp::CI			= ACI("Aflashp",		GUID(0x50000000,0x00000851), &Aeffect::CI, 0, NULL);
ACI								AflashFront::CI		= ACI("AflashFront",	GUID(0x50000000,0x00000852), &AeffectFront::CI, 0, NULL);
ACI								AflashBack::CI		= ACI("AflashBack",		GUID(0x50000000,0x00000853), &AeffectBack::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Amtext : public Aobject
{
public:
	AOBJ

								Amtext							(char *name, Aobject *father, int x, int y, int w, int h);
	virtual						~Amtext							();

	void						paint							(Abitmap *b);
	bool						notify							(Anode *o, int event, dword p);

	Aedit						*edit;
	Abutton						*ok;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Amtext::CI			= ACI("Amtext",		GUID(0x50000000,0x00000856), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Amtext::Amtext(char *name, Aobject *father, int x, int y, int w, int h) : Aobject(name, father, x, y, w, h)
{
	edit=new Aedit("edit", this, 4, 4, pos.w-34, pos.h-8);
	edit->state|=Aedit::stateMULTILINE;
	edit->setTooltips("text");
	edit->show(true);

	ok=new Abutton("ok", this, pos.w-26, 4, 22, pos.h-8, "OK");
	ok->setTooltips("ok");
	ok->show(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Amtext::~Amtext()
{
	delete(edit);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Amtext::paint(Abitmap *b)
{
	b->boxfa(0, 0, pos.w, pos.h, 0xffffffff, 0.8f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Amtext::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyPRESS:
		father->notify(this, nyCHANGE, 0);
		return true;
	}
	return Aobject::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aflashm : public Aobject
{
public:
	AOBJ

								Aflashm						(QIID qiid, char *name, Aobject *L, int x, int y, int w, int h);
	virtual						~Aflashm					();

	Apaddle						*pad[6];
	Aselcolor					*color[2];
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Aflashm::CI			= ACI("Aflashm",		GUID(0x50000000,0x00000857), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aflashm::Aflashm(QIID qiid, char *name, Aobject *L, int x, int y, int w, int h) : Aobject(name, L, x, y, w, h)
{
	static QIID qiidpad[]={ 0x7f32800e02bae279, 0x605a6739212b5400, 0x13bb9a14188eab30,
							0x1af461510a561440, 0x34523c9ca854df68, 0x556757cea9a5c0e0};
	Aresobj	o12=resdll.get(MAKEINTRESOURCE(PNG_PAD12), "PNG");
	Aresobj	o16=resdll.get(MAKEINTRESOURCE(PNG_PAD16), "PNG");
	char	str[256];
	int	i;
	for(i=0; i<6; i++)
	{
		bool	b=i>=3;
		sprintf(str, "flash controller %d  [elek_pad%d]", i+1, i+1);
		if(b)
			pad[i]=new Apaddle(MKQIID(qiid,qiidpad[i]), str, this, 30+i*14+8, 22, paddleELEK12);
			//pad[i]=new Apaddle(str, this, i*14+4, 21, 8, 13, &o12, 8, 13, paddleY);
		else	
			pad[i]=new Apaddle(MKQIID(qiid,qiidpad[i]), str, this, 30+i*12, 15, 8, 17, &o16, 8, 17, paddleY);
			//pad[i]=new Apaddle(str, this, i*20, 17, 24, 24, &o24, 24, 24, paddleY);
		pad[i]->setTooltips(str);
		pad[i]->set(b?0.5f:0.f);
		pad[i]->show(TRUE);
	}

	color[0]=new Aselcolor(MKQIID(qiid,0xc744187f1a54a4b8), "color #0", this, 0, 10, 12 ,12);
	color[0]->setTooltips("color #0");
	color[0]->set(0xff000000);
	color[0]->show(true);

	color[1]=new Aselcolor(MKQIID(qiid,0x2a985c8fe3f08824), "color #1", this, 0, 28, 12 ,12);
	color[1]->setTooltips("color #1");
	color[1]->set(0xffffffff);
	color[1]->show(true);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aflashm::~Aflashm()
{
	int	i;
	for(i=0; i<6; i++)
		delete(pad[i]);
	delete(color[0]);
	delete(color[1]);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct sHWLIST
{
	HWND						hwnd;
	HHOOK						hookin;
	HHOOK						hookout;
	sHWLIST						*next;	
} HWLIST;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int						count=0;
Aresource						resdll=Aresource("flashp", GetModuleHandle("flashp.dll"));

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define						KEYFLASH			

extern SDLLHook				USER32hook;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef BOOL WINAPI			FEndPaint(HWND, CONST LPPAINTSTRUCT);
typedef HDC WINAPI			FBeginPaint(HWND, LPPAINTSTRUCT);

Asection					sectionG;
Asection					sflash;
CShockwaveFlash				*flash[128];
int							nbflash=0;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HDC WINAPI MyBeginPaint(HWND hwnd, LPPAINTSTRUCT lpPaint)
{
	HDC	hdc=null;
	int	i;
	sflash.enter(__FILE__,__LINE__);
	for(i=0; i<nbflash; i++)
		if(flash[i]->hw==hwnd)
		{
			flash[i]->section.enter(__FILE__,__LINE__);
			hdc=flash[i]->hdc;
			break;
		}
	sflash.leave();
	if(hdc)
		return hdc;
	else
		return ((FBeginPaint *)(void *)USER32hook.Functions[0].OrigFn)(hwnd, lpPaint);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI MyEndPaint(HWND hwnd, PAINTSTRUCT *lpPaint)
{
	bool	ret=false;
	int		i;
	sflash.enter(__FILE__,__LINE__);
	for(i=0; i<nbflash; i++)
		if(flash[i]->hw==hwnd)
		{
			flash[i]->section.leave();
			ret=true;
			break;
		}
	sflash.leave();
	if(ret)
		return true;
	else
		return ((FEndPaint *)(void *)USER32hook.Functions[1].OrigFn)(hwnd, lpPaint);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SDLLHook USER32hook = 
{
    "USER32.DLL",
    false, NULL,		// Default hook disabled, NULL function pointer.
    {
        { "BeginPaint", MyBeginPaint, null },
        { "EndPaint", MyEndPaint, null },
        { NULL, NULL, null }
    }
};

HMODULE	hFlashOcx=NULL;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void init()
{
	if(count==0)
	{
		CoInitialize(NULL);
		AfxEnableControlContainer();
		{
			Aregistry	*reg=new Aregistry(FLASHOCXKEY, Aregistry::CLASSES_ROOT);
			bool		b=false;
			char		path[1024];
			memset(path, 0, sizeof(path));
			b=reg->read("", path, sizeof(path));
			delete(reg);
			if(b)
				hFlashOcx=LoadLibrary(path);
			else
				hFlashOcx=LoadLibrary("C:\\WINDOWS\\system32\\Macromed\\Flash\\flash.ocx");
			if(hFlashOcx)
				HookAPICalls(&USER32hook, hFlashOcx, true);
		}
	}
	count++;
	
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void end()
{
	count--;
	if(count==0)
	{
		CoUninitialize();
		if(hFlashOcx)
		{
			HookAPICalls(&USER32hook, hFlashOcx, false);
			FreeLibrary(hFlashOcx);
			hFlashOcx=null;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aflashp::Aflashp(QIID qiid, char *name, AeffectInfo *info, Acapsule *capsule) : Aeffect(qiid, name, info, capsule)
{
	swf=null;
	init();	// global init

	text=strdup("ELEKTRONIKA LIVE");

	front=new AflashFront(qiid, "flash front", this, 50);
	front->setTooltips("flash module");
	back=new AflashBack(qiid, "flash back", this, 50);
	back->setTooltips("flash module");
	
	

	//wflash=new Awflash(this, "flash", 400, 0, getVideoWidth(), getVideoHeight());
	initSWF(getVideoWidth(), getVideoHeight());
	if(swf)
	{
		swf->SetMenu(false);
		{
			long v=swf->FlashVersion();
			if(v<70000)
			{
				char str[1024];
				sprintf(str, "current flash version %x.%x [need 7.0 or more, please upgrade, visit macromedia.com]", (v>>16), (v&0xffff));
				delete(swf);
				swf=null;
				front->notify(front, nyERROR, (dword)str);
			}
		}
	}
	else
		front->notify(front, nyERROR, (dword)"flash plugin not installed [visit the macromedia web site to download it]");
	memset(filename, 0, sizeof(filename));
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aflashp::~Aflashp()
{
	delSWF();
	free(text);
	end();	// global free
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aflashp::save(class Afile *f)
{
	int	i;
	for(i=0; i<8; i++)
	{
		if(filename[i][0])
		{
			char	relpath[ALIB_PATH];
			bool	b=true;
			f->write(&b, sizeof(b));
			if(f->relativePath(relpath, filename[i]))
				f->writeString(relpath);
			else
				f->writeString(filename[i]);
		}
		else
		{
			bool	b=false;
			f->write(&b, sizeof(b));
		}
	}
	f->writeString(text);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aflashp::load(class Afile *f)
{
	int	i;
	for(i=0; i<8; i++)
	{
		bool	b;
		f->read(&b, sizeof(b));
		if(b)
		{
			char	path[ALIB_PATH];
			char	abspath[ALIB_PATH];
			f->readString(path);
			if(f->absoluPath(abspath, path))
				strcpy(filename[i], abspath);
			else
				strcpy(filename[i], path);
		}
	}
	{
		if(text)
			free(text);
		text=NULL;
		f->readStringAlloc(&text);
	}
	((AflashFront *)front)->loadflash();
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aflashp::savePreset(class Afile *f)
{
	int	i;
	for(i=0; i<8; i++)
	{
		int	n=-1;
		if(filename[i][0])
			n=addPresetFile(filename[i]);
		f->write(&n, sizeof(n));
	}
	f->writeString(text);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aflashp::loadPreset(class Afile *f)
{
	int	i;
	for(i=0; i<8; i++)
	{
		int	n; 
		f->read(&n, sizeof(n));
		if(n!=-1)
			getPresetFile(n, filename[i]);
		else
			filename[i][0]=0;
	}
	{
		if(text)
			free(text);
		text=NULL;
		f->readStringAlloc(&text);
	}
	((AflashFront *)front)->loadflash();
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aflashp::settings(bool emergency)
{
	AflashFront		*front=(AflashFront *)this->front;
	if(swf)
	{
		delSWF();
		initSWF(getVideoWidth(), getVideoHeight());
	}
	//((AflashFront *)front)->loadflash();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aflashp::actionStart(double time)
{
	AflashFront		*front=(AflashFront *)this->front;
	sectionG.enter(__FILE__,__LINE__);
	front->flashmode=AflashFront::modeSTART;
	sectionG.leave();
}

void Aflashp::actionStop()
{
	AflashFront		*front=(AflashFront *)this->front;
	sectionG.enter(__FILE__,__LINE__);
	front->flashmode=AflashFront::modeSTOP;
	sectionG.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aflashp::action(double time, double dtime, double beat, double dbeat)
{
	AflashFront		*front=(AflashFront *)this->front;
	AflashBack		*back=(AflashBack *)this->back;
	Avideo			*out=back->out;
	Asample			*in=back->in;
	int				i;

	if(in->isConnected())
	{
		int		mbas=0;
		int		mmed=0;
		int		mtre=0;
		sword	smp[128*2];
		sword	bas[128*2];
		sword	med[128*2];
		sword	tre[128*2];
		int		avaible;
		int		sget;

		in->enter(__FILE__,__LINE__);
		avaible=in->avaible();
		avaible-=in->skip(maxi((avaible-2048)>>1, 0));
		sget=in->getBuffer(smp, bas, med, tre, mini(avaible>>5, 128));
		in->leave();

		sget<<=1;	// stereo

		for(i=0; i<sget; i++)
		{
			mbas=max(mbas, abs((int)bas[i]));
			mmed=max(mmed, abs((int)med[i]));
			mtre=max(mtre, abs((int)tre[i]));
		}
		
		bass=mbas;
		medium=mmed;
		treble=mtre;
	}
	else
	{
		bass=0;
		medium=0;
		treble=0;
	}
	this->time=time;
	this->beat=beat;

	sectionG.enter(__FILE__,__LINE__);
	if(swf)
	{
		out->enter(__FILE__,__LINE__);
		{
			Abitmap *b=out->getBitmap();
			if(b)
				getBitmap(b);
		}
		out->leave();
	}
	sectionG.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aflashp::command(char *name, char *value)
{
	AflashFront	*front=(AflashFront *)this->front;
	front->sectionC.enter(__FILE__,__LINE__);
	if(!strcmp(name, "visible"))
	{
		strncpy(front->flashvisible, value, 15);
		front->flashvisible[14]=0;
	}
	else if(strstr(name, "tooltips"))
	{
		int	n=name[8]-'0';
		strncpy(front->flashtips[n], value, 127);
		front->flashtips[n][126]=0;
	}
	front->sectionC.leave();
	front->asyncNotify(this, nyCHANGE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AflashFront::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AflashFront::AflashFront(QIID qiid, char *name, Aflashp *e, int h) : AeffectFront(qiid, name, e, h)
{
	static qword  qidfm[]={	0xe8267c66fdeb3246, 0x3222222f551a4d80, 0xf82cf598cddf1c80, 0x0c4567a710df3ec0,
							0x46cf12eff12b0c24, 0xc4a882ee33db7f40, 0x7fcded47ecf2f984, 0xe56d7c03151bb978};
	Aresobj	o=resdll.get(MAKEINTRESOURCE(PNG_FRONT), "PNG");
	int		i;
	back=new Abitmap(&o);

	mtext=NULL;
	flashmode=modeSTOPPED;

	for(i=0; i<8; i++)
	{
		char	str[256];
		sprintf(str, "display flash movie #%d", i+1);
		flashm[i]=new Aflashm(MKQIID(qiid, qidfm[i]), str, this, 122, 0, 200, pos.h);
		flashm[i]->setTooltips(str);
	}
	flashm[0]->show(true);

	select=new Aselect(MKQIID(qiid, 0x66183f2a186bc5c0), "select flash movie", this, 18, 12, 4, 2, &resdll.get(MAKEINTRESOURCE(PNG_SELECT), "PNG"), 12, 12, dragdropFILE);
	select->setTooltips("select flash movie");
	select->set(0);
	select->show(TRUE);

	file=new Astatic("affiche", this, 192, 10, 130, 12);
	file->show(true);
	file->set("NOTHING", 0xff404040, alib.getFont(fontTERMINAL06), Astatic::RIGHT);
	file->setTooltips("movie filename");

	vtext=new Astatic("vtext", this, 250, 24, 74, 12);
	vtext->show(true);
	vtext->set("", 0xff404040, alib.getFont(fontTERMINAL06), Astatic::LEFT);
	vtext->setTooltips("text preview");

	zoom=new Apaddle(MKQIID(qiid, 0xeddcc49f59940d20), "zoom", this, 82, 12, paddleYOLI24);
	zoom->setTooltips("zoom");
	zoom->set(0.f);
	zoom->show(TRUE);

	btext=new Abutton("btext", this, 350, 16, 50, pos.h-32, "TEXT");
	btext->setTooltips("edit text");
	btext->show(TRUE);

	timer(40);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AflashFront::~AflashFront()
{
	int	i;
	if(mtext)
		delete(mtext);
	for(i=0; i<8; i++)
		delete(flashm[i]);
	delete(vtext);
	delete(btext);
	delete(zoom);
	delete(file);
	delete(select);
	delete(back);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AflashFront::updatePad()
{
	Aflashp			*flashp=(Aflashp *)effect;
	int				nm=select->get();
	Aflashm			*fm=flashm[nm];
	dword			col0=fm->color[0]->get();
	dword			col1=fm->color[1]->get();
	int				np[6];
	int				i;

	for(i=0; i<6; i++)
		np[i]=(int)(fm->pad[i]->get()*127.f);

	sectionG.enter(__FILE__,__LINE__);
	if(flashp->swf)
	{
		flashp->swf->section.enter(__FILE__,__LINE__);
		if(flashmode==modeRUNNING)
		{
			char	str[256];
			char	nam[256];
			float	p=(float)(flashp->beat-(int)flashp->beat);
			float	pp=(p<0.5f)?(1.f-p*2.f):((p-0.5f)*2.f);
			sprintf(str, "%f", flashp->beat);
			flashp->swf->SetVariable("elek_beat", str);
			sprintf(str, "%f", pp);
			flashp->swf->SetVariable("elek_pulse", str);
			sprintf(str, "%d", flashp->time);
			flashp->swf->SetVariable("elek_time", str);
			sprintf(str, "%d", flashp->bass);
			flashp->swf->SetVariable("elek_bass", str);
			sprintf(str, "%d", flashp->medium);
			flashp->swf->SetVariable("elek_medium", str);
			sprintf(str, "%d", flashp->treble);
			flashp->swf->SetVariable("elek_treeble", str);
			sprintf(str, "0x%x", col0);
			flashp->swf->SetVariable("elek_color0", str);
			sprintf(str, "0x%x", col1);
			flashp->swf->SetVariable("elek_color1", str);
			for(i=0; i<6; i++)
			{
				sprintf(nam, "elek_pad%d", i);
				sprintf(str, "%d", np[i]);
				flashp->swf->SetVariable(nam, str);
			}
		}
		flashp->swf->section.leave();
	}
	sectionG.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AflashFront::pulse()
{
	Aflashp			*flashp=(Aflashp *)effect;
	
	sectionG.enter(__FILE__,__LINE__);
	if(flashp->swf)
	{
		flashp->swf->section.enter(__FILE__,__LINE__);
		switch(flashmode)
		{
			case modeSTART:
			loadflash();
			flashmode=modeRUNNING;
			break;
			case modeSTOP:
			{
				char	nullname[ALIB_PATH];
				strcpy(nullname, effect->getRootPath());
				strcat(nullname, "\\bin\\null.swf");
				flashp->swf->put_Movie(nullname);
			}
			flashmode=modeSTOPPED;
			break;
		}
		flashp->swf->section.leave();
	}
	sectionG.leave();

	updatePad();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AflashFront::loadflash()
{
	Aflashp	*flashp=(Aflashp *)effect;
	int		nm=select->get();
	float	z=zoom->get();
	char	name[ALIB_PATH];
	char	nullname[ALIB_PATH];
	int		i;
	for(i=0; i<6; i++)
		flashm[nm]->pad[i]->show(false);
	strcpy(name, flashp->filename[nm]);
	strupr(name);
	sectionC.enter(__FILE__,__LINE__);
	strcpy(flashvisible, "000000");
	memset(flashtips, 0, sizeof(flashtips));
	sectionC.leave();
	strcpy(nullname, flashp->getRootPath());
	strcat(nullname, "\\bin\\null.swf");
	sectionG.enter(__FILE__,__LINE__);
	if(flashp->swf)
	{
		flashp->swf->section.enter(__FILE__,__LINE__);
		flashp->swf->put_Movie(nullname);
		flashp->swf->put_Movie(name);
		flashp->swf->Zoom(0);
		flashp->swf->Zoom((int)(100.f-z*99.f));
		flashp->swf->SetVariable("elek_text", flashp->text);
		flashp->swf->SetVariable("text", flashp->text);
		flashp->swf->section.leave();
		updatePad();
	}
	sectionG.leave();
	file->set(name, 0xff404040, alib.getFont(fontTERMINAL06), Astatic::RIGHT);
	vtext->set(flashp->text, 0xff404040, alib.getFont(fontTERMINAL06), Astatic::LEFT);
	sectionC.enter(__FILE__,__LINE__);
	for(i=0; i<8; i++)
		flashm[i]->show(i==nm);
	sectionC.leave();
	flashm[nm]->repaint();
	notify(flashp, nyCHANGE, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AflashFront::notify(Anode *o, int event, dword p)
{
	Aflashp	*flashp=(Aflashp *)effect;
	switch(event)
	{
		case nySELECTDROP:
		assert(o==select);
		{
			SelectDragDrop	*sdd=(SelectDragDrop *)p;
			assert(sdd->state&dragdropFILE);
			computeDrop(sdd->n, sdd->data);
		}
		return true;
		
		case nyCHANGE:
		if(o==flashp)
		{
			sectionC.enter(__FILE__,__LINE__);
			{
				int		nm=select->get();
				Aflashm	*fm=flashm[nm];
				int		i;
				char	*s=flashvisible;
				for(i=0; i<6; i++)
				{
					fm->pad[i]->show(*s!='0');
					if(flashtips[i][0])
						fm->pad[i]->setTooltips(flashtips[i]);
					else
					{
						char	str[256];
						sprintf(str, "flash controller %d  [elek_pad%d]", i+1, i+1);
						fm->pad[i]->setTooltips(str);
					}
					s++;
				}
			}
			sectionC.leave();
			repaint();
		}
		else if(o==select)
		{
			bool	b=flashp->getTableIsRunning();
			loadflash();
			if(!b)
			{
				char	nullname[ALIB_PATH];
				strcpy(nullname, flashp->getRootPath());
				strcat(nullname, "\\bin\\null.swf");
				sectionG.enter(__FILE__,__LINE__);
				if(flashp->swf)
				{
					flashp->swf->section.enter(__FILE__,__LINE__);
					flashp->swf->put_Movie(nullname);
					flashp->swf->section.leave();
				}
				sectionG.leave();
			}
		}
		else if(o==zoom)
		{
			float	z=zoom->get();
			sectionG.enter(__FILE__,__LINE__);
			if(flashp->swf)
			{
				flashp->swf->section.enter(__FILE__,__LINE__);
				flashp->swf->Zoom(0);
				flashp->swf->Zoom((int)(100.f-z*99.f));
				flashp->swf->section.leave();
			}
			sectionG.leave();
		}
		else if(o==mtext)
		{
			if(flashp->text)
				free(flashp->text);
			flashp->text=strdup(mtext->edit->caption);
			sectionG.enter(__FILE__,__LINE__);
			if(flashp->swf)
			{
				flashp->swf->section.enter(__FILE__,__LINE__);
				flashp->swf->SetVariable("elek_text", flashp->text);
				flashp->swf->SetVariable("text", flashp->text);
				flashp->swf->section.leave();
			}
			sectionG.leave();
			mtext->destroy();
			mtext=NULL;
			vtext->set(flashp->text, 0xff404040, alib.getFont(fontTERMINAL06), Astatic::LEFT);
		}
		break;

		case nyPRESS:
		if(o==btext)
		{
			mtext=new Amtext("mtext", this, 0, 0, pos.w, pos.h);
			mtext->setTooltips("text edit");
			mtext->edit->set(flashp->text);
			mtext->show(true);
			repaint();
		}
		break;
	}
	return AeffectFront::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AflashFront::dragdrop(int x, int y, int state, int event, void *data)
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
			computeDrop(select->get(), data);
			return true;
		}
		break;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AflashFront::computeDrop(int n, void *data)
{
	char	*s=(char *)data;
	int		nb=8;
	int		ns=select->get();
	int		n0=n;
	bool	bload=false;
	Aflashp	*flashp=(Aflashp *)effect;
	while(*s&&(nb--))
	{
		if(strstr(s, ".swf"))
			strcpy(flashp->filename[n], s);
		else
			notify(this, nyERROR, (dword)"error loading file");
		s+=strlen(s)+1;
		if(n==ns)
			bload=true;
		n=(n+1)&7;
	}
	if(bload)
	{
		loadflash();
		if(!flashp->getTableIsRunning())
		{
			char	nullname[ALIB_PATH];
			strcpy(nullname, flashp->getRootPath());
			strcat(nullname, "\\bin\\null.swf");
			sectionG.enter(__FILE__,__LINE__);
			if(flashp->swf)
			{
				flashp->swf->section.enter(__FILE__,__LINE__);
				flashp->swf->put_Movie(nullname);
				flashp->swf->section.leave();
			}
			sectionG.leave();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AflashBack::AflashBack(QIID qiid, char *name, Aflashp *e, int h) : AeffectBack(qiid, name, e, h)
{
	Aresobj	o=resdll.get(MAKEINTRESOURCE(PNG_BACK), "PNG");
	back=new Abitmap(&o);

	out=new Avideo(MKQIID(qiid, 0x07a0b8e25f338d00), "video output", this, pinOUT, pos.w-18, 10);
	out->setTooltips("video output");
	out->show(TRUE);

	in=new Asample(MKQIID(qiid, 0x340f4574a5c3145f), "audio in", this, pinIN, 10, 10);
	in->setTooltips("audio intput");
	in->show(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AflashBack::~AflashBack()
{
	delete(in);
	delete(out);
	delete(back);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AflashBack::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aeffect * AflashInfo::create(QIID qiid, char *name, Acapsule *capsule)
{
	//AFX_MANAGE_STATE(AfxGetStaticModuleState()) 
	return new Aflashp(qiid, name, this, capsule);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" 
{
	DLLEXPORT class Aplugz * getPlugz()
	{
		//AFX_MANAGE_STATE(AfxGetStaticModuleState());
		return new AflashInfo("flash", &Aflashp::CI, "flash player", "flash player");
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

afx_msg void CShockwaveFlash::FSCommand(LPSTR name, LPSTR value)
{
	flashp->command(name, value);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef LRESULT CALLBACK		FMyWndProc(HWND hw, UINT mes, WPARAM wp, LPARAM lp);

typedef struct TWndProc
{
	HWND						hw;
	Aflashp						*flashp;
	FMyWndProc					*oldProc;
	
								TWndProc()
								{
									hw=null;
									flashp=null;
									oldProc=null;
								};
} TWndProc;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Asection						sWndProc;
TWndProc						WndProc[128];
int								nbWndProc=0;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK MyWndProc(HWND hw, UINT mes, WPARAM wp, LPARAM lp)
{
	int	i=0;
	sWndProc.enter(__FILE__,__LINE__);
	for(i=0; i<nbWndProc; i++)
	{
		if(WndProc[i].hw==hw)
		{
			FMyWndProc	*fp=WndProc[i].oldProc;
			switch(mes)
			{
				case WM_PAINT:
				{
					int	a=5;
					a++;
				}
				break;
				
				case WM_TIMER:
				{
					int	a=5;
					a++;
					SendMessage(hw, WM_PAINT, 0, 0);
				}
				break;
			}
			sWndProc.leave();
			return fp(hw, mes, wp, lp);
		}
	}
	sWndProc.leave();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aflashp::initSWF(int w, int h)
{
	RECT	r;
	Awindow	*win=front->getWindow();
	
	r.top=0;
	r.left=0;
	r.bottom=h;
	r.right=w;

	swf=new CShockwaveFlash();
	swf->flashp=this;
	
	if(swf->Create("flash", 0, r, CWnd::FromHandle(win->hw), IDSHOCK))
	{
		HWND	hwnd=swf->GetSafeHwnd();
		DWORD	tid;
		HDC		hdc=GetDC(hwnd);
		sWndProc.enter(__FILE__,__LINE__);
		WndProc[nbWndProc].oldProc=(FMyWndProc *)(void *)SetWindowLongPtr(hwnd, GWL_WNDPROC, (LONG)(void *)MyWndProc);
		WndProc[nbWndProc].hw=hwnd;
		WndProc[nbWndProc].flashp=this;
		nbWndProc++;
		sWndProc.leave();
		swf->hw=hwnd;
		swf->hdc=CreateCompatibleDC(hdc);
		swf->hbmp=CreateCompatibleBitmap(hdc, w, h);
		HBITMAP	obmp=(HBITMAP)SelectObject(swf->hdc, swf->hbmp);
		ReleaseDC(hwnd, hdc);

		HINSTANCE	hmod=(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE);
		tid=GetCurrentThreadId();

		sflash.enter(__FILE__,__LINE__);
		flash[nbflash++]=swf;
		sflash.leave();
	}
	else
	{
		delete(swf);
		swf=null;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aflashp::delSWF()
{
	{
		sflash.enter(__FILE__,__LINE__);
		swf->section.enter(__FILE__,__LINE__);
		{
			int	i;
			for(i=0; i<nbflash; i++)
			{
				if(flash[i]==swf)
				{
					int	j;
					nbflash--;
					for(j=i; j<nbflash; j++)
						flash[j]=flash[j+1];
					break;
				}
			}
		}
		swf->section.leave();
		sflash.leave();
		DeleteObject(swf->hbmp);
		DeleteDC(swf->hdc);
	}
	sWndProc.enter(__FILE__,__LINE__);
	{
		int	i;
		for(i=0; i<nbWndProc; i++)
		{
			if(WndProc[i].flashp==this)
			{
				int	j;
				nbWndProc--;
				for(j=i; j<nbWndProc; j++)
					WndProc[i]=WndProc[i-1];
				break;
			}
		}
	}
	sWndProc.leave();
	delete(swf);
	swf=null;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aflashp::getBitmap(Abitmap *b)
{
	HWND	hwnd=swf->GetSafeHwnd();
	
	swf->section.enter(__FILE__,__LINE__);
	{
		BITMAPINFOHEADER	bmi;
		memset(&bmi, 0, sizeof(bmi));
		bmi.biSize=sizeof(bmi);
		bmi.biWidth=b->w;
		bmi.biHeight=b->h;
		bmi.biPlanes=1;
		bmi.biBitCount=b->nbBits;
		bmi.biCompression=BI_RGB;
		GetDIBits(swf->hdc, swf->hbmp, 0, b->h, b->body, (BITMAPINFO *)&bmi, DIB_RGB_COLORS);
	}
	swf->section.leave();
	b->flipY();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
