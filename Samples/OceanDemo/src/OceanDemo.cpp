/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
LGPL like the rest of the engine.
-----------------------------------------------------------------------------
*/

#include "OceanDemo.h"
#include "Ogre.h"

#include <cstdlib>

inline Ogre::String operator +(const Ogre::String& l,const CEGUI::String& o)
{
	return l+o.c_str();
}
/*
inline CEGUI::String operator +(const CEGUI::String& l,const Ogre::String& o)
{
	return l+o.c_str();
}
*/

/**********************************************************************
OS X Specific Resource Location Finding
**********************************************************************/
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE

Ogre::String bundlePath()
{
    char path[1024];
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    assert( mainBundle );

    CFURLRef mainBundleURL = CFBundleCopyBundleURL( mainBundle);
    assert( mainBundleURL);

    CFStringRef cfStringRef = CFURLCopyFileSystemPath( mainBundleURL, kCFURLPOSIXPathStyle);
    assert( cfStringRef);

    CFStringGetCString( cfStringRef, path, 1024, kCFStringEncodingASCII);

    CFRelease( mainBundleURL);
    CFRelease( cfStringRef);

    return Ogre::String( path);
}

#endif

/**********************************************************************
  Static declarations
**********************************************************************/
// Lights
#define NUM_LIGHTS 1

// the light
Ogre::Light* mLights[NUM_LIGHTS];
// billboards for lights
Ogre::BillboardSet* mLightFlareSets[NUM_LIGHTS];
Ogre::Billboard* mLightFlares[NUM_LIGHTS];
// Positions for lights
Ogre::Vector3 mLightPositions[NUM_LIGHTS] =
{
	Ogre::Vector3(00, 400, 00)
};
// Base orientations of the lights
Ogre::Real mLightRotationAngles[NUM_LIGHTS] = { 35 };
Ogre::Vector3 mLightRotationAxes[NUM_LIGHTS] = {
    Ogre::Vector3::UNIT_X
};
// Rotation speed for lights, degrees per second
Ogre::Real mLightSpeeds[NUM_LIGHTS] = { 30};

// Colours for the lights
Ogre::ColourValue mDiffuseLightColours[NUM_LIGHTS] =
{
	Ogre::ColourValue(0.6, 0.6, 0.6)
};

Ogre::ColourValue mSpecularLightColours[NUM_LIGHTS] =
{
	Ogre::ColourValue(0.5, 0.5, 0.5)
};

// Which lights are enabled
bool mLightState[NUM_LIGHTS] =
{
	true
};

// the light nodes
Ogre::SceneNode* mLightNodes[NUM_LIGHTS];
// the light node pivots
Ogre::SceneNode* mLightPivots[NUM_LIGHTS];

#define UVECTOR2(x, y) UVector2(cegui_reldim(x), cegui_reldim(y))
#define TEXTWIDGET_SIZE UVECTOR2(0.19, 0.06)
#define NUMBERWIDGET_SIZE UVECTOR2(0.065, 0.06)
#define SCROLLWIDGET_SIZE UVECTOR2(0.21, 0.02)

#define TEXTWIDGET_XPOS 0.01
#define NUMBERWIDGET_XPOS 0.37
#define SCROLLWIDGET_XPOS 0.50

#define TEXTWIDGET_YADJUST (-0.05f)
#define WIDGET_YSTART 0.2f
#define WIDGET_YOFFSET 0.15f

/*************************************************************************
	sub-class for ListboxTextItem that auto-sets the selection brush
	image.
*************************************************************************/
class MyListItem : public CEGUI::ListboxTextItem
{
public:
    MyListItem(const CEGUI::String& text, CEGUI::uint id) : CEGUI::ListboxTextItem(text, id)
	{
		setSelectionBrushImage("TaharezLook", "MultiListSelectionBrush");
	}
};



/*********************************************************************
    Main Program Entry Point
***********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main(int argc, char *argv[])
#endif
{
	// Create application object
	OceanDemo app;

	try {
		app.go();

	} catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == PLATFORM_WIN32
        MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        std::cerr << "An exception has occured: " <<
            e.getFullDescription().c_str() << std::endl;
#endif
    }

    return 0;
}

#ifdef __cplusplus
}
#endif

/*************************************************************************
	                    OceanDemo Methods
*************************************************************************/
OceanDemo::~OceanDemo()
{
	delete mGUISystem;
	delete mGUIRenderer;
    delete mFrameListener;

    // get rid of the shared pointers before shutting down ogre or exceptions occure
    mActiveFragmentProgram.setNull();
    mActiveFragmentParameters.setNull();
    mActiveVertexProgram.setNull();
    mActiveVertexParameters.setNull();
    mActiveMaterial.setNull();

    delete mRoot;

#ifdef OGRE_STATIC_LIB
	mStaticPluginLoader.unload();
#endif

}

//--------------------------------------------------------------------------
void OceanDemo::go(void)
{
    if (!setup())
        return;

    mRoot->startRendering();
}

//--------------------------------------------------------------------------
bool OceanDemo::setup(void)
{
	bool setupCompleted = false;

	Ogre::String mResourcePath;
	Ogre::String pluginsPath;
	// only use plugins.cfg if not static
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
	mResourcePath = bundlePath() + "/Contents/Resources/";
#endif
#ifndef OGRE_STATIC_LIB
	pluginsPath = mResourcePath + "plugins.cfg";
#endif

	mRoot = new Ogre::Root(pluginsPath,
		mResourcePath + "ogre.cfg", mResourcePath + "Ogre.log");

#ifdef OGRE_STATIC_LIB
	mStaticPluginLoader.load();
#endif


    setupResources();

    if (configure())
    {
        chooseSceneManager();
        createCamera();
        createViewports();

        // Set default mipmap level (NB some APIs ignore this)
        Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
        loadResources();


        if (setupGUI())
        {
            // Create the scene
            createScene();

            createFrameListener();

            // load some GUI stuff for demo.
            loadAllMaterialControlFiles(mMaterialControlsContainer);
            initDemoEventWiring();
            initComboBoxes();
            setupCompleted = true;
        }
    }

	return setupCompleted;
}

