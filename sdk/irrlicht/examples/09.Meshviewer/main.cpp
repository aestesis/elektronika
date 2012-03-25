/*
 This tutorial show how to create a more complex application with the engine. We construct
 a simple mesh viewer using the user interface API and the scenemanagement of Irrlicht.
 The tutorial show how to create and use Buttons, Windows, Toolbars, Menus, ComboBoxes,
 Tabcontrols, Editboxes, Images, MessageBoxes, SkyBoxes, and how to parse XML files
 with the integrated XML reader of the engine.

 We start like in most other tutorials: Include all nesessary header files, add a
 comment to let the engine be linked with the right .lib file in Visual Studio,
 and deklare some global variables. We also add two 'using namespece' statements, so
 we do not need to write the whole names of all classes. In this tutorial, we use a
 lot stuff from the gui namespace.
*/
#include <irrlicht.h>
#include <iostream>


using namespace irr;
using namespace gui;

#pragma comment(lib, "Irrlicht.lib")

IrrlichtDevice *Device = 0;
core::stringc StartUpModelFile;
core::stringw MessageText;
core::stringw Caption;
scene::IAnimatedMeshSceneNode* Model = 0;
scene::ISceneNode* SkyBox = 0;

/*
	The three following functions do several stuff used by the mesh viewer. 
	The first function showAboutText() simply displays a messagebox with a caption
	and a message text. The texts will be stored in the MessageText and 
	Caption variables at startup.
*/
void showAboutText()
{
	// create modal message box with the text
	// loaded from the xml file.
	Device->getGUIEnvironment()->addMessageBox(
		Caption.c_str(), MessageText.c_str());
}


/*
	The second function loadModel() loads a model and displays it using an
	addAnimatedMeshSceneNode and the scene manager. Nothing difficult. It also
	displays a short message box, if the model could not be loaded.
*/
void loadModel(const c8* fn)
{
	// modify the name if it a .pk3 file

	c8 filename[1024];
	strcpy(filename, fn);
	c8* found = 0;

	if (found = strstr(filename, ".pk3"))
	{
		Device->getFileSystem()->addZipFileArchive(filename);
		strcpy(found +1, "bsp");
	}

	// load a model into the engine

	if (Model)
		Model->remove();

	Model = 0;

	scene::IAnimatedMesh* m = Device->getSceneManager()->getMesh(filename);

	if (!m) 
	{
		// model could not be loaded

		if (StartUpModelFile != filename)
			Device->getGUIEnvironment()->addMessageBox(
			Caption.c_str(), L"The model could not be loaded. " \
			L"Maybe it is not a supported file format.");
		return;
	}

	// set default material properties

	Model = Device->getSceneManager()->addAnimatedMeshSceneNode(m);
	if (!found)
		Model->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
	Model->setMaterialFlag(video::EMF_LIGHTING, false);
	Model->setDebugDataVisible(true);
}


/*
	Finally, the third function creates a toolbox window. In this simple mesh viewer,
	this toolbox only contains a tab control with three edit boxes for changing
	the scale of the displayed model.
*/
void createToolBox()
{
	// remove tool box if already there
	IGUIEnvironment* env = Device->getGUIEnvironment();
	IGUIElement* root = env->getRootGUIElement();
	IGUIElement* e = root->getElementFromId(5000, true);
	if (e) e->remove();

	// create the toolbox window
	IGUIWindow* wnd = env->addWindow(core::rect<s32>(450,25,640,480),
		false, L"Toolset", 0, 5000);

	// create tab control and tabs
	IGUITabControl* tab = env->addTabControl(
		core::rect<s32>(2,20,640-452,480-7), wnd, true, true);

	IGUITab* t1 = tab->addTab(L"Scale");
	IGUITab* t2 = tab->addTab(L"Empty Tab");

	// add some edit boxes and a button to tab one
	env->addEditBox(L"1.0", core::rect<s32>(40,50,130,70), true, t1, 901);
	env->addEditBox(L"1.0", core::rect<s32>(40,80,130,100), true, t1, 902);
	env->addEditBox(L"1.0", core::rect<s32>(40,110,130,130), true, t1, 903);

	env->addButton(core::rect<s32>(10,150,100,190), t1, 1101, L"set");

	// bring irrlicht engine logo to front, because it
	// now may be below the newly created toolbox
	root->bringToFront(root->getElementFromId(666, true));
}


