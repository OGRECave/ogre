/*
  -----------------------------------------------------------------------------
  This source file is part of OGRE
  (Object-oriented Graphics Rendering Engine)
  For the latest info, see http://www.ogre3d.org/

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
#ifndef _RenderOperation_H__
#define _RenderOperation_H__

#include "OgrePrerequisites.h"
#include "OgreVertexIndexData.h"
#include "OgreHlmsPso.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {
namespace v1 {
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup RenderSystem
    *  @{
    */
    /** 'New' rendering operation using vertex buffers. */
    class _OgrePrivate RenderOperation {
    public:
        static AtomicScalar<uint32> MeshIndexId;

        /// This index is set to 0 by default. The RenderQueue will sort by mesh using this index.
        /// Two different RenderOperations may have the same meshIndex, but if so, performance could
        /// be degraded (it would hinder auto instancing, forces rebinding of the vertex & index buffer
        /// per Renderable, etc)
        /// It is the implementation's responsability to assign a (unique if possible) index.
        /// The static variable MeshIndexId is provided as an incrementing ID, but you're not forced to
        /// use it
        uint32 meshIndex;

        /// Vertex source data
        VertexData *vertexData;

        /// The type of operation to perform
        OperationType operationType;

        /** Specifies whether to use indexes to determine the vertices to use as input. If false, the vertices are
            simply read in sequence to define the primitives. If true, indexes are used instead to identify vertices
            anywhere in the buffer, and allowing vertices to be used more than once.
            If true, then the indexBuffer, indexStart and numIndexes properties must be valid. */
        bool useIndexes;

        /// Index data - only valid if useIndexes is true
        IndexData *indexData;

#if OGRE_DEBUG_MODE
        /// Debug pointer back to renderable which created this
        const Renderable* srcRenderable;
#endif

        /// The number of instances for the render operation - this option is supported
        /// in only a part of the render systems.
        size_t numberOfInstances;

        /// Specifies whether rendering to the vertex buffer.
        bool renderToVertexBuffer;

        /** A flag to indicate that it is possible for this operation to use a global
            vertex instance buffer if available.*/
        bool useGlobalInstancingVertexBufferIsAvailable;

    RenderOperation() :
            meshIndex(0),
            vertexData(0),
            operationType(OT_TRIANGLE_LIST),
            useIndexes(true),
            indexData(0),
#if OGRE_DEBUG_MODE
            srcRenderable(0),
#endif
            numberOfInstances(1),
            renderToVertexBuffer(false),
            useGlobalInstancingVertexBufferIsAvailable(true)
            {}


    };
    /** @} */
    /** @} */
}
}

#include "OgreHeaderSuffix.h"

#endif
