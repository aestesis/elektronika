/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	DVCAP.CPP					(c)	YoY'01						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<windows.h>
#include						<dshow.h>
#include						<d3d9.h>
#include						<assert.h>
//#include						<qedit.h>
#include						<xprtdefs.h>
#include						<dbt.h>
#include						"../alib/defSampleGrabber.h"

#include						"dvcap.h"
#include						"resource.h"

//#include						"leffect.h"
//#include						"CDVGraph.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								AdvcapInfo::CI	= ACI("AdvcapInfo",		GUID(0x11111112,0x00000100), &AeffectInfo::CI, 0, NULL);
ACI								Advcap::CI		= ACI("Advcap",			GUID(0x11111112,0x00000101), &Aeffect::CI, 0, NULL);
ACI								AdvcapFront::CI	= ACI("AdvcapFront",	GUID(0x11111112,0x00000102), &AeffectFront::CI, 0, NULL);
ACI								AdvcapBack::CI	= ACI("AdvcapBack",		GUID(0x11111112,0x00000103), &AeffectBack::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
								deviceUNKNOWN,
								deviceCAM,
								deviceVCR,
								deviceVCR_PAL,
								deviceVCR_NTSC
};

enum
{
								modeCAPTURE,
								modePRINT
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SAFE_RELEASE(pObject) if(pObject){ pObject->Release(); pObject = NULL;}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CHECK_ERROR(tsz,hr)                     \
{   if( S_OK != hr)                             \
        return false;                           \
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const CLSID CLSID_Dump2;

// {3D65EFFA-23FC-48dc-BD21-77BF5586367F}
DEFINE_GUID(CLSID_Dump2, 0x3d65effa, 0x23fc, 0x48dc, 0xbd, 0x21, 0x77, 0xbf, 0x55, 0x86, 0x36, 0x7f);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Icallback : public ISampleGrabberCB
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
		delete(this);
		return 0;
	}
public:
								Icallback					(Advcap *dvcap)	{ this->dvcap=dvcap; _ref_count=1; }
	Advcap						*dvcap;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HRESULT __stdcall Icallback::BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen)
{	
	Abuffer	*b=dvcap->smpbuf;
	if(b->write(pBuffer, BufferLen)!=BufferLen)
	{
		b->clear();
		b->write(pBuffer, BufferLen);
	}
	return 0;
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

void MyFreeMediaType(AM_MEDIA_TYPE& mt)
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

void MyDeleteMediaType(AM_MEDIA_TYPE *pmt)
{
    if (pmt != NULL)
    {
        MyFreeMediaType(*pmt); // See FreeMediaType for the implementation.
        CoTaskMemFree(pmt);
    }
}

HRESULT GetPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, IPin **ppPin)
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

HRESULT GetPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, IPin **ppPin, GUID mediatype)
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

