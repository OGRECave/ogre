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

// These need to be included prior to everything else to prevent name clashes.
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE && defined(__OBJC__)

#import <AppKit/AppKit.h>
#include <crt_externs.h>

extern int *_NSGetArgc(void);
extern char ***_NSGetArgv(void);
#endif

class TestBatch;
using namespace Ogre;

typedef std::map<String, OgreBites::SamplePlugin *> PluginMap;

#ifdef INCLUDE_RTSHADER_SYSTEM

/** This class demonstrates basic usage of the RTShader system.
    It sub class the material manager listener class and when a target scheme callback
    is invoked with the shader generator scheme it tries to create an equivalent shader
    based technique based on the default technique of the given material.
*/
class ShaderGeneratorTechniqueResolverListener : public MaterialManager::Listener
{
 public:

    ShaderGeneratorTechniqueResolverListener(RTShader::ShaderGenerator* pShaderGenerator)
    {
        mShaderGenerator = pShaderGenerator;
    }

    /** This is the hook point where shader based technique will be created.
        It will be called whenever the material manager won't find appropriate technique
        that satisfy the target scheme name. If the scheme name is out target RT Shader System
        scheme name we will try to create shader generated technique for it.
    */
    virtual Technique* handleSchemeNotFound(unsigned short schemeIndex,
                                            const String& schemeName, Material* originalMaterial, unsigned short lodIndex,
                                            const Renderable* rend)
    {
        Technique* generatedTech = NULL;

        // Case this is the default shader generator scheme.
        if (schemeName == RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME)
        {
            bool techniqueCreated;

            // Create shader generated technique for this material.
            techniqueCreated = mShaderGenerator->createShaderBasedTechnique(
                originalMaterial->getName(),
                MaterialManager::DEFAULT_SCHEME_NAME,
                schemeName);

            // Case technique registration succeeded.
            if (techniqueCreated)
            {
                // Force creating the shaders for the generated technique.
                mShaderGenerator->validateMaterial(schemeName, originalMaterial->getName());

                // Grab the generated technique.
                Material::TechniqueIterator itTech = originalMaterial->getTechniqueIterator();

                while (itTech.hasMoreElements())
                {
                    Technique* curTech = itTech.getNext();

                    if (curTech->getSchemeName() == schemeName)
                    {
                        generatedTech = curTech;
                        break;
                    }
                }
            }
        }

        return generatedTech;
    }

 protected:
    RTShader::ShaderGenerator* mShaderGenerator; // The shader generator instance.
};
#endif // INCLUDE_RTSHADER_SYSTEM

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

    void createDummyScene();
    void destroyDummyScene();
    bool initialiseRTShaderSystem(SceneManager* sceneMgr);
    void finaliseRTShaderSystem();

    /** Sets the timstep value
     *        @param timestep The time to simulate elapsed between each frame
     *        @remarks Use with care! Screenshots produced at different timesteps
     *            will almost certainly turn out different. */
    void setTimestep(Real timestep);

    /** Gets the current timestep value */
    Real getTimestep();

    VisualTest* getCurrentTest() { return mCurrentTest; }

#if (OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS) || (OGRE_PLATFORM == OGRE_PLATFORM_ANDROID)
    virtual bool touchCancelled(const OIS::MultiTouchEvent& evt)
    {
        return true;
    }
    virtual bool touchReleased(const OIS::MultiTouchEvent& evt)
    {
        return true;
    }
    virtual bool touchMoved(const OIS::MultiTouchEvent& evt)
    {
        return true;
    }
    virtual bool touchPressed(const OIS::MultiTouchEvent& evt)
    {
        return true;
    }
#endif

 protected:

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
#ifdef INCLUDE_RTSHADER_SYSTEM
    RTShader::ShaderGenerator* mShaderGenerator; // The Shader generator instance.
    ShaderGeneratorTechniqueResolverListener* mMaterialMgrListener;             // Shader generator material manager listener.
#endif // INCLUDE_RTSHADER_SYSTEM

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

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE && defined(__OBJC__)

// All this does is suppress some messages in the run log.  NSApplication does not
// implement buttonPressed and apps without a NIB have no target for the action.
@implementation NSApplication (_suppressUnimplementedActionWarning)
- (void) buttonPressed:(id)sender
{
    /* Do nothing */
}
@end


#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
@interface AppDelegate : NSObject <NSApplicationDelegate>
#else
@interface AppDelegate : NSObject
#endif
{
    NSTimer *mTimer;

    NSTimeInterval mLastFrameTime;
    TestContext *tc;
}

- (void)go;
- (void)renderOneFrame:(id)sender;
- (void)shutdown;

@property (retain) NSTimer *mTimer;
                   @property (nonatomic) NSTimeInterval mLastFrameTime;

                                         @end

                                         //static id mAppDelegate;

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
        tc = new TestContext(*_NSGetArgc(), *_NSGetArgv());
        tc->go();
        Root::getSingleton().getRenderSystem()->_initRenderTargets();

        // Clear event times
        Root::getSingleton().clearEventTimes();
    } catch( Exception& e ) {
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

- (void)shutdown {

    [NSApp terminate:nil];
}

- (void)renderOneFrame:(id)sender
{
    if(Root::getSingletonPtr() && Root::getSingleton().isInitialised() && tc->getCurrentTest())
    {
        Root::getSingleton().renderOneFrame((Real)[mTimer timeInterval]);
    }
    else
    {
        if(mTimer)
        {
            [mTimer invalidate];
            mTimer = nil;
        }

        tc->finishedTests();

        [NSApp performSelector:@selector(terminate:) withObject:nil afterDelay:0.0];
    }
}

@end
#elif OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS && defined(__OBJC__)
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
