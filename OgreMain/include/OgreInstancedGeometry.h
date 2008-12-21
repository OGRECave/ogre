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
#ifndef __InstancedGeometry_H__
#define __InstancedGeometry_H__

#include "OgrePrerequisites.h"
#include "OgreMovableObject.h"
#include "OgreSimpleRenderable.h"
#include "OgreSkeleton.h"
#include "OgreSkeletonInstance.h"
#include "OgreAnimationTrack.h"
#include "OgreBone.h"
#include "OgreIteratorWrappers.h"
#include "OgreMesh.h"

namespace Ogre {

	/** Pre-transforms and batches up meshes for efficient use as instanced geometry
		in a scene
	@remarks
		Shader instancing allows to save both memory and draw calls. While 
		StaticGeometry stores 500 times the same object in a batch to display 500 
		objects, this shader instancing implementation stores only 80 times the object, 
		and then re-uses the vertex data with different shader parameter.
		Although you save memory, you make more draw call. However, you still 
		make less draw calls than if you were rendering each object independently.
		Plus, you can move the batched objects independently of one another which 
		you cannot do with StaticGeometry.
	@par
		Therefore it is important when you are rendering a lot of geometry to 
		batch things up into as few rendering calls as possible. This
		class allows you to build a batched object from a series of entities 
		in order to benefit from this behaviour.
		Batching has implications of it's own though:
		@li Batched geometry cannot be subdivided; that means that the whole
			group will be displayed, or none of it will. This obivously has
			culling issues.
		@li A single material must apply for each batch. In fact this class 
			allows you to use multiple materials, but you should be aware that 
			internally this means that there is one batch per material. 
			Therefore you won't gain as much benefit from the batching if you 
			use many different materials; try to keep the number down.
	@par
		The bounding box information is computed whith object position only. 
		It doesn't take account of the object orientation. 
	@par
		The LOD settings of both the Mesh and the Materials used in 
		constructing this instanced geometry will be respected. This means that 
		if you use meshes/materials which have LOD, batches in the distance 
		will have a lower polygon count or material detail to those in the 
		foreground. Since each mesh might have different LOD distances, during 
		build the furthest distance at each LOD level from all meshes  
		in that BatchInstance is used. This means all the LOD levels change at the 
		same time, but at the furthest distance of any of them (so quality is 
		not degraded). Be aware that using Mesh LOD in this class will 
		further increase the memory required. Only generated LOD
		is supported for meshes.
	@par
		There are 2 ways you can add geometry to this class; you can add
		Entity objects directly with predetermined positions, scales and 
		orientations, or you can add an entire SceneNode and it's subtree, 
		including all the objects attached to it. Once you've added everthing
		you need to, you have to call build() the fix the geometry in place. 
	@par
		You should not construct instances of this class directly; instead, call 
		SceneManager::createInstancedGeometry, which gives the SceneManager the 
		option of providing you with a specialised version of this class if it
		wishes, and also handles the memory management for you like other 
		classes.
    @note
		Warning: this class only works with indexed triangle lists at the moment,		do not pass it triangle strips, fans or lines / points, or unindexed geometry.
	*/
	class _OgreExport  InstancedGeometry : public BatchedGeometryAlloc
	{
	public:
		/** Struct holding geometry optimised per SubMesh / lod level, ready
			for copying to instances. 
		@remarks
			Since we're going to be duplicating geometry lots of times, it's
			far more important that we don't have redundant vertex data. If a 
			SubMesh uses shared geometry, or we're looking at a lower LOD, not
			all the vertices are being referenced by faces on that submesh.
			Therefore to duplicate them, potentially hundreds or even thousands
			of times, would be extremely wasteful. Therefore, if a SubMesh at
			a given LOD has wastage, we create an optimised version of it's
			geometry which is ready for copying with no wastage.
		*/
		class _OgrePrivate OptimisedSubMeshGeometry : public BatchedGeometryAlloc
		{
		public:
			OptimisedSubMeshGeometry() :vertexData(0), indexData(0) {}
			~OptimisedSubMeshGeometry() 
			{
				delete vertexData;
				delete indexData;
			}
			VertexData *vertexData;
			IndexData *indexData;
		};
		typedef std::list<OptimisedSubMeshGeometry*> OptimisedSubMeshGeometryList;
		/// Saved link between SubMesh at a LOD and vertex/index data
		/// May point to original or optimised geometry
		struct SubMeshLodGeometryLink
		{
			VertexData* vertexData;
			IndexData* indexData;
		};
		typedef std::vector<SubMeshLodGeometryLink> SubMeshLodGeometryLinkList;
		typedef std::map<SubMesh*, SubMeshLodGeometryLinkList*> SubMeshGeometryLookup;
		/// Structure recording a queued submesh for the build
		struct QueuedSubMesh : public BatchedGeometryAlloc
		{
			SubMesh* submesh;
			/// Link to LOD list of geometry, potentially optimised
			SubMeshLodGeometryLinkList* geometryLodList;
			String materialName;
			Vector3 position;
			Quaternion orientation;
			Vector3 scale;
			/// Pre-transformed world AABB 
			AxisAlignedBox worldBounds;
			unsigned int ID;
		};
		typedef std::vector<QueuedSubMesh*> QueuedSubMeshList;
		typedef std::vector<String> QueuedSubMeshOriginList;
		/// Structure recording a queued geometry for low level builds
		struct QueuedGeometry : public BatchedGeometryAlloc
		{
			SubMeshLodGeometryLink* geometry;
			Vector3 position;
			Quaternion orientation;
			Vector3 scale;
			unsigned int ID;
		};
		typedef std::vector<QueuedGeometry*> QueuedGeometryList;
		
