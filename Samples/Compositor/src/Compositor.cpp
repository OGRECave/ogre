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

/**
    \file
        Compositor.cpp
    \brief
        Shows OGRE's Compositor feature
	\author
		W.J. :wumpus: van der Laan
			Ogre composition framework
		Manuel Bua
			Postfilter ideas and original out-of-core implementation
        Jeff (nfz) Doyle
            added gui framework to demo
*/

#include <Ogre.h>

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif


#include "Compositor.h"
#include "CompositorDemo_FrameListener.h"

/**********************************************************************
OS X Specific Resource Location Finding
**********************************************************************/
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE

Ogre::String bundlePath()
{
    char path[1024];
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    assert(mainBundle);
    
    CFURLRef mainBundleURL = CFBundleCopyBundleURL( mainBundle);
    assert(mainBundleURL);
    
    CFStringRef cfStringRef = CFURLCopyFileSystemPath( mainBundleURL, kCFURLPOSIXPathStyle);
    assert(cfStringRef);
    
    CFStringGetCString(cfStringRef, path, 1024, kCFStringEncodingASCII);
    
    CFRelease(mainBundleURL);
    CFRelease(cfStringRef);
    
    return Ogre::String(path);
}

#endif

/*************************************************************************
	                    CompositorDemo Methods
*************************************************************************/
    CompositorDemo::~CompositorDemo()
    {
        delete mGUISystem;
        delete mGUIRenderer;
        delete mFrameListener;

        delete mRoot;
#ifdef OGRE_STATIC_LIB
		mStaticPluginLoader.unload();
#endif

    }

//--------------------------------------------------------------------------
    void CompositorDemo::go(void)
    {
        if (!setup())
            return;

        mRoot->startRendering();
    }

//--------------------------------------------------------------------------
    bool CompositorDemo::setup(void)
    {
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
        bool carryOn = configure();
        if (!carryOn) return false;

        chooseSceneManager();
        createCamera();
        createViewports();

        // Set default mipmap level (NB some APIs ignore this)
        Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
        loadResources();

        // Create the scene
        createScene();

        createFrameListener();

        // load some GUI stuff for demo.
        //loadAllMaterialControlFiles(mMaterialControlsContainer);

        return true;

    }

//--------------------------------------------------------------------------
    bool CompositorDemo::configure(void)
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
    void CompositorDemo::chooseSceneManager(void)
    {
        // Get the SceneManager, in this case a generic one
        mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC, "ExampleSMInstance");
    }

//--------------------------------------------------------------------------
    void CompositorDemo::createCamera(void)
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
void CompositorDemo::createViewports(void)
{
    // Create one viewport, entire window
    Ogre::Viewport* vp = mWindow->addViewport(mCamera);
    vp->setBackgroundColour(Ogre::ColourValue(0,0,0));

    // Alter the camera aspect ratio to match the viewport
    mCamera->setAspectRatio(
        Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));
}

//--------------------------------------------------------------------------
    void CompositorDemo::setupResources(void)
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
                Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                    Ogre::String(bundlePath() + "/" + archName), typeName, secName);
#else
                Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                    archName, typeName, secName);
#endif

            }
        }

        Ogre::LogManager::getSingleton().logMessage( "Resource directories setup" );

    }

//-----------------------------------------------------------------------------------
	void CompositorDemo::loadResources(void)
	{
		// Initialise, parse all scripts etc
        Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

	}

