#ifndef _DXSEND_H_
#define _DXSEND_H_
//------------------------------------------------------------------------------
// File: Grabber.h
//
// Desc: DirectShow sample code - Header file for the SampleSender
//       example filter
//
// Copyright (c) 1997-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <streams.h>     // Active Movie (includes windows.h)
#include <initguid.h>    // declares DEFINE_GUID to declare an EXTERN_C const.

//------------------------------------------------------------------------------
// Define GUID for the sample grabber example so that it does NOT
// conflict with the official DirectX SampleSender filter
//------------------------------------------------------------------------------
// {930BFF74-A4F6-4ba7-85BB-61528CFFBC43}
DEFINE_GUID(CLSID_SampleSender, 
0x930bff74, 0xa4f6, 0x4ba7, 0x85, 0xbb, 0x61, 0x52, 0x8c, 0xff, 0xbc, 0x43);

// we define a callback typedef for this example. 
// Normally, you would make the SampleSender 
// support a COM interface, and in one of it's methods
// you would pass in a pointer to a COM interface 
// for calling back. See the 
// DX8 documentation for the SampleSender

typedef HRESULT (*SAMPLECALLBACK) (
    IMediaSample * pSample, 
    REFERENCE_TIME * StartTime, 
    REFERENCE_TIME * StopTime,
    BOOL TypeChanged );


DEFINE_GUID(IID_ISampleSender, 
0x930bff74, 0xa4f6, 0x4ba7, 0x85, 0xbb, 0x61, 0x52, 0x8c, 0xff, 0xbc, 0x43);


// we define the interface the app can use to program us
MIDL_INTERFACE("930BFF74-A4F6-4ba7-85BB-61528CFFBC43")
ISampleSender : public IUnknown
    {
    public:
        
        virtual HRESULT STDMETHODCALLTYPE SetAcceptedMediaType( 
            const CMediaType *pType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetConnectedMediaType( 
            AM_MEDIA_TYPE *pType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCallback( 
            SAMPLECALLBACK Callback) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDeliveryBuffer( 
            ALLOCATOR_PROPERTIES props,
            BYTE *pBuffer) = 0;
    };
        

class CSampleSenderInPin;
class CSampleSender;

//----------------------------------------------------------------------------
// this is a special allocator that KNOWS the person who is creating it
// will only create one of them. It allocates CMediaSamples that only 
// reference the buffer location that is set in the pin's renderer's
// data variable
//----------------------------------------------------------------------------

class CSampleSenderAllocator : public CMemAllocator
{
    friend class CSampleSenderInPin;
    friend class CSampleSender;

protected:

    // our pin who created us
    //
    CSampleSenderInPin * m_pPin;

public:

    CSampleSenderAllocator( CSampleSenderInPin * pParent, HRESULT *phr ) 
        : CMemAllocator( TEXT("SampleSenderAllocator"), NULL, phr ) 
        , m_pPin( pParent )
    {
    };

    ~CSampleSenderAllocator( )
    {
        // wipe out m_pBuffer before we try to delete it. It's not an allocated
        // buffer, and the default destructor will try to free it!
        m_pBuffer = NULL;
    }

    // we override this to tell whoever's upstream of us what kind of
    // properties we're going to demand to have
    //
    HRESULT GetAllocatorRequirements( ALLOCATOR_PROPERTIES *pProps );

    HRESULT Alloc( );

    void ReallyFree();
};

//----------------------------------------------------------------------------
// we override the input pin class so we can provide a media type
// to speed up connection times. When you try to connect a filesourceasync
// to a transform filter, DirectShow will insert a splitter and then
// start trying codecs, both audio and video, video codecs first. If
// your sample grabber's set to connect to audio, unless we do this, it
// will try all the video codecs first. Connection times are sped up x10
// for audio with just this minor modification!
//----------------------------------------------------------------------------

class CSampleSenderInPin : public CTransInPlaceInputPin
{
    friend class CSampleSenderAllocator;
    friend class CSampleSender;

    CSampleSenderAllocator * m_pPrivateAllocator;
    ALLOCATOR_PROPERTIES m_allocprops;
    BYTE * m_pBuffer;
    BOOL m_bMediaTypeChanged;

protected:

    CSampleSender * SampleSender( ) { return (CSampleSender*) m_pFilter; }
    HRESULT SetDeliveryBuffer( ALLOCATOR_PROPERTIES props, BYTE * m_pBuffer );

public:

    CSampleSenderInPin( CTransInPlaceFilter * pFilter, HRESULT * pHr ) 
        : CTransInPlaceInputPin( TEXT("SampleSenderInputPin"), pFilter, pHr, L"Input" )
        , m_pPrivateAllocator( NULL )
        , m_pBuffer( NULL )
        , m_bMediaTypeChanged( FALSE )
    {
        memset( &m_allocprops, 0, sizeof( m_allocprops ) );
    }

    ~CSampleSenderInPin( )
    {
        if( m_pPrivateAllocator ) delete m_pPrivateAllocator;
    }

    // override to provide major media type for fast connects

    HRESULT GetMediaType( int iPosition, CMediaType *pMediaType );

    // override this or GetMediaType is never called

    STDMETHODIMP EnumMediaTypes( IEnumMediaTypes **ppEnum );

    // override this to refuse any allocators besides
    // the one the user wants, if this is set

    STDMETHODIMP NotifyAllocator( IMemAllocator *pAllocator, BOOL bReadOnly );

    // override this so we always return the special allocator, if necessary

    STDMETHODIMP GetAllocator( IMemAllocator **ppAllocator );

    HRESULT SetMediaType( const CMediaType *pmt );

};

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

class CSampleSender : public CTransInPlaceFilter
{
    friend class CSampleSenderInPin;
    friend class CSampleSenderAllocator;

protected:

    CMediaType m_mtAccept;
    SAMPLECALLBACK m_callback;
    CCritSec m_Lock; // serialize access to our data

    BOOL IsReadOnly( ) { return !m_bModifiesData; }

    // PURE, override this to ensure we get 
    // connected with the right media type
    HRESULT CheckInputType( const CMediaType * pmt );

    // PURE, override this to callback 
    // the user when a sample is received
    HRESULT Transform( IMediaSample * pms );

    // override this so we can return S_FALSE directly. 
    // The base class CTransInPlace
    // Transform( ) method is called by it's 
    // Receive( ) method. There is no way
    // to get Transform( ) to return an S_FALSE value 
    // (which means "stop giving me data"),
    // to Receive( ) and get Receive( ) to return S_FALSE as well.

    HRESULT Receive( IMediaSample * pms );

public:

    static CUnknown *WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);

    // Expose ISampleSender
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
    DECLARE_IUNKNOWN;

    CSampleSender( IUnknown * pOuter, HRESULT * pHr, BOOL ModifiesData );

    // ISampleSender
    HRESULT SetAcceptedMediaType( const CMediaType * pmt );
    HRESULT GetConnectedMediaType( CMediaType * pmt );
    HRESULT SetCallback( SAMPLECALLBACK Callback );
    HRESULT SetDeliveryBuffer( ALLOCATOR_PROPERTIES props, BYTE * m_pBuffer );
};

#endif