		// forward declarations
		class LODBucket;
		class MaterialBucket;
		class BatchInstance;
		class InstancedObject;

		/** A GeometryBucket is a the lowest level bucket where geometry with 
			the same vertex & index format is stored. It also acts as the 
			renderable.
		*/
		class _OgreExport  GeometryBucket :	public SimpleRenderable
		{
		protected:
			
			/// Geometry which has been queued up pre-build (not for deallocation)
			QueuedGeometryList mQueuedGeometry;
			/// Pointer to the Batch
			InstancedGeometry*mBatch;
			/// Pointer to parent bucket
			MaterialBucket* mParent;
			/// String identifying the vertex / index format
			String mFormatString;
			/// Vertex information, includes current number of vertices
			/// committed to be a part of this bucket
			VertexData* mVertexData;
			/// Index information, includes index type which limits the max
			/// number of vertices which are allowed in one bucket
			IndexData* mIndexData;
			/// Size of indexes
			HardwareIndexBuffer::IndexType mIndexType;
			/// Maximum vertex indexable
			size_t mMaxVertexIndex;
			///	Index of the Texcoord where the index is stored
			unsigned short mTexCoordIndex;
			AxisAlignedBox mAABB;

			template<typename T>
			void copyIndexes(const T* src, T* dst, size_t count, size_t indexOffset)
			{
				if (indexOffset == 0)
				{
					memcpy(dst, src, sizeof(T) * count);
				}
				else
				{
					while(count--)
					{
						*dst++ = static_cast<T>(*src++ + indexOffset);
					}
				}
			}
		public:
			GeometryBucket(MaterialBucket* parent, const String& formatString, 
				const VertexData* vData, const IndexData* iData);
			GeometryBucket(MaterialBucket* parent,const String& formatString,GeometryBucket*bucket);
			virtual ~GeometryBucket();
			MaterialBucket* getParent(void) { return mParent; }
			Real getBoundingRadius(void) const;
			/// Get the vertex data for this geometry 
			const VertexData* getVertexData(void) const { return mVertexData; }
			/// Get the index data for this geometry 
			const IndexData* getIndexData(void) const { return mIndexData; }
			/// @copydoc Renderable::getMaterial
			const MaterialPtr& getMaterial(void) const;
			Technique* getTechnique(void) const;
	        void getWorldTransforms(Matrix4* xform) const;
			virtual unsigned short getNumWorldTransforms(void) const ;
			Real getSquaredViewDepth(const Camera* cam) const;
	        const LightList& getLights(void) const;
			bool getCastsShadows(void) const;
			String getFormatString(void) const;
			/** Try to assign geometry to this bucket.
			@returns false if there is no room left in this bucket
			*/
			bool assign(QueuedGeometry* qsm);
			/// Build
			void build();
			/// Dump contents for diagnostics
			void dump(std::ofstream& of) const;
			/// retun the BoundingBox information. Usefull when cloning the batch instance.
			AxisAlignedBox & getAABB(void){return mAABB;};
			/// @copydoc MovableObject::visitRenderables
			void visitRenderables(Renderable::Visitor* visitor, bool debugRenderables);

		};
		class _OgreExport  InstancedObject : public BatchedGeometryAlloc
		{
			friend class GeometryBucket;
		public:
			 enum TransformSpace
        {
            /// Transform is relative to the local space
            TS_LOCAL,
            /// Transform is relative to the space of the parent node
            TS_PARENT,
            /// Transform is relative to world space
            TS_WORLD
        };
			/// list of Geometry Buckets that contains the instanced object
			typedef std::vector<GeometryBucket*> GeometryBucketList;
		protected:
			GeometryBucketList mGeometryBucketList;
			unsigned short mIndex;
			Matrix4  mTransformation;
			Quaternion mOrientation;
			Vector3	mScale;
			Vector3 mPosition;
			SkeletonInstance* mSkeletonInstance;
			/// Cached bone matrices, including any world transform
			Matrix4 *mBoneWorldMatrices;
			/// Cached bone matrices in skeleton local space
			Matrix4 *mBoneMatrices;
			/// State of animation for animable meshes
			AnimationStateSet* mAnimationState;
			unsigned short mNumBoneMatrices;
			/// Records the last frame in which animation was updated
			unsigned long mFrameAnimationLastUpdated;
		public:
			InstancedObject(int index);
			InstancedObject(int index,SkeletonInstance *skeleton,AnimationStateSet*animations);
			~InstancedObject();
			void setPosition( Vector3  position);
			Vector3 & getPosition(void);
			void yaw(const Radian& angle);
			void pitch(const Radian& angle);
			void roll(const Radian& angle);
			void rotate(const Quaternion& q);
			void setScale(const Vector3& scale);
	        void setOrientation(const Quaternion& q);
	        void setPositionAndOrientation(Vector3 p, const Quaternion& q);
            Quaternion & getOrientation(void);
			void addBucketToList(GeometryBucket* bucket);
			void needUpdate();
			GeometryBucketList&getGeometryBucketList(void){return mGeometryBucketList;}
			void translate(const Matrix3& axes, const Vector3& move);
			void translate(const Vector3& d);
			Matrix3 getLocalAxes(void) const;
			void updateAnimation(void);
			AnimationState* getAnimationState(const String& name) const;
			SkeletonInstance*getSkeletonInstance(void){return mSkeletonInstance;}

		};
		/** A MaterialBucket is a collection of smaller buckets with the same 
			Material (and implicitly the same LOD). */
		class _OgreExport  MaterialBucket : public BatchedGeometryAlloc
		{
		public:
			/// list of Geometry Buckets in this BatchInstance
			typedef std::vector<GeometryBucket*> GeometryBucketList;
		protected:
			/// Pointer to parent LODBucket
			LODBucket* mParent;
			/// Material being used
			String mMaterialName;
			/// Pointer to material being used
			MaterialPtr mMaterial;
			/// Active technique
			Technique* mTechnique;
			int mLastIndex;
			/// list of Geometry Buckets in this BatchInstance
			GeometryBucketList mGeometryBucketList;
			// index to current Geometry Buckets for a given geometry format
			typedef std::map<String, GeometryBucket*> CurrentGeometryMap;
			CurrentGeometryMap mCurrentGeometryMap;
			/// Get a packed string identifying the geometry format
			String getGeometryFormatString(SubMeshLodGeometryLink* geom);
			
