#include "export.h"
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

#include "../stdafx.h"
#include "version.h"

#include "resource.h"

#include "playerwnd.h"

#include "../global.h"
#include "../stags.h"
#include "../sobject.h"
#include "../memcop.h"
#include "../edittext.h"

#include "../mcontext.h"
#include "util.h"

#include "stdio.h"
#include "resources.h"

// Test defines - for file streaming.
// #define KILOBYTESPERSEC		4			// How many kbytes per sec - set to 0 for fastest
// #define SIMULATE_STREAMING				// When defined, loads files piece by piece on seperate thread,
										// when not defined, loads the entire file from disk

struct ThreadData
{
	HWND		hwnd;
	FILE*		fp;
	void*		notifyData;
	char*		url;
	StreamData	streamData;
};

#define MAX_LOADSTRING 100
#define TIMER_PLAY		1
#define TIMER_CURSOR	2

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// The title bar text

// Foward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
//LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

/*
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SIMPLEWIN, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_SIMPLEWIN);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return msg.wParam;
}
*/


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage is only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//

/*
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_SIMPLEWIN);
	wcex.hCursor		= NULL; //LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL; //(HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCSTR)IDC_SIMPLEWIN;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

*/

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
/*
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}
*/

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//

/*
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static NativePlayerWnd* flashWin = 0;
	static OPENFILENAME ofn;
	static 	SPOINT	contextPoint;

	char szFileName[_ALIB_PATH];
	char szFileTitle[_ALIB_PATH];

	int wmId, wmEvent;
	PAINTSTRUCT ps;
	TCHAR szHello[MAX_LOADSTRING];
	LoadString(hInst, IDS_HELLO, szHello, MAX_LOADSTRING);

	switch (message) 
	{
		case WM_CREATE:
			{
				MenuStatus menuStatus;

				flashWin = new NativePlayerWnd();
				flashWin->initialize( hWnd );

				flashWin->GetMenuState( &menuStatus );
				flashWin->EnableMenus( menuStatus );

				#ifdef FIXEDSIZE
				SetWindowPos( hWnd, NULL, 0, 0, 300, 300, SWP_NOZORDER | SWP_NOMOVE );
				#endif
			}
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			delete flashWin;
			flashWin = 0;
			break;

		case WM_TIMER:
			flashWin->SetDC( GetDC( hWnd ) );			// the mouse sometimes causes a draw

			wmId = wParam;             // timer identifier 
			if ( wmId == TIMER_PLAY )
				flashWin->PlayTimerFire();
			else if ( wmId == TIMER_CURSOR )
				flashWin->CursorTimerFire();
			else
				FLASHASSERT( 0 );

			ReleaseDC( hWnd, flashWin->GetNWindowDC() );
			flashWin->SetDC( 0 );			// the mouse sometimes causes a draw
			break;

		#ifdef EDITTEXT
		case WM_KEYDOWN:
			{
				static BYTE vkCodes[] = {
					VK_LEFT,	//ID_KEY_LEFT
					VK_RIGHT,	//ID_KEY_RIGHT
					VK_HOME,	//ID_KEY_HOME
					VK_END,		//ID_KEY_END
					VK_INSERT,	//ID_KEY_INSERT
					VK_DELETE,	//ID_KEY_DELETE
					0,			//ID_KEY_CLEAR
					VK_BACK,	//ID_KEY_BACKSPACE
					0,			//ID_KEY_CUT		
					0,			//ID_KEY_COPY		
					0,			//ID_KEY_PASTE	
					0,			//ID_KEY_SELECT_ALL
					VK_RETURN,	//ID_KEY_ENTER	
					VK_UP,		//ID_KEY_UP		
					VK_DOWN,	//ID_KEY_DOWN		
					VK_PRIOR,	//ID_KEY_PAGE_UP	
					VK_NEXT,	//ID_KEY_PAGE_DOWN
					VK_TAB,		//ID_KEY_TAB		
					VK_ESCAPE	//ID_KEY_ESCAPE
				};
				int modifiers = 0;

				if (GetKeyState(VK_SHIFT) < 0) {
					modifiers |= Keyboard::ID_KEY_SHIFT;
				}
				if (GetKeyState(VK_CONTROL) < 0) {
					modifiers |= Keyboard::ID_KEY_CTRL;
				}
				BOOL handled = FALSE;
				for (int i=0; i<sizeof(vkCodes); i++) {
					if (wParam == vkCodes[i]) {
						handled = flashWin->HandleKeyDown(i+1, modifiers);
						break;
					}
				}
			}
			break;

		case WM_CHAR:
			if ( wParam != VK_TAB && wParam != VK_RETURN && wParam != VK_BACK) {
				flashWin->HandleKeyDown( wParam, 0 );
			}
			break;

		#endif

		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 
			// Parse the menu selections:
			switch (wmId)
			{
				#ifdef EDITTEXT
				case IDM_CUT:
					flashWin->HandleKeyDown(Keyboard::ID_KEY_CUT, 0);
					break;

				case IDM_COPY:
					flashWin->HandleKeyDown(Keyboard::ID_KEY_COPY, 0);
					break;

				case IDM_PASTE:
					flashWin->HandleKeyDown(Keyboard::ID_KEY_PASTE, 0);
					break;

				case IDM_CLEAR:
					flashWin->HandleKeyDown(Keyboard::ID_KEY_CLEAR, 0);
					break;

				// Not implemented.
				case IDM_UNDO:
					break;

				case IDM_SELECTALL:
					flashWin->HandleKeyDown(Keyboard::ID_KEY_SELECT_ALL, 0);
					break;
				#endif

				case IDM_ABOUT:
				   DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
				   break;

				case IDM_EXIT:
				   DestroyWindow(hWnd);
				   break;

				case IDM_OPEN:
					{
						FLASHOUTPUT( "---Test!----\n", CommDlgExtendedError() );

						memset(&ofn, 0, sizeof(ofn)); // initialize structure to 0/NULL

						strcpy( &szFileName[0], "*.swf" );
						szFileTitle[0] = 0;
						
						char* szFilter = "*.swf\0";

						ofn.lpstrFilter = szFilter;
						ofn.lStructSize = sizeof(ofn);
						ofn.lpstrFile = szFileName;
						ofn.nMaxFile = 255;
						ofn.lpstrDefExt = "swf";
						ofn.lpstrFileTitle = szFileTitle;
						ofn.nMaxFileTitle = 255;
						ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
			// 			#ifdef WIN32
			// 			if ( osInfo.dwMajorVersion > 3 )
			// 				ofn.Flags |= OFN_EXPLORER;
			// 			#endif
						ofn.nFilterIndex = 0;
						ofn.hwndOwner = hWnd;
						ofn.hInstance = 0;

						if ( GetOpenFileName( &ofn ) )
						{
							flashWin->ControlOpen( ofn.lpstrFile );
						}
						FLASHOUTPUT( "Return from Open=%d\n", CommDlgExtendedError() );
					}
					break;

				case IDM_CLOSE:
					flashWin->ControlClose();
					break;
					
				case IDM_100:
					flashWin->Control100();
					break;

				case IDM_ZOOMIN:
					flashWin->ControlZoomIn();
					break;

				case IDM_ZOOMIN_CONTEXT:
					flashWin->ControlZoomContext( contextPoint.x, contextPoint.y );
					break;					

				case IDM_ZOOMOUT:
					flashWin->ControlZoomOut();
					break;

				case IDM_VIEWALL:
					flashWin->ControlViewAll();
					break;

				case IDM_FULLSCREEN:
					ShowWindow( hWnd, SW_MAXIMIZE );
					break;

				case IDM_HIGHQ:
					flashWin->ControlHighQuality();
					break;

				case IDM_PLAY:
					flashWin->ControlPlay();
					break;

				case IDM_REWIND:
					flashWin->ControlRewind();
					break;

				case IDM_FORWARD:
					flashWin->ControlForward();
					break;

				case IDM_BACK:
					flashWin->ControlBack();
					break;

				case IDM_LOOP:
					flashWin->ControlLoop();
					break;

				default:
				   return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;


		case WM_PAINT:
			flashWin->SetDC( BeginPaint(hWnd, &ps) );

			SRECT rect;
			rect.xmin = ps.rcPaint.left;
			rect.ymin = ps.rcPaint.top;
			rect.xmax = ps.rcPaint.right;
			rect.ymax = ps.rcPaint.bottom;
			FLASHOUTPUT( "Repaint Rect (%d,%d)--(%d,%d)\n", rect.xmin, rect.ymin, rect.xmax, rect.ymax );

			flashWin->Repaint( &rect );

			flashWin->SetDC( 0 );
			EndPaint(hWnd, &ps);
			return 0;

		case WM_SIZE:
			flashWin->FreeBuffer();
			break;

		case WM_SETCURSOR: 
			{
				int nHittest = LOWORD(lParam);
				if ( nHittest == HTCLIENT && flashWin->UpdateCursor() )
					return true;
				else
					return DefWindowProc(hWnd, message, wParam, lParam);
			} 
			break;

		case WM_MOUSEMOVE:
			{
				int fwKeys = wParam;        // key flags 
				int xPos = LOWORD(lParam);  // horizontal position of cursor 
				int yPos = HIWORD(lParam);  // vertical position of cursor 

				flashWin->SetDC( GetDC( hWnd ) );			// the mouse sometimes causes a draw
				flashWin->MouseMove( xPos, yPos, fwKeys & MK_LBUTTON );
				ReleaseDC( hWnd, flashWin->GetNWindowDC() );
				flashWin->SetDC( 0 );
			}
			break; 						

		case WM_LBUTTONDOWN:
			{
				int xPos = LOWORD(lParam);  // horizontal position of cursor 
				int yPos = HIWORD(lParam);  // vertical position of cursor 

				SetCapture( hWnd );

				flashWin->SetDC( GetDC( hWnd ) );			// the mouse sometimes causes a draw
				flashWin->MouseDown( xPos, yPos );
				ReleaseDC( hWnd, flashWin->GetNWindowDC() );
				flashWin->SetDC( 0 );
			}
			break; 						
 
		case WM_LBUTTONUP:
			{
				int xPos = LOWORD(lParam);  // horizontal position of cursor 
				int yPos = HIWORD(lParam);  // vertical position of cursor 
				ReleaseCapture();

				flashWin->SetDC( GetDC( hWnd ) );			// the mouse sometimes causes a draw
				flashWin->MouseUp( xPos, yPos );
				ReleaseDC( hWnd, flashWin->GetNWindowDC() );
				flashWin->SetDC( 0 );
			}
			break; 				

		case WM_RBUTTONDOWN: 
			{
				const int MENUSIZE = 16;		// Sleazy magic numbers to give the code some facade of
				const int STATIC_CUTOFF = 6;	// respectablility.

				MenuStatus menuStatus;
				int i;
				int menuType = flashWin->GetPopupMenuState();
				HMENU hMenu;

				if ( menuType == SPlayer::MENU_EDIT )
				{	
					#ifdef EDITTEXT
					EditMenuStatus editMenuStatus;
					flashWin->GetEditMenuState( &editMenuStatus );

					// Load the text menu - all options are always active.
					hMenu  = LoadMenu(	hInst, MAKEINTRESOURCE( IDR_TEXT ) );

					EnableMenuItem(hMenu, IDM_UNDO, editMenuStatus.undo ? MF_ENABLED : MF_GRAYED);
					EnableMenuItem(hMenu, IDM_CUT, editMenuStatus.cut ? MF_ENABLED : MF_GRAYED);
					EnableMenuItem(hMenu, IDM_COPY, editMenuStatus.copy ? MF_ENABLED : MF_GRAYED);
					EnableMenuItem(hMenu, IDM_PASTE, editMenuStatus.paste ? MF_ENABLED : MF_GRAYED);
					EnableMenuItem(hMenu, IDM_CLEAR, editMenuStatus.clear ? MF_ENABLED : MF_GRAYED);
					EnableMenuItem(hMenu, IDM_SELECTALL, editMenuStatus.selectAll ? MF_ENABLED : MF_GRAYED);
					#endif
				}
				else
				{
					hMenu  = LoadMenu(	hInst, MAKEINTRESOURCE( IDR_MOVIE ) );
					flashWin->GetMenuState( &menuStatus );

					EnableMenuItem(hMenu, IDM_100, menuStatus.view100percent ? MF_ENABLED : MF_GRAYED);
					CheckMenuItem(hMenu, IDM_100, menuStatus.view100Check ? MF_CHECKED : MF_UNCHECKED);

					EnableMenuItem(hMenu, IDM_PLAY, menuStatus.play ? MF_ENABLED : MF_GRAYED);
					EnableMenuItem(hMenu, IDM_REWIND, menuStatus.rewind ? MF_ENABLED : MF_GRAYED);
					EnableMenuItem(hMenu, IDM_FORWARD, menuStatus.forward ? MF_ENABLED : MF_GRAYED);
					EnableMenuItem(hMenu, IDM_BACK, menuStatus.back ? MF_ENABLED : MF_GRAYED);
					EnableMenuItem(hMenu, IDM_LOOP, menuStatus.loop ? MF_ENABLED : MF_GRAYED);

					CheckMenuItem(hMenu, IDM_PLAY, menuStatus.playCheck ? MF_CHECKED : MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_LOOP, menuStatus.loopCheck ? MF_CHECKED : MF_UNCHECKED);

 					EnableMenuItem(hMenu, IDM_VIEWALL, menuStatus.viewAll ? MF_ENABLED : MF_GRAYED);
					CheckMenuItem(hMenu, IDM_VIEWALL,  menuStatus.viewAllCheck ? MF_CHECKED : MF_UNCHECKED);

					EnableMenuItem(hMenu, IDM_ZOOMIN_CONTEXT, menuStatus.zoomIn ? MF_ENABLED : MF_GRAYED);
					EnableMenuItem(hMenu, IDM_ZOOMOUT, menuStatus.zoomOut ? MF_ENABLED : MF_GRAYED);

					CheckMenuItem(hMenu, IDM_HIGHQ, menuStatus.highQChecked ? MF_CHECKED : MF_UNCHECKED);
				}				

				HMENU hPopUp = GetSubMenu(hMenu, 0);

				if ( menuType == SPlayer::MENU_NONE )
				{
					// Delete everything we don't need...which is everything but "About..."
					for ( i=0; i < (MENUSIZE-1); i++ )
					{
						DeleteMenu( hPopUp, 0, MF_BYPOSITION );
					}
				}
				else if ( menuType == SPlayer::MENU_NO_MOVIE )
				{
					// As above, but leaves in "Movie not loaded" text and separator
					for ( i=0; i < (MENUSIZE-3); i++ )
					{
						DeleteMenu( hPopUp, 0, MF_BYPOSITION );
					}
				}
				else if ( menuType == SPlayer::MENU_NORMAL )
				{
					// All menus available, except the "Movie not loaded..." line
					DeleteMenu(hPopUp, ID_X_COULDNOTLOADIMAGE, MF_BYCOMMAND);
				}
				else if (  menuType == SPlayer::MENU_STATIC )
				{
					// Take out everything from the separator under HQ to the "Movie not loaded..."
					for ( i=STATIC_CUTOFF; i < (MENUSIZE-2); i++ )
					{
						DeleteMenu( hPopUp, STATIC_CUTOFF, MF_BYPOSITION );
					}
				}

				// Track the pop-up menu
				POINT pt;
				pt.x = (S16)LOWORD(lParam);
				pt.y = (S16)HIWORD(lParam);
				contextPoint.x = pt.x;
				contextPoint.y = pt.y;

				ClientToScreen(hWnd, &pt);
				TrackPopupMenu(hPopUp, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, 0);
				DestroyMenu(hMenu);

				return 0;
			} 

		case WM_ACTIVATEAPP:
			{
				FLASHOUTPUT( "Activated\n" );
				if ( BOOL( wParam ) )
				{
					HPALETTE hPalIdeal = NativePalette::CreateIdentityPalette( hWnd );
					HDC hDC = GetDC(hWnd);

					// Realize our ideal palette to be sure all the colors are used in the device palette
					HPALETTE hOldPal = SelectPalette(hDC, hPalIdeal, FALSE);
					RealizePalette(hDC);

					// Restore the old palette
					SelectPalette(hDC, hOldPal, TRUE);
					
					ReleaseDC(hWnd, hDC);
					DeleteObject(hPalIdeal);
				}
			}
			break;
			

		case WM_PALETTECHANGED:
		    if ( HWND( wParam ) == hWnd )
				break;

		// FALL THROUGH

		case WM_QUERYNEWPALETTE:
			{
				FLASHOUTPUT( "QueryNewPalette\n" );
				HPALETTE hPalIdeal = NativePalette::CreateIdentityPalette( hWnd );
				if ( hPalIdeal ) {
					HDC hDC = GetDC(hWnd);

					// Realize our ideal palette to be sure all the colors are used in the device palette
					HPALETTE hOldPal = SelectPalette(hDC, hPalIdeal, FALSE);
					int result = RealizePalette(hDC);

					// Restore the old palette
					SelectPalette(hDC, hOldPal, TRUE);
// 					RealizePalette(hDC);
					
					ReleaseDC(hWnd, hDC);
					DeleteObject(hPalIdeal);

					if ( result )
					{
						flashWin->FreeBuffer();
					}
				}
			}
			return 0;

		case MSG_STREAM_NEW:
			{
				ThreadData* data = (ThreadData*) wParam;
				flashWin->StreamInNew( &data->streamData, data->url, (void*) lParam );
			}
			break;

		case MSG_STREAM_WRITE:
			{
				ThreadData* data = (ThreadData*) wParam;
				WriteBlock* write = (WriteBlock*) lParam;

				flashWin->StreamInWrite( &data->streamData, write->data, write->size );
				delete ( write );
			}
			break;

		case MSG_STREAM_DESTROY:
			{
				ThreadData* data = (ThreadData*) wParam;

				flashWin->StreamInDestroy( &data->streamData );
				FreeStr( data->url );
				delete ( data );
			}
			break;

 		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

*/

