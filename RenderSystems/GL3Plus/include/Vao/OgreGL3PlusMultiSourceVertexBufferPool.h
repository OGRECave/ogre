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

#ifndef _Ogre_GL3PlusMultiSourceVertexBufferPool_H_
#define _Ogre_GL3PlusMultiSourceVertexBufferPool_H_

#include "OgreGL3PlusPrerequisites.h"

#include "Vao/OgreMultiSourceVertexBufferPool.h"
#include "Vao/OgreGL3PlusVaoManager.h"

namespace Ogre
{
    class _OgreGL3PlusExport GL3PlusMultiSourceVertexBufferPool : public MultiSourceVertexBufferPool
    {
        size_t mVboPoolIndex;
        GLuint mVboName;

        GL3PlusVaoManager::BlockVec mFreeBlocks;

        /** @See GL3PlusVaoManager::allocateVbo. This is very similar, except we don't have to deal with
            stride changes (as the vertex format remains the same) and we can't request another
            pool if we're out of space (in other words, it's simpler).
        @param numVertices
            The number of vertices to allocate
        @param outBufferOffset
            0-based offset, in vertex count where the buffers start.
            Sets to mMaxVertices if the request couldn't be honoured
            (i.e. numVertices is bigger than the available unfragmented space)
        */
        void allocateVbo( size_t numVertices, size_t &outBufferOffset );

        /// Deallocates a buffer allocated with @allocateVbo. All params are in vertices, not bytes.
        void deallocateVbo( size_t bufferOffset, size_t numVertices );

        virtual void destroyVertexBuffersImpl( VertexBufferPackedVec &inOutVertexBuffers );

    public:
        GL3PlusMultiSourceVertexBufferPool( size_t vboPoolIndex, GLuint vboName,
                                            const VertexElement2VecVec &vertexElementsBySource,
                                            size_t maxVertices, BufferType bufferType,
                                            size_t internalBufferStart,
                                            VaoManager *vaoManager );
        virtual ~GL3PlusMultiSourceVertexBufferPool();

        void createVertexBuffers( VertexBufferPackedVec &outVertexBuffers, size_t numVertices,
                                  void * const *initialData, bool keepAsShadow );
    };
}

#endif
