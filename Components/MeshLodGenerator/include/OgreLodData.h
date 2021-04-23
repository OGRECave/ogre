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

#ifndef _LodData_H__
#define _LodData_H__

#include "OgreLodPrerequisites.h"
#include "OgreVectorSet.h"
#include "OgreVectorSetImpl.h"
#include "OgreVector.h"
#include "OgreHeaderPrefix.h"

#include <unordered_set>

#ifndef MESHLOD_QUALITY
/// MESHLOD_QUALITY=1 is fastest processing time.
/// MESHLOD_QUALITY=2 is balanced performance/quality (default)
/// MESHLOD_QUALITY=3 is best quality.
#define MESHLOD_QUALITY 2
#endif

namespace Ogre
{
/** \addtogroup Optional
*  @{
*/
/** \addtogroup MeshLodGenerator
*  @{
*/
struct _OgreLodExport LodData {

    static const Real NEVER_COLLAPSE_COST /*= std::numeric_limits<Real>::max()*/;
    static const Real UNINITIALIZED_COLLAPSE_COST /*= std::numeric_limits<Real>::infinity()*/;

    struct Edge;
    struct Vertex;
    struct Triangle;
    struct VertexHash;
    struct VertexEqual;

    typedef std::vector<Vertex> VertexList;
    typedef std::vector<Triangle> TriangleList;
    typedef std::unordered_set<Vertex*, VertexHash, VertexEqual> UniqueVertexSet;
    typedef std::multimap<Real, Vertex*> CollapseCostHeap;

    typedef VectorSet<Edge, 8> VEdges;
    typedef VectorSet<Triangle*, 7> VTriangles;

    /// Hash function for UniqueVertexSet.
    struct VertexHash {
        LodData* mGen;

        VertexHash() : mGen(0) { assert(0); }
        VertexHash(LodData* gen) { mGen = gen; }
        size_t operator() (const Vertex* v) const;
    };

    /// Equality function for UniqueVertexSet.
    struct VertexEqual {
        bool operator() (const Vertex* lhs, const Vertex* rhs) const;
    };

    // Directed edge
    struct Edge {
        Vertex* dst; // destination vertex. (other end of the edge)
        Real collapseCost; // cost of the edge.
        int refCount; // Reference count on how many triangles are using this edge. The edge will be removed when it gets 0.

        explicit Edge(Vertex* destination);
        bool operator== (const Edge& other) const;
        Edge& operator= (const Edge& b);
        Edge(const Edge& b);
        bool operator< (const Edge& other) const;
    };

    struct Vertex {
        Vector3 position;
        Vector3 normal;
        VEdges edges;
        VTriangles triangles;
        
        Vertex* collapseTo;
        bool seam;
        CollapseCostHeap::iterator costHeapPosition; /// Iterator pointing to the position in the mCollapseCostSet, which allows fast remove.

        void addEdge(const Edge& edge);
        void removeEdge(const Edge& edge);
    };
    
    struct Triangle {
        Vertex* vertex[3];
        Vector3 normal;
        bool isRemoved;
        unsigned short submeshID; /// ID of the submesh. Usable with mMesh.getSubMesh() function.
        unsigned int vertexID[3]; /// Vertex ID in the buffer associated with the submeshID.

        void computeNormal()
        {
            normal = Math::calculateBasicFaceNormal(vertex[0]->position, vertex[1]->position,
                                                    vertex[2]->position);
        }
        bool hasVertex(const Vertex* v) const
        {
            return (v == vertex[0] || v == vertex[1] || v == vertex[2]);
        }

        unsigned int getVertexID(const Vertex* v) const;
        bool isMalformed();
    };

    union IndexBufferPointer {
        unsigned short* pshort;
        unsigned int* pint;
    };

    struct IndexBufferInfo {
        size_t indexSize;
        size_t indexCount;
        IndexBufferPointer buf; /// Used by output providers only!
        size_t prevOnlyIndexCount; /// Used by output providers only!
        size_t prevIndexCount; /// Used by output providers only!
    };

    typedef std::vector<IndexBufferInfo> IndexBufferInfoList;

    /// Provides position based vertex lookup. Position is the real identifier of a vertex.
    UniqueVertexSet mUniqueVertexSet;

    VertexList mVertexList;
    TriangleList mTriangleList;

    /// Makes possible to get the vertices with the smallest collapse cost.
    CollapseCostHeap mCollapseCostHeap;
    IndexBufferInfoList mIndexBufferInfoList;
#if OGRE_DEBUG_MODE
    /**
     * @brief The name of the mesh being processed.
     *
     * This is separate from mMesh in order to allow for access from background threads.
     */
    String mMeshName;
#endif
    Real mMeshBoundingSphereRadius;
    bool mUseVertexNormals;

    template<typename T, typename A>
    static size_t getVectorIDFromPointer(const std::vector<T, A>& vec, const T* pointer) {
        size_t id = pointer - &vec.at(0);
        OgreAssertDbg(id < vec.size() && (&vec[id] == pointer), "Invalid pointer");
        return id;
    }

#if OGRE_COMPILER == OGRE_COMPILER_MSVC
    // We know it's safe to pass this pointer to VertexHash because it's not used there yet.
#   pragma warning ( push )
#   pragma warning ( disable: 4355 )
#endif
    LodData() :
        mUniqueVertexSet((UniqueVertexSet::size_type) 0,
        (const UniqueVertexSet::hasher&) VertexHash(this)),
        mMeshBoundingSphereRadius(0.0f),
        mUseVertexNormals(true)
    {}
#if OGRE_COMPILER == OGRE_COMPILER_MSVC
#   pragma warning ( pop )
#endif
};
/** @} */
/** @} */
}

#include "OgreHeaderSuffix.h"

#endif
