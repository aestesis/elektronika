// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __S_LIGHT_H_INCLUDED__
#define __S_LIGHT_H_INCLUDED__

#include "SColor.h"

namespace irr
{
namespace video  
{

//! structure for holding data describing a dynamic point light.
/** ambient light and point lights are the only light supported 
by the irrlicht engine.
*/
struct SLight
{
	SLight() : AmbientColor(0.0f,0.0f,0.0f), DiffuseColor(1.0f, 1.0f, 1.0f), 
		SpecularColor(0.0f,0.0f,0.0f), Position(0.0f, 0.0f, 0.0f), Radius(100.0f),
		CastShadows(true)
		 {};

	//! Ambient color emitted by the light
	SColorf AmbientColor; 

	//! Diffuse color emitted by the light. This is the primary color you
	//! might want to set.
	SColorf DiffuseColor; 

	//! Specular color emitted by the light. 
	SColorf SpecularColor; 

	//! Position of the light.
	core::vector3df Position; 

	//! Radius of light. Everything within this radius be be lighted.
	f32 Radius; 

	//! Does the light cast shadows?
	bool CastShadows;
};

} // end namespace video
} // end namespace irr

#endif