//--------------------------------------------------------------------------
bool OceanDemo::configure(void)
{
    // Show the configuration dialog and initialise the system
    // You can skip this and use root.restoreConfig() to load configuration
    // settings if you were sure there are valid ones saved in ogre.cfg
    if(mRoot->showConfigDialog())
    {
        // If returned true, user clicked OK so initialise
        // Here we choose to let the system create a default rendering window by passing 'true'
        mWindow = mRoot->initialise(true);
        return true;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------------------------------
void OceanDemo::chooseSceneManager(void)
{
    // Get the SceneManager, in this case a generic one
    mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC, "ExampleSMInstance");
}

//--------------------------------------------------------------------------
void OceanDemo::createCamera(void)
{
    // Create the camera
    mCamera = mSceneMgr->createCamera("PlayerCam");

    // Position it at 500 in Z direction
    mCamera->setPosition(Ogre::Vector3(0,0,0));
    // Look back along -Z
    mCamera->lookAt(Ogre::Vector3(0,0,-300));
    mCamera->setNearClipDistance(1);

}

//--------------------------------------------------------------------------
void OceanDemo::createViewports(void)
{
    // Create one viewport, entire window
    Ogre::Viewport* vp = mWindow->addViewport(mCamera);
    vp->setBackgroundColour(Ogre::ColourValue(0,0,0));

    // Alter the camera aspect ratio to match the viewport
    mCamera->setAspectRatio(
        Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));
}

//--------------------------------------------------------------------------
void OceanDemo::setupResources(void)
{
    // Load resource paths from config file
    Ogre::ConfigFile cf;

    #if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
        Ogre::String mResourcePath;
        mResourcePath = bundlePath() + "/Contents/Resources/";
        cf.load(mResourcePath + "resources.cfg");
    #else
        cf.load("resources.cfg");
    #endif

    // Go through all sections & settings in the file
    Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

    Ogre::String secName, typeName, archName;
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
                // OS X does not set the working directory relative to the app,
                // In order to make things portable on OS X we need to provide
                // the loading with it's own bundle path location
			if (!Ogre::StringUtil::startsWith(archName, "/", false)) // only adjust relative dirs			
				archName = bundlePath() + "/" + archName;
#endif
                Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                    archName, typeName, secName);

        }
    }

	Ogre::LogManager::getSingleton().logMessage( "Resource directories setup" );

}

//-----------------------------------------------------------------------------------
	void OceanDemo::loadResources(void)
	{
		// Initialise, parse scripts etc
        Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

	}

//--------------------------------------------------------------------------
bool OceanDemo::setupGUI(void)
{
    bool setupGUICompleted = false;
	// setup GUI system
	try
	{
        mGUIRenderer = new CEGUI::OgreCEGUIRenderer(mWindow, Ogre::RENDER_QUEUE_OVERLAY, false, 0, mSceneMgr);
        // load scheme and set up defaults

        mGUISystem = new CEGUI::System(mGUIRenderer, 0, 0, 0, (CEGUI::utf8*)"OceanDemoCegui.config");
        CEGUI::System::getSingleton().setDefaultMouseCursor("TaharezLook", "MouseArrow");
        setupGUICompleted = true;
	}
	catch(...)
	{

	}

	return setupGUICompleted;
}
//--------------------------------------------------------------------------
void OceanDemo::createScene(void)
{
    // Set ambient light
    mSceneMgr->setAmbientLight(Ogre::ColourValue(0.3, 0.3, 0.3));
	mSceneMgr->setSkyBox(true, "SkyBox", 1000);

    mMainNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();


    for (unsigned int i = 0; i < NUM_LIGHTS; ++i)
    {
        mLightPivots[i] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        mLightPivots[i]->rotate(mLightRotationAxes[i], Ogre::Angle(mLightRotationAngles[i]));
        // Create a light, use default parameters
        mLights[i] = mSceneMgr->createLight("Light" + Ogre::StringConverter::toString(i));
		mLights[i]->setPosition(mLightPositions[i]);
		mLights[i]->setDiffuseColour(mDiffuseLightColours[i]);
		mLights[i]->setSpecularColour(mSpecularLightColours[i]);
		mLights[i]->setVisible(mLightState[i]);
		//mLights[i]->setAttenuation(400, 0.1 , 1 , 0);
        // Attach light
        mLightPivots[i]->attachObject(mLights[i]);
		// Create billboard for light
        mLightFlareSets[i] = mSceneMgr->createBillboardSet("Flare" + Ogre::StringConverter::toString(i));
		mLightFlareSets[i]->setMaterialName("LightFlare");
		mLightPivots[i]->attachObject(mLightFlareSets[i]);
		mLightFlares[i] = mLightFlareSets[i]->createBillboard(mLightPositions[i]);
		mLightFlares[i]->setColour(mDiffuseLightColours[i]);
		mLightFlareSets[i]->setVisible(mLightState[i]);
    }

    // move the camera a bit right and make it look at the knot
	mCamera->moveRelative(Ogre::Vector3(50, 0, 100));
	mCamera->lookAt(0, 0, 0);

    // Define a plane mesh that will be used for the ocean surface
    Ogre::Plane oceanSurface;
    oceanSurface.normal = Ogre::Vector3::UNIT_Y;
    oceanSurface.d = 20;
    Ogre::MeshManager::getSingleton().createPlane("OceanSurface",
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        oceanSurface,
        1000, 1000, 50, 50, true, 1, 1, 1, Ogre::Vector3::UNIT_Z);

    mOceanSurfaceEnt = mSceneMgr->createEntity( "OceanSurface", "OceanSurface" );
    mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(mOceanSurfaceEnt);

}


