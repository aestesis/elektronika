// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __I_SCENE_NODE_ANIMATOR_H_INCLUDED__
#define __I_SCENE_NODE_ANIMATOR_H_INCLUDED__

#include "IUnknown.h"
#include "vector3d.h"

namespace irr
{
namespace scene
{
	class ISceneNode;

	//! Animates a scene node. Can animate position, rotation, material, and so on.
	/** A scene node animator is able to animate a scene node in a very simple way. It may
	change its position, rotation, scale and/or material. There are lots of animators
	to choose from. You can create scene node animators with the ISceneManager interface.
	*/
	class ISceneNodeAnimator : public IUnknown
	{
	public:

		//! destructor
		virtual ~ISceneNodeAnimator() {}

		//! Animates a scene node.
		//! \param node: Node to animate.
		//! \param timeMs: Current time in milli seconds.
		virtual void animateNode(ISceneNode* node, u32 timeMs) = 0;
	};
} // end namespace scene
} // end namespace irr

#endif

