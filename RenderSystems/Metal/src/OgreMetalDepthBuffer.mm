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
#include "OgreMetalDepthBuffer.h"
#include "OgreMetalRenderSystem.h"
#include "OgreRenderTarget.h"
#include "OgreMetalRenderTargetCommon.h"

#import <Metal/MTLRenderPass.h>
#import <Metal/MTLBlitCommandEncoder.h>

namespace Ogre
{
    MetalDepthBuffer::MetalDepthBuffer( uint16 poolId, MetalRenderSystem *renderSystem,
                                        uint32 width, uint32 height, uint32 fsaa,
                                        uint32 multiSampleQuality, PixelFormat pixelFormat,
                                        bool isDepthTexture, bool _isManual,
                                        id<MTLTexture> depthTexture, id<MTLTexture> stencilTexture,
                                        MetalDevice *device ) :
        DepthBuffer( poolId, 0, width, height, fsaa, "",
                     _isManual ),
        mDepthAttachmentDesc( 0 ),
        mStencilAttachmentDesc( 0 ),
        mDevice( device )
    {
        if( depthTexture )
        {
            mDepthAttachmentDesc = [MTLRenderPassDepthAttachmentDescriptor new];
            mDepthAttachmentDesc.loadAction = MTLLoadActionDontCare;
            mDepthAttachmentDesc.storeAction = MTLStoreActionStore;
            mDepthAttachmentDesc.texture = depthTexture;
        }

        if( stencilTexture )
        {
            mStencilAttachmentDesc = [MTLRenderPassStencilAttachmentDescriptor new];
            mStencilAttachmentDesc.loadAction = MTLLoadActionDontCare;
            mStencilAttachmentDesc.storeAction = MTLStoreActionStore;
            mStencilAttachmentDesc.texture = stencilTexture;
        }
    }
    //-----------------------------------------------------------------------------------
    MetalDepthBuffer::~MetalDepthBuffer()
    {
        mDepthAttachmentDesc = 0;
        mStencilAttachmentDesc = 0;
    }
    //-----------------------------------------------------------------------------------
    bool MetalDepthBuffer::isCompatible( RenderTarget *renderTarget) const
    {
        //First check they belong to the same GPU device.
        MetalDevice *device = 0;
        renderTarget->getCustomAttribute( "MetalDevice", &device );

        if( device == mDevice && renderTarget->suggestPixelFormat() != PF_UNKNOWN &&
            this->getWidth() == renderTarget->getWidth() &&
            this->getHeight() == renderTarget->getHeight() &&
            this->getFSAA() == renderTarget->getFSAA()// &&
            // mDepthTexture == renderTarget->prefersDepthTexture() &&
            // mFormat == renderTarget->getDesiredDepthBufferFormat()
            )
        {
            return true;
        }

        return false;
    }
    //-----------------------------------------------------------------------------------
    bool MetalDepthBuffer::copyToImpl( DepthBuffer *destination )
    {
        MetalDepthBuffer *dstDepthBuffer = static_cast<MetalDepthBuffer*>( destination );

        __unsafe_unretained id<MTLBlitCommandEncoder> blitEncoder = mDevice->getBlitEncoder();
        [blitEncoder copyFromTexture:mDepthAttachmentDesc.texture
                         sourceSlice:0
                         sourceLevel:0
                        sourceOrigin:MTLOriginMake( 0, 0, 0 )
                          sourceSize:MTLSizeMake( mWidth, mHeight, 1u )
                           toTexture:dstDepthBuffer->mDepthAttachmentDesc.texture
                    destinationSlice:0
                    destinationLevel:0
                   destinationOrigin:MTLOriginMake( 0, 0, 0 )];

        return true;
    }
}
