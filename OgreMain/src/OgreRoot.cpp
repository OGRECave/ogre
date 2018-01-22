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
#include "OgreStableHeaders.h"

#include "OgreRoot.h"

#include "OgreRenderSystem.h"
#include "OgreRenderWindow.h"
#include "OgreException.h"
#include "OgreControllerManager.h"
#include "OgreLogManager.h"
#include "OgreDynLibManager.h"
#include "OgreDynLib.h"
#include "OgreConfigFile.h"
#include "OgreMaterialManager.h"
#include "OgreRenderSystemCapabilitiesManager.h"
#include "OgreMeshManager.h"
#include "OgreMeshManager2.h"
#include "OgreTextureManager.h"
#include "OgreParticleSystemManager.h"
#include "OgreOldSkeletonManager.h"
#include "OgreProfiler.h"
#include "OgreConfigDialog.h"
#include "OgreArchiveManager.h"
#include "OgrePlugin.h"
#include "OgreFileSystem.h"
#include "OgreResourceBackgroundQueue.h"
#include "OgreEntity.h"
#include "OgreItem.h"
#include "OgreBillboardSet.h"
#include "OgreBillboardChain.h"
#include "OgreRibbonTrail.h"
#include "OgreLight.h"
#include "OgreManualObject.h"
#include "OgrePlatformInformation.h"
#include "OgreConvexBody.h"
#include "OgreFrameStats.h"
#include "OgreTimer.h"
#include "OgreLodStrategyManager.h"
#include "Threading/OgreDefaultWorkQueue.h"
#include "OgreFrameListener.h"
#include "OgreWireAabb.h"
#include "OgreNameGenerator.h"
#include "OgreHlmsManager.h"
#include "OgreHlmsCompute.h"
#include "OgreHlmsLowLevel.h"
#include "Animation/OgreSkeletonManager.h"
#include "Compositor/OgreCompositorManager2.h"

#if OGRE_NO_FREEIMAGE == 0
#include "OgreFreeImageCodec.h"
#endif
#if OGRE_NO_DDS_CODEC == 0
#include "OgreDDSCodec.h"
#endif
#if OGRE_NO_STBI_CODEC == 0
#include "OgreSTBICodec.h"
#endif
#if OGRE_NO_ZIP_ARCHIVE == 0
#include "OgreZip.h"
#endif
#include "OgreOITDCodec.h"

#include "OgreLwString.h"

#include "OgreHardwareBufferManager.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreExternalTextureSourceManager.h"
#include "OgreScriptCompiler.h"
#include "OgreWindowEventUtilities.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
#include "macUtils.h"
#endif
#if OGRE_NO_PVRTC_CODEC == 0
#  include "OgrePVRTCCodec.h"
#endif
#if OGRE_NO_ETC_CODEC == 0
#  include "OgreETCCodec.h"
#endif
#if OGRE_NO_ASTC_CODEC == 0
#  include "OgreASTCCodec.h"
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

#if OGRE_PLATFORM != OGRE_PLATFORM_NACL
    typedef void (*DLL_START_PLUGIN)(void);
    typedef void (*DLL_STOP_PLUGIN)(void);
