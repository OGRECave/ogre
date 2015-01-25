
#include "GraphicsSystem.h"
#include "GameState.h"
#include "SdlInputHandler.h"

#include "OgreRoot.h"
#include "OgreException.h"
#include "OgreConfigFile.h"

#include "OgreRenderWindow.h"
#include "OgreCamera.h"

#include "OgreHlmsUnlit.h"
#include "OgreHlmsPbs.h"
#include "OgreHlmsManager.h"
#include "OgreArchiveManager.h"

#include "Compositor/OgreCompositorManager2.h"

#include "OgreOverlaySystem.h"

#include <SDL_syswm.h>

namespace Demo
{
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#include <CoreFoundation/CoreFoundation.h>

// This function will locate the path to our application on OS X,
// unlike windows you can not rely on the curent working directory
// for locating your configuration files and resources.
std::string macBundlePath()
{
    char path[1024];
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    assert(mainBundle);

    CFURLRef mainBundleURL = CFBundleCopyBundleURL(mainBundle);
    assert(mainBundleURL);

    CFStringRef cfStringRef = CFURLCopyFileSystemPath( mainBundleURL, kCFURLPOSIXPathStyle);
    assert(cfStringRef);

    CFStringGetCString(cfStringRef, path, 1024, kCFStringEncodingASCII);

    CFRelease(mainBundleURL);
    CFRelease(cfStringRef);

    return std::string(path);
}
#endif