		public:
			MaterialBucket(LODBucket* parent, const String& materialName);
			virtual ~MaterialBucket();
			LODBucket* getParent(void) { return mParent; }
			/// Get the material name
			const String& getMaterialName(void) const { return mMaterialName; }
			/// Assign geometry to this bucket
			void assign(QueuedGeometry* qsm);
			/// Build
			void build();
			/// Add children to the render queue
			void addRenderables(RenderQueue* queue, uint8 group, 
				Real lodValue);
			/// Get the material for this bucket
			const MaterialPtr& getMaterial(void) const { return mMaterial; }
			/// Iterator over geometry
			typedef VectorIterator<GeometryBucketList> GeometryIterator;
			/// Get an iterator over the contained geometry
			GeometryIterator getGeometryIterator(void);
			/// Get the current Technique
			Technique* getCurrentTechnique(void) const { return mTechnique; }
			/// Dump contents for diagnostics
			void dump(std::ofstream& of) const;
			/// Return the geometry map
			MaterialBucket::CurrentGeometryMap* getMaterialBucketMap(void) const;
			/// Return the geometry list
			MaterialBucket::GeometryBucketList*getGeometryBucketList(void) const;
			/// fill in the map and the list
			void updateContainers(GeometryBucket* bucket, const String &format);
			void setLastIndex(int index){mLastIndex=index;}
			int getLastIndex(){return mLastIndex;}
			void setMaterial(const String & name);
			void visitRenderables(Renderable::Visitor* visitor, bool debugRenderables);
		
		};
		/** A LODBucket is a collection of smaller buckets with the same LOD. 
		@remarks
			LOD refers to Mesh LOD here. Material LOD can change separately
			at the next bucket down from this.
		*/
		class _OgreExport  LODBucket : public BatchedGeometryAlloc
		{
		public:
			/// Lookup of Material Buckets in this BatchInstance
			typedef std::map<String, MaterialBucket*> MaterialBucketMap;
		protected:
			/// Pointer to parent BatchInstance
			BatchInstance* mParent;
			/// LOD level (0 == full LOD)
			unsigned short mLod;
			/// lod value at which this LOD starts to apply (squared)
			Real mLodValue;
			/// Lookup of Material Buckets in this BatchInstance
			MaterialBucketMap mMaterialBucketMap;
			/// Geometry queued for a single LOD (deallocated here)
			QueuedGeometryList mQueuedGeometryList;
		public:
			LODBucket(BatchInstance* parent, unsigned short lod, Real lodValue);
			virtual ~LODBucket();
			BatchInstance* getParent(void) { return mParent; }
			/// Get the lod index
			ushort getLod(void) const { return mLod; }
			/// Get the lod value
			Real getLodValue(void) const { return mLodValue; }
			/// Assign a queued submesh to this bucket, using specified mesh LOD
			void assign(QueuedSubMesh* qsm, ushort atLod);
			/// Build
			void build();
			/// Add children to the render queue
			void addRenderables(RenderQueue* queue, uint8 group, 
				Real lodValue);
			/// Iterator over the materials in this LOD
			typedef MapIterator<MaterialBucketMap> MaterialIterator;
			/// Get an iterator over the materials in this LOD
			MaterialIterator getMaterialIterator(void);
			/// Dump contents for diagnostics
			void dump(std::ofstream& of) const;
			/// fill the map
			void updateContainers(MaterialBucket* bucket, String& name );
			void visitRenderables(Renderable::Visitor* visitor, bool debugRenderables);
			
		};
		/** The details of a topological BatchInstance which is the highest level of
			partitioning for this class.
		@remarks
			The size & shape of BatchInstances entirely depends on the SceneManager
			specific implementation. It is a MovableObject since it will be
			attached to a node based on the local centre - in practice it
			won't actually move (although in theory it could).
		*/
		class _OgreExport  BatchInstance : public MovableObject
		{
            friend class MaterialBucket;
			public:
		

