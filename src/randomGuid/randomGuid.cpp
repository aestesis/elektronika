// randomGuid.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "randomGuid.h"
#define MAX_LOADSTRING 100
#include <stdio.h>

LRESULT CALLBACK dlgproc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	hInst=hInstance;
	srand(GetTickCount());
	return (int)DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)dlgproc);
}


void gen(HWND hw)
{
	int		j;
	char	ser[128];
	__int64	d=(__int64)rand()*(__int64)rand()*(__int64)rand()*(__int64)rand()*(__int64)rand();
	sprintf(ser, "0x%16I64x", d);
	for(j=0; j<18; j++)
		if(ser[j]==' ')
			ser[j]='0';
	SetDlgItemText(hw, IDR_DISP, ser);
	if(OpenClipboard(NULL))
	{
		HGLOBAL	clipbuffer;
		char	*buffer;
		EmptyClipboard();
		clipbuffer=GlobalAlloc(GMEM_DDESHARE, strlen(ser)+1);
		if(clipbuffer)
		{
			buffer=(char*)GlobalLock(clipbuffer);
			if(buffer)
			{
				strcpy(buffer, LPCSTR(ser));
				GlobalUnlock(clipbuffer);
				SetClipboardData(CF_TEXT, clipbuffer);
			}
		}
		CloseClipboard();
	}
	
}


// Message handler for about box.
LRESULT CALLBACK dlgproc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
		gen(hDlg);
		return TRUE;
		
		case WM_CLOSE:
		EndDialog(hDlg, 0);
		break;		

		case WM_COMMAND:
		switch(LOWORD(wParam))
		{
			case IDR_GEN:
			gen(hDlg);
			return true;
		}
		break;
	}
	return FALSE;
}
