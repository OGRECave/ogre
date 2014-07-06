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

#include "Vao/OgreGL3PlusVaoManager.h"
#include "Vao/OgreGL3PlusStagingBuffer.h"
#include "Vao/OgreGL3PlusVertexArrayObject.h"
#include "Vao/OgreGL3PlusBufferInterface.h"
#include "Vao/OgreGL3PlusMultiSourceVertexBufferPool.h"

#include "OgreGL3PlusHardwareBufferManager.h" //GL3PlusHardwareBufferManager::getGLType

#include "OgreTimer.h"

namespace Ogre
{
    GL3PlusVaoManager::GL3PlusVaoManager()
    {
        //Keep pools of 128MB each for static meshes
        mDefaultPoolSize[CPU_INACCESSIBLE]  = 128 * 1024 * 1024;
        //Keep pools of 32MB each for dynamic vertex buffers
        mDefaultPoolSize[CPU_ACCESSIBLE]    = 32 * 1024 * 1024;
    }
    //-----------------------------------------------------------------------------------
    GL3PlusVaoManager::~GL3PlusVaoManager()
    {
        vector<GLuint>::type bufferNames;

        bufferNames.reserve( mStagingBuffers[0].size() + mStagingBuffers[1].size() );

        for( size_t i=0; i<2; ++i )
        {
            StagingBufferVec::const_iterator itor = mStagingBuffers[i].begin();
            StagingBufferVec::const_iterator end  = mStagingBuffers[i].end();

            while( itor != end )
            {
                bufferNames.push_back( static_cast<GL3PlusStagingBuffer*>(*itor)->getBufferName() );
                ++itor;
            }
        }

        for( size_t i=0; i<MAX_VBO_FLAG; ++i )
        {
            VboVec::const_iterator itor = mVbos[i].begin();
            VboVec::const_iterator end  = mVbos[i].end();

            while( itor != end )
            {
                bufferNames.push_back( itor->vboName );
                ++itor;
            }
        }

        if( !bufferNames.empty() )
        {
            glDeleteBuffers( bufferNames.size(), &bufferNames[0] );
            bufferNames.clear();
        }
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusVaoManager::allocateVbo( size_t sizeBytes, size_t bytesPerElement, BufferType bufferType,
                                         size_t &outVboIdx, size_t &outBufferOffset )
    {
        assert( bytesPerElement > 0 );

        VboFlag vboFlag = CPU_INACCESSIBLE;

        if( bufferType == BT_DYNAMIC )
        {
            sizeBytes   *= mDynamicBufferMultiplier;
            vboFlag     = CPU_ACCESSIBLE;
        }

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

                //Round to next multiple of bytesPerElement
                size_t newOffset = ( (block.offset + bytesPerElement - 1) /
                                     bytesPerElement ) * bytesPerElement;

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

        if( bestBlockIdx == ~0 )
        {
            bestVboIdx      = mVbos[vboFlag].size();
            bestBlockIdx    = 0;
            foundMatchingStride = true;

            Vbo newVbo;

            size_t poolSize = std::max( mDefaultPoolSize[vboFlag], sizeBytes );

            //TODO: Deal with Out of memory errors
            //No luck, allocate a new buffer.
            OCGLE( glGenBuffers( 1, &newVbo.vboName ) );
            OCGLE( glBindBuffer( GL_ARRAY_BUFFER, newVbo.vboName ) );

            if( mArbBufferStorage )
            {
                if( vboFlag == CPU_ACCESSIBLE )
                {
                    OCGLE( glBufferStorage( GL_ARRAY_BUFFER, poolSize, 0,
                                            GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT |
                                            GL_MAP_COHERENT_BIT ) );
                }
                else
                {
                    OCGLE( glBufferStorage( GL_ARRAY_BUFFER, poolSize, 0, 0 ) );
                }
            }
            else
            {
                OCGLE( glBufferData( GL_ARRAY_BUFFER, mDefaultPoolSize[vboFlag], 0,
                                     vboFlag == CPU_INACCESSIBLE ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW ) );
            }
            OCGLE( glBindBuffer( GL_ARRAY_BUFFER, 0 ) );

            newVbo.sizeBytes = poolSize;
            newVbo.freeBlocks.push_back( Block( 0, poolSize ) );

            mVbos[vboFlag].push_back( newVbo );
        }

        Vbo &bestVbo        = mVbos[vboFlag][bestVboIdx];
        Block &bestBlock    = bestVbo.freeBlocks[bestBlockIdx];

        size_t newOffset = ( (bestBlock.offset + bytesPerElement - 1) /
                             bytesPerElement ) * bytesPerElement;
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
            mVbos[vboFlag].erase( mVbos[vboFlag].begin() + bestVboIdx );

        outVboIdx       = bestVboIdx;
        outBufferOffset = newOffset;
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusVaoManager::deallocateVbo( size_t vboIdx, size_t bufferOffset, size_t sizeBytes,
                                           BufferType bufferType )
    {
        VboFlag vboFlag = bufferType == BT_DYNAMIC ? CPU_ACCESSIBLE: CPU_INACCESSIBLE;

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
    void GL3PlusVaoManager::mergeContiguousBlocks( BlockVec::iterator blockToMerge,
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
                efficientVectorRemove( blocks, blockToMerge );

                blockToMerge = blocks.begin() + idx;
                itor = blocks.begin();
                end  = blocks.end();
            }
            else if( blockToMerge->offset + blockToMerge->size == itor->offset )
            {
                blockToMerge->size += itor->size;
                size_t idx = blockToMerge - blocks.begin();
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
    VertexBufferPacked* GL3PlusVaoManager::createVertexBufferImpl( size_t numElements,
                                                                   uint32 bytesPerElement,
                                                                   BufferType bufferType,
                                                                   void *initialData, bool keepAsShadow,
                                                                   const VertexElement2Vec &vElements )
    {
        size_t vboIdx;
        size_t bufferOffset;

        allocateVbo( numElements * bytesPerElement, bytesPerElement, bufferType, vboIdx, bufferOffset );

        return 0;
    }
    //-----------------------------------------------------------------------------------
    MultiSourceVertexBufferPool* GL3PlusVaoManager::createMultiSourceVertexBufferPoolImpl(
                                                const VertexElement2VecVec &vertexElementsBySource,
                                                size_t maxNumVertices, size_t totalBytesPerVertex,
                                                BufferType bufferType )
    {
        size_t vboIdx;
        size_t bufferOffset;

        allocateVbo( maxNumVertices * totalBytesPerVertex, totalBytesPerVertex,
                     bufferType, vboIdx, bufferOffset );

        VboFlag vboFlag = CPU_INACCESSIBLE;

        if( bufferType == BT_DYNAMIC )
            vboFlag = CPU_ACCESSIBLE;

        const Vbo &vbo = mVbos[vboFlag][vboIdx];

        return OGRE_NEW GL3PlusMultiSourceVertexBufferPool( vboIdx, vbo.vboName, vertexElementsBySource,
                                                            maxNumVertices, bufferType,
                                                            bufferOffset, this );
    }
    //-----------------------------------------------------------------------------------
    GLuint GL3PlusVaoManager::createVao( const Vao &vaoRef )
    {
        GLuint vaoName;
        OCGLE( glGenVertexArrays( 1, &vaoName ) );
        OCGLE( glBindVertexArray( vaoName ) );

        size_t attributeIndex = 0;

        for( size_t i=0; i<vaoRef.vertexBuffers.size(); ++i )
        {
            const Vao::VertexBinding &binding = vaoRef.vertexBuffers[i];

            glBindBuffer( GL_ARRAY_BUFFER, binding.vertexBufferVbo );

            VertexElement2Vec::const_iterator it = binding.vertexElements.begin();
            VertexElement2Vec::const_iterator en = binding.vertexElements.end();

            while( it != en )
            {
                GLint typeCount = VertexElement::getTypeCount( it->mType );
                GLboolean normalised = GL_FALSE;

                switch( it->mType )
                {
                case VET_COLOUR:
                case VET_COLOUR_ABGR:
                case VET_COLOUR_ARGB:
                    // Because GL takes these as a sequence of single unsigned bytes, count needs to be 4
                    // VertexElement::getTypeCount treats them as 1 (RGBA)
                    // Also need to normalise the fixed-point data
                    typeCount = 4;
                    normalised = GL_TRUE;
                    break;
                default:
                    break;
                };

                switch( VertexElement::getBaseType( it->mType ) )
                {
                default:
                case VET_FLOAT1:
                    OCGLE( glVertexAttribPointer( attributeIndex, typeCount,
                                                  GL3PlusHardwareBufferManager::getGLType( it->mType ),
                                                  normalised, binding.stride, 0 ) );
                    break;
                case VET_DOUBLE1:
                    OCGLE( glVertexAttribLPointer( attributeIndex, typeCount,
                                                   GL3PlusHardwareBufferManager::getGLType( it->mType ),
                                                   binding.stride, 0 ) );
                    break;
                }

                OCGLE( glVertexAttribDivisor( attributeIndex, binding.instancingDivisor ) );
                OCGLE( glEnableVertexAttribArray( attributeIndex ) );

                ++attributeIndex;
                ++it;
            }

            OCGLE( glBindBuffer( GL_ARRAY_BUFFER, 0 ) );
        }

        OCGLE( glBindVertexArray( 0 ) );

        return vaoName;
    }
    //-----------------------------------------------------------------------------------
    VertexArrayObject* GL3PlusVaoManager::createVertexArrayObjectImpl(
                                                            const VertexBufferPackedVec &vertexBuffers,
                                                            IndexBufferPacked *indexBuffer )
    {
        Vao vao;

        vao.vertexBuffers.reserve( vertexBuffers.size() );

        {
            VertexBufferPackedVec::const_iterator itor = vertexBuffers.begin();
            VertexBufferPackedVec::const_iterator end  = vertexBuffers.end();

            while( itor != end )
            {
                Vao::VertexBinding vertexBinding;
                vertexBinding.vertexBufferVbo = static_cast<GL3PlusBufferInterface*>(
                                                    (*itor)->getBufferInterface() )->getVboName();
                vertexBinding.vertexElements = (*itor)->getVertexElements();
                ++itor;
            }
        }

        vao.indexBufferVbo  = static_cast<GL3PlusBufferInterface*>(
                                indexBuffer->getBufferInterface() )->getVboName();

        bool bFound = false;
        VaoVec::const_iterator itor = mVaos.begin();
        VaoVec::const_iterator end  = mVaos.end();

        while( itor != end && !bFound )
        {
            if( itor->indexBufferVbo == vao.indexBufferVbo &&
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
            mVaos.push_back( vao );
            itor = mVaos.begin() + mVaos.size() - 1;
        }

        GL3PlusVertexArrayObject *retVal = OGRE_NEW GL3PlusVertexArrayObject( itor->vaoName,
                                                                              vertexBuffers,
                                                                              indexBuffer );

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    StagingBuffer* GL3PlusVaoManager::createStagingBuffer( size_t sizeBytes, bool forUpload )
    {
        GLuint bufferName;
        GLenum target = forUpload ? GL_COPY_READ_BUFFER : GL_COPY_WRITE_BUFFER;
        OCGLE( glGenBuffers( 1, &bufferName ) );
        OCGLE( glBindBuffer( target, bufferName ) );

        if( mArbBufferStorage )
        {
            OCGLE( glBufferStorage( target, sizeBytes, 0,
                                    forUpload ? GL_MAP_WRITE_BIT : GL_MAP_READ_BIT ) );
        }
        else
        {
            OCGLE( glBufferData( target, sizeBytes, 0, forUpload ? GL_STREAM_DRAW : GL_STREAM_READ ) );
        }

        GL3PlusStagingBuffer *stagingBuffer = OGRE_NEW GL3PlusStagingBuffer( 0, sizeBytes, this,
                                                                             forUpload, bufferName );
        mStagingBuffers[forUpload].push_back( stagingBuffer );

        return stagingBuffer;
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusVaoManager::update(void)
    {
        unsigned long currentTimeMs = mTimer->getMilliseconds();

        FastArray<GLuint> bufferNames;

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
                        bufferNames.push_back( static_cast<GL3PlusStagingBuffer*>(
                                                    stagingBuffer)->getBufferName() );

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

        if( !bufferNames.empty() )
        {
            glDeleteBuffers( bufferNames.size(), &bufferNames[0] );
            bufferNames.clear();
        }
    }
    //-----------------------------------------------------------------------------------
    /*IndexBufferPacked* GL3PlusVaoManager::createIndexBufferImpl( IndexBufferPacked::IndexType indexType,
                                                      size_t numIndices, BufferType bufferType,
                                                      void *initialData, bool keepAsShadow )
    {
        return createIndexBufferImpl( 0, numIndices, indexType == IndexBufferPacked::IT_16BIT ? 2 : 4,
                                      bufferType, 0, initialData, keepAsShadow );
    }*/
}

