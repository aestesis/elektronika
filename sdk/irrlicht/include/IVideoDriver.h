// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __IRR_I_VIDEO_DRIVER_H_INCLUDED__
#define __IRR_I_VIDEO_DRIVER_H_INCLUDED__

#include "rect.h"
#include "SColor.h"
#include "ITexture.h"
#include "matrix4.h"
#include "dimension2d.h"
#include "position2d.h"
#include "IReadFile.h"
#include "SMaterial.h"
#include "SLight.h"
#include "IImageLoader.h"
#include "triangle3d.h"
#include "SExposedVideoData.h"
#include "IMaterialRenderer.h"
#include "EDriverTypes.h"
#include "IGPUProgrammingServices.h"

namespace irr
{
namespace scene
{
	class IMeshBuffer;
}

namespace video
{
	struct S3DVertex;
	struct S3DVertex2TCoords;

	//! enumeration for querying features of the video driver.
	enum E_VIDEO_DRIVER_FEATURE
	{
		//! Is driver able to render to a surface?
		EVDF_RENDER_TO_TARGET = 0,	

		//! Is driver able to render with a bilinear filter applied?
		EVDF_BILINEAR_FILER,			

		//! Is hardeware transform and lighting supported?
		EVDF_HARDWARE_TL,			

		//! Can the driver handle mip maps?
		EVDF_MIP_MAP,		

		//! Are stencilbuffers switched on and does the device support stencil buffers?
		EVDF_STENCIL_BUFFER,

		//! Is Vertex Shader 1.1 supported?
		EVDF_VERTEX_SHADER_1_1,

		//! Is Vertex Shader 2.0 supported?
		EVDF_VERTEX_SHADER_2_0,

		//! Is Vertex Shader 3.0 supported?
		EVDF_VERTEX_SHADER_3_0,

		//! Is Pixel Shader 1.1 supported?
		EVDF_PIXEL_SHADER_1_1,

		//! Is Pixel Shader 1.2 supported?
		EVDF_PIXEL_SHADER_1_2,

		//! Is Pixel Shader 1.3 supported?
		EVDF_PIXEL_SHADER_1_3,

		//! Is Pixel Shader 1.4 supported?
		EVDF_PIXEL_SHADER_1_4,

		//! Is Pixel Shader 2.0 supported?
		EVDF_PIXEL_SHADER_2_0,

		//! Is Pixel Shader 3.0 supported?
		EVDF_PIXEL_SHADER_3_0,

		//! Are ARB vertex programs v1.0 supported?
		EVDF_ARB_VERTEX_PROGRAM_1,

		//! Are ARB fragment programs v1.0 supported?
		EVDF_ARB_FRAGMENT_PROGRAM_1,
	};

	//! enumeration for geometry transformation states
	enum E_TRANSFORMATION_STATE
	{
		//! View transformation
		ETS_VIEW = 0,
		//! World transformation
		ETS_WORLD,
		//! Projection transformation
		ETS_PROJECTION,
		//! Not used
		ETS_COUNT
	};


	//! Interface to driver which is able to perform 2d and 3d gfx functions.
	/** The IVideoDriver interface is one of the most important interfaces of
	the Irrlicht Engine: All rendering and texture manipulating is done with
	this interface. You are able to use the Irrlicht Engine by only invoking methods
	of this interface if you would like to, although the irr::scene::ISceneManager interface
	provides a lot of powerful classes and methods to make the programmers life
	easier.
	*/
	class IVideoDriver : public IUnknown
	{
	public:

		//! destructor
		virtual ~IVideoDriver() {}

		//! Applications must call this method before performing any rendering. 
		//! \param backBuffer: Specifies if the back buffer should be cleared, which
		//! means that the screen is filled with a color specified with the parameter color.
		//! If this parameter is false, the back buffer will not be cleared and the color
		//! parameter is ignored.
		//! \param zBuffer: Speciefies if the depth or z buffer should be cleared. It is
		//! not nesesarry to do so, if only 2d drawing is used.
		//! \return Returns false if failed. Begin Scene can clear the back- and the z-buffer.
		virtual bool beginScene(bool backBuffer, bool zBuffer, SColor color) = 0;

