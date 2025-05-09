/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	GRAB.CPP					(c)	YoY'01						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define STRICT
#include <windows.h>
#include <ddraw.h>
#include <mmsystem.h>
#include "resource.h"
//#include "ddutil.h"
//#include "dxutil.h"

#include						"grab.h"
#include						"resource.h"
#include						<math.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								AgrabInfo::CI	= ACI("AgrabInfo",		GUID(0x11111112,0x00000050), &AeffectInfo::CI, 0, NULL);
ACI								Agrab::CI		= ACI("Agrab",			GUID(0x11111112,0x00000051), &Aeffect::CI, 0, NULL);
ACI								AgrabFront::CI	= ACI("AgrabFront",		GUID(0x11111112,0x00000052), &AeffectFront::CI, 0, NULL);
ACI								AgrabBack::CI	= ACI("AgrabBack",		GUID(0x11111112,0x00000053), &AeffectBack::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int						cursorGRAB		= Aobject::makeCursor(&resource.get(MAKEINTRESOURCE(PNG_GRABCURSOR), "PNG"), 10, 10);

static Arectangle				curi=Arectangle(248, 9, 31, 28);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void drawBox(HWND hwnd)
{
	RECT	r;
	GetWindowRect(hwnd, &r);
	HDC		hdc=GetDC(NULL);
	HBRUSH	hbrush=CreateSolidBrush(0x00ff00);
	FrameRect(hdc, &r, hbrush);
	ReleaseDC(NULL, hdc);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aselwnd : public Awindow
{
	AOBJ

								Aselwnd							(char *name, Agrab *grab, Abitmap *b);
	virtual						~Aselwnd						();				

	virtual bool				mouse							(int x, int y, int state, int event);
	void						paint							(Abitmap *b);
		
	Abitmap						*back;
	Agrab						*grab;
	int							test;
	int							x0,y0;
	int							x1,y1;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Aselwnd::CI		= ACI("Aselwnd",		GUID(0x11111112,0x00000055), &Aeffect::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aselwnd::Aselwnd(char *name, Agrab *grab, Abitmap *b) : Awindow(name, 0, 0, b->w, b->h)
{
	zorder(zorderTOP);
	this->grab=grab;
	this->back=b;
	bitmap->set(0, 0, b, bitmapNORMAL, bitmapNORMAL);
	test=1;
	x0=y0=x1=y1=0;
}

Aselwnd::~Aselwnd()
{
	delete(back);
}

void Aselwnd::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
	b->boxfa(x0, y0, x1, y1, 0xffffffff, 1.f, lineaXOR);
}

bool Aselwnd::mouse(int x, int y, int state, int event)
{
	switch(event)
	{
		case mouseNORMAL:
		if(test>=2)
		{
			x1=x;
			y1=y;
			repaint();
			return true;
		}
		break;

		case mouseLDOWN:
		if(test==1)
		{
			x0=x;
			y0=y;
			x1=x;
			y1=y;
			test++;
			mouseCapture(true);
			return TRUE;
		}
		break;

		case mouseLUP:
		if(test>=2)
		{
			int w=abs(x-x0)+1;;
			int h=abs(y-y0)+1;
			grab->section.enter(__FILE__,__LINE__);
			grab->xs=mini(x, x0);
			grab->ys=mini(y, y0);
			if(grab->bgrab)
				grab->bgrab->size(w, h);
			else
				grab->bgrab=new Abitmap(w, h);
			grab->active=true;
			grab->section.leave();
			mouseCapture(false);
			destroy();
			return TRUE;
		}
		break;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class Awinovr : public Awindow
{
	AOBJ

								Awinovr							(char *name, int x, int y, int w, int h);
	virtual						~Awinovr						();


	HRESULT						InitDirectDraw					(HWND hWnd);
	bool						HasOverlaySupport				();
	HRESULT						CreateDirectDrawSurfaces		(HWND hWnd);

	LPDIRECTDRAW7				g_pDD;
	LPDIRECTDRAWSURFACE7		g_pDDSPrimary;
	LPDIRECTDRAWSURFACE7		g_pDDSOverlay;
	LPDIRECTDRAWSURFACE7		g_pDDSOverlayBack;
	LPDIRECTDRAWSURFACE7		g_pDDSAnimationSheet;  
	DWORD						g_dwOverlayFlags;  
	DDCAPS						g_ddcaps;
	DDOVERLAYFX					g_OverlayFX;     
	bool						isOK;    
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Awinovr::CI	= ACI("Awinovr",		GUID(0x11111112,0x00000056), &Awindow::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Awinovr::Awinovr(char *name, int x, int y, int w, int h) : Awindow(name, x, y, w, h)
{
	g_pDD                = NULL;	
	g_pDDSPrimary        = NULL;  
	g_pDDSOverlay        = NULL;  
	g_pDDSOverlayBack    = NULL;
	g_dwOverlayFlags	 = 0;
	if(InitDirectDraw(hw)==S_OK)
	{
		if(HasOverlaySupport())
		{
			isOK=(CreateDirectDrawSurfaces(hw)==S_OK);
			return;
		}
	}
	isOK=false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Awinovr::~Awinovr()
{
    SAFE_RELEASE( g_pDDSOverlay ); // g_pDDSOverlayBack will be automatically released here
    SAFE_RELEASE( g_pDDSPrimary );
    SAFE_RELEASE( g_pDD );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HRESULT Awinovr::InitDirectDraw( HWND hWnd )
{
    DDSURFACEDESC2 ddsd;
    HRESULT	hr;

    // Create the main DirectDraw object
    if( FAILED( hr = DirectDrawCreateEx( NULL, (VOID**)&g_pDD, 
                                         IID_IDirectDraw7, NULL ) ) )
        return hr;

    // Request normal cooperative level to put us in windowed mode
    if( FAILED( hr = g_pDD->SetCooperativeLevel( hWnd, DDSCL_NORMAL ) ) ) 
        return hr;

    // Get driver capabilities to determine Overlay support.
    ZeroMemory( &g_ddcaps, sizeof(g_ddcaps) );
    g_ddcaps.dwSize = sizeof(g_ddcaps);

    if( FAILED( hr = g_pDD->GetCaps( &g_ddcaps, NULL ) ) )
        return hr;

    // Create the primary surface, which in windowed mode is the desktop.
    ZeroMemory(&ddsd,sizeof(ddsd));
    ddsd.dwSize         = sizeof(ddsd);
    ddsd.dwFlags        = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    if( FAILED( hr = g_pDD->CreateSurface( &ddsd, &g_pDDSPrimary, NULL ) ) )
        return hr;
        
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Awinovr::HasOverlaySupport()
{
    // Get driver capabilities to determine overlay support.
    ZeroMemory( &g_ddcaps, sizeof(g_ddcaps) );
    g_ddcaps.dwSize = sizeof(g_ddcaps);
    g_pDD->GetCaps( &g_ddcaps, NULL );
    
    // Does the driver support overlays in the current mode? 
    // The DirectDraw emulation layer does not support overlays
    // so overlay related APIs will fail without hardware support.  
    if( g_ddcaps.dwCaps & DDCAPS_OVERLAY )
    {
        // Make sure it supports stretching (scaling)
        if ( g_ddcaps.dwCaps & DDCAPS_OVERLAYSTRETCH )
            return TRUE;
        else
            return FALSE;
    }
    else
    {
        return FALSE;    
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HRESULT Awinovr::CreateDirectDrawSurfaces( HWND hWnd )
{
    DDSURFACEDESC2 ddsd;
    DDPIXELFORMAT  ddpfOverlayFormat;
    DDSCAPS2       ddscaps;
    HRESULT		   hr;
    
    // Release any previous surfaces
    SAFE_RELEASE( g_pDDSOverlay ); 

    // Set the overlay format to 16 bit RGB 5:6:5
    ZeroMemory( &ddpfOverlayFormat, sizeof(ddpfOverlayFormat) );
    ddpfOverlayFormat.dwSize        = sizeof(ddpfOverlayFormat);
    ddpfOverlayFormat.dwFlags       = DDPF_RGB;
    ddpfOverlayFormat.dwRGBBitCount = 16;
    ddpfOverlayFormat.dwRBitMask    = 0xF800; 
    ddpfOverlayFormat.dwGBitMask    = 0x07E0;
    ddpfOverlayFormat.dwBBitMask    = 0x001F; 

    // Setup the overlay surface's attributes in the surface descriptor
    ZeroMemory( &ddsd, sizeof(ddsd) );
    ddsd.dwSize            = sizeof(ddsd);
    ddsd.dwFlags           = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | 
                             DDSD_BACKBUFFERCOUNT | DDSD_PIXELFORMAT;
    ddsd.ddsCaps.dwCaps    = DDSCAPS_OVERLAY | DDSCAPS_FLIP | 
                             DDSCAPS_COMPLEX | DDSCAPS_VIDEOMEMORY;
    ddsd.dwBackBufferCount = 1;
    ddsd.dwWidth           = pos.w;
    ddsd.dwHeight          = pos.h;
    ddsd.ddpfPixelFormat   = ddpfOverlayFormat;  // Use 16 bit RGB 5:6:5 pixel format

    // Attempt to create the surface with theses settings
    if( FAILED( hr = g_pDD->CreateSurface( &ddsd, &g_pDDSOverlay, NULL ) ) ) 
        return hr;

    ZeroMemory(&ddscaps, sizeof(ddscaps));
    ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
    if( FAILED( hr = g_pDDSOverlay->GetAttachedSurface( &ddscaps, &g_pDDSOverlayBack ) ) )
        return hr;

    // Setup effects structure
    ZeroMemory( &g_OverlayFX, sizeof(g_OverlayFX) );
    g_OverlayFX.dwSize = sizeof(g_OverlayFX);

    // Setup overlay flags.
    g_dwOverlayFlags = DDOVER_SHOW;

/*
    // Check for destination color keying capability
    if (g_ddcaps.dwCKeyCaps & DDCKEYCAPS_DESTOVERLAY)
    {
        // Using a color key will clip the overlay 
        // when the mouse or other windows go on top of us. 
        DWORD dwDDSColor;

        // The color key can be any color, but a near black (not exactly) allows
        // the cursor to move around on the window without showing off the
        // color key, and also clips windows with exactly black text.
        CSurface frontSurface; 
        frontSurface.Create( g_pDDSPrimary );
        dwDDSColor = frontSurface.ConvertGDIColor( g_dwBackgroundColor );
        g_OverlayFX.dckDestColorkey.dwColorSpaceLowValue  = dwDDSColor;
        g_OverlayFX.dckDestColorkey.dwColorSpaceHighValue = dwDDSColor;
        g_dwOverlayFlags |= DDOVER_DDFX | DDOVER_KEYDESTOVERRIDE;
    }
    else
*/
    {
        LPDIRECTDRAWCLIPPER pClipper = NULL;

        // If not, we'll setup a clipper for the window.  This will fix the
        // problem on a few video cards - but the ones that don't shouldn't
        // care.
        if( FAILED( hr = g_pDD->CreateClipper(0, &pClipper, NULL) ) )
            return hr;

        if( FAILED( hr = pClipper->SetHWnd(0, hWnd ) ) )
            return hr;

        if( FAILED( hr = g_pDDSPrimary->SetClipper( pClipper ) ) )
            return hr;

        SAFE_RELEASE( pClipper );
    }


    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int				count=0;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void init()
{
	if(!count)
	{
	}
	count++;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void end()
{
	count--;
	if(count==0)
	{
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Agrab::Agrab(QIID qiid, char *name, AeffectInfo *info, Acapsule *capsule) : Aeffect(qiid, name, info, capsule)
{
	front=new AgrabFront(qiid, "grab front", this, 48);
	front->setTooltips("grabber");
	back=new AgrabBack(qiid, "grab back", this, 48);
	back->setTooltips("grabber");
	active=false;
	bgrab=NULL;
	xs=ys=0;
	hgrab=false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Agrab::~Agrab()
{
	if(bgrab)
		delete(bgrab);
	end();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Agrab::save(class Afile *f)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Agrab::load(class Afile *f)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Agrab::settings(bool emergency)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Agrab::action(double time, double dtime, double beat, double dbeat)
{
	int				w=getVideoWidth();
	int				h=getVideoHeight();
	Avideo			*out=((AgrabBack *)back)->out;
	section.enter(__FILE__,__LINE__);
	out->enter(__FILE__,__LINE__);
	{
		Abitmap	*b=out->getBitmap();
		if(b)
		{
			if(active)
			{
				bgrab->getDesktop(xs, ys);
				b->set(0, 0, b->w, b->h, 0, 0, bgrab->w, bgrab->h, bgrab, bitmapNORMAL, bitmapNORMAL);
			}
			else if(hgrab)
			{
				RECT r;
				if(GetWindowRect(hgrab, &r))
				{
					int	w=r.right-r.left+1;
					int	h=r.bottom-r.top+1;
					if(bgrab)
						bgrab->size(w, h);
					else
						bgrab=new Abitmap(w, h);
					bgrab->getDesktop(r.left, r.top);
					b->set(0, 0, b->w, b->h, 0, 0, bgrab->w, bgrab->h, bgrab, bitmapNORMAL, bitmapNORMAL);
				}
				else
					hgrab=null;
			}
			else
				b->boxf(0, 0, b->w, b->h, 0xff000000);
		}
	}
	out->leave();
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

void AgrabFront::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
	if(grabWnd)
		b->set(curi.x, curi.y, 31, 0, 31, 28, winbut);
	else
		b->set(curi.x, curi.y, 0, 0, 31, 28, winbut);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AgrabFront::AgrabFront(QIID qiid, char *name, Agrab *e, int h) : AeffectFront(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_GRAB), "PNG");
	back=new Abitmap(&o);

	select=new Abutton("select", this, 16, 16, 80, 16, "ZONE");
	select->setTooltips("select a desktop zone");
	select->show(true);

	overlay=new Abutton("overlay", this, 108, 16, 80, 16, "OVERLAY", Abutton::bt2STATES|Abutton::btCAPTION);
	overlay->setTooltips("lock overlay before launch other software (eg: media player, real), resolve black screen capture.");
	overlay->show(true);

	winbut=new Abitmap(&resource.get(MAKEINTRESOURCE(PNG_GRABWINBUT), "PNG"));

	win=NULL;
	grabWnd=false;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AgrabFront::~AgrabFront()
{
	if(win)
		win->destroy();
	delete(overlay);
	delete(select);
	delete(winbut);
	delete(back);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AgrabFront::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCHANGE:
		if(o==overlay)
		{
			if(win)
			{
				win->destroy();
				win=NULL;
			}
			else
			{
				win=new Awinovr("overlay window", -100, -100, 8, 8);
				if(!win->isOK)
				{
					overlay->setChecked(false);
					win->destroy();
					win=NULL;
					notify(this, nyERROR, (dword)"can't create overlay window");
				}
				else
					win->show(true);
			}
		}
		break;

		case nyCLICK:
		if(o==select)
		{
			Awindow	*w;
			Abitmap	*b=Abitmap::getDesktop();
			b->boxfa(0, 0, b->w, b->h, 0xffffffff, 0.2f);
			w=new Aselwnd("selwnd", (Agrab *)this->effect, b);
			w->show(true);
		}
		break;
	}
	return AeffectFront::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


bool AgrabFront::mouse(int x, int y, int state, int event)
{
	if(grabWnd)
	{
		switch(event)
		{
			case mouseNORMAL:
			{
				Awindow *w=getWindow();
				POINT	p;
				int		mx, my;

				cursor(cursorGRAB);

				w->getPosition(this, &mx, &my);
				p.x=x+mx+w->pos.x;
				p.y=y+my+w->pos.y;
/*
				{
					char str[1024];
					sprintf(str, "mx,my (%d,%d)  p(%d, %d)\r\n", mx, my, p.x, p.y);
					OutputDebugString(str);
				}
*/
				HWND	h=WindowFromPoint(p);
				if(hcur!=h)
				{
					InvalidateRect(NULL, NULL, true);
					UpdateWindow(NULL);
					hcur=h;
					UpdateWindow(hcur);
					drawBox(hcur);
					//UpdateWindow(hcur);
				}
			}
			return true;

			case mouseLUP:
			{
				((Agrab *)effect)->section.enter(__FILE__,__LINE__);
				((Agrab *)effect)->hgrab=hcur;
				((Agrab *)effect)->section.leave();
				InvalidateRect(NULL, NULL, true);
				UpdateWindow(NULL);
				mouseCapture(false);
				grabWnd=false;
			}
			repaint();
			return true;
		}
	}
	else
	{
		switch(event)
		{
			case mouseLDOWN:
			if(curi.contains(x, y))
			{
				cursor(cursorGRAB);
				((Agrab *)effect)->section.enter(__FILE__,__LINE__);
				((Agrab *)effect)->active=false;
				((Agrab *)effect)->hgrab=null;
				((Agrab *)effect)->section.leave();
				grabWnd=true;
				mouseCapture(true);
				repaint();
				return true;
			}

			case mouseNORMAL:
			if(curi.contains(x, y))
			{
				cursor(cursorHAND);
				return true;
			}
			break;
		}
	}
	return AeffectFront::mouse(x, y, state, event);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AgrabFront::pulse()
{
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AgrabBack::AgrabBack(QIID qiid, char *name, Agrab *e, int h) : AeffectBack(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_GRAB2), "PNG");
	back=new Abitmap(&o);

	out=new Avideo(MKQIID(qiid, 0xf1bf3042d7972060), "video output", this, pinOUT, pos.w-18, 10);
	out->setTooltips("video output");
	out->show(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AgrabBack::~AgrabBack()
{
	delete(out);
	delete(back);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AgrabBack::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aeffect * AgrabInfo::create(QIID qiid, char *name, Acapsule *capsule)
{
	return new Agrab(qiid, name, this, capsule);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aplugz * grabGetInfo()
{
	return new AgrabInfo("grabInfo", &Agrab::CI, "grabber", "grabber module - capture part of the windows desktop");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