// Mesage handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char version[256];

	switch (message)
	{
		case WM_INITDIALOG:
			sprintf( version, "%s %s", PLATFORM_STRING, VERSION_STRING );
			SetWindowText( GetDlgItem( hDlg, IDC_VERSION ), version );
			return TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
	}
    return FALSE;
}


//-----------------------------------------------------------------------------
// So much for the Windows stuff. NativePlayerWnd follows.
//-----------------------------------------------------------------------------

NativePlayerWnd::NativePlayerWnd(Aflash *f)
{
	flash=f;
	flashOK=666;
	currentURL = CreateStr( "" );
}

NativePlayerWnd::~NativePlayerWnd()
{
	ClearScript();
	delete currentURL;
}

/*
void NativePlayerWnd::initialize( HWND _hwnd )
{
	hwnd = _hwnd;

	cursorArrow		= LoadCursor( NULL, IDC_ARROW );
	cursorHand		= LoadCursor( hInst, MAKEINTRESOURCE(FID_HAND) );
	cursorButton	= LoadCursor( hInst, MAKEINTRESOURCE(FID_BUTTON) );
	cursorBeam		= LoadCursor( NULL, IDC_IBEAM );
}
*/

void NativePlayerWnd::PlayTimerFire()
{
	DoPlay( true );
}