		//! Presents the rendered image on the screen.
		//! Applications must call this method after performing any rendering. 
		//! \return Returns false if failed and true if succeeded.
		virtual bool endScene() = 0;

		//! Queries the features of the driver, returns true if a feature is available
		//! \param feature: A feature to query.
		//! \return Returns true if the feature is available, false if not.
		virtual bool queryFeature(E_VIDEO_DRIVER_FEATURE feature) = 0;

		//! Sets the view, world or projection transformation. 
		//! \param state: Transformation type to be set. Can be view, world or projection.
		//! \param mat: Matrix describing the transformation.
		virtual void setTransform(E_TRANSFORMATION_STATE state, const core::matrix4& mat) = 0;

		//! Returns the transformation set by setTransform
		virtual core::matrix4 getTransform(E_TRANSFORMATION_STATE state) = 0;

		//! Sets a material. All 3d drawing functions draw geometry now
		//! using this material.
		//! \param material: Material to be used from now on.
		virtual void setMaterial(const SMaterial& material) = 0;

		//! Returns a pointer to a texture. Loads the texture if it is not
		//! already loaded, and generates mipmap levels if wished.
		//! The texture can be in BMP, JPG, TGA, PCX and PSD format.
		//! For loading BMP, TGA, PCX and PSD files, the engine uses its own methods.
		//! PCX loading is based on some code by Dean P. Macri, who sent
		//! it in for free use by the engine. It currently not supports
		//! all .pcx formats. Also, compressed BMP's and TGAs don't work correctly.
		//! For loading JPG-Files the JPEG LIB 6b, written by 
		//! The Independent JPEG Group is used by the engine. Thanx for such a great
		//! library!
		//! \param filename: Filename of the texture to be loaded.
		//! \return Returns a pointer to the texture and NULL if the texture
		//! could not be loaded. 
		//! This pointer should not be dropped. See IUnknown::drop() for more information.
		virtual ITexture* getTexture(const c8* filename) = 0;

		//! Returns a pointer to a texture. Loads the texture if it is not
		//! already loaded, and generates mipmap levels if wished.
		//! The texture can be in BMP, JPG, TGA, PCX and PSD format.
		//! For loading BMP, TGA, PCX and PSD files, the engine uses its own methods.
		//! PCX loading is based on some code by Dean P. Macri, who sent
		//! it in for free use by the engine. It currently not supports
		//! all .pcx formats. Also, compressed BMP's and TGAs don't work correctly.
		//! For loading JPG-Files the JPEG LIB 6b, written by 
		//! The Independent JPEG Group is used by the engine. Thanx for such a great
		//! library!
		//! \param file: Pointer to an already opened file.
		//! \return Returns a pointer to the texture and NULL if the texture
		//! could not be loaded. 
		//! This pointer should not be dropped. See IUnknown::drop() for more information.
		virtual ITexture* getTexture(io::IReadFile* file) = 0;

		//! Creates an empty Texture of specified size.
		//! \param size: Size of the texture.
		//! \param name: A name for the texture. Later calls of getTexture() with this name
		//! will return this texture
		//! \param format: Desired color format of the texture. Please note that
		//! the driver may choose to create the texture in another color format.
		//! \return Returns a pointer to the new created Texture. 
		//! This pointer should not be dropped. See IUnknown::drop() for more information.
		//! The format of the new texture will be chosen by the driver, and will in most 
		//! cases have the ECF_A1R5G5B5 or ECF_A8R8G8B8 format.
		virtual ITexture* addTexture(const core::dimension2d<s32>& size,
			const c8* name, ECOLOR_FORMAT format = ECF_A1R5G5B5) = 0;

		//! Creates a texture from a loaded IImage.
		//! \param name: A name for the texture. Later calls of getTexture() with this name
		//! will return this texture
		//! \param image: Image from which the texture is created from.
		//! \return Returns a pointer to the new created Texture. 
		//! This pointer should not be dropped. See IUnknown::drop() for more information.
		//! The format of the new texture will be chosen by the driver, and will in most 
		//! cases have the ECF_A1R5G5B5 or ECF_A8R8G8B8 format.
		virtual ITexture* addTexture(const c8* name, IImage* image) = 0;

