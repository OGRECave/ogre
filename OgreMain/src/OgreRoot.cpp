/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
// Ogre includes
#include <memory>

#include "OgreStableHeaders.h"

#include "OgreRenderWindow.h"
#include "OgreControllerManager.h"
#include "OgreDynLibManager.h"
#include "OgreDynLib.h"
#include "OgreConfigFile.h"
#include "OgreRenderSystemCapabilitiesManager.h"
#include "OgreSkeletonManager.h"
#include "OgreConfigDialog.h"
#include "OgrePlugin.h"
#include "OgreShadowVolumeExtrudeProgram.h"
#include "OgreResourceBackgroundQueue.h"
#include "OgreBillboardSet.h"
#include "OgreBillboardChain.h"
#include "OgreRibbonTrail.h"
#include "OgreConvexBody.h"
#include "OgreTimer.h"
#include "OgreFrameListener.h"
#include "OgreLodStrategyManager.h"
#include "OgreFileSystemLayer.h"
#include "OgreStaticGeometry.h"
#include "OgreSceneManagerEnumerator.h"

#if OGRE_NO_DDS_CODEC == 0
#include "OgreDDSCodec.h"
#endif

#include "OgreHardwareBufferManager.h"
#include "OgreGpuProgramManager.h"
#include "OgreExternalTextureSourceManager.h"
#include "OgreCompositorManager.h"

#if OGRE_NO_PVRTC_CODEC == 0
#  include "OgrePVRTCCodec.h"
#endif
#if OGRE_NO_ETC_CODEC == 0
#  include "OgreETCCodec.h"
#endif
#if OGRE_NO_ASTC_CODEC == 0
#  include "OgreASTCCodec.h"
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
#include "macUtils.h"
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
#include "Android/OgreAndroidLogListener.h"
#endif

namespace Ogre {
    //-----------------------------------------------------------------------
    template<> Root* Singleton<Root>::msSingleton = 0;
    Root* Root::getSingletonPtr(void)
    {
        return msSingleton;
    }
    Root& Root::getSingleton(void)
    {
        assert( msSingleton );  return ( *msSingleton );
    }

    typedef void (*DLL_START_PLUGIN)(void);
    typedef void (*DLL_STOP_PLUGIN)(void);