void NativePlayerWnd::CursorTimerFire()
{
	#ifdef EDITTEXT
	BlinkCursor();
	#endif
}

void NativePlayerWnd::ClientRect( SRECT* rect )
{
	if(flash)
	{
		rect->xmin = 0;
		rect->xmax = flash->pos.w-1;
		rect->ymin = 0;
		rect->ymax = flash->pos.h-1;
	}
}

void NativePlayerWnd::InvalidateScreenArea( SRECT* r)
{
	// the simple solution is too send a repaint for the whole thing
	//flash->update(r->xmin, r->ymin, r->xmax-r->xmin, r->ymax-r->ymin);
	//InvalidateRect( hwnd, NULL, false );
}

void NativePlayerWnd::AdjustWindow( int width, int height )
{	
	#ifdef FIXEDSIZE
		// Then we don't adjust our size.
	#else
		//flash->size(width, height);
	#endif
}

BOOL NativePlayerWnd::StartTimer( int playTimerInterval, int cursorTimerInterval )
{
	if ( !loaded ) 
		return false;

	FLASHASSERT( playTimerInterval );
	FLASHASSERT( cursorTimerInterval );
	
	flash->cursorTimeInterval=cursorTimerInterval;
	flash->cursorTime=GetTickCount();
	playTimer = flash->timer(playTimerInterval);
		//SetTimer( hwnd, TIMER_PLAY, playTimerInterval, 0 );
	//cursorTimer = SetTimer( hwnd, TIMER_CURSOR, cursorTimerInterval, 0 );

	return true;	
}

