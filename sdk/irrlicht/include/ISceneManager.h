// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __I_SCENE_MANAGER_H_INCLUDED__
#define __I_SCENE_MANAGER_H_INCLUDED__

#include "array.h"
#include "IUnknown.h"
#include "vector3d.h"
#include "dimension2d.h"
#include "SColor.h"
#include "SMaterial.h"
#include "IEventReceiver.h"

namespace irr
{
	struct SKeyMap;

namespace io
{
	class IFileSystem;
}

namespace gui
{
	class IGUIFont;
}

namespace video
{
	class IVideoDriver;
}



namespace scene
{
	//! Enumeration with render times.
	/** A parameter passed to the registerNodeForRendering() method of the ISceneManager, 
	specifiing when the mode wants to be drawed in relation to the other nodes. */
	enum E_SCENE_NODE_RENDER_TIME
	{
		//! scene nodes which are lights or camera should use this,
		//! the very first pass.
		SNRT_LIGHT_AND_CAMERA,	

		//! this is used for sky boxes.
		SNRT_SKY_BOX,

		//! Default render time, all normal objects should use this.
		//! The scene Manager will determine by itself if an object is 
		//! transparent, and do everything necessary by itself.
		SNRT_DEFAULT,			

		//! Drawn after the transparent nodes, the time for drawing shadow volumes						
		SNRT_SHADOW,			

		//! Never used, value specifing how much parameters there are
		SNRT_COUNT				
	};

	class IMesh;
	class IAnimatedMesh;
	class ISceneNode;
	class ICameraSceneNode;
	class IAnimatedMeshSceneNode;
	class IBspTreeSceneNode;
	class ISceneNodeAnimator;
	class ISceneNodeAnimatorCollisionResponse;
	class ILightSceneNode;
	class IBillboardSceneNode;
	class ITerrainSceneNode;
	class IMeshLoader;
	class ISceneCollisionManager;
	class IParticleSystemSceneNode;
	class IDummyTransformationSceneNode;
	class ITriangleSelector;
	class IMetaTriangleSelector;
	class IMeshManipulator;
	class ITextSceneNode;

	//!	The Scene Manager manages scene nodes, mesh recources, cameras and all the other stuff.
	/** All Scene nodes can be created only here. There is a always growing list of scene 
	nodes for lots of purposes: Indoor rendering scene nodes like the Octree 
	(addOctTreeSceneNode()) or the binary space partition tree (addBspTreeSceneNode()),
	different Camera scene nodes (addCameraSceneNode(), addCameraSceneNodeMaya()),
	scene nodes for Light (addLightSceneNode()), Billboards (addBillboardSceneNode())
	and so on.
	A scene node is a node in the hirachical scene graph. Every scene node may have children,
	which are other scene nodes. Children move relative the their parents position. If the parent of a node is not
	visible, its children won't be visible too. In this way, it is for example easily possible
	to attach a light to a moving car, or to place a walking character on a moving platform
	on a moving ship.
	*/
	class ISceneManager : public IUnknown
	{
	public:

		//! destructor
		virtual ~ISceneManager() {};

		//! Gets pointer to an animateable mesh. Loads it if needed.
		//! Currently there are the following mesh formats supported:
		//! .obj(Alias Wavefront Maya), .ms3d(Milkshape3D), .bsp(Quake3 Level),
		//! .md2(Quake2 Model), .3ds(3D Studio), .x(Microsoft DirectX) More formats coming soon,
		//! make a feature request on the Irrlicht Engine homepage if you like.
		//! Special thanks go to Dean P. Macri who extended the Quake 3
		//! .bsp loader with the curved surfaces feature.
		//! \param filename: Filename of the mesh to load.
		//! \return Returns NULL if failed and the pointer to the mesh if 
		//! successful. 
		//! This pointer should not be dropped. See IUnknown::drop() for more information.
		virtual IAnimatedMesh* getMesh(const c8* filename) = 0;

		//! Returns the video driver.
		//! \return Returns pointer to the video Driver.
		//! This pointer should not be dropped. See IUnknown::drop() for more information.
		virtual video::IVideoDriver* getVideoDriver() = 0;

		//! Adds a test scene node for test purposes of the scene. It is a simple cube of (1,1,1) size. 
		//! \param size: Size of the cube.
		//! \param parent: Parent of the scene node. Can be NULL if no parent.
		//! \param id: Id of the node. This id can be used to identify the scene node.
		//! \param position: Position of the space relative to its parent where the
		//! scene node will be placed.
		//! \param rotation: Initital rotation of the scene node.
		//! \param scale: Initial scale of the scene node.
		//! \return Returns pointer to the created test scene node.
		//! This pointer should not be dropped. See IUnknown::drop() for more information.
		virtual ISceneNode* addTestSceneNode(f32 size=10.0f, ISceneNode* parent=0, s32 id=-1,
			const core::vector3df& position = core::vector3df(0,0,0),
			const core::vector3df& rotation = core::vector3df(0,0,0),
			const core::vector3df& scale = core::vector3df(1.0f, 1.0f, 1.0f)) = 0;

