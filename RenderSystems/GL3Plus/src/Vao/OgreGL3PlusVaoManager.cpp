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
#include "Vao/OgreGL3PlusTexBufferEmulatedPacked.h"
#include "Vao/OgreGL3PlusTexBufferPacked.h"
#include "Vao/OgreGL3PlusUavBufferPacked.h"
#include "Vao/OgreGL3PlusMultiSourceVertexBufferPool.h"
#include "Vao/OgreGL3PlusDynamicBuffer.h"
#include "Vao/OgreGL3PlusAsyncTicket.h"

#include "Vao/OgreIndirectBufferPacked.h"

#include "OgreRenderQueue.h"

#include "OgreGL3PlusHardwareBufferManager.h" //GL3PlusHardwareBufferManager::getGLType

#include "OgreTimer.h"
#include "OgreStringConverter.h"


namespace Ogre
{
    extern const GLuint64 kOneSecondInNanoSeconds;
    const GLuint64 kOneSecondInNanoSeconds = 1000000000;

    const GLuint GL3PlusVaoManager::VERTEX_ATTRIBUTE_INDEX[VES_COUNT] =
    {
        0,  // VES_POSITION - 1
        3,  // VES_BLEND_WEIGHTS - 1
        4,  // VES_BLEND_INDICES - 1
        1,  // VES_NORMAL - 1
        5,  // VES_DIFFUSE - 1
        6,  // VES_SPECULAR - 1
        7,  // VES_TEXTURE_COORDINATES - 1
        //There are up to 8 VES_TEXTURE_COORDINATES. Occupy range [7; 15)
        //Range [13; 15) overlaps with VES_BLEND_WEIGHTS2 & VES_BLEND_INDICES2
        //Index 15 is reserved for draw ID.

        //VES_BINORMAL uses slot 16. Lots of GPUs don't support more than 16 attributes
        //(even very modern ones like the GeForce 680). Since Binormal is rarely used, it
        //is technical (not artist controlled, unlike UVs) and can be replaced by a
        //4-component VES_TANGENT, we leave this one for the end.
        16, // VES_BINORMAL - 1
        2,  // VES_TANGENT - 1
        13, // VES_BLEND_WEIGHTS2 - 1
        14, // VES_BLEND_INDICES2 - 1
    };

    GL3PlusVaoManager::GL3PlusVaoManager( bool _supportsArbBufferStorage,
                                          bool emulateTexBuffers,
                                          bool _supportsIndirectBuffers,
                                          bool _supportsBaseInstance,
                                          bool _supportsSsbo ) :
        mArbBufferStorage( _supportsArbBufferStorage ),
        mEmulateTexBuffers( emulateTexBuffers ),
        mMaxVertexAttribs( 30 ),
        mDrawId( 0 )
    {
        //Keep pools of 128MB each for static meshes
        mDefaultPoolSize[CPU_INACCESSIBLE]  = 128 * 1024 * 1024;

        //Keep pools of 32MB each for dynamic vertex buffers
        for( size_t i=CPU_ACCESSIBLE_DEFAULT; i<=CPU_ACCESSIBLE_PERSISTENT_COHERENT; ++i )
            mDefaultPoolSize[i] = 32 * 1024 * 1024;

        mFrameSyncVec.resize( mDynamicBufferMultiplier, 0 );

        OCGE( glGetIntegerv( GLenum(GL_MAX_VERTEX_ATTRIBS), &mMaxVertexAttribs ) );

        if( mMaxVertexAttribs < 16 )
        {
            OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                         "GL_MAX_VERTEX_ATTRIBS = " + StringConverter::toString( mMaxVertexAttribs ) +
                         " this value must be >= 16 for Ogre to function "
                         "properly. Try updating your video card drivers.",
                         "GL3PlusVaoManager::GL3PlusVaoManager" );
        }

