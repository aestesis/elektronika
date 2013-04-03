/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright © Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 
****************************************************************************/

#ifndef SPLAYER_INCLUDED
#define SPLAYER_INCLUDED

// Define dragbuttons for now...
#define DRAGBUTTONS
#define ACTIONSCRIPT

#include "splay.h"
#include "bitbuf.h"

#define MaxExecActions 200000

#include "genericfonts.h"

#ifdef EDITTEXT
#include "edittext.h"
#endif

int HexChar(int v);
int ParseHexChar(char c);

enum {
	streamProtocolHttp,
	streamProtocolFtp,
	streamProtocolLocalFile
};

/*! MenuStatus is filled and passed to the Native code in order to specify
 *	which menus should be displayed. Flash will make this call whenever the
 *	menu state changes. The first group (starting with view100percent) are 
 *  boolean true if the option should be enabled on the menu. The second group
 *	(starting with view100Check) are true if the item should be checked.
 */
struct MenuStatus
{
	BOOL view100percent;	//!< true if the movie can be set to 1:1 zoom
	BOOL viewAll;			//!< true if the movie can be stretched to fit
	BOOL zoomIn;			//!< true if the user can zoom in
	BOOL zoomOut;			//!< true if the user can zoom out
	BOOL play;				//!< true if the user can play a movie
	BOOL rewind;			//!< true if the current movie can be rewound
	BOOL forward;			//!< true if the user can step forward one frame
	BOOL back;				//!< true if the user can step backward one frame
	BOOL loop;				//!< true if the current movie can be looped

	BOOL view100Check;		//!< true if the movie is at 1:1 zoom
	BOOL viewAllCheck;		//!< true if the movie is stretched to fit the window
	BOOL highQChecked;		//!< true if high quality is on
	BOOL loopCheck;			//!< true if looping is on
	BOOL playCheck;			//!< true if playing
};

/*! The structure EditMenuStatus is used to pass the state of an 
	editable text field.
 */
struct EditMenuStatus
{
	BOOL selectAll;		//!< true if the "Select All" menu item is enabled
	BOOL copy;			//!< true if the "Copy" menu item is enabled
	BOOL cut;			//!< true if the "Cut" menu item is enabled
	BOOL paste;			//!< true if the "Paste" menu item is enabled
	BOOL clear;			//!< true if the "Clear" menu item is enabled
	BOOL undo;			//!< Not enabled. Always false.
};

struct SButton {
	SObject*	button;
	SRECT		buttonBounds;	

	SButton() : button(NULL)
	{
		memset(&buttonBounds, 0, sizeof(buttonBounds));	
	}

	SButton& operator=(const SButton& b)
	{
		button		 = b.button;
		buttonBounds = b.buttonBounds;
		return(*this);
	}

	SButton& operator=(const SButton* b)
	{
		if ( b ) {
			button		 = b->button;
			buttonBounds = b->buttonBounds;
		}
		return(*this);
	}
};

class Keyboard
{
public:
	enum {
		ID_KEY_LEFT        = 1,
		ID_KEY_RIGHT       = 2,
		ID_KEY_HOME        = 3,
		ID_KEY_END         = 4,
		ID_KEY_INSERT      = 5,
		ID_KEY_DELETE      = 6,
		ID_KEY_CLEAR       = 7,
		ID_KEY_BACKSPACE   = 8,
		ID_KEY_CUT		   = 9,
		ID_KEY_COPY		   = 10,
		ID_KEY_PASTE	   = 11,
		ID_KEY_SELECT_ALL  = 12,
		ID_KEY_ENTER	   = 13,
		ID_KEY_UP		   = 14,
		ID_KEY_DOWN		   = 15,
		ID_KEY_PAGE_UP	   = 16,
		ID_KEY_PAGE_DOWN   = 17,
		ID_KEY_TAB		   = 18,
		ID_KEY_ESCAPE	   = 19
	};
	enum {
		ID_KEY_SHIFT	   = 0x01,
		ID_KEY_CTRL		   = 0x02,
		ID_KEY_ALT		   = 0x04
	};
};