void NativePlayerWnd::StopTimer()
{
	//KillTimer( hwnd, playTimer );
	//KillTimer( hwnd, cursorTimer );
}

void NativePlayerWnd::EnableMenus( const MenuStatus& menuStatus )
{
/*
	HMENU menu = GetMenu( hwnd );
	EnableMenuItem(menu, IDM_100, menuStatus.view100percent ? MF_ENABLED : MF_GRAYED);
	CheckMenuItem(menu, IDM_100, menuStatus.view100Check ? MF_CHECKED : MF_UNCHECKED);

	EnableMenuItem(menu, IDM_PLAY, menuStatus.play ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(menu, IDM_REWIND, menuStatus.rewind ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(menu, IDM_FORWARD, menuStatus.forward ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(menu, IDM_BACK, menuStatus.back ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(menu, IDM_LOOP, menuStatus.loop ? MF_ENABLED : MF_GRAYED);

	CheckMenuItem(menu, IDM_PLAY, menuStatus.playCheck ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(menu, IDM_LOOP, menuStatus.loopCheck ? MF_CHECKED : MF_UNCHECKED);

 	EnableMenuItem(menu, IDM_VIEWALL, menuStatus.viewAll ? MF_ENABLED : MF_GRAYED);
	CheckMenuItem(menu, IDM_VIEWALL,  menuStatus.viewAllCheck ? MF_CHECKED : MF_UNCHECKED);

	EnableMenuItem(menu, IDM_ZOOMIN, menuStatus.zoomIn ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(menu, IDM_ZOOMOUT, menuStatus.zoomOut ? MF_ENABLED : MF_GRAYED);

	CheckMenuItem(menu, IDM_HIGHQ, menuStatus.highQChecked ? MF_CHECKED : MF_UNCHECKED);
*/
}


