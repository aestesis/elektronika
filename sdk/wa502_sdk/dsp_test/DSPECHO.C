// Winamp test dsp library 0.9 for Winamp 2
// Copyright (C) 1997, Justin Frankel/Nullsoft
// Feel free to base any plugins on this "framework"...

#include <windows.h>
#include <commctrl.h>
#include "dsp.h"
#include "resource.h"

// avoid stupid CRT silliness
BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}


// pitch value
int g_pitch=100;

// pitch control window
HWND pitch_control_hwnd;

// auxilary pitch buffer (for resampling from)
short *pitch_buffer=NULL;
int pitch_buffer_len=0;
int quit_pitch=0;

// module getter.
winampDSPModule *getModule(int which);

void config(struct winampDSPModule *this_mod);
int init(struct winampDSPModule *this_mod);
void initpitch(struct winampDSPModule *this_mod);
void quit(struct winampDSPModule *this_mod);
void quitpitch(struct winampDSPModule *this_mod);
int modify_samples1(struct winampDSPModule *this_mod, short int *samples, int numsamples, int bps, int nch, int srate);
int modify_samples2(struct winampDSPModule *this_mod, short int *samples, int numsamples, int bps, int nch, int srate);
int modify_samples3(struct winampDSPModule *this_mod, short int *samples, int numsamples, int bps, int nch, int srate);

static BOOL CALLBACK pitchProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);

// Module header, includes version, description, and address of the module retriever function
winampDSPHeader hdr = { DSP_HDRVER, "Nullsoft DSP demo v0.3 for Winamp 2", getModule };

// first module
winampDSPModule mod =
{
	"Nullsoft Echo v0.2",
	NULL,	// hwndParent
	NULL,	// hDllInstance
	config,
	init,
	modify_samples1,
	quit
};


// second module
winampDSPModule mod2 =
{
	"Nullsoft Stereo Voice Removal v0.2",
	NULL,	// hwndParent
	NULL,	// hDllInstance
	config,
	init,
	modify_samples2,
	quit
};

winampDSPModule mod3 =
{
	"Nullsoft Pitch/Tempo Control v0.2",
	NULL,	// hwndParent
	NULL,	// hDllInstance
	config,
	initpitch,
	modify_samples3,
	quitpitch
};


#ifdef __cplusplus
extern "C" {
#endif
// this is the only exported symbol. returns our main header.
__declspec( dllexport ) winampDSPHeader *winampDSPGetHeader2()
{
	return &hdr;
}
#ifdef __cplusplus
}
#endif

// getmodule routine from the main header. Returns NULL if an invalid module was requested,
// otherwise returns either mod1 or mod2 depending on 'which'.
winampDSPModule *getModule(int which)
{
	switch (which)
	{
		case 0: return &mod;
		case 1: return &mod2;
		case 2: return &mod3;
		default:return NULL;
	}
}

// configuration. Passed this_mod, as a "this" parameter. Allows you to make one configuration
// function that shares code for all your modules (you don't HAVE to use it though, you can make
// config1(), config2(), etc...)
void config(struct winampDSPModule *this_mod)
{
	MessageBox(this_mod->hwndParent,"This module is Copyright(C) 1997, Justin Frankel/Nullsoft\n"
									"Notes:\n"
									" * 8 bit samples aren't supported.\n"
									" * Pitch control rules!\n"
									" * Voice removal sucks (works about 10% of the time)!\n"
									" * Echo isn't very good!\n"
									"etc... this is really just a test of the new\n"
									"DSP plug-in system. Nothing more.",
									"Configuration",MB_OK);
}

int init(struct winampDSPModule *this_mod)
{
	return 0;
}
void initpitch(struct winampDSPModule *this_mod)
{
	pitch_buffer_len=0;
	pitch_buffer=NULL;
	quit_pitch=0;		
	ShowWindow((pitch_control_hwnd=CreateDialog(this_mod->hDllInstance,MAKEINTRESOURCE(IDD_DIALOG1),this_mod->hwndParent,pitchProc)),SW_SHOW);
}

// cleanup (opposite of init()). Destroys the window, unregisters the window class
void quit(struct winampDSPModule *this_mod)
{
}

