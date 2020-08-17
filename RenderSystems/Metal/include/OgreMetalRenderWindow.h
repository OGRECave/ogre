/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2016 Torus Knot Software Ltd

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
#ifndef _OgreMetalRenderWindow_H_
#define _OgreMetalRenderWindow_H_

#include "OgreMetalPrerequisites.h"
#include "OgreRenderWindow.h"

#include "OgreMetalRenderTargetCommon.h"

#include "OgreMetalView.h"

#import <QuartzCore/CAMetalLayer.h>

namespace Ogre
{
    class MetalRenderWindow : public RenderWindow, public MetalRenderTargetCommon
    {
        bool    mClosed;

        CAMetalLayer        *mMetalLayer;
        id<CAMetalDrawable> mCurrentDrawable;
        id<MTLTexture>      mMsaaTex;
        OgreMetalView       *mMetalView;
#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
        NSWindow            *mWindow;
        id                  mResizeObserver;
#endif

        float mContentScalingFactor;

        inline void checkLayerSizeChanges(void);
    public:
        MetalRenderWindow( MetalDevice *ownerDevice, MetalRenderSystem *renderSystem );
        virtual ~MetalRenderWindow();

        virtual void swapBuffers(void);
        virtual void windowMovedOrResized(void);

        virtual bool nextDrawable(void);

        virtual void create( const String& name, unsigned int width, unsigned int height,
                             bool fullScreen, const NameValuePairList *miscParams );
        virtual void destroy(void);

        virtual void resize( unsigned int width, unsigned int height );
        virtual void reposition( int left, int top );

        virtual bool isClosed(void) const;

        // RenderTarget overloads.
        virtual void copyContentsToMemory(const Box& src, const PixelBox &dst, FrameBuffer buffer) {}
        virtual bool requiresTextureFlipping() const { return false; }

        virtual void getCustomAttribute( const String& name, void* pData );

        PixelFormat suggestPixelFormat() const override { return mFormat; }

        float getViewPointToPixelScale() override { return mContentScalingFactor; }
    };
}

#endif
