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

#include "OgreLodCollapseCostCurvature.h"
#include "OgreVector3.h"

namespace Ogre
{
    Real LodCollapseCostCurvature::computeEdgeCollapseCost( LodData* data, LodData::Vertex* src, LodData::Edge* dstEdge )
    {
        LodData::Vertex* dst = dstEdge->dst;

        // Degenerate case check
        // Are we going to invert a face normal of one of the neighboring faces?
        // Can occur when we have a very small remaining edge and collapse crosses it
        // Look for a face normal changing by > 90 degrees
        if(MESHLOD_QUALITY >= 2)   // 30% speedup if disabled.
        {
            LodData::VTriangles::iterator it = src->triangles.begin();
            LodData::VTriangles::iterator itEnd = src->triangles.end();
            for (; it != itEnd; ++it)
            {
                LodData::Triangle* triangle = *it;
                // Ignore the deleted faces (those including src & dest)
                if (!triangle->hasVertex(dst))
                {
                    // Test the new face normal
                    LodData::Vertex* pv0, * pv1, * pv2;

                    // Replace src with dest wherever it is
                    pv0 = (triangle->vertex[0] == src) ? dst : triangle->vertex[0];
                    pv1 = (triangle->vertex[1] == src) ? dst : triangle->vertex[1];
                    pv2 = (triangle->vertex[2] == src) ? dst : triangle->vertex[2];

                    // Cross-product 2 edges
                    Vector3 e1 = pv1->position - pv0->position;
                    Vector3 e2 = pv2->position - pv1->position;

                    Vector3 newNormal = e1.crossProduct(e2);

                    // Dot old and new face normal
                    // If < 0 then more than 90 degree difference
                    if (newNormal.dotProduct(triangle->normal) < 0.0f)
                    {
                        // Don't do it!
                        return LodData::NEVER_COLLAPSE_COST;
                    }
                }
            }
        }

        Real cost;

        // Special cases
        // If we're looking at a border vertex
        if (isBorderVertex(src))
        {
            if (dstEdge->refCount > 1)
            {
                // src is on a border, but the src-dest edge has more than one tri on it
                // So it must be collapsing inwards
                // Mark as very high-value cost
                // curvature = 1.0f;
                cost = 1.0f;
            }
            else
            {
                // Collapsing ALONG a border
                // We can't use curvature to measure the effect on the model
                // Instead, see what effect it has on 'pulling' the other border edges
                // The more colinear, the less effect it will have
                // So measure the 'kinkiness' (for want of a better term)

                // Find the only triangle using this edge.
                // PMTriangle* triangle = findSideTriangle(src, dst);

                cost = -1.0f;
                Vector3 collapseEdge = src->position - dst->position;
                collapseEdge.normalise();
                LodData::VEdges::iterator it = src->edges.begin();
                LodData::VEdges::iterator itEnd = src->edges.end();
                for (; it != itEnd; it++)
                {
                    LodData::Vertex* neighbor = it->dst;
                    if (neighbor != dst && it->refCount == 1)
                    {
                        Vector3 otherBorderEdge = src->position - neighbor->position;
                        otherBorderEdge.normalise();
                        // This time, the nearer the dot is to -1, the better, because that means
                        // the edges are opposite each other, therefore less kinkiness
                        // Scale into [0..1]
                        Real kinkiness = otherBorderEdge.dotProduct(collapseEdge);
                        cost = std::max<Real>(cost, kinkiness);
                    }
                }
                cost = (1.002f + cost) * 0.5f;
            }
        }
        else     // not a border
        {

            // Standard inner vertex
            // Calculate curvature
            // use the triangle facing most away from the sides
            // to determine our curvature term
            // Iterate over src's faces again
            cost = 1.0f;
            LodData::VTriangles::iterator it = src->triangles.begin();
            LodData::VTriangles::iterator itEnd = src->triangles.end();
            for (; it != itEnd; ++it)
            {
                Real mincurv = -1.0f; // curve for face i and closer side to it
                LodData::Triangle* triangle = *it;
                LodData::VTriangles::iterator it2 = src->triangles.begin();
                for (; it2 != itEnd; ++it2)
                {
                    LodData::Triangle* triangle2 = *it2;
                    if (triangle2->hasVertex(dst))
                    {

                        // Dot product of face normal gives a good delta angle
                        Real dotprod = triangle->normal.dotProduct(triangle2->normal);
                        // NB we do (1-..) to invert curvature where 1 is high curvature [0..1]
                        // Whilst dot product is high when angle difference is low
                        mincurv = std::max<Real>(mincurv, dotprod);
                    }
                }
                cost = std::min<Real>(cost, mincurv);
            }
            cost = (1.002f - cost) * 0.5f;
        }

        // check for texture seam ripping and multiple submeshes
        if (src->seam)
        {
            if (!dst->seam)
            {
                cost = std::max<Real>(cost, (Real)0.05f);
                cost *= 64;
            }
            else
            {
                if(MESHLOD_QUALITY >= 3)
                {
                    int seamNeighbors = 0;
                    LodData::Vertex* otherSeam;
                    LodData::VEdges::iterator it = src->edges.begin();
                    LodData::VEdges::iterator itEnd = src->edges.end();
                    for (; it != itEnd; it++)
                    {
                        LodData::Vertex* neighbor = it->dst;
                        if(neighbor->seam)
                        {
                            seamNeighbors++;
                            if(neighbor != dst)
                            {
                                otherSeam = neighbor;
                            }
                        }
                    }
                    if(seamNeighbors != 2 || (seamNeighbors == 2 && dst->edges.has(LodData::Edge(otherSeam))))
                    {
                        cost = std::max<Real>(cost, 0.05f);
                        cost *= 64;
                    }
                    else
                    {
                        cost = std::max<Real>(cost, 0.005f);
                        cost *= 8;
                    }
                }
                else
                {
                    cost = std::max<Real>(cost, 0.005f);
                    cost *= 8;
                }

            }
        }

        Real diff = src->normal.dotProduct(dst->normal) / 8.0f;
        Real dist = src->position.distance(dst->position);
        cost = cost * dist;
        if(data->mUseVertexNormals)
        {
            //Take into account vertex normals.
            Real normalCost = 0;
            LodData::VEdges::iterator it = src->edges.begin();
            LodData::VEdges::iterator itEnd = src->edges.end();
            for (; it != itEnd; ++it)
            {
                LodData::Vertex* neighbor = it->dst;
                Real beforeDist = neighbor->position.distance(src->position);
                Real afterDist = neighbor->position.distance(dst->position);
                Real beforeDot = neighbor->normal.dotProduct(src->normal);
                Real afterDot = neighbor->normal.dotProduct(dst->normal);
                normalCost = std::max<Real>(normalCost, std::max<Real>(diff, std::abs(beforeDot - afterDot)) *
                                            std::max<Real>(afterDist/8.0f, std::max<Real>(dist, std::abs(beforeDist - afterDist))));
            }
            cost = std::max<Real>(normalCost * 0.25f, cost);
        }

        OgreAssert(cost >= 0 && cost != LodData::UNINITIALIZED_COLLAPSE_COST, "Invalid collapse cost");
        return cost;
    }
}