		//! Adds a scene node for rendering an animated mesh model.
		//! \param mesh: Pointer to the loaded animated mesh to be displayed.
		//! \param parent: Parent of the scene node. Can be NULL if no parent.
		//! \param id: Id of the node. This id can be used to identify the scene node.
		//! \param position: Position of the space relative to its parent where the
		//! scene node will be placed.
		//! \param rotation: Initital rotation of the scene node.
		//! \param scale: Initial scale of the scene node.
		//! \return Returns pointer to the created scene node.
		//! This pointer should not be dropped. See IUnknown::drop() for more information.
		virtual IAnimatedMeshSceneNode* addAnimatedMeshSceneNode(IAnimatedMesh* mesh, ISceneNode* parent=0, s32 id=-1,
			const core::vector3df& position = core::vector3df(0,0,0),
			const core::vector3df& rotation = core::vector3df(0,0,0),
			const core::vector3df& scale = core::vector3df(1.0f, 1.0f, 1.0f)) = 0;

		//! Adds a scene node for rendering a static mesh.
		//! \param mesh: Pointer to the loaded static mesh to be displayed.
		//! \param parent: Parent of the scene node. Can be NULL if no parent.
		//! \param id: Id of the node. This id can be used to identify the scene node.
		//! \param position: Position of the space relative to its parent where the
		//! scene node will be placed.
		//! \param rotation: Initital rotation of the scene node.
		//! \param scale: Initial scale of the scene node.
		//! \return Returns pointer to the created scene node.
		//! This pointer should not be dropped. See IUnknown::drop() for more information.
		virtual ISceneNode* addMeshSceneNode(IMesh* mesh, ISceneNode* parent=0, s32 id=-1,
			const core::vector3df& position = core::vector3df(0,0,0),
			const core::vector3df& rotation = core::vector3df(0,0,0),
			const core::vector3df& scale = core::vector3df(1.0f, 1.0f, 1.0f)) = 0;

		//! Adds a scene node for rendering a animated water surface mesh.
		//! Looks really good when the Material type EMT_TRANSPARENT_REFLECTION
		//! is used.
		//! \param waveHeight: Height of the water waves.
		//! \param waveSpeed: Speed of the water waves.
		//! \param waveLenght: Lenght of a water wave.
		//! \param mesh: Pointer to the loaded static mesh to be displayed with water waves on it.
		//! \param parent: Parent of the scene node. Can be NULL if no parent.
		//! \param id: Id of the node. This id can be used to identify the scene node.
		//! \param position: Position of the space relative to its parent where the
		//! scene node will be placed.
		//! \param rotation: Initital rotation of the scene node.
		//! \param scale: Initial scale of the scene node.
		//! \return Returns pointer to the created scene node.
		//! This pointer should not be dropped. See IUnknown::drop() for more information.
		virtual ISceneNode* addWaterSurfaceSceneNode(IMesh* mesh,
			f32 waveHeight=2.0f, f32 waveSpeed=300.0f, f32 waveLenght=10.0f, 
			ISceneNode* parent=0, s32 id=-1,
			const core::vector3df& position = core::vector3df(0,0,0),
			const core::vector3df& rotation = core::vector3df(0,0,0),
			const core::vector3df& scale = core::vector3df(1.0f, 1.0f, 1.0f)) = 0;

		//! Adds a scene node for rendering using a binary space partition tree.
		//! This is a nice method for rendering indoor scenes. Based on a mesh given as parameter, the
		//! tree will be build. Note that The build process can be slow, if there are lots of polygons.
		//! Note that the implementation of the BspTree in the Irrlicht Engine is currently
		//! not complete a little bit buggy. You may want to use the OctTree instead, it already works.
		//! Use ISceneManager::addOctTreeSceneNode() for that.
		//! \param mesh: The mesh containing all geometry from which the binary space partition tree will be build.
		//! \param parent: Parent node of the bsp tree node.
		//! \param id: id of the node. This id can be used to identify the node.
		//! \return Returns the pointer to the IBspTreeSceneNode if successful, otherwise 0. 
		//! This pointer should not be dropped. See IUnknown::drop() for more information.
		virtual IBspTreeSceneNode* addBspTreeSceneNode(IMesh* mesh, ISceneNode* parent=0, s32 id=-1) = 0;

		//! Adds a scene node for rendering using a octtree to the scene graph. This a good method for rendering 
		//! scenes with lots of geometry. The Octree is built on the fly from the mesh, much
		//! faster then a bsp tree.
		//! \param mesh: The mesh containing all geometry from which the octtree will be build.
		//! If this animated mesh has more than one frames in it, the first frame is taken.
		//! \param parent: Parent node of the octtree node.
		//! \param id: id of the node. This id can be used to identify the node.
		//! \param minimalPolysPerNode: Specifies the minimal polygons contained a octree node.
		//! If a node gets less polys the this value, it will not be splitted into
		//! smaller nodes.
		//! \return Returns the pointer to the octtree if successful, otherwise 0. 
		//! This pointer should not be dropped. See IUnknown::drop() for more information.
		virtual ISceneNode* addOctTreeSceneNode(IAnimatedMesh* mesh, ISceneNode* parent=0, 
			s32 id=-1, s32 minimalPolysPerNode=128) = 0;

