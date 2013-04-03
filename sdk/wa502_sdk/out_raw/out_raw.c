#include <windows.h>
#include <shlobj.h>
#include "out.h"

#define PI_VER2 "v1.0"

#ifdef __alpha
#define PI_VER PI_VER2 " (AXP)"
#else
#define PI_VER PI_VER2 " (x86)"
#endif

int getwrittentime();
void config_read();
void config_write();


char config_waveoutdir[MAX_PATH]="";
HANDLE hwavfile;
int srate, numchan, bps;
volatile int writtentime, w_offset;
Out_Module out;
static int last_pause=0;


static int CALLBACK WINAPI BrowseCallbackProc( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if (uMsg == BFFM_INITIALIZED)
	{
		SetWindowText(hwnd,"Select Directory");
		SendMessage(hwnd,BFFM_SETSELECTION,(WPARAM)1,(LPARAM)config_waveoutdir);
	}
	return 0;
}


BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) DisableThreadLibraryCalls(hInst);
	return TRUE;
}



void config(HWND hwnd)
{
	char name[MAX_PATH];
	BROWSEINFO bi;
	ITEMIDLIST *idlist;
	bi.hwndOwner = hwnd;
	bi.pidlRoot = 0;
	bi.pszDisplayName = name;
	bi.lpszTitle = "Select a directory for .RAW file output:";
	bi.ulFlags = BIF_RETURNONLYFSDIRS;
	bi.lpfn = BrowseCallbackProc;
	bi.lParam = 0;
	idlist = SHBrowseForFolder( &bi );
	if (idlist) {
		SHGetPathFromIDList( idlist, config_waveoutdir );
		config_write();
	}
}

void about(HWND hwnd)
{
	MessageBox(hwnd,"Nullsoft Raw Writer Plug-in " PI_VER "\n"
					"Copyright (c) 1998-1999 Nullsoft, Inc.\n\n"
					"Compiled on " __DATE__ "\n","About",MB_OK);
}

void init()
{
	config_read();
}

void quit()
{
}
char *scanstr_back(char *str, char *toscan, char *defval)
{
	char *s=str+strlen(str)-1;
	if (strlen(str) < 1) return defval;
	if (strlen(toscan) < 1) return defval;
	while (1)
	{
		char *t=toscan;
		while (*t)
			if (*t++ == *s) return s;
		t=CharPrev(str,s);
		if (t==s) return defval;
		s=t;
	}
}

int open(int samplerate, int numchannels, int bitspersamp, int bufferlenms, int prebufferms)
{
	char *t,*p;
	char temp2[MAX_PATH],temp[MAX_PATH];
	GetWindowText(out.hMainWindow,temp2,sizeof(temp2));
	t=temp2;

	t=scanstr_back(temp2,"-",NULL);
	if (t) t[-1]=0;

	if (temp2[0] && temp2[1] == '.')
	{
		char *p1,*p2;
		p1=temp;
		p2=temp2;
		while (*p2) *p1++=*p2++;
		*p1=0;
		p1 = temp2+1;
		p2 = temp;
		while (*p2)
		{
			*p1++ = *p2++;
		}
		*p1=0;
		temp2[0] = '0';
	}
	p=temp2;
	while (*p != '.' && *p) p++;
	if (*p == '.') 
	{
		*p = '-';
		p=CharNext(p);
	}
	while (*p)
	{
		if (*p == '.' || *p == '/' || *p == '\\' || *p == '*' || 
			*p == '?' || *p == ':' || *p == '+' || *p == '\"' || 
			*p == '\'' || *p == '|' || *p == '<' || *p == '>') *p = '_';
		p=CharNext(p);
	}

	p=config_waveoutdir;
	if (p[0]) while (p[1]) p++;

	if (!config_waveoutdir[0] || config_waveoutdir[0] == ' ')
		config(out.hMainWindow);
	if (!config_waveoutdir[0])
		wsprintf(temp,"%s.raw",temp2);
	else if (p[0]=='\\')
		wsprintf(temp,"%s%s.raw",config_waveoutdir,temp2);
	else
		wsprintf(temp,"%s\\%s.raw",config_waveoutdir,temp2);
	hwavfile = CreateFile(temp,GENERIC_READ|GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if (!hwavfile) return -1;

 	w_offset = writtentime = 0;
	numchan = numchannels;
	srate = samplerate;
	bps = bitspersamp;
	return 0;
}

void close()
{
	CloseHandle(hwavfile);
}

int write(char *buf, int len)
{
	int t;
	writtentime += len;
	WriteFile(hwavfile,buf,len,&t,NULL);
	Sleep(0);
	return 0;
}

int canwrite()
{
	return last_pause?0:16*1024*1024;
}

int isplaying()
{
	return 0;
}

int pause(int pause)
{
	int t=last_pause;
	last_pause=pause;
	return t;
}

void setvolume(int volume)
{
}

void setpan(int pan)
{
}

void flush(int t)
{
  int a;
  w_offset=0;
  a = t - getwrittentime();
  w_offset=a;
}
	
int getwrittentime()
{
	int t=srate*numchan,l;
	int ms=writtentime;

	l=ms%t;
	ms /= t;
	ms *= 1000;
	ms += (l*1000)/t;

	if (bps == 16) ms/=2;

	return ms + w_offset;
}

Out_Module out = {
	OUT_VER,
	"Nullsoft RAW Writer plug-in " PI_VER,
	33,
	0, // hmainwindow
	0, // hdllinstance
	config,
	about,
	init,
	quit,
	open,
	close,
	write,
	canwrite,
	isplaying,
	pause,
	setvolume,
	setpan,
	flush,
	getwrittentime,
	getwrittentime
};

__declspec( dllexport ) Out_Module * winampGetOutModule()
{
	return &out;
}