//--------------------------------------------------------------------------
void OceanDemo::initComboBoxes(void)
{
	using namespace CEGUI;


	Combobox* cbobox = (Combobox*)WindowManager::getSingleton().getWindow("ShaderCombos");

    for(size_t idx = 0; idx < mMaterialControlsContainer.size(); ++idx  )
	{
        cbobox->addItem(new MyListItem( mMaterialControlsContainer[idx].getDisplayName().c_str(), static_cast<CEGUI::uint>(idx)));
	}

	// make first item visible
    if (cbobox->getItemCount() > 0)
	    cbobox->setItemSelectState((size_t)0, true);

    Editbox* eb;
    // set text in combobox
    if (!mMaterialControlsContainer.empty())
    {
        eb = (Editbox*)WindowManager::getSingleton().getWindow(cbobox->getName() + "__auto_editbox__");
        eb->setText(mMaterialControlsContainer[0].getDisplayName().c_str());
        handleShaderComboChanged(CEGUI::WindowEventArgs(cbobox));
    }

	cbobox = (Combobox*)WindowManager::getSingleton().getWindow("ModelCombos");
    Ogre::StringVectorPtr meshStringVector = Ogre::ResourceGroupManager::getSingleton().findResourceNames( Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "*.mesh" );
    Ogre::StringVector::iterator meshFileNameIterator = meshStringVector->begin();

    while ( meshFileNameIterator != meshStringVector->end() )
	{
        cbobox->addItem(new MyListItem( (*meshFileNameIterator).c_str(), 0 ));
        ++meshFileNameIterator;
	}

	// make first item visible
	//cbobox->setItemSelectState((CEGUI::uint)0, true);
 //   eb = (Editbox*)WindowManager::getSingleton().getWindow(cbobox->getName() + "__auto_editbox__");
 //   if (meshStringVector->begin() != meshStringVector->end())
 //   {
 //       eb->setText((*meshStringVector->begin()).c_str());
 //       handleModelComboChanged(CEGUI::WindowEventArgs(cbobox));
 //   }

}


//--------------------------------------------------------------------------
void OceanDemo::initDemoEventWiring(void)
{
	using namespace CEGUI;

	WindowManager::getSingleton().getWindow("ExitDemoBtn")->
		subscribeEvent(PushButton::EventClicked, CEGUI::Event::Subscriber(&OceanDemo::handleQuit, this));

	WindowManager::getSingleton().getWindow("ModelCombos")->
        subscribeEvent(Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber(&OceanDemo::handleModelComboChanged, this));

	WindowManager::getSingleton().getWindow("ShaderCombos")->
		subscribeEvent(Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber(&OceanDemo::handleShaderComboChanged, this));

	Window* wndw = WindowManager::getSingleton().getWindow("root");

	wndw->subscribeEvent(Window::EventMouseMove, CEGUI::Event::Subscriber(&OceanDemo_FrameListener::handleMouseMove, mFrameListener));

	wndw->subscribeEvent(Window::EventMouseButtonUp, CEGUI::Event::Subscriber(&OceanDemo_FrameListener::handleMouseButtonUp, mFrameListener));

	wndw->subscribeEvent(Window::EventMouseButtonDown, CEGUI::Event::Subscriber(&OceanDemo_FrameListener::handleMouseButtonDown, mFrameListener));

	wndw->subscribeEvent(Window::EventMouseWheel, CEGUI::Event::Subscriber(&OceanDemo_FrameListener::handleMouseWheelEvent, mFrameListener));
	wndw->subscribeEvent(Window::EventKeyDown, CEGUI::Event::Subscriber(&OceanDemo_FrameListener::handleKeyDownEvent, mFrameListener ));
	wndw->subscribeEvent(Window::EventKeyUp, CEGUI::Event::Subscriber(&OceanDemo_FrameListener::handleKeyUpEvent, mFrameListener ));

	wndw = WindowManager::getSingleton().getWindow("ModelSpinCB");
	wndw->subscribeEvent(Checkbox::EventCheckStateChanged, CEGUI::Event::Subscriber(&OceanDemo_FrameListener::handelModelSpinChange, mFrameListener ));

	wndw = WindowManager::getSingleton().getWindow("LightSpinCB");
	wndw->subscribeEvent(Checkbox::EventCheckStateChanged, CEGUI::Event::Subscriber(&OceanDemo_FrameListener::handelLightSpinChange, mFrameListener ));

	wndw = WindowManager::getSingleton().getWindow("CameraRBtn");
	wndw->subscribeEvent(RadioButton::EventSelectStateChanged, CEGUI::Event::Subscriber(&OceanDemo::handleMovementTypeChange, this ));

	wndw = WindowManager::getSingleton().getWindow("ModelRBtn");
	wndw->subscribeEvent(RadioButton::EventSelectStateChanged, CEGUI::Event::Subscriber(&OceanDemo::handleMovementTypeChange, this ));

	mVertScroll = (Scrollbar*)WindowManager::getSingleton().getWindow("VerticalScroll");
	mVertScroll->subscribeEvent(Scrollbar::EventScrollPositionChanged, CEGUI::Event::Subscriber(&OceanDemo::handleScrollControlsWindow, this ));

}


//--------------------------------------------------------------------------
void OceanDemo::doErrorBox(const char* text)
{
	using namespace CEGUI;

	WindowManager& winMgr = WindowManager::getSingleton();
	Window* root = winMgr.getWindow("root_wnd");

	FrameWindow* errbox;

	try
	{
		errbox = (FrameWindow*)winMgr.getWindow("ErrorBox");
	}
	catch(UnknownObjectException x)
	{
		// create frame window for box
		FrameWindow* fwnd = (FrameWindow*)winMgr.createWindow("TaharezLook/FrameWindow", "ErrorBox");
		root->addChildWindow(fwnd);
		fwnd->setPosition(UVECTOR2(0.25, 0.25f));
		fwnd->setMaxSize(UVECTOR2(1.0f, 1.0f));
		fwnd->setSize(UVECTOR2(0.5f, 0.5f));
		fwnd->setText("CEGUI Demo - Error!");
		fwnd->setDragMovingEnabled(false);
		fwnd->setSizingEnabled(false);
		fwnd->setAlwaysOnTop(true);
		fwnd->setCloseButtonEnabled(false);

		// create error text message
		Window* wnd = winMgr.createWindow("TaharezLook/StaticText", "ErrorBox/Message");
		fwnd->addChildWindow(wnd);
		wnd->setPosition(UVECTOR2(0.1f, 0.1f));
		wnd->setSize(UVECTOR2(0.8f, 0.5f));
		wnd->setProperty("VertFormatting", "VertCentred");
		wnd->setProperty("HorzFormatting", "HorzCentred");
		wnd->setProperty("BackgroundEnabled", "false");
		wnd->setProperty("FrameEnabled", "false");

		// create ok button
		wnd = (PushButton*)winMgr.createWindow("TaharezLook/Button", "ErrorBox/OkButton");
		fwnd->addChildWindow(wnd);
		wnd->setPosition(UVECTOR2(0.3f, 0.80f));
		wnd->setSize(UVECTOR2(0.4f, 0.1f));
		wnd->setText("Okay!");

		// subscribe event
		wnd->subscribeEvent(PushButton::EventClicked, CEGUI::Event::Subscriber(&OceanDemo::handleErrorBox, this ));

		errbox = fwnd;
	}

	errbox->getChild("ErrorBox/Message")->setText(text);
	errbox->show();
	errbox->activate();
}