		//! Adds a scene node for rendering using a octtree to the scene graph. This a good method for rendering 
		//! scenes with lots of geometry. The Octree is built on the fly from the mesh, much
		//! faster then a bsp tree.
		//! \param mesh: The mesh containing all geometry from which the octtree will be build.
		//! \param parent: Parent node of the octtree node.
		//! \param id: id of the node. This id can be used to identify the node.
		//! \param minimalPolysPerNode: Specifies the minimal polygons contained a octree node.
		//! If a node gets less polys the this value, it will not be splitted into
		//! smaller nodes.
		//! \return Returns the pointer to the octtree if successful, otherwise 0. 
		//! This pointer should not be dropped. See IUnknown::drop() for more information.
		virtual ISceneNode* addOctTreeSceneNode(IMesh* mesh, ISceneNode* parent=0, 
			s32 id=-1, s32 minimalPolysPerNode=128) = 0;

		//! Adds a camera scene node to the scene graph and sets it as active camera.
		//! \param position: Position of the space relative to its parent where the camera will be placed.
		//! \param lookat: Position where the camera will look at. Also known as target.
		//! \param parent: Parent scene node of the camera. Can be null. If the parent moves,
		//! the camera will move too.
		//! \param id: id of the camera. This id can be used to identify the camera.
		//! \return Returns pointer to interface to camera if successful, otherwise 0.
		//! This pointer should not be dropped. See IUnknown::drop() for more information.
		virtual ICameraSceneNode* addCameraSceneNode(ISceneNode* parent = 0,
			const core::vector3df& position = core::vector3df(0,0,0), 
			const core::vector3df& lookat = core::vector3df(0,0,100), s32 id=-1) = 0;

		//! Adds a camera scene node which is able to be controlled with the mouse similar
		//! like in the 3D Software Maya by Alias Wavefront.
		//! \param parent: Parent scene node of the camera. Can be null.
		//! \param rotateSpeed: Rotation speed of the camera.
		//! \param zoomSpeed: Zoom speed of the camera.
		//! \param tranlationSpeed: TranslationSpeed of the camera.
		//! \param id: id of the camera. This id can be used to identify the camera.
		//! \return Returns a pointer to the interface of the camera if successful, otherwise 0.
		//! This pointer should not be dropped. See IUnknown::drop() for more information.
		virtual ICameraSceneNode* addCameraSceneNodeMaya(ISceneNode* parent = 0,
			f32 rotateSpeed = -1500.0f, f32 zoomSpeed = 200.0f, f32 translationSpeed = 1500.0f, s32 id=-1) = 0;

		//! Adds a camera scene node which is able to be controled with the mouse and keys
		//! like in most first person shooters (FPS): 
		//! Look with the mouse, move with cursor keys. If you do not like the default
		//! key layout, you may want to specify your own. For example to make the camera
		//! be controlled by the cursor keys AND the keys W,A,S, and D, do something
		//! like this:
		//! \code
		//! SKeyMap keyMap[8];
		//! keyMap[0].Action = EKA_MOVE_FORWARD;
		//! keyMap[0].KeyCode = KEY_UP;
		//! keyMap[1].Action = EKA_MOVE_FORWARD;
		//! keyMap[1].KeyCode = KEY_KEY_W;
		//!
		//! keyMap[2].Action = EKA_MOVE_BACKWARD;
		//! keyMap[2].KeyCode = KEY_DOWN;
		//! keyMap[3].Action = EKA_MOVE_BACKWARD;
		//! keyMap[3].KeyCode = KEY_KEY_S;
		//!
		//! keyMap[4].Action = EKA_STRAFE_LEFT;
		//! keyMap[4].KeyCode = KEY_LEFT;
		//! keyMap[5].Action = EKA_STRAFE_LEFT;
		//! keyMap[5].KeyCode = KEY_KEY_A;
		//!
		//! keyMap[6].Action = EKA_STRAFE_RIGHT;
		//! keyMap[6].KeyCode = KEY_RIGHT;
		//! keyMap[7].Action = EKA_STRAFE_RIGHT;
		//! keyMap[7].KeyCode = KEY_KEY_D;
		//!
		//! camera = sceneManager->addCameraSceneNodeFPS(0, 100, 500, -1, keyMap, 8);
		//! \endcode
		//! \param parent: Parent scene node of the camera. Can be null.
		//! \param rotateSpeed: Speed with wich the camera is rotated. This can be done
		//! only with the mouse.
		//! \param movespeed: Speed with which the camera is moved. Movement is done with
		//! the cursor keys.
		//! \param id: id of the camera. This id can be used to identify the camera.
		//! \param keyMapArray: Optional pointer to an array of a keymap, specifying what
		//! keys should be used to move the camera. If this is null, the default keymap
		//! is used. You can define actions more then one time in the array, to bind 
		//! multiple keys to the same action.
		//! \param keyMapSize: Amount of items in the keymap array.
		//! \return Returns a pointer to the interface of the camera if successful, otherwise 0.
		//! This pointer should not be dropped. See IUnknown::drop() for more information.
		virtual ICameraSceneNode* addCameraSceneNodeFPS(ISceneNode* parent = 0,
			f32 rotateSpeed = 100.0f, f32 moveSpeed = 500.0f, s32 id=-1,
			SKeyMap* keyMapArray=0, s32 keyMapSize=0) = 0;