		//! Removes a texture from the texture cache and deletes it, freeing lot of
		//! memory. Please note that after calling this, the pointer to the ITexture
		//! may not be longer valid, if it was not grabbed before by other parts of 
		//! the engine for storing it longer. So it would be a good idea to set all
		//! materials which are using this texture to null or another texture first.
		//! \param texture: Texture to delete from the engines cache.
		virtual void removeTexture(ITexture* texture) = 0;

		//! Removes all texture from the texture cache and deletes them, freeing lot of
		//! memory. Please note that after calling this, the pointer to all ITextures
		//! may not be longer valid, if they were not grabbed before by other parts of 
		//! the engine for storing them longer. So it would be a good idea to set all
		//! materials which are using textures to null first.
		virtual void removeAllTextures() = 0;

		//! Creates an 1bit alpha channel of the texture based of an color key.
		//! This makes the texture transparent at the regions where this color
		//! key can be found when using for example draw2DImage with useAlphachannel
		//! = true.
		//! \param texture: Texture of which its alpha channel is modified.
		//! \param color: Color key color. Every pixel with this color will get transparent 
		//! like described above. Please note that the colors of a texture may get
		//! converted when loading it, so the color values may not be exactly the same
		//! in the engine and for example in picture edit programs. To avoid this 
		//! problem, you could use the makeColorKeyTexture method, which takes the position
		//! of a pixel instead a color value.
		virtual void makeColorKeyTexture(video::ITexture* texture, video::SColor color) = 0;

		//! Creates an 1bit alpha channel of the texture based of an color key position.
		//! This makes the texture transparent at the regions where this color
		//! key can be found when using for example draw2DImage with useAlphachannel
		//! = true.
		//! \param texture: Texture of which its alpha channel is modified.
		//! \param colorKeyPixelPos: Position of a pixel with the color key color.
		//! Every pixel with this color will get transparent 
		//! like described above. 
		virtual void makeColorKeyTexture(video::ITexture* texture,
			core::position2d<s32> colorKeyPixelPos) = 0;

		//! Sets a new render target. This will only work, if the driver
		//! supports the EVDF_RENDER_TO_TARGET feature, which can be 
		//! queried with queryFeature().
		//! \param texture: New render target.
		virtual void setRenderTarget(video::ITexture* texture) = 0;

		//! Sets a new viewport. Every rendering operation is done into this
		//! new area.
		//! \param Rectangle defining the new area of rendering operations.
		virtual void setViewPort(const core::rect<s32>& area) = 0;

		//! Gets the area of the current viewport. 
		//! \return Returns rectangle of the current vieport.
		virtual const core::rect<s32>& getViewPort() const = 0;

		//! Draws an indexed triangle list.
		//! Note that there may be at maximum 65536 vertices, because the
		//! index list is a array of 16 bit values each with a maximum value 
		//! of 65536. If there are more than 65536 vertices in the list, 
		//! results of this operation are not defined.
		//! \param vertices: Pointer to array of vertices.
		//! \param vertexCount: Amount of vertices in the array.
		//! \param indexList: Pointer to array of indizes.
		//! \param triangleCount: amount of Triangles. Usually amount of indizes / 3.
		virtual void drawIndexedTriangleList(const S3DVertex* vertices,
			s32 vertexCount, const u16* indexList, s32 triangleCount) = 0;

		//! Draws an indexed triangle list.
		//! Note that there may be at maximum 65536 vertices, because the
		//! index list is a array of 16 bit values each with a maximum value 
		//! of 65536. If there are more than 65536 vertices in the list, 
		//! results of this operation are not defined.
		//! \param vertices: Pointer to array of vertices.
		//! \param vertexCount: Amount of vertices in the array.
		//! \param indexList: Pointer to array of indizes.
		//! \param triangleCount: amount of Triangles. Usually amount of indizes / 3.
		virtual void drawIndexedTriangleList(const S3DVertex2TCoords* vertices,
			s32 vertexCount, const u16* indexList, s32 triangleCount) = 0;

