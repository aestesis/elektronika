// Winamp general purpose plug-in mini-SDK
// Copyright (C) 1997, Justin Frankel/Nullsoft

#include <windows.h>
#include <process.h>

#include "gen.h"
#include "resource.h"

#define SYSTRAY_ICON_BASE 1024

BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}

#define ENABLE_PREV 1
#define ENABLE_PLAY 2
#define ENABLE_STOP 4
#define ENABLE_NEXT 8
#define ENABLE_EJECT 16

#define NUM_ICONS 5

int config_enabled=0;

HICON Icons[NUM_ICONS];
char *tips[NUM_ICONS] = {
	"Previous Track",
	"Play/Pause",
	"Stop",
	"Next Track",
	"Open File"
};

BOOL CALLBACK ConfigProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
void config();
void quit();
int init();
void config_write();
void config_read();

#define PLUGIN_DESC "Nullsoft Tray Control"

winampGeneralPurposePlugin plugin =
{
	GPPHDR_VER,
	PLUGIN_DESC " v0.2",
	init,
	config,
	quit,
};

// we do stdcall for a little bit o size
void __stdcall do_icons()
{
	static int l=0;
	int i=NUM_ICONS;
	if (l == config_enabled) return;

	while (i--)
	{
		if (l & (1<<i))
    {
      NOTIFYICONDATA tnid={0,};
      tnid.cbSize=sizeof(NOTIFYICONDATA);
      tnid.hWnd=plugin.hwndParent;
      tnid.uID=i+SYSTRAY_ICON_BASE;
      Shell_NotifyIcon(NIM_DELETE, &tnid);
    }
	}
	l=config_enabled;	
	for (i = 0; i < NUM_ICONS; i ++)
	{
		if (config_enabled & (1<<i))
    {
			if (!Icons[i]) Icons[i] = LoadIcon(plugin.hDllInstance,MAKEINTRESOURCE(IDI_ICON1+i));

      {
        NOTIFYICONDATA tnid={0,};
        tnid.cbSize=sizeof(NOTIFYICONDATA);
        tnid.hWnd=plugin.hwndParent;
        tnid.uID=i+SYSTRAY_ICON_BASE;
        tnid.uFlags=NIF_ICON | NIF_TIP | NIF_MESSAGE;
        tnid.uCallbackMessage=WM_USER + 2707;
        tnid.hIcon=Icons[i];
        wsprintf(tnid.szTip,"%s - Winamp",tips[i]);
        Shell_NotifyIcon(NIM_ADD, &tnid);
      }
    }
	}
}


void config()
{
	DialogBox(plugin.hDllInstance,MAKEINTRESOURCE(IDD_DIALOG1),plugin.hwndParent,ConfigProc);
}

void quit()
{
	config_write();
	config_enabled=0;
	do_icons();
}

WNDPROC lpWndProcOld;
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_USER+2707)
	{
		switch (LOWORD(lParam))
		{
			case WM_LBUTTONDOWN:
				if (config_enabled) switch (LOWORD(wParam))
				{
					case 1024:
            {
              int a;
						  if ((a=SendMessage(hwnd,WM_USER,0,104)) == 0) // not playing, let's hit prev
						  {
							  SendMessage(hwnd,WM_COMMAND,40044,0);
						  }
						  else if (a != 3 && SendMessage(hwnd,WM_USER,0,105) > 2000) // restart
						  {
							  SendMessage(hwnd,WM_COMMAND,40045,0);
						  } 
              else 
              { // prev
							  SendMessage(hwnd,WM_COMMAND,40044,0);
						  }
            }
					return 0;
					case 1025:
						SendMessage(hwnd,WM_COMMAND,40045+(SendMessage(hwnd,WM_USER,0,104) == 1),0);
					return 0;
					case 1026:
            SendMessage(hwnd,WM_COMMAND,40047 + ((GetKeyState(VK_SHIFT) & (1<<15))?100:0) ,0);
					return 0;
					case 1027:
						SendMessage(hwnd,WM_COMMAND,40048,0);
					return 0;
					case 1028:
						SetForegroundWindow(hwnd);
						if (GetKeyState(VK_CONTROL) & (1<<15))
							SendMessage(hwnd,WM_COMMAND,40185,0);
						else if (GetKeyState(VK_SHIFT) & (1<<15))
							SendMessage(hwnd,WM_COMMAND,40187,0);
						else
							SendMessage(hwnd,WM_COMMAND,40029,0);
					return 0;
				}
			break;
		}
	}
	return CallWindowProc(lpWndProcOld,hwnd,message,wParam,lParam);
}

int init()
{
	config_read();

	lpWndProcOld = (WNDPROC)SetWindowLong(plugin.hwndParent,GWL_WNDPROC,(LONG)WndProc);

	do_icons();

	return 0;
}



BOOL CALLBACK ConfigProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			{
				int i;
				for (i = 0; i < NUM_ICONS; i++)
        {
					CheckDlgButton(hwndDlg,IDC_PREV+i,(config_enabled&(1<<i))?BST_CHECKED:BST_UNCHECKED);
          SetDlgItemText(hwndDlg,IDC_PREV+i,tips[i]);
        }
        SetWindowText(hwndDlg,PLUGIN_DESC);
			}
		break;
		case WM_COMMAND:
      if (LOWORD(wParam) >= IDC_PREV && LOWORD(wParam) <= IDC_PREV5)
      {
				int i;
  			config_enabled=0;
				for (i = 0; i < NUM_ICONS; i++)
					if (IsDlgButtonChecked(hwndDlg,IDC_PREV+i))
						config_enabled |= 1<<i;
				do_icons();
      }
      if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
      {
        EndDialog(hwndDlg,0);
      }
    break;
	}
	return FALSE;
}

char *ini_file=0;

void config_read()
{
  ini_file=(char*)SendMessage(plugin.hwndParent,WM_USER,0,334);
  if ((unsigned int)ini_file < 65536) ini_file="winamp.ini";

  config_enabled = GetPrivateProfileInt(PLUGIN_DESC,"BEN",config_enabled,ini_file);
}

void config_write()
{
	char string[32];
	wsprintf(string,"%d",config_enabled);
  WritePrivateProfileString(PLUGIN_DESC,"BEN",string,ini_file);
}

__declspec( dllexport ) winampGeneralPurposePlugin * winampGetGeneralPurposePlugin()
{
	return &plugin;
}