		//! Adds a dynamic light scene node to the scene graph. The light will cast dynamic light on all
		//! other scene nodes in the scene, which have the material flag video::MTF_LIGHTING
		//! turned on. (This is the default setting in most scene nodes).
		//! \param parent: Parent scene node of the light. Can be null. If the parent moves,
		//! the light will move too.
		//! \param position: Position of the space relative to its parent where the light will be placed.
		//! \param color: Diffuse color of the light. Ambient or Specular colors can be set manually with
		//! the ILightSceneNode::getLightData() method.
		//! \param radius: Radius of the light.
		//! \param id: id of the node. This id can be used to identify the node.
		//! \return Returns pointer to the interface of the light if successful, otherwise NULL.
		//! This pointer should not be dropped. See IUnknown::drop() for more information.
		virtual ILightSceneNode* addLightSceneNode(ISceneNode* parent = 0,
			const core::vector3df& position = core::vector3df(0,0,0),
			video::SColorf color = video::SColorf(1.0f, 1.0f, 1.0f),	f32 radius=100.0f, s32 id=-1) = 0;

		//! Adds a billboard scene node to the scene graph. A billboard is like a 3d sprite: A 2d element,
		//! which always looks to the camera. It is usually used for things like explosions, fire,
		//! lensflares and things like that.
		//! \param parent: Parent scene node of the billboard. Can be null. If the parent moves,
		//! the billboard will move too.
		//! \param position: Position of the space relative to its parent where the billboard will be placed.
		//! \param size: Size of the billboard. This size is 2 dimensional because a billboard only has
		//! width and height.
		//! \param id: An id of the node. This id can be used to identify the node.
		//! \return Returns pointer to the billboard if successful, otherwise NULL.
		//! This pointer should not be dropped. See IUnknown::drop() for more information.
		virtual IBillboardSceneNode* addBillboardSceneNode(ISceneNode* parent = 0,
			const core::dimension2d<f32>& size = core::dimension2d<f32>(10.0f, 10.0f),
			const core::vector3df& position = core::vector3df(0,0,0), s32 id=-1) = 0;

		//! Adds a skybox scene node to the scene graph. A skybox is a big cube with 6 textures on it and
		//! is drawed around the camera position. 
		//! \param top: Texture for the top plane of the box.
		//! \param bottom: Texture for the bottom plane of the box.
		//! \param left: Texture for the left plane of the box.
		//! \param right: Texture for the right plane of the box.
		//! \param front: Texture for the front plane of the box.
		//! \param parent: Parent scene node of the skybox. A skybox usually has no parent,
		//! so this should be null. Note: If a parent is set to the skybox, the box will not 
		//! change how it is drawed.
		//! \param id: An id of the node. This id can be used to identify the node.
		//! \return Returns a pointer to the sky box if successful, otherwise NULL.
		//! This pointer should not be dropped. See IUnknown::drop() for more information.
		virtual ISceneNode* addSkyBoxSceneNode(video::ITexture* top, video::ITexture* bottom,
			video::ITexture* left, video::ITexture* right, video::ITexture* front,
			video::ITexture* back, ISceneNode* parent = 0, s32 id=-1) = 0;

		//! Adds a particle system scene node to the scene graph.
		//! \param withDefaultEmitter: Creates a default working point emitter
		//! which emitts some particles. Set this to true to see a particle system
		//! in action. If set to false, you'll have to set the emitter you want by 
		//! calling IParticleSystemSceneNode::setEmitter().
		//! \param parent: Parent of the scene node. Can be NULL if no parent.
		//! \param id: Id of the node. This id can be used to identify the scene node.
		//! \param position: Position of the space relative to its parent where the
		//! scene node will be placed.
		//! \param rotation: Initital rotation of the scene node.
		//! \param scale: Initial scale of the scene node.
		//! \return Returns pointer to the created scene node.
		//! This pointer should not be dropped. See IUnknown::drop() for more information.
		virtual IParticleSystemSceneNode* addParticleSystemSceneNode(
			bool withDefaultEmitter=true, ISceneNode* parent=0, s32 id=-1,
			const core::vector3df& position = core::vector3df(0,0,0),
			const core::vector3df& rotation = core::vector3df(0,0,0),
			const core::vector3df& scale = core::vector3df(1.0f, 1.0f, 1.0f)) = 0;

