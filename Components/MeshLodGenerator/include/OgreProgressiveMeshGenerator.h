/*
 * -----------------------------------------------------------------------------
 * This source file is part of OGRE
 * (Object-oriented Graphics Rendering Engine)
 * For the latest info, see http://www.ogre3d.org/
 *
 * Copyright (c) 2000-2013 Torus Knot Software Ltd
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

#ifndef __ProgressiveMeshGenerator_H_
#define __ProgressiveMeshGenerator_H_

#include "OgreLodPrerequisites.h"
#include "OgreVector3.h"
#include "OgreSmallVector.h"
#include "OgreMesh.h"
#include "OgreLodConfig.h"

namespace Ogre
{

class OutsideMarker;

class _OgreLodExport ProgressiveMeshGeneratorBase
{
public:
	/**
	 * @brief Generates the Lod levels for a mesh.
	 * 
	 * @param lodConfig Specification of the requested Lod levels.
	 */
	virtual void generateLodLevels(LodConfig& lodConfig) = 0;

	/**
	 * @brief Generates the Lod levels for a mesh without configuring it.
	 *
	 * @param mesh Generate the Lod for this mesh.
	 */
	virtual void generateAutoconfiguredLodLevels(MeshPtr& mesh);

	/**
	 * @brief Fills Lod Config with a config, which works on any mesh.
	 *
	 * @param inMesh Optimize for this mesh.
	 * @param outLodConfig Lod configuration storing the output.
	 */
	virtual void getAutoconfig(MeshPtr& inMesh, LodConfig& outLodConfig);

	virtual ~ProgressiveMeshGeneratorBase() { }
};

/**
 * @brief Improved version of ProgressiveMesh.
 */
