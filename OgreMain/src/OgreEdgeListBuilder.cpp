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
#include "OgreStableHeaders.h"
#include "OgreEdgeListBuilder.h"
#include "OgreVertexIndexData.h"
#include "OgreOptimisedUtil.h"

namespace Ogre {

    EdgeData::EdgeData() : isClosed(false){}
    
    void EdgeData::log(Log* l)
    {
        EdgeGroupList::iterator i, iend;
        EdgeList::iterator ei, eiend;
        TriangleList::iterator ti, tiend;
        tiend = triangles.end();
        l->logMessage("Edge Data");
        l->logMessage("---------");
        size_t num = 0;
        for (ti = triangles.begin(); ti != tiend; ++ti, ++num)
        {
            Triangle& t = *ti;
            l->logMessage("Triangle " + StringConverter::toString(num) + " = {" +
                "indexSet=" + StringConverter::toString(t.indexSet) + ", " + 
                "vertexSet=" + StringConverter::toString(t.vertexSet) + ", " + 
                "v0=" + StringConverter::toString(t.vertIndex[0]) + ", " + 
                "v1=" + StringConverter::toString(t.vertIndex[1]) + ", " + 
                "v2=" + StringConverter::toString(t.vertIndex[2]) + "}"); 
        }
        iend = edgeGroups.end();
        for (i = edgeGroups.begin(); i != iend; ++i)
        {
            num = 0;
            eiend = i->edges.end();
            l->logMessage("Edge Group vertexSet=" + StringConverter::toString(i->vertexSet));
            for (ei = i->edges.begin(); ei != eiend; ++ei, ++num)
            {
                Edge& e = *ei;
                l->logMessage(
                    "Edge " + StringConverter::toString(num) + " = {\n" + 
                    "  tri0=" + StringConverter::toString(e.triIndex[0]) + ", \n" + 
                    "  tri1=" + StringConverter::toString(e.triIndex[1]) + ", \n" + 
                    "  v0=" + StringConverter::toString(e.vertIndex[0]) + ", \n" + 
                    "  v1=" + StringConverter::toString(e.vertIndex[1]) + ", \n"
                    "  degenerate=" + StringConverter::toString(e.degenerate) + " \n"
                    "}"); 
            }
        }
    }
    //---------------------------------------------------------------------
    EdgeListBuilder::EdgeListBuilder()
        : mEdgeData(0)
    {
    }
    //---------------------------------------------------------------------
    void EdgeListBuilder::addVertexData(const VertexData* vertexData)
    {
        if (vertexData->vertexStart != 0)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "The base vertex index of the vertex data must be zero for build edge list.",
                "EdgeListBuilder::addVertexData");
        }

        mVertexDataList.push_back(vertexData);
    }
    //---------------------------------------------------------------------
    void EdgeListBuilder::addIndexData(const IndexData* indexData, 
        size_t vertexSet, RenderOperation::OperationType opType)
    {
        if (opType != RenderOperation::OT_TRIANGLE_LIST &&
            opType != RenderOperation::OT_TRIANGLE_FAN &&
            opType != RenderOperation::OT_TRIANGLE_STRIP)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "Only triangle list, fan and strip are supported to build edge list.",
                "EdgeListBuilder::addIndexData");
        }

        Geometry geometry;
        geometry.indexData = indexData;
        geometry.vertexSet = vertexSet;
        geometry.opType = opType;
        geometry.indexSet = mGeometryList.size();
        mGeometryList.push_back(geometry);
    }
    //---------------------------------------------------------------------
    EdgeData* EdgeListBuilder::build(void)
    {
        /* Ok, here's the algorithm:
        For each set of indices in turn
          For each set of 3 indexes
            Create a new Triangle entry in the list
            For each vertex referenced by the tri indexes
              Get the position of the vertex as a Vector3 from the correct vertex buffer
              Attempt to locate this position in the existing common vertex set
              If not found
                Create a new common vertex entry in the list
              End If
              Populate the original vertex index and common vertex index 
            Next vertex
            Connect to existing edge(v1, v0) or create a new edge(v0, v1)
            Connect to existing edge(v2, v1) or create a new edge(v1, v2)
            Connect to existing edge(v0, v2) or create a new edge(v2, v0)
          Next set of 3 indexes
        Next index set

        Note that all edges 'belong' to the index set which originally caused them
        to be created, which also means that the 2 vertices on the edge are both referencing the 
        vertex buffer which this index set uses.
        */


        /* 
        There is a major consideration: 'What is a common vertex'? This is a
        crucial decision, since to form a completely close hull, you need to treat
        vertices which are not physically the same as equivalent. This is because
        there will be 'seams' in the model, where discrepancies in vertex components
        other than position (such as normals or texture coordinates) will mean
        that there are 2 vertices in the same place, and we MUST 'weld' them
        into a single common vertex in order to have a closed hull. Just looking
        at the unique vertex indices is not enough, since these seams would render
        the hull invalid.

        So, we look for positions which are the same across vertices, and treat 
        those as as single vertex for our edge calculation. However, this has
        it's own problems. There are OTHER vertices which may have a common 
        position that should not be welded. Imagine 2 cubes touching along one
        single edge. The common vertices on that edge, if welded, will cause 
        an ambiguous hull, since the edge will have 4 triangles attached to it,
        whilst a manifold mesh should only have 2 triangles attached to each edge.
        This is a problem.

        We deal with this with allow welded multiple pairs of edges. Using this
        techniques, we can build a individual hull even if the model which has a
        potentially ambiguous hull. This is feasible, because in the case of
        multiple hulls existing, each hull can cast same shadow in any situation.
        Notice: For stencil shadow, we intent to build a valid shadow volume for
        the mesh, not the valid hull for the mesh.
        */

        // Sort the geometries in the order of vertex set, so we can grouping
        // triangles by vertex set easy.
        std::sort(mGeometryList.begin(), mGeometryList.end(), geometryLess());
        // Initialize edge data
        mEdgeData = OGRE_NEW EdgeData();
        // resize the edge group list to equal the number of vertex sets
        mEdgeData->edgeGroups.resize(mVertexDataList.size());
        // Initialise edge group data
        for (unsigned short vSet = 0; vSet < mVertexDataList.size(); ++vSet)
        {
            mEdgeData->edgeGroups[vSet].vertexSet = vSet;
            mEdgeData->edgeGroups[vSet].vertexData = mVertexDataList[vSet];
            mEdgeData->edgeGroups[vSet].triStart = 0;
            mEdgeData->edgeGroups[vSet].triCount = 0;
        }

        // Build triangles and edge list
        GeometryList::const_iterator i, iend;
        iend = mGeometryList.end();
        for (i = mGeometryList.begin(); i != iend; ++i)
        {
            buildTrianglesEdges(*i);
        }

        // Allocate memory for light facing calculate
        mEdgeData->triangleLightFacings.resize(mEdgeData->triangles.size());

        // Record closed, ie the mesh is manifold
        mEdgeData->isClosed = mEdgeMap.empty();

        return mEdgeData;
    }
    //---------------------------------------------------------------------
    void EdgeListBuilder::buildTrianglesEdges(const Geometry &geometry)
    {
        size_t indexSet = geometry.indexSet;
        size_t vertexSet = geometry.vertexSet;
        const IndexData* indexData = geometry.indexData;
        RenderOperation::OperationType opType = geometry.opType;

        size_t iterations;
        
        switch (opType)
        {
        case RenderOperation::OT_TRIANGLE_LIST:
            iterations = indexData->indexCount / 3;
            break;
        case RenderOperation::OT_TRIANGLE_FAN:
        case RenderOperation::OT_TRIANGLE_STRIP:
            iterations = indexData->indexCount - 2;
            break;
        default:
            return; // Just in case
        };

        // The edge group now we are dealing with.
        EdgeData::EdgeGroup& eg = mEdgeData->edgeGroups[vertexSet];

        // locate position element & the buffer to go with it
        const VertexData* vertexData = mVertexDataList[vertexSet];
        const VertexElement* posElem = vertexData->vertexDeclaration->findElementBySemantic(VES_POSITION);
        HardwareVertexBufferSharedPtr vbuf = 
            vertexData->vertexBufferBinding->getBuffer(posElem->getSource());
        // lock the buffer for reading
        HardwareBufferLockGuard vertexLock(vbuf, HardwareBuffer::HBL_READ_ONLY);
        unsigned char* pBaseVertex = static_cast<unsigned char*>(vertexLock.pData);

        // Get the indexes ready for reading
        bool idx32bit = (indexData->indexBuffer->getType() == HardwareIndexBuffer::IT_32BIT);
        HardwareBufferLockGuard indexLock(indexData->indexBuffer, HardwareBuffer::HBL_READ_ONLY);
        unsigned short* p16Idx = static_cast<unsigned short*>(indexLock.pData) + indexData->indexStart;
        unsigned int* p32Idx = static_cast<unsigned int*>(indexLock.pData) + indexData->indexStart;

        // Iterate over all the groups of 3 indexes
        unsigned int index[3];
        // Get the triangle start, if we have more than one index set then this
        // will not be zero
        size_t triangleIndex = mEdgeData->triangles.size();
        // If it's first time dealing with the edge group, setup triStart for it.
        // Note that we are assume geometries sorted by vertex set.
        if (!eg.triCount)
        {
            eg.triStart = triangleIndex;
        }
        // Pre-reserve memory for less thrashing
        mEdgeData->triangles.reserve(triangleIndex + iterations);
        mEdgeData->triangleFaceNormals.reserve(triangleIndex + iterations);
        for (size_t t = 0; t < iterations; ++t)
        {
            EdgeData::Triangle tri;
            tri.indexSet = indexSet;
            tri.vertexSet = vertexSet;

            if (opType == RenderOperation::OT_TRIANGLE_LIST || t == 0)
            {
                // Standard 3-index read for tri list or first tri in strip / fan
                if (idx32bit)
                {
                    index[0] = p32Idx[0];
                    index[1] = p32Idx[1];
                    index[2] = p32Idx[2];
                    p32Idx += 3;
                }
                else
                {
                    index[0] = p16Idx[0];
                    index[1] = p16Idx[1];
                    index[2] = p16Idx[2];
                    p16Idx += 3;
                }
            }
            else
            {
                // Strips are formed from last 2 indexes plus the current one for
                // triangles after the first.
                // For fans, all the triangles share the first vertex, plus last
                // one index and the current one for triangles after the first.
                // We also make sure that all the triangles are process in the
                // _anti_ clockwise orientation
                index[(opType == RenderOperation::OT_TRIANGLE_STRIP) && (t & 1) ? 0 : 1] = index[2];
                // Read for the last tri index
                if (idx32bit)
                    index[2] = *p32Idx++;
                else
                    index[2] = *p16Idx++;
            }

            Vector3 v[3];
            for (size_t i = 0; i < 3; ++i)
            {
                // Populate tri original vertex index
                tri.vertIndex[i] = index[i];

                // Retrieve the vertex position
                unsigned char* pVertex = pBaseVertex + (index[i] * vbuf->getVertexSize());
                float* pFloat;
                posElem->baseVertexPointerToElement(pVertex, &pFloat);
                v[i].x = *pFloat++;
                v[i].y = *pFloat++;
                v[i].z = *pFloat++;
                // find this vertex in the existing vertex map, or create it
                tri.sharedVertIndex[i] = 
                    findOrCreateCommonVertex(v[i], vertexSet, indexSet, index[i]);
            }

            // Ignore degenerate triangle
            if (tri.sharedVertIndex[0] != tri.sharedVertIndex[1] &&
                tri.sharedVertIndex[1] != tri.sharedVertIndex[2] &&
                tri.sharedVertIndex[2] != tri.sharedVertIndex[0])
            {
                // Calculate triangle normal (NB will require recalculation for 
                // skeletally animated meshes)
                mEdgeData->triangleFaceNormals.push_back(
                    Math::calculateFaceNormalWithoutNormalize(v[0], v[1], v[2]));
                // Add triangle to list
                mEdgeData->triangles.push_back(tri);
                // Connect or create edges from common list
                connectOrCreateEdge(vertexSet, triangleIndex, 
                    tri.vertIndex[0], tri.vertIndex[1], 
                    tri.sharedVertIndex[0], tri.sharedVertIndex[1]);
                connectOrCreateEdge(vertexSet, triangleIndex, 
                    tri.vertIndex[1], tri.vertIndex[2], 
                    tri.sharedVertIndex[1], tri.sharedVertIndex[2]);
                connectOrCreateEdge(vertexSet, triangleIndex, 
                    tri.vertIndex[2], tri.vertIndex[0], 
                    tri.sharedVertIndex[2], tri.sharedVertIndex[0]);
                ++triangleIndex;
            }
        }

        // Update triCount for the edge group. Note that we are assume
        // geometries sorted by vertex set.
        eg.triCount = triangleIndex - eg.triStart;
    }
    //---------------------------------------------------------------------
    void EdgeListBuilder::connectOrCreateEdge(size_t vertexSet, size_t triangleIndex, 
        size_t vertIndex0, size_t vertIndex1, size_t sharedVertIndex0, 
        size_t sharedVertIndex1)
    {
        // Find the existing edge (should be reversed order) on shared vertices
        EdgeMap::iterator emi = mEdgeMap.find(std::pair<size_t, size_t>(sharedVertIndex1, sharedVertIndex0));
        if (emi != mEdgeMap.end())
        {
            // The edge already exist, connect it
            EdgeData::Edge& e = mEdgeData->edgeGroups[emi->second.first].edges[emi->second.second];
            // update with second side
            e.triIndex[1] = triangleIndex;
            e.degenerate = false;

            // Remove from the edge map, so we never supplied to connect edge again
            mEdgeMap.erase(emi);
        }
        else
        {
            // Not found, create new edge
            mEdgeMap.emplace(std::pair<size_t, size_t>(sharedVertIndex0, sharedVertIndex1),
                             std::pair<size_t, size_t>(vertexSet, mEdgeData->edgeGroups[vertexSet].edges.size()));
            EdgeData::Edge e;
            e.degenerate = true; // initialise as degenerate

            // Set only first tri, the other will be completed in connect existing edge
            e.triIndex[0] = triangleIndex;
            e.triIndex[1] = static_cast<size_t>(~0);
            e.sharedVertIndex[0] = sharedVertIndex0;
            e.sharedVertIndex[1] = sharedVertIndex1;
            e.vertIndex[0] = vertIndex0;
            e.vertIndex[1] = vertIndex1;
            mEdgeData->edgeGroups[vertexSet].edges.push_back(e);
        }
    }
    //---------------------------------------------------------------------
    size_t EdgeListBuilder::findOrCreateCommonVertex(const Vector3& vec, 
        size_t vertexSet, size_t indexSet, size_t originalIndex)
    {
        // Because the algorithm doesn't care about manifold or not, we just identifying
        // the common vertex by EXACT same position.
        // Hint: We can use quantize method for welding almost same position vertex fastest.
        std::pair<CommonVertexMap::iterator, bool> inserted = mCommonVertexMap.emplace(vec, mVertices.size());
        if (!inserted.second)
        {
            // Already existing, return old one
            return inserted.first->second;
        }
        // Not found, insert
        CommonVertex newCommon;
        newCommon.index = mVertices.size();
        newCommon.position = vec;
        newCommon.vertexSet = vertexSet;
        newCommon.indexSet = indexSet;
        newCommon.originalIndex = originalIndex;
        mVertices.push_back(newCommon);
        return newCommon.index;
    }
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void EdgeData::updateTriangleLightFacing(const Vector4& lightPos)
    {
        // Triangle face normals should be 1:1 with light facing flags
        assert(triangleFaceNormals.size() == triangleLightFacings.size());

        // Use optimised util to determine if triangle's face normal are light facing
        if(!triangleFaceNormals.empty())
        {
            OptimisedUtil::getImplementation()->calculateLightFacing(
                lightPos,
                &triangleFaceNormals.front(),
                &triangleLightFacings.front(),
                triangleLightFacings.size());
        }
    }
    //---------------------------------------------------------------------
    void EdgeData::updateFaceNormals(size_t vertexSet, 
        const HardwareVertexBufferSharedPtr& positionBuffer)
    {
        assert (positionBuffer->getVertexSize() == sizeof(float) * 3
            && "Position buffer should contain only positions!");

        // Triangle face normals should be 1:1 with triangles
        assert(triangleFaceNormals.size() == triangles.size());

        // Calculate triangles which are using this vertex set
        const EdgeData::EdgeGroup& eg = edgeGroups[vertexSet];
        if (eg.triCount != 0) 
        {
            HardwareBufferLockGuard positionsLock(positionBuffer, HardwareBuffer::HBL_READ_ONLY);
            OptimisedUtil::getImplementation()->calculateFaceNormals(
                static_cast<float*>(positionsLock.pData),
                &triangles[eg.triStart],
                &triangleFaceNormals[eg.triStart],
                eg.triCount);
        }
    }
    //---------------------------------------------------------------------
    EdgeData* EdgeData::clone()
    {
        EdgeData* newEdgeData = OGRE_NEW EdgeData();
        newEdgeData->triangles = triangles;
        newEdgeData->triangleFaceNormals = triangleFaceNormals;
        newEdgeData->triangleLightFacings = triangleLightFacings;
        newEdgeData->edgeGroups = edgeGroups;
        newEdgeData->isClosed = isClosed;
        return newEdgeData;
    }
    //---------------------------------------------------------------------
    void EdgeListBuilder::log(Log* l)
    {
        l->logMessage("EdgeListBuilder Log");
        l->logMessage("-------------------");
        l->logMessage("Number of vertex sets: " + StringConverter::toString(mVertexDataList.size()));
        l->logMessage("Number of index sets: " + StringConverter::toString(mGeometryList.size()));
        
        size_t i, j, k;
        // Log original vertex data
        for(i = 0; i < mVertexDataList.size(); ++i)
        {
            const VertexData* vData = mVertexDataList[i];
            l->logMessage(".");
            l->logMessage("Original vertex set " + 
                StringConverter::toString(i) + " - vertex count " + 
                StringConverter::toString(vData->vertexCount));
            const VertexElement* posElem = vData->vertexDeclaration->findElementBySemantic(VES_POSITION);
            HardwareVertexBufferSharedPtr vbuf = 
                vData->vertexBufferBinding->getBuffer(posElem->getSource());
            // lock the buffer for reading
            HardwareBufferLockGuard vertexLock(vbuf, HardwareBuffer::HBL_READ_ONLY);
            unsigned char* pBaseVertex = static_cast<unsigned char*>(vertexLock.pData);
            float* pFloat;
            for (j = 0; j < vData->vertexCount; ++j)
            {
                posElem->baseVertexPointerToElement(pBaseVertex, &pFloat);
                l->logMessage("Vertex " + StringConverter::toString(j) + 
                    ": (" + StringConverter::toString(pFloat[0]) + 
                    ", " + StringConverter::toString(pFloat[1]) + 
                    ", " + StringConverter::toString(pFloat[2]) + ")");
                pBaseVertex += vbuf->getVertexSize();
            }
        }

        // Log original index data
        for(i = 0; i < mGeometryList.size(); i++)
        {
            const IndexData* iData = mGeometryList[i].indexData;
            l->logMessage(".");
            l->logMessage("Original triangle set " + 
                StringConverter::toString(mGeometryList[i].indexSet) + " - index count " + 
                StringConverter::toString(iData->indexCount) + " - " + 
            "vertex set " + StringConverter::toString(mGeometryList[i].vertexSet) + " - " + 
            "operationType " + StringConverter::toString(mGeometryList[i].opType));
            // Get the indexes ready for reading
            HardwareBufferLockGuard indexLock(iData->indexBuffer, HardwareBuffer::HBL_READ_ONLY);
            unsigned short* p16Idx = static_cast<unsigned short*>(indexLock.pData);
            unsigned int* p32Idx = static_cast<unsigned int*>(indexLock.pData);
            bool isIT32 = iData->indexBuffer->getType() == HardwareIndexBuffer::IT_32BIT;

            for (j = 0; j < iData->indexCount;  )
            {
                if (isIT32)
                {
                    if (mGeometryList[i].opType == RenderOperation::OT_TRIANGLE_LIST
                        || j == 0)
                    {
                        unsigned int n1 = *p32Idx++;
                        unsigned int n2 = *p32Idx++;
                        unsigned int n3 = *p32Idx++;
                        l->logMessage("Triangle " + StringConverter::toString(j) +
                            ": (" + StringConverter::toString(n1) + 
                            ", " + StringConverter::toString(n2) + 
                            ", " + StringConverter::toString(n3) + ")");
                        j += 3;
                    }
                    else
                    {
                        l->logMessage("Triangle " + StringConverter::toString(j) + 
                            ": (" + StringConverter::toString(*p32Idx++) + ")");
                        j++;
                    }
                }
                else
                {
                    if (mGeometryList[i].opType == RenderOperation::OT_TRIANGLE_LIST
                        || j == 0)
                    {
                        unsigned short n1 = *p16Idx++;
                        unsigned short n2 = *p16Idx++;
                        unsigned short n3 = *p16Idx++;
                        l->logMessage("Index " + StringConverter::toString(j) +
                            ": (" + StringConverter::toString(n1) + 
                            ", " + StringConverter::toString(n2) + 
                            ", " + StringConverter::toString(n3) + ")");
                        j += 3;
                    }
                    else
                    {
                        l->logMessage("Triangle " + StringConverter::toString(j) + 
                            ": (" + StringConverter::toString(*p16Idx++) + ")");
                        j++;
                    }
                }
            }

            // Log common vertex list
            l->logMessage(".");
            l->logMessage("Common vertex list - vertex count " + 
                StringConverter::toString(mVertices.size()));
            for (k = 0; k < mVertices.size(); ++k)
            {
                CommonVertex& c = mVertices[k];
                l->logMessage("Common vertex " + StringConverter::toString(k) + 
                    ": (vertexSet=" + StringConverter::toString(c.vertexSet) + 
                    ", originalIndex=" + StringConverter::toString(c.originalIndex) + 
                    ", position=" + StringConverter::toString(c.position));
            }
        }

    }



}