			/// list of LOD Buckets in this BatchInstance
			typedef std::vector<LODBucket*> LODBucketList;
			typedef std::map<int, InstancedObject*> ObjectsMap;
			typedef MapIterator<ObjectsMap> InstancedObjectIterator;
		protected:
			
			/// Parent static geometry
			InstancedGeometry* mParent;
			/// Scene manager link
			SceneManager* mSceneMgr;
			/// Scene node
			SceneNode* mNode;
			/// Local list of queued meshes (not used for deallocation)
			QueuedSubMeshList mQueuedSubMeshes;
			/// Unique identifier for the BatchInstance
			uint32 mBatchInstanceID;

			ObjectsMap mInstancesMap;
		public:
			/// Lod values as built up - use the max at each level
			Mesh::LodValueList mLodValues;
			/// Local AABB relative to BatchInstance centre
			AxisAlignedBox mAABB;
			/// Local bounding radius
			Real mBoundingRadius;
			/// The current lod level, as determined from the last camera
			ushort mCurrentLod;
			/// Current lod value, passed on to do material lod later
			Real mLodValue;
            /// Current camera, passed on to do material lod later
            Camera *mCamera;
            /// Cached squared view depth value to avoid recalculation by GeometryBucket
            Real mSquaredViewDepth;
		protected:
			/// List of LOD buckets			
			LODBucketList mLodBucketList;
            /// Lod strategy reference
            const LodStrategy *mLodStrategy;

