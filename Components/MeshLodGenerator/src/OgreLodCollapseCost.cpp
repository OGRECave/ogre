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
    void LodCollapseCost::initCollapseCosts( LodData* data )
    {
        data->mCollapseCostHeap.clear();
        for (auto& v : data->mVertexList) {
            if (!v.edges.empty()) {
                initVertexCollapseCost(data, &v);
            } else {
#if OGRE_DEBUG_MODE
                LogManager::getSingleton().stream() << "In " << data->mMeshName << " never used vertex found with ID: " << data->mCollapseCostHeap.size() << ". "
                    << "Vertex position: ("
                    << v.position.x << ", "
                    << v.position.y << ", "
                    << v.position.z << ") "
                    << "It will be excluded from Lod level calculations.";
#endif
            }
        }
    }

    bool LodCollapseCost::isEdgeCollapsible(LodData::Vertex * src, LodData::Vertex * dst)
    {
        // For every primitive on the src vertex we need a primitive in the same submesh connecting src and dst.
        if (mPreventPunchingHoles)
        {
            for (auto& testTri : src->triangles)
            {
                auto srcSubmeshID = testTri->submeshID;
                bool canConnect = false;

                for (auto& solveTri : dst->triangles)
                {
                    if (solveTri->submeshID == srcSubmeshID && solveTri->hasVertex(src) && solveTri->hasVertex(dst))
                    {
                        canConnect = true;
                        break;
                    }
                }

                if (canConnect == false)
                    return false;
            }
        }
        if (mPreventBreakingLines)
        {
            for (auto& testLine : src->lines)
            {
                auto srcSubmeshID = testLine->submeshID;
                bool canConnect = false;

                for (auto& solveLine : dst->lines)
                {
                    if (solveLine->submeshID == srcSubmeshID && solveLine->hasVertex(src) && solveLine->hasVertex(dst))
                    {
                        canConnect = true;
                        break;
                    }
                }

                if (canConnect == false)
                    return false;
            }
        }
        return true;
    }

    void LodCollapseCost::computeVertexCollapseCost( LodData* data, LodData::Vertex* vertex, Real& collapseCost, LodData::Vertex*& collapseTo )
    {
        for (auto& e : vertex->edges) {
            if (isEdgeCollapsible(vertex, e.dst)) {
                e.collapseCost = computeEdgeCollapseCost(data, vertex, &e);
            } else {
                e.collapseCost = LodData::NEVER_COLLAPSE_COST;
            }
            if (collapseCost > e.collapseCost) {
                collapseCost = e.collapseCost;
                collapseTo = e.dst;
            }
        }
    }
    void LodCollapseCost::initVertexCollapseCost( LodData* data, LodData::Vertex* vertex )
    {
        OgreAssert(!vertex->edges.empty(), "");

        Real collapseCost = LodData::UNINITIALIZED_COLLAPSE_COST;
        LodData::Vertex* collapseTo = NULL;
        computeVertexCollapseCost(data, vertex, collapseCost, collapseTo);

        vertex->collapseTo = collapseTo;
        vertex->costHeapPosition = data->mCollapseCostHeap.emplace(collapseCost, vertex);
    }

    void LodCollapseCost::updateVertexCollapseCost( LodData* data, LodData::Vertex* vertex )
    {
        Real collapseCost = LodData::UNINITIALIZED_COLLAPSE_COST;
        LodData::Vertex* collapseTo = NULL;
        computeVertexCollapseCost(data, vertex, collapseCost, collapseTo);

        if (vertex->collapseTo != collapseTo || collapseCost != vertex->costHeapPosition->first) {
            OgreAssert(vertex->costHeapPosition != data->mCollapseCostHeap.end(), "");
            data->mCollapseCostHeap.erase(vertex->costHeapPosition);
            if (collapseCost != LodData::UNINITIALIZED_COLLAPSE_COST) {
                vertex->collapseTo = collapseTo;
                vertex->costHeapPosition = data->mCollapseCostHeap.emplace(collapseCost, vertex);
            } else {
#if OGRE_DEBUG_MODE
                vertex->collapseTo = NULL;
                vertex->costHeapPosition = data->mCollapseCostHeap.end();
#endif
            }
        }
    }

    bool LodCollapseCost::isBorderVertex(const LodData::Vertex* vertex) const
    {
        for (auto& e : vertex->edges) {
            if (e.refCount == 1)
                return true;
        }
        return false;
    }
}