//--------------------------------------------------------------------------
bool OceanDemo::handleQuit(const CEGUI::EventArgs& e)
{
	mRoot->queueEndRendering();
    return true;
}

//--------------------------------------------------------------------------
void OceanDemo::setShaderControlVal(const float val, const size_t index)
{
	char valTxtBuf[20];

	// set the text of the value
	sprintf(valTxtBuf, "%3.3f", val );
	mShaderControlContainer[index].NumberWidget->setText(valTxtBuf);
}

//--------------------------------------------------------------------------
bool OceanDemo::handleShaderControl(const CEGUI::EventArgs& e)
{
	using namespace CEGUI;
	using namespace Ogre;


	size_t index = ((Scrollbar*)((const WindowEventArgs&)e).window)->getID();
    const ShaderControl& ActiveShaderDef = mMaterialControlsContainer[mCurrentMaterial].getShaderControl(index);

	float val = ((Scrollbar*)((const WindowEventArgs&)e).window)->getScrollPosition();
	val = ActiveShaderDef.convertScrollPositionToParam(val);
	setShaderControlVal( val, index );

	if(mActivePass)
	{
		switch(ActiveShaderDef.ValType)
		{
			case GPU_VERTEX:
			case GPU_FRAGMENT:
				{
					GpuProgramParametersSharedPtr activeParameters =
						(ActiveShaderDef.ValType == GPU_VERTEX) ?
							mActiveVertexParameters : mActiveFragmentParameters;

					if(!activeParameters.isNull())
					{
						activeParameters->_writeRawConstant(
							ActiveShaderDef.PhysicalIndex + ActiveShaderDef.ElementIndex, val);
					}
				}
				break;

			case MAT_SPECULAR:
				{
					// get the specular values from the material pass
					ColourValue OldSpec(mActivePass->getSpecular());
					OldSpec[ActiveShaderDef.ElementIndex] = val;
					mActivePass->setSpecular( OldSpec );
				}

				break;

			case MAT_DIFFUSE:
				{
					// get the specular values from the material pass
					ColourValue OldSpec(mActivePass->getDiffuse());
					OldSpec[ActiveShaderDef.ElementIndex] = val;
					mActivePass->setDiffuse( OldSpec );
				}
				break;

			case MAT_AMBIENT:
				{
					// get the specular values from the material pass
					ColourValue OldSpec(mActivePass->getAmbient());
					OldSpec[ActiveShaderDef.ElementIndex] = val;
					mActivePass->setAmbient( OldSpec );
				}
				break;

			case MAT_SHININESS:
				// get the specular values from the material pass
				mActivePass->setShininess( val );
				break;
		}
	}

    return true;
}

