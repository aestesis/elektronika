typedef interface IWMPCore	IWMPCore;	// added by YoY
typedef interface IWMPMedia	IWMPMedia;	// added by YoY

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0361 */
/* Compiler settings for wmpplug.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __wmpplug_h__
#define __wmpplug_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IWMPPluginUI_FWD_DEFINED__
#define __IWMPPluginUI_FWD_DEFINED__
typedef interface IWMPPluginUI IWMPPluginUI;
#endif 	/* __IWMPPluginUI_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "wmp.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

/* interface __MIDL_itf_wmpplug_0000 */
/* [local] */ 

//=========================================================================
//
// Microsoft Windows Media Technologies
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
//=========================================================================
#define PLUGIN_INSTALLREGKEY                L"Software\\Microsoft\\MediaPlayer\\UIPlugins"
#define PLUGIN_INSTALLREGKEY_FRIENDLYNAME   L"FriendlyName"
#define PLUGIN_INSTALLREGKEY_DESCRIPTION    L"Description"
#define PLUGIN_INSTALLREGKEY_CAPABILITIES   L"Capabilities"
#define PLUGIN_INSTALLREGKEY_UNINSTALL      L"UninstallPath"
#define	PLUGIN_TYPE_BACKGROUND	( 0x1 )

#define	PLUGIN_TYPE_SEPARATEWINDOW	( 0x2 )

#define	PLUGIN_TYPE_DISPLAYAREA	( 0x3 )

#define	PLUGIN_TYPE_SETTINGSAREA	( 0x4 )

#define	PLUGIN_TYPE_METADATAAREA	( 0x5 )

#define	PLUGIN_FLAGS_HASPROPERTYPAGE	( 0x80000000 )

#define	PLUGIN_FLAGS_INSTALLAUTORUN	( 0x40000000 )

#define	PLUGIN_FLAGS_LAUNCHPROPERTYPAGE	( 0x20000000 )

#define	PLUGIN_FLAGS_ACCEPTSMEDIA	( 0x10000000 )

#define	PLUGIN_FLAGS_ACCEPTSPLAYLISTS	( 0x8000000 )

#define	PLUGIN_FLAGS_HASPRESETS	( 0x4000000 )

#define	PLUGIN_FLAGS_HIDDEN	( 0x2000000 )

#define PLUGIN_MISC_PRESETCOUNT      L"PresetCount"
#define PLUGIN_MISC_PRESETNAMES      L"PresetNames"
#define PLUGIN_MISC_CURRENTPRESET    L"CurrentPreset"
#define PLUGIN_SEPARATEWINDOW_RESIZABLE     L"Resizable"
#define PLUGIN_SEPARATEWINDOW_DEFAULTWIDTH  L"DefaultWidth"
#define PLUGIN_SEPARATEWINDOW_DEFAULTHEIGHT L"DefaultHeight"
#define PLUGIN_SEPARATEWINDOW_MINWIDTH      L"MinWidth"
#define PLUGIN_SEPARATEWINDOW_MINHEIGHT     L"MinHeight"
#define PLUGIN_SEPARATEWINDOW_MAXWIDTH      L"MaxWidth"
#define PLUGIN_SEPARATEWINDOW_MAXHEIGHT     L"MaxHeight"
#define PLUGIN_MISC_QUERYDESTROY            L"QueryDestroy"
#define PLUGIN_ALL_MEDIASENDTO              L"MediaSendTo"
#define PLUGIN_ALL_PLAYLISTSENDTO           L"PlaylistSendTo"
__inline BOOL WMPNotifyPluginAddRemove()
{
    return( ::PostMessage( HWND_BROADCAST, ::RegisterWindowMessageA( "WMPlayer_PluginAddRemove" ), 0, 0 ) );
}


extern RPC_IF_HANDLE __MIDL_itf_wmpplug_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wmpplug_0000_v0_0_s_ifspec;

#ifndef __IWMPPluginUI_INTERFACE_DEFINED__
#define __IWMPPluginUI_INTERFACE_DEFINED__

