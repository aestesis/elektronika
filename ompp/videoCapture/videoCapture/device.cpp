/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	DEVICE.CPP					(c)	YoY'05						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#include						<atlbase.h>
#include						"device.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define RELEASE(pObject)		if(pObject){ pObject->Release(); pObject = NULL;}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
	physicalUNKNOWN,
	physicalVIDEO,
	physicalAUDIO
};

// Helper function to associate a name with the type.
const char * GetPhysicalPinName(long lType, int *type)
{
    switch (lType) 
    {
		case PhysConn_Video_Tuner:            *type=physicalVIDEO; return "Video Tuner";
		case PhysConn_Video_Composite:        *type=physicalVIDEO; return "Video Composite";
		case PhysConn_Video_SVideo:           *type=physicalVIDEO; return "S-Video";
		case PhysConn_Video_RGB:              *type=physicalVIDEO; return "Video RGB";
		case PhysConn_Video_YRYBY:            *type=physicalVIDEO; return "Video YRYBY";
		case PhysConn_Video_SerialDigital:    *type=physicalVIDEO; return "Video Serial Digital";
		case PhysConn_Video_ParallelDigital:  *type=physicalVIDEO; return "Video Parallel Digital"; 
		case PhysConn_Video_SCSI:             *type=physicalVIDEO; return "Video SCSI";
		case PhysConn_Video_AUX:              *type=physicalVIDEO; return "Video AUX";
		case PhysConn_Video_1394:             *type=physicalVIDEO; return "Video 1394";
		case PhysConn_Video_USB:              *type=physicalVIDEO; return "Video USB";
		case PhysConn_Video_VideoDecoder:     *type=physicalVIDEO; return "Video Decoder";
		case PhysConn_Video_VideoEncoder:     *type=physicalVIDEO; return "Video Encoder";
	        
		case PhysConn_Audio_Tuner:            *type=physicalAUDIO; return "Audio Tuner";
		case PhysConn_Audio_Line:             *type=physicalAUDIO; return "Audio Line";
		case PhysConn_Audio_Mic:              *type=physicalAUDIO; return "Audio Microphone";
		case PhysConn_Audio_AESDigital:       *type=physicalAUDIO; return "Audio AES/EBU Digital";
		case PhysConn_Audio_SPDIFDigital:     *type=physicalAUDIO; return "Audio S/PDIF";
		case PhysConn_Audio_SCSI:             *type=physicalAUDIO; return "Audio SCSI";
		case PhysConn_Audio_AUX:              *type=physicalAUDIO; return "Audio AUX";
		case PhysConn_Audio_1394:             *type=physicalAUDIO; return "Audio 1394";
		case PhysConn_Audio_USB:              *type=physicalAUDIO; return "Audio USB";
		case PhysConn_Audio_AudioDecoder:     *type=physicalAUDIO; return "Audio Decoder";
	        
		default:                              *type=physicalUNKNOWN; return "Unknown Type";
    }    
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static HRESULT UnicodeToAnsi(LPCOLESTR pszW, LPSTR* ppszA)
{

    size_t cbAnsi, cCharacters;
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
    if (0 == WideCharToMultiByte(CP_ACP, 0, pszW, (int)cCharacters, *ppszA,
                  (int)cbAnsi, NULL, NULL))
    {
        dwError = GetLastError();
        CoTaskMemFree(*ppszA);
        *ppszA = NULL;
        return HRESULT_FROM_WIN32(dwError);
    }
    return NOERROR;
}

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

static HRESULT GetPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, IPin **ppPin, GUID mediatype)
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
			IEnumMediaTypes	*pEnumMT;
			AM_MEDIA_TYPE	*pmt;
			pPin->EnumMediaTypes(&pEnumMT);
			while(pEnumMT->Next(1, &pmt, 0)==S_OK)
			{
				if(pmt->majortype==mediatype)
				{
					*ppPin = pPin;
					MyDeleteMediaType(pmt);
					pEnumMT->Release();
					pEnum->Release();
					return S_OK;
				}
				MyDeleteMediaType(pmt);
 			}
			pEnumMT->Release();
        }
        pPin->Release();
    }
    pEnum->Release();
    return E_FAIL;  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class IcaptureCB : public ISampleGrabberCB
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
		--_ref_count;
		if(_ref_count)
			return _ref_count;
		delete this;
		return 0;
	}
