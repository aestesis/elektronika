/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	ef3f-stream.CPP				(c)	YoY'03						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<math.h>
#include						<d3d9.h>
#include						<D3dx9mesh.h>
#include						<dshow.h>
#include						<vmr9.h>
#include						<assert.h>
#include						<atlbase.h>

#include						"ef3d-stream.h"
#include						"resource.h"

#include						"../COMobjects/COMobjexp.h"				// my COM objects

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Aef3dstreamInfo::CI	= ACI("Aef3dstreamInfo",	GUID(0x11111112,0xB0000080), &Ainput3dInfo::CI, 0, NULL);
ACI								Aef3dstream::CI		= ACI("Aef3dstream",		GUID(0x11111112,0xB0000081), &Ainput3d::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define							TEXSIZE								512

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void b2wchar(WCHAR *dest, char *src)
{
	while(*src)
		*(dest++)=*(src++);
	*dest=0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class IelekAlloc : IVMRSurfaceAllocator9, IVMRImagePresenter9
{
public:

// IUnknown
ULONG __stdcall					AddRef								();
ULONG __stdcall					Release								();
HRESULT __stdcall				QueryInterface						(REFIID riid, void** ppv);

// IVMRSurfaceAllocator9
HRESULT	__stdcall				InitializeDevice					(DWORD_PTR,VMR9AllocationInfo *,DWORD *);
HRESULT	__stdcall				TerminateDevice						(DWORD_PTR);
HRESULT	__stdcall				GetSurface							(DWORD_PTR,DWORD,DWORD,IDirect3DSurface9 ** );
HRESULT	__stdcall				AdviseNotify						(IVMRSurfaceAllocatorNotify9 *);

// IVMRImagePresenter9
HRESULT	__stdcall				StartPresenting						(DWORD_PTR);
HRESULT	__stdcall				StopPresenting						(DWORD_PTR);
HRESULT	__stdcall				PresentImage						(DWORD_PTR,VMR9PresentationInfo *);

// IelekAlloc
								IelekAlloc							(Ad3d *d3d, bool full);
								~IelekAlloc							();
								
IDirect3DTexture9 *				getTexture							(float *cx, float *cy);

    Asection					section;

private:
    ULONG						m_cRef;
    Ad3d						*d3d;
    IDirect3DTexture9			*texture;
    IDirect3DTexture9			*textemp;
	IDirect3DTexture9			*texsurf;
    //IDirect3DSurface9			*surface;
    IVMRSurfaceAllocatorNotify9	*notify;
    float						cx,cy;
    bool						full;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IelekAlloc::IelekAlloc(Ad3d *d3d, bool full) : m_cRef(1)
{
	this->d3d=d3d;
	texture=NULL;
	textemp=NULL;
	texsurf=NULL;
	//surface=NULL;
	notify=NULL;
	this->full=full;
	
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IelekAlloc::~IelekAlloc()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IDirect3DTexture9 * IelekAlloc::getTexture(float *cx, float *cy)
{
	*cx=this->cx;
	*cy=this->cy;
	if(texture)
		texture->AddRef();
	return texture;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ULONG IelekAlloc::AddRef()
{
    return ++m_cRef;
}

ULONG IelekAlloc::Release()
{
    if(--m_cRef!=0)
        return m_cRef;
    delete this;
    return 0;
}

HRESULT IelekAlloc::QueryInterface(REFIID riid, void** ppv)
{
    if(riid == IID_IUnknown)
        *ppv = (IUnknown*)(IVMRSurfaceAllocator9*)this;
    else if(riid == IID_IVMRSurfaceAllocator9)
        *ppv = (IVMRSurfaceAllocator9*)this;
    else if(riid == IID_IVMRImagePresenter9)
        *ppv = (IVMRImagePresenter9*)this;
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IelekAlloc::InitializeDevice(DWORD_PTR user, VMR9AllocationInfo *info, DWORD *nbuf)
{
	cx=0.f;
	cy=0.f;

	if(D3DXCreateTexture(d3d->d3dev, info->dwWidth, info->dwHeight, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture)!=D3D_OK)
		return S_FALSE;

	if(D3DXCreateTexture(d3d->d3dev, info->dwWidth, info->dwWidth, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &texsurf)!=D3D_OK)
		return S_FALSE;


	cx=(float)1.f;
	cy=(float)1.f;
/*	
	if(full)
		if(D3DXCreateTexture(d3d->d3dev, s, s, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &textemp)!=D3D_OK)
			return S_FALSE;
*/	
	return S_OK;
}

HRESULT IelekAlloc::TerminateDevice(DWORD_PTR user)
{
	if(texture)
		texture->Release();
	texture=NULL;

	if(texsurf)
		texsurf->Release();
	texsurf=NULL;

	if(notify)
		notify->Release();
	notify=NULL;

	return S_OK;
}

HRESULT IelekAlloc::GetSurface(DWORD_PTR user, DWORD index, DWORD flags, IDirect3DSurface9 **surf)
{
	return texsurf->GetSurfaceLevel(0, surf);
}

HRESULT IelekAlloc::AdviseNotify(IVMRSurfaceAllocatorNotify9 *pAllocNotify)
{
	notify=pAllocNotify;
	notify->AddRef();
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IelekAlloc::StartPresenting(DWORD_PTR user)
{
	return S_OK;
}

HRESULT IelekAlloc::StopPresenting(DWORD_PTR user)
{
	return S_OK;
}

HRESULT IelekAlloc::PresentImage(DWORD_PTR user, VMR9PresentationInfo *info)
{
	HRESULT				hr=S_OK;

	section.enter(__FILE__,__LINE__);
	if(full)
	{
		/*
		IDirect3DSurface9	*stemp=NULL;
		
		if(textemp->GetSurfaceLevel(0, &stemp)==D3D_OK)
		{
			D3DSURFACE_DESC	desctemp;
			if(stemp->GetDesc(&desctemp)==D3D_OK)
			{
				D3DSURFACE_DESC	descsurf;
				if(surface->GetDesc(&descsurf)==D3D_OK)
				{
					HDC dctemp;
					if(stemp->GetDC(&dctemp)==D3D_OK)
					{
						HDC dcsurf;
						if(surface->GetDC(&dcsurf)==D3D_OK)
						{
							if(StretchBlt(dctemp, 0, 0, desctemp.Width, desctemp.Height, dcsurf, 0, 0, descsurf.Width, descsurf.Height, SRCCOPY))
							{
								texture->AddDirtyRect(NULL);
								hr=d3d->d3dev->UpdateTexture(textemp, texture);
							}
							surface->ReleaseDC(dcsurf);
						}
						stemp->ReleaseDC(dctemp);
					}
				}
			}
			stemp->Release();
		}
		*/
	}
	else
	{
		texture->AddDirtyRect(NULL);
		d3d->d3dev->UpdateTexture(texsurf, texture);
		/*
		IDirect3DSurface9	*stex=NULL;
		if(texture->GetSurfaceLevel(0, &stex)==D3D_OK)
		{
			RECT	r;
			POINT	p;

			r.top=0;
			r.left=0;
			r.right=info->szAspectRatio.cx;
			r.bottom=info->szAspectRatio.cy;
			
			p.x=0;
			p.y=0;
			
			hr=d3d->d3dev->UpdateSurface(surface, &r, stex, &p);
			stex->Release();
		}
		*/
	}
	section.leave();
	
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define TEXSIZE					512

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Adatastream
{
public:
	
	IGraphBuilder				*pGraph;
	ICaptureGraphBuilder2		*pBuild;
	IMediaControl				*pMC;
	IMediaSeeking				*pMS;
	IMediaPosition				*pMP;
	IVideoFrameStep				*pFS;
	IMediaEvent					*pME;
	IelekAlloc					*pAlloc;

	Asection					section;
	char						fname[4][ALIB_PATH];
	int							anim[4];
	int							current;
	bool						bload;
	bool						repaint;
	char						*error;

	Adatastream()
	{
		pGraph=NULL;
		pBuild=NULL;
		pMC=NULL;
		pMS=NULL;
		pMP=NULL;
		pFS=NULL;
		pME=NULL;
		pAlloc=NULL;
		strcpy(fname[0], "NOTHING");
		strcpy(fname[1], "NOTHING");
		strcpy(fname[2], "NOTHING");
		strcpy(fname[3], "NOTHING");
		current=-1;
		anim[0]=anim[1]=anim[2]=anim[3]=0;
		bload=false;
		repaint=false;
		error=NULL;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static HRESULT InitCaptureGraphBuilder(
  IGraphBuilder **ppGraph,  // Receives the pointer.
  ICaptureGraphBuilder2 **ppBuild  // Receives the pointer.
)
{
    if (!ppGraph || !ppBuild)
    {
        return E_POINTER;
    }
    IGraphBuilder			*pGraph = NULL;
    ICaptureGraphBuilder2	*pBuild = NULL;

    // Create the Capture Graph Builder.
    HRESULT hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, 
        CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void**)&pBuild);
    if (SUCCEEDED(hr))
    {
        // Create the Filter Graph Manager.
        hr = CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER,
            IID_IGraphBuilder, (void**)&pGraph);
        if (SUCCEEDED(hr))
        {
            // Initialize the Capture Graph Builder.
            pBuild->SetFiltergraph(pGraph);

            // Return both interface pointers to the caller.
            *ppBuild = pBuild;
            *ppGraph = pGraph; // The caller must release both interfaces.
            return S_OK;
        }
        else
        {
            pBuild->Release();
        }
    }
    return hr; // Failed
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int						count=0;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

Aef3dstream::Aef3dstream(QIID qiid, char *name, Aobject *f, int x, int y) : Ainput3d(qiid, name, f, x, y)
{
	char	str[1024];
	init();
	initOK=false;

	setTooltips("stream effect - play movie stream [warning beta input effect, could crash the system]");

	sprintf(str, "%s/movie", name);
	sel=new Aselect(MKQIID(qiid,0x9acf6bb5d7c5e4e0), str, this, 9, 9, 4, 1, &resource.get(MAKEINTRESOURCE(PNG_EF3D_SOURCE), "PNG"), 16, 16);
	sel->setTooltips("movie selector");
	sel->set(0);
	sel->show(TRUE);

	sprintf(str, "%s/pitch", name);
	pitch=new Apaddle(MKQIID(qiid,0xf853c3f821f93a40), str, this, 84, 9, paddleYOLI16);
	pitch->setTooltips("pitch");
	pitch->set(0.5f);
	pitch->show(TRUE);

	dt=new Adatastream();
	dt->current=0;
	settings(false);
	
	timer(100);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aef3dstream::~Aef3dstream()
{
	delete(dt);
	delete(sel);
	delete(pitch);
	end();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aef3dstream::pulse()
{
	bool	brepaint=false;
	char	*err=NULL;
	dt->section.enter(__FILE__,__LINE__);
	err=dt->error;
	dt->error=NULL;
	brepaint=dt->repaint;
	dt->repaint=false;
	dt->section.leave();
	if(brepaint)
		repaint();
	if(err)
		notify(this, nyERROR, (dword)err);		
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dstream::dragdrop(int x, int y, int state, int event, void *data)
{
	switch(event)
	{
		case dragdropOVER:
		if((x>=8)&&(x<=100)&&(y>=30)&&(y<=70))
			return true;
		break;
		
		case dragdropDROP:
		if(state&dragdropFILE)
		{
			dt->section.enter(__FILE__,__LINE__);
			{
				int		nb=4;
				char	*s=(char *)data;
				int		n=sel->get();
			
				while(*s&&(nb--))
				{
					char	name[ALIB_PATH];
					
					strcpy(name, s);
					strupr(name);
					
					if(strstr(name, ".AVI")||strstr(name, ".MPG")||strstr(name, ".MPEG"))
					{
						strcpy(dt->fname[n], name);
						n=(n+1)&3;
					}
					else
						this->notify(this, nyERROR, (dword)"load only AVI, MPEG files");
					s+=strlen(s)+1;
				}
			}
			dt->bload=true;
			dt->repaint=true;
			dt->section.leave();
		}
		break;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dstream::notify(Anode *o, int event, dword p)
{
	return Aobject::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aef3dstream::paint(Abitmap *b)
{
	Afont	*f=alib.getFont(fontTERMINAL06);
	int		i;
	
	b->boxfa(8, 30, 100, 70, 0xff001000, 0.2f);

	dt->section.enter(__FILE__,__LINE__);
	for(i=0; i<4; i++)
	{
		char	str[1024];
		int		w,l;
		
		sprintf(str, "[%d]", i);
		w=f->getWidth(str);
		f->set(b, 20-(w>>1), 10*i+32, str, 0xff404040);
		strcpy(str, dt->fname[i]);
		l=strlen(str);
		f->set(b, 40, 10*i+32, &str[maxi(l-9, 0)], 0xff404040);
		//sprintf(str, "(%d)", dt->anim[i]);
		//w=f->getWidth(str);
		//f->set(b, 180-(w>>1), 10*i+8, str, 0xff404040);
	}
	dt->repaint=false;
	dt->section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dstream::save(class Afile *f)
{
	char	relpath[ALIB_PATH];
	int		i;
	
	for(i=0; i<4; i++)
	{
		if(!strcmp(dt->fname[i], "NOTHING"))
		{
			f->writeString(dt->fname[i]);
		}
		else
		{
			if(f->relativePath(relpath, dt->fname[i]))
				f->writeString(relpath);
			else
				f->writeString(dt->fname[i]);
		}
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dstream::load(class Afile *f)
{
	int		i;
	char	fn[ALIB_PATH];
	char	abs[ALIB_PATH];
	for(i=0; i<4; i++)
	{
		f->readString(fn);
		if(!strcmp(fn, "NOTHING"))
		{
			strcpy(dt->fname[i], fn);
		}
		else
		{
			if(f->absoluPath(abs, fn))
				strcpy(dt->fname[i], abs);
			else
				strcpy(dt->fname[i], fn);
		}
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aef3dstream::settings(bool emergency)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dstream::init3D(class Aef3dInfo *info)
{
	WCHAR	wname[ALIB_PATH];
	char	bname[ALIB_PATH];
	
	initOK=false;

	dt->section.enter(__FILE__,__LINE__);
	strcpy(bname, dt->fname[dt->current]);
	dt->section.leave();

	if(!strcmp(bname, "NOTHING"))
		return true;
	
	b2wchar(wname, bname);
	
	if(InitCaptureGraphBuilder(&dt->pGraph, &dt->pBuild)!=S_OK)
	{
		dt->error="can't create DS graphBuilder [out of memory]";
		dt->pGraph=NULL;
		dt->pBuild=NULL;
		return false;
	}
		
	{
		HRESULT						hr=NULL;
		IBaseFilter					*pVmr=NULL;
		IBaseFilter					*pSource=NULL;
		IVMRFilterConfig9			*pConfig=NULL;
		IVMRSurfaceAllocatorNotify9	*pNotify=NULL;
		DWORD_PTR					user=1;
		IVMRSurfaceAllocator9		*al;

		dt->pAlloc=new IelekAlloc(info->d3d, (info->effect->getTEXtype()==ef3dTEXfull));
		dt->pAlloc->QueryInterface(IID_IVMRSurfaceAllocator9, (void **)&al);
		if(hr!=D3D_OK)
		{
			dt->error="can't create surface [out of video memory]";
			dt->pAlloc->Release();
			dt->pAlloc=NULL;
			dt->pGraph->Release();
			dt->pBuild->Release();
			dt->pGraph=NULL;
			dt->pBuild=NULL;
			return false;
		}
		
		hr=CoCreateInstance(CLSID_VideoMixingRenderer9, 0, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pVmr);
		if(hr!=D3D_OK)
		{
			dt->error="can't create VMR [file format error or out of video memory]";
			al->Release();
			dt->pAlloc->Release();
			dt->pAlloc=NULL;
			dt->pGraph->Release();
			dt->pBuild->Release();
			dt->pGraph=NULL;
			dt->pBuild=NULL;
			return false;
		}
		hr=dt->pGraph->AddFilter(pVmr, L"VMR9");
		
		hr=pVmr->QueryInterface(IID_IVMRFilterConfig9, (void**)&pConfig);
		if(hr!=D3D_OK)
		{
			dt->error="can't create VMR config utility [out of memory]";
			pVmr->Release();
			pVmr=NULL;
			al->Release();
			dt->pAlloc->Release();
			dt->pAlloc=NULL;
			dt->pGraph->Release();
			dt->pBuild->Release();
			dt->pGraph=NULL;
			dt->pBuild=NULL;
			return false;
		}
		hr=pConfig->SetRenderingMode(VMR9Mode_Renderless);
		pConfig->Release();

		hr=pVmr->QueryInterface(IID_IVMRSurfaceAllocatorNotify9, (void**)&pNotify);
		if(hr!=D3D_OK)
		{
			dt->error="can't create VMR notify utility [out of memory]";
			pVmr->Release();
			pVmr=NULL;
			al->Release();
			dt->pAlloc->Release();
			dt->pAlloc=NULL;
			dt->pGraph->Release();
			dt->pBuild->Release();
			dt->pGraph=NULL;
			dt->pBuild=NULL;
			return false;
		}
		hr=pNotify->AdviseSurfaceAllocator(user, al);
		pNotify->Release();
		al->Release();

		hr=dt->pGraph->AddSourceFilter(wname, L"Source1", &pSource);	
		if(hr!=D3D_OK)
		{
			dt->error="can't open movie file [file or format error]";
			pVmr->Release();
			pVmr=NULL;
			dt->pAlloc->Release();
			dt->pAlloc=NULL;
			dt->pGraph->Release();
			dt->pBuild->Release();
			dt->pGraph=NULL;
			dt->pBuild=NULL;
			return false;
		}
		hr=dt->pBuild->RenderStream(0, 0, pSource, 0, pVmr);
		pSource->Release();

		dt->pGraph->QueryInterface(IID_IVideoFrameStep, (void**)&dt->pFS);
		dt->pGraph->QueryInterface(IID_IMediaPosition, (void**)&dt->pMP);
		dt->pGraph->QueryInterface(IID_IMediaSeeking, (void**)&dt->pMS);
		dt->pGraph->QueryInterface(IID_IMediaEvent, (void**)&dt->pME);
		
        hr=dt->pGraph->QueryInterface(IID_IMediaControl, (void**)&dt->pMC);
        if(SUCCEEDED(hr)) 
        {
			hr=dt->pMC->Run();
        }
        else
        {
			dt->error="can't create media control [out of memory]";
			dt->pMC=NULL;
			pVmr->Release();
			pVmr=NULL;
			dt->pAlloc->Release();
			dt->pAlloc=NULL;
			dt->pGraph->Release();
			dt->pBuild->Release();
			dt->pGraph=NULL;
			dt->pBuild=NULL;
			return false;
        }
		pVmr->Release();
		pVmr=NULL;
	}
	
	initOK=true;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dstream::release3D(class Aef3dInfo *info)
{
	initOK=false;
	
	if(dt->pMP)
	{
		dt->pMP->Release();
		dt->pMP=NULL;
	}
	
	if(dt->pMS)
	{
		dt->pMS->Release();
		dt->pMS=NULL;
	}
	
	if(dt->pFS)
	{
		dt->pFS->Release();
		dt->pFS=NULL;
	}
	
	if(dt->pMC)
	{
		dt->pMC->Stop();
		dt->pMC->Release();
	}
	dt->pMC=NULL;

	if(dt->pME)
		dt->pME->Release();
	dt->pME=NULL;
	
	if(dt->pBuild)
		dt->pBuild->Release();
	dt->pBuild=NULL;

	if(dt->pGraph)
		dt->pGraph->Release();
	dt->pGraph=NULL;
	
	if(dt->pAlloc)
		dt->pAlloc->Release();
	dt->pAlloc=NULL;

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dstream::action(class Aef3dInfo *info)
{
	{
		int	c=sel->get();
		dt->section.enter(__FILE__,__LINE__);
		if(dt->bload||(dt->current!=c))
		{
			dt->bload=false;
			dt->current=c;
			release3D(info);
			init3D(info);
		}
		dt->section.leave();
	}
	if(initOK)
	{
		dt->section.enter(__FILE__,__LINE__);
		if(dt->pME)
		{
			long	code,param1,param2;
			HRESULT	hr;
			while(dt->pME->GetEvent(&code, &param1, &param2, 0)==S_OK)
			{
				// Free memory associated with callback, since we're not using it
				hr = dt->pME->FreeEventParams(code, param1, param2);

				if(EC_COMPLETE == code)
				{
					LONGLONG pos=0;
					hr=-1;

					if(dt->pMS)
						hr=dt->pMS->SetPositions(&pos, AM_SEEKING_AbsolutePositioning , NULL, AM_SEEKING_NoPositioning);

					if(hr!=S_OK)
					{
						dt->pMC->Stop();
						dt->pMC->Run();
					}
                }
            }
        }
		if(dt->pMP)
		{
			float	p=pitch->get();
			if(p==0.f)
				p=0.001f;
			dt->pMP->put_Rate((p<0.5f)?(p*2.f):((p-0.5f)*10.f));
		}
		dt->section.leave();
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IDirect3DTexture9 *	Aef3dstream::getTexture(int *type, float *cx, float *cy)
{
	if(initOK)
	{
		IDirect3DTexture9	*tex=NULL;
		dt->pAlloc->section.enter(__FILE__,__LINE__);
		tex=dt->pAlloc?(dt->pAlloc->getTexture(cx, cy)):NULL;
		dt->pAlloc->section.leave();
		*type=ef3dTEXpart;
		return tex;
	}
	*type=ef3dTEXnone;
	*cx=0.f;
	*cy=0.f;
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Ainput3d * Aef3dstreamInfo::create(QIID qiid, char *name, Aobject *father, int x, int y)
{
	return new Aef3dstream(qiid, name, father, x, y);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aplugz * ef3dstreamGetInfo()
{
	return new Aef3dstreamInfo("stream", &Aef3dstream::CI, "stream", "stream - [beta]");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
