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
#include "OgrePlatform.h"
#if OGRE_PLATFORM == OGRE_PLATFORM_SYMBIAN 
#	ifdef __GCCE__
#		include <staticlibinit_gcce.h>
#	endif

#	include <e32base.h> // for Symbian classes.
#	include <coemain.h> // for CCoeEnv.

#endif 

#include "SampleBrowser.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#elif OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
#import <UIKit/UIKit.h> 
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, INT)
#elif OGRE_PLATFORM == OGRE_PLATFORM_SYMBIAN    
int mainWithTrap();
int main()
{
	int res = 0;
    __UHEAP_MARK; 
 
	// Create the control environment.
	CCoeEnv* environment = new (ELeave) CCoeEnv();    

	TRAPD( err, environment->ConstructL( ETrue, 0 ) );

	if( err != KErrNone )
	{
		printf( "Unable to create a CCoeEnv!\n" );
		getchar();            
	}
    
    TRAP( err, res = mainWithTrap());

    // Close the stdout & stdin, else printf / getchar causes a memory leak.
    fclose( stdout );
    fclose( stdin );
    
	// Cleanup
	CCoeEnv::Static()->DestroyEnvironment();
	delete CCoeEnv::Static();
       
    __UHEAP_MARKEND;
    
    return res;
}    

int mainWithTrap()

#else
int main(int argc, char *argv[])
#endif
{
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	int retVal = UIApplicationMain(argc, argv, @"UIApplication", @"AppDelegate");
	[pool release];
	return retVal;
#else

	try
	{
		OgreBites::SampleBrowser sb;
		sb.go();
	}
	catch (Ogre::Exception& e)
	{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		MessageBoxA(NULL, e.getFullDescription().c_str(), "An exception has occurred!", MB_ICONERROR | MB_TASKMODAL);
#else
		std::cerr << "An exception has occurred: " << e.getFullDescription().c_str() << std::endl;
#endif
#if OGRE_PLATFORM == OGRE_PLATFORM_SYMBIAN
        getchar();
#endif

	}

#endif
	return 0;
}

#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
#   ifdef __OBJC__
@interface AppDelegate : NSObject <UIApplicationDelegate>
{
    NSTimer *mTimer;
    OgreBites::SampleBrowser sb;
}

- (void)go;
- (void)renderOneFrame:(id)sender;

@property (retain) NSTimer *mTimer;

@end

@implementation AppDelegate

@synthesize mTimer;

- (void)go {

    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

    try {
        sb.go();

        Ogre::Root::getSingleton().getRenderSystem()->_initRenderTargets();
        
        // Clear event times
		Ogre::Root::getSingleton().clearEventTimes();
    } catch( Ogre::Exception& e ) {
        std::cerr << "An exception has occurred: " <<
        e.getFullDescription().c_str() << std::endl;
    }
        
    mTimer = [NSTimer scheduledTimerWithTimeInterval:(NSTimeInterval)(1.0f / 60.0f)
                                              target:self
                                            selector:@selector(renderOneFrame:)
                                            userInfo:nil
                                             repeats:YES];
    
    [pool release];
}

- (void)applicationDidFinishLaunching:(UIApplication *)application {
    // Hide the status bar
    [[UIApplication sharedApplication] setStatusBarHidden:YES];

    [self go];
}

- (void)renderOneFrame:(id)sender
{
    [sb.mGestureView becomeFirstResponder];

    Root::getSingleton().renderOneFrame((Real)[mTimer timeInterval]);
}
        
- (void)applicationWillTerminate:(UIApplication *)application {
    Ogre::Root::getSingleton().queueEndRendering();
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
    [mTimer release];
    
    [super dealloc];
}

@end
#   endif
        
#endif
