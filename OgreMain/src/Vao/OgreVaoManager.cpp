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
#include "Vao/OgreVaoManager.h"
#include "Vao/OgreStagingBuffer.h"
#include "Vao/OgreVertexArrayObject.h"
#include "Vao/OgreConstBufferPacked.h"
#include "Vao/OgreTexBufferPacked.h"
#include "Vao/OgreUavBufferPacked.h"
#include "Vao/OgreIndirectBufferPacked.h"
#include "OgreTimer.h"
#include "OgreCommon.h"
#include "OgreLogManager.h"

namespace Ogre
{
    VaoManager::VaoManager() :
        mTimer( 0 ),
        mDefaultStagingBufferUnfencedTime( 300000 - 1000 ), //4 minutes, 59 seconds
        mDefaultStagingBufferLifetime( 300000 ), //5 minutes
        mSupportsPersistentMapping( false ),
        mSupportsIndirectBuffers( false ),
        mSupportsBaseInstance( true ),
        mDynamicBufferMultiplier( 3 ),
        mDynamicBufferCurrentFrame( 0 ),
        mNextStagingBufferTimestampCheckpoint( ~0 ),
        mFrameCount( 0 ),
        mNumGeneratedVaos( 0 ),
        mConstBufferAlignment( 256 ),
        mTexBufferAlignment( 256 ),
        mUavBufferAlignment( 256 ),
        mConstBufferMaxSize( 16 * 1024 * 1024 ), //Minimum guaranteed by GL.
        mTexBufferMaxSize( 64 * 1024 * 1024 ),  //Minimum guaranteed by GL. Intel HD Graphics 3000-5000/Iris provide 64M only
        mUavBufferMaxSize( 16 * 1024 * 1024 )    //Minimum guaranteed by GL.
    {
        mTimer = OGRE_NEW Timer();
    }
    //-----------------------------------------------------------------------------------
    VaoManager::~VaoManager()
    {
        for( size_t i=0; i<2; ++i )
        {
            StagingBufferVec::const_iterator itor = mRefedStagingBuffers[i].begin();
            StagingBufferVec::const_iterator end  = mRefedStagingBuffers[i].end();

            while( itor != end )
            {
                OGRE_DELETE *itor;
                ++itor;
            }

            itor = mZeroRefStagingBuffers[i].begin();
            end  = mZeroRefStagingBuffers[i].end();

            while( itor != end )
            {
                OGRE_DELETE *itor;
                ++itor;
            }
        }

        OGRE_DELETE mTimer;
        mTimer = 0;
    }
    //-----------------------------------------------------------------------------------
    uint32 VaoManager::calculateVertexSize( const VertexElement2Vec &vertexElements )
    {
        VertexElement2Vec::const_iterator itor = vertexElements.begin();
        VertexElement2Vec::const_iterator end  = vertexElements.end();

        uint32 bytesPerVertex = 0;

        while( itor != end )
        {
            bytesPerVertex += v1::VertexElement::getTypeSize( itor->mType );
            ++itor;
        }

        return bytesPerVertex;
    }
    //-----------------------------------------------------------------------------------
    VertexBufferPacked* VaoManager::createVertexBuffer( const VertexElement2Vec &vertexElements,
                                                        size_t numVertices, BufferType bufferType,
                                                        void *initialData, bool keepAsShadow )
    {
        uint32 bytesPerVertex = calculateVertexSize( vertexElements );

        VertexBufferPacked *retVal = createVertexBufferImpl( numVertices, bytesPerVertex, bufferType,
                                                             initialData, keepAsShadow, vertexElements );

        mBuffers[BP_TYPE_VERTEX].insert( retVal );
        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void VaoManager::destroyVertexBuffer( VertexBufferPacked *vertexBuffer )
    {
        if( vertexBuffer->getMultiSourcePool() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "Vertex Buffer belongs to a Multisource pool, not this VaoManager",
                         "VaoManager::destroyVertexBuffer" );
        }

        BufferPackedSet::iterator itor = mBuffers[ BP_TYPE_VERTEX ].find( vertexBuffer );

        if( itor == mBuffers[BP_TYPE_VERTEX].end() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                         "Vertex Buffer has already been destroyed or "
                         "doesn't belong to this VaoManager.",
                         "VaoManager::destroyVertexBuffer" );
        }

