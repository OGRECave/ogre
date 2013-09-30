#include "OgreLodData.h"

namespace Ogre
{

const Real LodData::NEVER_COLLAPSE_COST = std::numeric_limits<Real>::max();
const Real LodData::UNINITIALIZED_COLLAPSE_COST = std::numeric_limits<Real>::infinity();

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

void LodData::Triangle::computeNormal()
{
	// Cross-product 2 edges
	Vector3 e1 = vertex[1]->position - vertex[0]->position;
	Vector3 e2 = vertex[2]->position - vertex[1]->position;

	normal = e1.crossProduct(e2);
	normal.normalise();
}

bool LodData::Triangle::hasVertex(const LodData::Vertex* v) const
{
	return (v == vertex[0] || v == vertex[1] || v == vertex[2]);
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