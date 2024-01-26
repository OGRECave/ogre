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
        for (const auto& c : data->mCollapseCostHeap)
            assertValidVertex(data, c.second);
    }

    void LodCollapser::assertValidVertex(LodData* data, LodData::Vertex* v)
    {
        // Allows to find bugs in collapsing.
        for (const auto& t : v->triangles) {
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
        for (auto& e : vertex->edges) {
            OgreAssert(e.collapseCost == cost->computeEdgeCollapseCost(data, vertex, &e), "");
            LodData::Vertex* neighbor = e.dst;
            for (auto& e1 : vertex->edges) {
                OgreAssert(e1.collapseCost == cost->computeEdgeCollapseCost(data, neighbor, &e1), "");
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
        for (auto& i : triangle->vertex) {
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

        for (auto& i : line->vertex) {
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
        for (const auto& t : src->triangles) {
            if (t->hasVertex(dst)) {
                // Remove a triangle
                // Tasks:
                // 1. Add it to the collapsed edges list.
                // 2. Reduce index count for the Lods, which will not have this triangle.
                // 3. Remove references/pointers to this triangle and mark as removed.

                // 1. task
                unsigned int srcID = t->getVertexID(src);
                if (!hasSrcID(srcID, t->submeshID)) {
                    tmpCollapsedEdges.push_back(CollapsedEdge());
                    tmpCollapsedEdges.back().srcID = srcID;
                    tmpCollapsedEdges.back().dstID = t->getVertexID(dst);
                    tmpCollapsedEdges.back().submeshID = t->submeshID;
                }

                // 2. task
                data->mIndexBufferInfoList[t->submeshID].indexCount -= 3;
                output->triangleRemoved(data, t);
                // 3. task
                removeTriangleFromEdges(t, src);

            }
        }
        OgreAssert(tmpCollapsedEdges.size(), "");
        OgreAssert(dst->edges.find(LodData::Edge(src)) == dst->edges.end(), "");
        for (const auto& l : src->lines)
        {
            if (l->hasVertex(dst))
            {
                // Remove a line
                // Tasks:
                // 1. Add it to the collapsed edges list.
                // 2. Reduce index count for the Lods, which will not have this line.
                // 3. Remove references/pointers to this line and mark as removed.

                // 1. task
                unsigned int srcID = l->getVertexID(src);
                if (!hasSrcID(srcID, l->submeshID)) {
                    tmpCollapsedEdges.push_back(CollapsedEdge());
                    tmpCollapsedEdges.back().srcID = srcID;
                    tmpCollapsedEdges.back().dstID = l->getVertexID(dst);
                    tmpCollapsedEdges.back().submeshID = l->submeshID;
                }

                // 2. task
                data->mIndexBufferInfoList[l->submeshID].indexCount -= 2;
                output->lineRemoved(data, l);
                // 3. task
                removeLine(l, src);
            }
        }

        for (const auto& t : src->triangles) {
            if (!t->hasVertex(dst)) {
                // Replace a triangle
                // Tasks:
                // 1. Determine the edge which we will move along. (we need to modify single vertex only)
                // 2. Move along the selected edge.

                // 1. task
                unsigned int srcID = t->getVertexID(src);
                size_t id = findDstID(srcID, t->submeshID);
                if (id == std::numeric_limits<size_t>::max()) {
                    // Not found any edge to move along.
                    // Destroy the triangle.
                    data->mIndexBufferInfoList[t->submeshID].indexCount -= 3;
                    output->triangleRemoved(data, t);
                    removeTriangleFromEdges(t, src);
                    continue;
                }
                unsigned int dstID = tmpCollapsedEdges[id].dstID;

                // 2. task
                replaceVertexID(t, srcID, dstID, dst);

                output->triangleChanged(data, t);

#if MESHLOD_QUALITY >= 3
                t->computeNormal();
#endif
            }
        }

        for (const auto& l : src->lines)
        {
            if (!l->hasVertex(dst))
            {
                // Replace a line
                // Tasks:
                // 1. Determine the edge which we will move along. (we need to modify single vertex only)
                // 2. Move along the selected edge.

                // 1. task
                unsigned int srcID = l->getVertexID(src);
                size_t id = findDstID(srcID, l->submeshID);
                if (id == std::numeric_limits<size_t>::max()) {
                    // Not found any edge to move along.
                    // Destroy the triangle.
                    data->mIndexBufferInfoList[l->submeshID].indexCount -= 2;
                    output->lineRemoved(data, l);
                    removeLine(l, src);
                    continue;
                }
                unsigned int dstID = tmpCollapsedEdges[id].dstID;

                // 2. task
                replaceVertexID(l, srcID, dstID, dst);

                output->lineChanged(data, l);
            }
        }

        dst->seam |= src->seam; // Inherit seam property

#if MESHLOD_QUALITY <= 2
        for (const auto& e : src->edges) {
            cost->updateVertexCollapseCost(data, e.dst);
        }
#else
        // TODO: Find out why is this needed. assertOutdatedCollapseCost() fails on some
        // rare situations without this. For example goblin.mesh fails.
        typedef SmallVector<LodData::Vertex*, 64> UpdatableList;
        UpdatableList updatable;
        for (const auto& e : src->edges) {
            updatable.push_back(e.dst);
            for (const auto& e1 : src->edges) {
                updatable.push_back(e1.dst);
            }
        }

        // Remove duplicates.
        UpdatableList::iterator it = updatable.begin();
        UpdatableList::iterator itEnd = updatable.end();
        std::sort(it, itEnd);
        itEnd = std::unique(it, itEnd);

        for (const auto& u : updatable) {
            cost->updateVertexCollapseCost(data, u);
        }
#if OGRE_DEBUG_MODE
        for (const auto& e : src->edges) {
            assertOutdatedCollapseCost(data, cost, e.dst);
        }
        for (const auto& e : src->edges) {
            assertOutdatedCollapseCost(data, cost, e.dst);
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