//--------------------------------------------------------------------------
void OceanDemo::configureShaderControls(void)
{
	using namespace CEGUI;

    if (mMaterialControlsContainer.empty()) return;

    mActiveMaterial = Ogre::MaterialManager::getSingleton().getByName( mMaterialControlsContainer[mCurrentMaterial].getMaterialName() );
    if(!mActiveMaterial.isNull() && mActiveMaterial->getNumSupportedTechniques())
	{
        Ogre::Technique* currentTechnique = mActiveMaterial->getSupportedTechnique(0);
		if(currentTechnique)
		{
			mActivePass = currentTechnique->getPass(0);
			if(mActivePass)
			{
                if (mActivePass->hasFragmentProgram())
                {
				    mActiveFragmentProgram = mActivePass->getFragmentProgram();
    				mActiveFragmentParameters = mActivePass->getFragmentProgramParameters();
                }
                if (mActivePass->hasVertexProgram())
                {
				    mActiveVertexProgram = mActivePass->getVertexProgram();
				    mActiveVertexParameters = mActivePass->getVertexProgramParameters();
                }

                size_t activeControlCount = mMaterialControlsContainer[mCurrentMaterial].getShaderControlCount();
				mVertScroll->setDocumentSize( activeControlCount / 5.0f );
				mVertScroll->setScrollPosition(0.0f);

				// init the GUI controls
				// check the material entry for Params GUI list
                if(mMaterialControlsContainer[mCurrentMaterial].getShaderControlCount() > 0)
				{
					// if mShaderControlContainer size < Params list
					if( activeControlCount > mShaderControlContainer.size())
					{
						// resize container
						mShaderControlContainer.resize( activeControlCount );
					}
					Window* controlWindow = WindowManager::getSingleton().getWindow("ShaderControlsWin");
					// initialize each widget based on control data
					// iterate through the params GUI list
					for( size_t i=0; i < activeControlCount; ++i )
					{
						const ShaderControl& ActiveShaderDef = mMaterialControlsContainer[mCurrentMaterial].getShaderControl(i);

						// if TextWidget is NULL
						Window* activeTextWidget = mShaderControlContainer[i].TextWidget;
						if(activeTextWidget == NULL)
						{
							// create TextWidget

							mShaderControlContainer[i].TextWidget = activeTextWidget =
								WindowManager::getSingleton().createWindow("TaharezLook/StaticText",
                                ( ("UniformTxt" + Ogre::StringConverter::toString(i)).c_str() ));
							// add to Shader control window
							controlWindow->addChildWindow( activeTextWidget );
							// set position based on its index
							activeTextWidget->setPosition(UVECTOR2(TEXTWIDGET_XPOS, WIDGET_YSTART + TEXTWIDGET_YADJUST + WIDGET_YOFFSET * float(i)));
							activeTextWidget->setProperty("VertFormatting", "TopAligned");
							activeTextWidget->setProperty("HorzFormatting", "RightAligned");
							activeTextWidget->setProperty("FrameEnabled", "false");
							activeTextWidget->setInheritsAlpha(false);
							activeTextWidget->setProperty("BackgroundEnabled", "false");
							activeTextWidget->setMaxSize( TEXTWIDGET_SIZE );
							activeTextWidget->setMinSize( TEXTWIDGET_SIZE );
							activeTextWidget->setSize( TEXTWIDGET_SIZE );
						}

						// set TextWidget text to control name
                        activeTextWidget->setText( ActiveShaderDef.Name.c_str() );
						// make TextWidget visible
						activeTextWidget->show();

						// if NumberWidget is NULL
						Window* activeNumberWidget = mShaderControlContainer[i].NumberWidget;
						if(activeNumberWidget == NULL)
						{
							// create NumberWidget

							mShaderControlContainer[i].NumberWidget = activeNumberWidget =
								WindowManager::getSingleton().createWindow("TaharezLook/StaticText",
                                ( ("UniformNumTxt" + Ogre::StringConverter::toString(i)).c_str() ));
							// add to Shader control window
							controlWindow->addChildWindow( activeNumberWidget );
							// set position based on its index
							activeNumberWidget->setPosition(UVECTOR2(NUMBERWIDGET_XPOS, WIDGET_YSTART + TEXTWIDGET_YADJUST + WIDGET_YOFFSET * float(i)));
							activeNumberWidget->setProperty("HorzFormatting", "RightAligned");
							activeNumberWidget->setProperty("VertFormatting", "TopAligned");
							activeNumberWidget->setProperty("FrameEnabled", "false");
							activeNumberWidget->setInheritsAlpha(false);
							activeNumberWidget->setProperty("BackgroundEnabled", "false");
							activeNumberWidget->setMaxSize( NUMBERWIDGET_SIZE );
							activeNumberWidget->setMinSize( NUMBERWIDGET_SIZE );
							activeNumberWidget->setSize( NUMBERWIDGET_SIZE );
						}
						// make TextWidget visible
						activeNumberWidget->show();

						Scrollbar* activeScrollWidget = mShaderControlContainer[i].ScrollWidget;
						// if ScrollWidget is NULL
						if( activeScrollWidget == NULL )
						{
							// create ScrollWidget
							mShaderControlContainer[i].ScrollWidget = activeScrollWidget =
								(Scrollbar*)WindowManager::getSingleton().createWindow("TaharezLook/HorizontalScrollbar",
                                ( ("UniformSB" + Ogre::StringConverter::toString(i)).c_str() ));
							// add to Shader control window
							controlWindow->addChildWindow( activeScrollWidget );
							// set position based on its index
							activeScrollWidget->setPosition(UVECTOR2(SCROLLWIDGET_XPOS, WIDGET_YSTART + WIDGET_YOFFSET * float(i)));
							activeScrollWidget->setInheritsAlpha(false);
							activeScrollWidget->setMaxSize( SCROLLWIDGET_SIZE );
							activeScrollWidget->setMinSize( SCROLLWIDGET_SIZE );
							activeScrollWidget->setSize( SCROLLWIDGET_SIZE );
                            activeScrollWidget->setID( static_cast<CEGUI::uint>(i) );
							activeScrollWidget->setOverlapSize(0);
							// wire up ScrollWidget position changed event to handleShaderControl
							activeScrollWidget->subscribeEvent(Scrollbar::EventScrollPositionChanged, CEGUI::Event::Subscriber(&OceanDemo::handleShaderControl, this ));						}
						// set max value of ScrollWidget
						float maxval = ActiveShaderDef.getRange();
						activeScrollWidget->setDocumentSize(maxval);
						activeScrollWidget->setPageSize(0.000);
						activeScrollWidget->setStepSize(maxval/20);
						// get current value of param

						float uniformVal = 0.0;

						switch(ActiveShaderDef.ValType)
						{
							case GPU_VERTEX:
							case GPU_FRAGMENT:
								{
									Ogre::GpuProgramParametersSharedPtr activeParameters =
										(ActiveShaderDef.ValType == GPU_VERTEX) ?
											mActiveVertexParameters : mActiveFragmentParameters;
									if(!activeParameters.isNull())
									{
										// use param name to get index : use appropiate paramters ptr
										const Ogre::GpuConstantDefinition& def = 
											activeParameters->getConstantDefinition(ActiveShaderDef.ParamName);
										ActiveShaderDef.PhysicalIndex = def.physicalIndex;
										// use index to get RealConstantEntry
										const float* pFloat = activeParameters->getFloatPointer(ActiveShaderDef.PhysicalIndex);
										// set position of ScrollWidget as param value
										uniformVal = pFloat[ActiveShaderDef.ElementIndex];
										activeScrollWidget->setScrollPosition( ActiveShaderDef.convertParamToScrollPosition(uniformVal) );
									}
								}
								break;

							case MAT_SPECULAR:
								{
									// get the specular values from the material pass

									Ogre::ColourValue OldSpec(mActivePass->getSpecular());
									uniformVal = OldSpec[ActiveShaderDef.ElementIndex];
									activeScrollWidget->setScrollPosition( ActiveShaderDef.convertParamToScrollPosition(uniformVal) );
								}
								break;

							case MAT_DIFFUSE:
								{
									// get the diffuse values from the material pass

									Ogre::ColourValue OldSpec(mActivePass->getDiffuse());
									uniformVal = OldSpec[ActiveShaderDef.ElementIndex];
									activeScrollWidget->setScrollPosition( ActiveShaderDef.convertParamToScrollPosition(uniformVal) );
								}
								break;

							case MAT_AMBIENT:
								{
									// get the ambient values from the material pass

									Ogre::ColourValue OldSpec(mActivePass->getAmbient());
									uniformVal = OldSpec[ActiveShaderDef.ElementIndex];
									activeScrollWidget->setScrollPosition( ActiveShaderDef.convertParamToScrollPosition(uniformVal) );
								}
								break;

							case MAT_SHININESS:
								{
									// get the ambient values from the material pass

									uniformVal = mActivePass->getShininess();
									activeScrollWidget->setScrollPosition( ActiveShaderDef.convertParamToScrollPosition(uniformVal) );
								}

								break;

							case MAT_EMISSIVE:
								{
									// get the ambient values from the material pass

									//ColourValue OldSpec(mActivePass->gete());
									//activeScrollWidget->setScrollPosition( OldSpec.val[ActiveShaderDef->ElementIndex] );
								}
								break;
						}

						setShaderControlVal( uniformVal, i );
						activeScrollWidget->show();
					} // end of iterate
				}

				// turn off extra GUI widgets
				// iterate from 1 + active widgets to end
				for( size_t i = activeControlCount; i < mShaderControlContainer.size(); i++ )
				{
					// hide widget
					if( mShaderControlContainer[i].TextWidget != NULL )
					{
						mShaderControlContainer[i].TextWidget->hide();
					}

					if( mShaderControlContainer[i].NumberWidget != NULL )
					{
						mShaderControlContainer[i].NumberWidget->hide();
					}

					{
						mShaderControlContainer[i].ScrollWidget->hide();
					}

				}// end of iterate

			}
		}
	}

}

