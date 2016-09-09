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

#include "Vao/OgreMetalTexBufferPacked.h"
#include "Vao/OgreMetalBufferInterface.h"

#include "OgreMetalDevice.h"

#import <Metal/MTLRenderCommandEncoder.h>
#import <Metal/MTLComputeCommandEncoder.h>

namespace Ogre
{
    MetalTexBufferPacked::MetalTexBufferPacked(
                size_t internalBufStartBytes, size_t numElements, uint32 bytesPerElement,
                uint32 numElementsPadding, BufferType bufferType,
                void *initialData, bool keepAsShadow,
                VaoManager *vaoManager, MetalBufferInterface *bufferInterface, PixelFormat pf,
                MetalDevice *device ) :
        TexBufferPacked( internalBufStartBytes, numElements, bytesPerElement, numElementsPadding,
                         bufferType, initialData, keepAsShadow, vaoManager, bufferInterface, pf ),
        mDevice( device )
    {
    }
    //-----------------------------------------------------------------------------------
    MetalTexBufferPacked::~MetalTexBufferPacked()
    {
    }
    //-----------------------------------------------------------------------------------
    void MetalTexBufferPacked::bindBufferVS( uint16 slot, size_t offset, size_t sizeBytes )
    {
        assert( mDevice->mRenderEncoder || mDevice->mFrameAborted );
        assert( dynamic_cast<MetalBufferInterface*>( mBufferInterface ) );
        MetalBufferInterface *bufferInterface = static_cast<MetalBufferInterface*>( mBufferInterface );

        [mDevice->mRenderEncoder setVertexBuffer:bufferInterface->getVboName()
                                          offset:mFinalBufferStart * mBytesPerElement + offset
                                         atIndex:slot + OGRE_METAL_TEX_SLOT_START];
    }
    //-----------------------------------------------------------------------------------
    void MetalTexBufferPacked::bindBufferPS( uint16 slot, size_t offset, size_t sizeBytes )
    {
        assert( mDevice->mRenderEncoder || mDevice->mFrameAborted );
        assert( dynamic_cast<MetalBufferInterface*>( mBufferInterface ) );
        MetalBufferInterface *bufferInterface = static_cast<MetalBufferInterface*>( mBufferInterface );

        [mDevice->mRenderEncoder setFragmentBuffer:bufferInterface->getVboName()
                                            offset:mFinalBufferStart * mBytesPerElement + offset
                                           atIndex:slot + OGRE_METAL_TEX_SLOT_START];
    }
    //-----------------------------------------------------------------------------------
    void MetalTexBufferPacked::bindBufferCS( uint16 slot, size_t offset, size_t sizeBytes )
    {
        assert( dynamic_cast<MetalBufferInterface*>( mBufferInterface ) );

        __unsafe_unretained id<MTLComputeCommandEncoder> computeEncoder =
                mDevice->getComputeEncoder();
        MetalBufferInterface *bufferInterface = static_cast<MetalBufferInterface*>( mBufferInterface );

        [computeEncoder setBuffer:bufferInterface->getVboName()
                           offset:mFinalBufferStart * mBytesPerElement + offset
                          atIndex:slot + OGRE_METAL_CS_TEX_SLOT_START];
    }
}
