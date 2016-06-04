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
#include "OgreMetalDiscardBufferManager.h"
#include "Vao/OgreMetalStagingBuffer.h"

#import <Metal/MTLDevice.h>
#import <Metal/MTLBlitCommandEncoder.h>

namespace Ogre
{
namespace v1
{
    MetalHardwareBufferCommon::MetalHardwareBufferCommon( size_t sizeBytes, HardwareBuffer::Usage usage,
                                                          uint16 alignment,
                                                          MetalDiscardBufferManager *discardBufferMgr,
                                                          MetalDevice *device ) :
        mBuffer( 0 ),
        mSizeBytes( sizeBytes ),
        mDevice( device ),
        mDiscardBuffer( 0 ),
        mVaoManager( discardBufferMgr->getVaoManager() ),
        mStagingBuffer( 0 ),
        mLastFrameUsed( 0 ),
        mLastFrameGpuWrote( 0 )
    {
        mLastFrameUsed = mVaoManager->getFrameCount() - mVaoManager->getDynamicBufferMultiplier();
        mLastFrameGpuWrote = mLastFrameUsed;

        MTLResourceOptions resourceOptions = 0;

        if( usage & HardwareBuffer::HBU_WRITE_ONLY )
        {
            resourceOptions |= MTLResourceStorageModePrivate;
            resourceOptions |= MTLResourceCPUCacheModeWriteCombined;
        }
        else
        {
            resourceOptions |= MTLResourceStorageModeShared;
        }

        if( !(usage & HardwareBuffer::HBU_DISCARDABLE) )
        {
            mBuffer = [mDevice->mDevice newBufferWithLength:alignToNextMultiple( sizeBytes, 4u )
                                                    options:resourceOptions];
        }
        else
        {
            mDiscardBuffer = discardBufferMgr->createDiscardBuffer( sizeBytes, alignment );
        }
    }
    //-----------------------------------------------------------------------------------
    MetalHardwareBufferCommon::~MetalHardwareBufferCommon()
    {
        mBuffer = 0;
        if( mDiscardBuffer )
        {
            MetalDiscardBufferManager *discardBufferManager = mDiscardBuffer->getOwner();
            discardBufferManager->destroyDiscardBuffer( mDiscardBuffer );
            mDiscardBuffer = 0;
        }
    }
    //-----------------------------------------------------------------------------------
    void MetalHardwareBufferCommon::_notifyDeviceStalled(void)
    {
        mLastFrameUsed      = mVaoManager->getFrameCount() - mVaoManager->getDynamicBufferMultiplier();
        mLastFrameGpuWrote  = mLastFrameUsed;
    }
    //-----------------------------------------------------------------------------------
    id<MTLBuffer> MetalHardwareBufferCommon::getBufferName( size_t &outOffset )
    {
        mLastFrameUsed = mVaoManager->getFrameCount();
        outOffset = 0;
        return !mDiscardBuffer ? mBuffer : mDiscardBuffer->getBufferName( outOffset );
    }
    //-----------------------------------------------------------------------------------
    id<MTLBuffer> MetalHardwareBufferCommon::getBufferNameForGpuWrite(void)
    {
        assert( !mDiscardBuffer && "Discardable buffers can't be written from GPU!" );
        mLastFrameUsed      = mVaoManager->getFrameCount();
        mLastFrameGpuWrote  = mLastFrameUsed;
        return mBuffer;
    }
    //-----------------------------------------------------------------------------------
    void* MetalHardwareBufferCommon::lockImpl( size_t offset, size_t length,
                                               HardwareBuffer::LockOptions options,
                                               bool isLocked )
    {
        if( isLocked )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                         "Invalid attempt to lock a buffer that has already been locked",
                         "MetalHardwareBufferCommon::lock" );
        }

        void *retPtr = 0;

        const uint32 currentFrame       = mVaoManager->getFrameCount();
        const uint32 bufferMultiplier   = mVaoManager->getDynamicBufferMultiplier();

        if( mDiscardBuffer )
        {
            //If we're here, it was created with HBU_DISCARDABLE bit
            if( options == HardwareBuffer::HBL_READ_ONLY )
            {
                LogManager::getSingleton().logMessage(
                            "PERFORMANCE WARNING: reads from discardable "
                            "buffers are uncached. May be slow." );

                //We can't write from GPU to discardable memory. No need to check
            }
            else if( options == HardwareBuffer::HBL_NORMAL || options == HardwareBuffer::HBL_WRITE_ONLY )
            {
                if( currentFrame - mLastFrameUsed < bufferMultiplier )
                {
                    LogManager::getSingleton().logMessage(
                                "PERFORMANCE WARNING: locking with HBL_NORMAL or HBL_WRITE_ONLY for a "
                                "buffer created with HBU_DISCARDABLE bit is slow/stalling. Consider "
                                "locking w/ another locking option, or change the buffer's usage flag" );
                    mDevice->stall();
                }
            }

            retPtr = mDiscardBuffer->map( options != HardwareBuffer::HBL_DISCARD );
            retPtr = static_cast<void*>( static_cast<uint8*>( retPtr ) + offset );
        }
        else
        {
            if( mBuffer.storageMode != MTLStorageModePrivate )
            {
                if( options == HardwareBuffer::HBL_READ_ONLY )
                {
                    if( currentFrame - mLastFrameGpuWrote < bufferMultiplier )
                        mDevice->stall();
                }
                else if( options != HardwareBuffer::HBL_NO_OVERWRITE )
                {
                    if( currentFrame - mLastFrameUsed < bufferMultiplier )
                    {
                        LogManager::getSingleton().logMessage(
                                    "PERFORMANCE WARNING: locking to a non-HBU_WRITE_ONLY or "
                                    "non-HBU_DISCARDABLE for other than reading is slow/stalling." );

                        mDevice->stall();
                    }
                }

                retPtr = [mBuffer contents];
                retPtr = static_cast<void*>( static_cast<uint8*>( retPtr ) + offset );
            }
            else
            {
                //If we're here, the buffer was created with HBU_WRITE_ONLY, but not discardable.
                //Write to a staging buffer to avoid blocking. We don't have to care about
                //reading access.
                assert( (options != HardwareBuffer::HBL_NORMAL ||
                        options != HardwareBuffer::HBL_READ_ONLY) &&
                        "Reading from a write-only buffer! Create "
                        "the buffer without HBL_WRITE_ONLY bit (or use readData)" );

                assert( !mStagingBuffer && "Invalid state, and mStagingBuffer will leak" );

                mStagingBuffer = mVaoManager->getStagingBuffer( length, true );
                retPtr = mStagingBuffer->map( length );
            }
        }

