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
#ifndef __InstancedEntity_H__
#define __InstancedEntity_H__

#include "OgreMovableObject.h"
#include "OgreNode.h"
#include "OgreDualQuaternion.h"
#include "Animation/OgreSkeletonInstance.h"
#include "OgreHeaderPrefix.h"

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

    /** @see InstanceBatch to understand how instancing works.

        Instanced entities work in a very similar way as how an Entity works, as this interface
        tries to mimic it as most as possible to make the transition between Entity and InstancedEntity
        as straightforward and simple as possible.
        There are a couple inherent limitations, for example setRenderQueueGroup only works on
        the InstanceBatch level, not the individual instance. This limits Z sorting for alpha blending
        quite significantly
        An InstancedEntity won't appear in scene until a SceneNode is attached to them. Once the
        InstancedEntity is no longer needed, call InstanceBatch::removeInstancedEntity to put them
        back into a queue so the batch can return it back again when a new instance is requested.
        @par
        Internally, instanced entities that are not visible (i.e. by setting setVisible(false) or
        when they don't have a SceneNode attached to them) a Zero world matrix is sent to the vertex shader
        which in turn causes a zero area triangle.
        This obviously means no Vertex shader benefit, but saves a bit of pixel shader processing power.
        Also this means this object won't be considered when sizing the InstanceBatch's bounding box.
        @par
        Each instance has an instance ID unique within the batch, which matches the ID sent to the vertex
        shader by some techniques (like InstanceBatchShader).
        @par
        Differences between Entity and InstancedEntity:
        * Setting RenderQueueGroup and other Renderable stuff works at InstanceBatch level, not
        InstancedEntity. This is obviously a limitation from instancing in general, not this particular
        implementation

        @remarks
            Design discussion webpage
        @author
            Matias N. Goldberg ("dark_sylinc")
        @version
            1.0
     */
    class _OgreExport InstancedEntity : public Ogre::MovableObject
    {
        friend class InstanceBatch;
        friend class InstanceBatchShader;
        friend class InstanceBatchHW;
        friend class InstanceBatchHW_VTF;
        friend class BaseInstanceBatchVTF;
    protected:
        uint16 mInstanceId; //Note it may change after defragmenting!
        bool mInUse;
        InstanceBatch *mBatchOwner;

#ifdef OGRE_LEGACY_ANIMATIONS
        AnimationStateSet *mAnimationState;
        OldSkeletonInstance *mSkeletonInstance;
        Matrix4 *mBoneMatrices;  //Local space
        Matrix4 *mBoneWorldMatrices; //World space
        unsigned long mFrameAnimationLastUpdated;
#else
        SkeletonInstance    *mSkeletonInstance;
        BoneMemoryManager   *mBoneMemoryManager;
#endif

        InstancedEntity* mSharedTransformEntity;    //When not null, another InstancedEntity controls the skeleton
                                                
        /** Used in conjunction with bone matrix lookup. Tells the number of the transform for
            as arranged in the vertex texture */
        uint16 mTransformLookupNumber;

        /// Stores the master when we're the slave, store our slaves when we're the master
        typedef vector<InstancedEntity*>::type InstancedEntityVec;
        InstancedEntityVec mSharingPartners;

        /// Returns number of matrices written to transform, assumes transform has enough space
        size_t getTransforms( Matrix4 *xform ) const;
        /// Returns number of 32-bit values written
        size_t getTransforms3x4( float *xform ) const;

        /** Fills xform with the 4x3 world matrix (12 bytes)
        @remarks
            Assumes this object is attached to a Node
        */
        FORCEINLINE void writeSingleTransform3x4( float * RESTRICT_ALIAS xform ) const;

        /** Fills xform with 4x3 world matrices from skeletal animation (12 bytes each)
        @remarks
            Number of bytes written to xform is 12 * number of matrices
        @param xform
            The pointer to store the matrices
        @param boneIdxStart
            Iterator to the first bone index map (@See mIndexToBoneMap)
        @param boneIdxEnd
            Iterator to the last bone index map (@See mIndexToBoneMap)
        */
        FORCEINLINE void writeAnimatedTransform3x4( float * RESTRICT_ALIAS xform,
                                                    Mesh::IndexMap::const_iterator boneIdxStart,
                                                    Mesh::IndexMap::const_iterator boneIdxEnd ) const;

        /** Fills xform with 4x3 world matrices from skeletal animation (12 bytes each, LUT)
        @remarks
            Number of bytes written to xform is 12 * number of matrices
        @param xform
            The pointer to store the matrices
        @param boneIdxStart
            Iterator to the first bone index map (@See mIndexToBoneMap)
        @param boneIdxEnd
            Iterator to the last bone index map (@See mIndexToBoneMap)
        */
        FORCEINLINE void writeLutTransform3x4( float * RESTRICT_ALIAS xform,
                                                    Mesh::IndexMap::const_iterator boneIdxStart,
                                                    Mesh::IndexMap::const_iterator boneIdxEnd ) const;

        /** Fills xform with Dual Quaternion matrices from skeletal animation (32 bytes each)
        @remarks
            Number of bytes written to xform is 32 * number of matrices
        @param xform
            The pointer to store the matrices
        @param boneIdxStart
            Iterator to the first bone index map (@See mIndexToBoneMap)
        @param boneIdxEnd
            Iterator to the last bone index map (@See mIndexToBoneMap)
        */
        FORCEINLINE void writeDualQuatTransform( float * RESTRICT_ALIAS xform,
                                                    Mesh::IndexMap::const_iterator boneIdxStart,
                                                    Mesh::IndexMap::const_iterator boneIdxEnd ) const;

        /// Returns true if this InstancedObject is visible to the current camera
        bool findVisible( Camera *camera ) const;

        /// Creates/destroys our own skeleton, also tells slaves to unlink if we're destroying
        void createSkeletonInstance();
        void destroySkeletonInstance();

        /// When this entity is a slave, stopSharingTransform delegates to this function.
        /// nofityMaster = false is used to prevent iterator invalidation in specific cases.
        void stopSharingTransformAsSlave( bool notifyMaster );

        /// Just unlinks, and tells our master we're no longer sharing
        void unlinkTransform( bool notifyMaster=true );

        /// Called when a slave has unlinked from us
        void notifyUnlink( const InstancedEntity *slave );

        /** Sets whether the entity is in use. */
        void setInUse(bool used);

    public:
        InstancedEntity( IdType id, ObjectMemoryManager *objectMemoryManager, InstanceBatch *batchOwner,
                         uint32 instanceID,
                 #ifndef OGRE_LEGACY_ANIMATIONS
                         BoneMemoryManager *boneMemoryManager,
                 #endif
                         InstancedEntity* sharedTransformEntity = NULL );
        virtual ~InstancedEntity();

        /** Shares the entire transformation with another InstancedEntity. This is useful when a mesh
            has more than one submeshes, therefore creating multiple InstanceManagers (one for each
            submesh). With this function, sharing makes the skeleton to be shared (less memory) and
            updated once (performance optimization).
            Note that one InstancedEntity (i.e. submesh 0) must be chosen as "master" which will share
            with the other instanced entities (i.e. submeshes 1-N) which are called "slaves"
            @par
            Requirements to share trasnformations:
                * Both InstancedEntities must have use the same skeleton
                * An InstancedEntity can't be both "master" and "slave" at the same time
            @remarks
            Sharing does nothing if the original mesh doesn't have a skeleton
            When an InstancedEntity is removed (@see InstanceBatch::removeInstancedEntity), it stops
            sharing the transform. If the instanced entity was the master one, all it's slaves stop
            sharing and start having their own transform too.
            @param slave The InstancedEntity that should share with us and become our slave
            @return true if successfully shared (may fail if they aren't skeletally animated)
        */
        bool shareTransformWith( InstancedEntity *slave );

        /** @see shareTransformWith
            Stops sharing the transform if this is a slave, and notifies the master we're no longer
            a slave.
            If this is a master, tells all it's slave to stop sharing
            @remarks
            This function is automatically called in InstanceBatch::removeInstancedEntity
        */
        void stopSharingTransform();

        InstanceBatch* _getOwner() const { return mBatchOwner; }

        const String& getMovableType(void) const;

        const AxisAlignedBox& getBoundingBox(void) const;

        /** This is used by our batch owner to get the closest entity's depth, returns infinity
            when not attached to a scene node */
        Real getSquaredViewDepth( const Camera* cam ) const;

        /// @copydoc MovableObject::_notifyStaticDirty
        virtual void _notifyStaticDirty(void) const;

        /// Overloaded so we can register ourselves for updating our animations
        virtual void _notifyAttached( Node* parent );

#ifndef OGRE_LEGACY_ANIMATIONS
        virtual void _notifyParentNodeMemoryChanged(void);
#endif

        /// Do nothing, InstanceBatch takes care of this.
        void _updateRenderQueue( RenderQueue* queue, Camera *camera, const Camera *lodCamera )  {}

        /** @see Entity::hasSkeleton */
        bool hasSkeleton(void) const { return mSkeletonInstance != 0; }

#ifdef OGRE_LEGACY_ANIMATIONS
        /** @see Entity::getSkeleton */
        OldSkeletonInstance* getSkeleton(void) const { return mSkeletonInstance; }
#else
        SkeletonInstance* getSkeleton(void) const { return mSkeletonInstance; }
#endif

#ifdef OGRE_LEGACY_ANIMATIONS
        /** @see Entity::getAnimationState */
        AnimationState* getAnimationState(const String& name) const;
        /** @see Entity::getAllAnimationStates */
        AnimationStateSet* getAllAnimationStates(void) const;

        /** Called by InstanceBatch in <i>his</i> _updateRenderQueue to tell us we need
            to calculate our bone matrices.
            @remarks Assumes it has a skeleton (mSkeletonInstance != 0)
            @return true if something was actually updated
        */
        bool _updateAnimation(void);
#endif

        /** Sets the transformation look up number */
        void setTransformLookupNumber(uint16 num) { mTransformLookupNumber = num;}

        /** Tells if the entity is in use. */
        bool isInUse() const { return mInUse; }

        /** @copydoc MovableObject::isInScene. */
        virtual bool isInScene(void) const
        {
            //We assume that the instanced entity is in the scene if it is in use
            //It is in the scene whether it has a parent node or not
            return mInUse;
        }

        /** Sets the custom parameter for this instance @see InstanceManager::setNumCustomParams
            Because not all techniques support custom params, and some users may not need it while
            using millions of InstancedEntities, the params have been detached from InstancedEntity
            and stored in it's InstanceBatch instead, to reduce memory overhead.
        @remarks
            If this function is never called, all instances default to Vector4::ZERO. Watch out!
            If you destroy an instanced entity and then create it again (remember! Instanced entities
            are pre-allocated) it's custom param will contain the old value when it was destroyed.
        @param idx of the param. In the range [0; InstanceManager::getNumCustomParams())
        @param newParam New parameter
        */
        void setCustomParam( unsigned char idx, const Vector4 &newParam );
        const Vector4& getCustomParam( unsigned char idx );
    };
}
}

#include "OgreInstancedEntity.inl"

#include "OgreHeaderSuffix.h"

#endif
