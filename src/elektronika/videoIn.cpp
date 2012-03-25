/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	VIDEOIN.CPP					(c)	YoY'03						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#include						<atlbase.h>
#include						<dshow.h>
#include						<xprtdefs.h>
#include						<dbt.h>
#include						<math.h>
#include						<assert.h>
#include						<Dvdmedia.h>
#include						"../alib/defSampleGrabber.h"
#include						"resource.h"
#include						"videoin.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								AvideoinInfo::CI	= ACI("AvideoinInfo",		GUID(0x11111112,0x00000380), &AeffectInfo::CI, 0, NULL);
ACI								Avideoin::CI		= ACI("Avideoin",			GUID(0x11111112,0x00000381), &Aeffect::CI, 0, NULL);
ACI								AvideoinFront::CI	= ACI("AvideoinFront",		GUID(0x11111112,0x00000382), &AeffectFront::CI, 0, NULL);
ACI								AvideoinBack::CI	= ACI("AvideoinBack",		GUID(0x11111112,0x00000383), &AeffectBack::CI, 0, NULL);

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

typedef struct 
{
	CLSID						clsid;
	char						name[256];
	IMoniker					*moniker;
} Tdevice;

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

class Avidcap
{
public:


	enum
	{
								stateSTOP,
								statePAUSE,
								statePLAY,
								stateNODEVICE
	};
	
	enum
	{
								dialogSOURCE				=VfwCaptureDialog_Source,
								dialogFORMAT				=VfwCaptureDialog_Format,
								dialogDISPLAY				=VfwCaptureDialog_Display
	};

	enum
	{
								standardNONE,
								standardNTSC_M, 
								standardNTSC_M_J,
								standardNTSC_433,
								standardPAL_B,
								standardPAL_D,
								standardPAL_H,
								standardPAL_I,
								standardPAL_M,
								standardPAL_N,
								standardPAL_60,
								standardSECAM_B,
								standardSECAM_D,
								standardSECAM_G,
								standardSECAM_H,
								standardSECAM_K,
								standardSECAM_K1,
								standardSECAM_L,
								standardSECAM_L1,
								standardPAL_N_COMBO,
								standardMAXIMUM
	};	
	
	enum
	{
								analogBRIGHTNESS=VideoProcAmp_Brightness,
								analogCONTRAST=VideoProcAmp_Contrast,
								analogHUE=VideoProcAmp_Hue,
								analogSATURATION=VideoProcAmp_Saturation,
								analogSHARPNESS=VideoProcAmp_Sharpness,
								analogGAMMA=VideoProcAmp_Gamma,
								analogCOLOR=VideoProcAmp_ColorEnable,
								analogWHITE=VideoProcAmp_WhiteBalance,
								analogBACKLIGHT=VideoProcAmp_BacklightCompensation,
								analogGAIN=VideoProcAmp_Gain,
								analogMAXIMUM
	};


								Avidcap						();
								~Avidcap					();
								
	void						enumDevices					();
	void						clearEnumDevices			();

	IBaseFilter *				createSourceInstance		(int n);
 
	
	bool						init						(int n, int width, int height, double fps, int bits);
	void						release						();
	
	bool						getBitmap					(Abitmap *b, bool deinterlace);
	void						deinterlace					(Abitmap *b);
	
	int							getState					();
	bool						start						();
	void						stop						();
	
	bool						dialog						(HWND hw, int n);
	bool						isdialog					(int n);
	
	int							getAnalogVideoStandardInfo	();
	int							getAnalogVideoStandard		();
	void						setAnalogVideoStandard		(int f);
	
	int							getAnalogSourceInfo			(int in=-1, char *name=NULL);	// getAnalogSource() return the number of sources otherwise return the pin physical type (physicalVIDEO or physicalAUDIO)
	int							getAnalogSource				();
	bool						setAnalogSource				(int in);

	float						getAnalogSetting			(int set);
	bool						setAnalogSetting			(int set, float value);
	