/* interface IWMPPluginUI */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IWMPPluginUI;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4C5E8F9F-AD3E-4bf9-9753-FCD30D6D38DD")
    IWMPPluginUI : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetCore( 
            /* [in] */ IWMPCore *pCore) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Create( 
            /* [in] */ HWND hwndParent,
            /* [out] */ HWND *phwndWindow) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Destroy( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DisplayPropertyPage( 
            /* [in] */ HWND hwndParent) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProperty( 
            /* [in] */ const WCHAR *pwszName,
            /* [out] */ VARIANT *pvarProperty) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetProperty( 
            /* [in] */ const WCHAR *pwszName,
            /* [in] */ const VARIANT *pvarProperty) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE TranslateAccelerator( 
            /* [in] */ LPMSG lpmsg) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IWMPPluginUIVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMPPluginUI * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMPPluginUI * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMPPluginUI * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetCore )( 
            IWMPPluginUI * This,
            /* [in] */ IWMPCore *pCore);
        
        HRESULT ( STDMETHODCALLTYPE *Create )( 
            IWMPPluginUI * This,
            /* [in] */ HWND hwndParent,
            /* [out] */ HWND *phwndWindow);
        
        HRESULT ( STDMETHODCALLTYPE *Destroy )( 
            IWMPPluginUI * This);
        
        HRESULT ( STDMETHODCALLTYPE *DisplayPropertyPage )( 
            IWMPPluginUI * This,
            /* [in] */ HWND hwndParent);
        
        HRESULT ( STDMETHODCALLTYPE *GetProperty )( 
            IWMPPluginUI * This,
            /* [in] */ const WCHAR *pwszName,
            /* [out] */ VARIANT *pvarProperty);
        
        HRESULT ( STDMETHODCALLTYPE *SetProperty )( 
            IWMPPluginUI * This,
            /* [in] */ const WCHAR *pwszName,
            /* [in] */ const VARIANT *pvarProperty);
        
        HRESULT ( STDMETHODCALLTYPE *TranslateAccelerator )( 
            IWMPPluginUI * This,
            /* [in] */ LPMSG lpmsg);
        
        END_INTERFACE
    } IWMPPluginUIVtbl;

    interface IWMPPluginUI
    {
        CONST_VTBL struct IWMPPluginUIVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPPluginUI_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPPluginUI_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPPluginUI_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPPluginUI_SetCore(This,pCore)	\
    (This)->lpVtbl -> SetCore(This,pCore)

#define IWMPPluginUI_Create(This,hwndParent,phwndWindow)	\
    (This)->lpVtbl -> Create(This,hwndParent,phwndWindow)

#define IWMPPluginUI_Destroy(This)	\
    (This)->lpVtbl -> Destroy(This)

#define IWMPPluginUI_DisplayPropertyPage(This,hwndParent)	\
    (This)->lpVtbl -> DisplayPropertyPage(This,hwndParent)

#define IWMPPluginUI_GetProperty(This,pwszName,pvarProperty)	\
    (This)->lpVtbl -> GetProperty(This,pwszName,pvarProperty)

#define IWMPPluginUI_SetProperty(This,pwszName,pvarProperty)	\
    (This)->lpVtbl -> SetProperty(This,pwszName,pvarProperty)

#define IWMPPluginUI_TranslateAccelerator(This,lpmsg)	\
    (This)->lpVtbl -> TranslateAccelerator(This,lpmsg)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IWMPPluginUI_SetCore_Proxy( 
    IWMPPluginUI * This,
    /* [in] */ IWMPCore *pCore);


void __RPC_STUB IWMPPluginUI_SetCore_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPPluginUI_Create_Proxy( 
    IWMPPluginUI * This,
    /* [in] */ HWND hwndParent,
    /* [out] */ HWND *phwndWindow);


void __RPC_STUB IWMPPluginUI_Create_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPPluginUI_Destroy_Proxy( 
    IWMPPluginUI * This);


void __RPC_STUB IWMPPluginUI_Destroy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPPluginUI_DisplayPropertyPage_Proxy( 
    IWMPPluginUI * This,
    /* [in] */ HWND hwndParent);


void __RPC_STUB IWMPPluginUI_DisplayPropertyPage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPPluginUI_GetProperty_Proxy( 
    IWMPPluginUI * This,
    /* [in] */ const WCHAR *pwszName,
    /* [out] */ VARIANT *pvarProperty);


void __RPC_STUB IWMPPluginUI_GetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPPluginUI_SetProperty_Proxy( 
    IWMPPluginUI * This,
    /* [in] */ const WCHAR *pwszName,
    /* [in] */ const VARIANT *pvarProperty);


void __RPC_STUB IWMPPluginUI_SetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPPluginUI_TranslateAccelerator_Proxy( 
    IWMPPluginUI * This,
    /* [in] */ LPMSG lpmsg);


void __RPC_STUB IWMPPluginUI_TranslateAccelerator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IWMPPluginUI_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  HWND_UserSize(     unsigned long *, unsigned long            , HWND * ); 
unsigned char * __RPC_USER  HWND_UserMarshal(  unsigned long *, unsigned char *, HWND * ); 
unsigned char * __RPC_USER  HWND_UserUnmarshal(unsigned long *, unsigned char *, HWND * ); 
void                      __RPC_USER  HWND_UserFree(     unsigned long *, HWND * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif









/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0361 */
/* Compiler settings for effects.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __effects_h__
#define __effects_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IWMPEffects_FWD_DEFINED__
#define __IWMPEffects_FWD_DEFINED__
typedef interface IWMPEffects IWMPEffects;
#endif 	/* __IWMPEffects_FWD_DEFINED__ */


#ifndef __IWMPEffects2_FWD_DEFINED__
#define __IWMPEffects2_FWD_DEFINED__
typedef interface IWMPEffects2 IWMPEffects2;
#endif 	/* __IWMPEffects2_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "wmp.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

/* interface __MIDL_itf_effects_0000 */
/* [local] */ 

//=========================================================================
//
// Microsoft Windows Media Technologies
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
//=========================================================================
#define	EFFECT_CANGOFULLSCREEN	( 0x1 )

#define	EFFECT_HASPROPERTYPAGE	( 0x2 )

#define	EFFECT_VARIABLEFREQSTEP	( 0x4 )

#define	EFFECT_WINDOWEDONLY	( 0x8 )

#define	EFFECT2_FULLSCREENEXCLUSIVE	( 0x10 )

#define	SA_BUFFER_SIZE	( 1024 )


enum PlayerState
    {	stop_state	= 0,
	pause_state	= 1,
	play_state	= 2
    } ;

//**********************************************************************
// Define the minimum and maximum frequency ranges returned in our
// TimedLevel frequency array (i.e. first index in TimedLevel.frequency
// is at 20Hz and last is at 22050Hz).
//**********************************************************************
static const float kfltTimedLevelMaximumFrequency = 22050.0F;
static const float kfltTimedLevelMinimumFrequency = 20.0F;

/*
 * FREQUENCY_INDEX() returns the index into TimedLevel.frequency[] where 
 * the specified frequency is located in the power spectrum
 */
#define FREQUENCY_INDEX(FREQ)\
  (int)(((FREQ) - kfltTimedLevelMinimumFrequency) /\
    (((kfltTimedLevelMaximumFrequency - kfltTimedLevelMinimumFrequency) / SA_BUFFER_SIZE)))

typedef struct tagTimedLevel
    {
    unsigned char frequency[ 2 ][ 1024 ];
    unsigned char waveform[ 2 ][ 1024 ];
    int state;
    hyper timeStamp;
    } 	TimedLevel;



extern RPC_IF_HANDLE __MIDL_itf_effects_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_effects_0000_v0_0_s_ifspec;

#ifndef __IWMPEffects_INTERFACE_DEFINED__
#define __IWMPEffects_INTERFACE_DEFINED__

/* interface IWMPEffects */
/* [oleautomation][unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IWMPEffects;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D3984C13-C3CB-48e2-8BE5-5168340B4F35")
    IWMPEffects : public IUnknown
    {
    public:
        virtual /* [helpstring][local] */ HRESULT STDMETHODCALLTYPE Render( 
            /* [in] */ TimedLevel *pLevels,
            /* [in] */ HDC hdc,
            /* [in] */ RECT *prc) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE MediaInfo( 
            /* [in] */ LONG lChannelCount,
            /* [in] */ LONG lSampleRate,
            /* [in] */ BSTR bstrTitle) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetCapabilities( 
            /* [out] */ DWORD *pdwCapabilities) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetTitle( 
            /* [out] */ BSTR *bstrTitle) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetPresetTitle( 
            /* [in] */ LONG nPreset,
            /* [out] */ BSTR *bstrPresetTitle) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetPresetCount( 
            /* [out] */ LONG *pnPresetCount) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetCurrentPreset( 
            /* [in] */ LONG nPreset) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetCurrentPreset( 
            /* [out] */ LONG *pnPreset) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE DisplayPropertyPage( 
            /* [in] */ HWND hwndOwner) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GoFullscreen( 
            /* [in] */ BOOL fFullScreen) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE RenderFullScreen( 
            /* [in] */ TimedLevel *pLevels) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IWMPEffectsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMPEffects * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMPEffects * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMPEffects * This);
        
        /* [helpstring][local] */ HRESULT ( STDMETHODCALLTYPE *Render )( 
            IWMPEffects * This,
            /* [in] */ TimedLevel *pLevels,
            /* [in] */ HDC hdc,
            /* [in] */ RECT *prc);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MediaInfo )( 
            IWMPEffects * This,
            /* [in] */ LONG lChannelCount,
            /* [in] */ LONG lSampleRate,
            /* [in] */ BSTR bstrTitle);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetCapabilities )( 
            IWMPEffects * This,
            /* [out] */ DWORD *pdwCapabilities);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetTitle )( 
            IWMPEffects * This,
            /* [out] */ BSTR *bstrTitle);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetPresetTitle )( 
            IWMPEffects * This,
            /* [in] */ LONG nPreset,
            /* [out] */ BSTR *bstrPresetTitle);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetPresetCount )( 
            IWMPEffects * This,
            /* [out] */ LONG *pnPresetCount);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetCurrentPreset )( 
            IWMPEffects * This,
            /* [in] */ LONG nPreset);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetCurrentPreset )( 
            IWMPEffects * This,
            /* [out] */ LONG *pnPreset);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *DisplayPropertyPage )( 
            IWMPEffects * This,
            /* [in] */ HWND hwndOwner);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GoFullscreen )( 
            IWMPEffects * This,
            /* [in] */ BOOL fFullScreen);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *RenderFullScreen )( 
            IWMPEffects * This,
            /* [in] */ TimedLevel *pLevels);
        
        END_INTERFACE
    } IWMPEffectsVtbl;

    interface IWMPEffects
    {
        CONST_VTBL struct IWMPEffectsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPEffects_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPEffects_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPEffects_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPEffects_Render(This,pLevels,hdc,prc)	\
    (This)->lpVtbl -> Render(This,pLevels,hdc,prc)

#define IWMPEffects_MediaInfo(This,lChannelCount,lSampleRate,bstrTitle)	\
    (This)->lpVtbl -> MediaInfo(This,lChannelCount,lSampleRate,bstrTitle)

#define IWMPEffects_GetCapabilities(This,pdwCapabilities)	\
    (This)->lpVtbl -> GetCapabilities(This,pdwCapabilities)

#define IWMPEffects_GetTitle(This,bstrTitle)	\
    (This)->lpVtbl -> GetTitle(This,bstrTitle)

#define IWMPEffects_GetPresetTitle(This,nPreset,bstrPresetTitle)	\
    (This)->lpVtbl -> GetPresetTitle(This,nPreset,bstrPresetTitle)

#define IWMPEffects_GetPresetCount(This,pnPresetCount)	\
    (This)->lpVtbl -> GetPresetCount(This,pnPresetCount)

#define IWMPEffects_SetCurrentPreset(This,nPreset)	\
    (This)->lpVtbl -> SetCurrentPreset(This,nPreset)

#define IWMPEffects_GetCurrentPreset(This,pnPreset)	\
    (This)->lpVtbl -> GetCurrentPreset(This,pnPreset)

#define IWMPEffects_DisplayPropertyPage(This,hwndOwner)	\
    (This)->lpVtbl -> DisplayPropertyPage(This,hwndOwner)

#define IWMPEffects_GoFullscreen(This,fFullScreen)	\
    (This)->lpVtbl -> GoFullscreen(This,fFullScreen)

#define IWMPEffects_RenderFullScreen(This,pLevels)	\
    (This)->lpVtbl -> RenderFullScreen(This,pLevels)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][local] */ HRESULT STDMETHODCALLTYPE IWMPEffects_Render_Proxy( 
    IWMPEffects * This,
    /* [in] */ TimedLevel *pLevels,
    /* [in] */ HDC hdc,
    /* [in] */ RECT *prc);