//--------------------------------------------------------------------------
bool OceanDemo::handleModelComboChanged(const CEGUI::EventArgs& e)
{
	using namespace CEGUI;

    // get the selected mesh filename from the combo box
	CEGUI::ListboxItem* item = ((Combobox*)((const WindowEventArgs&)e).window)->getSelectedItem();

    // convert from CEGUI::String to Ogre::String
    Ogre::String meshName(item->getText().c_str());

    // hide the current entity
    if (mCurrentEntity)
    {
        mCurrentEntity->setVisible(false);
        // disconnect the entity from the scenenode
        mMainNode->detachObject(mCurrentEntity->getName());
    }

    // find the entity selected in combo box
    // an exception is raised by getEntity if it doesn't exist
    // so trap the exception if entity not found and load it
    try
    {
        mCurrentEntity = mSceneMgr->getEntity( meshName );
    }
    catch (Ogre::Exception e)
    // if not found create it
    {
        Ogre::MeshPtr pMesh = Ogre::MeshManager::getSingleton().load(meshName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
            //Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY,
			//Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY,
			//true, true
            ); //so we can still read it
        // Build tangent vectors, all our meshes use only 1 texture coordset
        //pMesh->buildTangentVectors(0, 1);
        // Create entity
        mCurrentEntity = mSceneMgr->createEntity(meshName, meshName);
    }

    // make the entity visible and attach it to our main scenenode
    mCurrentEntity->setVisible(true);
    mMainNode->attachObject(mCurrentEntity);

    // set the the entity's material to the current material
    //mCurrentEntity->setMaterialName(mMaterialControlsContainer[mCurrentMaterial].getMaterialName());
	configureShaderControls();
    return true;
}

//--------------------------------------------------------------------------
bool OceanDemo::handleShaderComboChanged(const CEGUI::EventArgs& e)
{
	using namespace CEGUI;

	CEGUI::ListboxItem* item = ((Combobox*)((const WindowEventArgs&)e).window)->getSelectedItem();

	mCurrentMaterial = item->getID();
    if (mOceanSurfaceEnt)
	    mOceanSurfaceEnt->setMaterialName(mMaterialControlsContainer[mCurrentMaterial].getMaterialName());
	configureShaderControls();

    return true;
}


//--------------------------------------------------------------------------
bool OceanDemo::handleScrollControlsWindow(const CEGUI::EventArgs& e)
{
	using namespace CEGUI;

	size_t controlCount = mShaderControlContainer.size();
	float scrollval = ((Scrollbar*)((const WindowEventArgs&)e).window)->getScrollPosition();

	for (size_t i = 0; i < controlCount; i++)
	{
		float ypos = WIDGET_YSTART + WIDGET_YOFFSET * float(i) - scrollval;
		mShaderControlContainer[i].TextWidget->setPosition(UVECTOR2( TEXTWIDGET_XPOS, ypos + TEXTWIDGET_YADJUST));
		mShaderControlContainer[i].NumberWidget->setPosition(UVECTOR2( NUMBERWIDGET_XPOS, ypos + TEXTWIDGET_YADJUST));
		mShaderControlContainer[i].ScrollWidget->setPosition(UVECTOR2( SCROLLWIDGET_XPOS, ypos ));
	}

    return true;
}

//--------------------------------------------------------------------------
bool OceanDemo::handleMovementTypeChange(const CEGUI::EventArgs& e)
{
	using namespace CEGUI;
	CEGUI::uint id = ((RadioButton*)((const WindowEventArgs&)e).window)->getSelectedButtonInGroup()->getID();
	if (id == 0)
	{
		mMouseMovement = mv_CAMERA;
	}
	else
	{
		mMouseMovement = mv_MODEL;
	}

    return true;
}


//--------------------------------------------------------------------------
bool OceanDemo::handleErrorBox(const CEGUI::EventArgs& e)
{
	CEGUI::WindowManager::getSingleton().getWindow((CEGUI::utf8*)"ErrorBox")->hide();
    return true;
}

//--------------------------------------------------------------------------
void OceanDemo::createFrameListener(void)
{
	mFrameListener= new OceanDemo_FrameListener(this);
	mRoot->addFrameListener(mFrameListener);
}


/*************************************************************************
	OceanDemo_FrameListener methods that handle all input for this GLSL demo.
*************************************************************************/

OceanDemo_FrameListener::OceanDemo_FrameListener(OceanDemo* main)
    : mMain(main)
    , mStatsOn(true)
    , mWriteToFile(false)
    , mLastMousePositionSet(false)
    , mSpinModel(true)
    , mSpinLight(false)
	, mMouse(0)
	, mKeyboard(0)
    , mLMBDown(false)
    , mRMBDown(false)
    , mProcessMovement(false)
    , mUpdateMovement(false)
    , mMoveFwd(false)
    , mMoveBck(false)
    , mMoveLeft(false)
    , mMoveRight(false)