		//! Adds a terrain scene node to the scene graph. This node 
		//! implements is a simple terrain renderer which uses a quad 
		//! tree for culling and a technique known as geo mip mapping
		//! for reducing the detail of triangle blocks which are far away.
		//! Please note, that this is still a very early alpha version. LOD support is currently
		//! disabled for example, so using this with big heightmaps will be extremely slow.
		//! \param parent: Parent scene node of the camera. Can be null.
		//! \param id: id of the camera. This id can be used to identify the camera.
		//! \param texture: Texture for the terrain. This can be a very huge 
		//! texture, for example 8000x8000 pixels, because it will be splitted
		//! into smaller ones. Please note that this is not a 
		//! hardware texture as usual (ITexture), but an IImage software texture.
		//! You can load this texture with IVideoDriver::createImageFromFile(). 
		//! \param heightmap: A grayscaled heightmap image. Like the texture,
		//! it can be created with IVideoDriver::createImageFromFile(). The amount
		//! of triangles created depends on the size of this texture, so use a small
		//! heightmap to increase rendering speed.
		//! \param detailmap: Detail map for the terrain. Can be null.
		//! \param stretchSize: Parameter defining how big a is pixel on the heightmap. 
		//! \param maxHeight: Defines how height a white pixel on the heighmap is.
		//! \return Returns pointer to the created scene node.
		//! This pointer should not be dropped. See IUnknown::drop() for more information.
		virtual ITerrainSceneNode* addTerrainSceneNode(
			ISceneNode* parent, s32 id,
			video::IImage* texture,
			video::IImage* heightmap, video::ITexture* detailmap, 
			const core::dimension2d<f32>& stretchSize = core::dimension2d<f32>(10.0f,10.0f),
			f32 maxHeight=200.0f, 
			const core::dimension2d<s32>& defaultVertexBlockSize = core::dimension2d<s32>(64,64)) = 0;

		//! Adds an empty scene node. Can be used for doing advanced transformations
		//! or structuring the scene graph.
		//! \return Returns pointer to the created scene node.
		//! This pointer should not be dropped. See IUnknown::drop() for more information.
		virtual ISceneNode* addEmptySceneNode(ISceneNode* parent=0, s32 id=-1) = 0;

		//! Adds a dummy transformation scene node to the scene graph.
		//! This scene node does not render itself, and does not respond to set/getPosition,
		//! set/getRotation and set/getScale. Its just a simple scene node that takes a 
		//! matrix as relative transformation, making it possible to insert any transformation
		//! anywhere into the scene graph.
		//! \return Returns pointer to the created scene node.
		//! This pointer should not be dropped. See IUnknown::drop() for more information.
		virtual IDummyTransformationSceneNode* addDummyTransformationSceneNode(
			ISceneNode* parent=0, s32 id=-1) = 0;

		//! Adds a text scene node, which is able to display 
		//! 2d text at a position in three dimensional space
		virtual ITextSceneNode* addTextSceneNode(gui::IGUIFont* font, const wchar_t* text,
			video::SColor color=video::SColor(100,255,255,255), 
			ISceneNode* parent = 0,	const core::vector3df& position = core::vector3df(0,0,0),
			s32 id=-1) = 0;

		//! Adds a Hill Plane mesh to the mesh pool. The mesh is generated on the fly
		//! and looks like a plane with some hills on it. It is uses mostly for quick
		//! tests of the engine only. You can specify how many hills there should be 
		//! on the plane and how high they should be. Also you must specify a name for
		//! the mesh, because the mesh is added to the mesh pool, and can be retieved
		//! again using ISceneManager::getMesh() with the name as parameter.
		//! \param name: The name of this mesh which must be specified in order
		//! to be able to retrieve the mesh later with ISceneManager::getMesh().
		//! \param tileSize: Size of a tile of the mesh. (10.0f, 10.0f) would be a
		//! good value to start, for example.
		//! \param tileCount: Specifies how much tiles there will be. If you specifiy 
		//! for example that a tile has the size (10.0f, 10.0f) and the tileCount is
		//! (10,10), than you get a field of 100 tiles wich has the dimension 100.0fx100.0f.
		//! \param material: Material of the hill mesh.
		//! \param hillHeight: Height of the hills. If you specify a negative value
		//! you will get holes instead of hills. If the height is 0, no hills will be
		//! created.
		//! \param countHills: Amount of hills on the plane. There will be countHills.X
		//! hills along the X axis and countHills.Y along the Y axis. So in total there 
		//! will be countHills.X * countHills.Y hills.
		//! \param textureRepeatCount: Defines how often the texture will be repeated in
		//! x and y direction.
        //! \return Returns null if the creation failed. The reason could be that you 
		//! specified some invalid parameters or that a mesh with that name already
		//! exists. If successful, a pointer to the mesh is returned. 
		//! This pointer should not be dropped. See IUnknown::drop() for more information.
		virtual IAnimatedMesh* addHillPlaneMesh(const c8* name,
			const core::dimension2d<f32>& tileSize, const core::dimension2d<s32>& tileCount,
			video::SMaterial* material = 0,	f32 hillHeight = 0.0f, 
			const core::dimension2d<f32>& countHills = core::dimension2d<f32>(0.0f, 0.0f),
			const core::dimension2d<f32>& textureRepeatCount = core::dimension2d<f32>(1.0f, 1.0f)) = 0;

