//------------------------------------------------------------------------------
// File: Snake.cpp
//
// Desc: Snake DirectShow filter
//
// Author : Ashok Jaiswal
//
// Data/Time : September 2004
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "fSnake.h"

#include <initguid.h>
#include "Snake.h"


//
// Setup data for filter registration
//
const AMOVIESETUP_MEDIATYPE sudPinTypes =
{	&MEDIATYPE_NULL,	// Major CLSID
	&MEDIASUBTYPE_NULL	// Minor type
};

const AMOVIESETUP_PIN psudPins[] =
{
	{
		L"Output",		// Pin's string name
		FALSE,			// Is it rendered
		TRUE,			// Is it an output
		FALSE,			// Allowed none
		FALSE,			// Allowed many
		&CLSID_NULL,	// Connects to filter
		L"Input",		// Connects to pin
		1,				// Number of types
		&sudPinTypes	// Pin type information
	},
};

const AMOVIESETUP_FILTER sudSnake =
{
	&CLSID_Snake,				// CLSID of filter
	L"Elektronika Source",		// Filter's name
	MERIT_DO_NOT_USE,			// Filter merit
	1,							// Number of pins
	psudPins					// Pin information
};


//
//  Object creation template
//
CFactoryTemplate g_Templates[] = 
{
	{
		L"Snake Filter",
		&CLSID_Snake,
		CSnakeFilter::CreateInstance,
		NULL,
		&sudSnake
	},
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);


//
// Filter registration functions
//
HRESULT DllRegisterServer()
{
    HRESULT hr = AMovieDllRegisterServer2( TRUE );
    if( FAILED(hr) )
        return hr;

    IFilterMapper2 *pFm2 = 0;

    hr = CoCreateInstance( CLSID_FilterMapper2
                           , NULL
                           , CLSCTX_INPROC_SERVER
                           , IID_IFilterMapper2
                           , (void **)&pFm2 );

    if(FAILED(hr))
        return hr;

    static const AMOVIESETUP_MEDIATYPE sudAVICoTypeOut =  {
        &MEDIATYPE_Video,
        &MEDIASUBTYPE_RGB24 };

    static const AMOVIESETUP_PIN psudAVICoPins[] =
    {
        { L"Output"       // strName
            , FALSE         // bRendered
            , TRUE          // bOutput
            , FALSE         // bZero
            , FALSE         // bMany
            , &CLSID_NULL   // clsConnectsToFilter
            , 0             // strConnectsToPin
            , 1             // nTypes
            , &sudAVICoTypeOut
        }
    };   // lpTypes

    REGFILTER2 rf2;
    rf2.dwVersion = 1;
    rf2.dwMerit = MERIT_DO_NOT_USE;
    rf2.cPins = NUMELMS(psudAVICoPins);
    rf2.rgPins = psudAVICoPins;

    hr = pFm2->RegisterFilter(
        CLSID_Snake,
        L"Elektronika Source",              
        0,                      
        &CLSID_VideoInputDeviceCategory,
        L"Elektronika Source",              
        &rf2);

    pFm2->Release();

    return hr;
}

HRESULT DllUnregisterServer()
{
    HRESULT hr = AMovieDllRegisterServer2( FALSE );
    if( FAILED(hr) )
        return hr;

    IFilterMapper2 *pFm2 = 0;

    hr = CoCreateInstance( CLSID_FilterMapper2
                           , NULL
                           , CLSCTX_INPROC_SERVER
                           , IID_IFilterMapper2
                           , (void **)&pFm2       );

    if(FAILED(hr))
        return hr;

    hr = pFm2->UnregisterFilter(
        &CLSID_VideoInputDeviceCategory,
        L"Elektronika Source",
        CLSID_Snake);

    pFm2->Release();

    return hr;
}

//
// Create a new instance of this class
//
CUnknown *CSnakeFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    //	DLLEntry does the right thing with the return code and
    //	returned value on failure
	CUnknown *pUnknown = new CSnakeFilter(pUnk, phr);
	if (pUnknown == NULL)
	{
		*phr = E_OUTOFMEMORY;
	}

	return pUnknown;
}

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

// standard DllMain called by CRT, wrap to DllEntryPoint from dshow/baseclasses project

extern "C" BOOL WINAPI DllMain(HINSTANCE, ULONG, LPVOID);

BOOL WINAPI DllMain(HINSTANCE hInstance, ULONG ulReason, LPVOID pv)
{
	return DllEntryPoint(hInstance, ulReason, pv);
}
