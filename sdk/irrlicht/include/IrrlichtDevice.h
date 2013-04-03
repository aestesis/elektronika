// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __I_IRRLICHT_DEVICE_H_INCLUDED__
#define __I_IRRLICHT_DEVICE_H_INCLUDED__

#include "irrTypes.h"
#include "IUnknown.h"
#include "dimension2d.h"
#include "IFileSystem.h"
#include "IVideoDriver.h"
#include "EDriverTypes.h"
#include "IGUIEnvironment.h"
#include "IEventReceiver.h"
#include "ISceneManager.h"
#include "ICursorControl.h"
#include "IVideoModeList.h"
#include "ITimer.h"
#include "ILogger.h"
#include "IOSOperator.h"

namespace irr
{
	//! The Irrlicht device. You can create it with createDevice(). 
	/** This is the most important class of the Irrlicht Engine. You can access everything
	in the engine if you have a pointer to an instance of this class. 
	*/
	class IrrlichtDevice : public IUnknown
	{
	public:

		//! destructor
		virtual ~IrrlichtDevice() {};

		//! Runs the device. Returns false if device wants to be deleted. Use it in this way:
		//! \code
		//! while(device->run())
		//! {
		//!		// draw everything here
		//! }
		//! \endcode
		virtual bool run() = 0;

		//! \return Returns a pointer the video driver.
		virtual video::IVideoDriver* getVideoDriver() = 0;

		//! \return Returns a pointer to the file system.
		virtual io::IFileSystem* getFileSystem() = 0;

		//! \return Returns a pointer to the gui environment.
		virtual gui::IGUIEnvironment* getGUIEnvironment() = 0;

		//! \return Returns a pointer to the scene manager.
		virtual scene::ISceneManager* getSceneManager() = 0;

		//! \return Returns a pointer to the mouse cursor control interface.
		virtual gui::ICursorControl* getCursorControl() = 0;

		//! \return Returns a pointer to the logger.
		virtual ILogger* getLogger() = 0;

		//! Gets a list with all video modes available. If you are confused 
		//! now, because you think you have to create an Irrlicht Device with a video
		//! mode before being able to get the video mode list, let me tell you that
		//! there is no need to start up an Irrlicht Device with DT_DIRECTX8, DT_OPENGL or
		//! DT_SOFTWARE: For this (and for lots of other reasons) the null device,
		//! DT_NULL exists.
		//! \return Returns a pointer to a list with all video modes supported
		//! by the gfx adapter.
		virtual video::IVideoModeList* getVideoModeList() = 0;

		//! Returns the operation system opertator object. It provides methods for
		//! getting operation system specific informations and doing operation system
		//! specific operations. Like for example exchanging data with the clipboard
		//! or reading the operation system version.
		virtual IOSOperator* getOSOperator() = 0;

		//! \return Returns a pointer to the ITimer object. With it the
		//! current Time can be received.
		virtual ITimer* getTimer() = 0;

		//! Sets the caption of the window.
		//! \param text: New text of the window caption.
		virtual void setWindowCaption(const wchar_t* text) = 0;

		//! \return Returns true if window is active. If the window is inactive,
		//! nothing need to be drawn. So if you don't want to draw anything when
		//! the window is inactive, create your drawing loop this way:
		//! \code
		//! while(device->run())
		//!		if (device->isWindowActive())
		//!		{
		//!			// draw everything here
		//!		}
		//! \endcode
		virtual bool isWindowActive() = 0;

		//! Notifies the device that it should close itself.
		//! IrrlichtDevice::run() will always return false after closeDevice() was called.
		virtual void closeDevice() = 0;

		//! Returns the version of the engine. The returned string
		//! will look like this: "1.2.3" or this: "1.2". 
		virtual const wchar_t* getVersion() = 0;

		//! Sets a new event receiver to receive events.
		virtual void setEventReceiver(IEventReceiver* receiver) = 0;

		//! Sets if the window should be resizeable in windowed mode. The default
		//! is false. This method only works in windowed mode.
		virtual void setResizeAble(bool resize=false) = 0;
	};

} // end namespace

#endif

