/*
** Example Winamp .RAW input plug-in
** Copyright (c) 1998, Justin Frankel/Nullsoft Inc.
*/

#include <windows.h>
#include <mmreg.h>
#include <msacm.h>
#include <math.h>

#include "in2.h"

// avoid CRT. Evil. Big. Bloated.
BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}

// post this to the main window at end of file (after playback as stopped)
#define WM_WA_MPEG_EOF WM_USER+2

// raw configuration
#define NCH 2
#define SAMPLERATE 44100
#define BPS 16


In_Module mod; // the output module (declared near the bottom of this file)
char lastfn[MAX_PATH]; // currently playing file (used for getting info on the current file)
int file_length; // file length, in bytes
int decode_pos_ms; // current decoding position, in milliseconds
int paused; // are we paused?
int seek_needed; // if != -1, it is the point that the decode thread should seek to, in ms.
char sample_buffer[576*NCH*(BPS/8)*2]; // sample buffer

HANDLE input_file=INVALID_HANDLE_VALUE; // input file handle

int killDecodeThread=0;					// the kill switch for the decode thread
HANDLE thread_handle=INVALID_HANDLE_VALUE;	// the handle to the decode thread

DWORD WINAPI __stdcall DecodeThread(void *b); // the decode thread procedure

void config(HWND hwndParent)
{
	MessageBox(hwndParent,
		"No configuration. .RAW files must be 44/16/2",
		"Configuration",MB_OK);
	// if we had a configuration we'd want to write it here :)
}
void about(HWND hwndParent)
{
	MessageBox(hwndParent,"Nullsoft RAW Player, by Justin Frankel","About Nullsoft RAW Player",MB_OK);
}

void init() { /* any one-time initialization goes here (configuration reading, etc) */ }

void quit() { /* one-time deinit, such as memory freeing */ }

int isourfile(char *fn) { return 0; } 
// used for detecting URL streams.. unused here. strncmp(fn,"http://",7) to detect HTTP streams, etc

