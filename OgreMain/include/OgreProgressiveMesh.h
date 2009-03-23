/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
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
#include "OgreHardwareVertexBuffer.h"
#include "OgreHardwareIndexBuffer.h"
#include "OgreRenderOperation.h"

namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup LOD
	*  @{
	*/
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

		/** The way to derive the quota of vertices which are reduced at each LOD. */
        enum VertexReductionQuota
		{
			/// A set number of vertices are removed at each reduction
			VRQ_CONSTANT,
			/// A proportion of the remaining number of vertices are removed at each reduction
			VRQ_PROPORTIONAL
		};

        typedef vector<IndexData*>::type LODFaceList;

        /** Constructor, takes the geometry data and index buffer. 
		@remarks
			DO NOT pass write-only, unshadowed buffers to this method! They will not
			work. Pass only shadowed buffers, or better yet perform mesh reduction as
			an offline process using DefaultHardwareBufferManager to manage vertex
			buffers in system memory.
		*/
        ProgressiveMesh(const VertexData* vertexData, const IndexData* indexData);
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
        virtual void build(ushort numLevels, LODFaceList* outList, 
			VertexReductionQuota quota = VRQ_PROPORTIONAL, Real reductionValue = 0.5f );

    protected:
        const VertexData *mpVertexData;
        const IndexData *mpIndexData;

        size_t mCurrNumIndexes;
		size_t mNumCommonVertices;

        // Internal classes
        class PMTriangle;
        class PMVertex;

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

	        PMFaceVertex* vertex[3]; // the 3 points that make this tri
	        Vector3   normal;    // unit vector orthogonal to this face
            bool      removed;   // true if this tri is now removed
			size_t index;
        };

        /** A vertex in the progressive mesh, holds info like collapse cost etc. 
		This vertex can actually represent several vertices in the final model, because
		vertices along texture seams etc will have been duplicated. In order to properly
		evaluate the surface properties, a single common vertex is used for these duplicates,
		and the faces hold the detail of the duplicated vertices.
		*/
        class _OgrePrivate PMVertex {
        public:
            PMVertex();
	        void setDetails(const Vector3& v, size_t index);
	        void removeIfNonNeighbor(PMVertex *n);
			bool isBorder(void);/// true if this vertex is on the edge of an open geometry patch
			bool isManifoldEdgeWith(PMVertex* v); // is edge this->src a manifold edge?
			void notifyRemoved(void);

            Vector3  position;  // location of point in euclidean space
	        size_t index;       // place of vertex in original list
            typedef set<PMVertex *>::type NeighborList;
            typedef set<PMVertex *>::type DuplicateList;
            NeighborList neighbor; // adjacent vertices
	        typedef set<PMTriangle *>::type FaceList;
            FaceList face;     // adjacent triangles

	        Real collapseCost;  // cached cost of collapsing edge
	        PMVertex * collapseTo; // candidate vertex for collapse
            bool      removed;   // true if this vert is now removed
			bool	  toBeRemoved; // denug

			bool seam;	/// true if this vertex is on a model seam where vertices are duplicated

        };

        typedef vector<PMTriangle>::type TriangleList;
        typedef vector<PMFaceVertex>::type FaceVertexList;
        typedef vector<PMVertex>::type CommonVertexList;
        typedef vector<Real>::type WorstCostList;

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
        WorstCostList mWorstCosts;

        /// Internal method for building PMWorkingData from geometry data
        void addWorkingData(const VertexData* vertexData, const IndexData* indexData);

        /// Internal method for initialising the edge collapse costs
        void initialiseEdgeCollapseCosts(void);
        /// Internal calculation method for deriving a collapse cost  from u to v
        Real computeEdgeCollapseCost(PMVertex *src, PMVertex *dest);
        /// Internal method evaluates all collapse costs from this vertex and picks the lowest for a single buffer
        Real computeEdgeCostAtVertexForBuffer(WorkingDataList::iterator idata, size_t vertIndex);
        /// Internal method evaluates all collapse costs from this vertex for every buffer and returns the worst
        void computeEdgeCostAtVertex(size_t vertIndex);
        /// Internal method to compute edge collapse costs for all buffers /
        void computeAllCosts(void);
        /// Internal method for getting the index of next best vertex to collapse
        size_t getNextCollapser(void);
        /// Internal method builds an new LOD based on the current state
        void bakeNewLOD(IndexData* pData);

        /** Internal method, collapses vertex onto it's saved collapse target. 
        @remarks
            This updates the working triangle list to drop a triangle and recalculates
            the edge collapse costs around the collapse target. 
            This also updates all the working vertex lists for the relevant buffer. 
        */
        void collapse(PMVertex *collapser);

		/** Internal debugging method */
		void dumpContents(const String& log);









    };


	/** @} */
	/** @} */

}

#endif 
