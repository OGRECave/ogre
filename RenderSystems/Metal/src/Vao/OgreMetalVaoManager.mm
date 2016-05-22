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

#include "Vao/OgreMetalVaoManager.h"
#include "Vao/OgreMetalStagingBuffer.h"
#include "Vao/OgreMetalVertexArrayObject.h"
#include "Vao/OgreMetalBufferInterface.h"
#include "Vao/OgreMetalConstBufferPacked.h"
#include "Vao/OgreMetalTexBufferPacked.h"
#include "Vao/OgreMetalUavBufferPacked.h"
#include "Vao/OgreMetalMultiSourceVertexBufferPool.h"
#include "Vao/OgreMetalDynamicBuffer.h"
#include "Vao/OgreMetalAsyncTicket.h"

#include "Vao/OgreIndirectBufferPacked.h"

#include "OgreMetalDevice.h"

#include "OgreRenderQueue.h"

#include "OgreTimer.h"
#include "OgreStringConverter.h"

namespace Ogre
{
    MetalVaoManager::MetalVaoManager( uint8 dynamicBufferMultiplier, MetalDevice *device ) :
        mDevice( device ),
        mDrawId( 0 )
    {
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        //On iOS alignment must match "the maximum accessed object" type. e.g.
        //if it's all float, then alignment = 4. if it's a float2, then alignment = 8.
        //The max. object is float4, so alignment = 16
        mConstBufferAlignment   = 16;
        mTexBufferAlignment     = 16;

        //Keep pools of 32MB each for static buffers
        mDefaultPoolSize[CPU_INACCESSIBLE]  = 32;

        //Keep pools of 8MB each for dynamic buffers
        for( size_t i=CPU_ACCESSIBLE_DEFAULT; i<=CPU_ACCESSIBLE_PERSISTENT_COHERENT; ++i )
            mDefaultPoolSize[i] = 8 * 1024 * 1024;

        //TODO: iOS v3 family does support indirect buffers.
        mSupportsIndirectBuffers    = false;
#else
        //OS X restrictions.
        mConstBufferAlignment   = 256;
        mTexBufferAlignment     = 256;

        //Keep pools of 128MB each for static buffers
        mDefaultPoolSize[CPU_INACCESSIBLE]  = 128 * 1024 * 1024;

        //Keep pools of 32MB each for dynamic buffers
        for( size_t i=CPU_ACCESSIBLE_DEFAULT; i<=CPU_ACCESSIBLE_PERSISTENT_COHERENT; ++i )
            mDefaultPoolSize[i] = 32 * 1024 * 1024;

        mSupportsIndirectBuffers    = true;
#endif

        mConstBufferMaxSize = 64 * 1024;        //64kb
        mTexBufferMaxSize   = 128 * 1024 * 1024;//128MB

        mSupportsPersistentMapping  = true;

        mDynamicBufferMultiplier = dynamicBufferMultiplier;

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        uint32 *drawIdPtr = static_cast<uint32*>( OGRE_MALLOC_SIMD( 4096 * sizeof(uint32),
                                                                    MEMCATEGORY_GEOMETRY ) );
        for( uint32 i=0; i<4096; ++i )
            drawIdPtr[i] = i;
        mDrawId = createConstBuffer( 4096 * sizeof(uint32), BT_IMMUTABLE, drawIdPtr, false );
        OGRE_FREE_SIMD( drawIdPtr, MEMCATEGORY_GEOMETRY );
        drawIdPtr = 0;
#else
        VertexElement2Vec vertexElements;
        vertexElements.push_back( VertexElement2( VET_UINT1, VES_COUNT ) );
        uint32 *drawIdPtr = static_cast<uint32*>( OGRE_MALLOC_SIMD( 4096 * sizeof(uint32),
                                                                    MEMCATEGORY_GEOMETRY ) );
        for( uint32 i=0; i<4096; ++i )
            drawIdPtr[i] = i;
        mDrawId = createVertexBuffer( vertexElements, 4096, BT_IMMUTABLE, drawIdPtr, true );
#endif
    }
    //-----------------------------------------------------------------------------------
    MetalVaoManager::~MetalVaoManager()
    {
        destroyAllVertexArrayObjects();
        deleteAllBuffers();
    }
    //-----------------------------------------------------------------------------------
    void MetalVaoManager::allocateVbo( size_t sizeBytes, size_t alignment, BufferType bufferType,
                                       size_t &outVboIdx, size_t &outBufferOffset )
    {
        assert( alignment > 0 );

        VboFlag vboFlag = bufferTypeToVboFlag( bufferType );

        if( bufferType >= BT_DYNAMIC_DEFAULT )
            sizeBytes   *= mDynamicBufferMultiplier;

        VboVec::const_iterator itor = mVbos[vboFlag].begin();
        VboVec::const_iterator end  = mVbos[vboFlag].end();

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
                    bestVboIdx      = itor - mVbos[vboFlag].begin();
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
            bestVboIdx      = mVbos[vboFlag].size();
            bestBlockIdx    = 0;
            foundMatchingStride = true;

            Vbo newVbo;

            size_t poolSize = std::max( mDefaultPoolSize[vboFlag], sizeBytes );

            //No luck, allocate a new buffer.
            MTLResourceOptions resourceOptions = 0;

            if( vboFlag == CPU_INACCESSIBLE )
                resourceOptions = MTLResourceStorageModePrivate;
            else
            {
                resourceOptions = MTLResourceCPUCacheModeWriteCombined;
                if( vboFlag == CPU_ACCESSIBLE_DEFAULT )
                    resourceOptions |= 0;
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
                else if( vboFlag == CPU_ACCESSIBLE_PERSISTENT )
                    resourceOptions |= MTLResourceStorageModeShared;
#else
                else if( vboFlag == CPU_ACCESSIBLE_PERSISTENT )
                    resourceOptions |= MTLResourceStorageModeManaged;
#endif
                else if( vboFlag == CPU_ACCESSIBLE_PERSISTENT_COHERENT )
                    resourceOptions |= MTLResourceCPUCacheModeDefaultCache;
            }

            newVbo.vboName = [mDevice->mDevice newBufferWithLength:poolSize options:resourceOptions];

            if( !newVbo.vboName )
            {
                OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                             "Out of GPU memory or driver refused.\n"
                             "Requested: " + StringConverter::toString( poolSize ) + " bytes.",
                             "MetalVaoManager::allocateVbo" );
            }

