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
#ifndef _OgreVertexShadowMapHelper_H_
#define _OgreVertexShadowMapHelper_H_

#include "OgrePrerequisites.h"

#include "Vao/OgreVertexArrayObject.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    typedef FastArray<VertexArrayObject*> VertexArrayObjectArray;

    class VertexShadowMapHelper
    {
    public:
        /// Copies all the pointers in inVao to outVao so they are identical without cloning
        /// any memory
        static void useSameVaos( VaoManager *vaoManager, const VertexArrayObjectArray &inVao,
                                 VertexArrayObjectArray &outVao );

        /** Reads all the vertex & index buffer from all the LOD levels in inVao, optimizes
            the buffers for shadow mapping, and stores them as new Vaos in outVao.
        @remarks
            If the buffers can't be optimized (i.e. it is invalid because an LOD level doesn't
            contain the VES_POSITION semantic), useSameVaos is used; which does not
            clone the Vaos.
        @param vaoManager
            VaoManager. Required for buffer management.
        @param inVao
            Input Vao to clone and optimize.
        @param outVao
            Output vao to store the cloned, optimized version.
        */
        static void optimizeForShadowMapping( VaoManager *vaoManager,
                                              const VertexArrayObjectArray &inVao,
                                              VertexArrayObjectArray &outVao );

        /** Copies the contents from srcData into dstData, but shrinking it by removing
            duplicated vertices.
            Duplicated vertices are not removed if there is not index buffer.
        @param dstData [inout]
            Pointer to copy to. Must be numVertices * sum(vertexElements[i]) in size,
            although amount of written bytes may be lower if there were duplicate
            vertices.
        @param vertexElements [in]
            An array with the vertex elements that needs to be copied
            (i.e. VES_POSITION, VES_BLEND_INDICES & VES_BLEND_WEIGHTS)
        @param vertexConversionLut [out]
            Table that maps the old vertices to their new location (only useful
            when the vertex data was actually shrunk)
        @param srcData [in]
            Pointers where to source the data from.
        @param srcOffset [in]
            Location in srcData to get each vertex element.
        @param srcBytesPerVertex [in]
            The vertex size per vertex buffer.
        @param numVertices [in]
            The total amount of vertices.
        @returns
            The new vertex count. Will be <= numVertices
        */
        static uint32 shrinkVertexBuffer( uint8 *dstData,
                                          const VertexElement2 *vertexElements[3],
                                          FastArray<uint32> &vertexConversionLut,
                                          bool hasIndexBuffer,
                                          const uint8 *srcData[3],
                                          const size_t srcOffset[3],
                                          const size_t srcBytesPerVertex[3],
                                          uint32 numVertices );

        /** Finds the first occurence of 'vertexBuffer' pointer in vao, and returns
            its indexes so that vao[outVaoIdx].getVertexBuffers()[outVertexBufferIdx]
            is the found first occurrence.
        @param vao
            Vao to look inside.
        @param vertexBuffer
            Vertex Buffer to look for.
        @param outVaoIdx
            The index to vao[outVaoIdx]. Value is not set if not found.
        @param outVertexBufferIdx
            The index to vao[outVaoIdx].getVertexBuffers[outVertexBufferIdx].
            Value is not set if not found.
        @return
            True if found, false if not found.
        */
        static bool findFirstAppearance( const VertexArrayObjectArray &vao,
                                         const VertexBufferPacked *vertexBuffer,
                                         size_t &outVaoIdx,
                                         size_t &outVertexBufferIdx );
    };

namespace v1
{
    class VertexShadowMapHelper
    {
    public:
        struct Geometry
        {
            v1::VertexData *vertexData;
            v1::IndexData *indexData;
        };

        typedef vector<Geometry>::type GeometryVec;

        static void useSameGeoms( const GeometryVec &inGeom, GeometryVec &outGeom );

        /** Reads all the vertex & index buffer from all the LOD levels in inVao, optimizes
            the buffers for shadow mapping, and stores them as new Vaos in outVao.
        @remarks
            If the buffers can't be optimized (i.e. it is invalid because an LOD level doesn't
            contain the VES_POSITION semantic), useSameVaos is used; which does not
            clone the Vaos.
        @param vaoManager
            VaoManager. Required for buffer management.
        @param inVao
            Input Vao to clone and optimize.
        @param outVao
            Output vao to store the cloned, optimized version.
        */
        static void optimizeForShadowMapping( const VertexShadowMapHelper::GeometryVec &inGeom,
                                              VertexShadowMapHelper::GeometryVec &outGeom );

        static bool findFirstAppearance( const GeometryVec &geom,
                                         const VertexData *vertexBuffer,
                                         size_t &outVaoIdx );
    };
    /** @} */
    /** @} */

} // namespace v1
} // namespace Ogre

#include "OgreHeaderSuffix.h"

#endif