    GraphicsSystem::GraphicsSystem( GameState *gameState,
                                    Ogre::ColourValue backgroundColour ) :
        BaseSystem( gameState ),
        mLogicSystem( 0 ),
        mSdlWindow( 0 ),
        mInputHandler( 0 ),
        mRoot( 0 ),
        mRenderWindow( 0 ),
        mSceneManager( 0 ),
        mCamera( 0 ),
        mWorkspace( 0 ),
        mOverlaySystem( 0 ),
        mQuit( false ),
        mBackgroundColour( backgroundColour )
    {
    }
    //-----------------------------------------------------------------------------------
    GraphicsSystem::~GraphicsSystem()
    {
        assert( !mRoot && "deinitialize() not called!!!" );
    }
    //-----------------------------------------------------------------------------------
    void GraphicsSystem::initialize(void)
    {
        if( SDL_Init( SDL_INIT_EVERYTHING ) != 0 )
        {
            OGRE_EXCEPT( Ogre::Exception::ERR_INTERNAL_ERROR, "Cannot initialize SDL2!",
                         "GraphicsSystem::initialize" );
        }

        Ogre::String pluginsPath;
        // only use plugins.cfg if not static
#ifndef OGRE_STATIC_LIB
    #if OGRE_DEBUG_MODE
        pluginsPath = mResourcePath + "plugins_d.cfg";
    #else
        pluginsPath = mResourcePath + "plugins.cfg";
    #endif
#endif

        mRoot = OGRE_NEW Ogre::Root( pluginsPath,
                                     mResourcePath + "ogre.cfg",
                                     mResourcePath + "Ogre.log" );

        if( !mRoot->restoreConfig() )
        {
            if( !mRoot->showConfigDialog() )
            {
                mQuit = true;
                return;
            }
        }

        mRoot->getRenderSystem()->setConfigOption( "sRGB Gamma Conversion", "Yes" );
        mRoot->initialise(false);

        Ogre::String title = "Ogre SDK Demo";
        int width   = 1280;
        int height  = 720;

        int screen = 0;
        int posX = SDL_WINDOWPOS_CENTERED_DISPLAY(screen);
        int posY = SDL_WINDOWPOS_CENTERED_DISPLAY(screen);

        bool fullscreen = false;
        if(fullscreen)
        {
            posX = SDL_WINDOWPOS_UNDEFINED_DISPLAY(screen);
            posY = SDL_WINDOWPOS_UNDEFINED_DISPLAY(screen);
        }

        mSdlWindow = SDL_CreateWindow(
                    title.c_str(),    // window title
                    posX,               // initial x position
                    posY,               // initial y position
                    width,              // width, in pixels
                    height,             // height, in pixels
                    SDL_WINDOW_SHOWN
                      | (fullscreen ? SDL_WINDOW_FULLSCREEN : 0) | SDL_WINDOW_RESIZABLE );

        //Get the native whnd
        SDL_SysWMinfo wmInfo;
        SDL_VERSION( &wmInfo.version );

        if( SDL_GetWindowWMInfo( mSdlWindow, &wmInfo ) == SDL_FALSE )
        {
            OGRE_EXCEPT( Ogre::Exception::ERR_INTERNAL_ERROR,
                         "Couldn't get WM Info! (SDL2)",
                         "GraphicsSystem::initialize" );
        }

        Ogre::String winHandle;
        Ogre::NameValuePairList params;

        switch( wmInfo.subsystem )
        {
    #ifdef WIN32
        case SDL_SYSWM_WINDOWS:
            // Windows code
            winHandle = Ogre::StringConverter::toString( (unsigned long)wmInfo.info.win.window );
            break;
    #elif __MACOSX__
        case SDL_SYSWM_COCOA:
            //required to make OGRE play nice with our window
            params.insert( std::make_pair("macAPI", "cocoa") );
            params.insert( std::make_pair("macAPICocoaUseNSView", "true") );

            winHandle  = Ogre::StringConverter::toString(WindowContentViewHandle(wmInfo));
            break;
    #else
        case SDL_SYSWM_X11:
            winHandle = Ogre::StringConverter::toString( (unsigned long)wmInfo.info.x11.window );
            break;
    #endif
        default:
            OGRE_EXCEPT( Ogre::Exception::ERR_NOT_IMPLEMENTED,
                         "Unexpected WM! (SDL2)",
                         "GraphicsSystem::initialize" );
            break;
        }

        params.insert( std::make_pair("title", title) );
        //params.insert( std::make_pair("FSAA", ) );
        //params.insert( std::make_pair("vsync", vsync ? "true" : "false") );

        /// \todo externalWindowHandle is deprecated according to the source code. Figure out a way to get parentWindowHandle
        /// to work properly. On Linux/X11 it causes an occasional GLXBadDrawable error.
        params.insert( std::make_pair("externalWindowHandle",  winHandle) );

        mRenderWindow = Ogre::Root::getSingleton().createRenderWindow( title, width, height,
                                                                       fullscreen, &params );

        mOverlaySystem = OGRE_NEW Ogre::v1::OverlaySystem();

        setupResources();
        loadResources();
        chooseSceneManager();
        createCamera();
        mWorkspace = setupCompositor();

        mInputHandler = new SdlInputHandler( mSdlWindow, mCurrentGameState,
                                             mCurrentGameState, mCurrentGameState );

        BaseSystem::initialize();
    }
    //-----------------------------------------------------------------------------------
    void GraphicsSystem::deinitialize(void)
    {
        BaseSystem::deinitialize();

        if( mSceneManager )
            mSceneManager->removeRenderQueueListener( mOverlaySystem );

        OGRE_DELETE mOverlaySystem;
        mOverlaySystem = 0;

        delete mInputHandler;
        mInputHandler = 0;

        if( mSdlWindow )
        {
            // Restore desktop resolution on exit
            SDL_SetWindowFullscreen( mSdlWindow, 0 );
            SDL_DestroyWindow( mSdlWindow );
            mSdlWindow = 0;
        }

        OGRE_DELETE mRoot;
        mRoot = 0;

        SDL_Quit();
    }
    //-----------------------------------------------------------------------------------
    void GraphicsSystem::update( float timeSinceLast )
    {
        SDL_Event evt;
        while( SDL_PollEvent( &evt ) )
        {
            switch( evt.type )
            {
            case SDL_WINDOWEVENT:
                handleWindowEvent( evt );
                break;
            case SDL_QUIT:
                mQuit = true;
                break;
            default:
                break;
            }

            mInputHandler->_handleSdlEvents( evt );
        }

        BaseSystem::update( timeSinceLast );

        if( mRenderWindow->isVisible() )
            mQuit |= !mRoot->renderOneFrame();
    }
    //-----------------------------------------------------------------------------------
    void GraphicsSystem::handleWindowEvent( const SDL_Event& evt )
    {
        switch( evt.window.event )
        {
            case SDL_WINDOWEVENT_SIZE_CHANGED:
                int w,h;
                SDL_GetWindowSize( mSdlWindow, &w, &h );
#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
                mRenderWindow->resize( w, h );
#else
                mRenderWindow->windowMovedOrResized();
#endif
                break;
            case SDL_WINDOWEVENT_RESIZED:
#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
                mRenderWindow->resize( evt.window.data1, evt.window.data2 );
#else
                mRenderWindow->windowMovedOrResized();
#endif
                break;
            case SDL_WINDOWEVENT_CLOSE:
                break;
            case SDL_WINDOWEVENT_SHOWN:
                mRenderWindow->setVisible(true);
                break;
            case SDL_WINDOWEVENT_HIDDEN:
                mRenderWindow->setVisible(false);
                break;
        }
    }
    //-----------------------------------------------------------------------------------
    void GraphicsSystem::processIncomingMessage( Mq::MessageId messageId, Mq::SendData data )
    {
    }
    //-----------------------------------------------------------------------------------
    void GraphicsSystem::setupResources(void)
    {
        // Load resource paths from config file
        Ogre::ConfigFile cf;
        cf.load(mResourcePath + "resources.cfg");

        // Go through all sections & settings in the file
        Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

        Ogre::String secName, typeName, archName;
        while( seci.hasMoreElements() )
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
                    Ogre::String(macBundlePath() + "/" + archName), typeName, secName);
#else
                Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                    archName, typeName, secName);