		//! Draws an indexed triangle fan.
		//! Note that there may be at maximum 65536 vertices, because the
		//! index list is a array of 16 bit values each with a maximum value 
		//! of 65536. If there are more than 65536 vertices in the list, 
		//! results of this operation are not defined.
		//! Please note that some of the implementation code for this method is based on 
		//! free code sent in by Mario Gruber, lots of thanks go to him!
		//! \param vertices: Pointer to array of vertices.
		//! \param vertexCount: Amount of vertices in the array.
		//! \param indexList: Pointer to array of indizes.
		//! \param triangleCount: amount of Triangles. Usually amount of indizes - 2.
		virtual void drawIndexedTriangleFan(const S3DVertex* vertices,
			s32 vertexCount, const u16* indexList, s32 triangleCount) = 0;

		//! Draws an indexed triangle fan.
		//! Note that there may be at maximum 65536 vertices, because the
		//! index list is a array of 16 bit values each with a maximum value 
		//! of 65536. If there are more than 65536 vertices in the list, 
		//! results of this operation are not defined.
		//! Please note that some of the implementation code for this method is based on 
		//! free code sent in by Mario Gruber, lots of thanks go to him!
		//! \param vertices: Pointer to array of vertices.
		//! \param vertexCount: Amount of vertices in the array.
		//! \param indexList: Pointer to array of indizes.
		//! \param triangleCount: amount of Triangles. Usually amount of indizes - 2.
		virtual void drawIndexedTriangleFan(const S3DVertex2TCoords* vertices,
			s32 vertexCount, const u16* indexList, s32 triangleCount) = 0;

		//! Draws a 3d line.
		//! This method usually simply calls drawIndexedTriangles with some 
		//! triangles. Note that the line is drawed using the current transformation
		//! matrix and material.
		//! This method was created for making culling debugging easier. It works with
		//! all drivers because it does simply a call to drawIndexedTriangleList and
		//! hence is not very fast but it might be useful for further development. 
		//! \param start: Start of the 3d line.
		//! \param end: End of the 3d line.
		//! \param color: Color of the line. 
		virtual void draw3DLine(const core::vector3df& start,
			const core::vector3df& end, SColor color = SColor(255,255,255,255)) = 0;

		//! Draws a 3d triangle.
		//! This method usually simply calls drawIndexedTriangles with some 
		//! triangles. Note that the line is drawed using the current transformation
		//! matrix and material.
		//! This method was created for making collision debugging easier. It works with
		//! all drivers because it does simply a call to drawIndexedTriangleList and
		//! hence is not very fast but it might be useful for further development. 
		//! \param triangle: The triangle to draw.
		//! \param color: Color of the line. 
		virtual void draw3DTriangle(const core::triangle3df& triangle,
			SColor color = SColor(255,255,255,255)) = 0;

		//! Draws a 3d axis aligned box.
		//! This method usually simply calls drawIndexedTriangles with some 
		//! triangles. Note that the line is drawed using the current transformation
		//! matrix and material. 
		//! This method was created for making culling debugging easier. It works with
		//! all drivers because it does simply a call to drawIndexedTriangleList and
		//! hence is not very fast but it might be useful for further development. 
		//! \param box: The axis aligned box to draw
		//! \param color: Color to use while drawing the box.
		virtual void draw3DBox(const core::aabbox3d<f32> box,
			SColor color = SColor(255,255,255,255)) = 0;

		//! Simply draws an 2d image, without any special effects
		//! \param texture: Pointer to texture to use.
		//! \param destPos: upper left 2d destination position where the image will be drawn.
		virtual void draw2DImage(video::ITexture* texture,
			const core::position2d<s32>& destPos) = 0;