{
    mRotateSpeed = 0;
    mMoveSpeed = 100;

	mNumScreenShots = 0;
	mTimeUntilNextToggle = 0;
    mSceneDetailIndex = 0;
    mMoveScale = 0.0f;
	mSpeed = MINSPEED;
	mRotX = 0;
	mRotY = 0;
    mRotScale = 0.0f;
	mTranslateVector = Ogre::Vector3::ZERO;
    mAniso = 1;
    mFiltering = Ogre::TFO_BILINEAR;
	mAvgFrameTime = 0.1;

	// using buffered input
	OIS::ParamList pl;
	size_t windowHnd = 0;
	std::ostringstream windowHndStr;
	mMain->getRenderWindow()->getCustomAttribute("WINDOW", &windowHnd);
	windowHndStr << windowHnd;
	pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

	mInputManager = OIS::InputManager::createInputSystem( pl );
	//Create all devices (We only catch joystick exceptions here, as, most people have Key/Mouse)
	mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject( OIS::OISKeyboard, true ));
	mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject( OIS::OISMouse, true ));

	unsigned int width, height, depth;
	int left, top;
	mMain->getRenderWindow()->getMetrics(width, height, depth, left, top);

	//Set Mouse Region.. if window resizes, we should alter this to reflect as well
	const OIS::MouseState &ms = mMouse->getMouseState();
	ms.width = width;
	ms.height = height;

	mMouse->setEventCallback(this);
	mKeyboard->setEventCallback(this);

	mQuit = false;
	mSkipCount = 0;
	mUpdateFreq = 50;

	mGuiRenderer = CEGUI::System::getSingleton().getRenderer();

	mGuiAvg   = CEGUI::WindowManager::getSingleton().getWindow("OPAverageFPS");
	mGuiCurr  = CEGUI::WindowManager::getSingleton().getWindow("OPCurrentFPS");
	mGuiBest  = CEGUI::WindowManager::getSingleton().getWindow("OPBestFPS");
	mGuiWorst = CEGUI::WindowManager::getSingleton().getWindow("OPWorstFPS");
	mGuiTris  = CEGUI::WindowManager::getSingleton().getWindow("OPTriCount");
	mGuiDbg   = CEGUI::WindowManager::getSingleton().getWindow("OPDebugMsg");
	mRoot	  = CEGUI::WindowManager::getSingleton().getWindow("root");
}

//--------------------------------------------------------------------------
OceanDemo_FrameListener::~OceanDemo_FrameListener()
{
	if(mInputManager)
	{
		mInputManager->destroyInputObject(mMouse);
		mInputManager->destroyInputObject(mKeyboard);
		OIS::InputManager::destroyInputSystem(mInputManager);
		mInputManager = 0;
	}
}


//--------------------------------------------------------------------------
bool OceanDemo_FrameListener::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
	mMouse->capture();
	mKeyboard->capture();

	if (mQuit)
		return false;
	else
	{
		mSkipCount++;
		if (mSkipCount >= mUpdateFreq)
		{
			mSkipCount = 0;
			updateStats();
		}
		// update movement process
		if(mProcessMovement || mUpdateMovement)
		{
			mTranslateVector.x += mMoveLeft ? mAvgFrameTime * -MOVESPEED : 0;
			mTranslateVector.x += mMoveRight ? mAvgFrameTime * MOVESPEED : 0;
			mTranslateVector.z += mMoveFwd ? mAvgFrameTime * -MOVESPEED : 0;
			mTranslateVector.z += mMoveBck ? mAvgFrameTime * MOVESPEED : 0;
			switch(mMain->getMouseMovement())
			{
				case mv_CAMERA:
                    mMain->getCamera()->yaw(Ogre::Angle(mRotX));
					mMain->getCamera()->pitch(Ogre::Angle(mRotY));
					mMain->getCamera()->moveRelative(mTranslateVector);
					break;

				case mv_MODEL:
					mMain->getMainNode()->yaw(Ogre::Angle(mRotX));
					//mMain->getMainNode()->pitch(mRotY);
					mMain->getMainNode()->translate(mTranslateVector);
					break;
                default:
                    break;
			}

			mUpdateMovement = false;
			mRotX = 0;
			mRotY = 0;
			mTranslateVector = Ogre::Vector3::ZERO;
		}

		if(mSpinModel)
		{
			mRotateSpeed = mAvgFrameTime * 20;
			mMain->getMainNode()->yaw( Ogre::Angle(mRotateSpeed) );
		}
		if(mSpinLight)
		{
	        mLightPivots[0]->rotate(mLightRotationAxes[0], Ogre::Angle(mRotateSpeed * 2.0f));
		}
		if(mWriteToFile)
		{
			char tmp[20];
			sprintf(tmp, "frame_%d.png", ++mNumScreenShots);
			mMain->getRenderWindow()->writeContentsToFile(tmp);
		}
		return true;
	}
}

//--------------------------------------------------------------------------
bool OceanDemo_FrameListener::mouseMoved (const OIS::MouseEvent &e)
{
	CEGUI::System::getSingleton().injectMouseMove( e.state.X.rel, e.state.Y.rel );
	CEGUI::System::getSingleton().injectMouseWheelChange(e.state.Z.rel);
	return true;
}

//--------------------------------------------------------------------------
bool OceanDemo_FrameListener::keyPressed (const OIS::KeyEvent &e)
{
    // give 'quitting' priority
	if (e.key == OIS::KC_ESCAPE)
    {
        mQuit = true;
        return false;
    }

    if (e.key == OIS::KC_SYSRQ )
    {
		Ogre::StringStream ss;
        ss << "screenshot_" << ++mNumScreenShots << ".png";
        mMain->getRenderWindow()->writeContentsToFile(ss.str());
        //mTimeUntilNextToggle = 0.5;
		mDebugText = "Saved: " + ss.str();
    }

    // do event injection
    CEGUI::System& cegui = CEGUI::System::getSingleton();
    cegui.injectKeyDown(e.key);
	cegui.injectChar(e.text);
	return true;
}

//--------------------------------------------------------------------------
bool OceanDemo_FrameListener::keyReleased (const OIS::KeyEvent &e)
{
	CEGUI::System::getSingleton().injectKeyUp(e.key);
	return true;
}

//--------------------------------------------------------------------------
bool OceanDemo_FrameListener::mousePressed (const OIS::MouseEvent &e, OIS::MouseButtonID id)
{
	CEGUI::System::getSingleton().injectMouseButtonDown(convertOISButtonToCegui(id));
	return true;
}

//--------------------------------------------------------------------------
bool OceanDemo_FrameListener::mouseReleased (const OIS::MouseEvent &e, OIS::MouseButtonID id)
{
	CEGUI::System::getSingleton().injectMouseButtonUp(convertOISButtonToCegui(id));
	return true;
}

