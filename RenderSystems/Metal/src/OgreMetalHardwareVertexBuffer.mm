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

#include "OgreMetalHardwareBufferManager.h"
#include "OgreMetalHardwareVertexBuffer.h"
#include "OgreMetalDiscardBufferManager.h"

namespace Ogre {
    MetalHardwareVertexBuffer::MetalHardwareVertexBuffer( MetalHardwareBufferManager *mgr,
                                                          size_t vertexSize,
                                                          size_t numVertices,
                                                          HardwareBuffer::Usage usage,
                                                          bool useShadowBuffer ) :
        HardwareVertexBuffer( mgr, vertexSize, numVertices, usage, false, useShadowBuffer ),
        mMetalHardwareBufferCommon( mSizeInBytes, usage, 16, mgr->_getDiscardBufferManager(),
                                    mgr->_getDiscardBufferManager()->getDevice() )
    {
    }
    //-----------------------------------------------------------------------------------
    MetalHardwareVertexBuffer::~MetalHardwareVertexBuffer()
    {
    }
    //-----------------------------------------------------------------------------------
    void* MetalHardwareVertexBuffer::lockImpl( size_t offset, size_t length, LockOptions options )
    {
        return mMetalHardwareBufferCommon.lockImpl( offset, length, options, mIsLocked );
    }
    //-----------------------------------------------------------------------------------
    void MetalHardwareVertexBuffer::unlockImpl(void)
    {
        mMetalHardwareBufferCommon.unlockImpl( mLockStart, mLockSize );
    }
    //-----------------------------------------------------------------------------------
    void MetalHardwareVertexBuffer::_notifyDeviceStalled(void)
    {
        mMetalHardwareBufferCommon._notifyDeviceStalled();
    }
    //-----------------------------------------------------------------------------------
    id<MTLBuffer> MetalHardwareVertexBuffer::getBufferName( size_t &outOffset )
    {
        return mMetalHardwareBufferCommon.getBufferName( outOffset );
    }
    //-----------------------------------------------------------------------------------
    id<MTLBuffer> MetalHardwareVertexBuffer::getBufferNameForGpuWrite(void)
    {
        return mMetalHardwareBufferCommon.getBufferNameForGpuWrite();
    }
    //-----------------------------------------------------------------------------------
    void MetalHardwareVertexBuffer::readData( size_t offset, size_t length, void* pDest )
    {
        if( mUseShadowBuffer )
        {
            mShadowBuffer->readData(offset, length, pDest);
        }
        else
        {
            mMetalHardwareBufferCommon.readData( offset, length, pDest );
        }
    }
    //-----------------------------------------------------------------------------------
    void MetalHardwareVertexBuffer::writeData( size_t offset, size_t length,
                                               const void* pSource, bool discardWholeBuffer )
    {
        // Update the shadow buffer
        if( mUseShadowBuffer )
        {
            mShadowBuffer->writeData(offset, length, pSource, discardWholeBuffer);
        }

        mMetalHardwareBufferCommon.writeData(offset, length, pSource, discardWholeBuffer);
    }
    //-----------------------------------------------------------------------------------
    void MetalHardwareVertexBuffer::copyData( HardwareBuffer& srcBuffer, size_t srcOffset,
                                              size_t dstOffset, size_t length, bool discardWholeBuffer )
    {
        if( srcBuffer.isSystemMemory() )
        {
            HardwareBuffer::copyData( srcBuffer, srcOffset, dstOffset, length, discardWholeBuffer );
        }
        else
        {
            MetalHardwareBufferCommon* metalBuffer =
                static_cast<MetalHardwareVertexBuffer&>(srcBuffer).getRenderSystemData();
            mMetalHardwareBufferCommon.copyData(metalBuffer, srcOffset, dstOffset, length,
                                                discardWholeBuffer);
        }
    }
    //-----------------------------------------------------------------------------------
    void MetalHardwareVertexBuffer::_updateFromShadow(void)
    {
        if( mUseShadowBuffer && mShadowUpdated && !mSuppressHardwareUpdate )
        {
            HardwareBufferLockGuard shadowLock(mShadowBuffer.get(), mLockStart, mLockSize, HBL_READ_ONLY);
            mMetalHardwareBufferCommon.writeData(mLockStart, mLockSize, shadowLock.pData, false);
            mShadowUpdated = false;
        }
    }
    //-----------------------------------------------------------------------------------
    MetalHardwareBufferCommon* MetalHardwareVertexBuffer::getRenderSystemData(void)
    {
        return &mMetalHardwareBufferCommon;
    }
}
