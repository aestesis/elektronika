

#include "COMobjects_h.h" // Generated by MIDL
#include "COMobjects.h"
#include "ElekAllocPresenter.h"



ULONG CElekAllocPresenter::AddRef()
{
    return ++m_cRef;
}

ULONG CElekAllocPresenter::Release()
{
    if(--m_cRef != 0)
        return m_cRef;
    delete this;
    return 0;
}

HRESULT CElekAllocPresenter::QueryInterface(REFIID riid, void** ppv)
{
    if(riid == IID_IUnknown)
        *ppv = (IUnknown*)this;
    else if(riid == IID_IElekAllocPresenter)
        *ppv = (IElekAllocPresenter*)this;
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}

HRESULT CElekAllocPresenter::setD3DEV(DWORD d3dev)
{
	return S_OK;
}





ULONG CElekAllocPresenterFactory::AddRef()
{
    return ++m_cRef;
}

ULONG CElekAllocPresenterFactory::Release()
{
    if(--m_cRef != 0)
        return m_cRef;
    delete this;
    return 0;
}

HRESULT CElekAllocPresenterFactory::QueryInterface(REFIID riid, void** ppv)
{
    if(riid == IID_IUnknown)
        *ppv = (IUnknown*)this;
    else if(riid == IID_IClassFactory)
        *ppv = (IClassFactory*)this;
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}

HRESULT CElekAllocPresenterFactory::CreateInstance(IUnknown *pUnknownOuter, 
    REFIID riid, void** ppv)
{
    if(pUnknownOuter != NULL)
        return CLASS_E_NOAGGREGATION;

    CElekAllocPresenter *pInsideCOM = new CElekAllocPresenter;
    if(pInsideCOM == NULL)
        return E_OUTOFMEMORY;

    HRESULT hr = pInsideCOM->QueryInterface(riid, ppv);
    pInsideCOM->Release();
    return hr;
}

HRESULT CElekAllocPresenterFactory::LockServer(BOOL bLock)
{
    if(bLock)
        g_com_cLocks++;
    else
        g_com_cLocks --;
    return S_OK;
}




