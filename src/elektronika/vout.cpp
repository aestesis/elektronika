/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	vout.CPP					(c)	YoY'01						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<ddraw.h>
#include						<assert.h>

#include						"vout.h"
#include						"resource.h"
#include						<math.h>
#include						"main.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								AvoutInfo::CI	= ACI("AvoutInfo",	GUID(0x11111112,0x00000020), &AeffectInfo::CI, 0, NULL);
ACI								Avout::CI		= ACI("Avout",		GUID(0x11111112,0x00000021), &Aeffect::CI, 0, NULL);
ACI								AvoutFront::CI	= ACI("AvoutFront",	GUID(0x11111112,0x00000022), &AeffectFront::CI, 0, NULL);
ACI								AvoutBack::CI	= ACI("AvoutBack",	GUID(0x11111112,0x00000023), &AeffectBack::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Avoutovr : public Awindow
{
	AOBJ

								Avoutovr						(char *name, int x, int y, int w, int h, Avout *vout);
	virtual						~Avoutovr						();

	virtual void				paint							(Abitmap *b);
	virtual bool				notify							(Anode *o, int event, dword p=0);
	virtual bool				mouse							(int x, int y, int state, int event);
	virtual bool				keyboard						(int event, int ascii, int scan, int state);

	virtual void				repaint							(bool now=FALSE);
	virtual void				repaint							(Arectangle r, bool now=FALSE);

	virtual bool				size							(int w, int h);
	virtual bool				show							(bool b);

	int							bac,lx,ly,wx,wy;

	HRESULT						InitDirectDraw					(HWND hWnd);
	bool						HasOverlaySupport				();
	HRESULT						CreateDirectDrawSurfaces		(HWND hWnd);
	HRESULT						moveOvr							();

	Avout						*vout;

	LPDIRECTDRAW7				g_pDD;
	LPDIRECTDRAWSURFACE7		g_pDDSPrimary;
	LPDIRECTDRAWSURFACE7		g_pDDSOverlay;
	LPDIRECTDRAWSURFACE7		g_pDDSOverlayBack;
	LPDIRECTDRAWSURFACE7		g_pDDSAnimationSheet;  
	DWORD						g_dwOverlayFlags;  
	DDCAPS						g_ddcaps;
	DDOVERLAYFX					g_OverlayFX; 
	RECT						g_rcSrc;
	RECT						g_rcDst;
	
	bool						b32b;
	    
	bool						isOK;    
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Avoutovr::CI	= ACI("Avoutovr",		GUID(0x11111112,0x00000026), &Awindow::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Avoutovr::Avoutovr(char *name, int x, int y, int w, int h, Avout *vout) : Awindow(name, x, y, w, h)
{
	zorder(zorderTOP);
	this->vout=vout;
	b32b=false;
	state|=stateNOSURFACESIZEUPDATE;
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
			if(isOK)
				moveOvr();
			return;
		}
	}
	bac=0;
	isOK=false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Avoutovr::~Avoutovr()
{
	vout->winX=pos.x;
	vout->winY=pos.y;
    SAFE_RELEASE( g_pDDSOverlay ); // g_pDDSOverlayBack will be automatically released here
    SAFE_RELEASE( g_pDDSPrimary );
    SAFE_RELEASE( g_pDD );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avoutovr::keyboard(int event, int ascii, int scan, int state)
{
	return true;
}

bool Avoutovr::size(int w, int h)
{
	//Asurface::size(w, h);
	SetWindowPos(hw, NULL, 0, 0, w, h, SWP_NOMOVE|SWP_NOZORDER);
	pos.w=w;
	pos.h=h;
	return TRUE;
}

void Avoutovr::paint(Abitmap *b)
{
	vout->sectionImage.enter(__FILE__,__LINE__);
	b->set(0, 0, vout->image, bitmapNORMAL, bitmapNORMAL);
	vout->sectionImage.leave();	
}

bool Avoutovr::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCHANGE:
		repaint();
		break;
	}
	return Awindow::notify(o, event, p);
}

bool Avoutovr::mouse(int x, int y, int state, int event)
{
	switch(event)
	{
		case mouseLDOWN:
		{
			int max=((pos.w*pos.w+pos.h*pos.h)*7)>>3;
			int	n=x*x+y*y;
			if(n>max)
			{
				wx=pos.x;
				wy=pos.y;
				lx=pos.x+x;
				ly=pos.y+y;
				bac=2;
			}
			else
			{
				wx=pos.x;
				wy=pos.y;
				lx=pos.x+x;
				ly=pos.y+y;
				bac=1;
				cursor(cursorSIZEALL);
			}
			mouseCapture(TRUE);
		}
		return TRUE;

		case mouseNORMAL:
		if(state&mouseL)
		{
			switch(bac)
			{
				case 1:
				move(wx+(x+pos.x)-lx, wy+(y+pos.y)-ly);
				if(isOK)
					moveOvr();
				break;

				case 2:
				vout->sectionImage.enter(__FILE__,__LINE__);
				size(maxi(x+1, 32), maxi(y+1, 24));
				vout->sectionImage.leave();
				if(isOK)
					moveOvr();
				break;
			}
		}
		else
		{
			int max=((pos.w*pos.w+pos.h*pos.h)*7)>>3;
			int	n=x*x+y*y;
			if(n>max)
				cursor(cursorSIZENWSE);
			else
				cursor(cursorCROSS);
		}
		return TRUE;

		case mouseLUP:
		if(bac)
		{
			bac=0;
			mouseCapture(FALSE);
		}
		return TRUE;

		case mouseRDOWN:
		vout->sectionImage.enter(__FILE__,__LINE__);
		size(vout->image->w, vout->image->h);
		vout->sectionImage.leave();
		if(isOK)
			moveOvr();
		return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avoutovr::repaint(bool now)
{
	Arectangle r;
	r.w=bitmap->w;
	r.h=bitmap->h;
	repaint(r, now);
}

void Avoutovr::repaint(Arectangle r, bool now)
{
	r.w++;	// todo: be better !!
	r.h++;
	if(updateSection.enter(__FILE__,__LINE__))
	{
		assert(!inUpdate);
		if(!inUpdate)
		{
			inUpdate=TRUE;
			if(hw)
			{
				Arectangle	mclip=bitmap->clip;
				Arectangle	mview=bitmap->view;
				bitmap->clip.intersect(r);
				bitmap->view=Arectangle(0, 0, bitmap->w, bitmap->h);
				update(bitmap);
				bitmap->clip=mclip;
				bitmap->view=mview;

				if(isOK)
				{
					HDC         hdc;
					if(g_pDDSOverlayBack->GetDC(&hdc)==DD_OK)
					{
						bitmap->set(hdc, 0, 0);
						g_pDDSOverlayBack->ReleaseDC(hdc);
					}
					{
						bool	b=TRUE;
						while(b)
						{
							HRESULT	hRet;
							hRet=g_pDDSOverlay->Flip(NULL, 0);
							switch(hRet)
							{
								case DD_OK:
								b=FALSE;
								break;

								case DDERR_GENERIC:
								b=FALSE;
								break;

								case DDERR_INVALIDOBJECT:
								b=FALSE;
								break;

								case DDERR_INVALIDPARAMS:
								b=FALSE;
								break;

								case DDERR_NOFLIPHW:
								b=FALSE;
								break;

								case DDERR_NOTFLIPPABLE:
								b=FALSE;
								break;

								case DDERR_SURFACEBUSY:
								b=FALSE;
								break;

								case DDERR_SURFACELOST:
								hRet=g_pDDSOverlayBack->Restore();
								if(hRet!=DD_OK)
									b=FALSE;
								break;

								case DDERR_UNSUPPORTED:
								b=FALSE;
								break;

								case DDERR_WASSTILLDRAWING:
								break;
							}
						}
					}
				}
			}
			inUpdate=FALSE;
		}
		updateSection.leave();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avoutovr::show(bool b)
{
	bool	br=Awindow::show(b);
	if(b)
	{
		if(isOK)
			moveOvr();
		repaint();		
	}
	else
	{
		if(isOK)
		    g_pDDSOverlay->UpdateOverlay(NULL, g_pDDSPrimary, NULL, DDOVER_HIDE, NULL);
	}
	return br;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HRESULT Avoutovr::moveOvr()
{
    POINT	p = {0, 0}; // Translation point for the window's client region
	DWORD	g_dwXRatio;
	DWORD	g_dwYRatio; 
/*
	g_rcDst.top=g_rcSrc.top=0;
	g_rcDst.left=g_rcSrc.left=0;
	g_rcDst.right=g_rcSrc.right=pos.w;
	g_rcDst.bottom=g_rcSrc.bottom=pos.h;
*/
    g_rcSrc.left   = 0;
    g_rcSrc.right  = bitmap->w;
    g_rcSrc.top    = 0;
    g_rcSrc.bottom = bitmap->h;

    g_rcDst.left   = 0;
    g_rcDst.right  = pos.w;
    g_rcDst.top    = 0;
    g_rcDst.bottom = pos.h;

    //GetClientRect(hw, &g_rcDst);

    g_dwXRatio = (g_rcDst.right - g_rcDst.left) * 1000 /
                 (g_rcSrc.right - g_rcSrc.left);
    g_dwYRatio = (g_rcDst.bottom - g_rcDst.top) * 1000 /
                 (g_rcSrc.bottom - g_rcSrc.top);

/*
    ClientToScreen( hw, &p );

    g_rcDst.left   = p.x;
    g_rcDst.top    = p.y;
    g_rcDst.bottom += p.y;
    g_rcDst.right  += p.x;
*/

    g_rcDst.left   = pos.x;
    g_rcDst.top    = pos.y;
    g_rcDst.bottom += pos.y;
    g_rcDst.right  += pos.x;

    if( g_pDD )
	{
		if( (g_ddcaps.dwCaps & DDCAPS_OVERLAYSTRETCH) && 
			(g_ddcaps.dwMinOverlayStretch)            && 
			(g_dwXRatio < g_ddcaps.dwMinOverlayStretch) )
		{
			// Window is too small
			g_rcDst.right = 2 * GetSystemMetrics(SM_CXSIZEFRAME) + g_rcDst.left + (pos.w
									 * (g_ddcaps.dwMinOverlayStretch + 1)) / 1000;
		}

		if( (g_ddcaps.dwCaps & DDCAPS_OVERLAYSTRETCH) && 
			(g_ddcaps.dwMaxOverlayStretch)            && 
			(g_dwXRatio > g_ddcaps.dwMaxOverlayStretch) )
		{
			// Window is too large
			g_rcDst.right = 2 * GetSystemMetrics(SM_CXSIZEFRAME) + g_rcDst.left + (pos.h
								   * (g_ddcaps.dwMaxOverlayStretch + 999)) / 1000;
		}

		// Recalculate the ratio's for the upcoming calculations
		g_dwXRatio = (g_rcDst.right  - g_rcDst.left) * 1000 / (g_rcSrc.right  - g_rcSrc.left);
		g_dwYRatio = (g_rcDst.bottom - g_rcDst.top)  * 1000 / (g_rcSrc.bottom - g_rcSrc.top);

		// Check to make sure we're within the screen's boundries, if not then fix
		// the problem by adjusting the source rectangle which we draw from.
		if (g_rcDst.left < 0)
		{
			g_rcSrc.left = -g_rcDst.left * 1000 / g_dwXRatio;
			g_rcDst.left = 0;
		}

		if (g_rcDst.right > GetSystemMetrics(SM_CXSCREEN))
		{
			g_rcSrc.right = bitmap->w - ((g_rcDst.right - GetSystemMetrics(SM_CXSCREEN)) *
									1000 / g_dwXRatio);
			g_rcDst.right = GetSystemMetrics(SM_CXSCREEN);
		}

		if (g_rcDst.bottom > GetSystemMetrics(SM_CYSCREEN))
		{
			g_rcSrc.bottom = bitmap->h - ((g_rcDst.bottom - GetSystemMetrics(SM_CYSCREEN))
									 * 1000 / g_dwYRatio);
			g_rcDst.bottom = GetSystemMetrics(SM_CYSCREEN);
		}

		if (g_rcDst.top < 0)
		{
			g_rcSrc.top = -g_rcDst.top * 1000 / g_dwYRatio;
			g_rcDst.top = 0;
		}

		// Make sure the coordinates fulfill the alignment requirements
		// these expressions (x & -y) just do alignment by dropping low order bits...
		// so to round up, we add first, then truncate.
		if( (g_ddcaps.dwCaps & DDCAPS_ALIGNBOUNDARYSRC) && 
			(g_ddcaps.dwAlignBoundarySrc) )
		{
			g_rcSrc.left = (g_rcSrc.left + g_ddcaps.dwAlignBoundarySrc / 2) & 
							-(signed) (g_ddcaps.dwAlignBoundarySrc);
		}

		if( (g_ddcaps.dwCaps & DDCAPS_ALIGNSIZESRC) && 
			(g_ddcaps.dwAlignSizeSrc) )
		{
			g_rcSrc.right = g_rcSrc.left + (g_rcSrc.right - g_rcSrc.left + g_ddcaps.dwAlignSizeSrc / 2) & 
							 -(signed) (g_ddcaps.dwAlignSizeSrc);
		}

		if( (g_ddcaps.dwCaps & DDCAPS_ALIGNBOUNDARYDEST) && 
			(g_ddcaps.dwAlignBoundaryDest) )
		{
			g_rcDst.left = ( g_rcDst.left + g_ddcaps.dwAlignBoundaryDest / 2 ) & 
						   -(signed) (g_ddcaps.dwAlignBoundaryDest);
		}

		if( (g_ddcaps.dwCaps & DDCAPS_ALIGNSIZEDEST) && 
			(g_ddcaps.dwAlignSizeDest) )
		{
			g_rcDst.right = g_rcDst.left + (g_rcDst.right - g_rcDst.left) & 
							-(signed) (g_ddcaps.dwAlignSizeDest);
		}
		
	}

    g_pDDSOverlay->UpdateOverlay( &g_rcSrc, g_pDDSPrimary, &g_rcDst, g_dwOverlayFlags, &g_OverlayFX);

	return S_OK;
}


HRESULT Avoutovr::InitDirectDraw( HWND hWnd )
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

bool Avoutovr::HasOverlaySupport()
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

HRESULT Avoutovr::CreateDirectDrawSurfaces( HWND hWnd )
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
	//ddpfOverlayFormat.dwRGBBitCount = 16;
	//ddpfOverlayFormat.dwRBitMask    = 0xF800; 
	//ddpfOverlayFormat.dwGBitMask    = 0x07E0;
	//ddpfOverlayFormat.dwBBitMask    = 0x001F; 

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
	//ddsd.ddpfPixelFormat   = ddpfOverlayFormat;  // Use 16 bit RGB 5:6:5 pixel format

	ddpfOverlayFormat.dwRGBBitCount = 32;
	ddpfOverlayFormat.dwRBitMask    = 0x00FF0000; 
	ddpfOverlayFormat.dwGBitMask    = 0x0000FF00;
	ddpfOverlayFormat.dwBBitMask    = 0x000000FF; 
	ddsd.ddpfPixelFormat   = ddpfOverlayFormat;  // Use 32 bit

	// Attempt to create the surface with theses settings
	if( FAILED( hr = g_pDD->CreateSurface( &ddsd, &g_pDDSOverlay, NULL ) ) ) 
	{
		ddpfOverlayFormat.dwRGBBitCount = 16;
		ddpfOverlayFormat.dwRBitMask    = 0xF800; 
		ddpfOverlayFormat.dwGBitMask    = 0x07E0;
		ddpfOverlayFormat.dwBBitMask    = 0x001F; 
		ddsd.ddpfPixelFormat   = ddpfOverlayFormat;  // Use 16 bit

		// Attempt to create the surface with theses settings
		if( FAILED( hr = g_pDD->CreateSurface( &ddsd, &g_pDDSOverlay, NULL ) ) ) 
			return hr;
			
		b32b=false;
	}
	else
		b32b=true;

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

class AvoutDisp : public Awindow
{
	AOBJ

								AvoutDisp						(char *name, int x, int y, int w, int h, Avout *vout);
	virtual						~AvoutDisp						();

	virtual void				paint							(Abitmap *b);
	virtual bool				notify							(Anode *o, int event, dword p=0);
	virtual bool				mouse							(int x, int y, int state, int event);
	virtual bool				keyboard						(int event, int ascii, int scan, int state);

	int							bac,lx,ly,wx,wy;
	Avout						*vout;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								AvoutDisp::CI	= ACI("AvoutDisp",	GUID(0x11111112,0x00000024), &AeffectBack::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AvoutDisp::AvoutDisp(char *name, int x, int y, int w, int h, Avout *vout) : Awindow(name, x, y, w, h)
{
	zorder(zorderTOP);
	state|=stateNOSURFACESIZEUPDATE;
	this->vout=vout;
	bac=0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AvoutDisp::~AvoutDisp()
{
	vout->winX=pos.x;
	vout->winY=pos.y;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AvoutDisp::keyboard(int event, int ascii, int scan, int state)
{
	return true;
}

void AvoutDisp::paint(Abitmap *b)
{
	vout->sectionImage.enter(__FILE__,__LINE__);
	b->set(0, 0, vout->image, bitmapNORMAL, bitmapNORMAL);
	//b->set(0, 0, b->w, b->h, 0, 0, vout->image->w, vout->image->h, vout->image, false);
	vout->sectionImage.leave();	
}

bool AvoutDisp::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCHANGE:
		repaint();
		break;
	}
	return Awindow::notify(o, event, p);
}

bool AvoutDisp::mouse(int x, int y, int state, int event)
{
	switch(event)
	{
		case mouseLDOWN:
		{
			int max=((pos.w*pos.w+pos.h*pos.h)*7)>>3;
			int	n=x*x+y*y;
			if(n>max)
			{
				wx=pos.x;
				wy=pos.y;
				lx=pos.x+x;
				ly=pos.y+y;
				bac=2;
			}
			else
			{
				wx=pos.x;
				wy=pos.y;
				lx=pos.x+x;
				ly=pos.y+y;
				bac=1;
				cursor(cursorSIZEALL);
			}
			mouseCapture(TRUE);
		}
		return TRUE;

		case mouseNORMAL:
		if(state&mouseL)
		{
			switch(bac)
			{
				case 1:
				move(wx+(x+pos.x)-lx, wy+(y+pos.y)-ly);
				break;

				case 2:
				vout->sectionImage.enter(__FILE__,__LINE__);
				size(maxi(x+1, 32), maxi(y+1, 24));
				vout->sectionImage.leave();
				break;
			}
		}
		else
		{
			int max=((pos.w*pos.w+pos.h*pos.h)*7)>>3;
			int	n=x*x+y*y;
			if(n>max)
				cursor(cursorSIZENWSE);
			else
				cursor(cursorCROSS);
		}
		return TRUE;

		case mouseLUP:
		if(bac)
		{
			bac=0;
			mouseCapture(FALSE);
		}
		return TRUE;

		case mouseRDOWN:
		vout->sectionImage.enter(__FILE__,__LINE__);
		size(bitmap->w, bitmap->h);
		vout->sectionImage.leave();
		return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AdirectX
{
public:

									AdirectX						(char *name, int driv, Avout *vout);
	virtual							~AdirectX						();

	virtual	void					display							(Abitmap *b);	

	Avout							*vout;

	HWND							hwnd;
	LPDIRECTDRAW4					lpDD;
	LPDIRECTDRAWSURFACE4			lpDDSPrimary;
	LPDIRECTDRAWSURFACE4			lpDDSBack;
	int								w,h;
	bool							allOK;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

class AlaunchDX : public Athread
{
public:
									AlaunchDX						(Avout *vout=NULL, int device=0);
									
	void							run								();
	
	AdirectX						*dx;
	Avout							*vout;
	int								device;
};

AlaunchDX::AlaunchDX(Avout *vout, int device)
{
	this->dx=NULL;
	this->vout=vout;
	this->device=device;
}

void AlaunchDX::run()
{
	dx=new AdirectX("videoOut display", device, vout);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct sDDinfo
{
	char							name[256];
	char							desc[256];
	HMONITOR						hMon;
	GUID							guid;

} TDDinfo;

///////////////////////////////////////////////////////////////////////////////////////////////////

static TDDinfo						DDinfo[32];
static int							nbDDinfo=0;

///////////////////////////////////////////////////////////////////////////////////////////////////

static bool							dxInit=false;

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

long FAR PASCAL WindowProc(HWND hw, UINT m, WPARAM wp, LPARAM lp)
{
   return DefWindowProc(hw, m, wp, lp);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static BOOL WINAPI DDback(GUID FAR *lpGUID, LPSTR lpDriverDescription, LPSTR lpDriverName, LPVOID lpContext, HMONITOR  hm)
{
	strcpy(DDinfo[nbDDinfo].name, lpDriverName);
	strcpy(DDinfo[nbDDinfo].desc, lpDriverDescription);
	DDinfo[nbDDinfo].hMon=hm;
	if(lpGUID)
		DDinfo[nbDDinfo].guid=*lpGUID;
	else
		memset(&DDinfo[nbDDinfo].guid, 0, sizeof(DDinfo[nbDDinfo].guid));
	nbDDinfo++;
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static void DDenum()
{
	memset(DDinfo, 0, sizeof(DDinfo));
	DirectDrawEnumerateEx(DDback , 0, DDENUM_ATTACHEDSECONDARYDEVICES|DDENUM_DETACHEDSECONDARYDEVICES|DDENUM_NONDISPLAYDEVICES);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

AdirectX::AdirectX(char *name, int driv, Avout *vout)
{
	LPDIRECTDRAW	lpDD_temp=NULL;
 	DDSURFACEDESC2	ddsd;
	DDSCAPS2		ddscaps;
	DWORD			hr;
	MONITORINFO		mi;
	int				x,y;
	
	hwnd=NULL;
	lpDD=NULL;
	lpDDSPrimary=NULL;
	lpDDSBack=NULL;
	this->vout=vout;
	allOK=false;

	{	
		WNDCLASS		wc;
		wc.style=CS_NOCLOSE|CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
		wc.lpfnWndProc=WindowProc;
		wc.cbClsExtra=0;
		wc.cbWndExtra=0;
		wc.hInstance=GetModuleHandle(NULL);
		wc.hIcon=NULL;
		wc.hCursor=NULL;
		wc.hbrBackground=NULL;
		wc.lpszMenuName=NULL;
		wc.lpszClassName="voutYoYclassDD";
		RegisterClass(&wc);
	}
	
	mi.cbSize=sizeof(mi);
	GetMonitorInfo(DDinfo[driv].hMon, &mi);
	x=mi.rcMonitor.left;
	y=mi.rcMonitor.top;
	w=mi.rcMonitor.right-x;
	h=mi.rcMonitor.bottom-y;

	hwnd=CreateWindowEx(WS_EX_TOPMOST, "voutYoYclassDD", "voutYoYclassDD", WS_POPUPWINDOW, x, y, w, h, NULL, NULL, GetModuleHandle(NULL), NULL);
	if(!hwnd)
		return;

	hr=DirectDrawCreate(&DDinfo[driv].guid, &lpDD_temp, NULL);
	if(hr!=DD_OK)
		return;
	
	lpDD_temp->QueryInterface(IID_IDirectDraw4, (LPVOID *)&lpDD);
	if(hr!=DD_OK)
		return;
	lpDD_temp->Release();    

	hr=lpDD->SetCooperativeLevel(hwnd, DDSCL_SETFOCUSWINDOW|DDSCL_CREATEDEVICEWINDOW|DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN);
	if(hr!=DD_OK)
		return;

	lpDD->SetDisplayMode(w, h, 32, 0, 0);
	if(hr!=DD_OK)
		return;

	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize             = sizeof( DDSURFACEDESC2 );
	ddsd.dwFlags            = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
	ddsd.ddsCaps.dwCaps     = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX ;
	ddsd.dwBackBufferCount  = 1;

	hr=lpDD->CreateSurface(&ddsd, &lpDDSPrimary, NULL);
	if(hr!=DD_OK)
		return;

	memset(&ddscaps, 0, sizeof(ddscaps));
	ddscaps.dwCaps          = DDSCAPS_BACKBUFFER;
	hr=lpDDSPrimary->GetAttachedSurface(&ddscaps, &lpDDSBack);
	if(hr!=DD_OK)
		return;

	allOK=true;

	return;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AdirectX::~AdirectX()
{
	if(lpDD)
	{
		lpDD->SetCooperativeLevel(hwnd, DDSCL_NORMAL); 
		if(lpDDSPrimary)
			lpDDSPrimary->Release();
		lpDD->Release();
	}
	if(hwnd)
		DestroyWindow(hwnd);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AdirectX::display(Abitmap *b)
{
	if(allOK)
	{
		HDC		hdc;
		HRESULT	hr;
		// todo: try a lpDDSBack->Lock() to have a better vsync
		hr=lpDDSBack->GetDC(&hdc);
		if(hr==DD_OK)
		{
			b->set(hdc, 0, 0, w, h, bltmodeANTIALIAS);
			/*
			BITMAPINFOHEADER	bi;
			memset(&bi, 0, sizeof(bi));
			bi.biSize=sizeof(bi); 
			bi.biWidth=b->w; 
			bi.biHeight=-b->h; 
			bi.biPlanes=1; 
			bi.biBitCount=32; 
			bi.biCompression=0; 
   			bi.biSizeImage=b->w*b->h*4;
			bi.biXPelsPerMeter=0; 
			bi.biYPelsPerMeter=0; 
			bi.biClrUsed=0; 
			bi.biClrImportant=0;
			SetStretchBltMode(hdc, HALFTONE);
			StretchDIBits(hdc, 0, 0, w, h, 0, 0, b->w, b->h, b->body, (BITMAPINFO *)&bi, DIB_RGB_COLORS, SRCCOPY);
			*/
			lpDDSBack->ReleaseDC(hdc);
		}
		{
			bool	b=TRUE;
			while(b)
			{
				HRESULT	hRet;
				hRet=lpDDSPrimary->Flip(NULL, DDFLIP_WAIT);
				switch(hRet)
				{
					case DD_OK:
					b=FALSE;
					break;

					case DDERR_GENERIC:
					b=FALSE;
					break;

					case DDERR_INVALIDOBJECT:
					b=FALSE;
					break;

					case DDERR_INVALIDPARAMS:
					b=FALSE;
					break;

					case DDERR_NOFLIPHW:
					b=FALSE;
					break;

					case DDERR_NOTFLIPPABLE:
					b=FALSE;
					break;

					case DDERR_SURFACEBUSY:
					b=FALSE;
					break;

					case DDERR_SURFACELOST:
					hRet=lpDDSPrimary->Restore();
					b=FALSE;
					break;

					case DDERR_UNSUPPORTED:
					b=FALSE;
					break;

					case DDERR_WASSTILLDRAWING:
					break;
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct sGDIinfo
{
	char							name[256];
	char							desc[256];
} TGDIinfo;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static TGDIinfo						GDIinfo[32];
static int							nbGDIinfo=0;

static bool							gdiInit=false;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void GDIenum()
{
	memset(GDIinfo, 0, sizeof(GDIinfo));
	{	// enum display
		DISPLAY_DEVICE	dd;
		HRESULT			hr=1;
		int				i=0;
		dd.cb=sizeof(DISPLAY_DEVICE);
		while(hr)
		{
			hr=EnumDisplayDevices(NULL, i, &dd, 0);
			if(hr)
			{
				TGDIinfo	*gi=&GDIinfo[nbGDIinfo++];
				strcpy(gi->name, dd.DeviceName);
				strcpy(gi->desc, dd.DeviceString);
			}
			i++;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Agdi
{
public:
						Agdi					(Avout *vout, char *name, char *desc);
	virtual				~Agdi					();

	void				repaint					();

	Avout				*vout;
	HDC					hdc;
	int					w,h;
	bool				isOK;
	char				name[256];
	char				desc[256];
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Agdi::Agdi(Avout *vout, char *name, char *desc)
{
	this->vout=vout;
	hdc=NULL;
	strcpy(this->name, name);
	strcpy(this->desc, desc);
	isOK=false;
	w=0;
	h=0;

	hdc=CreateDC(name, desc, NULL, NULL);
	if(!hdc)
	{
		DEVMODE dm;
		ZeroMemory(&dm, sizeof(dm));
		dm.dmSize = sizeof(dm);
		EnumDisplaySettingsEx(name, ENUM_REGISTRY_SETTINGS, &dm, 0);
		hdc=CreateDC(name, desc, 0, &dm);
		if(!hdc)
		{
			WCHAR	namew[256];
			WCHAR	descw[256];
			DEVMODEW dmw;
			ZeroMemory(&dmw, sizeof(dmw));
			dmw.dmSize = sizeof(dmw);
			MultiByteToWideChar(CP_ACP, 0, name, 255, namew, 255);
			MultiByteToWideChar(CP_ACP, 0, desc, 255, descw, 255);
			EnumDisplaySettingsExW(namew, ENUM_REGISTRY_SETTINGS, &dmw, 0);
			hdc=CreateDCW(namew, descw, 0, &dmw);
		}
	}

	isOK=hdc?true:false;

	if(isOK)
	{
		w=GetDeviceCaps(hdc, HORZRES);
		h=GetDeviceCaps(hdc, VERTRES);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Agdi::~Agdi()
{
	if(hdc)
		DeleteDC(hdc);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Agdi::repaint()
{
	if(isOK)
	{
		vout->sectionImage.enter(__FILE__,__LINE__);
		vout->image->set(hdc, 0, 0, w, h, bltmodeANTIALIAS);
		vout->sectionImage.leave();	
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Avout::Avout(QIID qiid, char *name, AeffectInfo *info, Acapsule *capsule) : Aeffect(qiid, name, info, capsule)
{
	notregistred=null;
	front=new AvoutFront(qiid, "videoOut front", this, 48);
	back=new AvoutBack(qiid, "videoOut back", this, 48);
	back->setTooltips("videoOut module");
	image=new Abitmap(getVideoWidth(), getVideoHeight());
	win=NULL;
	gdi=NULL;
	dx=NULL;
	runout=true;
	winrun=false;
	winX=20;
	winY=20;
	{
		MYwindow	*w=(MYwindow *)getTable()->getWindow();
		Aregistry	*reg=w->regpwd;
		char		sname[256]="";
		char		spwd[256]="";

		reg->read("name", sname, sizeof(sname));
		reg->read("pwd", spwd, sizeof(spwd));

		OKI_registred=true; //VerifyPassword(sname, spwd);
	}

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Avout::~Avout()
{
	change(0);
	delete(image);
	if(notregistred)
		delete(notregistred);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avout::load(class Afile *f)
{
	AvoutFront	*vf=((AvoutFront *)front);
	char		drv[1024];
	if(!f->readString(drv))
		return false;
	{
		Aitem	*i=(Aitem *)(vf->items->fchild);
		while(i)
		{
			if(!strcmp(drv, i->name))
				break;
			i=(Aitem *)i->next;
		}
  		if(i)
		{
			if(change(i->data))
				vf->list->setCurrentByData(i->data);
			else
				vf->list->setCurrentByData(0);
		}
		else
		{
			change(0);
			vf->list->setCurrentByData(0);
		}
	}
	return true;
}
   
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avout::save(class Afile *f)
{
	Aitem	*i=((AvoutFront *)front)->list->getCurrent();
	if(i)
	{
		if(!f->writeString(i->name))
			return false;
	}
	else
	{
		if(!f->writeString("[none]"))
			return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avout::settings(bool emergency)
{
	image->size(getVideoWidth(), getVideoHeight());

	if(emergency)
	{
		((AvoutFront *)front)->list->setCurrentByData(0);
		change(0);
	}

	{
		MYwindow	*w=(MYwindow *)getTable()->getWindow();
		Aregistry	*reg=w->regpwd;
		char		sname[256]="";
		char		spwd[256]="";

		reg->read("name", sname, sizeof(sname));
		reg->read("pwd", spwd, sizeof(spwd));

		OKI_registred=true; //VerifyPassword(sname, spwd);
	}
	if(win)
	{
		if(!change(current))
			((AvoutFront *)front)->list->setCurrentByData(0);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avout::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCLOSE:
		change(0);
		((AvoutFront *)front)->list->setCurrentByData(0);
		return true;
	}
	return this->Aeffect::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avout::change(int c)
{
	winChange.enter(__FILE__,__LINE__);
	if((current>=10)&&(current<42))
	{
		winrun=false;
		error=false;
		if(dx)
		{
			delete(dx);
			dx=NULL;
		}
		runout=true;
			
/*	
		winrun=false;
		error=false;
		if(dx)
			delete(dx);
		dx=NULL;
		runout=true;
*/
	}
	else
	{
		winrun=false;
		error=false;
		while(!runout)
			sleep(10);
		stop();
	}
	
	current=c;
	
	if((current>=10)&&(current<42))
	{
		if(current-10<nbDDinfo)
		{
			AlaunchDX	*ldx=new AlaunchDX(this, current-10);
			ldx->start();
			while(!ldx->dx)
				Athread::sleep(1);
			dx=ldx->dx;				
			delete(ldx);
			if(!dx->allOK)
			{
				delete(dx);
				dx=NULL;
				winChange.leave();
				return false;
			}
			
		/*
			dx=new AdirectX("videoOut display", current-10, this);
			if(!dx->allOK)
			{
				delete(dx);
				dx=NULL;
				winChange.leave();
				return false;
			}
		*/
		}
		else
		{
			winChange.leave();
			return false;
		}
		winrun=TRUE;
		runout=false;
		error=1;
		winChange.leave();
		return true;
	}
	else
	{
		start();
		while(error==0)
			sleep(10);
		if(error!=1)
			getTable()->notify(getTable(), nyERROR, (dword)"can't open device");
		winChange.leave();
		return (error==1);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avout::action(double time, double dtime, double beat, double dbeat)
{
	AvoutFront	*front=(AvoutFront *)this->front;
	int			s=((AvoutFront *)front)->source->get();
	Avideo		*from=(Avideo *)((AvoutBack *)back)->in[s]->getConnectedPin();
	winChange.enter(__FILE__,__LINE__);
	if(from)
	{
		from->enter(__FILE__,__LINE__);
		sectionImage.enter(__FILE__,__LINE__);
		{
			dword	color=front->color->get();
			if((color&0x00ffffff)==0x808080)
			{
				image->set(0, 0, from->image, bitmapNORMAL, bitmapNORMAL);
			}
			else
			{
				int		x,y;
				int		w=image->w;
				int		h=image->h;
				dword	*s=from->image->body32;
				dword	*d=image->body32;
				byte	fr,fg,fb;

				colorRGB(&fr, &fg, &fb, color);
				{
					int		mr=(int)fr*2;
					int		mg=(int)fg*2;
					int		mb=(int)fb*2;

					for(y=0; y<h; y++)
						for(x=0; x<w; x++)
						{
							byte	r,g,b,a;
							colorRGBA(&r, &g, &b, &a, *(s++));
							(*d++)=color32(mini(((int)r*mr)>>8, 255), mini(((int)g*mg)>>8, 255), mini(((int)b*mb)>>8, 255), a);
						}
				}
			}

//#define DISPINFO
#ifdef DISPINFO
			{
				static double	v[100];
				static double	t[100];
				static int	nb=0;
				Afont	*f=alib.getFont(fontTERMINAL06);
				int		i;
				double		min,max;
				double	sum=0.0;
				for(i=99; i>=1; i--)
					v[i]=v[i-1];
				min=max=v[0]=dtime;
				if(nb++>50)
				{
					for(i=0; i<100; i++)
						t[i]=v[i];
					nb=0;
				}
				for(i=0; i<100; i++)
				{
					min=mini(min, v[i]);
					max=maxi(max, v[i]);
					sum=(double)t[i];
				}
				{
					char	str[256];
					sprintf(str, "min %6.3f    max %6.3f   FPS %3d   iFPS %6.3f", (float)min, (float)max, getTable()->frameRate, (float)getTable()->instantFPS);
					f->set(image, 10, 10, str, 0xff00ff00);
					sprintf(str, "moy %6.3f  wait %3d   max/min %6.3f", (float)(sum/100.0), getTable()->instantWait, (float)max/min);
					f->set(image, 10, 20, str, 0xff00ff00);
					for(i=0; i<10; i++)
					{
						sprintf(str, "%6.3f", (float)t[i]);
						f->set(image, 10, 40+i*10, str, 0xff00ff00);
					}
				}
			}
#endif
		}
		if(!OKI_registred)
		{
			if(!notregistred)
				notregistred=new Abitmap(&resource.get(MAKEINTRESOURCE(PNG_EXPORT_AESTALPHA), "PNG"));
			image->set(0, 0, image->w, image->h, 0, 0, notregistred->w, notregistred->h, notregistred);
		}
		{
			Abitmap *bo=((AvoutBack *)back)->out->getBitmap();
			if(bo)
				bo->set(0, 0, image, bitmapNORMAL, bitmapNORMAL);
		}
		sectionImage.leave();
		from->leave();

		if(win||gdi||dx)
		{
			if(win)
				win->notify((Anode *)this, nyCHANGE, 0);
			if(gdi)
				PostThreadMessage(this->threadID, WM_USER, nyCHANGE, 0);
			if(dx)
				dx->display(image);
		}
	}
	winChange.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avout::run()
{
	MSG msg;
	memset(&msg, 0, sizeof(msg));
	win=NULL;
	gdi=NULL;
	if(current==0)
	{
		error=1;
		goto lblout;
	}
	if(current==1)
	{
		win=new AvoutDisp("videoOut display", winX, winY, getVideoWidth(), getVideoHeight(), this);
		win->show(TRUE);
	}
	else if(current==2)
	{
		win=new Avoutovr("videoOut display", winX, winY, getVideoWidth(), getVideoHeight(), this);
		if(!((Avoutovr *)win)->isOK)
		{
			error=-1;
			goto lblout;
		}
		win->repaint();
		win->show(TRUE);
	}
	else if((current>=10)&&(current<42))
	{
		assert(false); 
		/*
		if(current-10<nbDDinfo)
		{
			win=new AdirectX("videoOut display", current-10, table->videoW, table->videoH, this);
			if(!((AdirectX *)win)->allOK)
			{
				error=-1;
				goto lblout;
			}
			win->show(TRUE);
		}
		else
		{
			error=-1;
			goto lblout;
		}
		*/
	}
	else if((current>=100)&&(current<120))	// gdi (display)
	{
		int	n=current-100;
		if(n<nbGDIinfo)
		{
			gdi=new Agdi(this, GDIinfo[n].name, GDIinfo[n].desc);
			if(!gdi->isOK)
			{
				error=-1;
				goto lblout;
			}
		}
		else
		{
			error=-1;
			goto lblout;
		}
	}
	else
	{
		error=-1;
		return;
	}
	winrun=TRUE;
	runout=false;
	error=1;
	//setPriority(priorityHIGH);
	while(winrun)
	{
		while(PeekMessage(&msg, NULL, 0, 0, TRUE)&&(msg.message!=WM_QUIT)&&(winrun))
		{
			int	n=WM_USER;
			if((msg.hwnd==NULL)&&(msg.message==WM_USER)&&(msg.wParam==nyCHANGE))
			{
				gdi->repaint();
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		sleep(1);
	}
lblout:
	if(win)
		delete(win);
	win=NULL;
	if(gdi)
		delete(gdi);
	gdi=NULL;
	runout=true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AvoutFront::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

AvoutFront::AvoutFront(QIID qiid, char *name, Avout *e, int h) : AeffectFront(qiid, name, e, h)
{
	setTooltips("videoOut module");
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_VIDEOOUT), "PNG");
	back=new Abitmap(&o);

	items=new Aitem("video out", "video out");

	list=new Alist("video out", this, 16, 15, 218, 20, items);
	list->setTooltips("video out selector");
	list->show(TRUE);

	if(!dxInit)
	{
		DDenum();
		dxInit=TRUE;
	}

	if(!gdiInit)
	{
		GDIenum();
		gdiInit=true;
	}

	new Aitem("[none]", "no video out", items, 0);
	new Aitem("[window] a window", "a simple window", items, 1);
	new Aitem("[window] overlay window", "overlay window - activate dual head DVDmax on matrox board", items, (dword)2);
	{  
		int	i;
		for(i=0; i<nbDDinfo; i++)
		{
			char	str[256];
			sprintf(str, "[direct-x]   %s (%d)", DDinfo[i].desc, i+1);
			new Aitem(str, DDinfo[i].desc, items, (dword)(i+10));
		}
	}
	{
		int	i;
		for(i=0; i<nbGDIinfo; i++)
		{
			char	str[256];
			sprintf(str, "[gdi] %s (%d)", GDIinfo[i].desc, i+1);
			new Aitem(str, GDIinfo[i].desc, items, (dword)(i+100));
		}
	}

	source=new Aselect(MKQIID(qiid, 0x25211d79e7b4ab00), "source", this, 260, 18, 4, 1, &resource.get(MAKEINTRESOURCE(PNG_VMIX_SOURCES), "PNG"), 16, 16);
	source->setTooltips("source selector");
	source->set(0);
	source->show(TRUE);

	color=new Aselcolor(MKQIID(qiid, 0xd7b0e52e11bea540), "color", this, 470, 16);
	color->setTooltips("color");
	color->set(0xff808080);
	color->show(true);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AvoutFront::~AvoutFront()
{
	delete(back);
	delete(list);
	delete(items);
	delete(source);
	delete(color);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AvoutFront::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCHANGE:
		if(o==list)
		{
			if(!((Avout *)effect)->change(list->getCurrentData()))
				list->setCurrentByData(0);
		}
		break;
	}
	return AeffectFront::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AvoutFront::pulse()
{

};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static QIID qiidin[]={ 0x6dfbd4a491968b10, 0xe269496f5cb3ab60, 0xf2f087fcdd042ef4, 0x1019bcff969832a0};

AvoutBack::AvoutBack(QIID qiid, char *name, Avout *e, int h) : AeffectBack(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_VIDEOOUT2), "PNG");
	back=new Abitmap(&o);

	{
		int	i;
		for(i=0; i<4; i++)
		{
			char	str[128];
			sprintf(str, "video in %d", i+1);
			in[i]=new Avideo(MKQIID(qiid, qiidin[i]), str, this, pinIN, 10+20*i, 10);
			in[i]->setTooltips(str);
			in[i]->show(TRUE);
		}
	}

	out=new Avideo(MKQIID(qiid, 0x1d7e02f0f23f6dc0), "video out", this, pinOUT, pos.w-18, 10);
	out->setTooltips("video out");
	out->show(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AvoutBack::~AvoutBack()
{
	delete(back);
	delete(in[0]);
	delete(in[1]);
	delete(in[2]);
	delete(in[3]);
	delete(out);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AvoutBack::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aeffect * AvoutInfo::create(QIID qiid, char *name, Acapsule *capsule)
{
	return new Avout(qiid, name, this, capsule);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aplugz * voutGetInfo()
{
	return new AvoutInfo("voutInfo", &Avout::CI, "video out", "video out module");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *DDErrorString(HRESULT hr)
{
	switch (hr)
	{
	case DDERR_ALREADYINITIALIZED:           return "DDERR_ALREADYINITIALIZED";
	case DDERR_CANNOTATTACHSURFACE:          return "DDERR_CANNOTATTACHSURFACE";
	case DDERR_CANNOTDETACHSURFACE:          return "DDERR_CANNOTDETACHSURFACE";
	case DDERR_CURRENTLYNOTAVAIL:            return "DDERR_CURRENTLYNOTAVAIL";
	case DDERR_EXCEPTION:                    return "DDERR_EXCEPTION";
	case DDERR_GENERIC:                      return "DDERR_GENERIC";
	case DDERR_HEIGHTALIGN:                  return "DDERR_HEIGHTALIGN";
	case DDERR_INCOMPATIBLEPRIMARY:          return "DDERR_INCOMPATIBLEPRIMARY";
	case DDERR_INVALIDCAPS:                  return "DDERR_INVALIDCAPS";
	case DDERR_INVALIDCLIPLIST:              return "DDERR_INVALIDCLIPLIST";
	case DDERR_INVALIDMODE:                  return "DDERR_INVALIDMODE";
	case DDERR_INVALIDOBJECT:                return "DDERR_INVALIDOBJECT";
	case DDERR_INVALIDPARAMS:                return "DDERR_INVALIDPARAMS";
	case DDERR_INVALIDPIXELFORMAT:           return "DDERR_INVALIDPIXELFORMAT";
	case DDERR_INVALIDRECT:                  return "DDERR_INVALIDRECT";
	case DDERR_LOCKEDSURFACES:               return "DDERR_LOCKEDSURFACES";
	case DDERR_NO3D:                         return "DDERR_NO3D";
	case DDERR_NOALPHAHW:                    return "DDERR_NOALPHAHW";
	case DDERR_NOCLIPLIST:                   return "DDERR_NOCLIPLIST";
	case DDERR_NOCOLORCONVHW:                return "DDERR_NOCOLORCONVHW";
	case DDERR_NOCOOPERATIVELEVELSET:        return "DDERR_NOCOOPERATIVELEVELSET";
	case DDERR_NOCOLORKEY:                   return "DDERR_NOCOLORKEY";
	case DDERR_NOCOLORKEYHW:                 return "DDERR_NOCOLORKEYHW";
	case DDERR_NODIRECTDRAWSUPPORT:          return "DDERR_NODIRECTDRAWSUPPORT";
	case DDERR_NOEXCLUSIVEMODE:              return "DDERR_NOEXCLUSIVEMODE";
	case DDERR_NOFLIPHW:                     return "DDERR_NOFLIPHW";
	case DDERR_NOGDI:                        return "DDERR_NOGDI";
	case DDERR_NOMIRRORHW:                   return "DDERR_NOMIRRORHW";
	case DDERR_NOTFOUND:                     return "DDERR_NOTFOUND";
	case DDERR_NOOVERLAYHW:                  return "DDERR_NOOVERLAYHW";
	case DDERR_NORASTEROPHW:                 return "DDERR_NORASTEROPHW";
	case DDERR_NOROTATIONHW:                 return "DDERR_NOROTATIONHW";
	case DDERR_NOSTRETCHHW:                  return "DDERR_NOSTRETCHHW";
	case DDERR_NOT4BITCOLOR:                 return "DDERR_NOT4BITCOLOR";
	case DDERR_NOT4BITCOLORINDEX:            return "DDERR_NOT4BITCOLORINDEX";
	case DDERR_NOT8BITCOLOR:                 return "DDERR_NOT8BITCOLOR";
	case DDERR_NOTEXTUREHW:                  return "DDERR_NOTEXTUREHW";
	case DDERR_NOVSYNCHW:                    return "DDERR_NOVSYNCHW";
	case DDERR_NOZBUFFERHW:                  return "DDERR_NOZBUFFERHW";
	case DDERR_NOZOVERLAYHW:                 return "DDERR_NOZOVERLAYHW";
	case DDERR_OUTOFCAPS:                    return "DDERR_OUTOFCAPS";
	case DDERR_OUTOFMEMORY:                  return "DDERR_OUTOFMEMORY";
	case DDERR_OUTOFVIDEOMEMORY:             return "DDERR_OUTOFVIDEOMEMORY";
	case DDERR_OVERLAYCANTCLIP:              return "DDERR_OVERLAYCANTCLIP";
	case DDERR_OVERLAYCOLORKEYONLYONEACTIVE: return "DDERR_OVERLAYCOLORKEYONLYONEACTIVE";
	case DDERR_PALETTEBUSY:                  return "DDERR_PALETTEBUSY";
	case DDERR_COLORKEYNOTSET:               return "DDERR_COLORKEYNOTSET";
	case DDERR_SURFACEALREADYATTACHED:       return "DDERR_SURFACEALREADYATTACHED";
	case DDERR_SURFACEALREADYDEPENDENT:      return "DDERR_SURFACEALREADYDEPENDENT";
	case DDERR_SURFACEBUSY:                  return "DDERR_SURFACEBUSY";
	case DDERR_CANTLOCKSURFACE:              return "DDERR_CANTLOCKSURFACE";
	case DDERR_SURFACEISOBSCURED:            return "DDERR_SURFACEISOBSCURED";
	case DDERR_SURFACELOST:                  return "DDERR_SURFACELOST";
	case DDERR_SURFACENOTATTACHED:           return "DDERR_SURFACENOTATTACHED";
	case DDERR_TOOBIGHEIGHT:                 return "DDERR_TOOBIGHEIGHT";
	case DDERR_TOOBIGSIZE:                   return "DDERR_TOOBIGSIZE";
	case DDERR_TOOBIGWIDTH:                  return "DDERR_TOOBIGWIDTH";
	case DDERR_UNSUPPORTED:                  return "DDERR_UNSUPPORTED";
	case DDERR_UNSUPPORTEDFORMAT:            return "DDERR_UNSUPPORTEDFORMAT";
	case DDERR_UNSUPPORTEDMASK:              return "DDERR_UNSUPPORTEDMASK";
	case DDERR_VERTICALBLANKINPROGRESS:      return "DDERR_VERTICALBLANKINPROGRESS";
	case DDERR_WASSTILLDRAWING:              return "DDERR_WASSTILLDRAWING";
	case DDERR_XALIGN:                       return "DDERR_XALIGN";
	case DDERR_INVALIDDIRECTDRAWGUID:        return "DDERR_INVALIDDIRECTDRAWGUID";
	case DDERR_DIRECTDRAWALREADYCREATED:     return "DDERR_DIRECTDRAWALREADYCREATED";
	case DDERR_NODIRECTDRAWHW:               return "DDERR_NODIRECTDRAWHW";
	case DDERR_PRIMARYSURFACEALREADYEXISTS:  return "DDERR_PRIMARYSURFACEALREADYEXISTS";
	case DDERR_NOEMULATION:                  return "DDERR_NOEMULATION";
	case DDERR_REGIONTOOSMALL:               return "DDERR_REGIONTOOSMALL";
	case DDERR_CLIPPERISUSINGHWND:           return "DDERR_CLIPPERISUSINGHWND";
	case DDERR_NOCLIPPERATTACHED:            return "DDERR_NOCLIPPERATTACHED";
	case DDERR_NOHWND:                       return "DDERR_NOHWND";
	case DDERR_HWNDSUBCLASSED:               return "DDERR_HWNDSUBCLASSED";
	case DDERR_HWNDALREADYSET:               return "DDERR_HWNDALREADYSET";
	case DDERR_NOPALETTEATTACHED:            return "DDERR_NOPALETTEATTACHED";
	case DDERR_NOPALETTEHW:                  return "DDERR_NOPALETTEHW";
	case DDERR_BLTFASTCANTCLIP:              return "DDERR_BLTFASTCANTCLIP";
	case DDERR_NOBLTHW:                      return "DDERR_NOBLTHW";
	case DDERR_NODDROPSHW:                   return "DDERR_NODDROPSHW";
	case DDERR_OVERLAYNOTVISIBLE:            return "DDERR_OVERLAYNOTVISIBLE";
	case DDERR_NOOVERLAYDEST:                return "DDERR_NOOVERLAYDEST";
	case DDERR_INVALIDPOSITION:              return "DDERR_INVALIDPOSITION";
	case DDERR_NOTAOVERLAYSURFACE:           return "DDERR_NOTAOVERLAYSURFACE";
	case DDERR_EXCLUSIVEMODEALREADYSET:      return "DDERR_EXCLUSIVEMODEALREADYSET";
	case DDERR_NOTFLIPPABLE:                 return "DDERR_NOTFLIPPABLE";
	case DDERR_CANTDUPLICATE:                return "DDERR_CANTDUPLICATE";
	case DDERR_NOTLOCKED:                    return "DDERR_NOTLOCKED";
	case DDERR_CANTCREATEDC:                 return "DDERR_CANTCREATEDC";
	case DDERR_NODC:                         return "DDERR_NODC";
	case DDERR_WRONGMODE:                    return "DDERR_WRONGMODE";
	case DDERR_IMPLICITLYCREATED:            return "DDERR_IMPLICITLYCREATED";
	case DDERR_NOTPALETTIZED:                return "DDERR_NOTPALETTIZED";
	case DDERR_UNSUPPORTEDMODE:              return "DDERR_UNSUPPORTEDMODE";
	case DDERR_NOMIPMAPHW:                   return "DDERR_NOMIPMAPHW";
	case DDERR_INVALIDSURFACETYPE:           return "DDERR_INVALIDSURFACETYPE";
	case DDERR_DCALREADYCREATED:             return "DDERR_DCALREADYCREATED";
	case DDERR_CANTPAGELOCK:                 return "DDERR_CANTPAGELOCK";
	case DDERR_CANTPAGEUNLOCK:               return "DDERR_CANTPAGEUNLOCK";
	case DDERR_NOTPAGELOCKED:                return "DDERR_NOTPAGELOCKED";
	case DDERR_NOTINITIALIZED:               return "DDERR_NOTINITIALIZED";
	}
	return "Unknown Error";
}
