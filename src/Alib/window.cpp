/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"export.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	BITMAP.CPP					(c)	YoY'99						WEB: search aestesis
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<windows.h>
#include						<stdio.h>
#include						<zmouse.h>
#include						<assert.h>
#include						<shlobj.h>
#include						"interface.h"
#include						"window.h"
#include						"bitmap.h"
#include						"resource.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define							CLASSNAME						"YoYwindow"

#ifndef GET_X_LPARAM
#define							GET_X_LPARAM(x)					((int)((short)LOWORD(x)))
#endif

#ifndef GET_Y_LPARAM
#define							GET_Y_LPARAM(y)					((int)((short)HIWORD(y)))
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef WS_EX_LAYERED
#define WS_EX_LAYERED           0x00080000
#define LWA_COLORKEY            0x00000001
#define LWA_ALPHA               0x00000002
//#define	AC_SRC_OVER				0
//#define	AC_SRC_ALPHA			1
#define	ULW_COLORKEY			1 
#define	ULW_ALPHA				2 
#endif // ndef WS_EX_LAYERED

typedef BOOL (WINAPI *lpfnSetLayeredWindowAttributes)(HWND hWnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);
typedef BOOL (WINAPI *lpfnUpdateLayeredWindow)(HWND hwnd, HDC hdcDst, POINT *pptDst, SIZE *psize, HDC hdcSrc, POINT *pptSrc, COLORREF crKey, BLENDFUNCTION *pblend, DWORD dwFlags);
 
lpfnSetLayeredWindowAttributes	m_pSetLayeredWindowAttributes;
lpfnUpdateLayeredWindow			m_pUpdateLayeredWindow;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Asection					safeGUI;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// IDropTarget interface

class Adroptarget : public IDropTarget
{
public:
	int							nrefdrop;
	Awindow						*w;

								Adroptarget						()	{ w=NULL; nrefdrop=0; }
								~Adroptarget					()  { assert(nrefdrop==0); }

	void						setWindow						(Awindow *w) { this->w=w; }

	ADLL HRESULT __stdcall		QueryInterface					(REFIID riid, void **ppv);
	ULONG __stdcall				AddRef							()	{ InterlockedIncrement((LONG *)&nrefdrop); return nrefdrop; }
	ULONG __stdcall				Release							()	{ InterlockedDecrement((LONG *)&nrefdrop); return nrefdrop; }

	ADLL HRESULT __stdcall		DragEnter						(IDataObject *pDataObject, DWORD grfKeyState, _POINTL pt, DWORD *pdwEffect);
	ADLL HRESULT __stdcall		DragOver						(DWORD grfKeyState, _POINTL pt, DWORD *pdwEffect);
	ADLL HRESULT __stdcall		DragLeave						();
	ADLL HRESULT __stdcall		Drop							(IDataObject *pDataObject, DWORD grfKeyState, _POINTL pt, DWORD *pdwEffect);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// IDropSource interface

class Adropsource : public IDropSource
{
public:
	int							nrefdrop;

								Adropsource						()	
								{ 
									nrefdrop=0; 
								}
								~Adropsource					()
								{
									assert(nrefdrop==0);
								}

	ADLL HRESULT __stdcall		QueryInterface					(REFIID riid, void **ppv);
	ULONG __stdcall				AddRef							()	{ InterlockedIncrement((LONG *)&nrefdrop); return nrefdrop; }
	ULONG __stdcall				Release							()	{ if(InterlockedDecrement((LONG *)&nrefdrop)<=0) { delete(this); return 0; } return nrefdrop; }

	ADLL HRESULT __stdcall		QueryContinueDrag				(BOOL fEscapePressed, DWORD grfKeyState);
	ADLL HRESULT __stdcall		GiveFeedback					(DWORD dwEffect);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// IDataObject interface

class AdropString : public IDataObject
{
public:
	int							nrefdrop;
	HGLOBAL						text;

	ADLL						AdropString						(char *str);
	ADLL virtual				~AdropString					();

	ADLL HRESULT __stdcall		QueryInterface					(REFIID riid, void **ppv);
	ULONG __stdcall				AddRef							()	{ InterlockedIncrement((LONG *)&nrefdrop); return nrefdrop; }
	ULONG __stdcall				Release							()	{ InterlockedDecrement((LONG *)&nrefdrop); return nrefdrop; }

	ADLL HRESULT __stdcall		GetData							(FORMATETC *pFormatEtc, STGMEDIUM *pmedium);
    ADLL HRESULT __stdcall		GetDataHere						(FORMATETC *pFormatEtc, STGMEDIUM *pmedium);
    ADLL HRESULT __stdcall		QueryGetData					(FORMATETC *pFormatEtc);
    ADLL HRESULT __stdcall		GetCanonicalFormatEtc			(FORMATETC *pFormatEct, FORMATETC *pFormatEtcOut);
    ADLL HRESULT __stdcall		SetData							(FORMATETC *pFormatEtc, STGMEDIUM *pMedium,  BOOL fRelease);
    ADLL HRESULT __stdcall		EnumFormatEtc					(DWORD dwDirection, IEnumFORMATETC **ppEnumFormatEtc);
    ADLL HRESULT __stdcall		DAdvise							(FORMATETC *pFormatEtc, DWORD advf, IAdviseSink *, DWORD *);
    ADLL HRESULT __stdcall		DUnadvise						(DWORD dwConnection);
    ADLL HRESULT __stdcall		EnumDAdvise						(IEnumSTATDATA **ppEnumAdvise);
};

class AdropHdrop : public IDataObject
{
public:
	int							nrefdrop;
	HGLOBAL						hgDrop;

	ADLL						AdropHdrop						(char *files);
	ADLL virtual				~AdropHdrop						();

	ADLL HRESULT __stdcall		QueryInterface					(REFIID riid, void **ppv);
	ULONG __stdcall				AddRef							()	{ InterlockedIncrement((LONG *)&nrefdrop); return nrefdrop; }
	ULONG __stdcall				Release							()	{ InterlockedDecrement((LONG *)&nrefdrop); return nrefdrop; }