//--------------------------------------------------------------------------
CEGUI::MouseButton OceanDemo_FrameListener::convertOISButtonToCegui(int ois_button_id)
{
    switch (ois_button_id)
    {
	case 0: return CEGUI::LeftButton;
	case 1: return CEGUI::RightButton;
	case 2:	return CEGUI::MiddleButton;
	case 3: return CEGUI::X1Button;
	default: return CEGUI::LeftButton;
    }
}
//--------------------------------------------------------------------------
void OceanDemo_FrameListener::updateStats(void)
{
	static CEGUI::String currFps = "Current FPS: ";
	static CEGUI::String avgFps = "Average FPS: ";
	static CEGUI::String bestFps = "Best FPS: ";
	static CEGUI::String worstFps = "Worst FPS: ";
	static CEGUI::String tris = "Triangle Count: ";


	const Ogre::RenderTarget::FrameStats& stats = mMain->getRenderWindow()->getStatistics();

	mGuiAvg->setText(avgFps + Ogre::StringConverter::toString(stats.avgFPS));
	mGuiCurr->setText(currFps + Ogre::StringConverter::toString(stats.lastFPS));
	mGuiBest->setText(bestFps + Ogre::StringConverter::toString(stats.bestFPS)
		+ " " + Ogre::StringConverter::toString(stats.bestFrameTime)+" ms");
	mGuiWorst->setText(worstFps + Ogre::StringConverter::toString(stats.worstFPS)
		+ " " + Ogre::StringConverter::toString(stats.worstFrameTime)+" ms");

	mGuiTris->setText(tris + Ogre::StringConverter::toString(stats.triangleCount));
	mGuiDbg->setText(mDebugText.c_str());
	mAvgFrameTime = 1.0f/(stats.avgFPS + 1.0f);
	if (mAvgFrameTime > 0.1f) mAvgFrameTime = 0.1f;

}


//--------------------------------------------------------------------------
bool OceanDemo_FrameListener::handleMouseMove(const CEGUI::EventArgs& e)
{
	using namespace CEGUI;

	if( mLMBDown && !mRMBDown)
	{
		// rotate camera
		mRotX += -((const MouseEventArgs&)e).moveDelta.d_x * mAvgFrameTime * 10.0;
		mRotY += -((const MouseEventArgs&)e).moveDelta.d_y * mAvgFrameTime * 10.0;
		MouseCursor::getSingleton().setPosition( mLastMousePosition );
		mUpdateMovement = true;
	}
	else
	{
		if( mRMBDown && !mLMBDown)
		{
			// translate camera
			mTranslateVector.x += ((const MouseEventArgs&)e).moveDelta.d_x * mAvgFrameTime * MOVESPEED;
			mTranslateVector.y += -((const MouseEventArgs&)e).moveDelta.d_y * mAvgFrameTime * MOVESPEED;
			//mTranslateVector.z = 0;
			MouseCursor::getSingleton().setPosition( mLastMousePosition );
			mUpdateMovement = true;
		}
		else
		{
			if( mRMBDown && mLMBDown)
			{
				mTranslateVector.z += (((const MouseEventArgs&)e).moveDelta.d_x + ((const MouseEventArgs&)e).moveDelta.d_y) * mAvgFrameTime * MOVESPEED;
				MouseCursor::getSingleton().setPosition( mLastMousePosition );
				mUpdateMovement = true;
			}

		}
	}

    return true;
}

//--------------------------------------------------------------------------
bool OceanDemo_FrameListener::handleMouseButtonUp(const CEGUI::EventArgs& e)
{
	using namespace CEGUI;

	//Window* wndw = ((const WindowEventArgs&)e).window;
	if( ((const MouseEventArgs&)e).button == LeftButton )
	{
		mLMBDown = false;
	}

	if( ((const MouseEventArgs&)e).button == RightButton )
	{
		mRMBDown = false;
	}
	if( !mLMBDown && !mRMBDown )
	{
		MouseCursor::getSingleton().show();
		if(mLastMousePositionSet)
		{
			MouseCursor::getSingleton().setPosition( mLastMousePosition );
			mLastMousePositionSet = false;
		}
		mRoot->releaseInput();
	}

    return true;
}

//--------------------------------------------------------------------------
bool OceanDemo_FrameListener::handleMouseButtonDown(const CEGUI::EventArgs& e)
{
	using namespace CEGUI;

	//Window* wndw = ((const WindowEventArgs&)e).window;
	if( ((const MouseEventArgs&)e).button == LeftButton )
	{
		mLMBDown = true;
	}

	if( ((const MouseEventArgs&)e).button == RightButton )
	{
		mRMBDown = true;
	}

	if( mLMBDown || mRMBDown )
	{
		MouseCursor::getSingleton().hide();
		if (!mLastMousePositionSet)
		{
			mLastMousePosition = MouseCursor::getSingleton().getPosition();
			mLastMousePositionSet = true;
		}
		mRoot->captureInput();
	}

    return true;
}

//--------------------------------------------------------------------------
bool OceanDemo_FrameListener::handleMouseWheelEvent(const CEGUI::EventArgs& e)
{
	using namespace CEGUI;
	mTranslateVector.z += ((const MouseEventArgs&)e).wheelChange * -5.0;
	mUpdateMovement = true;

    return true;
}

//--------------------------------------------------------------------------
bool OceanDemo_FrameListener::handleKeyDownEvent(const CEGUI::EventArgs& e)
{
	using namespace CEGUI;

	CheckMovementKeys( ((const KeyEventArgs&)e).scancode , true);

    return true;
}

//--------------------------------------------------------------------------
bool OceanDemo_FrameListener::handleKeyUpEvent(const CEGUI::EventArgs& e)
{
	using namespace CEGUI;
	CheckMovementKeys( ((const KeyEventArgs&)e).scancode, false );

    return true;
}


//--------------------------------------------------------------------------
bool OceanDemo_FrameListener::handelModelSpinChange(const CEGUI::EventArgs& e)
{
	using namespace CEGUI;
	mSpinModel = ((Checkbox*)((const WindowEventArgs&)e).window)->isSelected();

    return true;
}

//--------------------------------------------------------------------------
bool OceanDemo_FrameListener::handelLightSpinChange(const CEGUI::EventArgs& e)
{
	using namespace CEGUI;
	mSpinLight = ((Checkbox*)((const WindowEventArgs&)e).window)->isSelected();

    return true;
}

//--------------------------------------------------------------------------
void OceanDemo_FrameListener::CheckMovementKeys( CEGUI::Key::Scan scancode, bool state )
{
	using namespace CEGUI;

	switch ( scancode )
	{
		case Key::A:
			mMoveLeft = state;
			break;

		case Key::D:
			mMoveRight = state;
			break;

		case Key::S:
			mMoveBck = state;
			break;

		case Key::W:
			mMoveFwd = state;
			break;

        default:
            break;

	}

	mProcessMovement = mMoveLeft || mMoveRight || mMoveFwd || mMoveBck;

}
