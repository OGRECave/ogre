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

#include "Vao/OgreNULLVaoManager.h"
#include "Vao/OgreNULLStagingBuffer.h"
#include "Vao/OgreNULLVertexArrayObject.h"
#include "Vao/OgreNULLBufferInterface.h"
#include "Vao/OgreNULLConstBufferPacked.h"
#include "Vao/OgreNULLTexBufferPacked.h"
#include "Vao/OgreNULLUavBufferPacked.h"
#include "Vao/OgreNULLMultiSourceVertexBufferPool.h"
#include "Vao/OgreNULLAsyncTicket.h"

#include "Vao/OgreIndirectBufferPacked.h"

#include "OgreRenderQueue.h"

#include "OgreTimer.h"
#include "OgreStringConverter.h"

namespace Ogre
{
    NULLVaoManager::NULLVaoManager() :
        mDrawId( 0 )
    {
        mConstBufferAlignment   = 256;
        mTexBufferAlignment     = 256;

        mConstBufferMaxSize = 64 * 1024;        //64kb
        mTexBufferMaxSize   = 128 * 1024 * 1024;//128MB

        mSupportsPersistentMapping  = true;
        mSupportsIndirectBuffers    = false;

        mDynamicBufferMultiplier = 1;

        VertexElement2Vec vertexElements;
        vertexElements.push_back( VertexElement2( VET_UINT1, VES_COUNT ) );
        uint32 *drawIdPtr = static_cast<uint32*>( OGRE_MALLOC_SIMD( 4096 * sizeof(uint32),
                                                                    MEMCATEGORY_GEOMETRY ) );
        for( uint32 i=0; i<4096; ++i )
            drawIdPtr[i] = i;
        mDrawId = createVertexBuffer( vertexElements, 4096, BT_IMMUTABLE, drawIdPtr, true );
    }
    //-----------------------------------------------------------------------------------
    NULLVaoManager::~NULLVaoManager()
    {
        destroyAllVertexArrayObjects();
        deleteAllBuffers();
    }
    //-----------------------------------------------------------------------------------
    VertexBufferPacked* NULLVaoManager::createVertexBufferImpl( size_t numElements,
                                                                   uint32 bytesPerElement,
                                                                   BufferType bufferType,
                                                                   void *initialData, bool keepAsShadow,
                                                                   const VertexElement2Vec &vElements )
    {
        NULLBufferInterface *bufferInterface = new NULLBufferInterface( 0 );
        VertexBufferPacked *retVal = OGRE_NEW VertexBufferPacked(
                                                        0, numElements, bytesPerElement, 0,
                                                        bufferType, initialData, keepAsShadow,
                                                        this, bufferInterface, vElements, 0, 0, 0 );

        if( initialData )
            bufferInterface->_firstUpload( initialData, 0, numElements );

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void NULLVaoManager::destroyVertexBufferImpl( VertexBufferPacked *vertexBuffer )
    {
    }
    //-----------------------------------------------------------------------------------
    MultiSourceVertexBufferPool* NULLVaoManager::createMultiSourceVertexBufferPoolImpl(
                                                const VertexElement2VecVec &vertexElementsBySource,
                                                size_t maxNumVertices, size_t totalBytesPerVertex,
                                                BufferType bufferType )
    {
        return OGRE_NEW NULLMultiSourceVertexBufferPool( 0, vertexElementsBySource,
                                                            maxNumVertices, bufferType,
                                                            0, this );
    }
    //-----------------------------------------------------------------------------------
    IndexBufferPacked* NULLVaoManager::createIndexBufferImpl( size_t numElements,
                                                                 uint32 bytesPerElement,
                                                                 BufferType bufferType,
                                                                 void *initialData, bool keepAsShadow )
    {
        NULLBufferInterface *bufferInterface = new NULLBufferInterface( 0 );
        IndexBufferPacked *retVal = OGRE_NEW IndexBufferPacked(
                                                        0, numElements, bytesPerElement, 0,
                                                        bufferType, initialData, keepAsShadow,
                                                        this, bufferInterface );

        if( initialData )
            bufferInterface->_firstUpload( initialData, 0, numElements );

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void NULLVaoManager::destroyIndexBufferImpl( IndexBufferPacked *indexBuffer )
    {
    }
    //-----------------------------------------------------------------------------------
    ConstBufferPacked* NULLVaoManager::createConstBufferImpl( size_t sizeBytes, BufferType bufferType,
                                                                 void *initialData, bool keepAsShadow )
    {
        uint32 alignment = mConstBufferAlignment;

        size_t bindableSize = sizeBytes;

        if( bufferType >= BT_DYNAMIC_DEFAULT )
        {
            //For dynamic buffers, the size will be 3x times larger
            //(depending on mDynamicBufferMultiplier); we need the
            //offset after each map to be aligned; and for that, we
            //sizeBytes to be multiple of alignment.
            sizeBytes = ( (sizeBytes + alignment - 1) / alignment ) * alignment;
        }

        NULLBufferInterface *bufferInterface = new NULLBufferInterface( 0 );
        ConstBufferPacked *retVal = OGRE_NEW NULLConstBufferPacked(
                                                        0, sizeBytes, 1, 0,
                                                        bufferType, initialData, keepAsShadow,
                                                        this, bufferInterface, bindableSize );

        if( initialData )
            bufferInterface->_firstUpload( initialData, 0, sizeBytes );

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void NULLVaoManager::destroyConstBufferImpl( ConstBufferPacked *constBuffer )
    {
    }
    //-----------------------------------------------------------------------------------
    TexBufferPacked* NULLVaoManager::createTexBufferImpl( PixelFormat pixelFormat, size_t sizeBytes,
                                                             BufferType bufferType,
                                                             void *initialData, bool keepAsShadow )
    {
        uint32 alignment = mTexBufferAlignment;

        VboFlag vboFlag = bufferTypeToVboFlag( bufferType );

        if( bufferType >= BT_DYNAMIC_DEFAULT )
        {
            //For dynamic buffers, the size will be 3x times larger
            //(depending on mDynamicBufferMultiplier); we need the
            //offset after each map to be aligned; and for that, we
            //sizeBytes to be multiple of alignment.
            sizeBytes = ( (sizeBytes + alignment - 1) / alignment ) * alignment;
        }

        NULLBufferInterface *bufferInterface = new NULLBufferInterface( 0 );
        TexBufferPacked *retVal = OGRE_NEW NULLTexBufferPacked(
                                                        0, sizeBytes, 1, 0,
                                                        bufferType, initialData, keepAsShadow,
                                                        this, bufferInterface, pixelFormat );

        if( initialData )
            bufferInterface->_firstUpload( initialData, 0, sizeBytes );

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void NULLVaoManager::destroyTexBufferImpl( TexBufferPacked *texBuffer )
    {
    }
    //-----------------------------------------------------------------------------------
    UavBufferPacked* NULLVaoManager::createUavBufferImpl( size_t numElements, uint32 bytesPerElement,
                                                          uint32 bindFlags,
                                                          void *initialData, bool keepAsShadow )
    {
        NULLBufferInterface *bufferInterface = new NULLBufferInterface( 0 );
        UavBufferPacked *retVal = OGRE_NEW NULLUavBufferPacked(
                                                        0, numElements, bytesPerElement,
                                                        bindFlags, initialData, keepAsShadow,
                                                        this, bufferInterface );

        if( initialData )
            bufferInterface->_firstUpload( initialData, 0, numElements );

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void NULLVaoManager::destroyUavBufferImpl( UavBufferPacked *uavBuffer )
    {
    }
    //-----------------------------------------------------------------------------------
    IndirectBufferPacked* NULLVaoManager::createIndirectBufferImpl( size_t sizeBytes,
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

        NULLBufferInterface *bufferInterface = 0;
        if( mSupportsIndirectBuffers )
        {
            bufferInterface = new NULLBufferInterface( 0 );
        }

        IndirectBufferPacked *retVal = OGRE_NEW IndirectBufferPacked(
                                                        0, sizeBytes, 1, 0,
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
    void NULLVaoManager::destroyIndirectBufferImpl( IndirectBufferPacked *indirectBuffer )
    {
        if( mSupportsIndirectBuffers )
        {
        }
    }
    //-----------------------------------------------------------------------------------
    VertexArrayObject* NULLVaoManager::createVertexArrayObjectImpl(
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

        NULLVertexArrayObject *retVal = OGRE_NEW NULLVertexArrayObject( idx,
                                                                        renderQueueId,
                                                                        vertexBuffers,
                                                                        indexBuffer,
                                                                        opType );

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void NULLVaoManager::destroyVertexArrayObjectImpl( VertexArrayObject *vao )
    {
        NULLVertexArrayObject *glVao = static_cast<NULLVertexArrayObject*>( vao );
        OGRE_DELETE glVao;
    }
    //-----------------------------------------------------------------------------------
    StagingBuffer* NULLVaoManager::createStagingBuffer( size_t sizeBytes, bool forUpload )
    {
        sizeBytes = std::max<size_t>( sizeBytes, 4 * 1024 * 1024 );
        NULLStagingBuffer *stagingBuffer = OGRE_NEW NULLStagingBuffer( 0, sizeBytes, this, forUpload );
        mRefedStagingBuffers[forUpload].push_back( stagingBuffer );

        if( mNextStagingBufferTimestampCheckpoint == (unsigned long)( ~0 ) )
            mNextStagingBufferTimestampCheckpoint = mTimer->getMilliseconds() + mDefaultStagingBufferLifetime;

        return stagingBuffer;
    }
    //-----------------------------------------------------------------------------------
    AsyncTicketPtr NULLVaoManager::createAsyncTicket( BufferPacked *creator,
                                                         StagingBuffer *stagingBuffer,
                                                         size_t elementStart, size_t elementCount )
    {
        return AsyncTicketPtr( OGRE_NEW NULLAsyncTicket( creator, stagingBuffer,
                                                            elementStart, elementCount ) );
    }
    //-----------------------------------------------------------------------------------
    void NULLVaoManager::_update(void)
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
                        stagingBuffer->getLastUsedTimestamp() + stagingBuffer->getLifetimeThreshold() );

                    /*if( stagingBuffer->getLastUsedTimestamp() + stagingBuffer->getUnfencedTimeThreshold() < currentTimeMs )
                    {
                        static_cast<NULLStagingBuffer*>( stagingBuffer )->cleanUnfencedHazards();
                    }*/

                    if( stagingBuffer->getLastUsedTimestamp() + stagingBuffer->getLifetimeThreshold() < currentTimeMs )
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
    uint8 NULLVaoManager::waitForTailFrameToFinish(void)
    {
        return mDynamicBufferCurrentFrame;
    }
    //-----------------------------------------------------------------------------------
    void NULLVaoManager::waitForSpecificFrameToFinish( uint32 frameCount )
    {
    }
    //-----------------------------------------------------------------------------------
    bool NULLVaoManager::isFrameFinished( uint32 frameCount )
    {
        return true;
    }
    //-----------------------------------------------------------------------------------
    NULLVaoManager::VboFlag NULLVaoManager::bufferTypeToVboFlag( BufferType bufferType )
    {
        return static_cast<VboFlag>( std::max( 0, (bufferType - BT_DYNAMIC_DEFAULT) +
                                                    CPU_ACCESSIBLE_DEFAULT ) );
    }
}

