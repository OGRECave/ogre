#ifndef __AppDelegate_H__
#define __AppDelegate_H__

#include "OgrePlatform.h"

#if !defined(OGRE_IS_IOS)
#  error This header is for use with iOS only
#endif

#ifdef __OBJC__

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>

// To use CADisplayLink for smoother animation on iPhone comment out
// the following line or define it to 1.  Use with caution, it can
// sometimes cause input lag.
#define USE_CADISPLAYLINK 1


@interface AppDelegate : NSObject <UIApplicationDelegate>
{
    NSTimer *mTimer;
    DemoApp demo;
    
    // Use of the CADisplayLink class is the preferred method for controlling your animation timing.
    // CADisplayLink will link to the main display and fire every vsync when added to a given run-loop.
    // The NSTimer class is used only as fallback when running on a pre 3.1 device where CADisplayLink
    // isn't available.
    id mDisplayLink;
    NSDate *mDate;
    double mLastFrameTime;
    double mStartTime;
    BOOL mDisplayLinkSupported;
}

- (void)go;
- (void)renderOneFrame:(id)sender;

@property (retain) NSTimer *mTimer;
@property (nonatomic) double mLastFrameTime;
@property (nonatomic) double mStartTime;

@end

@implementation AppDelegate

@synthesize mTimer;
@dynamic mLastFrameTime;
@dynamic mStartTime;

- (double)mLastFrameTime
{
    return mLastFrameTime;
}

- (void)setLastFrameTime:(double)frameInterval
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
    mStartTime = 0;
    mTimer = nil;
    
    try {
        demo.startDemo();
        
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

- (void)applicationDidFinishLaunching:(UIApplication *)application
{
    // Hide the status bar
    [[UIApplication sharedApplication] setStatusBarHidden:YES];
    
    mDisplayLinkSupported = FALSE;
    mLastFrameTime = 1;
    mStartTime = 0;
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

- (void)applicationWillTerminate:(UIApplication *)application
{
    Ogre::Root::getSingleton().queueEndRendering();
    
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
    [[UIApplication sharedApplication] performSelector:@selector(terminate:) withObject:nil afterDelay:0.0];
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    Ogre::Root::getSingleton().saveConfig();
    
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
}

- (void)renderOneFrame:(id)sender
{
    if(!OgreFramework::getSingletonPtr()->isOgreToBeShutDown() &&
       Ogre::Root::getSingletonPtr() && Ogre::Root::getSingleton().isInitialised())
    {
		if(OgreFramework::getSingletonPtr()->m_pRenderWnd->isActive())
		{
			mStartTime = OgreFramework::getSingletonPtr()->m_pTimer->getMillisecondsCPU();
            
			OgreFramework::getSingletonPtr()->m_pMouse->capture();
            
			OgreFramework::getSingletonPtr()->updateOgre(mLastFrameTime);
			OgreFramework::getSingletonPtr()->m_pRoot->renderOneFrame();
            
			mLastFrameTime = OgreFramework::getSingletonPtr()->m_pTimer->getMillisecondsCPU() - mStartTime;
		}
    }
    else
    {
		
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
        [[UIApplication sharedApplication] performSelector:@selector(terminate:) withObject:nil afterDelay:0.0];
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

#endif

#endif