int play(char *fn) 
{ 
	int maxlatency;
	int thread_id;

	input_file = CreateFile(fn,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,
		OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if (input_file == INVALID_HANDLE_VALUE) // error opening file
	{
		return 1;
	}
	file_length=GetFileSize(input_file,NULL);

	strcpy(lastfn,fn);
	paused=0;
	decode_pos_ms=0;
	seek_needed=-1;

	maxlatency = mod.outMod->Open(SAMPLERATE,NCH,BPS, -1,-1);
	if (maxlatency < 0) // error opening device
	{
		CloseHandle(input_file);
		input_file=INVALID_HANDLE_VALUE;
		return 1;
	}
	// dividing by 1000 for the first parameter of setinfo makes it
	// display 'H'... for hundred.. i.e. 14H Kbps.
	mod.SetInfo((SAMPLERATE*BPS*NCH)/1000,SAMPLERATE/1000,NCH,1);

	// initialize vis stuff
	mod.SAVSAInit(maxlatency,SAMPLERATE);
	mod.VSASetInfo(SAMPLERATE,NCH);

	mod.outMod->SetVolume(-666); // set the output plug-ins default volume

	killDecodeThread=0;
	thread_handle = (HANDLE) CreateThread(NULL,0,(LPTHREAD_START_ROUTINE) DecodeThread,(void *) &killDecodeThread,0,&thread_id);
	
	return 0; 
}

void pause() { paused=1; mod.outMod->Pause(1); }
void unpause() { paused=0; mod.outMod->Pause(0); }
int ispaused() { return paused; }

void stop() { 
	if (thread_handle != INVALID_HANDLE_VALUE)
	{
		killDecodeThread=1;
		if (WaitForSingleObject(thread_handle,INFINITE) == WAIT_TIMEOUT)
		{
			MessageBox(mod.hMainWindow,"error asking thread to die!\n","error killing decode thread",0);
			TerminateThread(thread_handle,0);
		}
		CloseHandle(thread_handle);
		thread_handle = INVALID_HANDLE_VALUE;
	}
	if (input_file != INVALID_HANDLE_VALUE)
	{
		CloseHandle(input_file);
		input_file=INVALID_HANDLE_VALUE;
	}

	mod.outMod->Close();

	mod.SAVSADeInit();
}

int getlength() { 
	return (file_length*10)/(SAMPLERATE/100*NCH*(BPS/8)); 
}

int getoutputtime() { 
	return decode_pos_ms+(mod.outMod->GetOutputTime()-mod.outMod->GetWrittenTime()); 
}

void setoutputtime(int time_in_ms) { 
	seek_needed=time_in_ms; 
}

void setvolume(int volume) { mod.outMod->SetVolume(volume); }
void setpan(int pan) { mod.outMod->SetPan(pan); }

int infoDlg(char *fn, HWND hwnd)
{
	// TODO: implement info dialog. 
	return 0;
}

void getfileinfo(char *filename, char *title, int *length_in_ms)
{
	if (!filename || !*filename)  // currently playing file
	{
		if (length_in_ms) *length_in_ms=getlength();
		if (title) 
		{
			char *p=lastfn+strlen(lastfn);
			while (*p != '\\' && p >= lastfn) p--;
			strcpy(title,++p);
		}
	}
	else // some other file
	{
		if (length_in_ms) 
		{
			HANDLE hFile;
			*length_in_ms=-1000;
			hFile = CreateFile(filename,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,
				OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
			if (hFile != INVALID_HANDLE_VALUE)
			{
				*length_in_ms = (GetFileSize(hFile,NULL)*10)/(SAMPLERATE/100*NCH*(BPS/8));
			}
			CloseHandle(hFile);
		}
		if (title) 
		{
			char *p=filename+strlen(filename);
			while (*p != '\\' && p >= filename) p--;
			strcpy(title,++p);
		}
	}
}

void eq_set(int on, char data[10], int preamp) 
{ 
	// most plug-ins can't even do an EQ anyhow.. I'm working on writing
	// a generic PCM EQ, but it looks like it'll be a little too CPU 
	// consuming to be useful :)
}


// render 576 samples into buf. 
// note that if you adjust the size of sample_buffer, for say, 1024
// sample blocks, it will still work, but some of the visualization 
// might not look as good as it could. Stick with 576 sample blocks
// if you can, and have an additional auxiliary (overflow) buffer if 
// necessary.. 
int get_576_samples(char *buf)
{
	int l;
	ReadFile(input_file,buf,576*NCH*(BPS/8),&l,NULL);
	return l;
}

DWORD WINAPI __stdcall DecodeThread(void *b)
{
	int done=0;
	while (! *((int *)b) ) 
	{
		if (seek_needed != -1)
		{
			int offs;
			decode_pos_ms = seek_needed-(seek_needed%1000);
			seek_needed=-1;
			done=0;
			mod.outMod->Flush(decode_pos_ms);
			offs = (decode_pos_ms/1000) * SAMPLERATE;
			SetFilePointer(input_file,offs*NCH*(BPS/8),NULL,FILE_BEGIN);
		}
		if (done)
		{
			mod.outMod->CanWrite();
			if (!mod.outMod->IsPlaying())
			{
				PostMessage(mod.hMainWindow,WM_WA_MPEG_EOF,0,0);
				return 0;
			}
			Sleep(10);
		}
		else if (mod.outMod->CanWrite() >= ((576*NCH*(BPS/8))<<(mod.dsp_isactive()?1:0)))
		{	
			int l=576*NCH*(BPS/8);
			l=get_576_samples(sample_buffer);
			if (!l) 
			{
				done=1;
			}
			else
			{
				mod.SAAddPCMData((char *)sample_buffer,NCH,BPS,decode_pos_ms);
				mod.VSAAddPCMData((char *)sample_buffer,NCH,BPS,decode_pos_ms);
				decode_pos_ms+=(576*1000)/SAMPLERATE;
				if (mod.dsp_isactive()) l=mod.dsp_dosamples((short *)sample_buffer,l/NCH/(BPS/8),BPS,NCH,SAMPLERATE)*(NCH*(BPS/8));
				mod.outMod->Write(sample_buffer,l);
			}
		}
		else Sleep(20);
	}
	return 0;
}



In_Module mod = 
{
	IN_VER,
	"Nullsoft RAW Player v0.0 "
#ifdef __alpha
	"(AXP)"
#else
	"(x86)"
#endif
	,
	0,	// hMainWindow
	0,  // hDllInstance
	"RAW\0RAW Audio File (*.RAW)\0"
	,
	1,	// is_seekable
	1, // uses output
	config,
	about,
	init,
	quit,
	getfileinfo,
	infoDlg,
	isourfile,
	play,
	pause,
	unpause,
	ispaused,
	stop,
	
	getlength,
	getoutputtime,
	setoutputtime,

	setvolume,
	setpan,

	0,0,0,0,0,0,0,0,0, // vis stuff


	0,0, // dsp

	eq_set,

	NULL,		// setinfo

	0 // out_mod

};

__declspec( dllexport ) In_Module * winampGetInModule2()
{
	return &mod;
}