#endif

    //-----------------------------------------------------------------------
    Root::Root(const String& pluginFileName, const String& configFileName,
        const String& logFileName)
      : mQueuedEnd(false)
      , mLogManager(0)
      , mRenderSystemCapabilitiesManager(0)
      , mFrameStats(0)
      , mCompositorManager2(0)
      , mNextFrame(0)
      , mFrameSmoothingTime(0.0f)
      , mRemoveQueueStructuresOnClear(false)
      , mDefaultMinPixelSize(0)
      , mFreqUpdatedBuffersUploadOption(v1::HardwareBuffer::HBU_DEFAULT)
      , mNextMovableObjectTypeFlag(1)
      , mIsInitialised(false)
      , mIsBlendIndicesGpuRedundant(true)
      , mIsBlendWeightsGpuRedundant(true)
    {
        // superclass will do singleton checking
        String msg;

        // Init
        mActiveRenderer = 0;
        mVersion = StringConverter::toString(OGRE_VERSION_MAJOR) + "." +
            StringConverter::toString(OGRE_VERSION_MINOR) + "." +
            StringConverter::toString(OGRE_VERSION_PATCH) +
            OGRE_VERSION_SUFFIX + " " +
            "(" + OGRE_VERSION_NAME + ")";
        mConfigFileName = configFileName;

        // Create log manager and default log file if there is no log manager yet
        if(LogManager::getSingletonPtr() == 0)
        {
            mLogManager = OGRE_NEW LogManager();
            mLogManager->createLog(logFileName, true, true);
        }

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
        mAndroidLogger = OGRE_NEW AndroidLogListener();
        mLogManager->getDefaultLog()->addListener(mAndroidLogger);
#endif

        // Dynamic library manager
        mDynLibManager = OGRE_NEW DynLibManager();

        mArchiveManager = OGRE_NEW ArchiveManager();

        // ResourceGroupManager
        mResourceGroupManager = OGRE_NEW ResourceGroupManager();

        // WorkQueue (note: users can replace this if they want)
        DefaultWorkQueue* defaultQ = OGRE_NEW DefaultWorkQueue("Root");
        // never process responses in main thread for longer than 10ms by default
        defaultQ->setResponseProcessingTimeLimit(10);
        // match threads to hardware
#if OGRE_THREAD_SUPPORT
        unsigned threadCount = OGRE_THREAD_HARDWARE_CONCURRENCY;
        if (!threadCount)
            threadCount = 1;
        defaultQ->setWorkerThreadCount(threadCount);
#endif
        // only allow workers to access rendersystem if threadsupport is 1
#if OGRE_THREAD_SUPPORT == 1
        defaultQ->setWorkersCanAccessRenderSystem(true);
#else
        defaultQ->setWorkersCanAccessRenderSystem(false);
#endif
        mWorkQueue = defaultQ;

        // ResourceBackgroundQueue
        mResourceBackgroundQueue = OGRE_NEW ResourceBackgroundQueue();

        // Create SceneManager enumerator (note - will be managed by singleton)
        mSceneManagerEnum = OGRE_NEW SceneManagerEnumerator();

        mRenderSystemCapabilitiesManager = OGRE_NEW RenderSystemCapabilitiesManager();

        // ..material manager
        mMaterialManager = OGRE_NEW MaterialManager();

        // Mesh manager
        mMeshManagerV1 = OGRE_NEW v1::MeshManager();

        mMeshManager = OGRE_NEW MeshManager();

        // Skeleton manager
        mOldSkeletonManager = OGRE_NEW v1::OldSkeletonManager();
        mSkeletonManager    = OGRE_NEW SkeletonManager();

        // ..particle system manager
        mParticleManager = OGRE_NEW ParticleSystemManager();

        mFrameStats = OGRE_NEW FrameStats();

        mTimer = OGRE_NEW Timer();

        // LOD strategy manager
        mLodStrategyManager = OGRE_NEW LodStrategyManager();

#if OGRE_PROFILING
        // Profiler
        mProfiler = OGRE_NEW Profiler();
        Profiler::getSingleton().setTimer(mTimer);
#endif


        mFileSystemArchiveFactory = OGRE_NEW FileSystemArchiveFactory();
        ArchiveManager::getSingleton().addArchiveFactory( mFileSystemArchiveFactory );
#   if OGRE_NO_ZIP_ARCHIVE == 0
        mZipArchiveFactory = OGRE_NEW ZipArchiveFactory();
        ArchiveManager::getSingleton().addArchiveFactory( mZipArchiveFactory );
        mEmbeddedZipArchiveFactory = OGRE_NEW EmbeddedZipArchiveFactory();
        ArchiveManager::getSingleton().addArchiveFactory( mEmbeddedZipArchiveFactory );
#   endif

#if OGRE_NO_DDS_CODEC == 0
        // Register image codecs
        DDSCodec::startup();
#endif
#if OGRE_NO_FREEIMAGE == 0
        // Register image codecs
        FreeImageCodec::startup();
#endif
#if OGRE_NO_PVRTC_CODEC == 0
        PVRTCCodec::startup();
#endif
#if OGRE_NO_ETC_CODEC == 0
        ETCCodec::startup();
#endif
#if OGRE_NO_STBI_CODEC == 0
        STBIImageCodec::startup();
#endif
#if OGRE_NO_ASTC_CODEC == 0
        ASTCCodec::startup();
#endif
        OITDCodec::startup();

        mHighLevelGpuProgramManager = OGRE_NEW HighLevelGpuProgramManager();

        mExternalTextureSourceManager = OGRE_NEW ExternalTextureSourceManager();

        mHlmsManager        = OGRE_NEW HlmsManager();
        mHlmsLowLevelProxy  = OGRE_NEW HlmsLowLevel();
        mHlmsCompute        = OGRE_NEW HlmsCompute( mHlmsLowLevelProxy->_getAutoParamDataSource() );

        mCompilerManager = OGRE_NEW ScriptCompilerManager();

        // Auto window
        mAutoWindow = 0;

        // instantiate and register base movable factories
        mEntityFactory = OGRE_NEW v1::EntityFactory();
        addMovableObjectFactory(mEntityFactory);
        mItemFactory = OGRE_NEW ItemFactory();
        addMovableObjectFactory(mItemFactory);
        mLightFactory = OGRE_NEW LightFactory();
        addMovableObjectFactory(mLightFactory);
        mBillboardSetFactory = OGRE_NEW v1::BillboardSetFactory();
        addMovableObjectFactory(mBillboardSetFactory);
        mManualObjectFactory = OGRE_NEW ManualObjectFactory();
        addMovableObjectFactory(mManualObjectFactory);
        mBillboardChainFactory = OGRE_NEW v1::BillboardChainFactory();
        addMovableObjectFactory(mBillboardChainFactory);
        mRibbonTrailFactory = OGRE_NEW v1::RibbonTrailFactory();
        addMovableObjectFactory(mRibbonTrailFactory);
        mWireAabbFactory = OGRE_NEW WireAabbFactory();
        addMovableObjectFactory(mWireAabbFactory);

        // Load plugins
        if (!pluginFileName.empty())
            loadPlugins(pluginFileName);

        LogManager::getSingleton().logMessage("*-*-* OGRE Initialising");
        msg = "*-*-* Version " + mVersion;
        LogManager::getSingleton().logMessage(msg);

        // Can't create managers until initialised
        mControllerManager = 0;

        mFirstTimePostWindowInit = false;

    }

    //-----------------------------------------------------------------------
    Root::~Root()
    {
        LogManager::getSingleton().stream(LML_TRIVIAL)
            << "Average FPS: " << mFrameStats->getAvgFps() << "\n"
            << "Average time: \t"<< mFrameStats->getAvgTime() << " ms\n"
            << "Best time: \t"  << mFrameStats->getBestTime() << " ms\n"
            << "Worst time: \t" << mFrameStats->getWorstTime()<< " ms";

#if OGRE_PROFILING
        OGRE_DELETE mProfiler;
#endif
        shutdown();

        OGRE_DELETE mSceneManagerEnum;
        OGRE_DELETE mRenderSystemCapabilitiesManager;

        OGRE_DELETE mExternalTextureSourceManager;

        OITDCodec::shutdown();
#if OGRE_NO_FREEIMAGE == 0
        FreeImageCodec::shutdown();
#endif
#if OGRE_NO_DDS_CODEC == 0
        DDSCodec::shutdown();
#endif
#if OGRE_NO_PVRTC_CODEC == 0
        PVRTCCodec::shutdown();
#endif
#if OGRE_NO_ETC_CODEC == 0
        ETCCodec::shutdown();
#endif
#if OGRE_NO_STBI_CODEC == 0
        STBIImageCodec::shutdown();
#endif
#if OGRE_NO_ASTC_CODEC == 0
        ASTCCodec::shutdown();
#endif

        OGRE_DELETE mLodStrategyManager;

        OGRE_DELETE mArchiveManager;

#   if OGRE_NO_ZIP_ARCHIVE == 0
        OGRE_DELETE mZipArchiveFactory;
        OGRE_DELETE mEmbeddedZipArchiveFactory;
#   endif
        OGRE_DELETE mFileSystemArchiveFactory;

        OGRE_DELETE mOldSkeletonManager;
        OGRE_DELETE mSkeletonManager;
        OGRE_DELETE mMeshManager;
        OGRE_DELETE mMeshManagerV1;
        OGRE_DELETE mParticleManager;

        OGRE_DELETE mMaterialManager;
        OGRE_DELETE mHlmsCompute;
        mHlmsCompute = 0;
        OGRE_DELETE mHlmsLowLevelProxy;
        mHlmsLowLevelProxy = 0;
        mHlmsManager->_changeRenderSystem((RenderSystem*)0);
        OGRE_DELETE mHlmsManager;
        mHlmsManager = 0;

        OGRE_DELETE mControllerManager;
        OGRE_DELETE mHighLevelGpuProgramManager;

        unloadPlugins();

        OGRE_DELETE mResourceBackgroundQueue;
        OGRE_DELETE mResourceGroupManager;

        OGRE_DELETE mEntityFactory;
        OGRE_DELETE mItemFactory;
        OGRE_DELETE mLightFactory;
        OGRE_DELETE mBillboardSetFactory;
        OGRE_DELETE mManualObjectFactory;
        OGRE_DELETE mBillboardChainFactory;
        OGRE_DELETE mRibbonTrailFactory;
        OGRE_DELETE mWireAabbFactory;

        OGRE_DELETE mWorkQueue;

        OGRE_DELETE mFrameStats;

        OGRE_DELETE mTimer;

        OGRE_DELETE mDynLibManager;

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
        mLogManager->getDefaultLog()->removeListener(mAndroidLogger);
        OGRE_DELETE mAndroidLogger;
#endif

        OGRE_DELETE mLogManager;

        OGRE_DELETE mCompilerManager;

        mAutoWindow = 0;
        mFirstTimePostWindowInit = false;


        StringInterface::cleanupDictionary ();
    }

    //-----------------------------------------------------------------------
    void Root::saveConfig(void)
    {
#if OGRE_PLATFORM == OGRE_PLATFORM_NACL || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
        OGRE_EXCEPT(Exception::ERR_CANNOT_WRITE_TO_FILE, "saveConfig is not supported on NaCl",
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

        for (RenderSystemList::const_iterator pRend = getAvailableRenderers().begin(); pRend != getAvailableRenderers().end(); ++pRend)
        {
            RenderSystem* rs = *pRend;
            of << std::endl;
            of << "[" << rs->getName() << "]" << std::endl;
            const ConfigOptionMap& opts = rs->getConfigOptions();
            for (ConfigOptionMap::const_iterator pOpt = opts.begin(); pOpt != opts.end(); ++pOpt)
            {
                of << pOpt->first << "=" << pOpt->second.currentValue << std::endl;
            }
        }

        of.close();

    }
    //-----------------------------------------------------------------------
    bool Root::restoreConfig(void)
    {
#if OGRE_PLATFORM == OGRE_PLATFORM_NACL || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
        OGRE_EXCEPT(Exception::ERR_CANNOT_WRITE_TO_FILE, "restoreConfig is not supported on NaCl",
            "Root::restoreConfig");
#endif

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
        catch (Exception& e)
        {
            if (e.getNumber() == Exception::ERR_FILE_NOT_FOUND)
            {
                return false;
            }
            else
            {
                throw;
            }
        }

        ConfigFile::SectionIterator iSection = cfg.getSectionIterator();
        while (iSection.hasMoreElements())
        {
            const String& renderSystem = iSection.peekNextKey();
            const ConfigFile::SettingsMultiMap& settings = *iSection.getNext();

            RenderSystem* rs = getRenderSystemByName(renderSystem);
            if (!rs)
            {
                // Unrecognised render system
                continue;
            }

            try
            {
                ConfigFile::SettingsMultiMap::const_iterator i;
                for (i = settings.begin(); i != settings.end(); ++i)
                {
                    rs->setConfigOption(i->first, i->second);
                }
            }
            catch( Exception &e )
            {
                LogManager::getSingleton().logMessage( e.getFullDescription() );
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
        return true;

    }

    //-----------------------------------------------------------------------
    bool Root::showConfigDialog( ConfigDialog* aCustomDialog /*= 0*/ )
    {
#if OGRE_PLATFORM == OGRE_PLATFORM_NACL || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
        OGRE_EXCEPT(Exception::ERR_CANNOT_WRITE_TO_FILE, "showConfigDialog is not supported on NaCl",
            "Root::showConfigDialog");
#endif

        // Displays the standard config dialog
        // Will use stored defaults if available
        ConfigDialog* dlg = aCustomDialog;
        bool isOk;

        restoreConfig();

        if( !dlg )
            dlg = OGRE_NEW ConfigDialog();
        isOk = dlg->display();
        if (isOk)
            saveConfig();
        if( !aCustomDialog )
            OGRE_DELETE dlg;
        return isOk;
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

        RenderSystemList::const_iterator pRend;
        for (pRend = getAvailableRenderers().begin(); pRend != getAvailableRenderers().end(); ++pRend)
        {
            RenderSystem* rs = *pRend;
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

            OGRE_DELETE mCompositorManager2;
        }

        mActiveRenderer = system;

        mHlmsManager->_changeRenderSystem( mActiveRenderer );

        // Tell scene managers
        SceneManagerEnumerator::getSingleton().setRenderSystem(system);

    }
    //-----------------------------------------------------------------------
    void Root::addRenderSystem(RenderSystem *newRend)
    {
        mRenderers.push_back(newRend);
    }
    //-----------------------------------------------------------------------
    SceneManager* Root::_getCurrentSceneManager(void) const
    {
        if (mSceneManagerStack.empty())
            return 0;
        else
            return mSceneManagerStack.back();
    }
    //-----------------------------------------------------------------------
    void Root::_pushCurrentSceneManager(SceneManager* sm)
    {
        mSceneManagerStack.push_back(sm);
    }
    //-----------------------------------------------------------------------
    void Root::_popCurrentSceneManager(SceneManager* sm)
    {
        assert (_getCurrentSceneManager() == sm && "Mismatched push/pop of SceneManager");

        mSceneManagerStack.pop_back();
    }
    //-----------------------------------------------------------------------
    RenderSystem* Root::getRenderSystem(void)
    {
        // Gets the currently active renderer
        return mActiveRenderer;

    }

    //-----------------------------------------------------------------------
    RenderWindow* Root::initialise(bool autoCreateWindow, const String& windowTitle, const String& customCapabilitiesConfig)
    {
        if (!mActiveRenderer)
            OGRE_EXCEPT(Exception::ERR_INVALID_STATE,
            "Cannot initialise - no render "
            "system has been selected.", "Root::initialise");

        if (!mControllerManager)
            mControllerManager = OGRE_NEW ControllerManager();

        // .rendercaps manager
        RenderSystemCapabilitiesManager& rscManager = RenderSystemCapabilitiesManager::getSingleton();
        // caller wants to load custom RenderSystemCapabilities form a config file
        if(customCapabilitiesConfig != BLANKSTRING)
        {
            ConfigFile cfg;
            cfg.load(customCapabilitiesConfig, "\t:=", false);

            // Capabilities Database setting must be in the same format as
            // resources.cfg in Ogre examples.
            ConfigFile::SettingsIterator iter = cfg.getSettingsIterator("Capabilities Database");
            while(iter.hasMoreElements())
            {
                String archType = iter.peekNextKey();
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
                String filename = iter.getNext();

                // Only adjust relative directories
                if (!StringUtil::startsWith(filename, "/", false))
                {
                    filename = StringUtil::replaceAll(filename, "../", "");
                    filename = String(macBundlePath() + "/Contents/Resources/" + filename);
                }
#else
                String filename = iter.getNext();
#endif

                rscManager.parseCapabilitiesFromArchive(filename, archType, true);
            }

            String capsName = cfg.getSetting("Custom Capabilities");
            // The custom capabilities have been parsed, let's retrieve them
            RenderSystemCapabilities* rsc = rscManager.loadParsedCapabilities(capsName);
            if(rsc == 0)
            {
                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
                    String("Cannot load a RenderSystemCapability named ") + capsName,
                    "Root::initialise");
            }

            // Tell RenderSystem to use the comon rsc
            useCustomRenderSystemCapabilities(rsc);
        }


        PlatformInformation::log(LogManager::getSingleton().getDefaultLog());
        mAutoWindow =  mActiveRenderer->_initialise(autoCreateWindow, windowTitle);


        if (autoCreateWindow && !mFirstTimePostWindowInit)
        {
            oneTimePostWindowInit();
            mAutoWindow->_setPrimary();
        }

        // Initialise timer
        mTimer->reset();
        mFrameStats->reset( mTimer->getMicroseconds() );

        // Init pools
        ConvexBody::_initialisePool();

        mIsInitialised = true;

        return mAutoWindow;

    }
    //-----------------------------------------------------------------------
    void Root::useCustomRenderSystemCapabilities(RenderSystemCapabilities* capabilities)
    {
        mActiveRenderer->useCustomRenderSystemCapabilities(capabilities);
    }
    //-----------------------------------------------------------------------
    String Root::getErrorDescription(long errorNumber)
    {

        // Pass to render system
        if (mActiveRenderer)
            return mActiveRenderer->getErrorDescription(errorNumber);
        else
            return "";

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
    const SceneManagerMetaData* Root::getSceneManagerMetaData(const String& typeName) const
    {
        return mSceneManagerEnum->getMetaData(typeName);
    }
    //-----------------------------------------------------------------------
    SceneManagerEnumerator::MetaDataIterator
    Root::getSceneManagerMetaDataIterator(void) const
    {
        return mSceneManagerEnum->getMetaDataIterator();

    }
    //-----------------------------------------------------------------------
    SceneManager* Root::createSceneManager(const String& typeName, size_t numWorkerThreads,
        InstancingThreadedCullingMethod threadedCullingMethod, const String& instanceName)
    {
        return mSceneManagerEnum->createSceneManager(typeName, numWorkerThreads,
                                                     threadedCullingMethod, instanceName);
    }
    //-----------------------------------------------------------------------
    SceneManager* Root::createSceneManager(SceneTypeMask typeMask, size_t numWorkerThreads,
        InstancingThreadedCullingMethod threadedCullingMethod, const String& instanceName)
    {
        return mSceneManagerEnum->createSceneManager(typeMask, numWorkerThreads,
                                                     threadedCullingMethod, instanceName);
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
    SceneManagerEnumerator::SceneManagerIterator Root::getSceneManagerIterator(void)
    {
        return mSceneManagerEnum->getSceneManagerIterator();
    }
    //-----------------------------------------------------------------------
    TextureManager* Root::getTextureManager(void)
    {
        return &TextureManager::getSingleton();
    }
    //-----------------------------------------------------------------------
    v1::MeshManager* Root::getMeshManagerV1(void)
    {
        return &v1::MeshManager::getSingleton();
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
        for (set<FrameListener*>::type::iterator i = mRemovedFrameListeners.begin(); i != mRemovedFrameListeners.end(); ++i)
            mFrameListeners.erase(*i);
        mRemovedFrameListeners.clear();

        for (set<FrameListener*>::type::iterator i = mAddedFrameListeners.begin(); i != mAddedFrameListeners.end(); ++i)
            mFrameListeners.insert(*i);
        mAddedFrameListeners.clear();
    }
    //-----------------------------------------------------------------------
    bool Root::_fireFrameStarted(FrameEvent& evt)
    {
#if OGRE_PROFILING
        if( OgreProfilerUseStableMarkers )
        {
            OgreProfileBeginGroup( "Frame", OGREPROF_GENERAL );
            OgreProfileGpuBegin( "Frame" );
        }
        else
        {
            uint32 hashValue = (uint32)mNextFrame;
            char tmpBuffer[32];
            LwString frameNum( LwString::FromEmptyPointer( tmpBuffer, sizeof(tmpBuffer) ) );
            frameNum.a( "Frame ", hashValue );
            OgreProfileBeginDynamicHashed( frameNum.c_str(), &hashValue );
            OgreProfileGpuBeginDynamicHashed( frameNum.c_str(), &hashValue );
        }
#endif
        _syncAddedRemovedFrameListeners();

        // Tell all listeners
        for (set<FrameListener*>::type::iterator i = mFrameListeners.begin(); i != mFrameListeners.end(); ++i)
        {
            if (!(*i)->frameStarted(evt))
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
        for (set<FrameListener*>::type::iterator i = mFrameListeners.begin(); i != mFrameListeners.end(); ++i)
        {
            if (!(*i)->frameRenderingQueued(evt))
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
        for (set<FrameListener*>::type::iterator i = mFrameListeners.begin(); i != mFrameListeners.end(); ++i)
        {
            if (!(*i)->frameEnded(evt))
            {
                ret = false;
                break;
            }
        }

        // Tell buffer manager to free temp buffers used this frame
        if (v1::HardwareBufferManager::getSingletonPtr())
            v1::HardwareBufferManager::getSingleton()._releaseBufferCopies();

        // Tell the queue to process responses
        mWorkQueue->processResponses();

#if OGRE_PROFILING
        if( OgreProfilerUseStableMarkers )
        {
            OgreProfileGpuEnd( "Frame" );
            OgreProfileEndGroup( "Frame", OGREPROF_GENERAL );
        }
        else
        {
            char tmpBuffer[32];
            LwString frameNum( LwString::FromEmptyPointer( tmpBuffer, sizeof(tmpBuffer) ) );
            frameNum.a( "Frame ", (uint32)(mNextFrame - 1u) );

            OgreProfileGpuEnd( frameNum.c_str() );
            OgreProfileEndGroup( frameNum.c_str(), OGREPROF_GENERAL );
        }
#endif

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
        assert(mActiveRenderer != 0);

        mActiveRenderer->_initRenderTargets();

        // Clear event times
        clearEventTimes();

        // Infinite loop, until broken out of by frame listeners
        // or break out by calling queueEndRendering()
        mQueuedEnd = false;

        while( !mQueuedEnd )
        {
            //Pump messages in all registered RenderWindow windows
            WindowEventUtilities::messagePump();

            if (!renderOneFrame())
                break;
        }
    }
    //-----------------------------------------------------------------------
    bool Root::renderOneFrame(void)
    {
        if(!_fireFrameStarted())
            return false;

        SceneManagerEnumerator::SceneManagerIterator itor = mSceneManagerEnum->getSceneManagerIterator();
        while( itor.hasMoreElements() )
        {
            SceneManager *sceneManager = itor.getNext();
            sceneManager->updateSceneGraph();
        }

        if (!_updateAllRenderTargets())
            return false;

        itor = mSceneManagerEnum->getSceneManagerIterator();
        while( itor.hasMoreElements() )
        {
            SceneManager *sceneManager = itor.getNext();
            sceneManager->clearFrameData();
        }

        mFrameStats->addSample( mTimer->getMicroseconds() );

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

        SceneManagerEnumerator::SceneManagerIterator itor = mSceneManagerEnum->getSceneManagerIterator();
        while( itor.hasMoreElements() )
        {
            SceneManager *sceneManager = itor.getNext();
            sceneManager->updateSceneGraph();
        }

        if (!_updateAllRenderTargets(evt))
            return false;

        itor = mSceneManagerEnum->getSceneManagerIterator();
        while( itor.hasMoreElements() )
        {
            SceneManager *sceneManager = itor.getNext();
            sceneManager->clearFrameData();
        }

        now = mTimer->getMicroseconds();
        mFrameStats->addSample( now );
        now /= 1000; // Convert to milliseconds.
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
        mResourceBackgroundQueue->shutdown();
        mWorkQueue->shutdown();

		OGRE_DELETE mCompositorManager2;
        mCompositorManager2 = 0;

        SceneManagerEnumerator::getSingleton().shutdownAll();
        shutdownPlugins();

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
        catch (Exception)
        {
            LogManager::getSingleton().logMessage(pluginsfile + " not found, automatic plugin loading disabled.");
            return;
        }

        pluginDir = cfg.getSetting("PluginFolder"); // Ignored on Mac OS X, uses Resources/ directory
        pluginList = cfg.getMultiSetting("Plugin");

        if (!pluginDir.empty() && *pluginDir.rbegin() != '/' && *pluginDir.rbegin() != '\\')
        {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT
            pluginDir += "\\";
#elif OGRE_PLATFORM == OGRE_PLATFORM_LINUX
            pluginDir += "/";
#endif
        }

        for( StringVector::iterator it = pluginList.begin(); it != pluginList.end(); ++it )
        {
            loadPlugin(pluginDir + (*it));
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
        for (PluginInstanceList::iterator i = mPlugins.begin(); i != mPlugins.end(); ++i)
        {
            (*i)->initialise();
        }
    }
    //-----------------------------------------------------------------------
    void Root::unloadPlugins(void)
    {
#if OGRE_PLATFORM != OGRE_PLATFORM_NACL && OGRE_PLATFORM != OGRE_PLATFORM_EMSCRIPTEN
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
    //-----------------------------------------------------------------------
    void Root::addResourceLocation(const String& name, const String& locType,
        const String& groupName, bool recursive)
    {
        ResourceGroupManager::getSingleton().addResourceLocation(
            name, locType, groupName, recursive);
    }
    //-----------------------------------------------------------------------
    void Root::removeResourceLocation(const String& name, const String& groupName)
    {
        ResourceGroupManager::getSingleton().removeResourceLocation(
            name, groupName);
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

        if (stream.isNull())
        {
            // save direct in filesystem
            std::fstream* fs = OGRE_NEW_T(std::fstream, MEMCATEGORY_GENERAL);
            fs->open(filename.c_str(), std::ios::out | std::ios::binary);
            if (!*fs)
            {
                OGRE_DELETE_T(fs, basic_fstream, MEMCATEGORY_GENERAL);
                OGRE_EXCEPT(Exception::ERR_CANNOT_WRITE_TO_FILE,
                "Can't open " + filename + " for writing", __FUNCTION__);
            }

            stream = DataStreamPtr(OGRE_NEW FileStreamDataStream(filename, fs));
        }

        return stream;

    }
    //---------------------------------------------------------------------
    DataStreamPtr Root::openFileStream(const String& filename, const String& groupName,
        const String& locationPattern)
    {
        DataStreamPtr stream;
        if (ResourceGroupManager::getSingleton().resourceExists(
            groupName, filename))
        {
            stream = ResourceGroupManager::getSingleton().openResource(
                filename, groupName);
        }
        else
        {
            // try direct
            std::ifstream *ifs = OGRE_NEW_T(std::ifstream, MEMCATEGORY_GENERAL);
            ifs->open(filename.c_str(), std::ios::in | std::ios::binary);
            if(!*ifs)
            {
                OGRE_DELETE_T(ifs, basic_ifstream, MEMCATEGORY_GENERAL);
                OGRE_EXCEPT(
                    Exception::ERR_FILE_NOT_FOUND, "'" + filename + "' file not found!", __FUNCTION__);
            }
            stream.bind(OGRE_NEW FileStreamDataStream(filename, ifs));
        }
        return stream;
    }
    //-----------------------------------------------------------------------
    void Root::convertColourValue(const ColourValue& colour, uint32* pDest)
    {
        assert(mActiveRenderer != 0);
        mActiveRenderer->convertColourValue(colour, pDest);
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
        if (!mIsInitialised)
        {
            OGRE_EXCEPT(Exception::ERR_INVALID_STATE,
            "Cannot create window - Root has not been initialised! "
            "Make sure to call Root::initialise before creating a window.", "Root::createRenderWindow");
        }
        if (!mActiveRenderer)
        {
            OGRE_EXCEPT(Exception::ERR_INVALID_STATE,
            "Cannot create window - no render "
            "system has been selected.", "Root::createRenderWindow");
        }
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
    bool Root::createRenderWindows(const RenderWindowDescriptionList& renderWindowDescriptions,
        RenderWindowList& createdWindows)
    {
        if (!mIsInitialised)
        {
            OGRE_EXCEPT(Exception::ERR_INVALID_STATE,
            "Cannot create window - Root has not been initialised! "
            "Make sure to call Root::initialise before creating a window.", "Root::createRenderWindows");
        }
        if (!mActiveRenderer)
        {
            OGRE_EXCEPT(Exception::ERR_INVALID_STATE,
                "Cannot create render windows - no render "
                "system has been selected.", "Root::createRenderWindows");
        }

        bool success;

        success = mActiveRenderer->_createRenderWindows(renderWindowDescriptions, createdWindows);
        if(success && !mFirstTimePostWindowInit)
        {
            oneTimePostWindowInit();
            createdWindows[0]->_setPrimary();
        }

        return success;
    }
    //-----------------------------------------------------------------------
    RenderTarget* Root::detachRenderTarget(RenderTarget* target)
    {
        if (!mActiveRenderer)
        {
            OGRE_EXCEPT(Exception::ERR_INVALID_STATE,
            "Cannot detach target - no render "
            "system has been selected.", "Root::detachRenderTarget");
        }

        return mActiveRenderer->detachRenderTarget( target->getName() );
    }
    //-----------------------------------------------------------------------
    RenderTarget* Root::detachRenderTarget(const String &name)
    {
        if (!mActiveRenderer)
        {
            OGRE_EXCEPT(Exception::ERR_INVALID_STATE,
            "Cannot detach target - no render "
            "system has been selected.", "Root::detachRenderTarget");
        }

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
        if (!mActiveRenderer)
        {
            OGRE_EXCEPT(Exception::ERR_INVALID_STATE,
            "Cannot get target - no render "
            "system has been selected.", "Root::getRenderTarget");
        }

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
#if OGRE_PLATFORM != OGRE_PLATFORM_NACL && OGRE_PLATFORM != OGRE_PLATFORM_EMSCRIPTEN
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
#if OGRE_PLATFORM != OGRE_PLATFORM_NACL && OGRE_PLATFORM != OGRE_PLATFORM_EMSCRIPTEN
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
        return mTimer;
    }
    //-----------------------------------------------------------------------
    void Root::oneTimePostWindowInit(void)
    {
        if (!mFirstTimePostWindowInit)
        {
            // Background loader
            mResourceBackgroundQueue->initialise();
            mWorkQueue->startup();
            //Do this now as we need the RS to be fully initialized
            mHlmsManager->_changeRenderSystem( mActiveRenderer );

            if( !mHlmsManager->getHlms( mHlmsLowLevelProxy->getType() ) )
                mHlmsManager->registerHlms( mHlmsLowLevelProxy, false );
            if( !mHlmsManager->getComputeHlms() )
                mHlmsManager->registerComputeHlms( mHlmsCompute );
            // Initialise material manager
            mMaterialManager->initialise();
            mCompositorManager2 = OGRE_NEW CompositorManager2( mActiveRenderer );
            // Init particle systems manager
            mParticleManager->_initialise();
            // Init mesh manager
            v1::MeshManager::getSingleton()._initialise();
            mMeshManager->_initialise();
            mMeshManager->_setVaoManager( mActiveRenderer->getVaoManager() );
            // Init plugins - after window creation so rsys resources available
            initialisePlugins();
            mFirstTimePostWindowInit = true;
        }

    }
    //-----------------------------------------------------------------------
    bool Root::_updateAllRenderTargets(void)
    {
        // update all targets but don't swap buffers
        //mActiveRenderer->_updateAllRenderTargets(false);
        mCompositorManager2->_update( *mSceneManagerEnum, mHlmsManager );

        // give client app opportunity to use queued GPU time
        bool ret = _fireFrameRenderingQueued();
        // block for final swap
        mCompositorManager2->_swapAllFinalTargets();

        // This belongs here, as all render targets must be updated before events are
        // triggered, otherwise targets could be mismatched.  This could produce artifacts,
        // for instance, with shadows.
        for (SceneManagerEnumerator::SceneManagerIterator it = getSceneManagerIterator(); it.hasMoreElements(); it.moveNext())
            it.peekNextValue()->_handleLodEvents();

        return ret;
    }
    //---------------------------------------------------------------------
    bool Root::_updateAllRenderTargets(FrameEvent& evt)
    {
        // update all targets but don't swap buffers
        mCompositorManager2->_update( *mSceneManagerEnum, mHlmsManager );
        // give client app opportunity to use queued GPU time
        bool ret = _fireFrameRenderingQueued(evt);
        // block for final swap
        mCompositorManager2->_swapAllFinalTargets();

        // This belongs here, as all render targets must be updated before events are
        // triggered, otherwise targets could be mismatched.  This could produce artifacts,
        // for instance, with shadows.
        for (SceneManagerEnumerator::SceneManagerIterator it = getSceneManagerIterator(); it.hasMoreElements(); it.moveNext())
            it.peekNextValue()->_handleLodEvents();

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
    unsigned int Root::getDisplayMonitorCount() const
    {
        if (!mActiveRenderer)
        {
            OGRE_EXCEPT(Exception::ERR_INVALID_STATE,
                "Cannot get display monitor count "
                "No render system has been selected.", "Root::getDisplayMonitorCount");
        }

        return mActiveRenderer->getDisplayMonitorCount();

    }
    //---------------------------------------------------------------------
    void Root::setWorkQueue(WorkQueue* queue)
    {
        if (mWorkQueue != queue)
        {
            // delete old one (will shut down)
            OGRE_DELETE mWorkQueue;

            mWorkQueue = queue;
            if (mIsInitialised)
                mWorkQueue->startup();

        }
    }



}
