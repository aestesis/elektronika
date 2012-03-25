// This is the current Techdemo of the Irrlicht Engine (c) 2003 by N.Gebhardt.
// This file is not documentated.

#include "CMainMenu.h"

CMainMenu::CMainMenu()
: startButton(0), device(0), start(false), fullscreen(true), selected(2),
	music(true), shadows(false), additive(false), transparent(true), vsync(true)
{
}



bool CMainMenu::run(bool& outFullscreen, bool& outMusic, bool& outShadows,
					bool& outAdditive, bool &outVSync, video::E_DRIVER_TYPE& outDriver)
{
	device = createDevice(video::EDT_SOFTWARE,
		core::dimension2d<s32>(512, 384), 16, false, false, false, this);

	device->getFileSystem()->addZipFileArchive("irrlicht.dat");
	device->getFileSystem()->addZipFileArchive("../../media/irrlicht.dat");

	video::IVideoDriver* driver = device->getVideoDriver();
	scene::ISceneManager* smgr = device->getSceneManager();
	gui::IGUIEnvironment* guienv = device->getGUIEnvironment();

	wchar_t tmp[255];
	swprintf(tmp, 255, L"Irrlicht Engine Techdemo v%s", device->getVersion());
	device->setWindowCaption(tmp);

	// load font

	gui::IGUIFont* font = guienv->getFont("../../media/fonthaettenschweiler.bmp");
	if (font)
		guienv->getSkin()->setFont(font);

	// add images

	gui::IGUIImage* img = guienv->addImage(core::rect<int>(0,0,512,384));
	img->setImage(driver->getTexture("../../media/techdemoback.bmp"));

    const s32 leftX = 260;

	// add tab control
	gui::IGUITabControl* tabctrl = guienv->addTabControl(core::rect<int>(leftX,10,512-10,384-10),
		0, true, true);
	gui::IGUITab* optTab = tabctrl->addTab(L"Demo");
	gui::IGUITab* aboutTab = tabctrl->addTab(L"About");

	// add list box

	gui::IGUIListBox* box = guienv->addListBox(core::rect<int>(10,10,220,75), optTab, 1);
	box->addItem(L"OpenGL 1.2");
	box->addItem(L"Direct3D 8.1");
	box->addItem(L"Direct3D 9.0b");	
	box->addItem(L"Irrlicht Software Renderer 1.0");
	box->setSelected(selected);

	// add button

	startButton = guienv->addButton(core::rect<int>(30,295,200,324), optTab, 2, L"Start Demo");

	// add checkbox

	guienv->addCheckBox(fullscreen, core::rect<int>(20,85,130,110), 
		optTab, 3, L"Fullscreen");
	guienv->addCheckBox(music, core::rect<int>(20,110,130,135), 
		optTab, 4, L"Music & Sfx");
	guienv->addCheckBox(shadows, core::rect<int>(20,135,230,160), 
		optTab, 5, L"Realtime shadows");
	guienv->addCheckBox(additive, core::rect<int>(20,160,230,185),
		optTab, 6, L"Old HW compatible blending");
	guienv->addCheckBox(vsync, core::rect<int>(20,185,230,210),
		optTab, 7, L"Vertical synchronisation");

	// add text

	wchar_t* text = L"Welcome to the Irrlicht Engine. Please select "\
		L"the settings you prefer and press 'Start Demo'. "\
		L"Right click for changing menu style.";

	guienv->addStaticText(text, core::rect<int>(10, 220, 220, 280),
		true, true, optTab);

	// add about text

	wchar_t* text2 = L"This is the tech demo of the Irrlicht engine. To start, "\
		L"select a device which works best with your hardware and press 'start demo'. "\
		L"What you currently see is displayed using the Software Renderer, but this would be too slow "\
		L"for the demo. The Irrlicht Engine was written by me, Nikolaus Gebhardt. The models, "\
		L"maps and textures were placed at my disposal by B.Collins, M.Cook and J.Marton. The music was created by "\
		L"M.Rohde and is played back by Audiere.\n"\
		L"For more informations, please visit the homepage of the Irrlicht engine:\nhttp://www.irrlicht.sourceforge.net";

	guienv->addStaticText(text2, core::rect<int>(20, 40, 220, 300),
		true, true, aboutTab);
	
	// add md2 model

	scene::IAnimatedMesh* mesh = smgr->getMesh("../../media/faerie.md2");
	scene::IAnimatedMeshSceneNode* node = smgr->addAnimatedMeshSceneNode(mesh);
	if (node)
	{
		node->setMaterialTexture(0, driver->getTexture("../../media/faerie2.bmp"));
		node->setMaterialFlag(video::EMF_LIGHTING, false);
		node->setFrameLoop(0, 310);
	}

	smgr->addCameraSceneNode(0, core::vector3df(45,0,0), core::vector3df(0,0,20));

	// irrlicht logo
	video::ITexture* irrlichtLogo = driver->getTexture("../../media/irrlichtlogo.bmp");

	// set transparency
	setTransparency();

	// draw all

	while(device->run())
	{
		if (device->isWindowActive())
		{
			driver->beginScene(false, true, video::SColor(0,0,0,0));
			guienv->drawAll();
			smgr->drawAll();	

			if (irrlichtLogo)
				driver->draw2DImage(irrlichtLogo, core::position2d<int>(10,10));

			driver->endScene();
		}
	}

	device->drop();

	outFullscreen = fullscreen;
	outMusic = music;
	outShadows = shadows;
	outAdditive = additive;
	outVSync = vsync;

	switch(selected)
	{
	case 0:	outDriver = video::EDT_OPENGL; break;
	case 1:	outDriver = video::EDT_DIRECTX8; break;
	case 2:	outDriver = video::EDT_DIRECTX9; break;	
	case 3:	outDriver = video::EDT_SOFTWARE; break;
	}

	return start;
}