            newVbo.sizeBytes = poolSize;
            newVbo.freeBlocks.push_back( Block( 0, poolSize ) );
            newVbo.dynamicBuffer = 0;

            if( vboFlag != CPU_INACCESSIBLE )
            {
                newVbo.dynamicBuffer = new MetalDynamicBuffer( newVbo.vboName, newVbo.sizeBytes  );
            }

            mVbos[vboFlag].push_back( newVbo );
        }

        Vbo &bestVbo        = mVbos[vboFlag][bestVboIdx];
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
    void MetalVaoManager::deallocateVbo( size_t vboIdx, size_t bufferOffset, size_t sizeBytes,
                                         BufferType bufferType )
    {
        VboFlag vboFlag = bufferTypeToVboFlag( bufferType );

        if( bufferType >= BT_DYNAMIC_DEFAULT )
            sizeBytes *= mDynamicBufferMultiplier;

        Vbo &vbo = mVbos[vboFlag][vboIdx];
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
    }
    //-----------------------------------------------------------------------------------
    void MetalVaoManager::mergeContiguousBlocks( BlockVec::iterator blockToMerge,
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
    VertexBufferPacked* MetalVaoManager::createVertexBufferImpl( size_t numElements,
                                                                   uint32 bytesPerElement,
                                                                   BufferType bufferType,
                                                                   void *initialData, bool keepAsShadow,
                                                                   const VertexElement2Vec &vElements )
    {
        size_t vboIdx;
        size_t bufferOffset;

        allocateVbo( numElements * bytesPerElement, bytesPerElement, bufferType, vboIdx, bufferOffset );

        VboFlag vboFlag = bufferTypeToVboFlag( bufferType );
        Vbo &vbo = mVbos[vboFlag][vboIdx];
        MetalBufferInterface *bufferInterface = new MetalBufferInterface( vboIdx, vbo.vboName,
                                                                          vbo.dynamicBuffer );

        VertexBufferPacked *retVal = OGRE_NEW VertexBufferPacked(
                                                        0, numElements, bytesPerElement,
                                                        bufferType, initialData, keepAsShadow,
                                                        this, bufferInterface, vElements, 0, 0, 0 );

        if( initialData )
            bufferInterface->_firstUpload( initialData, 0, numElements );

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void MetalVaoManager::destroyVertexBufferImpl( VertexBufferPacked *vertexBuffer )
    {
        MetalBufferInterface *bufferInterface = static_cast<MetalBufferInterface*>(
                                                        vertexBuffer->getBufferInterface() );


        deallocateVbo( bufferInterface->getVboPoolIndex(),
                       vertexBuffer->_getInternalBufferStart() * vertexBuffer->getBytesPerElement(),
                       vertexBuffer->getNumElements() * vertexBuffer->getBytesPerElement(),
                       vertexBuffer->getBufferType() );
    }
    //-----------------------------------------------------------------------------------
    MultiSourceVertexBufferPool* MetalVaoManager::createMultiSourceVertexBufferPoolImpl(
                                                const VertexElement2VecVec &vertexElementsBySource,
                                                size_t maxNumVertices, size_t totalBytesPerVertex,
                                                BufferType bufferType )
    {
        size_t vboIdx;
        size_t bufferOffset;

        allocateVbo( maxNumVertices * totalBytesPerVertex, totalBytesPerVertex,
                     bufferType, vboIdx, bufferOffset );

        VboFlag vboFlag = bufferTypeToVboFlag( bufferType );

        const Vbo &vbo = mVbos[vboFlag][vboIdx];

        return OGRE_NEW MetalMultiSourceVertexBufferPool( vboIdx, vbo.vboName, vertexElementsBySource,
                                                          maxNumVertices, bufferType,
                                                          bufferOffset, this );
    }
    //-----------------------------------------------------------------------------------
    IndexBufferPacked* MetalVaoManager::createIndexBufferImpl( size_t numElements,
                                                                 uint32 bytesPerElement,
                                                                 BufferType bufferType,
                                                                 void *initialData, bool keepAsShadow )
    {
        size_t vboIdx;
        size_t bufferOffset;

        allocateVbo( numElements * bytesPerElement, bytesPerElement, bufferType, vboIdx, bufferOffset );

        VboFlag vboFlag = bufferTypeToVboFlag( bufferType );

        Vbo &vbo = mVbos[vboFlag][vboIdx];
        MetalBufferInterface *bufferInterface = new MetalBufferInterface( vboIdx, vbo.vboName,
                                                                          vbo.dynamicBuffer );
        IndexBufferPacked *retVal = OGRE_NEW IndexBufferPacked(
                                                        bufferOffset, numElements, bytesPerElement,
                                                        bufferType, initialData, keepAsShadow,
                                                        this, bufferInterface );

        if( initialData )
            bufferInterface->_firstUpload( initialData, 0, numElements );

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void MetalVaoManager::destroyIndexBufferImpl( IndexBufferPacked *indexBuffer )
    {
        MetalBufferInterface *bufferInterface = static_cast<MetalBufferInterface*>(
                                                        indexBuffer->getBufferInterface() );


        deallocateVbo( bufferInterface->getVboPoolIndex(),
                       indexBuffer->_getInternalBufferStart() * indexBuffer->getBytesPerElement(),
                       indexBuffer->getNumElements() * indexBuffer->getBytesPerElement(),
                       indexBuffer->getBufferType() );
    }
    //-----------------------------------------------------------------------------------
    ConstBufferPacked* MetalVaoManager::createConstBufferImpl( size_t sizeBytes, BufferType bufferType,
                                                                 void *initialData, bool keepAsShadow )
    {
        size_t vboIdx;
        size_t bufferOffset;

        size_t alignment = mConstBufferAlignment;

        size_t bindableSize = sizeBytes;

        VboFlag vboFlag = bufferTypeToVboFlag( bufferType );

        if( bufferType >= BT_DYNAMIC_DEFAULT )
        {
            //For dynamic buffers, the size will be 3x times larger
            //(depending on mDynamicBufferMultiplier); we need the
            //offset after each map to be aligned; and for that, we
            //sizeBytes to be multiple of alignment.
            sizeBytes = ( (sizeBytes + alignment - 1) / alignment ) * alignment;
        }

        allocateVbo( sizeBytes, alignment, bufferType, vboIdx, bufferOffset );

        Vbo &vbo = mVbos[vboFlag][vboIdx];
        MetalBufferInterface *bufferInterface = new MetalBufferInterface( vboIdx, vbo.vboName,
                                                                              vbo.dynamicBuffer );
        ConstBufferPacked *retVal = OGRE_NEW MetalConstBufferPacked(
                                                        bufferOffset, sizeBytes, 1,
                                                        bufferType, initialData, keepAsShadow,
                                                        this, bufferInterface, bindableSize );

        if( initialData )
            bufferInterface->_firstUpload( initialData, 0, sizeBytes );

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void MetalVaoManager::destroyConstBufferImpl( ConstBufferPacked *constBuffer )
    {
        MetalBufferInterface *bufferInterface = static_cast<MetalBufferInterface*>(
                                                        constBuffer->getBufferInterface() );


        deallocateVbo( bufferInterface->getVboPoolIndex(),
                       constBuffer->_getInternalBufferStart() * constBuffer->getBytesPerElement(),
                       constBuffer->getNumElements() * constBuffer->getBytesPerElement(),
                       constBuffer->getBufferType() );
    }
    //-----------------------------------------------------------------------------------
    TexBufferPacked* MetalVaoManager::createTexBufferImpl( PixelFormat pixelFormat, size_t sizeBytes,
                                                           BufferType bufferType,
                                                           void *initialData, bool keepAsShadow )
    {
        size_t vboIdx;
        size_t bufferOffset;

        size_t alignment = mTexBufferAlignment;

        VboFlag vboFlag = bufferTypeToVboFlag( bufferType );

        if( bufferType >= BT_DYNAMIC_DEFAULT )
        {
            //For dynamic buffers, the size will be 3x times larger
            //(depending on mDynamicBufferMultiplier); we need the
            //offset after each map to be aligned; and for that, we
            //sizeBytes to be multiple of alignment.
            sizeBytes = ( (sizeBytes + alignment - 1) / alignment ) * alignment;
        }

        allocateVbo( sizeBytes, alignment, bufferType, vboIdx, bufferOffset );

        Vbo &vbo = mVbos[vboFlag][vboIdx];
        MetalBufferInterface *bufferInterface = new MetalBufferInterface( vboIdx, vbo.vboName,
                                                                          vbo.dynamicBuffer );
        TexBufferPacked *retVal = OGRE_NEW MetalTexBufferPacked(
                                                        bufferOffset, sizeBytes, 1,
                                                        bufferType, initialData, keepAsShadow,
                                                        this, bufferInterface, pixelFormat );

        if( initialData )
            bufferInterface->_firstUpload( initialData, 0, sizeBytes );

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void MetalVaoManager::destroyTexBufferImpl( TexBufferPacked *texBuffer )
    {
        MetalBufferInterface *bufferInterface = static_cast<MetalBufferInterface*>(
                                                        texBuffer->getBufferInterface() );


        deallocateVbo( bufferInterface->getVboPoolIndex(),
                       texBuffer->_getInternalBufferStart() * texBuffer->getBytesPerElement(),
                       texBuffer->getNumElements() * texBuffer->getBytesPerElement(),
                       texBuffer->getBufferType() );
    }
    //-----------------------------------------------------------------------------------
    UavBufferPacked* MetalVaoManager::createUavBufferImpl( size_t numElements, uint32 bytesPerElement,
                                                           uint32 bindFlags,
                                                           void *initialData, bool keepAsShadow )
    {
        size_t vboIdx;
        size_t bufferOffset;

        size_t alignment = mUavBufferAlignment;

        const BufferType bufferType = BT_DEFAULT;
        VboFlag vboFlag = bufferTypeToVboFlag( bufferType );

        allocateVbo( numElements * bytesPerElement, alignment, bufferType, vboIdx, bufferOffset );

        Vbo &vbo = mVbos[vboFlag][vboIdx];
        MetalBufferInterface *bufferInterface = new MetalBufferInterface( vboIdx, vbo.vboName,
                                                                          vbo.dynamicBuffer );
        UavBufferPacked *retVal = OGRE_NEW MetalUavBufferPacked(
                                                        bufferOffset, numElements, bytesPerElement,
                                                        bindFlags, initialData, keepAsShadow,
                                                        this, bufferInterface );

        if( initialData )
            bufferInterface->_firstUpload( initialData, 0, numElements );

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void MetalVaoManager::destroyUavBufferImpl( UavBufferPacked *uavBuffer )
    {
        MetalBufferInterface *bufferInterface = static_cast<MetalBufferInterface*>(
                                                        uavBuffer->getBufferInterface() );


        deallocateVbo( bufferInterface->getVboPoolIndex(),
                       uavBuffer->_getInternalBufferStart() * uavBuffer->getBytesPerElement(),
                       uavBuffer->getNumElements() * uavBuffer->getBytesPerElement(),
                       uavBuffer->getBufferType() );
    }
    //-----------------------------------------------------------------------------------
    IndirectBufferPacked* MetalVaoManager::createIndirectBufferImpl( size_t sizeBytes,
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

        MetalBufferInterface *bufferInterface = 0;
        if( mSupportsIndirectBuffers )
        {
            size_t vboIdx;
            VboFlag vboFlag = bufferTypeToVboFlag( bufferType );

            allocateVbo( sizeBytes, alignment, bufferType, vboIdx, bufferOffset );

            Vbo &vbo = mVbos[vboFlag][vboIdx];
            bufferInterface = new MetalBufferInterface( vboIdx, vbo.vboName, vbo.dynamicBuffer );
        }

        IndirectBufferPacked *retVal = OGRE_NEW IndirectBufferPacked(
                                                        bufferOffset, sizeBytes, 1,
                                                        bufferType, initialData, keepAsShadow,
                                                        this, bufferInterface );

        if( initialData )
        {
            if( mSupportsIndirectBuffers )
            {
                bufferInterface->_firstUpload( initialData, 0, sizeBytes );
            }
            else
            {
                memcpy( retVal->getSwBufferPtr(), initialData, sizeBytes );
            }
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void MetalVaoManager::destroyIndirectBufferImpl( IndirectBufferPacked *indirectBuffer )
    {
        if( mSupportsIndirectBuffers )
        {
            MetalBufferInterface *bufferInterface = static_cast<MetalBufferInterface*>(
                        indirectBuffer->getBufferInterface() );


            deallocateVbo( bufferInterface->getVboPoolIndex(),
                           indirectBuffer->_getInternalBufferStart() *
                                indirectBuffer->getBytesPerElement(),
                           indirectBuffer->getNumElements() * indirectBuffer->getBytesPerElement(),
                           indirectBuffer->getBufferType() );
        }
    }
    //-----------------------------------------------------------------------------------
    VertexArrayObject* MetalVaoManager::createVertexArrayObjectImpl(
                                                            const VertexBufferPackedVec &vertexBuffers,
                                                            IndexBufferPacked *indexBuffer,
                                                            OperationType opType )
    {
        size_t idx = mVertexArrayObjects.size();

        const int bitsOpType = 3;
        const int bitsVaoGl  = 2;
        const uint32 maskOpType = OGRE_RQ_MAKE_MASK( bitsOpType );
        const uint32 maskVaoGl  = OGRE_RQ_MAKE_MASK( bitsVaoGl );
        const uint32 maskVao    = OGRE_RQ_MAKE_MASK( RqBits::MeshBits - bitsOpType - bitsVaoGl );

        const uint32 shiftOpType    = RqBits::MeshBits - bitsOpType;
        const uint32 shiftVaoGl     = shiftOpType - bitsVaoGl;

        uint32 renderQueueId =
                ( (opType & maskOpType) << shiftOpType ) |
                ( (idx & maskVaoGl) << shiftVaoGl ) |
                (idx & maskVao);

        MetalVertexArrayObject *retVal = OGRE_NEW MetalVertexArrayObject( idx,
                                                                        renderQueueId,
                                                                        vertexBuffers,
                                                                        indexBuffer,
                                                                        opType );

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void MetalVaoManager::destroyVertexArrayObjectImpl( VertexArrayObject *vao )
    {
        MetalVertexArrayObject *glVao = static_cast<MetalVertexArrayObject*>( vao );
        OGRE_DELETE glVao;
    }
    //-----------------------------------------------------------------------------------
    StagingBuffer* MetalVaoManager::createStagingBuffer( size_t sizeBytes, bool forUpload )
    {
        sizeBytes = std::max<size_t>( sizeBytes, 4 * 1024 * 1024 );

        MTLResourceOptions resourceOptions = 0;

        if( forUpload )
            resourceOptions = MTLResourceCPUCacheModeWriteCombined|MTLResourceStorageModeShared;
        else
            resourceOptions = MTLResourceCPUCacheModeDefaultCache|MTLResourceStorageModeShared;

        id<MTLBuffer> bufferName = [mDevice->mDevice newBufferWithLength:sizeBytes
                                                                         options:resourceOptions];

        MetalStagingBuffer *stagingBuffer = OGRE_NEW MetalStagingBuffer( 0, sizeBytes, this, forUpload,
                                                                         bufferName, mDevice );
        mRefedStagingBuffers[forUpload].push_back( stagingBuffer );

        return stagingBuffer;
    }
    //-----------------------------------------------------------------------------------
    AsyncTicketPtr MetalVaoManager::createAsyncTicket( BufferPacked *creator,
                                                         StagingBuffer *stagingBuffer,
                                                         size_t elementStart, size_t elementCount )
    {
        return AsyncTicketPtr( OGRE_NEW MetalAsyncTicket( creator, stagingBuffer,
                                                            elementStart, elementCount ) );
    }
    //-----------------------------------------------------------------------------------
    void MetalVaoManager::_update(void)
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

                    /*if( stagingBuffer->getLastUsedTimestamp() - currentTimeMs >
                        stagingBuffer->getUnfencedTimeThreshold() )
                    {
                        static_cast<MetalStagingBuffer*>( stagingBuffer )->cleanUnfencedHazards();
                    }*/

                    if( stagingBuffer->getLastUsedTimestamp() - currentTimeMs >
                        stagingBuffer->getLifetimeThreshold() )
                    {
                        //Time to delete this buffer.
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

        mDynamicBufferCurrentFrame = (mDynamicBufferCurrentFrame + 1) % mDynamicBufferMultiplier;
    }
    //-----------------------------------------------------------------------------------
    uint8 MetalVaoManager::waitForTailFrameToFinish(void)
    {
        return mDynamicBufferCurrentFrame;
    }
    //-----------------------------------------------------------------------------------
    MetalVaoManager::VboFlag MetalVaoManager::bufferTypeToVboFlag( BufferType bufferType )
    {
        return static_cast<VboFlag>( std::max( 0, (bufferType - BT_DYNAMIC_DEFAULT) +
                                                    CPU_ACCESSIBLE_DEFAULT ) );
    }
}

