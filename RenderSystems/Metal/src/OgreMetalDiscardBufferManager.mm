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

#include "OgreMetalDiscardBufferManager.h"
#include "OgreMetalRenderSystem.h"

#import <Metal/MTLDevice.h>
#import <Metal/MTLBuffer.h>
#import <Metal/MTLBlitCommandEncoder.h>

namespace Ogre
{
    MetalDiscardBufferManager::MetalDiscardBufferManager( MetalDevice *device) :
        mBuffer( 0 ),
        mDevice( device )
    {
        const size_t defaultCapacity = 4 * 1024 * 1024;

        MTLResourceOptions resourceOptions = MTLResourceCPUCacheModeWriteCombined |
                                             MTLResourceStorageModeShared;

        mBuffer = [mDevice->mDevice newBufferWithLength:defaultCapacity options:resourceOptions];
        mBuffer.label = @"Discardable Buffer";
        mFreeBlocks.push_back( Block( 0, defaultCapacity ) );
    }
    //-------------------------------------------------------------------------
    MetalDiscardBufferManager::~MetalDiscardBufferManager()
    {
        MetalDiscardBufferVec::const_iterator itor = mDiscardBuffers.begin();
        MetalDiscardBufferVec::const_iterator end  = mDiscardBuffers.end();

        while( itor != end )
            OGRE_DELETE *itor++;
        mDiscardBuffers.clear();

        mBuffer = 0;
    }
    //-------------------------------------------------------------------------
    void MetalDiscardBufferManager::growToFit( size_t extraBytes, MetalDiscardBuffer *forDiscardBuffer )
    {
        assert( !(extraBytes & 0x04) && "extraBytes must be multiple of 4!" );

        const size_t oldCapacity = [mBuffer length];
        const size_t newCapacity = std::max( oldCapacity + extraBytes,
                                             oldCapacity + (oldCapacity >> 1u) + 1u );

        MTLResourceOptions resourceOptions = MTLResourceCPUCacheModeWriteCombined |
                                             MTLResourceStorageModeShared;
        id<MTLBuffer> oldBuffer = mBuffer;
        mBuffer = [mDevice->mDevice newBufferWithLength:newCapacity options:resourceOptions];
        mBuffer.label = @"Discardable Buffer";

        //TODO DEBUG: We need to check if we need to retain oldBuffer or Metal does that.
        NSLog( @"DEBUG Pre: Retain count is %ld", CFGetRetainCount( (__bridge CFTypeRef)oldBuffer ) );

        {
            //Update our buffers so they point to the new buffer, copy their blocks in use from old
            //MTLBuffer to new one, and tag all of them as in use by GPU (due to the copyFromBuffer);
            //except 'forDiscardBuffer' which we were told this data won't be used.
            __unsafe_unretained id<MTLBlitCommandEncoder> blitEncoder = mDevice->getBlitEncoder();

            const uint32 currentFrame = 0;//mVaoManager->getFrameCount();
            MetalDiscardBufferVec::iterator itor = mDiscardBuffers.begin();
            MetalDiscardBufferVec::iterator end  = mDiscardBuffers.end();

            while( itor != end )
            {
                if( *itor != forDiscardBuffer )
                {
                    (*itor)->mBuffer = mBuffer;
                    [blitEncoder copyFromBuffer:oldBuffer
                                   sourceOffset:(*itor)->getBlockStart()
                                       toBuffer:mBuffer
                              destinationOffset:(*itor)->getBlockStart()
                                           size:(*itor)->getBlockSize()];
                    (*itor)->mLastFrameUsed = currentFrame;
                }
                else
                {
                    (*itor)->mLastFrameUsed = 0;//currentFrame - mVaoManager->getDynamicBufferMultiplier();
                }

                ++itor;
            }
        }

        NSLog( @"DEBUG Pos: Retain count is %ld", CFGetRetainCount( (__bridge CFTypeRef)oldBuffer ) );

        LogManager::getSingleton().logMessage(
                    "PERFORMANCE WARNING: MetalDiscardBufferManager::growToFit must stall."
                    "Consider increasing the default discard capacity to at least " +
                    StringConverter::toString( newCapacity ) + " bytes" );

        //According to Metal docs, it's undefined behavior if both CPU & GPU
        //write to the same resource even if the regions don't overlap.
        mDevice->stall();

        NSLog( @"DEBUG Pos2: Retain count is %ld", CFGetRetainCount( (__bridge CFTypeRef)oldBuffer ) );

        mFreeBlocks.push_back( Block( oldCapacity, newCapacity - oldCapacity ) );

        {
            //All "unsafe" blocks are no longer unsafe, since we're using a new buffer.
            UnsafeBlockVec::const_iterator itor = mUnsafeBlocks.begin();
            UnsafeBlockVec::const_iterator end  = mUnsafeBlocks.end();

            while( itor != end )
            {
                mFreeBlocks.push_back( *itor );
                //MetalVaoManager::mergeContiguousBlocks( mFreeBlocks.end() - 1, mFreeBlocks );
                ++itor;
            }

            mUnsafeBlocks.clear();
        }
    }
    //-------------------------------------------------------------------------
    void MetalDiscardBufferManager::updateUnsafeBlocks(void)
    {
        const uint32 currentFrame       = 0;//mVaoManager->getFrameCount();
        const uint32 bufferMultiplier   = 1;//mVaoManager->getDynamicBufferMultiplier();

        UnsafeBlockVec::iterator itor = mUnsafeBlocks.begin();
        UnsafeBlockVec::iterator end  = mUnsafeBlocks.end();

        while( itor != end && (currentFrame - itor->lastFrameUsed) >= bufferMultiplier )
        {
            //This block is safe now to put back into free blocks.
            mFreeBlocks.push_back( *itor );
            //MetalVaoManager::mergeContiguousBlocks( mFreeBlocks.end() - 1, mFreeBlocks );
            ++itor;
        }

        mUnsafeBlocks.erase( mUnsafeBlocks.begin(), itor );
    }
    //-------------------------------------------------------------------------
    void MetalDiscardBufferManager::_notifyDeviceStalled(void)
    {
        {
            UnsafeBlockVec::iterator itor = mUnsafeBlocks.begin();
            UnsafeBlockVec::iterator end  = mUnsafeBlocks.end();

            while( itor != end )
            {
                //This block is safe now to put back into free blocks.
                mFreeBlocks.push_back( *itor );
                //MetalVaoManager::mergeContiguousBlocks( mFreeBlocks.end() - 1, mFreeBlocks );
                ++itor;
            }

            mUnsafeBlocks.clear();
        }

        {
            const uint32 currentFrame       = 0;//mVaoManager->getFrameCount();
            const uint32 bufferMultiplier   = 1;//mVaoManager->getDynamicBufferMultiplier();

            MetalDiscardBufferVec::const_iterator itor = mDiscardBuffers.begin();
            MetalDiscardBufferVec::const_iterator end  = mDiscardBuffers.end();

            while( itor != end )
            {
                (*itor)->mLastFrameUsed = currentFrame - bufferMultiplier;
                ++itor;
            }
        }
    }
    //-------------------------------------------------------------------------
    void MetalDiscardBufferManager::_getBlock( MetalDiscardBuffer *discardBuffer )
    {
        const size_t alignment = discardBuffer->getAlignment();
        const size_t sizeBytes = discardBuffer->getSizeBytes();

        if( discardBuffer->mBuffer )
        {
            //if( mVaoManager->getFrameCount() - discardBuffer->mLastFrameUsed >=
            //    mVaoManager->getDynamicBufferMultiplier() )
            {
                return; //Current block the buffer owns is safe to reuse.
            }
            // else
            {
                //Release the block back into the pool (sorted by mLastFrameUsed)
                UnsafeBlock unsafeBlock( discardBuffer->getBlockStart(),
                                         discardBuffer->getBlockSize(),
                                         discardBuffer->mLastFrameUsed );
                UnsafeBlockVec::iterator it = std::lower_bound( mUnsafeBlocks.begin(),
                                                                mUnsafeBlocks.end(),
                                                                unsafeBlock );
                mUnsafeBlocks.insert( it, unsafeBlock );
            }
        }

        updateUnsafeBlocks();

        //Find smallest block.
        auto itor = mFreeBlocks.begin();
        auto end = mFreeBlocks.end();

        auto smallestBlock = end;

        while( itor != end )
        {
            const size_t alignedOffset = std::min( itor->offset + itor->size,
                                                   alignToNextMultiple( itor->offset, alignment ) );

            if( sizeBytes <= itor->size - (alignedOffset - itor->offset) )
            {
                //We can use 'itor' block. Now check if it's smaller.
                if( smallestBlock == end || itor->size < smallestBlock->size )
                {
                    smallestBlock = itor;
                }
            }

            ++itor;
        }

        if( smallestBlock == end )
        {
            //No block can satisfy us. Resize (slow!)
            growToFit( sizeBytes, discardBuffer );

            //Recursive call: Try again. This time it will work since we have the space.
            discardBuffer->mBuffer = 0;
            _getBlock( discardBuffer );
            discardBuffer->mBuffer = mBuffer;
        }
        else
        {
            //Assign this block and shrink our records.
            discardBuffer->mBufferOffset        = alignToNextMultiple( smallestBlock->offset, alignment );
            discardBuffer->mBlockPrePadding     = discardBuffer->mBufferOffset - smallestBlock->offset;

            const size_t shrunkBytes = discardBuffer->getBlockSize();

            smallestBlock->offset   = discardBuffer->mBufferOffset + discardBuffer->mBufferSize;
            smallestBlock->size     -= shrunkBytes;

            if( smallestBlock->size == 0 )
            {
                mFreeBlocks.erase( smallestBlock );
            }
        }
    }
    //-------------------------------------------------------------------------
    MetalDiscardBuffer* MetalDiscardBufferManager::createDiscardBuffer( size_t bufferSize,
                                                                        uint16 alignment )
    {
        alignment = std::max<uint16>( 4u, alignment ); //Prevent alignments lower than 4 bytes.
        MetalDiscardBuffer *retVal = OGRE_NEW MetalDiscardBuffer( bufferSize, alignment,
                                                                  this );
        mDiscardBuffers.push_back( retVal );
        _getBlock( retVal );
        retVal->mBuffer = mBuffer;

        return retVal;
    }
    //-------------------------------------------------------------------------
    void MetalDiscardBufferManager::destroyDiscardBuffer( MetalDiscardBuffer *discardBuffer )
    {
        MetalDiscardBufferVec::iterator itor = std::find( mDiscardBuffers.begin(),
                                                          mDiscardBuffers.end(),
                                                          discardBuffer );

        if( itor != mDiscardBuffers.end() )
        {
            assert( discardBuffer->mOwner == this &&
                    "Manager says it owns the discard buffer, but discard buffer says it doesn't" );

            //Release the block back into the pool (sorted by mLastFrameUsed)
            UnsafeBlock unsafeBlock( discardBuffer->getBlockStart(),
                                     discardBuffer->getBlockSize(),
                                     discardBuffer->mLastFrameUsed );
            UnsafeBlockVec::iterator it = std::lower_bound( mUnsafeBlocks.begin(),
                                                            mUnsafeBlocks.end(),
                                                            unsafeBlock );
            mUnsafeBlocks.insert( it, unsafeBlock );

            efficientVectorRemove( mDiscardBuffers, itor );
            OGRE_DELETE discardBuffer;
        }
        else
        {
            OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR,
                         "discardBuffer doesn't belong to this "
                         "MetalDiscardBufferManager or was already freed",
                         "MetalDiscardBufferManager::destroyDiscardBuffer" );
        }
    }
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    MetalDiscardBuffer::MetalDiscardBuffer( size_t bufferSize, uint16 alignment,
                                            MetalDiscardBufferManager *owner ) :
        mBuffer( 0 ),
        mBlockPrePadding( 0 ),
        mBufferOffset( 0 ),
        mBufferSize( bufferSize ),
        mAlignment( alignment ),
        mLastFrameUsed( 0 ),
        mOwner( owner )
    {
        assert( alignment >= 4u );
    }
    //-------------------------------------------------------------------------
    void* MetalDiscardBuffer::map( bool noOverwrite )
    {
        if( !noOverwrite )
            mOwner->_getBlock( this );
        return reinterpret_cast<uint8*>( [mBuffer contents] ) + mBufferOffset;
    }
    //-------------------------------------------------------------------------
    void MetalDiscardBuffer::unmap(void)
    {
//#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
//        NSRange range = NSMakeRange( mBufferOffset, mBufferSize );
//        [mBuffer didModifyRange:range];
//#endif
    }
    //-------------------------------------------------------------------------
    id<MTLBuffer> MetalDiscardBuffer::getBufferName( size_t &outOffset )
    {
        mLastFrameUsed = 0;//mVaoManager->getFrameCount();
        outOffset = mBufferOffset;
        return mBuffer;
    }
}
