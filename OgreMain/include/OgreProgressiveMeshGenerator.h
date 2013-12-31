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

#ifndef __ProgressiveMeshGenerator_H_
#define __ProgressiveMeshGenerator_H_

#include "OgrePrerequisites.h"
#include "OgreVector3.h"
#include "OgreSmallVector.h"
#include "OgreMesh.h"
#include "OgreLodConfig.h"
#include "OgreLogManager.h"

namespace Ogre
{

class _OgreExport ProgressiveMeshGeneratorBase
{
public:
	/**
	 * @brief Generates the LOD levels for a mesh.
	 * 
	 * @param lodConfig Specification of the requested LOD levels.
	 */
	virtual void generateLodLevels(LodConfig& lodConfig) = 0;

	/**
	 * @brief Generates the LOD levels for a mesh without configuring it.
	 *
	 * @param mesh Generate the LOD for this mesh.
	 */
	virtual void generateAutoconfiguredLodLevels(MeshPtr& mesh);

	/**
	 * @brief Fills LOD Config with a config, which works on any mesh.
	 *
	 * @param inMesh Optimize for this mesh.
	 * @param outLodConfig LOD configuration storing the output.
	 */
	virtual void getAutoconfig(MeshPtr& inMesh, LodConfig& outLodConfig);

	virtual ~ProgressiveMeshGeneratorBase() { }
};

/**
 * @brief Improved version of ProgressiveMesh.
 */
class _OgreExport ProgressiveMeshGenerator :
	public ProgressiveMeshGeneratorBase
{
public:

	ProgressiveMeshGenerator();
	virtual ~ProgressiveMeshGenerator();

	/// @copydoc ProgressiveMeshGeneratorBase::generateLodLevels
	void generateLodLevels(LodConfig& lodConfig);

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
	struct PMVertexHash;
	struct PMVertexEqual;
	struct PMCollapseCostLess;
	struct PMCollapsedEdge;
	struct PMIndexBufferInfo;

	typedef vector<PMVertex>::type VertexList;
	typedef vector<PMTriangle>::type TriangleList;
	typedef HashSet<PMVertex*, PMVertexHash, PMVertexEqual> UniqueVertexSet;
	typedef multimap<Real, PMVertex*>::type CollapseCostHeap;
	typedef vector<PMVertex*>::type VertexLookupList;

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

		PMVertex* collapseTo;
		bool seam;
		CollapseCostHeap::iterator costHeapPosition; /// Iterator pointing to the position in the mCollapseCostSet, which allows fast remove.
	};

	struct _OgrePrivate PMTriangle {
		PMVertex* vertex[3];
		Vector3 normal;
		bool isRemoved;
		unsigned short submeshID; /// ID of the submesh. Usable with mMesh.getSubMesh() function.
		unsigned int vertexID[3]; /// Vertex ID in the buffer associated with the submeshID.

		void computeNormal();
		bool hasVertex(const PMVertex* v) const;
		unsigned int getVertexID(const PMVertex* v) const;
		bool isMalformed();
	};

	struct _OgrePrivate PMIndexBufferInfo {
		size_t indexSize;
		size_t indexCount;
	};

	union _OgrePrivate IndexBufferPointer {
		unsigned short* pshort;
		unsigned int* pint;
	};

	struct _OgrePrivate PMCollapsedEdge {
		unsigned int srcID;
		unsigned int dstID;
		unsigned short submeshID;
	};

	VertexLookupList mSharedVertexLookup;
	VertexLookupList mVertexLookup;
	VertexList mVertexList;
	TriangleList mTriangleList;
	UniqueVertexSet mUniqueVertexSet;
	CollapseCostHeap mCollapseCostHeap;
	CollapsedEdges tmpCollapsedEdges; // Tmp container used in collapse().
	IndexBufferInfoList mIndexBufferInfoList;

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

	size_t calcLodVertexCount(const LodLevel& lodConfig);
	void tuneContainerSize();
	void addVertexData(VertexData* vertexData, bool useSharedVertexLookup);
	void addIndexData(IndexData* indexData, bool useSharedVertexLookup, unsigned short submeshID);
    template<typename IndexType>
    void addIndexDataImpl(IndexType* iPos, const IndexType* iEnd,
                                                    VertexLookupList& lookup,
                                                    unsigned short submeshID)
    {

        // Loop through all triangles and connect them to the vertices.
        for (; iPos < iEnd; iPos += 3) {
            // It should never reallocate or every pointer will be invalid.
            OgreAssert(mTriangleList.capacity() > mTriangleList.size(), "");
            mTriangleList.push_back(PMTriangle());
            PMTriangle* tri = &mTriangleList.back();
            tri->isRemoved = false;
            tri->submeshID = submeshID;
            for (int i = 0; i < 3; i++) {
                // Invalid index: Index is bigger then vertex buffer size.
                OgreAssert(iPos[i] < lookup.size(), "");
                tri->vertexID[i] = iPos[i];
                tri->vertex[i] = lookup[iPos[i]];
            }
            if (tri->isMalformed()) {
#if OGRE_DEBUG_MODE
                stringstream str;
                str << "In " << mMeshName << " malformed triangle found with ID: " << getTriangleID(tri) << ". " <<
                std::endl;
                printTriangle(tri, str);
                str << "It will be excluded from LOD level calculations.";
                LogManager::getSingleton().stream() << str.str();
#endif
                tri->isRemoved = true;
                mIndexBufferInfoList[tri->submeshID].indexCount -= 3;
                continue;
            }
            tri->computeNormal();
            addTriangleToEdges(tri);
        }
    }

	void computeCosts();
	bool isBorderVertex(const PMVertex* vertex) const;
	PMEdge* getPointer(VEdges::iterator it);
	void computeVertexCollapseCost(PMVertex* vertex);
	Real computeEdgeCollapseCost(PMVertex* src, PMEdge* dstEdge);
	virtual void bakeLods();
	void collapse(PMVertex* vertex);
	void initialize();
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
};

}
#endif
