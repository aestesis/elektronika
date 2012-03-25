#ifndef _ELEK_CElekAllocPresenter_
#define _ELEK_CElekAllocPresenter_


#include "COMobjects.h"


// {10000002-0000-0000-0000-000000000001}
const CLSID CLSID_ElekAllocPresenter = 
    {0xa3573535,0x0000,0x0000,
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01}};


class CElekAllocPresenter : public IElekAllocPresenter

{
public:
    // IUnknown
    ULONG __stdcall AddRef();
    ULONG __stdcall Release();
    HRESULT __stdcall QueryInterface(REFIID riid, void** ppv);

	// IElekAllocPresenter
    HRESULT __stdcall setD3DEV(DWORD d3dev);

    CElekAllocPresenter() : m_cRef(1) { g_com_cLocks++; }
    ~CElekAllocPresenter() { g_com_cLocks--; }

private:
    ULONG m_cRef;
};


class CElekAllocPresenterFactory : public IClassFactory
{
public:
    // IUnknown
    ULONG __stdcall AddRef();
    ULONG __stdcall Release();
    HRESULT __stdcall QueryInterface(REFIID riid, void** ppv);

    // IClassFactory
    HRESULT __stdcall CreateInstance(IUnknown* pUnknownOuter, 
        REFIID riid, void** ppv);
    HRESULT __stdcall LockServer(BOOL bLock);

    CElekAllocPresenterFactory() : m_cRef(1) { g_com_cLocks++; }
    ~CElekAllocPresenterFactory() { g_com_cLocks--; }

private:
    ULONG m_cRef;
};



#endif