bool CMainMenu::OnEvent(SEvent event)
{
	if (event.EventType == irr::EET_MOUSE_INPUT_EVENT &&
		event.MouseInput.Event == EMIE_RMOUSE_LEFT_UP )
	{
		core::rect<s32> r(event.MouseInput.X, event.MouseInput.Y, 0, 0);
		gui::IGUIContextMenu* menu = device->getGUIEnvironment()->addContextMenu(r, 0, 45);
		menu->addItem(L"transparent menus", 666, transparent == false);
		menu->addItem(L"solid menus", 666, transparent == true);
		menu->addSeparator();
		menu->addItem(L"Cancel");
	}
	else
	if (event.EventType == EET_GUI_EVENT)
	{
		s32 id = event.GUIEvent.Caller->getID();
		switch(id)
		{
		case 45: // context menu
			if (event.GUIEvent.EventType == gui::EGET_MENU_ITEM_SELECTED)
			{
				s32 s = ((gui::IGUIContextMenu*)event.GUIEvent.Caller)->getSelectedItem();
				if (s == 0 || s == 1)
				{
					transparent = !transparent;
					setTransparency();
				}
			}
			break;
		case 1:
			if (event.GUIEvent.EventType == gui::EGET_LISTBOX_CHANGED)
			{
				selected = ((gui::IGUIListBox*)event.GUIEvent.Caller)->getSelected();
				startButton->setEnabled(selected != 3);
			}
			break;
		case 2:
			if (event.GUIEvent.EventType == gui::EGET_BUTTON_CLICKED )
			{
				device->closeDevice();
				start = true;
			}
		case 3:
			if (event.GUIEvent.EventType == gui::EGET_CHECKBOX_CHANGED )
				fullscreen = ((gui::IGUICheckBox*)event.GUIEvent.Caller)->isChecked();
			break;
		case 4:
			if (event.GUIEvent.EventType == gui::EGET_CHECKBOX_CHANGED )
				music = ((gui::IGUICheckBox*)event.GUIEvent.Caller)->isChecked();
			break;
		case 5:
			if (event.GUIEvent.EventType == gui::EGET_CHECKBOX_CHANGED )
				shadows = ((gui::IGUICheckBox*)event.GUIEvent.Caller)->isChecked();
			break;
		case 6:
			if (event.GUIEvent.EventType == gui::EGET_CHECKBOX_CHANGED )
				additive = ((gui::IGUICheckBox*)event.GUIEvent.Caller)->isChecked();
			break;
		case 7:
			if (event.GUIEvent.EventType == gui::EGET_CHECKBOX_CHANGED )
				vsync = ((gui::IGUICheckBox*)event.GUIEvent.Caller)->isChecked();
			break;			
		}
	}

	return false;
}


void CMainMenu::setTransparency()
{
	for (s32 i=0; i<gui::EGDC_COUNT ; ++i)
	{
		video::SColor col = device->getGUIEnvironment()->getSkin()->
			getColor((gui::EGUI_DEFAULT_COLOR)i);
		col.setAlpha(transparent ? 201 : 255);
		device->getGUIEnvironment()->getSkin()->setColor((gui::EGUI_DEFAULT_COLOR)i, col);
	}
}
