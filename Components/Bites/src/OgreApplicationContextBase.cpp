// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.

#include "OgreApplicationContextBase.h"

#include "OgreRoot.h"
#include "OgreGpuProgramManager.h"
#include "OgreConfigFile.h"
#include "OgreRenderWindow.h"
#include "OgreViewport.h"
#include "OgreOverlaySystem.h"
#include "OgreDataStream.h"
#include "OgreBitesConfigDialog.h"
#include "OgreWindowEventUtilities.h"
#include "OgreSceneNode.h"
#include "OgreCamera.h"

#include "OgreArchiveManager.h"

#include "OgreConfigPaths.h"

namespace OgreBites {

static const char* SHADER_CACHE_FILENAME = "cache.bin";

ApplicationContextBase::ApplicationContextBase(const Ogre::String& appName)
{
    mAppName = appName;
    mFSLayer = new Ogre::FileSystemLayer(mAppName);
    mRoot = NULL;
    mOverlaySystem = NULL;
    mFirstRun = true;

#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
    mMaterialMgrListener = NULL;
    mShaderGenerator = NULL;
#endif
}

ApplicationContextBase::~ApplicationContextBase()
{
    delete mFSLayer;
}

void ApplicationContextBase::initApp()
{
    createRoot();
    if (!oneTimeConfig()) return;

#if OGRE_PLATFORM != OGRE_PLATFORM_ANDROID
    // if the context was reconfigured, set requested renderer
    if (!mFirstRun) mRoot->setRenderSystem(mRoot->getRenderSystemByName(mNextRenderer));
#endif

    setup();

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
    mRoot->saveConfig();

    Ogre::Root::getSingleton().getRenderSystem()->_initRenderTargets();

    // Clear event times
    Ogre::Root::getSingleton().clearEventTimes();
#endif
}

void ApplicationContextBase::closeApp()
{
    shutdown();
    if (mRoot)
    {
#if OGRE_PLATFORM != OGRE_PLATFORM_ANDROID
        mRoot->saveConfig();
#endif
        OGRE_DELETE mRoot;
        mRoot = NULL;
    }

#ifdef OGRE_STATIC_LIB
    mStaticPluginLoader.unload();
#endif
}

bool ApplicationContextBase::initialiseRTShaderSystem()
{
#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
    if (Ogre::RTShader::ShaderGenerator::initialize())
    {
        mShaderGenerator = Ogre::RTShader::ShaderGenerator::getSingletonPtr();

        // Create and register the material manager listener if it doesn't exist yet.
        if (!mMaterialMgrListener) {
            mMaterialMgrListener = new SGTechniqueResolverListener(mShaderGenerator);
            Ogre::MaterialManager::getSingleton().addListener(mMaterialMgrListener);
        }

        return true;
    }
#endif
    return false;
}

void ApplicationContextBase::setRTSSWriteShadersToDisk(bool write)
{
#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
    if(!write) {
        mShaderGenerator->setShaderCachePath("");
        return;
    }

    // Set shader cache path.
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
    auto subdir = "";
#elif OGRE_PLATFORM == OGRE_PLATFORM_APPLE
    auto subdir = "org.ogre3d.RTShaderCache/";
#else
    auto subdir = "RTShaderCache";
#endif
    auto path = mFSLayer->getWritablePath(subdir);
    if (!Ogre::FileSystemLayer::fileExists(path))
    {
        Ogre::FileSystemLayer::createDirectory(path);
    }
    mShaderGenerator->setShaderCachePath(path);
#endif
}

void ApplicationContextBase::destroyRTShaderSystem()
{
#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
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

void ApplicationContextBase::setup()
{
    mRoot->initialise(false);
    createWindow(mAppName);

    locateResources();
    initialiseRTShaderSystem();
    loadResources();

    // adds context as listener to process context-level (above the sample level) events
    mRoot->addFrameListener(this);
}

void ApplicationContextBase::createRoot()
{
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
    mRoot = OGRE_NEW Ogre::Root("");
#else
    Ogre::String pluginsPath;
#   ifndef OGRE_STATIC_LIB
    pluginsPath = mFSLayer->getConfigFilePath("plugins.cfg");

    if (!Ogre::FileSystemLayer::fileExists(pluginsPath))
    {
        pluginsPath = Ogre::FileSystemLayer::resolveBundlePath(OGRE_CONFIG_DIR "/plugins.cfg");
    }
#   endif

    mRoot = OGRE_NEW Ogre::Root(pluginsPath, mFSLayer->getWritablePath("ogre.cfg"),
                                mFSLayer->getWritablePath("ogre.log"));
#endif

#ifdef OGRE_STATIC_LIB
    mStaticPluginLoader.load();
#endif
    mOverlaySystem = OGRE_NEW Ogre::OverlaySystem();
}

bool ApplicationContextBase::oneTimeConfig()
{
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
    mRoot->setRenderSystem(mRoot->getAvailableRenderers().at(0));
#else
    if (!mRoot->restoreConfig()) {
        return mRoot->showConfigDialog(OgreBites::getNativeConfigDialog());
    }
#endif
    return true;
}

void ApplicationContextBase::createDummyScene()
{
    mWindows[0].render->removeAllViewports();
    Ogre::SceneManager* sm = mRoot->createSceneManager("DefaultSceneManager", "DummyScene");
    sm->addRenderQueueListener(mOverlaySystem);
    Ogre::Camera* cam = sm->createCamera("DummyCamera");
    sm->getRootSceneNode()->attachObject(cam);
    mWindows[0].render->addViewport(cam);
#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
    // Initialize shader generator.
    // Must be before resource loading in order to allow parsing extended material attributes.
    if (!initialiseRTShaderSystem())
    {
        OGRE_EXCEPT(Ogre::Exception::ERR_FILE_NOT_FOUND,
                    "Shader Generator Initialization failed - Core shader libs path not found",
                    "ApplicationContextBase::createDummyScene");
    }

    mShaderGenerator->addSceneManager(sm);
#endif // OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
}

void ApplicationContextBase::destroyDummyScene()
{
    if(!mRoot->hasSceneManager("DummyScene"))
        return;

    Ogre::SceneManager*  dummyScene = mRoot->getSceneManager("DummyScene");
#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
    mShaderGenerator->removeSceneManager(dummyScene);
#endif
    dummyScene->removeRenderQueueListener(mOverlaySystem);
    mWindows[0].render->removeAllViewports();
    mRoot->destroySceneManager(dummyScene);
}

void ApplicationContextBase::enableShaderCache() const
{
    Ogre::GpuProgramManager::getSingleton().setSaveMicrocodesToCache(true);

    // Load for a package version of the shaders.
    Ogre::String path = mFSLayer->getWritablePath(SHADER_CACHE_FILENAME);
    std::ifstream inFile(path.c_str(), std::ios::binary);
    if (!inFile.is_open())
    {
        Ogre::LogManager::getSingleton().logWarning("Could not open '"+path+"'");
        return;
    }
    Ogre::LogManager::getSingleton().logMessage("Loading shader cache from '"+path+"'");
    Ogre::DataStreamPtr istream(new Ogre::FileStreamDataStream(path, &inFile, false));
    Ogre::GpuProgramManager::getSingleton().loadMicrocodeCache(istream);
}

void ApplicationContextBase::addInputListener(NativeWindowType* win, InputListener* lis)
{
    mInputListeners.insert(std::make_pair(0, lis));
}


void ApplicationContextBase::removeInputListener(NativeWindowType* win, InputListener* lis)
{
    mInputListeners.erase(std::make_pair(0, lis));
}

bool ApplicationContextBase::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    for(InputListenerList::iterator it = mInputListeners.begin();
            it != mInputListeners.end(); ++it) {
        it->second->frameRendered(evt);
    }

    return true;
}

NativeWindowPair ApplicationContextBase::createWindow(const Ogre::String& name, Ogre::uint32 w, Ogre::uint32 h, Ogre::NameValuePairList miscParams)
{
    NativeWindowPair ret = {NULL, NULL};

    if(!mWindows.empty())
    {
        // additional windows should reuse the context
        miscParams["currentGLContext"] = "true";
    }

    auto p = mRoot->getRenderSystem()->getRenderWindowDescription();
    miscParams.insert(p.miscParams.begin(), p.miscParams.end());
    p.miscParams = miscParams;
    p.name = name;

    if(w > 0 && h > 0)
    {
        p.width = w;
        p.height= h;
    }

    ret.render = mRoot->createRenderWindow(p);

    mWindows.push_back(ret);

    WindowEventUtilities::_addRenderWindow(ret.render);

    return ret;
}

void ApplicationContextBase::destroyWindow(const Ogre::String& name)
{
    for (auto it = mWindows.begin(); it != mWindows.end(); ++it)
    {
        if (it->render->getName() != name)
            continue;
        _destroyWindow(*it);
        mWindows.erase(it);
        return;
    }

    OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, "No window named '"+name+"'");
}

