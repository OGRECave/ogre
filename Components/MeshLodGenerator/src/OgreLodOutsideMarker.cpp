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
#include "OgreMeshLodPrecompiledHeaders.h"

namespace Ogre
{

LodOutsideMarker::LodOutsideMarker(LodData::VertexList & vertexList, Real boundingSphereRadius, Real walkAngle) :
    mEpsilon(boundingSphereRadius * std::numeric_limits<Real>::epsilon() * (Real)4.0), // How much floating point math error you have for equal values. This may depend on compiler flags.
    mVertexListOrig(vertexList),
    mWalkAngle(walkAngle)
{
    assert(!vertexList.empty());
    assert(mWalkAngle >= -1.0001f && mWalkAngle <= 1.0001f); // dot product range
    assert(boundingSphereRadius > 0.0f); // invalid meshes are having a bounding sphere radius of 0.0!
}

void LodOutsideMarker::markOutside()
{
    generateHull();
    markVertices();
}

void LodOutsideMarker::generateHull()
{
    initHull();
    {
        for (auto & t : mHull) {
            if (!t.removed) {
                CHVertex* furthestVertex = getFurthestVertex(&t);
                if (!furthestVertex) {
                    continue;
                }
                addVertex(furthestVertex);
            }
        }
    }
    cleanHull();
}

void LodOutsideMarker::initHull()
{
    mHull.clear();
    mHull.reserve(mVertexListOrig.size());
    mOutsideData.clear();
    OutsideDataList::iterator itOut, itOutEnd;
    mOutsideData.resize(mVertexListOrig.size());
    itOut = mOutsideData.begin();
    itOutEnd = mOutsideData.end();
    for (; itOut != itOutEnd; itOut++) {
        // reset output variables
        itOut->isOuterWallVertex = false;
        itOut->isInsideHull = false;
    }
    // We need to find 4 vertices, which are on the convex hull to start the algorithm.
    CHVertex* vertex[4] = { NULL, NULL, NULL, NULL };

    {
        // Get 1. vertex: minimum y vertex
        Real miny = std::numeric_limits<Real>::max();
        LodData::VertexList::iterator v, vEnd;
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
        LodData::VertexList::iterator v, vEnd;
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
        LodData::VertexList::iterator v, vEnd;
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
        LodData::VertexList::iterator v, vEnd;
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
    assert(getTetrahedronVolume(vertex[0], vertex[1], vertex[2], vertex[3]) > mEpsilon);
    // Centroid = (a + b + c + d) / 4
    mCentroid = vertex[0]->position + vertex[1]->position + vertex[2]->position + vertex[3]->position;
    mCentroid /= 4.0f;

    // Mark vertices, so that they will not be processed again.
    getOutsideData(vertex[0])->isInsideHull = true;
    getOutsideData(vertex[1])->isInsideHull = true;
    getOutsideData(vertex[2])->isInsideHull = true;
    getOutsideData(vertex[3])->isInsideHull = true;

    // Create the triangles
    createTriangle(vertex[0], vertex[1], vertex[2]);
    createTriangle(vertex[0], vertex[1], vertex[3]);
    createTriangle(vertex[0], vertex[2], vertex[3]);
    createTriangle(vertex[1], vertex[2], vertex[3]);
}

void LodOutsideMarker::createTriangle( CHVertex* v1, CHVertex* v2, CHVertex* v3 )
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

Real LodOutsideMarker::getPointToLineSqraredDistance(CHVertex* x1, CHVertex* x2, CHVertex* p)
{
    Real up = ((x2->position - x1->position).crossProduct(x1->position - p->position)).squaredLength();
    Real down = (x2->position - x1->position).squaredLength();
    return up / down;
}

Real LodOutsideMarker::getTetrahedronVolume(CHVertex* a, CHVertex* b, CHVertex* c, CHVertex* d)
{
    // V = |(a-d)*[(b-d)x(c-d)]| / 6
    return std::abs((a->position - d->position).dotProduct((b->position - d->position).crossProduct(c->position - d->position))) / 6.0f;
}

LodOutsideMarker::CHVertex* LodOutsideMarker::getFurthestVertex(CHTriangle* tri)
{
    // Find the furthest vertex from triangle plane towards the facing direction.
    CHVertex* furthestVertex = NULL;
    Real furthestDistance = 0;
    Plane plane(tri->normal, -tri->normal.dotProduct(tri->vertex[0]->position));
    plane.normalise();
    LodData::VertexList::iterator v, vEnd;
    v = mVertexListOrig.begin();
    vEnd = mVertexListOrig.end();
    for (; v != vEnd; v++) {
        if (getOutsideData(&*v)->isInsideHull) {
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

size_t LodOutsideMarker::addVertex( CHVertex* vertex )
{
    getOutsideData(vertex)->isInsideHull = true;
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

void LodOutsideMarker::getVisibleTriangles( const CHVertex* target, CHTrianglePList& visibleTriangles )
{

    CHTriangleList::iterator it = mHull.begin();
    CHTriangleList::iterator itEnd = mHull.end();
    for (; it != itEnd; it++) {
        if (it->removed) {
            continue;
        }
        Real dot1 = it->normal.dotProduct(it->vertex[0]->position);
        Real dot2 = it->normal.dotProduct(target->position);
        if(std::abs(dot2 - dot1) <= mEpsilon) {
            //Special case: The vertex is on the plane of the triangle
            //mVisibleTriangles.push_back(&*it);
            if (isInsideTriangle(target->position, *it)) {
                // Vertex is inside of a convex hull triangle.
                mVisibleTriangles.clear();
                return;
            } else {
                // If the vertex is outside, then we should add it to the hull.
                visibleTriangles.push_back(&*it);
            }
        } else if (dot1 < dot2) {
            visibleTriangles.push_back(&*it);
        }
    }
}

void LodOutsideMarker::addEdge(CHEdgeList& edges, CHVertex* a, CHVertex* b)
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

bool LodOutsideMarker::isInsideTriangle(const Vector3& ptarget, const CHTriangle& tri){

    // The idea is that if all angle is smaller to that point, than it should be inside.
    // NOTE: We assume that the vertex is on the triangle plane!

    const Vector3& p0 = tri.vertex[0]->position;
    const Vector3& p1 = tri.vertex[1]->position;
    const Vector3& p2 = tri.vertex[2]->position;
    const Vector3& n = tri.normal;

    bool b0, b1, b2;

    // It should not contain malformed triangles!
    assert(!isSamePosition(p0, p1) && !isSamePosition(p1, p2) && !isSamePosition(p2, p0));

    {
        Real d0 = pointToLineDir(ptarget, p0, p1, p2, n);
        if (std::abs(d0) <= mEpsilon){
            //Vertex is on the edge, so we need to compare length.
            return isInsideLine(ptarget, p0, p1);
        }
        b0 = d0 < 0.0f;
    }

    {
        Real d1 = pointToLineDir(ptarget, p1, p2, p0, n);
        if (std::abs(d1) <= mEpsilon){
            //Vertex is on the edge, so we need to compare length.
            return isInsideLine(ptarget, p1, p2);
        }
        b1 = d1 < 0.0f;
    }

    if (b0 != b1) {
        return false;
    }

    {
        Real d2 = pointToLineDir(ptarget, p2, p0, p1, n);
        if (std::abs(d2) <= mEpsilon){
            //Vertex is on the edge, so we need to compare length.
            return isInsideLine(ptarget, p2, p0);
        }
        b2 = d2 < 0.0f;
    }

    return (b1 == b2);
}

bool LodOutsideMarker::isSamePosition( const Vector3& p0, const Vector3& p1 )
{
    return (std::abs(p0.x - p1.x) <= mEpsilon && std::abs(p0.y - p1.y) <= mEpsilon && std::abs(p0.z - p1.z) <= mEpsilon);
}

Real LodOutsideMarker::pointToLineDir(const Vector3& ptarget, const Vector3& p0, const Vector3& p1, const Vector3& p2, const Vector3& n)
{
    return n.crossProduct(p1 - p0).dotProduct(ptarget - p0);
}

bool LodOutsideMarker::isInsideLine( const Vector3& ptarget, const Vector3& p0, const Vector3& p1 )
{
    // This function returns whether ptarget is between p0 and p1.
    // It is outside if:
    // -the dir vector is in the opposite direction
    // -the length is smaller.
    // -ptarget is not p0 or p1
    // NOTE: We assume that the 3 points are on the same line!

    Vector3 v1 = p1 - p0;
    Vector3 v2 = ptarget - p0;
    Real len1 = v1.squaredLength();
    Real len2 = v2.squaredLength();
    Real dot = v1.dotProduct(v2);
    return isSamePosition(ptarget, p1) || (
        dot > 0.0 // Same direction
        && len1 > len2); // Shorter
}

void LodOutsideMarker::getHorizon( const CHTrianglePList& tri, CHEdgeList& horizon)
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
    size_t end = horizon.size();
    size_t first = 0;
    size_t last = 0;
    size_t result = 0;
    // Removes edges, which are 2+ times in the sorted edge list.
    while (++first != end) {
        if (!(horizon[result] == horizon[first])) {
            if (!(horizon[last] == horizon[first])) {
                size_t dist = first - last;
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

void LodOutsideMarker::fillHorizon(CHEdgeList& horizon, CHVertex* target)
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

bool LodOutsideMarker::isVisible(CHTriangle* t, Vector3& v)
{
    // We don't need epsilon here, because we assume, that the centroid is not on the triangle.
    return t->normal.dotProduct(t->vertex[0]->position) < t->normal.dotProduct(v);
}

void LodOutsideMarker::cleanHull()
{
    // cleanHull will remove triangles, which are marked as "removed"
    // For fast performance, it will swap last item into the place of removed items.
    size_t end = mHull.size() - 1;
    size_t start = 0;
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

Ogre::MeshPtr LodOutsideMarker::createConvexHullMesh(const String& meshName, const String& resourceGroupName)
{
    // Based on the wiki sample: http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Generating+A+Mesh
    OgreAssert(!MeshManager::getSingleton().getByName(meshName, resourceGroupName), "Resource with given name should not exist");

    generateHull(); // calculate mHull triangles.

    // Convex hull can't be empty!
    assert(!mHull.empty());

    MeshPtr mesh = MeshManager::getSingleton().createManual(meshName, resourceGroupName, NULL);
    SubMesh* subMesh = mesh->createSubMesh();

    std::vector<float> vertexBuffer;
    std::vector<unsigned short> indexBuffer;
    // 3 position/triangle * 3 Real/position
    vertexBuffer.reserve(mHull.size() * 9);
    // 3 index / triangle
    indexBuffer.reserve(mHull.size() * 3);
    int id=0;
    // min & max position
    AxisAlignedBox bounds;

    for (auto & t : mHull) {
        assert(!t.removed);
        for(size_t n = 0; n < 3; n++){
            indexBuffer.push_back(id++);
            vertexBuffer.push_back(t.vertex[n]->position.x);
            vertexBuffer.push_back(t.vertex[n]->position.y);
            vertexBuffer.push_back(t.vertex[n]->position.z);
            bounds.merge(t.vertex[n]->position);
        }
    }

    /// Create vertex data structure for 8 vertices shared between submeshes
    mesh->sharedVertexData = new VertexData();
    mesh->sharedVertexData->vertexCount = mHull.size() * 3;

    /// Create declaration (memory format) of vertex data
    VertexDeclaration* decl = mesh->sharedVertexData->vertexDeclaration;
    size_t offset = 0;
    // 1st buffer
    offset += decl->addElement(0, offset, VET_FLOAT3, VES_POSITION).getSize();

    /// Allocate vertex buffer of the requested number of vertices (vertexCount) 
    /// and bytes per vertex (offset)
    HardwareVertexBufferSharedPtr vbuf = 
        HardwareBufferManager::getSingleton().createVertexBuffer(
        offset, mesh->sharedVertexData->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY);
    /// Upload the vertex data to the card
    vbuf->writeData(0, vbuf->getSizeInBytes(), &vertexBuffer[0], true);

    /// Set vertex buffer binding so buffer 0 is bound to our vertex buffer
    VertexBufferBinding* bind = mesh->sharedVertexData->vertexBufferBinding; 
    bind->setBinding(0, vbuf);

    /// Allocate index buffer of the requested number of vertices (ibufCount) 
    HardwareIndexBufferSharedPtr ibuf = HardwareBufferManager::getSingleton().
        createIndexBuffer(
        HardwareIndexBuffer::IT_16BIT, 
        indexBuffer.size(), 
        HardwareBuffer::HBU_STATIC_WRITE_ONLY);

    /// Upload the index data to the card
    ibuf->writeData(0, ibuf->getSizeInBytes(), &indexBuffer[0], true);

    /// Set parameters of the submesh
    subMesh->useSharedVertices = true;
    subMesh->indexData->indexBuffer = ibuf;
    subMesh->indexData->indexCount = indexBuffer.size();
    subMesh->indexData->indexStart = 0;

    /// Set bounding information (for culling)
    mesh->_setBounds(bounds);
    mesh->_setBoundingSphereRadius(bounds.getSize().length() / 2.0f);

    /// Set material to transparent blue
    subMesh->setMaterialName("Examples/TransparentBlue50");

    /// Notify -Mesh object that it has been loaded
    mesh->load();

    return mesh;
}

template<typename T>
void LodOutsideMarker::addHullTriangleVertices(std::vector<CHVertex*>& stack, T tri)
{
    OutsideData* v0 = getOutsideData(tri->vertex[0]);
    OutsideData* v1 = getOutsideData(tri->vertex[1]);
    OutsideData* v2 = getOutsideData(tri->vertex[2]);

    if (!v0->isOuterWallVertexInPass) {
        v0->isOuterWallVertexInPass = true;
        v0->isOuterWallVertex = true;
        stack.push_back(tri->vertex[0]);
    }
    if (!v1->isOuterWallVertexInPass) {
        v1->isOuterWallVertexInPass = true;
        v1->isOuterWallVertex = true;
        stack.push_back(tri->vertex[1]);
    }

    if (!v2->isOuterWallVertexInPass) {
        v2->isOuterWallVertexInPass = true;
        v2->isOuterWallVertex = true;
        stack.push_back(tri->vertex[2]);
    }
}

void LodOutsideMarker::markVertices()
{
    OutsideDataList::iterator v, vEnd;
    v = mOutsideData.begin();
    vEnd = mOutsideData.end();
    for (;v != vEnd; v++) {
        v->isOuterWallVertex = false;
    }
    std::vector<CHVertex*> stack;
    CHTriangleList::iterator tri, triEnd;
    tri = mHull.begin();
    triEnd = mHull.end();
    for (; tri != triEnd; tri++) {
        stack.clear();
        v = mOutsideData.begin();
        vEnd = mOutsideData.end();
        for (;v != vEnd; v++) {
            v->isOuterWallVertexInPass = false;
        }
        addHullTriangleVertices(stack, &*tri);
        while (!stack.empty()) {
            CHVertex* vert = stack.back();
            stack.pop_back();
            LodData::VTriangles::iterator it, itEnd;
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

}