enum {
	CURSOR_TYPE_ARROW,		// the arrow
	CURSOR_TYPE_HAND,		// a picture of a hand
	CURSOR_TYPE_BUTTON,		// a hand with a button finger extended
	CURSOR_TYPE_BEAM		// a bar for text editing
};

/*! The parent class of NativePlayerWnd, SPlayer deals with events (mouse, keyboard), drawing,
 *	cursors, etc. at a "higher" OS-independant level. It aggregates the ScriptPlayer, DisplayList,
 *	and CBitBuffer, acting as a sort of "main" object.
 */
class SPlayer {
public:
   
	// Our player Objects
	ScriptPlayer player;		// note that this is always layer #0 and the root of our layer list
	DisplayList display;
	CBitBuffer bits;
	
	#ifdef SOUND
		#ifdef ONE_SOUND_OBJECT
			static NativeSoundMix theSoundMix;		// there will only be one NativeSoundMix
		#else
			NativeSoundMix theSoundMix;				// one NativeSoundMix per SPlayer
		#endif
	#endif

	BOOL highQuality;
	BOOL autoQuality;
	int lastQualitySwitch;
	BOOL bestDither;

	BOOL showMenu;

	// Sound prebuffer property
	int nSoundBufferTime;

	// Counters for autoquality settings
	int nTooSlow;
	int nTooFast;
	int nTotal;
 	//int starved;

	// Streaming (netscape) code
	BOOL firstStream;	// true if we have not gotten a NewStream callback yet

	int mode;
	char* url;
	char* urlBase;
	char* name;

	// View Control
	int scaleMode;
	SRECT zoomRect;

	SPOINT scrollPt;
	BOOL scrolling;
	BOOL destructing;

#ifdef ACTIONSCRIPT
	// Global execution stack
	char **stack;
	int stackPos, stackSize;

	// If this flag is set, the execution limit was
	// exceeded and no further actions will be processed
#endif

	BOOL actionOverflow;

#ifdef EDITTEXT
	// Text editing
	SObject* focus;
	SaveFocus saveFocus;
	BOOL cursorBlink;
	BOOL IsSelecting();
#endif

#if defined(EDITTEXT)
	ScriptPlayer builtInFontsPlayer;
	enum {
	  kBuiltInFontsNotLoaded,
	  kBuiltInFontsLoaded,
	  kBuiltInFontsError
	} builtInFontsState;
	BOOL LoadBuiltInFonts();
#endif

#ifdef DRAGBUTTONS
	SObject* dragObject;
	BOOL	 dragStarted;
	SPOINT   dragPoint;
#endif

	// Action control
	int actionDepth;

	// Background playing control
	//BOOL syncToClock;	//	set if we should be skipping frames to keep the animation tightly synchronized to the clock
	S32 nextFrameTime;	// the tick value to display the next frame

	BOOL loaded;		// set when the first frame is loaded
	BOOL running;		// set if the player is running

	// cached mouse info
	int mouseState;			// -1 = no inited, 0 = mouse up, 1 = mouse down
	SPOINT mousePt;

	S32 mTimerOffset;

public:
	SPlayer();
	virtual ~SPlayer();

	void SetVersionVariable( ScriptPlayer *player );

	enum { updateNone, updateNow, updateLazy };
	void SetCamera(int update);
	void FreeLayers();
	void ClearScript();
	void GotoFrame(int);
	int CurrentFrame() { return player.GetFrame(); }

	#ifdef EDITTEXT
	BOOL TabKeyDown(int key, int modifiers);
	void BlinkCursor();
	BOOL HandleKeyDown(int key, int modifiers);
	void HandleKeyUp(int key, int modifiers);
	#endif

	/*! Deletes the bitmap backing the window, invalidates the display. Flash will 
	 *  rebuild the bitmap with the current screen depth and palette.
	 */
	void FreeBuffer();
	BOOL UpdateBuffer(BOOL render = true);
	void UnlockBuffer() { bits.UnlockBits(); }

