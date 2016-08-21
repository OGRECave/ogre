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
#include "OgreStableHeaders.h"
#include "OgreEntity.h"

#include "OgreMeshManager.h"
#include "OgreSubMesh.h"
#include "OgreSubEntity.h"
#include "OgreException.h"
#include "OgreSceneManager.h"
#include "OgreLogManager.h"
#include "OgreSkeleton.h"
#include "OgreOldBone.h"
#include "OgreCamera.h"
#include "OgreTagPoint.h"
#include "OgreAxisAlignedBox.h"
#include "OgreHardwareBufferManager.h"
#include "OgreVector4.h"
#include "OgreRoot.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreOldSkeletonInstance.h"
#include "OgreOptimisedUtil.h"
#include "OgreSceneNode.h"
#include "OgreLodStrategy.h"
#include "OgreLodListener.h"
#include "OgreMaterialManager.h"
#include "OgreHlmsManager.h"

namespace Ogre {
namespace v1 {
    extern const FastArray<Real> c_DefaultLodMesh;
    //-----------------------------------------------------------------------
    Entity::Entity ( IdType id, ObjectMemoryManager *objectMemoryManager, SceneManager *manager )
        : MovableObject( id, objectMemoryManager, manager, 1 ),
          mAnimationState(NULL),
          mSkelAnimVertexData(0),
          mTempVertexAnimInfo(),
          mSoftwareVertexAnimVertexData(0),
          mHardwareVertexAnimVertexData(0),
          mVertexAnimationAppliedThisFrame(false),
          mPreparedForShadowVolumes(false),
          mBoneWorldMatrices(NULL),
          mBoneMatrices(NULL),
          mNumBoneMatrices(0),
          mFrameAnimationLastUpdated(std::numeric_limits<unsigned long>::max()),
          mFrameBonesLastUpdated(NULL),
          mSharedSkeletonEntities(NULL),
          mDisplaySkeleton(false),
          mCurrentHWAnimationState(false),
          mHardwarePoseCount(0),
          mVertexProgramInUse(false),
          mSoftwareAnimationRequests(0),
          mSoftwareAnimationNormalsRequests(0),
          mSkipAnimStateUpdates(false),
          mAlwaysUpdateMainSkeleton(false),
          mSkeletonInstance(0),
          mInitialised(false),
          mLastParentXform(Matrix4::ZERO),
          mMeshStateCount(0)
    {
        mObjectData.mQueryFlags[mObjectData.mIndex] = SceneManager::QUERY_ENTITY_DEFAULT_MASK;
    }
    //-----------------------------------------------------------------------
    Entity::Entity( IdType id, ObjectMemoryManager *objectMemoryManager, SceneManager *manager,
                    const MeshPtr& mesh ) :
        MovableObject(id, objectMemoryManager, manager, 1),
        mMesh(mesh),
        mAnimationState(NULL),
        mSkelAnimVertexData(0),
        mSoftwareVertexAnimVertexData(0),
        mHardwareVertexAnimVertexData(0),
        mPreparedForShadowVolumes(false),
        mBoneWorldMatrices(NULL),
        mBoneMatrices(NULL),
        mNumBoneMatrices(0),
        mFrameAnimationLastUpdated(std::numeric_limits<unsigned long>::max()),
        mFrameBonesLastUpdated(NULL),
        mSharedSkeletonEntities(NULL),
        mDisplaySkeleton(false),
        mCurrentHWAnimationState(false),
        mVertexProgramInUse(false),
        mSoftwareAnimationRequests(0),
        mSoftwareAnimationNormalsRequests(0),
        mSkipAnimStateUpdates(false),
        mAlwaysUpdateMainSkeleton(false),
        mSkeletonInstance(0),
        mInitialised(false),
        mLastParentXform(Matrix4::ZERO),
        mMeshStateCount(0)
    {
        _initialise();
        mObjectData.mQueryFlags[mObjectData.mIndex] = SceneManager::QUERY_ENTITY_DEFAULT_MASK;
    }
    //-----------------------------------------------------------------------
    void Entity::_initialise(bool forceReinitialise)
    {
        if (forceReinitialise)
            _deinitialise();

        if (mInitialised)
            return;

        if (mMesh->isBackgroundLoaded() && !mMesh->isLoaded())
        {
            // register for a callback when mesh is finished loading
            // do this before asking for load to happen to avoid race
            mMesh->addListener(this);
        }
        
        // On-demand load
        mMesh->load();
        // If loading failed, or deferred loading isn't done yet, defer
        // Will get a callback in the case of deferred loading
        // Skeletons are cascade-loaded so no issues there
        if (!mMesh->isLoaded())
            return;

        // Is mesh skeletally animated?
        if (mMesh->hasSkeleton() && !mMesh->getOldSkeleton().isNull())
        {
            mSkeletonInstance = OGRE_NEW OldSkeletonInstance(mMesh->getOldSkeleton());
            mSkeletonInstance->load();
        }

        mLodMesh = mMesh->_getLodValueArray();

        // Build main subentity list
        buildSubEntityList(mMesh, &mSubEntityList);

        {
            //Without filling the renderables list, the RenderQueue won't
            //catch our sub entities and thus we won't be rendered
            mRenderables.reserve( mSubEntityList.size() );
            SubEntityList::iterator itor = mSubEntityList.begin();
            SubEntityList::iterator end  = mSubEntityList.end();
            while( itor != end )
            {
                mRenderables.push_back( &(*itor) );
                ++itor;
            }
        }

        // Check if mesh is using manual LOD
        if (mMesh->hasManualLodLevel())
        {
            ushort i, numLod;
            numLod = mMesh->getNumLodLevels();
            // NB skip LOD 0 which is the original
            for (i = 1; i < numLod; ++i)
            {
                const MeshLodUsage& usage = mMesh->getLodLevel(i);
                // Manually create entity
                Entity* lodEnt = OGRE_NEW Entity( Id::generateNewId<MovableObject>(),
                                                  mObjectMemoryManager, mManager,
                                                  usage.manualMesh );
                lodEnt->setName( mName + "Lod" + StringConverter::toString(i) );
                mLodEntityList.push_back(lodEnt);
            }
        }

        // Initialise the AnimationState, if Mesh has animation
        if (hasSkeleton())
        {
            mFrameBonesLastUpdated = OGRE_NEW_T(unsigned long, MEMCATEGORY_ANIMATION)(std::numeric_limits<unsigned long>::max());
            mNumBoneMatrices = mSkeletonInstance->getNumBones();
            mBoneMatrices = static_cast<Matrix4*>(OGRE_MALLOC_SIMD(sizeof(Matrix4) * mNumBoneMatrices, MEMCATEGORY_ANIMATION));
        }
        if (hasSkeleton() || hasVertexAnimation())
        {
            mAnimationState = OGRE_NEW AnimationStateSet();
            mMesh->_initAnimationState(mAnimationState);
            prepareTempBlendBuffers();
        }

        reevaluateVertexProcessing();

        HlmsManager *hlmsManager = Root::getSingleton().getHlmsManager();
        size_t numSubMeshes = mMesh->getNumSubMeshes();
        for( size_t i=0; i<numSubMeshes; ++i )
        {
            SubMesh *subMesh = mMesh->getSubMesh(i);
            if( subMesh->isMatInitialised() )
            {
                //Give preference to HLMS materials of the same name
                HlmsDatablock *datablock = hlmsManager->getDatablockNoDefault(
                                                    subMesh->getMaterialName() );
                if( datablock )
                {
                    mSubEntityList[i].setDatablock( datablock );
                }
                else
                {
                    mSubEntityList[i].setMaterialName( subMesh->getMaterialName(), mMesh->getGroup() );
                }
            }
        }

        Aabb aabb;
        if( mMesh->getBounds().isInfinite() )
            aabb = Aabb::BOX_INFINITE;
        else if( mMesh->getBounds().isNull() )
            aabb = Aabb::BOX_NULL;
        else
            aabb = Aabb( mMesh->getBounds().getCenter(), mMesh->getBounds().getHalfSize() );
        mObjectData.mLocalAabb->setFromAabb( aabb, mObjectData.mIndex );
        mObjectData.mWorldAabb->setFromAabb( aabb, mObjectData.mIndex );
        mObjectData.mLocalRadius[mObjectData.mIndex] = aabb.getRadius();
        mObjectData.mWorldRadius[mObjectData.mIndex] = aabb.getRadius();

        mInitialised = true;
        mMeshStateCount = mMesh->getStateCount();
    }
    //-----------------------------------------------------------------------
    void Entity::_deinitialise(void)
    {
        if (!mInitialised)
            return;

        // Delete submeshes
        mSubEntityList.clear();
        mRenderables.clear();
        
        // Delete LOD entities
        LODEntityList::iterator li, liend;
        liend = mLodEntityList.end();
        for (li = mLodEntityList.begin(); li != liend; ++li)
        {
            // Delete
            OGRE_DELETE *li;
            *li = 0;
        }
        mLodEntityList.clear();
        
        // Detach all child objects, do this manually to avoid needUpdate() call
        // which can fail because of deleted items
#ifdef ENABLE_INCOMPATIBLE_OGRE_2_0
        detachAllObjectsImpl();
#endif

        if (mSkeletonInstance) {
            OGRE_FREE_SIMD(mBoneWorldMatrices, MEMCATEGORY_ANIMATION);
            mBoneWorldMatrices = 0;

            if (mSharedSkeletonEntities) {
                mSharedSkeletonEntities->erase(this);
                if (mSharedSkeletonEntities->size() == 1)
                {
                    (*mSharedSkeletonEntities->begin())->stopSharingSkeletonInstance();
                }
                // Should never occur, just in case
                else if (mSharedSkeletonEntities->empty())
                {
                    OGRE_DELETE_T(mSharedSkeletonEntities, EntitySet, MEMCATEGORY_ANIMATION); mSharedSkeletonEntities = 0;
                    // using OGRE_FREE since unsigned long is not a destructor
                    OGRE_FREE(mFrameBonesLastUpdated, MEMCATEGORY_ANIMATION); mFrameBonesLastUpdated = 0;
                    OGRE_DELETE mSkeletonInstance; mSkeletonInstance = 0;
                    OGRE_FREE_SIMD(mBoneMatrices, MEMCATEGORY_ANIMATION); mBoneMatrices = 0;
                    OGRE_DELETE mAnimationState; mAnimationState = 0;
                }
            } else {
                // using OGRE_FREE since unsigned long is not a destructor
                OGRE_FREE(mFrameBonesLastUpdated, MEMCATEGORY_ANIMATION); mFrameBonesLastUpdated = 0;
                OGRE_DELETE mSkeletonInstance; mSkeletonInstance = 0;
                OGRE_FREE_SIMD(mBoneMatrices, MEMCATEGORY_ANIMATION); mBoneMatrices = 0;
                OGRE_DELETE mAnimationState; mAnimationState = 0;
            }
        }
        else
        {
            //Non-skeletally animated objects don't share the mAnimationState. Always delete.
            //See https://ogre3d.atlassian.net/browse/OGRE-504
            OGRE_DELETE mAnimationState;
            mAnimationState = 0;
        }

        OGRE_DELETE mSkelAnimVertexData; mSkelAnimVertexData = 0;
        OGRE_DELETE mSoftwareVertexAnimVertexData; mSoftwareVertexAnimVertexData = 0;
        OGRE_DELETE mHardwareVertexAnimVertexData; mHardwareVertexAnimVertexData = 0;

        mInitialised = false;
    }
    //-----------------------------------------------------------------------
    Entity::~Entity()
    {
        _deinitialise();
        // Unregister our listener
        mMesh->removeListener(this);
    }
    //-----------------------------------------------------------------------
    bool Entity::hasVertexAnimation(void) const
    {
        return mMesh->hasVertexAnimation();
    }
    //-----------------------------------------------------------------------
    const MeshPtr& Entity::getMesh(void) const
    {
        return mMesh;
    }
    //-----------------------------------------------------------------------
    SubEntity* Entity::getSubEntity(size_t index)
    {
        if (index >= mSubEntityList.size())
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Index out of bounds.",
                        "Entity::getSubEntity");
        return &mSubEntityList[index];
    }
    //-----------------------------------------------------------------------
    const SubEntity* Entity::getSubEntity(size_t index) const
    {
        if (index >= mSubEntityList.size())
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
            "Index out of bounds.",
            "Entity::getSubEntity");
        return &mSubEntityList[index];
    }
    //-----------------------------------------------------------------------
    SubEntity* Entity::getSubEntity(const String& name)
    {
        size_t index = mMesh->_getSubMeshIndex(name);
        return getSubEntity(index);
    }
    //-----------------------------------------------------------------------
    const SubEntity* Entity::getSubEntity(const String& name) const
    {
        size_t index = mMesh->_getSubMeshIndex(name);
        return getSubEntity(index);
    }
    //-----------------------------------------------------------------------
    size_t Entity::getNumSubEntities(void) const
    {
        return mSubEntityList.size();
    }
    //-----------------------------------------------------------------------
    void Entity::setDatablock( HlmsDatablock *datablock )
    {
        SubEntityList::iterator itor = mSubEntityList.begin();
        SubEntityList::iterator end  = mSubEntityList.end();

        while( itor != end )
        {
            itor->setDatablock( datablock );
            ++itor;
        }
    }
    //-----------------------------------------------------------------------
    void Entity::setDatablock( IdString datablockName )
    {
        HlmsManager *hlmsManager = Root::getSingleton().getHlmsManager();
        HlmsDatablock *datablock = hlmsManager->getDatablock( datablockName );

        setDatablock( datablock );
    }
    //-----------------------------------------------------------------------
    Entity* Entity::clone(void) const
    {
        if (!mManager)
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                        "Cannot clone an Entity that wasn't created through a "
                        "SceneManager", "Entity::clone");
        }
        Entity* newEnt = mManager->createEntity( getMesh(),
                                                 mObjectMemoryManager->getMemoryManagerType() );

        if (mInitialised)
        {
            // Copy material settings
            SubEntityList::const_iterator i;
            unsigned int n = 0;
            for (i = mSubEntityList.begin(); i != mSubEntityList.end(); ++i, ++n)
            {
                newEnt->getSubEntity(n)->setDatablock( i->getDatablock() );
            }
            if (mAnimationState)
            {
                OGRE_DELETE newEnt->mAnimationState;
                newEnt->mAnimationState = OGRE_NEW AnimationStateSet(*mAnimationState);
            }
        }

        return newEnt;
    }
    //-----------------------------------------------------------------------
    void Entity::setDatablockOrMaterialName( const String& name,
                                             const String& groupName /* = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME */)
    {
        // Set for all subentities
        SubEntityList::iterator i;
        for (i = mSubEntityList.begin(); i != mSubEntityList.end(); ++i)
        {
            i->setDatablockOrMaterialName(name, groupName);
        }
    }
    //-----------------------------------------------------------------------
    void Entity::setMaterialName( const String& name, const String& groupName /* = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME */)
    {
        // Set for all subentities
        SubEntityList::iterator i;
        for (i = mSubEntityList.begin(); i != mSubEntityList.end(); ++i)
        {
            i->setMaterialName(name, groupName);
        }

    }
    //-----------------------------------------------------------------------
    void Entity::setMaterial( const MaterialPtr& material )
    {
        // Set for all subentities
        SubEntityList::iterator i;
        for (i = mSubEntityList.begin(); i != mSubEntityList.end(); ++i)
        {
            i->setMaterial(material);
        }
    }
    //-----------------------------------------------------------------------
    void Entity::setRenderQueueSubGroup( uint8 subGroup )
    {
        // Set for all subentities
        SubEntityList::iterator i;
        for (i = mSubEntityList.begin(); i != mSubEntityList.end(); ++i)
            i->setRenderQueueSubGroup( subGroup );
#if !OGRE_NO_MESHLOD
        // Set render queue for all manual LOD entities
        if (mMesh->hasManualLodLevel())
        {
            LODEntityList::iterator li, liend;
            liend = mLodEntityList.end();
            for (li = mLodEntityList.begin(); li != liend; ++li)
            {
                if(*li != this)
                    (*li)->setRenderQueueSubGroup( subGroup );
            }
        }
#endif
    }
    //-----------------------------------------------------------------------
    void Entity::setUpdateBoundingBoxFromSkeleton(bool update)
    {
        mUpdateBoundingBoxFromSkeleton = update;
        if (mMesh->isLoaded() && mMesh->getBoneBoundingRadius() == Real(0))
        {
            mMesh->_computeBoneBoundingRadius();
        }
    }
    //-----------------------------------------------------------------------
