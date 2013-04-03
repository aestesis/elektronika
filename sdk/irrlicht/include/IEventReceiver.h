// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __I_EVENT_RECEIVER_H_INCLUDED__
#define __I_EVENT_RECEIVER_H_INCLUDED__

#include "ILogger.h"
#include "position2d.h"
#include "Keycodes.h"

namespace irr
{
	//! Enumeration for all event types there are.
	enum EEVENT_TYPE
	{
		//! An event of the graphical user interface.
		EET_GUI_EVENT = 0,

		//! A mouse input event.
		EET_MOUSE_INPUT_EVENT,

		//! A key input evant.
		EET_KEY_INPUT_EVENT,

		//! A log event
		EET_LOG_TEXT_EVENT
	};

	//! Enumeration for all mouse input events
	enum EMOUSE_INPUT_EVENT
	{
		//! Left mouse button was pressed down.
		EMIE_LMOUSE_PRESSED_DOWN = 0,

		//! Right mouse button was pressed down.
		EMIE_RMOUSE_PRESSED_DOWN,

		//! Middle mouse button was pressed down.
		EMIE_MMOUSE_PRESSED_DOWN,

		//! Left mouse button was left up.
		EMIE_LMOUSE_LEFT_UP,

		//! Right mouse button was left up.
		EMIE_RMOUSE_LEFT_UP,

		//! Middle mouse button was left up.
		EMIE_MMOUSE_LEFT_UP,

		//! The mouse cursor changed its position.
		EMIE_MOUSE_MOVED,

		//! The mouse wheel was moved. Use Wheel value in event data to find out 
		//! in what direction and how fast.
		EMIE_MOUSE_WHEEL, 
	};

	namespace gui
	{

		class IGUIElement;

		//! Enumeration for all events which are sendable by the gui system
		enum EGUI_EVENT_TYPE
		{
			//! A gui element has lost its focus.
			EGET_ELEMENT_FOCUS_LOST = 0,

			//! A gui element was hovered.
			EGET_ELEMENT_HOVERED,

			//! A hovered gui element was left
			EGET_ELEMENT_LEFT,

			//! A button was clicked.
			EGET_BUTTON_CLICKED,

			//! A scrollbar has changed its position.
			EGET_SCROLL_BAR_CHANGED,

			//! A checkbox has changed its check state.
			EGET_CHECKBOX_CHANGED,

			//! A new item in a listbox was seleted.
			EGET_LISTBOX_CHANGED, 

			//! An item in the listbox was selected, which was already selected.
			EGET_LISTBOX_SELECTED_AGAIN, 

			//! A file has been selected in the file open dialog
			EGET_FILE_SELECTED,

			//! A file open dialog has been closed without choosing a file
			EGET_FILE_CHOOSE_DIALOG_CANCELLED,

			//! 'Yes' was clicked on a messagebox
			EGET_MESSAGEBOX_YES,

			//! 'No' was clicked on a messagebox
			EGET_MESSAGEBOX_NO,

			//! 'OK' was clicked on a messagebox
			EGET_MESSAGEBOX_OK,

			//! 'Cancel' was clicked on a messagebox
			EGET_MESSAGEBOX_CANCEL,

			//! In an editbox was pressed 'ENTER'
			EGET_EDITBOX_ENTER,

			//! The tab was changed in an tab control
			EGET_TAB_CHANGED,

			//! A menu item was selected in a (context) menu
			EGET_MENU_ITEM_SELECTED,

		};
	} // end namespace gui


//! Struct for holding event data. An event can be a gui, mouse or keyboard event.
struct SEvent
{
	EEVENT_TYPE EventType;

	union
	{
		struct 
		{
			gui::IGUIElement* Caller;
			gui::EGUI_EVENT_TYPE EventType;			
		} GUIEvent;

		struct
		{
			s32 X, Y; // mouse positions;

			//! mouse wheel delta, usually 1.0 or -1.0. Only valid if event was
			//! EMIE_MOUSE_WHEEL
			f32 Wheel; 

			//! type of more event
			EMOUSE_INPUT_EVENT Event;
		} MouseInput;

		struct
		{
			wchar_t Char; // Character corresponding to the key (0, if not a character)
			EKEY_CODE Key; // Key pressed
			bool PressedDown; // if not pressed, then left up
			bool Shift; // true if shift was also pressed
			bool Control; // true if ctrl was also pressed
		} KeyInput;

		struct
		{
			const c8* Text;
			ELOG_LEVEL Level;
		} LogEvent;

	};

};

//! Interface of an object wich can receive events.
class IEventReceiver
{
public:

	//! called if an event happened. returns true if event was processed
	virtual bool OnEvent(SEvent event) = 0;
};


} // end namespace

#endif

