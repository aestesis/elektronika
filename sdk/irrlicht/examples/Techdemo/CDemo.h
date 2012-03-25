// This is the current Techdemo of the Irrlicht Engine (c) 2003 by N.Gebhardt.
// This file is not documentated.

#ifndef __C_DEMO_H_INCLUDED__
#define __C_DEMO_H_INCLUDED__

#if !defined(WIN32) && !defined(_XBOX) && !defined(OS2) && !defined(MACOS)
#define LINUX
#endif

#ifndef LINUX
#define USE_AUDIERE
#endif

#include <irrlicht.h>

#ifdef USE_AUDIERE
#include <windows.h>
#include <audiere.h> // your compiler throws an error here? get audiere from 
                     // http://audiere.sourceforge.net/ or comment 
                     // the 'define USE_AUDIERE' above.
using namespace audiere;
#pragma comment (lib, "audiere.lib")
#endif

using namespace irr;
const int CAMERA_COUNT = 7;

class CDemo : public IEventReceiver
{
public:

	CDemo(bool fullscreen, bool music, bool shadows, bool additive, bool vsync, video::E_DRIVER_TYPE driver);

	~CDemo();

	void run();

	virtual bool OnEvent(SEvent event);

private:

	void createLoadingScreen();
	void loadSceneData();
	void switchToNextScene();
	void shoot();
	void createParticleImpacts();

	bool fullscreen;
	bool music;
	bool shadows;
	bool additive;
	bool vsync;
	video::E_DRIVER_TYPE driverType;
	IrrlichtDevice *device;

#ifdef USE_AUDIERE
	void startAudiere();
	AudioDevicePtr audiereDevice;
	OutputStreamPtr stream;
	OutputStreamPtr ballSound;
	OutputStreamPtr impactSound;
#endif

	struct SParticleImpact
	{
		u32 when;
		core::vector3df pos;
		core::vector3df outVector;
	};

	int currentScene;
	video::SColor backColor;

	gui::IGUIStaticText* statusText;
	gui::IGUIInOutFader* inOutFader;

	scene::IAnimatedMesh* quakeLevelMesh;
	scene::ISceneNode* quakeLevelNode;
	scene::ISceneNode* skyboxNode;
	scene::IAnimatedMeshSceneNode* model1;
	scene::IAnimatedMeshSceneNode* model2;
	scene::IParticleSystemSceneNode* campFire;

	scene::IMetaTriangleSelector* metaSelector;
	scene::ITriangleSelector* mapSelector;

	u32 sceneStartTime;
	u32 timeForThisScene;

	core::array<SParticleImpact> Impacts;
};

#endif