    //-----------------------------------------------------------------------
    Root::Root(const String& pluginFileName, const String& configFileName,
        const String& logFileName)
      : mQueuedEnd(false)
      , mCurrentSceneManager(NULL)
      , mNextFrame(0)
      , mFrameSmoothingTime(0.0f)
      , mRemoveQueueStructuresOnClear(false)
      , mDefaultMinPixelSize(0)
      , mNextMovableObjectTypeFlag(1)
      , mIsInitialised(false)
      , mIsBlendIndicesGpuRedundant(true)
      , mIsBlendWeightsGpuRedundant(true)
    {
        // superclass will do singleton checking

        // Init
        mActiveRenderer = 0;
        mVersion = StringConverter::toString(OGRE_VERSION_MAJOR) + "." +
            StringConverter::toString(OGRE_VERSION_MINOR) + "." +
            StringConverter::toString(OGRE_VERSION_PATCH) +
            OGRE_VERSION_SUFFIX + " " +
            "(" + OGRE_VERSION_NAME + ")";
        mConfigFileName = configFileName;

        // Create log manager and default log file if there is no log manager yet
        if(!LogManager::getSingletonPtr())
        {
            mLogManager = std::make_unique<LogManager>();

#if OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
            // suppress writing log to Emscripten virtual FS, improves performance
            mLogManager->createLog(logFileName, true, true, true);
#else
            mLogManager->createLog(logFileName, true, true);
#endif
        }

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
        mAndroidLogger.reset(new AndroidLogListener());
        mLogManager->getDefaultLog()->addListener(mAndroidLogger.get());
#endif

        mDynLibManager = std::make_unique<DynLibManager>();
        mArchiveManager = std::make_unique<ArchiveManager>();
        mResourceGroupManager = std::make_unique<ResourceGroupManager>();

        // WorkQueue (note: users can replace this if they want)
        DefaultWorkQueue* defaultQ = OGRE_NEW DefaultWorkQueue("Root");
        // match threads to hardware
        int threadCount = OGRE_THREAD_HARDWARE_CONCURRENCY;
        // but clamp it at 2 by default - we dont scale much beyond that currently
        // yet it helps on android where it needlessly burns CPU
        threadCount = Math::Clamp(threadCount, 1, 2);
        defaultQ->setWorkerThreadCount(threadCount);

        // only allow workers to access rendersystem if threadsupport is 1
        defaultQ->setWorkersCanAccessRenderSystem(OGRE_THREAD_SUPPORT == 1);
        mWorkQueue.reset(defaultQ);

        // ResourceBackgroundQueue
        mResourceBackgroundQueue = std::make_unique<ResourceBackgroundQueue>();

        // Create SceneManager enumerator (note - will be managed by singleton)
        mSceneManagerEnum = std::make_unique<SceneManagerEnumerator>();
        mShadowTextureManager = std::make_unique<ShadowTextureManager>();
        mRenderSystemCapabilitiesManager = std::make_unique<RenderSystemCapabilitiesManager>();
        mMaterialManager = std::make_unique<MaterialManager>();
        mMeshManager = std::make_unique<MeshManager>();
        mSkeletonManager = std::make_unique<SkeletonManager>();
        mParticleManager = std::make_unique<ParticleSystemManager>();
        mTimer = std::make_unique<Timer>();
        mLodStrategyManager = std::make_unique<LodStrategyManager>();

#if OGRE_PROFILING
        // Profiler
        mProfiler.reset(new Profiler());
        Profiler::getSingleton().setTimer(mTimer.get());
#endif


        mFileSystemArchiveFactory = std::make_unique<FileSystemArchiveFactory>();
        ArchiveManager::getSingleton().addArchiveFactory( mFileSystemArchiveFactory.get() );
#   if OGRE_NO_ZIP_ARCHIVE == 0
        mZipArchiveFactory = std::make_unique<ZipArchiveFactory>();
        ArchiveManager::getSingleton().addArchiveFactory( mZipArchiveFactory.get() );
        mEmbeddedZipArchiveFactory = std::make_unique<EmbeddedZipArchiveFactory>();
        ArchiveManager::getSingleton().addArchiveFactory( mEmbeddedZipArchiveFactory.get() );
#   endif

#if OGRE_NO_DDS_CODEC == 0
        // Register image codecs
        DDSCodec::startup();
#endif
#if OGRE_NO_PVRTC_CODEC == 0
        PVRTCCodec::startup();
#endif
#if OGRE_NO_ETC_CODEC == 0
        ETCCodec::startup();
#endif
#if OGRE_NO_ASTC_CODEC == 0
        ASTCCodec::startup();
#endif

        mGpuProgramManager = std::make_unique<GpuProgramManager>();
        mExternalTextureSourceManager = std::make_unique<ExternalTextureSourceManager>();
        mCompositorManager = std::make_unique<CompositorManager>();
        mCompilerManager = std::make_unique<ScriptCompilerManager>();

        // Auto window
        mAutoWindow = 0;

        // instantiate and register base movable factories
        mEntityFactory = std::make_unique<EntityFactory>();
        addMovableObjectFactory(mEntityFactory.get());
        mLightFactory = std::make_unique<LightFactory>();
        addMovableObjectFactory(mLightFactory.get());
        mBillboardSetFactory = std::make_unique<BillboardSetFactory>();
        addMovableObjectFactory(mBillboardSetFactory.get());
        mManualObjectFactory = std::make_unique<ManualObjectFactory>();
        addMovableObjectFactory(mManualObjectFactory.get());
        mBillboardChainFactory = std::make_unique<BillboardChainFactory>();
        addMovableObjectFactory(mBillboardChainFactory.get());
        mRibbonTrailFactory = std::make_unique<RibbonTrailFactory>();
        addMovableObjectFactory(mRibbonTrailFactory.get());
        mStaticGeometryFactory = std::make_unique<StaticGeometryFactory>();
        addMovableObjectFactory(mStaticGeometryFactory.get());
        mRectangle2DFactory = std::make_unique<Rectangle2DFactory>();
        addMovableObjectFactory(mRectangle2DFactory.get());

        // Load plugins
        if (!pluginFileName.empty())
            loadPlugins(pluginFileName);

        LogManager::getSingleton().logMessage("*-*-* OGRE Initialising");
        LogManager::getSingleton().logMessage("*-*-* Version " + mVersion);

        // Can't create managers until initialised
        mControllerManager = 0;

        mFirstTimePostWindowInit = false;
    }

    //-----------------------------------------------------------------------
    Root::~Root()
    {
        shutdown();

#if OGRE_NO_DDS_CODEC == 0
        DDSCodec::shutdown();
#endif
#if OGRE_NO_PVRTC_CODEC == 0
        PVRTCCodec::shutdown();
#endif
#if OGRE_NO_ETC_CODEC == 0
        ETCCodec::shutdown();
#endif
#if OGRE_NO_ASTC_CODEC == 0
        ASTCCodec::shutdown();
#endif
		mCompositorManager.reset(); // needs rendersystem
        mParticleManager.reset(); // may use plugins
        mMaterialManager.reset(); // may use GPU program manager
        mGpuProgramManager.reset(); // may use plugins
        unloadPlugins();

        mAutoWindow = 0;

        StringInterface::cleanupDictionary();

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
        mLogManager->getDefaultLog()->removeListener(mAndroidLogger.get());
#endif
    }