		//! Draws an 2d image, using a color (if color is other than 
		//! Color(255,255,255,255)) and the alpha channel of the texture if wanted.
		//! \param texture: Texture to be drawn.
		//! \param destPos: Upper left 2d destination position where the image will be drawn.
		//! \param sourceRect: Source rectangle in the image.
		//! \param clipRect: Pointer to rectangle on the screen where the image is clipped to.
		//! This pointer can be NULL. Then the image is not clipped.
		//! \param color: Color with wich the image is colored. If the color equals 
		//! Color(255,255,255,255), the color is ignored. Note that the alpha component
		//! is used: If alpha is other than 255, the image will be transparent.
		//! \param useAlphaChannelOfTexture: If true, the alpha channel of the texture is 
		//! used to draw the image.
		virtual void draw2DImage(video::ITexture* texture, const core::position2d<s32>& destPos,
			const core::rect<s32>& sourceRect, const core::rect<s32>* clipRect = 0,
			SColor color=SColor(255,255,255,255), bool useAlphaChannelOfTexture=false) = 0;

		//! Draws a part of the texture into the rectangle. Suggested and first implemented by zola.
		//! \param texture: the texture to draw from 
		//! \param destRect: the rectangle to draw into 
		//! \param sourceRect: the rectangle denoting a part of the texture 
		//! \param clipRect: cipps the destination rectangle (may be 0) 
		//! \param colors: array of 4 colors denoting the color values of the corners of the destRect 
		//! \param useAlphaChannelOfTexture: true if alpha channel will be blended. 
		virtual void draw2DImage(video::ITexture* texture, const core::rect<s32>& destRect,
			const core::rect<s32>& sourceRect, const core::rect<s32>* clipRect = 0,
			video::SColor* colors=0, bool useAlphaChannelOfTexture=false) = 0;

		//! Draws an 2d rectangle.
		//! \param color: Color of the rectangle to draw. The alpha component will not
		//! be ignored and specifies how transparent the rectangle will be.
		//! \param pos: Position of the rectangle.
		//! \param clip: Pointer to rectangle against which the rectangle will be clipped.
		//! If the pointer is null, no clipping will be performed.
		virtual void draw2DRectangle(SColor color, const core::rect<s32>& pos,
			const core::rect<s32>* clip = 0) = 0;

		//! Draws a 2d line. 
		//! \param start: Screen coordinates of the start of the line in pixels.
		//! \param end: Screen coordinates of the start of the line in pixels.
		//! \param color: Color of the line to draw.
		virtual void draw2DLine(const core::position2d<s32>& start,
								const core::position2d<s32>& end, 
								SColor color=SColor(255,255,255,255)) = 0;

		//! Draws a shadow volume into the stencil buffer. To draw a stencil shadow, do
		//! this: Frist, draw all geometry. Then use this method, to draw the shadow
		//! volume. Then, use IVideoDriver::drawStencilShadow() to visualize the shadow.
		//! Please note that the code for the opengl version of the method is based on 
		//! free code sent in by Philipp Dortmann, lots of thanks go to him!
		//! \param triangles: Pointer to array of 3d vectors, specifing the shadow volume.
		//! \param count: Amount of triangles in the array.
		//! \param zfail: If set to true, zfail method is used, otherwise zpass.
		virtual void drawStencilShadowVolume(const core::vector3df* triangles, s32 count, bool zfail=true) = 0;

		//! Fills the stencil shadow with color. After the shadow volume has been drawn
		//! into the stencil buffer using IVideoDriver::drawStencilShadowVolume(), use this
		//! to draw the color of the shadow. 
		//! Please note that the code for the opengl version of the method is based on 
		//! free code sent in by Philipp Dortmann, lots of thanks go to him!
		//! \param clearStencilBuffer: Set this to false, if you want to draw every shadow 
		//! with the same color, and only want to call drawStencilShadow() once after all
		//! shadow volumes have been drawn. Set this to true, if you want to paint every
		//! shadow with its own color.
		//! \param leftUpEdge: Color of the shadow in the upper left corner of screen.
		//! \param rightUpEdge: Color of the shadow in the upper right corner of screen.
		//! \param leftDownEdge: Color of the shadow in the lower left corner of screen.
		//! \param rightDownEdge: Color of the shadow in the lower right corner of screen.
		virtual void drawStencilShadow(bool clearStencilBuffer=false, 
			video::SColor leftUpEdge = video::SColor(255,0,0,0), 
			video::SColor rightUpEdge = video::SColor(255,0,0,0),
			video::SColor leftDownEdge = video::SColor(255,0,0,0),
			video::SColor rightDownEdge = video::SColor(255,0,0,0)) = 0;

