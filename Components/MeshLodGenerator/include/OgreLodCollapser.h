
/*
 * -----------------------------------------------------------------------------
 * This source file is part of OGRE
 * (Object-oriented Graphics Rendering Engine)
 * For the latest info, see http://www.ogre3d.org/
 *
 * Copyright (c) 2000-2014 Torus Knot Software Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * -----------------------------------------------------------------------------
 */

#ifndef _LodCollapser_H__
#define _LodCollapser_H__

#include "OgreLodPrerequisites.h"
#include "OgreLodData.h"

namespace Ogre
{

    class _OgreLodExport LodCollapser
    {
    public:
        virtual ~LodCollapser() {}
        /// Reduces vertices until vertexCountLimit or collapseCostLimit is reached.
        virtual void collapse(LodData* data, LodCollapseCost* cost, LodOutputProvider* output, int vertexCountLimit, Real collapseCostLimit);

        /**
         * @brief Returns the last reduced vertex.
         *
         * You should call this function after generateLodLevels!
         *
         * @param data This parameter is not used, but this will guarantee that data is alive.
         * @param outVec The vector receiving the position of the vertex.
         * @return Whether the outVec was changed. If the mesh is reduced at least 1 vertex, then it returns true.
         */
        bool _getLastVertexPos(LodData* data, Vector3& outVec);

        /**
         * @brief Returns the destination of the edge, which was last reduced.
         *
         * You should call this function after generateLodLevels!
         *
         * @param data This parameter is not used, but this will guarantee that data is alive.
         * @param outVec The vector receiving the CollapseTo position.
         * @return Whether the outVec was changed. If the mesh is reduced at least 1 vertex, then it returns true.
         */
        bool _getLastVertexCollapseTo(LodData* data, Vector3& outVec);
    protected:
        struct CollapsedEdge
        {
            unsigned int srcID;
            unsigned int dstID;
            unsigned short submeshID;
        };

        typedef vector<CollapsedEdge>::type CollapsedEdges;

        /// tmp variable, to overcome allocation on every collapse.
        CollapsedEdges tmpCollapsedEdges;

        /// Last reduced vertex. Can be used for debugging purposes. For example the Mesh Lod Editor uses it to select edge.
        LodData::Vertex* mLastReducedVertex;

        /// Collapses a single vertex.
        void collapseVertex(LodData* data, LodCollapseCost* cost, LodOutputProvider* output, LodData::Vertex* src);
        void assertOutdatedCollapseCost(LodData* data, LodCollapseCost* cost, LodData::Vertex* vertex);
        void assertValidMesh(LodData* data);
        void assertValidVertex(LodData* data, LodData::Vertex* v);
        bool hasSrcID(unsigned int srcID, unsigned short submeshID);
        void removeTriangleFromEdges(LodData::Triangle* triangle, LodData::Vertex* skip);
        size_t findDstID(unsigned int srcID, unsigned short submeshID);
        void replaceVertexID(LodData::Triangle* triangle, unsigned int oldID, unsigned int newID, LodData::Vertex* dst);
    };

}
#endif


