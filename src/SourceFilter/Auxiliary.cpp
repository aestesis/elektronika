//------------------------------------------------------------------------------
// File: Auxiliary.cpp
//
// Desc: Useful auxiliaries for the Win32 Application.
//
// Author : Ashok Jaiswal
//
// Data/Time : September 2004
//------------------------------------------------------------------------------

#include <Windows.h>
#include "stdafx.h"

#ifdef _DEBUG

#include <stdio.h>
#include <assert.h>

void TRACE(LPCTSTR lpszFormat, ...)
{

    va_list args ;
    va_start(args, lpszFormat) ;

	int nBuf ;
    TCHAR szBuffer[1024] ;  // Large buffer for very long filenames (like with HTTP)

	nBuf = vsprintf(szBuffer, lpszFormat, args) ;

	// was there an error? was the expanded string too long?
	assert(nBuf >= 0) ;

	OutputDebugString(szBuffer) ;
//	MessageBox(NULL, szBuffer, "DirectShow", MB_OK) ;

    va_end(args) ;
}

#else

void TRACE(LPCTSTR lpszFormat, ...)
{
	// TODO: Place code here.
}

#endif