void __RPC_STUB IWMPEffects_Render_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IWMPEffects_MediaInfo_Proxy( 
    IWMPEffects * This,
    /* [in] */ LONG lChannelCount,
    /* [in] */ LONG lSampleRate,
    /* [in] */ BSTR bstrTitle);


void __RPC_STUB IWMPEffects_MediaInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IWMPEffects_GetCapabilities_Proxy( 
    IWMPEffects * This,
    /* [out] */ DWORD *pdwCapabilities);


void __RPC_STUB IWMPEffects_GetCapabilities_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IWMPEffects_GetTitle_Proxy( 
    IWMPEffects * This,
    /* [out] */ BSTR *bstrTitle);


void __RPC_STUB IWMPEffects_GetTitle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IWMPEffects_GetPresetTitle_Proxy( 
    IWMPEffects * This,
    /* [in] */ LONG nPreset,
    /* [out] */ BSTR *bstrPresetTitle);


void __RPC_STUB IWMPEffects_GetPresetTitle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IWMPEffects_GetPresetCount_Proxy( 
    IWMPEffects * This,
    /* [out] */ LONG *pnPresetCount);


void __RPC_STUB IWMPEffects_GetPresetCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IWMPEffects_SetCurrentPreset_Proxy( 
    IWMPEffects * This,
    /* [in] */ LONG nPreset);


