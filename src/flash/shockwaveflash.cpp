// CShockwaveFlash.cpp  : Definition of ActiveX Control wrapper class(es) created by Microsoft Visual C++


#include "stdafx.h"
#include "ShockwaveFlash.h"

/////////////////////////////////////////////////////////////////////////////
// CShockwaveFlash

IMPLEMENT_DYNCREATE(CShockwaveFlash, CWnd)

// CShockwaveFlash properties

// CShockwaveFlash operations

BEGIN_EVENTSINK_MAP(CShockwaveFlash, CWnd)
     ON_EVENT(CShockwaveFlash, IDSHOCK, 150, FSCommand, VTS_BSTR VTS_BSTR)
END_EVENTSINK_MAP()
