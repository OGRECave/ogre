/*
 * OgreApplicationContext.cpp
 *
 *  Created on: 18.05.2016
 *      Author: pavel
 */

#include "OgreApplicationContext.h"

#include "OgreRoot.h"
#include "OgreGpuProgramManager.h"
#include "OgreConfigFile.h"
#include "OgreRenderWindow.h"
#include "OgreViewport.h"
#include "OgreOverlaySystem.h"

#if OGRE_BITES_HAVE_SDL
#include <SDL_video.h>
#include <SDL_syswm.h>
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
#include "OgreArchiveManager.h"
#include "Android/OgreAPKFileSystemArchive.h"
#include "Android/OgreAPKZipArchive.h"
#endif

// Remove the comment below in order to make the RTSS use valid path for writing down the generated shaders.
// If cache path is not set - all shaders are generated to system memory.
//#define _RTSS_WRITE_SHADERS_TO_DISK

namespace OgreBites {

ApplicationContext::ApplicationContext(const Ogre::String& appName, bool grabInput)
#if (OGRE_THREAD_PROVIDER == 3) && (OGRE_NO_TBB_SCHEDULER == 1)
    : mTaskScheduler(tbb::task_scheduler_init::deferred)
    #endif
{
    mAppName = appName;
    mGrabInput = grabInput;
    mFSLayer = new Ogre::FileSystemLayer(mAppName);
    mRoot = NULL;
    mWindow = NULL;
    mOverlaySystem = NULL;
    mSDLWindow = NULL;
    mFirstRun = true;
#ifdef INCLUDE_RTSHADER_SYSTEM
    mMaterialMgrListener = NULL;
    mShaderGenerator = NULL;
#endif
}

ApplicationContext::~ApplicationContext()
{
    delete mFSLayer;
}

void ApplicationContext::initApp()
{
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
    createRoot();

    if (!oneTimeConfig()) return;

    if (!mFirstRun) mRoot->setRenderSystem(mRoot->getRenderSystemByName(mNextRenderer));

    setup();

    mRoot->saveConfig();

    Ogre::Root::getSingleton().getRenderSystem()->_initRenderTargets();

    // Clear event times
    Ogre::Root::getSingleton().clearEventTimes();
#elif OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
    createRoot();

    setup();

    //mRoot->saveConfig();

    Ogre::Root::getSingleton().getRenderSystem()->_initRenderTargets();

    // Clear event times
    Ogre::Root::getSingleton().clearEventTimes();

#else
    createRoot();
#if OGRE_PLATFORM == OGRE_PLATFORM_NACL
    mNextRenderer = mRoot->getAvailableRenderers()[0]->getName();
#else
    if (!oneTimeConfig()) return;
#endif

    // if the context was reconfigured, set requested renderer
    if (!mFirstRun) mRoot->setRenderSystem(mRoot->getRenderSystemByName(mNextRenderer));

    setup();
#endif
}

void ApplicationContext::closeApp()
{
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
    shutdown();
#else
    mRoot->saveConfig();
    shutdown();
    if (mRoot)
    {
        OGRE_DELETE mRoot;
    }

#ifdef OGRE_STATIC_LIB
    mStaticPluginLoader.unload();
#endif
#endif
#if (OGRE_THREAD_PROVIDER == 3) && (OGRE_NO_TBB_SCHEDULER == 1)
    if (mTaskScheduler.is_active())
        mTaskScheduler.terminate();
#endif

#if OGRE_BITES_HAVE_SDL
    SDL_DestroyWindow(mSDLWindow);
    mSDLWindow = 0;
#endif
}

bool ApplicationContext::initialiseRTShaderSystem()
{
#ifdef INCLUDE_RTSHADER_SYSTEM
    if (Ogre::RTShader::ShaderGenerator::initialize())
    {
        mShaderGenerator = Ogre::RTShader::ShaderGenerator::getSingletonPtr();

#if OGRE_PLATFORM != OGRE_PLATFORM_ANDROID && OGRE_PLATFORM != OGRE_PLATFORM_NACL && OGRE_PLATFORM != OGRE_PLATFORM_WINRT
        // Setup core libraries and shader cache path.
        Ogre::StringVector groupVector = Ogre::ResourceGroupManager::getSingleton().getResourceGroups();
        Ogre::StringVector::iterator itGroup = groupVector.begin();
        Ogre::StringVector::iterator itGroupEnd = groupVector.end();
        Ogre::String shaderCoreLibsPath;
        Ogre::String shaderCachePath;

        for (; itGroup != itGroupEnd; ++itGroup)
        {
            Ogre::ResourceGroupManager::LocationList resLocationsList = Ogre::ResourceGroupManager::getSingleton().getResourceLocationList(*itGroup);
            Ogre::ResourceGroupManager::LocationList::iterator it = resLocationsList.begin();
            Ogre::ResourceGroupManager::LocationList::iterator itEnd = resLocationsList.end();
            bool coreLibsFound = false;

            // Try to find the location of the core shader lib functions and use it
            // as shader cache path as well - this will reduce the number of generated files
            // when running from different directories.
            for (; it != itEnd; ++it)
            {
                if ((*it)->archive->getName().find("RTShaderLib") != Ogre::String::npos)
                {
                    shaderCoreLibsPath = (*it)->archive->getName() + "/cache/";
                    shaderCachePath = shaderCoreLibsPath;
                    coreLibsFound = true;
                    break;
                }
            }
            // Core libs path found in the current group.
            if (coreLibsFound)
                break;
        }

        // Core shader libs not found -> shader generating will fail.
        if (shaderCoreLibsPath.empty())
            return false;

#ifdef _RTSS_WRITE_SHADERS_TO_DISK
        // Set shader cache path.
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        shaderCachePath = Ogre::macCachePath();
#elif OGRE_PLATFORM == OGRE_PLATFORM_APPLE
        shaderCachePath = Ogre::macCachePath() + "/org.ogre3d.RTShaderCache";
#endif
        mShaderGenerator->setShaderCachePath(shaderCachePath);
#endif
#endif
        // Create and register the material manager listener if it doesn't exist yet.
        if (mMaterialMgrListener == NULL) {
            mMaterialMgrListener = new SGTechniqueResolverListener(mShaderGenerator);
            Ogre::MaterialManager::getSingleton().addListener(mMaterialMgrListener);
        }
    }

    return true;
#else
    return false;
#endif
}

void ApplicationContext::destroyRTShaderSystem()
{
#ifdef INCLUDE_RTSHADER_SYSTEM
    // Restore default scheme.
    Ogre::MaterialManager::getSingleton().setActiveScheme(Ogre::MaterialManager::DEFAULT_SCHEME_NAME);

    // Unregister the material manager listener.
    if (mMaterialMgrListener != NULL)
    {
        Ogre::MaterialManager::getSingleton().removeListener(mMaterialMgrListener);
        delete mMaterialMgrListener;
        mMaterialMgrListener = NULL;
    }

    // Destroy RTShader system.
    if (mShaderGenerator != NULL)
    {
        Ogre::RTShader::ShaderGenerator::destroy();
        mShaderGenerator = NULL;
    }
#endif
}

void ApplicationContext::setup()
{
#if OGRE_BITES_HAVE_SDL
    SDL_Init(0);
    SDL_InitSubSystem(SDL_INIT_VIDEO);
#endif

    mWindow = createWindow();
    setupInput(mGrabInput);
    locateResources();
#ifdef INCLUDE_RTSHADER_SYSTEM
    initialiseRTShaderSystem();
#endif
    loadResources();

    // adds context as listener to process context-level (above the sample level) events
    mRoot->addFrameListener(this);
#if OGRE_PLATFORM != OGRE_PLATFORM_ANDROID
    Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);
#endif
}