#ifdef DEBUG
void DumpTree(SObject* root, int depth)
{
	for ( int i = 0; i < depth; i++ )
		FLASHOUTPUT(" ");
	FLASHOUTPUT("Type=%i Name=%s addr=%X\n", root->character ? root->character->type : -1, root->name ? root->name : "", root);
	for ( SObject* obj = root->bottomChild; obj; obj = obj->above )
		DumpTree(obj, depth+1);
}
#endif


void NativePlayerWnd::ProcessFSCommand( char* cmd, char* args )
{
	if ( StrEqual(cmd, "quit") )
	{
		// Quit
		ExitApp();
	}
	else if ( StrEqual(cmd, "fullscreen") )
	{
		SetFullScreen( IsTrue(args) != 0 );
	}
	else if ( StrEqual(cmd, "allowscale") )
	{
		SetScaleMode( IsTrue(args) );
	}
	else if ( StrEqual(cmd, "exec") )
	{
		// make sure we don't pass any parameters 
		// to the app we want to spawn. We don't want
		// anybody to do "del *.*"
		int     len = strlen(args) + 1;
		char    *tmpString = new char[len];

		if ( tmpString == 0 )
			return;

		for ( int i = 0; i < len; i++ )
		{
			tmpString[i] = args[i];
			if ( tmpString[i] == ' ' )
			{
				tmpString[i] = 0;
				break;
			}
		}
		ExecSystem( tmpString );
		delete [] tmpString;
	}
	else if ( StrEqual(cmd, "showmenu") )
	{
		// Toggle the full screen flags
		/*
		showMenu = IsTrue(args);
		HMENU hMenu = GetMenu( hwnd );

		if (!showMenu) 
		{
			if (hMenu) 
			{
				// Save and clear the menu bar
				savedMenu = hMenu;
				SetMenu( hwnd, 0);
			}
		} 
		else 
		{
			SetMenu( hwnd, savedMenu );
		}
		*/
	}
	else
	{
		flash->command(cmd, args);
	}
}