void __RPC_STUB IWMPEffects_SetCurrentPreset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IWMPEffects_GetCurrentPreset_Proxy( 
    IWMPEffects * This,
    /* [out] */ LONG *pnPreset);


void __RPC_STUB IWMPEffects_GetCurrentPreset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IWMPEffects_DisplayPropertyPage_Proxy( 
    IWMPEffects * This,
    /* [in] */ HWND hwndOwner);


void __RPC_STUB IWMPEffects_DisplayPropertyPage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IWMPEffects_GoFullscreen_Proxy( 
    IWMPEffects * This,
    /* [in] */ BOOL fFullScreen);


void __RPC_STUB IWMPEffects_GoFullscreen_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IWMPEffects_RenderFullScreen_Proxy( 
    IWMPEffects * This,
    /* [in] */ TimedLevel *pLevels);


void __RPC_STUB IWMPEffects_RenderFullScreen_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IWMPEffects_INTERFACE_DEFINED__ */


#ifndef __IWMPEffects2_INTERFACE_DEFINED__
#define __IWMPEffects2_INTERFACE_DEFINED__

/* interface IWMPEffects2 */
/* [oleautomation][unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IWMPEffects2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("695386EC-AA3C-4618-A5E1-DD9A8B987632")
    IWMPEffects2 : public IWMPEffects
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetCore( 
            /* [in] */ IWMPCore *pPlayer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Create( 
            /* [in] */ HWND hwndParent) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Destroy( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE NotifyNewMedia( 
            /* [in] */ IWMPMedia *pMedia) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnWindowMessage( 
            /* [in] */ UINT msg,
            /* [in] */ WPARAM WParam,
            /* [in] */ LPARAM LParam,
            /* [in] */ LRESULT *plResultParam) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RenderWindowed( 
            /* [in] */ TimedLevel *pData,
            /* [in] */ BOOL fRequiredRender) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IWMPEffects2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMPEffects2 * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMPEffects2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMPEffects2 * This);
        
        /* [helpstring][local] */ HRESULT ( STDMETHODCALLTYPE *Render )( 
            IWMPEffects2 * This,
            /* [in] */ TimedLevel *pLevels,
            /* [in] */ HDC hdc,
            /* [in] */ RECT *prc);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MediaInfo )( 
            IWMPEffects2 * This,
            /* [in] */ LONG lChannelCount,
            /* [in] */ LONG lSampleRate,
            /* [in] */ BSTR bstrTitle);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetCapabilities )( 
            IWMPEffects2 * This,
            /* [out] */ DWORD *pdwCapabilities);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetTitle )( 
            IWMPEffects2 * This,
            /* [out] */ BSTR *bstrTitle);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetPresetTitle )( 
            IWMPEffects2 * This,
            /* [in] */ LONG nPreset,
            /* [out] */ BSTR *bstrPresetTitle);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetPresetCount )( 
            IWMPEffects2 * This,
            /* [out] */ LONG *pnPresetCount);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetCurrentPreset )( 
            IWMPEffects2 * This,
            /* [in] */ LONG nPreset);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetCurrentPreset )( 
            IWMPEffects2 * This,
            /* [out] */ LONG *pnPreset);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *DisplayPropertyPage )( 
            IWMPEffects2 * This,
            /* [in] */ HWND hwndOwner);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GoFullscreen )( 
            IWMPEffects2 * This,
            /* [in] */ BOOL fFullScreen);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *RenderFullScreen )( 
            IWMPEffects2 * This,
            /* [in] */ TimedLevel *pLevels);
        
        HRESULT ( STDMETHODCALLTYPE *SetCore )( 
            IWMPEffects2 * This,
            /* [in] */ IWMPCore *pPlayer);
        
        HRESULT ( STDMETHODCALLTYPE *Create )( 
            IWMPEffects2 * This,
            /* [in] */ HWND hwndParent);
        
        HRESULT ( STDMETHODCALLTYPE *Destroy )( 
            IWMPEffects2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *NotifyNewMedia )( 
            IWMPEffects2 * This,
            /* [in] */ IWMPMedia *pMedia);
        
        HRESULT ( STDMETHODCALLTYPE *OnWindowMessage )( 
            IWMPEffects2 * This,
            /* [in] */ UINT msg,
            /* [in] */ WPARAM WParam,
            /* [in] */ LPARAM LParam,
            /* [in] */ LRESULT *plResultParam);
        
        HRESULT ( STDMETHODCALLTYPE *RenderWindowed )( 
            IWMPEffects2 * This,
            /* [in] */ TimedLevel *pData,
            /* [in] */ BOOL fRequiredRender);
        
        END_INTERFACE
    } IWMPEffects2Vtbl;

    interface IWMPEffects2
    {
        CONST_VTBL struct IWMPEffects2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPEffects2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPEffects2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPEffects2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPEffects2_Render(This,pLevels,hdc,prc)	\
    (This)->lpVtbl -> Render(This,pLevels,hdc,prc)

#define IWMPEffects2_MediaInfo(This,lChannelCount,lSampleRate,bstrTitle)	\
    (This)->lpVtbl -> MediaInfo(This,lChannelCount,lSampleRate,bstrTitle)

#define IWMPEffects2_GetCapabilities(This,pdwCapabilities)	\
    (This)->lpVtbl -> GetCapabilities(This,pdwCapabilities)

#define IWMPEffects2_GetTitle(This,bstrTitle)	\
    (This)->lpVtbl -> GetTitle(This,bstrTitle)

#define IWMPEffects2_GetPresetTitle(This,nPreset,bstrPresetTitle)	\
    (This)->lpVtbl -> GetPresetTitle(This,nPreset,bstrPresetTitle)

#define IWMPEffects2_GetPresetCount(This,pnPresetCount)	\
    (This)->lpVtbl -> GetPresetCount(This,pnPresetCount)

#define IWMPEffects2_SetCurrentPreset(This,nPreset)	\
    (This)->lpVtbl -> SetCurrentPreset(This,nPreset)

#define IWMPEffects2_GetCurrentPreset(This,pnPreset)	\
    (This)->lpVtbl -> GetCurrentPreset(This,pnPreset)

#define IWMPEffects2_DisplayPropertyPage(This,hwndOwner)	\
    (This)->lpVtbl -> DisplayPropertyPage(This,hwndOwner)

#define IWMPEffects2_GoFullscreen(This,fFullScreen)	\
    (This)->lpVtbl -> GoFullscreen(This,fFullScreen)

#define IWMPEffects2_RenderFullScreen(This,pLevels)	\
    (This)->lpVtbl -> RenderFullScreen(This,pLevels)


#define IWMPEffects2_SetCore(This,pPlayer)	\
    (This)->lpVtbl -> SetCore(This,pPlayer)

#define IWMPEffects2_Create(This,hwndParent)	\
    (This)->lpVtbl -> Create(This,hwndParent)

#define IWMPEffects2_Destroy(This)	\
    (This)->lpVtbl -> Destroy(This)

#define IWMPEffects2_NotifyNewMedia(This,pMedia)	\
    (This)->lpVtbl -> NotifyNewMedia(This,pMedia)

#define IWMPEffects2_OnWindowMessage(This,msg,WParam,LParam,plResultParam)	\
    (This)->lpVtbl -> OnWindowMessage(This,msg,WParam,LParam,plResultParam)

#define IWMPEffects2_RenderWindowed(This,pData,fRequiredRender)	\
    (This)->lpVtbl -> RenderWindowed(This,pData,fRequiredRender)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IWMPEffects2_SetCore_Proxy( 
    IWMPEffects2 * This,
    /* [in] */ IWMPCore *pPlayer);


void __RPC_STUB IWMPEffects2_SetCore_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPEffects2_Create_Proxy( 
    IWMPEffects2 * This,
    /* [in] */ HWND hwndParent);


void __RPC_STUB IWMPEffects2_Create_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPEffects2_Destroy_Proxy( 
    IWMPEffects2 * This);


void __RPC_STUB IWMPEffects2_Destroy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPEffects2_NotifyNewMedia_Proxy( 
    IWMPEffects2 * This,
    /* [in] */ IWMPMedia *pMedia);


void __RPC_STUB IWMPEffects2_NotifyNewMedia_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPEffects2_OnWindowMessage_Proxy( 
    IWMPEffects2 * This,
    /* [in] */ UINT msg,
    /* [in] */ WPARAM WParam,
    /* [in] */ LPARAM LParam,
    /* [in] */ LRESULT *plResultParam);


void __RPC_STUB IWMPEffects2_OnWindowMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPEffects2_RenderWindowed_Proxy( 
    IWMPEffects2 * This,
    /* [in] */ TimedLevel *pData,
    /* [in] */ BOOL fRequiredRender);


void __RPC_STUB IWMPEffects2_RenderWindowed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IWMPEffects2_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  HWND_UserSize(     unsigned long *, unsigned long            , HWND * ); 
unsigned char * __RPC_USER  HWND_UserMarshal(  unsigned long *, unsigned char *, HWND * ); 
unsigned char * __RPC_USER  HWND_UserUnmarshal(unsigned long *, unsigned char *, HWND * ); 
void                      __RPC_USER  HWND_UserFree(     unsigned long *, HWND * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