void quitpitch(struct winampDSPModule *this_mod)
{
	if (this_mod == &mod3)
	{
		if (pitch_buffer) GlobalFree(pitch_buffer);
		pitch_buffer_len=0;
		pitch_buffer=NULL;
		quit_pitch=1;
		if (pitch_control_hwnd)
		{
			DestroyWindow(pitch_control_hwnd);
			pitch_control_hwnd=0;
		}

	}
}

short echo_buf[65536], echo_buf2[65536];

int modify_samples1(struct winampDSPModule *this_mod, short int *samples, int numsamples, int bps, int nch, int srate)
{
	// echo doesn't support 8 bit right now cause I'm lazy.
	if (bps==16)
	{
        int x,s;
        s = numsamples*nch;

        memcpy(echo_buf2,       echo_buf,       s*2);
        memcpy(echo_buf,        echo_buf+s,     s*2);
        memcpy(echo_buf+s,      echo_buf+s*2, s*2);
        memcpy(echo_buf+s*2,echo_buf+s*3, s*2);
        memcpy(echo_buf+s*3,samples, s*2);

        for (x = 0; x < s; x ++)
        {
                int s = samples[x]/2+echo_buf2[x]/2;
                samples[x] = (s>32767?32767:s<-32768?-32768:s);
        }
	}
	return numsamples;
}

int modify_samples3(struct winampDSPModule *this_mod, short int *samples, int numsamples, int bps, int nch, int srate)
{
	int pitch=g_pitch;
	int rlen =numsamples*bps/8*nch;
	int index=0, x;
	int n; 
	int dindex; 
	if (quit_pitch || g_pitch==100) return numsamples;
	if (g_pitch > 200) g_pitch=200;
	if (g_pitch < 50) g_pitch=50;
	pitch = 100000/pitch;
	n=(numsamples*pitch)/1000;
	dindex=(numsamples<<14)/n;
	if (pitch_buffer_len < rlen) 
	{
		pitch_buffer_len = rlen;
		GlobalFree(pitch_buffer);
		pitch_buffer=GlobalAlloc(GMEM_FIXED,rlen);
	}
	if (bps == 16 && nch == 2)
	{
		short *buf=pitch_buffer;
		memcpy(buf,samples,rlen);
		for (x = 0; x < n; x ++)
		{
			int p=(index>>14)<<1;
			index+=dindex;
			samples[0] = buf[p];
			samples[1] = buf[p+1];
			samples+=2;
		}
		return n;
	}
	else if (bps == 16 && nch == 1)
	{
		short *buf=pitch_buffer;
		memcpy(buf,samples,rlen);
		for (x = 0; x < n; x ++)
		{
			int p=(index>>14);
			index+=dindex;
			*samples++ = buf[p];
		}
		return n;
	}
	return numsamples;
}


int modify_samples2(struct winampDSPModule *this_mod, short int *samples, int numsamples, int bps, int nch, int srate)
{
	int x = numsamples;
	if (bps == 16)
	{
		short *a = samples;
		if (nch == 2) while (x--)
		{
			int l, r;
			l = a[1]-a[0];
			r = a[0]-a[1];		
			if (l < -32768) l = -32768;
			if (l > 32767) l = 32767;
			if (r < -32768) r = -32768;
			if (r > 32767) r = 32767;
			a[0] = l;
			a[1] = r;
			a+=2;
		}
	}
	return numsamples;
}


static BOOL CALLBACK pitchProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG)
	{
		SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETRANGEMAX,0,18);
		SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETRANGEMIN,0,-18);
	    SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETPOS,1,1);
	    SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETPOS,1,0);
		{
			char str[123];
			wsprintf(str,"%s%d%%",g_pitch>100?"+":"",g_pitch-100);
			SetDlgItemText(hwndDlg,IDC_BOOGA,str);
		}
	}
	if (uMsg == WM_VSCROLL)
	{
		HWND swnd = (HWND) lParam;
		if (swnd == GetDlgItem(hwndDlg,IDC_SLIDER1))
		{
			g_pitch = -SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_GETPOS,0,0)+100;
			{
				char str[123];
				wsprintf(str,"%s%d%%",g_pitch>100?"+":"",g_pitch-100);
				SetDlgItemText(hwndDlg,IDC_BOOGA,str);
			}
		}
	}
	return 0;
}


