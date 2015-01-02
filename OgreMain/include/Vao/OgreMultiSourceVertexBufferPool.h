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

#ifndef _Ogre_MultiSourceVertexBufferPool_H_
#define _Ogre_MultiSourceVertexBufferPool_H_

#include "OgrePrerequisites.h"

#include "Vao/OgreVertexBufferPacked.h"

namespace Ogre
{
    /** Vertex Array Objects do not need to change when:
            The same (internal) vertex buffer is still bound.
            The same vertex format is still used.
            The same (internal) index buffer is still bound.
        This also allows for MultDrawIndirect to render multiple meshes in just one call.
        Ogre takes care of pretending that multiple different vertex buffers are indepedent
        when internally we keep everything within the same buffers.
    @par
        However multisource buffers (i.e. having position in one buffer, UVs in another) pose
        a problem: In order to keep using the same VAO, the internal distance between the
        start (offset) of the position and the start of the UVs in the buffer must remain
        the same for all the meshes using the same multisource vertex declaration.
    @par
        Here's where MultiSourceVertexBufferPool comes into play: We need an extra layer
        on top of the VaoManager to manage multisource vertex buffers, in addition to
        the regular memory management the VaoManager already performs.
    @par
        First create a MultiSourceVertexBufferPool from the VaoManager, then request
        the group of vertex buffers to this class.
    @remarks
        Only the combination of buffers returned by createVertexBuffers can be bound together.
        Trying to bind a buffer from two different createVertexBuffers calls is not allowed.
    */
    class _OgreExport MultiSourceVertexBufferPool : public BufferPackedAlloc
    {
    protected:
        VertexElement2VecVec    mVertexElementsBySource;
        size_t                  mMaxVertices;
        BufferType              mBufferType;

        size_t                  mInternalBufferStart;   /// In Vertices
        vector<uint32>::type    mBytesPerVertexPerSource;
        vector<size_t>::type    mSourceOffset;          /// Where each source starts, in vertices
        VaoManager              *mVaoManager;

        virtual void destroyVertexBuffersImpl( VertexBufferPackedVec &inOutVertexBuffers ) = 0;

    public:
        MultiSourceVertexBufferPool( const VertexElement2VecVec &vertexElementsBySource,
                                     size_t maxVertices, BufferType bufferType,
                                     size_t internalBufferStart,
                                     VaoManager *vaoManager );
        virtual ~MultiSourceVertexBufferPool()
        {
        }

        /** Creates a vertex buffer based on the given parameters. Behind the scenes, the vertex buffer
            is part of much larger vertex buffer, in order to reduce bindings at runtime.
        @param outVertexBuffers [out]
            A list of the generated vertex buffers, one per buffer source in mVertexElementsPerSource.
            WARNING: Can be empty if the pool is out of memory or too fragmented to honour this request,
            in which case you need to use another pool (or free some memory from this pool).
        @param numVertices
            The number of vertices for these vertex buffer.
        @param initialData
            Initial data the buffer will hold upon creation. Can be null (i.e. you plan to upload later).
            Cannot be null when bufferType is BT_IMMUTABLE. Must have enough room to prevent an overflow.
            If non-null, the buffer must hold enough data for each vertex buffers.
            E.g. If mVertexElementsPerSource.size() == 3 then this is valid:
                initialData = 0;
            This is also valid:
                initialData[0] = buffer[numVertices * sumOfBytes(mVertexElementsPerSource[0])];
                initialData[1] = 0; //Unless bufferType == BT_IMMUTABLE
                initialData[2] = buffer[numVertices * sumOfBytes(mVertexElementsPerSource[2])];

            This is invalid:
                initialData[0] = whatever;
                initialData[1] = out of bounds;
        @param keepAsShadow
            Whether to keep the pointer "initialData" as a shadow copy of the contents.
            @See BufferPacked::BufferPacked regarding on who is responsible for freeing this pointer
            and what happens if an exception was raised.
        @return
            The desired vertex buffer pointer
        */
        virtual void createVertexBuffers( VertexBufferPackedVec &outVertexBuffers, size_t numVertices,
                                          void * const *initialData, bool keepAsShadow ) = 0;

        /** Destroys all the buffers returned from a call to createVertexBuffers.
            All the returned buffers from that call must be supplied. Not one more, not one less.
            They can be in any order, though.
        @param inOutVertexBuffers [in] [out]
            In: The list of buffers that was returned by createVertexBuffers. Can be sorted in any order
            Out: Cleared list.
        */
        void destroyVertexBuffers( VertexBufferPackedVec &inOutVertexBuffers );

        size_t getBytesOffsetToSource( uint8 sourceIdx ) const;
    };
}

#endif
