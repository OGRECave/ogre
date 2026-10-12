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

#ifndef __SampleBrowser_iOS_H__
#define __SampleBrowser_iOS_H__

#include "OgrePlatform.h"

#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
#error This header is for use with iOS only
#endif

#import <UIKit/UIKit.h> 
#import <QuartzCore/QuartzCore.h>

// Defaulting to 2 means that we run at 30 frames per second. For 60 frames, use a value of 1.
// 30 FPS is usually sufficient and results in lower power consumption.
#define DISPLAYLINK_FRAME_INTERVAL      2

#ifdef __OBJC__

namespace OgreBites
{
    class SampleBrowser;
}

@interface SampleBrowserGestureView : UIView
{
    OgreBites::SampleBrowser *mBrowser;
    void *mActiveTouch;   // single tracked UITouch (unretained, MRC)
}
@property (assign) OgreBites::SampleBrowser *mBrowser;

- (void)attachToWindow:(UIWindow *)window;

@end

#import "SampleBrowser.h"

@interface AppDelegate : NSObject <UIApplicationDelegate>
{
    OgreBites::SampleBrowser sb;

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

- (void)setMLastFrameTime:(NSTimeInterval)frameInterval
{
    // Frame interval defines how many display frames must pass between each time the
    // display link fires. A frame interval setting of less than one results in
    // undefined behavior.
    if (frameInterval >= 1)
    {
        mLastFrameTime = frameInterval;
    }
}

- (void)go {

    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

    try {
        sb.go();
    } catch( Ogre::Exception& e ) {
        std::cerr << "An exception has occurred: " <<
        e.getFullDescription().c_str() << std::endl;
    }

    [pool release];
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    mLastFrameTime = DISPLAYLINK_FRAME_INTERVAL;
    mDisplayLink = nil;

    [self go];

    return YES;
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    sb.shutdown();
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    // Reset event times and reallocate the date and displaylink objects
    Ogre::Root::getSingleton().clearEventTimes();
    mDate = [[NSDate alloc] init];
    mLastFrameTime = DISPLAYLINK_FRAME_INTERVAL; // Reset the timer

    mDisplayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(renderOneFrame:)];
    [mDisplayLink setFrameInterval:mLastFrameTime];
    [mDisplayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    Ogre::Root::getSingleton().saveConfig();

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
        Ogre::Root::getSingleton().renderOneFrame((Ogre::Real)differenceInSeconds);
    });
}

@end

@implementation SampleBrowserGestureView

@synthesize mBrowser;

- (id)initWithFrame:(CGRect)frame
{
    if ((self = [super initWithFrame:frame]))
    {
        mBrowser     = 0;
        mActiveTouch = NULL;

        self.userInteractionEnabled = YES;
        self.multipleTouchEnabled   = NO;    // single touch only
        self.opaque                 = NO;
        self.backgroundColor        = [UIColor clearColor];
        self.autoresizingMask       = UIViewAutoresizingFlexibleWidth |
                                      UIViewAutoresizingFlexibleHeight;
    }
    return self;
}

- (id)init
{
    return [self initWithFrame:[[UIScreen mainScreen] bounds]];
}

- (void)attachToWindow:(UIWindow *)window
{
    if (!window) return;

    self.frame = window.bounds;
    [window addSubview:self];
    [window bringSubviewToFront:self];
}

- (BOOL)canBecomeFirstResponder
{
    return YES;
}

- (void)dealloc {
    [super dealloc];
}

// SampleBrowser implements only the mouse* handlers. Its TouchAgnosticInputListenerChain
// converts finger events into mouse events and de-normalises the coordinates using the
// render window size, so x/y/dx/dy must be normalised to 0..1 here (no contentScaleFactor).
- (void)dispatchTouch:(UITouch *)touch ofType:(int)type
{
    if (!mBrowser || !touch) return;

    CGSize sz = self.bounds.size;
    if (sz.width <= 0.0f || sz.height <= 0.0f) return;

    CGPoint p    = [touch locationInView:self];
    CGPoint prev = [touch previousLocationInView:self];

    OgreBites::Event evt = {};
    evt.type             = type;
    evt.tfinger.type     = type;
    evt.tfinger.fingerId = 0;
    evt.tfinger.x        = (float)(p.x / sz.width);
    evt.tfinger.y        = (float)(p.y / sz.height);
    evt.tfinger.dx       = (float)((p.x - prev.x) / sz.width);
    evt.tfinger.dy       = (float)((p.y - prev.y) / sz.height);

    mBrowser->_fireInputEvent(evt, 0);
}

- (BOOL)isActiveTouchIn:(NSSet *)touches
{
    for (UITouch *t in touches)
        if ((void *)t == mActiveTouch)
            return YES;
    return NO;
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    if (mActiveTouch) return;            // already tracking a finger: ignore

    UITouch *t = [touches anyObject];
    if (!t) return;

    mActiveTouch = (void *)t;
    [self dispatchTouch:t ofType:OgreBites::FINGERDOWN];
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    if (![self isActiveTouchIn:touches]) return;
    [self dispatchTouch:(UITouch *)mActiveTouch ofType:OgreBites::FINGERMOTION];
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    if (![self isActiveTouchIn:touches]) return;
    [self dispatchTouch:(UITouch *)mActiveTouch ofType:OgreBites::FINGERUP];
    mActiveTouch = NULL;
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
    if (![self isActiveTouchIn:touches]) return;
    [self dispatchTouch:(UITouch *)mActiveTouch ofType:OgreBites::FINGERUP];
    mActiveTouch = NULL;
}
@end

#endif

#endif
