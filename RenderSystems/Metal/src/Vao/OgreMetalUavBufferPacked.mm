/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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

#include "Vao/OgreMetalUavBufferPacked.h"
#include "Vao/OgreMetalBufferInterface.h"
#include "Vao/OgreMetalTexBufferPacked.h"

#include "OgreMetalDevice.h"

#import <Metal/MTLComputeCommandEncoder.h>
#import <Metal/MTLRenderCommandEncoder.h>

namespace Ogre
{
    MetalUavBufferPacked::MetalUavBufferPacked(
                size_t internalBufStartBytes, size_t numElements, uint32 bytesPerElement,
                uint32 bindFlags, void *initialData, bool keepAsShadow,
                VaoManager *vaoManager, MetalBufferInterface *bufferInterface,
                MetalDevice *device ) :
        UavBufferPacked( internalBufStartBytes, numElements, bytesPerElement, bindFlags,
                         initialData, keepAsShadow, vaoManager, bufferInterface ),
        mDevice( device )
    {
    }
    //-----------------------------------------------------------------------------------
    MetalUavBufferPacked::~MetalUavBufferPacked()
    {
    }
    //-----------------------------------------------------------------------------------
    TexBufferPacked* MetalUavBufferPacked::getAsTexBufferImpl( PixelFormat pixelFormat )
    {
        assert( dynamic_cast<MetalBufferInterface*>( mBufferInterface ) );

        MetalBufferInterface *bufferInterface = static_cast<MetalBufferInterface*>( mBufferInterface );


        TexBufferPacked *retVal = OGRE_NEW MetalTexBufferPacked(
                                                        mInternalBufferStart * mBytesPerElement,
                                                        mNumElements, mBytesPerElement, 0,
                                                        mBufferType, (void*)0, false,
                                                        (VaoManager*)0, bufferInterface, pixelFormat,
                                                        mDevice );

        mTexBufferViews.push_back( retVal );

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void MetalUavBufferPacked::bindBufferAllRenderStages( uint16 slot, size_t offset )
    {
        assert( mDevice->mRenderEncoder || mDevice->mFrameAborted );
        assert( dynamic_cast<MetalBufferInterface*>( mBufferInterface ) );
        MetalBufferInterface *bufferInterface = static_cast<MetalBufferInterface*>( mBufferInterface );

        [mDevice->mRenderEncoder setVertexBuffer:bufferInterface->getVboName()
                                          offset:mFinalBufferStart * mBytesPerElement + offset
                                         atIndex:slot + OGRE_METAL_UAV_SLOT_START];
        [mDevice->mRenderEncoder setFragmentBuffer:bufferInterface->getVboName()
                                            offset:mFinalBufferStart * mBytesPerElement + offset
                                           atIndex:slot + OGRE_METAL_UAV_SLOT_START];
    }
    //-----------------------------------------------------------------------------------
    void MetalUavBufferPacked::bindBufferVS( uint16 slot, size_t offset, size_t sizeBytes )
    {
        assert( mDevice->mRenderEncoder || mDevice->mFrameAborted );
        assert( dynamic_cast<MetalBufferInterface*>( mBufferInterface ) );
        MetalBufferInterface *bufferInterface = static_cast<MetalBufferInterface*>( mBufferInterface );

        [mDevice->mRenderEncoder setVertexBuffer:bufferInterface->getVboName()
                                          offset:mFinalBufferStart * mBytesPerElement + offset
                                         atIndex:slot + OGRE_METAL_UAV_SLOT_START];
    }
    //-----------------------------------------------------------------------------------
    void MetalUavBufferPacked::bindBufferPS( uint16 slot, size_t offset, size_t sizeBytes )
    {
        assert( mDevice->mRenderEncoder || mDevice->mFrameAborted );
        assert( dynamic_cast<MetalBufferInterface*>( mBufferInterface ) );
        MetalBufferInterface *bufferInterface = static_cast<MetalBufferInterface*>( mBufferInterface );

        [mDevice->mRenderEncoder setFragmentBuffer:bufferInterface->getVboName()
                                            offset:mFinalBufferStart * mBytesPerElement + offset
                                           atIndex:slot + OGRE_METAL_UAV_SLOT_START];
    }
    //-----------------------------------------------------------------------------------
    void MetalUavBufferPacked::bindBufferCS( uint16 slot, size_t offset, size_t sizeBytes )
    {
        assert( dynamic_cast<MetalBufferInterface*>( mBufferInterface ) );

        __unsafe_unretained id<MTLComputeCommandEncoder> computeEncoder =
                mDevice->getComputeEncoder();
        MetalBufferInterface *bufferInterface = static_cast<MetalBufferInterface*>( mBufferInterface );

        [computeEncoder setBuffer:bufferInterface->getVboName()
                           offset:mFinalBufferStart * mBytesPerElement + offset
                          atIndex:slot + OGRE_METAL_CS_UAV_SLOT_START];
    }
}
