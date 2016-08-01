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
        mRenderSystem( renderSystem )
    {
        mIsFullScreen = false;
        mActive = false;
        mClosed = false;
        mFSAA = 1;
    }
    //-------------------------------------------------------------------------
    MetalRenderWindow::~MetalRenderWindow()
    {
        destroy();
    }
    //-------------------------------------------------------------------------
    void MetalRenderWindow::_metalUpdate(void)
    {
        // Handle display changes here
        if( mMetalView.layerSizeDidUpdate )
        {
            // set the metal layer to the drawable size in case orientation or size changes
            CGSize drawableSize = mMetalView.bounds.size;
            drawableSize.width  *= mMetalView.contentScaleFactor;
            drawableSize.height *= mMetalView.contentScaleFactor;

            mMetalLayer.drawableSize = drawableSize;

            // Resize anything if needed
            this->windowMovedOrResized();

            mMetalView.layerSizeDidUpdate = NO;
        }

        // do not retain current drawable beyond the frame.
        // There should be no strong references to this object outside of this view class
        mCurrentDrawable = [mMetalLayer nextDrawable];
        if( !mCurrentDrawable )
        {
            LogManager::getSingleton().logMessage( "Metal ERROR: Failed to get a drawable!",
                                                   LML_CRITICAL );
            //We're unable to render. Skip frame.
            //dispatch_semaphore_signal( _inflight_semaphore );
        }
        else
        {
            if( mFSAA > 1 )
                mColourAttachmentDesc.resolveTexture = mCurrentDrawable.texture;
            else
                mColourAttachmentDesc.texture = mCurrentDrawable.texture;
        }
    }
    //-------------------------------------------------------------------------
    void MetalRenderWindow::swapBuffers(void)
    {
        assert( mColourAttachmentDesc.loadAction != MTLLoadActionClear &&
                "A clear has been asked but no rendering command was "
                "issued and Ogre didn't catch it." );

        RenderWindow::swapBuffers();

        // Do not retain current drawable's texture beyond the frame.
        if( mFSAA > 1 )
            mColourAttachmentDesc.resolveTexture = 0;
        else
            mColourAttachmentDesc.texture = 0;

        // Schedule a present once rendering to the framebuffer is complete
        [mOwnerDevice->mCurrentCommandBuffer presentDrawable:mCurrentDrawable];
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
                MTLTextureDescriptor* desc = [MTLTextureDescriptor
                        texture2DDescriptorWithPixelFormat:
                        MetalMappings::getPixelFormat( mFormat, mHwGamma )
                        width: mWidth height: mHeight mipmapped: NO];
                desc.textureType = MTLTextureType2DMultisample;
                desc.sampleCount = mFSAA;

                mMsaaTex = [mOwnerDevice->mDevice newTextureWithDescriptor: desc];
            }

            detachDepthBuffer();

            ViewportList::iterator itor = mViewportList.begin();
            ViewportList::iterator end  = mViewportList.end();
            while( itor != end )
            {
                (*itor)->_updateDimensions();
                ++itor;
            }
        }
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

        CGRect frame;
        frame.origin.x = 0;
        frame.origin.y = 0;
        frame.size.width = mWidth;
        frame.size.height = mHeight;
        mMetalView = [[OgreMetalView alloc] initWithFrame:frame];
        mMetalLayer = (CAMetalLayer*)mMetalView.layer;

        mMetalLayer.device      = mOwnerDevice->mDevice;
        mMetalLayer.pixelFormat = MetalMappings::getPixelFormat( mFormat, mHwGamma );

        //This is the default but if we wanted to perform compute
        //on the final rendering layer we could set this to no
        mMetalLayer.framebufferOnly = YES;

        this->init( nil, nil );

        windowMovedOrResized();
    }
    //-------------------------------------------------------------------------
    void MetalRenderWindow::destroy()
    {
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
        CGRect frame = mMetalView.frame;
        frame.size.width    = width;
        frame.size.height   = height;
        mMetalView.frame = frame;

        detachDepthBuffer();
    }
    //-------------------------------------------------------------------------
    void MetalRenderWindow::reposition( int left, int top )
    {
        CGRect frame = mMetalView.frame;
        frame.origin.x = left;
        frame.origin.y = top;
        mMetalView.frame = frame;
    }
    //-------------------------------------------------------------------------
    bool MetalRenderWindow::isClosed(void) const
    {
        return mClosed;
    }
    //-------------------------------------------------------------------------
    void MetalRenderWindow::getCustomAttribute( const String& name, void* pData )
    {
        if( name == "MetalRenderTargetCommon" )
        {
            if( !mCurrentDrawable )
                _metalUpdate();
            *static_cast<MetalRenderTargetCommon**>(pData) = this;
        }
        else if( name == "MetalDevice" )
        {
            *static_cast<MetalDevice**>(pData) = this->getOwnerDevice();
        }
        else if( name == "UIView" )
        {
            *static_cast<void**>(pData) = (void*)CFBridgingRetain( mMetalView );
        }
        else
        {
            RenderTarget::getCustomAttribute( name, pData );
        }
    }
}
