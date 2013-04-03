/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright © Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 
****************************************************************************/

/****************************************************************************
  Porting Version of Flash 4 SDK
  Middlesoft, Inc. October 1999
  Release 10/22/1999 Lee Thomason, lee@middlesoft.com 
****************************************************************************/

#ifdef SOUND

#include "../global.h"
#include "sound.h"
#include "util.h"

U32 WINAPI SndPlayThread(LPVOID w)
{
	((NativeSoundMix*)w)->DoPlay();
	return 0;
}

void CALLBACK MixCallback(HWAVEOUT hwo,	UINT uMsg, U32 dwInstance, U32 dwParam1, U32 dwParam2)
{
	if ( uMsg == WOM_DONE ) {
		// The buffer is done playing
		NativeSoundMix* mixer = (NativeSoundMix*)dwInstance;

		// Track the number of sound buffers that have completed playing for this channel...
		int i = (int)(((WAVEHDR*)dwParam1)->dwUser);
	
		mixer->MarkBufferComplete( i );

		SetEvent(  mixer->GetPlayEvent() );	// signal the thread to quit
	}
}

void NativeSoundMix::NativeConstruct()
{
    hWaveOut = 0;
	isOpen	 = false;
	memset( &waveHDR, 0, sizeof( WAVEHDR ) * MAX_SOUND_BUFFERS );

	InitializeCriticalSection(&playLock);
	playThread = 0;
	playEvent = CreateEvent(0, false, false, 0);

	// Set the timer interval
	TIMECAPS tc;
	timeGetDevCaps(&tc, sizeof(tc));
	period = max(1, (int)tc.wPeriodMin);	// get the best possible resolution...
	timeBeginPeriod(period);
}

void NativeSoundMix::NativeDestruct()
{
	timeEndPeriod(period);
 	CloseHandle(playEvent);
}

void NativeSoundMix::GetNativeSoundCap( S32* format )
{
	// Check to capabilities of the waveform device
	WAVEOUTCAPS caps;
	waveOutGetDevCaps(WAVE_MAPPER, &caps, sizeof(caps));

	// the desired format
	*format = sndStereo | snd16Bit | snd22K;

	// Check for sample rate support
	if ( (caps.dwFormats & WAVE_FORMAT_4M16) == 0 ) {
		*format = (*format & ~sndRateMask) | min(*format & sndRateMask, snd22K);
	}

	if ( (caps.dwFormats & WAVE_FORMAT_2M16) == 0 ) {
		*format = (*format & ~sndRateMask) | min(*format & sndRateMask, snd11K);
	}

	// Check to turn off stereo support
	if ( (caps.dwFormats & WAVE_FORMAT_2S16) == 0 )
		*format &= ~sndStereo;

	// Check to turn off 16 bit sound
	if ( (caps.dwFormats & WAVE_FORMAT_2M16) == 0 )
		*format &= ~snd16Bit;
}

BOOL NativeSoundMix::OpenNativeDevice( CSound* format )
{
	if ( isOpen )
	{
		return true;
	}
	// Open a waveform device for output
	WAVEFORMATEX pcmWaveFormat;

  	pcmWaveFormat.wFormatTag		= WAVE_FORMAT_PCM;
 	pcmWaveFormat.nSamplesPerSec	= format->Rate();
 	pcmWaveFormat.nChannels			= format->NChannels();
 	pcmWaveFormat.wBitsPerSample	= format->BitsPerSample();
 	pcmWaveFormat.nBlockAlign		= ( format->BitsPerSample() * format->NChannels() ) / 8;
 	pcmWaveFormat.nAvgBytesPerSec	= pcmWaveFormat.nBlockAlign * pcmWaveFormat.nSamplesPerSec;

	if ( waveOutOpen(	&hWaveOut, 
							WAVE_MAPPER, 
							&pcmWaveFormat, 	
							(U32)MixCallback, 
							(U32)this, 
							CALLBACK_FUNCTION
				    ) == MMSYSERR_NOERROR )
	{
		isOpen = true;
		return true;
	}
	isOpen = false;
	return false;
}

