/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	VPONE.CPP					(c)	YoY'03						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<dshow.h>
#include						<math.h>
#include						<assert.h>
#include						<string.h>
#include						"tcpRemote.h"
#include						"../alib/defSampleGrabber.h"
#include						"resource.h"
#include						"vpone.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								AvponeInfo::CI	= ACI("AvponeInfo",		GUID(0x11111112,0x00000340), &AeffectInfo::CI, 0, NULL);
ACI								Avpone::CI		= ACI("Avpone",			GUID(0x11111112,0x00000341), &Aeffect::CI, 0, NULL);
ACI								AvponeFront::CI	= ACI("AvponeFront",	GUID(0x11111112,0x00000342), &AeffectFront::CI, 0, NULL);
ACI								AvponeBack::CI	= ACI("AvponeBack",		GUID(0x11111112,0x00000343), &AeffectBack::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define							RELEASE(pObject) if(pObject){ pObject->Release(); pObject = NULL;}

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

static HRESULT UnicodeToAnsi(LPCOLESTR pszW, LPSTR* ppszA)
{
    ULONG cbAnsi, cCharacters;
    DWORD dwError;

    // If input is null then just return the same.
    if (pszW == NULL)
    {
        *ppszA = NULL;
        return NOERROR;
    }

    cCharacters = wcslen(pszW)+1;
    // Determine number of bytes to be allocated for ANSI string. An
    // ANSI string can have at most 2 bytes per character (for Double
    // Byte Character Strings.)
    cbAnsi = cCharacters*2;

    // Use of the OLE allocator is not required because the resultant
    // ANSI  string will never be passed to another COM component. You
    // can use your own allocator.
    *ppszA = (LPSTR) malloc(cbAnsi);
    if (NULL == *ppszA)
        return E_OUTOFMEMORY;

    // Convert to ANSI.
    if (0 == WideCharToMultiByte(CP_ACP, 0, pszW, cCharacters, *ppszA,
                  cbAnsi, NULL, NULL))
    {
        dwError = GetLastError();
        CoTaskMemFree(*ppszA);
        *ppszA = NULL;
        return HRESULT_FROM_WIN32(dwError);
    }
    return NOERROR;

}

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

static HRESULT GetPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, IPin **ppPin)
{
    IEnumPins  *pEnum;
    IPin       *pPin;

	*ppPin=NULL;

    pFilter->EnumPins(&pEnum);
    while(pEnum->Next(1, &pPin, 0) == S_OK)
    {
        PIN_DIRECTION PinDirThis;
        pPin->QueryDirection(&PinDirThis);
		/*
		{
			PIN_INFO	pinfo;
			char		*name;
			pPin->QueryPinInfo(&pinfo);
			UnicodeToAnsi(pinfo.achName, &name);
			free(name);
		}
		*/
        if (PinDir == PinDirThis)
        {
			*ppPin = pPin;
			pEnum->Release();
			return S_OK;
        }
        pPin->Release();
    }
    pEnum->Release();
    return E_FAIL;  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void MyFreeMediaType(AM_MEDIA_TYPE& mt)
{
    if (mt.cbFormat != 0)
    {
        CoTaskMemFree((PVOID)mt.pbFormat);
        mt.cbFormat = 0;
        mt.pbFormat = NULL;
    }
    if (mt.pUnk != NULL)
    {
        // Unecessary because pUnk should not be used, but safest.
        mt.pUnk->Release();
        mt.pUnk = NULL;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void MyDeleteMediaType(AM_MEDIA_TYPE *pmt)
{
    if (pmt != NULL)
    {
        MyFreeMediaType(*pmt); // See FreeMediaType for the implementation.
        CoTaskMemFree(pmt);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void sprintfTime(char *str, double time)
{
	int h=0;
	int m=0;
	int s=0;
	int ms=0;
	h=(int)(time/3600.0);
	time-=((double)h*3600.0);
	m=(int)(time/60.0);
	time-=((double)m*60.0);
	s=(int)time;
	time-=(double)s;
	ms=(int)(time*100);
	sprintf(str, "%2d:%2d:%2d.%2d", h, m, s, ms);
	{
		char	*s=str;
		while(*s)
		{
			if(*s==' ')
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

class IvideoCB : public ISampleGrabberCB
{

	HRESULT	__stdcall			SampleCB					(double,struct IMediaSample *) {return 0;}
	HRESULT __stdcall			BufferCB					(double SampleTime, BYTE *pBuffer, long BufferLen);

private:
	//! COM reference count
	ULONG _ref_count;

public:
	//! Return a ptr to a different COM interface to this object
	HRESULT APIENTRY QueryInterface( REFIID iid, void** ppvObject )
	{
		// Match the interface and return the proper pointer
		if ( iid == IID_IUnknown) {
			*ppvObject = dynamic_cast<IUnknown*>( this );
		} else if ( iid == IID_ISampleGrabberCB ) {
			*ppvObject = dynamic_cast<ISampleGrabberCB*>( this );
		} else {
			// It didn't match an interface
			*ppvObject = NULL;
			return E_NOINTERFACE;
		}

		// Increment refcount on the pointer we're about to return
		this->AddRef();
		// Return success
		return S_OK;
	}

	//! Increment ref count
	ULONG APIENTRY AddRef()
	{
		return ( ++ _ref_count );
	}

	//! Decrement ref count
	ULONG APIENTRY Release()
	{
		_ref_count--;
		if(!_ref_count)
		{
			delete this;
			return 0;
		}
		return _ref_count;
	}
public:
								IvideoCB					();
								~IvideoCB					();
	
	Asection					section;								
	BYTE						*data;
	int							datalen;
	bool						bnew;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IvideoCB::IvideoCB()
{
	_ref_count=0;
	data=NULL;
	datalen=0;
	bnew=false;
	AddRef();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IvideoCB::~IvideoCB()
{
	if(data)
		free(data);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HRESULT __stdcall IvideoCB::BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen)
{	
	section.enter(__FILE__,__LINE__);
	if(BufferLen>datalen)
	{
		if(data)
			free(data);
		data=(byte *)malloc(BufferLen);
		if(data)
			datalen=BufferLen;
		else
			datalen=0;
	}
	if(data)
		memcpy(data, pBuffer, BufferLen);
	bnew=true;
	section.leave();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class IaudioCB : public ISampleGrabberCB
{

	HRESULT	__stdcall			SampleCB					(double,struct IMediaSample *) {return 0;}
	HRESULT __stdcall			BufferCB					(double SampleTime, BYTE *pBuffer, long BufferLen);

private:
	//! COM reference count
	ULONG _ref_count;

public:
	//! Return a ptr to a different COM interface to this object
	HRESULT APIENTRY QueryInterface( REFIID iid, void** ppvObject )
	{
		// Match the interface and return the proper pointer
		if ( iid == IID_IUnknown) {
			*ppvObject = dynamic_cast<IUnknown*>( this );
		} else if ( iid == IID_ISampleGrabberCB ) {
			*ppvObject = dynamic_cast<ISampleGrabberCB*>( this );
		} else {
			// It didn't match an interface
			*ppvObject = NULL;
			return E_NOINTERFACE;
		}

		// Increment refcount on the pointer we're about to return
		this->AddRef();
		// Return success
		return S_OK;
	}

	//! Increment ref count
	ULONG APIENTRY AddRef()
	{

		return ( ++ _ref_count );
	}

	//! Decrement ref count
	ULONG APIENTRY Release()
	{
		_ref_count--;
		if(!_ref_count)
		{
			delete this;
			return 0;
		}
		return _ref_count;
	}
public:
								IaudioCB					();
								~IaudioCB					();
	
	Asection					section;								
	Abuffer						*buffer;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IaudioCB::IaudioCB()
{
	_ref_count=0;
	buffer=new Abuffer("audio", 44100);
	AddRef();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IaudioCB::~IaudioCB()
{
	if(buffer)
		delete(buffer);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HRESULT __stdcall IaudioCB::BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen)
{	
	section.enter(__FILE__,__LINE__);
	buffer->write(pBuffer, BufferLen);
	section.leave();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int						vponecount=0;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AvponeGraph
{
public:
	enum
	{
								stateSTOP,
								statePAUSE,
								statePLAY
	};
	
	enum
	{
								eventCOMPLETE,
								eventDEVICELOST,
								eventMAX
	};

	bool						events[eventMAX];
	char						filename[ALIB_PATH];
	char						name[ALIB_PATH];
	float						fps;
	
	IGraphBuilder				*pGraph;
	ICaptureGraphBuilder2		*pBuild;
	IMediaControl				*pMC;
	IMediaSeeking				*pMS;
	IMediaPosition				*pMP;
	IVideoFrameStep				*pFS;
	IMediaEvent					*pME;
	ISampleGrabber				*pGrabVideo;
	ISampleGrabber				*pGrabAudio;
	IvideoCB					*videoCB;
	IaudioCB					*audioCB;

	short						membuf[2];


	bool						bVideo;
	bool						bAudio;
	//bool						bSeek;
	
	char						*error;
	
	Asection					section;

	bool						bInit;

								AvponeGraph						();
								~AvponeGraph						();	
								
	void						setFilename						(char *file);
	
	bool						initVideo						(IBaseFilter *source);
	bool						initAudio						(IBaseFilter *source);

	bool						initMovie						();
	void						releaseMovie					();
	
	void						stretchAudio					(Asample *audio, Abuffer *fm, float rate, float fps, double dtime, int smprate);
	bool						getAudio						(Asample *audio, double dtime);
	bool						getBitmap						(Abitmap *b);
	int							getState						();
	double						getPosition						();
	double						getTotal						();
	
	bool						stop							();
	bool						play							();
	bool						pause							();

	bool						seek							(double time);
	bool						setRate							(double rate);
	double						getRate							();
	
	void						getEvents						();
	bool						isEvent							(int event);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AvponeGraph::AvponeGraph()
{
	memset(filename, 0, sizeof(filename));
	memset(name, 0, sizeof(name));
	memset(membuf, 0, sizeof(membuf));
	fps=0.f;
	pGraph=NULL;
	pBuild=NULL;
	pMC=NULL;
	pMS=NULL;
	pMP=NULL;
	pFS=NULL;
	pME=NULL;
	pGrabVideo=NULL;
	videoCB=NULL;
	pGrabAudio=NULL;
	audioCB=NULL;
	bAudio=false;
	bVideo=false;
	bInit=true;
//	bSeek=false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AvponeGraph::~AvponeGraph()
{
	releaseMovie();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AvponeGraph::setFilename(char *filename)
{
	section.enter(__FILE__,__LINE__);
	{
		char	*s=strrchr(filename, '\\');
		if(s)
			strcpy(name, s+1);
		else
			strcpy(name, filename);
		strcpy(this->filename, filename);
	}
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				
void AvponeGraph::releaseMovie()
{
	section.enter(__FILE__,__LINE__);
	if(pMC)
		pMC->Stop();
	RELEASE(pGraph);
	RELEASE(pBuild);
	RELEASE(pMC);
	RELEASE(pMS);
	RELEASE(pMP);
	RELEASE(pFS);
	RELEASE(pME);
	RELEASE(pGrabVideo);
	RELEASE(videoCB);
	RELEASE(pGrabAudio);
	RELEASE(audioCB);
	bAudio=false;
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// video

bool AvponeGraph::initVideo(IBaseFilter *pSource)
{
	IBaseFilter			*pGrabber=NULL;
	IBaseFilter			*pNull=NULL;
	ISampleGrabberCB	*cb;
	AM_MEDIA_TYPE		mt;
	HRESULT				hr;

	{
		char				nm[1024];
		strcpy(nm, filename);
		strlwr(nm);
		if(strstr(nm, ".mp3")||strstr(nm, ".wav")||strstr(nm, ".wma"))
			return false;
	}
	
	hr=CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, reinterpret_cast<void**>(&pGrabber));
	if(hr!=S_OK)
	{
		error="can't create video grabber";
		return false;
	}

	hr=pGrabber->QueryInterface(IID_ISampleGrabber, reinterpret_cast<void**>(&pGrabVideo));
	if(hr!=S_OK)
	{
		error="can't query video grabber";
		RELEASE(pGrabber);
		return false;
	}
	
	videoCB=new IvideoCB();
	hr=videoCB->QueryInterface(IID_ISampleGrabberCB, reinterpret_cast<void**>(&cb));
	if(hr!=S_OK)
	{
		error="can't query video grabber callback";
		RELEASE(pGrabber);
		return false;
	}
	
	hr=pGrabVideo->SetCallback(cb, 1);
	cb->Release();
	if(hr!=S_OK)
	{
		error="can't set video grabber callback";
		RELEASE(pGrabber);
		return false;
	}
	
	hr = pGraph->AddFilter(pGrabber, L"Video Grabber");
	if(hr!=S_OK)
	{
		error="can't add video grabber";
		RELEASE(pGrabber);
		return false;
	}

	hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, reinterpret_cast<void**>(&pNull));
	if(hr!=S_OK)
	{
		error="can't create video Null";
		RELEASE(pGrabber);
		return false;
	}

	hr = pGraph->AddFilter(pNull, L"NullRenderer Video");
	if(hr!=S_OK)
	{
		error="can't add video Null";
		RELEASE(pNull);
		RELEASE(pGrabber);
		return false;
	}

	{
		memset(&mt, 0, sizeof(mt));
		mt.majortype = MEDIATYPE_Video;
		mt.subtype = MEDIASUBTYPE_ARGB32; //MEDIASUBTYPE_RGB32;
		hr=pGrabVideo->SetMediaType(&mt);
	}

	hr=pBuild->RenderStream(0, 0, pSource, 0, pGrabber);
	if(hr!=S_OK)
	{
		error="can't connect [missing codec]";
		RELEASE(pNull);
		RELEASE(pGrabber);
		return false;
	}
			
	hr=pBuild->RenderStream(0, &MEDIATYPE_Video, pGrabber, 0, pNull);
	RELEASE(pNull);
	RELEASE(pGrabber);
	if(hr!=S_OK)
	{
		error="can't built the decoder graph [missing codec]";
		return false;
	}
	
	hr=pGrabVideo->SetBufferSamples(true);
	if(hr!=S_OK)
	{
		error="can't bufferize video stream";
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// audio

bool AvponeGraph::initAudio(IBaseFilter *pSource)
{
	IBaseFilter			*pGrabber=NULL;
	IBaseFilter			*pNull=NULL;
	ISampleGrabberCB	*cb;
	AM_MEDIA_TYPE		mt;
	HRESULT				hr;

	hr=CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, reinterpret_cast<void**>(&pGrabber));
	if(hr!=S_OK)
	{
		error="can't create audio grabber";
		return false;
	}

	hr=pGrabber->QueryInterface(IID_ISampleGrabber, reinterpret_cast<void**>(&pGrabAudio));
	if(hr!=S_OK)
	{
		error="can't query audio grabber";
		RELEASE(pGrabber);
		return false;
	}
	
	audioCB=new IaudioCB();
	hr=audioCB->QueryInterface(IID_ISampleGrabberCB, reinterpret_cast<void**>(&cb));
	if(hr!=S_OK)
	{
		error="can't query audio grabber callback";
		RELEASE(pGrabber);
		return false;
	}
	
	hr=pGrabAudio->SetCallback(cb, 1);
	cb->Release();
	if(hr!=S_OK)
	{
		error="can't set audio grabber callback";
		RELEASE(pGrabber);
		return false;
	}
	
	hr = pGraph->AddFilter(pGrabber, L"Audio Grabber");
	if(hr!=S_OK)
	{
		error="can't add audio grabber";
		RELEASE(pGrabber);
		return false;
	}

	hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, reinterpret_cast<void**>(&pNull));
	if(hr!=S_OK)
	{
		error="can't create audio Null";
		RELEASE(pGrabber);
		return false;
	}

	hr = pGraph->AddFilter(pNull, L"NullRenderer Audio");
	if(hr!=S_OK)
	{
		error="can't add video Null";
		RELEASE(pNull);
		RELEASE(pGrabber);
		return false;
	}

	{
		memset(&mt, 0, sizeof(mt));
		mt.majortype = MEDIATYPE_Audio;
		mt.subtype = MEDIASUBTYPE_PCM;
		hr=pGrabAudio->SetMediaType(&mt);
	}

	hr=pBuild->RenderStream(0, 0, pSource, 0, pGrabber);
	if(hr!=S_OK)
	{
		error="can't connect [missing codec]";
		RELEASE(pNull);
		RELEASE(pGrabber);
		return false;
	}
			
	hr=pBuild->RenderStream(0, &MEDIATYPE_Audio, pGrabber, 0, pNull);
	RELEASE(pNull);
	RELEASE(pGrabber);
	if(hr!=S_OK)
	{
		error="can't built the decoder graph [missing codec]";
		return false;
	}
	
	hr=pGrabAudio->SetBufferSamples(true);
	if(hr!=S_OK)
	{
		error="can't bufferize audio stream";
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// all

bool AvponeGraph::initMovie()
{
	section.enter(__FILE__,__LINE__);
	bInit=true;
	{
		WCHAR	wname[ALIB_PATH];
		b2wchar(wname, filename);
		
		releaseMovie();
		
		if(InitCaptureGraphBuilder(&pGraph, &pBuild)!=S_OK)
		{
			error="can't create DS graphBuilder [out of memory]";
			releaseMovie();
			section.leave();
			return false;
		}
			
		{
			HRESULT				hr=NULL;
			IBaseFilter			*pSource=NULL;
			//IPin				*pIn=NULL;
			//IPin				*pOut=NULL;
			
			hr=pGraph->AddSourceFilter(wname, L"Source1", &pSource);	
			if(hr!=S_OK)
			{
				error="can't open movie file [file or format error]";
				releaseMovie();
				section.leave();
				return false;
			}
			
			bVideo=initVideo(pSource);
			/*
			if(!initVideo(pSource))
			{
				RELEASE(pSource);
				releaseMovie();
				section.leave();
				return false;
			}
			*/

			bAudio=initAudio(pSource);
			RELEASE(pSource);

			// controls

			pGraph->QueryInterface(IID_IVideoFrameStep, (void**)&pFS);
			pGraph->QueryInterface(IID_IMediaPosition, (void**)&pMP);
			pGraph->QueryInterface(IID_IMediaSeeking, (void**)&pMS);
			pGraph->QueryInterface(IID_IMediaEvent, (void**)&pME);
			
			pMS->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);

			hr=pGraph->QueryInterface(IID_IMediaControl, (void**)&pMC);
			if(hr!=S_OK)
			{
				error="can't create media control [out of memory]";
				releaseMovie();
				section.leave();
				return false;
			}
		}
	}
	section.leave();
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AvponeGraph::getBitmap(Abitmap *image)
{
	bool	ok=false;
	if(section.enter(__FILE__,__LINE__,!bInit))
	{
		if(pGrabVideo)
		{
			AM_MEDIA_TYPE	MediaType;
			HRESULT	hr = pGrabVideo->GetConnectedMediaType(&MediaType);
			if(hr==S_OK)
			{
				if(MediaType.pbFormat)
				{
					VIDEOINFOHEADER *pvh=(VIDEOINFOHEADER*)MediaType.pbFormat; 
					int				w=pvh->bmiHeader.biWidth;
					int				h=pvh->bmiHeader.biHeight;
					long			size=0; 

					if((w!=image->w)||(h!=image->h))
						image->size(w, h);
						
					size=image->bodysize;
					
					videoCB->section.enter(__FILE__,__LINE__);
					if(size>=videoCB->datalen)
					{
						if(videoCB->bnew)
						{
							int wsize=w*sizeof(dword);
							int y;
							dword *dst=image->body32;
							for(y=0; y<h; y++)
							{
								dword *src=((dword *)videoCB->data)+(h-1-y)*w;
								memcpy(dst, src, wsize);
								dst+=w;
							}
							videoCB->bnew=false;
							ok=true;
							bInit=false;
	//						bSeek=false;
						}
					}
					videoCB->section.leave();
				}
				MyFreeMediaType(MediaType);
			}
		}
		section.leave();
	}
	return ok;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AvponeGraph::stretchAudio(Asample *smp, Abuffer *fm, float rate, float fps, double dtime, int smprate)
{
	if(fm->getSize())
	{
		sword	buf[44100][2];
		sword	outbuf[44100][2];
		int		nout=(int)(dtime*44100/1000);
		int		nsmp=(int)(fm->getSize()/sizeof(buf[0]));

		nout=mini(nout, sizeof(outbuf)/sizeof(outbuf[0]));		// no buffer overflow
		nsmp=mini(mini((int)(rate*(float)smprate*dtime/1000.0), nsmp), 44100);
		fm->read(buf, nsmp*sizeof(buf[0]));

		smp->enter(__FILE__,__LINE__);
		if(nout==nsmp)
		{
			smp->addBuffer((sword *)buf, nsmp);
		}
		else if(nout>nsmp)	// slow play
		{
			double	s=0.f;
			double	ds=(float)(nsmp)/(float)nout;
			int		i;
			int		di=0;
			for(i=0; i<nout; i++)
			{
				int		si=(int)s;
				float	a1=(float)(s-(float)si);
				float	a0=1.f-a1;
				float	v0,v1;
				int		di=si;
				si--;
				if(si<0)
				{
					v0=(float)membuf[0]*a0;
					v1=(float)membuf[1]*a0;
				}
				else
				{
					v0=(float)buf[si][0]*a0;
					v1=(float)buf[si][1]*a0;
				}
				v0+=(float)buf[di][0]*a1;
				v1+=(float)buf[di][1]*a1;
				outbuf[i][0]=(sword)v0;
				outbuf[i][1]=(sword)v1;
				s+=ds;
			}
			smp->addBuffer((sword *)outbuf, nout);
		}
		else	// nout<nsmp // speed play
		{
			double	s=0.f;
			double	ds=(float)nsmp/(float)nout;
			int		i;
			for(i=0; i<nout; i++)
			{
				double	sn=s+ds;
				int		n0=(int)s;
				int		n1=(int)sn;
				float	a0=(float)(1.0-(s-n0));
				float	v0=buf[n0][0]*a0;
				float	v1=buf[n0][1]*a0;
				int		k;
				for(k=n0+1; k<n1; k++)
				{
					v0+=buf[k][0];
					v1+=buf[k][1];
				}
				if(n1<nsmp)
				{
					float	a1=(float)(sn-n1);
					v0+=buf[n1][0]*a1;
					v1+=buf[n1][1]*a1;
				}
				outbuf[i][0]=(sword)(v0/ds);
				outbuf[i][1]=(sword)(v1/ds);
				s=sn;
			}
			smp->addBuffer((sword *)outbuf, nout);
		}
		membuf[0]=((sword *)buf)[(nsmp-1)<<1];
		membuf[1]=((sword *)buf)[((nsmp-1)<<1)+1];
		smp->leave();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AvponeGraph::getAudio(Asample *audio, double dtime)
{
	bool	ok=false;
	if(section.enter(__FILE__,__LINE__,!bInit))
	{
		if(bAudio&&audio->getCountConnect())
		{
			AM_MEDIA_TYPE	MediaType;
			HRESULT	hr = pGrabAudio->GetConnectedMediaType(&MediaType);
			if(hr==S_OK)
			{
				if(MediaType.pbFormat)
				{
					WAVEFORMATEX	*wfe=(WAVEFORMATEX*)MediaType.pbFormat;

					//if(!bSeek)
					if(1)
					{
						audioCB->section.enter(__FILE__,__LINE__);
						if(wfe->wBitsPerSample==16&&wfe->nChannels==2)
						{
							int		size=(int)audioCB->buffer->getSize();
							if(size)
							{
								stretchAudio(audio, audioCB->buffer, (float)getRate(), fps, dtime, wfe->nSamplesPerSec);
								//audio->addBuffer(data, size/4);
								ok=true;
								bInit=false;
							}
						}
						audioCB->section.leave();
					}
					else
					{
						audioCB->section.enter(__FILE__,__LINE__);
						audioCB->buffer->clear();
						audioCB->section.leave();
					}

				}
				MyFreeMediaType(MediaType);
			}
		}
		else if(audioCB)
			audioCB->buffer->clear();
		section.leave();
	}
	return ok;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int AvponeGraph::getState()
{
	OAFilterState	pfs;
	section.enter(__FILE__,__LINE__);
	if(pMC&&(pMC->GetState(INFINITE, &pfs)==S_OK))
	{
		section.leave();
		return (int)pfs;
	}
	section.leave();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double AvponeGraph::getPosition()
{
	double	p=0.0;
	section.enter(__FILE__,__LINE__);
	if(pMS)
	{
		LONGLONG	curpos;
		HRESULT		hr=pMS->GetCurrentPosition(&curpos);
		p=(double)curpos/10000000.0;
	}
	section.leave();
	return p;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double AvponeGraph::getTotal()
{
	double	p=0.0;
	section.enter(__FILE__,__LINE__);
	if(pMS)
	{
		LONGLONG	duration;
		HRESULT		hr=pMS->GetDuration(&duration);
		p=(double)duration/10000000.0;
	}
	section.leave();
	return p;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AvponeGraph::seek(double time)
{
	LONGLONG	pos=(LONGLONG)(time*10000000.0);
	HRESULT		hr=S_OK;
	bool		b=false;
	section.enter(__FILE__,__LINE__);
	if(pMS)
	{
		pMS->SetPositions(&pos, AM_SEEKING_AbsolutePositioning , NULL, AM_SEEKING_NoPositioning);
		{
//			bSeek=true;
			if(videoCB)
			{
				videoCB->section.enter(__FILE__,__LINE__);
				videoCB->bnew=false;
				videoCB->section.leave();
			}
		}
		b=true;
	}
	section.leave();
	return b;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AvponeGraph::play()
{
	section.enter(__FILE__,__LINE__);
	if(pMC&&(pMC->Run()==S_OK))
	{
		section.leave();
		return true;
	}	
	section.leave();
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AvponeGraph::pause()
{
	section.enter(__FILE__,__LINE__);
	if(pMC&&(pMC->Pause()==S_OK))
	{
		section.leave();
		return true;
	}	
	section.leave();
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AvponeGraph::stop()
{
	section.enter(__FILE__,__LINE__);
	if(pMC&&(pMC->Stop()==S_OK))
	{
		section.leave();
		return true;
	}	
	section.leave();
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AvponeGraph::setRate(double rate)
{
	bool	b=false;
	if(section.enter(__FILE__,__LINE__,false))
	{
		if(pMS&&(pMS->SetRate(rate)==S_OK))
			b=true;
		section.leave();
	}	
	return b;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double AvponeGraph::getRate()
{
	double	rate=0;
	if(section.enter(__FILE__,__LINE__,false))
	{
		if(pMS)
			pMS->GetRate(&rate);
		section.leave();
	}
	return rate;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AvponeGraph::getEvents()
{
	section.enter(__FILE__,__LINE__);
	memset(events, 0, sizeof(events));
	if(pME)
	{
		long	code,param1,param2;
		HRESULT	hr;
		while(pME->GetEvent(&code, &param1, &param2, 0)==S_OK)
		{
			// Free memory associated with callback, since we're not using it
			hr = pME->FreeEventParams(code, param1, param2);
			
			switch(code)
			{
				case EC_COMPLETE:
				events[eventCOMPLETE]=true;
				break;
				
				case EC_DEVICE_LOST:
				events[eventDEVICELOST]=true;
				break;
			}
        }
    }
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AvponeGraph::isEvent(int event)
{
	bool	bev;
	section.enter(__FILE__,__LINE__);
	bev=events[event];
	section.leave();
	return bev;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AvponeBut : public Aobject
{
	AOBJ
public:
	
								AvponeBut						(char *name, class AvponeSel *sel, int x, int y);
								~AvponeBut						();
								
	virtual bool				dragdrop						(int x, int y, int state, int event, void *data=NULL);
	void						paint							(Abitmap *b);
	virtual bool				mouse							(int x, int y, int state, int event);
	
	void						setSnapshot						(Abitmap *b);

	Abitmap						*bitmap;
	Abitmap						*snap;
	class AvponeSel				*sel;
	bool						selected;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								AvponeBut::CI	= ACI("AvponeBut",	GUID(0x11111112,0x00000345), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AvponeSel : public AcontrolObj
{
	AOBJ
public:
	
								AvponeSel						(QIID qiid, char *name, Aobject *f, int x, int y);
								~AvponeSel						();
								
	virtual bool				notify							(Anode *o, int event, dword p);
	virtual void				pulse							();
	virtual void				paint							(Abitmap *b);
	
	virtual bool				sequence						(int nctrl, float value);
	void						set								(int sel);

	bool						load							(Afile *f);
	bool						save							(Afile *f);

	bool						savePreset						(Afile *f);
	bool						loadPreset						(Afile *f);

	void						setMovie						(int n, char *filename);
	
	Asection					section;
	AvponeBut					*but[9];
	bool						todo[9];
	char						file[9][ALIB_PATH];
	float						fps[9];
	char						codec[9][5];
	bool						ok[9];
	Abitmap						*mire;
	Abitmap						*nosnap;
	Abitmap						*loaded;
	int							nbut;
	int							nscan;
	int							nbtodo;
	bool						ctrlPaint;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								AvponeSel::CI	= ACI("AvponeSel",	GUID(0x11111112,0x00000346), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AvponeBut::AvponeBut(char *name, AvponeSel *sel, int x, int y) : Aobject(name, sel, x, y, 44, 44)
{
	this->sel=sel;
	selected=false;
	bitmap=new Abitmap(&resource.get(MAKEINTRESOURCE(PNG_VPONE_MOVIE), "PNG"));
	snap=new Abitmap(35, 27);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AvponeBut::~AvponeBut()
{
	delete(bitmap);
	delete(snap);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AvponeBut::paint(Abitmap *b)
{
	b->set(4, 5, snap, bitmapDEFAULT, bitmapDEFAULT);
	if(selected)
	{
		b->boxfa(4, 5, 3+snap->w, 4+snap->h, 0xffffffff, 0.1f);
		b->set(0, 0, 44, 0, 44, 44, bitmap, bitmapDEFAULT, bitmapDEFAULT);
	}
	else
	{
		b->set(0, 0, 0, 0, 44, 44, bitmap, bitmapDEFAULT, bitmapDEFAULT);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AvponeBut::mouse(int x, int y, int state, int event)
{
	switch(event)
	{
		case mouseLUP:
		case mouseNORMAL:
		cursor(cursorHANDSEL);
		return true;
		
		case mouseLDOWN:
		cursor(cursorHANDSEL);
		sel->notify(this, nySELECT, 0);
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AvponeBut::dragdrop(int x, int y, int state, int event, void *data)
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
			father->notify(this, nyUSER1, (dword)data);
		break;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AvponeBut::setSnapshot(Abitmap	*b)
{
	if(b)
		snap->set(0, 0, snap->w, snap->h, 0, 0, b->w, b->h, b, bitmapNORMAL, bitmapNORMAL);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AvponeSel::AvponeSel(QIID qiid, char *name, Aobject *f, int x, int y) : AcontrolObj(name, f, x, y, 138, 138)
{
	int	i;
	control=new Acontrol(qiid, name, Aeffect::getEffect(this), this, Acontrol::KEY_09);
	mire=new Abitmap(&resource.get(MAKEINTRESOURCE(PNG_VPONE_MIRE), "PNG"));
	nosnap=new Abitmap(&resource.get(MAKEINTRESOURCE(PNG_VPONE_NOSNAP), "PNG"));
	loaded=new Abitmap(&resource.get(MAKEINTRESOURCE(PNG_VP1_MIREOK), "PNG"));
	ctrlPaint=false;
	for(i=0; i<9; i++)
	{
		int	x=(i%3)*46;
		int	y=(i/3)*46;
		but[i]=new AvponeBut("movie", this, x, y);
		but[i]->setTooltips("movie");
		but[i]->setSnapshot(mire);
		if(!i)
			but[i]->selected=true;
		but[i]->show(TRUE);
	}
	nbut=0;
	nscan=0;
	nbtodo=0;
	memset(file, 0, sizeof(file));
	memset(todo, 0, sizeof(todo));
	memset(ok, 0, sizeof(ok));
	control->set(Acontrol::KEY, (float)nbut);
	timer(100);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AvponeSel::~AvponeSel()
{
	int	i;
	for(i=0; i<9; i++)
		delete(but[i]);
	delete(mire);
	delete(nosnap);
	delete(loaded);
	delete(control);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AvponeSel::set(int sel)
{
	but[nbut]->selected=false;
	but[nbut=sel]->selected=true;
	repaint();
	control->set(Acontrol::KEY, (float)nbut);
	father->notify(this, nySELECT, nbut);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AvponeSel::sequence(int nctrl, float value)
{
	int	n=(int)value;
	but[nbut]->selected=false;
	but[nbut=n]->selected=true;
	ctrlPaint=true;
	father->asyncNotify(this, nySELECT, nbut);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AvponeSel::setMovie(int i, char *filename)
{
	section.enter(__FILE__,__LINE__);
	strcpy(this->file[i], filename);
	if(todo[i]==false)
	{
		but[i]->setSnapshot(nosnap);
		todo[i]=true;
		nbtodo++;
	}
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AvponeSel::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCHANGE:
		break;

		case nySELECT:
		control->select();
		{
			int	i;
			for(i=0; i<9; i++)
				if(but[i]==o)
					break;
			if((i<9)&&(i!=nbut))
			{
				but[nbut]->selected=false;
				but[nbut=i]->selected=true;
				repaint();
				control->set(Acontrol::KEY, (float)nbut);
				father->notify(this, nySELECT, nbut);
			}
		}
		return true;
		
		case nyUSER1:
		{
			int	i;
			for(i=0; i<9; i++)
				if(but[i]==o)
					break;
			if(i<9)
			{
				char	*file=(char *)p;
				int		n=9;
				while(*file&&(n--))
				{
					setMovie(i, file);
					file+=strlen(file)+1;
					i=(i+1)%9;
				}
			}
			repaint();
		}
		break;
	}
	return Aobject::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AvponeSel::paint(Abitmap *b)
{
	Aobject::paint(b);
	controlPaint(b, pos.w, pos.h);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static qword fileSize(char *file)
{
	LARGE_INTEGER size;
	HANDLE h=CreateFile(file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(h)
	{
		GetFileSizeEx(h, &size);
		CloseHandle(h);
		return size.QuadPart;
	}
	return 0;
}

static qword calcMDSNAP(char *s)
{
	qword	q=fileSize(s);
	int		n=0;
	int		pos=0;
	while(*s)
	{
		q^=(qword)(((byte)*s+(byte)pos)&255)<<n;
		n=(n+((unsigned char)*s))%56;
		s++;
		pos++;
	}
	return q;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Abitmap * getElekSnapshot(char *file)
{
	static char	root[ALIB_PATH]="";

	if(!root[0])
	{
		GetModuleFileName(GetModuleHandle(null), root, sizeof(root));
		if(root[0])
		{
			char	*s=strrchr(root, '\\');
			if(s)
				*s=0;
		}
		strcat(root, "\\snaps\\");
	}

	{
		Abitmap *b=new Abitmap(4, 4);
		char	fnsnap[ALIB_PATH];

		strcpy(fnsnap, root);

		{
			char	nq[1024];
			qword	q=calcMDSNAP(file);
			sprintf(nq, "%I64x", q);
			strcat(fnsnap, nq);
			strcat(fnsnap, ".jpg");
		}

		if(!b->load(fnsnap))
		{
			delete(b);
			b=null;
		}

		if(!b)
		{
			float	fps;
			b=Amovie::getSnapshot(file, &fps);
			if(b)
				b->save(fnsnap, bitmapJPG);
		}

		return b;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AvponeSel::pulse()
{
	int		n=-1;
	char	file[ALIB_PATH];
	if(ctrlPaint)
	{
		repaint();
		ctrlPaint=false;
	}
	section.enter(__FILE__,__LINE__);
	if(nbtodo)
	{
		int	i;
		for(i=0; i<9; i++)
		{
			if(todo[i])
			{
				strcpy(file, this->file[i]);
				n=i;
				todo[i]=false;
				break;
			}
		}
		nbtodo--;
	}
	section.leave();
	if(n>=0)
	{
		float			fps=25;
		AmovieFileInfo	info;
		Abitmap			*b=getElekSnapshot(file);
		
		if(Amovie::getFileInfo(&info, file))
		{
			strcpy(this->codec[n], info.codec);
			fps=info.fps;
		}
		else
			this->codec[n][0]=0;
		
		this->fps[n]=fps;

		if(b)
		{
			but[n]->setSnapshot(b);
			delete(b);
			ok[n]=true;
		}
		else
		{
			but[n]->setSnapshot(loaded);
			ok[n]=true;
		}
		but[n]->repaint();
		if(nbut==n)
			father->notify(this, nySELECT, n);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AvponeSel::load(Afile *f)
{
	char	abs[ALIB_PATH];
	int		i;
	for(i=0; i<9; i++)
	{
		f->readString(file[i]);
		if(file[i][0])
		{
			if(f->absoluPath(abs, file[i]))
				strcpy(file[i], abs);
			todo[i]=true;
			nbtodo++;
		}
	}
	//father->notify(this, nySELECT, nbut);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AvponeSel::save(Afile *f)
{
	char	relpath[ALIB_PATH];
	int		i;

	for(i=0; i<9; i++)
	{
		if(f->relativePath(relpath, file[i]))
			f->writeString(relpath);
		else
			f->writeString(file[i]);
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AvponeSel::savePreset(Afile *f)
{
	Atable	*table=Atable::getTable(this);
	int		i;
	for(i=0; i<9; i++)
	{
		int	n=-1;
		if(file[i][0])
			n=table->addPresetFile(file[i]);
		f->write(&n, sizeof(n));
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AvponeSel::loadPreset(Afile *f)
{
	Atable	*table=Atable::getTable(this);
	int		i;
	for(i=0; i<9; i++)
	{ 
		int		n=-1;
		f->read(&n, sizeof(n));
		if(n==-1)
		{
			if(file[i][0])
			{
				file[i][0]=0;
				todo[i]=true;
				nbtodo++;
			}
		}
		else
		{
			char	fn[ALIB_PATH];
			table->getPresetFile(n, fn);
			if(strcmp(fn, file[i]))
			{
				strcpy(file[i], fn);
				todo[i]=true;
				nbtodo++;
			}
		}
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AvponeDisplay : public Aobject
{
	AOBJ
public:

	char						fname[ALIB_PATH];
	char						fps[128];
	char						codec[5];

								AvponeDisplay					(char *name, Aobject *f, int x, int y, AvponeGraph *dt);
								~AvponeDisplay					();
								
	void						paint							(Abitmap *b);
	virtual void				pulse							();
	
	AvponeGraph					*dt;
	Abitmap						*state;
	int							flip;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								AvponeDisplay::CI	= ACI("AvponeDisplay",	GUID(0x11111112,0x00000347), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AvponeDisplay::AvponeDisplay(char *name, Aobject *f, int x, int y, AvponeGraph *dt) : Aobject(name, f, x, y, 110, 50)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_VPONE_STATES), "PNG");
	memset(fname, 0, sizeof(fname));
	memset(fps, 0, sizeof(fps));
	memset(codec, 0, sizeof(codec));
	state=new Abitmap(&o);
	this->dt=dt;
	timer(100);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AvponeDisplay::~AvponeDisplay()
{
	delete(state);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AvponeDisplay::paint(Abitmap *b)
{
	Afont	*f=alib.getFont(fontTERMINAL06);
	int		ns=dt->getState();
	double	time=dt->getPosition();
	double	total=dt->getTotal();
	char	tstr[128];
	
	f->set(b, 0, 4, "FILE", 0xff4E4D90);
	f->set(b, 30, 4, fname, 0xff7590BA);
	f->set(b, 0, 14, "PLAY", 0xff4E4D90);
	f->set(b, 0, 24, "TOTAL", 0xff4E4D90);
	f->set(b, 0, 34, "FPS", 0xff4E4D90);
	f->set(b, 24, 34, fps, 0xff7590BA);
	f->set(b, 62, 34, codec, 0xff900000);
	if(fname[0])
	{
		sprintfTime(tstr, time);
		f->setFixed(b, 40, 14, tstr, 0xff7590BA);
		sprintfTime(tstr, total);
		f->setFixed(b, 40, 24, tstr, 0xff7590BA);
		if(ns!=AvponeGraph::stateSTOP)
		{
			if((flip++)&2)
				b->set(100, 34, 8*ns, 0, 8, 8, state, bitmapDEFAULT, bitmapDEFAULT);
		}
		else
			b->set(100, 34, 0, 0, 8, 8, state, bitmapDEFAULT, bitmapDEFAULT);
	}
	
	//f->set(b, 44, 34, "CODEC", 0xff4E4D90);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AvponeDisplay::pulse()
{
	repaint();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


ACI								AvponeSlide::CI	= ACI("AvponeSlide",	GUID(0x11111112,0x00000348), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AvponeSlide::AvponeSlide(char *name, Aobject *f, int x, int y, int w, int h) : Aobject(name, f, x, y, w, h)
{
	value=0;
	colorBACK=0xff010028;
	colorFORE=0xff6767CC;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AvponeSlide::~AvponeSlide()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AvponeSlide::paint(Abitmap *b)
{
	int	p=(int)(((float)pos.w-4.f-10.f)*value)+2;
	//b->box(0, 0, pos.w-1, pos.h-1, 0xff6767CC);
	//b->box(1, 1, pos.w-2, pos.h-2, 0xff6767CC);
	b->boxfa(0, 0, pos.w-1, pos.h-1, colorBACK);
	b->boxfa(p, 2, p+9, pos.h-3, colorFORE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AvponeSlide::mouse(int x, int y, int state, int event)
{
	switch(event)
	{
		case mouseLDOWN:
		mouseCapture(true);
		case mouseNORMAL:
		cursor(cursorHANDSEL);
		if(state&mouseL)
		{
			int	p=pos.w-4-10;
			value=(float)maxi(mini(x-2, p), 0)/(float)p;
			repaint();
			father->notify(this, nyCHANGE);
		}
		return true;

		case mouseLUP:
		cursor(cursorHANDSEL);
		mouseCapture(false);
		break;
		
		case mouseLEAVE:
		mouseCapture(false);
		break;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AvponeSlide::setValue(float v)
{
	value=v;
	repaint();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float AvponeSlide::getValue()
{
	return value;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Avpone::Avpone(QIID qiid, char *name, AeffectInfo *info, Acapsule *capsule) : Aeffect(qiid, name, info, capsule)
{
	vponecount++;
	rate=1.0;
	initrulez=false;

	image=new Abitmap(getVideoWidth(), getVideoHeight());
	dt=new AvponeGraph();
	
	front=new AvponeFront(qiid, "VPone front", this, 172);
	front->setTooltips("VP1 player");
	back=new AvponeBack(qiid, "VPone back", this, 172);
	back->setTooltips("VP1 player");

	{
		int i;
		for(i=0; i<9; i++)
		{
			char	name[128];
			sprintf(name, "movie filename #%d", i+1);
			oscMovie[i]=new AoscNode(name, this, ",s", name); 
		}
	}
	
	settings(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Avpone::~Avpone()
{
	delete(dt);
	delete(image);
	vponecount--;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avpone::load(class Afile *f)
{
	AvponeFront	*front=(AvponeFront *)this->front;
	return front->sel->load(f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avpone::save(class Afile *f)
{
	AvponeFront	*front=(AvponeFront *)this->front;
	return front->sel->save(f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avpone::savePreset(Afile *f)
{
	AvponeFront	*front=(AvponeFront *)this->front;
	return front->sel->savePreset(f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avpone::loadPreset(Afile *f)
{
	AvponeFront	*front=(AvponeFront *)this->front;
	return front->sel->loadPreset(f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avpone::settings(bool emergency)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avpone::oscMessage(class AoscMessage *reply, class AoscMessage *msg, class AoscNode *onode, int action)
{
	switch(action)
	{
		case oscGETVALUE:
		{
			int i;
			for(i=0; i<9; i++)
			{
				if(onode==oscMovie[i])
				{
					char *text=((AvponeFront *)front)->sel->file[i];
					if(text)
						reply->add(new AoscString(text));
					else
						reply->add(new AoscString(""));
					return true;	// reply used
				}
			}
		}
		break;

		case oscSETVALUE:
		if(msg->fchild&&msg->fchild->isCI(&AoscString::CI))
		{
			int i;
			for(i=0; i<9; i++)
			{
				if(onode==oscMovie[i])
				{
					AvponeFront *front=(AvponeFront *)this->front;
					char *s=((AoscString *)msg->fchild)->value;
					if(s)
					{
						front->sel->setMovie(i, s);
						return false;	// reply not used
					}
				}
			}
		}
		break;
	}
	return Aeffect::oscMessage(reply, msg, onode, action);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avpone::actionStart(double time)
{
	AvponeFront	*front=(AvponeFront *)this->front;
	//if((time==0.0)&&front->aplay->get())
	//	dt->seek(0);
}

void Avpone::action(double time, double dtime, double beat, double dbeat)
{
	if(!initrulez)
	{
		AvponeBack	*back=(AvponeBack *)this->back;
		AvponeFront	*front=(AvponeFront *)this->front;
		Abitmap		*b=back->out->getBitmap();
		double		mrate=dt->getRate();

	//if((time==0.0)&&front->aplay->get())
	//	dt->seek(0);
	
	//dt->getEvents();
	//if(dt->isEvent(AvponeGraph::eventCOMPLETE))
	//{
	//	bool	bloop=front->loop->isChecked();
	//	if(bloop)
	//		dt->seek(0);
	//	else
	//		dt->stop();
	//}

	//{
	//	double	cur=dt->getPosition();
	//	double	len=dt->getTotal();
	//	if(fabs(cur-len)<0.0001)
	//	{
	//		if(front->loop->get())
	//			dt->seek(0);
	//		else
	//			dt->stop();
	//	}
	//}

		if(rate!=mrate)
			dt->setRate(rate);
		
		if(b)
		{
			if(dt->getBitmap(image))
				b->set(0, 0, b->w, b->h, 0, 0, image->w, image->h, image, bitmapNORMAL, bitmapNORMAL);
			dt->getAudio(back->audio, dtime);
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

AvponeFront::AvponeFront(QIID qiid, char *name, Avpone *e, int h) : AeffectFront(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_VPONE_FRONT), "PNG");
	back=new Abitmap(&o);

	rate=new Apaddle(MKQIID(qiid, 0x0db2cad21a030180), "movie play rate", this, 302, 13, 150, 150, &resource.get(MAKEINTRESOURCE(PNG_VPONE_RATE), "PNG"), 150, 150, paddleY);
	rate->setTooltips("movie play rate");
	rate->set(0.5f);
	rate->show(TRUE);

	stop=new ActrlButton(MKQIID(qiid, 0x36843ac3272ce5a0), "stop", this, 200, 28, 25, 29, &resource.get(MAKEINTRESOURCE(PNG_VPONE_STOP), "PNG"));
	stop->setTooltips("stop movie");
	stop->show(TRUE);

	play=new ActrlButton(MKQIID(qiid, 0x1787d775da715e02), "play", this, 230, 28, 48, 29, &resource.get(MAKEINTRESOURCE(PNG_VPONE_PLAY), "PNG"));
	play->setTooltips("play movie");
	play->show(TRUE);
	
	aplay=new ActrlButton(MKQIID(qiid, 0x06269d565f768940), "aplay", this, 306, 13, 14, 27, &resource.get(MAKEINTRESOURCE(PNG_VPONE_BUTTON), "PNG"), Abutton::btBITMAP|Abutton::bt2STATES);
	aplay->setTooltips("auto play");
	aplay->show(TRUE);
	
	loop=new ActrlButton(MKQIID(qiid, 0x56d1f9d648d25440), "loop", this, 306, 134, 14, 27, &resource.get(MAKEINTRESOURCE(PNG_VPONE_BUTTON), "PNG"), Abutton::btBITMAP|Abutton::bt2STATES);
	loop->setTooltips("loop mode enable");
	loop->show(TRUE);

	playAll=new ActrlButton(MKQIID(qiid, 0x06284e56a54c6d40), "play bank", this, 174, 28, 14, 27, &resource.get(MAKEINTRESOURCE(PNG_VPONE_BUTTON), "PNG"), Abutton::btBITMAP|Abutton::bt2STATES);
	playAll->setTooltips("play all bank");
	playAll->show(TRUE);
	
	sel=new AvponeSel(MKQIID(qiid, 0xda4a1ca0df3feb00), "select movie", this, 13, 24);
	sel->setTooltips("select movie");
	sel->show(true);
	
	disp=new AvponeDisplay("infos", this, 174, 112, e->dt);
	disp->setTooltips("file info");
	disp->show(true);
	
	slide=new AvponeSlide("time line", this, 174, 64, 110, 14);
	slide->setTooltips("time line");
	slide->show(true);
	
	timer(40);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AvponeFront::~AvponeFront()
{
	delete(back);
	delete(rate);
	delete(stop);
	delete(play);
	delete(aplay);
	delete(loop);
	delete(playAll);
	delete(sel);
	delete(disp);
	delete(slide);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AvponeFront::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AvponeFront::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nySELECT:
		if(o==sel)
		{
			Avpone	*vpone=(Avpone *)effect;
			nmovie=p;
			vpone->initrulez=true;
			if(sel->ok[nmovie])
			{
				vpone->dt->setFilename(sel->file[nmovie]);
				vpone->dt->fps=sel->fps[nmovie];
				strcpy(disp->codec, sel->codec[nmovie]);
				strcpy(disp->fname, vpone->dt->name);
				strupr(disp->fname);
				sprintf(disp->fps, "%3.1f", vpone->dt->fps);
				if(!vpone->dt->initMovie())
					notify(this, nyERROR, (dword)vpone->dt->error);
				else
				{
					if(aplay->get())
						vpone->dt->play();
					else
						vpone->dt->pause();
				}
			}
			else
			{
				vpone->dt->setFilename("");
				vpone->dt->fps=0.f;
				disp->fname[0]=0;
				disp->fps[0]=0;
				vpone->dt->releaseMovie();
			}
			vpone->initrulez=false;
			// todo
		}
		break;
		
		case nyPRESS:
		if(o==play)
		{
			Avpone	*vpone=(Avpone *)effect;
			int		s=vpone->dt->getState();
			switch(s)
			{
				case AvponeGraph::stateSTOP:
				case AvponeGraph::statePAUSE:
				vpone->dt->play();
				break;
				
				case AvponeGraph::statePLAY:
				vpone->dt->pause();
				break;
			}
		}
		else if(o==stop)
		{
			Avpone	*vpone=(Avpone *)effect;
			vpone->dt->stop();
		}
		break;
		
		case nyCHANGE:
		if(o==rate)
		{
			Avpone	*vpone=(Avpone *)effect;
			double	f=rate->get();
			{
				f=(pow((f-0.5)*2.0, 3.0)/2.0)+0.5f;
				vpone->rate=(f<0.5)?((f*0.999+0.001)*2.0):((f-0.5)*8.0+1.0);
			}
		}
		else if(o==slide)
		{
			Avpone	*vpone=(Avpone *)effect;
			float	v=slide->getValue();
			double	total=vpone->dt->getTotal();
			double	time=v*total;
			vpone->dt->seek(time);
		}
		return TRUE;
	}
	return AeffectFront::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AvponeFront::pulse()
{
	Avpone	*vpone=(Avpone *)effect;
	double	cur=vpone->dt->getPosition();
	double	len=vpone->dt->getTotal();
	if(fabs(cur-len)<0.0001)
	{
		if(playAll->get())
		{
			if(nmovie<8)
				sel->set((nmovie+1)%9);
			else if(loop->get())
				sel->set((nmovie+1)%9);
			else
				vpone->dt->stop();
		}
		else if(loop->get())
			vpone->dt->seek(0);
		else
			vpone->dt->stop();
	}
	slide->setValue((float)(cur/len));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AvponeBack::AvponeBack(QIID qiid, char *name, Avpone *e, int h) : AeffectBack(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_VPONE_BACK), "PNG");
	back=new Abitmap(&o);

	out=new Avideo(MKQIID(qiid, 0x20335285189c68a0), "video out", this, pinOUT, pos.w-18, 10);
	out->setTooltips("video out");
	out->show(TRUE);

	audio=new Asample(MKQIID(qiid, 0x2033052ab89c68a0), "audio out", this, pinOUT, pos.w-38, 10);
	audio->setTooltips("audio out");
	audio->show(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AvponeBack::~AvponeBack()
{
	delete(back);
	delete(out);
	delete(audio);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AvponeBack::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aeffect * AvponeInfo::create(QIID qiid, char *name, Acapsule *capsule)
{
	return new Avpone(qiid, name, this, capsule);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aplugz * vponeGetInfo()
{
	return new AvponeInfo("vponeInfo", &Avpone::CI, "VP1 player", "VP1 player");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