        //The minimum alignment for these buffers is 16 because some
        //places of Ogre assume such alignment for SIMD reasons.
        GLint alignment = 1; //initial value according to specs
        OCGE( glGetIntegerv( GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignment ) );
        mConstBufferAlignment = std::max<uint32>( alignment, 16u );
        mTexBufferAlignment = 16;
        if( !emulateTexBuffers )
        {
            alignment = 1; //initial value according to specs
            OCGE( glGetIntegerv( GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT, &alignment ) );
            mTexBufferAlignment = std::max<uint32>( alignment, 16u );
        }
        if( _supportsSsbo )
        {
            alignment = 1; //initial value according to specs
            OCGE( glGetIntegerv( GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &alignment ) );
            mUavBufferAlignment = std::max<uint32>( alignment, 16u );
        }

        GLint maxBufferSize = 16384; //minimum value according to specs
        OCGE( glGetIntegerv( GL_MAX_UNIFORM_BLOCK_SIZE, &maxBufferSize ) );
        mConstBufferMaxSize = static_cast<size_t>( maxBufferSize );
        maxBufferSize = 65536; //minimum value according to specs
        OCGE( glGetIntegerv( GL_MAX_TEXTURE_BUFFER_SIZE, &maxBufferSize ) );
        mTexBufferMaxSize = static_cast<size_t>( maxBufferSize );
        if( _supportsSsbo )
        {
            OCGE( glGetIntegerv( GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &maxBufferSize ) );
            mUavBufferMaxSize = static_cast<size_t>( maxBufferSize );
        }

        mSupportsPersistentMapping  = mArbBufferStorage;
        mSupportsIndirectBuffers    = _supportsIndirectBuffers;
        mSupportsBaseInstance       = _supportsBaseInstance;

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
        deleteAllBuffers();

        vector<GLuint>::type bufferNames;

        bufferNames.reserve( mRefedStagingBuffers[0].size() + mRefedStagingBuffers[1].size() +
                             mZeroRefStagingBuffers[0].size() + mZeroRefStagingBuffers[1].size() );

        for( size_t i=0; i<2; ++i )
        {
            //Collect the buffer names from all staging buffers to use one API call
            StagingBufferVec::const_iterator itor = mRefedStagingBuffers[i].begin();
            StagingBufferVec::const_iterator end  = mRefedStagingBuffers[i].end();

            while( itor != end )
            {
                bufferNames.push_back( static_cast<GL3PlusStagingBuffer*>(*itor)->getBufferName() );
                ++itor;
            }

            itor = mZeroRefStagingBuffers[i].begin();
            end  = mZeroRefStagingBuffers[i].end();

            while( itor != end )
            {
                bufferNames.push_back( static_cast<GL3PlusStagingBuffer*>(*itor)->getBufferName() );
                ++itor;
            }
        }

