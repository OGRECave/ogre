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

#include "OgreMetalRenderWindow.h"
#include "OgreMetalMappings.h"
#include "OgreMetalRenderSystem.h"
#include "OgreViewport.h"

namespace Ogre
{
    MetalRenderWindow::MetalRenderWindow( MetalDevice *ownerDevice, MetalRenderSystem *renderSystem ) :
        RenderWindow(),
        MetalRenderTargetCommon( ownerDevice ),
        mMetalLayer( 0 ),
        mCurrentDrawable( 0 ),
        mMsaaTex( 0 ),
        mContentScalingFactor(1)
    {
        mIsFullScreen = false;
        mActive = false;
        mFSAA = 1;
    }
    //-------------------------------------------------------------------------
    MetalRenderWindow::~MetalRenderWindow()
    {
        destroy();
    }
    //-------------------------------------------------------------------------
    inline void MetalRenderWindow::checkLayerSizeChanges(void)
    {
        // Handle display changes here
        if( mMetalView.layerSizeDidUpdate )
        {
            // set the metal layer to the drawable size in case orientation or size changes
            CGSize drawableSize = CGSizeMake(mMetalView.bounds.size.width, mMetalView.bounds.size.height);
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
            drawableSize.width  *= mMetalView.layer.contentsScale;
            drawableSize.height *= mMetalView.layer.contentsScale;
#else
            NSScreen* screen = mMetalView.window.screen ?: [NSScreen mainScreen];
            drawableSize.width *= screen.backingScaleFactor;
            drawableSize.height *= screen.backingScaleFactor;
#endif
            mMetalLayer.drawableSize = drawableSize;

            // Resize anything if needed
            this->windowMovedOrResized();

            mMetalView.layerSizeDidUpdate = NO;
        }
    }
    //-------------------------------------------------------------------------
    void MetalRenderWindow::swapBuffers(void)
    {
        /*assert( (mOwnerDevice->mFrameAborted ||
                mColourAttachmentDesc.loadAction != MTLLoadActionClear) &&
                "A clear has been asked but no rendering command was "
                "issued and Ogre didn't catch it." );*/

        RenderWindow::swapBuffers();

        // Do not retain current drawable's texture beyond the frame.
        if( mFSAA > 1 )
            mColourAttachmentDesc.resolveTexture = 0;
        else
            mColourAttachmentDesc.texture = 0;

        if( !mOwnerDevice->mFrameAborted )
        {
            // Schedule a present once rendering to the framebuffer is complete
            const CFTimeInterval presentationTime = mMetalView.presentationTime;

            if( presentationTime < 0 )
            {
                [mOwnerDevice->mCurrentCommandBuffer presentDrawable:mCurrentDrawable];
            }
            else
            {
                [mOwnerDevice->mCurrentCommandBuffer presentDrawable:mCurrentDrawable
                                                              atTime:presentationTime];
            }
        }
        mCurrentDrawable = 0;
    }
    //-------------------------------------------------------------------------
    void MetalRenderWindow::windowMovedOrResized(void)
    {
        if( mWidth != mMetalLayer.drawableSize.width ||
            mHeight != mMetalLayer.drawableSize.height )
        {
            mWidth  = mMetalLayer.drawableSize.width;
            mHeight = mMetalLayer.drawableSize.height;

            if( mFSAA > 1 && mWidth > 0 && mHeight > 0 )
            {
                mMsaaTex = 0;
                mColourAttachmentDesc.texture = 0;
                MTLTextureDescriptor* desc = [MTLTextureDescriptor
                        texture2DDescriptorWithPixelFormat:
                        MetalMappings::getPixelFormat( mFormat, mHwGamma )
                        width: mWidth height: mHeight mipmapped: NO];
                desc.textureType = MTLTextureType2DMultisample;
                desc.sampleCount = mFSAA;
                desc.usage = MTLTextureUsageRenderTarget;

                mMsaaTex = [mOwnerDevice->mDevice newTextureWithDescriptor: desc];
                mColourAttachmentDesc.texture = mMsaaTex;
            }

            detachDepthBuffer();

            ViewportList::iterator itor = mViewportList.begin();
            ViewportList::iterator end  = mViewportList.end();
            while( itor != end )
            {
                (*itor).second->_updateDimensions();
                ++itor;
            }
        }
    }

