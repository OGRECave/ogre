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

#ifndef _Ogre_VaoManager_H_
#define _Ogre_VaoManager_H_

#include "OgrePrerequisites.h"

#include "Vao/OgreVertexBufferPacked.h"
#include "Vao/OgreIndexBufferPacked.h"

namespace Ogre
{
    class _OgreExport VaoManager
    {
    protected:
        virtual VertexBufferPacked* createVertexBufferImpl( size_t numElements,
                                                            uint32 bytesPerElement,
                                                            BufferType bufferType,
                                                            void *initialData, bool keepAsShadow,
                                                            const VertexElement2Vec &vertexElements );

        virtual IndexBufferPacked* createIndexBufferImpl( size_t numElements,
                                                          uint32 bytesPerElement,
                                                          BufferType bufferType,
                                                          void *initialData, bool keepAsShadow );

    public:
        /** Creates a vertex buffer based on the given parameters. Behind the scenes, the vertex buffer
            is part of much larger vertex buffer, in order to reduce bindings at runtime.
        @param vertexElements
            A list of element bindings for this vertex buffer. Once created, changing VertexElements
            is not possible, you'll have to create another Vertex Buffer.
        @param numVertices
            The number of vertices for this vertex
        @param bufferType
            The type of buffer for this vertex buffer. @See BufferType::BT_DYNAMIC special case.
        @param initialData
            Initial data the buffer will hold upon creation. Can be null (i.e. you plan to upload later).
            Cannot be null when bufferType is BT_IMMUTABLE. Must have enough room to prevent an overflow.
        @param keepAsShadow
            Whether to keep the pointer "initialData" as a shadow copy of the contents.
            @See BufferPacked::BufferPacked regarding on who is responsible for freeing this pointer
            and what happens if an exception was raised.
        @return
            The desired vertex buffer pointer
        */
        VertexBufferPacked* createVertexBuffer( const VertexElement2Vec &vertexElements,
                                                size_t numVertices, BufferType bufferType,
                                                void *initialData, bool keepAsShadow );

        /** Creates an index buffer based on the given parameters. Behind the scenes, the buffer
            is actually part of much larger buffer, in order to reduce bindings at runtime.
        @remarks
            @See createVertexBuffer for the remaining parameters not documented here.
        @param indexType
            Whether this Index Buffer should be 16-bit (recommended) or 32-bit
        @param numIndices
            The number of indices
        @return
            The desired index buffer pointer
        */
        IndexBufferPacked* createIndexBuffer( IndexBufferPacked::IndexType indexType,
                                              size_t numIndices, BufferType bufferType,
                                              void *initialData, bool keepAsShadow );
    };
}

#endif
