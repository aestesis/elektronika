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

#ifndef _NATIVEPLAYERWND_H_
#define _NATIVEPLAYERWND_H_

#include "../splayer.h"
#include "flash.h"

/*!
 *	The NativePlayerWnd is the link between the OS messages/events and Flash. 
 *	It is the main objects, and creates or aggregrates almost all of the 
 *	Flash player.
 *	<br>
 *	In addition to the functions implemented below, NativePlayerWnd needs
 *	to react to menu selection (if relevant to your version of the player). Also,
 *	a SetMenuState call can be used at any time to trigger a EnableMenus call.
 *	The menus, and the resulting calls, are as follows:
 *	<br>
 *		FILE MENU:<br>
 *		Open calls ControlOpen( char* filenameToOpen )<br>
 *		Close calls ControlClose()<br>
 *		Exit handled by native code.<br>
 *		VIEW MENU:<br>
 *		100% calls Control100()<br>
 *		View All calls ControlViewAll()<br>
 *		Zoom In	calls ControlZoomIn()<br>
 *		Zoom Out calls ControlZoomOut()<br>
 *		Full Screen	handled by native code.<br>
 *		High Quality calls ControlHighQuality()<br>
 *		CONTROL MENU: <br>
 *		Play calls ControlPlay()<br>
 *		Rewind calls ControlRewind()<br>
 *		StepForward calls ControlForward()<br>
 *		StepBack calls ControlBack()<br>
 *		Loop calls ControlLoop()<br>
 */
class NativePlayerWnd : public SPlayer
{
  public:	
	class Aflash	*flash;
	int				flashOK;
	/*! */
	NativePlayerWnd(class Aflash *f);
	/*! */
	virtual ~NativePlayerWnd();	

	/*! There are two timers:<br>
	 *	1. PlayTimer, which fires every frame to redraw, and <br>
	 *	2. CursorTimer, which causes the edit text cursor to blink.<br>
	 *	How the timers work is left completely up to the implementation. When the PlayerTimer fires, it 
	 *  calls DoPlay( true ). When the CursorTimer fires, it calls BlinkCursor(). 
	 *  The time passed in to StartTimer is in milliseconds.
	 */
	virtual BOOL StartTimer( int playTimerInterval, int cursorTimerInterval );
	/*! Stops both timers. */
	virtual void StopTimer();													

	/*! Screen function to get the size of the client window (The client window is the
	 *  drawing area for Flash) into SRECT. NOTE: the returned rectangle
	 *	must be 0 based. (That is xmin == ymin == 0.)
	 */
	virtual void ClientRect( SRECT* );

	/*! Flash uses this call to request a cursor shape change. Update Cursor
	 *	calls GetCursorType() to get the description of the current cursor type.
	 */
	virtual BOOL UpdateCursor();	

	/*! Requests an OS repaint message be sent for the (0 based) SRECT passed in.
	 *	It is acceptabe to generate a message for the entire client window.
	 */
	virtual void InvalidateScreenArea( SRECT* );

	/*! Screen function to set the size of the client window. (The client window is the
	 *  drawing area for Flash.) If you have a fixed size window, meaning your client 
	 *  window can't or doesn't want to change size, this call should do nothing.
	 */
    virtual void AdjustWindow( int width, int height );

	/*! Flash makes this call and passes the state of the menus to the function
	 *	through the menuStatus parameter. An application that uses menus can
	 *  use this data to set them enabled, disabled, or checked. A call to 
	 *  SetMenuState will result in a call back to EnableMenus; use this to
	 *	request the EnableMenus call.
	 */
	virtual void EnableMenus( const MenuStatus& menuStatus );

	#ifdef EDITTEXT_CLIPBOARD_WIDE
		/*! Flash makes this call to gets a text string from the clipboard. May
			return NULL if there is none. If EDITTEXT_CLIPBOARD_WIDE is defined,
			then uses Unicode, else uses ascii. Note that in either format, Flash
			uses \n for a newline, and null to terminate the string. The native code
			should allocate the string and Flash core will delete it.
		 */
		U16*  GetStringFromClipboard();

		/*! Flash makes this call to set a text string from the clipboard. If 
			EDITTEXT_CLIPBOARD_WIDE is defined,	then uses Unicode, else uses ascii.
			Note that in either format, Flash uses \n for a newline, and null to 
			terminate the string.
		 */
		void  SetStringToClipboard( const U16* text );
	#else
		char* GetStringFromClipboard();
		void  SetStringToClipboard( const char* text );
	#endif

	/*! Streaming function. See the SPlayer::StreamPostURLNotify
	 *	reference for a full description. Note that the post is not
	 *	supported, at this time, in the demo version.
	 */
	virtual void StreamPostURLNotify(	const char* url, 
										const char* window,
										U32			len, 
										const char* buf, 
										void*		notifyData )	
					{}

	/*! Streaming function. See the SPlayer::StreamGetURLNotify
	 *	reference for a full description.
	 */
	virtual void StreamGetURLNotify(	const char* url,
										const char* window,
										void*		notifyData
								   );

	virtual void StreamGetResourceNotify(Aresobj *o, void *notifyData);


	/*! Streaming function. See the SPlayer::CloseStream
	 *	reference for a full description.
	 */	
	virtual void CloseStream( StreamData* streamData );

	/*! Streaming function -- in some cases. See the SPlayer::ProcessFSCommand
	 *	reference for a full description.
	 */	
	virtual void ProcessFSCommand( char* command, char* args );

	// ------ Public methods used by Win32				    --------
	// ------ These are NOT part of the standard interface. --------
	//void initialize( HWND hwnd );

	//HDC  GetNWindowDC();				// This is a call for the Flash code to use
	//void ReleaseWindowDC()		{}		// This is a call for the Flash code to use

//	void SetDC( HDC dc ) { windowDC = dc; }  // Only for use by the Window Message Handler

	int Width()					{ SRECT rect; ClientRect( &rect ); return ( rect.xmax - rect.xmin + 1 ); } 
	int Height()				{ SRECT rect; ClientRect( &rect ); return ( rect.ymax - rect.ymin + 1 ); }

	void PlayTimerFire();
	void CursorTimerFire();
	char* GetCurrentURL()		{ return currentURL; }

protected:
	void ExitApp()						{ }	
	void SetFullScreen( BOOL full )		{ }
	void SetScaleMode( BOOL scale )		{ scaleMode = scale ? DisplayList::showAll : DisplayList::noScale;
										  SetCamera(SPlayer::updateLazy);
										}
	void ExecSystem( const char* cmd)	{ WinExec(cmd, SW_SHOW); }

	//HPALETTE idealPalette;
	//HCURSOR	cursorArrow,
	//		cursorHand,
	//		cursorButton,
	//		cursorBeam;
	//HWND hwnd;
	int playTimer;
	int cursorTimer;
	//HDC windowDC;
	//HMENU savedMenu;	// If the menu is hidden, this stores the menu handle so it can be restored.
	char*	currentURL;	// The "current" url information. Used by GetURL.

};

DWORD WINAPI StreamThread(  LPVOID lpParameter );

#define MSG_STREAM_NEW     (WM_USER+0)
#define MSG_STREAM_WRITE   (WM_USER+1)
#define MSG_STREAM_DESTROY (WM_USER+2)

struct WriteBlock
{
	char data[1024];
	int size;
};

#endif
