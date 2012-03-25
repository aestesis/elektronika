// CShockwaveFlash.h  : Declaration of ActiveX Control wrapper class(es) created by Microsoft Visual C++

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CShockwaveFlash

#include						<interface.h>

#define IDSHOCK	2

#define	FLASHOCXKEY				"\\CLSID\\{D27CDB6E-AE6D-11cf-96B8-444553540000}\\InprocServer32"


class CShockwaveFlash : public CWnd
{
protected:
	DECLARE_DYNCREATE(CShockwaveFlash)
	DECLARE_EVENTSINK_MAP()
public:
	CLSID const& GetClsid()
	{
		static CLSID const clsid
			= { 0xD27CDB6E, 0xAE6D, 0x11CF, { 0x96, 0xB8, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0 } };
		return clsid;
	}
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle,
						const RECT& rect, CWnd* pParentWnd, UINT nID, 
						CCreateContext* pContext = NULL)
	{ 
		return CreateControl(GetClsid(), lpszWindowName, dwStyle, rect, pParentWnd, nID); 
	}

    BOOL Create(LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, 
				UINT nID, CFile* pPersist = NULL, BOOL bStorage = FALSE,
				BSTR bstrLicKey = NULL)
	{ 
		return CreateControl(GetClsid(), lpszWindowName, dwStyle, rect, pParentWnd, nID,
		pPersist, bStorage, bstrLicKey); 
	}

// Attributes
public:
	Asection	section;