#ifdef ENABLE_INCOMPATIBLE_OGRE_2_0
    void Entity::_notifyCurrentCamera(Camera* cam)
    {
        //TODO (dark_sylinc): This whole state tracking thing feels wrong.
        // Calculate the LOD
        if (mParentNode)
        {
            // Get mesh lod strategy
            const LodStrategy *meshStrategy = mMesh->getLodStrategy();
            // Get the appropriate LOD value
            Real lodValue = meshStrategy->getValue(this, cam);
            // Bias the LOD value
            Real biasedMeshLodValue = lodValue * mMeshLodFactorTransformed;


            // Get the index at this biased depth
            ushort newMeshLodIndex = mMesh->getLodIndex(biasedMeshLodValue);
            // Apply maximum detail restriction (remember lower = higher detail)
            newMeshLodIndex = std::max<ushort>(mMaxMeshLodIndex, newMeshLodIndex);
            // Apply minimum detail restriction (remember higher = lower detail)
            newMeshLodIndex = std::min<ushort>(mMinMeshLodIndex, newMeshLodIndex);

            // Construct event object
            EntityMeshLodChangedEvent evt;
            evt.entity = this;
            evt.camera = cam;
            evt.lodValue = biasedMeshLodValue;
            evt.previousLodIndex = mCurrentMeshLod;
            evt.newLodIndex = newMeshLodIndex;

            // Notify LOD event listeners
            cam->getSceneManager()->_notifyEntityMeshLodChanged(evt);

            // Change LOD index
            //mMeshLodIndex = evt.newLodIndex;

            // Now do material LOD
            lodValue *= mMaterialLodFactorTransformed;


            SubEntityList::iterator i, iend;
            iend = mSubEntityList.end();
            for (i = mSubEntityList.begin(); i != iend; ++i)
            {
                // Get sub-entity material
                const MaterialPtr& material = i->getMaterial();
                
                // Get material LOD strategy
                const LodStrategy *materialStrategy = material->getLodStrategy();
                
                // Recalculate LOD value if strategies do not match
                Real biasedMaterialLodValue;
                if (meshStrategy == materialStrategy)
                    biasedMaterialLodValue = lodValue;
                else
                    biasedMaterialLodValue = materialStrategy->getValue(this, cam) * materialStrategy->transformBias(mMaterialLodFactor);

                // Get the index at this biased depth
                unsigned short idx = material->getLodIndex(biasedMaterialLodValue);
                // Apply maximum detail restriction (remember lower = higher detail)
                idx = std::max(mMaxMaterialLodIndex, idx);
                // Apply minimum detail restriction (remember higher = lower detail)
                idx = std::min(mMinMaterialLodIndex, idx);

                // Construct event object
                EntityMaterialLodChangedEvent subEntEvt;
                subEntEvt.subEntity = &(*i);
                subEntEvt.camera = cam;
                subEntEvt.lodValue = biasedMaterialLodValue;
                subEntEvt.previousLodIndex = i->mMaterialLodIndex;
                subEntEvt.newLodIndex = idx;

                // Notify LOD event listeners
                cam->getSceneManager()->_notifyEntityMaterialLodChanged(subEntEvt);

                // Change LOD index
                i->mMaterialLodIndex = subEntEvt.newLodIndex;
                // Also invalidate any camera distance cache
                (*i)->_invalidateCameraCache ();
            }


        }
        // Notify any child objects
        ChildObjectList::iterator child_itr = mChildObjectList.begin();
        ChildObjectList::iterator child_itr_end = mChildObjectList.end();
        for( ; child_itr != child_itr_end; ++child_itr)
        {
            (*child_itr).second->_notifyCurrentCamera(cam);
        }
    }
