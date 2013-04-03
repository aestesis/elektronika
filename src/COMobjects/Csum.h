#ifndef _ELEK_CSUM_
#define _ELEK_CSUM_


#include "COMobjects.h"


// {10000002-0000-0000-0000-000000000001}
const CLSID CLSID_Sum = 
    {0xa3573535,0x0000,0x0000,
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01}};


class Csum : public ISum

{
public:
    // IUnknown
    ULONG __stdcall AddRef();
    ULONG __stdcall Release();
    HRESULT __stdcall QueryInterface(REFIID riid, void** ppv);

    // ISum
    HRESULT __stdcall Sum(int x, int y, int* retval);

    Csum() : m_cRef(1) { g_com_cLocks++; }
    ~Csum() { g_com_cLocks--; }

private:
    ULONG m_cRef;
};


class CsumFactory : public IClassFactory
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

    CsumFactory() : m_cRef(1) { g_com_cLocks++; }
    ~CsumFactory() { g_com_cLocks--; }

private:
    ULONG m_cRef;
};



#endif