// Operations
public:

	long get_ReadyState()
	{
		section.enter();
		long result;
		InvokeHelper(DISPID_READYSTATE, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		section.leave();
		return result;
	}
	long get_TotalFrames()
	{
		section.enter();
		long result;
		InvokeHelper(0x7c, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		section.leave();
		return result;
	}
	BOOL get_Playing()
	{
		section.enter();
		BOOL result;
		InvokeHelper(0x7d, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
		section.leave();
		return result;
	}
	void put_Playing(BOOL newValue)
	{
		section.enter();
		static BYTE parms[] = VTS_BOOL ;
		InvokeHelper(0x7d, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms, newValue);
		section.leave();
	}
	long get_Quality()
	{
		section.enter();
		long result;
		InvokeHelper(0x69, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		section.leave();
		return result;
	}
	void put_Quality(long newValue)
	{
		section.enter();
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x69, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms, newValue);
		section.leave();
	}
	long get_ScaleMode()
	{
		section.enter();
		long result;
		InvokeHelper(0x78, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		section.leave();
		return result;
	}
	void put_ScaleMode(long newValue)
	{
		section.enter();
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x78, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms, newValue);
		section.leave();
	}
	long get_AlignMode()
	{
		section.enter();
		long result;
		InvokeHelper(0x79, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		section.leave();
		return result;
	}
	void put_AlignMode(long newValue)
	{
		section.enter();
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x79, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms, newValue);
		section.leave();
	}
	long get_BackgroundColor()
	{
		section.enter();
		long result;
		InvokeHelper(0x7b, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		section.leave();
		return result;
	}
	void put_BackgroundColor(long newValue)
	{
		section.enter();
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x7b, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms, newValue);
		section.leave();
	}
	BOOL get_Loop()
	{
		section.enter();
		BOOL result;
		InvokeHelper(0x6a, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
		section.leave();
		return result;
	}
	void put_Loop(BOOL newValue)
	{
		section.enter();
		static BYTE parms[] = VTS_BOOL ;
		InvokeHelper(0x6a, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms, newValue);
		section.leave();
	}
	CString get_Movie()
	{
		section.enter();
		CString result;
		InvokeHelper(0x66, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, NULL);
		section.leave();
		return result;
	}
	void put_Movie(LPCTSTR newValue)
	{
		section.enter();
		try
		{		
			static BYTE parms[] = VTS_BSTR ;
			InvokeHelper(0x66, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms, newValue);
		}
		catch(...)
		{
		}
		section.leave();
	}
	long get_FrameNum()
	{
		section.enter();
		long result;
		InvokeHelper(0x6b, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		section.leave();
		return result;
	}
	void put_FrameNum(long newValue)
	{
		section.enter();
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x6b, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms, newValue);
		section.leave();
	}
	void SetZoomRect(long left, long top, long right, long bottom)
	{
		section.enter();
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x6d, DISPATCH_METHOD, VT_EMPTY, NULL, parms, left, top, right, bottom);
		section.leave();
	}
	void Zoom(long factor)
	{
		section.enter();
		try
		{
			static BYTE parms[] = VTS_I4 ;
			InvokeHelper(0x76, DISPATCH_METHOD, VT_EMPTY, NULL, parms, factor);
		}
		catch(...)
		{
		}
		section.leave();
	}
	void Pan(long x, long y, long mode)
	{
		section.enter();
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 ;
		InvokeHelper(0x77, DISPATCH_METHOD, VT_EMPTY, NULL, parms, x, y, mode);
		section.leave();
	}
	void Play()
	{
		section.enter();
		InvokeHelper(0x70, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
		section.leave();
	}
	void Stop()
	{
		section.enter();
		InvokeHelper(0x71, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
		section.leave();
	}
	void Back()
	{
		section.enter();
		InvokeHelper(0x72, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
		section.leave();
	}
	void Forward()
	{
		section.enter();
		InvokeHelper(0x73, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
		section.leave();
	}
	void Rewind()
	{
		section.enter();
		InvokeHelper(0x74, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
		section.leave();
	}
	void StopPlay()
	{
		section.enter();
		InvokeHelper(0x7e, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
		section.leave();
	}
	void GotoFrame(long FrameNum)
	{
		section.enter();
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x7f, DISPATCH_METHOD, VT_EMPTY, NULL, parms, FrameNum);
		section.leave();
	}
	long CurrentFrame()
	{
		section.enter();
		long result;
		InvokeHelper(0x80, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
		section.leave();
		return result;
	}
	BOOL IsPlaying()
	{
		section.enter();
		BOOL result;
		InvokeHelper(0x81, DISPATCH_METHOD, VT_BOOL, (void*)&result, NULL);
		section.leave();
		return result;
	}
	long PercentLoaded()
	{
		section.enter();
		long result;
		InvokeHelper(0x82, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
		section.leave();
		return result;
	}
	BOOL FrameLoaded(long FrameNum)
	{
		section.enter();
		BOOL result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x83, DISPATCH_METHOD, VT_BOOL, (void*)&result, parms, FrameNum);
		section.leave();
		return result;
	}
	long FlashVersion()
	{
		section.enter();
		long result;
		InvokeHelper(0x84, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
		section.leave();
		return result;
	}
	CString get_WMode()
	{
		section.enter();
		CString result;
		InvokeHelper(0x85, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, NULL);
		section.leave();
		return result;
	}
	void put_WMode(LPCTSTR newValue)
	{
		section.enter();
		static BYTE parms[] = VTS_BSTR ;
		InvokeHelper(0x85, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms, newValue);
		section.leave();
	}
	CString get_SAlign()
	{
		section.enter();
		CString result;
		InvokeHelper(0x86, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, NULL);
		section.leave();
		return result;
	}
	void put_SAlign(LPCTSTR newValue)
	{
		section.enter();
		static BYTE parms[] = VTS_BSTR ;
		InvokeHelper(0x86, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms, newValue);
		section.leave();
	}
	BOOL get_Menu()
	{
		section.enter();
		BOOL result;
		InvokeHelper(0x87, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
		section.leave();
		return result;
	}
	void put_Menu(BOOL newValue)
	{
		section.enter();
		static BYTE parms[] = VTS_BOOL ;
		InvokeHelper(0x87, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms, newValue);
		section.leave();
	}
	CString get_Base()
	{
		section.enter();
		CString result;
		InvokeHelper(0x88, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, NULL);
		section.leave();
		return result;
	}
	void put_Base(LPCTSTR newValue)
	{
		section.enter();
		static BYTE parms[] = VTS_BSTR ;
		InvokeHelper(0x88, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms, newValue);
		section.leave();
	}
	CString get_Scale()
	{
		section.enter();
		CString result;
		InvokeHelper(0x89, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, NULL);
		section.leave();
		return result;
	}
	void put_Scale(LPCTSTR newValue)
	{
		section.enter();
		static BYTE parms[] = VTS_BSTR ;
		InvokeHelper(0x89, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms, newValue);
		section.leave();
	}
	BOOL get_DeviceFont()
	{
		section.enter();
		BOOL result;
		InvokeHelper(0x8a, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
		section.leave();
		return result;
	}
	void put_DeviceFont(BOOL newValue)
	{
		section.enter();
		static BYTE parms[] = VTS_BOOL ;
		InvokeHelper(0x8a, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms, newValue);
		section.leave();
	}
	BOOL get_EmbedMovie()
	{
		section.enter();
		BOOL result;
		InvokeHelper(0x8b, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
		section.leave();
		return result;
	}
	void put_EmbedMovie(BOOL newValue)
	{
		section.enter();
		static BYTE parms[] = VTS_BOOL ;
		InvokeHelper(0x8b, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms, newValue);
		section.leave();
	}
	CString get_BGColor()
	{
		section.enter();
		CString result;
		InvokeHelper(0x8c, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, NULL);
		section.leave();
		return result;
	}
	void put_BGColor(LPCTSTR newValue)
	{
		section.enter();
		static BYTE parms[] = VTS_BSTR ;
		InvokeHelper(0x8c, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms, newValue);
		section.leave();
	}
	CString get_Quality2()
	{
		section.enter();
		CString result;
		InvokeHelper(0x8d, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, NULL);
		section.leave();
		return result;
	}
	void put_Quality2(LPCTSTR newValue)
	{
		section.enter();
		static BYTE parms[] = VTS_BSTR ;
		InvokeHelper(0x8d, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms, newValue);
		section.leave();
	}
	void LoadMovie(long layer, LPCTSTR url)
	{
		section.enter();
		static BYTE parms[] = VTS_I4 VTS_BSTR ;
		InvokeHelper(0x8e, DISPATCH_METHOD, VT_EMPTY, NULL, parms, layer, url);
		section.leave();
	}
	void TGotoFrame(LPCTSTR target, long FrameNum)
	{
		section.enter();
		static BYTE parms[] = VTS_BSTR VTS_I4 ;
		InvokeHelper(0x8f, DISPATCH_METHOD, VT_EMPTY, NULL, parms, target, FrameNum);
		section.leave();
	}
	void TGotoLabel(LPCTSTR target, LPCTSTR label)
	{
		section.enter();
		static BYTE parms[] = VTS_BSTR VTS_BSTR ;
		InvokeHelper(0x90, DISPATCH_METHOD, VT_EMPTY, NULL, parms, target, label);
		section.leave();
	}
	long TCurrentFrame(LPCTSTR target)
	{
		section.enter();
		long result;
		static BYTE parms[] = VTS_BSTR ;
		InvokeHelper(0x91, DISPATCH_METHOD, VT_I4, (void*)&result, parms, target);
		section.leave();
		return result;
	}
	CString TCurrentLabel(LPCTSTR target)
	{
		section.enter();
		CString result;
		static BYTE parms[] = VTS_BSTR ;
		InvokeHelper(0x92, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms, target);
		section.leave();
		return result;
	}
	void TPlay(LPCTSTR target)
	{
		section.enter();
		static BYTE parms[] = VTS_BSTR ;
		InvokeHelper(0x93, DISPATCH_METHOD, VT_EMPTY, NULL, parms, target);
		section.leave();
	}
	void TStopPlay(LPCTSTR target)
	{
		section.enter();
		static BYTE parms[] = VTS_BSTR ;
		InvokeHelper(0x94, DISPATCH_METHOD, VT_EMPTY, NULL, parms, target);
		section.leave();
	}
	void SetVariable(LPCTSTR name, LPCTSTR value)
	{
		section.enter();
		try
		{
			static BYTE parms[] = VTS_BSTR VTS_BSTR ;
			InvokeHelper(0x97, DISPATCH_METHOD, VT_EMPTY, NULL, parms, name, value);
		}
		catch(...)
		{
		}
		section.leave();
	}
	CString GetVariable(LPCTSTR name)
	{
		section.enter();
		CString result;
		static BYTE parms[] = VTS_BSTR ;
		InvokeHelper(0x98, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms, name);
		section.leave();
		return result;
	}
	void TSetProperty(LPCTSTR target, long property, LPCTSTR value)
	{
		section.enter();
		static BYTE parms[] = VTS_BSTR VTS_I4 VTS_BSTR ;
		InvokeHelper(0x99, DISPATCH_METHOD, VT_EMPTY, NULL, parms, target, property, value);
		section.leave();
	}
	CString TGetProperty(LPCTSTR target, long property)
	{
		section.enter();
		CString result;
		static BYTE parms[] = VTS_BSTR VTS_I4 ;
		InvokeHelper(0x9a, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms, target, property);
		section.leave();
		return result;
	}
	void TCallFrame(LPCTSTR target, long FrameNum)
	{
		section.enter();
		static BYTE parms[] = VTS_BSTR VTS_I4 ;
		InvokeHelper(0x9b, DISPATCH_METHOD, VT_EMPTY, NULL, parms, target, FrameNum);
		section.leave();
	}
	void TCallLabel(LPCTSTR target, LPCTSTR label)
	{
		section.enter();
		static BYTE parms[] = VTS_BSTR VTS_BSTR ;
		InvokeHelper(0x9c, DISPATCH_METHOD, VT_EMPTY, NULL, parms, target, label);
		section.leave();
	}
	void TSetPropertyNum(LPCTSTR target, long property, double value)
	{
		section.enter();
		static BYTE parms[] = VTS_BSTR VTS_I4 VTS_R8 ;
		InvokeHelper(0x9d, DISPATCH_METHOD, VT_EMPTY, NULL, parms, target, property, value);
		section.leave();
	}
	double TGetPropertyNum(LPCTSTR target, long property)
	{
		section.enter();
		double result;
		static BYTE parms[] = VTS_BSTR VTS_I4 ;
		InvokeHelper(0x9e, DISPATCH_METHOD, VT_R8, (void*)&result, parms, target, property);
		section.leave();
		return result;
	}
	double TGetPropertyAsNumber(LPCTSTR target, long property)
	{
		section.enter();
		double result;
		static BYTE parms[] = VTS_BSTR VTS_I4 ;
		InvokeHelper(0xac, DISPATCH_METHOD, VT_R8, (void*)&result, parms, target, property);
		section.leave();
		return result;
	}
	CString get_SWRemote()
	{
		section.enter();
		CString result;
		InvokeHelper(0x9f, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, NULL);
		section.leave();
		return result;
	}
	void put_SWRemote(LPCTSTR newValue)
	{
		section.enter();
		static BYTE parms[] = VTS_BSTR ;
		InvokeHelper(0x9f, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms, newValue);
		section.leave();
	}
	CString get_FlashVars()
	{
		section.enter();
		CString result;
		InvokeHelper(0xaa, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, NULL);
		section.leave();
		return result;
	}
	void put_FlashVars(LPCTSTR newValue)
	{
		section.enter();
		static BYTE parms[] = VTS_BSTR ;
		InvokeHelper(0xaa, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms, newValue);
		section.leave();
	}
	CString get_AllowScriptAccess()
	{
		section.enter();
		CString result;
		InvokeHelper(0xab, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, NULL);
		section.leave();
		return result;
	}
	void put_AllowScriptAccess(LPCTSTR newValue)
	{
		section.enter();
		static BYTE parms[] = VTS_BSTR ;
		InvokeHelper(0xab, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms, newValue);
		section.leave();
	}
	CString get_MovieData()
	{
		section.enter();
		CString result;
		InvokeHelper(0xbe, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, NULL);
		section.leave();
		return result;
	}
	void put_MovieData(LPCTSTR newValue)
	{
		section.enter();
		try
		{
			static BYTE parms[] = VTS_BSTR ;
			InvokeHelper(0xbe, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms, newValue);
		}
		catch(...)
		{
		}
		section.leave();
	}
	
	
	class Aflashp				*flashp;
	HWND						hw;
	HDC							hdc;
	HBITMAP						hbmp;


	virtual afx_msg void FSCommand(LPSTR name, LPSTR value);
};