public:
	IcaptureCB					();
	~IcaptureCB					();

	Asection					section;								
	BYTE						*data;
	int							datalen;
	bool						bnew;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IcaptureCB::IcaptureCB()
{
	_ref_count=1;
	data=NULL;
	datalen=0;
	bnew=false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IcaptureCB::~IcaptureCB()
{
	if(data)
		free(data);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HRESULT __stdcall IcaptureCB::BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen)
{	
	section.lock();
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
	section.unlock();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	static int avFlags[]={		AnalogVideo_None,
								AnalogVideo_NTSC_M, 
								AnalogVideo_NTSC_M_J,
								AnalogVideo_NTSC_433,
								AnalogVideo_PAL_B,
								AnalogVideo_PAL_D,
								AnalogVideo_PAL_H,
								AnalogVideo_PAL_I,
								AnalogVideo_PAL_M,
								AnalogVideo_PAL_N,
								AnalogVideo_PAL_60,
								AnalogVideo_SECAM_B,
								AnalogVideo_SECAM_D,
								AnalogVideo_SECAM_G,
								AnalogVideo_SECAM_H,
								AnalogVideo_SECAM_K,
								AnalogVideo_SECAM_K1,
								AnalogVideo_SECAM_L,
								AnalogVideo_SECAM_L1,
								AnalogVideo_PAL_N_COMBO
	};

	static char *avStandard[]={
								"None",
								"NTSC_M", 
								"NTSC_M_J",
								"NTSC_433",
								"PAL_B",
								"PAL_D",
								"PAL_H",
								"PAL_I",
								"PAL_M",
								"PAL_N",
								"PAL_60",
								"SECAM_B",
								"SECAM_D",
								"SECAM_G",
								"SECAM_H",
								"SECAM_K",
								"SECAM_K1",
								"SECAM_L",
								"SECAM_L1",
								"PAL_N_COMBO"
	};
	
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Avidcap::Avidcap()
{
	memset(devices, 0, sizeof(devices));
	nbdevices=0;
	error=NULL;
	ok=false;
	
	pGraph=NULL;
	pBuild=NULL;
	pMC=NULL;
	pME=NULL;
	pGrabVideo=NULL;
	captureCB=NULL;

	//pDF=NULL;
	//pVA=NULL;
	//pET=NULL;
	//pCC=NULL;
	pDLG=NULL;
	pVDEC=NULL;
	pCROSS=NULL;
	pVPA=NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Avidcap::~Avidcap()
{
	release();
	clearEnumDevices();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avidcap::clearEnumDevices()
{
	int	i;
	for(i=0; i<nbdevices; i++)
	{
		RELEASE(devices[i].moniker);
	}
	nbdevices=0;
	memset(devices, 0, sizeof(devices));
}

void Avidcap::enumDevices()
{
    ICreateDevEnum	*pSysDevEnum=NULL;
    section.lock();
	clearEnumDevices();
    if(CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC, IID_ICreateDevEnum, (void **)&pSysDevEnum)==S_OK)
    {
		IEnumMoniker	*pEnumCat = NULL;

		//
		// WARNING!
		//
		// Some third-party filters throw an exception (int 3) during enumeration
		// on Debug builds, often due to heap corruption in RtlFreeHeap().
		// Sometimes it is done intentionally to prevent reverse engineering.
		// 
		// This is not an issue on Release builds.
		//

		if(pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0)==S_OK)
		{
			IMoniker	*pMoniker;
			ULONG		cFetched;
			VARIANT		varName={0};


			if(pEnumCat)
			{
				while(pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
				{
					IPropertyBag	*pPropBag;
					if(pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag)==S_OK)
					{
						varName.vt=VT_BSTR;
						if(pPropBag->Read(L"FriendlyName", &varName, 0)==S_OK)
						{
							VARIANT varFilterClsid;
							char	*name;
							UnicodeToAnsi(varName.bstrVal, &name);
							strcpy(devices[nbdevices].name, name);
							free(name);
							SysFreeString(varName.bstrVal);
							varFilterClsid.vt=VT_BSTR;
							if(pPropBag->Read(L"CLSID", &varFilterClsid, 0)==S_OK)
							{
								if(CLSIDFromString(varFilterClsid.bstrVal, &devices[nbdevices].clsid) == S_OK)
								{
									devices[nbdevices].moniker=pMoniker;
									pMoniker->AddRef();
									nbdevices++;
								}
								SysFreeString(varFilterClsid.bstrVal);
							}
						}
						RELEASE(pPropBag);
					}
				}
				RELEASE(pMoniker);
			}
			RELEASE(pEnumCat);
		}
		RELEASE(pSysDevEnum);
    }
	{
		int	i;
		for(i=0; i<nbdevices; i++)
		{
			int	mem[128];
			int	n=0;
			int	j;

			for(j=0; j<nbdevices; j++)
				if(!strcmp(devices[i].name, devices[j].name))
					mem[n++]=j;

			if(n>1)
			{
				int	k;
				for(k=0; k<n; k++)
				{
					char	str[128];
					sprintf(str, " #%d", k+1);
					strcat(devices[mem[k]].name, str);
				}
			}
		}
	}
    section.unlock();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IBaseFilter * Avidcap::createSourceInstance(int n)
{
    ICreateDevEnum	*pSysDevEnum=NULL;
    IBaseFilter		*pSource=NULL;
    section.lock();
	devices[n].moniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pSource);
    section.unlock();
    return pSource;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avidcap::init(int n, int wt, int ht, double fps, int bits)
{
	release();
	section.lock();
	if((n>=0)&&(n<nbdevices))
	{
		if(InitCaptureGraphBuilder(&pGraph, &pBuild)!=S_OK)
		{
			error="can't create DS graphBuilder [out of memory]";
			release();
			section.unlock();
			return false;
		}
			
		{
			HRESULT				hr=NULL;
			IBaseFilter			*pSource=NULL;
			IBaseFilter			*pGrabber=NULL;
			IBaseFilter			*pNull=NULL;
			IPin				*pIn=NULL;
			IPin				*pOut=NULL;
			ISampleGrabberCB	*cb=NULL;
			AM_MEDIA_TYPE		mt;
			
			pSource=createSourceInstance(n);
			if(!pSource)
			{
				error="can't create video source";
				release();
				section.unlock();
				return false;
			}

			hr = pGraph->AddFilter(pSource, L"source");
			if(hr!=S_OK)
			{
				error="can't add video source";
				RELEASE(pSource);
				release();
				section.unlock();
				return false;
			}

			hr=CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, reinterpret_cast<void**>(&pGrabber));
			if(hr!=S_OK)
			{
				error="can't create video grabber";
				RELEASE(pSource);
				release();
				section.unlock();
				return false;
			}

			hr=pGrabber->QueryInterface(IID_ISampleGrabber, reinterpret_cast<void**>(&pGrabVideo));
			if(hr!=S_OK)
			{
				error="can't query video grabber";
				RELEASE(pSource);
				RELEASE(pGrabber);
				release();
				section.unlock();
				return false;
			}

			memset(&mt, 0, sizeof(mt));
			mt.majortype = MEDIATYPE_Video;
			mt.subtype = (bits==32)?MEDIASUBTYPE_RGB32:MEDIASUBTYPE_RGB24;

			hr=pGrabVideo->SetMediaType(&mt);

			captureCB=new IcaptureCB();
			hr=captureCB->QueryInterface(IID_ISampleGrabberCB, reinterpret_cast<void**>(&cb));
			if(hr!=S_OK)
			{
				error="can't query video grabber callback";
				RELEASE(pSource);
				RELEASE(pGrabber);
				release();
				section.unlock();
				return false;
			}
			hr=pGrabVideo->SetCallback(cb, 1);
			cb->Release();
			if(hr!=S_OK)
			{
				error="can't set video grabber callback";
				RELEASE(pSource);
				RELEASE(pGrabber);
				release();
				section.unlock();
				return false;
			}

			hr = pGraph->AddFilter(pGrabber, L"Video Grabber");
			if(hr!=S_OK)
			{
				error="can't add video grabber";
				RELEASE(pSource);
				RELEASE(pGrabber);
				release();
				section.unlock();
				return false;
			}
			
			hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, reinterpret_cast<void**>(&pNull));
			if(hr!=S_OK)
			{
				error="can't create video Null";
				RELEASE(pSource);
				RELEASE(pGrabber);
				release();
				section.unlock();
				return false;
			}

			hr = pGraph->AddFilter(pNull, L"NullRenderer Video");
			if(hr!=S_OK)
			{
				error="can't add video Null";
				RELEASE(pNull);
				RELEASE(pSource);
				RELEASE(pGrabber);
				release();
				section.unlock();
				return false;
			}

			hr = pBuild->RenderStream(0, &MEDIATYPE_Video, pSource, NULL, pGrabber);
			if(hr!=S_OK)
			{
				error="can't render video stream";
				RELEASE(pSource);
				RELEASE(pGrabber);
				RELEASE(pNull);
				release();
				section.unlock();
				return false;
			}

//			pSource->QueryInterface(IID_IAMDroppedFrames, (void**)&pDF);
//			pSource->QueryInterface(IID_IAMVideoProcAmp, (void**)&pVA);
//			pSource->QueryInterface(IID_IAMExtTransport, (void**)&pET);
//			pSource->QueryInterface(IID_IAMCameraControl, (void**)&pCC);

			pSource->QueryInterface(IID_IAMVfwCaptureDialogs, (void**)&pDLG);
			pSource->QueryInterface(IID_IAMAnalogVideoDecoder, (void**)&pVDEC);
			pSource->QueryInterface(IID_IAMVideoProcAmp, (void**)&pVPA);
			pBuild->FindInterface(&LOOK_UPSTREAM_ONLY, NULL, pSource, IID_IAMCrossbar, (void**)&pCROSS);
			
			
			//IAMVideoControl

			{	// set frame rate (only for analog video capture)
				IPin	*pin;			
				if(GetPin(pSource, PINDIR_OUTPUT, &pin, MEDIATYPE_Video)==S_OK)
				{
					AM_MEDIA_TYPE	MediaType;
					if(pin->ConnectionMediaType(&MediaType)==S_OK)
					{
						if(MediaType.pbFormat&&(MediaType.formattype==FORMAT_VideoInfo))
						{
							VIDEOINFOHEADER *pvh=(VIDEOINFOHEADER*)MediaType.pbFormat;
							IAMStreamConfig	*pSC;
							if(pin->QueryInterface(IID_IAMStreamConfig, (void**)&pSC)==S_OK)
							{
								double	v=10000000.0/fps;
								pvh->AvgTimePerFrame=(REFERENCE_TIME)v;
								
								{
									pvh->bmiHeader.biBitCount=bits;
									{
										int w=wt;
										int h=ht;
										pvh->bmiHeader.biSizeImage=(bits>>3)*w*h;
										pvh->bmiHeader.biWidth=w;
										pvh->bmiHeader.biHeight=h;
										pvh->AvgTimePerFrame=(REFERENCE_TIME)v;
										ok=(pSC->SetFormat(&MediaType)==S_OK);
									}
								}								
								pSC->Release();
							}
						}
						MyFreeMediaType(MediaType);
					}
					pin->Release();
				}
			}
		
			{	// set DV decode size
				IIPDVDec	*pDVdec;
				if(pBuild->FindInterface(NULL, NULL, pSource, IID_IIPDVDec, (void **)&pDVdec)==S_OK)
				{
					if(wt<512)
						pDVdec->put_IPDisplay(DVRESOLUTION_HALF);
					else
						pDVdec->put_IPDisplay(DVRESOLUTION_FULL);
					pDVdec->Release();
				}
			}

			RELEASE(pSource);

			hr = pBuild->RenderStream(0, &MEDIATYPE_Video, pGrabber, 0, pNull);
			RELEASE(pGrabber);
			RELEASE(pNull);
			if(hr!=S_OK)
			{
				error="can't render video/null stream [out of memory]";
				release();
				section.unlock();
				return false;
			}

			hr=pGrabVideo->SetBufferSamples(true);
			if(hr!=S_OK)
			{
				error="can't bufferize video stream [out of memory]";
				release();
				section.unlock();
				return false;
			}

			pGraph->QueryInterface(IID_IMediaEvent, (void**)&pME);

			hr=pGraph->QueryInterface(IID_IMediaControl, (void**)&pMC);
			if(hr!=S_OK)
			{
				error="can't create media control [out of memory]";
				release();
				section.unlock();
				return false;
			}
		}
	}
	section.unlock();
	ok=true;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avidcap::release()
{
	section.lock();
	ok=false;
	if(pMC)
		pMC->Stop();
		
	RELEASE(pGraph);
	RELEASE(pBuild);
	RELEASE(pMC);
	RELEASE(pME);
	if(pGrabVideo)
		pGrabVideo->SetCallback(NULL, 1);
	RELEASE(pGrabVideo);
	RELEASE(captureCB);

	//RELEASE(pDF);
	//RELEASE(pVA);
	//RELEASE(pET);
	//RELEASE(pCC);
	
	RELEASE(pDLG);
	RELEASE(pVDEC);
	RELEASE(pCROSS);
	RELEASE(pVPA);

	section.unlock();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avidcap::getBitmap(MediaBitmap *image, bool deinterlace)
{
	bool	ok=false;
	section.lock();
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

				if((w==image->getWidth())||(h==image->getHeight()))
				{
					size=w*h*4;

					captureCB->section.lock();
					if(size>=captureCB->datalen)
					{
						if(captureCB->bnew)
						{
							{
								int		w=image->getWidth();
								int		h=image->getHeight();
								DWORD	*src=((DWORD *)captureCB->data);
								int		x,y;
								assert(w*h*4==captureCB->datalen);
								for(y=0; y<h; y++)
								{
									DWORD	*dst=((DWORD *)image->getPixels())+(h-1-y)*w;
									for(x=0; x<w; x++)
									{
										MPbyte	rc,gc,bc;
										MPcolorRGB(&rc, &gc, &bc, *(src++));
										*(dst++)=MPcolor32(rc, gc, bc, 255);
									}
								}
							}
							captureCB->bnew=false;
							if(deinterlace)
								this->deinterlace(image);
						}
						ok=true;
					}
					captureCB->section.unlock();
				}
			}
			MyFreeMediaType(MediaType);
		}
	}
	section.unlock();
	return ok;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avidcap::deinterlace(MediaBitmap *b)
{
	MPdword	*bits=b->getPixels();
	int		w=b->getWidth();
	int		h=b->getHeight();
	int	y;
	int	size=w*sizeof(MPdword);
	int	n=h-2;
	for(y=0; y<n; y+=2)
	{
		MPdword	*s0=bits+y*w;
		MPdword	*s1=bits+(y+2)*w;
		MPdword	*d=bits+(y+1)*w;
		int		x;
		for(x=0; x<w; x++)
		{
			byte	r0,g0,b0;
			byte	r1,g1,b1;
			MPcolorRGB(&r0, &g0, &b0, *(s0++));
			MPcolorRGB(&r1, &g1, &b1, *(s1++));
			*(d++)=MPcolor32(((int)r0+(int)r1)>>1, ((int)g0+(int)g1)>>1, ((int)b0+(int)b1)>>1);
		}
	}
	{
		MPdword	*s=bits+(h-2)*w;
		MPdword	*d=bits+(h-1)*w;
		memcpy(d, s, size);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avidcap::start()
{
	section.lock();
	if(pMC)
		pMC->Run();
	section.unlock();
	return (getState()==statePLAY);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avidcap::stop()
{
	section.lock();
	if(pMC)
		pMC->Stop();
	section.unlock();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Avidcap::getState()
{
	OAFilterState	pfs;
	section.lock();
	if(pMC&&(pMC->GetState(INFINITE, &pfs)==S_OK))
	{
		section.unlock();
		return (int)pfs;
	}
	section.unlock();
	return stateNODEVICE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avidcap::dialog(HWND hwnd, int n)
{
	bool b=false;
	section.lock();
	if(pDLG)
		if(pDLG->HasDialog(n)==S_OK)
			b=true;
	section.unlock();
	return b;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avidcap::isdialog(int n)
{
	bool b=false;
	section.lock();
	if(pDLG)
		if(pDLG->HasDialog(n)==S_OK)
			b=true;
	section.unlock();
	return b;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Avidcap::getAnalogVideoStandardInfo()
{
	long	f=0;
	section.lock();
	if(pVDEC)
		pVDEC->get_AvailableTVFormats(&f);
	section.unlock();
	return (int)f;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
void Avidcap::setAnalogVideoStandard(int f)
{
	section.lock();
	if(pVDEC)
		pVDEC->put_TVFormat((long)f);
	section.unlock();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
int Avidcap::getAnalogVideoStandard()
{
	int	ret=0;
	section.lock();
	if(pVDEC)
	{
		long std;
		if(pVDEC->get_TVFormat(&std)==S_OK)
		{
			ret=std;
		}
	}
	section.unlock();
	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Avidcap::getAnalogSourceInfo(int in, char *name)
{
	int ret=0;
	section.lock();
	if(pCROSS)
	{
		if(in==-1)
		{
			long	nout=0;
			long	nin=0;
			if(pCROSS->get_PinCounts(&nout, &nin)==S_OK)
				ret=nin;
		}
		else
		{
			long					t;
			long					link;
			int						audvid;
			pCROSS->get_CrossbarPinInfo(true, in, &link, &t);
			const char				*s=GetPhysicalPinName(t, &audvid);
			if(audvid==physicalVIDEO)
			{
				strcpy(name, s);
				ret=true;
			}
			else
				name[0]=0;
		}
	}	
	section.unlock();
	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avidcap::setAnalogSource(int in)
{
	bool ret=false;
	section.lock();
	if(pCROSS)
	{
		{
			long	nout=0;
			long	nin=0;
			int		out;
			if(pCROSS->get_PinCounts(&nout, &nin)==S_OK)
			{
				for(out=0; out<nout; out++)
				{
					long	t;
					long	link;
					pCROSS->get_CrossbarPinInfo(false, out, &link, &t);
					if(t==PhysConn_Video_VideoDecoder)
						break;
				}
				if(out<nout)
					ret=(pCROSS->Route(out, in)==S_OK);
			}
		}
	}	
	section.unlock();
	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Avidcap::getAnalogSource()
{
	int ret=-1;
	section.lock();
	if(pCROSS)
	{
		{
			long	nout=0;
			long	nin=0;
			int		out;
			if(pCROSS->get_PinCounts(&nout, &nin)==S_OK)
			{
				for(out=0; out<nout; out++)
				{
					long	t;
					long	link;
					pCROSS->get_CrossbarPinInfo(false, out, &link, &t);
					if(t==PhysConn_Video_VideoDecoder)
						break;
				}
				if(out<nout)
				{
					long	in;
					if(pCROSS->get_IsRoutedTo(out, &in)==S_OK)
						ret=in;
				}
			}
		}
	}	
	section.unlock();
	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float Avidcap::getAnalogSetting(int set)
{
	float	v=0.5f;
	section.lock();
	if(pVPA)
	{
		long min,max,stp,def,flags;
		if(pVPA->GetRange(set, &min, &max, &stp, &def, &flags)==S_OK)
		{
			long val;
			if(pVPA->Get(set, &val, &flags)==S_OK)
				v=((float)val-(float)min)/((float)max-(float)min);
		}
	}
	section.unlock();
	return v;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avidcap::setAnalogSetting(int set, float value)
{
	bool	ret=false;
	section.lock();
	if(pVPA)
	{
		long min,max,stp,def,flags;
		if(pVPA->GetRange(set, &min, &max, &stp, &def, &flags)==S_OK)
		{
			long val=(long)(value*((float)max-(float)min)+(float)min);
			if(pVPA->Set(set, val, VideoProcAmp_Flags_Manual)==S_OK)
				ret=true;
		}
	}
	section.unlock();
	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
