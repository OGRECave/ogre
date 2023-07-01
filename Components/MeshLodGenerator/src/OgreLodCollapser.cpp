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
#include "OgreMeshLodPrecompiledHeaders.h"

namespace Ogre
{

    void LodCollapser::collapse( LodData* data, LodCollapseCost* cost, LodOutputProvider* output, int vertexCountLimit, Real collapseCostLimit )
    {
        while (data->mCollapseCostHeap.size() > static_cast<size_t>(vertexCountLimit))
        {
            LodData::CollapseCostHeap::iterator nextVertex = data->mCollapseCostHeap.begin();
            if (nextVertex->first < collapseCostLimit)
            {
                mLastReducedVertex = nextVertex->second;
                collapseVertex(data, cost, output, mLastReducedVertex);
            } else {
                break;
            }
        }
    }

#if OGRE_DEBUG_MODE
    void LodCollapser::assertValidMesh(LodData* data)
    {
        // Allows to find bugs in collapsing.
        //  size_t s1 = mUniqueVertexSet.size();
        //  size_t s2 = mCollapseCostHeap.size();
        LodData::CollapseCostHeap::iterator it = data->mCollapseCostHeap.begin();
        LodData::CollapseCostHeap::iterator itEnd = data->mCollapseCostHeap.end();
        while (it != itEnd) {
            assertValidVertex(data, it->second);
            it++;
        }
    }

    void LodCollapser::assertValidVertex(LodData* data, LodData::Vertex* v)
    {
        // Allows to find bugs in collapsing.
        LodData::VTriangles::iterator it = v->triangles.begin();
        LodData::VTriangles::iterator itEnd = v->triangles.end();
        for (; it != itEnd; it++) {
            LodData::Triangle* t = *it;
            for (int i = 0; i < 3; i++) {
                OgreAssert(t->vertex[i]->costHeapPosition != data->mCollapseCostHeap.end(), "");
                t->vertex[i]->edges.findExists(LodData::Edge(t->vertex[i]->collapseTo));
                for (int n = 0; n < 3; n++) {
                    if (i != n) {
                        LodData::VEdges::iterator edgeIt = t->vertex[i]->edges.findExists(LodData::Edge(t->vertex[n]));
                        OgreAssert(edgeIt->collapseCost != LodData::UNINITIALIZED_COLLAPSE_COST, "");
                    } else {
                        OgreAssert(t->vertex[i]->edges.find(LodData::Edge(t->vertex[n])) == t->vertex[i]->edges.end(), "");
                    }
                }
            }
        }
    }

    void LodCollapser::assertOutdatedCollapseCost( LodData* data, LodCollapseCost* cost, LodData::Vertex* vertex )
    {
        // Validates that collapsing has updated all edges needed by computeEdgeCollapseCost.
        // This will OgreAssert if the dependencies inside computeEdgeCollapseCost changes.
        LodData::VEdges::iterator it = vertex->edges.begin();
        LodData::VEdges::iterator itEnd = vertex->edges.end();
        for (; it != itEnd; it++) {
            OgreAssert(it->collapseCost == cost->computeEdgeCollapseCost(data, vertex, &*it), "");
            LodData::Vertex* neighbor = it->dst;
            LodData::VEdges::iterator it2 = neighbor->edges.begin();
            LodData::VEdges::iterator it2End = neighbor->edges.end();
            for (; it2 != it2End; it2++) {
                OgreAssert(it2->collapseCost == cost->computeEdgeCollapseCost(data, neighbor, &*it2), "");
            }
        }
    }
#endif // if OGRE_DEBUG_MODE