        if( vertexBuffer->getBufferType() >= BT_DYNAMIC_DEFAULT )
        {
            //We need to delay the removal of this buffer until
            //we're sure it's not in use by the GPU anymore
            DelayedBuffer delayedBuffer( vertexBuffer, mFrameCount, mDynamicBufferCurrentFrame );
            mDelayedDestroyBuffers.push_back( delayedBuffer );
        }
        else
        {
            destroyVertexBufferImpl( vertexBuffer );
            OGRE_DELETE vertexBuffer;
        }

        mBuffers[ BP_TYPE_VERTEX ].erase( itor );
    }
    //-----------------------------------------------------------------------------------
    IndexBufferPacked* VaoManager::createIndexBuffer( IndexBufferPacked::IndexType indexType,
                                                      size_t numIndices, BufferType bufferType,
                                                      void *initialData, bool keepAsShadow )
    {
        IndexBufferPacked *retVal;
        retVal = createIndexBufferImpl( numIndices, indexType == IndexBufferPacked::IT_16BIT ? 2 : 4,
                                        bufferType, initialData, keepAsShadow );
        mBuffers[BP_TYPE_INDEX].insert( retVal );
        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void VaoManager::destroyIndexBuffer( IndexBufferPacked *indexBuffer )
    {
        BufferPackedSet::iterator itor = mBuffers[ BP_TYPE_INDEX ].find( indexBuffer );

        if( itor == mBuffers[BP_TYPE_INDEX].end() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                         "Index Buffer has already been destroyed or "
                         "doesn't belong to this VaoManager.",
                         "VaoManager::destroyIndexBuffer" );
        }

        if( indexBuffer->getBufferType() >= BT_DYNAMIC_DEFAULT )
        {
            //We need to delay the removal of this buffer until
            //we're sure it's not in use by the GPU anymore
            DelayedBuffer delayedBuffer( indexBuffer, mFrameCount, mDynamicBufferCurrentFrame );
            mDelayedDestroyBuffers.push_back( delayedBuffer );
        }
        else
        {
            destroyIndexBufferImpl( indexBuffer );
            OGRE_DELETE *itor;
        }

        mBuffers[ BP_TYPE_INDEX ].erase( itor );
    }
    //-----------------------------------------------------------------------------------
    ConstBufferPacked* VaoManager::createConstBuffer( size_t sizeBytes, BufferType bufferType,
                                                      void *initialData, bool keepAsShadow )
    {
        ConstBufferPacked *retVal;
        retVal = createConstBufferImpl( sizeBytes, bufferType, initialData, keepAsShadow );
        mBuffers[BP_TYPE_CONST].insert( retVal );
        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void VaoManager::destroyConstBuffer( ConstBufferPacked *constBuffer )
    {
        BufferPackedSet::iterator itor = mBuffers[ BP_TYPE_CONST ].find( constBuffer );

        if( itor == mBuffers[BP_TYPE_CONST].end() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                         "Constant Buffer has already been destroyed or "
                         "doesn't belong to this VaoManager.",
                         "VaoManager::destroyConstBuffer" );
        }

        if( constBuffer->getBufferType() >= BT_DYNAMIC_DEFAULT )
        {
            //We need to delay the removal of this buffer until
            //we're sure it's not in use by the GPU anymore
            DelayedBuffer delayedBuffer( constBuffer, mFrameCount, mDynamicBufferCurrentFrame );
            mDelayedDestroyBuffers.push_back( delayedBuffer );
        }
        else
        {
            destroyConstBufferImpl( constBuffer );
            OGRE_DELETE *itor;
        }

        mBuffers[ BP_TYPE_CONST ].erase( itor );
    }
    //-----------------------------------------------------------------------------------
    TexBufferPacked* VaoManager::createTexBuffer( PixelFormat pixelFormat, size_t sizeBytes,
                                                  BufferType bufferType,
                                                  void *initialData, bool keepAsShadow )
    {
        TexBufferPacked *retVal;
        retVal = createTexBufferImpl( pixelFormat, sizeBytes, bufferType, initialData, keepAsShadow );
        mBuffers[BP_TYPE_TEX].insert( retVal );
        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void VaoManager::destroyTexBuffer( TexBufferPacked *texBuffer )
    {
        BufferPackedSet::iterator itor = mBuffers[ BP_TYPE_TEX ].find( texBuffer );

        if( itor == mBuffers[BP_TYPE_TEX].end() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                         "Texture Buffer has already been destroyed or "
                         "doesn't belong to this VaoManager.",
                         "VaoManager::destroyTexBuffer" );
        }

        if( texBuffer->getBufferType() >= BT_DYNAMIC_DEFAULT )
        {
            //We need to delay the removal of this buffer until
            //we're sure it's not in use by the GPU anymore
            DelayedBuffer delayedBuffer( texBuffer, mFrameCount, mDynamicBufferCurrentFrame );
            mDelayedDestroyBuffers.push_back( delayedBuffer );
        }
        else
        {
            destroyTexBufferImpl( texBuffer );
            OGRE_DELETE *itor;
        }

        mBuffers[ BP_TYPE_TEX ].erase( itor );
    }
    //-----------------------------------------------------------------------------------
    UavBufferPacked* VaoManager::createUavBuffer( size_t numElements, uint32 bytesPerElement,
                                                  uint32 bindFlags,
                                                  void *initialData, bool keepAsShadow )
    {
        UavBufferPacked *retVal;
        retVal = createUavBufferImpl( numElements, bytesPerElement, bindFlags,
                                      initialData, keepAsShadow );
        mBuffers[BP_TYPE_UAV].insert( retVal );
        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void VaoManager::destroyUavBuffer( UavBufferPacked *uavBuffer )
    {
        BufferPackedSet::iterator itor = mBuffers[BP_TYPE_UAV].find( uavBuffer );

        if( itor == mBuffers[BP_TYPE_UAV].end() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                         "UAV Buffer has already been destroyed or "
                         "doesn't belong to this VaoManager.",
                         "VaoManager::destroyUavBuffer" );
        }

        assert( uavBuffer->getBufferType() == BT_DEFAULT );

        destroyUavBufferImpl( uavBuffer );
        OGRE_DELETE *itor;

        mBuffers[BP_TYPE_UAV].erase( itor );
    }
    //-----------------------------------------------------------------------------------
    IndirectBufferPacked* VaoManager::createIndirectBuffer( size_t sizeBytes, BufferType bufferType,
                                                            void *initialData, bool keepAsShadow )
    {
        IndirectBufferPacked *retVal;
        retVal = createIndirectBufferImpl( sizeBytes, bufferType, initialData, keepAsShadow );
        mBuffers[BP_TYPE_INDIRECT].insert( retVal );
        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void VaoManager::destroyIndirectBuffer( IndirectBufferPacked *indirectBuffer )
    {
        BufferPackedSet::iterator itor = mBuffers[ BP_TYPE_INDIRECT ].find( indirectBuffer );

        if( itor == mBuffers[BP_TYPE_INDIRECT].end() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                         "Indirect Buffer has already been destroyed or "
                         "doesn't belong to this VaoManager.",
                         "VaoManager::destroyIndirectBuffer" );
        }

        if( indirectBuffer->getBufferType() >= BT_DYNAMIC_DEFAULT )
        {
            //We need to delay the removal of this buffer until
            //we're sure it's not in use by the GPU anymore
            DelayedBuffer delayedBuffer( indirectBuffer, mFrameCount, mDynamicBufferCurrentFrame );
            mDelayedDestroyBuffers.push_back( delayedBuffer );
        }
        else
        {
            destroyIndirectBufferImpl( indirectBuffer );
            OGRE_DELETE *itor;
        }

        mBuffers[BP_TYPE_INDIRECT].erase( itor );
    }
    //-----------------------------------------------------------------------------------
    VertexArrayObject* VaoManager::createVertexArrayObject( const VertexBufferPackedVec &vertexBuffers,
                                                            IndexBufferPacked *indexBuffer,
                                                            OperationType opType )
    {
        if( vertexBuffers.size() > 1 )
        {
            size_t multiSourceId                            = vertexBuffers[0]->getMultiSourceId();
            MultiSourceVertexBufferPool *multiSourcePool    = vertexBuffers[0]->getMultiSourcePool();

            VertexBufferPackedVec::const_iterator itor = vertexBuffers.begin();
            VertexBufferPackedVec::const_iterator end  = vertexBuffers.end();

            while( itor != end )
            {
                if( !(*itor)->getMultiSourcePool() )
                {
                    OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                                 "Cannot use a non-multisource vertex buffer "
                                 "in a multisource declaration",
                                 "VaoManager::createVertexArrayObject" );
                }

                if( multiSourceId != (*itor)->getMultiSourceId() ||
                    multiSourcePool != (*itor)->getMultiSourcePool() )
                {
                    OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                                 "Multisource vertex buffers can only be bound together with "
                                 "the other buffers returned by the same "
                                 "MultiSourceVertexBufferPool::createVertexBuffer call. "
                                 "Don't mix vertex buffers from different pools or calls.",
                                 "VaoManager::createVertexArrayObject" );
                }

                ++itor;
            }
        }

        VertexArrayObject *retVal;
        retVal = createVertexArrayObjectImpl( vertexBuffers, indexBuffer, opType );
        mVertexArrayObjects.insert( retVal );
        ++mNumGeneratedVaos;
        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void VaoManager::destroyVertexArrayObject( VertexArrayObject *vao )
    {
        VertexArrayObjectSet::iterator itor = mVertexArrayObjects.find( vao );

        if( itor == mVertexArrayObjects.end() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                         "Vertex Array Object has already been destroyed or "
                         "doesn't belong to this VaoManager.",
                         "VaoManager::destroyVertexArrayObject" );
        }

        destroyVertexArrayObjectImpl( vao );
        mVertexArrayObjects.erase( itor );
    }
    //-----------------------------------------------------------------------------------
    void VaoManager::destroyAllVertexArrayObjects(void)
    {
        VertexArrayObjectSet::const_iterator itor = mVertexArrayObjects.begin();
        VertexArrayObjectSet::const_iterator end  = mVertexArrayObjects.end();

        while( itor != end )
        {
            destroyVertexArrayObjectImpl( *itor );
            ++itor;
        }

        mVertexArrayObjects.clear();
    }
    //-----------------------------------------------------------------------------------
    void VaoManager::deleteAllBuffers(void)
    {
        for( int i=0; i<NUM_BUFFER_PACKED_TYPES; ++i )
        {
            BufferPackedSet::const_iterator itor = mBuffers[i].begin();
            BufferPackedSet::const_iterator end  = mBuffers[i].end();

            while( itor != end )
            {
                //For some RS 'callDestroyBufferImpl' is unnecessary and will only
                //increase shutdown times. However for other RS, this is necessary.
                callDestroyBufferImpl( *itor );
                OGRE_DELETE *itor++;
            }

            mBuffers[i].clear();
        }

        {
            DelayedBufferVec::const_iterator itor = mDelayedDestroyBuffers.begin();
            DelayedBufferVec::const_iterator end  = mDelayedDestroyBuffers.end();

            while( itor != end )
            {
                //For some RS 'callDestroyBufferImpl' is unnecessary and will only
                //increase shutdown times. However for other RS, this is necessary.
                callDestroyBufferImpl( itor->bufferPacked );
                OGRE_DELETE itor->bufferPacked;
                ++itor;
            }

            mDelayedDestroyBuffers.clear();
        }
    }
    //-----------------------------------------------------------------------------------
    StagingBuffer* VaoManager::getStagingBuffer( size_t minSizeBytes, bool forUpload )
    {
        StagingBuffer *candidates[NUM_STALL_TYPES];
        memset( candidates, 0, sizeof( candidates ) );

        StagingBufferVec::const_iterator itor = mZeroRefStagingBuffers[forUpload].begin();
        StagingBufferVec::const_iterator end  = mZeroRefStagingBuffers[forUpload].end();

        while( itor != end )
        {
            if( forUpload )
            {
                if( minSizeBytes <= (*itor)->getMaxSize() )
                {
                    StagingStallType stallType = (*itor)->uploadWillStall( minSizeBytes );
                    candidates[stallType] = *itor;

                    //This is best case scenario, we can stop looking.
                    if( stallType == STALL_NONE )
                        break;
                }
            }
            else
            {
                if( (*itor)->canDownload( minSizeBytes ) )
                {
                    candidates[0] = *itor;
                    break;
                }
            }

            ++itor;
        }

        StagingBuffer *retVal = candidates[STALL_FULL];

        for( size_t i=0; i<NUM_STALL_TYPES && !retVal; ++i )
            retVal = candidates[i];

        if( !retVal )
        {
            //No buffer is large enough. Get a new one.
            retVal = createStagingBuffer( minSizeBytes, forUpload );
        }
        else
        {
            retVal->addReferenceCount();
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void VaoManager::destroyDelayedBuffers( uint8 fromDynamicFrame )
    {
        DelayedBufferVec::iterator itor = mDelayedDestroyBuffers.begin();
        DelayedBufferVec::iterator end  = mDelayedDestroyBuffers.end();

        while( itor != end )
        {
            if( itor->frameNumDynamic != fromDynamicFrame || itor->frame == mFrameCount )
                break;

            assert( mFrameCount - itor->frame == mDynamicBufferMultiplier &&
                    "Delayed buffer must be destroyed in the last buffered frame!" );

            callDestroyBufferImpl( itor->bufferPacked );

            OGRE_DELETE itor->bufferPacked;

            ++itor;
        }

        mDelayedDestroyBuffers.erase( mDelayedDestroyBuffers.begin(), itor );
    }
    //-----------------------------------------------------------------------------------
    inline void VaoManager::callDestroyBufferImpl( BufferPacked *bufferPacked )
    {
        switch( bufferPacked->getBufferPackedType() )
        {
        case BP_TYPE_VERTEX:
            assert( dynamic_cast<VertexBufferPacked*>( bufferPacked ) );
            destroyVertexBufferImpl( static_cast<VertexBufferPacked*>( bufferPacked ) );
            break;
        case BP_TYPE_INDEX:
            assert( dynamic_cast<IndexBufferPacked*>( bufferPacked ) );
            destroyIndexBufferImpl( static_cast<IndexBufferPacked*>( bufferPacked ) );
            break;
        case BP_TYPE_CONST:
            assert( dynamic_cast<ConstBufferPacked*>( bufferPacked ) );
            destroyConstBufferImpl( static_cast<ConstBufferPacked*>( bufferPacked ) );
            break;
        case BP_TYPE_TEX:
            assert( dynamic_cast<TexBufferPacked*>( bufferPacked ) );
            destroyTexBufferImpl( static_cast<TexBufferPacked*>( bufferPacked ) );
            break;
        case BP_TYPE_UAV:
            assert( dynamic_cast<UavBufferPacked*>( bufferPacked ) );
            destroyUavBufferImpl( static_cast<UavBufferPacked*>( bufferPacked ) );
            break;
        case BP_TYPE_INDIRECT:
            assert( dynamic_cast<IndirectBufferPacked*>( bufferPacked ) );
            destroyIndirectBufferImpl( static_cast<IndirectBufferPacked*>( bufferPacked ) );
            break;
        }
    }
    //-----------------------------------------------------------------------------------
    void VaoManager::_update(void)
    {
        ++mFrameCount;
    }
    //-----------------------------------------------------------------------------------
    void VaoManager::_notifyStagingBufferEnteredZeroRef( StagingBuffer *stagingBuffer )
    {
        StagingBufferVec &refedStagingBuffers = mRefedStagingBuffers[stagingBuffer->getUploadOnly()];
        StagingBufferVec::iterator itor = std::find( refedStagingBuffers.begin(),
                                                     refedStagingBuffers.end(), stagingBuffer );

        assert( itor != refedStagingBuffers.end() );
        efficientVectorRemove( refedStagingBuffers, itor );

        mZeroRefStagingBuffers[stagingBuffer->getUploadOnly()].push_back( stagingBuffer );
    }
    //-----------------------------------------------------------------------------------
    void VaoManager::_notifyStagingBufferLeftZeroRef( StagingBuffer *stagingBuffer )
    {
        StagingBufferVec &zeroRefStagingBuffers = mZeroRefStagingBuffers[stagingBuffer->getUploadOnly()];
        StagingBufferVec::iterator itor = std::find( zeroRefStagingBuffers.begin(),
                                                     zeroRefStagingBuffers.end(), stagingBuffer );

        assert( itor != zeroRefStagingBuffers.end() );
        efficientVectorRemove( zeroRefStagingBuffers, itor );

        mRefedStagingBuffers[stagingBuffer->getUploadOnly()].push_back( stagingBuffer );
    }
    //-----------------------------------------------------------------------------------
    void VaoManager::setDefaultStagingBufferlifetime( uint32 lifetime, uint32 unfencedTime )
    {
        if( unfencedTime > lifetime )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "unfencedTime must be equal or lower than lifetime",
                         "VaoManager::setDefaultStagingBufferlifetime" );
        }

        if( unfencedTime == lifetime )
        {
            LogManager::getSingleton().logMessage(
                        "WARNING: lifetime is equal to unfencedTime in "
                        "VaoManager::setDefaultStagingBufferlifetime. This could give you random "
                        "stalls or framerate hiccups. You should set unfencedTime to some time "
                        "earlier to lifetime. Like 1 second earlier. But not too distant either "
                        "to prevent API overhead.", LML_CRITICAL );
        }

        mDefaultStagingBufferLifetime       = lifetime;
        mDefaultStagingBufferUnfencedTime   = unfencedTime;
    }
}