		public:
			BatchInstance(InstancedGeometry* parent, const String& name, SceneManager* mgr, 
				uint32 BatchInstanceID);
			virtual ~BatchInstance();
			// more fields can be added in subclasses
			InstancedGeometry* getParent(void) const { return mParent;}
			/// Assign a queued mesh to this BatchInstance, read for final build
			void assign(QueuedSubMesh* qmesh);
			/// Build this BatchInstance
			void build();
			/// Get the BatchInstance ID of this BatchInstance
			uint32 getID(void) const { return mBatchInstanceID; }
			/// Get the centre point of the BatchInstance
//			const Vector3& getCentre(void) const { return mCentre; }
			const String& getMovableType(void) const;
			void _notifyCurrentCamera(Camera* cam);
			const AxisAlignedBox& getBoundingBox(void) const;
			void  setBoundingBox(AxisAlignedBox& box);
			Real getBoundingRadius(void) const;
			void _updateRenderQueue(RenderQueue* queue);
			bool isVisible(void) const;
			/// @copydoc MovableObject::visitRenderables
			void visitRenderables(Renderable::Visitor* visitor, 
				bool debugRenderables = false);

		//	uint32 getTypeFlags(void) const;

			typedef VectorIterator<LODBucketList> LODIterator;
			/// Get an iterator over the LODs in this BatchInstance
			LODIterator getLODIterator(void);
			/// Shared set of lights for all GeometryBuckets
			const LightList& getLights(void) const;

			/// update the bounding box of the BatchInstance according to the positions of the objects
			void updateBoundingBox();

			/// Dump contents for diagnostics
			void dump(std::ofstream& of) const;
			/// fill in the list 
			void updateContainers(LODBucket* bucket );
			/// attach the BatchInstance to the scene
			void attachToScene();
			void addInstancedObject(int index, InstancedObject* object);
			InstancedObject*  isInstancedObjectPresent(int index);
			InstancedObjectIterator getObjectIterator();
			SceneNode*getSceneNode(void){return mNode;}
			ObjectsMap& getInstancesMap(void){return  mInstancesMap;}
			/// change the shader used to render the batch instance
			
		};
		/** Indexed BatchInstance map based on packed x/y/z BatchInstance index, 10 bits for
			each axis.
		*/
		typedef std::map<uint32, BatchInstance*> BatchInstanceMap;
		/** Simple vectors where are stored all the renderoperations of the Batch.
			This vector is used when we want to delete the batch, in order to delete only one time each
			render operation.

		*/
		typedef std::vector<RenderOperation*> RenderOperationVector;
	protected:
		// General state & settings
		SceneManager* mOwner;
		String mName;
		bool mBuilt;
		Real mUpperDistance;
		Real mSquaredUpperDistance;
		bool mCastShadows;
		Vector3 mBatchInstanceDimensions;
		Vector3 mHalfBatchInstanceDimensions;
		Vector3 mOrigin;
		bool mVisible;
        /// The render queue to use when rendering this object
        uint8 mRenderQueueID;
		/// Flags whether the RenderQueue's default should be used.
		bool mRenderQueueIDSet;
		/// number of objects in the batch
		unsigned int mObjectCount;
		QueuedSubMeshList mQueuedSubMeshes;
		BatchInstance*mInstancedGeometryInstance;
		/**this is just a pointer to the base skeleton that will be used for each animated object in the batches
		This pointer has a value only during the creation of the InstancedGeometry
		*/
		SkeletonPtr mBaseSkeleton;
		SkeletonInstance *mSkeletonInstance;
		/**This is the main animation state. All "objects" in the batch will use an instance of this animation
		state
		*/
		AnimationStateSet* mAnimationState;
		/// List of geometry which has been optimised for SubMesh use
		/// This is the primary storage used for cleaning up later
		OptimisedSubMeshGeometryList mOptimisedSubMeshGeometryList;

