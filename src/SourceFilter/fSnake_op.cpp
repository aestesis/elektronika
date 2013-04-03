//------------------------------------------------------------------------------
// File: fSnake_op.cpp
//
// Desc: implement CSnakeStream class
// 
// Author : Ashok Jaiswal
//
// Data/Time : September 2004
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "fSnake.h"
#include "fSnake_op.h"
#include "stdio.h"
#include <tchar.h>
#include <varargs.h>
#include "sharedmem.h"
#include "r.h"

// #define _DEBUGSNAKE

#ifdef _DEBUGSNAKE

/*
__inline void dbgprint(char *str)
{
	FILE	*f=fopen("c:\\eleksource.txt", "a");
	if(f)
	{
		fprintf(f, "%s\r\n", str);
		fclose(f);
	}
}
*/
__inline void dbgprint(char *str, ...)
{
    va_list ap;
	{
		FILE	*f=fopen("c:\\eleksource.txt", "a");
		if(f)
		{
			va_start(ap, str);
			fprintf(f, str, ap);
			va_end(ap);
			fprintf(f, "\r\n");
			fclose(f);
		}
	}
}

#else
__inline void dbgprint(char *str, ...)
{
}
#endif


static double getRealTime()	// in second
{
	LARGE_INTEGER	freq;
	LARGE_INTEGER	time;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&time);
	return (double)time.QuadPart/(double)freq.QuadPart;
}


// Set: Cannot set any properties.
HRESULT CSnakeStream::Set(REFGUID guidPropSet, DWORD dwID,
	void *pInstanceData, DWORD cbInstanceData, void *pPropData,
	DWORD cbPropData)
{
	dbgprint("[IN] >> CSnakeStream::Set");
	return E_NOTIMPL;
}

// Get: Return the pin category (our only property).
HRESULT CSnakeStream::Get(	REFGUID guidPropSet, // Which property set.
							DWORD dwPropID, // Which property in that set.
							void *pInstanceData, // Instance data (ignore).
							DWORD cbInstanceData, // Size of the instance data (ignore).
							void *pPropData, // Buffer to receive the property data.
							DWORD cbPropData, // Size of the buffer.
							DWORD *pcbReturned // Return the size of the property.
							)
{
	dbgprint("[IN] >> CSnakeStream::Get | dwPropID=%d", dwPropID);
	if (guidPropSet != AMPROPSETID_Pin)
		return E_PROP_SET_UNSUPPORTED;
	if (dwPropID != AMPROPERTY_PIN_CATEGORY)
		return E_PROP_ID_UNSUPPORTED;
	if (pPropData == NULL && pcbReturned == NULL)
		return E_POINTER;
	if (pcbReturned)
		*pcbReturned = sizeof(GUID);
	if (pPropData == NULL) // Caller just wants to know the size.
		return S_OK;
	if (cbPropData < sizeof(GUID)) // The buffer is too small.
			return E_UNEXPECTED;
	switch(pinType)
	{
		case pinCAPTURE:
		*(GUID *)pPropData = PIN_CATEGORY_CAPTURE;
		break;

		default:
		case pinPREVIEW:
		*(GUID *)pPropData = PIN_CATEGORY_CAPTURE;
		break;
	}
	return S_OK;
}

// QuerySupported: Query whether the pin supports the specified property.
HRESULT CSnakeStream::QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport)
{
	dbgprint("[IN] >> CSnakeStream::QuerySupported | dwPropID=%d", dwPropID);
	if (guidPropSet != AMPROPSETID_Pin)
		return E_PROP_SET_UNSUPPORTED;
	if (dwPropID != AMPROPERTY_PIN_CATEGORY)
		return E_PROP_ID_UNSUPPORTED;
	if (pTypeSupport)
	// We support getting this property, but not setting it.
		*pTypeSupport = KSPROPERTY_SUPPORT_GET;
	return S_OK;
}

