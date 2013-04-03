// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __S_K_3D_VERTEX_H_INCLUDED__
#define __S_K_3D_VERTEX_H_INCLUDED__

#include "vector3d.h"
#include "vector2d.h"
#include "SColor.h"

namespace irr
{
namespace video
{

//! Enumeration for all vertex types there are.
enum E_VERTEX_TYPE
{
	//! Standard vertex type used by the Irrlicht engine.
	EVT_STANDARD = 0, 

	//! Vertex with two texture coordinates. Usually used for geometry with lightmaps
	//! or other special materials.
	EVT_2TCOORDS
};

//! standard vertex used by the Irrlicht engine.
struct S3DVertex
{
	//! default constructor
	S3DVertex() {};

	//! constructor
	S3DVertex(f32 x, f32 y, f32 z, f32 nx, f32 ny, f32 nz, SColor c, f32 tu, f32 tv)
		: Pos(x,y,z), Normal(nx,ny,nz), Color(c), TCoords(tu,tv) {}

	//! constructor
	S3DVertex(const core::vector3df& pos, const core::vector3df& normal,
		video::SColor& color, const core::vector2d<f32>& tcoords)
		: Pos(pos), Normal(normal), Color(color), TCoords(tcoords) {}

	//! Position
	core::vector3df Pos;	
	
	//! Normal vector
	core::vector3df Normal;	

	//! Color
	irr::video::SColor Color;				

	//! Texture coordinates
	core::vector2d<f32> TCoords;	



	bool operator == (const S3DVertex& other) const
	{
		return (Pos == other.Pos && Normal == other.Normal &&
					Color == other.Color && TCoords == other.TCoords);				
	}

	bool operator != (const S3DVertex& other) const
	{
		return (Pos != other.Pos || Normal != other.Normal ||
					Color != other.Color || TCoords != other.TCoords);				
	}
};


//! Vertex with two texture coordinates.
/** Usually used for geometry with lightmaps
or other special materials.
*/
struct S3DVertex2TCoords
{
	//! default constructor
	S3DVertex2TCoords() {};

	//! constructor
	S3DVertex2TCoords(f32 x, f32 y, f32 z, SColor c, f32 tu, f32 tv, f32 tu2, f32 tv2)
		: Pos(x,y,z), Color(c), TCoords(tu,tv), TCoords2(tu2,tv2) {}

	//! Position
	core::vector3df Pos;		

	//! Normal
	core::vector3df Normal;		

	//! Color
	SColor Color;				

	//! First set of texture coordinates
	core::vector2d<f32> TCoords;	

	//! Second set of texture coordinates
	core::vector2d<f32> TCoords2;	
};


} // end namespace video
} // end namespace irr

#endif

