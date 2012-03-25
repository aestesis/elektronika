// flashPlayer.h : main header file for the FLASHPLAYER DLL
//

#if !defined(AFX_FLASHPLAYER_H__1A8752E5_8FF0_4E45_AF8A_6727DF61B5CD__INCLUDED_)
#define AFX_FLASHPLAYER_H__1A8752E5_8FF0_4E45_AF8A_6727DF61B5CD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CFlashPlayerApp
// See flashPlayer.cpp for the implementation of this class
//

class CFlashPlayerApp : public CWinApp
{
public:
	CFlashPlayerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFlashPlayerApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CFlashPlayerApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FLASHPLAYER_H__1A8752E5_8FF0_4E45_AF8A_6727DF61B5CD__INCLUDED_)
