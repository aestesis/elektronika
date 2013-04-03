// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __I_MESH_MANIPULATOR_H_INCLUDED__
#define __I_MESH_MANIPULATOR_H_INCLUDED__

#include "IUnknown.h"
#include "vector3d.h"
#include "IMeshBuffer.h"
#include "aabbox3d.h"

namespace irr
{
namespace scene
{

	class IMesh;
	class IAnimatedMesh;
	struct SMesh;


	//! An interface for easily manipulate meshes.
	/** Scale, set alpha value, flip surfaces, and so on. This exists for fixing problems 
	with wrong imported or exported meshes quickly after loading. It is not intended for doing mesh
	modifications and/or animations during runtime.
	*/
	class IMeshManipulator : public IUnknown
	{
	public:

		//! destructor
		virtual ~IMeshManipulator() {};

		//! Flips the direction of surfaces. Changes backfacing triangles to frontfacing
		//! triangles and vice versa.
		//! \param mesh: Mesh on which the operation is performed.
		virtual void flipSurfaces(scene::IMesh* mesh) const = 0;

		//! Sets the alpha vertex color value of the whole mesh to a new value
		//! \param mesh: Mesh on which the operation is performed.
		//! \param alpha: New alpha value. Must be a value between 0 and 255.
		virtual void setVertexColorAlpha(scene::IMesh* mesh, s32 alpha) const = 0;

		//! Recalculates all normals of the mesh.
		//! \param mesh: Mesh on which the operation is performed.
		virtual void recalculateNormals(scene::IMesh* mesh) const = 0;

		//! Scales the whole mesh.
		//! \param mesh: Mesh on which the operation is performed.
		//! \param scale: Scale factor.
		virtual void scaleMesh(scene::IMesh* mesh, const core::vector3df& scale) const = 0;

		//! Clones a static IMesh into a modifyable SMesh. All meshbuffers in the SMesh
		//! are of type SMeshBuffer or SMeshBufferLightMap.
		//! \param mesh: Mesh to copy.
		//! \return Returns the cloned mesh.
		//! If you no longer need the cloned mesh, you should call SMesh::drop().
		//! See IUnknown::drop() for more information.
		virtual SMesh* createMeshCopy(scene::IMesh* mesh) const = 0;

		//! Creates a planar texture mapping on the mesh
		//! \param mesh: Mesh on which the operation is performed.
		//! \param resolution: resolution of the planar mapping. This is the value
		//! specifying which is the relation between world space and 
		//! texture coordinate space.
		virtual void makePlanarTextureMapping(scene::IMesh* mesh, f32 resolution=0.001f) const = 0;

		//! Recalculates the bounding box for a meshbuffer
		virtual void recalculateBoundingBox(scene::IMeshBuffer* buffer) const = 0;

		//! Returns amount of polygons in mesh.
		virtual s32 getPolyCount(scene::IMesh* mesh) const = 0;

		//! Returns amount of polygons in mesh.
		virtual s32 getPolyCount(scene::IAnimatedMesh* mesh) const = 0;
	};

} // end namespace scene
} // end namespace irr


#endif

