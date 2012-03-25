#ifndef __IMediaDet_FWD_DEFINED__
#define __IMediaDet_FWD_DEFINED__
typedef interface IMediaDet IMediaDet;
#endif 	/* __IMediaDet_FWD_DEFINED__ */

#ifndef __ISampleGrabberCB_FWD_DEFINED__
#define __ISampleGrabberCB_FWD_DEFINED__
typedef interface ISampleGrabberCB ISampleGrabberCB;
#endif 	/* __ISampleGrabberCB_FWD_DEFINED__ */

#ifndef __ISampleGrabber_FWD_DEFINED__
#define __ISampleGrabber_FWD_DEFINED__
typedef interface ISampleGrabber ISampleGrabber;
#endif 	/* __ISampleGrabber_FWD_DEFINED__ */





#ifndef __SampleGrabber_FWD_DEFINED__
#define __SampleGrabber_FWD_DEFINED__

#ifdef __cplusplus
typedef class SampleGrabber SampleGrabber;
#else
typedef struct SampleGrabber SampleGrabber;
#endif /* __cplusplus */

#endif 	/* __SampleGrabber_FWD_DEFINED__ */



#ifndef __NullRenderer_FWD_DEFINED__
#define __NullRenderer_FWD_DEFINED__

#ifdef __cplusplus
typedef class NullRenderer NullRenderer;
#else
typedef struct NullRenderer NullRenderer;
#endif /* __cplusplus */

#endif 	/* __NullRenderer_FWD_DEFINED__ */


#ifndef __MediaDet_FWD_DEFINED__
#define __MediaDet_FWD_DEFINED__

#ifdef __cplusplus
typedef class MediaDet MediaDet;
#else
typedef struct MediaDet MediaDet;
#endif /* __cplusplus */

#endif 	/* __MediaDet_FWD_DEFINED__ */









#ifndef __ISampleGrabberCB_INTERFACE_DEFINED__
#define __ISampleGrabberCB_INTERFACE_DEFINED__

/* interface ISampleGrabberCB */
/* [unique][helpstring][local][uuid][object] */ 


