/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
-----------------------------------------------------------------------------
*/
/*
-----------------------------------------------------------------------------
Filename:    ExampleApplication.h
Description: Base class for all the OGRE examples
-----------------------------------------------------------------------------
*/

#ifndef __ExampleApplication_H__
#define __ExampleApplication_H__

#include "Ogre.h"
#include "OgreConfigFile.h"
#include "ExampleFrameListener.h"
// Static plugins declaration section
// Note that every entry in here adds an extra header / library dependency
#ifdef OGRE_STATIC_LIB
#  define OGRE_STATIC_GL
#  if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#    define OGRE_STATIC_Direct3D9
     // dx10 will only work on vista, so be careful about statically linking
#    if OGRE_USE_D3D10
#      define OGRE_STATIC_Direct3D10
#    endif
#    if OGRE_USE_D3D11
#      define OGRE_STATIC_Direct3D11
#    endif
#  endif
#  define OGRE_STATIC_BSPSceneManager
#  define OGRE_STATIC_ParticleFX
#  define OGRE_STATIC_CgProgramManager
#  ifdef OGRE_USE_PCZ
#    define OGRE_STATIC_PCZSceneManager
#    define OGRE_STATIC_OctreeZone
#  else
#    define OGRE_STATIC_OctreeSceneManager
#  endif
#  if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
#     undef OGRE_STATIC_CgProgramManager
#     undef OGRE_STATIC_GL
#     define OGRE_STATIC_GLES 1
#     ifdef __OBJC__
#       import <UIKit/UIKit.h>
#     endif
#  endif
#  include "OgreStaticPluginLoader.h"
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
#   include "macUtils.h"
#endif

#ifdef USE_RTSHADER_SYSTEM

/** This class simply demonstrates basic usage of the CRTShader system.
It sub class the material manager listener class and when a target scheme callback
is invoked with the shader generator scheme it tries to create an equvialent shader
based technique based on the default technique of the given material.
*/
class ShaderGeneratorTechniqueResolverListener : public MaterialManager::Listener
{
public:

	ShaderGeneratorTechniqueResolverListener(RTShader::ShaderGenerator* pShaderGenerator)
	{
		mShaderGenerator = pShaderGenerator;
	}

	virtual Technique* handleSchemeNotFound(unsigned short schemeIndex, 
		const String& schemeName, Material* originalMaterial, unsigned short lodIndex, 
		const Renderable* rend)
	{		
		// Case this is the default shader generator scheme.
		if (schemeName == RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME)
		{
			MaterialRegisterIterator itFind = mRegisteredMaterials.find(originalMaterial);
			bool techniqueCreated = false;

			// This material was not registered before.
			if (itFind == mRegisteredMaterials.end())
			{
				techniqueCreated = mShaderGenerator->createShaderBasedTechnique(
					originalMaterial->getName(), 
					MaterialManager::DEFAULT_SCHEME_NAME, 
					schemeName);				
			}
			mRegisteredMaterials[originalMaterial] = techniqueCreated;
		}

		return NULL;
	}

protected:
	typedef std::map<Material*, bool>		MaterialRegisterMap;
	typedef MaterialRegisterMap::iterator	MaterialRegisterIterator;


protected:
	MaterialRegisterMap				mRegisteredMaterials;		// Registered material map.
	RTShader::ShaderGenerator*		mShaderGenerator;			// The shader generator instance.
};
#endif

using namespace Ogre;

/** Base class which manages the standard startup of an Ogre application.
    Designed to be subclassed for specific examples if required.
*/
class ExampleApplication
{
public:
    /// Standard constructor
    ExampleApplication()
    {
        mFrameListener = 0;
        mRoot = 0;
		// Provide a nice cross platform solution for locating the configuration files
		// On windows files are searched for in the current working directory, on OS X however
		// you must provide the full path, the helper function macBundlePath does this for us.
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
		mResourcePath = macBundlePath() + "/Contents/Resources/";
        mConfigPath = mResourcePath;
#elif OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
        mResourcePath = macBundlePath() + "/";
        mConfigPath = mResourcePath;
#else
		mResourcePath = "";
        mConfigPath = mResourcePath;
#endif

#ifdef USE_RTSHADER_SYSTEM
		mShaderGenerator	 = NULL;		
		mMaterialMgrListener = NULL;
#endif
    }
    /// Standard destructor
    virtual ~ExampleApplication()
    {
        if (mFrameListener)
            delete mFrameListener;
        if (mRoot)
            OGRE_DELETE mRoot;

#ifdef OGRE_STATIC_LIB
		mStaticPluginLoader.unload();
#endif
    }

    /// Start the example
    virtual void go(void)
    {
        if (!setup())
            return;

        mRoot->startRendering();

        // clean up
        destroyScene();	

#ifdef USE_RTSHADER_SYSTEM
		// Finalize shader generator.
		finalizeShaderGenerator();
#endif

    }

protected:
    Root *mRoot;
#ifdef OGRE_STATIC_LIB
	StaticPluginLoader mStaticPluginLoader;
#endif
    Camera* mCamera;
    SceneManager* mSceneMgr;
    ExampleFrameListener* mFrameListener;
    RenderWindow* mWindow;
	Ogre::String mResourcePath;
	Ogre::String mConfigPath;
#ifdef USE_RTSHADER_SYSTEM
	RTShader::ShaderGenerator*					mShaderGenerator;			// The Shader generator instance.
	ShaderGeneratorTechniqueResolverListener*	mMaterialMgrListener;		// Material manager listener.	
#endif