BOOL NativePlayerWnd::UpdateCursor()
{
	int type = GetCursorType();
/*
	switch( type )
	{
		case CURSOR_TYPE_ARROW:
		flash->SetCursor( cursorArrow );
		return true;

		case CURSOR_TYPE_HAND:
		SetCursor( cursorHand );
		return true;

		case CURSOR_TYPE_BUTTON:
		SetCursor( cursorButton );
		return true;

		case CURSOR_TYPE_BEAM:
		SetCursor( cursorBeam );
		return true;
	}
*/
	// todo flash
	return false;
}

#ifdef EDITTEXT
#ifdef EDITTEXT_CLIPBOARD_WIDE

// ------Support for Unicode-------

U16* NativePlayerWnd::GetStringFromClipboard()
{
	const U16* text;
	U16* ret = 0;

	if ( OpenClipboard( 0 ) ) 
	{
		text = (U16*) GetClipboardData( CF_UNICODETEXT );
		if ( text )
		{
			ret = new U16[ WideStrLen( text ) + 1 ];

			// windows uses /r/n, while Flash uses /r...sigh
			// take out the /n
			const U16* p;
			U16* q;
			for( p = text, q = ret; *p; p++ )
			{
				if ( *p != '\n' )
					*q++ = *p;
			}
			*q = 0;
		}
		CloseClipboard();
	}
	return ret;
}