//-----------------------------------------------------------------------------------
    void CompositorDemo::createScene(void)
    {
		mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_MODULATIVE);
		mSceneMgr->setShadowFarDistance(1000);
        // setup GUI system
        mGUIRenderer = new CEGUI::OgreCEGUIRenderer(mWindow, Ogre::RENDER_QUEUE_OVERLAY, false, 3000, mSceneMgr);
        // load scheme and set up defaults
        mGUISystem = new CEGUI::System(mGUIRenderer, (CEGUI::ResourceProvider *)0, (CEGUI::XMLParser*)0,
            (CEGUI::ScriptModule*)0, (CEGUI::utf8*)"CompositorDemoCegui.config");
        CEGUI::System::getSingleton().setDefaultMouseCursor("TaharezLook", "MouseArrow");

		Ogre::MovableObject::setDefaultVisibilityFlags(0x00000001);

		// Set ambient light
		mSceneMgr->setAmbientLight(Ogre::ColourValue(0.3, 0.3, 0.2));

		Ogre::Light* l = mSceneMgr->createLight("Light2");
		Ogre::Vector3 dir(-1,-1,0);
		dir.normalise();
		l->setType(Ogre::Light::LT_DIRECTIONAL);
		l->setDirection(dir);
		l->setDiffuseColour(1, 1, 0.8);
		l->setSpecularColour(1, 1, 1);


		Ogre::Entity* pEnt;

		// House
		pEnt = mSceneMgr->createEntity( "1", "tudorhouse.mesh" );
		Ogre::SceneNode* n1 = mSceneMgr->getRootSceneNode()->createChildSceneNode(Ogre::Vector3(350, 450, -200));
		n1->attachObject( pEnt );

		pEnt = mSceneMgr->createEntity( "2", "tudorhouse.mesh" );
		Ogre::SceneNode* n2 = mSceneMgr->getRootSceneNode()->createChildSceneNode(Ogre::Vector3(-350, 450, -200));
		n2->attachObject( pEnt );

		pEnt = mSceneMgr->createEntity( "3", "knot.mesh" );
		mSpinny = mSceneMgr->getRootSceneNode()->createChildSceneNode(Ogre::Vector3(0, 0, 300));
		mSpinny->attachObject( pEnt );
		pEnt->setMaterialName("Examples/MorningCubeMap");

		mSceneMgr->setSkyBox(true, "Examples/MorningSkyBox");


		Ogre::Plane plane;
		plane.normal = Ogre::Vector3::UNIT_Y;
		plane.d = 100;
		Ogre::MeshManager::getSingleton().createPlane("Myplane",
			Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane,
			1500, 1500, 10, 10, true, 1, 5, 5, Ogre::Vector3::UNIT_Z);
		Ogre::Entity* pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
		pPlaneEnt->setMaterialName("Examples/Rockwall");
		pPlaneEnt->setCastShadows(false);
		mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);

		mCamera->setPosition(-400, 50, 900);
		mCamera->lookAt(0,80,0);
		
		createTextures();

        connectEventHandlers();
		/// Create a couple of hard coded postfilter effects as an example of how to do it
		/// but the preferred method is to use compositor scripts.
		createEffects();
    }
//-----------------------------------------------------------------------------------
    void CompositorDemo::createFrameListener(void)
    {
        mFrameListener = new CompositorDemo_FrameListener(this);
		mFrameListener->setSpinningNode(mSpinny);

    }
//--------------------------------------------------------------------------
    void CompositorDemo::connectEventHandlers(void)
    {
        CEGUI::WindowManager::getSingleton().getWindow("ExitDemoBtn")->
            subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&CompositorDemo::handleQuit, this));
    }