void ApplicationContextBase::_destroyWindow(const NativeWindowPair& win)
{
#if !OGRE_BITES_HAVE_SDL
    // remove window event listener before destroying it
    WindowEventUtilities::_removeRenderWindow(win.render);
#endif
    mRoot->destroyRenderTarget(win.render);
}

void ApplicationContextBase::_fireInputEvent(const Event& event, uint32_t windowID) const
{
    for(InputListenerList::iterator it = mInputListeners.begin();
            it != mInputListeners.end(); ++it)
    {
        if(it->first != windowID) continue;

        InputListener& l = *it->second;

        switch (event.type)
        {
        case KEYDOWN:
            l.keyPressed(event.key);
            break;
        case KEYUP:
            l.keyReleased(event.key);
            break;
        case MOUSEBUTTONDOWN:
            l.mousePressed(event.button);
            break;
        case MOUSEBUTTONUP:
            l.mouseReleased(event.button);
            break;
        case MOUSEWHEEL:
            l.mouseWheelRolled(event.wheel);
            break;
        case MOUSEMOTION:
            l.mouseMoved(event.motion);
            break;
        case FINGERDOWN:
            // for finger down we have to move the pointer first
            l.touchMoved(event.tfinger);
            l.touchPressed(event.tfinger);
            break;
        case FINGERUP:
            l.touchReleased(event.tfinger);
            break;
        case FINGERMOTION:
            l.touchMoved(event.tfinger);
            break;
        case TEXTINPUT:
            l.textInput(event.text);
            break;
        }
    }
}

