//------------------------------------------------------------------------------
// File: fSnake_op.h
//
// Desc: define CSnakeStream class
//
// Author : Ashok Jaiswal
//
// Data/Time : September 2004
//------------------------------------------------------------------------------

#if !defined(FSNAKE_OP_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
#define FSNAKE_OP_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "sharedmem.h"

#define pinCAPTURE	0
#define pinPREVIEW	1


class CSnakeStream : public CSourceStream, public IKsPropertySet, public IAMStreamConfig, public IAMDroppedFrames
{
public:
	CSnakeStream(HRESULT *phr, CSnakeFilter *pParent, LPCWSTR pPinName, int pinType);
	virtual ~CSnakeStream(void);

// Attributes
public:

// Operations
public:

	class Asharedmem	*mem;
	BYTE				image[SHAREDMEMBODYSIZE];
	REFERENCE_TIME		currenttime;
	double				starttime;
	int					m_frame;
	REFERENCE_TIME		AvgTimePerFrame;	//	10 000 000 / fps
	int					pinType;
	long				dropped;
	long				notdropped;

// Overrides
protected:
	///////////////////////////
	// Media Type support

	// media types filter have
	virtual HRESULT GetMediaType(CMediaType *pMediaType);

	///////////////////////////
	// Buffer Negotiation support

	// buffer size
	virtual HRESULT DecideBufferSize(IMemAllocator *pMemAlloc, ALLOCATOR_PROPERTIES *pProperties);

	///////////////////////////
	// Data Source support

	// media sample
	virtual HRESULT FillBuffer(IMediaSample *pSample);

	// signal
	virtual HRESULT OnThreadCreate(void);
	virtual HRESULT OnThreadDestroy(void);
	virtual HRESULT OnThreadStartPlay(void);


    DECLARE_IUNKNOWN

    // override this to publicise our interfaces
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);


	
	// IKsPropertySet /////////////////////////////////////////////////////////////////////////////////////:

    virtual /* [local] */ HRESULT STDMETHODCALLTYPE Set( 
        /* [in] */ REFGUID guidPropSet,
        /* [in] */ DWORD dwPropID,
        /* [size_is][in] */ LPVOID pInstanceData,
        /* [in] */ DWORD cbInstanceData,
        /* [size_is][in] */ LPVOID pPropData,
        /* [in] */ DWORD cbPropData);
    
    virtual /* [local] */ HRESULT STDMETHODCALLTYPE Get( 
        /* [in] */ REFGUID guidPropSet,
        /* [in] */ DWORD dwPropID,
        /* [size_is][in] */ LPVOID pInstanceData,
        /* [in] */ DWORD cbInstanceData,
        /* [size_is][out] */ LPVOID pPropData,
        /* [in] */ DWORD cbPropData,
        /* [out] */ DWORD *pcbReturned);
    
    virtual HRESULT STDMETHODCALLTYPE QuerySupported( 
        /* [in] */ REFGUID guidPropSet,
        /* [in] */ DWORD dwPropID,
        /* [out] */ DWORD *pTypeSupport);

	// IAMPushSource /////////////////////////////////////////////////////////////////////////////////////:
/*
	virtual HRESULT STDMETHODCALLTYPE GetMaxStreamOffset(REFERENCE_TIME *prtMaxOffset);
	virtual HRESULT STDMETHODCALLTYPE GetPushSourceFlags(ULONG *pFlags);
	virtual HRESULT STDMETHODCALLTYPE GetStreamOffset(REFERENCE_TIME *prtOffset);
	virtual HRESULT STDMETHODCALLTYPE SetMaxStreamOffset(REFERENCE_TIME rtMaxOffset);
	virtual HRESULT STDMETHODCALLTYPE SetPushSourceFlags(ULONG Flags);
	virtual HRESULT STDMETHODCALLTYPE SetStreamOffset(REFERENCE_TIME rtOffset);
	virtual HRESULT STDMETHODCALLTYPE GetLatency(REFERENCE_TIME *prtLatency);
*/

	// IAMStreamConfig /////////////////////////////////////////////////////////////////////////////////////
        virtual HRESULT STDMETHODCALLTYPE SetFormat( 
            /* [in] */ AM_MEDIA_TYPE *pmt);
        
        virtual HRESULT STDMETHODCALLTYPE GetFormat( 
            /* [out] */ AM_MEDIA_TYPE **ppmt);
        
        virtual HRESULT STDMETHODCALLTYPE GetNumberOfCapabilities( 
            /* [out] */ int *piCount,
            /* [out] */ int *piSize);
        
        virtual HRESULT STDMETHODCALLTYPE GetStreamCaps( 
            /* [in] */ int iIndex,
            /* [out] */ AM_MEDIA_TYPE **ppmt,
            /* [out] */ BYTE *pSCC);


	// IAMDroppedFrames /////////////////////////////////////////////////////////////////////////////////////

        virtual HRESULT STDMETHODCALLTYPE GetNumDropped( 
            /* [out] */ long *plDropped);
        
        virtual HRESULT STDMETHODCALLTYPE GetNumNotDropped( 
            /* [out] */ long *plNotDropped);
        
        virtual HRESULT STDMETHODCALLTYPE GetDroppedInfo( 
            /* [in] */ long lSize,
            /* [out] */ long *plArray,
            /* [out] */ long *plNumCopied);
        
        virtual HRESULT STDMETHODCALLTYPE GetAverageFrameSize( 
            /* [out] */ long *plAverageSize);


	// IQualityControl //////////////////////////////////////////////////////////////////////////////////////
        virtual HRESULT STDMETHODCALLTYPE Notify( 
            /* [in] */ IBaseFilter *pSelf,
            /* [in] */ Quality q);


// Overrides
protected:

// Implementations
protected:

// member variables
private:

};


#endif // !defined(FSNAKE_OP_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