		//! Adds a static terrain mesh to the mesh pool. The mesh is generated on the fly
		//! from a texture file and a height map file. Both files may be huge
		//! (8000x8000 pixels would be no problem) because the generator splits the
		//! files into smaller textures if necessary. 
		//! You must specify a name for the mesh, because the mesh is added to the mesh pool,
		//! and can be retieved again using ISceneManager::getMesh() with the name as parameter.
		//! \param meshname: The name of this mesh which must be specified in order
		//! to be able to retrieve the mesh later with ISceneManager::getMesh().
		//! \param texture: Texture for the terrain. Please note that this is not a 
		//! hardware texture as usual (ITexture), but an IImage software texture.
		//! You can load this texture with IVideoDriver::createImageFromFile(). 
		//! \param heightmap: A grayscaled heightmap image. Like the texture,
		//! it can be created with IVideoDriver::createImageFromFile(). The amount
		//! of triangles created depends on the size of this texture, so use a small
		//! heightmap to increase rendering speed.
		//! \param stretchSize: Parameter defining how big a is pixel on the heightmap. 
		//! \param maxHeight: Defines how height a white pixel on the heighmap is.
		//! \return Returns null if the creation failed. The reason could be that you 
		//! specified some invalid parameters, that a mesh with that name already
		//! exists, or that a texture could not be found. If successful, a pointer to the mesh is returned. 
		//! This pointer should not be dropped. See IUnknown::drop() for more information.
		virtual IAnimatedMesh* addTerrainMesh(const c8* meshname,
			video::IImage* texture, video::IImage* heightmap,
			const core::dimension2d<f32>& stretchSize = core::dimension2d<f32>(10.0f,10.0f),
			f32 maxHeight=200.0f, 
			const core::dimension2d<s32>& defaultVertexBlockSize = core::dimension2d<s32>(64,64)) = 0;

		//! Returns the root scene node. This is the scene node wich is parent 
		//! of all scene nodes. The root scene node is a special scene node which
		//! only exists to manage all scene nodes. It is not rendered and cannot
		//! be removed from the scene.
		//! \return Returns a pointer to the root scene node.
		virtual ISceneNode* getRootSceneNode() = 0;

		//! Returns the first scene node with the specified id.
		//! \param id: The id to search for
		//! \param start: Scene node to start from. All children of this scene
		//! node are searched. If null is specified, the root scene node is
		//! taken.
		//! \return Returns pointer to the first scene node with this id,
		//! and null if no scene node could be found.
		virtual ISceneNode* getSceneNodeFromId(s32 id, ISceneNode* start=0) = 0;

		//! Returns the current active camera.
		//! \return The active camera is returned. Note that this can be NULL, if there
		//! was no camera created yet.
		virtual ICameraSceneNode* getActiveCamera() = 0;

		//! Sets the active camera. The previous active camera will be deactivated.
		//! \param camera: The new camera which should be active.
		virtual void setActiveCamera(ICameraSceneNode* camera) = 0;

		//! Sets the color of stencil buffers shadows drawed by the scene manager.
		virtual void setShadowColor(video::SColor color = video::SColor(150,0,0,0)) = 0;

		//! Returns the current color of shadows.
		virtual video::SColor getShadowColor() const = 0;

		//! Registers a node for rendering it at a specific time.
		//! This method should only be used by SceneNodes when they get a 
		//! ISceneNode::OnPreRender() call.
		virtual void registerNodeForRendering(ISceneNode* node, E_SCENE_NODE_RENDER_TIME time = SNRT_DEFAULT) = 0;

		//! Draws all the scene nodes. This can only be invoked between
		//! IVideoDriver::beginScene() and IVideoDriver::endScene().
		virtual void drawAll() = 0;

		//! Creates a rotation animator, which rotates the attached scene node around itself.
		//! \param rotationPerSecond: Specifies the speed of the animation
		//! \return Returns the animator. Attach it to a scene node with ISceneNode::addAnimator()
		//! and the animator will animate it.
		//! If you no longer need the animator, you should call ISceneNodeAnimator::drop().
		//! See IUnknown::drop() for more information.
		virtual ISceneNodeAnimator* createRotationAnimator(const core::vector3df& rotationPerSecond) = 0;

		//! Creates a fly circle animator, which lets the attached scene node fly 
		//! around a center.
		//! \param center: Center of the circle.
		//! \param radius: Radius of the circle.
		//! \param speed: Specifies the speed of the flight.
		//! \return Returns the animator. Attach it to a scene node with ISceneNode::addAnimator()
		//! and the animator will animate it.
		//! If you no longer need the animator, you should call ISceneNodeAnimator::drop().
		//! See IUnknown::drop() for more information.
		virtual ISceneNodeAnimator* createFlyCircleAnimator(const core::vector3df& center, 
			f32 radius, f32 speed=0.001f) = 0;