#endif
    //-----------------------------------------------------------------------
    void Entity::_updateRenderQueue(RenderQueue* queue, Camera *camera, const Camera *lodCamera)
    {
        /*// Do nothing if not initialised yet
        if (!mInitialised)
            return;

        // Check mesh state count, will be incremented if reloaded
        if (mMesh->getStateCount() != mMeshStateCount)
        {
            // force reinitialise
            _initialise(true);
        }

        {
            FastArray<unsigned char>::const_iterator itCurrentMatLod = mCurrentMaterialLod.begin();
            SubEntityList::iterator itor = mSubEntityList.begin();
            SubEntityList::iterator end  = mSubEntityList.end();
            while( itor != end )
            {
                // Propagate Lod index to our sub entities (which is already calculated by now).
                itor->mMaterialLodIndex = *itCurrentMatLod++;
                ++itor;
            }
        }

        Entity* displayEntity = this;
        // Check we're not using a manual LOD
        if (mCurrentMeshLod > 0 && mMesh->hasManualLodLevel())
        {
            // Use alternate entity
            assert( static_cast< size_t >( mCurrentMeshLod - 1 ) < mLodEntityList.size() &&
                "No LOD EntityList - did you build the manual LODs after creating the entity?");
            // index - 1 as we skip index 0 (original LOD)
            if (hasSkeleton() && mLodEntityList[mCurrentMeshLod - 1]->hasSkeleton())
            {
                // Copy the animation state set to lod entity, we assume the lod
                // entity only has a subset animation states
                AnimationStateSet* targetState = mLodEntityList[mCurrentMeshLod - 1]->mAnimationState;
                if (mAnimationState != targetState) // only copy if LODs use different skeleton instances
                {
                    if (mAnimationState->getDirtyFrameNumber() != targetState->getDirtyFrameNumber()) // only copy if animation was updated
                        mAnimationState->copyMatchingState(targetState);
                }
            }
            displayEntity = mLodEntityList[mCurrentMeshLod - 1];
        }

        // Add each visible SubEntity to the queue
        SubEntityList::iterator i, iend;
        iend = displayEntity->mSubEntityList.end();
        for (i = displayEntity->mSubEntityList.begin(); i != iend; ++i)
        {
            //TODO: (dark_sylinc) send our mLightList in this function,
            //instead of overloading getLights
            queue->addRenderable( &(*i), mRenderQueueID, mRenderQueuePriority );
        }

        if (getAlwaysUpdateMainSkeleton() && hasSkeleton() && (mCurrentMeshLod > 0))
        {
            //check if an update was made
            if (cacheBoneMatrices())
            {
                getSkeleton()->_updateTransforms();
                //We will mark the skeleton as dirty. Otherwise, if in the same frame the entity will 
                //be rendered first with a low LOD and then with a high LOD the system wont know that
                //the bone matrices has changed and there for will not update the vertex buffers
                getSkeleton()->_notifyManualBonesDirty();
            }
        }

        // Since we know we're going to be rendered, take this opportunity to
        // update the animation
        if (displayEntity->hasSkeleton() || displayEntity->hasVertexAnimation())
        {
            displayEntity->updateAnimation();
        }

        // HACK to display bones
        // This won't work if the entity is not centered at the origin
        // TODO work out a way to allow bones to be rendered when Entity not centered
        if (mDisplaySkeleton && hasSkeleton())
        {
            int numBones = mSkeletonInstance->getNumBones();
            for (unsigned short b = 0; b < numBones; ++b)
            {
                OldBone* bone = mSkeletonInstance->getBone(b);
                queue->addRenderable(bone->getDebugRenderable(1), mRenderQueueID, mRenderQueuePriority);
            }
        }*/

        // Since we know we're going to be rendered, take this opportunity to
        // update the animation
        if (hasSkeleton() || hasVertexAnimation())
        {
            updateAnimation();
        }
    }
    //-----------------------------------------------------------------------
    AnimationState* Entity::getAnimationState(const String& name) const
    {
        if (!mAnimationState)
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Entity is not animated",
                        "Entity::getAnimationState");
        }

        return mAnimationState->getAnimationState(name);
    }
    //-----------------------------------------------------------------------
    bool Entity::hasAnimationState(const String& name) const
    {
        return mAnimationState && mAnimationState->hasAnimationState(name);
    }
    //-----------------------------------------------------------------------
    AnimationStateSet* Entity::getAllAnimationStates(void) const
    {
        return mAnimationState;
    }
    //-----------------------------------------------------------------------
    const String& Entity::getMovableType(void) const
    {
        return EntityFactory::FACTORY_TYPE_NAME;
    }
    //-----------------------------------------------------------------------
    bool Entity::tempVertexAnimBuffersBound(void) const
    {
        // Do we still have temp buffers for software vertex animation bound?
        bool ret = true;
        if (mMesh->sharedVertexData[VpNormal] && mMesh->getSharedVertexDataAnimationType() != VAT_NONE)
        {
            ret = ret && mTempVertexAnimInfo.buffersCheckedOut(true, mMesh->getSharedVertexDataAnimationIncludesNormals());
        }
        for (SubEntityList::const_iterator i = mSubEntityList.begin();
            i != mSubEntityList.end(); ++i)
        {
            const SubEntity &sub = *i;
            if (!sub.getSubMesh()->useSharedVertices
                && sub.getSubMesh()->getVertexAnimationType() != VAT_NONE)
            {
                ret = ret && sub._getVertexAnimTempBufferInfo()->buffersCheckedOut(
                    true, sub.getSubMesh()->getVertexAnimationIncludesNormals());
            }
        }
        return ret;
    }
    //-----------------------------------------------------------------------
    bool Entity::tempSkelAnimBuffersBound(bool requestNormals) const
    {
        // Do we still have temp buffers for software skeleton animation bound?
        if (mSkelAnimVertexData)
        {
            if (!mTempSkelAnimInfo.buffersCheckedOut(true, requestNormals))
                return false;
        }
        for (SubEntityList::const_iterator i = mSubEntityList.begin();
             i != mSubEntityList.end(); ++i)
        {
            const SubEntity &sub = *i;
            if (sub.mSkelAnimVertexData)
            {
                if (!sub.mTempSkelAnimInfo.buffersCheckedOut(true, requestNormals))
                    return false;
            }
        }
        return true;
    }
    //-----------------------------------------------------------------------
    void Entity::updateAnimation(void)
    {
        // Do nothing if not initialised yet
        if (!mInitialised)
            return;

        bool hwAnimation = isHardwareAnimationEnabled();
        bool isNeedUpdateHardwareAnim = hwAnimation && !mCurrentHWAnimationState;
        bool forcedSwAnimation = getSoftwareAnimationRequests()>0;
        bool forcedNormals = getSoftwareAnimationNormalsRequests()>0;
        bool softwareAnimation = !hwAnimation || forcedSwAnimation;
        // Blend normals in s/w only if we're not using h/w animation,
        // since shadows only require positions
        bool blendNormals = !hwAnimation || forcedNormals;
        // Animation dirty if animation state modified or manual bones modified
        bool animationDirty =
            (mFrameAnimationLastUpdated != mAnimationState->getDirtyFrameNumber()) ||
            (hasSkeleton() && getSkeleton()->getManualBonesDirty());
        
        //update the current hardware animation state
        mCurrentHWAnimationState = hwAnimation;

        // We only do these tasks if animation is dirty
        // Or, if we're using a skeleton and manual bones have been moved
        // Or, if we're using software animation and temp buffers are unbound
        if (animationDirty ||
            (softwareAnimation && hasVertexAnimation() && !tempVertexAnimBuffersBound()) ||
            (softwareAnimation && hasSkeleton() && !tempSkelAnimBuffersBound(blendNormals)))
        {
            if (hasVertexAnimation())
            {
                if (softwareAnimation)
                {
                    // grab & bind temporary buffer for positions (& normals if they are included)
                    if (mSoftwareVertexAnimVertexData
                        && mMesh->getSharedVertexDataAnimationType() != VAT_NONE)
                    {
                        bool useNormals = mMesh->getSharedVertexDataAnimationIncludesNormals();
                        mTempVertexAnimInfo.checkoutTempCopies(true, useNormals);
                        // NB we suppress hardware upload while doing blend if we're
                        // hardware animation, because the only reason for doing this
                        // is for shadow, which need only be uploaded then
                        mTempVertexAnimInfo.bindTempCopies(mSoftwareVertexAnimVertexData,
                            hwAnimation);
                    }
                    SubEntityList::iterator i, iend;
                    iend = mSubEntityList.end();
                    for (i = mSubEntityList.begin(); i != iend; ++i)
                    {
                        // Blend dedicated geometry
                        SubEntity &se = *i;
                        if (se.mSoftwareVertexAnimVertexData
                            && se.getSubMesh()->getVertexAnimationType() != VAT_NONE)
                        {
                            bool useNormals = se.getSubMesh()->getVertexAnimationIncludesNormals();
                            se.mTempVertexAnimInfo.checkoutTempCopies(true, useNormals);
                            se.mTempVertexAnimInfo.bindTempCopies(se.mSoftwareVertexAnimVertexData,
                                hwAnimation);
                        }

                    }
                }
                applyVertexAnimation(hwAnimation);
            }

            if (hasSkeleton())
            {
                cacheBoneMatrices();

                // Software blend?
                if (softwareAnimation)
                {
                    const Matrix4* blendMatrices[256];

                    // Ok, we need to do a software blend
                    // Firstly, check out working vertex buffers
                    if (mSkelAnimVertexData)
                    {
                        // Blend shared geometry
                        // NB we suppress hardware upload while doing blend if we're
                        // hardware animation, because the only reason for doing this
                        // is for shadow, which need only be uploaded then
                        mTempSkelAnimInfo.checkoutTempCopies(true, blendNormals);
                        mTempSkelAnimInfo.bindTempCopies(mSkelAnimVertexData,
                            hwAnimation);
                        // Prepare blend matrices, TODO: Move out of here
                        Mesh::prepareMatricesForVertexBlend(blendMatrices,
                            mBoneMatrices, mMesh->sharedBlendIndexToBoneIndexMap);
                        // Blend, taking source from either mesh data or morph data
                        Mesh::softwareVertexBlend(
                            (mMesh->getSharedVertexDataAnimationType() != VAT_NONE) ?
                                mSoftwareVertexAnimVertexData : mMesh->sharedVertexData[VpNormal],
                            mSkelAnimVertexData,
                            blendMatrices, mMesh->sharedBlendIndexToBoneIndexMap.size(),
                            blendNormals);
                    }
                    SubEntityList::iterator i, iend;
                    iend = mSubEntityList.end();
                    for (i = mSubEntityList.begin(); i != iend; ++i)
                    {
                        // Blend dedicated geometry
                        SubEntity &se = *i;
                        if (se.mSkelAnimVertexData)
                        {
                            se.mTempSkelAnimInfo.checkoutTempCopies(true, blendNormals);
                            se.mTempSkelAnimInfo.bindTempCopies(se.mSkelAnimVertexData,
                                hwAnimation);
                            // Prepare blend matrices, TODO: Move out of here
                            Mesh::prepareMatricesForVertexBlend(blendMatrices,
                                mBoneMatrices, se.mSubMesh->blendIndexToBoneIndexMap);
                            // Blend, taking source from either mesh data or morph data
                            Mesh::softwareVertexBlend(
                                (se.getSubMesh()->getVertexAnimationType() != VAT_NONE)?
                                    se.mSoftwareVertexAnimVertexData : se.mSubMesh->vertexData[VpNormal],
                                se.mSkelAnimVertexData,
                                blendMatrices, se.mSubMesh->blendIndexToBoneIndexMap.size(),
                                blendNormals);
                        }

                    }

                }
            }

            mFrameAnimationLastUpdated = mAnimationState->getDirtyFrameNumber();
        }

        // Need to update the child object's transforms when animation dirty
        // or parent node transform has altered.
        if (hasSkeleton() && 
            (isNeedUpdateHardwareAnim || 
            animationDirty || mLastParentXform != _getParentNodeFullTransform()))
        {
            // Cache last parent transform for next frame use too.
            mLastParentXform = _getParentNodeFullTransform();

            // Also calculate bone world matrices, since are used as replacement world matrices,
            // but only if it's used (when using hardware animation and skeleton animated).
            if (hwAnimation && _isSkeletonAnimated())
            {
                // Allocate bone world matrices on demand, for better memory footprint
                // when using software animation.
                if (!mBoneWorldMatrices)
                {
                    mBoneWorldMatrices =
                        static_cast<Matrix4*>(OGRE_MALLOC_SIMD(sizeof(Matrix4) * mNumBoneMatrices, MEMCATEGORY_ANIMATION));
                }

                OptimisedUtil::getImplementation()->concatenateAffineMatrices(
                    mLastParentXform,
                    mBoneMatrices,
                    mBoneWorldMatrices,
                    mNumBoneMatrices);
            }
        }
    }
    //-----------------------------------------------------------------------
    ushort Entity::initHardwareAnimationElements(VertexData* vdata,
                                                 ushort numberOfElements, bool animateNormals)
    {
        ushort elemsSupported = numberOfElements;
        if (vdata->hwAnimationDataList.size() < numberOfElements)
        {
            elemsSupported = 
                vdata->allocateHardwareAnimationElements(numberOfElements, animateNormals);
        }
        // Initialise parametrics in case we don't use all of them
        for (size_t i = 0; i < vdata->hwAnimationDataList.size(); ++i)
        {
            vdata->hwAnimationDataList[i].parametric = 0.0f;
        }
        // reset used count
        vdata->hwAnimDataItemsUsed = 0;
                
        return elemsSupported;

    }
    //-----------------------------------------------------------------------
    void Entity::applyVertexAnimation(bool hardwareAnimation)
    {
        const MeshPtr& msh = getMesh();
        bool swAnim = !hardwareAnimation || (mSoftwareAnimationRequests>0);

        // make sure we have enough hardware animation elements to play with
        if (hardwareAnimation)
        {
            if (mHardwareVertexAnimVertexData
                && msh->getSharedVertexDataAnimationType() != VAT_NONE)
            {
                ushort supportedCount =
                  initHardwareAnimationElements(mHardwareVertexAnimVertexData,
                    (msh->getSharedVertexDataAnimationType() == VAT_POSE)
                    ? mHardwarePoseCount : 1, 
                    msh->getSharedVertexDataAnimationIncludesNormals());
                
                if (msh->getSharedVertexDataAnimationType() == VAT_POSE && 
                    supportedCount < mHardwarePoseCount)
                {
                    LogManager::getSingleton().stream() <<
                      "Vertex program assigned to Entity '" << mName << 
                      "' claimed to support " << mHardwarePoseCount << 
                      " morph/pose vertex sets, but in fact only " << supportedCount <<
                      " were able to be supported in the shared mesh data.";
                    mHardwarePoseCount = supportedCount;
                }
                    
            }
            for (SubEntityList::iterator si = mSubEntityList.begin();
                si != mSubEntityList.end(); ++si)
            {
                SubEntity &sub = *si;
                if (sub.getSubMesh()->getVertexAnimationType() != VAT_NONE &&
                    !sub.getSubMesh()->useSharedVertices)
                {
                    ushort supportedCount = initHardwareAnimationElements(
                        sub._getHardwareVertexAnimVertexData(),
                        (sub.getSubMesh()->getVertexAnimationType() == VAT_POSE)
                        ? sub.mHardwarePoseCount : 1,
                        sub.getSubMesh()->getVertexAnimationIncludesNormals());

                    if (sub.getSubMesh()->getVertexAnimationType() == VAT_POSE &&
                        supportedCount < sub.mHardwarePoseCount)
                    {
                        LogManager::getSingleton().stream() <<
                        "Vertex program assigned to SubEntity of '" << mName << 
                        "' claimed to support " << sub.mHardwarePoseCount <<
                        " morph/pose vertex sets, but in fact only " << supportedCount <<
                        " were able to be supported in the mesh data.";
                        sub.mHardwarePoseCount = supportedCount;
                    }
                    
                }
            }

        }
        else
        {
            // May be blending multiple poses in software
            // Suppress hardware upload of buffers
            // Note, we query position buffer here but it may also include normals
            if (mSoftwareVertexAnimVertexData &&
                mMesh->getSharedVertexDataAnimationType() == VAT_POSE)
            {
                const VertexElement* elem = mSoftwareVertexAnimVertexData
                    ->vertexDeclaration->findElementBySemantic(VES_POSITION);
                HardwareVertexBufferSharedPtr buf = mSoftwareVertexAnimVertexData
                    ->vertexBufferBinding->getBuffer(elem->getSource());
                buf->suppressHardwareUpdate(true);
                
                initialisePoseVertexData(mMesh->sharedVertexData[VpNormal],
                                         mSoftwareVertexAnimVertexData,
                                         mMesh->getSharedVertexDataAnimationIncludesNormals());
            }
            for (SubEntityList::iterator si = mSubEntityList.begin();
                si != mSubEntityList.end(); ++si)
            {
                SubEntity &sub = *si;
                if (!sub.getSubMesh()->useSharedVertices &&
                    sub.getSubMesh()->getVertexAnimationType() == VAT_POSE)
                {
                    VertexData* data = sub._getSoftwareVertexAnimVertexData();
                    const VertexElement* elem = data->vertexDeclaration
                        ->findElementBySemantic(VES_POSITION);
                    HardwareVertexBufferSharedPtr buf = data
                        ->vertexBufferBinding->getBuffer(elem->getSource());
                    buf->suppressHardwareUpdate(true);
                    // if we're animating normals, we need to start with zeros
                    initialisePoseVertexData(sub.getSubMesh()->vertexData[VpNormal], data,
                        sub.getSubMesh()->getVertexAnimationIncludesNormals());
                }
            }
        }


        // Now apply the animation(s)
        // Note - you should only apply one morph animation to each set of vertex data
        // at once; if you do more, only the last one will actually apply
        markBuffersUnusedForAnimation();
        ConstEnabledAnimationStateIterator animIt = mAnimationState->getEnabledAnimationStateIterator();
        while(animIt.hasMoreElements())
        {
            const AnimationState* state = animIt.getNext();
            Animation* anim = msh->_getAnimationImpl(state->getAnimationName());
            if (anim)
            {
                anim->apply(this, state->getTimePosition(), state->getWeight(),
                    swAnim, hardwareAnimation);
            }
        }
        // Deal with cases where no animation applied
        restoreBuffersForUnusedAnimation(hardwareAnimation);

        // Unsuppress hardware upload if we suppressed it
        if (!hardwareAnimation)
        {
            if (mSoftwareVertexAnimVertexData &&
                msh->getSharedVertexDataAnimationType() == VAT_POSE)
            {
                // if we're animating normals, if pose influence < 1 need to use the base mesh
                if (mMesh->getSharedVertexDataAnimationIncludesNormals())
                    finalisePoseNormals(mMesh->sharedVertexData[VpNormal], mSoftwareVertexAnimVertexData);
            
                const VertexElement* elem = mSoftwareVertexAnimVertexData
                    ->vertexDeclaration->findElementBySemantic(VES_POSITION);
                HardwareVertexBufferSharedPtr buf = mSoftwareVertexAnimVertexData
                    ->vertexBufferBinding->getBuffer(elem->getSource());
                buf->suppressHardwareUpdate(false);
            }
            for (SubEntityList::iterator si = mSubEntityList.begin();
                si != mSubEntityList.end(); ++si)
            {
                SubEntity &sub = *si;
                if (!sub.getSubMesh()->useSharedVertices &&
                    sub.getSubMesh()->getVertexAnimationType() == VAT_POSE)
                {
                    VertexData* data = sub._getSoftwareVertexAnimVertexData();
                    // if we're animating normals, if pose influence < 1 need to use the base mesh
                    if (sub.getSubMesh()->getVertexAnimationIncludesNormals())
                        finalisePoseNormals(sub.getSubMesh()->vertexData[VpNormal], data);
                    
                    const VertexElement* elem = data->vertexDeclaration
                        ->findElementBySemantic(VES_POSITION);
                    HardwareVertexBufferSharedPtr buf = data
                        ->vertexBufferBinding->getBuffer(elem->getSource());
                    buf->suppressHardwareUpdate(false);
                }
            }
        }

    }
    //-----------------------------------------------------------------------------
    void Entity::markBuffersUnusedForAnimation(void)
    {
        mVertexAnimationAppliedThisFrame = false;
        for (SubEntityList::iterator i = mSubEntityList.begin();
            i != mSubEntityList.end(); ++i)
        {
            i->_markBuffersUnusedForAnimation();
        }
    }
    //-----------------------------------------------------------------------------
    void Entity::_markBuffersUsedForAnimation(void)
    {
        mVertexAnimationAppliedThisFrame = true;
        // no cascade
    }
    //-----------------------------------------------------------------------------
    void Entity::restoreBuffersForUnusedAnimation(bool hardwareAnimation)
    {
        // Rebind original positions if:
        //  We didn't apply any animation and
        //    We're morph animated (hardware binds keyframe, software is missing)
        //    or we're pose animated and software (hardware is fine, still bound)
        if (mMesh->sharedVertexData[VpNormal] &&
            !mVertexAnimationAppliedThisFrame &&
            (!hardwareAnimation || mMesh->getSharedVertexDataAnimationType() == VAT_MORPH))
        {
            // Note, VES_POSITION is specified here but if normals are included in animation
            // then these will be re-bound too (buffers must be shared)
            const VertexElement* srcPosElem =
                mMesh->sharedVertexData[VpNormal]->vertexDeclaration->
                    findElementBySemantic(VES_POSITION);
            HardwareVertexBufferSharedPtr srcBuf =
                mMesh->sharedVertexData[VpNormal]->vertexBufferBinding->getBuffer(
                    srcPosElem->getSource());

            // Bind to software
            const VertexElement* destPosElem =
                mSoftwareVertexAnimVertexData->vertexDeclaration->findElementBySemantic(VES_POSITION);
            mSoftwareVertexAnimVertexData->vertexBufferBinding->setBinding(
                destPosElem->getSource(), srcBuf);
                
        }

        // rebind any missing hardware pose buffers
        // Caused by not having any animations enabled, or keyframes which reference
        // no poses
        if (mMesh->sharedVertexData[VpNormal] && hardwareAnimation
            && mMesh->getSharedVertexDataAnimationType() == VAT_POSE)
        {
            bindMissingHardwarePoseBuffers(mMesh->sharedVertexData[VpNormal],
                                           mHardwareVertexAnimVertexData);
        }


        for (SubEntityList::iterator i = mSubEntityList.begin();
            i != mSubEntityList.end(); ++i)
        {
            i->_restoreBuffersForUnusedAnimation(hardwareAnimation);
        }

    }
    //---------------------------------------------------------------------
    void Entity::bindMissingHardwarePoseBuffers(const VertexData* srcData, 
        VertexData* destData)
    {
        // For hardware pose animation, also make sure we've bound buffers to all the elements
        // required - if there are missing bindings for elements in use,
        // some rendersystems can complain because elements refer
        // to an unbound source.
        // Get the original position source, we'll use this to fill gaps
        const VertexElement* srcPosElem =
            srcData->vertexDeclaration->findElementBySemantic(VES_POSITION);
        HardwareVertexBufferSharedPtr srcBuf =
            srcData->vertexBufferBinding->getBuffer(
                srcPosElem->getSource());

        for (VertexData::HardwareAnimationDataList::const_iterator i = destData->hwAnimationDataList.begin();
            i != destData->hwAnimationDataList.end(); ++i)
        {
            const VertexData::HardwareAnimationData& animData = *i;
            if (!destData->vertexBufferBinding->isBufferBound(
                animData.targetBufferIndex))
            {
                // Bind to a safe default
                destData->vertexBufferBinding->setBinding(
                    animData.targetBufferIndex, srcBuf);
            }
        }

    }
    //-----------------------------------------------------------------------
    void Entity::initialisePoseVertexData(const VertexData* srcData, 
        VertexData* destData, bool animateNormals)
    {
    
        // First time through for a piece of pose animated vertex data
        // We need to copy the original position values to the temp accumulator
        const VertexElement* origelem = 
            srcData->vertexDeclaration->findElementBySemantic(VES_POSITION);
        const VertexElement* destelem = 
            destData->vertexDeclaration->findElementBySemantic(VES_POSITION);
        HardwareVertexBufferSharedPtr origBuffer = 
            srcData->vertexBufferBinding->getBuffer(origelem->getSource());
        HardwareVertexBufferSharedPtr destBuffer = 
            destData->vertexBufferBinding->getBuffer(destelem->getSource());
        destBuffer->copyData(*origBuffer.get(), 0, 0, destBuffer->getSizeInBytes(), true);
    
        // If normals are included in animation, we want to reset the normals to zero
        if (animateNormals)
        {
            const VertexElement* normElem =
                destData->vertexDeclaration->findElementBySemantic(VES_NORMAL);
                
            if (normElem)
            {
                HardwareVertexBufferSharedPtr buf = 
                    destData->vertexBufferBinding->getBuffer(normElem->getSource());
                char* pBase = static_cast<char*>(buf->lock(HardwareBuffer::HBL_NORMAL));
                pBase += destData->vertexStart * buf->getVertexSize();
                
                for (size_t v = 0; v < destData->vertexCount; ++v)
                {
                    float* pNorm;
                    normElem->baseVertexPointerToElement(pBase, &pNorm);
                    *pNorm++ = 0.0f;
                    *pNorm++ = 0.0f;
                    *pNorm++ = 0.0f;
                    
                    pBase += buf->getVertexSize();
                }
                buf->unlock();
            }
        }
    }
    //-----------------------------------------------------------------------
    void Entity::finalisePoseNormals(const VertexData* srcData, VertexData* destData)
    {
        const VertexElement* destNormElem =
            destData->vertexDeclaration->findElementBySemantic(VES_NORMAL);
        const VertexElement* srcNormElem =
            srcData->vertexDeclaration->findElementBySemantic(VES_NORMAL);
            
        if (destNormElem && srcNormElem)
        {
            HardwareVertexBufferSharedPtr srcbuf = 
                srcData->vertexBufferBinding->getBuffer(srcNormElem->getSource());
            HardwareVertexBufferSharedPtr dstbuf = 
                destData->vertexBufferBinding->getBuffer(destNormElem->getSource());
            char* pSrcBase = static_cast<char*>(srcbuf->lock(HardwareBuffer::HBL_READ_ONLY));
            char* pDstBase = static_cast<char*>(dstbuf->lock(HardwareBuffer::HBL_NORMAL));
            pSrcBase += srcData->vertexStart * srcbuf->getVertexSize();
            pDstBase += destData->vertexStart * dstbuf->getVertexSize();
            
            // The goal here is to detect the length of the vertices, and to apply
            // the base mesh vertex normal at one minus that length; this deals with 
            // any individual vertices which were either not affected by any pose, or
            // were not affected to a complete extent
            // We also normalise every normal to deal with over-weighting
            for (size_t v = 0; v < destData->vertexCount; ++v)
            {
                float* pDstNorm;
                destNormElem->baseVertexPointerToElement(pDstBase, &pDstNorm);
                Vector3 norm(pDstNorm[0], pDstNorm[1], pDstNorm[2]);
                Real len = norm.length();
                if (len + 1e-4f < 1.0f)
                {
                    // Poses did not completely fill in this normal
                    // Apply base mesh
                    float baseWeight = 1.0f - (float)len;
                    float* pSrcNorm;
                    srcNormElem->baseVertexPointerToElement(pSrcBase, &pSrcNorm);
                    norm.x += *pSrcNorm++ * baseWeight;
                    norm.y += *pSrcNorm++ * baseWeight;
                    norm.z += *pSrcNorm++ * baseWeight;
                }
                norm.normalise();
                
                *pDstNorm++ = (float)norm.x;
                *pDstNorm++ = (float)norm.y;
                *pDstNorm++ = (float)norm.z;
                
                pDstBase += dstbuf->getVertexSize();
                pSrcBase += dstbuf->getVertexSize();
            }
            srcbuf->unlock();
            dstbuf->unlock();
        }
    }
    //-----------------------------------------------------------------------
    void Entity::_updateAnimation(void)
    {
        // Externally visible method
        if (hasSkeleton() || hasVertexAnimation())
        {
            updateAnimation();
        }
    }
    //-----------------------------------------------------------------------
    bool Entity::_isAnimated(void) const
    {
        return (mAnimationState && mAnimationState->hasEnabledAnimationState()) ||
               (getSkeleton() && getSkeleton()->hasManualBones());
    }
    //-----------------------------------------------------------------------
    bool Entity::_isSkeletonAnimated(void) const
    {
        return getSkeleton() &&
            (mAnimationState->hasEnabledAnimationState() || getSkeleton()->hasManualBones());
    }
    //-----------------------------------------------------------------------
    VertexData* Entity::_getSkelAnimVertexData(void) const
    {
        assert (mSkelAnimVertexData && "Not software skinned or has no shared vertex data!");
        return mSkelAnimVertexData;
    }
    //-----------------------------------------------------------------------
    VertexData* Entity::_getSoftwareVertexAnimVertexData(void) const
    {
        assert (mSoftwareVertexAnimVertexData && "Not vertex animated or has no shared vertex data!");
        return mSoftwareVertexAnimVertexData;
    }
    //-----------------------------------------------------------------------
    VertexData* Entity::_getHardwareVertexAnimVertexData(void) const
    {
        assert (mHardwareVertexAnimVertexData && "Not vertex animated or has no shared vertex data!");
        return mHardwareVertexAnimVertexData;
    }
    //-----------------------------------------------------------------------
    TempBlendedBufferInfo* Entity::_getSkelAnimTempBufferInfo(void)
    {
        return &mTempSkelAnimInfo;
    }
    //-----------------------------------------------------------------------
    TempBlendedBufferInfo* Entity::_getVertexAnimTempBufferInfo(void)
    {
        return &mTempVertexAnimInfo;
    }
    //-----------------------------------------------------------------------
    bool Entity::cacheBoneMatrices(void)
    {
        Root& root = Root::getSingleton();
        unsigned long currentFrameNumber = root.getNextFrameNumber();
        if ((*mFrameBonesLastUpdated != currentFrameNumber) ||
            (hasSkeleton() && getSkeleton()->getManualBonesDirty()))
        {
            if ((!mSkipAnimStateUpdates) && (*mFrameBonesLastUpdated != currentFrameNumber))
                mSkeletonInstance->setAnimationState(*mAnimationState);
            mSkeletonInstance->_getBoneMatrices(mBoneMatrices);
            *mFrameBonesLastUpdated  = currentFrameNumber;

            return true;
        }
        return false;
    }
    //-----------------------------------------------------------------------
    void Entity::setDisplaySkeleton(bool display)
    {
        mDisplaySkeleton = display;
    }
    //-----------------------------------------------------------------------
    bool Entity::getDisplaySkeleton(void) const
    {
        return mDisplaySkeleton;
    }
    //-----------------------------------------------------------------------
    Entity* Entity::getManualLodLevel(size_t index) const
    {
#if !OGRE_NO_MESHLOD
        assert(index < mLodEntityList.size());

        return mLodEntityList[index];
#else
        OgreAssert(0, "This feature is disabled in ogre configuration!");
        return NULL;
#endif
    }
    //-----------------------------------------------------------------------
    size_t Entity::getNumManualLodLevels(void) const
    {
        return mLodEntityList.size();
    }
    //-----------------------------------------------------------------------
    void Entity::buildSubEntityList(MeshPtr& mesh, SubEntityList* sublist)
    {
        // Create SubEntities
        unsigned short i, numSubMeshes;
        SubMesh* subMesh;

        numSubMeshes = mesh->getNumSubMeshes();
        sublist->reserve( numSubMeshes );
        for (i = 0; i < numSubMeshes; ++i)
        {
            subMesh = mesh->getSubMesh(i);
            sublist->push_back( SubEntity( this, subMesh ) );
        }
    }
    //-----------------------------------------------------------------------
    void Entity::setPolygonModeOverrideable(bool overrideable)
    {
        SubEntityList::iterator i, iend;
        iend = mSubEntityList.end();

        for( i = mSubEntityList.begin(); i != iend; ++i )
        {
            i->setPolygonModeOverrideable(overrideable);
        }
    }

    //-----------------------------------------------------------------------
    TagPoint* Entity::attachObjectToBone(const String &boneName, MovableObject *pMovable, const Quaternion &offsetOrientation, const Vector3 &offsetPosition)
    {
#ifdef ENABLE_INCOMPATIBLE_OGRE_2_0
        if (mChildObjectList.find(pMovable->getName()) != mChildObjectList.end())
        {
            OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM,
                "An object with the name " + pMovable->getName() + " already attached",
                "Entity::attachObjectToBone");
        }
        if(pMovable->isAttached())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Object already attached to a sceneNode or a OldBone",
                "Entity::attachObjectToBone");
        }
        if (!hasSkeleton())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "This entity's mesh has no skeleton to attach object to.",
                "Entity::attachObjectToBone");
        }
        OldBone* bone = mSkeletonInstance->getBone(boneName);
        if (!bone)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Cannot locate bone named " + boneName,
                "Entity::attachObjectToBone");
        }

        TagPoint *tp = mSkeletonInstance->createTagPointOnBone(
            bone, offsetOrientation, offsetPosition);
        tp->setParentEntity(this);
        tp->setChildObject(pMovable);

        attachObjectImpl(pMovable, tp);

        return tp;
