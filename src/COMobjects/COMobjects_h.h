

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Thu Sep 01 19:10:36 2011
 */
/* Compiler settings for COMobjects.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 7.00.0555 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

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

#ifndef __COMobjects_h_h__
#define __COMobjects_h_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __ISum_FWD_DEFINED__
#define __ISum_FWD_DEFINED__
typedef interface ISum ISum;
#endif 	/* __ISum_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __ISum_INTERFACE_DEFINED__
#define __ISum_INTERFACE_DEFINED__

/* interface ISum */
/* [uuid][object] */ 


EXTERN_C const IID IID_ISum;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a3573535-0000-0000-0000-000000000001")
    ISum : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Sum( 
            /* [in] */ int x,
            /* [in] */ int y,
            /* [retval][out] */ int *retval) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISumVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISum * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISum * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISum * This);
        
        HRESULT ( STDMETHODCALLTYPE *Sum )( 
            ISum * This,
            /* [in] */ int x,
            /* [in] */ int y,
            /* [retval][out] */ int *retval);
        
        END_INTERFACE
    } ISumVtbl;

    interface ISum
    {
        CONST_VTBL struct ISumVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISum_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ISum_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ISum_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ISum_Sum(This,x,y,retval)	\
    ( (This)->lpVtbl -> Sum(This,x,y,retval) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ISum_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