		/** Cached links from SubMeshes to (potentially optimised) geometry
			This is not used for deletion since the lookup may reference
			original vertex data
		*/
		SubMeshGeometryLookup mSubMeshGeometryLookup;
			
		/// Map of BatchInstances
		BatchInstanceMap mBatchInstanceMap;
		/** This vector stores all the renderOperation used in the batch. 
		See the type definition for more details.
		*/
		RenderOperationVector mRenderOps;
		/** Virtual method for getting a BatchInstance most suitable for the
			passed in bounds. Can be overridden by subclasses.
		*/
		virtual BatchInstance* getBatchInstance(const AxisAlignedBox& bounds, bool autoCreate);
		/** Get the BatchInstance within which a point lies */
		virtual BatchInstance* getBatchInstance(const Vector3& point, bool autoCreate);
		/** Get the BatchInstance using indexes */
		virtual BatchInstance* getBatchInstance(ushort x, ushort y, ushort z, bool autoCreate);
		/** Get the BatchInstance using a packed index, returns null if it doesn't exist. */
		virtual BatchInstance* getBatchInstance(uint32 index);
		/** Get the BatchInstance indexes for a point.
		*/
		virtual void getBatchInstanceIndexes(const Vector3& point, 
			ushort& x, ushort& y, ushort& z);
		/** get the first BatchInstance or create on if it does not exists.
		*/
		virtual BatchInstance* getInstancedGeometryInstance(void);
		/** Pack 3 indexes into a single index value
		*/
		virtual uint32 packIndex(ushort x, ushort y, ushort z);
		/** Get the volume intersection for an indexed BatchInstance with some bounds.
		*/
		virtual Real getVolumeIntersection(const AxisAlignedBox& box,  
			ushort x, ushort y, ushort z);
		/** Get the bounds of an indexed BatchInstance.
		*/
		virtual AxisAlignedBox getBatchInstanceBounds(ushort x, ushort y, ushort z);
		/** Get the centre of an indexed BatchInstance.
		*/
		virtual Vector3 getBatchInstanceCentre(ushort x, ushort y, ushort z);
		/** Calculate world bounds from a set of vertex data. */
		virtual AxisAlignedBox calculateBounds(VertexData* vertexData, 
			const Vector3& position, const Quaternion& orientation, 
			const Vector3& scale);
		/** Look up or calculate the geometry data to use for this SubMesh */
		SubMeshLodGeometryLinkList* determineGeometry(SubMesh* sm);
		/** Split some shared geometry into dedicated geometry. */
		void splitGeometry(VertexData* vd, IndexData* id, 
			SubMeshLodGeometryLink* targetGeomLink);

		typedef std::map<size_t, size_t> IndexRemap;
		/** Method for figuring out which vertices are used by an index buffer
			and calculating a remap lookup for a vertex buffer just containing
			those vertices. 
		*/
		template <typename T>
		void buildIndexRemap(T* pBuffer, size_t numIndexes, IndexRemap& remap)
		{
			remap.clear();
			for (size_t i = 0; i < numIndexes; ++i)
			{
				// use insert since duplicates are silently discarded
				remap.insert(IndexRemap::value_type(*pBuffer++, remap.size()));
				// this will have mapped oldindex -> new index IF oldindex
				// wasn't already there
			}
		}
		/** Method for altering indexes based on a remap. */
		template <typename T>
		void remapIndexes(T* src, T* dst, const IndexRemap& remap, 
				size_t numIndexes)
		{
			for (size_t i = 0; i < numIndexes; ++i)
			{
				// look up original and map to target
				IndexRemap::const_iterator ix = remap.find(*src++);
				assert(ix != remap.end());
				*dst++ = static_cast<T>(ix->second);
			}
		}
		
	public:
		/// Constructor; do not use directly (@see SceneManager::createInstancedGeometry)
		InstancedGeometry(SceneManager* owner, const String& name);
		/// Destructor
		virtual ~InstancedGeometry();