		//! Draws a mesh buffer
		//! \param mb: Buffer to draw;
		virtual void drawMeshBuffer(scene::IMeshBuffer* mb) = 0;

		//! Sets the fog mode. These are global values attached to each 3d object
		//! rendered, which has the fog flag enabled in its material.
		//! \param color: Color of the fog
		//! \param linearFog: Set this to true for linear fog, otherwise exponential fog is applied.
		//! \param start: Only used in linear fog mode (linearFog=true). Specifies where fog starts.
		//! \param end: Only used in linear fog mode (linearFog=true). Specifies where fog ends.
		//! \param density: Only used in expotential fog mode (linearFog=false). Must be a value between 0 and 1.
		//! \param pixelFog: Set this to false for vertex fog, and true if you want pixel fog.
		//! \param rangeFog: Set this to true to enable range-based vertex fog. The distance
		//! from the viewer is used to compute the fog, not the z-coordinate. This is
		//! better, but slower. This is only available with D3D and vertex fog.
		virtual void setFog(SColor color=SColor(0,255,255,255), bool linearFog=true, f32 start=50.0f, f32 end=100.0f, 
			f32 density=0.01f, bool pixelFog=false, bool rangeFog=false) = 0;

		//! Returns the size of the screen or render window.
		//! \return Size of screen or render window.
		virtual core::dimension2d<s32> getScreenSize() = 0;

		//! Returns current frames per second value.
		//! \return Returns amount of frames per second drawn.
		virtual s32 getFPS() = 0;

		//! Returns amount of primitives (mostly triangles) which were drawn in the last frame.
		//! Together with getFPS() very useful method for statistics.
		//! \return Amount of primitives drawn in the last frame.
		virtual u32 getPrimitiveCountDrawn() = 0;

		//! Deletes all dynamic lights which were previously added with addDynamicLight().
		virtual void deleteAllDynamicLights() = 0;

		//! Adds a dynamic light.
		//! \param light: Data specifing the dynamic light.
		virtual void addDynamicLight(const SLight& light) = 0;

		//! Sets the dynamic ambient light color. The default color is
		//! (0,0,0,0) which means it is dark.
		//! \param color: New color of the ambient light.
		virtual void setAmbientLight(const SColorf& color) = 0;

		//! Returns the maximal amount of dynamic lights the device can handle
		//! \return Maximal amount of dynamic lights.
		virtual s32 getMaximalDynamicLightAmount() = 0;

		//! Returns current amount of dynamic lights set
		//! \return Current amount of dynamic lights set
		virtual s32 getDynamicLightCount() = 0;

		//! Returns light data which was previously set with IVideDriver::addDynamicLight().
		//! \param idx: Zero based index of the light. Must be greater than 0 and smaller
		//! than IVideoDriver()::getDynamicLightCount.
		//! \return Light data.
		virtual const SLight& getDynamicLight(s32 idx) = 0;
        
		//! \return Returns the name of the video driver. Example: In case of the DirectX8
		//! driver, it would return "Direct3D8.1".
		virtual const wchar_t* getName() = 0;

		//! Adds an external image loader to the engine. This is useful if 
		//! the Irrlicht Engine should be able to load textures of currently 
		//! unsupported file formats (e.g .gif). The IImageLoader only needs
		//! to be implemented for loading this file format. A pointer to
		//! the implementation can be passed to the engine using this method.
		//! \param loader: Pointer to the external loader created.
		virtual void addExternalImageLoader(IImageLoader* loader) = 0;

		//! Returns the maximum amount of primitives (mostly vertices) which
		//! the device is able to render with one drawIndexedTriangleList
		//! call.
		virtual s32 getMaximalPrimitiveCount() = 0;

