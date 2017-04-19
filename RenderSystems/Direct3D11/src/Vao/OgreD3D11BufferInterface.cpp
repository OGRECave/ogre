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

#include "Vao/OgreD3D11BufferInterface.h"
#include "Vao/OgreD3D11VaoManager.h"
#include "Vao/OgreD3D11StagingBuffer.h"
#include "Vao/OgreD3D11DynamicBuffer.h"

namespace Ogre
{
    D3D11BufferInterface::D3D11BufferInterface( size_t vboPoolIdx, ID3D11Buffer *d3dBuffer,
                                                D3D11DynamicBuffer *dynamicBuffer ) :
        mVboPoolIdx( vboPoolIdx ),
        mVboName( d3dBuffer ),
        mMappedPtr( 0 ),
        mUnmapTicket( (size_t)~0 ),
        mDynamicBuffer( dynamicBuffer ),
        mInitialData( 0 )
    {
    }
    //-----------------------------------------------------------------------------------
    D3D11BufferInterface::~D3D11BufferInterface()
    {
        _deleteInitialData();
    }
    //-----------------------------------------------------------------------------------
    void D3D11BufferInterface::_firstUpload( void *data )
    {
        //In D3D11, we upload non-immutable buffers the traditional way.
        //For immutable buffers, we store the data into CPU memory until the last moment
        //when the buffer may be need, and only then gets the D3D11 buffer created and
        //batched together with many other buffers (and mInitialData may be freed then).

        if( mBuffer->mBufferType == BT_IMMUTABLE )
        {
            if( mBuffer->mShadowCopy )
            {
                //Reference the shadow copy directly.
                mInitialData = mBuffer->mShadowCopy;
            }
            else
            {
                //The initial data pointer may be lost. Copy it to a temporary location.
                mInitialData = OGRE_MALLOC_SIMD( mBuffer->getTotalSizeBytes(), MEMCATEGORY_GEOMETRY );
                memcpy( mInitialData, data, mBuffer->getTotalSizeBytes() );
            }
        }
        else
        {
            upload( data, 0, mBuffer->getNumElements() );
        }
    }
    //-----------------------------------------------------------------------------------
    void* RESTRICT_ALIAS_RETURN D3D11BufferInterface::map( size_t elementStart, size_t elementCount,
                                                           MappingState prevMappingState,
                                                           bool bAdvanceFrame )
    {
        size_t bytesPerElement = mBuffer->mBytesPerElement;

        D3D11VaoManager *vaoManager = static_cast<D3D11VaoManager*>( mBuffer->mVaoManager );

        vaoManager->waitForTailFrameToFinish();

        size_t dynamicCurrentFrame = advanceFrame( bAdvanceFrame );

        {
            //Non-persistent buffers just map the small region they'll need.
            size_t offset = mBuffer->mInternalBufferStart + elementStart +
                            mBuffer->mNumElements * dynamicCurrentFrame;
            size_t length = elementCount;

            mMappedPtr = mDynamicBuffer->map( offset * bytesPerElement,
                                              length * bytesPerElement,
                                              mUnmapTicket );
        }

        //For regular maps, mLastMappingStart is 0. So that we can later flush correctly.
        mBuffer->mLastMappingStart = 0;
        mBuffer->mLastMappingCount = elementCount;

        return mMappedPtr;
    }
    //-----------------------------------------------------------------------------------
    void D3D11BufferInterface::_deleteInitialData(void)
    {
        if( mInitialData )
        {
            if( !mBuffer->mShadowCopy )
                OGRE_FREE_SIMD( mInitialData, MEMCATEGORY_GEOMETRY );
            mInitialData = 0;
        }
    }
    //-----------------------------------------------------------------------------------
    void D3D11BufferInterface::_setVboName( size_t vboPoolIdx, ID3D11Buffer *vboName,
                                            size_t internalBufferStartBytes )
    {
        mVboPoolIdx = vboPoolIdx;
        mVboName    = vboName;

        mBuffer->mInternalBufferStart   = internalBufferStartBytes / mBuffer->mBytesPerElement;
        mBuffer->mFinalBufferStart      = internalBufferStartBytes / mBuffer->mBytesPerElement;
    }
    //-----------------------------------------------------------------------------------
    void D3D11BufferInterface::unmap( UnmapOptions unmapOption,
                                      size_t flushStartElem, size_t flushSizeElem )
    {
        //All arguments aren't really used by D3D11, these asserts are for the other APIs.
        assert( flushStartElem <= mBuffer->mLastMappingCount &&
                "Flush starts after the end of the mapped region!" );
        assert( flushStartElem + flushSizeElem <= mBuffer->mLastMappingCount &&
                "Flush region out of bounds!" );

        mDynamicBuffer->unmap( mUnmapTicket );
        mMappedPtr = 0;
    }
    //-----------------------------------------------------------------------------------
    void D3D11BufferInterface::advanceFrame(void)
    {
        advanceFrame( true );
    }
    //-----------------------------------------------------------------------------------
    size_t D3D11BufferInterface::advanceFrame( bool bAdvanceFrame )
    {
        D3D11VaoManager *vaoManager = static_cast<D3D11VaoManager*>( mBuffer->mVaoManager );
        size_t dynamicCurrentFrame = mBuffer->mFinalBufferStart - mBuffer->mInternalBufferStart;
        dynamicCurrentFrame /= mBuffer->mNumElements;

        dynamicCurrentFrame = (dynamicCurrentFrame + 1) % vaoManager->getDynamicBufferMultiplier();

        if( bAdvanceFrame )
        {
            mBuffer->mFinalBufferStart = mBuffer->mInternalBufferStart +
                                            dynamicCurrentFrame * mBuffer->mNumElements;
        }

        return dynamicCurrentFrame;
    }
    //-----------------------------------------------------------------------------------
    void D3D11BufferInterface::regressFrame(void)
    {
        D3D11VaoManager *vaoManager = static_cast<D3D11VaoManager*>( mBuffer->mVaoManager );
        size_t dynamicCurrentFrame = mBuffer->mFinalBufferStart - mBuffer->mInternalBufferStart;
        dynamicCurrentFrame /= mBuffer->mNumElements;

        dynamicCurrentFrame = (dynamicCurrentFrame + vaoManager->getDynamicBufferMultiplier() - 1) %
                                vaoManager->getDynamicBufferMultiplier();

        mBuffer->mFinalBufferStart = mBuffer->mInternalBufferStart +
                                        dynamicCurrentFrame * mBuffer->mNumElements;
    }
}