void ApplicationContext::createRoot()
{
#if (OGRE_THREAD_PROVIDER == 3) && (OGRE_NO_TBB_SCHEDULER == 1)
    mTaskScheduler.initialize(OGRE_THREAD_HARDWARE_CONCURRENCY);
#endif
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
    mRoot = Ogre::Root::getSingletonPtr();
#else
    Ogre::String pluginsPath = Ogre::BLANKSTRING;
#   ifndef OGRE_STATIC_LIB
    pluginsPath = mFSLayer->getConfigFilePath("plugins.cfg");
#   endif
    mRoot = OGRE_NEW Ogre::Root(pluginsPath, mFSLayer->getWritablePath("ogre.cfg"),
                                mFSLayer->getWritablePath("ogre.log"));

#   ifdef OGRE_STATIC_LIB
    mStaticPluginLoader.load();
#   endif
#endif

    mOverlaySystem = OGRE_NEW Ogre::OverlaySystem();
}

bool ApplicationContext::oneTimeConfig()
{
    if (!mRoot->restoreConfig())
        return mRoot->showConfigDialog();
    return true;
}

void ApplicationContext::createDummyScene()
{
    mWindow->removeAllViewports();
    Ogre::SceneManager* sm = mRoot->createSceneManager(Ogre::ST_GENERIC, "DummyScene");
    sm->addRenderQueueListener(mOverlaySystem);
    Ogre::Camera* cam = sm->createCamera("DummyCamera");
    mWindow->addViewport(cam);
#ifdef INCLUDE_RTSHADER_SYSTEM
    // Initialize shader generator.
    // Must be before resource loading in order to allow parsing extended material attributes.
    if (!initialiseRTShaderSystem())
    {
        OGRE_EXCEPT(Ogre::Exception::ERR_FILE_NOT_FOUND,
                    "Shader Generator Initialization failed - Core shader libs path not found",
                    "ApplicationContext::createDummyScene");
    }

    mShaderGenerator->addSceneManager(sm);
#endif // INCLUDE_RTSHADER_SYSTEM
}