		//! Enables or disables a texture creation flag. This flag defines how
		//! textures should be created. By changing this value, you can influence
		//! the speed of rendering a lot. But please note that the video drivers
		//! take this value only as recommendation. It could happen that you
		//! enable the ETCM_ALWAYS_16_BIT mode, but the driver creates 32 bit
		//! textures.
		//! \param flag: Texture creation flag.
		//! \param enbabled: Specifies if the given flag should be enabled or disabled.
		virtual void setTextureCreationFlag(E_TEXTURE_CREATION_FLAG flag, bool enabled) = 0;

		//! Returns if a texture creation flag is enabled or disabled.
		//! You can change this value using setTextureCreationMode().
		//! \param flag: Texture creation flag.
		//! \return Returns the current texture creation mode.
		virtual bool getTextureCreationFlag(E_TEXTURE_CREATION_FLAG flag) = 0;

		//! Creates a software image from a file. No hardware texture will
		//! be created for this image. This method is useful for example if
		//! you want to read a heightmap for a terrain renderer.
		//! \param filename: Name of the file from which the image is created.
		//! \return Returns the created image.
		//! If you no longer need the image, you should call IImage::drop().
		//! See IUnknown::drop() for more information.
		virtual IImage* createImageFromFile(const char* filename) = 0;

		//! Creates a software image from a file. No hardware texture will
		//! be created for this image. This method is useful for example if
		//! you want to read a heightmap for a terrain renderer.
		//! \param file: File from which the image is created.
		//! \return Returns the created image.
		//! If you no longer need the image, you should call IImage::drop().
		//! See IUnknown::drop() for more information.
		virtual IImage* createImageFromFile(io::IReadFile* file) = 0;

		//! Creates a software image from a byte array. No hardware texture will
		//! be created for this image. This method is useful for example if
		//! you want to read a heightmap for a terrain renderer.
		//! \param format: Desired color format of the texture
		//! \param size: Desired the size of the image
		//! \param data: a byte array with pixelcolor information
		//! \return Returns the created image.
		//! If you no longer need the image, you should call IImage::drop().
		//! See IUnknown::drop() for more information.
		virtual IImage* createImageFromData(ECOLOR_FORMAT format, const core::dimension2d<s32>& size, void *data) = 0;

		//! Only used by the internal engine. Used to notify the driver that
		//! the window was resized.
		virtual void OnResize(const core::dimension2d<s32>& size) = 0;

		//! Adds a new material renderer to the video device. 
		//! Use this method to extend the VideoDriver with new MaterialTypes. To extend the
		//! engine using this method do the following:
		//! Derive a class from IMaterialRenderer and override the methods you need. For
		//! setting the right renderstates, you can try to get a pointer to the real rendering device
		//! using IVideoDriver::getExposedVideoData(). Add you class with 
		//! IVideoDriver::addMaterialRenderer() and if you want an object in the engine to be displayed
		//! with your new material, set the MaterialType member of the SMaterial struct to the 
		//! value returned by this method.
		//! \return Returns the number of the
		//! material type which can be set in SMaterial::MaterialType to use the renderer.
		//! -1 is returned if an error occured. (For example if you tried to add
		//! an material renderer to the software renderer or the null device, which do not accept
		//! material renderers.)
		virtual s32 addMaterialRenderer(IMaterialRenderer* renderer) = 0;

		//! Returns pointer to material renderer or null if not existing.
		//! \param idx: Id of the material renderer. Can be a value of the E_MATERIAL_TYPE or a 
		//! value which was returned by addMaterialRenderer().
		virtual IMaterialRenderer* getMaterialRenderer(s32 idx) = 0;

		//! Returns driver and operating system specific data about the IVideoDriver. This
		//! method should only be used if the engine should be extended without having
		//! to modify the source of the engine.
		virtual SExposedVideoData getExposedVideoData() = 0;

		//! Returns type of video driver
		virtual E_DRIVER_TYPE getDriverType() = 0;

		//! Returns pointer to the IGPUProgrammingServices interface. Returns 0 if the 
		//! videodriver does not support this. (For example the Software and the NULL device
		//! will always return 0)
		virtual IGPUProgrammingServices* getGPUProgrammingServices() = 0;
	};

} // end namespace video
} // end namespace irr


#endif

