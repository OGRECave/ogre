/*
-----------------------------------------------------------------------------
This source file is part of OGRE 
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
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
#ifndef __XSIMESHEXPORTER_H__
#define __XSIMESHEXPORTER_H__

#include "OgrePrerequisites.h"
#include "OgreVector2.h"
#include "OgreVector3.h"
#include "OgreColourValue.h"
#include "OgreMesh.h"
#include "OgreXSIHelper.h"
#include <xsi_x3dobject.h>
#include <xsi_string.h>
#include <xsi_application.h>
#include <xsi_geometry.h>
#include <xsi_triangle.h>
#include <xsi_polygonface.h>
#include <xsi_facet.h>
#include <xsi_point.h>
#include <xsi_polygonmesh.h>
#include <xsi_shapekey.h>
#include <xsi_clip.h>
#include <xsi_clipcontainer.h>


namespace Ogre {

    /** Class for performing a mesh export from XSI.
    */
    class XsiMeshExporter
    {
    public:
        XsiMeshExporter();
        virtual ~XsiMeshExporter();

		struct LodData
		{
			Mesh::LodDistanceList distances;
			ProgressiveMesh::VertexReductionQuota quota;
			Real reductionValue;
		};

		

		
		/** Build an OGRe mesh ready for export.
        @remarks
            Every PolygonMesh object is exported as a different SubMesh. Other
            object types are ignored.
		@param mergeSubMeshes Whether to merge submeshes with the same material
		@param exportChildren Whether to cascade down each objects children
        @param edgeLists Whether to calculate edge lists
        @param tangents Whether to calculate tangents
		@param animList List of animations in use (of any type)
		@param materialPrefix Prefix to give all materials
		@param lod LOD generation parameters (if required)
		@param skeletonName Name of the skeleton to link to if animated
		@returns List of deformers (bones) which were found whilst exporting (if
			skeletonName was provided) which can be used to determine the skeleton.
        */
        DeformerMap& buildMeshForExport(
            bool mergeSubMeshes, bool exportChildren, bool edgeLists, 
			bool tangents, VertexElementSemantic tangentSemantic, bool tangentsSplitMirrored, bool tangentsSplitRotated, bool tangentsUseParity,
			bool vertexAnimation, AnimationList& animList, Real fps, const String& materialPrefix = StringUtil::BLANK,
			LodData* lod = 0, const String& skeletonName = "");

		/** Perform final export of built mesh. 
		@param fileName Target file name
		@param skelAABB AABB of skeleton bones as found in animation, to pad
			final bounds of mesh.
		*/
		void exportMesh(const String& fileName, const AxisAlignedBox& skelAABB);

		/** Get a list of materials which were located during the last call
		 *  to exportMesh. 
		 */
		MaterialMap& getMaterials(void);

		/** Get the map from texture projection names to uv indexes. */
		TextureProjectionMap& getTextureProjectionMap(void);
    protected:
		MeshPtr mMesh;
        // XSI Objects
        XSI::Application mXsiApp;
        /** This struct represents a unique vertex, identified from a unique 
        combination of components.
        */
        class UniqueVertex
        {
        public:
            Vector3 position;
            Vector3 normal;
            Vector3 uv[OGRE_MAX_TEXTURE_COORD_SETS];
            RGBA colour;
            // The index of the next component with the same base details
            // but with some variation
            size_t nextIndex;

            UniqueVertex();
            bool operator==(const UniqueVertex& rhs) const;

        };
        typedef std::vector<UniqueVertex> UniqueVertexList;
		// dynamic index list; 32-bit until we know the max vertex index
		typedef std::vector<uint32> IndexList;

		/** An entry for a PolygonMesh - need the parent X3DObject too */
		class PolygonMeshEntry
		{
		public:
			XSI::PolygonMesh mesh;
			XSI::X3DObject obj;

			PolygonMeshEntry(XSI::PolygonMesh& themesh, XSI::X3DObject& theobj)
				:mesh(themesh), obj(theobj)
			{
			}

		};
		/// ordering function, required for set
		struct PolygonMeshEntryLess
		{
			bool operator()(PolygonMeshEntry* lhs, PolygonMeshEntry* rhs)
			{
				// can't name objects the same in XSI, so use that
				return XSItoOgre(lhs->obj.GetName()) < XSItoOgre(rhs->obj.GetName());
			}
		};

		/** Set of polygon mesh objects we're going to process 
		  * Use a set to avoid exporting the same object twice when manually selected
		  * and as a child of a selected object.
		  */
		typedef std::set<PolygonMeshEntry*,PolygonMeshEntryLess> PolygonMeshList;
		PolygonMeshList mXsiPolygonMeshList;


	
		/// List of deformers we've found whilst parsing the objects
		DeformerMap mXsiDeformerMap;
		/// LIst of materials we've found whilst parsing the objects
		MaterialMap mXsiMaterialMap;
		/// Map from texture projection names to uv index
		TextureProjectionMap mTextureProjectionMap;
		/// Material prefix
		String mMaterialPrefix;


		/// Build a list of PolygonMesh instances from selection
		void buildPolygonMeshList(bool includeChildren);
		/// Tidy up
		void cleanupPolygonMeshList(void);
		/// Recursive method to locate PolygonMeshes
		void findPolygonMeshes(XSI::X3DObject& x3dObj, bool recurse);
		/// Build the mesh
		void buildMesh(Mesh* pMesh, bool mergeSubmeshes, bool lookForBoneAssignments, 
			bool vertexAnimation, AnimationList& animList, Real fps);
		/// Process a single PolygonMesh into one or more ProtoSubMeshes
		void processPolygonMesh(Mesh* pMesh, PolygonMeshEntry* pm, bool lookForBoneAssignments, unsigned short progressUpdates);
		/// Find deformers and bone assignments
		void processBoneAssignments(Mesh* pMesh, PolygonMeshEntry* pm);
		/// Find shape keys for a given mesh
		void processShapeKeys(Mesh* pMesh, PolygonMeshEntry* pm);
		
		/// Tidy up
		void cleanupDeformerMap(void);
		void cleanupMaterialMap(void);

		typedef std::map<size_t, size_t> IndexRemap;
		/** Working area which will become a submesh once we've finished figuring
			out what goes in there.
		*/
		struct ProtoSubMesh
		{
			// Name of the submesh (may be blank if we're merging)
			String name;
			// Material name
			String materialName;
			// unique vertex list
			UniqueVertexList uniqueVertices;
			// Defines number of texture coord sets and their dimensions
			std::vector<ushort> textureCoordDimensions;
			// Vertex colours?
			bool hasVertexColours;
			// Last polymesh entry added to this proto
			PolygonMeshEntry* lastMeshEntry;
			// Index offset for last polymesh entry
			size_t lastMeshIndexOffset;
			// index list
			IndexList indices;
			// map of polygon mesh -> position index offset (only > 0 when submeshes merged)
			typedef std::map<PolygonMeshEntry*, size_t> PolygonMeshOffsetMap;
			PolygonMeshOffsetMap polygonMeshOffsetMap;
			// map original position index (+any PM offset) -> first real instance in this one
			IndexRemap posIndexRemap;
			Mesh::VertexBoneAssignmentList boneAssignments;
			/// By-value pose list, build up ready for transfer later
			std::list<Pose> poseList;
			/// List of XSI shape keys which are being used in this proto
			XSI::CRefArray shapeKeys;

			ProtoSubMesh() : lastMeshEntry(0), lastMeshIndexOffset(0) {}

			
		};

		/// Global shape key to pose mapping
		struct ShapeKeyToPoseEntry
		{
			XSI::CRef shapeKey;
			size_t poseIndex;
			size_t targetHandle;

		};
		typedef std::list<ShapeKeyToPoseEntry> ShapeKeyMapping;
		ShapeKeyMapping mShapeKeyMapping;

		struct ShapeClipEntry
		{
			XSI::Clip clip;
			ShapeKeyToPoseEntry* keytoPose;
			long startFrame;
			long endFrame;
			long originalStartFrame; // in case clamped
		};
		typedef std::list<ShapeClipEntry> ShapeClipList;


		/// List of ProtoSubMeshes that use the same material but are not geometrically compatible
		typedef std::list<ProtoSubMesh*> ProtoSubMeshList;

		/// List of proto submeshes by material
		typedef std::map<String, ProtoSubMeshList*> MaterialProtoSubMeshMap;
		/// List of proto submeshes by material
		MaterialProtoSubMeshMap mMaterialProtoSubmeshMap;
		/// List of deviant proto submeshes by polygon index (clusters)
		typedef std::map<size_t, ProtoSubMesh*> PolygonToProtoSubMeshList;
		/// List of deviant proto submeshes by polygon index (clusters)
		PolygonToProtoSubMeshList mPolygonToProtoSubMeshList;
		/// Primary ProtoSubMesh (the one used by the PolygonMesh by default)
		ProtoSubMesh* mMainProtoMesh;
		// Current PolygonMesh texture coord information
		typedef std::vector<ushort> TextureCoordDimensionList;
		TextureCoordDimensionList mCurrentTextureCoordDimensions;
		// Current PolygonMesh sampler-ordered UV information
		typedef std::vector<Vector3*> SamplerSetList;
		SamplerSetList mCurrentSamplerSets;
		// Current PolygonMesh has Vertex colours?
		bool mCurrentHasVertexColours;
		// Current list of deformers as we discover them (will become bones)

		/// Export the current list of proto submeshes, and clear list
		void exportProtoSubMeshes(Mesh* pMesh);
        /// Export a single ProtoSubMesh 
        void exportProtoSubMesh(Mesh* pMesh, ProtoSubMesh* proto);
		/// Export vertex animations
		void exportAnimations(Mesh* pMesh, AnimationList& animList, Real fps);
		/// Build a list of all shape clips
		void buildShapeClipList(ShapeClipList& listToPopulate);
		/// Build a list of all shape clips in a container
		void buildShapeClipList(XSI::ClipContainer& container, ShapeClipList& listToPopulate);
		/// Build a derived clip list for just a specific submesh, and a list of keys to sample
		void deriveShapeClipAndKeyframeList(ushort targetIndex, 
			AnimationEntry& animEntry, ShapeClipList& inClipList, 
			ShapeClipList& outClipList, std::set<long>& keyFrameList);
		/// Retrieve a ProtoSubMesh for the given material name 
		/// (creates if required, validates if re-using)
		ProtoSubMesh* createOrRetrieveProtoSubMesh(const String& materialName,
			const String& name, TextureCoordDimensionList& texCoordDims,
			bool hasVertexColours);
		/// Method called at the start of processing a PolygonMesh
		bool preprocessPolygonMesh(PolygonMeshEntry* mesh);
		/// Method called at the end of processing a PolygonMesh
		void postprocessPolygonMesh(PolygonMeshEntry* mesh);

        /** Try to look up an existing vertex with the same information, or
            create a new one.
        @remarks
			Note that we buid up the list of unique position indexes that are
			actually used by each ProtoSubMesh as we go. When new positions
			are found, they are added and a remap entry created to take account
			of the fact that there may be extra vertices created in between, or
			there may be gaps due to clusters meaning not every position is
			used by every ProtoSubMesh. When an existing entry is found, we
			compare the vertex data, and if it differs, create a new vertex and
			'chain' it to the previous instances of this position index through
			nextIndex. This means that every position vertex has a single 
			remapped starting point in the per-ProtoSubMesh vertex list, and a 
			unidirectional linked list of variants of that vertex where other
			components differ.
        @par
            Note that this re-uses as many vertices as possible, and also places
            every unique vertex in it's final index in one pass, so the return 
			value from this method can be used as an adjusted vertex index.
        @returns The index of the unique vertex
        */
        size_t createOrRetrieveUniqueVertex(ProtoSubMesh* proto, 
			size_t positionIndex, bool positionIndexIsOriginal,
			const UniqueVertex& vertex);

        /** Templatised method for writing indexes */
        template <typename T> void writeIndexes(T* buf, IndexList& indexes);

        /** Create and fill a vertex buffer */
        void createVertexBuffer(VertexData* vd, unsigned short bufIdx, 
			UniqueVertexList& uniqueVertexList);

		/** Find out the sampler indices for the given triangle */
		void deriveSamplerIndices(const XSI::Triangle& tri, const XSI::PolygonFace& face, 
			size_t* samplerIndices);
		/** Get a single sampler index */
		size_t getSamplerIndex(const XSI::Facet &f, const XSI::Point &p);

		/** Register the use of a given XSI material. */
		void registerMaterial(const String& name, XSI::Material mat);



    };

}
#endif