void NativeSoundMix::CloseNativeDevice()
{
	if ( !isOpen ) return;

	SetEvent(playEvent);	// signal the thread to quit

	EnterCritical(); // do this after we kill the timer proc to prevent a deadlock

	if ( !isOpen ) {
		LeaveCritical();
		return;	// It is possible for timeKillEvent to call CloseDevice
	}
	isOpen = false;
	
	HWAVEOUT hW = hWaveOut;
	hWaveOut = 0;	// flag that we are shutting down so PollSound will bail

	int res = waveOutReset(hW);
	FLASHASSERT(res == MMSYSERR_NOERROR);

	for ( int i = 0; i < nBuffers; i++ ) {
		int limit = 50;
		while ( limit-- ) {
			int res = waveOutUnprepareHeader(hW, waveHDR+i, sizeof(WAVEHDR));
			if ( res != WAVERR_STILLPLAYING ) {
				FLASHASSERT(res == MMSYSERR_NOERROR);
				break;
			}
			Sleep(1);
		}
	}

	res = waveOutClose(hW);
	FLASHASSERT(res == MMSYSERR_NOERROR);

	CloseHandle(playThread);
	playThread = 0;
	LeaveCritical();
}

void NativeSoundMix::BuffersReady()
{
	U32 id;

	playThread = CreateThread(0, 0, SndPlayThread, this, 0, &id);

	int p = GetThreadPriority(playThread);
	SetThreadPriority(playThread, THREAD_PRIORITY_TIME_CRITICAL);
	p = GetThreadPriority(playThread);
	SetThreadPriority(playThread, 31);
}

void NativeSoundMix::DoPlay()
{
	while ( true ) {
		// Wait for a buffer done event
		WaitForSingleObject(playEvent, 10*1000);
		EnterCritical();
		if ( !hWaveOut || !playEvent ) {
			LeaveCritical();
			return;
		}

		// If a buffer is done, fill it...
		int nSent = 0;
		for ( int i = 0; i < nBuffers; i++ ) {
			WAVEHDR* hdr = waveHDR+i;
			if ( (hdr->dwFlags & WHDR_DONE) || !(hdr->dwFlags & WHDR_PREPARED) ) {
				if ( buffer[i] )
				{
	 				BuildAndWrite( &waveHdr[i], false );
				}
// 				SendBuffer(i);
				nSent++;
			}
		}

		if ( nSent > nBuffers-2 ) 
			EnlargeBuffers();

		if ( nSilent > nBuffers ) 
			CloseDevice();
		LeaveCritical();
	}
}

void NativeSoundMix::BuildAndWrite( WaveHeader* wave, BOOL silence )
{
	int i = wave->bufferId;
	WAVEHDR* hdr = &waveHDR[i];

	// Clean up the header
	int res = waveOutUnprepareHeader(hWaveOut, hdr, sizeof(WAVEHDR));
	FLASHASSERT(res == MMSYSERR_NOERROR);

	// Generate the sound info
	if ( silence ) {
		memset(buffer[i], mixFormat.Is8Bit() ? 0x80 : 0, bufferBytes);
	} else {
		BuildBuffer( i );
	}

	res = waveOutPrepareHeader(hWaveOut, hdr, sizeof(WAVEHDR));
	FLASHASSERT(res == MMSYSERR_NOERROR);
	res = waveOutWrite(hWaveOut, hdr, sizeof(WAVEHDR)); 
	FLASHASSERT(res == MMSYSERR_NOERROR);
}

void NativeSoundMix::BuffersUpdated()
{
	for ( int i = 0; i < MAX_SOUND_BUFFERS; i++ )
	{
		waveHDR[i].lpData	= (char*) buffer[i];
		waveHDR[i].dwUser	= i;			// Put in the buffer id in the user space
		waveHDR[i].dwBufferLength = bufferBytes;
	}
}

void NativeSoundMix::EnterCritical()	{ EnterCriticalSection(&playLock); }
void NativeSoundMix::LeaveCritical()	{ LeaveCriticalSection(&playLock); }

#endif