		/// Get the name of this object
		const String& getName(void) const { return mName; }
		/** Adds an Entity to the static geometry.
		@remarks
			This method takes an existing Entity and adds its details to the 
			list of	elements to include when building. Note that the Entity
			itself is not copied or referenced in this method; an Entity is 
			passed simply so that you can change the materials of attached 
			SubEntity objects if you want. You can add the same Entity 
			instance multiple times with different material settings 
			completely safely, and destroy the Entity before destroying 
			this InstancedGeometry if you like. The Entity passed in is simply 
			used as a definition.
		@note Must be called before 'build'.
        @note All added entities must use the same lod strategy.
		@param ent The Entity to use as a definition (the Mesh and Materials 
			referenced will be recorded for the build call).
		@param position The world position at which to add this Entity
		@param orientation The world orientation at which to add this Entity
		@param scale The scale at which to add this entity
		*/
		virtual void addEntity(Entity* ent, const Vector3& position,
			const Quaternion& orientation = Quaternion::IDENTITY, 
			const Vector3& scale = Vector3::UNIT_SCALE);

		/** Adds all the Entity objects attached to a SceneNode and all it's
			children to the static geometry.
		@remarks
			This method performs just like addEntity, except it adds all the 
			entities attached to an entire sub-tree to the geometry. 
			The position / orientation / scale parameters are taken from the
			node structure instead of being specified manually. 
		@note
			The SceneNode you pass in will not be automatically detached from 
			it's parent, so if you have this node already attached to the scene
			graph, you will need to remove it if you wish to avoid the overhead
			of rendering <i>both</i> the original objects and their new static
			versions! We don't do this for you incase you are preparing this 
			in advance and so don't want the originals detached yet. 
		@note Must be called before 'build'.
        @note All added entities must use the same lod strategy.
		@param node Pointer to the node to use to provide a set of Entity 
			templates
		*/
		virtual void addSceneNode(const SceneNode* node);

		/** Build the geometry. 
		@remarks
			Based on all the entities which have been added, and the batching 
			options which have been set, this method constructs	the batched 
			geometry structures required. The batches are added to the scene 
			and will be rendered unless you specifically hide them.
		@note
			Once you have called this method, you can no longer add any more 
			entities.
		*/
		virtual void build(void);
			/** Add a new batch instance
		@remarks
				This method add a new instance of the whole batch, by creating a new 
				BatchInstance, containing new lod buckets, material buckets and geometry buckets.
				The new geometry buckets will use the same buffers as the base bucket.
		@note
			no note
		*/
		void addBatchInstance(void);
		/** Destroys all the built geometry state (reverse of build). 
		@remarks
			You can call build() again after this and it will pick up all the
			same entities / nodes you queued last time.
		*/
		virtual void destroy(void);

		/** Clears any of the entities / nodes added to this geometry and 
			destroys anything which has already been built.
		*/
		virtual void reset(void);

		/** Sets the distance at which batches are no longer rendered.
		@remarks
			This lets you turn off batches at a given distance. This can be 
			useful for things like detail meshes (grass, foliage etc) and could
			be combined with a shader which fades the geometry out beforehand 
			to lessen the effect.
		@param dist Distance beyond which the batches will not be rendered 
			(the default is 0, which means batches are always rendered).
		*/
		virtual void setRenderingDistance(Real dist) { 
			mUpperDistance = dist; 
			mSquaredUpperDistance = mUpperDistance * mUpperDistance;
		}

		/** Gets the distance at which batches are no longer rendered. */
		virtual Real getRenderingDistance(void) const { return mUpperDistance; }

		/** Gets the squared distance at which batches are no longer rendered. */
		virtual Real getSquaredRenderingDistance(void) const 
		{ return mSquaredUpperDistance; }

		/** Hides or shows all the batches. */
		virtual void setVisible(bool visible);

		/** Are the batches visible? */
		virtual bool isVisible(void) const { return mVisible; }

