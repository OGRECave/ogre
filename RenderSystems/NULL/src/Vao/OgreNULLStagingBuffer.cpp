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

#include "Vao/OgreNULLStagingBuffer.h"
#include "Vao/OgreNULLVaoManager.h"
#include "Vao/OgreNULLBufferInterface.h"

#include "OgreStringConverter.h"

namespace Ogre
{
    NULLStagingBuffer::NULLStagingBuffer( size_t internalBufferStart, size_t sizeBytes,
                                                VaoManager *vaoManager, bool uploadOnly ) :
        StagingBuffer( internalBufferStart, sizeBytes, vaoManager, uploadOnly ),
        mMappedPtr( 0 ),
        mNullDataPtr( 0 )
    {
        mNullDataPtr = reinterpret_cast<uint8*>( OGRE_MALLOC_SIMD( sizeBytes, MEMCATEGORY_RENDERSYS ) );
    }
    //-----------------------------------------------------------------------------------
    NULLStagingBuffer::~NULLStagingBuffer()
    {
        OGRE_FREE_SIMD( mNullDataPtr, MEMCATEGORY_RENDERSYS );
        mNullDataPtr = 0;
    }
    //-----------------------------------------------------------------------------------
    void* NULLStagingBuffer::mapImpl( size_t sizeBytes )
    {
        assert( mUploadOnly );

        mMappingStart = 0;
        mMappingCount = sizeBytes;

        mMappedPtr = mNullDataPtr + mInternalBufferStart + mMappingStart;

        return mMappedPtr;
    }
    //-----------------------------------------------------------------------------------
    void NULLStagingBuffer::unmapImpl( const Destination *destinations, size_t numDestinations )
    {
        mMappedPtr = 0;

        for( size_t i=0; i<numDestinations; ++i )
        {
            const Destination &dst = destinations[i];

            NULLBufferInterface *bufferInterface = static_cast<NULLBufferInterface*>(
                                                        dst.destination->getBufferInterface() );

            assert( dst.destination->getBufferType() == BT_DEFAULT );

            size_t dstOffset = dst.dstOffset + dst.destination->_getInternalBufferStart() *
                                                        dst.destination->getBytesPerElement();

            uint8 *dstPtr = bufferInterface->getNullDataPtr();

            memcpy( dstPtr + dstOffset,
                    mNullDataPtr + mInternalBufferStart + mMappingStart + dst.srcOffset,
                    dst.length );
        }
    }
    //-----------------------------------------------------------------------------------
    StagingStallType NULLStagingBuffer::uploadWillStall( size_t sizeBytes )
    {
        assert( mUploadOnly );
        return STALL_NONE;
    }
    //-----------------------------------------------------------------------------------
    //
    //  DOWNLOADS
    //
    //-----------------------------------------------------------------------------------
    size_t NULLStagingBuffer::_asyncDownload( BufferPacked *source, size_t srcOffset,
                                                 size_t srcLength )
    {
        size_t freeRegionOffset = getFreeDownloadRegion( srcLength );

        if( freeRegionOffset == -1u )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "Cannot download the request amount of " +
                         StringConverter::toString( srcLength ) + " bytes to this staging buffer. "
                         "Try another one (we're full of requests that haven't been read by CPU yet)",
                         "NULLStagingBuffer::_asyncDownload" );
        }

        assert( !mUploadOnly );
        assert( dynamic_cast<NULLBufferInterface*>( source->getBufferInterface() ) );
        assert( (srcOffset + srcLength) <= source->getTotalSizeBytes() );

        NULLBufferInterface *bufferInterface = static_cast<NULLBufferInterface*>(
                                                            source->getBufferInterface() );

        uint8 *srcPtr = bufferInterface->getNullDataPtr();

        memcpy( mNullDataPtr + mInternalBufferStart + freeRegionOffset,
                srcPtr + source->_getFinalBufferStart() * source->getBytesPerElement() + srcOffset,
                srcLength );

        return freeRegionOffset;
    }
    //-----------------------------------------------------------------------------------
    const void* NULLStagingBuffer::_mapForReadImpl( size_t offset, size_t sizeBytes )
    {
        assert( !mUploadOnly );

        mMappingStart = offset;
        mMappingCount = sizeBytes;

        mMappedPtr = mNullDataPtr + mInternalBufferStart + mMappingStart;

        //Put the mapped region back to our records as "available" for subsequent _asyncDownload
        _cancelDownload( offset, sizeBytes );

        return mMappedPtr;
    }
}
