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
#include "OgreMetalHardwareIndexBuffer.h"
#include "OgreMetalDiscardBufferManager.h"

namespace Ogre {
namespace v1 {
    MetalHardwareIndexBuffer::MetalHardwareIndexBuffer( MetalHardwareBufferManagerBase *mgr,
                                                        IndexType idxType,
                                                        size_t numIndexes,
                                                        HardwareBuffer::Usage usage,
                                                        bool useShadowBuffer ) :
        HardwareIndexBuffer( mgr, idxType, numIndexes, usage, false, false ),
        mMetalHardwareBufferCommon( mSizeInBytes, usage, 4, mgr->_getDiscardBufferManager(),
                                    mgr->_getDiscardBufferManager()->getDevice() )
    {
    }
    //-----------------------------------------------------------------------------------
    MetalHardwareIndexBuffer::~MetalHardwareIndexBuffer()
    {
    }
    //-----------------------------------------------------------------------------------
    void* MetalHardwareIndexBuffer::lockImpl( size_t offset, size_t length, LockOptions options )
    {
        return mMetalHardwareBufferCommon.lockImpl( offset, length, options, mIsLocked );
    }
    //-----------------------------------------------------------------------------------
    void MetalHardwareIndexBuffer::unlockImpl(void)
    {
        mMetalHardwareBufferCommon.unlockImpl( mLockStart, mLockSize );
    }
    //-----------------------------------------------------------------------------------
    void MetalHardwareIndexBuffer::_notifyDeviceStalled(void)
    {
        mMetalHardwareBufferCommon._notifyDeviceStalled();
    }
    //-----------------------------------------------------------------------------------
    id<MTLBuffer> MetalHardwareIndexBuffer::getBufferName( size_t &outOffset )
    {
        return mMetalHardwareBufferCommon.getBufferName( outOffset );
    }
    //-----------------------------------------------------------------------------------
    id<MTLBuffer> MetalHardwareIndexBuffer::getBufferNameForGpuWrite(void)
    {
        return mMetalHardwareBufferCommon.getBufferNameForGpuWrite();
    }
    //-----------------------------------------------------------------------------------
    void MetalHardwareIndexBuffer::readData( size_t offset, size_t length, void* pDest )
    {
        if( mUseShadowBuffer )
        {
            void* srcData = mShadowBuffer->lock(offset, length, HBL_READ_ONLY);
            memcpy(pDest, srcData, length);
            mShadowBuffer->unlock();
        }
        else
        {
            mMetalHardwareBufferCommon.readData( offset, length, pDest );
        }
    }
    //-----------------------------------------------------------------------------------
    void MetalHardwareIndexBuffer::writeData( size_t offset, size_t length,
                                               const void* pSource, bool discardWholeBuffer )
    {
        // Update the shadow buffer
        if( mUseShadowBuffer )
        {
            void* destData = mShadowBuffer->lock( offset, length,
                                                  discardWholeBuffer ? HBL_DISCARD : HBL_NORMAL );
            memcpy( destData, pSource, length );
            mShadowBuffer->unlock();
        }

        mMetalHardwareBufferCommon.writeData( offset, length, pSource,
                                              discardWholeBuffer ||
                                              (offset == 0 && length == mSizeInBytes) );
    }
    //-----------------------------------------------------------------------------------
    void MetalHardwareIndexBuffer::copyData( HardwareBuffer& srcBuffer, size_t srcOffset,
                                              size_t dstOffset, size_t length, bool discardWholeBuffer )
    {
        if( srcBuffer.isSystemMemory() )
        {
            HardwareBuffer::copyData( srcBuffer, srcOffset, dstOffset, length, discardWholeBuffer );
        }
        else
        {
            MetalHardwareBufferCommon *metalBuffer = reinterpret_cast<MetalHardwareBufferCommon*>(
                        srcBuffer.getRenderSystemData() );
            mMetalHardwareBufferCommon.copyData( metalBuffer, srcOffset,
                                                 dstOffset, length,
                                                 discardWholeBuffer ||
                                                 (dstOffset == 0 && length == mSizeInBytes) );
        }
    }
    //-----------------------------------------------------------------------------------
    void MetalHardwareIndexBuffer::_updateFromShadow(void)
    {
        if( mUseShadowBuffer && mShadowUpdated && !mSuppressHardwareUpdate )
        {
            const void *srcData = mShadowBuffer->lock( mLockStart, mLockSize, HBL_READ_ONLY );

            const bool discardBuffer = mLockStart == 0 && mLockSize == mSizeInBytes;
            mMetalHardwareBufferCommon.writeData( mLockStart, mLockSize, srcData, discardBuffer );

            mShadowBuffer->unlock();
            mShadowUpdated = false;
        }
    }
    //-----------------------------------------------------------------------------------
    void* MetalHardwareIndexBuffer::getRenderSystemData(void)
    {
        return &mMetalHardwareBufferCommon;
    }
}
}
