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

#include "Vao/OgreNULLBufferInterface.h"
#include "Vao/OgreNULLVaoManager.h"
#include "Vao/OgreNULLStagingBuffer.h"

namespace Ogre
{
    NULLBufferInterface::NULLBufferInterface( size_t vboPoolIdx ) :
        mVboPoolIdx( vboPoolIdx ),
        mMappedPtr( 0 ),
        mNullDataPtr( 0 )
    {
    }
    //-----------------------------------------------------------------------------------
    NULLBufferInterface::~NULLBufferInterface()
    {
        if( mNullDataPtr )
        {
            OGRE_FREE_SIMD( mNullDataPtr, MEMCATEGORY_RENDERSYS );
            mNullDataPtr = 0;
        }
    }
    //-----------------------------------------------------------------------------------
    void NULLBufferInterface::_firstUpload( const void *data, size_t elementStart, size_t elementCount )
    {
        //In OpenGL; immutable buffers are a charade. They're mostly there to satisfy D3D11's needs.
        //However we emulate the behavior and trying to upload to an immutable buffer will result
        //in an exception or an assert, thus we temporarily change the type.
        BufferType originalBufferType = mBuffer->mBufferType;
        if( mBuffer->mBufferType == BT_IMMUTABLE )
            mBuffer->mBufferType = BT_DEFAULT;

        upload( data, elementStart, elementCount );

        mBuffer->mBufferType = originalBufferType;
    }
    //-----------------------------------------------------------------------------------
    void* RESTRICT_ALIAS_RETURN NULLBufferInterface::map( size_t elementStart, size_t elementCount,
                                                          MappingState prevMappingState,
                                                          bool bAdvanceFrame )
    {
        size_t bytesPerElement = mBuffer->mBytesPerElement;

        NULLVaoManager *vaoManager = static_cast<NULLVaoManager*>( mBuffer->mVaoManager );
        bool canPersistentMap = vaoManager->supportsArbBufferStorage();

        size_t dynamicCurrentFrame = advanceFrame( bAdvanceFrame );

        if( prevMappingState == MS_UNMAPPED || !canPersistentMap )
        {
            //Non-persistent buffers just map the small region they'll need.
            size_t offset = mBuffer->mInternalBufferStart + elementStart +
                            mBuffer->_getInternalNumElements() * dynamicCurrentFrame;
            size_t length = elementCount;

            if( mBuffer->mBufferType >= BT_DYNAMIC_PERSISTENT && canPersistentMap )
            {
                //Persistent buffers map the *whole* assigned buffer,
                //we later care for the offsets and lengths
                offset = mBuffer->mInternalBufferStart;
                length = mBuffer->_getInternalNumElements() * vaoManager->getDynamicBufferMultiplier();
            }

            mMappedPtr = mNullDataPtr + offset * bytesPerElement;
        }

        //For regular maps, mLastMappingStart is 0. So that we can later flush correctly.
        mBuffer->mLastMappingStart = 0;
        mBuffer->mLastMappingCount = elementCount;

        char *retVal = (char*)mMappedPtr;

        if( mBuffer->mBufferType >= BT_DYNAMIC_PERSISTENT && canPersistentMap )
        {
            //For persistent maps, we've mapped the whole 3x size of the buffer. mLastMappingStart
            //points to the right offset so that we can later flush correctly.
            size_t lastMappingStart = elementStart +
                    mBuffer->_getInternalNumElements() * dynamicCurrentFrame;
            mBuffer->mLastMappingStart = lastMappingStart;
            retVal += lastMappingStart * bytesPerElement;
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void NULLBufferInterface::unmap( UnmapOptions unmapOption,
                                        size_t flushStartElem, size_t flushSizeElem )
    {
        assert( flushStartElem <= mBuffer->mLastMappingCount &&
                "Flush starts after the end of the mapped region!" );
        assert( flushStartElem + flushSizeElem <= mBuffer->mLastMappingCount &&
                "Flush region out of bounds!" );

        bool canPersistentMap = static_cast<NULLVaoManager*>( mBuffer->mVaoManager )->
                                                                supportsArbBufferStorage();

        if( mBuffer->mBufferType <= BT_DYNAMIC_PERSISTENT ||
            unmapOption == UO_UNMAP_ALL || !canPersistentMap )
        {
            if( !flushSizeElem )
                flushSizeElem = mBuffer->mLastMappingCount - flushStartElem;

            if( unmapOption == UO_UNMAP_ALL || !canPersistentMap ||
                mBuffer->mBufferType == BT_DYNAMIC_DEFAULT )
            {
                mMappedPtr = 0;
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void NULLBufferInterface::advanceFrame(void)
    {
        advanceFrame( true );
    }
    //-----------------------------------------------------------------------------------
    size_t NULLBufferInterface::advanceFrame( bool bAdvanceFrame )
    {
        NULLVaoManager *vaoManager = static_cast<NULLVaoManager*>( mBuffer->mVaoManager );
        size_t dynamicCurrentFrame = mBuffer->mFinalBufferStart - mBuffer->mInternalBufferStart;
        dynamicCurrentFrame /= mBuffer->_getInternalNumElements();

        dynamicCurrentFrame = (dynamicCurrentFrame + 1) % vaoManager->getDynamicBufferMultiplier();

        if( bAdvanceFrame )
        {
            mBuffer->mFinalBufferStart = mBuffer->mInternalBufferStart +
                                            dynamicCurrentFrame * mBuffer->_getInternalNumElements();
        }

        return dynamicCurrentFrame;
    }
    //-----------------------------------------------------------------------------------
    void NULLBufferInterface::regressFrame(void)
    {
        NULLVaoManager *vaoManager = static_cast<NULLVaoManager*>( mBuffer->mVaoManager );
        size_t dynamicCurrentFrame = mBuffer->mFinalBufferStart - mBuffer->mInternalBufferStart;
        dynamicCurrentFrame /= mBuffer->_getInternalNumElements();

        dynamicCurrentFrame = (dynamicCurrentFrame + vaoManager->getDynamicBufferMultiplier() - 1) %
                                vaoManager->getDynamicBufferMultiplier();

        mBuffer->mFinalBufferStart = mBuffer->mInternalBufferStart +
                                        dynamicCurrentFrame * mBuffer->_getInternalNumElements();
    }
    //-----------------------------------------------------------------------------------
    void NULLBufferInterface::_notifyBuffer( BufferPacked *buffer )
    {
        BufferInterface::_notifyBuffer( buffer );

        mNullDataPtr = reinterpret_cast<uint8*>( OGRE_MALLOC_SIMD( mBuffer->getTotalSizeBytes(),
                                                                    MEMCATEGORY_RENDERSYS ) );
    }
}
