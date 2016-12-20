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

#include "Vao/OgreD3D11StagingBuffer.h"
#include "Vao/OgreD3D11VaoManager.h"
#include "Vao/OgreD3D11BufferInterface.h"
#include "OgreD3D11Device.h"

#include "OgreStringConverter.h"

namespace Ogre
{
    D3D11StagingBuffer::D3D11StagingBuffer( size_t sizeBytes, VaoManager *vaoManager, bool uploadOnly,
                                            ID3D11Buffer *stagingBuffer, D3D11Device &device ) :
        StagingBuffer( 0 /*No internalBufferStart in D3D11 (should be implemented in D3D12)*/,
                       sizeBytes, vaoManager, uploadOnly ),
        mVboName( stagingBuffer ),
        mMappedPtr( 0 ),
        mDevice( device )
    {
    }
    //-----------------------------------------------------------------------------------
    D3D11StagingBuffer::~D3D11StagingBuffer()
    {
    }
    //-----------------------------------------------------------------------------------
    void* D3D11StagingBuffer::mapImpl( size_t sizeBytes )
    {
        assert( mUploadOnly );

        D3D11_MAP mapFlag = D3D11_MAP_WRITE_NO_OVERWRITE;

        if( mMappingStart + sizeBytes > mSizeBytes )
            mMappingStart = 0;

        if( mMappingStart == 0 )
            mapFlag = D3D11_MAP_WRITE_DISCARD;

        mMappingCount = sizeBytes;

        D3D11_MAPPED_SUBRESOURCE mappedSubres;
        mDevice.GetImmediateContext()->Map( mVboName, 0, mapFlag,
                                            0, &mappedSubres );
        mMappedPtr = reinterpret_cast<uint8*>( mappedSubres.pData ) + mMappingStart;

        return mMappedPtr;
    }
    //-----------------------------------------------------------------------------------
    void D3D11StagingBuffer::unmapImpl( const Destination *destinations, size_t numDestinations )
    {
        ID3D11DeviceContextN *d3dContext = mDevice.GetImmediateContext();
        d3dContext->Unmap( mVboName, 0 );
        mMappedPtr = 0;

        for( size_t i=0; i<numDestinations; ++i )
        {
            const Destination &dst = destinations[i];

            D3D11BufferInterface *bufferInterface = static_cast<D3D11BufferInterface*>(
                                                        dst.destination->getBufferInterface() );

            assert( dst.destination->getBufferType() == BT_DEFAULT );

            UINT dstOffset = static_cast<UINT>( dst.dstOffset +
                                                dst.destination->_getInternalBufferStart() *
                                                dst.destination->getBytesPerElement() );

            D3D11_BOX srcBox;
            ZeroMemory( &srcBox, sizeof(D3D11_BOX) );
            srcBox.left     = mMappingStart + dst.srcOffset;
            srcBox.right    = mMappingStart + dst.srcOffset + dst.length;
            srcBox.back     = 1;
            srcBox.bottom   = 1;

            d3dContext->CopySubresourceRegion( bufferInterface->getVboName(), 0,
                                               dstOffset, 0, 0, mVboName, 0, &srcBox );
        }

        //We must wrap exactly to zero so that next map uses DISCARD.
        if( mMappingStart + mMappingCount > mSizeBytes )
            mMappingStart = 0;
    }
    //-----------------------------------------------------------------------------------
    StagingStallType D3D11StagingBuffer::uploadWillStall( size_t sizeBytes )
    {
        assert( mUploadOnly );

        size_t mappingStart = mMappingStart;

        StagingStallType retVal = STALL_NONE;

        //We don't really know, it's inaccurate. But don't trust the driver
        //will be able to run a stall-free discard every time; thus STALL_PARTIAL
        if( mappingStart + sizeBytes > mSizeBytes )
        {
            retVal = STALL_PARTIAL;
            mappingStart = 0;
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    //
    //  DOWNLOADS
    //
    //-----------------------------------------------------------------------------------
    size_t D3D11StagingBuffer::_asyncDownload( BufferPacked *source, size_t srcOffset,
                                               size_t srcLength )
    {
        size_t freeRegionOffset = getFreeDownloadRegion( srcLength );

        if( freeRegionOffset == -1 )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "Cannot download the request amount of " +
                         StringConverter::toString( srcLength ) + " bytes to this staging buffer. "
                         "Try another one (we're full of requests that haven't been read by CPU yet)",
                         "D3D11StagingBuffer::_asyncDownload" );
        }

        assert( !mUploadOnly );
        assert( dynamic_cast<D3D11BufferInterface*>( source->getBufferInterface() ) );
        assert( (srcOffset + srcLength) <= source->getTotalSizeBytes() );

        D3D11BufferInterface *bufferInterface = static_cast<D3D11BufferInterface*>(
                                                            source->getBufferInterface() );

        D3D11_BOX srcBox;
        ZeroMemory( &srcBox, sizeof(D3D11_BOX) );
        srcBox.left     = source->_getFinalBufferStart() * source->getBytesPerElement() + srcOffset;
        srcBox.right    = srcBox.left + srcLength;
        srcBox.back     = 1;
        srcBox.bottom   = 1;

        ID3D11DeviceContextN *d3dContext = mDevice.GetImmediateContext();
        d3dContext->CopySubresourceRegion( mVboName, 0, freeRegionOffset,
                                           0, 0, bufferInterface->getVboName(),
                                           0, &srcBox );

        return freeRegionOffset;
    }
    //-----------------------------------------------------------------------------------
    const void* D3D11StagingBuffer::_mapForReadImpl( size_t offset, size_t sizeBytes )
    {
        assert( !mUploadOnly );

        mMappingStart = offset;
        mMappingCount = sizeBytes;

        D3D11_MAPPED_SUBRESOURCE mappedSubres;
        mDevice.GetImmediateContext()->Map( mVboName, 0, D3D11_MAP_READ, 0, &mappedSubres );
        mMappedPtr = reinterpret_cast<uint8*>( mappedSubres.pData ) + mMappingStart;

        //Put the mapped region back to our records as "available" for subsequent _asyncDownload
        _cancelDownload( offset, sizeBytes );

        return mMappedPtr;
    }
}
