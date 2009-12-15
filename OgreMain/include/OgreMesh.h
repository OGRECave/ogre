/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#ifndef __Mesh_H__
#define __Mesh_H__

#include "OgrePrerequisites.h"

#include "OgreResource.h"
#include "OgreVertexIndexData.h"
#include "OgreAxisAlignedBox.h"
#include "OgreVertexBoneAssignment.h"
#include "OgreIteratorWrappers.h"
#include "OgreProgressiveMesh.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreSkeleton.h"
#include "OgreAnimationTrack.h"
#include "OgrePose.h"
#include "OgreDataStream.h"


namespace Ogre {


	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Resources
	*  @{
	*/
	/** Resource holding data about 3D mesh.
        @remarks
            This class holds the data used to represent a discrete
            3-dimensional object. Mesh data usually contains more
            than just vertices and triangle information; it also
            includes references to materials (and the faces which use them),
            level-of-detail reduction information, convex hull definition,
            skeleton/bones information, keyframe animation etc.
            However, it is important to note the emphasis on the word
            'discrete' here. This class does not cover the large-scale
            sprawling geometry found in level / landscape data.
        @par
            Multiple world objects can (indeed should) be created from a
            single mesh object - see the Entity class for more info.
            The mesh object will have it's own default
            material properties, but potentially each world instance may
            wish to customise the materials from the original. When the object
            is instantiated into a scene node, the mesh material properties
            will be taken by default but may be changed. These properties
            are actually held at the SubMesh level since a single mesh may
            have parts with different materials.
        @par
            As described above, because the mesh may have sections of differing
            material properties, a mesh is inherently a compound construct,
            consisting of one or more SubMesh objects.
            However, it strongly 'owns' it's SubMeshes such that they
            are loaded / unloaded at the same time. This is contrary to
            the approach taken to hierarchically related (but loosely owned)
            scene nodes, where data is loaded / unloaded separately. Note
            also that mesh sub-sections (when used in an instantiated object)
            share the same scene node as the parent.
    */

    struct MeshLodUsage;
    class LodStrategy;

    class _OgreExport Mesh: public Resource
    {
        friend class SubMesh;
        friend class MeshSerializerImpl;
        friend class MeshSerializerImpl_v1_4;
        friend class MeshSerializerImpl_v1_2;
        friend class MeshSerializerImpl_v1_1;

    public:
		typedef vector<Real>::type LodValueList;
        typedef vector<MeshLodUsage>::type MeshLodUsageList;
        /// Multimap of vertex bone assignments (orders by vertex index)
		typedef multimap<size_t, VertexBoneAssignment>::type VertexBoneAssignmentList;
        typedef MapIterator<VertexBoneAssignmentList> BoneAssignmentIterator;
        typedef vector<SubMesh*>::type SubMeshList;
        typedef vector<unsigned short>::type IndexMap;

    protected:
        /** A list of submeshes which make up this mesh.
            Each mesh is made up of 1 or more submeshes, which
            are each based on a single material and can have their
            own vertex data (they may not - they can share vertex data
            from the Mesh, depending on preference).
        */
        SubMeshList mSubMeshList;
	
        /** Internal method for making the space for a vertex element to hold tangents. */
        void organiseTangentsBuffer(VertexData *vertexData, 
			VertexElementSemantic targetSemantic, unsigned short index, 
			unsigned short sourceTexCoordSet);

    public:
		/** A hashmap used to store optional SubMesh names.
			Translates a name into SubMesh index
		*/
		typedef HashMap<String, ushort> SubMeshNameMap ;

		
    protected:

        DataStreamPtr mFreshFromDisk;

		SubMeshNameMap mSubMeshNameMap ;

        /// Local bounding box volume
        AxisAlignedBox mAABB;
		/// Local bounding sphere radius (centered on object)
		Real mBoundRadius;

        /// Optional linked skeleton
        String mSkeletonName;
        SkeletonPtr mSkeleton;

       
        VertexBoneAssignmentList mBoneAssignments;

        /// Flag indicating that bone assignments need to be recompiled
        bool mBoneAssignmentsOutOfDate;

        /** Build the index map between bone index and blend index */
        void buildIndexMap(const VertexBoneAssignmentList& boneAssignments,
            IndexMap& boneIndexToBlendIndexMap, IndexMap& blendIndexToBoneIndexMap);
        /** Compile bone assignments into blend index and weight buffers. */
        void compileBoneAssignments(const VertexBoneAssignmentList& boneAssignments,
            unsigned short numBlendWeightsPerVertex, 
            IndexMap& blendIndexToBoneIndexMap,
            VertexData* targetVertexData);

        const LodStrategy *mLodStrategy;
		bool mIsLodManual;
		ushort mNumLods;
		MeshLodUsageList mMeshLodUsageList;

		HardwareBuffer::Usage mVertexBufferUsage;
		HardwareBuffer::Usage mIndexBufferUsage;
		bool mVertexBufferShadowBuffer;
		bool mIndexBufferShadowBuffer;


        bool mPreparedForShadowVolumes;
        bool mEdgeListsBuilt;
        bool mAutoBuildEdgeLists;

		/// Storage of morph animations, lookup by name
		typedef map<String, Animation*>::type AnimationList;
		AnimationList mAnimationsList;
		/// The vertex animation type associated with the shared vertex data
		mutable VertexAnimationType mSharedVertexDataAnimationType;
		/// Do we need to scan animations for animation types?
		mutable bool mAnimationTypesDirty;

		/// List of available poses for shared and dedicated geometryPoseList
		PoseList mPoseList;


        /** Loads the mesh from disk.  This call only performs IO, it
            does not parse the bytestream or check for any errors therein.
            It also does not set up submeshes, etc.  You have to call load()
            to do that.
         */
        void prepareImpl(void);
        /** Destroys data cached by prepareImpl.
         */
        void unprepareImpl(void);
        /// @copydoc Resource::loadImpl
        void loadImpl(void);
		/// @copydoc Resource::postLoadImpl
		void postLoadImpl(void);
        /// @copydoc Resource::unloadImpl
        void unloadImpl(void);
		/// @copydoc Resource::calculateSize
		size_t calculateSize(void) const;



    public:
        /** Default constructor - used by MeshManager
            @warning
                Do not call this method directly.
        */
        Mesh(ResourceManager* creator, const String& name, ResourceHandle handle,
            const String& group, bool isManual = false, ManualResourceLoader* loader = 0);
        ~Mesh();

		// NB All methods below are non-virtual since they will be
        // called in the rendering loop - speed is of the essence.

        /** Creates a new SubMesh.
            @remarks
                Method for manually creating geometry for the mesh.
                Note - use with extreme caution - you must be sure that
                you have set up the geometry properly.
        */
        SubMesh* createSubMesh(void);

		/** Creates a new SubMesh and gives it a name
		*/
		SubMesh* createSubMesh(const String& name);
		
		/** Gives a name to a SubMesh
		*/
		void nameSubMesh(const String& name, ushort index);

		/** Removes a name from a SubMesh
		*/
		void unnameSubMesh(const String& name);
		
		/** Gets the index of a submesh with a given name.
        @remarks
            Useful if you identify the SubMeshes by name (using nameSubMesh)
            but wish to have faster repeat access.
        */
		ushort _getSubMeshIndex(const String& name) const;

        /** Gets the number of sub meshes which comprise this mesh.
        */
        unsigned short getNumSubMeshes(void) const;

        /** Gets a pointer to the submesh indicated by the index.
        */
        SubMesh* getSubMesh(unsigned short index) const;

		/** Gets a SubMesh by name
		*/
		SubMesh* getSubMesh(const String& name) const ;
		
		/** Destroy a SubMesh with the given index. 
		 @note This will invalidate the contents of any existing Entity, or
		 any other object that is referring to the SubMesh list. Entity will
		 detect this and reinitialise, but it is still a disruptive action.
		*/
		void destroySubMesh(unsigned short index);

		/** Destroy a SubMesh with the given name. 
		 @note This will invalidate the contents of any existing Entity, or
		 any other object that is referring to the SubMesh list. Entity will
		 detect this and reinitialise, but it is still a disruptive action.
		 */
		void destroySubMesh(const String& name);
		
        typedef VectorIterator<SubMeshList> SubMeshIterator;
        /// Gets an iterator over the available submeshes
        SubMeshIterator getSubMeshIterator(void)
        { return SubMeshIterator(mSubMeshList.begin(), mSubMeshList.end()); }
      
        /** Shared vertex data.
            @remarks
                This vertex data can be shared among multiple submeshes. SubMeshes may not have
                their own VertexData, they may share this one.
            @par
                The use of shared or non-shared buffers is determined when
                model data is converted to the OGRE .mesh format.
        */
        VertexData *sharedVertexData;

        /** Shared index map for translating blend index to bone index.
            @remarks
                This index map can be shared among multiple submeshes. SubMeshes might not have
                their own IndexMap, they might share this one.
            @par
                We collect actually used bones of all bone assignments, and build the
                blend index in 'packed' form, then the range of the blend index in vertex
                data VES_BLEND_INDICES element is continuous, with no gaps. Thus, by
                minimising the world matrix array constants passing to GPU, we can support
                more bones for a mesh when hardware skinning is used. The hardware skinning
                support limit is applied to each set of vertex data in the mesh, in other words, the
                hardware skinning support limit is applied only to the actually used bones of each
                SubMeshes, not all bones across the entire Mesh.
            @par
                Because the blend index is different to the bone index, therefore, we use
                the index map to translate the blend index to bone index.
            @par
                The use of shared or non-shared index map is determined when
                model data is converted to the OGRE .mesh format.
        */
        IndexMap sharedBlendIndexToBoneIndexMap;

        /** Makes a copy of this mesh object and gives it a new name.
            @remarks
                This is useful if you want to tweak an existing mesh without affecting the original one. The
                newly cloned mesh is registered with the MeshManager under the new name.
            @param newName The name to give the clone
            @param newGroup Optional name of the new group to assign the clone to;
                if you leave this blank, the clone will be assigned to the same
                group as this Mesh.
        */
        MeshPtr clone(const String& newName, const String& newGroup = StringUtil::BLANK);

        /** Get the axis-aligned bounding box for this mesh.
        */
        const AxisAlignedBox& getBounds(void) const;

		/** Gets the radius of the bounding sphere surrounding this mesh. */
		Real getBoundingSphereRadius(void) const;

        /** Manually set the bounding box for this Mesh.
            @remarks
            Calling this method is required when building manual meshes now, because OGRE can no longer 
            update the bounds for you, because it cannot necessarily read vertex data back from 
            the vertex buffers which this mesh uses (they very well might be write-only, and even
            if they are not, reading data from a hardware buffer is a bottleneck).
            @param pad If true, a certain padding will be added to the bounding box to separate it from the mesh
        */
        void _setBounds(const AxisAlignedBox& bounds, bool pad = true);

        /** Manually set the bounding radius. 
        @remarks
            Calling this method is required when building manual meshes now, because OGRE can no longer 
            update the bounds for you, because it cannot necessarily read vertex data back from 
            the vertex buffers which this mesh uses (they very well might be write-only, and even
            if they are not, reading data from a hardware buffer is a bottleneck).
        */
        void _setBoundingSphereRadius(Real radius);

        /** Sets the name of the skeleton this Mesh uses for animation.
        @remarks
            Meshes can optionally be assigned a skeleton which can be used to animate
            the mesh through bone assignments. The default is for the Mesh to use no
            skeleton. Calling this method with a valid skeleton filename will cause the
            skeleton to be loaded if it is not already (a single skeleton can be shared
            by many Mesh objects).
        @param skelName The name of the .skeleton file to use, or an empty string to use
            no skeleton
        */
        void setSkeletonName(const String& skelName);

        /** Returns true if this Mesh has a linked Skeleton. */
        bool hasSkeleton(void) const;

		/** Returns whether or not this mesh has some kind of vertex animation. 
		*/
		bool hasVertexAnimation(void) const;

		/** Gets a pointer to any linked Skeleton. 
        @returns Weak reference to the skeleton - copy this if you want to hold a strong pointer.
        */
        const SkeletonPtr& getSkeleton(void) const;

        /** Gets the name of any linked Skeleton */
        const String& getSkeletonName(void) const;
        /** Initialise an animation set suitable for use with this mesh. 
        @remarks
            Only recommended for use inside the engine, not by applications.
        */
        void _initAnimationState(AnimationStateSet* animSet);

		/** Refresh an animation set suitable for use with this mesh. 
		@remarks
		Only recommended for use inside the engine, not by applications.
		*/
		void _refreshAnimationState(AnimationStateSet* animSet);
        /** Assigns a vertex to a bone with a given weight, for skeletal animation. 
        @remarks    
            This method is only valid after calling setSkeletonName.
            Since this is a one-off process there exists only 'addBoneAssignment' and
            'clearBoneAssignments' methods, no 'editBoneAssignment'. You should not need
            to modify bone assignments during rendering (only the positions of bones) and OGRE
            reserves the right to do some internal data reformatting of this information, depending
            on render system requirements.
        @par
            This method is for assigning weights to the shared geometry of the Mesh. To assign
            weights to the per-SubMesh geometry, see the equivalent methods on SubMesh.
        */
        void addBoneAssignment(const VertexBoneAssignment& vertBoneAssign);

        /** Removes all bone assignments for this mesh. 
        @remarks
            This method is for modifying weights to the shared geometry of the Mesh. To assign
            weights to the per-SubMesh geometry, see the equivalent methods on SubMesh.
        */
        void clearBoneAssignments(void);

        /** Internal notification, used to tell the Mesh which Skeleton to use without loading it. 
        @remarks
            This is only here for unusual situation where you want to manually set up a
            Skeleton. Best to let OGRE deal with this, don't call it yourself unless you
            really know what you're doing.
        */
        void _notifySkeleton(SkeletonPtr& pSkel);


        /** Gets an iterator for access all bone assignments. 
        */
        BoneAssignmentIterator getBoneAssignmentIterator(void);

		/** Gets a const reference to the list of bone assignments
		*/
		const VertexBoneAssignmentList& getBoneAssignments() const { return mBoneAssignments; }


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
		void generateLodLevels(const LodValueList& lodValues, 
			ProgressiveMesh::VertexReductionQuota reductionMethod, Real reductionValue);

		/** Returns the number of levels of detail that this mesh supports. 
		@remarks
			This number includes the original model.
		*/
		ushort getNumLodLevels(void) const;
		/** Gets details of the numbered level of detail entry. */
		const MeshLodUsage& getLodLevel(ushort index) const;
		/** Adds a new manual level-of-detail entry to this Mesh.
		@remarks
			As an alternative to generating lower level of detail versions of a mesh, you can
			use your own manually modelled meshes as lower level versions. This lets you 
			have complete control over the LOD, and in addition lets you scale down other
			aspects of the model which cannot be done using the generated method; for example, 
			you could use less detailed materials and / or use less bones in the skeleton if
			this is an animated mesh. Therefore for complex models you are likely to be better off
			modelling your LODs yourself and using this method, whilst for models with fairly
			simple materials and no animation you can just use the generateLodLevels method.
		@param value The value from which this Lod will apply.
		@param meshName The name of the mesh which will be the lower level detail version.
		*/
		void createManualLodLevel(Real value, const String& meshName);

		/** Changes the alternate mesh to use as a manual LOD at the given index.
		@remarks
			Note that the index of a LOD may change if you insert other LODs. If in doubt,
			use getLodIndex().
		@param index The index of the level to be changed
		@param meshName The name of the mesh which will be the lower level detail version.
		*/
		void updateManualLodLevel(ushort index, const String& meshName);

		/** Retrieves the level of detail index for the given lod value. 
		@note The value passed in is the 'transformed' value. If you are dealing with
		an original source value (e.g. distance), use LodStrategy::transformUserValue
		to turn this into a lookup value.
		*/
		ushort getLodIndex(Real value) const;

		/** Returns true if this mesh is using manual LOD.
		@remarks
			A mesh can either use automatically generated LOD, or it can use alternative
			meshes as provided by an artist. A mesh can only use either all manual LODs 
			or all generated LODs, not a mixture of both.
		*/
		bool isLodManual(void) const { return mIsLodManual; }

		/** Internal methods for loading LOD, do not use. */
		void _setLodInfo(unsigned short numLevels, bool isManual);
		/** Internal methods for loading LOD, do not use. */
		void _setLodUsage(unsigned short level, MeshLodUsage& usage);
		/** Internal methods for loading LOD, do not use. */
		void _setSubMeshLodFaceList(unsigned short subIdx, unsigned short level, IndexData* facedata);

        /** Removes all LOD data from this Mesh. */
        void removeLodLevels(void);

		/** Sets the policy for the vertex buffers to be used when loading
			this Mesh.
		@remarks
			By default, when loading the Mesh, static, write-only vertex and index buffers 
			will be used where possible in order to improve rendering performance. 
			However, such buffers
			cannot be manipulated on the fly by CPU code (although shader code can). If you
			wish to use the CPU to modify these buffers, you should call this method. Note,
			however, that it only takes effect after the Mesh has been reloaded. Note that you
			still have the option of manually repacing the buffers in this mesh with your
			own if you see fit too, in which case you don't need to call this method since it
			only affects buffers created by the mesh itself.
		@par
			You can define the approach to a Mesh by changing the default parameters to 
			MeshManager::load if you wish; this means the Mesh is loaded with those options
			the first time instead of you having to reload the mesh after changing these options.
		@param usage The usage flags, which by default are 
			HardwareBuffer::HBU_STATIC_WRITE_ONLY
		@param shadowBuffer If set to true, the vertex buffers will be created with a
            system memory shadow buffer. You should set this if you want to be able to
			read from the buffer, because reading from a hardware buffer is a no-no.
		*/
		void setVertexBufferPolicy(HardwareBuffer::Usage usage, bool shadowBuffer = false);
		/** Sets the policy for the index buffers to be used when loading
			this Mesh.
		@remarks
			By default, when loading the Mesh, static, write-only vertex and index buffers 
			will be used where possible in order to improve rendering performance. 
			However, such buffers
			cannot be manipulated on the fly by CPU code (although shader code can). If you
			wish to use the CPU to modify these buffers, you should call this method. Note,
			however, that it only takes effect after the Mesh has been reloaded. Note that you
			still have the option of manually repacing the buffers in this mesh with your
			own if you see fit too, in which case you don't need to call this method since it
			only affects buffers created by the mesh itself.
		@par
			You can define the approach to a Mesh by changing the default parameters to 
			MeshManager::load if you wish; this means the Mesh is loaded with those options
			the first time instead of you having to reload the mesh after changing these options.
		@param usage The usage flags, which by default are 
			HardwareBuffer::HBU_STATIC_WRITE_ONLY
		@param shadowBuffer If set to true, the index buffers will be created with a
            system memory shadow buffer. You should set this if you want to be able to
			read from the buffer, because reading from a hardware buffer is a no-no.
		*/
		void setIndexBufferPolicy(HardwareBuffer::Usage usage, bool shadowBuffer = false);
        /** Gets the usage setting for this meshes vertex buffers. */
        HardwareBuffer::Usage getVertexBufferUsage(void) const { return mVertexBufferUsage; }
        /** Gets the usage setting for this meshes index buffers. */
        HardwareBuffer::Usage getIndexBufferUsage(void) const { return mIndexBufferUsage; }
        /** Gets whether or not this meshes vertex buffers are shadowed. */
        bool isVertexBufferShadowed(void) const { return mVertexBufferShadowBuffer; }
        /** Gets whether or not this meshes index buffers are shadowed. */
        bool isIndexBufferShadowed(void) const { return mIndexBufferShadowBuffer; }
       

        /** Rationalises the passed in bone assignment list.
        @remarks
            OGRE supports up to 4 bone assignments per vertex. The reason for this limit
            is that this is the maximum number of assignments that can be passed into
            a hardware-assisted blending algorithm. This method identifies where there are
            more than 4 bone assignments for a given vertex, and eliminates the bone
            assignments with the lowest weights to reduce to this limit. The remaining
            weights are then re-balanced to ensure that they sum to 1.0.
        @param vertexCount The number of vertices.
        @param assignments The bone assignment list to rationalise. This list will be modified and
            entries will be removed where the limits are exceeded.
        @returns The maximum number of bone assignments per vertex found, clamped to [1-4]
        */
        unsigned short _rationaliseBoneAssignments(size_t vertexCount, VertexBoneAssignmentList& assignments);

        /** Internal method, be called once to compile bone assignments into geometry buffer. 
        @remarks
            The OGRE engine calls this method automatically. It compiles the information 
            submitted as bone assignments into a format usable in realtime. It also 
            eliminates excessive bone assignments (max is OGRE_MAX_BLEND_WEIGHTS)
            and re-normalises the remaining assignments.
        */
        void _compileBoneAssignments(void);

        /** Internal method, be called once to update the compiled bone assignments.
        @remarks
            The OGRE engine calls this method automatically. It updates the compiled bone
            assignments if requested.
        */
        void _updateCompiledBoneAssignments(void);

        /** This method builds a set of tangent vectors for a given mesh into a 3D texture coordinate buffer.
        @remarks
            Tangent vectors are vectors representing the local 'X' axis for a given vertex based
            on the orientation of the 2D texture on the geometry. They are built from a combination
            of existing normals, and from the 2D texture coordinates already baked into the model.
            They can be used for a number of things, but most of all they are useful for 
            vertex and fragment programs, when you wish to arrive at a common space for doing
            per-pixel calculations.
        @par
            The prerequisites for calling this method include that the vertex data used by every
            SubMesh has both vertex normals and 2D texture coordinates.
		@param targetSemantic The semantic to store the tangents in. Defaults to 
			the explicit tangent binding, but note that this is only usable on more
			modern hardware (Shader Model 2), so if you need portability with older
			cards you should change this to a texture coordinate binding instead.
        @param sourceTexCoordSet The texture coordinate index which should be used as the source
            of 2D texture coordinates, with which to calculate the tangents.
        @param index The element index, ie the texture coordinate set which should be used to store the 3D
            coordinates representing a tangent vector per vertex, if targetSemantic is 
			VES_TEXTURE_COORDINATES. If this already exists, it will be overwritten.
		@param splitMirrored Sets whether or not to split vertices when a mirrored tangent space
			transition is detected (matrix parity differs). @see TangentSpaceCalc::setSplitMirrored
		@param splitRotated Sets whether or not to split vertices when a rotated tangent space
			is detected. @see TangentSpaceCalc::setSplitRotated
		@param storeParityInW If true, store tangents as a 4-vector and include parity in w
        */
        void buildTangentVectors(VertexElementSemantic targetSemantic = VES_TANGENT,
			unsigned short sourceTexCoordSet = 0, unsigned short index = 0, 
			bool splitMirrored = false, bool splitRotated = false, bool storeParityInW = false);

        /** Ask the mesh to suggest parameters to a future buildTangentVectors call, 
			should you wish to use texture coordinates to store the tangents. 
        @remarks
            This helper method will suggest source and destination texture coordinate sets
            for a call to buildTangentVectors. It will detect when there are inappropriate
            conditions (such as multiple geometry sets which don't agree). 
            Moreover, it will return 'true' if it detects that there are aleady 3D 
            coordinates in the mesh, and therefore tangents may have been prepared already.
		@param targetSemantic The semantic you intend to use to store the tangents
			if they are not already present;
			most likely options are VES_TEXTURE_COORDINATES or VES_TANGENT; you should
			use texture coordinates if you want compatibility with older, pre-SM2
			graphics cards, and the tangent binding otherwise.
        @param outSourceCoordSet Reference to a source texture coordinate set which 
            will be populated
        @param outIndex Reference to a destination element index (e.g. texture coord set)
			which will be populated
        */
        bool suggestTangentVectorBuildParams(VertexElementSemantic targetSemantic,
			unsigned short& outSourceCoordSet, unsigned short& outIndex);

        /** Builds an edge list for this mesh, which can be used for generating a shadow volume
            among other things.
        */
        void buildEdgeList(void);
        /** Destroys and frees the edge lists this mesh has built. */
        void freeEdgeList(void);

        /** This method prepares the mesh for generating a renderable shadow volume. 
        @remarks
            Preparing a mesh to generate a shadow volume involves firstly ensuring that the 
            vertex buffer containing the positions for the mesh is a standalone vertex buffer,
            with no other components in it. This method will therefore break apart any existing
            vertex buffers this mesh holds if position is sharing a vertex buffer. 
            Secondly, it will double the size of this vertex buffer so that there are 2 copies of 
            the position data for the mesh. The first half is used for the original, and the second 
            half is used for the 'extruded' version of the mesh. The vertex count of the main 
            VertexData used to render the mesh will remain the same though, so as not to add any 
            overhead to regular rendering of the object.
            Both copies of the position are required in one buffer because shadow volumes stretch 
            from the original mesh to the extruded version. 
        @par
            Because shadow volumes are rendered in turn, no additional
            index buffer space is allocated by this method, a shared index buffer allocated by the
            shadow rendering algorithm is used for addressing this extended vertex buffer.
        */
        void prepareForShadowVolume(void);

        /** Return the edge list for this mesh, building it if required. 
        @remarks
            You must ensure that the Mesh as been prepared for shadow volume 
            rendering if you intend to use this information for that purpose.
        @lodIndex The LOD at which to get the edge list, 0 being the highest.
        */
        EdgeData* getEdgeList(unsigned short lodIndex = 0);

        /** Return the edge list for this mesh, building it if required. 
        @remarks
            You must ensure that the Mesh as been prepared for shadow volume 
            rendering if you intend to use this information for that purpose.
        @lodIndex The LOD at which to get the edge list, 0 being the highest.
        */
        const EdgeData* getEdgeList(unsigned short lodIndex = 0) const;

        /** Returns whether this mesh has already had it's geometry prepared for use in 
            rendering shadow volumes. */
        bool isPreparedForShadowVolumes(void) const { return mPreparedForShadowVolumes; }

		/** Returns whether this mesh has an attached edge list. */
		bool isEdgeListBuilt(void) const { return mEdgeListsBuilt; }

        /** Prepare matrices for software indexed vertex blend.
            @remarks
                This function organise bone indexed matrices to blend indexed matrices,
                so software vertex blending can access to the matrix via blend index
                directly.
            @param blendMatrices Pointer to an array of matrix pointers to store
                prepared results, which indexed by blend index
            @param boneMatrices Pointer to an array of matrices to be used to blend,
                which indexed by bone index
            @param indexMap The index map used to translate blend index to bone index
        */
        static void prepareMatricesForVertexBlend(const Matrix4** blendMatrices,
            const Matrix4* boneMatrices, const IndexMap& indexMap);

        /** Performs a software indexed vertex blend, of the kind used for
            skeletal animation although it can be used for other purposes. 
        @remarks
        This function is supplied to update vertex data with blends 
        done in software, either because no hardware support is available, 
        or that you need the results of the blend for some other CPU operations.
        @param sourceVertexData VertexData class containing positions, normals,
            blend indices and blend weights.
        @param targetVertexData VertexData class containing target position
            and normal buffers which will be updated with the blended versions.
            Note that the layout of the source and target position / normal 
            buffers must be identical, ie they must use the same buffer indexes
        @param blendMatrices Pointer to an array of matrix pointers to be used to blend,
            indexed by blend indices in the sourceVertexData
        @param numMatrices Number of matrices in the blendMatrices, it might be used
            as a hint for optimisation.
        @param blendNormals If true, normals are blended as well as positions
        */
        static void softwareVertexBlend(const VertexData* sourceVertexData, 
            const VertexData* targetVertexData,
            const Matrix4* const* blendMatrices, size_t numMatrices,
            bool blendNormals);

        /** Performs a software vertex morph, of the kind used for
            morph animation although it can be used for other purposes. 
        @remarks
			This function will linearly interpolate positions between two
			source buffers, into a third buffer.
        @param t Parametric distance between the start and end buffer positions
        @param b1 Vertex buffer containing VET_FLOAT3 entries for the start positions
		@param b2 Vertex buffer containing VET_FLOAT3 entries for the end positions
		@param targetVertexData VertexData destination; assumed to have a separate position
			buffer already bound, and the number of vertices must agree with the
			number in start and end
		*/
        static void softwareVertexMorph(Real t, 
            const HardwareVertexBufferSharedPtr& b1, 
			const HardwareVertexBufferSharedPtr& b2, 
			VertexData* targetVertexData);

        /** Performs a software vertex pose blend, of the kind used for
            morph animation although it can be used for other purposes. 
        @remarks
			This function will apply a weighted offset to the positions in the 
			incoming vertex data (therefore this is a read/write operation, and 
			if you expect to call it more than once with the same data, then
			you would be best to suppress hardware uploads of the position buffer
			for the duration)
        @param weight Parametric weight to scale the offsets by
		@param vertexOffsetMap Potentially sparse map of vertex index -> offset
		@param targetVertexData VertexData destination; assumed to have a separate position
			buffer already bound, and the number of vertices must agree with the
			number in start and end
		*/
		static void softwareVertexPoseBlend(Real weight, 
			const map<size_t, Vector3>::type& vertexOffsetMap,
			VertexData* targetVertexData);
        /** Gets a reference to the optional name assignments of the SubMeshes. */
        const SubMeshNameMap& getSubMeshNameMap(void) const { return mSubMeshNameMap; }

        /** Sets whether or not this Mesh should automatically build edge lists
            when asked for them, or whether it should never build them if
            they are not already provided.
        @remarks
            This allows you to create meshes which do not have edge lists calculated, 
            because you never want to use them. This value defaults to 'true'
            for mesh formats which did not include edge data, and 'false' for 
            newer formats, where edge lists are expected to have been generated
            in advance.
        */
        void setAutoBuildEdgeLists(bool autobuild) { mAutoBuildEdgeLists = autobuild; }
        /** Sets whether or not this Mesh should automatically build edge lists
            when asked for them, or whether it should never build them if
            they are not already provided.
        */
        bool getAutoBuildEdgeLists(void) const { return mAutoBuildEdgeLists; }

		/** Gets the type of vertex animation the shared vertex data of this mesh supports.
		*/
		virtual VertexAnimationType getSharedVertexDataAnimationType(void) const;

		/** Creates a new Animation object for vertex animating this mesh. 
        @param name The name of this animation
        @param length The length of the animation in seconds
        */
        virtual Animation* createAnimation(const String& name, Real length);

        /** Returns the named vertex Animation object. 
		@param name The name of the animation
		*/
        virtual Animation* getAnimation(const String& name) const;

		/** Internal access to the named vertex Animation object - returns null 
			if it does not exist. 
		@param name The name of the animation
		*/
		virtual Animation* _getAnimationImpl(const String& name) const;

		/** Returns whether this mesh contains the named vertex animation. */
		virtual bool hasAnimation(const String& name);

        /** Removes vertex Animation from this mesh. */
        virtual void removeAnimation(const String& name);

		/** Gets the number of morph animations in this mesh. */
		virtual unsigned short getNumAnimations(void) const;

		/** Gets a single morph animation by index. 
		*/
		virtual Animation* getAnimation(unsigned short index) const;

		/** Removes all morph Animations from this mesh. */
		virtual void removeAllAnimations(void);
		/** Gets a pointer to a vertex data element based on a morph animation 
			track handle.
		@remarks
			0 means the shared vertex data, 1+ means a submesh vertex data (index+1)
		*/
		VertexData* getVertexDataByTrackHandle(unsigned short handle);
        /** Iterates through all submeshes and requests them 
            to apply their texture aliases to the material they use.
        @remarks
            The submesh will only apply texture aliases to the material if matching
            texture alias names are found in the material.  If a match is found, the
            submesh will automatically clone the original material and then apply its
            texture to the new material.
        @par
            This method is normally called by the protected method loadImpl when a 
            mesh if first loaded.
        */
  		void updateMaterialForAllSubMeshes(void);

		/** Internal method which, if animation types have not been determined,
			scans any vertex animations and determines the type for each set of
			vertex data (cannot have 2 different types).
		*/
		void _determineAnimationTypes(void) const;
		/** Are the derived animation types out of date? */
		bool _getAnimationTypesDirty(void) const { return mAnimationTypesDirty; }

		/** Create a new Pose for this mesh or one of its submeshes.
  	    @param target The target geometry index; 0 is the shared Mesh geometry, 1+ is the
			dedicated SubMesh geometry belonging to submesh index + 1.
		@param name Name to give the pose, which is optional
		@returns A new Pose ready for population
		*/
		Pose* createPose(ushort target, const String& name = StringUtil::BLANK);
		/** Get the number of poses.*/
		size_t getPoseCount(void) const { return mPoseList.size(); }
		/** Retrieve an existing Pose by index.*/
		Pose* getPose(ushort index);
		/** Retrieve an existing Pose by name.*/
		Pose* getPose(const String& name);
		/** Destroy a pose by index.
		@note This will invalidate any animation tracks referring to this pose or those after it.
		*/
		void removePose(ushort index);
		/** Destroy a pose by name.
		@note This will invalidate any animation tracks referring to this pose or those after it.
		*/
		void removePose(const String& name);
		/** Destroy all poses */
		void removeAllPoses(void);

		typedef VectorIterator<PoseList> PoseIterator;
		typedef ConstVectorIterator<PoseList> ConstPoseIterator;

		/** Get an iterator over all the poses defined. */
		PoseIterator getPoseIterator(void);
		/** Get an iterator over all the poses defined. */
		ConstPoseIterator getPoseIterator(void) const;
		/** Get pose list */
		const PoseList& getPoseList(void) const;

        /** Get lod strategy used by this mesh. */
        const LodStrategy *getLodStrategy() const;
        /** Set the lod strategy used by this mesh. */
        void setLodStrategy(LodStrategy *lodStrategy);

    };

    /** Specialisation of SharedPtr to allow SharedPtr to be assigned to MeshPtr 
    @note Has to be a subclass since we need operator=.
    We could templatise this instead of repeating per Resource subclass, 
    except to do so requires a form VC6 does not support i.e.
    ResourceSubclassPtr<T> : public SharedPtr<T>
    */
    class _OgreExport MeshPtr : public SharedPtr<Mesh> 
    {
    public:
        MeshPtr() : SharedPtr<Mesh>() {}
        explicit MeshPtr(Mesh* rep) : SharedPtr<Mesh>(rep) {}
        MeshPtr(const MeshPtr& r) : SharedPtr<Mesh>(r) {} 
        MeshPtr(const ResourcePtr& r);
        /// Operator used to convert a ResourcePtr to a MeshPtr
        MeshPtr& operator=(const ResourcePtr& r);
    protected:
        /// Override destroy since we need to delete Mesh after fully defined
        void destroy(void);
    };

	/** A way of recording the way each LODs is recorded this Mesh. */
	struct MeshLodUsage
	{
        /** User-supplied values used to determine when th is lod applies.
        @remarks
            This is required in case the lod strategy changes.
        */
        Real userValue;

		/** Value used by to determine when this lod applies.
		@remarks
			May be interpretted differently by different strategies.
            Transformed from user-supplied values with LodStrategy::transformUserValue.
		*/
		Real value;
		
		/// Only relevant if mIsLodManual is true, the name of the alternative mesh to use
		String manualName;
		/// Hard link to mesh to avoid looking up each time
		mutable MeshPtr manualMesh;
        /// Edge list for this LOD level (may be derived from manual mesh)
        mutable EdgeData* edgeData;

		MeshLodUsage() : userValue(0.0), value(0.0), edgeData(0) {}
	};

	/** @} */
	/** @} */


} // namespace

#endif
