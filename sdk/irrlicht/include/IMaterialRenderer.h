// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __I_MATERIAL_RENDERER_H_INCLUDED__
#define __I_MATERIAL_RENDERER_H_INCLUDED__

#include "IUnknown.h"
#include "SMaterial.h"

namespace irr
{
namespace video  
{

class IMaterialRendererServices;

//!	Interface for material rendering. Can be used to extend the engine with new materials.
/** Refer to IVideoDriver::addMaterialRenderer() for more informations on how to extend the engine
 with new materials.
*/
class IMaterialRenderer : public IUnknown
{
public:

	//! destructor
	virtual ~IMaterialRenderer() {};

	//! Called by the IVideoDriver implementation the let the renderer
	//! set its needed render states. This is called during the IVideoDriver::setMaterial() call.
	//! When overriding this, you can set some renderstates or for example a vertex or pixel shader
	//! if you like.
	//! \param material: The new material parameters to be set. The renderer may change the material
	//! flags in this material. For example if this material does not accept the zbuffer = true, it
	//! can set it to false. This is useful, because in the next lastMaterial will be just the material
	//! in this call.
	//! \param lastMaterial: The material parameters which have been set before this material.
	//! \param resetAllRenderstates: True if all renderstates should really be reset. This is usually
	//! true if the last rendering mode was not a usual 3d rendering mode, but for example 
	//! a 2d rendering mode.
	//! You should reset really all renderstates if this is true, no matter if the lastMaterial had
	//! some similar settings. This is used because in most cases, some common renderstates are not
	//! changed if they are already there, for example bilinear filtering, wireframe, gouraudshading, 
	//! lighting, zbuffer, zwriteenable, backfaceculling and fogenable.
	//! \param services: Interface providing some methods for changing advanced, internal
	//! states of a IVideoDriver.
	virtual void OnSetMaterial(SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* service) {};

	//! Called by the IVideoDriver to unset this material. Called during the 
	//! IVideoDriver::setMaterial() call before the new material will get the OnSetMaterial()
	//! call.
	virtual void OnUnsetMaterial() {};

	//! Returns if the material is transparent. The scene managment needs to know this
	//! for being able to sort the materials by opaque and transparent.
	virtual bool isTransparent() { return false; }
};

//! Interface providing some methods for changing advanced, internal states of a IVideoDriver.
class IMaterialRendererServices
{
public: 

	//! Can be called by an IMaterialRenderer to make its work easier.
	//! Sets all basic renderstates if needed.
	//! Basic render states are diffuse, ambient, specular, and emissive color, specular power,
	//! bilinear and trilinear filtering, wireframe mode,
	//! grouraudshading, lighting, zbuffer, zwriteenable, backfaceculling and fog enabling.
	virtual void setBasicRenderStates(const SMaterial& material, 
		const SMaterial& lastMaterial,
		bool resetAllRenderstates) = 0;

	//! Sets a vertex shader constant.
	//! \param data: Data to be set in the constants
	//! \param startRegister: First register to be set
	//! \oaram constantAmount: Amount of registers to be set
	virtual void setVertexShaderConstant(f32* data, s32 startRegister, s32 constantAmount=1) = 0;

	//! Sets a pixel shader constant.
	//! \param data: Data to be set in the constants
	//! \param startRegister: First register to be set
	//! \oaram constantAmount: Amount of registers to be set
	virtual void setPixelShaderConstant(f32* data, s32 startRegister, s32 constantAmount=1) = 0;
};

} // end namespace video
} // end namespace irr

#endif