	ScriptPlayer* OpenLayer(int layerDepth, BOOL loadVars=false);
    ScriptPlayer* OpenSprite(ScriptThread *targetThread);
	ScriptPlayer* OpenVariableLoader(int layer, const char *dstSprite);
	void ClearLayer(int layerDepth);

	void DoActions();

	void Run();
	void Suspend();

	void Play(BOOL rewind=true) { player.Play(rewind); }
	void StopPlay() { player.StopPlay(); }

	void DoPlay(BOOL wait);	// a routine to call repeatedly in response to timer messages or null events
	
	void DoButton(SPOINT* pt, BOOL mouseIsDown, BOOL updateScreen = true);

	/*! MouseMove is a method provided for use by the native code. <br>
	 *  MouseMove should be called whenever the mouse position needs to be updated.
	 *	(The actual frequency should be often enough to update the mouse, but not so often as to
	 *	clog the system.) It should be called irrespective of whether the mouse button is down.
	 */
	void MouseMove( int x, int y, BOOL mouseIsDown);

	/*! MouseDown is a method provided for use by the native code. <br>
	 *	MouseDown should be called when the mouse button is pressed.
	 */
	void MouseDown( int x, int y );

	/*! MouseUp is a method provided for use by the native code. <br>
	 *	MouseUp should be called when the mouse button is released.
	 */
	void MouseUp( int x, int y );

	/*! CancelCapture is a method provided for use by the native code. <br>
	 *	CancelCapture should be called if the mouse capture needs to be aborted for 
	 *	some reason. outsideWindow is true if the mouse has left the window. (Elvis 
	 *	has left the building.)
	 */
	void CancelCapture(BOOL outsideWindow = true);

	void Home();
	void Zoom(SPOINT size, SPOINT center);
	void ZoomF(SFIXED factor, SPOINT* center = 0);
	//void Zoom(BOOL enlarge, SPOINT* center) { ZoomF(enlarge ? fixed1/2 : fixed1*2, center); }
	void Zoom(SRECT* r);
	void Zoom100();
	void ZoomScale(SFIXED scale);

	// Action Helpers
	void Rewind() {GotoFrame(0);}
	void Forward() {GotoFrame(CurrentFrame()+1);}
	void Back() {GotoFrame(CurrentFrame()-1);}

	// The Scripting API
	enum { panPixels = 0, panPercent = 1 };
	void Pan(long x, long y, int mode);
	void Zoom(int factor);
	void SetZoomRect(long left, long top, long right, long bottom);
	int TotalFrames() { return player.numFrames; }
	int PercentLoaded() { return (int)(player.len*100/player.scriptLen); }
	BOOL FrameLoaded(int frameNum) { return player.FrameComplete(frameNum); }
	BOOL IsPlaying() { return player.playing; }

	ScriptThread *ResolveVariable(char *name, ScriptThread *thread, char **varName);
	ScriptThread *ResolveFrameNum(char *name, ScriptThread *thread, int *frameNum);
	void SetVariable(ScriptThread *thread, char *name, char *value, BOOL updateFocus);
	char *GetVariable(ScriptThread *thread, char *name);
	ScriptThread* FindTarget(SObject* base, char* name);
	char* GetTargetPath(SObject* base);

#ifdef ACTIONSCRIPT
	void Push(char *value);
	char *Pop();
	void PushNum(double value);
	double PopNum();
	BOOL CallFrame(ScriptThread *thread, int frameNum);
#endif

	double GetProperty(ScriptThread *thread, int propType);
	char *GetPropertyStr(ScriptThread *thread, int propType);

	void SetProperty(ScriptThread *thread, int propType, double value);
	void SetPropertyStr(ScriptThread *thread, int propType, char *value);

 	void UpdateScreen();	// update the screen immediately

	/*! OnDraw is a method provided for use by the native code. <br>
	 *  OnDraw should be called when the play timer fires, or when
	 *	the screen should be rendered.
	 */
	void OnDraw();