		//! Creates a fly straight animator, which lets the attached scene node
		//! fly or move along a line between two points.
		//! \param startPoint: Start point of the line.
		//! \param endPoint: End point of the line.
		//! \param timeForWay: Time in milli seconds how long the node should need to
		//! move from the start point to the end point.
		//! \param loop: If set to false, the node stops when the end point is reached.
		//! If loop is true, the node begins again at the start.
		//! \return Returns the animator. Attach it to a scene node with ISceneNode::addAnimator()
		//! and the animator will animate it.
		//! If you no longer need the animator, you should call ISceneNodeAnimator::drop().
		//! See IUnknown::drop() for more information.
		virtual ISceneNodeAnimator* createFlyStraightAnimator(const core::vector3df& startPoint,
			const core::vector3df& endPoint, u32 timeForWay, bool loop=false) = 0;

		//! Creates a texture animator, which switches the textures of the target scene
		//! node based on a list of textures.
		//! \param textures: List of textures to use.
		//! \param timePerFrame: Time in milliseconds, how long any texture in the list
		//! should be visible.
		//! \param loop: If set to to false, the last texture remains set, and the animation
		//! stops. If set to true, the animation restarts with the first texture.
		//! \return Returns the animator. Attach it to a scene node with ISceneNode::addAnimator()
		//! and the animator will animate it.
		//! If you no longer need the animator, you should call ISceneNodeAnimator::drop().
		//! See IUnknown::drop() for more information.
		virtual ISceneNodeAnimator* createTextureAnimator(const core::array<video::ITexture*>& textures,
			s32 timePerFrame, bool loop=true) = 0;

		//! Creates a scene node animator, which deletes the scene node after
		//! some time automaticly.
		//! \param when: Time in milliseconds, after when the node will be deleted.
		//! \return Returns the animator. Attach it to a scene node with ISceneNode::addAnimator()
		//! and the animator will animate it.
		//! If you no longer need the animator, you should call ISceneNodeAnimator::drop().
		//! See IUnknown::drop() for more information.
		virtual ISceneNodeAnimator* createDeleteAnimator(u32 timeMs) = 0;

		//! Creates a special scene node animator for doing automatic collision detection 
		//! and response. See ISceneNodeAnimatorCollisionResponse for details.
		//! \param world: Triangle selector holding all triangles of the world with which
		//! the scene node may collide. You can create a triangle selector with 
		//! ISceneManager::createTriangleSelector();
		//! \param sceneNode: SceneNode which should be manipulated. After you added this animator
		//! to the scene node, the scene node will not be able to move through walls and is
		//! affected by gravity.
		//! \param ellipsoidRadius: Radius of the ellipsoid with which collision detection and
		//! response is done.  If you have got a scene node, and you are unsure about
		//! how big the radius should be, you could use the following code to determine
		//! it:
		//! \code
		//! core::aabbox<f32> box = yourSceneNode->getBoundingBox();
		//! core::vector3df radius = box.MaxEdge - box.getCenter();
		//! \endcode
		//! \param gravityPerSecond: Sets the gravity of the environment. A good example value would be
		//! core::vector3df(0,-100.0f,0) for letting gravity affect all object to
		//! fall down. For bigger gravity, make increase the length of the vector.
		//! You can disable gravity by setting it to core::vector3df(0,0,0).
		//! \param accelerationPerSecond: Sets the acceleration per second value. This value controls how much
		//! gravity is increased by time, if the scene node is falling down.
		//! \param ellipsoidTranslation: By default, the ellipsoid for collision detection is created around
		//! the center of the scene node, which means that the ellipsoid surrounds
		//! it completely. If this is not what you want, you may specify a translation
		//! for the ellipsoid.
		//! \return Returns the animator. Attach it to a scene node with ISceneNode::addAnimator()
		//! and the animator will cause it to do collision detection and response.
		//! If you no longer need the animator, you should call ISceneNodeAnimator::drop().
		//! See IUnknown::drop() for more information.
		virtual ISceneNodeAnimatorCollisionResponse* createCollisionResponseAnimator(
			ITriangleSelector* world, ISceneNode* sceneNode, 
			const core::vector3df& ellipsoidRadius = core::vector3df(30,60,30),
			const core::vector3df& gravityPerSecond = core::vector3df(0,-100.0f,0),
			f32 accelerationPerSecond=100.0f,
			const core::vector3df& ellipsoidTranslation = core::vector3df(0,0,0),
			f32 slidingValue = 0.0005f) = 0;

		//! Creates a follow spline animator. The animator modifies the position of
		//! the attached scene node to make it follow a hermite spline. 
		//! The code of the is based on a scene node
		//! Matthias Gall sent in. Thanks! I adapted the code just a little bit. Matthias 
		//! wrote:
 		//! Uses a subset of hermite splines: either cardinal splines (tightness != 0.5) or catmull-rom-splines (tightness == 0.5)
		//! but this is just my understanding of this stuff, I'm not a mathematician, so this might be wrong ;)
        virtual ISceneNodeAnimator* createFollowSplineAnimator(s32 startTime,
			const core::array< core::vector3df >& points,
			f32 speed = 1.0f, f32 tightness = 0.5f) = 0;