/*
	To get all the events sent by the GUI Elements, we need to create an event
	receiver. This one is really simple. If an event occurs, it checks the id
	of the caller and the event type, and starts an action based on these values.
	For example, if a menu item with id 100 was selected, if opens a file-open-dialog.
*/
class MyEventReceiver : public IEventReceiver
{
public:
	virtual bool OnEvent(SEvent event)
	{
		if (event.EventType == EET_GUI_EVENT)
		{
			s32 id = event.GUIEvent.Caller->getID();
			IGUIEnvironment* env = Device->getGUIEnvironment();

			switch(event.GUIEvent.EventType)
			{
			case EGET_MENU_ITEM_SELECTED:
				{
					// a menu item was clicked

					IGUIContextMenu* menu = (IGUIContextMenu*)event.GUIEvent.Caller;
					s32 id = menu->getItemCommandId(menu->getSelectedItem());
					
					switch(id)
					{
					case 100: // File -> Open Model
						env->addFileOpenDialog(L"Please select a model file to open");
						break;
					case 200: // File -> Quit
						Device->closeDevice();
						break;
					case 300: // View -> Skybox
						SkyBox->setVisible(!SkyBox->isVisible());
						break;
					case 400: // View -> Debug Information
						if (Model)
							Model->setDebugDataVisible(!Model->isDebugDataVisible());
						break;
					case 500: // Help->About
						showAboutText();
						break;
					case 610: // View -> Material -> Solid
						if (Model)
							Model->setMaterialType(video::EMT_SOLID);
						break;
					case 620: // View -> Material -> Transparent
						if (Model)
							Model->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
						break;
					case 630: // View -> Material -> Reflection
						if (Model)
							Model->setMaterialType(video::EMT_SPHERE_MAP);
						break;
					}
                    break;
				}

			case EGET_FILE_SELECTED:
				{
					// load the model file, selected in the file open dialog
					IGUIFileOpenDialog* dialog = 
						(IGUIFileOpenDialog*)event.GUIEvent.Caller;
					loadModel(core::stringc(dialog->getFilename()).c_str());
				}

			case EGET_BUTTON_CLICKED:

				switch(id)
				{
				case 1101:
					{
						// set scale
						gui::IGUIElement* root = env->getRootGUIElement();
						core::vector3df scale;
						core::stringc s;

						s = root->getElementFromId(901, true)->getText();
						scale.X = (f32)atof(s.c_str());
						s = root->getElementFromId(902, true)->getText();
						scale.Y = (f32)atof(s.c_str());
						s = root->getElementFromId(903, true)->getText();
						scale.Z = (f32)atof(s.c_str());

						if (Model)
							Model->setScale(scale);
					}
					break;
				case 1102:
					env->addFileOpenDialog(L"Please select a model file to open");
					break;
				case 1103:
					showAboutText();
					break;
				case 1104:
					createToolBox();
					break;
				}

				break;
			}
		}

		return false;
	}
};


/*
	Most of the hard work is done. We only need to create the Irrlicht Engine device
	and all the buttons, menus and toolbars. 
	We start up the engine as usual, using createDevice(). To make our application
	catch events, we set our eventreceiver as parameter. The #ifdef WIN32 preprocessor
	commands are not necesarry, but I included them to make the tutorial use DirectX on
	Windows and OpenGL on all other platforms like Linux. 
	As you can see, there is also a unusual call to IrrlichtDevice::setResizeAble(). 
	This makes the render window resizeable, which is quite useful for a mesh viewer.
*/

