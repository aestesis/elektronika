// flash.h : main header file for the flash DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "flash_i.h"


// CflashApp
// See flash.cpp for the implementation of this class
//

class CflashApp : public CWinApp
{
public:
	CflashApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