    // These internal methods package up the stages in the startup process
    /** Sets up the application - returns false if the user chooses to abandon configuration. */
    virtual bool setup(void)
    {

		String pluginsPath;
		// only use plugins.cfg if not static
#ifndef OGRE_STATIC_LIB
#if OGRE_DEBUG_MODE
		pluginsPath = mResourcePath + "plugins_d.cfg";
#else
		pluginsPath = mResourcePath + "plugins.cfg";
#endif
#endif
		
        mRoot = OGRE_NEW Root(pluginsPath, 
            mConfigPath + "ogre.cfg", mResourcePath + "Ogre.log");
#ifdef OGRE_STATIC_LIB
		mStaticPluginLoader.load();
#endif
        setupResources();

        bool carryOn = configure();
        if (!carryOn) 
			return false;

        chooseSceneManager();
        createCamera();
        createViewports();
#ifdef USE_RTSHADER_SYSTEM
		// Initialize shader generator.
		carryOn = initializeShaderGenerator(mSceneMgr);
		if (!carryOn) 
			return false;
#endif

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
#ifdef USE_RTSHADER_SYSTEM
	virtual bool initializeShaderGenerator(SceneManager* sceneMgr)
	{	
		if (RTShader::ShaderGenerator::initialize())
		{
			mShaderGenerator = RTShader::ShaderGenerator::getSingletonPtr();

			// Set the scene manager.
			mShaderGenerator->setSceneManager(sceneMgr);

			// Setup core libraries and shader cache path.
			ResourceGroupManager::LocationList resLocationsList = ResourceGroupManager::getSingleton().getResourceLocationList(ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
			ResourceGroupManager::LocationList::iterator it = resLocationsList.begin();
			ResourceGroupManager::LocationList::iterator itEnd = resLocationsList.end();
			String shaderCoreLibsPath;
			String shaderCachePath;

			// Default cache path is current directory;
			shaderCachePath = "./";

			// Try to find the location of the core shader lib functions and use it
			// as shader cache path as well - this will reduce the number of generated files
			// when running from different directories.
			for (; it != itEnd; ++it)
			{

				if ((*it)->archive->getName().find("RTShaderLib") != String::npos)
				{
					shaderCoreLibsPath = (*it)->archive->getName() + "/";
					shaderCachePath    = shaderCoreLibsPath;
					break;
				}
			}

			// Core shader libs not found -> shader generating will fail.
			if (shaderCoreLibsPath.empty())			
				return false;			

			// Add resource location for the core shader libs. 
			ResourceGroupManager::getSingleton().addResourceLocation(shaderCoreLibsPath , "FileSystem");
				
			// Set shader cache path.
			mShaderGenerator->setShaderCachePath(shaderCachePath);		
									
			// Create and register the material manager listener.
			mMaterialMgrListener = new ShaderGeneratorTechniqueResolverListener(mShaderGenerator);				
			MaterialManager::getSingleton().addListener(mMaterialMgrListener);
		}

		return true;
	}

	virtual void finalizeShaderGenerator()
	{
		// Unregister the material manager listener.
		if (mMaterialMgrListener != NULL)
		{			
			MaterialManager::getSingleton().removeListener(mMaterialMgrListener);
			delete mMaterialMgrListener;
			mMaterialMgrListener = NULL;
		}

		// Finalize CRTShader system.
		if (mShaderGenerator != NULL)
		{
			RTShader::ShaderGenerator::finalize();
			mShaderGenerator = NULL;
		}
	}
#endif
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
        // Create the SceneManager, in this case a generic one
        mSceneMgr = mRoot->createSceneManager(ST_GENERIC, "ExampleSMInstance");
    }
    virtual void createCamera(void)
    {
        // Create the camera
        mCamera = mSceneMgr->createCamera("PlayerCam");

        // Position it at 500 in Z direction
        mCamera->setPosition(Vector3(0,0,500));
        // Look back along -Z
        mCamera->lookAt(Vector3(0,0,-300));
        mCamera->setNearClipDistance(5);

    }
    virtual void createFrameListener(void)
    {
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
        mFrameListener= new ExampleFrameListener(mWindow, mCamera, true, true, true);
#else
        mFrameListener= new ExampleFrameListener(mWindow, mCamera);
#endif
        mFrameListener->showDebugOverlay(true);
        mRoot->addFrameListener(mFrameListener);
    }

    virtual void createScene(void) = 0;    // pure virtual - this has to be overridden

    virtual void destroyScene(void){}    // Optional to override this

    virtual void createViewports(void)
    {
        // Create one viewport, entire window
        Viewport* vp = mWindow->addViewport(mCamera);
        vp->setBackgroundColour(ColourValue(0,0,0));

        // Alter the camera aspect ratio to match the viewport
        mCamera->setAspectRatio(
            Real(vp->getActualWidth()) / Real(vp->getActualHeight()));
    }

    /// Method which will define the source of resources (other than current folder)
    virtual void setupResources(void)
    {
        // Load resource paths from config file
        ConfigFile cf;
#if OGRE_DEBUG_MODE
        cf.load(mResourcePath + "resources_d.cfg");
#else
		cf.load(mResourcePath + "resources.cfg");
#endif

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
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
                // OS X does not set the working directory relative to the app,
                // In order to make things portable on OS X we need to provide
                // the loading with it's own bundle path location
				if (!StringUtil::startsWith(archName, "/", false)) // only adjust relative dirs
					archName = String(macBundlePath() + "/" + archName);
#endif
                ResourceGroupManager::getSingleton().addResourceLocation(
                    archName, typeName, secName);

            }
        }
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
