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

#include "Vao/OgreD3D11VaoManager.h"
#include "Vao/OgreD3D11StagingBuffer.h"
#include "Vao/OgreD3D11VertexArrayObject.h"
#include "Vao/OgreD3D11BufferInterface.h"
#include "Vao/OgreD3D11CompatBufferInterface.h"
#include "Vao/OgreD3D11ConstBufferPacked.h"
#include "Vao/OgreD3D11TexBufferPacked.h"
//#include "Vao/OgreD3D11MultiSourceVertexBufferPool.h"
#include "Vao/OgreD3D11DynamicBuffer.h"
#include "Vao/OgreD3D11AsyncTicket.h"

#include "Vao/OgreIndirectBufferPacked.h"

#include "OgreD3D11Device.h"
#include "OgreD3D11HLSLProgram.h"
#include "OgreD3D11RenderSystem.h"

#include "OgreRenderQueue.h"

#include "OgreD3D11HardwareBufferManager.h" //D3D11HardwareBufferManager::getGLType

#include "OgreTimer.h"
#include "OgreStringConverter.h"

namespace Ogre
{

    D3D11VaoManager::D3D11VaoManager( bool _supportsIndirectBuffers, D3D11Device &device,
                                      D3D11RenderSystem *renderSystem ) :
        mVaoNames( 1 ),
        mDevice( device ),
        mDrawId( 0 ),
        mD3D11RenderSystem( renderSystem )
    {
        mDefaultPoolSize[VERTEX_BUFFER][BT_IMMUTABLE]   = 64 * 1024 * 1024;
        mDefaultPoolSize[INDEX_BUFFER][BT_IMMUTABLE]    = 64 * 1024 * 1024;
        mDefaultPoolSize[SHADER_BUFFER][BT_IMMUTABLE]   = 64 * 1024 * 1024;

        mDefaultPoolSize[VERTEX_BUFFER][BT_DEFAULT]     = 32 * 1024 * 1024;
        mDefaultPoolSize[INDEX_BUFFER][BT_DEFAULT]      = 16 * 1024 * 1024;
        mDefaultPoolSize[SHADER_BUFFER][BT_DEFAULT]     = 16 * 1024 * 1024;

        mDefaultPoolSize[VERTEX_BUFFER][BT_DYNAMIC_DEFAULT] = 16 * 1024 * 1024;
        mDefaultPoolSize[INDEX_BUFFER][BT_DYNAMIC_DEFAULT]  = 16 * 1024 * 1024;
        mDefaultPoolSize[SHADER_BUFFER][BT_DYNAMIC_DEFAULT] = 16 * 1024 * 1024;

        mFrameSyncVec.resize( mDynamicBufferMultiplier, 0 );

        //There's no way to query the API, grrr... but this
        //is the minimum supported by all known GPUs
        mConstBufferAlignment   = 256;
        mTexBufferAlignment     = 256;

        //64kb by spec. DX 11.1 on Windows 8.1 allows to query to see if there's more.
        //But it's a PITA to use a special path just for that.
        mConstBufferMaxSize = 4096 * 1024;
        //Umm... who knows? Just use GL3's minimum.
        mTexBufferMaxSize = 128 * 1024 * 1024;

        mSupportsPersistentMapping  = false;
        mSupportsIndirectBuffers    = _supportsIndirectBuffers;

        VertexElement2Vec vertexElements;
        vertexElements.push_back( VertexElement2( VET_UINT1, VES_COUNT ) );
        uint32 *drawIdPtr = static_cast<uint32*>( OGRE_MALLOC_SIMD( 4096 * sizeof(uint32),
                                                                    MEMCATEGORY_GEOMETRY ) );
        for( uint32 i=0; i<4096; ++i )
            drawIdPtr[i] = i;
        mDrawId = createVertexBuffer( vertexElements, 4096, BT_IMMUTABLE, drawIdPtr, true );
    }
    //-----------------------------------------------------------------------------------
    D3D11VaoManager::~D3D11VaoManager()
    {
        {
            //Destroy all buffers which don't have a pool
            BufferPackedVec::const_iterator itor = mBuffers[BP_TYPE_CONST].begin();
            BufferPackedVec::const_iterator end  = mBuffers[BP_TYPE_CONST].end();
            while( itor != end )
                destroyConstBufferImpl( static_cast<ConstBufferPacked*>( *itor++ ) );
        }

        destroyAllVertexArrayObjects();
        deleteAllBuffers();

        for( size_t i=0; i<2; ++i )
        {
            //Collect the buffer names from all staging buffers to use one API call
            StagingBufferVec::const_iterator itor = mStagingBuffers[i].begin();
            StagingBufferVec::const_iterator end  = mStagingBuffers[i].end();

            while( itor != end )
            {
                static_cast<D3D11StagingBuffer*>(*itor)->getBufferName()->Release();
                ++itor;
            }
        }

        for( size_t i=0; i<NumInternalBufferTypes; ++i )
        {
            for( size_t j=0; j<BT_DYNAMIC_DEFAULT+1; ++j )
            {
                //Free pointers and collect the buffer names from all VBOs to use one API call
                VboVec::iterator itor = mVbos[i][j].begin();
                VboVec::iterator end  = mVbos[i][j].end();

                while( itor != end )
                {
                    if( itor->vboName )
                        itor->vboName->Release();
                    delete itor->dynamicBuffer;
                    itor->dynamicBuffer = 0;
                    ++itor;
                }
            }
        }

        D3D11SyncVec::const_iterator itor = mFrameSyncVec.begin();
        D3D11SyncVec::const_iterator end  = mFrameSyncVec.end();

        while( itor != end )
        {
            (*itor)->Release();
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void D3D11VaoManager::allocateVbo( size_t sizeBytes, size_t alignment, BufferType bufferType,
                                       InternalBufferType internalType,
                                       size_t &outVboIdx, size_t &outBufferOffset )
    {
        assert( alignment > 0 );

        //Immutable buffers are delayed as much as possible so we can merge all of them.
        //See createDelayedImmutableBuffers
        assert( bufferType != BT_IMMUTABLE );

        if( bufferType >= BT_DYNAMIC_DEFAULT )
        {
            bufferType  = BT_DYNAMIC_DEFAULT; //Persitent mapping not supported in D3D11.
            sizeBytes   *= mDynamicBufferMultiplier;
        }

        VboVec::const_iterator itor = mVbos[internalType][bufferType].begin();
        VboVec::const_iterator end  = mVbos[internalType][bufferType].end();

        //Find a suitable VBO that can hold the requested size. We prefer those free
        //blocks that have a matching stride (the current offset is a multiple of
        //bytesPerElement) in order to minimize the amount of memory padding.
        size_t bestVboIdx   = ~0;
        size_t bestBlockIdx = ~0;
        bool foundMatchingStride = false;

        while( itor != end && !foundMatchingStride )
        {
            BlockVec::const_iterator blockIt = itor->freeBlocks.begin();
            BlockVec::const_iterator blockEn = itor->freeBlocks.end();

            while( blockIt != blockEn && !foundMatchingStride )
            {
                const Block &block = *blockIt;

                //Round to next multiple of alignment
                size_t newOffset = ( (block.offset + alignment - 1) / alignment ) * alignment;

                if( sizeBytes <= block.size - (newOffset - block.offset) )
                {
                    bestVboIdx      = itor - mVbos[internalType][bufferType].begin();
                    bestBlockIdx    = blockIt - itor->freeBlocks.begin();

                    if( newOffset == block.offset )
                        foundMatchingStride = true;
                }

                ++blockIt;
            }

            ++itor;
        }

        if( bestBlockIdx == (size_t)~0 )
        {
            bestVboIdx      = mVbos[internalType][bufferType].size();
            bestBlockIdx    = 0;
            foundMatchingStride = true;

            Vbo newVbo;

            size_t poolSize = std::max( mDefaultPoolSize[internalType][bufferType], sizeBytes );

            ID3D11DeviceN *d3dDevice = mDevice.get();

            D3D11_BUFFER_DESC desc;
            ZeroMemory( &desc, sizeof(D3D11_BUFFER_DESC) );
            desc.ByteWidth  = poolSize;
            desc.CPUAccessFlags = 0;
            if( bufferType == BT_IMMUTABLE )
                desc.Usage = D3D11_USAGE_IMMUTABLE;
            else if( bufferType == BT_DEFAULT )
                desc.Usage = D3D11_USAGE_DEFAULT;
            else if( bufferType >= BT_DYNAMIC_DEFAULT )
            {
                desc.Usage = D3D11_USAGE_DYNAMIC;
                desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            }

            if( internalType == VERTEX_BUFFER )
                desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            else if( internalType == INDEX_BUFFER )
                desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
            else
                desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

            HRESULT hr = d3dDevice->CreateBuffer( &desc, 0, &newVbo.vboName );

            if( FAILED( hr ) )
            {
                String errorDescription = mDevice.getErrorDescription(hr);
                OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                             "Failed to create buffer. ID3D11Device::CreateBuffer.\n"
                             "Error code: " + StringConverter::toString( hr ) + ".\n" +
                             errorDescription +
                             "Requested: " + StringConverter::toString( poolSize ) + " bytes.",
                             "D3D11VaoManager::allocateVbo" );
            }

            newVbo.sizeBytes = poolSize;
            newVbo.freeBlocks.push_back( Block( 0, poolSize ) );
            newVbo.dynamicBuffer = 0;

            if( bufferType >= BT_DYNAMIC_DEFAULT )
            {
                newVbo.dynamicBuffer = new D3D11DynamicBuffer( newVbo.vboName, newVbo.sizeBytes,
                                                               mDevice );
            }

            mVbos[internalType][bufferType].push_back( newVbo );
        }

        Vbo &bestVbo        = mVbos[internalType][bufferType][bestVboIdx];
        Block &bestBlock    = bestVbo.freeBlocks[bestBlockIdx];

        size_t newOffset = ( (bestBlock.offset + alignment - 1) / alignment ) * alignment;
        size_t padding = newOffset - bestBlock.offset;
        //Shrink our records about available data.
        bestBlock.size   -= sizeBytes + padding;
        bestBlock.offset = newOffset + sizeBytes;

        if( !foundMatchingStride )
        {
            //This is a stride changer, record as such.
            StrideChangerVec::iterator itStride = std::lower_bound( bestVbo.strideChangers.begin(),
                                                                    bestVbo.strideChangers.end(),
                                                                    newOffset, StrideChanger() );
            bestVbo.strideChangers.insert( itStride, StrideChanger( newOffset, padding ) );
        }

        if( bestBlock.size == 0 )
            bestVbo.freeBlocks.erase( bestVbo.freeBlocks.begin() + bestBlockIdx );

        outVboIdx       = bestVboIdx;
        outBufferOffset = newOffset;
    }
    //-----------------------------------------------------------------------------------
    void D3D11VaoManager::deallocateVbo( size_t vboIdx, size_t bufferOffset, size_t sizeBytes,
                                         BufferType bufferType, InternalBufferType internalType )
    {
        assert( bufferType <= BT_DYNAMIC_DEFAULT );

        if( bufferType >= BT_DYNAMIC_DEFAULT )
            sizeBytes *= mDynamicBufferMultiplier;

        Vbo &vbo = mVbos[internalType][bufferType][vboIdx];
        StrideChangerVec::iterator itStride = std::lower_bound( vbo.strideChangers.begin(),
                                                                vbo.strideChangers.end(),
                                                                bufferOffset, StrideChanger() );

        if( itStride != vbo.strideChangers.end() && itStride->offsetAfterPadding == bufferOffset )
        {
            bufferOffset    -= itStride->paddedBytes;
            sizeBytes       += itStride->paddedBytes;

            vbo.strideChangers.erase( itStride );
        }

        //See if we're contiguous to a free block and make that block grow.
        vbo.freeBlocks.push_back( Block( bufferOffset, sizeBytes ) );
        mergeContiguousBlocks( vbo.freeBlocks.end() - 1, vbo.freeBlocks );

        if( vbo.freeBlocks.back().size == vbo.sizeBytes && bufferType == BT_IMMUTABLE )
        {
            //Immutable buffer is empty. It can't be filled again. Release the GPU memory.
            //The vbo is not removed from mVbos since that would alter the index of other
            //buffers (except if this is the last one).
            vbo.vboName->Release();
            vbo.vboName = 0;

            while( !mVbos[internalType][bufferType].empty() &&
                   mVbos[internalType][bufferType].back().vboName == 0 )
            {
                mVbos[internalType][bufferType].pop_back();
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void D3D11VaoManager::createImmutableBuffer( InternalBufferType internalType,
                                                 size_t sizeBytes, void *initialData,
                                                 Vbo &inOutVbo )
    {
        const size_t poolSize = sizeBytes;

        ID3D11DeviceN *d3dDevice = mDevice.get();

        D3D11_BUFFER_DESC desc;
        ZeroMemory( &desc, sizeof(D3D11_BUFFER_DESC) );
        desc.ByteWidth  = poolSize;
        desc.CPUAccessFlags = 0;
        desc.Usage = D3D11_USAGE_IMMUTABLE;

        if( internalType == VERTEX_BUFFER )
            desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        else if( internalType == INDEX_BUFFER )
            desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        else
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA subResData;
        ZeroMemory( &subResData, sizeof(D3D11_SUBRESOURCE_DATA) );
        subResData.pSysMem = initialData;

        HRESULT hr = d3dDevice->CreateBuffer( &desc, &subResData, &inOutVbo.vboName );

        if( FAILED( hr ) )
        {
            String errorDescription = mDevice.getErrorDescription(hr);
            OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                         "Failed to create buffer. ID3D11Device::CreateBuffer.\n"
                         "Error code: " + StringConverter::toString( hr ) + ".\n" +
                         errorDescription +
                         "Requested: " + StringConverter::toString( poolSize ) + " bytes.",
                         "D3D11VaoManager::createImmutableBuffer" );
        }

        inOutVbo.sizeBytes = poolSize;
        inOutVbo.freeBlocks.clear(); //No free blocks
        inOutVbo.dynamicBuffer = 0;

        mVbos[internalType][BT_IMMUTABLE].push_back( inOutVbo );
    }
    //-----------------------------------------------------------------------------------
    void D3D11VaoManager::mergeContiguousBlocks( BlockVec::iterator blockToMerge,
                                                   BlockVec &blocks )
    {
        BlockVec::iterator itor = blocks.begin();
        BlockVec::iterator end  = blocks.end();

        while( itor != end )
        {
            if( itor->offset + itor->size == blockToMerge->offset )
            {
                itor->size += blockToMerge->size;
                size_t idx = itor - blocks.begin();

                //When blockToMerge is the last one, its index won't be the same
                //after removing the other iterator, they will swap.
                if( idx == blocks.size() - 1 )
                    idx = blockToMerge - blocks.begin();

                efficientVectorRemove( blocks, blockToMerge );

                blockToMerge = blocks.begin() + idx;
                itor = blocks.begin();
                end  = blocks.end();
            }
            else if( blockToMerge->offset + blockToMerge->size == itor->offset )
            {
                blockToMerge->size += itor->size;
                size_t idx = blockToMerge - blocks.begin();

                //When blockToMerge is the last one, its index won't be the same
                //after removing the other iterator, they will swap.
                if( idx == blocks.size() - 1 )
                    idx = itor - blocks.begin();

                efficientVectorRemove( blocks, itor );

                blockToMerge = blocks.begin() + idx;
                itor = blocks.begin();
                end  = blocks.end();
            }
            else
            {
                ++itor;
            }
        }
    }
    //-----------------------------------------------------------------------------------
    VertexBufferPacked* D3D11VaoManager::createVertexBufferImpl( size_t numElements,
                                                                 uint32 bytesPerElement,
                                                                 BufferType bufferType,
                                                                 void *initialData, bool keepAsShadow,
                                                                 const VertexElement2Vec &vElements )
    {
        size_t vboIdx;
        size_t bufferOffset;
        ID3D11Buffer *vboName;
        D3D11DynamicBuffer *dynamicBuffer;

        if( bufferType == BT_IMMUTABLE )
        {
            vboIdx          = 0;
            bufferOffset    = 0;
            vboName         = 0;
            dynamicBuffer   = 0;
        }
        else
        {
            allocateVbo( numElements * bytesPerElement, bytesPerElement, bufferType, VERTEX_BUFFER,
                         vboIdx, bufferOffset );

            BufferType vboFlag = bufferType;
            if( vboFlag >= BT_DYNAMIC_DEFAULT )
                vboFlag = BT_DYNAMIC_DEFAULT;
            Vbo &vbo = mVbos[VERTEX_BUFFER][vboFlag][vboIdx];
            vboName         = vbo.vboName;
            dynamicBuffer   = vbo.dynamicBuffer;
        }

        D3D11BufferInterface *bufferInterface = new D3D11BufferInterface( vboIdx, vboName,
                                                                          dynamicBuffer );
        VertexBufferPacked *retVal = OGRE_NEW VertexBufferPacked(
                                                        bufferOffset, numElements, bytesPerElement,
                                                        bufferType, initialData, keepAsShadow,
                                                        this, bufferInterface, vElements, 0, 0, 0 );

        if( initialData )
            bufferInterface->_firstUpload( initialData );

        if( bufferType == BT_IMMUTABLE )
            mDelayedBuffers[VERTEX_BUFFER].push_back( retVal );

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void D3D11VaoManager::destroyVertexBufferImpl( VertexBufferPacked *vertexBuffer )
    {
        D3D11BufferInterface *bufferInterface = static_cast<D3D11BufferInterface*>(
                                                        vertexBuffer->getBufferInterface() );

        if( bufferInterface->_getInitialData() != 0 )
        {
            //Immutable buffer that never made to the GPU.
            removeBufferFromDelayedQueue( mDelayedBuffers[VERTEX_BUFFER], vertexBuffer );
        }
        else
        {
            deallocateVbo( bufferInterface->getVboPoolIndex(),
                           vertexBuffer->_getInternalBufferStart() * vertexBuffer->getBytesPerElement(),
                           vertexBuffer->getNumElements() * vertexBuffer->getBytesPerElement(),
                           vertexBuffer->getBufferType(), VERTEX_BUFFER );
        }
    }
    //-----------------------------------------------------------------------------------
    void D3D11VaoManager::removeBufferFromDelayedQueue( BufferPackedVec &container,
                                                        BufferPacked *buffer )
    {
        assert( buffer->getBufferType() == BT_IMMUTABLE );

        BufferPackedVec::iterator itor = container.begin();
        BufferPackedVec::iterator end  = container.end();
        while( itor != end )
        {
            if( *itor == buffer )
            {
                itor = efficientVectorRemove( container, itor );
                itor = container.end();
                end  = container.end();
            }
            else
            {
                ++itor;
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void D3D11VaoManager::createDelayedImmutableBuffers(void)
    {
        size_t totalBytes = 0;

        for( size_t i=0; i<NumInternalBufferTypes; ++i )
        {
            BufferPackedVec::const_iterator itor = mDelayedBuffers[i].begin();

            while( itor != mDelayedBuffers[i].end() )
            {
                //Each iteration means a new pool
                Vbo newVbo;

                //Calculate the size for this pool
                BufferPackedVec::const_iterator end = mDelayedBuffers[i].end();

                while( itor != end )
                {
                    totalBytes = alignToNextMultiple( totalBytes, (*itor)->getBytesPerElement() );
                    totalBytes += (*itor)->getTotalSizeBytes();

                    if( totalBytes + (*itor)->getTotalSizeBytes() > mDefaultPoolSize[i][BT_IMMUTABLE] )
                        end = itor + 1;

                    ++itor;
                }

                uint8 *mergedData = reinterpret_cast<uint8*>(OGRE_MALLOC_SIMD( totalBytes,
                                                                               MEMCATEGORY_GEOMETRY ));
                size_t dstOffset = 0;

                //Merge the binary data as a contiguous array
                itor = mDelayedBuffers[i].begin();
                while( itor != end )
                {
                    D3D11BufferInterface *bufferInterface = static_cast<D3D11BufferInterface*>(
                                (*itor)->getBufferInterface() );

                    const size_t dstOffsetBeforeAlignment = dstOffset;
                    dstOffset = alignToNextMultiple( dstOffset, (*itor)->getBytesPerElement() );

                    if( dstOffsetBeforeAlignment != dstOffset )
                    {
                        //This is a stride changer, record as such.
                        StrideChangerVec::iterator itStride = std::lower_bound(
                                    newVbo.strideChangers.begin(),
                                    newVbo.strideChangers.end(),
                                    dstOffset, StrideChanger() );
                        const size_t padding = dstOffset - dstOffsetBeforeAlignment;
                        newVbo.strideChangers.insert( itStride, StrideChanger( dstOffset, padding ) );
                    }

                    memcpy( mergedData + dstOffset, bufferInterface->_getInitialData(),
                            (*itor)->getTotalSizeBytes() );

                    bufferInterface->_deleteInitialData();

                    dstOffset += (*itor)->getTotalSizeBytes();

                    if( totalBytes + (*itor)->getTotalSizeBytes() > mDefaultPoolSize[i][BT_IMMUTABLE] )
                        end = itor + 1;

                    ++itor;
                }

                //Create the actual D3D11 object loaded with the merged data
                createImmutableBuffer( static_cast<InternalBufferType>(i), totalBytes,
                                       mergedData, newVbo );

                const size_t vboIdx = mVbos[i][BT_IMMUTABLE].size() - 1;
                ID3D11Buffer *vboName = newVbo.vboName;
                dstOffset = 0;

                //Each buffer needs to be told about its new D3D11 object
                //(and pool index & where it starts).
                itor = mDelayedBuffers[i].begin();
                while( itor != end )
                {
                    D3D11BufferInterface *bufferInterface = static_cast<D3D11BufferInterface*>(
                                (*itor)->getBufferInterface() );
                    dstOffset = alignToNextMultiple( dstOffset, (*itor)->getBytesPerElement() );

                    bufferInterface->_setVboName( vboIdx, vboName, dstOffset );

                    dstOffset += (*itor)->getTotalSizeBytes();
                    ++itor;
                }

                OGRE_FREE_SIMD( mergedData, MEMCATEGORY_GEOMETRY );
                mergedData = 0;
            }
        }

        reorganizeImmutableVaos();

        for( size_t i=0; i<NumInternalBufferTypes; ++i )
            mDelayedBuffers[i].clear();
    }
    //-----------------------------------------------------------------------------------
    void D3D11VaoManager::reorganizeImmutableVaos(void)
    {
        VertexArrayObjectVec::const_iterator itor = mVertexArrayObjects.begin();
        VertexArrayObjectVec::const_iterator end  = mVertexArrayObjects.end();

        while( itor != end )
        {
            VertexArrayObject *vertexArrayObject = *itor;

            bool needsUpdate = false;

            const VertexBufferPackedVec &vertexBuffers = vertexArrayObject->getVertexBuffers();
            VertexBufferPackedVec::const_iterator itVertexBuf = vertexBuffers.begin();
            VertexBufferPackedVec::const_iterator enVertexBuf = vertexBuffers.end();

            while( itVertexBuf != enVertexBuf && !needsUpdate )
            {
                if( (*itVertexBuf)->getBufferType() == BT_IMMUTABLE )
                {
                    BufferPackedVec::const_iterator it = std::find(
                                mDelayedBuffers[VERTEX_BUFFER].begin(),
                                mDelayedBuffers[VERTEX_BUFFER].end(),
                                *itVertexBuf );

                    if( it != mDelayedBuffers[VERTEX_BUFFER].end() )
                        needsUpdate = true;
                }

                ++itVertexBuf;
            }

            if( vertexArrayObject->getIndexBuffer() &&
                vertexArrayObject->getIndexBuffer()->getBufferType() == BT_IMMUTABLE )
            {
                BufferPackedVec::const_iterator it = std::find( mDelayedBuffers[INDEX_BUFFER].begin(),
                                                                mDelayedBuffers[INDEX_BUFFER].end(),
                                                                vertexArrayObject->getIndexBuffer() );
                if( it != mDelayedBuffers[INDEX_BUFFER].end() )
                    needsUpdate = true;
            }

            if( needsUpdate )
            {
                if( vertexArrayObject->getVaoName() )
                    releaseVao( vertexArrayObject );

                VaoVec::iterator itor = findVao( vertexArrayObject->getVertexBuffers(),
                                                 vertexArrayObject->getIndexBuffer(),
                                                 vertexArrayObject->getOperationType() );

                D3D11VertexArrayObject *d3dVao = static_cast<D3D11VertexArrayObject*>(
                            vertexArrayObject );

                uint32 uniqueVaoId = extractUniqueVaoIdFromRenderQueueId( d3dVao->getRenderQueueId() );

                const uint32 renderQueueId = generateRenderQueueId( itor->vaoName, uniqueVaoId );

                *d3dVao = D3D11VertexArrayObject( itor->vaoName,
                                                  renderQueueId,
                                                  d3dVao->getVertexBuffers(),
                                                  d3dVao->getIndexBuffer(),
                                                  d3dVao->getOperationType(),
                                                  itor->sharedData );
            }

            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    MultiSourceVertexBufferPool* D3D11VaoManager::createMultiSourceVertexBufferPoolImpl(
                                                const VertexElement2VecVec &vertexElementsBySource,
                                                size_t maxNumVertices, size_t totalBytesPerVertex,
                                                BufferType bufferType )
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "Not implemented. Come back later.",
                     "D3D11VaoManager::createMultiSourceVertexBufferPoolImpl" );
        /*size_t vboIdx;
        size_t bufferOffset;

        allocateVbo( maxNumVertices * totalBytesPerVertex, totalBytesPerVertex,
                     bufferType, VERTEX_BUFFER, vboIdx, bufferOffset );

        BufferType vboFlag = bufferType;
        if( vboFlag >= BT_DYNAMIC_DEFAULT )
            vboFlag = BT_DYNAMIC_DEFAULT;
        const Vbo &vbo = mVbos[VERTEX_BUFFER][vboFlag][vboIdx];

        return OGRE_NEW D3D11MultiSourceVertexBufferPool( vboIdx, vbo.vboName, vertexElementsBySource,
                                                          maxNumVertices, bufferType,
                                                          bufferOffset, this );*/
    }
    //-----------------------------------------------------------------------------------
    IndexBufferPacked* D3D11VaoManager::createIndexBufferImpl( size_t numElements,
                                                                 uint32 bytesPerElement,
                                                                 BufferType bufferType,
                                                                 void *initialData, bool keepAsShadow )
    {
        size_t vboIdx;
        size_t bufferOffset;
        ID3D11Buffer *vboName;
        D3D11DynamicBuffer *dynamicBuffer;

        if( bufferType == BT_IMMUTABLE )
        {
            vboIdx          = 0;
            bufferOffset    = 0;
            vboName         = 0;
            dynamicBuffer   = 0;
        }
        else
        {
            allocateVbo( numElements * bytesPerElement, bytesPerElement, bufferType, INDEX_BUFFER,
                         vboIdx, bufferOffset );

            BufferType vboFlag = bufferType;
            if( vboFlag >= BT_DYNAMIC_DEFAULT )
                vboFlag = BT_DYNAMIC_DEFAULT;
            Vbo &vbo = mVbos[INDEX_BUFFER][vboFlag][vboIdx];
            vboName         = vbo.vboName;
            dynamicBuffer   = vbo.dynamicBuffer;
        }

        D3D11BufferInterface *bufferInterface = new D3D11BufferInterface( vboIdx, vboName,
                                                                          dynamicBuffer );
        IndexBufferPacked *retVal = OGRE_NEW IndexBufferPacked(
                                                        bufferOffset, numElements, bytesPerElement,
                                                        bufferType, initialData, keepAsShadow,
                                                        this, bufferInterface );

        if( initialData )
            bufferInterface->_firstUpload( initialData );

        if( bufferType == BT_IMMUTABLE )
            mDelayedBuffers[INDEX_BUFFER].push_back( retVal );

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void D3D11VaoManager::destroyIndexBufferImpl( IndexBufferPacked *indexBuffer )
    {
        D3D11BufferInterface *bufferInterface = static_cast<D3D11BufferInterface*>(
                                                        indexBuffer->getBufferInterface() );

        if( bufferInterface->_getInitialData() != 0 )
        {
            //Immutable buffer that never made to the GPU.
            removeBufferFromDelayedQueue( mDelayedBuffers[INDEX_BUFFER], indexBuffer );
        }
        else
        {
            deallocateVbo( bufferInterface->getVboPoolIndex(),
                           indexBuffer->_getInternalBufferStart() * indexBuffer->getBytesPerElement(),
                           indexBuffer->getNumElements() * indexBuffer->getBytesPerElement(),
                           indexBuffer->getBufferType(), INDEX_BUFFER );
        }
    }
    //-----------------------------------------------------------------------------------
    D3D11CompatBufferInterface* D3D11VaoManager::createShaderBufferInterface( bool constantBuffer,
                                                                              size_t sizeBytes,
                                                                              BufferType bufferType,
                                                                              void *initialData )
    {
        ID3D11DeviceN *d3dDevice = mDevice.get();

        D3D11_BUFFER_DESC desc;
        ZeroMemory( &desc, sizeof(D3D11_BUFFER_DESC) );
        desc.BindFlags      = constantBuffer ? D3D11_BIND_CONSTANT_BUFFER : D3D11_BIND_SHADER_RESOURCE;
        desc.ByteWidth      = sizeBytes;
        desc.CPUAccessFlags = 0;
        if( bufferType == BT_IMMUTABLE )
            desc.Usage = D3D11_USAGE_IMMUTABLE;
        else if( bufferType == BT_DEFAULT )
            desc.Usage = D3D11_USAGE_DEFAULT;
        else if( bufferType >= BT_DYNAMIC_DEFAULT )
        {
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        }

        D3D11_SUBRESOURCE_DATA subResData;
        ZeroMemory( &subResData, sizeof(D3D11_SUBRESOURCE_DATA) );
        subResData.pSysMem = initialData;
        ID3D11Buffer *vboName = 0;

        HRESULT hr = d3dDevice->CreateBuffer( &desc, initialData ? &subResData : 0, &vboName );

        if( FAILED( hr ) )
        {
            String errorDescription = mDevice.getErrorDescription(hr);
            OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                         "Failed to create buffer. ID3D11Device::CreateBuffer.\n"
                         "Error code: " + StringConverter::toString( hr ) + ".\n" +
                         errorDescription +
                         "Requested: " + StringConverter::toString( sizeBytes ) + " bytes.",
                         "D3D11VaoManager::createShaderBufferInterface" );
        }

        return new D3D11CompatBufferInterface( 0, vboName, mDevice );
    }
    //-----------------------------------------------------------------------------------
    ConstBufferPacked* D3D11VaoManager::createConstBufferImpl( size_t sizeBytes, BufferType bufferType,
                                                               void *initialData, bool keepAsShadow )
    {
        //Const buffers don't get batched together since D3D11 doesn't allow binding just a
        //region and has a 64kb limit. Only D3D11.1 on Windows 8.1 supports this feature.
        D3D11CompatBufferInterface *bufferInterface = createShaderBufferInterface( true,
                                                                                   sizeBytes,
                                                                                   bufferType,
                                                                                   initialData );
        ConstBufferPacked *retVal = OGRE_NEW D3D11ConstBufferPacked(
                                                        sizeBytes, 1,
                                                        bufferType,
                                                        initialData, keepAsShadow,
                                                        this, bufferInterface,
                                                        mDevice );

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void D3D11VaoManager::destroyConstBufferImpl( ConstBufferPacked *constBuffer )
    {
        D3D11CompatBufferInterface *bufferInterface = static_cast<D3D11CompatBufferInterface*>(
                                                                constBuffer->getBufferInterface() );

        bufferInterface->getVboName()->Release();
    }
    //-----------------------------------------------------------------------------------
    TexBufferPacked* D3D11VaoManager::createTexBufferImpl( PixelFormat pixelFormat, size_t sizeBytes,
                                                           BufferType bufferType,
                                                           void *initialData, bool keepAsShadow )
    {
        BufferInterface *bufferInterface = 0;
        size_t bufferOffset = 0;

        uint32 alignment = mTexBufferAlignment;

        if( bufferType >= BT_DYNAMIC_DEFAULT )
        {
            //For dynamic buffers, the size will be 3x times larger
            //(depending on mDynamicBufferMultiplier); we need the
            //offset after each map to be aligned; and for that, we
            //sizeBytes to be multiple of alignment.
            sizeBytes = ( (sizeBytes + alignment - 1) / alignment ) * alignment;
        }

        if( mD3D11RenderSystem->_getFeatureLevel() > D3D_FEATURE_LEVEL_11_0 )
        {
            //D3D11.1 supports NO_OVERWRITE on shader buffers, use the common pool
            size_t vboIdx;

            BufferType vboFlag = bufferType;
            if( vboFlag >= BT_DYNAMIC_DEFAULT )
                vboFlag = BT_DYNAMIC_DEFAULT;

            allocateVbo( sizeBytes, alignment, bufferType, SHADER_BUFFER, vboIdx, bufferOffset );

            Vbo &vbo = mVbos[SHADER_BUFFER][vboFlag][vboIdx];
            bufferInterface = new D3D11BufferInterface( vboIdx, vbo.vboName, vbo.dynamicBuffer );
        }
        else
        {
            //D3D11.0 and below doesn't support NO_OVERWRITE on shader buffers. Use the basic interface.
            bufferInterface = createShaderBufferInterface( false, sizeBytes, bufferType, initialData );
        }

        const size_t numElements        = sizeBytes;
        const size_t bytesPerElement    = 1;

        D3D11TexBufferPacked *retVal = OGRE_NEW D3D11TexBufferPacked(
                                                        bufferOffset, numElements, bytesPerElement,
                                                        bufferType, initialData, keepAsShadow,
                                                        this, bufferInterface, pixelFormat, mDevice );

        if( mD3D11RenderSystem->_getFeatureLevel() > D3D_FEATURE_LEVEL_11_0 )
        {
            if( initialData )
                static_cast<D3D11BufferInterface*>( bufferInterface )->_firstUpload( initialData );

            if( bufferType == BT_IMMUTABLE )
                mDelayedBuffers[SHADER_BUFFER].push_back( retVal );
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void D3D11VaoManager::destroyTexBufferImpl( TexBufferPacked *texBuffer )
    {
        if( mD3D11RenderSystem->_getFeatureLevel() > D3D_FEATURE_LEVEL_11_0 )
        {
            D3D11BufferInterface *bufferInterface = static_cast<D3D11BufferInterface*>(
                        texBuffer->getBufferInterface() );

            if( bufferInterface->_getInitialData() != 0 )
            {
                //Immutable buffer that never made to the GPU.
                removeBufferFromDelayedQueue( mDelayedBuffers[SHADER_BUFFER], texBuffer );
            }
            else
            {
                deallocateVbo( bufferInterface->getVboPoolIndex(),
                               texBuffer->_getInternalBufferStart() * texBuffer->getBytesPerElement(),
                               texBuffer->getNumElements() * texBuffer->getBytesPerElement(),
                               texBuffer->getBufferType(), SHADER_BUFFER );
            }
        }
        else
        {
            D3D11CompatBufferInterface *bufferInterface = static_cast<D3D11CompatBufferInterface*>(
                                                                    texBuffer->getBufferInterface() );

            bufferInterface->getVboName()->Release();
        }
    }
    //-----------------------------------------------------------------------------------
    IndirectBufferPacked* D3D11VaoManager::createIndirectBufferImpl( size_t sizeBytes,
                                                                       BufferType bufferType,
                                                                       void *initialData,
                                                                       bool keepAsShadow )
    {
        const size_t alignment = 4;
        size_t bufferOffset = 0;

        if( bufferType >= BT_DYNAMIC_DEFAULT )
        {
            //For dynamic buffers, the size will be 3x times larger
            //(depending on mDynamicBufferMultiplier); we need the
            //offset after each map to be aligned; and for that, we
            //sizeBytes to be multiple of alignment.
            sizeBytes = ( (sizeBytes + alignment - 1) / alignment ) * alignment;
        }

        D3D11BufferInterface *bufferInterface = 0;
        if( mSupportsIndirectBuffers )
        {
            assert( bufferType != BT_IMMUTABLE && "Immutable indirect buffers not implemented yet" );

            size_t vboIdx;
            BufferType vboFlag = bufferType;
            if( vboFlag >= BT_DYNAMIC_DEFAULT )
                vboFlag = BT_DYNAMIC_DEFAULT;

            allocateVbo( sizeBytes, alignment, bufferType, SHADER_BUFFER, vboIdx, bufferOffset );

            Vbo &vbo = mVbos[SHADER_BUFFER][vboFlag][vboIdx];
            bufferInterface = new D3D11BufferInterface( vboIdx, vbo.vboName, vbo.dynamicBuffer );
        }

        IndirectBufferPacked *retVal = OGRE_NEW IndirectBufferPacked(
                                                        bufferOffset, sizeBytes, 1,
                                                        bufferType, initialData, keepAsShadow,
                                                        this, bufferInterface );

        if( initialData )
        {
            if( mSupportsIndirectBuffers )
            {
                bufferInterface->_firstUpload( initialData );
            }
            else
            {
                memcpy( retVal->getSwBufferPtr(), initialData, sizeBytes );
            }
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void D3D11VaoManager::destroyIndirectBufferImpl( IndirectBufferPacked *indirectBuffer )
    {
        if( mSupportsIndirectBuffers )
        {
            D3D11BufferInterface *bufferInterface = static_cast<D3D11BufferInterface*>(
                        indirectBuffer->getBufferInterface() );

            if( bufferInterface->_getInitialData() != 0 )
            {
                //Immutable buffer that never made to the GPU.
                removeBufferFromDelayedQueue( mDelayedBuffers[SHADER_BUFFER], indirectBuffer );
            }
            else
            {
                deallocateVbo( bufferInterface->getVboPoolIndex(),
                               indirectBuffer->_getInternalBufferStart() *
                                    indirectBuffer->getBytesPerElement(),
                               indirectBuffer->getNumElements() * indirectBuffer->getBytesPerElement(),
                               indirectBuffer->getBufferType(), SHADER_BUFFER );
            }
        }
    }
    //-----------------------------------------------------------------------------------
    D3D11VaoManager::VaoVec::iterator D3D11VaoManager::findVao(
                                                        const VertexBufferPackedVec &vertexBuffers,
                                                        IndexBufferPacked *indexBuffer,
                                                        v1::RenderOperation::OperationType opType )
    {
        Vao vao;

        vao.operationType = opType;
        vao.vertexBuffers.reserve( vertexBuffers.size() );

        {
            VertexBufferPackedVec::const_iterator itor = vertexBuffers.begin();
            VertexBufferPackedVec::const_iterator end  = vertexBuffers.end();

            while( itor != end )
            {
                Vao::VertexBinding vertexBinding;
                vertexBinding.vertexBufferVbo   = static_cast<D3D11BufferInterface*>(
                                                        (*itor)->getBufferInterface() )->getVboName();
                vertexBinding.vertexElements    = (*itor)->getVertexElements();
                vertexBinding.stride            = calculateVertexSize( vertexBinding.vertexElements );
                vertexBinding.offset            = 0;
                vertexBinding.instancingDivisor = 0;

                /*const MultiSourceVertexBufferPool *multiSourcePool = (*itor)->getMultiSourcePool();
                if( multiSourcePool )
                {
                    vertexBinding.offset = multiSourcePool->getBytesOffsetToSource(
                                                            (*itor)->_getSourceIndex() );
                }*/

                vao.vertexBuffers.push_back( vertexBinding );

                ++itor;
            }
        }

        vao.refCount = 0;

        if( indexBuffer )
        {
            vao.indexBufferVbo  = static_cast<D3D11BufferInterface*>(
                                    indexBuffer->getBufferInterface() )->getVboName();
            vao.indexType       = indexBuffer->getIndexType();
        }
        else
        {
            vao.indexBufferVbo  = 0;
            vao.indexType       = IndexBufferPacked::IT_16BIT;
        }

        bool bFound = false;
        VaoVec::iterator itor = mVaos.begin();
        VaoVec::iterator end  = mVaos.end();

        while( itor != end && !bFound )
        {
            if( itor->operationType == vao.operationType &&
                itor->indexBufferVbo == vao.indexBufferVbo &&
                itor->indexType == vao.indexType &&
                itor->vertexBuffers == vao.vertexBuffers )
            {
                bFound = true;
            }
            else
            {
                ++itor;
            }
        }

        if( !bFound )
        {
            vao.vaoName = createVao( vao );

            //Bake all the D3D11 data that is shared by all VAOs.
            vao.sharedData = new D3D11VertexArrayObjectShared( vertexBuffers, indexBuffer,
                                                               opType, mDrawId );

            mVaos.push_back( vao );
            itor = mVaos.begin() + mVaos.size() - 1;
        }

        return itor;
    }
    //-----------------------------------------------------------------------------------
    uint32 D3D11VaoManager::createVao( const Vao &vaoRef )
    {
        return mVaoNames++;
    }
    //-----------------------------------------------------------------------------------
    void D3D11VaoManager::releaseVao( VertexArrayObject *vao )
    {
        D3D11VertexArrayObject *glVao = static_cast<D3D11VertexArrayObject*>( vao );

        VaoVec::iterator itor = mVaos.begin();
        VaoVec::iterator end  = mVaos.end();

        while( itor != end && itor->vaoName != glVao->getVaoName() )
            ++itor;

        if( itor != end )
        {
            --itor->refCount;

            if( !itor->refCount )
            {
                //TODO: Remove cached ID3D11InputLayout from all D3D11HLSLProgram
                //(the one generated in D3D11HLSLProgram::getLayoutForVao)
                delete glVao->mSharedData;
                glVao->mSharedData = 0;
                efficientVectorRemove( mVaos, itor );
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void D3D11VaoManager::bindDrawId()
    {
        D3D11BufferInterface *bufferInterface = static_cast<D3D11BufferInterface*>(
                                                    mDrawId->getBufferInterface() );

        ID3D11Buffer *vertexBuffer = bufferInterface->getVboName();
        UINT stride = mDrawId->getBytesPerElement();
        UINT offset = 0;

        mDevice.GetImmediateContext()->IASetVertexBuffers(
                    15,
                    1,
                    &vertexBuffer,
                    &stride,
                    &offset );
    }
    //-----------------------------------------------------------------------------------
    uint32 D3D11VaoManager::generateRenderQueueId( uint32 vaoName, uint32 uniqueVaoId )
    {
        //Mix mNumGeneratedVaos with the D3D11 Vao for better sorting purposes:
        //  If we only use the D3D11's vao, the RQ will sort Meshes with
        //  multiple submeshes mixed with other meshes.
        //  For cache locality, and assuming all of them have the same GL vao,
        //  we prefer the RQ to sort:
        //      1. Mesh A - SubMesh 0
        //      2. Mesh A - SubMesh 1
        //      3. Mesh B - SubMesh 0
        //      4. Mesh B - SubMesh 1
        //      5. Mesh D - SubMesh 0
        //  If we don't mix mNumGeneratedVaos in it; the following could be possible:
        //      1. Mesh B - SubMesh 1
        //      2. Mesh D - SubMesh 0
        //      3. Mesh A - SubMesh 1
        //      4. Mesh B - SubMesh 0
        //      5. Mesh A - SubMesh 0
        //  Thus thrashing the cache unnecessarily.
        const int bitsVaoGl  = 5;
        const uint32 maskVaoGl  = OGRE_RQ_MAKE_MASK( bitsVaoGl );
        const uint32 maskVao    = OGRE_RQ_MAKE_MASK( RqBits::MeshBits - bitsVaoGl );

        const uint32 shiftVaoGl     = RqBits::MeshBits - bitsVaoGl;

        uint32 renderQueueId =
                ( (vaoName & maskVaoGl) << shiftVaoGl ) |
                (uniqueVaoId & maskVao);

        return renderQueueId;
    }
    //-----------------------------------------------------------------------------------
    uint32 D3D11VaoManager::extractUniqueVaoIdFromRenderQueueId( uint32 rqId )
    {
        const int bitsVaoGl  = 5;
        const uint32 maskVao = OGRE_RQ_MAKE_MASK( RqBits::MeshBits - bitsVaoGl );
        return rqId & maskVao;
    }
    //-----------------------------------------------------------------------------------
    VertexArrayObject* D3D11VaoManager::createVertexArrayObjectImpl(
                                                            const VertexBufferPackedVec &vertexBuffers,
                                                            IndexBufferPacked *indexBuffer,
                                                            v1::RenderOperation::OperationType opType )
    {
        {
            bool hasImmutableDelayedBuffer = false;
            VertexBufferPackedVec::const_iterator itor = vertexBuffers.begin();
            VertexBufferPackedVec::const_iterator end  = vertexBuffers.end();

            while( itor != end && !hasImmutableDelayedBuffer )
            {
                if( (*itor)->getBufferType() == BT_IMMUTABLE )
                    hasImmutableDelayedBuffer = true;
                ++itor;
            }

            if( indexBuffer && indexBuffer->getBufferType() == BT_IMMUTABLE )
                hasImmutableDelayedBuffer = true;

            if( hasImmutableDelayedBuffer )
            {
                const uint32 renderQueueId = generateRenderQueueId( 0, mNumGeneratedVaos );
                //If the Vao contains an immutable buffer that is delayed, we can't
                //create the actual Vao yet. We'll modify the pointer later.
                D3D11VertexArrayObject *retVal = OGRE_NEW D3D11VertexArrayObject( 0,
                                                                                  renderQueueId,
                                                                                  vertexBuffers,
                                                                                  indexBuffer,
                                                                                  opType,
                                                                                  0 );
                return retVal;
            }
        }

        VaoVec::iterator itor = findVao( vertexBuffers, indexBuffer, opType );

        const uint32 renderQueueId = generateRenderQueueId( itor->vaoName, mNumGeneratedVaos );

        D3D11VertexArrayObject *retVal = OGRE_NEW D3D11VertexArrayObject( itor->vaoName,
                                                                          renderQueueId,
                                                                          vertexBuffers,
                                                                          indexBuffer,
                                                                          opType,
                                                                          itor->sharedData );

        ++itor->refCount;

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void D3D11VaoManager::destroyVertexArrayObjectImpl( VertexArrayObject *vao )
    {
        releaseVao( vao );

        D3D11VertexArrayObject *glVao = static_cast<D3D11VertexArrayObject*>( vao );
        //We delete it here because this class has no virtual destructor on purpose
        OGRE_DELETE glVao;
    }
    //-----------------------------------------------------------------------------------
    StagingBuffer* D3D11VaoManager::createStagingBuffer( size_t sizeBytes, bool forUpload )
    {
        sizeBytes = std::max<size_t>( sizeBytes, 4 * 1024 * 1024 );

        ID3D11DeviceN *d3dDevice = mDevice.get();

        D3D11_BUFFER_DESC desc;
        ZeroMemory( &desc, sizeof(D3D11_BUFFER_DESC) );
        desc.ByteWidth  = sizeBytes;

        if( forUpload )
        {
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            desc.Usage          = D3D11_USAGE_DYNAMIC;
            desc.BindFlags      = D3D11_BIND_VERTEX_BUFFER|D3D11_BIND_INDEX_BUFFER;
        }
        else
        {
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
            desc.Usage          = D3D11_USAGE_STAGING;
        }

        ID3D11Buffer *bufferName = 0;
        HRESULT hr = d3dDevice->CreateBuffer( &desc, 0, &bufferName );

        if( FAILED( hr ) )
        {
            String errorDescription = mDevice.getErrorDescription(hr);
            OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                         "Failed to create buffer. ID3D11Device::CreateBuffer.\n"
                         "Error code: " + StringConverter::toString( hr ) + ".\n" +
                         errorDescription +
                         "Requested: " + StringConverter::toString( sizeBytes ) +
                         String(" bytes. For ") + (forUpload ? "Uploading" : "Downloading"),
                         "D3D11VaoManager::createStagingBuffer" );
        }

        D3D11StagingBuffer *stagingBuffer = OGRE_NEW D3D11StagingBuffer( sizeBytes, this,
                                                                         forUpload, bufferName,
                                                                         mDevice );
        mStagingBuffers[forUpload].push_back( stagingBuffer );

        return stagingBuffer;
    }
    //-----------------------------------------------------------------------------------
    AsyncTicketPtr D3D11VaoManager::createAsyncTicket( BufferPacked *creator,
                                                       StagingBuffer *stagingBuffer,
                                                       size_t elementStart, size_t elementCount )
    {
        return AsyncTicketPtr( OGRE_NEW D3D11AsyncTicket( creator, stagingBuffer,
                                                          elementStart, elementCount,
                                                          mDevice ) );
    }
    //-----------------------------------------------------------------------------------
    void D3D11VaoManager::_beginFrame(void)
    {
        createDelayedImmutableBuffers();

        //TODO: If we have many tiny immutable buffers, get the data back to CPU,
        //destroy the buffers and create a unified immutable buffer.
    }
    //-----------------------------------------------------------------------------------
    void D3D11VaoManager::_update(void)
    {
        VaoManager::_update();

        unsigned long currentTimeMs = mTimer->getMilliseconds();

        if( currentTimeMs >= mNextStagingBufferTimestampCheckpoint )
        {
            mNextStagingBufferTimestampCheckpoint = (unsigned long)(~0);

            for( size_t i=0; i<2; ++i )
            {
                StagingBufferVec::iterator itor = mZeroRefStagingBuffers[i].begin();
                StagingBufferVec::iterator end  = mZeroRefStagingBuffers[i].end();

                while( itor != end )
                {
                    StagingBuffer *stagingBuffer = *itor;

                    mNextStagingBufferTimestampCheckpoint = std::min(
                                                    mNextStagingBufferTimestampCheckpoint,
                                                    stagingBuffer->getLastUsedTimestamp() +
                                                    currentTimeMs );

                    if( stagingBuffer->getLastUsedTimestamp() - currentTimeMs >
                        stagingBuffer->getLifetimeThreshold() )
                    {
                        //Time to delete this buffer.
                        static_cast<D3D11StagingBuffer*>(stagingBuffer)->getBufferName()->Release();

                        //We have to remove it from two lists.
                        StagingBufferVec::iterator itFullList = std::find( mStagingBuffers[i].begin(),
                                                                           mStagingBuffers[i].end(),
                                                                           stagingBuffer );
                        efficientVectorRemove( mStagingBuffers[i], itFullList );
                        delete *itor;

                        itor = efficientVectorRemove( mZeroRefStagingBuffers[i], itor );
                        end  = mZeroRefStagingBuffers[i].end();
                    }
                    else
                    {
                        ++itor;
                    }
                }
            }
        }

        if( !mDelayedDestroyBuffers.empty() &&
            mDelayedDestroyBuffers.front().frameNumDynamic == mDynamicBufferCurrentFrame )
        {
            waitForTailFrameToFinish();
            destroyDelayedBuffers( mDynamicBufferCurrentFrame );
        }

        if( mFrameSyncVec[mDynamicBufferCurrentFrame] )
        {
            mFrameSyncVec[mDynamicBufferCurrentFrame]->Release();
            mFrameSyncVec[mDynamicBufferCurrentFrame] = 0;
        }

        mFrameSyncVec[mDynamicBufferCurrentFrame] = createFence();
        mDynamicBufferCurrentFrame = (mDynamicBufferCurrentFrame + 1) % mDynamicBufferMultiplier;
    }
    //-----------------------------------------------------------------------------------
    ID3D11Query* D3D11VaoManager::createFence( D3D11Device &device )
    {
        ID3D11Query *retVal = 0;

        D3D11_QUERY_DESC queryDesc;
        queryDesc.Query     = D3D11_QUERY_EVENT;
        queryDesc.MiscFlags = 0;
        HRESULT hr = device.get()->CreateQuery( &queryDesc, &retVal );

        if( FAILED( hr ) )
        {
            String errorDescription = device.getErrorDescription(hr);
            OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                         "Failed to create frame fence.\n"
                         "Error code: " + StringConverter::toString( hr ) + ".\n" +
                         errorDescription,
                         "D3D11VaoManager::_update" );
        }

        // Insert the fence into D3D's commands
        device.GetImmediateContext()->End( retVal );

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    ID3D11Query* D3D11VaoManager::createFence(void)
    {
        return D3D11VaoManager::createFence( mDevice );
    }
    //-----------------------------------------------------------------------------------
    uint8 D3D11VaoManager::waitForTailFrameToFinish(void)
    {
        if( mFrameSyncVec[mDynamicBufferCurrentFrame] )
        {
            waitFor( mFrameSyncVec[mDynamicBufferCurrentFrame] );
            mFrameSyncVec[mDynamicBufferCurrentFrame]->Release();
            mFrameSyncVec[mDynamicBufferCurrentFrame] = 0;
        }

        return mDynamicBufferCurrentFrame;
    }
    //-----------------------------------------------------------------------------------
    ID3D11Query* D3D11VaoManager::waitFor( ID3D11Query *fenceName, ID3D11DeviceContextN *deviceContext )
    {
        HRESULT hr = S_FALSE;

        while( hr == S_FALSE )
        {
            hr = deviceContext->GetData( fenceName, NULL, 0, 0 );

            if( FAILED( hr ) )
            {
                OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                             "Failure while waiting for a D3D11 Fence. Could be out of GPU memory. "
                             "Update your video card drivers. If that doesn't help, "
                             "contact the developers.",
                             "D3D11VaoManager::waitFor" );

                return fenceName;
            }

            //Give HyperThreading threads a breath on this spinlock.
            YieldProcessor();
        } // spin until event is finished

        return fenceName;
    }
    //-----------------------------------------------------------------------------------
    ID3D11Query* D3D11VaoManager::waitFor( ID3D11Query *fenceName )
    {
        return waitFor( fenceName, mDevice.GetImmediateContext() );
    }
}
