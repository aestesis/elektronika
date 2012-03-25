

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0366 */
/* at Thu Jan 05 08:00:41 2006
 */
/* Compiler settings for .\flash.idl:
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

#ifndef __flash_i_h__
#define __flash_i_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IShockwaveflash_FWD_DEFINED__
#define __IShockwaveflash_FWD_DEFINED__
typedef interface IShockwaveflash IShockwaveflash;
#endif 	/* __IShockwaveflash_FWD_DEFINED__ */


#ifndef __Shockwaveflash_FWD_DEFINED__
#define __Shockwaveflash_FWD_DEFINED__

#ifdef __cplusplus
typedef class Shockwaveflash Shockwaveflash;
#else
typedef struct Shockwaveflash Shockwaveflash;
#endif /* __cplusplus */

#endif 	/* __Shockwaveflash_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IShockwaveflash_INTERFACE_DEFINED__
#define __IShockwaveflash_INTERFACE_DEFINED__

/* interface IShockwaveflash */
/* [unique][helpstring][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_IShockwaveflash;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F7384183-774C-49BD-9149-0BE59109717F")
    IShockwaveflash : public IDispatch
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct IShockwaveflashVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IShockwaveflash * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IShockwaveflash * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IShockwaveflash * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IShockwaveflash * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IShockwaveflash * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IShockwaveflash * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IShockwaveflash * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } IShockwaveflashVtbl;

    interface IShockwaveflash
    {
        CONST_VTBL struct IShockwaveflashVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IShockwaveflash_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IShockwaveflash_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IShockwaveflash_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IShockwaveflash_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IShockwaveflash_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IShockwaveflash_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IShockwaveflash_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IShockwaveflash_INTERFACE_DEFINED__ */



#ifndef __flashLib_LIBRARY_DEFINED__
#define __flashLib_LIBRARY_DEFINED__

/* library flashLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_flashLib;

EXTERN_C const CLSID CLSID_Shockwaveflash;

#ifdef __cplusplus

class DECLSPEC_UUID("A328704B-4783-43DC-87DA-85A15B0EA61F")
Shockwaveflash;
#endif
#endif /* __flashLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