#endif
        return 0;
    }

    //-----------------------------------------------------------------------
    void Entity::attachObjectImpl(MovableObject *pObject, TagPoint *pAttachingPoint)
    {
#ifdef ENABLE_INCOMPATIBLE_OGRE_2_0
        assert(mChildObjectList.find(pObject->getName()) == mChildObjectList.end());
        mChildObjectList[pObject->getName()] = pObject;
        pObject->_notifyAttached(pAttachingPoint, true);
#endif
    }

    //-----------------------------------------------------------------------
    MovableObject* Entity::detachObjectFromBone(const String &name)
    {
#ifdef ENABLE_INCOMPATIBLE_OGRE_2_0
        ChildObjectList::iterator i = mChildObjectList.find(name);

        if (i == mChildObjectList.end())
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "No child object entry found named " + name,
                "Entity::detachObjectFromBone");
        }
        MovableObject *obj = i->second;
        detachObjectImpl(obj);
        mChildObjectList.erase(i);

        return obj;
#endif
        return 0;
    }
    //-----------------------------------------------------------------------
    void Entity::detachObjectFromBone(MovableObject* obj)
    {
#ifdef ENABLE_INCOMPATIBLE_OGRE_2_0
        ChildObjectList::iterator i, iend;
        iend = mChildObjectList.end();
        for (i = mChildObjectList.begin(); i != iend; ++i)
        {
            if (i->second == obj)
            {
                detachObjectImpl(obj);
                mChildObjectList.erase(i);
                break;
            }
        }
#endif
    }
    //-----------------------------------------------------------------------
    void Entity::detachAllObjectsFromBone(void)
    {
#ifdef ENABLE_INCOMPATIBLE_OGRE_2_0
        detachAllObjectsImpl();
#endif
    }
    //-----------------------------------------------------------------------
    void Entity::detachObjectImpl(MovableObject* pObject)
    {
#ifdef ENABLE_INCOMPATIBLE_OGRE_2_0
        TagPoint* tp = static_cast<TagPoint*>(pObject->getParentNode());

        // free the TagPoint so we can reuse it later
        mSkeletonInstance->freeTagPoint(tp);

        pObject->_notifyAttached((TagPoint*)0);
#endif
    }
