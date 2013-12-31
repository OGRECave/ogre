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

// Optimize for best output.
// #define PM_BEST_QUALITY

// Optimize for best performance.
// #define PM_WORST_QUALITY

#include "OgreStableHeaders.h"
#include "OgreProgressiveMeshGenerator.h"
#include "OgreHardwareIndexBuffer.h"
#include "OgreHardwareBufferManager.h"
#include "OgreSubMesh.h"
#include "OgreMesh.h"
#include "OgreLodStrategy.h"
#include "OgrePixelCountLodStrategy.h"

namespace Ogre
{

#define NEVER_COLLAPSE_COST std::numeric_limits<Real>::max()
#define UNINITIALIZED_COLLAPSE_COST (std::numeric_limits<Real>::infinity())

void ProgressiveMeshGeneratorBase::getAutoconfig( MeshPtr& inMesh, LodConfig& outLodConfig )
{
	outLodConfig.mesh = inMesh;
	outLodConfig.strategy = AbsolutePixelCountLodStrategy::getSingletonPtr();
	LodLevel lodLevel;
	lodLevel.reductionMethod = LodLevel::VRM_COLLAPSE_COST;
	Real radius = inMesh->getBoundingSphereRadius();
	for (int i = 2; i < 6; i++) {
	Real i4 = (Real) (i * i * i * i);
	Real i5 = i4 * (Real) i;
		// Distance = pixel count
		// Constant: zoom of the LOD. This could be scaled based on resolution.
		//     Higher constant means first LOD is nearer to camera. Smaller constant means the first LOD is further away from camera.
		// i4: The stretching. Normally you want to have more LOD level in the near, then in far away.
		//     i4 means distance is divided by 16=(2*2*2*2), 81, 256, 625=(5*5*5*5).
		//     if 16 would be smaller, the first LOD would be nearer. if 625 would be bigger, the last LOD would be further away.
		// if you increase 16 and decrease 625, first and Last LOD distance would be smaller.
		lodLevel.distance = 3388608.f / i4;
		
		// reductionValue = collapse cost
		// Radius: Edges are multiplied by the length, when calculating collapse cost. So as a base value we use radius, which should help in balancing collapse cost to any mesh size.
		// The constant and i5 are playing together. 1/(1/100k*i5)
		// You need to determine the quality of nearest LOD and the furthest away first.
		// I have chosen 1/(1/100k*(2^5)) = 3125 for nearest LOD and 1/(1/100k*(5^5)) = 32 for nearest LOD.
		// if you divide radius by a bigger number, it means smaller reduction. So radius/3125 is very small reduction for nearest LOD.
		// if you divide radius by a smaller number, it means bigger reduction. So radius/32 means aggressive reduction for furthest away LOD.
		// current values: 3125, 411, 97, 32
		lodLevel.reductionValue = radius / 100000.f * i5;
		outLodConfig.levels.push_back(lodLevel);
	}
}

void ProgressiveMeshGeneratorBase::generateAutoconfiguredLodLevels( MeshPtr& mesh )
{
	LodConfig lodConfig;
	getAutoconfig(mesh, lodConfig);
	generateLodLevels(lodConfig);
}

ProgressiveMeshGenerator::ProgressiveMeshGenerator() :
    mUniqueVertexSet((UniqueVertexSet::size_type) 0, (const UniqueVertexSet::hasher&) PMVertexHash(this)),
    mMesh(), mMeshBoundingSphereRadius(0.0f),
    mCollapseCostLimit(NEVER_COLLAPSE_COST)
{
	OgreAssert(NEVER_COLLAPSE_COST < UNINITIALIZED_COLLAPSE_COST && NEVER_COLLAPSE_COST != UNINITIALIZED_COLLAPSE_COST, "");
}

ProgressiveMeshGenerator::~ProgressiveMeshGenerator()
{
}

void ProgressiveMeshGenerator::tuneContainerSize()
{
	// Get Vertex count for container tuning.
	bool sharedVerticesAdded = false;
	size_t vertexCount = 0;
	size_t vertexLookupSize = 0;
	size_t sharedVertexLookupSize = 0;
	unsigned short submeshCount = mMesh->getNumSubMeshes();
	for (unsigned short i = 0; i < submeshCount; i++) {
		const SubMesh* submesh = mMesh->getSubMesh(i);
		if (!submesh->useSharedVertices) {
			size_t count = submesh->vertexData->vertexCount;
			vertexLookupSize = std::max(vertexLookupSize, count);
			vertexCount += count;
		} else if (!sharedVerticesAdded) {
			sharedVerticesAdded = true;
			sharedVertexLookupSize = mMesh->sharedVertexData->vertexCount;
			vertexCount += sharedVertexLookupSize;
		}
	}

	// Tune containers:
	mUniqueVertexSet.rehash(4 * vertexCount); // less then 0.25 item/bucket for low collision rate

	// There are less triangles then 2 * vertexCount. Except if there are bunch of triangles,
	// where all vertices have the same position, but that would not make much sense.
	mTriangleList.reserve(2 * vertexCount);

	mVertexList.reserve(vertexCount);
	mSharedVertexLookup.reserve(sharedVertexLookupSize);
	mVertexLookup.reserve(vertexLookupSize);
	mIndexBufferInfoList.resize(submeshCount);
}

void ProgressiveMeshGenerator::initialize()
{
#if OGRE_DEBUG_MODE
	mMeshName = mMesh->getName();
#endif
	unsigned short submeshCount = mMesh->getNumSubMeshes();
	for (unsigned short i = 0; i < submeshCount; ++i) {
		const SubMesh* submesh = mMesh->getSubMesh(i);
		VertexData* vertexData = (submesh->useSharedVertices ? mMesh->sharedVertexData : submesh->vertexData);
		addVertexData(vertexData, submesh->useSharedVertices);
        if(submesh->indexData->indexCount > 0)
            addIndexData(submesh->indexData, submesh->useSharedVertices, i);
	}

	// These were only needed for addIndexData() and addVertexData().
	mSharedVertexLookup.clear();
	mVertexLookup.clear();
	mUniqueVertexSet.clear();
}

void ProgressiveMeshGenerator::PMTriangle::computeNormal()
{
	// Cross-product 2 edges
	Vector3 e1 = vertex[1]->position - vertex[0]->position;
	Vector3 e2 = vertex[2]->position - vertex[1]->position;

	normal = e1.crossProduct(e2);
	normal.normalise();
}
void ProgressiveMeshGenerator::addVertexData(VertexData* vertexData, bool useSharedVertexLookup)
{
	if ((useSharedVertexLookup && !mSharedVertexLookup.empty())) { // We already loaded the shared vertex buffer.
		return;
	}
	OgreAssert(vertexData->vertexCount != 0, "");

	// Locate position element and the buffer to go with it.
	const VertexElement* elem = vertexData->vertexDeclaration->findElementBySemantic(VES_POSITION);

	// Only float supported.
	OgreAssert(elem->getSize() == 12, "");

	HardwareVertexBufferSharedPtr vbuf = vertexData->vertexBufferBinding->getBuffer(elem->getSource());

	// Lock the buffer for reading.
	unsigned char* vStart = static_cast<unsigned char*>(vbuf->lock(HardwareBuffer::HBL_READ_ONLY));
	unsigned char* vertex = vStart;
	size_t vSize = vbuf->getVertexSize();
	unsigned char* vEnd = vertex + vertexData->vertexCount * vSize;

	VertexLookupList& lookup = useSharedVertexLookup ? mSharedVertexLookup : mVertexLookup;
	lookup.clear();

	// Loop through all vertices and insert them to the Unordered Map.
	for (; vertex < vEnd; vertex += vSize) {
		float* pFloat;
		elem->baseVertexPointerToElement(vertex, &pFloat);
		mVertexList.push_back(PMVertex());
		PMVertex* v = &mVertexList.back();
		v->position.x = pFloat[0];
		v->position.y = pFloat[1];
		v->position.z = pFloat[2];
		std::pair<UniqueVertexSet::iterator, bool> ret;
		ret = mUniqueVertexSet.insert(v);
		if (!ret.second) {
			// Vertex position already exists.
			mVertexList.pop_back();
			v = *ret.first; // Point to the existing vertex.
			v->seam = true;
		} else {
#if OGRE_DEBUG_MODE
			// Needed for an assert, don't remove it.
			v->costHeapPosition = mCollapseCostHeap.end();
#endif
			v->seam = false;
		}
		lookup.push_back(v);
	}
	vbuf->unlock();
}

void ProgressiveMeshGenerator::addIndexData(IndexData* indexData, bool useSharedVertexLookup, unsigned short submeshID)
{
	const HardwareIndexBufferSharedPtr& ibuf = indexData->indexBuffer;
	size_t isize = ibuf->getIndexSize();
	mIndexBufferInfoList[submeshID].indexSize = isize;
	mIndexBufferInfoList[submeshID].indexCount = indexData->indexCount;
	if (indexData->indexCount == 0) {
		// Locking a zero length buffer on linux with nvidia cards fails.
		return;
	}
	VertexLookupList& lookup = useSharedVertexLookup ? mSharedVertexLookup : mVertexLookup;

	// Lock the buffer for reading.
	char* iStart = static_cast<char*>(ibuf->lock(HardwareBuffer::HBL_READ_ONLY));
	char* iEnd = iStart + ibuf->getSizeInBytes();
	if (isize == sizeof(unsigned short)) {
		addIndexDataImpl<unsigned short>((unsigned short*) iStart, (unsigned short*) iEnd, lookup, submeshID);
	} else {
		// Unsupported index size.
		OgreAssert(isize == sizeof(unsigned int), "");
		addIndexDataImpl<unsigned int>((unsigned int*) iStart, (unsigned int*) iEnd, lookup, submeshID);
	}
	ibuf->unlock();
}


bool ProgressiveMeshGenerator::PMTriangle::hasVertex(const PMVertex* v) const
{
	return (v == vertex[0] || v == vertex[1] || v == vertex[2]);
}

void ProgressiveMeshGenerator::replaceVertexID(PMTriangle* triangle, unsigned int oldID, unsigned int newID, PMVertex* dst)
{
	dst->triangles.addNotExists(triangle);
	// NOTE: triangle is not removed from src. This is implementation specific optimization.

	// Its up to the compiler to unroll everything.
	for (int i = 0; i < 3; i++) {
		if (triangle->vertexID[i] == oldID) {
			for (int n = 0; n < 3; n++) {
				if (i != n) {
					// This is implementation specific optimization to remove following line.
					// removeEdge(triangle->vertex[i], PMEdge(triangle->vertex[n]));

					removeEdge(triangle->vertex[n], PMEdge(triangle->vertex[i]));
					addEdge(triangle->vertex[n], PMEdge(dst));
					addEdge(dst, PMEdge(triangle->vertex[n]));
				}
			}
			triangle->vertex[i] = dst;
			triangle->vertexID[i] = newID;
			return;
		}
	}
	OgreAssert(0, "");
}

unsigned int ProgressiveMeshGenerator::PMTriangle::getVertexID(const PMVertex* v) const
{
	for (int i = 0; i < 3; i++) {
		if (vertex[i] == v) {
			return vertexID[i];
		}
	}
	OgreAssert(0, "");
	return 0;
}

void ProgressiveMeshGenerator::removeTriangleFromEdges(PMTriangle* triangle, PMVertex* skip)
{
	// skip is needed if we are iterating on the vertex's edges or triangles.
	for (int i = 0; i < 3; i++) {
		if (triangle->vertex[i] != skip) {
			triangle->vertex[i]->triangles.removeExists(triangle);
		}
	}
	for (int i = 0; i < 3; i++) {
		for (int n = 0; n < 3; n++) {
			if (i != n && triangle->vertex[i] != skip) {
				removeEdge(triangle->vertex[i], PMEdge(triangle->vertex[n]));
			}
		}
	}
}

void ProgressiveMeshGenerator::printTriangle(PMTriangle* triangle, stringstream& str)
{
	for (int i = 0; i < 3; i++) {
		str << (i + 1) << ". vertex position: ("
		    << triangle->vertex[i]->position.x << ", "
		    << triangle->vertex[i]->position.y << ", "
		    << triangle->vertex[i]->position.z << ") "
		    << "vertex ID: " << triangle->vertexID[i] << std::endl;
	}
}

bool ProgressiveMeshGenerator::PMTriangle::isMalformed()
{
	return vertex[0] == vertex[1] || vertex[0] == vertex[2] || vertex[1] == vertex[2];
}

bool ProgressiveMeshGenerator::isDuplicateTriangle(PMTriangle* triangle, PMTriangle* triangle2)
{
	for (int i = 0; i < 3; i++) {
		if (triangle->vertex[i] != triangle2->vertex[0] ||
		    triangle->vertex[i] != triangle2->vertex[1] ||
		    triangle->vertex[i] != triangle2->vertex[2]) {
			return false;
		}
	}
	return true;
}

ProgressiveMeshGenerator::PMTriangle* ProgressiveMeshGenerator::isDuplicateTriangle(PMTriangle* triangle)
{
	// duplicate triangle detection (where all vertices has the same position)
	VTriangles::iterator itEnd = triangle->vertex[0]->triangles.end();
	VTriangles::iterator it = triangle->vertex[0]->triangles.begin();
	for (; it != itEnd; ++it) {
		PMTriangle* t = *it;
		if (isDuplicateTriangle(triangle, t)) {
			return *it;
		}
	}
	return NULL;
}
int ProgressiveMeshGenerator::getTriangleID(PMTriangle* triangle)
{
	return static_cast<int>((triangle - &mTriangleList[0]) / sizeof(PMTriangle));
}
void ProgressiveMeshGenerator::addTriangleToEdges(PMTriangle* triangle)
{
#ifdef PM_BEST_QUALITY
	PMTriangle* duplicate = isDuplicateTriangle(triangle);
	if (duplicate != NULL) {
#if OGRE_DEBUG_MODE
		stringstream str;
		str << "In " << mMeshName << " duplicate triangle found." << std::endl;
		str << "Triangle " << getTriangleID(triangle) << " positions:" << std::endl;
		printTriangle(triangle, str);
		str << "Triangle " << getTriangleID(duplicate) << " positions:" << std::endl;
		printTriangle(duplicate, str);
		str << "Triangle " << getTriangleID(triangle) << " will be excluded from LOD level calculations.";
		LogManager::getSingleton().stream() << str;
#endif
		triangle->isRemoved = true;
		mIndexBufferInfoList[triangle->submeshID].indexCount -= 3;
		return;
	}
#endif // ifdef PM_BEST_QUALITY
	for (int i = 0; i < 3; i++) {
		triangle->vertex[i]->triangles.addNotExists(triangle);
	}
	for (int i = 0; i < 3; i++) {
		for (int n = 0; n < 3; n++) {
			if (i != n) {
				addEdge(triangle->vertex[i], PMEdge(triangle->vertex[n]));
			}
		}
	}
}

bool ProgressiveMeshGenerator::isBorderVertex(const PMVertex* vertex) const
{
	VEdges::const_iterator it = vertex->edges.begin();
	VEdges::const_iterator itEnd = vertex->edges.end();
	for (; it != itEnd; ++it) {
		if (it->refCount == 1) {
			return true;
		}
	}
	return false;
}

ProgressiveMeshGenerator::PMTriangle* ProgressiveMeshGenerator::findSideTriangle(const PMVertex* v1, const PMVertex* v2)
{
	VTriangles::const_iterator it = v1->triangles.begin();
	VTriangles::const_iterator itEnd = v1->triangles.end();
	for (; it != itEnd; ++it) {
		PMTriangle* triangle = *it;
		if (triangle->hasVertex(v2)) {
			return triangle;
		}
	}
	OgreAssert(0, "");
	return NULL;
}

ProgressiveMeshGenerator::PMEdge* ProgressiveMeshGenerator::getPointer(VEdges::iterator it)
{
	return &*it;
}

void ProgressiveMeshGenerator::computeCosts()
{
	mCollapseCostHeap.clear();
	VertexList::iterator it = mVertexList.begin();
	VertexList::iterator itEnd = mVertexList.end();
	for (; it != itEnd; ++it) {
		if (!it->edges.empty()) {

			computeVertexCollapseCost(&*it);
		} else {
#if OGRE_DEBUG_MODE
			LogManager::getSingleton().stream() << "In " << mMeshName << " never used vertex found with ID: " << mCollapseCostHeap.size() << ". "
			    << "Vertex position: ("
			    << it->position.x << ", "
			    << it->position.y << ", "
			    << it->position.z << ") "
			    << "It will be excluded from LOD level calculations.";
#endif
		}
	}
}

void ProgressiveMeshGenerator::computeVertexCollapseCost(PMVertex* vertex)
{
	Real collapseCost = UNINITIALIZED_COLLAPSE_COST;
	OgreAssert(!vertex->edges.empty(), "");
	VEdges::iterator it = vertex->edges.begin();
	for (; it != vertex->edges.end(); ++it) {
		it->collapseCost = computeEdgeCollapseCost(vertex, getPointer(it));
		if (collapseCost > it->collapseCost) {
			collapseCost = it->collapseCost;
			vertex->collapseTo = it->dst;
		}
	}
	OgreAssert(collapseCost != UNINITIALIZED_COLLAPSE_COST, "");
	vertex->costHeapPosition = mCollapseCostHeap.insert(std::make_pair(collapseCost, vertex));

}

Real ProgressiveMeshGenerator::computeEdgeCollapseCost(PMVertex* src, PMEdge* dstEdge)
{
	// This is based on Ogre's collapse cost calculation algorithm.
	// 65% of the time is spent in this function!

	PMVertex* dst = dstEdge->dst;

#ifndef PM_WORST_QUALITY
	// 30% speedup if disabled.

	// Degenerate case check
	// Are we going to invert a face normal of one of the neighboring faces?
	// Can occur when we have a very small remaining edge and collapse crosses it
	// Look for a face normal changing by > 90 degrees
	{
		VTriangles::iterator it = src->triangles.begin();
		VTriangles::iterator itEnd = src->triangles.end();
		for (; it != itEnd; ++it) {
			PMTriangle* triangle = *it;
			// Ignore the deleted faces (those including src & dest)
			if (!triangle->hasVertex(dst)) {
				// Test the new face normal
				PMVertex* pv0, * pv1, * pv2;

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
				if (newNormal.dotProduct(triangle->normal) < 0.0f) {
					// Don't do it!
					return NEVER_COLLAPSE_COST;
				}
			}
		}
	}
#endif

	Real cost;

	// Special cases
	// If we're looking at a border vertex
	if (isBorderVertex(src)) {
		if (dstEdge->refCount > 1) {
			// src is on a border, but the src-dest edge has more than one tri on it
			// So it must be collapsing inwards
			// Mark as very high-value cost
			// curvature = 1.0f;
			cost = 1.0f;
		} else {
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
			VEdges::iterator it = src->edges.begin();
			VEdges::iterator itEnd = src->edges.end();
			for (; it != itEnd; ++it) {
				PMVertex* neighbor = it->dst;
				if (neighbor != dst && it->refCount == 1) {
					Vector3 otherBorderEdge = src->position - neighbor->position;
					otherBorderEdge.normalise();
					// This time, the nearer the dot is to -1, the better, because that means
					// the edges are opposite each other, therefore less kinkiness
					// Scale into [0..1]
					Real kinkiness = otherBorderEdge.dotProduct(collapseEdge);
					cost = std::max(cost, kinkiness);
				}
			}
			cost = (1.002f + cost) * 0.5f;
		}
	} else { // not a border

		// Standard inner vertex
		// Calculate curvature
		// use the triangle facing most away from the sides
		// to determine our curvature term
		// Iterate over src's faces again
		cost = 1.0f;
		VTriangles::iterator it = src->triangles.begin();
		VTriangles::iterator itEnd = src->triangles.end();
		for (; it != itEnd; ++it) {
			Real mincurv = -1.0f; // curve for face i and closer side to it
			PMTriangle* triangle = *it;
			VTriangles::iterator it2 = src->triangles.begin();
			for (; it2 != itEnd; ++it2) {
				PMTriangle* triangle2 = *it2;
				if (triangle2->hasVertex(dst)) {

					// Dot product of face normal gives a good delta angle
					Real dotprod = triangle->normal.dotProduct(triangle2->normal);
					// NB we do (1-..) to invert curvature where 1 is high curvature [0..1]
					// Whilst dot product is high when angle difference is low
					mincurv = std::max(mincurv, dotprod);
				}
			}
			cost = std::min(cost, mincurv);
		}
		cost = (1.002f - cost) * 0.5f;
	}

	// check for texture seam ripping and multiple submeshes
	if (src->seam) {
		if (!dst->seam) {
			cost = std::max(cost, (Real)0.05f);
			cost *= 64;
		} else {
#ifdef PM_BEST_QUALITY
			int seamNeighbors = 0;
			PMVertex* otherSeam;
			VEdges::iterator it = src->edges.begin();
			VEdges::iterator itEnd = src->edges.end();
			for (; it != itEnd; ++it) {
				PMVertex* neighbor = it->dst;
				if(neighbor->seam) {
					seamNeighbors++;
					if(neighbor != dst){
						otherSeam = neighbor;
					}
				}
			}
			if(seamNeighbors != 2 || (seamNeighbors == 2 && dst->edges.has(PMEdge(otherSeam)))) {
				cost = std::max(cost, (Real)0.05f);
				cost *= 64;
			} else {
				cost = std::max(cost, (Real)0.005f);
				cost *= 8;
			}
#else
			cost = std::max(cost, (Real)0.005f);
			cost *= 8;
#endif
			
		}
	}

	OgreAssert(cost >= 0, "");
	return cost * src->position.distance(dst->position);
}

#if OGRE_DEBUG_MODE
void ProgressiveMeshGenerator::assertOutdatedCollapseCost(PMVertex* vertex)
{
	// Validates that collapsing has updated all edges needed by computeEdgeCollapseCost.
	// This will OgreAssert if the dependencies inside computeEdgeCollapseCost changes.
	VEdges::iterator it = vertex->edges.begin();
	VEdges::iterator itEnd = vertex->edges.end();
	for (; it != itEnd; it++) {
		OgreAssert(it->collapseCost == computeEdgeCollapseCost(vertex, getPointer(it)), "");
		PMVertex* neighbor = it->dst;
		VEdges::iterator it2 = neighbor->edges.begin();
		VEdges::iterator it2End = neighbor->edges.end();
		for (; it2 != it2End; it2++) {
			OgreAssert(it2->collapseCost == computeEdgeCollapseCost(neighbor, getPointer(it2)), "");
		}
	}
}
#endif // ifndef NDEBUG

void ProgressiveMeshGenerator::updateVertexCollapseCost(PMVertex* vertex)
{
	Real collapseCost = UNINITIALIZED_COLLAPSE_COST;
	PMVertex* collapseTo = 0;
	VEdges::iterator it = vertex->edges.begin();
	VEdges::iterator itEnd = vertex->edges.end();
	for (; it != itEnd; ++it) {
		it->collapseCost = computeEdgeCollapseCost(vertex, getPointer(it));
		if (collapseCost > it->collapseCost) {
			collapseCost = it->collapseCost;
			collapseTo = it->dst;
		}
	}
	if (vertex->collapseTo != collapseTo || collapseCost != vertex->costHeapPosition->first) {
		OgreAssert(vertex->collapseTo != NULL, "");
		OgreAssert(vertex->costHeapPosition != mCollapseCostHeap.end(), "");
		mCollapseCostHeap.erase(vertex->costHeapPosition);
		if (collapseCost != UNINITIALIZED_COLLAPSE_COST) {
			vertex->collapseTo = collapseTo;
			vertex->costHeapPosition = mCollapseCostHeap.insert(std::make_pair(collapseCost, vertex));
		} else {
#if OGRE_DEBUG_MODE
			vertex->collapseTo = NULL;
			vertex->costHeapPosition = mCollapseCostHeap.end();
#endif
		}
	}
}

void ProgressiveMeshGenerator::generateLodLevels(LodConfig& lodConfig)
{
#if OGRE_DEBUG_MODE

	// Do not call this with empty LOD.
	OgreAssert(!lodConfig.levels.empty(), "");

	// Too many LOD levels.
	OgreAssert(lodConfig.levels.size() <= 0xffff, "");

	// LOD distances needs to be sorted.
	Mesh::LodValueList values;
	for (size_t i = 0; i < lodConfig.levels.size(); i++) {
		values.push_back(lodConfig.levels[i].distance);
	}
	lodConfig.strategy->assertSorted(values);
#endif
	mMesh = lodConfig.mesh;
	mMeshBoundingSphereRadius = mMesh->getBoundingSphereRadius();
	mMesh->removeLodLevels();
	tuneContainerSize();
	initialize(); // Load vertices and triangles
	computeCosts(); // Calculate all collapse costs
#if OGRE_DEBUG_MODE
	assertValidMesh();
#endif // ifndef NDEBUG

	computeLods(lodConfig);

	mMesh.get()->_configureMeshLodUsage(lodConfig);
}

void ProgressiveMeshGenerator::computeLods(LodConfig& lodConfigs)
{
	size_t vertexCount = mVertexList.size();
	size_t lastBakeVertexCount = vertexCount;
	size_t lodCount = lodConfigs.levels.size();
	for (unsigned short curLod = 0; curLod < lodCount; curLod++) {
		size_t neededVertexCount = calcLodVertexCount(lodConfigs.levels[curLod]);
		for (; neededVertexCount < vertexCount; vertexCount--) {
			CollapseCostHeap::iterator nextVertex = mCollapseCostHeap.begin();
			if (nextVertex != mCollapseCostHeap.end() && nextVertex->first < mCollapseCostLimit) {
				collapse(nextVertex->second);
			} else {
				break;
			}
		}
		lodConfigs.levels[curLod].outUniqueVertexCount = vertexCount;
		lodConfigs.levels[curLod].outSkipped = (lastBakeVertexCount == vertexCount);
		if (!lodConfigs.levels[curLod].outSkipped) {
			lastBakeVertexCount = vertexCount;
#if OGRE_DEBUG_MODE
			assertValidMesh();
#endif // ifndef NDEBUG
			bakeLods();
		}
	}
}

size_t ProgressiveMeshGenerator::findDstID(unsigned int srcID, unsigned short submeshID)
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

bool ProgressiveMeshGenerator::hasSrcID(unsigned int srcID, unsigned short submeshID)
{
	// This will only return exact matches.
	for (size_t i = 0; i < tmpCollapsedEdges.size(); i++) {
		if (tmpCollapsedEdges[i].srcID == srcID && tmpCollapsedEdges[i].submeshID == submeshID) {
			return true;
		}
	}
	return false; // Not found
}

#if OGRE_DEBUG_MODE
void ProgressiveMeshGenerator::assertValidMesh()
{
	// Allows to find bugs in collapsing.
//	size_t s1 = mUniqueVertexSet.size();
//	size_t s2 = mCollapseCostHeap.size();
	CollapseCostHeap::iterator it = mCollapseCostHeap.begin();
	CollapseCostHeap::iterator itEnd = mCollapseCostHeap.end();
	while (it != itEnd) {
		assertValidVertex(it->second);
		it++;
	}
}

void ProgressiveMeshGenerator::assertValidVertex(PMVertex* v)
{
	// Allows to find bugs in collapsing.
	VTriangles::iterator it = v->triangles.begin();
	VTriangles::iterator itEnd = v->triangles.end();
	for (; it != itEnd; it++) {
		PMTriangle* t = *it;
		for (int i = 0; i < 3; i++) {
			OgreAssert(t->vertex[i]->costHeapPosition != mCollapseCostHeap.end(), "");
			t->vertex[i]->edges.findExists(PMEdge(t->vertex[i]->collapseTo));
			for (int n = 0; n < 3; n++) {
				if (i != n) {
					VEdges::iterator edgeIt = t->vertex[i]->edges.findExists(PMEdge(t->vertex[n]));
					OgreAssert(edgeIt->collapseCost != UNINITIALIZED_COLLAPSE_COST, "");
				} else {
					OgreAssert(t->vertex[i]->edges.find(PMEdge(t->vertex[n])) == t->vertex[i]->edges.end(), "");
				}
			}
		}
	}
}
#endif // ifndef NDEBUG

void ProgressiveMeshGenerator::collapse(PMVertex* src)
{
	PMVertex* dst = src->collapseTo;
#if OGRE_DEBUG_MODE
	assertValidVertex(dst);
	assertValidVertex(src);
#endif // ifndef NDEBUG
	OgreAssert(src->costHeapPosition->first != NEVER_COLLAPSE_COST, "");
	OgreAssert(src->costHeapPosition->first != UNINITIALIZED_COLLAPSE_COST, "");
	OgreAssert(!src->edges.empty(), "");
	OgreAssert(!src->triangles.empty(), "");
	OgreAssert(src->edges.find(PMEdge(dst)) != src->edges.end(), "");

	// It may have vertexIDs and triangles from different submeshes(different vertex buffers),
	// so we need to connect them correctly based on deleted triangle's edge.
	// mCollapsedEdgeIDs will be used, when looking up the connections for replacement.
	tmpCollapsedEdges.clear();
	VTriangles::iterator it = src->triangles.begin();
	VTriangles::iterator itEnd = src->triangles.end();
	for (; it != itEnd; ++it) {
		PMTriangle* triangle = *it;
		if (triangle->hasVertex(dst)) {
			// Remove a triangle
			// Tasks:
			// 1. Add it to the collapsed edges list.
			// 2. Reduce index count for the LOD levels, which will not have this triangle.
			// 3. Mark as removed, so it will not be added in upcoming LOD levels.
			// 4. Remove references/pointers to this triangle.

			// 1. task
			unsigned int srcID = triangle->getVertexID(src);
			if (!hasSrcID(srcID, triangle->submeshID)) {
				tmpCollapsedEdges.push_back(PMCollapsedEdge());
				tmpCollapsedEdges.back().srcID = srcID;
				tmpCollapsedEdges.back().dstID = triangle->getVertexID(dst);
				tmpCollapsedEdges.back().submeshID = triangle->submeshID;
			}

			// 2. task
			mIndexBufferInfoList[triangle->submeshID].indexCount -= 3;

			// 3. task
			triangle->isRemoved = true;

			// 4. task
			removeTriangleFromEdges(triangle, src);
		}
	}
	OgreAssert(tmpCollapsedEdges.size(), "");
	OgreAssert(dst->edges.find(PMEdge(src)) == dst->edges.end(), "");

	it = src->triangles.begin();
	for (; it != itEnd; ++it) {
		PMTriangle* triangle = *it;
		if (!triangle->hasVertex(dst)) {
			// Replace a triangle
			// Tasks:
			// 1. Determine the edge which we will move along. (we need to modify single vertex only)
			// 2. Move along the selected edge.

			// 1. task
			unsigned int srcID = triangle->getVertexID(src);
			size_t id = findDstID(srcID, triangle->submeshID);
			if (id == std::numeric_limits<size_t>::max()) {
				// Not found any edge to move along.
				// Destroy the triangle.
				triangle->isRemoved = true;
				mIndexBufferInfoList[triangle->submeshID].indexCount -= 3;
				removeTriangleFromEdges(triangle, src);
				continue;
			}
			unsigned int dstID = tmpCollapsedEdges[id].dstID;

			// 2. task
			replaceVertexID(triangle, srcID, dstID, dst);

#ifdef PM_BEST_QUALITY
			triangle->computeNormal();
#endif
		}
	}
	
	dst->seam |= src->seam; // Inherit seam property

#ifndef PM_BEST_QUALITY
	VEdges::iterator it3 = src->edges.begin();
	VEdges::iterator it3End = src->edges.end();
	for (; it3 != it3End; ++it3) {
		updateVertexCollapseCost(it3->dst);
	}
#else
	// TODO: Find out why is this needed. assertOutdatedCollapseCost() fails on some
	// rare situations without this. For example goblin.mesh fails.
	typedef SmallVector<PMVertex*, 64> UpdatableList;
	UpdatableList updatable;
	VEdges::iterator it3 = src->edges.begin();
	VEdges::iterator it3End = src->edges.end();
	for (; it3 != it3End; ++it3) {
		updatable.push_back(it3->dst);
		VEdges::iterator it4End = it3->dst->edges.end();
		VEdges::iterator it4 = it3->dst->edges.begin();
		for (; it4 != it4End; ++it4) {
			updatable.push_back(it4->dst);
		}
	}

	// Remove duplicates.
	UpdatableList::iterator it5 = updatable.begin();
	UpdatableList::iterator it5End = updatable.end();
	std::sort(it5, it5End);
	it5End = std::unique(it5, it5End);

	for (; it5 != it5End; ++it5) {
		updateVertexCollapseCost(*it5);
	}
#if OGRE_DEBUG_MODE
	it3 = src->edges.begin();
	it3End = src->edges.end();
	for (; it3 != it3End; ++it3) {
		assertOutdatedCollapseCost(it3->dst);
	}
	it3 = dst->edges.begin();
	it3End = dst->edges.end();
	for (; it3 != it3End; ++it3) {
		assertOutdatedCollapseCost(it3->dst);
	}
	assertOutdatedCollapseCost(dst);
#endif // ifndef NDEBUG
#endif // ifndef PM_BEST_QUALITY
	mCollapseCostHeap.erase(src->costHeapPosition); // Remove src from collapse costs.
	src->edges.clear(); // Free memory
	src->triangles.clear(); // Free memory
#if OGRE_DEBUG_MODE
	src->costHeapPosition = mCollapseCostHeap.end();
	assertValidVertex(dst);
#endif
}
size_t ProgressiveMeshGenerator::calcLodVertexCount(const LodLevel& lodConfig)
{
	size_t uniqueVertices = mVertexList.size();
	switch (lodConfig.reductionMethod) {
	case LodLevel::VRM_PROPORTIONAL:
		mCollapseCostLimit = NEVER_COLLAPSE_COST;
		return uniqueVertices - (size_t)((Real)uniqueVertices * lodConfig.reductionValue);

	case LodLevel::VRM_CONSTANT:
	{
		mCollapseCostLimit = NEVER_COLLAPSE_COST;
		size_t reduction = (size_t) lodConfig.reductionValue;
		if (reduction < uniqueVertices) {
			return uniqueVertices - reduction;
		} else {
			return 0;
		}
	}

	case LodLevel::VRM_COLLAPSE_COST:
		mCollapseCostLimit = lodConfig.reductionValue;
		return 0;

	default:
		OgreAssert(0, "");
		return uniqueVertices;
	}
}

void ProgressiveMeshGenerator::bakeLods()
{

	unsigned short submeshCount = mMesh->getNumSubMeshes();
	std::auto_ptr<IndexBufferPointer> indexBuffer(new IndexBufferPointer[submeshCount]);

	// Create buffers.
	for (unsigned short i = 0; i < submeshCount; i++) {
		SubMesh::LODFaceList& lods = mMesh->getSubMesh(i)->mLodFaceList;
		size_t indexCount = mIndexBufferInfoList[i].indexCount;
		lods.push_back(OGRE_NEW IndexData());
		lods.back()->indexStart = 0;

		if (indexCount == 0) {
			//If the index is empty we need to create a "dummy" triangle, just to keep the index buffer from being empty.
			//The main reason for this is that the OpenGL render system will crash with a segfault unless the index has some values.
			//This should hopefully be removed with future versions of Ogre. The most preferred solution would be to add the
			//ability for a submesh to be excluded from rendering for a given LOD (which isn't possible currently 2012-12-09).
			lods.back()->indexCount = 3;
		} else {
			lods.back()->indexCount = indexCount;
		}

		lods.back()->indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(
			mIndexBufferInfoList[i].indexSize == 2 ?
			HardwareIndexBuffer::IT_16BIT : HardwareIndexBuffer::IT_32BIT,
			lods.back()->indexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);

		indexBuffer.get()[i].pshort =
			static_cast<unsigned short*>(lods.back()->indexBuffer->lock(0, lods.back()->indexBuffer->getSizeInBytes(),
			HardwareBuffer::HBL_DISCARD));

		//Check if we should fill it with a "dummy" triangle.
		if (indexCount == 0) {
			memset(indexBuffer.get()[i].pshort, 0, 3 * mIndexBufferInfoList[i].indexSize);
		}
	}

	// Fill buffers.
	size_t triangleCount = mTriangleList.size();
	for (size_t i = 0; i < triangleCount; i++) {
		if (!mTriangleList[i].isRemoved) {
			OgreAssert(mIndexBufferInfoList[mTriangleList[i].submeshID].indexCount != 0, "");
			if (mIndexBufferInfoList[mTriangleList[i].submeshID].indexSize == 2) {
				for (int m = 0; m < 3; m++) {
					*(indexBuffer.get()[mTriangleList[i].submeshID].pshort++) =
					    static_cast<unsigned short>(mTriangleList[i].vertexID[m]);
				}
			} else {
				for (int m = 0; m < 3; m++) {
					*(indexBuffer.get()[mTriangleList[i].submeshID].pint++) =
					    static_cast<unsigned int>(mTriangleList[i].vertexID[m]);
				}
			}
		}
	}

	// Close buffers.
	for (unsigned short i = 0; i < submeshCount; i++) {
		SubMesh::LODFaceList& lods = mMesh->getSubMesh(i)->mLodFaceList;
		lods.back()->indexBuffer->unlock();
	}
}

ProgressiveMeshGenerator::PMEdge::PMEdge(PMVertex* destination) :
	dst(destination)
#if OGRE_DEBUG_MODE
	, collapseCost(UNINITIALIZED_COLLAPSE_COST)
#endif
	, refCount(0)
{

}

ProgressiveMeshGenerator::PMEdge::PMEdge(const PMEdge& b)
{
	*this = b;
}

bool ProgressiveMeshGenerator::PMEdge::operator< (const PMEdge& other) const
{
	return (size_t) dst < (size_t) other.dst;   // Comparing pointers for uniqueness.
}

ProgressiveMeshGenerator::PMEdge& ProgressiveMeshGenerator::PMEdge::operator= (const PMEdge& b)
{
	dst = b.dst;
	collapseCost = b.collapseCost;
	refCount = b.refCount;
	return *this;
}

bool ProgressiveMeshGenerator::PMEdge::operator== (const PMEdge& other) const
{
	return dst == other.dst;
}

void ProgressiveMeshGenerator::addEdge(PMVertex* v, const PMEdge& edge)
{
	OgreAssert(edge.dst != v, "");
	VEdges::iterator it;
	it = v->edges.add(edge);
	if (it == v->edges.end()) {
		v->edges.back().refCount = 1;
	} else {
		it->refCount++;
	}
}

void ProgressiveMeshGenerator::removeEdge(PMVertex* v, const PMEdge& edge)
{
	VEdges::iterator it = v->edges.findExists(edge);
	if (it->refCount == 1) {
		v->edges.remove(it);
	} else {
		it->refCount--;
	}
}

void ProgressiveMeshGenerator::cleanupMemory()
{
	this->mCollapseCostHeap.clear();
	this->mIndexBufferInfoList.clear();
	this->mSharedVertexLookup.clear();
	this->mVertexLookup.clear();
	this->mUniqueVertexSet.clear();
	this->mVertexList.clear();
	this->mTriangleList.clear();
}

bool ProgressiveMeshGenerator::PMVertexEqual::operator() (const PMVertex* lhs, const PMVertex* rhs) const
{
	return lhs->position == rhs->position;
}

size_t ProgressiveMeshGenerator::PMVertexHash::operator() (const PMVertex* v) const
{
	// Stretch the values to an integer grid.
	Real stretch = (Real)0x7fffffff / mGen->mMeshBoundingSphereRadius;
	int hash = (int)(v->position.x * stretch);
	hash ^= (int)(v->position.y * stretch) * 0x100;
	hash ^= (int)(v->position.z * stretch) * 0x10000;
	return (size_t)hash;
}

template<typename T, unsigned S>
void ProgressiveMeshGenerator::VectorSet<T, S>::addNotExists(const T& item)
{
	OgreAssert(find(item) == this->end(), "");
	this->push_back(item);
}

template<typename T, unsigned S>
void ProgressiveMeshGenerator::VectorSet<T, S>::remove(iterator it)
{
	// Thats my trick to remove an item from the vector very fast!
	// It works similar to the heap_pop().
	// It swaps the removable item to the back, then pops it.
	*it = this->back();
	this->pop_back();
}

template<typename T, unsigned S>
typename ProgressiveMeshGenerator::VectorSet<T, S>::iterator ProgressiveMeshGenerator::VectorSet<T, S>::add(const T& item)
{
	iterator it = find(item);
	if (it == this->end()) {
		this->push_back(item);
		return this->end();
	}
	return it;
}

template<typename T, unsigned S>
void ProgressiveMeshGenerator::VectorSet<T, S>::removeExists(const T& item)
{
	iterator it = find(item);
	OgreAssert(it != this->end(), "");
	remove(it);
}

template<typename T, unsigned S>
bool ProgressiveMeshGenerator::VectorSet<T, S>::remove(const T& item)
{
	iterator it = find(item);
	if (it != this->end()) {
		remove(it);
		return true;
	} else {
		return false;
	}
}

template<typename T, unsigned S>
void ProgressiveMeshGenerator::VectorSet<T, S>::replaceExists(const T& oldItem, const T& newItem)
{
	iterator it = find(oldItem);
	OgreAssert(it != this->end(), "");
	*it = newItem;
}

template<typename T, unsigned S>
bool ProgressiveMeshGenerator::VectorSet<T, S>::has(const T& item)
{
	return find(item) != this->end();
}

template<typename T, unsigned S>
typename ProgressiveMeshGenerator::VectorSet<T, S>::iterator ProgressiveMeshGenerator::VectorSet<T, S>::find(const T& item)
{
	return std::find(this->begin(), this->end(), item);
}

template<typename T, unsigned S>
typename ProgressiveMeshGenerator::VectorSet<T, S>::iterator ProgressiveMeshGenerator::VectorSet<T, S>::findExists(
    const T& item)
{
	iterator it = find(item);
	OgreAssert(it != this->end(), "");
	return it;
}

}
