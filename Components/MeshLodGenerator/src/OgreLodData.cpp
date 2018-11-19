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
// Use float limits instead of Real limits, because LodConfigSerializer may convert them to float.
const Real LodData::NEVER_COLLAPSE_COST = std::numeric_limits<float>::max();
const Real LodData::UNINITIALIZED_COLLAPSE_COST = std::numeric_limits<float>::infinity();

void LodData::Vertex::addEdge( const LodData::Edge& edge )
{
    OgreAssert(edge.dst != this, "");
    VEdges::iterator it;
    it = edges.add(edge);
    if (it == edges.end()) {
        edges.back().refCount = 1;
    } else {
        it->refCount++;
    }
}

void LodData::Vertex::removeEdge( const LodData::Edge& edge )
{
    VEdges::iterator it = edges.findExists(edge);
    if (it->refCount == 1) {
        edges.remove(it);
    } else {
        it->refCount--;
    }
}

bool LodData::VertexEqual::operator() (const LodData::Vertex* lhs, const LodData::Vertex* rhs) const
{
    return lhs->position == rhs->position;
}

size_t LodData::VertexHash::operator() (const LodData::Vertex* v) const
{
    // Stretch the values to an integer grid.
    Real stretch = (Real)0x7fffffff / mGen->mMeshBoundingSphereRadius;
    int hash = (int)(v->position.x * stretch);
    hash ^= (int)(v->position.y * stretch) * 0x100;
    hash ^= (int)(v->position.z * stretch) * 0x10000;
    return (size_t)hash;
}

unsigned int LodData::Triangle::getVertexID(const LodData::Vertex* v) const
{
    for (int i = 0; i < 3; i++) {
        if (vertex[i] == v) {
            return vertexID[i];
        }
    }
    OgreAssert(0, "");
    return 0;
}
bool LodData::Triangle::isMalformed()
{
    return vertex[0] == vertex[1] || vertex[0] == vertex[2] || vertex[1] == vertex[2];
}

LodData::Edge::Edge(LodData::Vertex* destination) :
    dst(destination)
#if OGRE_DEBUG_MODE
    , collapseCost(UNINITIALIZED_COLLAPSE_COST)
#endif
    , refCount(0)
{

}

LodData::Edge::Edge(const LodData::Edge& b)
{
    *this = b;
}

bool LodData::Edge::operator< (const LodData::Edge& other) const
{
    return (size_t) dst < (size_t) other.dst;   // Comparing pointers for uniqueness.
}

LodData::Edge& LodData::Edge::operator= (const LodData::Edge& b)
{
    dst = b.dst;
    collapseCost = b.collapseCost;
    refCount = b.refCount;
    return *this;
}

bool LodData::Edge::operator== (const LodData::Edge& other) const
{
    return dst == other.dst;
}

}