	IGraphBuilder				*pGraph;
	ICaptureGraphBuilder2		*pBuild;
	IMediaControl				*pMC;
	IcaptureCB					*captureCB;
	ISampleGrabber				*pGrabVideo;
	IMediaEvent					*pME;

//	IAMDroppedFrames			*pDF;
//	IAMExtTransport				*pET;
//	IAMCameraControl			*pCC;

	IAMVideoProcAmp				*pVPA;
	IAMAnalogVideoDecoder		*pVDEC;
	IAMCrossbar					*pCROSS;
	
	IAMVfwCaptureDialogs		*pDLG;
	
	Asection					section;
	Tdevice						devices[128];
	int							nbdevices;
	char						*error;
	bool						ok;
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
    section.enter(__FILE__,__LINE__);
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
    section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IBaseFilter * Avidcap::createSourceInstance(int n)
{
    ICreateDevEnum	*pSysDevEnum=NULL;
    IBaseFilter		*pSource=NULL;
    section.enter(__FILE__,__LINE__);
	devices[n].moniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pSource);
    section.leave();
    return pSource;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avidcap::init(int n, int wt, int ht, double fps, int bits)
{
	release();
	section.enter(__FILE__,__LINE__);
	if((n>=0)&&(n<nbdevices))
	{
		if(InitCaptureGraphBuilder(&pGraph, &pBuild)!=S_OK)
		{
			error="can't create DS graphBuilder [out of memory]";
			release();
			section.leave();
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
				section.leave();
				return false;
			}

			hr = pGraph->AddFilter(pSource, L"source");
			if(hr!=S_OK)
			{
				error="can't add video source";
				RELEASE(pSource);
				release();
				section.leave();
				return false;
			}

			hr=CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, reinterpret_cast<void**>(&pGrabber));
			if(hr!=S_OK)
			{
				error="can't create video grabber";
				RELEASE(pSource);
				release();
				section.leave();
				return false;
			}

			hr=pGrabber->QueryInterface(IID_ISampleGrabber, reinterpret_cast<void**>(&pGrabVideo));
			if(hr!=S_OK)
			{
				error="can't query video grabber";
				RELEASE(pSource);
				RELEASE(pGrabber);
				release();
				section.leave();
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
				section.leave();
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
				section.leave();
				return false;
			}

			hr = pGraph->AddFilter(pGrabber, L"Video Grabber");
			if(hr!=S_OK)
			{
				error="can't add video grabber";
				RELEASE(pSource);
				RELEASE(pGrabber);
				release();
				section.leave();
				return false;
			}
			
			hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, reinterpret_cast<void**>(&pNull));
			if(hr!=S_OK)
			{
				error="can't create video Null";
				RELEASE(pSource);
				RELEASE(pGrabber);
				release();
				section.leave();
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
				section.leave();
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
				section.leave();
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
									int	i,j,k,nk;
									for(i=0; i<Atable::reso_MAXIMUM; i++)
										if((Atable::infoReso[i].width>=wt)&&(Atable::infoReso[i].height>=ht))
											break;
											
									// 32 bits
									pvh->bmiHeader.biBitCount=32;
									for(j=i; j>=0; j--)
									{
										int w=Atable::infoReso[j].width;
										int h=Atable::infoReso[j].height;
										pvh->bmiHeader.biSizeImage=4*w*h;
										pvh->bmiHeader.biWidth=w;
										pvh->bmiHeader.biHeight=h;
										nk=(w>=512)?2:1;
										for(k=nk; k>=1; k--)
										{
											pvh->AvgTimePerFrame=(REFERENCE_TIME)(v/(double)k);
											if(pSC->SetFormat(&MediaType)==S_OK)
												break;
										}
										if(k>=1)
											break;
									}
									
									if(j<0)	// 24 bits
									{
										pvh->bmiHeader.biBitCount=24;
										for(j=i; j>=0; j--)
										{
											int w=Atable::infoReso[j].width;
											int h=Atable::infoReso[j].height;
											pvh->bmiHeader.biSizeImage=3*w*h;
											pvh->bmiHeader.biWidth=w;
											pvh->bmiHeader.biHeight=h;
											nk=(w>=512)?2:1;
											for(k=nk; k>=1; k--)
											{
												pvh->AvgTimePerFrame=(REFERENCE_TIME)(v/(double)k);
												if(pSC->SetFormat(&MediaType)==S_OK)
													break;
											}
											if(k>=1)
												break;
										}
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
				section.leave();
				return false;
			}

			hr=pGrabVideo->SetBufferSamples(true);
			if(hr!=S_OK)
			{
				error="can't bufferize video stream [out of memory]";
				release();
				section.leave();
				return false;
			}

			pGraph->QueryInterface(IID_IMediaEvent, (void**)&pME);

			hr=pGraph->QueryInterface(IID_IMediaControl, (void**)&pMC);
			if(hr!=S_OK)
			{
				error="can't create media control [out of memory]";
				release();
				section.leave();
				return false;
			}
		}
	}
	section.leave();
	ok=true;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avidcap::release()
{
	section.enter(__FILE__,__LINE__);
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

	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avidcap::getBitmap(Abitmap *image, bool deinterlace)
{
	bool	ok=false;
	section.enter(__FILE__,__LINE__);
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

				captureCB->section.enter(__FILE__,__LINE__);
				if(size>=captureCB->datalen)
				{
					if(captureCB->bnew)
					{
						memcpy(image->body, captureCB->data, captureCB->datalen);
						image->flipY();
						captureCB->bnew=false;
						if(deinterlace)
							this->deinterlace(image);
					}
					ok=true;
				}
				captureCB->section.leave();
			}
			MyFreeMediaType(MediaType);
		}
	}
	section.leave();
	return ok;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avidcap::deinterlace(Abitmap *b)
{
	int	y;
	int	size=b->w*sizeof(dword);
	int	n=b->h-2;
	for(y=0; y<n; y+=2)
	{
		dword	*s0=&b->body32[b->adr[y]];
		dword	*s1=&b->body32[b->adr[y+2]];
		dword	*d=&b->body32[b->adr[y+1]];
		int		x;
		for(x=0; x<b->w; x++)
		{
			byte	r0,g0,b0;
			byte	r1,g1,b1;
			colorRGB(&r0, &g0, &b0, *(s0++));
			colorRGB(&r1, &g1, &b1, *(s1++));
			*(d++)=color32(((int)r0+(int)r1)>>1, ((int)g0+(int)g1)>>1, ((int)b0+(int)b1)>>1);
		}
	}
	{
		dword	*s=&b->body32[b->adr[b->h-2]];
		dword	*d=&b->body32[b->adr[b->h-1]];
		memcpy(d, s, size);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avidcap::start()
{
	section.enter(__FILE__,__LINE__);
	if(pMC)
		pMC->Run();
	section.leave();
	return (getState()==statePLAY);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avidcap::stop()
{
	section.enter(__FILE__,__LINE__);
	if(pMC)
		pMC->Stop();
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Avidcap::getState()
{
	OAFilterState	pfs;
	section.enter(__FILE__,__LINE__);
	if(pMC&&(pMC->GetState(INFINITE, &pfs)==S_OK))
	{
		section.leave();
		return (int)pfs;
	}
	section.leave();
	return stateNODEVICE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avidcap::dialog(HWND hwnd, int n)
{
	bool b=false;
	section.enter(__FILE__,__LINE__);
	if(pDLG)
		if(pDLG->HasDialog(n)==S_OK)
			b=true;
	section.leave();
	return b;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avidcap::isdialog(int n)
{
	bool b=false;
	section.enter(__FILE__,__LINE__);
	if(pDLG)
		if(pDLG->HasDialog(n)==S_OK)
			b=true;
	section.leave();
	return b;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Avidcap::getAnalogVideoStandardInfo()
{
	long	f=0;
	section.enter(__FILE__,__LINE__);
	if(pVDEC)
		pVDEC->get_AvailableTVFormats(&f);
	section.leave();
	return (int)f;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
void Avidcap::setAnalogVideoStandard(int f)
{
	section.enter(__FILE__,__LINE__);
	if(pVDEC)
		pVDEC->put_TVFormat((long)f);
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
int Avidcap::getAnalogVideoStandard()
{
	int	ret=0;
	section.enter(__FILE__,__LINE__);
	if(pVDEC)
	{
		long std;
		if(pVDEC->get_TVFormat(&std)==S_OK)
		{
			ret=std;
		}
	}
	section.leave();
	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Avidcap::getAnalogSourceInfo(int in, char *name)
{
	int ret=0;
	section.enter(__FILE__,__LINE__);
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
	section.leave();
	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avidcap::setAnalogSource(int in)
{
	bool ret=false;
	section.enter(__FILE__,__LINE__);
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
	section.leave();
	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Avidcap::getAnalogSource()
{
	int ret=-1;
	section.enter(__FILE__,__LINE__);
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
	section.leave();
	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float Avidcap::getAnalogSetting(int set)
{
	float	v=0.5f;
	section.enter(__FILE__,__LINE__);
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
	section.leave();
	return v;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avidcap::setAnalogSetting(int set, float value)
{
	bool	ret=false;
	section.enter(__FILE__,__LINE__);
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
	section.leave();
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

Avideoin::Avideoin(QIID qiid, char *name, AeffectInfo *info, Acapsule *capsule) : Aeffect(qiid, name, info, capsule)
{
	vcap=new Avidcap();
	image=new Abitmap(getVideoWidth(), getVideoHeight());
	front=new AvideoinFront(qiid, "video in front", this, 76);
	front->setTooltips("videoInput module");
	back=new AvideoinBack(qiid, "video in back", this, 76);
	back->setTooltips("videoInput module");
	settings(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Avideoin::~Avideoin()
{
	delete(vcap);
	delete(image);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avideoin::load(class Afile *f)
{
	return ((AvideoinFront *)front)->load(f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avideoin::save(class Afile *f)
{
	return ((AvideoinFront *)front)->save(f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avideoin::settings(bool emergency)
{
	if(emergency)
		((AvideoinFront *)front)->device->setCurrentByData(666);
	Aitem *i=((AvideoinFront *)front)->device->getCurrent();
	if(!vcap->init(i->data, getVideoWidth(), getVideoHeight(), getFrameRate(), 32))
	{
		if(!vcap->init(i->data, getVideoWidth(), getVideoHeight(), getFrameRate(), 24))
			notify(this, nyERROR, (dword)vcap->error);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avideoin::actionStart(int time)
{
	AvideoinFront *front=(AvideoinFront*)this->front;
	vcap->setAnalogVideoStandard(avFlags[front->standard->getCurrentData()]);
	vcap->setAnalogSource(front->source->getCurrentData());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avideoin::action(double time, double dtime, double beat, double dbeat)
{
	AvideoinFront	*front=(AvideoinFront *)this->front;
	AvideoinBack	*back=(AvideoinBack *)this->back;
	Abitmap			*bb=back->video->getBitmap();
	bool			deinterlace=front->deinterlace->isChecked();
	float			zoom=front->pad[8]->get();
	float			aspect=front->pad[9]->get();
	
	if(bb)
	{
		// todo: add test etat play/stop and play
		if(vcap->getBitmap(image, deinterlace))
		{
			int	dx=(int)((zoom*(float)bb->w)+((aspect>0.5f)?((float)bb->w*(aspect-0.5f)):0.f));
			int	dy=(int)((zoom*(float)bb->h)+((aspect<0.5f)?((float)bb->h*(0.5f-aspect)):0.f));
			bb->set(-dx, -dy, bb->w+dx*2, bb->h+dy*2, 0, 0, image->w, image->h, image, bitmapDEFAULT, bitmapDEFAULT);
			bb->fillAlpha(1.f);
		}
		else
			bb->boxf(0, 0, bb->w, bb->h, 0xff000000);
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

typedef struct
{
	qword	qiid;
	int		analog;
	char	*name;
	char	*help;
	float	def;
} TPAD;

static TPAD tpad[VIN_MAXPAD]=
{
	{ 0x5632147852205251, Avidcap::analogBRIGHTNESS, "brightness", "brightness", 0.5f},
	{ 0x9654230025522124, Avidcap::analogCONTRAST, "contrast", "contrast", 0.5f},
	{ 0x6523145875663211, Avidcap::analogHUE, "hue", "hue", 0.5f},
	{ 0x8548142023502066, Avidcap::analogSATURATION, "saturation", "saturation", 0.5f},
	{ 0x5328915151020651, Avidcap::analogSHARPNESS, "sharpness", "sharpness", 0.5f},
	{ 0x1206513148123151, Avidcap::analogGAMMA, "gamma", "gamma correction", 0.5f},
	{ 0x8191218521310241, Avidcap::analogWHITE, "white", "white balance", 0.5f},
	{ 0x8465165222010055, Avidcap::analogGAIN, "gain", "gain correction", 0.5f},
	{ 0x6516512065102006, Avidcap::analogMAXIMUM, "zoom", "zoom correction", 0.f},
	{ 0x6510560561561155, Avidcap::analogMAXIMUM, "aspect", "aspect correction", 0.5f}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AvideoinFront::AvideoinFront(QIID qiid, char *name, Avideoin *e, int h) : AeffectFront(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_VIDEOIN_FRONT), "PNG");
	back=new Abitmap(&o);

	idevice=new Aitem("video input device", "video input device");
	strcpy(olddev, "[none]");
	strcpy(curdev, "[none]");

	device=new Alist("video input device", this, 10, 16, 150, 16, idevice);
	device->setTooltips("video input device");
	device->show(TRUE);

	isource=new Aitem("video input source", "video input source");
	source=new Alist("video input source", this, 190, 16, 80, 16, isource);
	source->setTooltips("video input source");
	source->show(TRUE);
	new Aitem("[source]", "video input source", isource, -1);
	source->setCurrentByData(-1);
	
	istandard=new Aitem("video input standard", "video input standard");
	standard=new Alist("video input standard", this, 280, 16, 80, 16, istandard);
	standard->setTooltips("video input standard");
	standard->show(TRUE);
	new Aitem("[standard]", "video input standard", istandard, 0);
	standard->setCurrentByData(0);
	
	deinterlace=new Abutton("deinterlace", this, 382, 16, 16, 16, &resource.get(MAKEINTRESOURCE(PNG_VIDEOIN_DEINTERLACE), "PNG"), Abutton::btBITMAP|Abutton::bt2STATES);
	deinterlace->setTooltips("video capture deinterlace");
	deinterlace->show(true);

	idialog=new Aitem("video input dialog", "video input dialog");
	dialog=new Alist("video input dialog", this, 420, 16, 80, 16, idialog);
	dialog->setTooltips("video input dialog");
	dialog->show(TRUE);
	new Aitem("[dialog]", "video input properties dialog", idialog, 0);
	dialog->setCurrentByData(0);

	{
		int	i;
		for(i=0; i<VIN_MAXPAD; i++)
		{
			int	x=i*32+16+(i/4)*16;
			pad[i]=new Apaddle(MKQIID(qiid, tpad[i].qiid), tpad[i].name, this, x, 40, paddleYOLI16);
			pad[i]->setTooltips(tpad[i].help);
			pad[i]->set(tpad[i].def);
			pad[i]->show(TRUE);
		}
	}
	
	refresh();

	{
		DEV_BROADCAST_DEVICEINTERFACE filterData;
		memset(&filterData, 0, sizeof(filterData));
		filterData.dbcc_size = sizeof(filterData);
		filterData.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
		filterData.dbcc_classguid = AM_KSCATEGORY_CAPTURE;
		hDevNotify=RegisterDeviceNotification(getWindow()->hw, &filterData, DEVICE_NOTIFY_WINDOW_HANDLE);
		assert(hDevNotify != NULL);
	}
	
	timer(200);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AvideoinFront::~AvideoinFront()
{
	delete(device);
	delete(idevice);
	delete(dialog);
	delete(idialog);
	delete(standard);
	delete(istandard);
	delete(source);
	delete(isource);
	delete(deinterlace);
	{
		int	i;
		for(i=0; i<VIN_MAXPAD; i++)
			delete(pad[i]);
	}
	delete(back);
	if(hDevNotify)
		UnregisterDeviceNotification(hDevNotify);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AvideoinFront::load(Afile *f)
{
	char	name[1024];
	bool	deinter=false;
	f->readString(name);
	if(device->setCurrentByName(name))
		notify(device, nyCHANGE, 0);
	f->readString(name);
	if(standard->setCurrentByName(name))
		notify(standard, nyCHANGE, 0);
	f->readString(name);
	if(source->setCurrentByName(name))
		notify(source, nyCHANGE, 0);
	f->read(&deinter, sizeof(deinter));
	deinterlace->setChecked(deinter);

	if(!device->getCurrent())
		device->setCurrentByData(666);
	if(!standard->getCurrent())
		standard->setCurrentByData(0);
	if(!source->getCurrent())
		source->setCurrentByData(-1);

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AvideoinFront::save(Afile *f)
{
	Aitem	*id=device->getCurrent();
	Aitem	*is=standard->getCurrent();
	Aitem	*isrc=source->getCurrent();
	bool	deinter=deinterlace->isChecked();
	f->writeString(id->name);
	f->writeString(is->name);
	f->writeString(isrc->name);
	f->write(&deinter, sizeof(deinter));
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AvideoinFront::refresh()
{
	Avidcap	*vcap=((Avideoin *)effect)->vcap;
	int		nold=vcap->nbdevices;
	int	i;
	while(idevice->fchild)
		delete(idevice->fchild);
	new Aitem("[none]", "no video out", idevice, (dword)666);
	vcap->enumDevices();
	for(i=0; i<vcap->nbdevices; i++)
		new Aitem(vcap->devices[i].name, vcap->devices[i].name, idevice, (dword)i);
	if(!device->setCurrentByName(curdev))
	{
		strcpy(olddev, curdev);
		strcpy(curdev, "[none]");
		device->setCurrentByData(666);
		vcap->release();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AvideoinFront::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AvideoinFront::pulse()
{
	Avideoin	*vin=(Avideoin *)effect;
	Avidcap		*vcap=vin->vcap;
	dword		d=device->getCurrentData();
	
	if(vin->getTable()->running)
	{
		switch(vcap->getState())
		{
			case Avidcap::stateSTOP:
			if(!vcap->start())
			{
				//vcap->release();
				//device->setCurrentByData(666);
			}
			break;
		}
	}
	else
	{
		switch(vcap->getState())
		{
			case Avidcap::statePLAY:
			vcap->stop();
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AvideoinFront::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCHANGE:
		if(o==device)
		{
			Avideoin	*vin=(Avideoin *)effect;
			Avidcap		*vcap=vin->vcap;
			Aitem		*i=device->getCurrent();
			strcpy(olddev, curdev);
			strcpy(curdev, i->name);
			if(!vcap->init(i->data, vin->getVideoWidth(), vin->getVideoHeight(), vin->getFrameRate(), 32))
			{
				if(!vcap->init(i->data, vin->getVideoWidth(), vin->getVideoHeight(), vin->getFrameRate(), 24))
					notify(this, nyERROR, (dword)vcap->error);
			}
			{
				while(idialog->fchild)
					delete(idialog->fchild);
				new Aitem("[dialog]", "video input properties dialog", idialog, 0);
				if(vcap->isdialog(Avidcap::dialogDISPLAY))
					new Aitem("display", "video input display properties", idialog, (dword)Avidcap::dialogDISPLAY);
				if(vcap->isdialog(Avidcap::dialogFORMAT))
					new Aitem("format", "video input format properties", idialog, (dword)Avidcap::dialogFORMAT);
				if(vcap->isdialog(Avidcap::dialogSOURCE))
					new Aitem("source", "video input source properties", idialog, (dword)Avidcap::dialogSOURCE);
				dialog->setCurrentByData(0);
			}
			{
				int	i;
				int	stdi=vcap->getAnalogVideoStandardInfo();
				int	std=vcap->getAnalogVideoStandard();
				int	istd=0;
				while(istandard->fchild)
					delete(istandard->fchild);
				new Aitem("[standard]", "video input standard", istandard, 0);
				for(i=0; i<Avidcap::standardMAXIMUM; i++)
				{
					if(stdi&avFlags[i])
					{
						new Aitem(avStandard[i], avStandard[i], istandard, i);
						if(avFlags[i]==std)
							istd=i;
					}
				}
				standard->setCurrentByData(istd);
			}
			{
				int	i;
				int	nsrc=vcap->getAnalogSourceInfo();
				isource->clear();
				new Aitem("[source]", "video input source", isource, -1);
				for(i=0; i<nsrc; i++)
				{
					char	name[128];
					if(vcap->getAnalogSourceInfo(i, name))
						new Aitem(name, name, isource, i);
				}
				source->setCurrentByData(vcap->getAnalogSource());
			}
			{
				int	i;
				for(i=0; i<VIN_MAXPAD; i++)
				{
					if(tpad[i].analog!=Avidcap::analogMAXIMUM)
						pad[i]->set(vcap->getAnalogSetting(tpad[i].analog));
				}
			}
		}
		else if(o==dialog)
		{
			Avideoin	*vin=(Avideoin *)effect;
			Avidcap		*vcap=vin->vcap;
			dword		d=dialog->getCurrentData();
			if(d)
			{
				bool	b=(vcap->getState()==Avidcap::statePLAY);
				if(b)
					vcap->stop();
				vcap->dialog(getWindow()->hw, d);
				if(b)
					vcap->start();
				dialog->setCurrentByData(0);
			}
		}
		else if(o==standard)
		{
			Avideoin	*vin=(Avideoin *)effect;
			Avidcap		*vcap=vin->vcap;
			vcap->setAnalogVideoStandard(avFlags[standard->getCurrentData()]);
		}
		else if(o==source)
		{
			Avideoin	*vin=(Avideoin *)effect;
			Avidcap		*vcap=vin->vcap;
			vcap->setAnalogSource(source->getCurrentData());
		}
		else
		{
			Avideoin	*vin=(Avideoin *)effect;
			Avidcap		*vcap=vin->vcap;
			int			i;
			for(i=0; i<VIN_MAXPAD; i++)
			{
				if(o==pad[i])
				{
					if(tpad[i].analog!=Avidcap::analogMAXIMUM)
						vcap->setAnalogSetting(tpad[i].analog, pad[i]->get());
				}
			}
		}
		break;
	}
	return AeffectFront::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AvideoinFront::system(int event, dword d0, dword d1)
{
	switch(event)
	{
		case systemDEVICECHANGE:
		switch(d0)
		{
			case DBT_DEVICEARRIVAL:
			case DBT_DEVICEREMOVECOMPLETE:
			case DBT_DEVICEREMOVEPENDING:
			{
				PDEV_BROADCAST_HDR pdbh = (PDEV_BROADCAST_HDR) d1;
				if (pdbh->dbch_devicetype==DBT_DEVTYP_DEVICEINTERFACE)
				{
					PDEV_BROADCAST_DEVICEINTERFACE pdbi = (PDEV_BROADCAST_DEVICEINTERFACE) d1;
					if (pdbi->dbcc_classguid == AM_KSCATEGORY_CAPTURE)
					{
						refresh();
						return true;
					}
				}
			}
			break;
		}
		break;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AvideoinBack::AvideoinBack(QIID qiid, char *name, Avideoin *e, int h) : AeffectBack(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_VIDEOIN_BACK), "PNG");
	back=new Abitmap(&o);

	video=new Avideo(MKQIID(qiid, 0x3a53465cb99b3080), "video out", this, pinOUT, pos.w-18, 10);
	video->setTooltips("video out");
	video->show(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AvideoinBack::~AvideoinBack()
{
	delete(back);
	delete(video);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AvideoinBack::paint(Abitmap *b)
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

Aeffect * AvideoinInfo::create(QIID qiid, char *name, Acapsule *capsule)
{
	return new Avideoin(qiid, name, this, capsule);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aplugz * videoinGetInfo()
{
	return new AvideoinInfo("videoinInfo", &Avideoin::CI, "video Input", "video Input module");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
