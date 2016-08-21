/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#ifndef __Entity_H__
#define __Entity_H__

#include "OgrePrerequisites.h"
#include "OgreCommon.h"

#include "OgreMovableObject.h"
#include "OgreQuaternion.h"
#include "OgreVector3.h"
#include "OgreHardwareBufferManager.h"
#include "OgreRenderable.h"
#include "OgreResourceGroupManager.h"
#include "OgreSubEntity.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {
namespace v1 {
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Scene
    *  @{
    */
    /** Defines an instance of a discrete, movable object based on a Mesh.
    @remarks
        Ogre generally divides renderable objects into 2 groups, discrete
        (separate) and relatively small objects which move around the world,
        and large, sprawling geometry which makes up generally immovable
        scenery, aka 'level geometry'.
    @par
        The Mesh and SubMesh classes deal with the definition of the geometry
        used by discrete movable objects. Entities are actual instances of
        objects based on this geometry in the world. Therefore there is
        usually a single set Mesh for a car, but there may be multiple
        entities based on it in the world. Entities are able to override
        aspects of the Mesh it is defined by, such as changing material
        properties per instance (so you can have many cars using the same
        geometry but different textures for example). Because a Mesh is split
        into SubMeshes for this purpose, the Entity class is a grouping class
        (much like the Mesh class) and much of the detail regarding
        individual changes is kept in the SubEntity class. There is a 1:1
        relationship between SubEntity instances and the SubMesh instances
        associated with the Mesh the Entity is based on.
    @par
        Entity and SubEntity classes are never created directly. Use the
        createEntity method of the SceneManager (passing a model name) to
        create one.
    @par
        Entities are included in the scene by associating them with a
        SceneNode, using the attachEntity method. See the SceneNode class
        for full information.
    @note
        No functions were declared virtual to improve performance.
    */
    class _OgreExport Entity : public MovableObject, public Resource::Listener
    {
        // Allow EntityFactory full access
        friend class EntityFactory;
        friend class SubEntity;
    public:
        
        typedef set<Entity*>::type EntitySet;
        typedef map<unsigned short, bool>::type SchemeHardwareAnimMap;

    protected:

        /** Private constructor (instances cannot be created directly).
        */
        Entity( IdType id, ObjectMemoryManager *objectMemoryManager, SceneManager *manager );
        /** Private constructor.
        */
        Entity( IdType id, ObjectMemoryManager *objectMemoryManager, SceneManager *manager,
                const MeshPtr& mesh );

        /** The Mesh that this Entity is based on.
        */
        MeshPtr mMesh;

        /** List of SubEntities (point to SubMeshes).
        */
        typedef vector<SubEntity>::type SubEntityList;
        SubEntityList mSubEntityList;


        /// State of animation for animable meshes
        AnimationStateSet* mAnimationState;


        /// Temp buffer details for software skeletal anim of shared geometry
        TempBlendedBufferInfo mTempSkelAnimInfo;
        /// Vertex data details for software skeletal anim of shared geometry
        VertexData* mSkelAnimVertexData;
        /// Temp buffer details for software vertex anim of shared geometry
        TempBlendedBufferInfo mTempVertexAnimInfo;
        /// Vertex data details for software vertex anim of shared geometry
        VertexData* mSoftwareVertexAnimVertexData;
        /// Vertex data details for hardware vertex anim of shared geometry
        /// - separate since we need to s/w anim for shadows whilst still altering
        ///   the vertex data for hardware morphing (pos2 binding)
        VertexData* mHardwareVertexAnimVertexData;
        /// Have we applied any vertex animation to shared geometry?
        bool mVertexAnimationAppliedThisFrame;
        /// Have the temp buffers already had their geometry prepared for use in rendering shadow volumes?
        bool mPreparedForShadowVolumes;

        /** Internal method - given vertex data which could be from the Mesh or
            any submesh, finds the temporary blend copy.
        */
        const VertexData* findBlendedVertexData(const VertexData* orig);
        /** Internal method - given vertex data which could be from the Mesh or
            any SubMesh, finds the corresponding SubEntity.
        */
        SubEntity* findSubEntityForVertexData(const VertexData* orig);

        /** Internal method for extracting metadata out of source vertex data
            for fast assignment of temporary buffers later.
        */
        void extractTempBufferInfo(VertexData* sourceData, TempBlendedBufferInfo* info);
        /** Internal method to clone vertex data definitions but to remove blend buffers. */
        VertexData* cloneVertexDataRemoveBlendInfo(const VertexData* source);
        /** Internal method for preparing this Entity for use in animation. */
        void prepareTempBlendBuffers(void);
        /** Mark all vertex data as so far unanimated.
        */
        void markBuffersUnusedForAnimation(void);
        /** Internal method to restore original vertex data where we didn't
            perform any vertex animation this frame.
        */
        void restoreBuffersForUnusedAnimation(bool hardwareAnimation);

        /** Ensure that any unbound  pose animation buffers are bound to a safe
            default.
        @param srcData
            Original vertex data containing original positions.
        @param destData
            Hardware animation vertex data to be checked.
        */
        void bindMissingHardwarePoseBuffers(const VertexData* srcData, 
            VertexData* destData);
            
        /** When performing software pose animation, initialise software copy
            of vertex data.
        */
        void initialisePoseVertexData(const VertexData* srcData, VertexData* destData, 
            bool animateNormals);

        /** When animating normals for pose animation, finalise normals by filling in
            with the reference mesh normal where applied normal weights < 1.
        */
        void finalisePoseNormals(const VertexData* srcData, VertexData* destData);

        /// Cached bone matrices, including any world transform.
        Matrix4 *mBoneWorldMatrices;
        /// Cached bone matrices in skeleton local space, might shares with other entity instances.
        Matrix4 *mBoneMatrices;
        unsigned short mNumBoneMatrices;
        /// Records the last frame in which animation was updated.
        unsigned long mFrameAnimationLastUpdated;

        /// Perform all the updates required for an animated entity.
        void updateAnimation(void);

        /// Records the last frame in which the bones was updated.
        /// It's a pointer because it can be shared between different entities with
        /// a shared skeleton.
        unsigned long *mFrameBonesLastUpdated;

        /** A set of all the entities which shares a single OldSkeletonInstance.
            This is only created if the entity is in fact sharing it's OldSkeletonInstance with
            other Entities.
        */
        EntitySet* mSharedSkeletonEntities;

        /** Private method to cache bone matrices from skeleton.
        @return
            True if the bone matrices cache has been updated. False if note.
        */
        bool cacheBoneMatrices(void);

        /// Flag determines whether or not to display skeleton.
        bool mDisplaySkeleton;
        /** Flag indicating whether hardware animation is supported by this entities materials
            data is saved per scehme number.
        */
        SchemeHardwareAnimMap mSchemeHardwareAnim;

        /// Current state of the hardware animation as represented by the entities parameters.
        bool mCurrentHWAnimationState;

        /// Number of hardware poses supported by materials.
        ushort mHardwarePoseCount;
        /// Flag indicating whether we have a vertex program in use on any of our subentities.
        bool mVertexProgramInUse;
        /// Counter indicating number of requests for software animation.
        int mSoftwareAnimationRequests;
        /// Counter indicating number of requests for software blended normals.
        int mSoftwareAnimationNormalsRequests;
        /// Flag indicating whether to skip automatic updating of the Skeleton's AnimationState.
        bool mSkipAnimStateUpdates;
        /// Flag indicating whether to update the main entity skeleton even when an LOD is displayed.
        bool mAlwaysUpdateMainSkeleton;
        /// Flag indicating whether to update the bounding box from the bones of the skeleton.
        bool mUpdateBoundingBoxFromSkeleton;

        /** List of LOD Entity instances (for manual LODs).
            We don't know when the mesh is using manual LODs whether one LOD to the next will have the
            same number of SubMeshes, therefore we have to allow a separate Entity list
            with each alternate one.
        */
        typedef vector<Entity*>::type LODEntityList;
        LODEntityList mLodEntityList;

        /** This Entity's personal copy of the skeleton, if skeletally animated.
        */
        OldSkeletonInstance* mSkeletonInstance;

        /// Has this entity been initialised yet?
        bool mInitialised;

        /// Last parent transform.
        Matrix4 mLastParentXform;

        /// Mesh state count, used to detect differences.
        size_t mMeshStateCount;

        /** Builds a list of SubEntities based on the SubMeshes contained in the Mesh. */
        void buildSubEntityList(MeshPtr& mesh, SubEntityList* sublist);

        /// Internal implementation of attaching a 'child' object to this entity and assign the parent node to the child entity.
        void attachObjectImpl(MovableObject *pMovable, TagPoint *pAttachingPoint);

        /// Internal implementation of detaching a 'child' object of this entity and clear the parent node of the child entity.
        void detachObjectImpl(MovableObject* pObject);

        /// Ensures reevaluation of the vertex processing usage.
        void reevaluateVertexProcessing(void);

        /** Calculates the kind of vertex processing in use.
        @remarks
            This function's return value is calculated according to the current 
            active scheme. This is due to the fact that RTSS schemes may be different
            in their handling of hardware animation.
        */
        bool calcVertexProcessing(void);
    
        /// Apply vertex animation.
        void applyVertexAnimation(bool hardwareAnimation);
        /// Initialise the hardware animation elements for given vertex data.
        ushort initHardwareAnimationElements(VertexData* vdata, ushort numberOfElements, bool animateNormals);
        /// Are software vertex animation temp buffers bound?
        bool tempVertexAnimBuffersBound(void) const;
        /// Are software skeleton animation temp buffers bound?
        bool tempSkelAnimBuffersBound(bool requestNormals) const;

    public:
        /** Default destructor.
        */
        ~Entity();

        /** Gets the Mesh that this Entity is based on.
        */
        const MeshPtr& getMesh(void) const;

        /** Gets a pointer to a SubEntity, ie a part of an Entity.
        */
        SubEntity* getSubEntity(size_t index);
        const SubEntity* getSubEntity(size_t index) const;

        /** Gets a pointer to a SubEntity by name
        @remarks 
            Names should be initialized during a Mesh creation.
        */
        SubEntity* getSubEntity( const String& name );
        const SubEntity* getSubEntity( const String& name ) const;

        /** Retrieves the number of SubEntity objects making up this entity.
        */
        size_t getNumSubEntities(void) const;

        /// Sets the given HLMS datablock to all SubEntities
        void setDatablock( HlmsDatablock *datablock );

        /// Sets the given HLMS datablock to all SubEntities
        void setDatablock( IdString datablockName );

        /** Clones this entity and returns a pointer to the clone.
        @remarks
            Useful method for duplicating an entity. The new entity must be
            given a unique name, and is not attached to the scene in any way
            so must be attached to a SceneNode to be visible (exactly as
            entities returned from SceneManager::createEntity).
        @param newName
            Name for the new entity.
        */
        Entity* clone(void) const;

        /** Sets the material to use for the whole of this Item.
        @remarks
            This is a shortcut method to set all the materials for all
            subentities of this Item. Only use this method is you want to
            set the same material for all subentities or if you know there
            is only one. Otherwise call getSubItem() and call the same
            method on the individual SubItem.
        */
        void setDatablockOrMaterialName( const String& name,
                                         const String& groupName = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME );

        /** Sets the material to use for the whole of this entity.
        @remarks
            This is a shortcut method to set all the materials for all
            subentities of this entity. Only use this method is you want to
            set the same material for all subentities or if you know there
            is only one. Otherwise call getSubEntity() and call the same
            method on the individual SubEntity.
        */
        void setMaterialName( const String& name, const String& groupName = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME );

        
        /** Sets the material to use for the whole of this entity.
        @remarks
            This is a shortcut method to set all the materials for all
            subentities of this entity. Only use this method is you want to
            set the same material for all subentities or if you know there
            is only one. Otherwise call getSubEntity() and call the same
            method on the individual SubEntity.
        */
        void setMaterial(const MaterialPtr& material);

        /** Sets the RenderQueue subgroup ID for the whole of this entity.
        @remarks
            This is a shortcut method to set all the subgroups for all
            subentities of this entity. Only use this method is you want to
            set the same subgroup for all subentities or if you know there
            is only one. Otherwise call getSubEntity() and call the same
            method on the individual SubEntity.
        */
        void setRenderQueueSubGroup( uint8 subGroup );

        /// @copydoc MovableObject::setRenderQueueGroup.
        void setRenderQueueGroup(uint8 queueID);

        /** @copydoc MovableObject::_updateRenderQueue.
        */
        void _updateRenderQueue(RenderQueue* queue, Camera *camera, const Camera *lodCamera);

        /** @copydoc MovableObject::getMovableType */
        const String& getMovableType(void) const;

        /** For entities based on animated meshes, gets the AnimationState object for a single animation.
        @remarks
            You animate an entity by updating the animation state objects. Each of these represents the
            current state of each animation available to the entity. The AnimationState objects are
            initialised from the Mesh object.
        */
        AnimationState* getAnimationState(const String& name) const;
        /** Returns whether the AnimationState with the given name exists. */
        bool hasAnimationState(const String& name) const;
        /** For entities based on animated meshes, gets the AnimationState objects for all animations.
        @return
            In case the entity is animated, this functions returns the pointer to a AnimationStateSet
            containing all animations of the entries. If the entity is not animated, it returns 0.
        @remarks
            You animate an entity by updating the animation state objects. Each of these represents the
            current state of each animation available to the entity. The AnimationState objects are
            initialised from the Mesh object.
        */
        AnimationStateSet* getAllAnimationStates(void) const;

        /** Tells the Entity whether or not it should display it's skeleton, if it has one.
        */
        void setDisplaySkeleton(bool display);

        /** Returns whether or not the entity is currently displaying its skeleton.
        */
        bool getDisplaySkeleton(void) const;

        /** Gets a pointer to the entity representing the numbered manual level of detail.
        @remarks
            The zero-based index never includes the original entity, unlike
            Mesh::getLodLevel.
        */
        Entity* getManualLodLevel(size_t index) const;

        /** Returns the number of manual levels of detail that this entity supports.
        @remarks
            This number never includes the original entity, it is difference
            with Mesh::getNumLodLevels.
        */
        size_t getNumManualLodLevels(void) const;

        /** Sets whether the polygon mode of this entire entity may be
            overridden by the camera detail settings.
        */
        void setPolygonModeOverrideable(bool PolygonModeOverrideable);
        /** Attaches another object to a certain bone of the skeleton which this entity uses.
        @remarks
            This method can be used to attach another object to an animated part of this entity,
            by attaching it to a bone in the skeleton (with an offset if required). As this entity
            is animated, the attached object will move relative to the bone to which it is attached.
        @par
            An exception is thrown if the movable object is already attached to the bone, another bone or scenenode.
            If the entity has no skeleton or the bone name cannot be found then an exception is thrown.
        @param boneName
            The name of the bone (in the skeleton) to attach this object
        @param pMovable
            Pointer to the object to attach
        @param offsetOrientation
            An adjustment to the orientation of the attached object, relative to the bone.
        @param offsetPosition
            An adjustment to the position of the attached object, relative to the bone.
        @return
            The TagPoint to which the object has been attached
        */
        TagPoint* attachObjectToBone(const String &boneName,
            MovableObject *pMovable,
            const Quaternion &offsetOrientation = Quaternion::IDENTITY,
            const Vector3 &offsetPosition = Vector3::ZERO);

        /** Detach a MovableObject previously attached using attachObjectToBone.
            If the movable object name is not found then an exception is raised.
        @param movableName
            The name of the movable object to be detached.
        */
        MovableObject* detachObjectFromBone(const String &movableName);

        /** Detaches an object by pointer.
        @remarks
            Use this method to destroy a MovableObject which is attached to a bone of belonging this entity.
            But sometimes the object may be not in the child object list because it is a LOD entity,
            this method can safely detect and ignore in this case and won't raise an exception.
        */
        void detachObjectFromBone(MovableObject* obj);

        /// Detach all MovableObjects previously attached using attachObjectToBone
        void detachAllObjectsFromBone(void);

#ifdef ENABLE_INCOMPATIBLE_OGRE_2_0
        /// Contains the child objects (attached to bones) indexed by name.
        typedef map<String, MovableObject*>::type ChildObjectList;
        typedef MapIterator<ChildObjectList> ChildObjectListIterator;
        /** Gets an iterator to the list of objects attached to bones on this entity. */
        ChildObjectListIterator getAttachedObjectIterator(void);
#endif
        /** @copydoc ShadowCaster::getEdgeList. */
        EdgeData* getEdgeList(void);
        /** @copydoc ShadowCaster::hasEdgeList. */
        bool hasEdgeList(void);

        /** Internal method for retrieving bone matrix information. */
        const Matrix4* _getBoneMatrices(void) const { return mBoneMatrices;}
        /** Internal method for retrieving bone matrix information. */
        unsigned short _getNumBoneMatrices(void) const { return mNumBoneMatrices; }
        /** Returns whether or not this entity is skeletally animated. */
        bool hasSkeleton(void) const { return mSkeletonInstance != 0; }
        /** Get this Entity's personal skeleton instance. */
        OldSkeletonInstance* getSkeleton(void) const { return mSkeletonInstance; }
        /** Returns whether or not hardware animation is enabled.
        @remarks
            Because fixed-function indexed vertex blending is rarely supported
            by existing graphics cards, hardware animation can only be done if
            the vertex programs in the materials used to render an entity support
            it. Therefore, this method will only return true if all the materials
            assigned to this entity have vertex programs assigned, and all those
            vertex programs must support 'includes_morph_animation true' if using
            morph animation, 'includes_pose_animation true' if using pose animation
            and 'includes_skeletal_animation true' if using skeletal animation.

            Also note the the function returns value according to the current active
            scheme. This is due to the fact that RTSS schemes may be different in their
            handling of hardware animation.
        */
        bool isHardwareAnimationEnabled(void);

        /** @copydoc MovableObject::_notifyAttached */
        void _notifyAttached( Node* parent );
        /** Returns the number of requests that have been made for software animation
        @remarks
            If non-zero then software animation will be performed in updateAnimation
            regardless of the current setting of isHardwareAnimationEnabled or any
            internal optimise for eliminate software animation. Requests for software
            animation are made by calling the addSoftwareAnimationRequest() method.
        */
        int getSoftwareAnimationRequests(void) const { return mSoftwareAnimationRequests; }
        /** Returns the number of requests that have been made for software animation of normals
        @remarks
            If non-zero, and getSoftwareAnimationRequests() also returns non-zero,
            then software animation of normals will be performed in updateAnimation
            regardless of the current setting of isHardwareAnimationEnabled or any
            internal optimise for eliminate software animation. Currently it is not
            possible to force software animation of only normals. Consequently this
            value is always less than or equal to that returned by getSoftwareAnimationRequests().
            Requests for software animation of normals are made by calling the
            addSoftwareAnimationRequest() method with 'true' as the parameter.
        */
        int getSoftwareAnimationNormalsRequests(void) const { return mSoftwareAnimationNormalsRequests; }
        /** Add a request for software animation
        @remarks
            Tells the entity to perform animation calculations for skeletal/vertex
            animations in software, regardless of the current setting of
            isHardwareAnimationEnabled().  Software animation will be performed
            any time one or more requests have been made.  If 'normalsAlso' is
            'true', then the entity will also do software blending on normal
            vectors, in addition to positions. This advanced method useful for
            situations in which access to actual mesh vertices is required,
            such as accurate collision detection or certain advanced shading
            techniques. When software animation is no longer needed,
            the caller of this method should always remove the request by calling
            removeSoftwareAnimationRequest(), passing the same value for
            'normalsAlso'.
        */
        void addSoftwareAnimationRequest(bool normalsAlso);
        /** Removes a request for software animation
        @remarks
            Calling this decrements the entity's internal counter of the number
            of requests for software animation.  If the counter is already zero
            then calling this method throws an exception.  The 'normalsAlso'
            flag if set to 'true' will also decrement the internal counter of
            number of requests for software animation of normals.
        */
        void removeSoftwareAnimationRequest(bool normalsAlso);

        /** Shares the OldSkeletonInstance with the supplied entity.
            Note that in order for this to work, both entities must have the same
            Skeleton.
        */
        void shareSkeletonInstanceWith(Entity* entity);

        /** Returns whether or not this entity is either morph or pose animated.
        */
        bool hasVertexAnimation(void) const;


        /** Stops sharing the OldSkeletonInstance with other entities.
        */
        void stopSharingSkeletonInstance();


        /** Returns whether this entity shares it's SkeltonInstance with other entity instances.
        */
        inline bool sharesSkeletonInstance() const { return mSharedSkeletonEntities != NULL; }

        /** Returns a pointer to the set of entities which share a OldSkeletonInstance.
            If this instance does not share it's OldSkeletonInstance with other instances @c NULL will be returned
        */
        inline const EntitySet* getSkeletonInstanceSharingSet() const { return mSharedSkeletonEntities; }

        /** Updates the internal animation state set to include the latest
            available animations from the attached skeleton.
        @remarks
            Use this method if you manually add animations to a skeleton, or have
            linked the skeleton to another for animation purposes since creating
            this entity.
        @note
            If you have called getAnimationState prior to calling this method,
            the pointers will still remain valid.
        */
        void refreshAvailableAnimationState(void);

        /** Advanced method to perform all the updates required for an animated entity.
        @remarks
            You don't normally need to call this, but it's here in case you wish
            to manually update the animation of an Entity at a specific point in
            time. Animation will not be updated more than once a frame no matter
            how many times you call this method.
        */
        void _updateAnimation(void);

        /** Tests if any animation applied to this entity.
        @remarks
            An entity is animated if any animation state is enabled, or any manual bone
            applied to the skeleton.
        */
        bool _isAnimated(void) const;

        /** Tests if skeleton was animated.
        */
        bool _isSkeletonAnimated(void) const;

        /** Advanced method to get the temporarily blended skeletal vertex information
            for entities which are software skinned.
        @remarks
            Internal engine will eliminate software animation if possible, this
            information is unreliable unless added request for software animation
            via addSoftwareAnimationRequest.
        @note
            The positions/normals of the returned vertex data is in object space.
        */
        VertexData* _getSkelAnimVertexData(void) const;
        /** Advanced method to get the temporarily blended software vertex animation information
        @remarks
            Internal engine will eliminate software animation if possible, this
            information is unreliable unless added request for software animation
            via addSoftwareAnimationRequest.
        @note
            The positions/normals of the returned vertex data is in object space.
        */
        VertexData* _getSoftwareVertexAnimVertexData(void) const;
        /** Advanced method to get the hardware morph vertex information
        @note
            The positions/normals of the returned vertex data is in object space.
        */
        VertexData* _getHardwareVertexAnimVertexData(void) const;
        /** Advanced method to get the temp buffer information for software
            skeletal animation.
        */
        TempBlendedBufferInfo* _getSkelAnimTempBufferInfo(void);
        /** Advanced method to get the temp buffer information for software
            morph animation.
        */
        TempBlendedBufferInfo* _getVertexAnimTempBufferInfo(void);
        /// Override to return specific type flag.
        uint32 getTypeFlags(void) const;
        /// Retrieve the VertexData which should be used for GPU binding.
        VertexData* getVertexDataForBinding( bool casterPass );

        /// Identify which vertex data we should be sending to the renderer.
        enum VertexDataBindChoice
        {
            BIND_ORIGINAL,
            BIND_SOFTWARE_SKELETAL,
            BIND_SOFTWARE_MORPH,
            BIND_HARDWARE_MORPH
        };
        /// Choose which vertex data to bind to the renderer.
        VertexDataBindChoice chooseVertexDataForBinding(bool hasVertexAnim);

        /** Are buffers already marked as vertex animated? */
        bool _getBuffersMarkedForAnimation(void) const { return mVertexAnimationAppliedThisFrame; }
        /** Mark just this vertex data as animated.
        */
        void _markBuffersUsedForAnimation(void);

        /** Has this Entity been initialised yet?
        @remarks
            If this returns false, it means this Entity hasn't been completely
            constructed yet from the underlying resources (Mesh, Skeleton), which 
            probably means they were delay-loaded and aren't available yet. This
            Entity won't render until it has been successfully initialised, nor
            will many of the manipulation methods function.
        */
        bool isInitialised(void) const { return mInitialised; }

        /** Try to initialise the Entity from the underlying resources.
        @remarks
            This method builds the internal structures of the Entity based on it
            resources (Mesh, Skeleton). This may or may not succeed if the 
            resources it references have been earmarked for background loading,
            so you should check isInitialised afterwards to see if it was successful.
        @param forceReinitialise
            If @c true, this forces the Entity to tear down it's
            internal structures and try to rebuild them. Useful if you changed the
            content of a Mesh or Skeleton at runtime.
        */
        void _initialise(bool forceReinitialise = false);
        /** Tear down the internal structures of this Entity, rendering it uninitialised. */
        void _deinitialise(void);

        /** Entity's skeleton's AnimationState will not be automatically updated when set to true.
            Useful if you wish to handle AnimationState updates manually.
        */
        void setSkipAnimationStateUpdate(bool skip) {
            mSkipAnimStateUpdates = skip;
        }
        
        /** Entity's skeleton's AnimationState will not be automatically updated when set to true.
            Useful if you wish to handle AnimationState updates manually.
        */
        bool getSkipAnimationStateUpdate() const {
            return mSkipAnimStateUpdates;
        }

        /** The skeleton of the main entity will be updated even if the an LOD entity is being displayed.
            useful if you have entities attached to the main entity. Otherwise position of attached
            entities will not be updated.
        */
        void setAlwaysUpdateMainSkeleton(bool update) {
            mAlwaysUpdateMainSkeleton = update;
        }

        /** The skeleton of the main entity will be updated even if the an LOD entity is being displayed.
            useful if you have entities attached to the main entity. Otherwise position of attached
            entities will not be updated.
        */
        bool getAlwaysUpdateMainSkeleton() const {
            return mAlwaysUpdateMainSkeleton;
        }

        /** If true, the skeleton of the entity will be used to update the bounding box for culling.
            Useful if you have skeletal animations that move the bones away from the root.  Otherwise, the
            bounding box of the mesh in the binding pose will be used.
        @remarks
            When true, the bounding box will be generated to only enclose the bones that are used for skinning.
            Also the resulting bounding box will be expanded by the amount of GetMesh()->getBoneBoundingRadius().
            The expansion amount can be changed on the mesh to achieve a better fitting bounding box.
        */
        void setUpdateBoundingBoxFromSkeleton(bool update);

        /** If true, the skeleton of the entity will be used to update the bounding box for culling.
            Useful if you have skeletal animations that move the bones away from the root.  Otherwise, the
            bounding box of the mesh in the binding pose will be used.
        */
        bool getUpdateBoundingBoxFromSkeleton() const {
            return mUpdateBoundingBoxFromSkeleton;
        }

        
    };

    /** Factory object for creating Entity instances */
    class _OgreExport EntityFactory : public MovableObjectFactory
    {
    protected:
        virtual MovableObject* createInstanceImpl( IdType id, ObjectMemoryManager *objectMemoryManager,
                                                   SceneManager *manager,
                                                   const NameValuePairList* params = 0 );
    public:
        EntityFactory() {}
        ~EntityFactory() {}

        static String FACTORY_TYPE_NAME;

        const String& getType(void) const;
        void destroyInstance( MovableObject* obj);

    };
    /** @} */
    /** @} */

}
} // namespace Ogre

#include "OgreHeaderSuffix.h"

#endif // __Entity_H__