    bool MetalRenderWindow::nextDrawable(void)
    {
        bool isSuccess = true;

        @autoreleasepool
        {
            if( !mCurrentDrawable )
            {
                if( mMetalView.layerSizeDidUpdate )
                    checkLayerSizeChanges();

                // do not retain current drawable beyond the frame.
                // There should be no strong references to this object outside of this view class
                mCurrentDrawable = [mMetalLayer nextDrawable];
                if( !mCurrentDrawable )
                {
                    LogManager::getSingleton().logMessage( "Metal ERROR: Failed to get a drawable!",
                                                           LML_CRITICAL );
                    //We're unable to render. Skip frame.
                    //dispatch_semaphore_signal( _inflight_semaphore );

                    isSuccess = false;
                }
                else
                {
                    if( mFSAA > 1 )
                        mColourAttachmentDesc.resolveTexture = mCurrentDrawable.texture;
                    else
                        mColourAttachmentDesc.texture = mCurrentDrawable.texture;
                }
            }
        }

        return isSuccess;
    }
    //-------------------------------------------------------------------------
    void MetalRenderWindow::create( const String& name, unsigned int width, unsigned int height,
                                    bool fullScreen, const NameValuePairList *miscParams )
    {
        mWidth  = width;
        mHeight = height;

        mName   = name;
        mIsFullScreen = fullScreen;

        mActive = true;
        mClosed = false;

        mFormat = PF_B8G8R8A8;
        mHwGamma = true;

        if( miscParams )
        {
            NameValuePairList::const_iterator opt;
            NameValuePairList::const_iterator end = miscParams->end();

            opt = miscParams->find("FSAA");
            if( opt != end )
                mFSAA = std::max( 1u, StringConverter::parseUnsignedInt( opt->second ) );

            opt = miscParams->find("gamma");
            if( opt != end )
                mHwGamma = StringConverter::parseBool( opt->second );

#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
            opt = miscParams->find("externalWindowHandle");
            if( opt != end )
            {
                LogManager::getSingleton().logMessage("Mac Cocoa Window: Rendering on an external NSWindow*");
                mWindow = (__bridge NSWindow*)reinterpret_cast<void*>(StringConverter::parseSizeT(opt->second));
                assert( mWindow &&
                       "Unable to get a pointer to the parent NSWindow."
                       "Was the 'externalWindowHandle' parameter set correctly in the call to createRenderWindow()?");
            }
#endif
        }

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        CGRect frame;
#else
        NSRect frame;
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        frame.origin.x = 0;
        frame.origin.y = 0;
        frame.size.width = mWidth;
        frame.size.height = mHeight;
#else
        frame = [mWindow.contentView bounds];
        mContentScalingFactor = mWindow.backingScaleFactor;
#endif
        mMetalView = [[OgreMetalView alloc] initWithFrame:frame];

#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
        NSView *view = mWindow.contentView;
        [view addSubview:mMetalView];
        mResizeObserver = [[NSNotificationCenter defaultCenter] addObserverForName:NSWindowDidResizeNotification object:mWindow queue:nil usingBlock:^(NSNotification *){
          mMetalView.frame = [mWindow.contentView bounds];
        }];
#endif

        mMetalLayer = (CAMetalLayer*)mMetalView.layer;
        mMetalLayer.device      = mOwnerDevice->mDevice;
        mMetalLayer.pixelFormat = MetalMappings::getPixelFormat( mFormat, mHwGamma );

        //This is the default but if we wanted to perform compute
        //on the final rendering layer we could set this to no
        mMetalLayer.framebufferOnly = YES;

        this->init( nil, nil );

#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
        checkLayerSizeChanges();
#endif
        windowMovedOrResized();
    }
    //-------------------------------------------------------------------------
    void MetalRenderWindow::destroy()
    {
#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
        [[NSNotificationCenter defaultCenter] removeObserver:mResizeObserver];
#endif
        mActive = false;
        mClosed = true;

        mMetalLayer = 0;
        mCurrentDrawable = 0;
        mMsaaTex = 0;
        mMetalView = 0;
    }
    //-------------------------------------------------------------------------
    void MetalRenderWindow::resize( unsigned int width, unsigned int height )
    {
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        CGRect frame = mMetalView.frame;
        frame.size.width    = width;
        frame.size.height   = height;
        mMetalView.frame = frame;
#else
        mMetalView.frame = [mWindow.contentView bounds];
#endif
        detachDepthBuffer();
    }
    //-------------------------------------------------------------------------
    void MetalRenderWindow::reposition( int left, int top )
    {
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        CGRect frame = mMetalView.frame;
        frame.origin.x = left;
        frame.origin.y = top;
        mMetalView.frame = frame;
#else
        mMetalView.frame = [mWindow.contentView bounds];
#endif
    }
    //-------------------------------------------------------------------------
    void MetalRenderWindow::getCustomAttribute( const String& name, void* pData )
    {
        if( name == "mNumMRTs" )
        {
            *static_cast<uint8*>(pData) = 1u;
        }
        else if( name == "UIView" )
        {
            *static_cast<void**>(pData) = const_cast<void*>((const void*)CFBridgingRetain( mMetalView ));
        }
        else
        {
            RenderTarget::getCustomAttribute( name, pData );
        }
    }
}