void NativePlayerWnd::SetStringToClipboard( const U16* text )
{
	if ( text && *text && OpenClipboard( 0 ) ) 
	{
		HGLOBAL globalMem = GlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE, ( WideStrLen( text ) + 1 ) * 2 );
		U16* globalText = (U16*) GlobalLock( globalMem );
		if ( globalText )
		{
			WideStrCopy( globalText, text );
			GlobalUnlock( globalMem );
			EmptyClipboard();
			SetClipboardData( CF_UNICODETEXT, globalText );
		}
		CloseClipboard();
	}
}

#else

// ------Support for ASCII-------

char* NativePlayerWnd::GetStringFromClipboard()
{
	const char* text;
	char* ret = 0;

	if ( OpenClipboard( 0 ) ) 
	{
		text = (char*) GetClipboardData( CF_TEXT );
		if ( text )
		{
			ret = new char[ strlen( text ) + 1 ];

			// windows uses /r/n, while Flash uses /r...sigh
			// take out the /n
			const char* p;
			char* q;
			for( p = text, q = ret; *p; p++ )
			{
				if ( *p != '\n' )
					*q++ = *p;
			}
			*q = 0;
		}
		CloseClipboard();
	}
	return ret;
}

void NativePlayerWnd::SetStringToClipboard( const char* text )
{
	if ( text && *text && OpenClipboard( 0 ) ) 
	{
		HGLOBAL globalMem = GlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE, ( strlen( text ) + 1 ) * 2 );
		char* globalText = (char*) GlobalLock( globalMem );
		if ( globalText )
		{
			strcpy( globalText, text );
			GlobalUnlock( globalMem );
			EmptyClipboard();
			SetClipboardData( CF_TEXT, globalText );
		}
		CloseClipboard();
	}
}

