/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2012 Torus Knot Software Ltd

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
// The underlying algorithms in this class are based heavily on:
/*
 *  Progressive Mesh type Polygon Reduction Algorithm
 *  by Stan Melax (c) 1998
 */

#ifndef __ProgressiveMesh_H_
#define __ProgressiveMesh_H_

#include "OgrePrerequisites.h"
#include "OgreVector3.h"
#include "OgreVector2.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreHardwareIndexBuffer.h"
#include "OgreRenderOperation.h"
#include "OgreSmallVector.h"

namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup LOD
	*  @{
	*/
	class Mesh;
	class SubMesh;
	
	class _OgreExport BitArray
	{
	public:
		BitArray()					: bits_ptr(NULL) {}
		BitArray(int bits_count)	: bits_ptr(NULL) { resize(bits_count); }
		BitArray& operator=(const BitArray& ba)	{ bits = ba.bits; bits_ptr = bits.size() > 0 ? &bits.front() : NULL; return *this; }
		
		bool getBit(size_t i) const	{ return (bits_ptr[i >> 3] & bit_mask[i & 7]) != 0; }
		void setBit(size_t i)		{ bits_ptr[i >> 3] |= bit_mask[i & 7]; }
		void clearBit(size_t i)		{ bits_ptr[i >> 3] &= ~bit_mask[i & 7]; }
		void clearAllBits()			{ memset(bits_ptr, 0, bits.size()); }
		
		bool empty() const			{ return bits.empty(); }
		void resize(size_t bits_count)
		{		
			bits.resize((bits_count + 7) / 8);
			bits_ptr = bits.size() > 0 ? &bits.front() : NULL;
			clearAllBits();
		}
		
		size_t getBitsCount() const
		{
			size_t count = 0;
			for(unsigned char *ptr = bits_ptr, *end_ptr = bits_ptr + bits.size(); ptr != end_ptr; ++ptr)
			{
				const unsigned char b = *ptr;
				count += bit_count[b & 0xF] + bit_count[b >> 4];
			}
			return count;
		}
		
	private:
		unsigned char*				bits_ptr;		// it`s so performance critical, so we place raw data pointer before all other members
		vector<unsigned char>::type	bits;
		
		const static unsigned char	bit_mask[8];	// = { 1, 2, 4, 8, 16, 32, 64, 128 };
		const static unsigned char	bit_count[16];	// = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };
	};
	
    /** This class reduces the complexity of the geometry it is given.
        This class is dedicated to reducing the number of triangles in a given mesh
        taking into account seams in both geometry and texture co-ordinates and meshes 
        which have multiple frames.
    @par
        The primary use for this is generating LOD versions of Mesh objects, but it can be
        used by any geometry provider. The only limitation at the moment is that the 
        provider uses a common vertex buffer for all LODs and one index buffer per LOD.
        Therefore at the moment this class can only handle indexed geometry.
    @par
        NB the interface of this class will certainly change when compiled vertex buffers are
        supported.
    */
	class _OgreExport ProgressiveMesh : public ProgMeshAlloc
    {
    public:
		typedef vector<Real>::type LodValueList;
		
		/** The way to derive the quota of vertices which are reduced at each LOD. */
        enum VertexReductionQuota
		{
			/// A set number of vertices are removed at each reduction
			VRQ_CONSTANT,
			/// A proportion of the remaining number of vertices are removed at each reduction
			VRQ_PROPORTIONAL,
			/// All vertices with reduction error cost less than reductionValue * sqr(lodDistance[lodLevel] / lodDistance[0]) 
			/// are removed at each reduction. Error cost is calculated as introduced error area divided by squared mesh diagonal
			VRQ_ERROR_COST
		};

		
		/** Automatically generates lower level of detail versions of this mesh for use
			 when a simpler version of the model is acceptable for rendering.
		 @remarks
			 There are 2 ways that you can create level-of-detail (LOD) versions of a mesh;
			 the first is to call this method, which does fairly extensive calculations to
			 work out how to simplify the mesh whilst having the minimum affect on the model.
			 The alternative is to actually create simpler versions of the mesh yourself in 
			 a modelling tool, and having exported them, tell the 'master' mesh to use these
			 alternative meshes for lower detail versions; this is done by calling the 
			 createManualLodLevel method.
		 @par
			 As well as creating the lower detail versions of the mesh, this method will
			 also associate them with depth values. As soon as an object is at least as far
			 away from the camera as the depth value associated with it's LOD, it will drop 
			 to that level of detail. 
		 @par
			 I recommend calling this method before mesh export, not at runtime.
			 @param lodValues A list of lod values indicating the values at which new lods should be
			 generated. These are 'user values', before being potentially 
			 transformed by the strategy, so for the distance strategy this is an
			 unsquared distance for example.
		 @param reductionMethod The way to determine the number of vertices collapsed per LOD
		 @param reductionValue Meaning depends on reductionMethod, typically either the proportion
			 of remaining vertices to collapse or a fixed number of vertices.
		 */
		static bool generateLodLevels(Mesh* pMesh, const LodValueList& lodValues,
									  VertexReductionQuota reductionMethod, Real reductionValue);

		/** Automatically generates lower level of detail versions of this mesh for use
			when a simpler version of the model is acceptable for rendering. 
		 @remarks
			Useful for importing of external mesh with unknown size and structure into something manageable.
		 @par
			Simplifies vertex structure to { pos, norm, tex0 } stored in single stream.
			Removes unused vertices, performing reindexing.
		 @par
			Can optionally discard first LOD level (i.e. original geometry), unused vertices would be removed.
		 */
		static MeshPtr generateSimplifiedMesh(const String& name, const String& groupName, Mesh* inMesh,
											  bool dropOriginalGeometry, const LodValueList& lodValues,
											  VertexReductionQuota reductionMethod, Real reductionValue,
											  size_t* removedVertexDuplicatesCount);
	protected:
		typedef vector<ProgressiveMesh*>::type ProgressiveMeshList;

		/// Allocates internal resources
		static void initializeProgressiveMeshList(ProgressiveMeshList& pmList, Mesh* pMesh);

		/// Deletes allocated internal resources.
		static void freeProgressiveMeshList(ProgressiveMeshList* pmList);

        /** Constructor, takes SubMesh pointer. 
		@remarks
			DO NOT pass write-only, unshadowed buffers to this method! They will not
			work. Pass only shadowed buffers, or better yet perform mesh reduction as
			an offline process using DefaultHardwareBufferManager to manage vertex
			buffers in system memory.
		*/
		ProgressiveMesh(SubMesh* pSubMesh);
        virtual ~ProgressiveMesh();

        /** Adds an extra vertex position buffer. 
        @remarks
            As well as the main vertex buffer, the client of this class may add extra versions
            of the vertex buffer which will also be taken into account when the cost of 
            simplifying the mesh is taken into account. This is because the cost of
            simplifying an animated mesh cannot be calculated from just the reference position,
            multiple positions needs to be assessed in order to find the best simplification option.
		@par
			DO NOT pass write-only, unshadowed buffers to this method! They will not
			work. Pass only shadowed buffers, or better yet perform mesh reduction as
			an offline process using DefaultHardwareBufferManager to manage vertex
			buffers in system memory.
        @param buffer Pointer to x/y/z buffer with vertex positions. The number of vertices
            must be the same as in the original GeometryData passed to the constructor.
        */
        virtual void addExtraVertexPositionBuffer(const VertexData* vertexData);

        /** Builds the progressive mesh with the specified number of levels.
        @param numLevels The number of levels to include in the output excluding the full detail version.
        @param outList Pointer to a list of LOD geometry data which will be completed by the application.
			Each entry is a reduced form of the mesh, in decreasing order of detail.
		@param quota The way to derive the number of vertices removed at each LOD
		@param reductionValue Either the proportion of vertices to remove at each level, or a fixed
			number of vertices to remove at each level, depending on the value of quota
        */
		static bool build(ProgressiveMeshList& pmInList,
						  const LodStrategy *lodStrategy, const LodValueList& lodValues,
						  VertexReductionQuota quota, Real reductionValue = 0.5f);
						
    protected:
		/// Can be NULL for non-indexed subMeshes, such PM would be skipped
		SubMesh* mSubMesh;
		
        VertexData *mVertexData;
        IndexData *mIndexData;

		vector<IndexData*>::type mLodFaceList;
		
		size_t mRemovedVertexDuplicatesCount;	
		size_t mCurrNumIndexes;
		Real mInvSquaredBoundBoxDiagonal;
		int mVertexComponentFlags;	

        // Internal classes
        class PMTriangle;
        class PMVertex;
		struct vertexLess;

    public: // VC6 hack

        /** A vertex as used by a face. This records the index of the actual vertex which is used
		by the face, and a pointer to the common vertex used for surface evaluation. */
		class _OgrePrivate PMFaceVertex {
		public:
			size_t realIndex;
			PMVertex* commonVertex;
		};

	protected:

        /** A triangle in the progressive mesh, holds extra info like face normal. */
        class _OgrePrivate PMTriangle {
        public:
            PMTriangle();
            void setDetails(size_t index, PMFaceVertex *v0, PMFaceVertex *v1, PMFaceVertex *v2);
	        void computeNormal(void);
	        void replaceVertex(PMFaceVertex *vold, PMFaceVertex *vnew);
	        bool hasCommonVertex(PMVertex *v) const;
	        bool hasFaceVertex(PMFaceVertex *v) const;
			PMFaceVertex* getFaceVertexFromCommon(PMVertex* commonVert);
	        void notifyRemoved(void);

	        PMFaceVertex*	vertex[3];	// the 3 points that make this tri
	        Vector3			normal;		// unit vector orthogonal to this face
			Real			area;
            bool			removed;	// true if this tri is now removed
			size_t			index;
        };

        /** A vertex in the progressive mesh, holds info like collapse cost etc. 
		This vertex can actually represent several vertices in the final model, because
		vertices along texture seams etc will have been duplicated. In order to properly
		evaluate the surface properties, a single common vertex is used for these duplicates,
		and the faces hold the detail of the duplicated vertices.
		*/
        class _OgrePrivate PMVertex {
        public:
			enum BorderStatus { BS_UNKNOWN = 0, BS_NOT_BORDER, BS_BORDER };
            typedef SmallVector<PMVertex *, 8> NeighborList;
	        typedef SmallVector<PMTriangle *, 8> FaceList;

		public:
            PMVertex() : mBorderStatus(BS_UNKNOWN), removed(false) {}

			void setDetails(size_t index, const Vector3& pos, const Vector3& normal, const Vector2& uv);
		
			bool isNearEnough(PMVertex* other) const;
	        void removeIfNonNeighbor(PMVertex *n);
			void initBorderStatus(void);/// Set mBorderStatus to BS_BORDER if this vertex is on the edge of an open geometry patch
			bool isManifoldEdgeWith(PMVertex* v); // is edge this->src a manifold edge?
			void notifyRemoved(void);
			void calculateNormal();
		
            Vector3 position;  // location of point in euclidean space
			Vector3 normal;
			Vector2 uv;
			
	        size_t index;       // place of vertex in original list

			BorderStatus mBorderStatus;			
            bool      removed;   // true if this vert is now removed
			bool	  toBeRemoved; // debug

	        Real collapseCost;  // cached cost of collapsing edge
	        PMVertex * collapseTo; // candidate vertex for collapse
			
            NeighborList neighbor; // adjacent vertices
            FaceList face;     // adjacent triangles
        };
		
        typedef vector<PMTriangle>::type TriangleList;
        typedef vector<PMFaceVertex>::type FaceVertexList;
        typedef vector<PMVertex>::type CommonVertexList;
		typedef std::pair<Real, unsigned int> CostIndexPair;
		typedef vector<CostIndexPair>::type WorstCostList;

        /// Data used to calculate the collapse costs
        struct PMWorkingData
        {
            TriangleList mTriList; /// List of faces
            FaceVertexList mFaceVertList; // The vertex details referenced by the triangles
			CommonVertexList mVertList; // The master list of common vertices
        };

        typedef vector<PMWorkingData>::type WorkingDataList;
        /// Multiple copies, 1 per vertex buffer
        WorkingDataList mWorkingData;

        /// The worst collapse cost from all vertex buffers for each vertex
        WorstCostList	mWorstCosts;		// sorted by cost, but some of entries are invalidated, so check invalidCostMask
		BitArray		mInvalidCostMask;	// indexed by vertex index
		size_t			mInvalidCostCount;
		size_t			mWorstCostsSize;
		size_t			mNextWorstCostHint;	// getNextCollapser() uses it to reduce complexity from O(n^2) to O(n)
			
		/// Temporary variable used in computeEdgeCollapseCost, declared here to avoid multiple memory allocations
		mutable PMVertex::FaceList mEdgeAdjacentSides;

        /// Internal method for building PMWorkingData from geometry data
        void addWorkingData(const VertexData* vertexData, const IndexData* indexData);
		void mergeWorkingDataBorders();
		
        /// Internal method for initialising the edge collapse costs
        void initialiseEdgeCollapseCosts(void);
        /// Internal calculation method for deriving a collapse cost  from u to v
        Real computeEdgeCollapseCost(PMVertex *src, PMVertex *dest) const;
        /// Internal calculation method, return true if edge collapse flip some neighbor face normal
        bool collapseInvertsNormals(PMVertex *src, PMVertex *dest) const;
        /// Internal method evaluates all collapse costs from this vertex and picks the lowest for a single buffer
        Real computeEdgeCostAtVertexForBuffer(PMVertex* v);
        /// Internal method evaluates all collapse costs from this vertex for every buffer and returns the worst
        Real computeEdgeCostAtVertex(size_t vertIndex);
        /// Internal method to compute edge collapse costs for all buffers /
        void computeAllCosts(void);

        /// Internal methods for lazy costs recomputing
		static size_t getInvalidCostCount(ProgressiveMesh::ProgressiveMeshList& pmList);
		static bool recomputeInvalidCosts(ProgressiveMeshList& pmInList);
		void recomputeInvalidCosts();
		void sortIndexesByCost();
		static int cmpByCost(const void* p1, const void* p2); // comparator for mWorstCosts sorting
		
        /// Internal methods for getting the index of next best vertex to collapse among all submeshes
		static void getNextCollapser(ProgressiveMeshList& pmList, ProgressiveMesh*& pm, CostIndexPair*& bestCollapser);
		CostIndexPair* getNextCollapser();
		
        /// Internal method builds an new LOD based on the current state
        void bakeNewLOD(IndexData* pData);
		/// Internal method builds an LODs usage, possibly skipping first LOD, that can be used as original geometry
		static void bakeLodUsage(Mesh* pMesh, LodStrategy *lodStrategy, const LodValueList& lodValues, bool skipFirstLodLevel = false);

        /** Internal method, collapses vertex onto it's saved collapse target. 
        @remarks
            This updates the working triangle list to drop a triangle and recalculates
            the edge collapse costs around the collapse target. 
            This also updates all the working vertex lists for the relevant buffer. 
        */
        void collapse(PMVertex *collapser);

		/// We can defragment mesh, removing unused vertices and re-indexing other, storing old-to-new mapping in index map
		typedef std::pair<unsigned, PMVertex*> IndexVertexPair;
		/// Optionally discards first LOD level (i.e. original geometry), removes unused vertices, remaps indexes.
		static void bakeSimplifiedMesh(Mesh* destMesh, Mesh* srcMesh, ProgressiveMeshList& pmList, bool dropFirstLodLevel = false);
		/// Defragments vertices, removing unused. Useful if original geometry is redundant or dropped at all.
		static void	createSimplifiedVertexData(vector<IndexVertexPair>::type& usedVertices, VertexData* inVData, VertexData*& outVData, AxisAlignedBox& aabox);
		/// During vertices defragmentation vertices are re-indexed, so old-to-new mapping is stored in index map by this function.
		static void createIndexMap(vector<IndexVertexPair>::type& usedVertices, unsigned allVertexCount, vector<unsigned>::type& indexMap);
		
		/** Internal debugging method */
		void dumpContents(const String& log);
    };
			
	template <typename T> struct HardwareBufferLockGuard
	{
		HardwareBufferLockGuard(const T& p, HardwareBuffer::LockOptions options)
		: pBuf(p)
		{
			pData = pBuf->lock(options);
		}
		HardwareBufferLockGuard(const T& p, size_t offset, size_t length, HardwareBuffer::LockOptions options)
		: pBuf(p)
		{
			pData = pBuf->lock(offset, length, options);
		}		
		~HardwareBufferLockGuard()
		{
			pBuf->unlock();
		}
		const T& pBuf;
		void* pData;
	};
	
	typedef HardwareBufferLockGuard<HardwareVertexBufferSharedPtr> VertexBufferLockGuard;
	typedef HardwareBufferLockGuard<HardwareIndexBufferSharedPtr> IndexBufferLockGuard;
	
	/** @} */
	/** @} */
}

#endif 
