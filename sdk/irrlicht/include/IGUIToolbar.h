// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __I_GUI_TOOL_BAR_H_INCLUDED__
#define __I_GUI_TOOL_BAR_H_INCLUDED__

#include "IGUIElement.h"

namespace irr
{
namespace video
{
	class ITexture;
}
namespace gui
{
	class IGUIButton;

	//! Stays at the top of its parent like the menu bar and contains tool buttons
	class IGUIToolBar : public IGUIElement
	{
	public:

		//! constructor
		IGUIToolBar(IGUIEnvironment* environment, IGUIElement* parent, s32 id, core::rect<s32> rectangle)
			: IGUIElement(environment, parent, id, rectangle) {}

		//! destructor
		~IGUIToolBar() {};

		//! Adds a button to the tool bar
		virtual IGUIButton* addButton(s32 id=-1, const wchar_t* text=0,
			video::ITexture* img=0, video::ITexture* pressedimg=0,
			bool isPushButton=false) = 0;
	};


} // end namespace gui
} // end namespace irr

#endif

