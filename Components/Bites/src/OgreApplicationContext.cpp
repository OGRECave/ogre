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

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
    mAssetMgr = NULL;
    mAConfig = NULL;
    mAndroidWinHdl = 0;
#endif

#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
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
    createRoot();
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
    if (!oneTimeConfig()) return;

    if (!mFirstRun) mRoot->setRenderSystem(mRoot->getRenderSystemByName(mNextRenderer));

    setup();

    mRoot->saveConfig();

    Ogre::Root::getSingleton().getRenderSystem()->_initRenderTargets();

    // Clear event times
    Ogre::Root::getSingleton().clearEventTimes();
#else

#if OGRE_PLATFORM == OGRE_PLATFORM_NACL
    mNextRenderer = mRoot->getAvailableRenderers()[0]->getName();
#else
    if (!oneTimeConfig()) return;
#endif

#if OGRE_PLATFORM != OGRE_PLATFORM_ANDROID
    // if the context was reconfigured, set requested renderer
    if (!mFirstRun) mRoot->setRenderSystem(mRoot->getRenderSystemByName(mNextRenderer));
#endif

    setup();
#endif
}

void ApplicationContext::closeApp()
{
#if OGRE_PLATFORM != OGRE_PLATFORM_ANDROID
    mRoot->saveConfig();
#endif

    shutdown();
    if (mRoot)
    {
        OGRE_DELETE mRoot;
        mRoot = NULL;
    }

    mWindow = NULL;

#ifdef OGRE_STATIC_LIB
    mStaticPluginLoader.unload();
#endif

#if (OGRE_THREAD_PROVIDER == 3) && (OGRE_NO_TBB_SCHEDULER == 1)
    if (mTaskScheduler.is_active())
        mTaskScheduler.terminate();
#endif

#if OGRE_BITES_HAVE_SDL
    SDL_DestroyWindow(mSDLWindow);
    mSDLWindow = NULL;
#endif
}

bool ApplicationContext::initialiseRTShaderSystem()
{
#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
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
#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
    //mShaderGenerator->invalidateScheme( Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME );
    //mShaderGenerator->removeAllShaderBasedTechniques();
    //mShaderGenerator->flushShaderCache();

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
#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
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

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
    mRoot = OGRE_NEW Ogre::Root();
#else
    Ogre::String pluginsPath = Ogre::BLANKSTRING;
#   ifndef OGRE_STATIC_LIB
    pluginsPath = mFSLayer->getConfigFilePath("plugins.cfg");
#   endif
    mRoot = OGRE_NEW Ogre::Root(pluginsPath, mFSLayer->getWritablePath("ogre.cfg"),
                                mFSLayer->getWritablePath("ogre.log"));
#endif

#ifdef OGRE_STATIC_LIB
    mStaticPluginLoader.load();
#endif
    mOverlaySystem = OGRE_NEW Ogre::OverlaySystem();
}

bool ApplicationContext::oneTimeConfig()
{
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
    mRoot->setRenderSystem(mRoot->getAvailableRenderers().at(0));
#else
    if (!mRoot->restoreConfig())
        return mRoot->showConfigDialog();
#endif
    return true;
}

void ApplicationContext::createDummyScene()
{
    mWindow->removeAllViewports();
    Ogre::SceneManager* sm = mRoot->createSceneManager(Ogre::ST_GENERIC, "DummyScene");
    sm->addRenderQueueListener(mOverlaySystem);
    Ogre::Camera* cam = sm->createCamera("DummyCamera");
    mWindow->addViewport(cam);
#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
    // Initialize shader generator.
    // Must be before resource loading in order to allow parsing extended material attributes.
    if (!initialiseRTShaderSystem())
    {
        OGRE_EXCEPT(Ogre::Exception::ERR_FILE_NOT_FOUND,
                    "Shader Generator Initialization failed - Core shader libs path not found",
                    "ApplicationContext::createDummyScene");
    }

    mShaderGenerator->addSceneManager(sm);
#endif // OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
}

void ApplicationContext::destroyDummyScene()
{
    if(!mRoot->hasSceneManager("DummyScene"))
        return;

    Ogre::SceneManager*  dummyScene = mRoot->getSceneManager("DummyScene");
#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
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
    miscParams["externalWindowHandle"] = Ogre::StringConverter::toString(mAndroidWinHdl);
    miscParams["androidConfig"] = Ogre::StringConverter::toString(reinterpret_cast<size_t>(mAConfig));
    miscParams["preserveContext"] = "true"; //Optionally preserve the gl context, prevents reloading all resources, this is false by default

    return Ogre::Root::getSingleton().createRenderWindow(mAppName, 0, 0, false, &miscParams);
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
void ApplicationContext::initAppForAndroid(AConfiguration* config, struct android_app* app)
{
    mAConfig = config;
    mAndroidWinHdl = reinterpret_cast<size_t>(app->window);
    mAssetMgr = app->activity->assetManager;
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

void ApplicationContext::_fireInputEventAndroid(AInputEvent* event, int wheel) {
    Event evt = {0};

    static TouchFingerEvent lastTouch = {0};

    if(wheel) {
        evt.type = SDL_MOUSEWHEEL;
        evt.wheel.y = wheel;
        _fireInputEvent(evt);
        mLastTouch.fingerId = -1; // prevent move-jump after pinch is over
        return;
    }

    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        int32_t action = AMOTION_EVENT_ACTION_MASK & AMotionEvent_getAction(event);

        switch (action) {
        case AMOTION_EVENT_ACTION_DOWN:
            evt.type = SDL_FINGERDOWN;
            break;
        case AMOTION_EVENT_ACTION_UP:
            evt.type = SDL_FINGERUP;
            break;
        case AMOTION_EVENT_ACTION_MOVE:
            evt.type = SDL_FINGERMOTION;
            break;
        default:
            return;
        }

        evt.tfinger.fingerId = AMotionEvent_getPointerId(event, 0);
        evt.tfinger.x = AMotionEvent_getRawX(event, 0) / mWindow->getWidth();
        evt.tfinger.y = AMotionEvent_getRawY(event, 0) / mWindow->getHeight();

        if(evt.type == SDL_FINGERMOTION) {
            if(evt.tfinger.fingerId != mLastTouch.fingerId)
                return; // wrong finger

            evt.tfinger.dx = evt.tfinger.x - mLastTouch.x;
            evt.tfinger.dy = evt.tfinger.y - mLastTouch.y;
        }

        lastTouch = evt.tfinger;
    } else {
        if(AKeyEvent_getKeyCode(event) != AKEYCODE_BACK)
            return;

        evt.type = AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_DOWN ? SDL_KEYDOWN : SDL_KEYUP;
        evt.key.keysym.scancode = SDL_SCANCODE_ESCAPE;
    }

    _fireInputEvent(evt);
}
#endif

void ApplicationContext::_fireInputEvent(const Event& event) {
    switch (event.type)
    {
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
        // for finger down we have to move the pointer first
        touchMoved(event.tfinger);
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
    Ogre::ArchiveManager::getSingleton().addArchiveFactory( new Ogre::APKFileSystemArchiveFactory(mAssetMgr) );
    Ogre::ArchiveManager::getSingleton().addArchiveFactory( new Ogre::APKZipArchiveFactory(mAssetMgr) );
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

#       ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
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
#       endif /* OGRE_BUILD_COMPONENT_RTSHADERSYSTEM */
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
        default:
            _fireInputEvent(event);
            break;
        }
    }
#endif
}

}


