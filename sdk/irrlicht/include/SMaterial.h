// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __S_MATERIAL_H_INCLUDED__
#define __S_MATERIAL_H_INCLUDED__

#include "SColor.h"
#include "ITexture.h"

namespace irr
{
namespace video
{
	//! Abstracted and easy to use fixed function pipeline material modes.
	enum E_MATERIAL_TYPE
	{
		//! Standard solid material. Only first texture is used, which is
		//! supposed to be the diffuse material.
		EMT_SOLID = 0,			

		//! Solid material with 2 texture layers. The second is blended onto the
		//! first using the alpha value of the vertex colors.
		//! This material is currently not implemented in OpenGL, but it
		//! works with DirectX.
		EMT_SOLID_2_LAYER,

		//! Material type with standard lightmap technique: 
		//! There should be 2 textures: The first texture layer is a diffuse map,
		//! the second is a light map. Vertex light is ignored.
		EMT_LIGHTMAP,

		//! Material type with lightmap technique like EMT_LIGHTMAP, but
		//! lightmap and diffuse texture are not modulated, but added instead.
		EMT_LIGHTMAP_ADD,

		//! Material type with standard lightmap technique: 
		//! There should be 2 textures: The first texture layer is a diffuse map,
		//! the second is a light map. Vertex light is ignored.
		//! The texture colors are effectively multiplyied by 2 for brightening.
		//! like known in DirectX as D3DTOP_MODULATE2X.
		EMT_LIGHTMAP_M2,

		//! Material type with standard lightmap technique: 
		//! There should be 2 textures: The first texture layer is a diffuse map,
		//! the second is a light map. Vertex light is ignored.
		//! The texture colors are effectively multiplyied by 4 for brightening.
		//! like known in DirectX as D3DTOP_MODULATE4X.
		EMT_LIGHTMAP_M4,

		//! Like EMT_LIGHTMAP, but also supports dynamic lighting.
		EMT_LIGHTMAP_LIGHTING,

		//! Like EMT_LIGHTMAP_M2, but also supports dynamic lighting.
		EMT_LIGHTMAP_LIGHTING_M2,

		//! Like EMT_LIGHTMAP_4, but also supports dynamic lighting.
		EMT_LIGHTMAP_LIGHTING_M4,

		//! Makes the material look like it was reflection the environment
		//! around it. To make this possible, a texture called 'sphere map'
		//! is used, which must be set as Texture1.
		EMT_SPHERE_MAP,

		//! A reflecting material with an 
		//! optional additional non reflecting texture layer. The reflection
		//! map should be set as Texture 1.
		//! Please note that this material type is currently not 100% implemented
		//! in OpenGL. It works in DirectX8.
		EMT_REFLECTION_2_LAYER,

		//! A transparent material. Only the first texture is used.
		//! The new color is calculated by simply adding the source color and
		//! the dest color. This means if for example a billboard using a texture with 
		//! black background and a red circle on it is drawed with this material, the
		//! result is that only the red circle will be drawn a little bit transparent,
		//! and everything which was black is 100% transparent and not visible.
		//! This material type is useful for e.g. particle effects.
		EMT_TRANSPARENT_ADD_COLOR,

		//! Makes the material transparent based on the texture alpha channel.
		//! Only first texture is used. Please note that this material is currenly
		//! NOT IMPLEMENTED.
		EMT_TRANSPARENT_ALPHA_CHANNEL,	

		//! Makes the material transparent based on the vertex alpha value.
		EMT_TRANSPARENT_VERTEX_ALPHA,

		//! A transparent reflecting material with an 
		//! optional additional non reflecting texture layer. The reflection
		//! map should be set as Texture 1. The transparency depends on the
		//! alpha value in the vertex colors. A texture which will not reflect
		//! can be set als Texture 2.
		//! Please note that this material type is currently not 100% implemented
		//! in OpenGL. It works in DirectX8.
		EMT_TRANSPARENT_REFLECTION_2_LAYER,

		//! This value is not used. It only forces this enumeration to compile in 32 bit. 
		EMT_FORCE_32BIT = 0x7fffffff	
	};

	//! Material flags
	enum E_MATERIAL_FLAG
	{
		//! Draw as wireframe or filled triangles? Default: false
		EMF_WIREFRAME = 0,

		//! Flat or Gouraud shading? Default: true
		EMF_GOURAUD_SHADING,

		//! Will this material be lighted? Default: true
		EMF_LIGHTING,

		//! Is the ZBuffer enabled? Default: true
		EMF_ZBUFFER,

