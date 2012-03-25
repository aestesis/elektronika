//------------------------------------------------------------------------------
// File: Auxiliary.h
//
// Desc: Useful auxiliaries for the Win32 Application.
//
// Author : Ashok Jaiswal
//
// Data/Time : September 2004
//------------------------------------------------------------------------------

#if !defined(AUXILIARY_H__3F5BAE08_581B_4008_ABB4_0A4D2D3456C6__INCLUDED_)
#define AUXILIARY_H__3F5BAE08_581B_4008_ABB4_0A4D2D3456C6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


//
// Function prototypes
//

//#ifdef _DEBUG

void TRACE(LPCTSTR lpszFormat, ...) ;

//#endif


//
// Macros
//

#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }

#define JIF(x) { if (FAILED(hr=(x))) \
	{ TRACE(TEXT("FAILED(hr=0x%x) in ") TEXT(#x) TEXT("\n"), hr); return hr; }}

#define LIF(x) { if (FAILED(hr=(x))) \
	{ TRACE(TEXT("FAILED(hr=0x%x) in ") TEXT(#x) TEXT("\n"), hr); }}


//
// Useful Defines
//


#endif // !defined(AUXILIARY_H__3F5BAE08_581B_4008_ABB4_0A4D2D3456C6__INCLUDED_)
