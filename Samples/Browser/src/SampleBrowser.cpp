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
#include <Cocoa/Cocoa.h>

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#elif OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
#import <UIKit/UIKit.h> 
#import <QuartzCore/QuartzCore.h>

// To use CADisplayLink for smoother animation on iPhone comment out
// the following line or define it to 1.  Use with caution, it can
// sometimes cause input lag.
#define USE_CADISPLAYLINK 1
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

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#   ifdef __OBJC__

// All this does is suppress some messages in the run log.  NSApplication does not
// implement buttonPressed and apps without a NIB have no target for the action.
@implementation NSApplication (_suppressUnimplementedActionWarning)
- (void) buttonPressed:(id)sender
{
    /* Do nothing */
}
@end


@interface AppDelegate : NSObject <NSApplicationDelegate>
{
    NSTimer *mTimer;
    OgreBites::SampleBrowser sb;

    NSDate *mDate;
    NSTimeInterval mLastFrameTime;
}

- (void)go;
- (void)renderOneFrame:(id)sender;

@property (retain) NSTimer *mTimer;
@property (nonatomic) NSTimeInterval mLastFrameTime;

@end

#if (OGRE_PLATFORM == OGRE_PLATFORM_APPLE) && __LP64__
static id mAppDelegate;
#endif

@implementation AppDelegate

@synthesize mTimer;
@dynamic mLastFrameTime;

- (NSTimeInterval)mLastFrameTime
{
    return mLastFrameTime;
}

- (void)setLastFrameTime:(NSTimeInterval)frameInterval
{
    // Frame interval defines how many display frames must pass between each time the
    // display link fires. The display link will only fire 30 times a second when the
    // frame internal is two on a display that refreshes 60 times a second. The default
    // frame interval setting of one will fire 60 times a second when the display refreshes
    // at 60 times a second. A frame interval setting of less than one results in undefined
    // behavior.
    if (frameInterval >= 1)
    {
        mLastFrameTime = frameInterval;
    }
}

- (void)go {
    
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    mLastFrameTime = 1;
    mTimer = nil;

    try {
        sb.go();
        
        Ogre::Root::getSingleton().getRenderSystem()->_initRenderTargets();
        
        // Clear event times
		Ogre::Root::getSingleton().clearEventTimes();
    } catch( Ogre::Exception& e ) {
        std::cerr << "An exception has occurred: " <<
        e.getFullDescription().c_str() << std::endl;
    }
    
    mTimer = [NSTimer scheduledTimerWithTimeInterval:(NSTimeInterval)(1.0f / 60.0f) * mLastFrameTime
                                              target:self
                                            selector:@selector(renderOneFrame:)
                                            userInfo:nil
                                             repeats:YES];
    [pool release];
}
    
- (void)applicationDidFinishLaunching:(NSNotification *)application {
    mLastFrameTime = 1;
    mTimer = nil;

    [self go];
}
        
- (void)renderOneFrame:(id)sender
{
    if(Ogre::Root::getSingletonPtr() && Ogre::Root::getSingleton().isInitialised())
    {
        Ogre::Root::getSingleton().renderOneFrame((Ogre::Real)[mTimer timeInterval]);
    }
    else
    {
        [mTimer invalidate];
        mTimer = nil;
        [NSApp performSelector:@selector(terminate:) withObject:nil afterDelay:0.0];
    }
}

- (void)dealloc {
    if(mTimer)
    {
        [mTimer invalidate];
        mTimer = nil;
    }

    [super dealloc];
}

@end
#   endif
        
#endif
        
int main(int argc, char *argv[])
#endif
{
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	int retVal = UIApplicationMain(argc, argv, @"UIApplication", @"AppDelegate");
	[pool release];
	return retVal;
#elif (OGRE_PLATFORM == OGRE_PLATFORM_APPLE) && __LP64__
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    
    mAppDelegate = [[AppDelegate alloc] init];
    [[NSApplication sharedApplication] setDelegate:mAppDelegate];
	int retVal = NSApplicationMain(argc, (const char **) argv);

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

    // Use of the CADisplayLink class is the preferred method for controlling your animation timing.
    // CADisplayLink will link to the main display and fire every vsync when added to a given run-loop.
    // The NSTimer class is used only as fallback when running on a pre 3.1 device where CADisplayLink
    // isn't available.
    id mDisplayLink;
    NSDate* mDate;
    NSTimeInterval mLastFrameTime;
    BOOL mDisplayLinkSupported;
}