	/*! OnDraw is a method provided for use by the native code. <br>
	 *	It is used to request a repaint of a screen rectangle. Usefull
	 *	for responding to re-paint requests from the OS.
	 */
	void Repaint( SRECT* rect );

	void CheckUpdate();		// generate an InvalidateRect if the screen needs redrawn

	virtual BOOL StartTimer( int playTimerInterval, int cursorTimerInterval ) = 0;		
	virtual void StopTimer() = 0;
	virtual void InvalidateScreenArea( SRECT* ) = 0;
	virtual void ClientRect(SRECT*) = 0;
    virtual void AdjustWindow( int width, int height ) = 0;

	// Internet, loading, and streaming functions
	char* BuildRelative(char* base, char* url);
	char* NSResolveURL(char* actionURL);
	void NSGetURL(char* actionURL, char* window, char* postData);
	void NetscapeDoURL(char* url, char* window, char* postData, LoadInfo* info );
	void LoadLayer(	char* url,
					int layer,
					char* postData,
					char *target,
					int loadMethod);

	/*! GetURL provides support for all URL retrieval. It is not generally used by Native code.
	 */
	void GetURL( char* url, char* target = "", char* postData = "", int loadMethod = 0);

	/*! StreamPostURLNotify is a call made to the Native code to start a Post.
	 *	It mimics the netscape call NPN_PostURLNotify. (See the plug-in guide
	 *	at http://developer.netscape.com/docs/manuals/ )
	 *	\param		url		URL of the POST request, specified by the plug-in.
	 *	\param		window	Target window. For values, see NPN_GetURL.
	 *	\param		len		Length of the buffer buf.
	 *	\param		buf		Path to local temporary file or data buffer that contains the data to post.
	 *	\param		notifydata    Plug-in-private value
	 */
	virtual void StreamPostURLNotify(	const char* url, 
										const char* window,
										U32			len, 
										const char* buf, 
										void*		notifyData
								   ) = 0;

	/*! StreamGetURLNotify is a call made to the Native code to start a Get.
	 *	It mimics the netscape call NPN_GetURLNotify (See the plug-in guide
	 *	at http://developer.netscape.com/docs/manuals/ )
	 *	\param		url		URL of the GET request, specified by the plug-in.
	 *	\param		window	Target window. For values, see NPN_GetURL.
	 *	\param		notifydata    Plug-in-private value	 */
	virtual void StreamGetURLNotify(	const char* url,
										const char* window,
										void*		notifyData
								   ) = 0;

	virtual void StreamGetResourceNotify(Aresobj *o, void *notifyData)=0;

	/*! A data stream is delivered to Flash in 3 steps:<br>
	 *	1) StreamInNew<br>
	 *	2) StreamInWrite<br>
	 *	3) StreamInDestroy<br>
	 *	Note that Flash has only one thread and these calls need to be on that thread; it may 
	 *	be necessary to perform synchronization in native code.<br>
	 *	StreamInNew is either a response to a POST or GET,
	 *	or can be used to initiate streaming (and loading a movie) without 
	 *  a request from Flash. If used to initiate streaming, it is possible
	 *  the incoming stream will confilct with the existing movie. There
	 *  are 2 solutions to this problem:<br>
	 *	1) only use StreamGetURLNotify to load the first movie - useful for plugins<br>
	 *  2) use ControlOpen to start streaming, which will clear existing movies and strings.<br>
	 *	StreamInNew is modeled after the netscape NPP_NewStream.
	 *	\param		streamData		Storage space for data -- used by Flash core.
	 *	\param		url				URL of the request, which was (usually) passed to the native code
									by StreamGetURLNotify.
	 *	\param		notifyData		The data passed in by StreamGetURLNotify -- null if the stream is
     *								initiated by the native code.
	 */
	void StreamInNew(	StreamData* streamData, 
						char*		  url,
						void*		  notifyData );