        for( size_t i=0; i<MAX_VBO_FLAG; ++i )
        {
            //Free pointers and collect the buffer names from all VBOs to use one API call
            VboVec::iterator itor = mVbos[i].begin();
            VboVec::iterator end  = mVbos[i].end();

            while( itor != end )
            {
                bufferNames.push_back( itor->vboName );
                delete itor->dynamicBuffer;
                itor->dynamicBuffer = 0;
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
                size_t padding = newOffset - block.offset;

                if( sizeBytes + padding <= block.size )
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
            OCGE( glGenBuffers( 1, &newVbo.vboName ) );
            OCGE( glBindBuffer( GL_ARRAY_BUFFER, newVbo.vboName ) );

            GLenum error = 0;
            int trustCounter = 1000;
            //Reset the error code. Trust counter prevents an infinite loop
            //just in case we encounter a moronic GL implementation.
            while( glGetError() && trustCounter-- );

            if( mArbBufferStorage )
            {
                GLbitfield flags = 0;

                if( vboFlag >= CPU_ACCESSIBLE_DEFAULT )
                {
                    flags |= GL_MAP_WRITE_BIT;

                    if( vboFlag >= CPU_ACCESSIBLE_PERSISTENT )
                    {
                        flags |= GL_MAP_PERSISTENT_BIT;

                        if( vboFlag >= CPU_ACCESSIBLE_PERSISTENT_COHERENT )
                            flags |= GL_MAP_COHERENT_BIT;
                    }
                }

                glBufferStorage( GL_ARRAY_BUFFER, poolSize, 0, flags );

                error = glGetError();
            }
            else
            {
                glBufferData( GL_ARRAY_BUFFER, poolSize, 0,
                              vboFlag == CPU_INACCESSIBLE ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW );

                error = glGetError();
            }

            //OpenGL can't continue after any GL_OUT_OF_MEMORY has been raised,
            //thus ignore the trustCounter in that case.
            if( (error != 0 && trustCounter != 0) || error == GL_OUT_OF_MEMORY )
            {
                OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                             "Out of GPU memory or driver refused.\n"
                             "glGetError code: " + StringConverter::toString( error ) + ".\n"
                             "Requested: " + StringConverter::toString( poolSize ) + " bytes.",
                             "GL3PlusVaoManager::allocateVbo" );
            }

            OCGE( glBindBuffer( GL_ARRAY_BUFFER, 0 ) );

            newVbo.sizeBytes = poolSize;
            newVbo.freeBlocks.push_back( Block( 0, poolSize ) );
            newVbo.dynamicBuffer = 0;

            if( vboFlag != CPU_INACCESSIBLE )
            {
                newVbo.dynamicBuffer = new GL3PlusDynamicBuffer( newVbo.vboName, newVbo.sizeBytes,
                                                                 this, bufferType );
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
    void GL3PlusVaoManager::deallocateVbo( size_t vboIdx, size_t bufferOffset, size_t sizeBytes,
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
    VertexBufferPacked* GL3PlusVaoManager::createVertexBufferImpl( size_t numElements,
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
        GL3PlusBufferInterface *bufferInterface = new GL3PlusBufferInterface( vboIdx, vbo.vboName,
                                                                              vbo.dynamicBuffer );
        VertexBufferPacked *retVal = OGRE_NEW VertexBufferPacked(
                                                        bufferOffset, numElements, bytesPerElement, 0,
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


        deallocateVbo( bufferInterface->getVboPoolIndex(),
                       vertexBuffer->_getInternalBufferStart() * vertexBuffer->getBytesPerElement(),
                       vertexBuffer->_getInternalTotalSizeBytes(),
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

        VboFlag vboFlag = bufferTypeToVboFlag( bufferType );

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

        VboFlag vboFlag = bufferTypeToVboFlag( bufferType );

        Vbo &vbo = mVbos[vboFlag][vboIdx];
        GL3PlusBufferInterface *bufferInterface = new GL3PlusBufferInterface( vboIdx, vbo.vboName,
                                                                              vbo.dynamicBuffer );
        IndexBufferPacked *retVal = OGRE_NEW IndexBufferPacked(
                                                        bufferOffset, numElements, bytesPerElement, 0,
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


        deallocateVbo( bufferInterface->getVboPoolIndex(),
                       indexBuffer->_getInternalBufferStart() * indexBuffer->getBytesPerElement(),
                       indexBuffer->_getInternalTotalSizeBytes(),
                       indexBuffer->getBufferType() );
    }
    //-----------------------------------------------------------------------------------
    ConstBufferPacked* GL3PlusVaoManager::createConstBufferImpl( size_t sizeBytes, BufferType bufferType,
                                                                 void *initialData, bool keepAsShadow )
    {
        size_t vboIdx;
        size_t bufferOffset;

        GLint alignment = mConstBufferAlignment;
        size_t requestedSize = sizeBytes;

        VboFlag vboFlag = bufferTypeToVboFlag( bufferType );

        if( bufferType >= BT_DYNAMIC_DEFAULT )
        {
            //For dynamic buffers, the size will be 3x times larger
            //(depending on mDynamicBufferMultiplier); we need the
            //offset after each map to be aligned; and for that, we
            //sizeBytes to be multiple of alignment.
            sizeBytes = alignToNextMultiple( sizeBytes, alignment );
        }

        allocateVbo( sizeBytes, alignment, bufferType, vboIdx, bufferOffset );

        Vbo &vbo = mVbos[vboFlag][vboIdx];
        GL3PlusBufferInterface *bufferInterface = new GL3PlusBufferInterface( vboIdx, vbo.vboName,
                                                                              vbo.dynamicBuffer );
        ConstBufferPacked *retVal = OGRE_NEW GL3PlusConstBufferPacked(
                                                        bufferOffset, requestedSize, 1,
                                                        (sizeBytes - requestedSize) / 1,
                                                        bufferType, initialData, keepAsShadow,
                                                        this, bufferInterface );

        if( initialData )
            bufferInterface->_firstUpload( initialData, 0, requestedSize );

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusVaoManager::destroyConstBufferImpl( ConstBufferPacked *constBuffer )
    {
        GL3PlusBufferInterface *bufferInterface = static_cast<GL3PlusBufferInterface*>(
                                                        constBuffer->getBufferInterface() );


        deallocateVbo( bufferInterface->getVboPoolIndex(),
                       constBuffer->_getInternalBufferStart() * constBuffer->getBytesPerElement(),
                       constBuffer->_getInternalTotalSizeBytes(),
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
        size_t requestedSize = sizeBytes;

        VboFlag vboFlag = bufferTypeToVboFlag( bufferType );

        if( mEmulateTexBuffers )
        {
            // Align to the texture size since we must copy the PBO to a texture.
            ushort maxTexSizeBytes = 2048 * PixelUtil::getNumElemBytes( pixelFormat );
            // We need another line of maxTexSizeBytes for uploading
            //to create a rectangle when calling glTexSubImage2D().
            sizeBytes = alignToNextMultiple( sizeBytes, maxTexSizeBytes );
        }
        
        if( bufferType >= BT_DYNAMIC_DEFAULT )
        {
            //For dynamic buffers, the size will be 3x times larger
            //(depending on mDynamicBufferMultiplier); we need the
            //offset after each map to be aligned; and for that, we
            //sizeBytes to be multiple of alignment.
            sizeBytes = alignToNextMultiple( sizeBytes, alignment );
        }

        allocateVbo( sizeBytes, alignment, bufferType, vboIdx, bufferOffset );

        Vbo &vbo = mVbos[vboFlag][vboIdx];
        GL3PlusBufferInterface *bufferInterface = new GL3PlusBufferInterface( vboIdx, vbo.vboName,
                                                                              vbo.dynamicBuffer );
        TexBufferPacked *retVal;

        if( !mEmulateTexBuffers )
        {
            retVal = OGRE_NEW GL3PlusTexBufferPacked( bufferOffset, requestedSize, 1,
                                                      (sizeBytes - requestedSize) / 1,
                                                      bufferType, initialData, keepAsShadow,
                                                      this, bufferInterface, pixelFormat );
        }
        else
        {
            retVal = OGRE_NEW GL3PlusTexBufferEmulatedPacked( bufferOffset, requestedSize, 1,
                                                              (sizeBytes - requestedSize) / 1,
                                                              bufferType, initialData, keepAsShadow,
                                                              this, bufferInterface, pixelFormat );
        }

        if( initialData )
            bufferInterface->_firstUpload( initialData, 0, requestedSize );

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusVaoManager::destroyTexBufferImpl( TexBufferPacked *texBuffer )
    {
        GL3PlusBufferInterface *bufferInterface = static_cast<GL3PlusBufferInterface*>(
                                                        texBuffer->getBufferInterface() );


        deallocateVbo( bufferInterface->getVboPoolIndex(),
                       texBuffer->_getInternalBufferStart() * texBuffer->getBytesPerElement(),
                       texBuffer->_getInternalTotalSizeBytes(),
                       texBuffer->getBufferType() );
    }
    //-----------------------------------------------------------------------------------
    UavBufferPacked* GL3PlusVaoManager::createUavBufferImpl( size_t numElements, uint32 bytesPerElement,
                                                             uint32 bindFlags,
                                                             void *initialData, bool keepAsShadow )
    {
        size_t vboIdx;
        size_t bufferOffset;

        GLint alignment = mUavBufferAlignment;

        //UAV Buffers can't be dynamic.
        const BufferType bufferType = BT_DEFAULT;
        VboFlag vboFlag = bufferTypeToVboFlag( bufferType );

        allocateVbo( numElements * bytesPerElement, alignment, bufferType, vboIdx, bufferOffset );

        Vbo &vbo = mVbos[vboFlag][vboIdx];
        GL3PlusBufferInterface *bufferInterface = new GL3PlusBufferInterface( vboIdx, vbo.vboName,
                                                                              vbo.dynamicBuffer );
        UavBufferPacked *retVal = OGRE_NEW GL3PlusUavBufferPacked(
                                                        bufferOffset, numElements, bytesPerElement,
                                                        bindFlags, initialData, keepAsShadow,
                                                        this, bufferInterface );

        if( initialData )
            bufferInterface->_firstUpload( initialData, 0, numElements );

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusVaoManager::destroyUavBufferImpl( UavBufferPacked *uavBuffer )
    {
        GL3PlusBufferInterface *bufferInterface = static_cast<GL3PlusBufferInterface*>(
                                                        uavBuffer->getBufferInterface() );


        deallocateVbo( bufferInterface->getVboPoolIndex(),
                       uavBuffer->_getInternalBufferStart() * uavBuffer->getBytesPerElement(),
                       uavBuffer->_getInternalTotalSizeBytes(),
                       uavBuffer->getBufferType() );
    }
    //-----------------------------------------------------------------------------------
    IndirectBufferPacked* GL3PlusVaoManager::createIndirectBufferImpl( size_t sizeBytes,
                                                                       BufferType bufferType,
                                                                       void *initialData,
                                                                       bool keepAsShadow )
    {
        const size_t alignment = 4;
        size_t bufferOffset = 0;
        size_t requestedSize = sizeBytes;

        if( bufferType >= BT_DYNAMIC_DEFAULT )
        {
            //For dynamic buffers, the size will be 3x times larger
            //(depending on mDynamicBufferMultiplier); we need the
            //offset after each map to be aligned; and for that, we
            //sizeBytes to be multiple of alignment.
            sizeBytes = alignToNextMultiple( sizeBytes, alignment );
        }

        GL3PlusBufferInterface *bufferInterface = 0;
        if( mSupportsIndirectBuffers )
        {
            size_t vboIdx;
            VboFlag vboFlag = bufferTypeToVboFlag( bufferType );

            allocateVbo( sizeBytes, alignment, bufferType, vboIdx, bufferOffset );

            Vbo &vbo = mVbos[vboFlag][vboIdx];
            bufferInterface = new GL3PlusBufferInterface( vboIdx, vbo.vboName, vbo.dynamicBuffer );
        }

        IndirectBufferPacked *retVal = OGRE_NEW IndirectBufferPacked(
                                                        bufferOffset, requestedSize, 1,
                                                        (sizeBytes - requestedSize) / 1,
                                                        bufferType, initialData, keepAsShadow,
                                                        this, bufferInterface );

        if( initialData )
        {
            if( mSupportsIndirectBuffers )
            {
                bufferInterface->_firstUpload( initialData, 0, requestedSize );
            }
            else
            {
                memcpy( retVal->getSwBufferPtr(), initialData, requestedSize );
            }
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusVaoManager::destroyIndirectBufferImpl( IndirectBufferPacked *indirectBuffer )
    {
        if( mSupportsIndirectBuffers )
        {
            GL3PlusBufferInterface *bufferInterface = static_cast<GL3PlusBufferInterface*>(
                        indirectBuffer->getBufferInterface() );


            deallocateVbo( bufferInterface->getVboPoolIndex(),
                           indirectBuffer->_getInternalBufferStart() *
                                indirectBuffer->getBytesPerElement(),
                           indirectBuffer->_getInternalTotalSizeBytes(),
                           indirectBuffer->getBufferType() );
        }
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

            size_t bindAccumOffset = 0;

            while( it != en )
            {
                GLint typeCount = v1::VertexElement::getTypeCount( it->mType );
                GLboolean normalised = v1::VertexElement::isTypeNormalized( it->mType ) ? GL_TRUE :
                                                                                          GL_FALSE;

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

                GLuint attributeIndex = VERTEX_ATTRIBUTE_INDEX[it->mSemantic - 1];

                if( it->mSemantic == VES_TEXTURE_COORDINATES )
                {
                    assert( uvCount < 8 && "Up to 8 UVs are supported." );
                    attributeIndex += uvCount;
                    ++uvCount;
                }

                assert( ( uvCount < 6 || (it->mSemantic != VES_BLEND_WEIGHTS2 &&
                                          it->mSemantic != VES_BLEND_INDICES2) ) &&
                        "Available UVs get reduced from 8 to 6 when"
                        " VES_BLEND_WEIGHTS2/INDICES2 is present" );

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
                                                 normalised, binding.stride,
                                                 (void*)(binding.offset + bindAccumOffset) ) );
                    break;
                case VET_BYTE4:
                case VET_UBYTE4:
                case VET_SHORT2:
                case VET_USHORT2:
                case VET_UINT1:
                case VET_INT1:
                    OCGE( glVertexAttribIPointer( attributeIndex, typeCount,
                                                  v1::GL3PlusHardwareBufferManager::getGLType( it->mType ),
                                                  binding.stride,
                                                  (void*)(binding.offset + bindAccumOffset) ) );
                    break;
                case VET_DOUBLE1:
                    OCGE( glVertexAttribLPointer( attributeIndex, typeCount,
                                                  v1::GL3PlusHardwareBufferManager::getGLType( it->mType ),
                                                  binding.stride,
                                                  (void*)(binding.offset + bindAccumOffset) ) );
                    break;
                }

                OCGE( glVertexAttribDivisor( attributeIndex, binding.instancingDivisor ) );
                OCGE( glEnableVertexAttribArray( attributeIndex ) );

                bindAccumOffset += v1::VertexElement::getTypeSize( it->mType );

                ++it;
            }

            OCGE( glBindBuffer( GL_ARRAY_BUFFER, 0 ) );
        }

        {
            //Now bind the Draw ID.
            bindDrawId();
        }

        if( vaoRef.indexBufferVbo )
            glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vaoRef.indexBufferVbo );

        OCGE( glBindVertexArray( 0 ) );

        return vaoName;
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusVaoManager::bindDrawId(void)
    {
        GL3PlusBufferInterface *drawIdBufferInterface = static_cast<GL3PlusBufferInterface*>(
                                                                mDrawId->getBufferInterface() );
        const GLuint drawIdIdx = 15;
        OCGE( glBindBuffer( GL_ARRAY_BUFFER, drawIdBufferInterface->getVboName() ) );
        OCGE( glVertexAttribIPointer( drawIdIdx, 1, GL_UNSIGNED_INT, sizeof(uint32),
                                     (void*)(mDrawId->_getFinalBufferStart() *
                                             mDrawId->getBytesPerElement()) ) );
        OCGE( glVertexAttribDivisor( drawIdIdx, 1 ) );
        OCGE( glEnableVertexAttribArray( drawIdIdx ) );
        OCGE( glBindBuffer( GL_ARRAY_BUFFER, 0 ) );
    }
    //-----------------------------------------------------------------------------------
    VertexArrayObject* GL3PlusVaoManager::createVertexArrayObjectImpl(
                                                            const VertexBufferPackedVec &vertexBuffers,
                                                            IndexBufferPacked *indexBuffer,
                                                            OperationType opType )
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
            mVaos.push_back( vao );
            itor = mVaos.begin() + mVaos.size() - 1;
        }

        //Mix mNumGeneratedVaos with the GL Vao for better sorting purposes:
        //  If we only use the GL's vao, the RQ will sort Meshes with
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
                ( (itor->vaoName & maskVaoGl) << shiftVaoGl ) |
                (mNumGeneratedVaos & maskVao);

        GL3PlusVertexArrayObject *retVal = OGRE_NEW GL3PlusVertexArrayObject( itor->vaoName,
                                                                              renderQueueId,
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

        while( itor != end && itor->vaoName != glVao->getVaoName() )
            ++itor;

        if( itor != end )
        {
            --itor->refCount;

            if( !itor->refCount )
            {
				GLuint vaoName = glVao->getVaoName();
                OCGE( glDeleteVertexArrays( 1, &vaoName ) );

                efficientVectorRemove( mVaos, itor );
            }
        }

        //We delete it here because this class has no virtual destructor on purpose
        OGRE_DELETE glVao;
    }
    //-----------------------------------------------------------------------------------
    StagingBuffer* GL3PlusVaoManager::createStagingBuffer( size_t sizeBytes, bool forUpload )
    {
        sizeBytes = std::max<size_t>( sizeBytes, 4 * 1024 * 1024 );

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
        mRefedStagingBuffers[forUpload].push_back( stagingBuffer );

        if( mNextStagingBufferTimestampCheckpoint == (unsigned long)( ~0 ) )
            mNextStagingBufferTimestampCheckpoint = mTimer->getMilliseconds() + mDefaultStagingBufferLifetime;

        return stagingBuffer;
    }
    //-----------------------------------------------------------------------------------
    AsyncTicketPtr GL3PlusVaoManager::createAsyncTicket( BufferPacked *creator,
                                                         StagingBuffer *stagingBuffer,
                                                         size_t elementStart, size_t elementCount )
    {
        return AsyncTicketPtr( OGRE_NEW GL3PlusAsyncTicket( creator, stagingBuffer,
                                                            elementStart, elementCount ) );
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusVaoManager::_update(void)
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
                        stagingBuffer->getLastUsedTimestamp() + stagingBuffer->getLifetimeThreshold() );

                    if( stagingBuffer->getLastUsedTimestamp() + stagingBuffer->getUnfencedTimeThreshold() < currentTimeMs )
                    {
                        static_cast<GL3PlusStagingBuffer*>( stagingBuffer )->cleanUnfencedHazards();
                    }

                    if( stagingBuffer->getLastUsedTimestamp() + stagingBuffer->getLifetimeThreshold() < currentTimeMs )
                    {
                        //Time to delete this buffer.
                        bufferNames.push_back( static_cast<GL3PlusStagingBuffer*>(
                                                    stagingBuffer)->getBufferName() );

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

        if( !mDelayedDestroyBuffers.empty() &&
            mDelayedDestroyBuffers.front().frameNumDynamic == mDynamicBufferCurrentFrame )
        {
            waitForTailFrameToFinish();
            destroyDelayedBuffers( mDynamicBufferCurrentFrame );
        }

        VaoManager::_update();

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
            waitFor( mFrameSyncVec[mDynamicBufferCurrentFrame] );
            mFrameSyncVec[mDynamicBufferCurrentFrame] = 0;
        }

        return mDynamicBufferCurrentFrame;
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusVaoManager::waitForSpecificFrameToFinish( uint32 frameCount )
    {
        if( frameCount == mFrameCount )
        {
            //Full stall
            glFinish();

            //All of the other per-fences are not needed anymore.
            GLSyncVec::const_iterator itor = mFrameSyncVec.begin();
            GLSyncVec::const_iterator end  = mFrameSyncVec.end();

            while( itor != end )
            {
                if( *itor )
                    OCGE( glDeleteSync( *itor ) );
                ++itor;
            }
        }
        else if( mFrameCount - frameCount <= mDynamicBufferMultiplier )
        {
            //Let's wait on one of our existing fences...
            //frameDiff has to be in range [1; mDynamicBufferMultiplier]
            size_t frameDiff = mFrameCount - frameCount;
            const size_t idx = (mDynamicBufferCurrentFrame +
                                mDynamicBufferMultiplier - frameDiff) % mDynamicBufferMultiplier;
            if( mFrameSyncVec[idx] )
            {
                mFrameSyncVec[idx] = waitFor( mFrameSyncVec[idx] );

                //Delete all the fences until this frame we've just waited.
                size_t nextIdx = mDynamicBufferCurrentFrame;
                while( nextIdx != idx )
                {
                    if( mFrameSyncVec[nextIdx] )
                    {
                        OCGE( glDeleteSync( mFrameSyncVec[nextIdx] ) );
                        mFrameSyncVec[nextIdx] = 0;
                    }
                    nextIdx = (nextIdx + 1u) % mDynamicBufferMultiplier;
                }
            }
        }
        else
        {
            //No stall
        }
    }
    //-----------------------------------------------------------------------------------
    bool GL3PlusVaoManager::isFrameFinished( uint32 frameCount )
    {
        bool retVal = true;
        if( frameCount == mFrameCount )
        {
            //Full stall
            //retVal = true;
        }
        else if( mFrameCount - frameCount <= mDynamicBufferMultiplier )
        {
            //frameDiff has to be in range [1; mDynamicBufferMultiplier]
            size_t frameDiff = mFrameCount - frameCount;
            const size_t idx = (mDynamicBufferCurrentFrame +
                                mDynamicBufferMultiplier - frameDiff) % mDynamicBufferMultiplier;

            if( mFrameSyncVec[idx] )
            {
                //Ask GL API to return immediately and tells us about the fence
                GLenum waitRet = glClientWaitSync( mFrameSyncVec[idx], 0, 0 );
                if( waitRet == GL_ALREADY_SIGNALED || waitRet == GL_CONDITION_SATISFIED )
                {
                    //Delete all the fences until this frame we've just waited.
                    size_t nextIdx = mDynamicBufferCurrentFrame;
                    while( nextIdx != idx )
                    {
                        if( mFrameSyncVec[nextIdx] )
                        {
                            OCGE( glDeleteSync( mFrameSyncVec[nextIdx] ) );
                            mFrameSyncVec[nextIdx] = 0;
                        }
                        nextIdx = (nextIdx + 1u) % mDynamicBufferMultiplier;
                    }

                    retVal = false;
                }
            }
            else
            {
                retVal = false;
            }
        }
        else
        {
            //No stall
            retVal = false;
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    GLsync GL3PlusVaoManager::waitFor( GLsync fenceName )
    {
        GLbitfield waitFlags    = 0;
        GLuint64 waitDuration   = 0;
        while( true )
        {
            GLenum waitRet = glClientWaitSync( fenceName, waitFlags, waitDuration );
            if( waitRet == GL_ALREADY_SIGNALED || waitRet == GL_CONDITION_SATISFIED )
            {
                OCGE( glDeleteSync( fenceName ) );
                return 0;
            }

            if( waitRet == GL_WAIT_FAILED )
            {
                OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                             "Failure while waiting for a GL Fence. Could be out of GPU memory. "
                             "Update your video card drivers. If that doesn't help, "
                             "contact the developers.",
                             "GL3PlusVaoManager::getDynamicBufferCurrentFrame" );

                return fenceName;
            }

            // After the first time, need to start flushing, and wait for a looong time.
            waitFlags = GL_SYNC_FLUSH_COMMANDS_BIT;
            waitDuration = kOneSecondInNanoSeconds;
        }

        return fenceName;
    }
    //-----------------------------------------------------------------------------------
    GLuint GL3PlusVaoManager::getAttributeIndexFor( VertexElementSemantic semantic )
    {
        return VERTEX_ATTRIBUTE_INDEX[semantic - 1];
    }
    //-----------------------------------------------------------------------------------
    GL3PlusVaoManager::VboFlag GL3PlusVaoManager::bufferTypeToVboFlag( BufferType bufferType )
    {
        return static_cast<VboFlag>( std::max( 0, (bufferType - BT_DYNAMIC_DEFAULT) +
                                                    CPU_ACCESSIBLE_DEFAULT ) );
    }
}

