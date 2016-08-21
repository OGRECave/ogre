/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#ifndef __InstanceBatch_H__
#define __InstanceBatch_H__

#include "OgrePrerequisites.h"
#include "OgreRenderOperation.h"
#include "OgreRenderable.h"
#include "OgreMovableObject.h"
#include "OgreMesh.h"
#include "OgreHeaderPrefix.h"
#include "Math/Array/OgreObjectMemoryManager.h"

namespace Ogre
{
namespace v1
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Scene
    *  @{
    */

    /** InstanceBatch forms part of the new Instancing system
        This is an abstract class that must be derived to implement different instancing techniques
        (@see InstanceManager::InstancingTechnique)
        OGRE wasn't truly thought for instancing. OGRE assumes that either:
            a. One MovableObject -> No Renderable
            b. One MovableObject -> One Renderable
            c. One MovableObject -> Many Renderable.
        However, instances work on reverse: Many MovableObject have the same Renderable.
        <b>Instancing is already difficult to cull by a CPU</b>, but the main drawback from this assumption
        is that it makes it even harder to take advantage from OGRE's culling capabilities
        (i.e. @see OctreeSceneManager)
    @par
        To workaround this problem, InstanceBatch updates on almost every frame,
        growing the bounding box to fit all instances that are not being culled individually.
        This helps by avoiding a huge bbox that may cover the whole scene, which decreases shadow
        quality considerably (as it is seen as large shadow receiver)
        Furthermore, if no individual instance is visible, the InstanceBatch switches it's visibility
        (@see MovableObject::setVisible) to avoid sending this Renderable to the GPU. This happens because
        even when no individual instance is visible, their merged bounding box may cause OGRE to think
        the batch is visible (i.e. the camera is looking between object A & B, but A & B aren't visible)
    @par
        <b>As it happens with instancing in general, all instanced entities from the same batch will share
        the same textures and materials</b>
    @par
        Each InstanceBatch preallocates a fixed amount of mInstancesPerBatch instances once it's been
        built (@see build, @see buildFrom).
        @see createInstancedEntity and @see removeInstancedEntity on how to retrieve those instances
        remove them from scene.
        Note that, on GPU side, removing an instance from scene doesn't save GPU cycles on what
        respects vertex shaders, but saves a little fillrate and pixel shaders; unless all instances
        are removed, which saves GPU.
        For more information, @see InstancedEntity
        For information on how Ogre manages multiple Instance batches, @see InstanceManager

    @remarks
        Design discussion webpage
    @author
        Matias N. Goldberg ("dark_sylinc")
    @version
        1.0
    */
    class _OgreExport InstanceBatch : public Renderable, public MovableObject
    {
    public:
        typedef vector<InstancedEntity*>::type  InstancedEntityVec;
        typedef vector<Vector4>::type           CustomParamsVec;
        typedef FastArray<InstancedEntity*>     InstancedEntityArray;

        enum SkeletalAnimationMode
        {
            SKELETONS_NOT_SUPPORTED,
            SKELETONS_SUPPORTED,
            SKELETONS_LUT
        };

    protected:
        RenderOperation     mRenderOperation;
        size_t              mInstancesPerBatch;

        InstanceManager     *mCreator;
        ObjectMemoryManager mLocalObjectMemoryManager; ///Only one render queue is used

        MeshPtr             mMeshReference;
        Mesh::IndexMap const *mIndexToBoneMap;

        //InstancedEntities are all allocated at build time and kept as "unused"
        //when they're requested, they're removed from there when requested,
        //and put back again when they're no longer needed
        //Note each InstancedEntity has a unique ID ranging from [0; mInstancesPerBatch)
        InstancedEntityVec  mInstancedEntities;
        InstancedEntityVec  mUnusedEntities;

#ifdef OGRE_LEGACY_ANIMATIONS
        /** This variable may change after we refactor animations. In the meantime: some
            techniques animate all entities, some techniques don't animate anything
            (eg. HW Basic) and other techniques animate a few (eg. HW VTF LUT)
        */
        InstancedEntityArray mAnimatedEntities;
#endif

        ///@see InstanceManager::setNumCustomParams(). Because this may not even be used,
        ///our implementations keep the params separate from the InstancedEntity to lower
        ///the memory overhead. They default to Vector4::ZERO
        CustomParamsVec     mCustomParams;

        SkeletalAnimationMode   mTechnSupportsSkeletal;

        /// Cached distance to last camera for getSquaredViewDepth
        mutable Real mCachedCameraDist;
        /// The camera for which the cached distance is valid
        mutable const Camera *mCachedCamera;

        /// Tells that the list of entity instances with shared transforms has changed
        bool mTransformSharingDirty;

        bool mStaticDirty;

        vector<Aabb>::type      mThreadAabbs;
        MovableObjectArray      mCulledInstances; /// Only for HW Basic & HW VTF

        /// When true remove the memory of the VertexData we've created because no one else will
        bool mRemoveOwnVertexData;
        /// When true remove the memory of the IndexData we've created because no one else will
        bool mRemoveOwnIndexData;

        virtual void setupVertices( const SubMesh* baseSubMesh ) = 0;
        virtual void setupIndices( const SubMesh* baseSubMesh ) = 0;
        virtual void createAllInstancedEntities(void);
        virtual void deleteAllInstancedEntities(void);
        virtual void deleteUnusedInstancedEntities(void);
        /// Creates a new InstancedEntity instance
        virtual InstancedEntity* generateInstancedEntity(size_t num);

        /// Returns false on errors that would prevent building this batch from the given submesh
        virtual bool checkSubMeshCompatibility( const SubMesh* baseSubMesh );

        /** @see _defragmentBatch */
        void defragmentBatchNoCull( InstancedEntityVec &usedEntities, CustomParamsVec &usedParams );

        /** @see _defragmentBatch
            This one takes the entity closest to the minimum corner of the bbox, then starts
            gathering entities closest to this entity. There might be much better algorithms (i.e.
            involving space partition), but this one is simple and works well enough
        */
        void defragmentBatchDoCull( InstancedEntityVec &usedEntities, CustomParamsVec &usedParams );

        /** Used by HW Basic & HW VTF techniques to cull from multiple threads.
            @see InstancingTheadedCullingMethod
        */
        void instanceBatchCullFrustumThreadedImpl( const Camera *frustum, const Camera *lodCamera,
                                                   uint32 combinedVisibilityFlags );

    public:
        InstanceBatch( IdType id, ObjectMemoryManager *objectMemoryManager,
                       InstanceManager *creator, MeshPtr &meshReference, const MaterialPtr &material,
                       size_t instancesPerBatch, const Mesh::IndexMap *indexToBoneMap );
        virtual ~InstanceBatch();

        MeshPtr& _getMeshRef() { return mMeshReference; }

        /** Raises an exception if trying to change it after being built
        */
        void _setInstancesPerBatch( size_t instancesPerBatch );

        const Mesh::IndexMap* _getIndexToBoneMap() const { return mIndexToBoneMap; }

        /** Returns true if this technique supports skeletal animation
        @remarks
            A virtual function could have been used, but using a simple variable overridden
            by the derived class is faster than virtual call overhead. And both are clean
            ways of implementing it.
        */
        SkeletalAnimationMode _supportsSkeletalAnimation() const { return mTechnSupportsSkeletal; }

#ifdef OGRE_LEGACY_ANIMATIONS
        /// Updates animations from all our entities.
        void _updateAnimations(void);
#endif

        /** Updates the bounds of only our entities from multiple threads. To be called before
            _updateBounds (which is single threaded). @see InstanceManager::updateDirtyBatches
        @param threadIdx
            The index of this thread, must be unique for each thread
        */
        void _updateEntitiesBoundsThread( size_t threadIdx );

        /** @see InstanceManager::updateDirtyBatches */
        void _updateBounds(void);

        /** Some techniques have a limit on how many instances can be done.
            Sometimes even depends on the material being used.
        @par
            Note this is a helper function, as such it takes a submesh base to compute
            the parameters, instead of using the object's own. This allows
            querying for a technique without requiering to actually build it.
        @param baseSubMesh The base submesh that will be using to build it.
        @param flags Flags to pass to the InstanceManager. @see InstanceManagerFlags
        @return The max instances limit
        */
        virtual size_t calculateMaxNumInstances( const SubMesh *baseSubMesh, uint16 flags ) const = 0;

        /** Constructs all the data needed to use this batch, as well as the
            InstanceEntities. Placed here because in the constructor virtual
            tables may not have been yet filled.
        @param baseSubMesh A sub mesh which the instances will be based upon from.
        @remarks
            Call this only ONCE. This is done automatically by Ogre::InstanceManager
            Caller is responsable for freeing buffers in this RenderOperation
            Buffers inside the RenderOp may be null if the built failed.
        @return
            A render operation which is very useful to pass to other InstanceBatches
            (@see buildFrom) so that they share the same vertex buffers and indices,
            when possible
        */
        virtual RenderOperation build( const SubMesh* baseSubMesh );

        /** Instancing consumes significantly more GPU memory than regular rendering
            methods. However, multiple batches can share most, if not all, of the
            vertex & index buffers to save memory.
            Derived classes are free to overload this method to manipulate what to
            reference from Render Op.
            For example, Hardware based instancing uses it's own vertex buffer for the
            last source binding, but shares the other sources.
        @param renderOperation The RenderOp to reference.
        @remarks
            Caller is responsable for freeing buffers passed as input arguments
            This function replaces the need to call build()
        */
        virtual void buildFrom( const SubMesh *baseSubMesh, const RenderOperation &renderOperation );

        const Ogre::v1::MeshPtr& _getMeshReference(void) const { return mMeshReference; }

        /** @return true if it can not create more InstancedEntities
            (Num InstancedEntities == mInstancesPerBatch)
        */
        bool isBatchFull(void) const { return mUnusedEntities.empty(); }

        /** Returns true if it no instanced entity has been requested or all of them have been removed
        */
        bool isBatchUnused(void) const { return mUnusedEntities.size() == mInstancedEntities.size(); }

        /** Fills the input vector with the instances that are currently being used or were requested.
            Used for defragmentation, @see InstanceManager::defragmentBatches
        */
        void getInstancedEntitiesInUse( InstancedEntityVec &outEntities, CustomParamsVec &outParams );

#ifdef OGRE_LEGACY_ANIMATIONS
        /// Schedules the given Instanced Entity to be updated every frame in @see _updateAnimations
        void _addAnimatedInstance( InstancedEntity *instancedEntity );

        /** Removes an instanced already scheduled for animation update. @See _updateAnimations
        @remarks
            Does nothing if the animation wasn't already added.
        */
        void _removeAnimatedInstance( const InstancedEntity *instancedEntity );
#endif

        /** @see InstanceManager::defragmentBatches
            This function takes InstancedEntities and pushes back all entities it can fit here
            Extra entities in mUnusedEntities are destroyed
            (so that used + unused = mInstancedEntities.size())
        @param optimizeCulling true will call the DoCull version, false the NoCull
        @param usedEntities Array of InstancedEntities to parent with this batch. Those reparented
            are removed from this input vector
        @param usedParams Array of Custom parameters correlated with the InstancedEntities in usedEntities.
            They follow the fate of the entities in that vector.
        @remarks:
            This function assumes caller holds data to mInstancedEntities! Otherwise
            you can get memory leaks. Don't call this directly if you don't know what you're doing!
        */
        void _defragmentBatch( bool optimizeCulling, InstancedEntityVec &usedEntities,
                                CustomParamsVec &usedParams );

        /** @see InstanceManager::_defragmentBatchDiscard
            Destroys unused entities and clears the mInstancedEntity container which avoids leaving
            dangling pointers from reparented InstancedEntities
            Usually called before deleting this pointer. Don't call directly!
        */
        void _defragmentBatchDiscard(void);

        /** Tells this batch to stop updating animations, positions, rotations, and display
            all it's active instances. Some implementations allow to keep culling individual
            instances while others may not.
            This option makes the batch behave pretty much like Static Geometry, plust the GPU RAM
            memory advantages (less VRAM, less bandwidth) but no LOD support. Very useful for
            billboards of trees, repeating vegetation, modular buildings, etc.
            @remarks
                When individual culling is disabled (or not supported) This function moves a lot of
                processing time from the CPU to the GPU. If the GPU is already a bottleneck,
                you may see a decrease in performance instead!
                @See updateStaticDirty if you've made a change to an InstancedEntity and wish
                that change to take effect. Be sure to call this after you've set all your instances
                and not once per change.
        */
        bool setStatic( bool bStatic );

        /** Called by InstancedEntity(s) or directly to tell us we need to update the bounds
            Should only useful if this batch is static.
        */
        virtual void _notifyStaticDirty(void);

        /** Returns a pointer to a new InstancedEntity ready to use
            Note it's actually preallocated, so no memory allocation happens at
            this point.
            @remarks
                Returns NULL if all instances are being used
        */
        InstancedEntity* createInstancedEntity();

        /** Removes an InstancedEntity from the scene retrieved with
            getNewInstancedEntity, putting back into a queue
            @remarks
                Throws an exception if the instanced entity wasn't created by this batch
                Removed instanced entities save little CPU time, but _not_ GPU
        */
        void removeInstancedEntity( InstancedEntity *instancedEntity );

#ifdef OGRE_LEGACY_ANIMATIONS
        /** Tells whether world bone matrices need to be calculated.
            This does not include bone matrices which are calculated regardless
        */
        bool useBoneWorldMatrices() const { return mTechnSupportsSkeletal != SKELETONS_LUT; }
#endif

        /** Tells that the list of entity instances with shared transforms has changed */
        void _markTransformSharingDirty() { mTransformSharingDirty = true; }

        /** @see InstancedEntity::setCustomParam */
        void _setCustomParam( InstancedEntity *instancedEntity, unsigned char idx, const Vector4 &newParam );

        /** @see InstancedEntity::getCustomParam */
        const Vector4& _getCustomParam( InstancedEntity *instancedEntity, unsigned char idx );

        //Renderable overloads
        /** @copydoc Renderable::getRenderOperation. */
        void getRenderOperation( RenderOperation& op, bool casterPass ) { op = mRenderOperation; }

        /** @copydoc Renderable::getSquaredViewDepth. */
        Real getSquaredViewDepth( const Camera* cam ) const;
        /** @copydoc Renderable::getLights. */
        const LightList& getLights( void ) const;

        /** @copydoc MovableObject::getMovableType. */
        const String& getMovableType(void) const;

        // resolve ambiguity of get/setUserAny due to inheriting from Renderable and MovableObject
        using Renderable::getUserAny;
        using Renderable::setUserAny;
    };
}
} // namespace Ogre

#include "OgreHeaderSuffix.h"

#endif // __InstanceBatch_H__