Ogre::String ApplicationContextBase::getDefaultMediaDir()
{
    return Ogre::FileSystemLayer::resolveBundlePath(OGRE_MEDIA_DIR);
}

void ApplicationContextBase::locateResources()
{
    auto& rgm = Ogre::ResourceGroupManager::getSingleton();
    // load resource paths from config file
    Ogre::ConfigFile cf;
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
    Ogre::Archive* apk = Ogre::ArchiveManager::getSingleton().load("", "APKFileSystem", true);
    cf.load(apk->open(mFSLayer->getConfigFilePath("resources.cfg")));
#else
    Ogre::String resourcesPath = mFSLayer->getConfigFilePath("resources.cfg");
    if (Ogre::FileSystemLayer::fileExists(resourcesPath) || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN)
    {
        cf.load(resourcesPath);
    }
    else
    {
        rgm.addResourceLocation(getDefaultMediaDir(), "FileSystem", Ogre::RGN_DEFAULT);
    }

#endif

    Ogre::String sec, type, arch;
    // go through all specified resource groups
    Ogre::ConfigFile::SettingsBySection_::const_iterator seci;
    for(seci = cf.getSettingsBySection().begin(); seci != cf.getSettingsBySection().end(); ++seci) {
        sec = seci->first;
        const Ogre::ConfigFile::SettingsMultiMap& settings = seci->second;
        Ogre::ConfigFile::SettingsMultiMap::const_iterator i;

        // go through all resource paths
        for (i = settings.begin(); i != settings.end(); i++)
        {
            type = i->first;
            arch = Ogre::FileSystemLayer::resolveBundlePath(i->second);

            rgm.addResourceLocation(arch, type, sec);
        }
    }

    if(rgm.getResourceLocationList(Ogre::RGN_INTERNAL).empty())
    {
        const auto& mediaDir = getDefaultMediaDir();
        // add default locations
        rgm.addResourceLocation(mediaDir + "/ShadowVolume", "FileSystem", Ogre::RGN_INTERNAL);
#ifdef OGRE_BUILD_COMPONENT_TERRAIN
        rgm.addResourceLocation(mediaDir + "/Terrain", "FileSystem", Ogre::RGN_INTERNAL);
#endif
#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
        rgm.addResourceLocation(mediaDir + "/RTShaderLib/materials", "FileSystem", Ogre::RGN_INTERNAL);
        rgm.addResourceLocation(mediaDir + "/RTShaderLib/GLSL", "FileSystem", Ogre::RGN_INTERNAL);
        rgm.addResourceLocation(mediaDir + "/RTShaderLib/HLSL_Cg", "FileSystem", Ogre::RGN_INTERNAL);
#endif
    }

    sec = Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME;
    const Ogre::ResourceGroupManager::LocationList genLocs = rgm.getResourceLocationList(sec);

    OgreAssert(!genLocs.empty(), ("Resource Group '"+sec+"' must contain at least one entry").c_str());

    arch = genLocs.front().archive->getName();

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
    arch = Ogre::FileSystemLayer::resolveBundlePath("Contents/Resources/Media");
#elif OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
    arch = Ogre::FileSystemLayer::resolveBundlePath("Media");
#else
    arch = Ogre::StringUtil::replaceAll(arch, "Media/../../Tests/Media", "");
    arch = Ogre::StringUtil::replaceAll(arch, "media/../../Tests/Media", "");
#endif
    type = genLocs.front().archive->getType();

    bool hasCgPlugin = false;
    const Ogre::Root::PluginInstanceList& plugins = getRoot()->getInstalledPlugins();
    for(size_t i = 0; i < plugins.size(); i++)
    {
        if(plugins[i]->getName() == "Cg Program Manager")
        {
            hasCgPlugin = true;
            break;
        }
    }

    bool use_HLSL_Cg_shared = hasCgPlugin || Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("hlsl");

    // Add locations for supported shader languages
    if(Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("glsles"))
    {
        rgm.addResourceLocation(arch + "/materials/programs/GLSL", type, sec);
        rgm.addResourceLocation(arch + "/materials/programs/GLSLES", type, sec);
    }
    else if(Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("glsl"))
    {
        rgm.addResourceLocation(arch + "/materials/programs/GLSL", type, sec);
        rgm.addResourceLocation(arch + "/materials/programs/GLSL120", type, sec);

        if(Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("glsl150"))
        {
            rgm.addResourceLocation(arch + "/materials/programs/GLSL150", type, sec);
        }
        if(Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("glsl400"))
        {
            rgm.addResourceLocation(arch + "/materials/programs/GLSL400", type, sec);
        }
    }
    else if(Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("hlsl"))
    {
        rgm.addResourceLocation(arch + "/materials/programs/HLSL", type, sec);
    }

    if (Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("spirv"))
    {
        rgm.addResourceLocation(arch + "/materials/programs/SPIRV", type, sec);
    }

    if(hasCgPlugin)
        rgm.addResourceLocation(arch + "/materials/programs/Cg", type, sec);
    if (use_HLSL_Cg_shared)
        rgm.addResourceLocation(arch + "/materials/programs/HLSL_Cg", type, sec);
}