    bool LodCollapser::hasSrcID(unsigned int srcID, size_t submeshID)
    {
        // This will only return exact matches.
        for (auto & tmpCollapsedEdge : tmpCollapsedEdges) {
            if (tmpCollapsedEdge.srcID == srcID && tmpCollapsedEdge.submeshID == submeshID) {
                return true;
            }
        }
        return false; // Not found
    }
    void LodCollapser::removeTriangleFromEdges(LodData::Triangle* triangle, LodData::Vertex* skip)
    {
        triangle->isRemoved = true;
        // skip is needed if we are iterating on the vertex's edges or triangles.
        for (auto & i : triangle->vertex) {
            if (i != skip) {
                i->triangles.removeExists(triangle);
            }
        }
        for (int i = 0; i < 3; i++) {
            for (int n = 0; n < 3; n++) {
                if (i != n && triangle->vertex[i] != skip) {
                    triangle->vertex[i]->removeEdge(LodData::Edge(triangle->vertex[n]));
                }
            }
        }
    }
    void LodCollapser::removeLine(LodData::Line* line, LodData::Vertex* skip)
    {
        line->isRemoved = true;

        for (auto & i : line->vertex) {
            if (i != skip) {
                i->lines.removeExists(line);
            }
        }
    }

    size_t LodCollapser::findDstID(unsigned int srcID, size_t submeshID)
    {
        // Tries to find a compatible edge.

        // Exact match search.
        for (size_t i = 0; i < tmpCollapsedEdges.size(); i++) {
            if (tmpCollapsedEdges[i].srcID == srcID && tmpCollapsedEdges[i].submeshID == submeshID) {
                return i;
            }
        }

        // Usable match search.
        for (size_t i = 0; i < tmpCollapsedEdges.size(); i++) {
            if (tmpCollapsedEdges[i].submeshID == submeshID) {
                return i;
            }
        }
        return std::numeric_limits<size_t>::max(); // Not found
    }
    void LodCollapser::replaceVertexID(LodData::Triangle* triangle, unsigned int oldID, unsigned int newID, LodData::Vertex* dst)
    {
        dst->triangles.addNotExists(triangle);
        // NOTE: triangle is not removed from src. This is implementation specific optimization.

        // Its up to the compiler to unroll everything.
        for (int i = 0; i < 3; i++) {
            if (triangle->vertexID[i] == oldID) {
                for (int n = 0; n < 3; n++) {
                    if (i != n) {
                        // This is implementation specific optimization to remove following line.
                        // triangle->vertex[i]->removeEdge(LodData::Edge(triangle->vertex[n]));

                        triangle->vertex[n]->removeEdge(LodData::Edge(triangle->vertex[i]));
                        triangle->vertex[n]->addEdge(LodData::Edge(dst));
                        dst->addEdge(LodData::Edge(triangle->vertex[n]));
                    }
                }
                triangle->vertex[i] = dst;
                triangle->vertexID[i] = newID;
                return;
            }
        }
        OgreAssert(0, "");
    }
    void LodCollapser::replaceVertexID(LodData::Line* line, unsigned int oldID, unsigned int newID, LodData::Vertex* dst)
    {
        dst->lines.addNotExists(line);
        // NOTE: line is not removed from src. This is implementation specific optimization.

        for (int i = 0; i < 2; i++) {
            if (line->vertexID[i] == oldID) {
                line->vertex[i] = dst;
                line->vertexID[i] = newID;
                return;
            }
        }
        OgreAssert(0, "");
    }
    void LodCollapser::collapseVertex( LodData* data, LodCollapseCost* cost, LodOutputProvider* output, LodData::Vertex* src )
    {
        LodData::Vertex* dst = src->collapseTo;
#if OGRE_DEBUG_MODE
        assertValidVertex(data, dst);
        assertValidVertex(data, src);
#endif
        OgreAssert(src->costHeapPosition->first != LodData::NEVER_COLLAPSE_COST, "");
        OgreAssert(src->costHeapPosition->first != LodData::UNINITIALIZED_COLLAPSE_COST, "");
        OgreAssert(!src->edges.empty(), "");
        OgreAssert(!src->triangles.empty(), "");
        OgreAssert(src->edges.find(LodData::Edge(dst)) != src->edges.end(), "");

        // It may have vertexIDs and triangles from different submeshes(different vertex buffers),
        // so we need to connect them correctly based on deleted triangle's edge.
        // mCollapsedEdgeIDs will be used, when looking up the connections for replacement.
        tmpCollapsedEdges.clear();
        LodData::VTriangles::iterator it = src->triangles.begin();
        LodData::VTriangles::iterator itEnd = src->triangles.end();
        for (; it != itEnd; ++it) {
            LodData::Triangle* triangle = *it;
            if (triangle->hasVertex(dst)) {
                // Remove a triangle
                // Tasks:
                // 1. Add it to the collapsed edges list.
                // 2. Reduce index count for the Lods, which will not have this triangle.
                // 3. Remove references/pointers to this triangle and mark as removed.

                // 1. task
                unsigned int srcID = triangle->getVertexID(src);
                if (!hasSrcID(srcID, triangle->submeshID)) {
                    tmpCollapsedEdges.push_back(CollapsedEdge());
                    tmpCollapsedEdges.back().srcID = srcID;
                    tmpCollapsedEdges.back().dstID = triangle->getVertexID(dst);
                    tmpCollapsedEdges.back().submeshID = triangle->submeshID;
                }

                // 2. task
                data->mIndexBufferInfoList[triangle->submeshID].indexCount -= 3;
                output->triangleRemoved(data, triangle);
                // 3. task
                removeTriangleFromEdges(triangle, src);

            }
        }
        OgreAssert(tmpCollapsedEdges.size(), "");
        OgreAssert(dst->edges.find(LodData::Edge(src)) == dst->edges.end(), "");

        LodData::VLines::iterator lineIt = src->lines.begin();
        LodData::VLines::iterator lineItEnd = src->lines.end();
        for (; lineIt != lineItEnd; ++lineIt)
        {
            LodData::Line* line = *lineIt;
            if (line->hasVertex(dst))
            {
                // Remove a line
                // Tasks:
                // 1. Add it to the collapsed edges list.
                // 2. Reduce index count for the Lods, which will not have this line.
                // 3. Remove references/pointers to this line and mark as removed.

                // 1. task
                unsigned int srcID = line->getVertexID(src);
                if (!hasSrcID(srcID, line->submeshID)) {
                    tmpCollapsedEdges.push_back(CollapsedEdge());
                    tmpCollapsedEdges.back().srcID = srcID;
                    tmpCollapsedEdges.back().dstID = line->getVertexID(dst);
                    tmpCollapsedEdges.back().submeshID = line->submeshID;
                }

                // 2. task
                data->mIndexBufferInfoList[line->submeshID].indexCount -= 2;
                output->lineRemoved(data, line);
                // 3. task
                removeLine(line, src);
            }
        }

        it = src->triangles.begin();
        for (; it != itEnd; ++it) {
            LodData::Triangle* triangle = *it;
            if (!triangle->hasVertex(dst)) {
                // Replace a triangle
                // Tasks:
                // 1. Determine the edge which we will move along. (we need to modify single vertex only)
                // 2. Move along the selected edge.

                // 1. task
                unsigned int srcID = triangle->getVertexID(src);
                size_t id = findDstID(srcID, triangle->submeshID);
                if (id == std::numeric_limits<size_t>::max()) {
                    // Not found any edge to move along.
                    // Destroy the triangle.
                    data->mIndexBufferInfoList[triangle->submeshID].indexCount -= 3;
                    output->triangleRemoved(data, triangle);
                    removeTriangleFromEdges(triangle, src);
                    continue;
                }
                unsigned int dstID = tmpCollapsedEdges[id].dstID;

                // 2. task
                replaceVertexID(triangle, srcID, dstID, dst);

                output->triangleChanged(data, triangle);

#if MESHLOD_QUALITY >= 3
                triangle->computeNormal();
#endif
            }
        }

        lineIt = src->lines.begin();
        for (; lineIt != lineItEnd; ++lineIt)
        {
            LodData::Line* line = *lineIt;
            if (!line->hasVertex(dst))
            {
                // Replace a line
                // Tasks:
                // 1. Determine the edge which we will move along. (we need to modify single vertex only)
                // 2. Move along the selected edge.

                // 1. task
                unsigned int srcID = line->getVertexID(src);
                size_t id = findDstID(srcID, line->submeshID);
                if (id == std::numeric_limits<size_t>::max()) {
                    // Not found any edge to move along.
                    // Destroy the triangle.
                    data->mIndexBufferInfoList[line->submeshID].indexCount -= 2;
                    output->lineRemoved(data, line);
                    removeLine(line, src);
                    continue;
                }
                unsigned int dstID = tmpCollapsedEdges[id].dstID;

                // 2. task
                replaceVertexID(line, srcID, dstID, dst);

                output->lineChanged(data, line);
            }
        }

        dst->seam |= src->seam; // Inherit seam property

#if MESHLOD_QUALITY <= 2
        LodData::VEdges::iterator it3 = src->edges.begin();
        LodData::VEdges::iterator it3End = src->edges.end();
        for (; it3 != it3End; ++it3) {
            cost->updateVertexCollapseCost(data, it3->dst);
        }
#else
        // TODO: Find out why is this needed. assertOutdatedCollapseCost() fails on some
        // rare situations without this. For example goblin.mesh fails.
        typedef SmallVector<LodData::Vertex*, 64> UpdatableList;
        UpdatableList updatable;
        LodData::VEdges::iterator it3 = src->edges.begin();
        LodData::VEdges::iterator it3End = src->edges.end();
        for (; it3 != it3End; it3++) {
            updatable.push_back(it3->dst);
            LodData::VEdges::iterator it4End = it3->dst->edges.end();
            LodData::VEdges::iterator it4 = it3->dst->edges.begin();
            for (; it4 != it4End; it4++) {
                updatable.push_back(it4->dst);
            }
        }

        // Remove duplicates.
        UpdatableList::iterator it5 = updatable.begin();
        UpdatableList::iterator it5End = updatable.end();
        std::sort(it5, it5End);
        it5End = std::unique(it5, it5End);

        for (; it5 != it5End; it5++) {
            cost->updateVertexCollapseCost(data, *it5);
        }
#if OGRE_DEBUG_MODE
        it3 = src->edges.begin();
        it3End = src->edges.end();
        for (; it3 != it3End; it3++) {
            assertOutdatedCollapseCost(data, cost, it3->dst);
        }
        it3 = dst->edges.begin();
        it3End = dst->edges.end();
        for (; it3 != it3End; it3++) {
            assertOutdatedCollapseCost(data, cost, it3->dst);
        }
        assertOutdatedCollapseCost(data, cost, dst);
#endif // ifndef OGRE_DEBUG_MODE
#endif // ifndef MESHLOD_QUALITY
        data->mCollapseCostHeap.erase(src->costHeapPosition); // Remove src from collapse costs.
        src->edges.clear(); // Free memory
        src->lines.clear(); // Free memory
        src->triangles.clear(); // Free memory
#if OGRE_DEBUG_MODE
        src->costHeapPosition = data->mCollapseCostHeap.end();
        assertValidVertex(data, dst);
#endif
    }

    bool LodCollapser::_getLastVertexPos(LodData* data, Vector3& outVec)
    {
        if(mLastReducedVertex){
            outVec = mLastReducedVertex->position;
            return true;
        } else {
            return false;
        }
    }

    bool LodCollapser::_getLastVertexCollapseTo(LodData* data, Vector3& outVec )
    {
        if(mLastReducedVertex){
            outVec = mLastReducedVertex->collapseTo->position;
            return true;
        } else {
            return false;
        }
    }
}
