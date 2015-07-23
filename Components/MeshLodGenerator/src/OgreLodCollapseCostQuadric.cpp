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

// The algorithm in this file is based heavily on:
/*
 * Progressive Mesh type Polygon Reduction Algorithm
 * by Stan Melax (c) 1998
 */

#include "OgreLodCollapseCostQuadric.h"
#include "OgreVector3.h"

namespace Ogre
{

    void LodCollapseCostQuadric::initCollapseCosts( LodData* data )
    {
        mTrianglePlaneQuadricList.resize(data->mTriangleList.size());
        for(size_t i=0; i<mTrianglePlaneQuadricList.size(); i++)
        {
            computeTrianglePlaneQuadric(data, i);
        }
        mVertexQuadricList.resize(data->mVertexList.size());
        for (size_t i=0; i<mVertexQuadricList.size(); i++)
        {
            computeVertexQuadric(data, i);
        }
        LodCollapseCost::initCollapseCosts(data);
    }

    void LodCollapseCostQuadric::computeTrianglePlaneQuadric( LodData* data, size_t triangleID )
    {
        LodData::Triangle& triangle = data->mTriangleList[triangleID];
        Matrix4& quadric = mTrianglePlaneQuadricList[triangleID];
        Real plane[4];
        plane[0] = triangle.normal.x;
        plane[1] = triangle.normal.y;
        plane[2] = triangle.normal.z;
        Vector3& v0 = triangle.vertex[0]->position;
        plane[3] = -v0.dotProduct(triangle.normal);
        for(int i=0; i<4; i++)
        {
            for(int n=0; n<4; n++)
            {
                quadric[i][n] = plane[i] * plane[n];
            }
        }
    }

    Real LodCollapseCostQuadric::computeEdgeCollapseCost( LodData* data, LodData::Vertex* src, LodData::Edge* dstEdge )
    {
        LodData::Vertex* dst = dstEdge->dst;

        if (isBorderVertex(src))
        {
            return LodData::NEVER_COLLAPSE_COST;
        }
        if (src->seam && !dst->seam)
        {
            return LodData::NEVER_COLLAPSE_COST;
        }

        Matrix4 Qnew = mVertexQuadricList[LodData::getVectorIDFromPointer(data->mVertexList, src)] +
                       mVertexQuadricList[LodData::getVectorIDFromPointer(data->mVertexList, dst)];

        Vector4 Vnew(dst->position);

        // error = Vnew^T * Qnew * Vnew
        Real cost =
            (Vnew[0] * Qnew[0][0] + Vnew[1] * Qnew[0][1] + Vnew[2] * Qnew[0][2] + Vnew[3] * Qnew[0][3]) * Vnew[0] +
            (Vnew[0] * Qnew[1][0] + Vnew[1] * Qnew[1][1] + Vnew[2] * Qnew[1][2] + Vnew[3] * Qnew[1][3]) * Vnew[1] +
            (Vnew[0] * Qnew[2][0] + Vnew[1] * Qnew[2][1] + Vnew[2] * Qnew[2][2] + Vnew[3] * Qnew[2][3]) * Vnew[2] +
            (Vnew[0] * Qnew[3][0] + Vnew[1] * Qnew[3][1] + Vnew[2] * Qnew[3][2] + Vnew[3] * Qnew[3][3]) * Vnew[3];

        if (dst->seam)
        {
            cost *= 8;
        }

        return cost;

    }

    void LodCollapseCostQuadric::computeVertexQuadric( LodData* data, size_t vertexID )
    {
        Matrix4& quadric = mVertexQuadricList[vertexID];
        quadric = Matrix4::ZERO;
        LodData::Vertex& vertex = data->mVertexList[vertexID];
        LodData::VTriangles::iterator tri, triEnd;
        tri = vertex.triangles.begin();
        triEnd = vertex.triangles.end();
        for (; tri != triEnd; ++tri)
        {
            size_t id = LodData::getVectorIDFromPointer(data->mTriangleList, *tri);
            quadric = quadric + mTrianglePlaneQuadricList[id];
        }
    }

    void LodCollapseCostQuadric::updateVertexCollapseCost( LodData* data, LodData::Vertex* vertex )
    {
        computeVertexQuadric(data, LodData::getVectorIDFromPointer(data->mVertexList, vertex));

        LodCollapseCost::updateVertexCollapseCost(data, vertex);
    }



}