    //-----------------------------------------------------------------------
    void Root::saveConfig(void)
    {
#if OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
        OGRE_EXCEPT(Exception::ERR_CANNOT_WRITE_TO_FILE, "saveConfig is not supported",
            "Root::saveConfig");
#endif
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        // Check the Documents directory within the application sandbox
        Ogre::String outBaseName, extension, configFileName;
        Ogre::StringUtil::splitFilename(mConfigFileName, outBaseName, extension);
        configFileName = iOSDocumentsDirectory() + "/" + outBaseName;
		std::ofstream of(configFileName.c_str());
        if (of.is_open())
            mConfigFileName = configFileName;
        else
            mConfigFileName.clear();
#else
        if (mConfigFileName.empty())
            return;

        std::ofstream of(mConfigFileName.c_str());
#endif
        if (!of)
            OGRE_EXCEPT(Exception::ERR_CANNOT_WRITE_TO_FILE, "Cannot create settings file.",
            "Root::saveConfig");

        if (mActiveRenderer)
        {
            of << "Render System=" << mActiveRenderer->getName() << std::endl;
        }
        else
        {
            of << "Render System=" << std::endl;
        }

        for (const auto& r : getAvailableRenderers())
        {
            of << std::endl;
            of << "[" << r->getName() << "]" << std::endl;
            for (const auto& o : r->getConfigOptions()) {
                of << o.first << "=" << o.second.currentValue << std::endl;
            }
        }

        of.close();

    }
    //-----------------------------------------------------------------------
    bool Root::restoreConfig(void)
    {
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        // Read the config from Documents first(user config) if it exists on iOS.
        // If it doesn't exist or is invalid then use mConfigFileName

        Ogre::String outBaseName, extension, configFileName;
        Ogre::StringUtil::splitFilename(mConfigFileName, outBaseName, extension);
        configFileName = iOSDocumentsDirectory() + "/" + outBaseName;

        std::ifstream fp;
        fp.open(configFileName.c_str(), std::ios::in);
        if(fp.is_open())
        {
            // A config file exists in the users Documents dir, we'll use it
            mConfigFileName = configFileName;
        }
        else
        {
            std::ifstream configFp;

            // This might be the first run because there is no config file in the
            // Documents directory.  It could also mean that a config file isn't being used at all

            // Try the path passed into initialise
            configFp.open(mConfigFileName.c_str(), std::ios::in);

            // If we can't open this file then we have no default config file to work with
            // Use the documents dir then.
            if(!configFp.is_open())
            {
                // Check to see if one was included in the app bundle
                mConfigFileName = macBundlePath() + "/ogre.cfg";

                configFp.open(mConfigFileName.c_str(), std::ios::in);

                // If we can't open this file then we have no default config file to work with
                // Use the Documents dir then.
                if(!configFp.is_open())
                    mConfigFileName = configFileName;
            }

            configFp.close();
        }

        fp.close();
#endif

        if (mConfigFileName.empty ())
            return true;

        // Restores configuration from saved state
        // Returns true if a valid saved configuration is
        //   available, and false if no saved config is
        //   stored, or if there has been a problem
        ConfigFile cfg;

        try {
            // Don't trim whitespace
            cfg.load(mConfigFileName, "\t:=", false);
        }
        catch (FileNotFoundException&)
        {
            return false;
        }

        bool optionError = false;
        for(auto& seci : cfg.getSettingsBySection()) {
            const ConfigFile::SettingsMultiMap& settings = seci.second;
            const String& renderSystem = seci.first;

            RenderSystem* rs = getRenderSystemByName(renderSystem);
            if (!rs)
            {
                // Unrecognised render system
                continue;
            }

            for (const auto& p : settings)
            {
                try
                {
                    rs->setConfigOption(p.first, p.second);
                }
                catch(const InvalidParametersException& e)
                {
                    LogManager::getSingleton().logError(e.getDescription());
                    optionError = true;
                    continue;
                }
            }
        }

        RenderSystem* rs = getRenderSystemByName(cfg.getSetting("Render System"));
        if (!rs)
        {
            // Unrecognised render system
            return false;
        }

        String err = rs->validateConfigOptions();
        if (err.length() > 0)
            return false;

        setRenderSystem(rs);

        // Successful load
        return !optionError;
    }

    //-----------------------------------------------------------------------
    bool Root::showConfigDialog(ConfigDialog* dialog) {
        if(dialog) {
            if(!mActiveRenderer)
                restoreConfig();

            if (dialog->display()) {
                saveConfig();
                return true;
            }

            return false;
        }

        // just select the first available render system
        if (!mRenderers.empty())
        {
            setRenderSystem(mRenderers.front());
            return true;
        }

        return false;
    }

    //-----------------------------------------------------------------------
    const RenderSystemList& Root::getAvailableRenderers(void)
    {
        // Returns a vector of renders

        return mRenderers;

    }

    //-----------------------------------------------------------------------
    RenderSystem* Root::getRenderSystemByName(const String& name)
    {
        if (name.empty())
        {
            // No render system
            return NULL;
        }

        for (auto *rs : getAvailableRenderers())
        {
            if (rs->getName() == name)
                return rs;
        }

        // Unrecognised render system
        return NULL;
    }

