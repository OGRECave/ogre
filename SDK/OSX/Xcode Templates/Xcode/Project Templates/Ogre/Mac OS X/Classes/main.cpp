/*
 -----------------------------------------------------------------------------
 This source file is part of OGRE
 (Object-oriented Graphics Rendering Engine)
 For the latest info, see http://www.ogre3d.org/
 
 Copyright (c) 2000-2009 Torus Knot Software Ltd
 
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
#include "InputHandler.h"
#include "Simulation.h"

#include "Ogre.h"
#include "OgreException.h"
#include "OgreWindowEventUtilities.h"

void go(void);

#ifdef OGRE_STATIC_LIB
#  define OGRE_STATIC_GL
#  if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#    define OGRE_STATIC_Direct3D9
// dx10 will only work on vista, so be careful about statically linking
#    if OGRE_USE_D3D10
#      define OGRE_STATIC_Direct3D10
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
#include "macUtils.h"
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
#include <UIKit/UIKit.h>
#endif

using namespace Ogre;

#include "OgreErrorDialog.h"

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
    try {
        go();
    } catch( Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occurred!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        std::cerr << "An exception has occurred: " << e.getFullDescription();
#endif
    }

    return 0;
#endif
}

void go(void) {

    Ogre::Root *mRoot;
	Ogre::RenderWindow *mWindow;
	Ogre::SceneManager *mSceneManager;
	Ogre::Camera *mCamera;
    Ogre::String mPluginFile;
    Ogre::String mConfigFile;
#ifdef OGRE_STATIC_LIB
    Ogre::StaticPluginLoader mStaticPluginLoader;
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
    mPluginFile = "";
    mConfigFile = Ogre::macBundlePath() + "/ogre.cfg";
#else
    mPluginFile = Ogre::macBundlePath() + "/Contents/Resources/plugins.cfg";
    mConfigFile = Ogre::macBundlePath() + "/Contents/Resources/ogre.cfg";
#endif
	// Fire up an Ogre rendering window. Clearing the first two (of three) params will let us 
	// specify plugins and resources in code instead of via text file
	mRoot = OGRE_NEW Ogre::Root(mPluginFile, mConfigFile);

#ifdef OGRE_STATIC_LIB
    mStaticPluginLoader.load();
#endif

	// Load the basic resource location(s)
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
		Ogre::macBundlePath() + "/Contents/Resources", "FileSystem", "General");
#if OGRE_PLATFORM != OGRE_PLATFORM_IPHONE
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
        Ogre::macBundlePath() + "/Contents/Resources/gui.zip", "Zip", "GUI");
#endif
    
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
		"c:\\windows\\fonts", "FileSystem", "GUI");
#endif

	Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("General");
#if OGRE_PLATFORM != OGRE_PLATFORM_IPHONE
	Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("GUI");
#endif

    if(mRoot->showConfigDialog())
    {
        // If returned true, user clicked OK so initialise
        // Here we choose to let the system create a default rendering window by passing 'true'
        mWindow = mRoot->initialise(true);
        if(!mWindow)
            OGRE_EXCEPT (Ogre::Exception::ERR_INVALID_STATE, "Could not create a window",
                         "main");

    } else {
        return;
    }

	// Since this is basically a CEGUI app, we can use the ST_GENERIC scene manager for now; in a later article 
	// we'll see how to change this
	mSceneManager = mRoot->createSceneManager(Ogre::ST_GENERIC);
	mCamera = mSceneManager->createCamera("camera");
    // Position it at 500 in Z direction
    mCamera->setPosition(Ogre::Vector3(0,0,500));
    // Look back along -Z
    mCamera->lookAt(Ogre::Vector3(0,0,-300));
    mCamera->setNearClipDistance(5);

    Ogre::Viewport* vp = mWindow->addViewport(mCamera);
    vp->setBackgroundColour(Ogre::ColourValue(0.5,0.5,0.5));

    // Alter the camera aspect ratio to match the viewport
    mCamera->setAspectRatio(Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));

	// This next bit is for the sake of the input handler
	unsigned long hWnd;
	mWindow->getCustomAttribute("WINDOW", &hWnd);

	// Set up the input handlers
	Simulation *sim = new Simulation();
	InputHandler *handler = new InputHandler(sim, hWnd);
	sim->requestStateChange(SIMULATION);

	while (sim->getCurrentState() != SHUTDOWN) {
		handler->capture();

		// Run the message pump
		Ogre::WindowEventUtilities::messagePump();

        if (!mRoot->renderOneFrame())
            break;
	}

    mRoot->shutdown();

#ifdef OGRE_STATIC_LIB
    mStaticPluginLoader.unload();
#endif

	// Clean up after ourselves
//	OGRE_DELETE mRoot;
	delete handler;
	delete sim;

	return;
}

#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
#   ifdef __OBJC__
@interface AppDelegate : NSObject <UIApplicationDelegate>
{
}

- (void)go;

@end

@implementation AppDelegate

- (void)go {
    try {
        go();
    } catch( Ogre::Exception& e ) {
        std::cerr << "An exception has occurred: " <<
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