//-----------------------------------------------------------------------------------
	/// Create the hard coded postfilter effects
	void CompositorDemo::createEffects(void)
	{
	    // Bloom compositor is loaded from script but here is the hard coded equivalent
//		CompositorPtr comp = CompositorManager::getSingleton().create(
//				"Bloom", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
//			);
//		{
//			CompositionTechnique *t = comp->createTechnique();
//			{
//				CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("rt0");
//				def->width = 128;
//				def->height = 128;
//				def->format = PF_A8R8G8B8;
//			}
//			{
//				CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("rt1");
//				def->width = 128;
//				def->height = 128;
//				def->format = PF_A8R8G8B8;
//			}
//			{
//				CompositionTargetPass *tp = t->createTargetPass();
//				tp->setInputMode(CompositionTargetPass::IM_PREVIOUS);
//				tp->setOutputName("rt1");
//			}
//			{
//				CompositionTargetPass *tp = t->createTargetPass();
//				tp->setInputMode(CompositionTargetPass::IM_NONE);
//				tp->setOutputName("rt0");
//				CompositionPass *pass = tp->createPass();
//				pass->setType(CompositionPass::PT_RENDERQUAD);
//				pass->setMaterialName("Ogre/Compositor/Blur0");
//				pass->setInput(0, "rt1");
//			}
//			{
//				CompositionTargetPass *tp = t->createTargetPass();
//				tp->setInputMode(CompositionTargetPass::IM_NONE);
//				tp->setOutputName("rt1");
//				CompositionPass *pass = tp->createPass();
//				pass->setType(CompositionPass::PT_RENDERQUAD);
//				pass->setMaterialName("Ogre/Compositor/Blur1");
//				pass->setInput(0, "rt0");
//			}
//			{
//				CompositionTargetPass *tp = t->getOutputTargetPass();
//				tp->setInputMode(CompositionTargetPass::IM_PREVIOUS);
//				{ CompositionPass *pass = tp->createPass();
//				pass->setType(CompositionPass::PT_RENDERQUAD);
//				pass->setMaterialName("Ogre/Compositor/BloomBlend");
//				pass->setInput(0, "rt1");
//				}
//			}
//		}
	    // Glass compositor is loaded from script but here is the hard coded equivalent
		/// Glass effect
//		CompositorPtr comp2 = CompositorManager::getSingleton().create(
//				"Glass", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
//			);
//		{
//			CompositionTechnique *t = comp2->createTechnique();
//			{
//				CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("rt0");
//				def->width = 0;
//				def->height = 0;
//				def->format = PF_R8G8B8;
//			}
//			{
//				CompositionTargetPass *tp = t->createTargetPass();
//				tp->setInputMode(CompositionTargetPass::IM_PREVIOUS);
//				tp->setOutputName("rt0");
//			}
//			{
//				CompositionTargetPass *tp = t->getOutputTargetPass();
//				tp->setInputMode(CompositionTargetPass::IM_NONE);
//				{ CompositionPass *pass = tp->createPass();
//				pass->setType(CompositionPass::PT_RENDERQUAD);
//				pass->setMaterialName("Ogre/Compositor/GlassPass");
//				pass->setInput(0, "rt0");
//				}
//			}
//		}
		/// Motion blur effect
		Ogre::CompositorPtr comp3 = Ogre::CompositorManager::getSingleton().create(
				"Motion Blur", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
			);
		{
			Ogre::CompositionTechnique *t = comp3->createTechnique();
			{
				Ogre::CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("scene");
				def->width = 0;
				def->height = 0;
				def->formatList.push_back(Ogre::PF_R8G8B8);
			}
			{
				Ogre::CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("sum");
				def->width = 0;
				def->height = 0;
				def->formatList.push_back(Ogre::PF_R8G8B8);
			}
			{
				Ogre::CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("temp");
				def->width = 0;
				def->height = 0;
				def->formatList.push_back(Ogre::PF_R8G8B8);
			}
			/// Render scene
			{
				Ogre::CompositionTargetPass *tp = t->createTargetPass();
				tp->setInputMode(Ogre::CompositionTargetPass::IM_PREVIOUS);
				tp->setOutputName("scene");
			}
			/// Initialisation pass for sum texture
			{
				Ogre::CompositionTargetPass *tp = t->createTargetPass();
				tp->setInputMode(Ogre::CompositionTargetPass::IM_PREVIOUS);
				tp->setOutputName("sum");
				tp->setOnlyInitial(true);
			}
			/// Do the motion blur
			{
				Ogre::CompositionTargetPass *tp = t->createTargetPass();
				tp->setInputMode(Ogre::CompositionTargetPass::IM_NONE);
				tp->setOutputName("temp");
				{ Ogre::CompositionPass *pass = tp->createPass();
				pass->setType(Ogre::CompositionPass::PT_RENDERQUAD);
				pass->setMaterialName("Ogre/Compositor/Combine");
				pass->setInput(0, "scene");
				pass->setInput(1, "sum");
				}
			}
			/// Copy back sum texture
			{
				Ogre::CompositionTargetPass *tp = t->createTargetPass();
				tp->setInputMode(Ogre::CompositionTargetPass::IM_NONE);
				tp->setOutputName("sum");
				{ Ogre::CompositionPass *pass = tp->createPass();
				pass->setType(Ogre::CompositionPass::PT_RENDERQUAD);
				pass->setMaterialName("Ogre/Compositor/Copyback");
				pass->setInput(0, "temp");
				}
			}
			/// Display result
			{
				Ogre::CompositionTargetPass *tp = t->getOutputTargetPass();
				tp->setInputMode(Ogre::CompositionTargetPass::IM_NONE);
				{ Ogre::CompositionPass *pass = tp->createPass();
				pass->setType(Ogre::CompositionPass::PT_RENDERQUAD);
				pass->setMaterialName("Ogre/Compositor/MotionBlur");
				pass->setInput(0, "sum");
				}
			}
		}
		/// Heat vision effect
		Ogre::CompositorPtr comp4 = Ogre::CompositorManager::getSingleton().create(
				"Heat Vision", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
			);
		{
			Ogre::CompositionTechnique *t = comp4->createTechnique();
			{
				Ogre::CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("scene");
				def->width = 256;
				def->height = 256;
				def->formatList.push_back(Ogre::PF_R8G8B8);
			}
			{
				Ogre::CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("temp");
				def->width = 256;
				def->height = 256;
				def->formatList.push_back(Ogre::PF_R8G8B8);
			}
			/// Render scene
			{
				Ogre::CompositionTargetPass *tp = t->createTargetPass();
				tp->setInputMode(Ogre::CompositionTargetPass::IM_PREVIOUS);
				tp->setOutputName("scene");
			}
			/// Light to heat pass
			{
				Ogre::CompositionTargetPass *tp = t->createTargetPass();
				tp->setInputMode(Ogre::CompositionTargetPass::IM_NONE);
				tp->setOutputName("temp");
				{
					Ogre::CompositionPass *pass = tp->createPass();
					pass->setType(Ogre::CompositionPass::PT_RENDERQUAD);
					pass->setIdentifier(0xDEADBABE); /// Identify pass for use in listener
					pass->setMaterialName("Fury/HeatVision/LightToHeat");
					pass->setInput(0, "scene");
				}
			}
			/// Display result
			{
				Ogre::CompositionTargetPass *tp = t->getOutputTargetPass();
				tp->setInputMode(Ogre::CompositionTargetPass::IM_NONE);
				{
					Ogre::CompositionPass *pass = tp->createPass();
					pass->setType(Ogre::CompositionPass::PT_RENDERQUAD);
					pass->setMaterialName("Fury/HeatVision/Blur");
					pass->setInput(0, "temp");
				}
			}
		}
	}
	