#endif
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void GraphicsSystem::registerHlms(void)
    {
        Ogre::Archive *archiveUnlit = Ogre::ArchiveManager::getSingletonPtr()->load(
                        "/home/matias/Projects/SDK/Ogre2-Hlms-private/Samples/Media/Hlms/Unlit/GLSL",
                        "FileSystem", true );

        Ogre::HlmsUnlit *hlmsUnlit = OGRE_NEW Ogre::HlmsUnlit( archiveUnlit );
        Ogre::Root::getSingleton().getHlmsManager()->registerHlms( hlmsUnlit );

        Ogre::Archive *archivePbs = Ogre::ArchiveManager::getSingletonPtr()->load(
                        "/home/matias/Projects/SDK/Ogre2-Hlms-private/Samples/Media/Hlms/Pbs/GLSL",
                        "FileSystem", true );
        Ogre::HlmsPbs *hlmsPbs = OGRE_NEW Ogre::HlmsPbs( archivePbs );
        Ogre::Root::getSingleton().getHlmsManager()->registerHlms( hlmsPbs );
    }
    //-----------------------------------------------------------------------------------
    void GraphicsSystem::loadResources(void)
    {
        registerHlms();

        // Initialise, parse scripts etc
        Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
    }
    //-----------------------------------------------------------------------------------
    void GraphicsSystem::chooseSceneManager(void)
    {
        Ogre::InstancingTheadedCullingMethod threadedCullingMethod =
                Ogre::INSTANCING_CULLING_SINGLETHREAD;
#ifdef _DEBUG
        //Debugging multithreaded code is a PITA, disable it.
        const size_t numThreads = 1;
#else
        //getNumLogicalCores() may return 0 if couldn't detect
        const size_t numThreads = std::max<size_t>( 1, Ogre::PlatformInformation::getNumLogicalCores() );
        //See doxygen documentation regarding culling methods.
        //In some cases you may still want to use single thread.
        //if( numThreads > 1 )
        //	threadedCullingMethod = Ogre::INSTANCING_CULLING_THREADED;
#endif
        // Create the SceneManager, in this case a generic one
        mSceneManager = mRoot->createSceneManager( Ogre::ST_GENERIC,
                                                   numThreads,
                                                   threadedCullingMethod,
                                                   "ExampleSMInstance" );

        mSceneManager->addRenderQueueListener( mOverlaySystem );
    }
    //-----------------------------------------------------------------------------------
    void GraphicsSystem::createCamera(void)
    {
        mCamera = mSceneManager->createCamera( "Main Camera" );

        // Position it at 500 in Z direction
        mCamera->setPosition( Ogre::Vector3( 0, 5, 15 ) );
        // Look back along -Z
        mCamera->lookAt( Ogre::Vector3( 0, 0, 0 ) );
        mCamera->setNearClipDistance( 0.2f );
        mCamera->setFarClipDistance( 1000.0f );
    }
    //-----------------------------------------------------------------------------------
    Ogre::CompositorWorkspace* GraphicsSystem::setupCompositor()
    {
        Ogre::CompositorManager2 *compositorManager = mRoot->getCompositorManager2();

        const Ogre::IdString workspaceName( "Demo Workspace" );
        if( !compositorManager->hasWorkspaceDefinition( workspaceName ) )
        {
            compositorManager->createBasicWorkspaceDef( workspaceName, mBackgroundColour,
                                                        Ogre::IdString() );
        }

        return compositorManager->addWorkspace( mSceneManager, mRenderWindow, mCamera,
                                                workspaceName, true );
    }
}