		/** Sets whether this geometry should cast shadows.
		@remarks
			No matter what the settings on the original entities,
			the InstancedGeometry class defaults to not casting shadows. 
			This is because, being static, unless you have moving lights
			you'd be better to use precalculated shadows of some sort.
			However, if you need them, you can enable them using this
			method. If the SceneManager is set up to use stencil shadows,
			edge lists will be copied from the underlying meshes on build.
			It is essential that all meshes support stencil shadows in this
			case.
		@note If you intend to use stencil shadows, you must set this to 
			true before calling 'build' as well as making sure you set the
			scene's shadow type (that should always be the first thing you do
			anyway). You can turn shadows off temporarily but they can never 
			be turned on if they were not at the time of the build. 
		*/
		virtual void setCastShadows(bool castShadows);
		/// Will the geometry from this object cast shadows?
		virtual bool getCastShadows(void) { return mCastShadows; }

		/** Sets the size of a single BatchInstance of geometry.
		@remarks
			This method allows you to configure the physical world size of 
			each BatchInstance, so you can balance culling against batch size. Entities
			will be fitted within the batch they most closely fit, and the 
			eventual bounds of each batch may well be slightly larger than this
			if they overlap a little. The default is Vector3(1000, 1000, 1000).
		@note Must be called before 'build'.
		@param size Vector3 expressing the 3D size of each BatchInstance.
		*/
		virtual void setBatchInstanceDimensions(const Vector3& size) { 
			mBatchInstanceDimensions = size; 
			mHalfBatchInstanceDimensions = size * 0.5;
		}
		/** Gets the size of a single batch of geometry. */
		virtual const Vector3& getBatchInstanceDimensions(void) const { return mBatchInstanceDimensions; }
		/** Sets the origin of the geometry.
		@remarks
			This method allows you to configure the world centre of the geometry,
			thus the place which all BatchInstances surround. You probably don't need 
			to mess with this unless you have a seriously large world, since the
			default set up can handle an area 1024 * mBatchInstanceDimensions, and 
			the sparseness of population is no issue when it comes to rendering.
			The default is Vector3(0,0,0).
		@note Must be called before 'build'.
		@param size Vector3 expressing the 3D origin of the geometry.
		*/
		virtual void setOrigin(const Vector3& origin) { mOrigin = origin; }
		/** Gets the origin of this geometry. */
		virtual const Vector3& getOrigin(void) const { return mOrigin; }

        /** Sets the render queue group this object will be rendered through.
        @remarks
            Render queues are grouped to allow you to more tightly control the ordering
            of rendered objects. If you do not call this method, all  objects default
            to the default queue (RenderQueue::getDefaultQueueGroup), which is fine for 
			most objects. You may want to alter this if you want to perform more complex
			rendering.
        @par
            See RenderQueue for more details.
        @param queueID Enumerated value of the queue group to use.
        */
        virtual void setRenderQueueGroup(uint8 queueID);

        /** Gets the queue group for this entity, see setRenderQueueGroup for full details. */
        virtual uint8 getRenderQueueGroup(void) const;
		/// Iterator for iterating over contained BatchInstances
		typedef MapIterator<BatchInstanceMap> BatchInstanceIterator;
		/// Get an iterator over the BatchInstances in this geometry
		BatchInstanceIterator getBatchInstanceIterator(void);
		/// get the mRenderOps vector.
		RenderOperationVector& getRenderOperationVector(){return mRenderOps;}
		/// @copydoc MovableObject::visitRenderables
		void visitRenderables(Renderable::Visitor* visitor, 
			bool debugRenderables = false);

		/** Dump the contents of this InstancedGeometry to a file for diagnostic
		 	purposes.
		*/
		virtual void dump(const String& filename) const;
		/**
		@remarks
		Return the skeletonInstance that will be used 
		*/
		SkeletonInstance *getBaseSkeletonInstance(void){return mSkeletonInstance;}
		/**
		@remarks
		Return the skeleton that is shared by all instanced objects.
		*/
		SkeletonPtr getBaseSkeleton(void){return mBaseSkeleton;}
		/**
		@remarks
		Return the animation state that will be cloned each time an InstancedObject is made
		*/
		AnimationStateSet* getBaseAnimationState(void){return mAnimationState;}
		/**
		@remarks
		return the total number of object that are in all the batches
		*/
		unsigned int getObjectCount(void){return mObjectCount;}
		                  


	};

}

#endif