STDMETHODIMP CSnakeStream::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
	dbgprint("[IN] >> CSnakeStream::NonDelegatingQueryInterface");
    if(riid == IID_IKsPropertySet) 
	{
        return GetInterface(static_cast<IKsPropertySet*>(this), ppv);
	} 	
	else if(riid == IID_IAMStreamConfig) 
	{
        return GetInterface(static_cast<IAMStreamConfig*>(this), ppv);
    }
	else if(riid == IID_IAMDroppedFrames) 
	{
        return GetInterface(static_cast<IAMDroppedFrames*>(this), ppv);
    }
	else
	{
        return CSourceStream::NonDelegatingQueryInterface(riid, ppv);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

//
// Constructor
//
CSnakeStream::CSnakeStream(HRESULT *phr, CSnakeFilter *pParent, LPCWSTR pPinName, int pinType)
	: CSourceStream(NAME("CElektronikaStream"), phr, pParent, pPinName)
	, mem(NULL)

{
    CAutoLock cAutoLock(m_pFilter->pStateLock());
	dbgprint("[IN] >> CSnakeStream::CSnakeStream 'constructor'");
	m_frame=0;
	dropped=0;
	notdropped=false;
	this->pinType=pinType;
	AvgTimePerFrame=(REFERENCE_TIME)(10000000.0/25.00);
}


//
// Destructor
//
CSnakeStream::~CSnakeStream()
{
	dbgprint("[IN] >> CSnakeStream::~CSnakeStream  'destructor'");
}


//
// GetMediaType
//
HRESULT CSnakeStream::GetMediaType(CMediaType *pMediaType)
{
	CAutoLock lock(m_pFilter->pStateLock());
	dbgprint("[IN] >> CSnakeStream::GetMediaType");
	if(pMediaType==NULL)
		return E_POINTER;
	ZeroMemory(pMediaType, sizeof(CMediaType));
	{
		VIDEOINFO *pvi = (VIDEOINFO *)pMediaType->AllocFormatBuffer(sizeof(VIDEOINFO));
		if (NULL == pvi)
			return E_OUTOFMEMORY;

		ZeroMemory(pvi, sizeof(VIDEOINFO));

		pvi->AvgTimePerFrame=AvgTimePerFrame;

		pvi->bmiHeader.biCompression	= BI_RGB;
		pvi->bmiHeader.biBitCount		= 24;
		pvi->bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
		pvi->bmiHeader.biWidth			= 320;
		pvi->bmiHeader.biHeight			= 240;
		pvi->bmiHeader.biPlanes			= 1;
		pvi->bmiHeader.biSizeImage		= GetBitmapSize(&pvi->bmiHeader);
		pvi->bmiHeader.biClrImportant	= 0;

		SetRectEmpty(&(pvi->rcSource));	// we want the whole image area rendered.
		SetRectEmpty(&(pvi->rcTarget));	// no particular destination rectangle

		pMediaType->SetType(&MEDIATYPE_Video);
		pMediaType->SetFormatType(&FORMAT_VideoInfo);
		pMediaType->SetTemporalCompression(FALSE);

		const GUID SubTypeGUID = GetBitmapSubtype(&pvi->bmiHeader);
		pMediaType->SetSubtype(&SubTypeGUID);
		pMediaType->SetSampleSize(pvi->bmiHeader.biSizeImage);
	}
	dbgprint("[OK] << CSnakeStream::GetMediaType");
	return S_OK;
}


//
// DecideBufferSize
//
HRESULT CSnakeStream::DecideBufferSize(IMemAllocator *pMemAlloc, ALLOCATOR_PROPERTIES *pProperties)
{
	CAutoLock cAutoLock(m_pFilter->pStateLock());
	ASSERT(pMemAlloc);
	ASSERT(pProperties);

	dbgprint("[IN] >> CSnakeStream::DecideBufferSize");

	HRESULT hr;

	// TODO: set the properties
	{
		VIDEOINFO *pvi = (VIDEOINFO *)m_mt.Format();
		pProperties->cBuffers = 3;
		pProperties->cbBuffer = pvi->bmiHeader.biSizeImage;
	}

	// Ask the allocator to reserve us some sample memory, NOTE the function
	// can succeed (that is return NOERROR) but still not have allocated the
	// memory that we requested, so we must check we got whatever we wanted

	ALLOCATOR_PROPERTIES Actual;
	hr = pMemAlloc->SetProperties(pProperties, &Actual);
	if (FAILED(hr)) 
		return hr;

	// Is this allocator unsuitable
	if (Actual.cbBuffer < pProperties->cbBuffer) 
		return E_FAIL;

	return S_OK;
}


//
// FillBuffer
//
HRESULT CSnakeStream::FillBuffer(IMediaSample *pSample)
{
	CAutoLock	lock(m_pFilter->pStateLock());
	BYTE		*pBuffer;
	long		lSize;

	dbgprint("[IN] >> CSnakeStream::FillBuffer");

	if(mem->data)
	{
		if(!mem->data->locked)
		{
			mem->data->locked=true;
			if(m_frame!=mem->data->frame)
			{
				int	n=mem->data->frame-m_frame;
				{
					int y;
					BYTE *d=(BYTE*)image;
					for(y=0; y<240; y++)
					{
						BYTE *s=((BYTE*)mem->data->body)+(239-y)*(320*3);
						CopyMemory(d, s, 320*3);
						d+=320*3;
					}
				}
				//CopyMemory(image, mem->data->body, SHAREDMEMBODYSIZE);
				m_frame=mem->data->frame;
				notdropped++;
				dropped+=(n>1)?(n-1):0;
			}
			mem->data->locked=false;
		}
	}

	if(SUCCEEDED(pSample->GetPointer(&pBuffer)))
	{
		lSize = pSample->GetSize();
		if(lSize==SHAREDMEMBODYSIZE)
			CopyMemory(pBuffer, image, SHAREDMEMBODYSIZE);	
	}

	{
		REFERENCE_TIME	time0=currenttime;
		REFERENCE_TIME	time1=(REFERENCE_TIME)(10000000.0*(getRealTime()-starttime));
		pSample->SetTime(&time0, &time1);
		//pSample->SetMediaTime(&time0, &time1);
		pSample->SetDiscontinuity(false);
		pSample->SetPreroll(false);
		pSample->SetSyncPoint(true);
		currenttime=time1;
	}

	return S_OK;
}


//
// OnThreadCreate
//
HRESULT CSnakeStream::OnThreadCreate(void)
{
	dbgprint("[IN] >> CSnakeStream::OnThreadCreate");

	mem=new Asharedmem();
	currenttime=0;
	starttime=getRealTime();
	dropped=0;
	notdropped=0;
	if(mem->thefirst)
	{
		if(mem->data)
		{
			HBITMAP	hb=LoadBitmap(GetModuleHandle("elektronika.ax"), MAKEINTRESOURCE(IDB_BITMAPAESTESIS));
			if(hb)
			{
				int		size=320*240*4;
				BYTE	*rgb=(BYTE*)new BYTE[size];
				int		x,y;
				BYTE	*d=&mem->data->body[0];
				BYTE	*s=rgb;

				if((rgb!=NULL) && GetBitmapBits(hb, size, rgb))
				{
					for(y=0; y<240; y++)
					{
						//BYTE	*s=rgb+(239-y)*(320*4);
						for(x=0; x<320; x++)
						{
							BYTE	r=*(s++);
							BYTE	g=*(s++);
							BYTE	b=*(s++);
							s++;
							*(d++)=r;
							*(d++)=g;
							*(d++)=b;
						}
					}
					delete(rgb);
				}

				DeleteObject(hb);
			}
		}
	}
	return CSourceStream::OnThreadCreate();
}


//
// OnThreadDestroy
//
HRESULT CSnakeStream::OnThreadDestroy(void)
{
	dbgprint("[IN] >> CSnakeStream::OnThreadDestroy");
	HRESULT r=CSourceStream::OnThreadDestroy();
	if(mem)
		delete(mem);
	return r;
}


//
// OnThreadStartPlay
//
HRESULT CSnakeStream::OnThreadStartPlay(void)
{
	dbgprint("[IN] >> CSnakeStream::OnThreadPlay");
	return CSourceStream::OnThreadStartPlay();
}



	// IAMStreamConfig /////////////////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE CSnakeStream::SetFormat( 
            /* [in] */ AM_MEDIA_TYPE *pmt)
{
	CAutoLock lock(m_pFilter->pStateLock());

	dbgprint("[IN] >> CSnakeStream::SetFormat");

	if(!pmt)
		return E_POINTER;
	if((pmt->majortype==MEDIATYPE_Video)&&(pmt->subtype==MEDIASUBTYPE_RGB24))
	{
		if(pmt->formattype==FORMAT_VideoInfo)
		{
			VIDEOINFO *pvi = (VIDEOINFO *)pmt->pbFormat;
			AvgTimePerFrame=pvi->AvgTimePerFrame;
			return S_OK;
		}
	}
	return S_FALSE;
}
        
