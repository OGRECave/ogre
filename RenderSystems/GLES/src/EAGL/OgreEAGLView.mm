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
--------------------------------------------------------------------------*/

#include "OgreEAGLView.h"

#include "OgreGLESPrerequisites.h"

#include "OgreRoot.h"
#include "OgreRenderWindow.h"
#include "OgreGLESRenderSystem.h"

#import <QuartzCore/QuartzCore.h>

using namespace Ogre;

@implementation EAGLView

@synthesize mWindowName;

- (NSString *)description
{
    return [NSString stringWithFormat:@"EAGLView frame dimensions x: %.0f y: %.0f w: %.0f h: %.0f", 
            [self frame].origin.x,
            [self frame].origin.y,
            [self frame].size.width,
            [self frame].size.height];
}

+ (Class)layerClass
{
    return [CAEAGLLayer class];
}

- (void)layoutSubviews
{
    // Change the viewport orientation based upon the current device orientation.
    // Note: This only operates on the main viewport, usually the main view.

    [[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
    UIDeviceOrientation deviceOrientation = [UIDevice currentDevice].orientation;
    [[UIDevice currentDevice] endGeneratingDeviceOrientationNotifications];

    // Return if the orientation is not a valid interface orientation(face up, face down)
    if(!UIDeviceOrientationIsValidInterfaceOrientation(deviceOrientation))
        return;

    // Check if orientation is supported
    NSString *rotateToOrientation = @"";
    if(deviceOrientation == UIInterfaceOrientationPortrait)
        rotateToOrientation = @"UIInterfaceOrientationPortrait";
    else if(deviceOrientation == UIInterfaceOrientationPortraitUpsideDown)
        rotateToOrientation = @"UIInterfaceOrientationPortraitUpsideDown";
    else if(deviceOrientation == UIInterfaceOrientationLandscapeLeft)
        rotateToOrientation = @"UIInterfaceOrientationLandscapeLeft";
    else if(deviceOrientation == UIInterfaceOrientationLandscapeRight)
        rotateToOrientation = @"UIInterfaceOrientationLandscapeRight";

    NSArray *supportedOrientations = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"UISupportedInterfaceOrientations"];

    BOOL supported = [supportedOrientations containsObject:rotateToOrientation];

    if (!supported)
        return;

    // Get the window using the name that we saved
    RenderWindow *window = static_cast<RenderWindow *>(Root::getSingleton().getRenderSystem()->getRenderTarget(mWindowName));

    if(window != NULL)
    {
        // Get the window size and initialize temp variables
        unsigned int w = 0, h = 0;
        unsigned int width = self.bounds.size.width;
        unsigned int height = self.bounds.size.height;

        Ogre::Viewport *viewPort = window->getViewport(0);

        if (UIDeviceOrientationIsLandscape(deviceOrientation))
        {
            w = std::max(width, height);
            h = std::min(width, height);
        }
        else
        {
            h = std::max(width, height);
            w = std::min(width, height);
        }

        width = w;
        height = h;

        // Resize the window
        window->resize(width, height);

        // After rotation the aspect ratio of the viewport has changed, update that as well.
        viewPort->getCamera()->setAspectRatio((Real) width / (Real) height);
    }
}

@end
