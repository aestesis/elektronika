// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __I_GUI_ELEMENT_H_INCLUDED__
#define __I_GUI_ELEMENT_H_INCLUDED__

#include "IUnknown.h"
#include "irrList.h"
#include "rect.h"
#include "irrString.h"
#include "IEventReceiver.h"

namespace irr
{
namespace gui
{

class IGUIEnvironment;

//! Base class of all GUI elements.
class IGUIElement : public IUnknown, public IEventReceiver
{
public:

	//! Constructor
	IGUIElement(IGUIEnvironment* environment, IGUIElement* parent, s32 id, core::rect<s32> rectangle)
		: Environment(environment), IsVisible(true), IsEnabled(true),
			Parent(parent), RelativeRect(rectangle), ID(id)
	{
		AbsoluteRect = RelativeRect;
		AbsoluteClippingRect = AbsoluteRect;

		if (Parent)
		{
			AbsoluteRect += Parent->getAbsolutePosition().UpperLeftCorner;
			AbsoluteClippingRect = AbsoluteRect;
			AbsoluteClippingRect.clipAgainst(Parent->AbsoluteClippingRect);
		}

		if (Parent)
			Parent->addChild(this);
	}


	//! Destructor
	virtual ~IGUIElement()
	{
		// delete all children
		core::list<IGUIElement*>::Iterator it = Children.begin();
		for (; it != Children.end(); ++it)
		{
			(*it)->Parent = 0;
			(*it)->drop();
		}
	};


	//! Returns parent of this element.
	IGUIElement* getParent()
	{
		return Parent;
	}


	//! Returns the relative rectangle of this element.
	core::rect<s32> getRelativePosition()
	{
		return RelativeRect;
	}


	//! Sets the relative rectangle of this element.
	void setRelativePosition(const core::rect<s32>& r)
	{
		RelativeRect = r;
		updateAbsolutePosition();
	}


	//! Returns the absolute rectangle of element.
	core::rect<s32> getAbsolutePosition()
	{
		return AbsoluteRect;
	}


	//! Updates the absolute position.
	virtual void updateAbsolutePosition()
	{
		core::rect<s32> parentAbsolute(0,0,0,0);
		core::rect<s32> parentAbsoluteClip;

		if (Parent)
		{
			parentAbsolute = Parent->AbsoluteRect;
			parentAbsoluteClip = Parent->AbsoluteClippingRect;
		}

		AbsoluteRect = RelativeRect + parentAbsolute.UpperLeftCorner;

		if (!Parent)
			parentAbsoluteClip = AbsoluteRect;

		AbsoluteClippingRect = AbsoluteRect;
		AbsoluteClippingRect.clipAgainst(parentAbsoluteClip);		

		// update all children
		core::list<IGUIElement*>::Iterator it = Children.begin();
		for (; it != Children.end(); ++it)
			(*it)->updateAbsolutePosition();
	}


	//! Returns the child element, which is at the position of the point.
	IGUIElement* getElementFromPoint(const core::position2d<s32>& point)
	{
		IGUIElement* target = 0;

		// we have to search from back to front.

		core::list<IGUIElement*>::Iterator it = Children.getLast();

		if (IsVisible)
			while(it != Children.end())
			{
				target = (*it)->getElementFromPoint(point);
				if (target)
					return target;

				--it;
			}

		if (AbsoluteRect.isPointInside(point) && IsVisible)
			target = this;
		
		return target;
	}


	//! Adds a GUI element as new child of this element.
	virtual void addChild(IGUIElement* child)
	{
		if (child)
		{
			Children.push_back(child);
			child->grab();
		}
	}


	//! Removes a child.
	virtual void removeChild(IGUIElement* child)
	{
		core::list<IGUIElement*>::Iterator it = Children.begin();
		for (; it != Children.end(); ++it)
			if ((*it) == child)
			{
				(*it)->drop();
				Children.erase(it);
				return;
			}
	}


	//! Removes this element.
	virtual void remove()
	{
		if (Parent)
			Parent->removeChild(this);
	}


	//! Draws the element and its children.
	virtual void draw()
	{
		if (!IsVisible)
			return;

		core::list<IGUIElement*>::Iterator it = Children.begin();
		for (; it != Children.end(); ++it)
			(*it)->draw();
	}


	//! Moves this element.
	virtual void move(core::position2d<s32> absoluteMovement)
	{
		RelativeRect += absoluteMovement;
		updateAbsolutePosition();
	}


	//! Returns true if element is visible.
	virtual bool isVisible()
	{
		return IsVisible;
	}



	//! Sets the visible state of this element.
	virtual void setVisible(bool visible)
	{
		IsVisible = visible;
	}


	//! Returns true if element is enabled.
	virtual bool isEnabled()
	{
		return IsEnabled;
	}


	//! Sets the enabled state of this element.
	virtual void setEnabled(bool enabled)
	{
		IsEnabled = enabled;
	}


	//! Sets the new caption of this element.
	virtual void setText(const wchar_t* text)
	{
		Text = text;
	}


	//! Returns caption of this element.
	virtual const wchar_t* getText()
	{
		return Text.c_str();
	}


	//! Returns id. Can be used to identify the element.
	virtual s32 getID()
	{
		return ID;
	}

	//! Sets the id of this element
	virtual void setID(s32 id)
	{
		ID = id;
	}


	//! Called if an event happened.
	virtual bool OnEvent(SEvent event)
	{
		if (Parent)
			Parent->OnEvent(event);
		return true;
	}


	//! Brings a child to front
	//! \return Returns true if successful, false if not.
	virtual bool bringToFront(IGUIElement* element)
	{
		core::list<IGUIElement*>::Iterator it = Children.begin();
		for (; it != Children.end(); ++it)
		{
			if (element == (*it))
			{
				Children.erase(it);
				Children.push_back(element);
				return true;
			}
		}

		return false;
	}

	//! Returns list with children of this element
	virtual const core::list<IGUIElement*>& getChildren() const
	{
		return Children;
	}

	//! Finds the first element with the given id.
	//! \param id: Id to search for.
	//! \param searchchildren: Set this to true, if also children of this 
	//! element may contain the element with the searched id and they 
	//! should be searched too.
	//! \return Returns the first element with the given id. If no element
	//! with this id was found, 0 is returned.
	virtual IGUIElement* getElementFromId(s32 id, bool searchchildren=false) const
	{
		IGUIElement* e = 0;

		core::list<IGUIElement*>::Iterator it = Children.begin();
		for (; it != Children.end(); ++it)
		{
			if ((*it)->getID() == id)
				return (*it);

			if (searchchildren)
				e = (*it)->getElementFromId(id, true);

			if (e)
				return e;
		}		

		return e;
	}

protected:

	//! List of all children of this element
	core::list<IGUIElement*> Children;

	//! Pointer to the parent
	IGUIElement* Parent;

	//! relative rect of element
	core::rect<s32> RelativeRect;

	//! absolute rect of element
	core::rect<s32> AbsoluteRect;

	//! absolute clipping rect of element
	core::rect<s32> AbsoluteClippingRect;

	//! is visible?
	bool IsVisible;

	//! is enabled?
	bool IsEnabled;

	//! caption
	core::stringw Text;

	//! id
	s32 ID;

	//! GUI Environment
	IGUIEnvironment* Environment;
};


} // end namespace gui
} // end namespace irr

#endif

