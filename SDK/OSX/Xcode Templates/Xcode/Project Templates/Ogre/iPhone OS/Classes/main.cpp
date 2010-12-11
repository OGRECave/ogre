//|||||||||||||||||||||||||||||||||||||||||||||||

#include "DemoApp.h"

//|||||||||||||||||||||||||||||||||||||||||||||||

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
#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#endif

#if OGRE_PLATFORM == PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

//|||||||||||||||||||||||||||||||||||||||||||||||

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
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
	try
    {
		DemoApp demo;
		demo.startDemo();
    }
	catch(Ogre::Exception& e)
    {
#if OGRE_PLATFORM == PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        MessageBoxA(NULL, e.what(), "An exception has occurred!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        fprintf(stderr, "An exception has occurred: %s\n", e.what());
#endif
    }
    
    return 0;
#endif
}

//|||||||||||||||||||||||||||||||||||||||||||||||

#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
#   ifdef __OBJC__
@interface AppDelegate : NSObject <UIApplicationDelegate>
{
    DemoApp demo;
    NSTimer *mTimer;
    
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
- (void)orientationChanged:(NSNotification *)notification;

@property (retain) NSTimer *mTimer;
@property (nonatomic) NSTimeInterval mLastFrameTime;

@end

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
    try {
        new OgreFramework();
        if(!OgreFramework::getSingletonPtr()->initOgre("DemoApp v1.0", &demo, 0))
            return;
        
        demo.setShutdown(false);
        
        OgreFramework::getSingletonPtr()->m_pLog->logMessage("Demo initialized!");
        
        demo.setupDemoScene();
        OgreFramework::getSingletonPtr()->m_pRenderWnd->resetStatistics();
        
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
        
        [[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(orientationChanged:)
                                                     name:UIDeviceOrientationDidChangeNotification object:nil];
        
    } catch( Ogre::Exception& e ) {
        std::cerr << "An exception has occurred: " <<
        e.getFullDescription().c_str() << std::endl;
        }
        
        [pool release];
        }
        
- (void)orientationChanged:(NSNotification *)notification
{
    size_t v = 0;
    Ogre::Root::getSingleton().getAutoCreatedWindow()->getCustomAttribute("VIEW", &v);
    
    [(UIView *)v setNeedsLayout];
}

        - (void)renderOneFrame:(id)sender
        {
#pragma unused(sender)
            if(OgreFramework::getSingletonPtr()->m_pRenderWnd->isActive())
            {
                // NSTimerInterval is a simple typedef for double
                NSTimeInterval currentFrameTime = -[mDate timeIntervalSinceNow];
                NSTimeInterval differenceInSeconds = currentFrameTime - mLastFrameTime;
                mLastFrameTime = currentFrameTime;
                
                OgreFramework::getSingletonPtr()->m_pMouse->capture();
                
                OgreFramework::getSingletonPtr()->updateOgre(differenceInSeconds);
                OgreFramework::getSingletonPtr()->m_pRoot->renderOneFrame();
            }
        }
        
        - (void)applicationDidFinishLaunching:(UIApplication *)application {
#pragma unused(application)
            // Hide the status bar
            [[UIApplication sharedApplication] setStatusBarHidden:YES];
            
            mDisplayLinkSupported = FALSE;
            mLastFrameTime = 1;
            mDisplayLink = nil;
            mTimer = nil;
            
            // A system version of 3.1 or greater is required to use CADisplayLink. The NSTimer
            // class is used as fallback when it isn't available.
            NSString *reqSysVer = @"3.1";
            NSString *currSysVer = [[UIDevice currentDevice] systemVersion];
            if ([currSysVer compare:reqSysVer options:NSNumericSearch] != NSOrderedAscending)
                mDisplayLinkSupported = TRUE;
            
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
            
            [self go];
            
            [window release];
        }
        
        - (void)applicationWillTerminate:(UIApplication *)application {
#pragma unused(application)
            Ogre::Root::getSingleton().queueEndRendering();
        }
        
- (void)applicationWillResignActive:(UIApplication *)application
{
    Ogre::Root::getSingleton().saveConfig();
    
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [[UIDevice currentDevice] endGeneratingDeviceOrientationNotifications];
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    [[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(orientationChanged:)
                                                 name:UIDeviceOrientationDidChangeNotification object:nil];
}

        - (void)dealloc {
            if (mDisplayLinkSupported)
            {
                [mDisplayLink invalidate];
                mDisplayLink = nil;
            }
            else
            {
                [mTimer invalidate];
                mTimer = nil;
            }
            
            [mDate release];
            mDate = nil;
            
            [super dealloc];
        }
        
        @end
#   endif
#endif