#endif
#endif

/*
HDC NativePlayerWnd::GetNWindowDC()
{
	FLASHASSERT( windowDC );
	return windowDC;
}
*/

void NativePlayerWnd::CloseStream( StreamData* streamData )
{
	// The quick and easy, again. Doesn't actually stop the stream, just prevents
	// future writes.
	streamData->scriptPlayer = 0;
}

void NativePlayerWnd::StreamGetURLNotify(	const char* url,
											const char* window,
											void*		notifyData )
{
	// The quick and easy solution - support for file loading only. But, in order
	// to bring some aura of respectability to this demo, it will be implemented
	// on a thread and simulating a network connection.

	// Remove the file: prefix
	const char* fname;
	char* cmd = SkipPrefix( (char*) url, "file://");
	if ( cmd )
		fname = cmd;
	else
		fname = url;
	
	// We now have an fname that may or may not be a valid file. Try to open it:
	FILE* fp = fopen( url, "rb" );
	if ( fp )
	{
		#ifdef SIMULATE_STREAMING
			DWORD id;
			ThreadData* data = new ThreadData;

			data->fp = fp;
			data->notifyData = notifyData;
			data->hwnd = hwnd; 
			data->url = CreateStr( (char*)url );

			CreateThread( 0, 0, StreamThread, data, 0, &id );
		#else
			StreamData streamData;
			char block[256];
			int  read;

			StreamInNew( &streamData, (char*) url, notifyData );
			while ( read = fread( block, 1, 256, fp ) )
			{
				StreamInWrite( &streamData, &block, read );
			}
			StreamInDestroy( &streamData );
		#endif
	}
}

void NativePlayerWnd::StreamGetResourceNotify(Aresobj *o, void *notifyData)
{
	void	*adr=o->lock();
	if(adr)
	{
		StreamData streamData;
		StreamInNew(&streamData, (char*)"", notifyData);
		StreamInWrite(&streamData, adr, o->getSize());
		StreamInDestroy(&streamData);
	}
	o->unlock();
}

#ifdef SIMULATE_STREAMING
DWORD WINAPI StreamThread(  LPVOID lpParameter )
{
	ThreadData* data = (ThreadData*) lpParameter;

	int  delay;
	int  kilobytes = KILOBYTESPERSEC;

	if ( kilobytes > 0 )
		delay = 250 / kilobytes;
	else
		delay = 0;

	// put in some arbitrary delays
	SleepEx( delay*2, false );
	PostMessage( data->hwnd, MSG_STREAM_NEW, (DWORD) data, (DWORD) data->notifyData );

	SleepEx( delay*2, false );

	char block[256];
	int  read;
	
	while ( read = fread( block, 1, 256, data->fp ) )
	{
		WriteBlock* write = new WriteBlock;
		memcpy( write->data, block, read );
		write->size = read;

		PostMessage( data->hwnd, MSG_STREAM_WRITE, (DWORD) data, (DWORD) write );
		SleepEx( delay, false );
	}
	PostMessage( data->hwnd, MSG_STREAM_DESTROY, (DWORD) data, 0 );

	return 0;
}
#endif 