int main()
{
	// ask user for driver

	video::E_DRIVER_TYPE driverType;

	printf("Please select the driver you want for the mesh viewer:\n"\
		" (a) Direct3D 9.0c\n (b) Direct3D 8.1\n (c) OpenGL 1.2\n"\
		" (d) Software Renderer\n (e) NullDevice\n (otherKey) exit\n\n");

	char key;
	std::cin >> key;

	switch(key)
	{
		case 'a': driverType = video::EDT_DIRECTX9;	break;
		case 'b': driverType = video::EDT_DIRECTX8;	break;
		case 'c': driverType = video::EDT_OPENGL;   break;
		case 'd': driverType = video::EDT_SOFTWARE; break;
		case 'e': driverType = video::EDT_NULL;     break;
		default: return 1;
	}	

	// create device and exit if creation failed

	MyEventReceiver receiver;
	Device = createDevice(driverType, core::dimension2d<s32>(640, 480),
		16, false, false, false, &receiver);

	if (Device == 0)
		return 1; // could not create selected driver.

	Device->setResizeAble(true);

	Device->setWindowCaption(L"Irrlicht Engine - Loading...");

	video::IVideoDriver* driver = Device->getVideoDriver();
	IGUIEnvironment* env = Device->getGUIEnvironment();
	scene::ISceneManager* smgr = Device->getSceneManager();

	driver->setTextureCreationFlag(video::ETCF_ALWAYS_32_BIT, true);

	/*
		The next step is to read the configuration file. It is stored in the xml 
		format and looks a little bit like this:

		<?xml version="1.0"?>
		<config>
			<startUpModel file="some filename" />
			<messageText caption="Irrlicht Engine Mesh Viewer">
				Hello!
			</messageText>
		</config>

		We need the data stored in there to be written into the global variables
		StartUpModelFile, MessageText and Caption. This is now done using the
		Irrlicht Engine integrated XML parser:
	*/

	// read configuration from xml file

	io::IXMLReader* xml = Device->getFileSystem()->createXMLReader(
		"../../media/config.xml");

	while(xml && xml->read())
	{
		switch(xml->getNodeType())
		{
		case io::EXN_TEXT:
			// in this xml file, the only text which occurs is the messageText
			MessageText = xml->getNodeData();
			break;
		case io::EXN_ELEMENT:
			{
				if (core::stringw("startUpModel") == xml->getNodeName())
					StartUpModelFile = xml->getAttributeValue(L"file");
				else
				if (core::stringw("messageText") == xml->getNodeName())
					Caption = xml->getAttributeValue(L"caption");
			}
			break;
		}
	}

	if (xml)
		xml->drop(); // don't forget to delete the xml reader 

	/*
		That wasn't difficult. Now we'll set a nicer font and create the
		Menu. It is possible to create submenus for every menu item. The call
		menu->addItem(L"File", -1, true, true); for example adds a new menu
		Item with the name "File" and the id -1. The following parameter says
		that the menu item should be enabled, and the last one says, that
		there should be a submenu. The submenu can now be accessed with
		menu->getSubMenu(0), because the "File" entry is the menu item with
		index 0.
	*/

	// set a nicer font

	IGUISkin* skin = env->getSkin();
	IGUIFont* font = env->getFont("../../media/fonthaettenschweiler.bmp");
	if (font)
		skin->setFont(font);

	// create menu

	gui::IGUIContextMenu* menu = env->addMenu();
	menu->addItem(L"File", -1, true, true);
	menu->addItem(L"View", -1, true, true);
	menu->addItem(L"Help", -1, true, true);

	gui::IGUIContextMenu* submenu;
	submenu = menu->getSubMenu(0);
	submenu->addItem(L"Open Model File...", 100);
	submenu->addSeparator();
	submenu->addItem(L"Quit", 200);

	submenu = menu->getSubMenu(1);
	submenu->addItem(L"toggle sky box visibility", 300);
	submenu->addItem(L"toggle model debug information", 400);
	submenu->addItem(L"model material", -1, true, true );

	submenu = submenu->getSubMenu(2);
	submenu->addItem(L"Solid", 610);
	submenu->addItem(L"Transparent", 620);
	submenu->addItem(L"Reflection", 630);

	submenu = menu->getSubMenu(2);
	submenu->addItem(L"About", 500);

	/*
		Below the toolbar, we want a toolbar, onto which we can place 
		colored buttons and important looking stuff like a senseless
		combobox.
	*/

	// create toolbar

	gui::IGUIToolBar* bar = env->addToolBar();
	bar->addButton(1102, 0, driver->getTexture("../../media/open.bmp"));
	bar->addButton(1103, 0, driver->getTexture("../../media/help.bmp"));
	bar->addButton(1104, 0, driver->getTexture("../../media/tools.bmp"));

	// create a combobox with some senseless texts

	gui::IGUIComboBox* box = env->addComboBox(core::rect<s32>(100,5,200,25), bar);
	box->addItem(L"Bilinear");
	box->addItem(L"Trilinear");
	box->addItem(L"Anisotropic");
	box->addItem(L"Isotropic");
	box->addItem(L"Psychedelic");
	box->addItem(L"No filtering");

	/*
		To make the editor look a little bit better, we disable transparent
		gui elements, and add a Irrlicht Engine logo. In addition, a text,
		which will show the current frame per second value is created, and
		the window caption changed.
	*/

	// disable alpha

	for (s32 i=0; i<gui::EGDC_COUNT ; ++i)
	{
		video::SColor col = env->getSkin()->getColor((gui::EGUI_DEFAULT_COLOR)i);
		col.setAlpha(255);
		env->getSkin()->setColor((gui::EGUI_DEFAULT_COLOR)i, col);
	}

	// add a tabcontrol

	createToolBox();

	// create fps text 

	IGUIStaticText* fpstext = env->addStaticText(L"", core::rect<s32>(210,26,270,41), true);

	// set window caption

	Caption += " - [";
	Caption += driver->getName();
	Caption += "]";
	Device->setWindowCaption(Caption.c_str());

	/*
		That's nearly the whole application. We simply show the about 
		message box at start up, and load the first model. To make everything
		look better, a skybox is created and a user controled camera,
		to make the application a little bit more interactive. Finally,
		everything is drawed in a standard drawing loop.
	*/

	// show about message box and load default model
	showAboutText();
	loadModel(StartUpModelFile.c_str());

	// add skybox 

	SkyBox = smgr->addSkyBoxSceneNode(
		driver->getTexture("../../media/irrlicht2_up.jpg"),
		driver->getTexture("../../media/irrlicht2_dn.jpg"),
		driver->getTexture("../../media/irrlicht2_lf.jpg"),
		driver->getTexture("../../media/irrlicht2_rt.jpg"),
		driver->getTexture("../../media/irrlicht2_ft.jpg"),
		driver->getTexture("../../media/irrlicht2_bk.jpg"));

	// add a camera scene node 

	smgr->addCameraSceneNodeMaya();

	// load the irrlicht engine logo

	video::ITexture* irrLogo = 
		driver->getTexture("../../media/irrlichtlogoaligned.jpg");

	// draw everything

	while(Device->run() && driver)
		if (Device->isWindowActive())
		{
			driver->beginScene(true, true, video::SColor(150,50,50,50));

			smgr->drawAll();
			env->drawAll();

			// draw irrlicht engine logo
			driver->draw2DImage(irrLogo,
				core::position2d<s32>(10, driver->getScreenSize().Height - 50),
				core::rect<s32>(0,0,108-22,460-429));
		
			driver->endScene();

			core::stringw str = L"FPS: ";
			str += driver->getFPS();
			fpstext->setText(str.c_str());
		}

	Device->drop();
	return 0;
}
