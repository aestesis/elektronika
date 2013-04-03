// flash.h : main header file for the FLASH DLL
//

#if !defined(AFX_FLASH_H__3BD85DE0_1659_4C9F_AC9B_23279741D03A__INCLUDED_)
#define AFX_FLASH_H__3BD85DE0_1659_4C9F_AC9B_23279741D03A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CFlashApp
// See flash.cpp for the implementation of this class
//

class CFlashApp : public CWinApp
{
public:
	CFlashApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFlashApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CFlashApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FLASH_H__3BD85DE0_1659_4C9F_AC9B_23279741D03A__INCLUDED_)