		//! May be written to the zbuffer or is it readonly. Default: true
		//! This flag is ignored, if the material type is a transparent type.
		EMF_ZWRITE_ENABLE,

		//! Is backfaceculling enabled? Default: true
		EMF_BACK_FACE_CULLING,

		//! Is bilinear filtering enabled? Default: true
		EMF_BILINEAR_FILTER,

		//! Is trilinear filtering enabled? Default: false
		//! If the trilinear filter flag is enabled,
		//! the bilinear filtering flag is ignored.
		EMF_TRILINEAR_FILTER,

		//! Is fog enabled? Default: false
		EMF_FOG_ENABLE,

		//! This is not a flag, but a value indicating how much flags there are.
		EMF_MATERIAL_FLAG_COUNT 
	};

	const s32 MATERIAL_MAX_TEXTURES = 2;


	//! struct for holding a material
	struct SMaterial
	{
		//! default constructor, creates a solid material with standard colors
		SMaterial()
		: AmbientColor(255,255,255,255), DiffuseColor(255,255,255,255),
		EmissiveColor(0,0,0,0), SpecularColor(0,0,0,0), Texture1(0), Texture2(0),
			MaterialType(EMT_SOLID), Wireframe(false), Lighting(true),
			ZBuffer(true), ZWriteEnable(true), BackfaceCulling(true),
			GouraudShading(true), Shininess(0.0f), BilinearFilter(true),
			TrilinearFilter(false),	FogEnable(false)
		{}

		//! Type of the material. Specifies how everything is blended together
		E_MATERIAL_TYPE MaterialType;	

		//! How much ambient light (a global light) is reflected by this material.
		//! The default is full white, meaning objects are completely globally illuminated.
		//! Reduce this if you want to see diffuse or specular light effects, or change
		//! the blend of colours to make the object have a base colour other than white.
		SColor AmbientColor;		

		//! How much diffuse light coming from a light source is reflected by this material.
		//! The default is full white.
		SColor DiffuseColor;		

		//! Light emitted by this material. Default is to emitt no light.
		SColor EmissiveColor;	

		//! How much specular light (highlights from a light) is reflected. 
		//! The default is to reflect no specular light. 
		SColor SpecularColor;	

		//! Value affecting the size of specular highlights.
		f32 Shininess;			

		//! Texture layer union.
		union
		{
			struct
			{
				//! Primary texture layer.
				ITexture* Texture1;	

				//! Secondary texture layer
				ITexture* Texture2;	
			};

			ITexture* Textures[MATERIAL_MAX_TEXTURES];
		};

		//! material flag union. This enables the user to access the
		//! material flag using e.g: material.Wireframe = true or
		//! material.flag[EMF_WIREFRAME] = true;
		union
		{
			struct
			{
				//! Draw as wireframe or filled triangles? Default: false
				bool Wireframe;

				//! Flat or Gouraud shading? Default: true
				bool GouraudShading;				

				//! Will this material be lighted? Default: true
				bool Lighting;						

				//! Is the ZBuffer enabled? Default: true
				bool ZBuffer;						

				//! May be written to the zbuffer or is it readonly. Default: true
				//! This flag is ignored, if the MaterialType is a transparent type.
				bool ZWriteEnable;					

				//! Is backfaceculling enabled? Default: true
				bool BackfaceCulling;				

				//! Is bilinear filtering enabled? Default: true
				bool BilinearFilter;	

				//! Is trilinear filtering enabled? Default: false
				//! If the trilinear filter flag is enabled,
				//! the bilinear filtering flag is ignored.
				bool TrilinearFilter;

				//! Is fog enabled? Default: false
				bool FogEnable;
			};

			bool Flags[EMF_MATERIAL_FLAG_COUNT];
		};


		//! Compare operator
		inline bool operator!=(const SMaterial& b)
		{
			return MaterialType != b.MaterialType ||
				AmbientColor != b.AmbientColor ||
				DiffuseColor != b.DiffuseColor ||
				EmissiveColor != b.EmissiveColor ||
				SpecularColor != b.SpecularColor ||
				Shininess != b.Shininess ||
				Wireframe != b.Wireframe ||
				GouraudShading != b.GouraudShading ||
				Lighting != b.Lighting ||
				ZBuffer != b.ZBuffer ||
				ZWriteEnable != b.ZWriteEnable ||
				BackfaceCulling != b.BackfaceCulling ||
				BilinearFilter != b.BilinearFilter ||
				TrilinearFilter != b.TrilinearFilter ||
				FogEnable != b.FogEnable;
		}	
	};

} // end namespace video
} // end namespace irr

#endif

