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

#include "OgreMetalHardwareBufferCommon.h"
#include "OgreMetalDevice.h"
#include "OgreDefaultHardwareBufferManager.h"

#import <Metal/MTLDevice.h>
#import <Metal/MTLBlitCommandEncoder.h>

namespace Ogre
{
    MetalHardwareBufferCommon::MetalHardwareBufferCommon( size_t sizeBytes, Usage usage, bool useShadowBuffer,
                                                          uint16 alignment,
                                                          MetalDevice *device ) :
        HardwareBuffer(usage, false),
        mBuffer( 0 ),
        mDevice( device )
    {
        mSizeInBytes = sizeBytes;

        MTLResourceOptions resourceOptions = 0;

        // FIXME always using shared storage for now
        if( usage & HBU_DETAIL_WRITE_ONLY & 0)
        {
            resourceOptions |= MTLResourceStorageModePrivate;
            resourceOptions |= MTLResourceCPUCacheModeWriteCombined;
        }
        else
        {
            resourceOptions |= MTLResourceStorageModeShared;
        }

        mBuffer = [mDevice->mDevice newBufferWithLength:alignToNextMultiple( sizeBytes, 4u )
                                                    options:resourceOptions];

        if (useShadowBuffer)
        {
            mShadowBuffer.reset(new DefaultHardwareBuffer(mSizeInBytes));
        }
    }
    //-----------------------------------------------------------------------------------
    MetalHardwareBufferCommon::~MetalHardwareBufferCommon()
    {
        mBuffer = 0;
    }
    //-----------------------------------------------------------------------------------
    id<MTLBuffer> MetalHardwareBufferCommon::getBufferName( size_t &outOffset )
    {
        outOffset = 0;
        return mBuffer;
    }
    //-----------------------------------------------------------------------------------
    void* MetalHardwareBufferCommon::lockImpl( size_t offset, size_t length, LockOptions options)
    {
        if ((options == HBL_READ_ONLY || options == HBL_NORMAL) && mBuffer.storageMode == MTLStorageModePrivate)
        {
            LogManager::getSingleton().logWarning("HardwareBuffer - UNIMPLEMENTED implicit GPU to HOST copy (slow)");
        }

        void *retPtr = mBuffer.contents;
        return static_cast<uint8*>( retPtr ) + offset;
    }
    //-----------------------------------------------------------------------------------
    void MetalHardwareBufferCommon::unlockImpl()
    {
    }
    //-----------------------------------------------------------------------------------
    void MetalHardwareBufferCommon::readData( size_t offset, size_t length, void* pDest )
    {
        // just use memcpy
        HardwareBufferLockGuard thisLock(this, offset, length, HBL_READ_ONLY);
        memcpy(pDest, thisLock.pData, length);
    }
    //-----------------------------------------------------------------------------------
    void MetalHardwareBufferCommon::writeData(size_t offset, size_t length, const void* pSource,
                                              bool discardWholeBuffer)
    {
        discardWholeBuffer = discardWholeBuffer || (offset == 0 && length == mSizeInBytes);
        if (mShadowBuffer)
        {
            mShadowBuffer->writeData(offset, length, pSource, discardWholeBuffer);
        }

        writeDataImpl(offset, length, pSource, discardWholeBuffer);
    }
    void MetalHardwareBufferCommon::writeDataImpl(size_t offset, size_t length, const void* pSource,
                                                  bool discardWholeBuffer)
    {
        // FIXME
        if(true || mBuffer.storageMode == MTLStorageModePrivate)
        {
            void *dstData = lockImpl( offset, length, HBL_DISCARD);
            memcpy( dstData, pSource, length );
            unlockImpl();
        }
    }
    //-----------------------------------------------------------------------------------
    void MetalHardwareBufferCommon::copyData( HardwareBuffer& _srcBuffer, size_t srcOffset,
                                              size_t dstOffset, size_t length, bool discardWholeBuffer )
    {
        if (mShadowBuffer)
        {
            mShadowBuffer->copyData(_srcBuffer, srcOffset, dstOffset, length, discardWholeBuffer);
        }

        auto srcBuffer = static_cast<MetalHardwareBufferCommon*>(&_srcBuffer);
        discardWholeBuffer = discardWholeBuffer || (dstOffset == 0 && length == mSizeInBytes);

        if( srcBuffer->mBuffer.storageMode == MTLStorageModePrivate )
        {
            size_t srcOffsetStart = 0;
            __unsafe_unretained id<MTLBuffer> srcBuf = srcBuffer->getBufferName( srcOffsetStart );
            __unsafe_unretained id<MTLBuffer> dstBuf = mBuffer;

            __unsafe_unretained id<MTLBlitCommandEncoder> blitEncoder = mDevice->getBlitEncoder();
            [blitEncoder copyFromBuffer:srcBuf
                           sourceOffset:srcOffset + srcOffsetStart
                               toBuffer:dstBuf
                      destinationOffset:dstOffset
                                   size:length];
        }
        else
        {
            HardwareBuffer::LockOptions dstOption;
            if( discardWholeBuffer )
                dstOption = HardwareBuffer::HBL_DISCARD;
            else
                dstOption = HardwareBuffer::HBL_WRITE_ONLY;

            const void *srcData = srcBuffer->lockImpl( srcOffset, length, HBL_READ_ONLY);
            void *dstData = this->lockImpl( dstOffset, length, dstOption );

            memcpy( dstData, srcData, length );

            this->unlockImpl();
            srcBuffer->unlockImpl();
        }
    }

    //-----------------------------------------------------------------------------------
    void MetalHardwareBufferCommon::_updateFromShadow(void)
    {
        if( mShadowBuffer && mShadowUpdated && !mSuppressHardwareUpdate )
        {
            HardwareBufferLockGuard shadowLock(mShadowBuffer.get(), mLockStart, mLockSize, HBL_READ_ONLY);
            writeDataImpl(mLockStart, mLockSize, shadowLock.pData, false);
            mShadowUpdated = false;
        }
    }
}