        return retPtr;
    }
    //-----------------------------------------------------------------------------------
    void MetalHardwareBufferCommon::unlockImpl( size_t lockStart, size_t lockSize )
    {
        if( mDiscardBuffer )
            mDiscardBuffer->unmap();

        if( mStagingBuffer )
        {
            static_cast<MetalStagingBuffer*>( mStagingBuffer )->_unmapToV1( this, lockStart, lockSize );
            mStagingBuffer->removeReferenceCount();
            mStagingBuffer = 0;
        }
    }
    //-----------------------------------------------------------------------------------
    void MetalHardwareBufferCommon::readData( size_t offset, size_t length, void* pDest )
    {
        assert( (offset + length) <= mSizeBytes );

        void const *srcData = 0;
        StagingBuffer *stagingBuffer = 0;

        const uint32 currentFrame       = mVaoManager->getFrameCount();
        const uint32 bufferMultiplier   = mVaoManager->getDynamicBufferMultiplier();

        if( mDiscardBuffer )
           {
            //We can't write from GPU to discardable memory. No need to check.
            srcData = mDiscardBuffer->map( true );
        }
        else
        {
            if( mBuffer.storageMode != MTLStorageModePrivate )
            {
                if( currentFrame - mLastFrameGpuWrote < bufferMultiplier )
                    mDevice->stall();
                srcData = [mBuffer contents];
            }
            else
            {
                //Reading from HBL_WRITE_ONLY.
                stagingBuffer = mVaoManager->getStagingBuffer( length, false );
                size_t stagingBufferOffset = static_cast<MetalStagingBuffer*>(
                            stagingBuffer )->_asyncDownloadV1( this, offset, length );
                mDevice->stall();
                srcData = stagingBuffer->_mapForRead( stagingBufferOffset, length );
                offset = 0;
            }
        }

        srcData = static_cast<const void*>( static_cast<const uint8*>( srcData ) + offset );

        memcpy( pDest, srcData, length );

        if( stagingBuffer )
            stagingBuffer->removeReferenceCount();
    }
    //-----------------------------------------------------------------------------------
    void MetalHardwareBufferCommon::writeData( size_t offset, size_t length,
                                               const void* pSource,
                                               bool discardWholeBuffer )
    {
        if( (discardWholeBuffer && mDiscardBuffer) || mBuffer.storageMode == MTLStorageModePrivate )
        {
            //Fast path is through locking (it either discards or already uses a StagingBuffer).
            void *dstData = this->lockImpl( offset, length, HardwareBuffer::HBL_DISCARD, false );
            memcpy( dstData, pSource, length );
            this->unlockImpl( offset, length );
        }
        else
        {
            //Use a StagingBuffer to avoid blocking
            StagingBuffer *stagingBuffer = mVaoManager->getStagingBuffer( length, true );
            void *dstData = stagingBuffer->map( length );
            memcpy( dstData, pSource, length );
            static_cast<MetalStagingBuffer*>( stagingBuffer )->_unmapToV1( this, offset, length );
            stagingBuffer->removeReferenceCount();
        }
    }
    //-----------------------------------------------------------------------------------
    void MetalHardwareBufferCommon::copyData( MetalHardwareBufferCommon *srcBuffer, size_t srcOffset,
                                              size_t dstOffset, size_t length, bool discardWholeBuffer )
    {
        if( !this->mDiscardBuffer || srcBuffer->mBuffer.storageMode == MTLStorageModePrivate )
        {
            size_t srcOffsetStart = 0;
            __unsafe_unretained id<MTLBuffer> srcBuf = srcBuffer->getBufferName( srcOffsetStart );
            __unsafe_unretained id<MTLBuffer> dstBuf = this->getBufferNameForGpuWrite();

            __unsafe_unretained id<MTLBlitCommandEncoder> blitEncoder = mDevice->getBlitEncoder();
            [blitEncoder copyFromBuffer:srcBuf
                           sourceOffset:srcOffset + srcOffsetStart
                               toBuffer:dstBuf
                      destinationOffset:dstOffset
                                   size:length];

            if( this->mDiscardBuffer )
                mDevice->stall();
        }
        else
        {
            HardwareBuffer::LockOptions dstOption;
            if( discardWholeBuffer )
                dstOption = HardwareBuffer::HBL_DISCARD;
            else
                dstOption = HardwareBuffer::HBL_WRITE_ONLY;

            const void *srcData = srcBuffer->lockImpl( srcOffset, length,
                                                       HardwareBuffer::HBL_READ_ONLY, false );
            void *dstData = this->lockImpl( dstOffset, length, dstOption, false );

            memcpy( dstData, srcData, length );

            this->unlockImpl( dstOffset, length );
            srcBuffer->unlockImpl( srcOffset, length );
        }
    }
}
}
