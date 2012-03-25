// This is the current Techdemo of the Irrlicht Engine (c) 2003 by N.Gebhardt.
// This file is not documentated.

#ifdef _WIN32
#include <windows.h>
#endif

#include <irrlicht.h>
#include <stdio.h>

#include "CMainMenu.h"
#include "CDemo.h"

using namespace irr;

#pragma comment(lib, "Irrlicht.lib")

#ifdef _WIN32
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main()
#endif
{
	bool fullscreen = false;
	bool music = true;
	bool shadows = false;
	bool additive = false;
	bool vsync = false;

	video::E_DRIVER_TYPE driverType = video::EDT_DIRECTX9;

	CMainMenu menu;

#ifndef _DEBUG
	if (menu.run(fullscreen, music, shadows, additive, vsync, driverType))
#endif
	{
		CDemo demo(fullscreen, music, shadows, additive, vsync, driverType);
		demo.run();		
	}	

	return 0;
}