class _OgreLodExport ProgressiveMeshGenerator :
	public ProgressiveMeshGeneratorBase
{
public:

	friend class Ogre::OutsideMarker;

	ProgressiveMeshGenerator();
	virtual ~ProgressiveMeshGenerator();

	/// @copydoc ProgressiveMeshGeneratorBase::generateLodLevels
	void generateLodLevels(LodConfig& lodConfig);

	/**
	 * @brief Returns the last reduced vertex.
	 *
	 * You should call this function after generateLodLevels!
	 *
	 * @param outVec The vector receiving the position of the vertex.
	 * @return Whether the outVec was changed. If the mesh is reduced at least 1 vertex, then it returns true.
	 */
	bool _getLastVertexPos(Vector3& outVec);

	/**
	 * @brief Returns the destination of the edge, which was last reduced.
	 *
	 * You should call this function after generateLodLevels!
	 *
	 * @param outVec The vector receiving the CollapseTo position.
	 * @return Whether the outVec was changed. If the mesh is reduced at least 1 vertex, then it returns true.
	 */
	bool _getLastVertexCollapseTo(Vector3& outVec);

	MeshPtr _generateConvexHull(const String& meshName, int step);
protected:

	// VectorSet is basically a helper to use a vector as a small set container.
	// Also these functions keep the code clean and fast.
	// You can insert in O(1) time, if you know that it doesn't exists.
	// You can remove in O(1) time, if you know the position of the item.
	template<typename T, unsigned S>
	struct _OgrePrivate VectorSet :
		public SmallVector<T, S> {
		typedef typename SmallVector<T, S>::iterator iterator;

		void addNotExists(const T& item); // Complexity: O(1)!!
		void remove(iterator it); // Complexity: O(1)!!
		iterator add(const T& item); // Complexity: O(N)
		void removeExists(const T& item); // Complexity: O(N)
		bool remove(const T& item); // Complexity: O(N)
		void replaceExists(const T& oldItem, const T& newItem); // Complexity: O(N)
		bool has(const T& item); // Complexity: O(N)
		iterator find(const T& item); // Complexity: O(N)
		iterator findExists(const T& item); // Complexity: O(N)
	};

	struct PMEdge;
	struct PMVertex;
	struct PMTriangle;
	struct PMTriangleCache;
	struct PMVertexHash;
	struct PMVertexEqual;
	struct PMProfiledEdge;
	struct PMCollapsedEdge;
	struct PMIndexBufferInfo;

	typedef vector<PMVertex>::type VertexList;
	typedef vector<PMTriangle>::type TriangleList;
	typedef vector<PMTriangleCache>::type TriangleCacheList;
	typedef HashSet<PMVertex*, PMVertexHash, PMVertexEqual> UniqueVertexSet;
	typedef multimap<Real, PMVertex*>::type CollapseCostHeap;
	typedef vector<PMVertex*>::type VertexLookupList;
	typedef HashMultiMap<PMVertex*, PMProfiledEdge> ProfileLookup;

	typedef VectorSet<PMEdge, 8> VEdges;
	typedef VectorSet<PMTriangle*, 7> VTriangles;

	typedef vector<PMCollapsedEdge>::type CollapsedEdges;
	typedef vector<PMIndexBufferInfo>::type IndexBufferInfoList;

	// Hash function for UniqueVertexSet.
	struct _OgrePrivate PMVertexHash {
		ProgressiveMeshGenerator* mGen;

		PMVertexHash() { assert(0); }
		PMVertexHash(ProgressiveMeshGenerator* gen) { mGen = gen; }
		size_t operator() (const PMVertex* v) const;
	};

	// Equality function for UniqueVertexSet.
	struct _OgrePrivate PMVertexEqual {
		bool operator() (const PMVertex* lhs, const PMVertex* rhs) const;
	};

	// Directed edge
	struct _OgrePrivate PMEdge {
		PMVertex* dst;
		Real collapseCost;
		int refCount;

		explicit PMEdge(PMVertex* destination);
		bool operator== (const PMEdge& other) const;
		PMEdge& operator= (const PMEdge& b);
		PMEdge(const PMEdge& b);
		bool operator< (const PMEdge& other) const;
	};

	struct _OgrePrivate PMVertex {
		Vector3 position;
		VEdges edges;
		VTriangles triangles; /// Triangle ID set, which are using this vertex.
		
		Vector3 normal;
		bool hasProfile;
		PMVertex* collapseTo;
		bool seam;
		CollapseCostHeap::iterator costHeapPosition; /// Iterator pointing to the position in the mCollapseCostSet, which allows fast remove.
		bool isOuterWallVertex;
		bool isOuterWallVertexInPass;
		bool isInsideHull;
	};

	struct _OgrePrivate PMTriangleCache {
			unsigned int vertexID[3];
	};
	
	struct _OgrePrivate PMTriangle {
		PMVertex* vertex[3];
		Vector3 normal;
		bool isRemoved;
		unsigned short submeshID; /// ID of the submesh. Usable with mMesh.getSubMesh() function.
		unsigned int vertexID[3]; /// Vertex ID in the buffer associated with the submeshID.
		PMTriangleCache* prevLod;
		bool vertexChanged;

		void computeNormal();
		bool hasVertex(const PMVertex* v) const;
		unsigned int getVertexID(const PMVertex* v) const;
		bool isMalformed();
	};

	union _OgrePrivate IndexBufferPointer {
		unsigned short* pshort;
		unsigned int* pint;
	};

	struct _OgrePrivate PMIndexBufferInfo {
		size_t indexSize;
		size_t indexCount;
		size_t prevIndexCount;
		size_t prevOnlyIndexCount;
		IndexBufferPointer buf;
	};

	struct _OgrePrivate PMCollapsedEdge {
		unsigned int srcID;
		unsigned int dstID;
		unsigned short submeshID;
	};

	struct _OgrePrivate PMProfiledEdge {
		PMVertex* dst;
		Real cost;
	};

	VertexLookupList mSharedVertexLookup;
	VertexLookupList mVertexLookup;
	VertexList mVertexList;
	TriangleList mTriangleList;
	TriangleCacheList mTriangleCacheList;
	UniqueVertexSet mUniqueVertexSet;
	CollapseCostHeap mCollapseCostHeap;
	CollapsedEdges tmpCollapsedEdges; // Tmp container used in collapse().
	IndexBufferInfoList mIndexBufferInfoList;
	ProfileLookup mProfileLookup;

	MeshPtr mMesh;

#ifndef NDEBUG
	/**
	 * @brief The name of the mesh being processed.
	 *
	 * This is separate from mMesh in order to allow for access from background threads.
	 */
	String mMeshName;
#endif
	Real mMeshBoundingSphereRadius;
	Real mCollapseCostLimit;
	bool mUseVertexNormals;
	PMVertex* mLastReducedVertex;
	Real mOutsideWeight;
	Real mOutsideWalkAngle;
	int mLastIndexBufferID;

	size_t calcLodVertexCount(const LodLevel& lodConfig);
	void tuneContainerSize();
	void addVertexData(VertexData* vertexData, bool useSharedVertexLookup);
	template<typename IndexType>
	void addIndexDataImpl(IndexType* iPos, const IndexType* iEnd, VertexLookupList& lookup, unsigned short submeshID);
	void addIndexData(IndexData* indexData, bool useSharedVertexLookup, unsigned short submeshID);

	void computeCosts();
	bool isBorderVertex(const PMVertex* vertex) const;
	PMEdge* getPointer(VEdges::iterator it);
	void initVertexCollapseCost(PMVertex* vertex);
	void computeVertexCollapseCost(PMVertex* vertex, Real& collapseCost, PMVertex*& collapseTo);
	Real computeEdgeCollapseCost(PMVertex* src, PMEdge* dstEdge);
	virtual void bakeLods();
	virtual void bakeMergedLods(bool firstBufferPass);
	virtual void bakeDummyLods();
	void collapse(PMVertex* vertex);
	void initialize(LodConfig& lodConfig);
	void injectProfile(LodProfile& profile);
	void computeLods(LodConfig& lodConfigs);
	void updateVertexCollapseCost(PMVertex* src);

	bool hasSrcID(unsigned int srcID, unsigned short submeshID);
	size_t findDstID(unsigned int srcID, unsigned short submeshID);
	void replaceVertexID(PMTriangle* triangle, unsigned int oldID, unsigned int newID, PMVertex* dst);

#ifndef NDEBUG
	void assertValidVertex(PMVertex* v);
	void assertValidMesh();
	void assertOutdatedCollapseCost(PMVertex* vertex);
#endif // ifndef NDEBUG

	void addTriangleToEdges(PMTriangle* triangle);
	void removeTriangleFromEdges(PMTriangle* triangle, PMVertex* skip = NULL);
	void addEdge(PMVertex* v, const PMEdge& edge);
	void removeEdge(PMVertex* v, const PMEdge& edge);
	void printTriangle(PMTriangle* triangle, stringstream& str);
	PMTriangle* findSideTriangle(const PMVertex* v1, const PMVertex* v2);
	bool isDuplicateTriangle(PMTriangle* triangle, PMTriangle* triangle2);
	PMTriangle* isDuplicateTriangle(PMTriangle* triangle);
	int getTriangleID(PMTriangle* triangle);
	void cleanupMemory();

	void markOuterWall();
	void markOuterWallForHullTriangle();
	void addHullTriangleVertices(std::vector<PMVertex*>& stack, PMTriangle* tri);
};

}
#endif