	/*!	StreamInWrite delivers a block of SWF data to the Flash core. Based on netscape NPP_Write.
	 *	Returns >= 0 to continue, return -1 if error
	 *	\param		streamData		Storage space for data -- used by Flash core.
	 *	\param		buffer			Pointer to a block o' SWF data.
	 *	\param		length			length of that block.
	 */
	int StreamInWrite(	StreamData*	streamData,
						void* buffer,
						int	  length );
						
	/*!	StreamInDestroy ends the delivery of SWF data to the Flash core. 
	 *	Based on netscape NPP_DestroyStream. Deallocates memory stored by streamData.
	 *	\param		streamData		Storage space for data -- used by Flash core.
	 */
	void StreamInDestroy( StreamData* streamData );	

	/*! If a stream needs to be axed mid-way (after the Stream in new, but before completion)
	 *  this call kills it. The most basic functionality of this call should be to set
	 *	streamData->scriptPlayer to NULL. That way, future calls to StreamInWrite and 
	 *	StreamInDestroy will not write SWF data. It is a good idea to call StreamInDestroy on the
	 *	closed stream to clean up memory.
	 */
	virtual void CloseStream( StreamData* streamData )	= 0;

	/*! FSCommand sends a command from the movie to the player. The ProcessFSCommand method call 
	 * is made to the NativePlayerWnd when it needs to process an FSCommand. 
	 * It has two parameters: Command and Argument. Since the FSCommand does something different
	 * on every host platform, there are a wide range of actions that FSCommand can cause. Please
	 * see the Flash 4 help file on FSCommand for a complete description.
	 */
	virtual void ProcessFSCommand( char* command, char* args ) = 0;

	virtual void EnableMenus( const MenuStatus& menuStatus ) = 0;

	/*! SetMenuState will determine the state of the menu options and call EnableMenus.
	 */
	void SetMenuState();						

	/*! GetMenuState puts the status (enabled, disabled, checked) of the various menu items into 
	 *  the MenuStatus structure. It has no side effects and can be called at any time.
	 */
	void GetMenuState( MenuStatus* menuStatus );

	#ifdef EDITTEXT
	/*! GetEditMenuState puts the status (enabled, disabled) of the edit text popup
	 *	menu items into the EditMenuStatus structure. It has no side effects and can be 
	 *  called at any time. If no field has the focus, all of the fields in the structure
	 *	will be false.
	 */
	void GetEditMenuState( EditMenuStatus* editMenuStatus );
	#endif

	enum {
		MENU_EDIT,
		MENU_NONE,
		MENU_NO_MOVIE,
		MENU_NORMAL,
		MENU_STATIC
	};	
	/*!	Based on the current mouse cursor, GetPopupMenuState assumes a popup menu has 
	 *	been requested and returns an enumerated for what type of menu it should be:
	 *	MENU_EDIT, MENU_NONE, MENU_NO_MOVIE, MENU_NORMAL, or MENU_STATIC. Please see
	 *	the design doc for a description of the various menu types.
	 */
	int	 GetPopupMenuState();

	/*! The Control* methods are provided for use by the native code. Their
	 *  functionality (and names) reflects menu item choices.<br>
	 *	ControlOpen opens and plays a movie given a URL to a local or remote object.
	 */
	void ControlOpen(class Aresobj *o);


	void ControlOpen( char* url );
	void ControlClose();						//!< Closes a playing movie
	void ControlPlay();							//!< Play or stop (toggle)
	void ControlRewind();						//!< Rewind the current movie to the beginning
	void ControlForward();						//!< Move the movie forward one frame
	void ControlBack();							//!< Move the movie backward one frame
	void ControlLoop();							//!< Sets the movie looping or non-looping
	void ControlViewAll();						//!< Sets the movie to fill the client window
	void Control100();							//!< Sets the movie to its actual size (100% zoom)
	void ControlZoomIn();						//!< Zoom in
	void ControlZoomContext( int x, int y );	//!< Zoom in, from context menu. An x and y coordinate to zoom to must be specified.
	void ControlZoomOut();						//!< Zoom out
	void ControlHighQuality();					//!< Toggles high quality on and off

	void LoadMovie(char* path);