void ApplicationContext::destroyDummyScene()
{
    if(!mRoot->hasSceneManager("DummyScene"))
        return;

    Ogre::SceneManager*  dummyScene = mRoot->getSceneManager("DummyScene");
#ifdef INCLUDE_RTSHADER_SYSTEM
    mShaderGenerator->removeSceneManager(dummyScene);
#endif
    dummyScene->removeRenderQueueListener(mOverlaySystem);
    mWindow->removeAllViewports();
    mRoot->destroySceneManager(dummyScene);
}

Ogre::RenderWindow *ApplicationContext::createWindow()
{
    mRoot->initialise(false, mAppName);
    Ogre::NameValuePairList miscParams;
#if OGRE_PLATFORM == OGRE_PLATFORM_NACL
    miscParams["pp::Instance"] = Ogre::StringConverter::toString((unsigned long)mNaClInstance);
    miscParams["SwapCallback"] = Ogre::StringConverter::toString((unsigned long)mNaClSwapCallback);
    // create 1x1 window - we will resize later
    return mRoot->createRenderWindow(mAppName, mInitWidth, mInitHeight, false, &miscParams);

#elif (OGRE_PLATFORM == OGRE_PLATFORM_WINRT)
    Ogre::RenderWindow* res;
    if(mNativeWindow.Get())
    {
        miscParams["externalWindowHandle"] = Ogre::StringConverter::toString((size_t)reinterpret_cast<void*>(mNativeWindow.Get()));
        res = mRoot->createRenderWindow(mAppName, mNativeWindow->Bounds.Width, mNativeWindow->Bounds.Height, false, &miscParams);
    }
#       if !__OGRE_WINRT_PHONE_80
    else if(mNativeControl)
    {
        miscParams["windowType"] = "SurfaceImageSource";
        res = mRoot->createRenderWindow(mAppName, mNativeControl->ActualWidth, mNativeControl->ActualHeight, false, &miscParams);
        void* pUnk = NULL;
        res->getCustomAttribute("ImageBrush", &pUnk);
        mNativeControl->Fill = reinterpret_cast<Windows::UI::Xaml::Media::ImageBrush^>(pUnk);
    }
#       endif // !__OGRE_WINRT_PHONE_80

    return res;

#elif OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
    return NULL;
#else
    Ogre::ConfigOptionMap ropts = mRoot->getRenderSystem()->getConfigOptions();

    size_t w, h;

    std::istringstream mode(ropts["Video Mode"].currentValue);
    Ogre::String token;
    mode >> w; // width
    mode >> token; // 'x' as seperator between width and height
    mode >> h; // height

    miscParams["FSAA"] = ropts["FSAA"].currentValue;
    miscParams["vsync"] = ropts["VSync"].currentValue;

#if OGRE_BITES_HAVE_SDL
    mSDLWindow = SDL_CreateWindow(mAppName.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_RESIZABLE);

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(mSDLWindow, &wmInfo);

#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
    miscParams["parentWindowHandle"] = Ogre::StringConverter::toString(size_t(wmInfo.info.x11.window));
#elif OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    miscParams["externalWindowHandle"] = Ogre::StringConverter::toString(size_t(wmInfo.info.win.window));
#elif OGRE_PLATFORM == OGRE_PLATFORM_APPLE
    assert(wmInfo.subsystem == SDL_SYSWM_COCOA);
    miscParams["externalWindowHandle"] = Ogre::StringConverter::toString(size_t(wmInfo.info.cocoa.window));
#endif
#endif
    return mRoot->createRenderWindow(mAppName, w, h, false, &miscParams);
#endif
}

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
void ApplicationContext::initAppForAndroid(Ogre::RenderWindow *window, struct android_app* app)
{
    mWindow = window;

    if(app != NULL)
    {
        mAssetMgr = app->activity->assetManager;
        Ogre::ArchiveManager::getSingleton().addArchiveFactory( new Ogre::APKFileSystemArchiveFactory(app->activity->assetManager) );
        Ogre::ArchiveManager::getSingleton().addArchiveFactory( new Ogre::APKZipArchiveFactory(app->activity->assetManager) );
    }
}

