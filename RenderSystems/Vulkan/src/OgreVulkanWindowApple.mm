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

#import <Foundation/Foundation.h>
#import <QuartzCore/CAMetalLayer.h>
#include <TargetConditionals.h>

#if TARGET_OS_OSX
#import <AppKit/AppKit.h>
#endif

namespace Ogre
{
    /** Resolves the "externalWindowHandle" passed to VulkanWindow to the
        CAMetalLayer that VK_EXT_metal_surface needs.

        Accepted handles are a CAMetalLayer* (any Apple platform) and, on
        macOS, additionally an NSWindow* or NSView* - mirroring what the Metal
        RenderSystem accepts. If the view is not backed by a CAMetalLayer yet,
        one is attached; hosted by the view it then tracks the view size
        automatically. The attached layer renders at point resolution
        (contentsScale 1.0), consistent with the sizes the windowing toolkit
        reports for an external window; a HiDPI-aware host can instead pass
        its own CAMetalLayer with a higher contentsScale.

        Returns NULL if no CAMetalLayer could be derived from the handle. */
    void *getCAMetalLayer( size_t windowHandle )
    {
        id handle = (__bridge id)reinterpret_cast<void *>( windowHandle );

        if( [handle isKindOfClass:[CAMetalLayer class]] )
            return (__bridge void *)handle;

#if TARGET_OS_OSX
        NSView *view = nil;
        if( [handle isKindOfClass:[NSWindow class]] )
            view = ( (NSWindow *)handle ).contentView;
        else if( [handle isKindOfClass:[NSView class]] )
            view = (NSView *)handle;

        if( view )
        {
            if( ![view.layer isKindOfClass:[CAMetalLayer class]] )
            {
                // setLayer before wantsLayer makes the view layer-hosting:
                // AppKit leaves the layer's contentsScale alone
                [view setLayer:[CAMetalLayer layer]];
                [view setWantsLayer:YES];
            }
            return (__bridge void *)view.layer;
        }
#endif
        return NULL;
    }
}  // namespace Ogre