//--------------------------------------------------------------------------
	void CompositorDemo::createTextures(void)
	{
		using namespace Ogre;

		TexturePtr tex = TextureManager::getSingleton().createManual(
			"HalftoneVolume",
			"General",
			TEX_TYPE_3D,
			64,64,64,
			0,
			PF_A8
		);

		HardwarePixelBufferSharedPtr ptr = tex->getBuffer(0,0);
		ptr->lock(HardwareBuffer::HBL_DISCARD);
		const PixelBox &pb = ptr->getCurrentLock();
		Ogre::uint8 *data = static_cast<Ogre::uint8*>(pb.data);

		size_t height = pb.getHeight();
		size_t width = pb.getWidth();
		size_t depth = pb.getDepth();
		size_t rowPitch = pb.rowPitch;
		size_t slicePitch = pb.slicePitch;

		for (size_t z = 0; z < depth; ++z)
		{
			for (size_t y = 0; y < height; ++y)
			{
				for(size_t x = 0; x < width; ++x)
				{
					float fx = 32-(float)x+0.5f;
					float fy = 32-(float)y+0.5f;
					float fz = 32-((float)z)/3+0.5f;
					float distanceSquare = fx*fx+fy*fy+fz*fz;
					data[slicePitch*z + rowPitch*y + x] =  0x00;
					if (distanceSquare < 1024.0f)
						data[slicePitch*z + rowPitch*y + x] +=  0xFF;
				}
			}
		}
		ptr->unlock();

		Ogre::Viewport *vp = mRoot->getAutoCreatedWindow()->getViewport(0); 

		TexturePtr tex2 = TextureManager::getSingleton().createManual(
			"DitherTex",
			"General",
			TEX_TYPE_2D,
			vp->getActualWidth(),vp->getActualHeight(),1,
			0,
			PF_A8
		);

		HardwarePixelBufferSharedPtr ptr2 = tex2->getBuffer(0,0);
		ptr2->lock(HardwareBuffer::HBL_DISCARD);
		const PixelBox &pb2 = ptr2->getCurrentLock();
		Ogre::uint8 *data2 = static_cast<Ogre::uint8*>(pb2.data);
		
		size_t height2 = pb2.getHeight();
		size_t width2 = pb2.getWidth();
		size_t rowPitch2 = pb2.rowPitch;

		for (size_t y = 0; y < height2; ++y)
		{
			for(size_t x = 0; x < width2; ++x)
			{
				data2[rowPitch2*y + x] = Ogre::Math::RangeRandom(64.0,192);
			}
		}
		
		ptr2->unlock();
	}
//--------------------------------------------------------------------------
    bool CompositorDemo::handleQuit(const CEGUI::EventArgs& e)
    {
        mRoot->queueEndRendering();
        return true;
    }


#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
	int main(int argc, char *argv[])
#endif
{
   // Create application object
    CompositorDemo app;

    try {
        app.go();
    } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        std::cerr << "An exception has occured: " << e.getFullDescription();
#endif
    }


    return 0;
}

#ifdef __cplusplus
}
#endif
