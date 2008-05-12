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

/*
-----------------------------------------------------------------------------
Filename:    ExampleApplication.h
Description: Base class for all the OGRE ReferenceApplication examples
-----------------------------------------------------------------------------
*/

#ifndef __ExampleRefAppApplication_H__
#define __ExampleRefAppApplication_H__

#include "OgreReferenceAppLayer.h"
#include "OgreConfigFile.h"
#include "ExampleRefAppFrameListener.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#    include <SDL.h>
#endif

using namespace Ogre;
using namespace OgreRefApp;

/** Base class which manages the standard startup of an Ogre application
    based on the ReferenceApplication layer.
    Designed to be subclassed for specific examples if required.
*/
class ExampleRefAppApplication
{
public:
    /// Standard constructor
    ExampleRefAppApplication()
    {
        mFrameListener = 0;
        mRoot = 0;
        mWorld = 0;
    }
    /// Standard destructor
    virtual ~ExampleRefAppApplication()
    {
        if (mFrameListener)
            delete mFrameListener;
        if (mWorld)
            delete mWorld;
        if (mRoot)
            delete mRoot;
    }

    /// Start the example
    virtual void go(void)
    {
        if (!setup())
            return;

        mRoot->startRendering();
    }

protected:
    Root *mRoot;
    CollideCamera* mCamera;
    SceneManager* mSceneMgr;
    FrameListener* mFrameListener;
    RenderWindow* mWindow;
    World* mWorld;

    // These internal methods package up the stages in the startup process
    /** Sets up the application - returns false if the user chooses to abandon configuration. */
    virtual bool setup(void)
    {
        mRoot = new Root();

        setupResources();

        bool carryOn = configure();
        if (!carryOn) return false;

        chooseSceneManager();
        createWorld();
        createCamera();
        createViewports();

        // Set default mipmap level (NB some APIs ignore this)
        TextureManager::getSingleton().setDefaultNumMipmaps(5);

		// Create any resource listeners (for loading screens)
		createResourceListener();
		// Load resources
		loadResources();

        // Create the scene
        createScene();

        createFrameListener();

        return true;

    }
    /** Configures the application - returns false if the user chooses to abandon configuration. */
    virtual bool configure(void)
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

    virtual void chooseSceneManager(void)
    {
        // Get the SceneManager, in this case a generic one
        mSceneMgr = mRoot->createSceneManager(ST_GENERIC, "RefAppSMInstance");
    }
    virtual void createCamera(void)
    {
        // Create the camera
        mCamera = mWorld->createCamera("PlayerCam");

        // Position it at 500 in Z direction
        mCamera->setPosition(Vector3(0,0,500));
        // Look back along -Z
        mCamera->lookAt(Vector3(0,0,-300));
        mCamera->setNearClipDistance(5);

    }
    virtual void createFrameListener(void)
    {
        mFrameListener= new ExampleRefAppFrameListener(mWindow, mCamera);
        mRoot->addFrameListener(mFrameListener);
    }

    virtual void createScene(void) = 0;    // pure virtual - this has to be overridden

    virtual void createViewports(void)
    {
        // Create one viewport, entire window
        Viewport* vp = mWindow->addViewport(mCamera->getRealCamera());
        vp->setBackgroundColour(ColourValue(0,0,0));
    }

    /// Method which will define the source of resources (other than current folder)
    virtual void setupResources(void)
    {
        // Load resource paths from config file
        ConfigFile cf;
        cf.load("resources.cfg");

        // Go through all sections & settings in the file
        ConfigFile::SectionIterator seci = cf.getSectionIterator();

        String secName, typeName, archName;
        while (seci.hasMoreElements())
        {
            secName = seci.peekNextKey();
            ConfigFile::SettingsMultiMap *settings = seci.getNext();
            ConfigFile::SettingsMultiMap::iterator i;
            for (i = settings->begin(); i != settings->end(); ++i)
            {
                typeName = i->first;
                archName = i->second;
                ResourceGroupManager::getSingleton().addResourceLocation(
                    archName, typeName, secName);
            }
        }
    }
    virtual void createWorld(void)
    {
        mWorld = new World(mSceneMgr);
    }
	/// Optional override method where you can create resource listeners (e.g. for loading screens)
	virtual void createResourceListener(void)
	{

	}

	/// Optional override method where you can perform resource group loading
	/// Must at least do ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
	virtual void loadResources(void)
	{
		// Initialise, parse scripts etc
		ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

	}



};


#endif
