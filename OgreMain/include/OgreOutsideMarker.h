/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2013 Torus Knot Software Ltd

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
#ifndef __OutsideMarker_H__
#define __OutsideMarker_H__

#include "OgrePrerequisites.h"
#include "OgreProgressiveMeshGenerator.h"
#include "OgreResourceGroupManager.h"
#include <vector>
#include "OgreHeaderPrefix.h"

namespace Ogre
{

/// This class will mark vertices of a mesh, which are visible from far away (from outside).
/// Triangle is visible if each vertex of it is visible.
class _OgreExport OutsideMarker
{
public:
	/**
	 * @param vertexList List of vertices. vertex.position is the input and vertex.isOuterWallVertex is the output of the algorithm.
	 * @param walkAngle Walk angle in dot product values. Allowed range is from -1 to 1. Default = 0. Smaller value is bigger angle.
	 *        If you set it to 1 then you can disable walking and it will only mark the vertices on the convex hull.
	 */
	OutsideMarker(ProgressiveMeshGenerator::VertexList & vertexList, Real boundingSphereRadius, Real walkAngle, int step);
	void markOutside(); /// Mark vertices, which are visible from outside.

private:
	typedef ProgressiveMeshGenerator::PMVertex CHVertex;

	struct CHTriangle {
		bool removed; // Whether the triangle is excluded from hull.
		CHVertex* vertex[3];
		Vector3 normal;
		void computeNormal();
	};

	typedef std::vector<CHTriangle> CHTriangleList;
	typedef std::vector<CHTriangle*> CHTrianglePList;
	typedef std::vector<std::pair<CHVertex*, CHVertex*> > CHEdgeList;

	CHTriangleList mHull; /// Contains the current triangles of the convex hull.
	CHTrianglePList mVisibleTriangles; /// Temporary vector for addVisibleEdges function (prevent allocation every call).
	CHEdgeList mEdges; /// Temporary vector for the horizon edges, when inserting a new vertex into the hull.
	ProgressiveMeshGenerator::VertexList& mVertexListOrig; /// Source of input and output of the algorithm.
	Vector3 mCentroid; /// Centroid of the convex hull.
	Real mWalkAngle; /// Angle limit, when walking inside for marking vertices.
	int mStep;

	void initHull(); /// Initializes the hull for expansion.
	void createTriangle(CHVertex* v1, CHVertex* v2, CHVertex* v3); /// Sets the vertices of a triangle (called from initHull only).
	Real getTetrahedronVolume(CHVertex* v0, CHVertex* v1, CHVertex* v2, CHVertex* v3);
	Real getPointToLineSqraredDistance(CHVertex* x1, CHVertex* x2, CHVertex* vertex);
	void generateHull(); /// Generates the hull.
	int addVertex(CHVertex* vertex); /// Adds vertex to hull.
	void addEdge(CHEdgeList& edges, CHVertex* a, CHVertex* b); /// Add edge to the list of removable edges.
	void cleanHull(); /// Removes Triangles, which are having CHTriangle::removed = true.
	bool isVisible(CHTriangle* triangle, Vector3& vertex); /// Whether face is visible from point.
	CHVertex* getFurthestVertex(CHTriangle* hull); /// Gets furthest vertex from face.
	void getVisibleTriangles(const CHVertex* target, CHTrianglePList& visibleTriangles); /// Adds visible edges to the list, when viewing from target point.
	void getHorizon(const CHTrianglePList& tri, CHEdgeList& ); /// Removes edges, which are not on the horizon.
	void fillHorizon(CHEdgeList& e, CHVertex* target); /// Caps the hole with faces connecting to target vertex.
	void markVertices(); /// if we have the convex hull, this will walk on the faces which have less then 90 degree difference.
	template<typename T>
	void addHullTriangleVertices(std::vector<CHVertex*>& stack, T tri); /// Add triangle to stack (called from markVertices only).
};
}

#endif /* ifndef __OutsideMarker_H__ */