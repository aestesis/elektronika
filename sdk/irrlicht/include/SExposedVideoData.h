// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __S_EXPOSED_VIDEO_DATA_H_INCLUDED__
#define __S_EXPOSED_VIDEO_DATA_H_INCLUDED__

// forward declarations for internal pointers
struct IDirect3D9;
struct IDirect3DDevice9;
struct IDirect3D8;
struct IDirect3DDevice8;

namespace irr
{
namespace video
{

//! structure for holding data describing a driver and operating system specific data.
/** This data can be retrived by IVideoDriver::getExposedVideoData(). Use this with caution. 
 This only should be used to make it possible to extend the engine easily without
 modification of its source. Note that this structure does not contain any valid data, if
 you are using the software or the null device.
*/
struct SExposedVideoData
{
	union
	{
		struct
		{
			//! Pointer to the IDirect3D9 interface
			IDirect3D9* D3D9;

			//! Pointer to the IDirect3D9 interface
			IDirect3DDevice9* D3DDev9;

			//! Window handle. Get with for example 
			//! HWND h = reinterpret_cast<HWND>(exposedData.D3D9.HWnd)
			s32 HWnd;

		} D3D9;

		struct
		{
			//! Pointer to the IDirect3D8 interface
			IDirect3D8* D3D8;

			//! Pointer to the IDirect3D8 interface
			IDirect3DDevice8* D3DDev8;

			//! Window handle. Get with for example with:
			//! HWND h = reinterpret_cast<HWND>(exposedData.D3D8.HWnd)
			s32 HWnd;

		} D3D8;

		struct 
		{
			//! Private GDI Device Context. Get if for example with:
			//! HDC h = reinterpret_cast<HDC>(exposedData.OpenGLWin32.HDc)
			s32 HDc; 

			//! Permanent Rendering Context. Get if for example with:
			//! HGLRC h = reinterpret_cast<HGLRC>(exposedData.OpenGLWin32.HRc)
			s32 HRc; 

			//! Window handle. Get with for example with:
			//! HWND h = reinterpret_cast<HWND>(exposedData.OpenGLWin32.HWnd)
			s32 HWnd;
		
		} OpenGLWin32;

		struct 
		{
			// TODO
		} OpenGLLinux;
	};
};

} // end namespace video
} // end namespace irr


#endif

