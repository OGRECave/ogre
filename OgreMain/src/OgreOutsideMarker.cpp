/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#include "OgreStableHeaders.h"
#include "OgreOutsideMarker.h"
#include "OgreMeshManager.h"
#include "OgreRoot.h"
#include "OgreHardwareBufferManager.h"
#include "OgreSubMesh.h"

namespace Ogre
{

OutsideMarker::OutsideMarker(ProgressiveMeshGenerator::VertexList & vertexList, Real boundingSphereRadius, Real walkAngle, int step) :
	mVertexListOrig(vertexList),
	mWalkAngle(walkAngle),
	mStep(step)
{
	assert(!vertexList.empty());
	assert(mWalkAngle >= -1.0001f && mWalkAngle <= 1.0001f); // dot product range
	assert(boundingSphereRadius > 0.0f); // invalid meshes are having a bounding sphere radius of 0.0!
}

void OutsideMarker::markOutside()
{
	generateHull();
	markVertices();
}

void OutsideMarker::generateHull()
{
	initHull();
	{
		for (size_t i = 0; i < mHull.size(); i++) {
			if (!mHull[i].removed) {
				CHVertex* furthestVertex = getFurthestVertex(&mHull[i]);
				if (!furthestVertex) {
					continue;
				}
				addVertex(furthestVertex);
			}
		}
	}
	cleanHull();
}

void OutsideMarker::initHull()
{
	mHull.clear();
	mHull.reserve(mVertexListOrig.size());
	ProgressiveMeshGenerator::VertexList::iterator v, vEnd;
	v = mVertexListOrig.begin();
	vEnd = mVertexListOrig.end();
	for (; v != vEnd; v++) {
		// reset output variables
		v->isOuterWallVertex = false;
		v->isInsideHull = false;
	}
	// We need to find 4 vertices, which are on the convex hull to start the algorithm.
	CHVertex* vertex[4] = { nullptr, nullptr, nullptr, nullptr };

	{
		// Get 1. vertex: minimum y vertex
		Real miny = std::numeric_limits<Real>::max();
		v = mVertexListOrig.begin();
		vEnd = mVertexListOrig.end();
		for (; v != vEnd; v++) {
			Vector3& pos = v->position;
			if (pos.y < miny) {
				miny = pos.y;
				vertex[0] = &*v;
			}
		}
		assert(vertex[0]); // Vertex not found!
	}

	{
		// Get 2. vertex: furthest from 1. vertex
		Real maxdist = 0.0;
		v = mVertexListOrig.begin();
		vEnd = mVertexListOrig.end();
		for (; v != vEnd; v++) {
			Real dist = vertex[0]->position.squaredDistance(v->position);
			if (dist > maxdist) {
				maxdist = dist;
				vertex[1] = &*v;
			}
		}
		assert(vertex[1]); // Vertex not found!
	}

	{
		// Get 3. vertex: furthest from 1. vertex and 2. vertex
		Real maxdist = 0.0;
		v = mVertexListOrig.begin();
		vEnd = mVertexListOrig.end();
		for (; v != vEnd; v++) {
			Real dist = getPointToLineSqraredDistance(vertex[0], vertex[1], &*v);
			if (dist > maxdist) {
				maxdist = dist;
				vertex[2] = &*v;
			}
		}
		assert(vertex[2]); // Vertex not found!
	}

	{
		// Get 4. vertex: furthest from 1-2-3 triangle
		Real maxdist = 0.0f;
		Plane plane(vertex[0]->position, vertex[1]->position, vertex[2]->position);
		plane.normalise();
		v = mVertexListOrig.begin();
		vEnd = mVertexListOrig.end();
		for (; v != vEnd; v++) {
			Real dist = std::abs(plane.getDistance(v->position));
			if (dist > maxdist) {
				maxdist = dist;
				vertex[3] = &*v;
			}
		}
		assert(vertex[3]); // Vertex not found!
	}

	// Volume should be bigger than 0, so that we can guarantee that the centroid point is inside the hull
	assert(getTetrahedronVolume(vertex[0], vertex[1], vertex[2], vertex[3]) > 0);
	// Centroid = (a + b + c + d) / 4
	mCentroid = vertex[0]->position + vertex[1]->position + vertex[2]->position + vertex[3]->position;
	mCentroid /= 4.0f;

	// Mark vertices, so that they will not be processed again.
	vertex[0]->isInsideHull = true;
	vertex[1]->isInsideHull = true;
	vertex[2]->isInsideHull = true;
	vertex[3]->isInsideHull = true;

	// Create the triangles
	createTriangle(vertex[0], vertex[1], vertex[2]);
	createTriangle(vertex[0], vertex[1], vertex[3]);
	createTriangle(vertex[0], vertex[2], vertex[3]);
	createTriangle(vertex[1], vertex[2], vertex[3]);
}

void OutsideMarker::createTriangle( CHVertex* v1, CHVertex* v2, CHVertex* v3 )
{
	CHTriangle tri;
	tri.removed = false;
	tri.vertex[0] = v1;
	tri.vertex[1] = v2;
	tri.vertex[2] = v3;
	tri.computeNormal();
	if (isVisible(&tri, mCentroid)) {
		std::swap(tri.vertex[0], tri.vertex[1]);
		tri.computeNormal();
	}
	mHull.push_back(tri);
}

Real OutsideMarker::getPointToLineSqraredDistance(CHVertex* x1, CHVertex* x2, CHVertex* p)
{
	Real up = ((x2->position - x1->position).crossProduct(x1->position - p->position)).squaredLength();
	Real down = (x2->position - x1->position).squaredLength();
	return up / down;
}

Real OutsideMarker::getTetrahedronVolume(CHVertex* a, CHVertex* b, CHVertex* c, CHVertex* d)
{
	// V = |(a-d)*[(b-d)x(c-d)]| / 6
	return std::abs((a->position - d->position).dotProduct((b->position - d->position).crossProduct(c->position - d->position))) / 6.0f;
}

OutsideMarker::CHVertex* OutsideMarker::getFurthestVertex(CHTriangle* tri)
{
	// Find the furthest vertex from triangle plane towards the facing direction.
	CHVertex* furthestVertex = nullptr;
	Real furthestDistance = 0;
	Plane plane(tri->normal, -tri->normal.dotProduct(tri->vertex[0]->position));
	plane.normalise();
	ProgressiveMeshGenerator::VertexList::iterator v, vEnd;
	v = mVertexListOrig.begin();
	vEnd = mVertexListOrig.end();
	for (; v != vEnd; v++) {
		if (v->isInsideHull) {
			continue;
		}
		Real dist = plane.getDistance(v->position);
		if (dist > furthestDistance) {
			furthestDistance = dist;
			furthestVertex = &*v;
		}
	}
	return furthestVertex;
}

int OutsideMarker::addVertex( CHVertex* vertex )
{
	vertex->isInsideHull = true;
	// We use class members for triangles and edges only to prevent allocation.
	mVisibleTriangles.clear();
	mEdges.clear();
	getVisibleTriangles(vertex, mVisibleTriangles); // get the hull triangles, which are facing the vertex.
	if (mVisibleTriangles.empty()) {
		return 0; // It's inside the Hull.
	}
	getHorizon(mVisibleTriangles, mEdges); // Returns the edges of a circle around the triangles. Also removes the triangles
	fillHorizon(mEdges, vertex); // Creates triangles from the edges towards the vertex.
	return mEdges.size();
}

void OutsideMarker::getVisibleTriangles( const CHVertex* target, CHTrianglePList& visibleTriangles )
{

	CHTriangleList::iterator it = mHull.begin();
	CHTriangleList::iterator itEnd = mHull.end();
	for (; it != itEnd; it++) {
		if (it->removed) {
			continue;
		}
		Real dot1 = it->normal.dotProduct(it->vertex[0]->position);
		Real dot2 = it->normal.dotProduct(target->position);
		if (dot1 < dot2) {
			visibleTriangles.push_back(&*it);
		}
	}
}

void OutsideMarker::addEdge(CHEdgeList& edges, CHVertex* a, CHVertex* b)
{
	// We need undirectional edges, so we sort the vertices by their pointer like integers.
	// This is needed, because we want to remove edges which were inside multiple triangles,
	// because those are not on the horizon.
	if (a <= b) {
		edges.push_back(CHEdgeList::value_type(a, b));
	} else {
		edges.push_back(CHEdgeList::value_type(b, a));
	}
}

void OutsideMarker::getHorizon( const CHTrianglePList& tri, CHEdgeList& horizon)
{
	// Create edge list and remove triangles
	CHTrianglePList::const_iterator it2 = tri.begin();
	CHTrianglePList::const_iterator it2End = tri.end();
	for (; it2 != it2End; it2++) {
		addEdge(horizon, (*it2)->vertex[0], (*it2)->vertex[1]);
		addEdge(horizon, (*it2)->vertex[1], (*it2)->vertex[2]);
		addEdge(horizon, (*it2)->vertex[2], (*it2)->vertex[0]);
		(*it2)->removed = true;
	}
	// inside edges are twice in the edge list, because it was added by 2 triangles.
	assert(!horizon.empty());
	std::sort(horizon.begin(), horizon.end());
	int end = horizon.size();
	int first = 0;
	int last = 0;
	int result = 0;
	// Removes edges, which are 2+ times in the sorted edge list.
	while (++first != end) {
		if (!(horizon[result] == horizon[first])) {
			if (!(horizon[last] == horizon[first])) {
				int dist = first - last;
				if (dist == 1) {
					horizon[result++] = horizon[last];
				}
				last = first;
			}
		}
	}
	if (first - last == 1) {
		horizon[result++] = horizon[last];
	}
	horizon.resize(result);
}

void OutsideMarker::fillHorizon(CHEdgeList& horizon, CHVertex* target)
{
	CHTriangle tri;
	tri.vertex[2] = target;
	tri.removed = false;
	CHEdgeList::iterator it = horizon.begin();
	CHEdgeList::iterator itEnd = horizon.end();
	for (;it != itEnd; it++) {
		tri.vertex[0] = it->first;
		tri.vertex[1] = it->second;
		tri.computeNormal();
		if (isVisible(&tri, mCentroid)) {
			std::swap(tri.vertex[0], tri.vertex[1]);
			tri.computeNormal();
		}
		mHull.push_back(tri);
	}
}

bool OutsideMarker::isVisible(CHTriangle* t, Vector3& v)
{
	// We don't need epsilon here, because we assume, that the centroid is not on the triangle.
	return t->normal.dotProduct(t->vertex[0]->position) < t->normal.dotProduct(v);
}

void OutsideMarker::cleanHull()
{
	// cleanHull will remove triangles, which are marked as "removed"
	// For fast performance, it will swap last item into the place of removed items.
	int end = mHull.size() - 1;
	int start = 0;
	while (start <= end) {
		if (mHull[start].removed) {
			// Replace removed item with last item
			mHull[start].removed = mHull[end].removed;
			mHull[start].vertex[0] = mHull[end].vertex[0];
			mHull[start].vertex[1] = mHull[end].vertex[1];
			mHull[start].vertex[2] = mHull[end].vertex[2];
			mHull[start].normal = mHull[end].normal;
			end--;
		} else {
			start++;
		}
	}
	end++;
	mHull.resize(end);
}

template<typename T>
void OutsideMarker::addHullTriangleVertices(std::vector<CHVertex*>& stack, T tri)
{
	if (!tri->vertex[0]->isOuterWallVertexInPass) {
		tri->vertex[0]->isOuterWallVertexInPass = true;
		tri->vertex[0]->isOuterWallVertex = true;
		stack.push_back(tri->vertex[0]);
	}
	if (!tri->vertex[1]->isOuterWallVertexInPass) {
		tri->vertex[1]->isOuterWallVertexInPass = true;
		tri->vertex[1]->isOuterWallVertex = true;
		stack.push_back(tri->vertex[1]);
	}
	if (!tri->vertex[2]->isOuterWallVertexInPass) {
		tri->vertex[2]->isOuterWallVertexInPass = true;
		tri->vertex[2]->isOuterWallVertex = true;
		stack.push_back(tri->vertex[2]);
	}
}

void OutsideMarker::markVertices()
{
	ProgressiveMeshGenerator::VertexList::iterator v, vEnd;
	v = mVertexListOrig.begin();
	vEnd = mVertexListOrig.end();
	for (;v != vEnd; v++) {
		v->isOuterWallVertex = false;
	}
	std::vector<CHVertex*> stack;
	CHTriangleList::iterator tri, triEnd;
	tri = mHull.begin();
	triEnd = mHull.end();
	for (; tri != triEnd; tri++) {
		stack.clear();
		v = mVertexListOrig.begin();
		vEnd = mVertexListOrig.end();
		for (; v != vEnd; v++) {
			v->isOuterWallVertexInPass = false;
		}
		addHullTriangleVertices(stack, &*tri);
		while (!stack.empty()) {
			CHVertex* vert = stack.back();
			stack.pop_back();
			ProgressiveMeshGenerator::VTriangles::iterator it, itEnd;
			it = vert->triangles.begin();
			itEnd = vert->triangles.end();
			for (; it != itEnd; it++) {
				if (tri->normal.dotProduct((*it)->normal) > mWalkAngle) {
					addHullTriangleVertices(stack, *it);
				}
			}
		}
	}
}

void OutsideMarker::CHTriangle::computeNormal()
{
	Vector3 e1 = vertex[1]->position - vertex[0]->position;
	Vector3 e2 = vertex[2]->position - vertex[1]->position;

	normal = e1.crossProduct(e2);
	normal.normalise();
}

}