    //-----------------------------------------------------------------------
    void Root::setRenderSystem(RenderSystem* system)
    {
        // Sets the active rendering system
        // Can be called direct or will be called by
        //   standard config dialog

        // Is there already an active renderer?
        // If so, disable it and init the new one
        if( mActiveRenderer && mActiveRenderer != system )
        {
            mActiveRenderer->shutdown();
        }

        mActiveRenderer = system;
        // Tell scene managers
        if(mSceneManagerEnum)
            mSceneManagerEnum->setRenderSystem(system);

        if(RenderSystem::Listener* ls = RenderSystem::getSharedListener())
            ls->eventOccurred("RenderSystemChanged");
    }
    //-----------------------------------------------------------------------
    void Root::addRenderSystem(RenderSystem *newRend)
    {
        mRenderers.push_back(newRend);
    }
    //-----------------------------------------------------------------------
    RenderSystem* Root::getRenderSystem(void)
    {
        // Gets the currently active renderer
        return mActiveRenderer;

    }

    //-----------------------------------------------------------------------
    RenderWindow* Root::initialise(bool autoCreateWindow, const String& windowTitle)
    {
        OgreAssert(mActiveRenderer, "Cannot initialise");

        if (!mControllerManager)
            mControllerManager = std::make_unique<ControllerManager>();

        PlatformInformation::log(LogManager::getSingleton().getDefaultLog());
        mActiveRenderer->_initialise();

        // Initialise timer
        mTimer->reset();

        // Init pools
        ConvexBody::_initialisePool();

        mIsInitialised = true;

        if (autoCreateWindow)
        {
            auto desc = mActiveRenderer->getRenderWindowDescription();
            desc.name = windowTitle;
            mAutoWindow = createRenderWindow(desc);
        }

        return mAutoWindow;

    }
    //-----------------------------------------------------------------------
    void Root::useCustomRenderSystemCapabilities(RenderSystemCapabilities* capabilities)
    {
        mActiveRenderer->useCustomRenderSystemCapabilities(capabilities);
    }
    //-----------------------------------------------------------------------
    void Root::addSceneManagerFactory(SceneManagerFactory* fact)
    {
        mSceneManagerEnum->addFactory(fact);
    }
    //-----------------------------------------------------------------------
    void Root::removeSceneManagerFactory(SceneManagerFactory* fact)
    {
        mSceneManagerEnum->removeFactory(fact);
    }
    //-----------------------------------------------------------------------
    const StringVector& Root::getSceneManagerTypes(void) const { return mSceneManagerEnum->getMetaData(); }
    //-----------------------------------------------------------------------
    SceneManager* Root::createSceneManager(const String& typeName, const String& instanceName)
    {
        return mSceneManagerEnum->createSceneManager(typeName, instanceName);
    }
    //-----------------------------------------------------------------------
    void Root::destroySceneManager(SceneManager* sm)
    {
        mSceneManagerEnum->destroySceneManager(sm);
    }
    //-----------------------------------------------------------------------
    SceneManager* Root::getSceneManager(const String& instanceName) const
    {
        return mSceneManagerEnum->getSceneManager(instanceName);
    }
    //---------------------------------------------------------------------
    bool Root::hasSceneManager(const String& instanceName) const
    {
        return mSceneManagerEnum->hasSceneManager(instanceName);
    }
    //-----------------------------------------------------------------------
    const SceneManagerInstanceMap& Root::getSceneManagers(void) const { return mSceneManagerEnum->getSceneManagers(); }
    //-----------------------------------------------------------------------
    TextureManager* Root::getTextureManager(void)
    {
        return &TextureManager::getSingleton();
    }
    //-----------------------------------------------------------------------
    MeshManager* Root::getMeshManager(void)
    {
        return &MeshManager::getSingleton();
    }
    //-----------------------------------------------------------------------
    void Root::addFrameListener(FrameListener* newListener)
    {
        mRemovedFrameListeners.erase(newListener);
        mAddedFrameListeners.insert(newListener);
    }
    //-----------------------------------------------------------------------
    void Root::removeFrameListener(FrameListener* oldListener)
    {
        mAddedFrameListeners.erase(oldListener);
        mRemovedFrameListeners.insert(oldListener);
    }
    //-----------------------------------------------------------------------
    void Root::_syncAddedRemovedFrameListeners()
    {
        for (auto& l : mRemovedFrameListeners)
            mFrameListeners.erase(l);
        mRemovedFrameListeners.clear();

        for (auto& l : mAddedFrameListeners)
            mFrameListeners.insert(l);
        mAddedFrameListeners.clear();
    }
    //-----------------------------------------------------------------------
    bool Root::_fireFrameStarted(FrameEvent& evt)
    {
        OgreProfileBeginGroup("Frame", OGREPROF_GENERAL);
        _syncAddedRemovedFrameListeners();

        // Tell all listeners
        for (auto *l : mFrameListeners)
        {
            if(mRemovedFrameListeners.find(l) != mRemovedFrameListeners.end())
                continue;

            if (!l->frameStarted(evt))
                return false;
        }

        return true;
    }
    //-----------------------------------------------------------------------
    bool Root::_fireFrameRenderingQueued(FrameEvent& evt)
    {
        // Increment next frame number
        ++mNextFrame;
        _syncAddedRemovedFrameListeners();

        // Tell all listeners
        for (auto *l : mFrameListeners)
        {
            if(mRemovedFrameListeners.find(l) != mRemovedFrameListeners.end())
                continue;

            if (!l->frameRenderingQueued(evt))
                return false;
        }

        return true;
    }
    //-----------------------------------------------------------------------
    bool Root::_fireFrameEnded(FrameEvent& evt)
    {
        _syncAddedRemovedFrameListeners();

        // Tell all listeners
        bool ret = true;
        for (auto *l : mFrameListeners)
        {
            if(mRemovedFrameListeners.find(l) != mRemovedFrameListeners.end())
                continue;

            if (!l->frameEnded(evt))
            {
                ret = false;
                break;
            }
        }

        // Tell buffer manager to free temp buffers used this frame
        if (HardwareBufferManager::getSingletonPtr())
            HardwareBufferManager::getSingleton()._releaseBufferCopies();

        // Tell the queue to process responses
        mWorkQueue->processMainThreadTasks();

        OgreProfileEndGroup("Frame", OGREPROF_GENERAL);

        return ret;
    }
    //-----------------------------------------------------------------------
    bool Root::_fireFrameStarted()
    {
        FrameEvent evt;
        populateFrameEvent(FETT_STARTED, evt);

        return _fireFrameStarted(evt);
    }
    //-----------------------------------------------------------------------
    bool Root::_fireFrameRenderingQueued()
    {
        FrameEvent evt;
        populateFrameEvent(FETT_QUEUED, evt);

        return _fireFrameRenderingQueued(evt);
    }
    //-----------------------------------------------------------------------
    bool Root::_fireFrameEnded()
    {
        FrameEvent evt;
        populateFrameEvent(FETT_ENDED, evt);
        return _fireFrameEnded(evt);
    }
    //---------------------------------------------------------------------
    void Root::populateFrameEvent(FrameEventTimeType type, FrameEvent& evtToUpdate)
    {
        unsigned long now = mTimer->getMilliseconds();
        evtToUpdate.timeSinceLastEvent = calculateEventTime(now, FETT_ANY);
        evtToUpdate.timeSinceLastFrame = calculateEventTime(now, type);
    }
    //-----------------------------------------------------------------------
    Real Root::calculateEventTime(unsigned long now, FrameEventTimeType type)
    {
        // Calculate the average time passed between events of the given type
        // during the last mFrameSmoothingTime seconds.

        EventTimesQueue& times = mEventTimes[type];
        times.push_back(now);

        if(times.size() == 1)
            return 0;

        // Times up to mFrameSmoothingTime seconds old should be kept
        unsigned long discardThreshold =
            static_cast<unsigned long>(mFrameSmoothingTime * 1000.0f);

        // Find the oldest time to keep
        EventTimesQueue::iterator it = times.begin(),
            end = times.end()-2; // We need at least two times
        while(it != end)
        {
            if (now - *it > discardThreshold)
                ++it;
            else
                break;
        }

        // Remove old times
        times.erase(times.begin(), it);

        return Real(times.back() - times.front()) / ((times.size()-1) * 1000);
    }
    //-----------------------------------------------------------------------
    void Root::queueEndRendering(bool state /* = true */)
    {
        mQueuedEnd = state;
    }
    //-----------------------------------------------------------------------
    bool Root::endRenderingQueued(void)
    {
        return mQueuedEnd;
    }
    //-----------------------------------------------------------------------
    void Root::startRendering(void)
    {
        OgreAssert(mActiveRenderer, "no RenderSystem");

        mActiveRenderer->_initRenderTargets();

        // Clear event times
        clearEventTimes();

        // Infinite loop, until broken out of by frame listeners
        // or break out by calling queueEndRendering()
        mQueuedEnd = false;

        while( !mQueuedEnd )
        {
            if (!renderOneFrame())
                break;
        }
    }
    //-----------------------------------------------------------------------
    bool Root::renderOneFrame(void)
    {
        if(!_fireFrameStarted())
            return false;

        if (!_updateAllRenderTargets())
            return false;

        return _fireFrameEnded();
    }
    //---------------------------------------------------------------------
    bool Root::renderOneFrame(Real timeSinceLastFrame)
    {
        FrameEvent evt;
        evt.timeSinceLastFrame = timeSinceLastFrame;

        unsigned long now = mTimer->getMilliseconds();
        evt.timeSinceLastEvent = calculateEventTime(now, FETT_ANY);

        if(!_fireFrameStarted(evt))
            return false;

        if (!_updateAllRenderTargets(evt))
            return false;

        now = mTimer->getMilliseconds();
        evt.timeSinceLastEvent = calculateEventTime(now, FETT_ANY);

        return _fireFrameEnded(evt);
    }
    //-----------------------------------------------------------------------
    void Root::shutdown(void)
    {
        if(mActiveRenderer)
            mActiveRenderer->_setViewport(NULL);

        // Since background thread might be access resources,
        // ensure shutdown before destroying resource manager.
        mWorkQueue->shutdown();

        if(mSceneManagerEnum)
            mSceneManagerEnum->shutdownAll();
        if(mFirstTimePostWindowInit)
        {
            shutdownPlugins();
            mParticleManager->removeAllTemplates(true);
            mFirstTimePostWindowInit = false;
        }
        mSceneManagerEnum.reset();
        mShadowTextureManager.reset();

        ShadowVolumeExtrudeProgram::shutdown();
        ResourceGroupManager::getSingleton().shutdownAll();

        // Destroy pools
        ConvexBody::_destroyPool();


        mIsInitialised = false;

        LogManager::getSingleton().logMessage("*-*-* OGRE Shutdown");
    }
    //-----------------------------------------------------------------------
    void Root::loadPlugins( const String& pluginsfile )
    {
        StringVector pluginList;
        String pluginDir;
        ConfigFile cfg;

        try {
            cfg.load( pluginsfile );
        }
        catch (Exception& e)
        {
            LogManager::getSingleton().logError(e.getDescription()+" - skipping automatic plugin loading");
            return;
        }

        pluginDir = cfg.getSetting("PluginFolder");
        pluginList = cfg.getMultiSetting("Plugin");

        StringUtil::trim(pluginDir);
        if(pluginDir.empty() || pluginDir[0] == '.')
        {
            // resolve relative path with regards to configfile
            String baseDir, filename;
            StringUtil::splitFilename(pluginsfile, filename, baseDir);
            pluginDir = baseDir + pluginDir;
        }

        if(char* val = getenv("OGRE_PLUGIN_DIR"))
        {
            pluginDir = val;
            LogManager::getSingleton().logMessage(
                "setting PluginFolder from OGRE_PLUGIN_DIR environment variable");
        }

        pluginDir = FileSystemLayer::resolveBundlePath(pluginDir);

        if (!pluginDir.empty() && *pluginDir.rbegin() != '/' && *pluginDir.rbegin() != '\\')
        {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT
            pluginDir += "\\";
#else
            pluginDir += "/";
#endif
        }

        for(auto& p : pluginList)
        {
            loadPlugin(pluginDir + p);
        }
    }
    //-----------------------------------------------------------------------
    void Root::shutdownPlugins(void)
    {
        // NB Shutdown plugins in reverse order to enforce dependencies
        for (PluginInstanceList::reverse_iterator i = mPlugins.rbegin(); i != mPlugins.rend(); ++i)
        {
            (*i)->shutdown();
        }
    }
    //-----------------------------------------------------------------------
    void Root::initialisePlugins(void)
    {
        for (auto *p : mPlugins)
        {
            p->initialise();
        }
    }
    //-----------------------------------------------------------------------
    void Root::unloadPlugins(void)
    {
#if OGRE_PLATFORM != OGRE_PLATFORM_EMSCRIPTEN
        // unload dynamic libs first
        for (PluginLibList::reverse_iterator i = mPluginLibs.rbegin(); i != mPluginLibs.rend(); ++i)
        {
            // Call plugin shutdown
            #ifdef __GNUC__
            __extension__
            #endif
            DLL_STOP_PLUGIN pFunc = reinterpret_cast<DLL_STOP_PLUGIN>((*i)->getSymbol("dllStopPlugin"));
            // this will call uninstallPlugin
            pFunc();
            // Unload library & destroy
            DynLibManager::getSingleton().unload(*i);

        }
        mPluginLibs.clear();

        // now deal with any remaining plugins that were registered through other means
        for (PluginInstanceList::reverse_iterator i = mPlugins.rbegin(); i != mPlugins.rend(); ++i)
        {
            // Note this does NOT call uninstallPlugin - this shutdown is for the
            // detail objects
            (*i)->uninstall();
        }
        mPlugins.clear();
#endif
    }
    //---------------------------------------------------------------------
    DataStreamPtr Root::createFileStream(const String& filename, const String& groupName,
        bool overwrite, const String& locationPattern)
    {
        // Does this file include path specifiers?
        String path, basename;
        StringUtil::splitFilename(filename, basename, path);

        // no path elements, try the resource system first
        DataStreamPtr stream;
        if (path.empty())
        {
            try
            {
                stream = ResourceGroupManager::getSingleton().createResource(
                    filename, groupName, overwrite, locationPattern);
            }
            catch (...) {}

        }

        if (!stream)
        {
            // save direct in filesystem
            stream = _openFileStream(filename, std::ios::out | std::ios::binary);
        }

        return stream;

    }
    //---------------------------------------------------------------------
    DataStreamPtr Root::openFileStream(const String& filename, const String& groupName)
    {
        DataStreamPtr ret;
        if(auto rgm = ResourceGroupManager::getSingletonPtr())
            ret = rgm->openResource(filename, groupName, NULL, false);

        if(ret)
            return ret;

        return _openFileStream(filename, std::ios::in | std::ios::binary);
    }
    //-----------------------------------------------------------------------
    RenderWindow* Root::getAutoCreatedWindow(void)
    {
        return mAutoWindow;
    }
    //-----------------------------------------------------------------------
    RenderWindow* Root::createRenderWindow(const String &name, unsigned int width, unsigned int height,
            bool fullScreen, const NameValuePairList *miscParams)
    {
        OgreAssert(mIsInitialised,
                   "Cannot create window! Make sure to call Root::initialise before creating a window");
        OgreAssert(mActiveRenderer, "Cannot create window");

        RenderWindow* ret;
        ret = mActiveRenderer->_createRenderWindow(name, width, height, fullScreen, miscParams);

        // Initialisation for classes dependent on first window created
        if(!mFirstTimePostWindowInit)
        {
            oneTimePostWindowInit();
            ret->_setPrimary();
        }

        return ret;

    }
    //-----------------------------------------------------------------------
    RenderTarget* Root::detachRenderTarget(RenderTarget* target)
    {
        OgreAssert(mActiveRenderer, "Cannot detach target");
        return mActiveRenderer->detachRenderTarget( target->getName() );
    }
    //-----------------------------------------------------------------------
    RenderTarget* Root::detachRenderTarget(const String &name)
    {
        OgreAssert(mActiveRenderer, "Cannot detach target");
        return mActiveRenderer->detachRenderTarget( name );
    }
    //-----------------------------------------------------------------------
    void Root::destroyRenderTarget(RenderTarget* target)
    {
        detachRenderTarget(target);
        OGRE_DELETE target;
    }
    //-----------------------------------------------------------------------
    void Root::destroyRenderTarget(const String &name)
    {
        RenderTarget* target = getRenderTarget(name);
        destroyRenderTarget(target);
    }
    //-----------------------------------------------------------------------
    RenderTarget* Root::getRenderTarget(const String &name)
    {
        OgreAssert(mActiveRenderer, "Cannot get target");
        return mActiveRenderer->getRenderTarget(name);
    }
    //---------------------------------------------------------------------
    void Root::installPlugin(Plugin* plugin)
    {
        LogManager::getSingleton().logMessage("Installing plugin: " + plugin->getName());

        mPlugins.push_back(plugin);
        plugin->install();

        // if rendersystem is already initialised, call rendersystem init too
        if (mIsInitialised)
        {
            plugin->initialise();
        }

        LogManager::getSingleton().logMessage("Plugin successfully installed");
    }
    //---------------------------------------------------------------------
    void Root::uninstallPlugin(Plugin* plugin)
    {
        LogManager::getSingleton().logMessage("Uninstalling plugin: " + plugin->getName());
        PluginInstanceList::iterator i =
            std::find(mPlugins.begin(), mPlugins.end(), plugin);
        if (i != mPlugins.end())
        {
            if (mIsInitialised)
                plugin->shutdown();
            plugin->uninstall();
            mPlugins.erase(i);
        }
        LogManager::getSingleton().logMessage("Plugin successfully uninstalled");

    }
    //-----------------------------------------------------------------------
    void Root::loadPlugin(const String& pluginName)
    {
#if OGRE_PLATFORM != OGRE_PLATFORM_EMSCRIPTEN
        // Load plugin library
        DynLib* lib = DynLibManager::getSingleton().load( pluginName );
        // Store for later unload
        // Check for existence, because if called 2+ times DynLibManager returns existing entry
        if (std::find(mPluginLibs.begin(), mPluginLibs.end(), lib) == mPluginLibs.end())
        {
            mPluginLibs.push_back(lib);

            // Call startup function
                        #ifdef __GNUC__
                        __extension__
                        #endif
            DLL_START_PLUGIN pFunc = (DLL_START_PLUGIN)lib->getSymbol("dllStartPlugin");

            if (!pFunc)
                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Cannot find symbol dllStartPlugin in library " + pluginName,
                    "Root::loadPlugin");

            // This must call installPlugin
            pFunc();
        }
#endif
    }
    //-----------------------------------------------------------------------
    void Root::unloadPlugin(const String& pluginName)
    {
#if OGRE_PLATFORM != OGRE_PLATFORM_EMSCRIPTEN
        PluginLibList::iterator i;

        for (i = mPluginLibs.begin(); i != mPluginLibs.end(); ++i)
        {
            if ((*i)->getName() == pluginName)
            {
                // Call plugin shutdown
                                #ifdef __GNUC__
                                __extension__
                                #endif
                DLL_STOP_PLUGIN pFunc = (DLL_STOP_PLUGIN)(*i)->getSymbol("dllStopPlugin");
                // this must call uninstallPlugin
                pFunc();
                // Unload library (destroyed by DynLibManager)
                DynLibManager::getSingleton().unload(*i);
                mPluginLibs.erase(i);
                return;
            }

        }
#endif
    }
    //-----------------------------------------------------------------------
    Timer* Root::getTimer(void)
    {
        return mTimer.get();
    }
    //-----------------------------------------------------------------------
    void Root::oneTimePostWindowInit(void)
    {
        // log RenderSystem caps
        mActiveRenderer->getCapabilities()->log(LogManager::getSingleton().getDefaultLog());

        // Background loader
        mWorkQueue->startup();
        // Initialise material manager
        mMaterialManager->initialise();
        // Init particle systems manager
        mParticleManager->_initialise();
        // Init mesh manager
        MeshManager::getSingleton()._initialise();
        // Init plugins - after window creation so rsys resources available
        initialisePlugins();
        mFirstTimePostWindowInit = true;
    }
    //-----------------------------------------------------------------------
    bool Root::_updateAllRenderTargets(void)
    {
        // update all targets but don't swap buffers
        mActiveRenderer->_updateAllRenderTargets(false);
        // give client app opportunity to use queued GPU time
        bool ret = _fireFrameRenderingQueued();
        // block for final swap
        mActiveRenderer->_swapAllRenderTargetBuffers();

        // This belongs here, as all render targets must be updated before events are
        // triggered, otherwise targets could be mismatched.  This could produce artifacts,
        // for instance, with shadows.
        for (const auto& m : getSceneManagers())
            m.second->_handleLodEvents();

        return ret;
    }
    //---------------------------------------------------------------------
    bool Root::_updateAllRenderTargets(FrameEvent& evt)
    {
        // update all targets but don't swap buffers
        mActiveRenderer->_updateAllRenderTargets(false);
        // give client app opportunity to use queued GPU time
        bool ret = _fireFrameRenderingQueued(evt);
        // block for final swap
        mActiveRenderer->_swapAllRenderTargetBuffers();

        // This belongs here, as all render targets must be updated before events are
        // triggered, otherwise targets could be mismatched.  This could produce artifacts,
        // for instance, with shadows.
        for (const auto& m : getSceneManagers())
            m.second->_handleLodEvents();

        return ret;
    }
    //-----------------------------------------------------------------------
    void Root::clearEventTimes(void)
    {
        // Clear event times
        for(int i=0; i<FETT_COUNT; ++i)
            mEventTimes[i].clear();
    }
    //---------------------------------------------------------------------
    void Root::addMovableObjectFactory(MovableObjectFactory* fact,
        bool overrideExisting)
    {
        MovableObjectFactoryMap::iterator facti = mMovableObjectFactoryMap.find(
            fact->getType());
        if (!overrideExisting && facti != mMovableObjectFactoryMap.end())
        {
            OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM,
                "A factory of type '" + fact->getType() + "' already exists.",
                "Root::addMovableObjectFactory");
        }

        if (fact->requestTypeFlags())
        {
            if (facti != mMovableObjectFactoryMap.end() && facti->second->requestTypeFlags())
            {
                // Copy type flags from the factory we're replacing
                fact->_notifyTypeFlags(facti->second->getTypeFlags());
            }
            else
            {
                // Allocate new
                fact->_notifyTypeFlags(_allocateNextMovableObjectTypeFlag());
            }
        }

        // Save
        mMovableObjectFactoryMap[fact->getType()] = fact;

        LogManager::getSingleton().logMessage("MovableObjectFactory for type '" +
            fact->getType() + "' registered.");

    }
    //---------------------------------------------------------------------
    bool Root::hasMovableObjectFactory(const String& typeName) const
    {
        return !(mMovableObjectFactoryMap.find(typeName) == mMovableObjectFactoryMap.end());
    }
    //---------------------------------------------------------------------
    MovableObjectFactory* Root::getMovableObjectFactory(const String& typeName)
    {
        MovableObjectFactoryMap::iterator i =
            mMovableObjectFactoryMap.find(typeName);
        if (i == mMovableObjectFactoryMap.end())
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
                "MovableObjectFactory of type " + typeName + " does not exist",
                "Root::getMovableObjectFactory");
        }
        return i->second;
    }
    //---------------------------------------------------------------------
    uint32 Root::_allocateNextMovableObjectTypeFlag(void)
    {
        if (mNextMovableObjectTypeFlag == SceneManager::USER_TYPE_MASK_LIMIT)
        {
            OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM,
                "Cannot allocate a type flag since "
                "all the available flags have been used.",
                "Root::_allocateNextMovableObjectTypeFlag");

        }
        uint32 ret = mNextMovableObjectTypeFlag;
        mNextMovableObjectTypeFlag <<= 1;
        return ret;

    }
    //---------------------------------------------------------------------
    void Root::removeMovableObjectFactory(MovableObjectFactory* fact)
    {
        MovableObjectFactoryMap::iterator i = mMovableObjectFactoryMap.find(
            fact->getType());
        if (i != mMovableObjectFactoryMap.end())
        {
            mMovableObjectFactoryMap.erase(i);
        }

    }
    //---------------------------------------------------------------------
    Root::MovableObjectFactoryIterator
    Root::getMovableObjectFactoryIterator(void) const
    {
        return MovableObjectFactoryIterator(mMovableObjectFactoryMap.begin(),
            mMovableObjectFactoryMap.end());

    }
    //---------------------------------------------------------------------
    void Root::setWorkQueue(WorkQueue* queue)
    {
        if (mWorkQueue.get() != queue)
        {
            mWorkQueue.reset(queue);
            if (mIsInitialised)
                mWorkQueue->startup();

        }
    }
}