		//! Creates a simple ITriangleSelector, based on a mesh. Triangle selectors
		//! can be used for doing collision detection. Don't use this selector
		//! for a huge amount of triangles like in Quake3 maps.
		//! Instead, use for example ISceneManager::createOctTreeTriangleSelector().
		//! Please note that the created triangle selector is not automaticly attached
		//! to the scene node. You will have to call ISceneNode::setTriangleSelector()
		//! for this. To create and attach a triangle selector is done like this:
		//! \code
		//! ITriangleSelector* s = sceneManager->createTriangleSelector(yourMesh,
		//!		yourSceneNode);
		//! yourSceneNode->setTriangleSelector(s);
		//! s->drop();
		//! \endcode
		//! \param mesh: Mesh of which the triangles are taken.
		//! \param node: Scene node of which visibility and transformation is used.
		//! \return Returns the selector, or null if not successful.
		//! If you no longer need the selector, you should call ITriangleSelector::drop().
		//! See IUnknown::drop() for more information.
		virtual ITriangleSelector* createTriangleSelector(IMesh* mesh, ISceneNode* node) = 0;

		//! Creates a simple dynamic ITriangleSelector, based on a axis aligned bounding box. Triangle selectors
		//! can be used for doing collision detection. Every time when triangles are 
		//! queried, the triangle selector gets the bounding box of the scene node,
		//! an creates new triangles. In this way, it works good with animated scene nodes.
		//! \param node: Scene node of which the bounding box, visibility and transformation is used.
		//! \return Returns the selector, or null if not successful.
		//! If you no longer need the selector, you should call ITriangleSelector::drop().
		//! See IUnknown::drop() for more information.
		virtual ITriangleSelector* createTriangleSelectorFromBoundingBox(ISceneNode* node) = 0;

		//! Creates a simple ITriangleSelector, based on a mesh. Triangle selectors
		//! can be used for doing collision detection. This triangle selector is
		//! optimized for huge amounts of triangle, it organizes them in an octtree.
		//! Please note that the created triangle selector is not automaticly attached
		//! to the scene node. You will have to call ISceneNode::setTriangleSelector()
		//! for this. To create and attach a triangle selector is done like this:
		//! \code
		//! ITriangleSelector* s = sceneManager->createOctTreeTriangleSelector(yourMesh,
		//!		yourSceneNode);
		//! yourSceneNode->setTriangleSelector(s);
		//! s->drop();
		//! \endcode
		//! \param mesh: Mesh of which the triangles are taken.
		//! \param node: Scene node of which visibility and transformation is used.
		//! \param minimalPolysPerNode: Specifies the minimal polygons contained a octree node.
		//! If a node gets less polys the this value, it will not be splitted into
		//! smaller nodes.
		//! \return Returns the selector, or null if not successful.
		//! If you no longer need the selector, you should call ITriangleSelector::drop().
		//! See IUnknown::drop() for more information.
		virtual ITriangleSelector* createOctTreeTriangleSelector(IMesh* mesh,
			ISceneNode* node, s32 minimalPolysPerNode=32) = 0;

		//! Creates a meta triangle selector which is nothing more than a 
		//! collection of one or more triangle selectors providing together
		//! the interface of one triangle selector. In this way, 
		//! collision tests can be done with different triangle soups in one pass.
		//! \return Returns the selector, or null if not successful.
		//! If you no longer need the selector, you should call ITriangleSelector::drop().
		//! See IUnknown::drop() for more information.
		virtual IMetaTriangleSelector* createMetaTriangleSelector() = 0;

		//! Adds an external mesh loader. If you want the engine to be extended with
		//! file formats it currently is not able to load (e.g. .cob), just implement
		//! the IMeshLoader interface in your loading class and add it with this method.
		//! \param externalLoader: Implementation of a new mesh loader.
		virtual void addExternalMeshLoader(IMeshLoader* externalLoader) = 0;

		//! Returns a pointer to the scene collision manager.
		virtual ISceneCollisionManager* getSceneCollisionManager() = 0;

		//! Returns a pointer to the mesh manipulator.
		virtual IMeshManipulator* getMeshManipulator() = 0;

		//! Adds a scene node to the deletion queue. The scene node is immediatly
		//! deleted when it's secure. Which means when the scene node does not
		//! execute animators and things like that. This method is for example
		//! used for deleting scene nodes by their scene node animators. In
		//! most other cases, a ISceneNode::remove() call is enough, using this
		//! deletion queue is not necessary.
		//! See ISceneManager::createDeleteAnimator() for details.
		//! \param node: Node to detete.
		virtual void addToDeletionQueue(ISceneNode* node) = 0;

		//! Posts an input event to the environment. Usually you do not have to
		//! use this method, it is used by the internal engine.
		virtual bool postEventFromUser(SEvent event) = 0;

		//! Clears the whole scene. All scene nodes are removed. 
		virtual void clear() = 0;
	};


} // end namespace scene
} // end namespace irr

#endif