HRESULT STDMETHODCALLTYPE CSnakeStream::GetFormat( 
            /* [out] */ AM_MEDIA_TYPE **ppmt)
{
	CAutoLock lock(m_pFilter->pStateLock());

	dbgprint("[IN] >> CSnakeStream::GetFormat");

	if(!ppmt)
		return E_POINTER;
	*ppmt=NULL;

	{
		CMediaType	*pMediaType=new CMediaType();
		if(GetMediaType(pMediaType)!=S_OK)
		{
			delete(pMediaType);
			return E_OUTOFMEMORY;
		}
		*ppmt=pMediaType;
		dbgprint("[OK] << CSnakeStream::GetFormat");
		return S_OK;
	}
	return S_FALSE;
}
        
HRESULT STDMETHODCALLTYPE CSnakeStream::GetNumberOfCapabilities( 
            /* [out] */ int *piCount,
            /* [out] */ int *piSize)
{
	CAutoLock lock(m_pFilter->pStateLock());

	dbgprint("[IN] >> CSnakeStream::GetNumberOfCapabilities");

	if(!(piCount&&piSize))
		return E_POINTER;
	*piCount=1;
	*piSize=sizeof(VIDEO_STREAM_CONFIG_CAPS);

	return S_OK;
}

        
HRESULT STDMETHODCALLTYPE CSnakeStream::GetStreamCaps( 
            /* [in] */ int iIndex,
            /* [out] */ AM_MEDIA_TYPE **ppmt,
            /* [out] */ BYTE *pSCC)
{
	CAutoLock lock(m_pFilter->pStateLock());

	dbgprint("[IN] >> CSnakeStream::GetStreamCaps");

	if(iIndex==0)
	{
		HRESULT	hr;

		if(!(ppmt&&pSCC))
			return E_POINTER;
		
		if((hr=GetFormat(ppmt))!=S_OK)
			return hr;

		{
			VIDEO_STREAM_CONFIG_CAPS	*scc=(VIDEO_STREAM_CONFIG_CAPS *)pSCC;
			ZeroMemory(scc, sizeof(VIDEO_STREAM_CONFIG_CAPS));
			scc->guid=FORMAT_VideoInfo;
			scc->VideoStandard=AnalogVideo_None;
			scc->InputSize.cx=320;
			scc->InputSize.cy=240;
			scc->MinCroppingSize.cx=320;
			scc->MaxCroppingSize.cy=240;
			scc->MaxCroppingSize.cx=320;
			scc->MaxCroppingSize.cy=240;
			scc->CropGranularityX=1;
			scc->CropGranularityY=1;
			scc->CropAlignX=1;
			scc->CropAlignY=1;
			scc->MinOutputSize.cx=320;
			scc->MinOutputSize.cy=240;
			scc->MaxOutputSize.cx=320;
			scc->MaxOutputSize.cy=240;
			scc->OutputGranularityX=1;
			scc->OutputGranularityY=1;
			scc->StretchTapsX=0;
			scc->StretchTapsY=0;
			scc->ShrinkTapsX=0;
			scc->ShrinkTapsY=0;
		}

		return S_OK;
	}
	else
		return E_INVALIDARG;
	return S_FALSE;
}

	// IQualityControl /////////////////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE CSnakeStream::Notify( 
            /* [in] */ IBaseFilter *pSelf,
            /* [in] */ Quality q)
{

	dbgprint("[IN] >> CSnakeStream::Notify | type %d", q.Type);

	if(!pSelf)
		return E_POINTER;

	//

	return S_OK;
}

	// IAMDroppedFrames /////////////////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE CSnakeStream::GetNumDropped( 
            /* [out] */ long *plDropped)
{
	if(!plDropped)
		return E_POINTER;
	*plDropped=dropped;
	return S_OK;
}
        
HRESULT STDMETHODCALLTYPE CSnakeStream::GetNumNotDropped( 
            /* [out] */ long *plNotDropped)
{
	if(!plNotDropped)
		return E_POINTER;
	*plNotDropped=notdropped;
	return S_OK;
}
        
HRESULT STDMETHODCALLTYPE CSnakeStream::GetDroppedInfo( 
            /* [in] */ long lSize,
            /* [out] */ long *plArray,
            /* [out] */ long *plNumCopied)
{
	if(!(plNumCopied&&plArray))
		return E_POINTER;
	plNumCopied=0;
	return S_OK;
}
        
HRESULT STDMETHODCALLTYPE CSnakeStream::GetAverageFrameSize( 
            /* [out] */ long *plAverageSize)

{
	if(!plAverageSize)
		return E_POINTER;
	*plAverageSize=SHAREDMEMBODYSIZE;
	return S_OK;
}
