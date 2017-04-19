/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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

#include "OgreStableHeaders.h"
#include "Vao/OgreBufferPacked.h"
#include "Vao/OgreBufferInterface.h"
#include "Vao/OgreVaoManager.h"
#include "OgreException.h"
#include "OgreLogManager.h"

namespace Ogre
{
    BufferPacked::BufferPacked( size_t internalBufferStartBytes, size_t numElements,
                                uint32 bytesPerElement, uint32 numElementsPadding,
                                BufferType bufferType, void *initialData,
                                bool keepAsShadow, VaoManager *vaoManager,
                                BufferInterface *bufferInterface ) :
        mInternalBufferStart( internalBufferStartBytes / bytesPerElement ),
        mFinalBufferStart( internalBufferStartBytes / bytesPerElement ),
        mNumElements( numElements ),
        mBytesPerElement( bytesPerElement ),
        mNumElementsPadding( numElementsPadding ),
        mBufferType( bufferType ),
        mVaoManager( vaoManager ),
        mMappingState( MS_UNMAPPED ),
        mBufferInterface( bufferInterface ),
        mLastMappingStart( 0 ),
        mLastMappingCount( 0 ),
        mShadowCopy( 0 )
#if OGRE_DEBUG_MODE
    ,   mLastFrameMapped( ~0 ),
        mLastFrameMappedAndAdvanced( ~0 )
#endif
    {
        assert( !(internalBufferStartBytes % bytesPerElement) );

        if( mBufferInterface )
            mBufferInterface->_notifyBuffer( this );

        if( !initialData && mBufferType == BT_IMMUTABLE )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "Immutable buffers MUST provide initial data!",
                         "BufferPacked::BufferPacked" );
        }

        if( keepAsShadow )
        {
            if( mBufferType >= BT_DYNAMIC_DEFAULT )
            {
                OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                             "Dynamic buffers can't have a shadow copy!",
                             "BufferPacked::BufferPacked" );
            }

            mShadowCopy = initialData;
        }
    }
    //-----------------------------------------------------------------------------------
    BufferPacked::~BufferPacked()
    {
        if( mMappingState != MS_UNMAPPED )
        {
            LogManager::getSingleton().logMessage( "WARNING: Deleting mapped buffer without "
                                                   "having it unmapped. This is often sign of"
                                                   " a resource leak or a bad pattern. "
                                                   "Umapping the buffer for you...",
                                                   LML_CRITICAL );

            unmap( UO_UNMAP_ALL );
        }

        delete mBufferInterface;
        mBufferInterface = 0;

        //The shadow copy must be destroyed after the mBufferInterface
        if( mShadowCopy )
        {
            OGRE_FREE_SIMD( mShadowCopy, MEMCATEGORY_GEOMETRY );
            mShadowCopy = 0;
        }
    }
    //-----------------------------------------------------------------------------------
    void BufferPacked::_setBufferInterface( BufferInterface *bufferInterface )
    {
        mBufferInterface = bufferInterface;
    }
    //-----------------------------------------------------------------------------------
    AsyncTicketPtr BufferPacked::readRequest( size_t elementStart, size_t elementCount )
    {
        StagingBuffer *stagingBuffer = mVaoManager->getStagingBuffer( elementCount * mBytesPerElement,
                                                                      false );

        return mVaoManager->createAsyncTicket( this, stagingBuffer, elementStart, elementCount );
    }
    //-----------------------------------------------------------------------------------
    void BufferPacked::upload( const void *data, size_t elementStart, size_t elementCount )
    {
        if( mBufferType == BT_IMMUTABLE )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                         "Cannot use upload on an immutable buffer!",
                         "BufferPacked::upload" );
        }

        if( elementCount + elementStart > mNumElements )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "Size of the provided data goes out of bounds!",
                         "BufferPacked::upload" );
        }

        if( mShadowCopy )
        {
            memcpy( (char*)mShadowCopy + elementStart * mBytesPerElement,
                    data, elementCount * mBytesPerElement );
        }

        mBufferInterface->upload( data, elementStart, elementCount );
    }
    //-----------------------------------------------------------------------------------
    void* RESTRICT_ALIAS_RETURN BufferPacked::map( size_t elementStart, size_t elementCount,
                                                   bool bAdvanceFrame )
    {
        if( mBufferType < BT_DYNAMIC_DEFAULT )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                         "Only dynamic buffers can be mapped! Use upload instead.",
                         "BufferPacked::map" );
        }

        if( !elementCount )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "Mapping 0 bytes is forbidden!",
                         "BufferPacked::map" );
        }

        if( isCurrentlyMapped() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                         "Buffer already mapped! Only persistent buffers can call"
                         "map more than once without unmapping, but you still need "
                         "to call unmap( UO_KEEP_PERSISTENT ) between maps",
                         "BufferPacked::map" );
        }

