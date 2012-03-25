// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __I_GPU_PROGRAMMING_SERVICES_H_INCLUDED__
#define __I_GPU_PROGRAMMING_SERVICES_H_INCLUDED__

#include "IUnknown.h"

namespace irr
{

namespace io
{
	class IReadFile;
}

namespace video  
{

class IVideoDriver;
class IShaderConstantSetCallBack;

//!	Interface making it possible to create and use programs running on the GPU.
class IGPUProgrammingServices
{
public:

	//! Adds a new material renderer to the VideoDriver, using pixel and/or 
	//! vertex shaders to render geometry.
	//! Note that you should call IVideoDriver::queryFeature() before to check 
	//! if the IVideoDriver supports the vertex and/or pixel shader version your are using.  
	//! The material is added to the VideoDriver like with IVideoDriver::addMaterialRenderer()
	//! and can be used like it had been added with that method.
	//! \param vertexShaderProgram: String containing the source of the vertex shader program. This can be
	//! 0 if no vertex program should be used. 
	//! For DX8 programs, the will always input registers look like this:
	//! v0: position, v1: normal, 
	//! v2: color, v3: texture cooridnates, v4: texture coordinates 2 if available.
	//! For DX9 programs, you can manually set the registers using the dcl_ statements.
	//! \param pixelShaderProgram: String containing the source of the pixel shader program.
	//! This can be 0 if you don't want to use a pixel shader. 
	//! \param callback: Pointer to an implementation of IShaderConstantSetCallBack in which you 
	//! can set the needed vertex and pixel shader program constants. Set this to 0 if you don't need this.
	//! \param baseMaterial: Base material which renderstates will be used to shade the
	//! material.
	//! \return Returns the number of the
	//! material type which can be set in SMaterial::MaterialType to use the renderer.
	//! -1 is returned if an error occured. -1 is returned for example if a vertex or pixel shader
	//! program could not be compiled, the error strings are then printed out into the error log, and
	//! can be catched with a custom event receiver.
	virtual s32 addShaderMaterial(const c8* vertexShaderProgram = 0,
		const c8* pixelShaderProgram = 0, 
		IShaderConstantSetCallBack* callback = 0,
		E_MATERIAL_TYPE baseMaterial = video::EMT_SOLID) = 0;

	//! Like IGPUProgrammingServices::addShaderMaterial(), but tries to load the 
	//! programs from files.
	//! \param vertexShaderProgram: Text file containing the source of the vertex shader program.
	//! Set to null if no shader should be created.
	//! \param pixelShaderProgram: Text file containing the source of the pixel shader program. Set to 
	//! 0 if no shader should be created.
	virtual s32 addShaderMaterialFromFiles(io::IReadFile* vertexShaderProgram = 0,
		io::IReadFile* pixelShaderProgram = 0,
		IShaderConstantSetCallBack* callback = 0,
		E_MATERIAL_TYPE baseMaterial = video::EMT_SOLID) = 0;

	//! Like IGPUProgrammingServices::addShaderMaterial(), but tries to load the 
	//! programs from files.
	//! \param vertexShaderProgramFileName: Text file name containing the source of the 
	//! vertex shader program. 
	//! Set to null if no shader should be created.
	//! \param pixelShaderProgramFileName: Text file name containing the source of the
	//! pixel shader program. Set to 0 if no shader should be created.
	virtual s32 addShaderMaterialFromFiles(const c8* vertexShaderProgramFileName = 0,
		const c8* pixelShaderProgramFileName = 0,
		IShaderConstantSetCallBack* callback = 0,
		E_MATERIAL_TYPE baseMaterial = video::EMT_SOLID) = 0;

};

class IShaderConstantSetCallBack : public IUnknown
{
public:

	//! Destructor.
	virtual ~IShaderConstantSetCallBack() {}

	//! Called by the engine when the vertex and/or pixel shader constants for an
	//! material renderer should be set.
	virtual void OnSetConstants(IMaterialRendererServices* services) = 0;
};

} // end namespace video
} // end namespace irr

#endif