#ifdef ENABLE_INCOMPATIBLE_OGRE_2_0
    //-----------------------------------------------------------------------
    void Entity::detachAllObjectsImpl(void)
    {

        ChildObjectList::const_iterator i, iend;
        iend = mChildObjectList.end();
        for (i = mChildObjectList.begin(); i != iend; ++i)
        {
            detachObjectImpl(i->second);
        }
        mChildObjectList.clear()
    }
#endif

#ifdef ENABLE_INCOMPATIBLE_OGRE_2_0
    //-----------------------------------------------------------------------
    Entity::ChildObjectListIterator Entity::getAttachedObjectIterator()
    {
        return ChildObjectListIterator(mChildObjectList.begin(), mChildObjectList.end());
    }
#endif
    //-----------------------------------------------------------------------
    void Entity::prepareTempBlendBuffers(void)
    {
        if (mSkelAnimVertexData)
        {
            OGRE_DELETE mSkelAnimVertexData;
            mSkelAnimVertexData = 0;
        }
        if (mSoftwareVertexAnimVertexData)
        {
            OGRE_DELETE mSoftwareVertexAnimVertexData;
            mSoftwareVertexAnimVertexData = 0;
        }
        if (mHardwareVertexAnimVertexData)
        {
            OGRE_DELETE mHardwareVertexAnimVertexData;
            mHardwareVertexAnimVertexData = 0;
        }

        if (hasVertexAnimation())
        {
            // Shared data
            if (mMesh->sharedVertexData[VpNormal]
                && mMesh->getSharedVertexDataAnimationType() != VAT_NONE)
            {
                // Create temporary vertex blend info
                // Prepare temp vertex data if needed
                // Clone without copying data, don't remove any blending info
                // (since if we skeletally animate too, we need it)
                mSoftwareVertexAnimVertexData = mMesh->sharedVertexData[VpNormal]->clone(false);
                extractTempBufferInfo(mSoftwareVertexAnimVertexData, &mTempVertexAnimInfo);

                // Also clone for hardware usage, don't remove blend info since we'll
                // need it if we also hardware skeletally animate
                mHardwareVertexAnimVertexData = mMesh->sharedVertexData[VpNormal]->clone(false);
            }
        }

        if (hasSkeleton())
        {
            // Shared data
            if (mMesh->sharedVertexData[VpNormal])
            {
                // Create temporary vertex blend info
                // Prepare temp vertex data if needed
                // Clone without copying data, remove blending info
                // (since blend is performed in software)
                mSkelAnimVertexData =
                    cloneVertexDataRemoveBlendInfo(mMesh->sharedVertexData[VpNormal]);
                extractTempBufferInfo(mSkelAnimVertexData, &mTempSkelAnimInfo);
            }

        }

        // Do SubEntities
        SubEntityList::iterator i, iend;
        iend = mSubEntityList.end();
        for (i = mSubEntityList.begin(); i != iend; ++i)
        {
            i->prepareTempBlendBuffers();
        }

        // It's prepared for shadow volumes only if mesh has been prepared for shadow volumes.
        mPreparedForShadowVolumes = mMesh->isPreparedForShadowVolumes();
    }
    //-----------------------------------------------------------------------
    void Entity::extractTempBufferInfo(VertexData* sourceData, TempBlendedBufferInfo* info)
    {
        info->extractFrom(sourceData);
    }
    //-----------------------------------------------------------------------
    VertexData* Entity::cloneVertexDataRemoveBlendInfo(const VertexData* source)
    {
        // Clone without copying data
        VertexData* ret = source->clone(false);
        bool removeIndices = Ogre::Root::getSingleton().isBlendIndicesGpuRedundant();
        bool removeWeights = Ogre::Root::getSingleton().isBlendWeightsGpuRedundant();
         
        unsigned short safeSource = 0xFFFF;
        const VertexElement* blendIndexElem =
            source->vertexDeclaration->findElementBySemantic(VES_BLEND_INDICES);
        if (blendIndexElem)
        {
            //save the source in order to prevent the next stage from unbinding it.
            safeSource = blendIndexElem->getSource();
            if (removeIndices)
            {
                // Remove buffer reference
                ret->vertexBufferBinding->unsetBinding(blendIndexElem->getSource());
            }
        }
        if (removeWeights)
        {
            // Remove blend weights
            const VertexElement* blendWeightElem =
                source->vertexDeclaration->findElementBySemantic(VES_BLEND_WEIGHTS);
            if (blendWeightElem &&
                blendWeightElem->getSource() != safeSource)
            {
                // Remove buffer reference
                ret->vertexBufferBinding->unsetBinding(blendWeightElem->getSource());
            }
        }

        // remove elements from declaration
        if (removeIndices)
            ret->vertexDeclaration->removeElement(VES_BLEND_INDICES);
        if (removeWeights)
            ret->vertexDeclaration->removeElement(VES_BLEND_WEIGHTS);

        // Close gaps in bindings for effective and safely
        if (removeWeights || removeIndices)
            ret->closeGapsInBindings();

        return ret;
    }
    //-----------------------------------------------------------------------
    EdgeData* Entity::getEdgeList(void)
    {
#if OGRE_NO_MESHLOD
        unsigned short mMeshLodIndex = 0;
#endif
        // Get from Mesh
        return mMesh->getEdgeList(mCurrentMeshLod);
    }
    //-----------------------------------------------------------------------
    bool Entity::hasEdgeList(void)
    {
#if OGRE_NO_MESHLOD
        unsigned short mMeshLodIndex = 0;
#endif
        // check if mesh has an edge list attached
        // give mesh a chance to built it if scheduled
        return (mMesh->getEdgeList(mCurrentMeshLod) != NULL);
    }
    //-----------------------------------------------------------------------
    bool Entity::isHardwareAnimationEnabled(void)
    {
        //find whether the entity has hardware animation for the current active sceme
        unsigned short schemeIndex = MaterialManager::getSingleton()._getActiveSchemeIndex();
        SchemeHardwareAnimMap::iterator it = mSchemeHardwareAnim.find(schemeIndex);
        if (it == mSchemeHardwareAnim.end())
        {
            //evaluate the animation hardware value
            it = mSchemeHardwareAnim.insert(
                SchemeHardwareAnimMap::value_type(schemeIndex,
                    calcVertexProcessing())).first;
        }
        return it->second;
    }

    //-----------------------------------------------------------------------
    void Entity::reevaluateVertexProcessing(void)
    {
        //clear the cache so that the values will be reevaluated
        mSchemeHardwareAnim.clear();
    }
    //-----------------------------------------------------------------------
    bool Entity::calcVertexProcessing(void)
    {
        // init
        bool hasHardwareAnimation = false;
        bool firstPass = true;

        SubEntityList::iterator i, iend;
        iend = mSubEntityList.end();
        for (i = mSubEntityList.begin(); i != iend; ++i)
        {
            SubEntity &sub = *i;

            const MaterialPtr& m = sub.getMaterial();

            if( m.isNull() )
            {
                mVertexProgramInUse = true;
                if (hasSkeleton())
                {
                    VertexAnimationType animType = VAT_NONE;
                    if (sub.getSubMesh()->useSharedVertices)
                    {
                        animType = mMesh->getSharedVertexDataAnimationType();
                    }
                    else
                    {
                        animType = sub.getSubMesh()->getVertexAnimationType();
                    }

                    //Disable hw animation if there are morph and pose animations, at least for now.
                    //Enable if there's only skeleton animation
                    return animType == VAT_NONE;
                }

                return false;
            }

            // Make sure it's loaded
            m->load();
            Technique* t = m->getBestTechnique(0, &sub);
            if (!t)
            {
                // No supported techniques
                continue;
            }
            if (t->getNumPasses() == 0)
            {
                // No passes, invalid
                continue;
            }
            Pass* p = t->getPass(0);
            if (p->hasVertexProgram())
            {
                if (mVertexProgramInUse == false)
                {
                    // If one material uses a vertex program, set this flag
                    // Causes some special processing like forcing a separate light cap
                    mVertexProgramInUse = true;
                }

                if (hasSkeleton())
                {
                    // All materials must support skinning for us to consider using
                    // hardware animation - if one fails we use software
                    if (firstPass)
                    {
                        hasHardwareAnimation = p->getVertexProgram()->isSkeletalAnimationIncluded();
                        firstPass = false;
                    }
                    else
                    {
                        hasHardwareAnimation = hasHardwareAnimation &&
                            p->getVertexProgram()->isSkeletalAnimationIncluded();
                    }
                }

                VertexAnimationType animType = VAT_NONE;
                if (sub.getSubMesh()->useSharedVertices)
                {
                    animType = mMesh->getSharedVertexDataAnimationType();
                }
                else
                {
                    animType = sub.getSubMesh()->getVertexAnimationType();
                }
                if (animType == VAT_MORPH)
                {
                    // All materials must support morph animation for us to consider using
                    // hardware animation - if one fails we use software
                    if (firstPass)
                    {
                        hasHardwareAnimation = p->getVertexProgram()->isMorphAnimationIncluded();
                        firstPass = false;
                    }
                    else
                    {
                        hasHardwareAnimation = hasHardwareAnimation &&
                            p->getVertexProgram()->isMorphAnimationIncluded();
                    }
                }
                else if (animType == VAT_POSE)
                {
                    // All materials must support pose animation for us to consider using
                    // hardware animation - if one fails we use software
                    if (firstPass)
                    {
                        hasHardwareAnimation = p->getVertexProgram()->isPoseAnimationIncluded();
                        if (sub.getSubMesh()->useSharedVertices)
                            mHardwarePoseCount = p->getVertexProgram()->getNumberOfPosesIncluded();
                        else
                            sub.mHardwarePoseCount = p->getVertexProgram()->getNumberOfPosesIncluded();
                        firstPass = false;
                    }
                    else
                    {
                        hasHardwareAnimation = hasHardwareAnimation &&
                            p->getVertexProgram()->isPoseAnimationIncluded();
                        if (sub.getSubMesh()->useSharedVertices)
                            mHardwarePoseCount = std::max(mHardwarePoseCount,
                                p->getVertexProgram()->getNumberOfPosesIncluded());
                        else
                            sub.mHardwarePoseCount = std::max(sub.mHardwarePoseCount,
                                p->getVertexProgram()->getNumberOfPosesIncluded());
                    }
                }

            }
        }

        // Should be force update of animation if they exists, due reevaluate
        // vertex processing might switchs between hardware/software animation,
        // and then we'll end with NULL or incorrect mBoneWorldMatrices, or
        // incorrect blended software animation buffers.
        if (mAnimationState)
        {
            mFrameAnimationLastUpdated = mAnimationState->getDirtyFrameNumber() - 1;
        }

        return hasHardwareAnimation;
    }
    //-----------------------------------------------------------------------
    const VertexData* Entity::findBlendedVertexData(const VertexData* orig)
    {
        bool skel = hasSkeleton();

        if (orig == mMesh->sharedVertexData[VpNormal] || orig == mMesh->sharedVertexData[VpShadow])
        {
            return skel? mSkelAnimVertexData : mSoftwareVertexAnimVertexData;
        }
        SubEntityList::iterator i, iend;
        iend = mSubEntityList.end();
        for (i = mSubEntityList.begin(); i != iend; ++i)
        {
            if (orig == i->getSubMesh()->vertexData[VpNormal] ||
                orig == i->getSubMesh()->vertexData[VpShadow])
            {
                return skel? i->_getSkelAnimVertexData() : i->_getSoftwareVertexAnimVertexData();
            }
        }
        // None found
        OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
            "Cannot find blended version of the vertex data specified.",
            "Entity::findBlendedVertexData");
    }
    //-----------------------------------------------------------------------
    SubEntity* Entity::findSubEntityForVertexData(const VertexData* orig)
    {
        if (orig == mMesh->sharedVertexData[VpNormal] || orig == mMesh->sharedVertexData[VpShadow])
        {
            return 0;
        }

        SubEntityList::iterator i, iend;
        iend = mSubEntityList.end();
        for (i = mSubEntityList.begin(); i != iend; ++i)
        {
            if (orig == i->getSubMesh()->vertexData[VpNormal] ||
                orig == i->getSubMesh()->vertexData[VpShadow])
            {
                return &(*i);
            }
        }

        // None found
        return 0;
    }
    //-----------------------------------------------------------------------
    void Entity::addSoftwareAnimationRequest(bool normalsAlso)
    {
        mSoftwareAnimationRequests++;
        if (normalsAlso) {
            mSoftwareAnimationNormalsRequests++;
        }
    }
    //-----------------------------------------------------------------------
    void Entity::removeSoftwareAnimationRequest(bool normalsAlso)
    {
        if (mSoftwareAnimationRequests == 0 ||
            (normalsAlso && mSoftwareAnimationNormalsRequests == 0))
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Attempt to remove nonexistent request.",
                        "Entity::removeSoftwareAnimationRequest");
        }
        mSoftwareAnimationRequests--;
        if (normalsAlso) {
            mSoftwareAnimationNormalsRequests--;
        }
    }
    //-----------------------------------------------------------------------
    void Entity::_notifyAttached( Node* parent )
    {
        MovableObject::_notifyAttached(parent);
        // Also notify LOD entities
        LODEntityList::iterator i, iend;
        iend = mLodEntityList.end();
        for (i = mLodEntityList.begin(); i != iend; ++i)
        {
            (*i)->_notifyAttached(parent);
        }
    }
    //-----------------------------------------------------------------------
    void Entity::setRenderQueueGroup(uint8 queueID)
    {
        MovableObject::setRenderQueueGroup(queueID);
#if !OGRE_NO_MESHLOD
        // Set render queue for all manual LOD entities
        if (mMesh->hasManualLodLevel())
        {
            LODEntityList::iterator li, liend;
            liend = mLodEntityList.end();
            for (li = mLodEntityList.begin(); li != liend; ++li)
            {
                if(*li != this)
                    (*li)->setRenderQueueGroup(queueID);
            }
        }
#endif
    }
    //-----------------------------------------------------------------------
    void Entity::shareSkeletonInstanceWith(Entity* entity)
    {
        if (entity->getMesh()->getOldSkeleton() != getMesh()->getOldSkeleton())
        {
            OGRE_EXCEPT(Exception::ERR_RT_ASSERTION_FAILED,
                "The supplied entity has a different skeleton.",
                "Entity::shareSkeletonWith");
        }
        if (!mSkeletonInstance)
        {
            OGRE_EXCEPT(Exception::ERR_RT_ASSERTION_FAILED,
                "This entity has no skeleton.",
                "Entity::shareSkeletonWith");
        }
        if (mSharedSkeletonEntities != NULL && entity->mSharedSkeletonEntities != NULL)
        {
            OGRE_EXCEPT(Exception::ERR_RT_ASSERTION_FAILED,
                "Both entities already shares their SkeletonInstances! At least "
                "one of the instances must not share it's instance.",
                "Entity::shareSkeletonWith");
        }

        //check if we already share our skeletoninstance, we don't want to delete it if so
        if (mSharedSkeletonEntities != NULL)
        {
            entity->shareSkeletonInstanceWith(this);
        }
        else
        {
            OGRE_DELETE mSkeletonInstance;
            OGRE_FREE_SIMD(mBoneMatrices, MEMCATEGORY_ANIMATION);
            OGRE_DELETE mAnimationState;
            // using OGRE_FREE since unsigned long is not a destructor
            OGRE_FREE(mFrameBonesLastUpdated, MEMCATEGORY_ANIMATION);
            mSkeletonInstance = entity->mSkeletonInstance;
            mNumBoneMatrices = entity->mNumBoneMatrices;
            mBoneMatrices = entity->mBoneMatrices;
            mAnimationState = entity->mAnimationState;
            mFrameBonesLastUpdated = entity->mFrameBonesLastUpdated;
            if (entity->mSharedSkeletonEntities == NULL)
            {
                entity->mSharedSkeletonEntities = OGRE_NEW_T(EntitySet, MEMCATEGORY_ANIMATION)();
                entity->mSharedSkeletonEntities->insert(entity);
            }
            mSharedSkeletonEntities = entity->mSharedSkeletonEntities;
            mSharedSkeletonEntities->insert(this);
        }
    }
    //-----------------------------------------------------------------------
    void Entity::stopSharingSkeletonInstance()
    {
        if (mSharedSkeletonEntities == NULL)
        {
            OGRE_EXCEPT(Exception::ERR_RT_ASSERTION_FAILED,
                "This entity is not sharing it's skeletoninstance.",
                "Entity::shareSkeletonWith");
        }
        //check if there's no other than us sharing the skeleton instance
        if (mSharedSkeletonEntities->size() == 1)
        {
            //just reset
            OGRE_DELETE_T(mSharedSkeletonEntities, EntitySet, MEMCATEGORY_ANIMATION);
            mSharedSkeletonEntities = 0;
        }
        else
        {
            mSkeletonInstance = OGRE_NEW OldSkeletonInstance(mMesh->getOldSkeleton());
            mSkeletonInstance->load();
            mAnimationState = OGRE_NEW AnimationStateSet();
            mMesh->_initAnimationState(mAnimationState);
            mFrameBonesLastUpdated = OGRE_NEW_T(unsigned long, MEMCATEGORY_ANIMATION)(std::numeric_limits<unsigned long>::max());
            mNumBoneMatrices = mSkeletonInstance->getNumBones();
            mBoneMatrices = static_cast<Matrix4*>(OGRE_MALLOC_SIMD(sizeof(Matrix4) * mNumBoneMatrices, MEMCATEGORY_ANIMATION));

            mSharedSkeletonEntities->erase(this);
            if (mSharedSkeletonEntities->size() == 1)
            {
                (*mSharedSkeletonEntities->begin())->stopSharingSkeletonInstance();
            }
            mSharedSkeletonEntities = 0;
        }
    }
    //-----------------------------------------------------------------------
    void Entity::refreshAvailableAnimationState(void)
    {
        mMesh->_refreshAnimationState(mAnimationState);
    }
    //-----------------------------------------------------------------------
    VertexData* Entity::getVertexDataForBinding( bool casterPass )
    {
        Entity::VertexDataBindChoice c =
            chooseVertexDataForBinding(mMesh->getSharedVertexDataAnimationType() != VAT_NONE);
        switch(c)
        {
        case BIND_ORIGINAL:
            return mMesh->sharedVertexData[casterPass];
        case BIND_HARDWARE_MORPH:
            assert( !casterPass );
            return mHardwareVertexAnimVertexData;
        case BIND_SOFTWARE_MORPH:
            assert( !casterPass );
            return mSoftwareVertexAnimVertexData;
        case BIND_SOFTWARE_SKELETAL:
            assert( !casterPass );
            return mSkelAnimVertexData;
        };
        // keep compiler happy
        return mMesh->sharedVertexData[casterPass];
    }
    //-----------------------------------------------------------------------
    Entity::VertexDataBindChoice Entity::chooseVertexDataForBinding(bool vertexAnim)
    {
        if (hasSkeleton())
        {
            if (!isHardwareAnimationEnabled())
            {
                // all software skeletal binds same vertex data
                // may be a 2-stage s/w transform including morph earlier though
                return BIND_SOFTWARE_SKELETAL;
            }
            else if (vertexAnim)
            {
                // hardware morph animation
                return BIND_HARDWARE_MORPH;
            }
            else
            {
                // hardware skeletal, no morphing
                return BIND_ORIGINAL;
            }
        }
        else if (vertexAnim)
        {
            // morph only, no skeletal
            if (isHardwareAnimationEnabled())
            {
                return BIND_HARDWARE_MORPH;
            }
            else
            {
                return BIND_SOFTWARE_MORPH;
            }

        }
        else
        {
            return BIND_ORIGINAL;
        }

    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    String EntityFactory::FACTORY_TYPE_NAME = "Entity";
    //-----------------------------------------------------------------------
    const String& EntityFactory::getType(void) const
    {
        return FACTORY_TYPE_NAME;
    }
    //-----------------------------------------------------------------------
    MovableObject* EntityFactory::createInstanceImpl( IdType id,
                                                      ObjectMemoryManager *objectMemoryManager,
                                                      SceneManager* manager,
                                                      const NameValuePairList* params )
    {
        // must have mesh parameter
        MeshPtr pMesh;
        if (params != 0)
        {
            String groupName = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME;

            NameValuePairList::const_iterator ni;

            ni = params->find("resourceGroup");
            if (ni != params->end())
            {
                groupName = ni->second;
            }

            ni = params->find("mesh");
            if (ni != params->end())
            {
                // Get mesh (load if required)
                pMesh = MeshManager::getSingleton().load(
                    ni->second,
                    // autodetect group location
                    groupName );
            }

        }
        if (pMesh.isNull())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "'mesh' parameter required when constructing an Entity.",
                "EntityFactory::createInstance");
        }

        return OGRE_NEW Entity( id, objectMemoryManager, manager, pMesh );
    }
    //-----------------------------------------------------------------------
    void EntityFactory::destroyInstance( MovableObject* obj)
    {
        OGRE_DELETE obj;
    }

}
}