	/*! Currently not used. Writes an "ideal" palette into ctab. Just a suggestion.
	 */
	void CreateIdealPalette( SColorTable* ctab );

	void XSetCapture()			{}		// These should not be needed if the client window always captures
	void XReleaseCapture()		{}		// the mouse when it is down.
	virtual BOOL UpdateCursor() = 0;

	/*! Query the current cursor type. Returns either 
	 *  CURSOR_TYPE_ARROW, CURSOR_TYPE_HAND, CURSOR_TYPE_BUTTON, CURSOR_TYPE_BEAM
	 */
	int GetCursorType();				// Returns the type of cursor that should be displayed. 
										// UpdateCursor() uses this.

//#endif

    void RemoveSprite(ScriptThread *thread, BOOL createPlaceholder = true);

#ifdef DRAGBUTTONS
	void UpdateDropTarget();
	void UpdateDragObject(SPOINT *pt);
#endif

	// Button interface

	void		ButtonGoDirection(int dir);
	void		ButtonTabNext();
	void		ButtonTabPrevious();

	void		ButtonEnter();

	BOOL		ButtonFind(SButton*, SButton*);

	// Button implementation

	class ButtonWeight {
	public:
		SObject*	button;
		SRECT		buttonBounds;
		S32			weight;
	
		ButtonWeight()
		: button(NULL), weight(0)
		{

		}
	};

	class ButtonOrder {
	public:
		SObject* button[4];
		SRECT bounds[4];

		ButtonOrder() { Reset(); }
		void Reset() { memset(this, 0, sizeof(ButtonOrder)); }
	};

	enum { UP = 0, DOWN = 1, RIGHT = 2, LEFT = 3 };

	SObject*	ButtonChoose(SObject*, P_SRECT, int, SObject*, P_SRECT, SObject*, P_SRECT);

	void		DoButton(SObject* pt, BOOL mouseIsDown, BOOL updateScreen = true);
	int			NumButtons(SObject*); 
	void		ButtonFocusSet(SObject*);
	void		ButtonFocusRemove(SObject*);
	
	void		ButtonOrdering(SButton*, ButtonOrder*);
	BOOL		IsButtonUp(P_SRECT, P_SRECT);
	BOOL		IsButtonDown(P_SRECT, P_SRECT);
	BOOL		IsButtonLeft(P_SRECT, P_SRECT);
	BOOL		IsButtonRight(P_SRECT, P_SRECT);
	BOOL		IsButtonAligned(P_SRECT, int, P_SRECT);
	BOOL		IsButtonOverlapped(P_SRECT, int, P_SRECT);

	P_SRECT		ButtonCloser(P_SRECT, P_SRECT, P_SRECT);

	// Methods used for tabbing implementation

	ButtonWeight* BuildButtonTabMap();

	void		ButtonSort(ButtonWeight* array, int size);
	S32			CalcButtonWeight(P_SRECT bounds);

	// Methods used to help enumerate all buttons

	void		AddButton(SObject* obj);
	void		RemoveAllButtons();

	// Data members

	ButtonOrder		 buttonOrder;
	SButton			 currentButton;

	SObject**		buttonArray;
	int				buttonIndex;
	int				arraySize;

	// from ShockwaveFlash.h
	SButton			m_lastButton;
	int				m_Focus;

protected:
// 	void  OnDraw( NativePlayerWnd* );
// 	void  LoadFile( URLWrapper *url, int layer, char *dstSprite = 0, char *postData = 0, BOOL loadVars = FALSE);
};

int LayerNum(char* s);

//
// A Helper Object that combines variables into a form suitable for URLs or a POST operation
//

class URLBuilder {
public:
	char* buf;
	int len;	// the first empty spot
	int size;

public:
	URLBuilder() { buf = 0; len = 0; size = 0; }
	~URLBuilder() { FreeStr(buf); }

	void AppendChar(char);
	void AppendInt(int);
	void AppendString(char*);
	void EncodeString(char*);

	void EncodeVariables(ScriptVariable* var);
};

void ExtractVariables(ScriptThread* thread, char* url);


#endif