EXTERN_C const IID IID_ISampleGrabberCB;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0579154A-2B53-4994-B0D0-E773148EFF85")
    ISampleGrabberCB : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SampleCB( 
            double SampleTime,
            IMediaSample *pSample) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BufferCB( 
            double SampleTime,
            BYTE *pBuffer,
            long BufferLen) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISampleGrabberCBVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISampleGrabberCB * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISampleGrabberCB * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISampleGrabberCB * This);
        
        HRESULT ( STDMETHODCALLTYPE *SampleCB )( 
            ISampleGrabberCB * This,
            double SampleTime,
            IMediaSample *pSample);
        
        HRESULT ( STDMETHODCALLTYPE *BufferCB )( 
            ISampleGrabberCB * This,
            double SampleTime,
            BYTE *pBuffer,
            long BufferLen);
        
        END_INTERFACE
    } ISampleGrabberCBVtbl;

    interface ISampleGrabberCB
    {
        CONST_VTBL struct ISampleGrabberCBVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISampleGrabberCB_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISampleGrabberCB_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISampleGrabberCB_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISampleGrabberCB_SampleCB(This,SampleTime,pSample)	\
    (This)->lpVtbl -> SampleCB(This,SampleTime,pSample)

#define ISampleGrabberCB_BufferCB(This,SampleTime,pBuffer,BufferLen)	\
    (This)->lpVtbl -> BufferCB(This,SampleTime,pBuffer,BufferLen)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ISampleGrabberCB_SampleCB_Proxy( 
    ISampleGrabberCB * This,
    double SampleTime,
    IMediaSample *pSample);


void __RPC_STUB ISampleGrabberCB_SampleCB_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISampleGrabberCB_BufferCB_Proxy( 
    ISampleGrabberCB * This,
    double SampleTime,
    BYTE *pBuffer,
    long BufferLen);


void __RPC_STUB ISampleGrabberCB_BufferCB_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISampleGrabberCB_INTERFACE_DEFINED__ */











#ifndef __ISampleGrabber_INTERFACE_DEFINED__
#define __ISampleGrabber_INTERFACE_DEFINED__

/* interface ISampleGrabber */
/* [unique][helpstring][local][uuid][object] */ 


EXTERN_C const IID IID_ISampleGrabber;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6B652FFF-11FE-4fce-92AD-0266B5D7C78F")
    ISampleGrabber : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetOneShot( 
            BOOL OneShot) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetMediaType( 
            const AM_MEDIA_TYPE *pType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetConnectedMediaType( 
            AM_MEDIA_TYPE *pType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetBufferSamples( 
            BOOL BufferThem) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCurrentBuffer( 
            /* [out][in] */ long *pBufferSize,
            /* [out] */ long *pBuffer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCurrentSample( 
            /* [retval][out] */ IMediaSample **ppSample) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCallback( 
            ISampleGrabberCB *pCallback,
            long WhichMethodToCallback) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISampleGrabberVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISampleGrabber * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISampleGrabber * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISampleGrabber * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetOneShot )( 
            ISampleGrabber * This,
            BOOL OneShot);
        
        HRESULT ( STDMETHODCALLTYPE *SetMediaType )( 
            ISampleGrabber * This,
            const AM_MEDIA_TYPE *pType);
        
        HRESULT ( STDMETHODCALLTYPE *GetConnectedMediaType )( 
            ISampleGrabber * This,
            AM_MEDIA_TYPE *pType);
        
        HRESULT ( STDMETHODCALLTYPE *SetBufferSamples )( 
            ISampleGrabber * This,
            BOOL BufferThem);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrentBuffer )( 
            ISampleGrabber * This,
            /* [out][in] */ long *pBufferSize,
            /* [out] */ long *pBuffer);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrentSample )( 
            ISampleGrabber * This,
            /* [retval][out] */ IMediaSample **ppSample);
        
        HRESULT ( STDMETHODCALLTYPE *SetCallback )( 
            ISampleGrabber * This,
            ISampleGrabberCB *pCallback,
            long WhichMethodToCallback);
        
        END_INTERFACE
    } ISampleGrabberVtbl;

    interface ISampleGrabber
    {
        CONST_VTBL struct ISampleGrabberVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISampleGrabber_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISampleGrabber_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISampleGrabber_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISampleGrabber_SetOneShot(This,OneShot)	\
    (This)->lpVtbl -> SetOneShot(This,OneShot)

#define ISampleGrabber_SetMediaType(This,pType)	\
    (This)->lpVtbl -> SetMediaType(This,pType)

#define ISampleGrabber_GetConnectedMediaType(This,pType)	\
    (This)->lpVtbl -> GetConnectedMediaType(This,pType)

#define ISampleGrabber_SetBufferSamples(This,BufferThem)	\
    (This)->lpVtbl -> SetBufferSamples(This,BufferThem)

#define ISampleGrabber_GetCurrentBuffer(This,pBufferSize,pBuffer)	\
    (This)->lpVtbl -> GetCurrentBuffer(This,pBufferSize,pBuffer)

#define ISampleGrabber_GetCurrentSample(This,ppSample)	\
    (This)->lpVtbl -> GetCurrentSample(This,ppSample)

#define ISampleGrabber_SetCallback(This,pCallback,WhichMethodToCallback)	\
    (This)->lpVtbl -> SetCallback(This,pCallback,WhichMethodToCallback)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ISampleGrabber_SetOneShot_Proxy( 
    ISampleGrabber * This,
    BOOL OneShot);


void __RPC_STUB ISampleGrabber_SetOneShot_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISampleGrabber_SetMediaType_Proxy( 
    ISampleGrabber * This,
    const AM_MEDIA_TYPE *pType);


void __RPC_STUB ISampleGrabber_SetMediaType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISampleGrabber_GetConnectedMediaType_Proxy( 
    ISampleGrabber * This,
    AM_MEDIA_TYPE *pType);


void __RPC_STUB ISampleGrabber_GetConnectedMediaType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISampleGrabber_SetBufferSamples_Proxy( 
    ISampleGrabber * This,
    BOOL BufferThem);


void __RPC_STUB ISampleGrabber_SetBufferSamples_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISampleGrabber_GetCurrentBuffer_Proxy( 
    ISampleGrabber * This,
    /* [out][in] */ long *pBufferSize,
    /* [out] */ long *pBuffer);


void __RPC_STUB ISampleGrabber_GetCurrentBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISampleGrabber_GetCurrentSample_Proxy( 
    ISampleGrabber * This,
    /* [retval][out] */ IMediaSample **ppSample);


void __RPC_STUB ISampleGrabber_GetCurrentSample_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISampleGrabber_SetCallback_Proxy( 
    ISampleGrabber * This,
    ISampleGrabberCB *pCallback,
    long WhichMethodToCallback);


void __RPC_STUB ISampleGrabber_SetCallback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISampleGrabber_INTERFACE_DEFINED__ */






#ifndef __IMediaDet_INTERFACE_DEFINED__
#define __IMediaDet_INTERFACE_DEFINED__

/* interface IMediaDet */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IMediaDet;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("65BD0710-24D2-4ff7-9324-ED2E5D3ABAFA")
    IMediaDet : public IUnknown
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Filter( 
            /* [retval][out] */ IUnknown **pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Filter( 
            /* [in] */ IUnknown *newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_OutputStreams( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CurrentStream( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_CurrentStream( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_StreamType( 
            /* [retval][out] */ GUID *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_StreamTypeB( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_StreamLength( 
            /* [retval][out] */ double *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Filename( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Filename( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetBitmapBits( 
            double StreamTime,
            long *pBufferSize,
            char *pBuffer,
            long Width,
            long Height) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE WriteBitmapBits( 
            double StreamTime,
            long Width,
            long Height,
            BSTR Filename) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_StreamMediaType( 
            /* [retval][out] */ AM_MEDIA_TYPE *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetSampleGrabber( 
            /* [out] */ ISampleGrabber **ppVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_FrameRate( 
            /* [retval][out] */ double *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE EnterBitmapGrabMode( 
            double SeekTime) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMediaDetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMediaDet * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMediaDet * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMediaDet * This);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Filter )( 
            IMediaDet * This,
            /* [retval][out] */ IUnknown **pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Filter )( 
            IMediaDet * This,
            /* [in] */ IUnknown *newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_OutputStreams )( 
            IMediaDet * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CurrentStream )( 
            IMediaDet * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_CurrentStream )( 
            IMediaDet * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_StreamType )( 
            IMediaDet * This,
            /* [retval][out] */ GUID *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_StreamTypeB )( 
            IMediaDet * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_StreamLength )( 
            IMediaDet * This,
            /* [retval][out] */ double *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Filename )( 
            IMediaDet * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Filename )( 
            IMediaDet * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetBitmapBits )( 
            IMediaDet * This,
            double StreamTime,
            long *pBufferSize,
            char *pBuffer,
            long Width,
            long Height);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *WriteBitmapBits )( 
            IMediaDet * This,
            double StreamTime,
            long Width,
            long Height,
            BSTR Filename);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_StreamMediaType )( 
            IMediaDet * This,
            /* [retval][out] */ AM_MEDIA_TYPE *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetSampleGrabber )( 
            IMediaDet * This,
            /* [out] */ ISampleGrabber **ppVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_FrameRate )( 
            IMediaDet * This,
            /* [retval][out] */ double *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *EnterBitmapGrabMode )( 
            IMediaDet * This,
            double SeekTime);
        
        END_INTERFACE
    } IMediaDetVtbl;

    interface IMediaDet
    {
        CONST_VTBL struct IMediaDetVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMediaDet_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMediaDet_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMediaDet_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMediaDet_get_Filter(This,pVal)	\
    (This)->lpVtbl -> get_Filter(This,pVal)

#define IMediaDet_put_Filter(This,newVal)	\
    (This)->lpVtbl -> put_Filter(This,newVal)

#define IMediaDet_get_OutputStreams(This,pVal)	\
    (This)->lpVtbl -> get_OutputStreams(This,pVal)

#define IMediaDet_get_CurrentStream(This,pVal)	\
    (This)->lpVtbl -> get_CurrentStream(This,pVal)

#define IMediaDet_put_CurrentStream(This,newVal)	\
    (This)->lpVtbl -> put_CurrentStream(This,newVal)

#define IMediaDet_get_StreamType(This,pVal)	\
    (This)->lpVtbl -> get_StreamType(This,pVal)

#define IMediaDet_get_StreamTypeB(This,pVal)	\
    (This)->lpVtbl -> get_StreamTypeB(This,pVal)

#define IMediaDet_get_StreamLength(This,pVal)	\
    (This)->lpVtbl -> get_StreamLength(This,pVal)

#define IMediaDet_get_Filename(This,pVal)	\
    (This)->lpVtbl -> get_Filename(This,pVal)

#define IMediaDet_put_Filename(This,newVal)	\
    (This)->lpVtbl -> put_Filename(This,newVal)

#define IMediaDet_GetBitmapBits(This,StreamTime,pBufferSize,pBuffer,Width,Height)	\
    (This)->lpVtbl -> GetBitmapBits(This,StreamTime,pBufferSize,pBuffer,Width,Height)

#define IMediaDet_WriteBitmapBits(This,StreamTime,Width,Height,Filename)	\
    (This)->lpVtbl -> WriteBitmapBits(This,StreamTime,Width,Height,Filename)

#define IMediaDet_get_StreamMediaType(This,pVal)	\
    (This)->lpVtbl -> get_StreamMediaType(This,pVal)

#define IMediaDet_GetSampleGrabber(This,ppVal)	\
    (This)->lpVtbl -> GetSampleGrabber(This,ppVal)

#define IMediaDet_get_FrameRate(This,pVal)	\
    (This)->lpVtbl -> get_FrameRate(This,pVal)

#define IMediaDet_EnterBitmapGrabMode(This,SeekTime)	\
    (This)->lpVtbl -> EnterBitmapGrabMode(This,SeekTime)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IMediaDet_get_Filter_Proxy( 
    IMediaDet * This,
    /* [retval][out] */ IUnknown **pVal);


void __RPC_STUB IMediaDet_get_Filter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IMediaDet_put_Filter_Proxy( 
    IMediaDet * This,
    /* [in] */ IUnknown *newVal);


void __RPC_STUB IMediaDet_put_Filter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IMediaDet_get_OutputStreams_Proxy( 
    IMediaDet * This,
    /* [retval][out] */ long *pVal);


void __RPC_STUB IMediaDet_get_OutputStreams_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IMediaDet_get_CurrentStream_Proxy( 
    IMediaDet * This,
    /* [retval][out] */ long *pVal);


void __RPC_STUB IMediaDet_get_CurrentStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IMediaDet_put_CurrentStream_Proxy( 
    IMediaDet * This,
    /* [in] */ long newVal);


void __RPC_STUB IMediaDet_put_CurrentStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IMediaDet_get_StreamType_Proxy( 
    IMediaDet * This,
    /* [retval][out] */ GUID *pVal);


void __RPC_STUB IMediaDet_get_StreamType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IMediaDet_get_StreamTypeB_Proxy( 
    IMediaDet * This,
    /* [retval][out] */ BSTR *pVal);


void __RPC_STUB IMediaDet_get_StreamTypeB_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IMediaDet_get_StreamLength_Proxy( 
    IMediaDet * This,
    /* [retval][out] */ double *pVal);


void __RPC_STUB IMediaDet_get_StreamLength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IMediaDet_get_Filename_Proxy( 
    IMediaDet * This,
    /* [retval][out] */ BSTR *pVal);


void __RPC_STUB IMediaDet_get_Filename_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IMediaDet_put_Filename_Proxy( 
    IMediaDet * This,
    /* [in] */ BSTR newVal);


void __RPC_STUB IMediaDet_put_Filename_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IMediaDet_GetBitmapBits_Proxy( 
    IMediaDet * This,
    double StreamTime,
    long *pBufferSize,
    char *pBuffer,
    long Width,
    long Height);


void __RPC_STUB IMediaDet_GetBitmapBits_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IMediaDet_WriteBitmapBits_Proxy( 
    IMediaDet * This,
    double StreamTime,
    long Width,
    long Height,
    BSTR Filename);


void __RPC_STUB IMediaDet_WriteBitmapBits_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IMediaDet_get_StreamMediaType_Proxy( 
    IMediaDet * This,
    /* [retval][out] */ AM_MEDIA_TYPE *pVal);


void __RPC_STUB IMediaDet_get_StreamMediaType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IMediaDet_GetSampleGrabber_Proxy( 
    IMediaDet * This,
    /* [out] */ ISampleGrabber **ppVal);


void __RPC_STUB IMediaDet_GetSampleGrabber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IMediaDet_get_FrameRate_Proxy( 
    IMediaDet * This,
    /* [retval][out] */ double *pVal);


void __RPC_STUB IMediaDet_get_FrameRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IMediaDet_EnterBitmapGrabMode_Proxy( 
    IMediaDet * This,
    double SeekTime);


void __RPC_STUB IMediaDet_EnterBitmapGrabMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IMediaDet_INTERFACE_DEFINED__ */







EXTERN_C const CLSID CLSID_SampleGrabber;

#ifdef __cplusplus

class DECLSPEC_UUID("C1F400A0-3F08-11d3-9F0B-006008039E37")
SampleGrabber;
#endif




EXTERN_C const CLSID CLSID_NullRenderer;

#ifdef __cplusplus

class DECLSPEC_UUID("C1F400A4-3F08-11d3-9F0B-006008039E37")
NullRenderer;
#endif




EXTERN_C const CLSID CLSID_MediaDet;

#ifdef __cplusplus

class DECLSPEC_UUID("65BD0711-24D2-4ff7-9324-ED2E5D3ABAFA")
MediaDet;
#endif
