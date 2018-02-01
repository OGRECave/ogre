/*
  -----------------------------------------------------------------------------
  This source file is part of OGRE
  (Object-oriented Graphics Rendering Engine)
  For the latest info, see http://www.ogre3d.org/

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

#ifndef __TestContext_H__
#define __TestContext_H__

#include "VisualTest.h"
#include "SampleContext.h"
#include "SamplePlugin.h"

#include <iostream> // for Apple

class TestBatch;
using namespace Ogre;

typedef std::map<String, OgreBites::SamplePlugin *> PluginMap;

/** The common environment that all of the tests run in */
class TestContext : public OgreBites::SampleContext
{
 public:

    TestContext(int argc = 0, char** argv = 0);
    virtual ~TestContext();

    /** Does basic setup for the context */
    virtual void setup();

    /** Frame listener callback, handles updating of the tests at the start of frames
     *        @param evt The frame event (passed in for the framelistener) */
    virtual bool frameStarted(const FrameEvent& evt);

    /** Frame listener callback, handles updating of the tests at the end of frames
     *        @param evt The frame event (passed in for the framelistener) */
    virtual bool frameEnded(const FrameEvent& evt);

    /** Runs a given test or sample
     *        @param s The OgreBites::Sample to run
     *        @remarks If s is a VisualTest, then timing and rand will be setup for
     *            determinism. */
    virtual void runSample(OgreBites::Sample* s);

    /** Loads test plugins
     *        @param set The name of the test set to load
     *        @return The initial tets or sample to run */
    OgreBites::Sample* loadTests(String set);

    /** Setup the Root */
    virtual void createRoot();

    /** Start it up */
    virtual void go(OgreBites::Sample* initialSample = 0);

    /** Handles the config dialog */
    virtual bool oneTimeConfig();

    /** Set up directories for the tests to output to */
    virtual void setupDirectories(String batchName);

    /** Called after tests successfully complete, generates output */
    virtual void finishedTests();

    /** Sets the timstep value
     *        @param timestep The time to simulate elapsed between each frame
     *        @remarks Use with care! Screenshots produced at different timesteps
     *            will almost certainly turn out different. */
    void setTimestep(Real timestep);

    /** Gets the current timestep value */
    Real getTimestep();

    VisualTest* getCurrentTest() { return mCurrentTest; }

    /// Returns whether the entire test was successful or not.
    bool wasSuccessful() const {
        return mSuccess;
    }

 protected:
    bool mSuccess;

    /// The timestep
    Real mTimestep;

    /// Path to the test plugin directory
    String mPluginDirectory;

    /// List of available test sets
    std::map<String, StringVector> mTestSets;

    /// The tests to be run
    std::deque<OgreBites::Sample*> mTests;

    /// Path to the output directory for the running test
    String mOutputDir;

    /// Path to the reference set location
    String mReferenceSetPath;

    /// The active test (0 if none is active)
    VisualTest* mCurrentTest;

    /// The current frame of a running test
    unsigned int mCurrentFrame;

    /// Info about the running batch of tests
    TestBatch* mBatch;

    // A structure to map plugin names to class types
    PluginMap mPluginNameMap;

    // command line options
    // Is a reference set being generated?
    bool mReferenceSet;
    // Should html output be created?
    bool mGenerateHtml;
    // Force the config dialog
    bool mForceConfig;
    // Do not confine mouse to window
    bool mNoGrabMouse;
    // Show usage details
    bool mHelp;
    // Render system to use
    String mRenderSystemName;
    // Optional name for this batch
    String mBatchName;
    // Set to compare against
    String mCompareWith;
    // Optional comment
    String mComment;
    // Name of the test set to use
    String mTestSetName;
    // Location to output a test summary (used for CTest)
    String mSummaryOutputDir;
};

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS && defined(__OBJC__)
#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>

@interface AppDelegate : NSObject <UIApplicationDelegate>
{
    TestContext *tc;

    CADisplayLink *mDisplayLink;
    NSDate* mDate;
    NSTimeInterval mLastFrameTime;
}

- (void)go;
- (void)renderOneFrame:(id)sender;

@property (nonatomic) NSTimeInterval mLastFrameTime;

                      @end

                      @implementation AppDelegate

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
    NSArray * arguments = [[NSProcessInfo processInfo] arguments];
    char *argv[[arguments count]+1];
    int i = 0;
    for (NSString *str in arguments)
    {
        argv[i++] = (char *)[str UTF8String];
    }
    argv[i] = NULL;

    try {
        tc = new TestContext([arguments count], &argv[0]);
        tc->go();

        Root::getSingleton().getRenderSystem()->_initRenderTargets();

        // Clear event times
        Root::getSingleton().clearEventTimes();
    } catch( Exception& e ) {
        std::cerr << "An exception has occurred: " <<
            e.getFullDescription().c_str() << std::endl;
    }

    [pool release];
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    // Defaulting to 2 means that we run at 30 frames per second. For 60 frames, use a value of 1.
    // 30 FPS is usually sufficient and results in lower power consumption.
    mLastFrameTime = 2;
    mDisplayLink = nil;

    [self go];

    return YES;
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    tc->finishedTests();
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    // Reset event times and reallocate the date and displaylink objects
    Root::getSingleton().clearEventTimes();
    mDate = [[NSDate alloc] init];
    mLastFrameTime = 2; // Reset the timer

    mDisplayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(renderOneFrame:)];
    [mDisplayLink setFrameInterval:mLastFrameTime];
    [mDisplayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    Root::getSingleton().saveConfig();

    [mDate release];
    mDate = nil;

    [mDisplayLink invalidate];
    mDisplayLink = nil;
}

- (void)renderOneFrame:(id)sender
{
    // NSTimeInterval is a simple typedef for double
    NSTimeInterval currentFrameTime = -[mDate timeIntervalSinceNow];
    NSTimeInterval differenceInSeconds = currentFrameTime - mLastFrameTime;
    mLastFrameTime = currentFrameTime;

    dispatch_async(dispatch_get_main_queue(), ^(void)
                   {
                       Root::getSingleton().renderOneFrame((Real)differenceInSeconds);
                   });

    if(Root::getSingletonPtr() && Root::getSingleton().isInitialised() && !tc->getCurrentTest())
    {
        tc->finishedTests();

        // Force the app to exit. Acts like a crash which isn't very elegant but good enough in this case.
        exit(0);
    }
}

@end

#endif // OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS && defined(__OBJC__)

#endif