Ogre::DataStreamPtr ApplicationContext::openAPKFile(const Ogre::String& fileName)
{
    Ogre::DataStreamPtr stream;
    AAsset* asset = AAssetManager_open(mAssetMgr, fileName.c_str(), AASSET_MODE_BUFFER);
    if(asset)
    {
        off_t length = AAsset_getLength(asset);
        void* membuf = OGRE_MALLOC(length, Ogre::MEMCATEGORY_GENERAL);
        memcpy(membuf, AAsset_getBuffer(asset), length);
        AAsset_close(asset);

        stream = Ogre::DataStreamPtr(new Ogre::MemoryDataStream(membuf, length, true, true));
    }
    return stream;
}
#endif

void ApplicationContext::setupInput(bool _grab)
{
#if OGRE_BITES_HAVE_SDL
    if (SDL_InitSubSystem(SDL_INIT_EVENTS) != 0)
    {
        OGRE_EXCEPT(Ogre::Exception::ERR_INVALID_STATE,
                    Ogre::String("Could not initialize SDL2 input: ")
                    + SDL_GetError(),
                    "SampleContext::setupInput");
    }

    SDL_ShowCursor(SDL_FALSE);

    SDL_bool grab = SDL_bool(_grab);

    SDL_SetWindowGrab(mSDLWindow, grab);
    SDL_SetRelativeMouseMode(grab);
#endif
}

void ApplicationContext::locateResources()
{
#if OGRE_PLATFORM == OGRE_PLATFORM_NACL
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation("Essential.zip", "EmbeddedZip", "Essential");
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation("Popular.zip", "EmbeddedZip", "Popular");
#else
    // load resource paths from config file
    Ogre::ConfigFile cf;
#   if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
    cf.load(openAPKFile(mFSLayer->getConfigFilePath("resources.cfg")));
#   else
    cf.load(mFSLayer->getConfigFilePath("resources.cfg"));
#   endif
    Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
    Ogre::String sec, type, arch;

    // go through all specified resource groups
    while (seci.hasMoreElements())
    {
        sec = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap* settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;

        // go through all resource paths
        for (i = settings->begin(); i != settings->end(); i++)
        {
            type = i->first;
            arch = i->second;

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
            // OS X does not set the working directory relative to the app,
            // In order to make things portable on OS X we need to provide
            // the loading with it's own bundle path location
            if (!Ogre::StringUtil::startsWith(arch, "/", false)) // only adjust relative dirs
                arch = Ogre::String(Ogre::macBundlePath() + "/" + arch);
#endif
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch, type, sec);
        }
    }


    const Ogre::ResourceGroupManager::LocationList genLocs = Ogre::ResourceGroupManager::getSingleton().getResourceLocationList("General");
    arch = genLocs.front()->archive->getName();
#   if OGRE_PLATFORM != OGRE_PLATFORM_ANDROID
#       if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
    arch = Ogre::macBundlePath() + "/Contents/Resources/Media";
#       elif OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
    arch = Ogre::macBundlePath() + "/Media";
#       else
    arch = Ogre::StringUtil::replaceAll(arch, "Media/../../Tests/Media", "");
    arch = Ogre::StringUtil::replaceAll(arch, "media/../../Tests/Media", "");
#       endif
    type = "FileSystem";
    sec = "Popular";

#		ifdef OGRE_BUILD_PLUGIN_CG
    bool use_HLSL_Cg_shared = true;
#		else
    bool use_HLSL_Cg_shared = Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("hlsl");
#		endif

    // Add locations for supported shader languages
    if(Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("glsles"))
    {
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch + "/materials/programs/GLSLES", type, sec);
    }
    else if(Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("glsl"))
    {
        if(Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("glsl150"))
        {
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch + "/materials/programs/GLSL150", type, sec);
        }
        else
        {
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch + "/materials/programs/GLSL", type, sec);
        }

        if(Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("glsl400"))
        {
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch + "/materials/programs/GLSL400", type, sec);
        }
    }
    else if(Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("hlsl"))
    {
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch + "/materials/programs/HLSL", type, sec);
    }
