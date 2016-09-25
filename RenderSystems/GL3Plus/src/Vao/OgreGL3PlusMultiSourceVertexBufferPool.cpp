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

#include "Vao/OgreGL3PlusMultiSourceVertexBufferPool.h"
#include "Vao/OgreGL3PlusBufferInterface.h"
#include "Vao/OgreVertexBufferPacked.h"

namespace Ogre
{
    GL3PlusMultiSourceVertexBufferPool::GL3PlusMultiSourceVertexBufferPool(
                                                size_t vboPoolIndex, GLuint vboName,
                                                const VertexElement2VecVec &vertexElementsBySource,
                                                size_t maxVertices, BufferType bufferType,
                                                size_t internalBufferStart,
                                                VaoManager *vaoManager ) :
        MultiSourceVertexBufferPool( vertexElementsBySource, maxVertices, bufferType,
                                     internalBufferStart, vaoManager ),
        mVboPoolIndex( vboPoolIndex ),
        mVboName( vboName )
    {
    }
    //-----------------------------------------------------------------------------------
    GL3PlusMultiSourceVertexBufferPool::~GL3PlusMultiSourceVertexBufferPool()
    {
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusMultiSourceVertexBufferPool::allocateVbo( size_t numVertices,
                                                          size_t &outBufferOffset )
    {
        if( mBufferType >= BT_DYNAMIC_DEFAULT )
            numVertices *= mVaoManager->getDynamicBufferMultiplier();

        GL3PlusVaoManager::BlockVec::iterator blockIt = mFreeBlocks.begin();
        GL3PlusVaoManager::BlockVec::iterator blockEn = mFreeBlocks.end();

        while( blockIt != blockEn && numVertices < blockIt->size )
            ++blockIt;

        if( blockIt != blockEn )
        {
            GL3PlusVaoManager::Block &bestBlock = *blockIt;

            //Tell caller the offset
            outBufferOffset = bestBlock.offset;

            //Shrink our records about available data.
            bestBlock.size   -= numVertices;
            bestBlock.offset += numVertices;

            if( bestBlock.size == 0 )
                efficientVectorRemove( mFreeBlocks, blockIt );
        }
        else
        {
            outBufferOffset = mMaxVertices;
        }
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusMultiSourceVertexBufferPool::deallocateVbo( size_t bufferOffset, size_t numVertices )
    {
        if( mBufferType >= BT_DYNAMIC_DEFAULT )
            numVertices *= mVaoManager->getDynamicBufferMultiplier();

        //See if we're contiguous to a free block and make that block grow.
        mFreeBlocks.push_back( GL3PlusVaoManager::Block( bufferOffset, numVertices ) );
        GL3PlusVaoManager::mergeContiguousBlocks( mFreeBlocks.end() - 1, mFreeBlocks );
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusMultiSourceVertexBufferPool::createVertexBuffers(
                                                    VertexBufferPackedVec &outVertexBuffers,
                                                    size_t numVertices,
                                                    void * const *initialData, bool keepAsShadow )
    {
        size_t vertexOffset;
        allocateVbo( numVertices, vertexOffset );

        if( vertexOffset == mMaxVertices )
        {
            for( size_t i=0; i<mVertexElementsBySource.size(); ++i )
            {
                GL3PlusBufferInterface *bufferInterface = new GL3PlusBufferInterface( 0, mVboName, 0 /*TODO*/ );
                void *_initialData = 0;
                if( initialData )
                    _initialData = initialData[i];

                outVertexBuffers.push_back(
                    OGRE_NEW VertexBufferPacked( mInternalBufferStart + vertexOffset + mSourceOffset[i],
                                                 numVertices, mBytesPerVertexPerSource[i], 0,
                                                 mBufferType, _initialData, keepAsShadow, mVaoManager,
                                                 bufferInterface, mVertexElementsBySource[i],
                                                 vertexOffset, this, i ) );
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusMultiSourceVertexBufferPool::destroyVertexBuffersImpl(
                                                    VertexBufferPackedVec &inOutVertexBuffers )
    {
        //Any of the vertex buffers will do (base class already checked they're all from the same group).
        VertexBufferPacked *vertexBuffer = inOutVertexBuffers[0];
        uint32 numVertices = vertexBuffer->getNumElements();

        deallocateVbo( vertexBuffer->_getInternalBufferStart() - mInternalBufferStart -
                       mSourceOffset[vertexBuffer->_getSourceIndex()], numVertices );
    }
}

