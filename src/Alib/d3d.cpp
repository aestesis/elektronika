/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"export.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	D3D.CPP						(c)	YoY'03						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<d3d9.h>
#include						<stdio.h>
#include						"d3d.h"
#include						"window.h"
#include						"interface.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL ACI						Ad3d::CI=ACI("Ad3d", GUID(0xAE57E515,0x00003000), &Anode::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Ad3d::Ad3d(char *name, dword flags)
{
	this->w=0;
	this->h=0;
	this->flags=flags;
	d3dev=NULL;
	isOK=(d3d=Direct3DCreate9(D3D_SDK_VERSION))?true:false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Ad3d::~Ad3d()
{
    if(d3dev!=NULL)
        d3dev->Release();
    if(d3d!=NULL)
        d3d->Release();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL int Ad3d::getAdapterCount()
{
	if(d3d)
		return d3d->GetAdapterCount();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL int Ad3d::getAdapterWidth(int adapter)
{
	D3DDISPLAYMODE	mode;
	memset(&mode, 0, sizeof(mode));
	d3d->GetAdapterDisplayMode(adapter, &mode);
	return mode.Width;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL int Ad3d::getAdapterHeight(int adapter)
{
	D3DDISPLAYMODE	mode;
	memset(&mode, 0, sizeof(mode));
	d3d->GetAdapterDisplayMode(adapter, &mode);
	return mode.Height;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Ad3d::getAdapterName(char *name, int adapter)
{
	if(d3d)
	{
		D3DADAPTER_IDENTIFIER9	id;
		if(d3d->GetAdapterIdentifier(adapter, 0, &id)==D3D_OK)
		{
			sprintf(name, "%s [%s]", id.Description, id.DeviceName);
			return true;
		}
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL HMONITOR Ad3d::getAdapterMonitor(int adapter)
{
	if(d3d)
		return d3d->GetAdapterMonitor(adapter);
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Ad3d::createDevice(Awindow *win, int adapter, bool fullscreen)
{
	HRESULT		r;
	int			w=640;
	int			h=480;
	
	
	if(d3dev) 
	{
		d3dev->Release();
		d3dev=NULL;
	}
	
	if(d3d)
	{
		D3DPRESENT_PARAMETERS d3dpp; 
		ZeroMemory(&d3dpp, sizeof(d3dpp));
		
		d3dpp.hDeviceWindow=win->hw;

		if(fullscreen)
		{
			HMONITOR	hm=getAdapterMonitor(adapter);
			MONITORINFO	mi;
			
			mi.cbSize=sizeof(mi);
			if(GetMonitorInfo(hm, &mi))
			{
				w=mi.rcMonitor.right-mi.rcMonitor.left;
				h=mi.rcMonitor.bottom-mi.rcMonitor.top;
			}

			d3dpp.Windowed=FALSE;
			d3dpp.BackBufferWidth=w;
			d3dpp.BackBufferHeight=h;
			d3dpp.BackBufferFormat=D3DFMT_X8R8G8B8;//D3DFMT_R5G6B5;
			d3dpp.SwapEffect=D3DSWAPEFFECT_DISCARD;
			d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
			d3dpp.BackBufferCount = 1;
			d3dpp.PresentationInterval=D3DPRESENT_INTERVAL_ONE;
			d3dpp.Flags=0;
			d3dpp.EnableAutoDepthStencil=TRUE;
			d3dpp.AutoDepthStencilFormat=D3DFMT_D16;
		}
		else
		{
			d3dpp.Windowed = true;
			d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
			d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
			d3dpp.BackBufferCount = 1;
			d3dpp.PresentationInterval=D3DPRESENT_INTERVAL_ONE;
			d3dpp.Flags=0;
			d3dpp.EnableAutoDepthStencil=TRUE;
			d3dpp.AutoDepthStencilFormat=D3DFMT_D16;

			w=win->pos.w;
			h=win->pos.h;
		}
		
		r=d3d->CreateDevice(adapter, D3DDEVTYPE_HAL, win->hw, D3DCREATE_MULTITHREADED|D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &d3dev);
		if(r!=D3D_OK)
			r=d3d->CreateDevice(adapter, D3DDEVTYPE_HAL, win->hw, D3DCREATE_MULTITHREADED|D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &d3dev);
		switch(r)
		{
			case D3DERR_INVALIDCALL:
			Alasterror="invalid call";
			break;
			
			case D3DERR_NOTAVAILABLE:
			Alasterror="not avaible";
			break;
			
			case D3DERR_OUTOFVIDEOMEMORY:
			Alasterror="out of video memory";
			break;
		}
		
		isOK=(r==D3D_OK);
		
		if(isOK)
		{
			this->w=w;
			this->h=h;
		}
		else
		{
			this->w=0;
			this->h=0;
		}
		
		return isOK;
	}
	
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Ad3d::createDevice(HWND hwnd, int adapter, bool fullscreen)
{
	HRESULT		r;
	int			w=640;
	int			h=480;
	
	
	if(d3dev)
	{
		d3dev->Release();
		d3dev=NULL;
	}
	
	if(d3d)
	{
		D3DPRESENT_PARAMETERS d3dpp; 
		ZeroMemory(&d3dpp, sizeof(d3dpp));
		
		d3dpp.hDeviceWindow=hwnd;

		if(fullscreen)
		{
			HMONITOR	hm=getAdapterMonitor(adapter);
			MONITORINFO	mi;
			
			mi.cbSize=sizeof(mi);
			if(GetMonitorInfo(hm, &mi))
			{
				w=mi.rcMonitor.right-mi.rcMonitor.left;
				h=mi.rcMonitor.bottom-mi.rcMonitor.top;
			}

			d3dpp.Windowed=FALSE;
			d3dpp.BackBufferWidth=w;
			d3dpp.BackBufferHeight=h;
			d3dpp.BackBufferFormat=D3DFMT_X8R8G8B8;//D3DFMT_R5G6B5;
			d3dpp.SwapEffect=D3DSWAPEFFECT_DISCARD;
			d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
			d3dpp.BackBufferCount = 1;
			d3dpp.PresentationInterval=D3DPRESENT_INTERVAL_ONE;
			d3dpp.Flags=0;
			d3dpp.EnableAutoDepthStencil=TRUE;
			d3dpp.AutoDepthStencilFormat=D3DFMT_D16;
		}
		else
		{
			RECT	r;
			d3dpp.Windowed = true;
			d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
			d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
			d3dpp.BackBufferCount = 1;
			d3dpp.PresentationInterval=D3DPRESENT_INTERVAL_ONE;
			d3dpp.Flags=0;
			d3dpp.EnableAutoDepthStencil=TRUE;
			d3dpp.AutoDepthStencilFormat=D3DFMT_D16;

			GetClientRect(hwnd, &r);
			w=r.right-r.left;
			h=r.bottom-r.top;
		}
		
		r=d3d->CreateDevice(adapter, D3DDEVTYPE_HAL, hwnd, D3DCREATE_MULTITHREADED|D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &d3dev);
		if(r!=D3D_OK)
			r=d3d->CreateDevice(adapter, D3DDEVTYPE_HAL, hwnd, D3DCREATE_MULTITHREADED|D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &d3dev);
		switch(r)
		{
			case D3DERR_INVALIDCALL:
			Alasterror="invalid call";
			break;
			
			case D3DERR_NOTAVAILABLE:
			Alasterror="not avaible";
			break;
			
			case D3DERR_OUTOFVIDEOMEMORY:
			Alasterror="out of video memory";
			break;
		}
		
		isOK=(r==D3D_OK);
		
		if(isOK)
		{
			this->w=w;
			this->h=h;
		}
		else
		{
			this->w=0;
			this->h=0;
		}
		
		return isOK;
	}
	
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void Ad3d::releaseDevice()
{
	if(d3dev)
	{
		d3dev->Release();
		d3dev=NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void * Ad3d::addSwapChain(class Awindow *win)
{
	IDirect3DSwapChain9		*chain=null;
	HRESULT					r;
	int						w=640;
	int						h=480;
	
	if(d3dev)
	{
		D3DPRESENT_PARAMETERS d3dpp; 
		ZeroMemory(&d3dpp, sizeof(d3dpp));
		
		d3dpp.hDeviceWindow=win->hw;

		{
			d3dpp.Windowed = true;
			d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
			d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
			d3dpp.BackBufferCount = 1;
			d3dpp.PresentationInterval=D3DPRESENT_INTERVAL_ONE;
			d3dpp.Flags=0;
			//d3dpp.EnableAutoDepthStencil=TRUE;
			//d3dpp.AutoDepthStencilFormat=D3DFMT_D16;

			w=win->pos.w;
			h=win->pos.h;
		}

		r=d3dev->CreateAdditionalSwapChain(&d3dpp, &chain);
		switch(r)
		{
			case D3DERR_INVALIDCALL:
			Alasterror="invalid call";
			break;
			
			case D3DERR_NOTAVAILABLE:
			Alasterror="not avaible";
			break;
			
			case D3DERR_OUTOFVIDEOMEMORY:
			Alasterror="out of video memory";
			break;
		}
	}
	return (void *)chain;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