#if OGRE_DEBUG_MODE
        {
            uint32 currentFrame = mVaoManager->getFrameCount();
            if( mLastFrameMappedAndAdvanced == currentFrame )
            {
                OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                             "Mapping the buffer twice within the same frame detected! "
                             "This is not allowed.", "BufferPacked::map" );
            }

            if( //This is a different frame from the last time we called map
                mLastFrameMapped != currentFrame &&
                //map was called, but bAdvanceFrame was not.
                (int)(mLastFrameMapped - mLastFrameMappedAndAdvanced) > 0 )
            {
                OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                             "Last frame called map( bAdvanceFrame = false ) but "
                             "didn't call advanceFrame!!!.", "BufferPacked::map" );
            }

            mLastFrameMapped = currentFrame;

            if( bAdvanceFrame )
                mLastFrameMappedAndAdvanced = currentFrame;
        }
#endif

        MappingState prevMappingState = mMappingState;
        mMappingState = MS_MAPPED;

        return mBufferInterface->map( elementStart, elementCount, prevMappingState, bAdvanceFrame );
    }
    //-----------------------------------------------------------------------------------
    void BufferPacked::unmap( UnmapOptions unmapOption, size_t flushStartElem, size_t flushSizeElem )
    {
        if( mMappingState == MS_UNMAPPED )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                         "Buffer wasn't mapped!",
                         "BufferPacked::unmap" );
        }

        mBufferInterface->unmap( unmapOption, flushStartElem, flushSizeElem );

        if( unmapOption == UO_UNMAP_ALL || mBufferType == BT_DYNAMIC_DEFAULT ||
            !mVaoManager->supportsPersistentMapping() )
        {
            mMappingState = MS_UNMAPPED;
        }

        mLastMappingStart = 0;
        mLastMappingCount = 0;
    }
    //-----------------------------------------------------------------------------------
    void BufferPacked::advanceFrame(void)
    {
#if OGRE_DEBUG_MODE
        if( mLastFrameMappedAndAdvanced == mVaoManager->getFrameCount() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                         "Calling advanceFrame twice in the same frame (or map was already "
                         "called with advanceFrame = true). This is not allowed.",
                         "BufferPacked::advanceFrame" );
        }

        mLastFrameMappedAndAdvanced = mVaoManager->getFrameCount();
#endif

        if( isCurrentlyMapped() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                         "Don't advanceFrame while mapped!",
                         "BufferPacked::map" );
        }

        return mBufferInterface->advanceFrame();
    }
    //-----------------------------------------------------------------------------------
    void BufferPacked::regressFrame(void)
    {
#if OGRE_DEBUG_MODE
        if( mLastFrameMappedAndAdvanced != mVaoManager->getFrameCount() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                         "regressFrame can only be called after calling advanceFrame ",
                         "BufferPacked::advanceFrame" );
        }

        --mLastFrameMappedAndAdvanced;
#endif

        if( isCurrentlyMapped() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                         "Don't regressFrame while mapped!",
                         "BufferPacked::map" );
        }

        return mBufferInterface->regressFrame();
    }
    //-----------------------------------------------------------------------------------
    bool BufferPacked::isCurrentlyMapped(void) const
    {
        if( mMappingState == MS_UNMAPPED )
            return false;

        if( mLastMappingCount == 0 )
            return false;

        return true;
    }

    //-----------------------------------------------------------------------------------
    void BufferPacked::_setShadowCopy( void* copy )
    {
        if( mBufferType >= BT_DYNAMIC_DEFAULT )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                "Dynamic buffers can't have a shadow copy!",
                "BufferPacked::BufferPacked" );
        }
        mShadowCopy = copy;
    }

}