- (void)go;
- (void)renderOneFrame:(id)sender;

@property (retain) NSTimer *mTimer;
@property (nonatomic) NSTimeInterval mLastFrameTime;

@end

@implementation AppDelegate

@synthesize mTimer;
@dynamic mLastFrameTime;

- (id)init
{
    if((self = [super init]))
    {
        [[NSApplication sharedApplication] setDelegate:self];
    }
    return self;
}

- (NSTimeInterval)mLastFrameTime
{
    return mLastFrameTime;
}

- (void)setLastFrameTime:(NSTimeInterval)frameInterval
{
    // Frame interval defines how many display frames must pass between each time the
    // display link fires. The display link will only fire 30 times a second when the
    // frame internal is two on a display that refreshes 60 times a second. The default
    // frame interval setting of one will fire 60 times a second when the display refreshes
    // at 60 times a second. A frame interval setting of less than one results in undefined
    // behavior.
    if (frameInterval >= 1)
    {
        mLastFrameTime = frameInterval;
    }
}

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

    if (mDisplayLinkSupported)
    {
        // CADisplayLink is API new to iPhone SDK 3.1. Compiling against earlier versions will result in a warning, but can be dismissed
        // if the system version runtime check for CADisplayLink exists in -initWithCoder:. The runtime check ensures this code will
        // not be called in system versions earlier than 3.1.
        mDate = [[NSDate alloc] init];
        mLastFrameTime = -[mDate timeIntervalSinceNow];
        
        mDisplayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:self selector:@selector(renderOneFrame:)];
        [mDisplayLink setFrameInterval:mLastFrameTime];
        [mDisplayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    }
    else
    {
        mTimer = [NSTimer scheduledTimerWithTimeInterval:(NSTimeInterval)(1.0f / 60.0f) * mLastFrameTime
                                                  target:self
                                                selector:@selector(renderOneFrame:)
                                                userInfo:nil
                                                 repeats:YES];
    }
    
    [pool release];
}

- (void)applicationDidFinishLaunching:(UIApplication *)application {
    // Hide the status bar
    [[UIApplication sharedApplication] setStatusBarHidden:YES];

    mDisplayLinkSupported = FALSE;
    mLastFrameTime = 1;
    mDisplayLink = nil;
    mTimer = nil;

    // A system version of 3.1 or greater is required to use CADisplayLink. The NSTimer
    // class is used as fallback when it isn't available.
#if USE_CADISPLAYLINK
    NSString *reqSysVer = @"3.1";
    NSString *currSysVer = [[UIDevice currentDevice] systemVersion];
    if ([currSysVer compare:reqSysVer options:NSNumericSearch] != NSOrderedAscending)
        mDisplayLinkSupported = TRUE;
#endif
    
    [self go];
}

- (void)renderOneFrame:(id)sender
{
    [sb.mGestureView becomeFirstResponder];

    if (mDisplayLinkSupported)
    {
        // NSTimerInterval is a simple typedef for double
        NSTimeInterval currentFrameTime = -[mDate timeIntervalSinceNow];
        NSTimeInterval differenceInSeconds = currentFrameTime - mLastFrameTime;
        mLastFrameTime = currentFrameTime;

        Root::getSingleton().renderOneFrame((Real)differenceInSeconds);
    }
    else
    {
        Root::getSingleton().renderOneFrame((Real)[mTimer timeInterval]);
    }
}
        
- (void)applicationWillTerminate:(UIApplication *)application {
    Ogre::Root::getSingleton().queueEndRendering();
}

- (void)dealloc {
    if (mDisplayLinkSupported)
    {
        [mDate release];
        mDate = nil;

        [mDisplayLink invalidate];
        mDisplayLink = nil;
    }
    else
    {
        [mTimer invalidate];
        mTimer = nil;
    }

    [super dealloc];
}

@end
#   endif
        
#endif
