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
        Grass.cpp
    \brief
        Specialisation of OGRE's framework application to show the
        use of the StaticGeometry class to create 'baked' instances of
		many meshes, to create effects like grass efficiently.
**/

#include "ExampleApplication.h"
#include "OgrePaging.h"


#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif


#define KEY_PRESSED(_key,_timeDelay, _macro) \
{ \
    if (mKeyboard->isKeyDown(_key) && timeDelay <= 0) \
    { \
		timeDelay = _timeDelay; \
        _macro ; \
    } \
}

class PagingListener : public ExampleFrameListener
{
protected:
	SceneManager* mSceneManager;
public:
	PagingListener(RenderWindow* win, Camera* cam, SceneManager* sceneManager)
		: ExampleFrameListener(win, cam), 
		mSceneManager(sceneManager)
	{
	}


	bool frameRenderingQueued(const FrameEvent& evt)
	{
		if( ExampleFrameListener::frameRenderingQueued(evt) == false )
			return false;


		return true;
	}
};



class PagingApp : public ExampleApplication, public PageProvider
{
public:
	PagingApp() : mPageManager(0) {}
	~PagingApp()
	{
		OGRE_DELETE mPageManager;
	}
	
protected:

	PageManager* mPageManager;

	void createScene(void)
    {
		LogManager::getSingleton().setLogDetail(LL_BOREME);

		mPageManager = OGRE_NEW PageManager();

//		PagedWorld* world = mPageManager->createWorld();
//		PagedWorldSection* sec = world->createSection("Grid2D", mSceneMgr);

//		Grid2DPageStrategyData* data = static_cast<Grid2DPageStrategyData*>(sec->getStrategyData());

		// accept defaults for now

		mPageManager->setDebugDisplayLevel(1);

		// hook up self to provide pages procedurally
		mPageManager->setPageProvider(this);

		mCamera->setPosition(0, 100, 0);

		mPageManager->addCamera(mCamera);

	}

	// callback on PageProvider
	bool prepareProceduralPage(Page* page, PagedWorldSection* section)
	{
		// say we populated something just so it doesn't try to load any more
		return true;
	}

	bool loadProceduralPage(Page* page, PagedWorldSection* section)
	{
		// say we populated something just so it doesn't try to load any more
		return true;
	}
    // Create new frame listener
    void createFrameListener(void)
    {
        mFrameListener= new PagingListener(mWindow, mCamera, mSceneMgr);
        mRoot->addFrameListener(mFrameListener);
    }
};

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main(int argc, char **argv)
#endif
{
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
        NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
        int retVal = UIApplicationMain(argc, argv, @"UIApplication", @"AppDelegate");
        [pool release];
        return retVal;
#else
    // Create application object
    PagingApp app;

    try {
        app.go();
    } catch( Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        std::cerr << "An exception has occured: " << e.getFullDescription();
#endif
    }

    return 0;
#endif
}

#ifdef __cplusplus
}
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
#   ifdef __OBJC__
@interface AppDelegate : NSObject <UIApplicationDelegate>
{
}

- (void)go;

@end

@implementation AppDelegate

- (void)go {
    // Create application object
    PagingApp app;
    try {
        app.go();
    } catch( Ogre::Exception& e ) {
        std::cerr << "An exception has occured: " <<
        e.getFullDescription().c_str() << std::endl;
    }
}

- (void)applicationDidFinishLaunching:(UIApplication *)application {
    // Hide the status bar
    [[UIApplication sharedApplication] setStatusBarHidden:YES];

    // Create a window
    UIWindow *window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];

    // Create an image view
    UIImageView *imageView = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"Default.png"]];
    [window addSubview:imageView];
    
    // Create an indeterminate status indicator
    UIActivityIndicatorView *indicator = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleWhite];
    [indicator setFrame:CGRectMake(150, 280, 20, 20)];
    [indicator startAnimating];
    [window addSubview:indicator];
    
    // Display our window
    [window makeKeyAndVisible];
    
    // Clean up
    [imageView release];
    [indicator release];

    [NSThread detachNewThreadSelector:@selector(go) toTarget:self withObject:nil];
}

- (void)applicationWillTerminate:(UIApplication *)application {
    Root::getSingleton().queueEndRendering();
}

//- (void)applicationWillResignActive:(UIApplication *)application
//{
//    // Pause FrameListeners and rendering
//}
//
//- (void)applicationDidBecomeActive:(UIApplication *)application
//{
//    // Resume FrameListeners and rendering
//}

- (void)dealloc {
    [super dealloc];
}

@end
#   endif

#endif