void ApplicationContextBase::loadResources()
{
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}

void ApplicationContextBase::reconfigure(const Ogre::String &renderer, Ogre::NameValuePairList &options)
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
            Ogre::RenderWindow* win = getRenderWindow();

            if (it->second == "Landscape Left")
                win->getViewport(0)->setOrientationMode(Ogre::OR_LANDSCAPELEFT, true);
            else if (it->second == "Landscape Right")
                win->getViewport(0)->setOrientationMode(Ogre::OR_LANDSCAPERIGHT, true);
            else if (it->second == "Portrait")
                win->getViewport(0)->setOrientationMode(Ogre::OR_PORTRAIT, true);
        }
#endif
    }

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
    // Need to save the config on iOS to make sure that changes are kept on disk
    mRoot->saveConfig();
#endif
    mRoot->queueEndRendering();   // break from render loop
}

void ApplicationContextBase::shutdown()
{
    const auto& gpuMgr = Ogre::GpuProgramManager::getSingleton();
    if (gpuMgr.getSaveMicrocodesToCache() && gpuMgr.isCacheDirty())
    {
        Ogre::String path = mFSLayer->getWritablePath(SHADER_CACHE_FILENAME);
        std::fstream outFile(path.c_str(), std::ios::out | std::ios::binary);

        if (outFile.is_open())
        {
            Ogre::LogManager::getSingleton().logMessage("Writing shader cache to "+path);
            Ogre::DataStreamPtr ostream(new Ogre::FileStreamDataStream(path, &outFile, false));
            gpuMgr.saveMicrocodeCache(ostream);
        }
        else
            Ogre::LogManager::getSingleton().logWarning("Cannot open shader cache for writing "+path);
    }

#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
    // Destroy the RT Shader System.
    destroyRTShaderSystem();
#endif

    for(auto it = mWindows.rbegin(); it != mWindows.rend(); ++it)
    {
        _destroyWindow(*it);
    }
    mWindows.clear();

    if (mOverlaySystem)
    {
        OGRE_DELETE mOverlaySystem;
    }

    mInputListeners.clear();
}

void ApplicationContextBase::pollEvents()
{
    // just avoid "window not responding"
    WindowEventUtilities::messagePump();
}

}
