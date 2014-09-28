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
#include "Vao/OgreGL3PlusConstBufferPacked.h"
#include "Vao/OgreGL3PlusTexBufferPacked.h"
#include "Vao/OgreGL3PlusMultiSourceVertexBufferPool.h"

#include "Vao/OgreIndirectBufferPacked.h"

#include "OgreGL3PlusHardwareBufferManager.h" //GL3PlusHardwareBufferManager::getGLType

#include "OgreTimer.h"

namespace Ogre
{
    static const GLuint64 kOneSecondInNanoSeconds = 1000000000;

    GL3PlusVaoManager::GL3PlusVaoManager( bool supportsArbBufferStorage, bool supportsIndirectBuffers ) :
        mArbBufferStorage( supportsArbBufferStorage ),
        mDrawId( 0 )
    {
        //Keep pools of 128MB each for static meshes
        mDefaultPoolSize[CPU_INACCESSIBLE]  = 128 * 1024 * 1024;
        //Keep pools of 32MB each for dynamic vertex buffers
        mDefaultPoolSize[CPU_ACCESSIBLE]    = 32 * 1024 * 1024;

        mFrameSyncVec.resize( mDynamicBufferMultiplier, 0 );

        GLint alignment;
        glGetIntegerv( GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignment );
        mConstBufferAlignment = alignment;
        glGetIntegerv( GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT, &alignment );
        mTexBufferAlignment = alignment;

        mSupportsIndirectBuffers = supportsIndirectBuffers;

        mVertexAttributeIndex[VES_POSITION - 1]             = 0;
        mVertexAttributeIndex[VES_NORMAL - 1]               = 1;
        mVertexAttributeIndex[VES_TANGENT - 1]              = 2;
        mVertexAttributeIndex[VES_BLEND_WEIGHTS - 1]        = 3;
        mVertexAttributeIndex[VES_BLEND_INDICES - 1]        = 4;
        mVertexAttributeIndex[VES_DIFFUSE - 1]              = 5;
        mVertexAttributeIndex[VES_SPECULAR - 1]             = 6;
        mVertexAttributeIndex[VES_TEXTURE_COORDINATES - 1]  = 7;
        //There are up to 8 VES_TEXTURE_COORDINATES. Occupy range [7; 15)
        //Index 15 is reserved for draw ID.

        //VES_BINORMAL uses slot 16. Lots of GPUs don't support more than 16 attributes
        //(even very modern ones like the GeForce 680). Since Binormal is rarely used, it
        //is technical (not artist controlled, unlike UVs) and can be replaced by a
        //4-component VES_TANGENT, we leave this one for the end.
        mVertexAttributeIndex[VES_BINORMAL - 1]             = 16;

        VertexElement2Vec vertexElements;
        vertexElements.push_back( VertexElement2( VET_UINT1, VES_COUNT ) );
        uint32 *drawIdPtr = static_cast<uint32*>( OGRE_MALLOC_SIMD( 4096 * sizeof(uint32),
                                                                    MEMCATEGORY_GEOMETRY ) );
        for( uint32 i=0; i<4096; ++i )
            drawIdPtr[i] = i;
        mDrawId = createVertexBuffer( vertexElements, 4096, BT_IMMUTABLE, drawIdPtr, true );
    }
    //-----------------------------------------------------------------------------------
    GL3PlusVaoManager::~GL3PlusVaoManager()
    {
        destroyAllVertexArrayObjects();
        VaoManager::deleteAllBuffers( mVertexBuffers );
        VaoManager::deleteAllBuffers( mIndexBuffers );

        vector<GLuint>::type bufferNames;

        bufferNames.reserve( mStagingBuffers[0].size() + mStagingBuffers[1].size() );

        for( size_t i=0; i<2; ++i )
        {
            //Collect the buffer names from all staging buffers to use one API call
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
            //Keep collecting the buffer names from all VBOs to use one API call
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
            OCGE( glDeleteBuffers( bufferNames.size(), &bufferNames[0] ) );
            bufferNames.clear();
        }

        GLSyncVec::const_iterator itor = mFrameSyncVec.begin();
        GLSyncVec::const_iterator end  = mFrameSyncVec.end();

        while( itor != end )
        {
            OCGE( glDeleteSync( *itor ) );
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusVaoManager::allocateVbo( size_t sizeBytes, size_t alignment, BufferType bufferType,
                                         size_t &outVboIdx, size_t &outBufferOffset )
    {
        assert( alignment > 0 );

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

            //TODO: Deal with Out of memory errors
            //No luck, allocate a new buffer.
            OCGE( glGenBuffers( 1, &newVbo.vboName ) );
            OCGE( glBindBuffer( GL_ARRAY_BUFFER, newVbo.vboName ) );

            if( mArbBufferStorage )
            {
                if( vboFlag == CPU_ACCESSIBLE )
                {
                    OCGE( glBufferStorage( GL_ARRAY_BUFFER, poolSize, 0,
                                            GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT |
                                            GL_MAP_COHERENT_BIT ) );
                }
                else
                {
                    OCGE( glBufferStorage( GL_ARRAY_BUFFER, poolSize, 0, 0 ) );
                }
            }
            else
            {
                OCGE( glBufferData( GL_ARRAY_BUFFER, mDefaultPoolSize[vboFlag], 0,
                                     vboFlag == CPU_INACCESSIBLE ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW ) );
            }
            OCGE( glBindBuffer( GL_ARRAY_BUFFER, 0 ) );

            newVbo.sizeBytes = poolSize;
            newVbo.freeBlocks.push_back( Block( 0, poolSize ) );

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
            mVbos[vboFlag].erase( mVbos[vboFlag].begin() + bestVboIdx );

        outVboIdx       = bestVboIdx;
        outBufferOffset = newOffset;
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusVaoManager::deallocateVbo( size_t vboIdx, size_t bufferOffset, size_t sizeBytes,
                                           BufferType bufferType )
    {
        VboFlag vboFlag = bufferType == BT_DYNAMIC ? CPU_ACCESSIBLE: CPU_INACCESSIBLE;

        if( bufferType == BT_DYNAMIC )
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

        VboFlag vboFlag = CPU_INACCESSIBLE;

        if( bufferType == BT_DYNAMIC )
            vboFlag = CPU_ACCESSIBLE;

        GL3PlusBufferInterface *bufferInterface = new GL3PlusBufferInterface( 0,
                                                                    GL_ARRAY_BUFFER,
                                                                    mVbos[vboFlag][vboIdx].vboName );
        VertexBufferPacked *retVal = OGRE_NEW VertexBufferPacked(
                                                        bufferOffset, numElements, bytesPerElement,
                                                        bufferType, initialData, keepAsShadow,
                                                        this, bufferInterface, vElements, 0, 0, 0 );

        if( initialData )
            bufferInterface->_firstUpload( initialData, 0, numElements );

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusVaoManager::destroyVertexBufferImpl( VertexBufferPacked *vertexBuffer )
    {
        GL3PlusBufferInterface *bufferInterface = static_cast<GL3PlusBufferInterface*>(
                                                        vertexBuffer->getBufferInterface() );


        deallocateVbo( bufferInterface->getVboPoolIndex(), vertexBuffer->_getInternalBufferStart(),
                       vertexBuffer->getNumElements() * vertexBuffer->getBytesPerElement(),
                       vertexBuffer->getBufferType() );
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
    IndexBufferPacked* GL3PlusVaoManager::createIndexBufferImpl( size_t numElements,
                                                                 uint32 bytesPerElement,
                                                                 BufferType bufferType,
                                                                 void *initialData, bool keepAsShadow )
    {
        size_t vboIdx;
        size_t bufferOffset;

        allocateVbo( numElements * bytesPerElement, bytesPerElement, bufferType, vboIdx, bufferOffset );

        VboFlag vboFlag = CPU_INACCESSIBLE;

        if( bufferType == BT_DYNAMIC )
            vboFlag = CPU_ACCESSIBLE;

        GL3PlusBufferInterface *bufferInterface = new GL3PlusBufferInterface( 0,
                                                                    GL_ARRAY_BUFFER,
                                                                    mVbos[vboFlag][vboIdx].vboName );
        IndexBufferPacked *retVal = OGRE_NEW IndexBufferPacked(
                                                        bufferOffset, numElements, bytesPerElement,
                                                        bufferType, initialData, keepAsShadow,
                                                        this, bufferInterface );

        if( initialData )
            bufferInterface->_firstUpload( initialData, 0, numElements );

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusVaoManager::destroyIndexBufferImpl( IndexBufferPacked *indexBuffer )
    {
        GL3PlusBufferInterface *bufferInterface = static_cast<GL3PlusBufferInterface*>(
                                                        indexBuffer->getBufferInterface() );


        deallocateVbo( bufferInterface->getVboPoolIndex(), indexBuffer->_getInternalBufferStart(),
                       indexBuffer->getNumElements() * indexBuffer->getBytesPerElement(),
                       indexBuffer->getBufferType() );
    }
    //-----------------------------------------------------------------------------------
    ConstBufferPacked* GL3PlusVaoManager::createConstBufferImpl( size_t sizeBytes, BufferType bufferType,
                                                                 void *initialData, bool keepAsShadow )
    {
        size_t vboIdx;
        size_t bufferOffset;

        GLint alignment = mConstBufferAlignment;

        size_t bindableSize = sizeBytes;

        VboFlag vboFlag = CPU_INACCESSIBLE;

        if( bufferType == BT_DYNAMIC )
        {
            vboFlag = CPU_ACCESSIBLE;

            //For dynamic buffers, the size will be 3x times larger
            //(depending on mDynamicBufferMultiplier); we need the
            //offset after each map to be aligned; and for that, we
            //sizeBytes to be multiple of alignment.
            sizeBytes = ( (sizeBytes + alignment - 1) / alignment ) * alignment;
        }

        allocateVbo( sizeBytes, alignment, bufferType, vboIdx, bufferOffset );

        GL3PlusBufferInterface *bufferInterface = new GL3PlusBufferInterface( 0,
                                                                    GL_UNIFORM_BUFFER,
                                                                    mVbos[vboFlag][vboIdx].vboName );
        ConstBufferPacked *retVal = OGRE_NEW GL3PlusConstBufferPacked(
                                                        bufferOffset, sizeBytes, 1,
                                                        bufferType, initialData, keepAsShadow,
                                                        this, bufferInterface, bindableSize );

        if( initialData )
            bufferInterface->_firstUpload( initialData, 0, sizeBytes );

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusVaoManager::destroyConstBufferImpl( ConstBufferPacked *constBuffer )
    {
        GL3PlusBufferInterface *bufferInterface = static_cast<GL3PlusBufferInterface*>(
                                                        constBuffer->getBufferInterface() );


        deallocateVbo( bufferInterface->getVboPoolIndex(), constBuffer->_getInternalBufferStart(),
                       constBuffer->getNumElements() * constBuffer->getBytesPerElement(),
                       constBuffer->getBufferType() );
    }
    //-----------------------------------------------------------------------------------
    TexBufferPacked* GL3PlusVaoManager::createTexBufferImpl( PixelFormat pixelFormat, size_t sizeBytes,
                                                             BufferType bufferType,
                                                             void *initialData, bool keepAsShadow )
    {
        size_t vboIdx;
        size_t bufferOffset;

        GLint alignment = mTexBufferAlignment;

        VboFlag vboFlag = CPU_INACCESSIBLE;

        if( bufferType == BT_DYNAMIC )
        {
            vboFlag = CPU_ACCESSIBLE;

            //For dynamic buffers, the size will be 3x times larger
            //(depending on mDynamicBufferMultiplier); we need the
            //offset after each map to be aligned; and for that, we
            //sizeBytes to be multiple of alignment.
            sizeBytes = ( (sizeBytes + alignment - 1) / alignment ) * alignment;
        }

        allocateVbo( sizeBytes, alignment, bufferType, vboIdx, bufferOffset );

        GL3PlusBufferInterface *bufferInterface = new GL3PlusBufferInterface( 0,
                                                                    GL_TEXTURE_BUFFER,
                                                                    mVbos[vboFlag][vboIdx].vboName );
        TexBufferPacked *retVal = OGRE_NEW GL3PlusTexBufferPacked(
                                                        bufferOffset, sizeBytes, 1,
                                                        bufferType, initialData, keepAsShadow,
                                                        this, bufferInterface, pixelFormat );

        if( initialData )
            bufferInterface->_firstUpload( initialData, 0, sizeBytes );

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusVaoManager::destroyTexBufferImpl( TexBufferPacked *texBuffer )
    {
        GL3PlusBufferInterface *bufferInterface = static_cast<GL3PlusBufferInterface*>(
                                                        texBuffer->getBufferInterface() );


        deallocateVbo( bufferInterface->getVboPoolIndex(), texBuffer->_getInternalBufferStart(),
                       texBuffer->getNumElements() * texBuffer->getBytesPerElement(),
                       texBuffer->getBufferType() );
    }
    //-----------------------------------------------------------------------------------
    IndirectBufferPacked* GL3PlusVaoManager::createIndirectBufferImpl( size_t sizeBytes,
                                                                       BufferType bufferType,
                                                                       void *initialData,
                                                                       bool keepAsShadow )
    {
        size_t vboIdx;
        size_t bufferOffset;

        size_t alignment = 4;

        VboFlag vboFlag = CPU_INACCESSIBLE;

        if( bufferType == BT_DYNAMIC )
        {
            vboFlag = CPU_ACCESSIBLE;

            //For dynamic buffers, the size will be 3x times larger
            //(depending on mDynamicBufferMultiplier); we need the
            //offset after each map to be aligned; and for that, we
            //sizeBytes to be multiple of alignment.
            sizeBytes = ( (sizeBytes + alignment - 1) / alignment ) * alignment;
        }

        allocateVbo( sizeBytes, alignment, bufferType, vboIdx, bufferOffset );

        GL3PlusBufferInterface *bufferInterface = new GL3PlusBufferInterface( 0,
                                                                    GL_DRAW_INDIRECT_BUFFER,
                                                                    mVbos[vboFlag][vboIdx].vboName );
        IndirectBufferPacked *retVal = OGRE_NEW IndirectBufferPacked(
                                                        bufferOffset, sizeBytes, 1,
                                                        bufferType, initialData, keepAsShadow,
                                                        this, bufferInterface );

        if( initialData )
            bufferInterface->_firstUpload( initialData, 0, sizeBytes );

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusVaoManager::destroyIndirectBufferImpl( IndirectBufferPacked *indirectBuffer )
    {
        GL3PlusBufferInterface *bufferInterface = static_cast<GL3PlusBufferInterface*>(
                                                        indirectBuffer->getBufferInterface() );


        deallocateVbo( bufferInterface->getVboPoolIndex(), indirectBuffer->_getInternalBufferStart(),
                       indirectBuffer->getNumElements() * indirectBuffer->getBytesPerElement(),
                       indirectBuffer->getBufferType() );
    }
    //-----------------------------------------------------------------------------------
    GLuint GL3PlusVaoManager::createVao( const Vao &vaoRef )
    {
        GLuint vaoName;
        OCGE( glGenVertexArrays( 1, &vaoName ) );
        OCGE( glBindVertexArray( vaoName ) );

        GLuint uvCount = 0;

        for( size_t i=0; i<vaoRef.vertexBuffers.size(); ++i )
        {
            const Vao::VertexBinding &binding = vaoRef.vertexBuffers[i];

            glBindBuffer( GL_ARRAY_BUFFER, binding.vertexBufferVbo );

            VertexElement2Vec::const_iterator it = binding.vertexElements.begin();
            VertexElement2Vec::const_iterator en = binding.vertexElements.end();

            while( it != en )
            {
                GLint typeCount = v1::VertexElement::getTypeCount( it->mType );
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

                GLuint attributeIndex = mVertexAttributeIndex[it->mSemantic] + uvCount;

                if( it->mSemantic == VES_TEXTURE_COORDINATES )
                {
                    assert( uvCount < 8 && "Up to 8 UVs are supported." );
                    attributeIndex += uvCount;
                    ++uvCount;
                }

                if( it->mSemantic == VES_BINORMAL )
                {
                    LogManager::getSingleton().logMessage(
                                "WARNING: VES_BINORMAL will not render properly in "
                                "many GPUs where GL_MAX_VERTEX_ATTRIBS = 16. Consider"
                                " changing for VES_TANGENT with 4 components or use"
                                " QTangents", LML_CRITICAL );
                }

                switch( v1::VertexElement::getBaseType( it->mType ) )
                {
                default:
                case VET_FLOAT1:
                    OCGE( glVertexAttribPointer( attributeIndex, typeCount,
                                                 v1::GL3PlusHardwareBufferManager::getGLType( it->mType ),
                                                 normalised, binding.stride, (void*)binding.offset ) );
                    break;
                case VET_DOUBLE1:
                    OCGE( glVertexAttribLPointer( attributeIndex, typeCount,
                                                  v1::GL3PlusHardwareBufferManager::getGLType( it->mType ),
                                                  binding.stride, (void*)binding.offset ) );
                    break;
                }

                OCGE( glVertexAttribDivisor( attributeIndex, binding.instancingDivisor ) );
                OCGE( glEnableVertexAttribArray( attributeIndex ) );

                ++attributeIndex;
                ++it;
            }

            OCGE( glBindBuffer( GL_ARRAY_BUFFER, 0 ) );
        }

        {
            //Now bind the Draw ID.
            GL3PlusBufferInterface *drawIdBufferInterface = static_cast<GL3PlusBufferInterface*>(
                        mDrawId->getBufferInterface() );
            const GLuint drawIdIdx = 15;
            OCGE( glBindBuffer( GL_ARRAY_BUFFER, drawIdBufferInterface->getVboName() ) );
            OCGE( glVertexAttribPointer( drawIdIdx, 1, GL_UNSIGNED_INT, GL_FALSE,
                                         sizeof(uint32), (void*)mDrawId->_getFinalBufferStart() ) );
            OCGE( glVertexAttribDivisor( drawIdIdx, 1 ) );
            OCGE( glEnableVertexAttribArray( drawIdIdx ) );
            OCGE( glBindBuffer( GL_ARRAY_BUFFER, 0 ) );
        }

        if( vaoRef.indexBufferVbo )
            glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vaoRef.indexBufferVbo );

        OCGE( glBindVertexArray( 0 ) );

        return vaoName;
    }
    //-----------------------------------------------------------------------------------
    VertexArrayObject* GL3PlusVaoManager::createVertexArrayObjectImpl(
                                                            const VertexBufferPackedVec &vertexBuffers,
                                                            IndexBufferPacked *indexBuffer,
                                                            v1::RenderOperation::OperationType opType )
    {
        Vao vao;

        vao.vertexBuffers.reserve( vertexBuffers.size() );

        {
            VertexBufferPackedVec::const_iterator itor = vertexBuffers.begin();
            VertexBufferPackedVec::const_iterator end  = vertexBuffers.end();

            while( itor != end )
            {
                Vao::VertexBinding vertexBinding;
                vertexBinding.vertexBufferVbo   = static_cast<GL3PlusBufferInterface*>(
                                                        (*itor)->getBufferInterface() )->getVboName();
                vertexBinding.vertexElements    = (*itor)->getVertexElements();
                vertexBinding.stride            = calculateVertexSize( vertexBinding.vertexElements );
                vertexBinding.offset            = 0;
                vertexBinding.instancingDivisor = 0;

                const MultiSourceVertexBufferPool *multiSourcePool = (*itor)->getMultiSourcePool();
                if( multiSourcePool )
                {
                    vertexBinding.offset = multiSourcePool->getBytesOffsetToSource(
                                                            (*itor)->_getSourceIndex() );
                }

                vao.vertexBuffers.push_back( vertexBinding );

                ++itor;
            }
        }

        vao.refCount = 0;

        if( indexBuffer )
        {
            vao.indexBufferVbo  = static_cast<GL3PlusBufferInterface*>(
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
            if( itor->indexBufferVbo == vao.indexBufferVbo &&
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
            mVaos.push_back( vao );
            itor = mVaos.begin() + mVaos.size() - 1;
        }

        GL3PlusVertexArrayObject *retVal = OGRE_NEW GL3PlusVertexArrayObject( itor->vaoName,
                                                                              vertexBuffers,
                                                                              indexBuffer,
                                                                              opType );

        ++itor->refCount;

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusVaoManager::destroyVertexArrayObjectImpl( VertexArrayObject *vao )
    {
        GL3PlusVertexArrayObject *glVao = static_cast<GL3PlusVertexArrayObject*>( vao );

        VaoVec::iterator itor = mVaos.begin();
        VaoVec::iterator end  = mVaos.end();

        while( itor != end && itor->vaoName != glVao->mVaoName )
            ++itor;

        if( itor != end )
        {
            --itor->refCount;

            if( !itor->refCount )
            {
                OCGE( glDeleteVertexArrays( 1, &glVao->mVaoName ) );
            }
        }

        OGRE_DELETE glVao;
    }
    //-----------------------------------------------------------------------------------
    StagingBuffer* GL3PlusVaoManager::createStagingBuffer( size_t sizeBytes, bool forUpload )
    {
        GLuint bufferName;
        GLenum target = forUpload ? GL_COPY_READ_BUFFER : GL_COPY_WRITE_BUFFER;
        OCGE( glGenBuffers( 1, &bufferName ) );
        OCGE( glBindBuffer( target, bufferName ) );

        if( mArbBufferStorage )
        {
            OCGE( glBufferStorage( target, sizeBytes, 0,
                                    forUpload ? GL_MAP_WRITE_BIT : GL_MAP_READ_BIT ) );
        }
        else
        {
            OCGE( glBufferData( target, sizeBytes, 0, forUpload ? GL_STREAM_DRAW : GL_STREAM_READ ) );
        }

        GL3PlusStagingBuffer *stagingBuffer = OGRE_NEW GL3PlusStagingBuffer( 0, sizeBytes, this,
                                                                             forUpload, bufferName );
        mStagingBuffers[forUpload].push_back( stagingBuffer );

        return stagingBuffer;
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusVaoManager::_update(void)
    {
        VaoManager::_update();

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
            OCGE( glDeleteBuffers( bufferNames.size(), &bufferNames[0] ) );
            bufferNames.clear();
        }

        if( mFrameSyncVec[mDynamicBufferCurrentFrame] )
        {
            OCGE( glDeleteSync( mFrameSyncVec[mDynamicBufferCurrentFrame] ) );
        }
        OCGE( mFrameSyncVec[mDynamicBufferCurrentFrame] = glFenceSync( GL_SYNC_GPU_COMMANDS_COMPLETE, 0 ) );
        mDynamicBufferCurrentFrame = (mDynamicBufferCurrentFrame + 1) % mDynamicBufferMultiplier;
    }
    //-----------------------------------------------------------------------------------
    uint8 GL3PlusVaoManager::waitForTailFrameToFinish(void)
    {
        if( mFrameSyncVec[mDynamicBufferCurrentFrame] )
        {
            GLbitfield waitFlags    = 0;
            GLuint64 waitDuration   = 0;
            while( true )
            {
                GLenum waitRet = glClientWaitSync( mFrameSyncVec[mDynamicBufferCurrentFrame],
                                                   waitFlags, waitDuration );
                if( waitRet == GL_ALREADY_SIGNALED || waitRet == GL_CONDITION_SATISFIED )
                {
                    OCGE( glDeleteSync( mFrameSyncVec[mDynamicBufferCurrentFrame] ) );
                    mFrameSyncVec[mDynamicBufferCurrentFrame] = 0;

                    return mDynamicBufferCurrentFrame;
                }

                if( waitRet == GL_WAIT_FAILED )
                {
                    OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                                 "Failure while waiting for a GL Fence. Could be out of GPU memory. "
                                 "Update your video card drivers. If that doesn't help, "
                                 "contact the developers.",
                                 "GL3PlusVaoManager::getDynamicBufferCurrentFrame" );
                }

                // After the first time, need to start flushing, and wait for a looong time.
                waitFlags = GL_SYNC_FLUSH_COMMANDS_BIT;
                waitDuration = kOneSecondInNanoSeconds;
            }
        }

        return mDynamicBufferCurrentFrame;
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