#       ifdef OGRE_BUILD_PLUGIN_CG
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch + "/materials/programs/Cg", type, sec);
#       endif
    if (use_HLSL_Cg_shared)
    {
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch + "/materials/programs/HLSL_Cg", type, sec);
    }

#       ifdef INCLUDE_RTSHADER_SYSTEM
    if(Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("glsles"))
    {
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch + "/RTShaderLib/GLSL", type, sec);
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch + "/RTShaderLib/GLSLES", type, sec);
    }
    else if(Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("glsl"))
    {
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch + "/RTShaderLib/GLSL", type, sec);
        if(Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("glsl150"))
        {
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch + "/RTShaderLib/GLSL150", type, sec);
        }
    }
    else if(Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("hlsl"))
    {
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch + "/RTShaderLib/HLSL", type, sec);
    }
#           ifdef OGRE_BUILD_PLUGIN_CG
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch + "/RTShaderLib/Cg", type, sec);
#           endif
    if (use_HLSL_Cg_shared)
    {
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch + "/RTShaderLib/HLSL_Cg", type, sec);
    }
#       endif /* INCLUDE_RTSHADER_SYSTEM */
#   endif /* OGRE_PLATFORM != OGRE_PLATFORM_ANDROID */
#endif /* OGRE_PLATFORM == OGRE_PLATFORM_NACL */
}

void ApplicationContext::loadResources()
{
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}

void ApplicationContext::reconfigure(const Ogre::String &renderer, Ogre::NameValuePairList &options)
{
    mNextRenderer = renderer;
    Ogre::RenderSystem* rs = mRoot->getRenderSystemByName(renderer);

    // set all given render system options
    for (Ogre::NameValuePairList::iterator it = options.begin(); it != options.end(); it++)
    {
        rs->setConfigOption(it->first, it->second);

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        // Change the viewport orientation on the fly if requested
        if(it->first == "Orientation")
        {
            if (it->second == "Landscape Left")
                mWindow->getViewport(0)->setOrientationMode(Ogre::OR_LANDSCAPELEFT, true);
            else if (it->second == "Landscape Right")
                mWindow->getViewport(0)->setOrientationMode(Ogre::OR_LANDSCAPERIGHT, true);
            else if (it->second == "Portrait")
                mWindow->getViewport(0)->setOrientationMode(Ogre::OR_PORTRAIT, true);
        }
#endif
    }

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
    // Need to save the config on iOS to make sure that changes are kept on disk
    mRoot->saveConfig();
#endif
    mRoot->queueEndRendering();   // break from render loop
}

void ApplicationContext::shutdown()
{
    // remove window event listener before shutting down SDL
    Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);

    if (mOverlaySystem)
    {
        OGRE_DELETE mOverlaySystem;
    }

#if OGRE_BITES_HAVE_SDL
    SDL_QuitSubSystem(SDL_INIT_EVENTS);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
#endif
}

void ApplicationContext::captureInputDevices()
{
#if OGRE_BITES_HAVE_SDL
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            mRoot->queueEndRendering();
            break;
        case SDL_WINDOWEVENT:
            if(event.window.event == SDL_WINDOWEVENT_RESIZED) {
                mWindow->resize(event.window.data1, event.window.data2);
                windowResized(mWindow);
            }
            break;
        case SDL_KEYDOWN:
            // Ignore repeated signals from key being held down.
            if (event.key.repeat) break;
            keyPressed(event.key);
            break;
        case SDL_KEYUP:
            keyReleased(event.key);
            break;
        case SDL_MOUSEBUTTONDOWN:
            mousePressed(event.button);
            break;
        case SDL_MOUSEBUTTONUP:
            mouseReleased(event.button);
            break;
        case SDL_MOUSEWHEEL:
            mouseWheelRolled(event.wheel);
            break;
        case SDL_MOUSEMOTION:
            mouseMoved(event.motion);
            break;
        case SDL_FINGERDOWN:
            touchPressed(event.tfinger);
            break;
        case SDL_FINGERUP:
            touchReleased(event.tfinger);
            break;
        case SDL_FINGERMOTION:
            touchMoved(event.tfinger);
            break;
        }
    }
#endif
}

}


