// COMobjects.cpp : Defines the entry point for the application.
//

long g_com_cLocks = 0;

#include "stdafx.h"
#include "COMobjects.h"
#include "registry.h"



HRESULT __stdcall DllCanUnloadNow()
{
    if(g_com_cLocks == 0)
        return S_OK;
    else
        return S_FALSE;
}

HRESULT __stdcall DllGetClassObject(REFCLSID clsid, REFIID riid, 
    void** ppv)

{
	if(clsid==CLSID_Sum)
	{
		CsumFactory	*pFactory = new CsumFactory;
		if(pFactory == NULL)
			return E_OUTOFMEMORY;

		HRESULT hr = pFactory->QueryInterface(riid, ppv);
		pFactory->Release();
		return hr;
	}
/*	
	if(clsid==CLSID_ElekAllocPresenter)
	{
		CElekAllocPresenterFactory* pFactory = new CElekAllocPresenterFactory;
		if(pFactory == NULL)
			return E_OUTOFMEMORY;

		HRESULT hr = pFactory->QueryInterface(riid, ppv);
		pFactory->Release();
		return hr;
	}
*/	
	return CLASS_E_CLASSNOTAVAILABLE;
}

 
HRESULT __stdcall DllRegisterServer()
{
	HRESULT	hr=0;
/*	
	hr=RegisterServer(GetModuleHandle("COMobjects"), 
        CLSID_ElekAllocPresenter, 
        "ElekAllocPresenter.elektronika", "ElekAllocPresenter.elektronika", 
        "ElekAllocPresenter.elektronika.1");
*/        
    return RegisterServer(GetModuleHandle("COMobjects"), 
        CLSID_Sum, 
        "Sum.elektronika", "Sum.elektronika", 
        "Sum.elektronika.1");
}

HRESULT __stdcall DllUnregisterServer()
{
	HRESULT	hr=0;
/*
    hr=UnregisterServer(CLSID_ElekAllocPresenter, "ElekAllocPresenter.elektronika", "ElekAllocPresenter.elektronika.1");
*/
    return UnregisterServer(CLSID_Sum, "Sum.elektronika", "Sum.elektronika.1");
}