	ADLL HRESULT __stdcall		GetData							(FORMATETC *pFormatEtc, STGMEDIUM *pmedium);
    ADLL HRESULT __stdcall		GetDataHere						(FORMATETC *pFormatEtc, STGMEDIUM *pmedium);
    ADLL HRESULT __stdcall		QueryGetData					(FORMATETC *pFormatEtc);
    ADLL HRESULT __stdcall		GetCanonicalFormatEtc			(FORMATETC *pFormatEct, FORMATETC *pFormatEtcOut);
    ADLL HRESULT __stdcall		SetData							(FORMATETC *pFormatEtc, STGMEDIUM *pMedium,  BOOL fRelease);
    ADLL HRESULT __stdcall		EnumFormatEtc					(DWORD dwDirection, IEnumFORMATETC **ppEnumFormatEtc);
    ADLL HRESULT __stdcall		DAdvise							(FORMATETC *pFormatEtc, DWORD advf, IAdviseSink *, DWORD *);
    ADLL HRESULT __stdcall		DUnadvise						(DWORD dwConnection);
    ADLL HRESULT __stdcall		EnumDAdvise						(IEnumSTATDATA **ppEnumAdvise);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Atasklist
{
public:
	class Atasklist				*prev;
	class Atasklist				*next;
	int							time;
	int							inc;
	Aobject						*o;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void							timerFunction();

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Awindowlist
{
public:
	static Awindowlist			*list;
	static Asection				section;

	static void					add(Awindow *w);
	static void					del(Awindow *w);

	static void					showOff();

	class Awindowlist			*next;
	Awindow						*w;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Awindowlist						* Awindowlist::list=NULL;
Asection						Awindowlist::section;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Awindowlist::add(Awindow *w)
{
	section.enter(__FILE__, __LINE__);
	Awindowlist	*wl=new Awindowlist();
	wl->w=w;
	wl->next=list;
	list=wl;
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Awindowlist::del(Awindow *w)
{
	section.enter(__FILE__, __LINE__);
	{
		Awindowlist	*wl=list;
		Awindowlist	*wo=NULL;
		while(wl)
		{
			if(wl->w==w)
			{
				if(wo)
					wo->next=wl->next;
				else
					list=wl->next;
				delete(wl);
				break;
			}
			wo=wl;
			wl=wl->next;
		}
	}
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Awindowlist::showOff()
{
	section.enter(__FILE__, __LINE__);
	{
		Awindowlist	*wl=list;
		while(wl)
		{
			wl->w->show(false);
			wl=wl->next;
		}
	}
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int						nbwindow=0;
static HINSTANCE				hinst=NULL;
bool							messageLoopRUN=TRUE;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL ACI						Awindow::CI=ACI("Awindow", GUID(0xAE57E515,0x00000004), &Asurface::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static HANDLE					ht=NULL;
static Atasklist				*tlistf=NULL;
static Atasklist				*tlistl=NULL;
static int						nbtlist=0;
static Asection					timesection;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void							destroyList();

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static Aidle					*idle=null;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Aidle::Aidle()
{
	safeGUI.enter(__FILE__, __LINE__);
	this->nextIdle=idle;
	idle=this;
	safeGUI.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Aidle::~Aidle()
{
	safeGUI.enter(__FILE__, __LINE__);
	{
		Aidle	*i=idle;
		Aidle	*o=null;
		while(i)
		{
			if(i==this)
			{
				if(o)
					o->nextIdle=nextIdle;
				else
					idle=nextIdle;
				break;
			}
			o=i;
			i=i->nextIdle;
		}
	}
	safeGUI.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void idleFunction()
{
	Aidle	*i=idle;
	while(i)
	{
		Aidle *s=i->nextIdle;
		i->idlePulse();
		i=s;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK wndproc(HWND hw, UINT mes, WPARAM wp, LPARAM lp)
{
	Awindow	*w=(Awindow *)GetWindowLong(hw, GWL_USERDATA);
	switch(mes)
	{
		case WM_APP:
		SetWindowLong(hw, GWL_USERDATA, (dword)0);
		/*
		if(w&&w->useBlend)
		{
			DeleteObject(w->hdc);
			DeleteObject(w->hbm);
		}
		*/
		DestroyWindow(hw);
		break;

		case WM_CREATE:
		{
			CREATESTRUCT	*cs=(CREATESTRUCT *)lp;
			w=(Awindow *)cs->lpCreateParams;
			w->hw=hw;
			SetWindowLong(hw, GWL_USERDATA, (dword)w);
			if(w->useBlend)
			{
				w->hdc=CreateCompatibleDC(NULL);
				w->hbm=CreateBitmap(w->pos.w, w->pos.h, 1, w->bitmap->nbBits, w->bitmap->body);
				SelectObject(w->hdc, w->hbm);
			}
		}
		{
			w->droptarget=new Adroptarget();
			w->droptarget->setWindow(w);
			HRESULT rr=RegisterDragDrop(hw, w->droptarget);
			switch(rr)
			{
				case E_OUTOFMEMORY:
				assert(E_OUTOFMEMORY==0);
				break;

				case DRAGDROP_E_INVALIDHWND:
				assert(DRAGDROP_E_INVALIDHWND==0);
				break;

				case DRAGDROP_E_ALREADYREGISTERED:
				assert(DRAGDROP_E_ALREADYREGISTERED==0);
				break;
 			}
			SetTimer(hw, 1, 200, NULL);
		}
		break;

		case WM_LBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MOUSEWHEEL:		// todo
		case WM_MOUSEMOVE:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_SIZING:
		case WM_SIZE:
		case WM_MOVE:
		case WM_PAINT:
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_CHAR:
		case WM_TIMER:
		case WM_DROPFILES:
		case WM_DEVICECHANGE:
		case WM_SHOWWINDOW:
		case WM_CLOSE:
		if(w)
			return w->NCnotify(mes, wp, lp);
		break;
		
		case WM_ACTIVATE:
		if((LOWORD(wp)!=WA_INACTIVE)&&w&&(w->state&Awindow::stateVISIBLE))
			w->repaint();
		break;
		
		default:
		if(w)
			return w->NCnotify(mes, wp, lp);
		break;
	};
	return DefWindowProc(hw, mes, wp, lp);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static HCURSOR	hcurHand			= NULL;
static HCURSOR	hcurHandKeep		= NULL;
static HCURSOR	hcurHandSel			= NULL;
static HCURSOR	hcurInk				= NULL;
static HCURSOR	hcurMagnify			= NULL;
static HCURSOR	hcurMagnifyAdd		= NULL;
static HCURSOR	hcurMagnifySub		= NULL;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static HCURSOR	customCursor[MAXCUSTOMCURSORS];

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int	icon=Awindow::NCmakeCursor(&alibres.get(MAKEINTRESOURCE(PNG_AESTTV), "PNG"), 0, 0)-cursorCUSTOM;

__inline void openWindow(int n)
{
	if(!n)
	{
		//memset(customCursor, 0, sizeof(customCursor));
		{
			HMODULE hUser32=GetModuleHandle("USER32.DLL");
			m_pSetLayeredWindowAttributes=(lpfnSetLayeredWindowAttributes)GetProcAddress(hUser32, "SetLayeredWindowAttributes");
			m_pUpdateLayeredWindow=(lpfnUpdateLayeredWindow)GetProcAddress(hUser32, "UpdateLayeredWindow");
		}
		{
			WNDCLASS	wc;
			hinst=GetModuleHandle(NULL);
			wc.style=CS_DBLCLKS|(m_pUpdateLayeredWindow?CS_BYTEALIGNCLIENT:(CS_PARENTDC|CS_BYTEALIGNCLIENT));
			wc.lpfnWndProc=wndproc; 
			wc.cbClsExtra=0; 
			wc.cbWndExtra=0; 
			wc.hInstance=hinst; 
			wc.hIcon=customCursor[icon]; //LoadIcon(NULL, IDI_EXCLAMATION);	// IDI_APPLICATION
			wc.hCursor=LoadCursor(NULL, IDC_ARROW);
			wc.hbrBackground=NULL;
			wc.lpszMenuName=NULL;
			wc.lpszClassName=CLASSNAME;
			RegisterClass(&wc);
		}

		hcurHand=		LoadCursor(alibres.hinstance, MAKEINTRESOURCE(IDC_HAND2));
		hcurHandKeep=	LoadCursor(alibres.hinstance, MAKEINTRESOURCE(IDC_HANDKEEP));
		hcurHandSel=	LoadCursor(alibres.hinstance, MAKEINTRESOURCE(IDC_HANDSEL));
		hcurInk=		LoadCursor(alibres.hinstance, MAKEINTRESOURCE(IDC_INK));
		hcurMagnify=	LoadCursor(alibres.hinstance, MAKEINTRESOURCE(IDC_MAGNIFY));
		hcurMagnifyAdd=	LoadCursor(alibres.hinstance, MAKEINTRESOURCE(IDC_MAGNIFY_ADD));
		hcurMagnifySub=	LoadCursor(alibres.hinstance, MAKEINTRESOURCE(IDC_MAGNIFY_SUB));

		OleInitialize(NULL);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__inline void closeWindow(int n)
{
	if(!n)
	{
		int	i;
		for(i=0; i<countof(customCursor); i++)
			if(customCursor[i])
				DestroyCursor(customCursor[i]);
		UnregisterClass(CLASSNAME, hinst);
		OleUninitialize();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Awindow::Awindow(char *name, int x, int y, int w, int h, bool useBlend) : Asurface(name, NULL, x, y, w, h)
{
	Awindowlist::add(this);
	inUpdate=FALSE;
	droptarget=NULL;
	dropType=0;
	openWindow(nbwindow++);
	this->useBlend=useBlend&&m_pUpdateLayeredWindow;
	hw=NULL;
	pos.x=x;
	pos.y=y;
	pos.w=w;
	pos.h=h;
	mouseX=-1;
	mouseY=-1;
	mouseState=0;
	objectTrack=NULL;
	mouseCaptureOBJ=NULL;
	keyboardFocusOBJ=this;
	state|=stateFOCUS;
	mouseTime=GetTickCount();
	CreateWindowEx((this->useBlend?WS_EX_LAYERED:0), CLASSNAME, name, WS_CLIPCHILDREN|WS_POPUP, x, y, w, h, NULL, NULL, hinst, this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Awindow::Awindow(char *name, HWND hf, int x, int y, int w, int h, bool useBlend) : Asurface(name, NULL, x, y, w, h)
{
	Awindowlist::add(this);
	inUpdate=FALSE;
	droptarget=NULL;
	dropType=0;
	openWindow(nbwindow++);
	this->useBlend=useBlend&&m_pUpdateLayeredWindow;
	hw=NULL;
	pos.x=x;
	pos.y=y;
	pos.w=w;
	pos.h=h;
	this->hwFather=hf;
	mouseX=-1;
	mouseY=-1;
	mouseState=0;
	objectTrack=NULL;
	mouseCaptureOBJ=NULL;
	keyboardFocusOBJ=this;
	state|=stateFOCUS;
	mouseTime=GetTickCount();
	CreateWindowEx((this->useBlend?WS_EX_LAYERED:0), CLASSNAME, name, WS_CLIPCHILDREN|WS_POPUP, x, y, w, h, hf, NULL, hinst, this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Awindow::~Awindow()
{
	Awindowlist::del(this);
	if(mouseCaptureOBJ)
		mouseCapture(false, NULL);
	//SetWindowLong(hw, GWL_USERDATA, 0);
	KillTimer(hw, 1);
	SetWindowLong(hw, GWL_USERDATA, (dword)0);
	if(useBlend)
	{
		DeleteObject(hdc);
		DeleteObject(hbm);
	}
	RevokeDragDrop(hw);
	delete(droptarget);
	PostMessage(hw, WM_APP, 0, 0);
	hw=NULL;
	closeWindow(--nbwindow);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static DWORD guiThreadID;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Awindow::NCmessageLoop()
{
	MSG msg;
	messageLoopRUN=TRUE;
	memset(&msg, 0, sizeof(msg));
	guiThreadID=GetCurrentThreadId();
	while(msg.message!=WM_QUIT)
	{
		int		time=GetTickCount();
		safeGUI.enter(__FILE__,__LINE__);
		while(PeekMessage(&msg, NULL, 0, 0, TRUE)&&(msg.message!=WM_QUIT))
		{
			if(!msg.hwnd)
			{
				if(msg.message==WM_APP)
				{
					{
						asyncMessage	*am=(asyncMessage *)msg.lParam;
						am->to->notify(am->o, am->event, am->p);
						delete(am);
					}
				}
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			if((int)GetTickCount()-time>5)
				break;
		}
		safeGUI.leave();
		safeGUI.enter(__FILE__,__LINE__);
		timerFunction();
		safeGUI.leave();
		safeGUI.enter(__FILE__,__LINE__);
		destroyList();
		safeGUI.leave();
		safeGUI.enter(__FILE__,__LINE__);
		idleFunction();
		safeGUI.leave();
		Sleep(1);
	}
	messageLoopRUN=FALSE;
	safeGUI.enter(__FILE__,__LINE__);
	destroyList();
	safeGUI.leave();
	return msg.wParam;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Awindow::NCshowOff()
{
	Awindowlist::showOff();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Awindow::NCasyncNotify(class asyncMessage *am)
{
	PostThreadMessage(guiThreadID, WM_APP, 0, (LPARAM)am);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Awindow::minimize(bool min)
{
	return ShowWindow(hw, min?SW_MINIMIZE:SW_RESTORE)?TRUE:FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Awindow::isMinimized()
{
	WINDOWPLACEMENT	wp;
	GetWindowPlacement(hw, &wp);
	return (wp.showCmd&SW_MINIMIZE)?TRUE:FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Awindow::show(bool visible)
{
	Aobject::show(visible);
	return ShowWindow(hw, visible?SW_SHOW:SW_HIDE)?TRUE:FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Awindow::move(int x, int y)
{
	if(SetWindowPos(hw, NULL, x, y, 0, 0, SWP_NOSIZE|SWP_NOZORDER))
	{
		pos.x=x;
		pos.y=y;
		return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Awindow::size(int w, int h)
{
	if(!(state&stateNOSURFACESIZEUPDATE))
		Asurface::size(w, h);
	else
	{
		pos.w=w;
		pos.h=h;
	}
	SetWindowPos(hw, NULL, 0, 0, w, h, SWP_NOMOVE|SWP_NOZORDER);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Awindow::zorder(int action)
{
	switch(action)
	{
		case zorderFRONT:
		state&=~stateTOP;
		SetWindowPos(hw, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
		break;

		case zorderBACK:
		state&=~stateTOP;
		SetWindowPos(hw, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
		break;

		case zorderTOP:
		state|=stateTOP;
		SetWindowPos(hw, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
		break;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Awindow::mouseCapture(bool b, Aobject *o)
{
/*
	{
		char	str[1024];
		sprintf(str,"mouse %s %4d %4d\r\n", mouseEVENTS[event], x, y);
		OutputDebugString(str);
	}
*/
	if(b)
	{
		if(mouseCaptureOBJ)
			mouseCaptureOBJ->mouse(0, 0, 0, mouseLOSTCAPTURE);
		SetFocus(hw);
		SetCapture(hw);
		mouseCaptureOBJ=o?o:this;
	}
	else
	{
		if((o==mouseCaptureOBJ)||(o==NULL))
		{
			ReleaseCapture();
			mouseCaptureOBJ=NULL;
		}
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Awindow::focus(Aobject *o)
{
	if(SetFocus(hw))
	{
		if(keyboardFocusOBJ)
		{
			keyboardFocusOBJ->state&=~stateFOCUS;
			keyboardFocusOBJ->keyboard(keyboardLOSTFOCUS, 0, 0, 0);
		}
		if(o)
			keyboardFocusOBJ=o;
		else
			keyboardFocusOBJ=this;
		keyboardFocusOBJ->state|=stateFOCUS;
		keyboardFocusOBJ->keyboard(keyboardGETFOCUS, 0, 0, 0);
		return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void Awindow::repaint(bool now)
{
	Arectangle r;
	r.w=bitmap->w;
	r.h=bitmap->h;
	if((r.w>0)&&(r.h>0))
		repaint(r, now);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void Awindow::repaint(Arectangle r, bool now)
{
	r.w++;	// todo: be better !!
	r.h++;
	if(state&stateVISIBLE)
	{
		if(updateSection.enter(__FILE__,__LINE__))
		{
			assert(!inUpdate);
			if(!inUpdate)
			{
				inUpdate=TRUE;
				if(hw)
				{
					RECT		rw;
					Arectangle	mclip=bitmap->clip;
					Arectangle	mview=bitmap->view;
					if(bitmap->clip.intersect(r))
					{
						bitmap->view=Arectangle(0, 0, bitmap->w, bitmap->h);
						update(bitmap);
						bitmap->clip=mclip;
						bitmap->view=mview;
						rw.left=r.x;
						rw.top=r.y;
						rw.right=r.x+r.w-1;
						rw.bottom=r.y+r.h-1;
						if(state&stateNOSURFACESIZEUPDATE)
							InvalidateRect(hw, NULL, false);
						else if(!useBlend)
							InvalidateRect(hw, &rw, false);
					}
					bitmap->clip=mclip;
				}
				inUpdate=FALSE;
			}
			if(useBlend)
			{
				HDC				dcD=GetDC(NULL);
				BLENDFUNCTION	bf;
				byte			r,g,b,a;
				POINT			ps,pd;
				SIZE			sz;
				bitmap->set(hdc, 0, 0);
				bf.BlendOp=AC_SRC_OVER;
				bf.BlendFlags=0;
				bf.SourceConstantAlpha=(bitmap->flags&bitmapGLOBALALPHA)?((byte)(bitmap->alpha*255.99f)):255;
				bf.AlphaFormat=(bitmap->flags&bitmapALPHA)?AC_SRC_ALPHA:0;
				colorRGBA(&r, &g, &b, &a, bitmap->colorkey);
				pd.x=pos.x;
				pd.y=pos.y;
				ps.x=0;
				ps.y=0;
				sz.cx=pos.w;
				sz.cy=pos.h;
				int	v=ULW_ALPHA|((bitmap->flags&bitmapCOLORKEY)?ULW_COLORKEY:0);
				m_pUpdateLayeredWindow(hw, dcD, &pd, &sz, hdc, &ps, RGB(r, g, b)|(a<<24), &bf, ((bitmap->flags&bitmapCOLORKEY)?ULW_COLORKEY:ULW_ALPHA));
			}
			else if(now&&hw)
				UpdateWindow(hw);
			updateSection.leave();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void Awindow::del(Anode *o)
{
	if(mouseCaptureOBJ==o)
	{
		mouseCaptureOBJ=NULL;
		ReleaseCapture();
	}
	if(keyboardFocusOBJ==o)
	{
		o->state&=~Aobject::stateFOCUS;
		keyboardFocusOBJ=this;
		state|=Aobject::stateFOCUS;
		keyboardFocusOBJ->keyboard(keyboardGETFOCUS, 0, 0, 0);
	}
	Asurface::del(o);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Awindow::NCsystem(Aobject *o, int event, dword p0, dword p1)
{
	{
		Anode *oo=o->fchild;
		while(oo)
		{
			if(oo->isCI(&Aobject::CI))
				NCsystem((Aobject *)oo, event, p0, p1);
			oo=oo->next;
		}
	}
	return o->system(event, p0, p1);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Awindow::NCmouse(int x, int y, int state, int event)
{
	mouseX=x;
	mouseY=y;
	mouseState=state;
	mouseTime=GetTickCount();
	/*
	{
		char	str[1024];
		sprintf(str,"mouse %4d %4d %8x\n", x, y, event);
		OutputDebugString(str);
	}
	*/
	if(mouseCaptureOBJ)
	{
		Aobject	*o=mouseCaptureOBJ;
		while(o->father)
		{
			x-=o->pos.x;
			y-=o->pos.y;
			o=(Aobject *)o->father;
		}
		if(mouseCaptureOBJ==this)
			return mouse(x, y, state|mouseCAPTURE, event);
		return (mouseCaptureOBJ->NCmouse(x, y, state|mouseCAPTURE, event)&1);
	}
	return Asurface::NCmouse(x, y, state, event);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL dword Awindow::NCnotify(dword m, dword wp, dword lp)
{
	switch(m)
	{
		case WM_LBUTTONDBLCLK:
		{
			int		state=0;
			if(wp&MK_CONTROL)
				state|=mouseCTRL;
			if(wp&MK_SHIFT)
				state|=mouseSHIFT;
			if(wp&MK_LBUTTON)
				state|=mouseL;
			if(wp&MK_MBUTTON)
				state|=mouseM;
			if(wp&MK_RBUTTON)
				state|=mouseR;
			NCmouse(GET_X_LPARAM(lp), GET_Y_LPARAM(lp), state, mouseLDOWN);
			NCmouse(GET_X_LPARAM(lp), GET_Y_LPARAM(lp), state, mouseLDBLCLK);
		}
 		break;

		case WM_LBUTTONDOWN:
		{
			int		state=0;
			if(wp&MK_CONTROL)
				state|=mouseCTRL;
			if(wp&MK_SHIFT)
				state|=mouseSHIFT;
			if(wp&MK_LBUTTON)
				state|=mouseL;
			if(wp&MK_MBUTTON)
				state|=mouseM;
			if(wp&MK_RBUTTON)
				state|=mouseR;
			NCmouse(GET_X_LPARAM(lp), GET_Y_LPARAM(lp), state, mouseLDOWN);
		}
		break;

		case WM_LBUTTONUP:
		{
			int		state=0;
			if(wp&MK_CONTROL)
				state|=mouseCTRL;
			if(wp&MK_SHIFT)
				state|=mouseSHIFT;
			if(wp&MK_LBUTTON)
				state|=mouseL;
			if(wp&MK_MBUTTON)
				state|=mouseM;
			if(wp&MK_RBUTTON)
				state|=mouseR;
			NCmouse(GET_X_LPARAM(lp), GET_Y_LPARAM(lp), state, mouseLUP);
		}
		break;

		case WM_MBUTTONDBLCLK:
		{
			int		state=0;
			if(wp&MK_CONTROL)
				state|=mouseCTRL;
			if(wp&MK_SHIFT)
				state|=mouseSHIFT;
			if(wp&MK_LBUTTON)
				state|=mouseL;
			if(wp&MK_MBUTTON)
				state|=mouseM;
			if(wp&MK_RBUTTON)
				state|=mouseR;
			NCmouse(GET_X_LPARAM(lp), GET_Y_LPARAM(lp), state, mouseMDOWN);
			NCmouse(GET_X_LPARAM(lp), GET_Y_LPARAM(lp), state, mouseMDBLCLK);
		}
		break;

		case WM_MBUTTONDOWN:
		{
			int		state=0;
			if(wp&MK_CONTROL)
				state|=mouseCTRL;
			if(wp&MK_SHIFT)
				state|=mouseSHIFT;
			if(wp&MK_LBUTTON)
				state|=mouseL;
			if(wp&MK_MBUTTON)
				state|=mouseM;
			if(wp&MK_RBUTTON)
				state|=mouseR;
			NCmouse(GET_X_LPARAM(lp), GET_Y_LPARAM(lp), state, mouseMDOWN);
		}
		break;

		case WM_MBUTTONUP:
		{
			int		state=0;
			if(wp&MK_CONTROL)
				state|=mouseCTRL;
			if(wp&MK_SHIFT)
				state|=mouseSHIFT;
			if(wp&MK_LBUTTON)
				state|=mouseL;
			if(wp&MK_MBUTTON)
				state|=mouseM;
			if(wp&MK_RBUTTON)
				state|=mouseR;
			NCmouse(GET_X_LPARAM(lp), GET_Y_LPARAM(lp), state, mouseMUP);
		}
		break;

		//case WM_MOUSEHOVER:
		//break;

		//case WM_MOUSELEAVE:
		//break;

		case WM_MOUSEWHEEL:
		{
			int		state=0;
			mouseW=(int)(short)HIWORD(wp);
			if(wp&MK_CONTROL)
				state|=mouseCTRL;
			if(wp&MK_SHIFT)
				state|=mouseSHIFT;
			if(wp&MK_LBUTTON)
				state|=mouseL;
			if(wp&MK_MBUTTON)
				state|=mouseM;
			if(wp&MK_RBUTTON)
				state|=mouseR;
			NCmouse(mouseX, mouseY, state, mouseWHEEL);
		}
		break;

		case WM_MOUSEMOVE:
		{
			int		state=0;
			if(wp&MK_CONTROL)
				state|=mouseCTRL;
			if(wp&MK_SHIFT)
				state|=mouseSHIFT;
			if(wp&MK_LBUTTON)
				state|=mouseL;
			if(wp&MK_MBUTTON)
				state|=mouseM;
			if(wp&MK_RBUTTON)
				state|=mouseR;
			NCmouse(GET_X_LPARAM(lp), GET_Y_LPARAM(lp), state, mouseNORMAL);
		}
		break;

		case WM_RBUTTONDBLCLK:
		{
			int		state=0;
			if(wp&MK_CONTROL)
				state|=mouseCTRL;
			if(wp&MK_SHIFT)
				state|=mouseSHIFT;
			if(wp&MK_LBUTTON)
				state|=mouseL;
			if(wp&MK_MBUTTON)
				state|=mouseM;
			if(wp&MK_RBUTTON)
				state|=mouseR;
			NCmouse(GET_X_LPARAM(lp), GET_Y_LPARAM(lp), state, mouseRDOWN);
			NCmouse(GET_X_LPARAM(lp), GET_Y_LPARAM(lp), state, mouseRDBLCLK);
		}
		break;

		case WM_RBUTTONDOWN:
		{
			int		state=0;
			if(wp&MK_CONTROL)
				state|=mouseCTRL;
			if(wp&MK_SHIFT)
				state|=mouseSHIFT;
			if(wp&MK_LBUTTON)
				state|=mouseL;
			if(wp&MK_MBUTTON)
				state|=mouseM;
			if(wp&MK_RBUTTON)
				state|=mouseR;
			NCmouse(GET_X_LPARAM(lp), GET_Y_LPARAM(lp), state, mouseRDOWN);
		}
		break;

		case WM_RBUTTONUP:
		{
			int		state=0;
			if(wp&MK_CONTROL)
				state|=mouseCTRL;
			if(wp&MK_SHIFT)
				state|=mouseSHIFT;
			if(wp&MK_LBUTTON)
				state|=mouseL;
			if(wp&MK_MBUTTON)
				state|=mouseM;
			if(wp&MK_RBUTTON)
				state|=mouseR;
			NCmouse(GET_X_LPARAM(lp), GET_Y_LPARAM(lp), state, mouseRUP);
		}
		break;

		case WM_TIMER:
		if((!(mouseState&mousePULSE))&&((GetTickCount()-mouseTime)>200))
		{
			NCmouse(mouseX, mouseY, mouseState|mousePULSE, mouseSTAY);
		}
		break;

		case WM_SIZE:
		if(!(state&stateNOSURFACESIZEUPDATE))
			Asurface::size(LOWORD(lp), HIWORD(lp));
		else
		{
			pos.w=LOWORD(lp);
			pos.h=HIWORD(lp);
		}
		break;

		case WM_NCPAINT:
		return 0;

		case WM_PAINT:
		if(!(state&stateNOPAINT))
		{
			if(updateSection.enter(__FILE__,__LINE__))
			{
				PAINTSTRUCT ps;
				HDC         hdc;
				hdc=BeginPaint(hw, &ps);
				if(!(state&stateNOSURFACESIZEUPDATE))
					bitmap->set(hdc, 0, 0, bltmodeANTIALIAS);
				else
					bitmap->set(hdc, 0, 0, pos.w, pos.h, bltmodeANTIALIAS);
				ReleaseDC(hw, hdc);
				EndPaint(hw, &ps);
				updateSection.leave();
			}
		}
		return 0;

		case WM_MOVE:
		pos.x=GET_X_LPARAM(lp);
		pos.y=GET_Y_LPARAM(lp);
		/*
		if(bitmap->flags&(bitmapCOLORKEY|bitmapGLOBALALPHA|bitmapALPHA))
		{
			RECT	r;
			r.top=pos.y;
			r.left=pos.x;
			r.bottom=pos.y+pos.h;
			r.right=pos.x+pos.w;
			InvalidateRect(NULL, &r, FALSE);
		}
		*/
		break;

		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			int		s=((GetKeyState(VK_SHIFT)&0x8000)?keyboardSHIFT:0)|((GetKeyState(VK_MENU)&0x8000)?keyboardALT:0)|((GetKeyState(VK_CONTROL)&0x8000)?keyboardCTRL:0);
			keyboardFocusOBJ->NCkeyboard(keyboardDOWN, wp, (lp>>16)&255, s);
		}
		return 0;

		case WM_SYSKEYUP:
		case WM_KEYUP:
		{
			int		s=((GetKeyState(VK_SHIFT)&0x8000)?keyboardSHIFT:0)|((GetKeyState(VK_MENU)&0x8000)?keyboardALT:0)|((GetKeyState(VK_CONTROL)&0x8000)?keyboardCTRL:0);
			keyboardFocusOBJ->NCkeyboard(keyboardUP, wp, (lp>>16)&255, s);
		}
		return 0;

		case WM_CHAR:
		{
			int		s=((GetKeyState(VK_SHIFT)&0x8000)?keyboardSHIFT:0)|((GetKeyState(VK_MENU)&0x8000)?keyboardALT:0)|((GetKeyState(VK_CONTROL)&0x8000)?keyboardCTRL:0);
			keyboardFocusOBJ->NCkeyboard(keyboardCHAR, wp, (lp>>16)&255, s);
		}
		return 0;

		case WM_DEVICECHANGE:
		NCsystem(this, systemDEVICECHANGE, wp, lp);
		break;
		
		case WM_SHOWWINDOW:
		if(notify(this, nyMINIMIZE, 0))
			return 0;
		break;
		
		case WM_CLOSE:
		return notify(this, nyCLOSE, 0);
	}
	return DefWindowProc(this->hw, m, wp, lp);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void timerFunction()
{
	int	total=GetTickCount();
	timesection.enter(__FILE__,__LINE__);
	if(tlistf)
	{
		Atasklist	*l=tlistf;
		int			t=total;
		int			nbt=nbtlist;

		int	nbtimer=0;
		while(l)
		{
			nbtimer++;
			l=l->next;
		}
		l=tlistf;

		//assert(nbtimer==nbtlist);
		
		while(l&&nbt--)
		{
			Atasklist	*next=l->next;
			Atasklist	*prev=l->prev;
			if(t-l->time>0)
			{
				Aobject	*o=l->o;
				l->time=t+l->inc;
				if(next)
				{
					if(prev)
						prev->next=next;
					else
						tlistf=next;
					next->prev=prev;
					
					tlistl->next=l;
					l->prev=tlistl;
					l->next=NULL;
					tlistl=l;
				}
				o->pulse();
				t=GetTickCount();
				if(t-total>5)
					break;
			}
			l=next;
		}
	}
	timesection.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool NCglobalTimer(Aobject *o, int time)
{
	timesection.enter(__FILE__,__LINE__);
	{
		Atasklist	*l=tlistf;
		while(l)
		{
			Atasklist	*prev=l->prev;
			Atasklist	*next=l->next;
			if(l->o==o)
			{
				if(prev)
					prev->next=next;
				else
					tlistf=next;
				if(next)
					next->prev=prev;
				else
					tlistl=prev;
				delete(l);
				nbtlist--;
				break;
			}
			l=next;
		}
	}
	if(time>0)
	{
		Atasklist	*l=new Atasklist();
		l->time=GetTickCount()+time;
		l->inc=time;
		l->o=o;
		if(tlistf)
			tlistf->prev=l;
		else
			tlistl=l;
		l->next=tlistf;
		l->prev=NULL;
		tlistf=l;
		nbtlist++;
	}
	timesection.leave();
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL int Awindow::NCmakeCursor(Aresobj *res, int hx, int hy)
{
	int	ncus;
	
	for(ncus=0; ncus<MAXCUSTOMCURSORS; ncus++)
		if(!customCursor[ncus])
			break;
			
	if(ncus<MAXCUSTOMCURSORS)
	{
		ICONINFO	iconinfo={0};
		Abitmap		*b=new Abitmap(res);
		int			x,y;
		
		if(b->nbBits!=32)
		{
			delete(b);
			return cursorNONE;
		}

		HDC		dc=GetDC(NULL);
		HDC		dcMask=CreateCompatibleDC(dc); 
		HDC		dcPaint=CreateCompatibleDC(dc);
		
		HBITMAP bmMask=CreateBitmap(b->w, b->h, 1, 1, NULL);
		HBITMAP bmPaint=CreateBitmap(b->w, b->h, 1, 32, NULL);
		
		HBITMAP bmMaskOld=(HBITMAP)SelectObject(dcMask, bmMask);
		HBITMAP bmPaintOld=(HBITMAP)SelectObject(dcPaint, bmPaint);

		switch(b->nbBits)
		{		
			case 32:
			for(y=0; y<b->h; y++)
			{
				dword	*s=&b->body32[b->adr[y]];
				for(x=0; x<b->w; x++)
				{
					dword	c=*(s++);
					byte	r,g,b,a;
					colorRGBA(&r, &g, &b, &a, c);
					if(a)
					{
						SetPixel(dcMask, x, y, 0x00000000);
						SetPixel(dcPaint, x, y, RGB(r, g, b));
					}
					else
					{
						SetPixel(dcMask, x, y, 0x00ffffff);
						SetPixel(dcPaint, x, y, 0);
					}
				}
			}
			break;
		}
		
		delete(b);

		iconinfo.fIcon=false;
		iconinfo.xHotspot=hx;
		iconinfo.yHotspot=hy;
		iconinfo.hbmMask=bmMask;
		iconinfo.hbmColor=bmPaint;

		SelectObject(dcMask, bmMaskOld);
		SelectObject(dcPaint, bmPaintOld);
		DeleteDC(dcMask);
		DeleteDC(dcPaint);
		ReleaseDC(NULL, dc);

		customCursor[ncus]=(HCURSOR)CreateIconIndirect(&iconinfo);
		
		DeleteObject(bmMask);
		DeleteObject(bmPaint);
		
		return cursorCUSTOM+ncus;
	}
	return cursorNONE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Awindow::NCcursor(int cursor)
{
	HCURSOR	hc=NULL;
	switch(cursor)
	{
		default:
		if(cursor>=cursorCUSTOM)
		{
			int	ncur=cursor-cursorCUSTOM;
			if(ncur<MAXCUSTOMCURSORS)
			{
				hc=customCursor[ncur];
				if(hc)
					break;
			}
		}
		case cursorNORMAL:
		hc=LoadCursor(NULL, IDC_ARROW);
		break;
		case cursorCROSS:
		hc=LoadCursor(NULL, IDC_CROSS);
		break;
		case cursorHAND:
		hc=hcurHand;
		break;
		case cursorHANDKEEP:
		hc=hcurHandKeep;
		break;
		case cursorHANDSEL:
		hc=hcurHandSel;
		break;
		case cursorINK:
		hc=hcurInk;
		break;
		case cursorHELP:
		hc=LoadCursor(NULL, IDC_HELP);
		break;
		case cursorTEXT:
		hc=LoadCursor(NULL, IDC_IBEAM);
		break;
		case cursorSIZEALL:
		hc=LoadCursor(NULL, IDC_SIZEALL);
		break;
		case cursorSIZENESW:
		hc=LoadCursor(NULL, IDC_SIZENESW);
		break;
		case cursorSIZENWSE:
		hc=LoadCursor(NULL, IDC_SIZENWSE);
		break;
		case cursorSIZENS:
		hc=LoadCursor(NULL, IDC_SIZENS);
		break;
		case cursorSIZEWE:
		hc=LoadCursor(NULL, IDC_SIZEWE);
		break;
		case cursorWAIT:
		hc=LoadCursor(NULL, IDC_WAIT);
		break;
		case cursorMAGNIFY:
		hc=hcurMagnify;
		break;
		case cursorMAGNIFYADD:
		hc=hcurMagnifyAdd;
		break;
		case cursorMAGNIFYSUB:
		hc=hcurMagnifySub;
		break;
		case cursorNO:
		hc=LoadCursor(NULL, IDC_NO);
		break;
	}
	SetCursor(hc);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct todest 
{
	Aobject			*o;
	struct todest	*next;
} Ttodest;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static Ttodest		*destList=NULL;	// to be shared ??? (different DLLs)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void NCdestroy(Aobject *o)
{
	if(o)
	{
		Ttodest	*d=destList;
		while(d)
		{
			if(d->o==o)
				break;
			d=d->next;
		}
		if(!d)
		{
			Ttodest	*pd=(Ttodest *)malloc(sizeof(Ttodest));
			pd->o=o;
			pd->next=destList;
			destList=pd;
		}
		o->timer();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void destroyList()
{
	while(destList)
	{
		Ttodest	*pd=destList;
		destList=pd->next;
		delete(pd->o);
		free(pd);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL HRESULT __stdcall Adroptarget::QueryInterface(REFIID riid, void **ppv)
{
	if (IsEqualIID(riid,IID_IUnknown))
	{
		*ppv = static_cast<IUnknown *>(this);
		AddRef();
		return S_OK;
	}
	else if (IsEqualIID(riid,IID_IDropTarget))
	{
		*ppv = static_cast<IDropTarget *>(this);
		AddRef();
		return S_OK;
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL HRESULT __stdcall Adroptarget::DragEnter(IDataObject *pDataObject, DWORD grfKeyState, _POINTL pt, DWORD *pdwEffect)
{
	if(w)
		return w->NCdragEnter(pt.x, pt.y, pDataObject, grfKeyState, pdwEffect)?S_OK:S_FALSE;
	return S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL HRESULT __stdcall Adroptarget::DragOver(DWORD grfKeyState, _POINTL pt, DWORD *pdwEffect)
{
	if(w)
		return w->NCdragOver(pt.x, pt.y, grfKeyState, pdwEffect)?S_OK:S_FALSE;
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL HRESULT __stdcall Adroptarget::DragLeave()
{
	if(w)
		return w->NCdragLeave()?S_OK:S_FALSE;
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL HRESULT __stdcall Adroptarget::Drop(IDataObject *pDataObject, DWORD grfKeyState, _POINTL pt, DWORD *pdwEffect)
{
	if(w)
		return w->NCdrop(pt.x, pt.y, pDataObject, grfKeyState, pdwEffect)?S_OK:S_FALSE;
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Awindow::NCdragEnter(int x, int y, IDataObject *pDataObject, DWORD grfKeyState, DWORD *pdwEffect)
{
	FORMATETC	fe;
	int			state;
	HRESULT		r;

	memset(&fe, 0, sizeof(fe));

	x-=pos.x;
	y-=pos.y;
	mouseX=x;
	mouseY=y;

	dropType=0;

	fe.cfFormat=CF_TEXT;
	fe.dwAspect=DVASPECT_CONTENT;
	fe.tymed=TYMED_HGLOBAL;
	r=pDataObject->QueryGetData(&fe);
	if(r==S_OK)
		dropType|=dragdropTEXT;

	fe.cfFormat=CF_BITMAP;
	fe.tymed=TYMED_HGLOBAL;
	r=pDataObject->QueryGetData(&fe);
	if(r==S_OK)
		dropType|=dragdropBITMAP;

	fe.cfFormat=CF_DIB;
	fe.tymed=TYMED_HGLOBAL;
	r=pDataObject->QueryGetData(&fe);
	if(r==S_OK)
		dropType|=dragdropBITMAP;

	fe.cfFormat=CF_WAVE;
	fe.tymed=TYMED_HGLOBAL;
	r=pDataObject->QueryGetData(&fe);
	if(r==S_OK)
		dropType|=dragdropSOUND;

	fe.cfFormat=CF_HDROP;
	fe.tymed=TYMED_HGLOBAL;
	r=pDataObject->QueryGetData(&fe);
	if(r==S_OK)
		dropType|=dragdropFILE;

	state=dropType;

    if (grfKeyState & MK_CONTROL)
		state|=mouseCTRL;
    if (grfKeyState & MK_ALT)
		state|=mouseALT;
    if (grfKeyState & MK_SHIFT)
		state|=mouseSHIFT;
    if (grfKeyState & MK_LBUTTON)
		state|=mouseL;
    if (grfKeyState & MK_MBUTTON)
		state|=mouseM;
    if (grfKeyState & MK_RBUTTON)
		state|=mouseR;
	
	NCdragdrop(x, y, state, dragdropENTER);
	return true;
}

bool Awindow::NCdragOver(int x, int y, DWORD grfKeyState, DWORD *pdwEffect)
{
	int	state=dropType;

	x-=pos.x;
	y-=pos.y;
	mouseX=x;
	mouseY=y;

    if (grfKeyState & MK_CONTROL)
		state|=mouseCTRL;
    if (grfKeyState & MK_ALT)
		state|=mouseALT;
    if (grfKeyState & MK_SHIFT)
		state|=mouseSHIFT;
    if (grfKeyState & MK_LBUTTON)
		state|=mouseL;
    if (grfKeyState & MK_MBUTTON)
		state|=mouseM;
    if (grfKeyState & MK_RBUTTON)
		state|=mouseR;

	return NCdragdrop(x, y, state, dragdropOVER);
}

bool Awindow::NCdragLeave()
{
	return NCdragdrop(mouseX, mouseY, 0, dragdropLEAVE);
}

bool Awindow::NCdrop(int x, int y, IDataObject *pDataObject, DWORD grfKeyState, DWORD *pdwEffect)
{
	FORMATETC	fe;
	STGMEDIUM	sm;
	int			state=dropType;

	memset(&fe, 0, sizeof(fe));
	memset(&sm, 0, sizeof(sm));

	x-=pos.x;
	y-=pos.y;
	mouseX=x;
	mouseY=y;

    if (grfKeyState & MK_CONTROL)
		state|=mouseCTRL;
    if (grfKeyState & MK_ALT)
		state|=mouseALT;
    if (grfKeyState & MK_SHIFT)
		state|=mouseSHIFT;
    if (grfKeyState & MK_LBUTTON)
		state|=mouseL;
    if (grfKeyState & MK_MBUTTON)
		state|=mouseM;
    if (grfKeyState & MK_RBUTTON)
		state|=mouseR;

	if(state&dragdropFILE)
	{
		HDROP		hdrop;
		bool		b=false;
		HRESULT		r;

		
		fe.cfFormat=CF_HDROP;
		fe.tymed=TYMED_HGLOBAL;
		fe.dwAspect=1;
		fe.lindex=-1;

		r=pDataObject->GetData(&fe, &sm);

		if(r==S_OK)
		{
			hdrop=(HDROP)GlobalLock(sm.hGlobal);
			if(!hdrop)
			{
				ReleaseStgMedium(&sm);
				return false;
			}

			{
				int		nb=DragQueryFile(hdrop, -1, NULL, 0);
				int		i;
				char	*str=(char *)malloc(nb*ALIB_PATH);
				char	*s=str;
				for(i=0; i<nb; i++)
					if(DragQueryFile(hdrop, i, s, ALIB_PATH)>0)
						s+=strlen(s)+1;
				*s=0;
				GlobalUnlock(sm.hGlobal);
				ReleaseStgMedium(&sm);
				b=NCdragdrop(x, y, state, dragdropDROP, (void *)str);
				free(str);
			}
		}
		else
		{
			/*  debug, get format that bug elektro

			IEnumFORMATETC *en;
			FORMATETC fmt;
			pDataObject->EnumFormatEtc(DATADIR_GET, &en);
			
			while(en->Next(1, &fmt, 0)==S_OK)
			{
				int a=5;
				a++;
			}
			*/

		}
		return b;
	}
	else if(state&dragdropTEXT)
	{
		char		*text;
		bool		b;
		HRESULT		r;

		fe.cfFormat=CF_TEXT;
		fe.dwAspect=DVASPECT_CONTENT;
		fe.tymed=TYMED_HGLOBAL;

		r=pDataObject->GetData(&fe, &sm);

		if(r!=S_OK)
			return false;

	    text=(char *)GlobalLock(sm.hGlobal);
		if(!text)
        {
			ReleaseStgMedium(&sm);
			return false;
        }
        
		b=NCdragdrop(x, y, state, dragdropDROP, (void *)text);
		
		GlobalUnlock(sm.hGlobal);
		ReleaseStgMedium(&sm);
		return b;
	}
	
	return false;
}

ADLL void Awindow::dropText(char *text)
{
	Adropsource *src=new Adropsource();
	AdropString	*txt=new AdropString(text);
	DWORD		effect;
	DoDragDrop(txt, src, DROPEFFECT_COPY, &effect);
	delete(src);
	delete(txt);
}

ADLL void Awindow::dropFiles(char *files)
{
	Adropsource *src=new Adropsource();
	AdropHdrop	*hdrop=new AdropHdrop(files);
	DWORD		effect=0;
	long		ret=DoDragDrop(hdrop, src, DROPEFFECT_COPY, &effect); //CO_E_NOTINITIALIZED
	delete(src);
	delete(hdrop);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL HRESULT __stdcall Adropsource::QueryInterface(REFIID riid, void **ppv)
{
	if (IsEqualIID(riid,IID_IUnknown))
	{
		*ppv = static_cast<IUnknown *>(this);
		AddRef();
		return S_OK;
	}
	else if (IsEqualIID(riid,IID_IDropSource))
	{
		*ppv = static_cast<IDropSource *>(this);
		AddRef();
		return S_OK;
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL HRESULT __stdcall Adropsource::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{
	if(fEscapePressed)
		return DRAGDROP_S_CANCEL;
	if(!(grfKeyState & MK_LBUTTON))
		return DRAGDROP_S_DROP;
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL HRESULT __stdcall Adropsource::GiveFeedback(DWORD dwEffect)
{
	return DRAGDROP_S_USEDEFAULTCURSORS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL AdropString::AdropString(char *str)
{
	nrefdrop=0;
	text=GlobalAlloc(GMEM_FIXED, sizeof(TCHAR)*(lstrlen(str)+1));
	lstrcpy((TCHAR*)text, str);
}

ADLL AdropString::~AdropString()
{
	GlobalFree(text);
	assert(nrefdrop==0);
}

ADLL HRESULT __stdcall AdropString::QueryInterface(REFIID riid, void **ppv)
{
	if (IsEqualIID(riid,IID_IUnknown))
	{
		*ppv = static_cast<IUnknown *>(this);
		AddRef();
		return S_OK;
	}
	else if (IsEqualIID(riid,IID_IDataObject))
	{
		*ppv = static_cast<IDataObject *>(this);
		AddRef();
		return S_OK;
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
}

ADLL HRESULT __stdcall AdropString::GetData(FORMATETC *pFormatEtc, STGMEDIUM *pmedium)
{
	if((pFormatEtc->dwAspect & DVASPECT_CONTENT) && (pFormatEtc->tymed & TYMED_HGLOBAL) && (pFormatEtc->cfFormat == CF_TEXT))
	{
		pmedium->tymed			= TYMED_HGLOBAL;
		pmedium->pUnkForRelease = NULL;
		{
			DWORD	len    = GlobalSize(text);
			PVOID	source = GlobalLock(text);
			HGLOBAL	dest   = GlobalAlloc(GMEM_FIXED, len);
		    memcpy(dest, source, len);
		    GlobalUnlock(text);
		    pmedium->hGlobal=dest;
		}
		return S_OK;
	}
	return DV_E_FORMATETC;
}

ADLL HRESULT __stdcall AdropString::GetDataHere(FORMATETC *pFormatEtc, STGMEDIUM *pmedium)
{
	return DATA_E_FORMATETC;
}

ADLL HRESULT __stdcall AdropString::QueryGetData(FORMATETC *pFormatEtc)
{
	if((pFormatEtc->dwAspect & DVASPECT_CONTENT) && (pFormatEtc->tymed & TYMED_HGLOBAL) && (pFormatEtc->cfFormat == CF_TEXT))
		return S_OK;
	return DV_E_FORMATETC;	
}

ADLL HRESULT __stdcall AdropString::GetCanonicalFormatEtc(FORMATETC *pFormatEct, FORMATETC *pFormatEtcOut)
{
	pFormatEtcOut->ptd = NULL;
    return E_NOTIMPL;
}

ADLL HRESULT __stdcall AdropString::SetData(FORMATETC *pFormatEtc, STGMEDIUM *pMedium,  BOOL fRelease)
{
    return E_NOTIMPL;
}

ADLL HRESULT __stdcall AdropString::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppEnumFormatEtc)
{
    // only the get direction is supported for OLE
    if(dwDirection == DATADIR_GET)
    {
        // for Win2k+ you can use the SHCreateStdEnumFmtEtc API call, however
        // to support all Windows platforms we need to implement IEnumFormatEtc ourselves.
		FORMATETC formatEtc = {CF_TEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
		return SHCreateStdEnumFmtEtc(1, &formatEtc, ppEnumFormatEtc);
    }
	return E_NOTIMPL;
}

ADLL HRESULT __stdcall AdropString::DAdvise(FORMATETC *pFormatEtc, DWORD advf, IAdviseSink *, DWORD *)
{
	return OLE_E_ADVISENOTSUPPORTED;
}

ADLL HRESULT __stdcall AdropString::DUnadvise(DWORD dwConnection)
{
	return OLE_E_ADVISENOTSUPPORTED;
}

ADLL HRESULT __stdcall AdropString::EnumDAdvise(IEnumSTATDATA **ppEnumAdvise)
{
	return OLE_E_ADVISENOTSUPPORTED;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL AdropHdrop::AdropHdrop(char *files)
{
	int		size=1;
	{
		char	*s=files;
		while(*s)
		{
			int l=strlen(s);
			size+=l+1;
			s+=l+1;
		}
	}

	nrefdrop=0;

	{
		DROPFILES*     pDrop;
		int				uBuffSize = sizeof(DROPFILES) + sizeof(TCHAR) * size;
		hgDrop= GlobalAlloc ( GHND | GMEM_SHARE, uBuffSize );
	    if ( hgDrop )
		{
		    pDrop = (DROPFILES*) GlobalLock ( hgDrop );
			pDrop->pFiles = sizeof(DROPFILES);
		    pDrop->fWide = FALSE;	// not unicode
			{
				TCHAR *pszBuff = (TCHAR*) (LPBYTE(pDrop) + sizeof(DROPFILES));
				memcpy(pszBuff, files, size);
			}
			GlobalUnlock ( hgDrop );
		}
	}
}

ADLL AdropHdrop::~AdropHdrop()
{
	GlobalFree(hgDrop);
	assert(nrefdrop==0);
}

ADLL HRESULT __stdcall AdropHdrop::QueryInterface(REFIID riid, void **ppv)
{
	if (IsEqualIID(riid,IID_IUnknown))
	{
		*ppv = static_cast<IUnknown *>(this);
		AddRef();
		return S_OK;
	}
	else if (IsEqualIID(riid,IID_IDataObject))
	{
		*ppv = static_cast<IDataObject *>(this);
		AddRef();
		return S_OK;
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
}

ADLL HRESULT __stdcall AdropHdrop::GetData(FORMATETC *pFormatEtc, STGMEDIUM *pmedium)
{
	if((pFormatEtc->tymed & TYMED_HGLOBAL) && (pFormatEtc->cfFormat == CF_HDROP))
	{
		pmedium->tymed			= TYMED_HGLOBAL;
		pmedium->pUnkForRelease = NULL;
		{
			DWORD	len    = GlobalSize(hgDrop);
			PVOID	source = GlobalLock(hgDrop);
			HGLOBAL	dest   = GlobalAlloc(GMEM_FIXED, len);
		    memcpy(dest, source, len);
		    GlobalUnlock(hgDrop);
		    pmedium->hGlobal=dest;
		}
		return S_OK;
	}
	return DV_E_FORMATETC;
}

ADLL HRESULT __stdcall AdropHdrop::GetDataHere(FORMATETC *pFormatEtc, STGMEDIUM *pmedium)
{
	return DATA_E_FORMATETC;
}

ADLL HRESULT __stdcall AdropHdrop::QueryGetData(FORMATETC *pFormatEtc)
{
	if((pFormatEtc->tymed & TYMED_HGLOBAL) && (pFormatEtc->cfFormat == CF_HDROP))
		return S_OK;
	return DV_E_FORMATETC;	
}

ADLL HRESULT __stdcall AdropHdrop::GetCanonicalFormatEtc(FORMATETC *pFormatEct, FORMATETC *pFormatEtcOut)
{
	pFormatEtcOut->ptd = NULL;
    return E_NOTIMPL;
}

ADLL HRESULT __stdcall AdropHdrop::SetData(FORMATETC *pFormatEtc, STGMEDIUM *pMedium,  BOOL fRelease)
{
    return E_NOTIMPL;
}

ADLL HRESULT __stdcall AdropHdrop::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppEnumFormatEtc)
{
    // only the get direction is supported for OLE
    if(dwDirection == DATADIR_GET)
    {
        // for Win2k+ you can use the SHCreateStdEnumFmtEtc API call, however
        // to support all Windows platforms we need to implement IEnumFormatEtc ourselves.
		FORMATETC formatEtc = {CF_HDROP, 0, 0, -1, TYMED_HGLOBAL};
		return SHCreateStdEnumFmtEtc(1, &formatEtc, ppEnumFormatEtc);
    }
	return E_NOTIMPL;
}

ADLL HRESULT __stdcall AdropHdrop::DAdvise(FORMATETC *pFormatEtc, DWORD advf, IAdviseSink *, DWORD *)
{
	return OLE_E_ADVISENOTSUPPORTED;
}

ADLL HRESULT __stdcall AdropHdrop::DUnadvise(DWORD dwConnection)
{
	return OLE_E_ADVISENOTSUPPORTED;
}

ADLL HRESULT __stdcall AdropHdrop::EnumDAdvise(IEnumSTATDATA **ppEnumAdvise)
{
	return OLE_E_ADVISENOTSUPPORTED;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