static BOOL WINAPI IsEqualObject(IUnknown *pFirst, IUnknown *pSecond)
{
    if (pFirst == pSecond)
        return TRUE;

    LPUNKNOWN pUnknown1;     // Retrieve the IUnknown interface
    LPUNKNOWN pUnknown2;     // Retrieve the other IUnknown interface
    HRESULT hr;              // General OLE return code

    hr = pFirst->QueryInterface(IID_IUnknown,(void **) &pUnknown1);
    if(hr!=S_OK)
		return false;

    hr = pSecond->QueryInterface(IID_IUnknown,(void **) &pUnknown2);
    if(hr!=S_OK)
		return false;

    pUnknown1->Release();
    pUnknown2->Release();

    return (pUnknown1 == pUnknown2);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Advcap::Advcap(QIID qiid, char *name, AeffectInfo *info, Acapsule *capsule) : Aeffect(qiid, name, info, capsule)
{
	mode=modeCAPTURE;
    m_pGraph=NULL;
    m_pMediaControl=NULL;
    m_pMediaEvent=NULL;
    m_pDeviceFilter=NULL;
    pDVSplit=NULL;
    m_pGrabber=NULL;
    m_pGrabberSound=NULL;
    m_pNull=NULL;
    m_pNullSound=NULL;
	m_pIAMExtDevice=NULL;
	m_pIAMExtTransport=NULL;
	m_pIAMTCReader=NULL;
	device=deviceUNKNOWN;
	devicemode=deviceUNKNOWN;
    memset(m_DeviceName, 0, sizeof(m_DeviceName));
	smpbuf=new Abuffer("dvcap sound buffer", SMPSIZE*4);

	bOK=false;
	bDV=false;
	bGO=false;

	bOK=initDV();
	if(bOK)
	{
		bDV=enumDV();
		if(bDV)
		{
			bGO=startDV();
		}
	}

	image=new Abitmap(160, 120);

	front=new AdvcapFront(qiid, "dvcap front", this, 50);
	front->setTooltips("dv capture module");
	back=new AdvcapBack(qiid, "dvcap back", this, 50);
	back->setTooltips("dv capture module");

	if(!bOK)
		front->notify(this, nyERROR, (dword)"can't initialize capture");
	else if(!bDV)
		front->notify(this, nyERROR, (dword)"can't find DV device");
	else if(!bGO)
		front->notify(this, nyERROR, (dword)"can't start DV device");
}

Advcap::~Advcap()
{
	AdvcapFront	*df=(AdvcapFront *)front;
	scapture.enter(__FILE__,__LINE__);
	if(df->g_hDevNotify)
	{
		UnregisterDeviceNotification(df->g_hDevNotify);
		df->g_hDevNotify=NULL;
	}
	scapture.leave();
	closeDV();
	delete(image);
	delete(smpbuf);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Advcap::load(class Afile *f)
{
	return ((AdvcapFront *)front)->load(f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Advcap::save(class Afile *f)
{
	return ((AdvcapFront *)front)->save(f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Advcap::initDV()
{
	HRESULT			hr;
	AM_MEDIA_TYPE	mt;
	pBF_grabber=NULL;
	pBF_grabberSound=NULL;

	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,IID_IGraphBuilder,(void**)&m_pGraph);
	if(hr!=S_OK)
		return false;

    // Helps the building all Graphs
    hr = CoCreateInstance((REFCLSID)CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC, (REFIID)IID_ICaptureGraphBuilder2, (void **)&m_pCapture);
	if(hr!=S_OK)
		return false;

    hr = m_pCapture->SetFiltergraph(m_pGraph);
	if(hr!=S_OK)
		return false;

	//

    hr = m_pGraph->QueryInterface(IID_IMediaControl, (void **) & m_pMediaControl);
	if(hr!=S_OK)
		return false;

	// video grabber init //

	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, reinterpret_cast<void**>(&pBF_grabber));
	if(hr!=S_OK)
		return false;

	hr = pBF_grabber->QueryInterface(IID_ISampleGrabber, reinterpret_cast<void**>(&m_pGrabber));
	if(hr!=S_OK)
		return false;

	hr = m_pGraph->AddFilter(pBF_grabber, L"SampleGrabber");
	if(hr!=S_OK)
		return false;

/*
	{
		IBaseFilter	*src;
		hr = CoCreateInstance(CLSID_Dump2, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void **)(&src));
		if(hr!=S_OK)
		{
			switch(hr)
			{
				case REGDB_E_CLASSNOTREG:
				{
					int	a=5;
					a++;
				}
				break;
			}
			return false;
		}
	}
*/

	memset(&mt, 0, sizeof(mt));
	mt.majortype = MEDIATYPE_Video;
    mt.subtype = MEDIASUBTYPE_RGB32;

	hr = m_pGrabber->SetMediaType(&mt);
	if(hr!=S_OK)
		return false;

	m_pGrabber->SetBufferSamples(true);
	if(hr!=S_OK)
		return false;

	hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, reinterpret_cast<void**>(&m_pNull));
	if(hr!=S_OK)
		return false;

	hr = m_pGraph->AddFilter(m_pNull, L"NullRenderer");
	if(hr!=S_OK)
		return false;

	// sound grabber init //

	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, reinterpret_cast<void**>(&pBF_grabberSound));
	if(hr!=S_OK)
		return false;

	hr = pBF_grabberSound->QueryInterface(IID_ISampleGrabber, reinterpret_cast<void**>(&m_pGrabberSound));
	if(hr!=S_OK)
		return false;

	hr = m_pGraph->AddFilter(pBF_grabberSound, L"SampleGrabberSound");
	if(hr!=S_OK)
		return false;

	Icallback *m_pCallback=new Icallback(this);
	m_pGrabberSound->SetCallback(m_pCallback, 1);
	m_pCallback->Release();

	memset(&mt, 0, sizeof(mt));
	mt.majortype = MEDIATYPE_Audio;
    mt.subtype = MEDIASUBTYPE_PCM;

	hr = m_pGrabberSound->SetMediaType(&mt);
	if(hr!=S_OK)
		return false;

	m_pGrabberSound->SetBufferSamples(true);
	if(hr!=S_OK)
		return false;

	hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, reinterpret_cast<void**>(&m_pNullSound));
	if(hr!=S_OK)
		return false;

	hr = m_pGraph->AddFilter(m_pNullSound, L"NullRenderer Sound");
	if(hr!=S_OK)
		return false;


//	hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, reinterpret_cast<void**>(&m_pNull));
//	if(hr!=S_OK)
//		return false;

//	hr = m_pGraph->AddFilter(m_pNull, L"NullRenderer");
//	if(hr!=S_OK)
//		return false;

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Advcap::closeDV()
{
	scapture.enter(__FILE__,__LINE__);

	bOK=false;
	bDV=false;
	bGO=false;

	m_pMediaControl->Stop();

    SAFE_RELEASE(pBF_grabber);
    SAFE_RELEASE(pBF_grabberSound);
    SAFE_RELEASE(m_pMediaEvent);
	if(m_pGrabberSound)
		m_pGrabberSound->SetCallback(NULL, 1);
    SAFE_RELEASE(m_pGrabber);
    SAFE_RELEASE(m_pGrabberSound);
    SAFE_RELEASE(m_pMediaControl) ;
    SAFE_RELEASE(m_pDeviceFilter);
    SAFE_RELEASE(m_pIAMExtDevice);
    SAFE_RELEASE(m_pIAMExtTransport);
    SAFE_RELEASE(m_pIAMTCReader);
    //SAFE_RELEASE(pDVSplit);
    SAFE_RELEASE(m_pGraph); 

	device=deviceUNKNOWN;
	devicemode=deviceUNKNOWN;

	scapture.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Advcap::enumDV()
{
    HRESULT				hr = S_OK;
    ICreateDevEnum*     pCreateDevEnum = NULL;
    IEnumMoniker *      pEnumMoniker = NULL;
    IMoniker *          pMoniker = NULL;
    ULONG               nFetched = 0;

    // Create Device Enumerator
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, reinterpret_cast<PVOID *>(&pCreateDevEnum));
    if(hr!=S_OK)	return false;
		
    
    // Create the enumerator of the monikers for the specified Device Class & reset them 
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumMoniker, 0);
    if(hr!=S_OK)	return false;

    pEnumMoniker->Reset();

    // Loop through to the last moniker
    while((pEnumMoniker->Next( 1, &pMoniker, &nFetched )==S_OK) && pMoniker)    
    {
        IPropertyBag *pPropBag;
        hr = pMoniker->BindToStorage( 0, 0, IID_IPropertyBag, (void **)&pPropBag );
	    if(hr!=S_OK)	return false;

        //Friendly name
        VARIANT varFriendlyName;
        varFriendlyName.vt = VT_BSTR;
        hr = pPropBag->Read( L"FriendlyName", &varFriendlyName, 0 );
	    if(hr!=S_OK)	return false;
      
#ifdef UNICODE
        wcscpy(m_DeviceName, varFriendlyName.bstrVal);
#else
        WideCharToMultiByte( CP_ACP, 0, varFriendlyName.bstrVal, -1, m_DeviceName, sizeof(m_DeviceName), 0, 0 );    
#endif
        VariantClear( &varFriendlyName );
        
        if(!lstrcmp(TEXT("Microsoft DV Camera and VCR"), m_DeviceName))
        {

            hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&m_pDeviceFilter );
		    if(hr!=S_OK)	return false;

            hr = m_pGraph->AddFilter(m_pDeviceFilter, L"Filter");
		    if(hr!=S_OK)	return false;

            SAFE_RELEASE(pPropBag );
            SAFE_RELEASE(pMoniker);


            break;
        }
        else
            hr = E_FAIL;

        SAFE_RELEASE(pPropBag );
        SAFE_RELEASE(pMoniker);
    }//end of while

    SAFE_RELEASE( pEnumMoniker );
    SAFE_RELEASE( pCreateDevEnum );

	if(m_pDeviceFilter)
	{
		hr = m_pDeviceFilter->QueryInterface(IID_IAMExtTransport, (void **) &m_pIAMExtTransport);
		if(hr!=S_OK)	return false;
  
		hr = m_pDeviceFilter->QueryInterface(IID_IAMExtDevice, (void **) &m_pIAMExtDevice);
		if(hr!=S_OK)	return false;

		hr = m_pDeviceFilter->QueryInterface(IID_IAMTimecodeReader, (void **) &m_pIAMTCReader);
		if(hr!=S_OK)	return false;

		hr = m_pGraph->QueryInterface(IID_IMediaEventEx, reinterpret_cast<PVOID *>(&m_pMediaEvent)); 
		if(hr!=S_OK)	return false;

		hr = CoCreateInstance(CLSID_DVSplitter, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, reinterpret_cast<PVOID *>(&pDVSplit));
		if(hr!=S_OK)	return false;

		m_pGraph->AddFilter(pDVSplit, L"DV splitter");
	}

    return hr==S_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Advcap::startDV()
{
    IPin	*pOut = NULL, *pIn = NULL;
	bool	b=false;
    HRESULT hr;

    hr = GetPin(m_pDeviceFilter, PINDIR_OUTPUT, &pOut, MEDIATYPE_Interleaved);
    if (FAILED(hr)) return false;
    hr = GetPin(pDVSplit, PINDIR_INPUT, &pIn);
    if (FAILED(hr)) 
    {
        pOut->Release();
        return false;
     }
    hr = m_pGraph->Connect(pOut, pIn);
    pIn->Release();
    pOut->Release();
    if (FAILED(hr)) return false;


    hr = GetPin(pDVSplit, PINDIR_OUTPUT, &pOut, MEDIATYPE_Video);
    if (FAILED(hr)) return false;
    hr = GetPin(pBF_grabber, PINDIR_INPUT, &pIn);
    if (FAILED(hr)) 
    {
        pOut->Release();
        return false;
     }
    hr = m_pGraph->Connect(pOut, pIn);
    pIn->Release();
    pOut->Release();
    if (FAILED(hr)) return false;

	// video >> (grabber -> null) pin

    hr = GetPin(pBF_grabber, PINDIR_OUTPUT, &pOut);
    if (FAILED(hr)) return false;
    hr = GetPin(m_pNull, PINDIR_INPUT, &pIn);
    if (FAILED(hr)) 
    {
        pOut->Release();
        return false;
     }
    hr = m_pGraph->Connect(pOut, pIn);
    pIn->Release();
    pOut->Release();
    if (FAILED(hr)) return false;

	// audio >> (dv -> grabber) pin

	{
		IBaseFilter	*bf=pDVSplit;
		if(bf)
		{
		    hr = GetPin(bf, PINDIR_OUTPUT, &pOut, MEDIATYPE_Audio);
			if (FAILED(hr)) return false;
			hr = GetPin(pBF_grabberSound, PINDIR_INPUT, &pIn);
			if (FAILED(hr)) 
			{
				pOut->Release();
				return false;
			}
			hr = m_pGraph->Connect(pOut, pIn);
			pIn->Release();
			pOut->Release();
			if (FAILED(hr)) return false;
			bf->Release();
		}
	}

	// audio >> (grabber -> null) pin
/*
    hr = GetPin(pBF_grabberSound, PINDIR_OUTPUT, &pOut);
    if (FAILED(hr)) return false;
    hr = GetPin(m_pNullSound, PINDIR_INPUT, &pIn);
    if (FAILED(hr)) 
    {
        pOut->Release();
        return false;
     }
    hr = m_pGraph->Connect(pOut, pIn);
    pIn->Release();
    pOut->Release();
    if (FAILED(hr)) return false;
*/



	//hr = m_pGraph->Render(pOut);
	//if(hr!=S_OK)
	//	return false;
/*
	hr = ConnectFilters(m_pGraph, m_pDeviceFilter, pF);

	if(hr!=S_OK)
		return false;
	hr = ConnectFilters(m_pGraph, m_pNull, pF);
	if(hr!=S_OK)
		return false;

*/

/*
	//IPin	*pin;    
	//hr = GetPin(m_pDeviceFilter, PINDIR_OUTPUT, &pin);
    //if(hr==S_OK)
    {
		//
		hr=ConnectFilters(m_pGraph, m_pDeviceFilter, (IBaseFilter *)m_pGrabber);
		if(hr==S_OK)
			b=true;
		//pin->Release();
	}
*/
	//hr=m_pNull->Run(0);

	setDecodeSize(2);


	hr=m_pMediaControl->Run();
	//if(hr!=S_OK)
	//	return false;

	infoDV();

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Advcap::infoDV()
{
	HRESULT	hr;
    LONG    lDeviceType=0;

    assert(m_pDeviceFilter);

	device=deviceUNKNOWN;
	devicemode=deviceUNKNOWN;
    hr = m_pIAMExtDevice->GetCapability(ED_DEVCAP_DEVICE_TYPE, &lDeviceType, 0);
	if(hr==S_OK)
	{
		switch (lDeviceType)
		{
			case ED_DEVTYPE_VCR:
			device=deviceVCR;
			{
				LONG    lMediaType = 0;
			    LONG    lInSignalMode = 0;
				hr = m_pIAMExtTransport->GetStatus(ED_MEDIA_TYPE, &lMediaType);
				if(hr==S_OK)
				{
					if(ED_MEDIA_NOT_PRESENT != lMediaType)
					{
						assert(ED_MEDIA_DVC == lMediaType);

						hr = m_pIAMExtTransport->GetTransportBasicParameters(ED_TRANSBASIC_INPUT_SIGNAL, &lInSignalMode, NULL);
						if(hr==S_OK)
						{
							switch (lInSignalMode)
							{
								case ED_TRANSBASIC_SIGNAL_525_60_SDL :
								case ED_TRANSBASIC_SIGNAL_525_60_SD :
								//m_AvgTimePerFrame = 33;  // 33 milli-sec (29.97 FPS)
								devicemode=deviceVCR_NTSC;
								break;

								case ED_TRANSBASIC_SIGNAL_625_50_SDL :
								case ED_TRANSBASIC_SIGNAL_625_50_SD :
								//m_AvgTimePerFrame = 40;  // 40 milli-sec (25FPS)
								devicemode=deviceVCR_PAL;
								break;
							}
						}
					}
				}
			}
			break;

			case ED_DEVTYPE_CAMERA:
			device=deviceCAM;
			break;
		} 
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Advcap::stopDV()
{
	
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IIPDVDec * Advcap::getDVdec()
{
	HRESULT			hr;
	IIPDVDec		*pDVDec = NULL;
	IEnumFilters	*pFilters = NULL;
	IBaseFilter		*pFilter = NULL;
	ULONG			n;

	if(m_pGraph==NULL)
		return NULL;

	if(FAILED(m_pGraph->EnumFilters(&pFilters)))
		return NULL;
  
	while(pFilters->Next(1, &pFilter, &n) == S_OK)
	{
		hr = pFilter->QueryInterface(IID_IIPDVDec, (void**)&pDVDec);
		if(!FAILED(hr)) 
		{
			SAFE_RELEASE(pFilter);
			SAFE_RELEASE(pFilters);
			return pDVDec;
		}
		SAFE_RELEASE(pFilter);
	}
	SAFE_RELEASE(pFilters);
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Advcap::setDecodeSize(int size)
{
	IIPDVDec	*pDVDec = getDVdec();
	HRESULT		hr;
	int			res;


	if(pDVDec==NULL)
		return false;

	pDVDec->get_IPDisplay(&res);

	switch(size)
	{
		case 1:
		hr = pDVDec->put_IPDisplay(DVRESOLUTION_FULL);
		break;

		case 2:
		hr = pDVDec->put_IPDisplay(DVRESOLUTION_HALF);
		break;

		case 3:
		hr = pDVDec->put_IPDisplay(DVRESOLUTION_QUARTER);
		break;

		case 4:
		hr = pDVDec->put_IPDisplay(DVRESOLUTION_DC);
		break;

		default:
		hr=-1;
		break;
	}
	SAFE_RELEASE(pDVDec);
	return !FAILED(hr);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IBaseFilter * Advcap::getDecomp()
{
	IEnumFilters	*filters;
	IBaseFilter		*filter;
	HRESULT			hr;
	char			*name;

	m_pGraph->EnumFilters(&filters);
	while(filters->Next(1, &filter, 0)==S_OK)
	{
		FILTER_INFO	fi;

		hr=filter->QueryFilterInfo(&fi);
		if (FAILED(hr))
		{
			SAFE_RELEASE(filter);
			continue;
		}

		UnicodeToAnsi(fi.achName, &name);
		if(strstr(name, "Decoder")) // search filter Decompressor
		{
			free(name);
			SAFE_RELEASE(filters);
			return filter;
		}

		free(name);
		SAFE_RELEASE(filter);
	}
	SAFE_RELEASE(filters);
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


bool Advcap::NukeFilters(IBaseFilter *pFilter, BOOL bNukeDownStream)
{
    HRESULT hr = S_OK;
    IPin *pPin = NULL, *pToPin = NULL;
    IEnumPins *pEnumPins = NULL;

    ULONG uFetched = 0;
    PIN_INFO PinInfo;
    LONG lDelayCount = 100;

    assert(m_pGraph);
    
    // Validating the the pointer to the Filter is not null
    if(!pFilter)
    {
		assert(false);
        return false;
    }

    // enumerate all the pins on this filter
    // reset the enumerator to the first pin
    hr = pFilter->EnumPins(&pEnumPins);
    CHECK_ERROR( TEXT("CAVCGraph::NukeFilters():: Could not enumerate pins on the filter to be nuked fro."), hr);
    pEnumPins->Reset(); 
   
    // Loop through all the pins of the filter
    while( SUCCEEDED(pEnumPins->Next(1, &pPin, &uFetched)) && pPin )
    {
        // Get the pin & its pin_info struct that this filter's pin is connected to 
        hr = pPin->ConnectedTo(&pToPin);
        if(SUCCEEDED(hr) &&pToPin )
        {
            hr = pToPin->QueryPinInfo(&PinInfo);
            CHECK_ERROR( TEXT("pToPin->QueryPinInfo failed."), hr);
                   
            // Check that this ConnectedTo Pin is a input pin thus validating that our filter's pin is an output pin
            if(PinInfo.dir == PINDIR_INPUT && bNukeDownStream)
            {
                // thus we have a pin on the downstream filter so nuke everything downstream of that filter recursively
                NukeFilters(PinInfo.pFilter, bNukeDownStream);
                // Disconnect the two pins and remove the downstream filter
                m_pGraph->Disconnect(pToPin);
                m_pGraph->Disconnect(pPin);
                //always leave the Camera filter in the graph
                if (PinInfo.pFilter != m_pDeviceFilter)
                {
                    if(FAILED(m_pGraph->RemoveFilter(PinInfo.pFilter)))
                    {
                        //Dump(TEXT("CAVCGraph::NukeFilters():: The Filter cannot be removed from the filtergraph"));
						assert(false);
                    }
                }

            }

            SAFE_RELEASE(PinInfo.pFilter);                 
            SAFE_RELEASE(pToPin);
            //Dump(TEXT("CAVCGraph::NukeFilters():: release ToPin\n"));
        }
        SAFE_RELEASE(pPin);       
    }

    SAFE_RELEASE(pEnumPins);
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Advcap::action(double time, double dtime, double beat, double dbeat)
{
	int				wo=getVideoWidth();
	int				ho=getVideoHeight();
	Avideo			*out=((AdvcapBack *)back)->out;
	Asample			*sout=((AdvcapBack *)back)->sout;
	bool			bvideo=false;

	scapture.enter(__FILE__,__LINE__);
	if(bOK&&bDV&&bGO)
	{
		HRESULT	hr;
		AM_MEDIA_TYPE MediaType;
		memset(&MediaType, 0, sizeof(MediaType));

		hr = m_pGrabber->GetConnectedMediaType(&MediaType); 
		if((hr==S_OK)&&MediaType.pbFormat)
		{
			VIDEOINFOHEADER *pvh=(VIDEOINFOHEADER*)MediaType.pbFormat; 
			int				w=pvh->bmiHeader.biWidth;
			int				h=pvh->bmiHeader.biHeight;
			long			size=0; 

			if((w!=image->w)||(h!=image->h))
				image->size(w, h);
			
			size=image->bodysize;
			hr=m_pGrabber->GetCurrentBuffer(&size, (long *)image->body);

			if(hr==S_OK)
			{
				image->flipY();
				out->enter(__FILE__,__LINE__);
				{
					Abitmap	*b=out->getBitmap();
					if(b)
						b->set(0, 0, b->w, b->h, 3, 3, image->w-6, image->h-6, image, bitmapNORMAL, bitmapNORMAL);
				}
				out->leave();
				bvideo=true;
			}
			MyFreeMediaType(MediaType);	
		}

		hr = m_pGrabberSound->GetConnectedMediaType(&MediaType); 
		if((hr==S_OK)&&MediaType.pbFormat)
		{
			WAVEFORMATEX	*wf=(WAVEFORMATEX *)MediaType.pbFormat;
			long			size=0;
			sword			sample[16384*2];
			size=mini((int)smpbuf->getSize(), 16384*4);
			smpbuf->read(sample, size);
			{
				sword	smp[65536*2];
				int		nsample=size>>2;
				if(wf->nSamplesPerSec==44100)
				{
					sout->enter(__FILE__,__LINE__);
					sout->addBuffer(sample, nsample);
					sout->leave();
				}
				else if(wf->nSamplesPerSec<44100)
				{
					int	dx=((int)wf->nSamplesPerSec<<16)/44100;
					int	nsmp=nsample*44100/(int)wf->nSamplesPerSec;
					int	x=0;
					int	i;
					for(i=0; i<nsmp; i++)
					{
						int	n=i<<1;
						int	xn=(x>>16)<<1;
						smp[n]=sample[xn];
						smp[n+1]=sample[xn+1];
						x+=dx;
					}
					sout->enter(__FILE__,__LINE__);
					sout->addBuffer(smp, nsmp);
					sout->leave();
				}
				else	// >
				{
				}
			}
			MyFreeMediaType(MediaType);	
		}
	}
	scapture.leave();

	if(!bvideo)
	{
		out->enter(__FILE__,__LINE__);
		{
			Abitmap	*b=out->getBitmap();
			if(b)
				b->boxf(0, 0, b->w, b->h, 0xff000000);
		}
		out->leave();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Advcap::settings(bool emergency)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AdvcapFront::system(int event, dword d0, dword d1)
{
	switch(event)
	{
		case systemDEVICECHANGE:
		switch(d0)
		{
			case DBT_DEVICEREMOVECOMPLETE:
			case DBT_DEVICEREMOVEPENDING:
			{
				PDEV_BROADCAST_HDR pdbh = (PDEV_BROADCAST_HDR) d1;
				if (pdbh->dbch_devicetype != DBT_DEVTYP_DEVICEINTERFACE)
					break;

				{
					PDEV_BROADCAST_DEVICEINTERFACE pdbi = (PDEV_BROADCAST_DEVICEINTERFACE) d1;
					if (pdbi->dbcc_classguid != AM_KSCATEGORY_CAPTURE)
						break;

					{
						Advcap	*e=(Advcap *)effect;
						e->scapture.enter(__FILE__,__LINE__);
						e->closeDV();
						e->scapture.leave();
					}
				}
				notify(this, nyALERT, (dword)"DV device removed");
			}
			break;

			case DBT_DEVICEARRIVAL:
			{
				PDEV_BROADCAST_HDR pdbh = (PDEV_BROADCAST_HDR) d1;
				if (pdbh->dbch_devicetype != DBT_DEVTYP_DEVICEINTERFACE)
					break;

				{
					PDEV_BROADCAST_DEVICEINTERFACE pdbi = (PDEV_BROADCAST_DEVICEINTERFACE) d1;
					if (pdbi->dbcc_classguid != AM_KSCATEGORY_CAPTURE)
						break;

					{
						Advcap	*e=(Advcap *)effect;

						e->scapture.enter(__FILE__,__LINE__);
						if(e->bDV)
							e->closeDV();

						e->bOK=e->initDV();
						if(e->bOK)
						{
							e->bDV=e->enumDV();
							if(e->bDV)
								e->bGO=e->startDV();
						}
						e->scapture.leave();
						if(!e->bGO)
							notify(this, nyERROR, (dword)"can't start new DV device");
						else
							notify(this, nyALERT, (dword)"new DV device detected");
					}
				}
			}
			break;
		}
	}
	return true;
}

bool AdvcapFront::load(class Afile *f)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AdvcapFront::save(class Afile *f)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AdvcapFront::AdvcapFront(QIID qiid, char *name, Advcap *e, int h) : AeffectFront(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_DVCAP), "PNG");
	back=new Abitmap(&o);
	{
		DEV_BROADCAST_DEVICEINTERFACE filterData;
		memset(&filterData, 0, sizeof(filterData));
		filterData.dbcc_size = sizeof(filterData);
		filterData.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
		filterData.dbcc_classguid = AM_KSCATEGORY_CAPTURE;
		g_hDevNotify=RegisterDeviceNotification(getWindow()->hw, &filterData, DEVICE_NOTIFY_WINDOW_HANDLE);
		assert(g_hDevNotify != NULL);
	}
	buttonStop=new Abutton("stop", this, 290, (pos.h-16)>>1, 16, 16, &resource.get(MAKEINTRESOURCE(PNG_BUTTONSTOP), "PNG"));
	buttonStop->setTooltips("stop");
	buttonStop->show(TRUE);
	buttonRew=new Abutton("rewind", this, 310, (pos.h-16)>>1, 16, 16, &resource.get(MAKEINTRESOURCE(PNG_BUTTONREWIND), "PNG"));
	buttonRew->setTooltips("rewind");
	buttonRew->show(TRUE);
	buttonRec=new Abutton("record", this, 330, (pos.h-16)>>1, 16, 16, &resource.get(MAKEINTRESOURCE(PNG_BUTTONREC), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	buttonRec->setTooltips("record");
	buttonRec->show(TRUE);
	buttonPlay=new Abutton("play", this, 350, (pos.h-16)>>1, 16, 16, &resource.get(MAKEINTRESOURCE(PNG_BUTTONPLAY), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	buttonPlay->setTooltips("play");
	buttonPlay->show(TRUE);
	buttonFew=new Abutton("few", this, 370, (pos.h-16)>>1, 16, 16, &resource.get(MAKEINTRESOURCE(PNG_BUTTONFWRD), "PNG"));
	buttonFew->setTooltips("few");
	buttonFew->show(TRUE);
	tm1=new Astatic("time code", this, 400, (pos.h-20)>>1, 25, 20);
	tm1->setTooltips("time code");
	tm1->show(true);
	tm2=new Astatic("time code", this, 425, (pos.h-20)>>1, 25, 20);
	tm2->setTooltips("time code");
	tm2->show(true);
	tm3=new Astatic("time code", this, 450, (pos.h-20)>>1, 25, 20);
	tm3->setTooltips("time code");
	tm3->show(true);
	tm4=new Astatic("time code", this, 475, (pos.h-20)>>1, 25, 20);
	tm4->setTooltips("time code");
	tm4->show(true);
	/*
	bMode=new Abutton("mode", this, 8, 8, 100, 16, "DV CAPTURE");
	bMode->setTooltips("select DV mode");
	bMode->show(TRUE);
	*/
	timer(100);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AdvcapFront::~AdvcapFront()
{
	//delete(bMode);
	delete(tm1);
	delete(tm2);
	delete(tm3);
	delete(tm4);
	delete(buttonFew);
	delete(buttonPlay);
	delete(buttonRec);
	delete(buttonRew);
	delete(buttonStop);
 	delete(back);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AdvcapFront::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AdvcapFront::pulse()
{
	dword	i1=0,i2=0,i3=0,i4=0;
	Advcap	*e=(Advcap *)effect;
	Afont	*f=alib.getFont(fontSEGMENT10);
	dword	color=0xff202020;
	char	str[128];
	int		device=deviceUNKNOWN;

	e->scapture.enter(__FILE__,__LINE__);
	if(e->device!=deviceUNKNOWN)
	{
	    TIMECODE_SAMPLE TimecodeSample;

	    TimecodeSample.dwFlags = ED_DEVCAP_TIMECODE_READ;
		TimecodeSample.timecode.dwFrames = 0;

		device=e->device;

		if(e->m_pIAMTCReader->GetTimecode(&TimecodeSample)==S_OK)
		{
			i1 = (TimecodeSample.timecode.dwFrames & 0xff000000) >> 24;
			i2 = (TimecodeSample.timecode.dwFrames & 0x00ff0000) >> 16;
			i3 = (TimecodeSample.timecode.dwFrames & 0x0000ff00) >>  8;
			i4 = (TimecodeSample.timecode.dwFrames & 0x000000ff);
			if((i1>99)||(i2>99)||(i3>99)||(i4>99))
			{
				i1=0;
				i2=0;
				i3=0;
				i4=0;
			}
		}
	}
	e->scapture.leave();

	if(i1==0x3f)
		i1=i2=i3=i4=0;

	if(mt1!=i1)
	{
		mt1=i1;
		sprintf(str, "%.2x", i1);
		tm1->set(str , color, f, Astatic::CENTER);
	}
	if(mt2!=i2)
	{
		mt2=i2;
		sprintf(str, "%.2x", i2);
		tm2->set(str , color, f, Astatic::CENTER);
	}
	if(mt3!=i3)
	{
		mt3=i3;
		sprintf(str, "%.2x", i3);
		tm3->set(str , color, f, Astatic::CENTER);
	}
	if(mt4!=i4)
	{
		mt4=i4;
		sprintf(str, "%.2x", i4);
		tm4->set(str , color, f, Astatic::CENTER);
	}

	if(device!=mdevice)
	{
		if(device==deviceVCR)
		{
			buttonPlay->state|=stateENABLE;
			buttonRec->state|=stateENABLE;
			buttonRew->state|=stateENABLE;
			buttonFew->state|=stateENABLE;
			buttonStop->state|=stateENABLE;
		}
		else
		{
			buttonPlay->state&=~stateENABLE;
			buttonRec->state&=~stateENABLE;
			buttonRew->state&=~stateENABLE;
			buttonFew->state&=~stateENABLE;
			buttonStop->state&=~stateENABLE;
		}
		mdevice=device;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AdvcapFront::notify(Anode *o, int event, dword p)
{
	Advcap	*e=(Advcap *)effect;
	switch(event)
	{
		case nyCHANGE:
		e->scapture.enter(__FILE__,__LINE__);
		if(e->device==deviceVCR)
		{
			bool	p=buttonPlay->isChecked();
			bool	r=buttonRec->isChecked();
			if(o==buttonPlay)
			{
				if(p)
					e->m_pIAMExtTransport->put_Mode(ED_MODE_PLAY);
				else
					e->m_pIAMExtTransport->put_Mode(ED_MODE_STOP);
			}
			else if(o==buttonRec)
			{
				if(p)
					buttonPlay->setChecked(false);				
				if(r)
					e->m_pIAMExtTransport->put_Mode(ED_MODE_RECORD);
				else
					e->m_pIAMExtTransport->put_Mode(ED_MODE_STOP);
			}
		}
		e->scapture.leave();
		return true;

		case nyCLICK:
		e->scapture.enter(__FILE__,__LINE__);
		if(e->device==deviceVCR)
		{
			bool	p=buttonPlay->isChecked();
			bool	r=buttonRec->isChecked();
			if(o==buttonRew)
			{
				if(p)
					e->m_pIAMExtTransport->put_Mode(ED_MODE_PLAY_FASTEST_REV);
				else
					e->m_pIAMExtTransport->put_Mode(ED_MODE_REW);
			}
			else if(o==buttonFew)
			{
				if(p)
					e->m_pIAMExtTransport->put_Mode(ED_MODE_PLAY_FASTEST_FWD);
				else
					e->m_pIAMExtTransport->put_Mode(ED_MODE_FF);
			}
			else if(o==buttonStop)
			{
				if(p)
					buttonPlay->setChecked(false);
				if(r)
					buttonRec->setChecked(false);
				e->m_pIAMExtTransport->put_Mode(ED_MODE_STOP);
			}
		}
		/*
		if(o==bMode)
		{
			e->mode=(e->mode==modeCAPTURE)?modePRINT:modeCAPTURE;
			switch(e->mode)
			{
				case modeCAPTURE:
				bMode->setCaption("DV CAPTURE");
				break;

				case modePRINT:
				bMode->setCaption("DV PRINT");
				break;

				default:
				assert(false);
				break;
			}
		}
		*/
		e->scapture.leave();
		return true;
	}
	return AeffectFront::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AdvcapBack::AdvcapBack(QIID qiid, char *name, Advcap *e, int h) : AeffectBack(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_DVCAP2), "PNG");
	back=new Abitmap(&o);

	out=new Avideo(MKQIID(qiid, 0xeb6d251dc92bbc80),"video out", this, pinOUT, pos.w-18, 10);
	out->setTooltips("video out");
	out->show(TRUE);

	sout=new Asample(MKQIID(qiid, 0x15c5980e6cbbd970), "audio out", this, pinOUT, pos.w-38, 10);
	sout->setTooltips("audio out");
	sout->show(TRUE);
/*
	in=new Avideo("video in", this, pinIN, 10, 10);
	in->setTooltips("video in");
	in->show(TRUE);

	sin=new Asample("audio in", this, pinIN, 30, 10);
	sin->setTooltips("audio in");
	sin->show(TRUE);
*/
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AdvcapBack::~AdvcapBack()
{
	delete(back);
	delete(out);
	delete(sout);
	//delete(in);
	//delete(sin);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AdvcapBack::paint(Abitmap *b)
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

Aeffect * AdvcapInfo::create(QIID qiid, char *name, Acapsule *capsule)
{
	return new Advcap(qiid, name, this, capsule);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aplugz * dvcapGetInfo()
{
	return new AdvcapInfo("dvcapInfo", &Advcap::CI, "dv capture", "dv capture module");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
