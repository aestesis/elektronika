// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __I_TERRAIN_SCENE_NODE_H_INCLUDED__
#define __I_TERRAIN_SCENE_NODE_H_INCLUDED__

#include "ISceneNode.h"
#include "vector2d.h"

namespace irr
{
namespace scene
{

//! A terrain scene node using simple geomipmapping.
//! Please note, that this is still a very early alpha version. LOD support is currently
//! disabled for example, so using this with big heightmaps will be extremely slow.
class ITerrainSceneNode : public ISceneNode
{
public:

	//! constructor
	ITerrainSceneNode(ISceneNode* parent, ISceneManager* mgr, s32 id, 
		const core::vector3df& position = core::vector3df(0,0,0),
		const core::vector3df& rotation = core::vector3df(0,0,0),
		const core::vector3df& scale = core::vector3df(1.0f, 1.0f, 1.0f))
			: ISceneNode(parent, mgr, id, position, rotation, scale) {}

	//! Calculates height of a point in the terrain
	//! \param pos: 2d position of the terrain. You could take the X and Z 
	//! component of a 3d vector for this. 
	//! \param outHeight: the calculated height at the 2d position will be 
	//! stored in this float if the 2d position is inside the terrain. 
	//! \return Returns true, if the 2d position is insied the terrain and
	//! a valid height value has been calculated. Otherwise, false is returned.
	virtual bool getHeight(core::vector2d<f32> pos, f32& outHeight) = 0;

	//! Sets the Level of detail to a fixed level for all tiles.
	//! Only use this for debug purposes.
	//! \param i: Level to add to the current debug level of detail,
	//! may be negative. To disable debugging LOD, set this to 0.
	virtual void setDebugLOD(s32 i) = 0;
};

} // end namespace scene
} // end namespace irr


#endif

