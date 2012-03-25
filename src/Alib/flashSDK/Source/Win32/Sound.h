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

#ifndef NATIVE_SOUND_INCLUDED
#define NATIVE_SOUND_INCLUDED

#ifdef SOUND

#include "../sndmix.h"
#include "../stdafx.h"

/* The Win32 code does not use polling, so it is inlined to nothing. */
inline void PollSound()	{}

/*! NativeSoundMix encapselates a hardware device. It has methods for
 *	opening and closing the device, managing data shared my multiple
 *	threads, and some buffer management. Sound is not completely portable.
 *	This interface may need to be modified for a particular OS. Please
 *	refer to the Design document.
 */
class NativeSoundMix : public CSoundMix
{
  public:
	/*! CSoundMix does not use a contructor. For compatibility with CSoundMix,
	 *	NativeConstruct is called for object "construction."
	 */
	virtual void NativeConstruct();

	/*! CSoundMix does not use a destructor. For compatibility with CSoundMix,
	 *	NativeDestruct is called for object "destruction."
	 */
	virtual void NativeDestruct();

	/*!	The sound data needs to be protected is multi-threaded systems. Only
	 *	one thread can have the critical semaphor. This call requests,
	 *	and blocks, access to the critical sound semaphor.
	 */
	virtual void EnterCritical();
	/*! */
	virtual void LeaveCritical();

	/*!	BuildAndWrite is passed the WaveHeader to an available buffer. It should call
	 *	BuildBuffer to repack the buffer and then send the buffer to the hardware 
	 *	device. If silence is true, BuildBuffer should not be called, and BuildAndWrite
	 *	should write silence to the buffer and send it to the hardware.<br>
	 *	BuildAndWrite is called from core code when the device is opened, and called
	 *	by the Native code when a buffer completes playing.
	 */
	virtual void BuildAndWrite( WaveHeader* hdr, BOOL silence );

	/*!	GetNativeSoundCap gets the best native sound format. Called inside a critical 
	 *  section, when the hardware device is closed. This format is OR'd
	 *	from the following variables:<br>
	 *	sndMono, sndStereo<br>
	 *	snd8Bit, snd16Bit<br>
	 *	snd5K, snd11K, snd22K, snd44K
	 */
	virtual void GetNativeSoundCap( S32* format );

	/*!	Opens the hardware device in the given format. This call
	 *	should not change CSound* format.
	 */
	virtual BOOL OpenNativeDevice( CSound* format );

	/*! BuffersReady is called when the device is open, the buffers are 
	 *	allocated, and the buffers are filled with data. A signal to 
	 *	start playing sound.
	 */ 
	virtual void BuffersReady();

	/*!	When the buffers are allocated or enlarged, BuffersUpdated is called
	 *	to inform the native code.
	 */
	virtual void BuffersUpdated();

	/*! Closes the hardware device.
	 */
	virtual void CloseNativeDevice();

	/*! Queries the state of the hardware device. Should be a fast call - it
	 *	may be prudent to buffer the device state rather than actually asking
	 *	the device.
	 */
	virtual BOOL IsDeviceOpen()			{ return isOpen; }

	//------------ Win32 Code -----------------------//
	void DoPlay();
	HANDLE GetPlayEvent()	{ return playEvent; }

  private:
	// Windows support
	HWAVEOUT			hWaveOut;
	WAVEHDR				waveHDR[MAX_SOUND_BUFFERS];
	CRITICAL_SECTION	playLock;
	HANDLE				playThread;
	HANDLE				playEvent;
	BOOL				isOpen;
};

// extern NativeSoundMix theSoundMix;

#endif
#endif