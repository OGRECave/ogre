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
#include "OgreStableHeaders.h"
#include "OgreInstancedEntity.h"
#include "OgreInstanceBatch.h"
#include "OgreOldSkeletonInstance.h"
#include "OgreAnimationState.h"
#include "OgreOptimisedUtil.h"
#include "OgreCamera.h"
#include "OgreSceneManager.h"
#include "OgreException.h"

#include "Math/Array/OgreBooleanMask.h"

namespace Ogre
{
namespace v1
{
    InstancedEntity::InstancedEntity(IdType id, ObjectMemoryManager *objectMemoryManager,
                                        InstanceBatch *batchOwner, uint32 instanceID,
                                 #ifndef OGRE_LEGACY_ANIMATIONS
                                        BoneMemoryManager *boneMemoryManager,
                                 #endif
                                        InstancedEntity* sharedTransformEntity ) :
                MovableObject( id, objectMemoryManager, (SceneManager*)0, 1 ),
                mInstanceId( instanceID ),
                mInUse( false ),
                mBatchOwner( batchOwner ),
#ifdef OGRE_LEGACY_ANIMATIONS
                mAnimationState( 0 ),
                mSkeletonInstance( 0 ),
                mBoneMatrices(0),
                mBoneWorldMatrices(0),
                mFrameAnimationLastUpdated(std::numeric_limits<unsigned long>::max() - 1),
#else
                mSkeletonInstance( 0 ),
                mBoneMemoryManager( boneMemoryManager ),
#endif
                mSharedTransformEntity( 0 ),
                mTransformLookupNumber(instanceID)
    {
        setInUse( false );

        mName = batchOwner->getName() + "/IE_" + StringConverter::toString(mInstanceId);

        const AxisAlignedBox &bounds = batchOwner->_getMeshReference()->getBounds();
        Aabb aabb( bounds.getCenter(), bounds.getHalfSize() );
        mObjectData.mLocalAabb->setFromAabb( aabb, mObjectData.mIndex );
        mObjectData.mLocalRadius[mObjectData.mIndex] = aabb.getRadius();

        if (sharedTransformEntity)
        {
            sharedTransformEntity->shareTransformWith(this);
        }
        else
        {
            createSkeletonInstance();
        }
    }

    InstancedEntity::~InstancedEntity()
    {
        unlinkTransform();
        destroySkeletonInstance();
    }

    bool InstancedEntity::shareTransformWith( InstancedEntity *slave )
    {
        if( !this->mBatchOwner->_getMeshRef()->hasSkeleton() ||
            this->mBatchOwner->_getMeshRef()->getOldSkeleton().isNull() ||
            !this->mBatchOwner->_supportsSkeletalAnimation() )
        {
            return false;
        }

        if( this->mSharedTransformEntity  )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE, "Attempted to share '" + mName + "' transforms "
                                            "with slave '" + slave->mName + "' but '" + mName +"' is "
                                            "already sharing. Hierarchical sharing not allowed.",
                                            "InstancedEntity::shareTransformWith" );
            return false;
        }

        if( this->mBatchOwner->_getMeshRef()->getOldSkeleton() !=
            slave->mBatchOwner->_getMeshRef()->getOldSkeleton() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE, "Sharing transforms requires both instanced"
                                            " entities to have the same skeleton",
                                            "InstancedEntity::shareTransformWith" );
            return false;
        }

        slave->unlinkTransform();
        slave->destroySkeletonInstance();

#ifdef OGRE_LEGACY_ANIMATIONS
        slave->mSkeletonInstance    = this->mSkeletonInstance;
        slave->mAnimationState      = this->mAnimationState;
        slave->mBoneMatrices        = this->mBoneMatrices;
        if (mBatchOwner->useBoneWorldMatrices())
        {
            slave->mBoneWorldMatrices   = this->mBoneWorldMatrices;
        }
#else
        slave->mSkeletonInstance    = this->mSkeletonInstance;
#endif
        slave->mSharedTransformEntity = this;
        //The sharing partners are kept in the parent entity 
        this->mSharingPartners.push_back( slave );
        
        slave->mBatchOwner->_markTransformSharingDirty();

        return true;
    }
    //-----------------------------------------------------------------------
    void InstancedEntity::stopSharingTransform()
    {
        if( mSharedTransformEntity )
        {
            stopSharingTransformAsSlave( true );
        }
        else
        {
            //Tell the ones sharing skeleton with us to use their own
            InstancedEntityVec::const_iterator itor = mSharingPartners.begin();
            InstancedEntityVec::const_iterator end  = mSharingPartners.end();
            while( itor != end )
            {
                (*itor)->stopSharingTransformAsSlave( false );
                ++itor;
            }
            mSharingPartners.clear();
        }
    }
    //-----------------------------------------------------------------------
    const String& InstancedEntity::getMovableType(void) const
    {
        static String sType = "InstancedEntity";
        return sType;
    }
    //-----------------------------------------------------------------------
    size_t InstancedEntity::getTransforms( Matrix4 *xform ) const
    {
        size_t retVal = 1;

        //When not attached, returns zero matrix to avoid rendering this one, not identity
        if( isVisible() && isInScene() )
        {
            if( !mSkeletonInstance )
            {
                *xform = _getParentNodeFullTransform();
            }
            else
            {
#ifdef OGRE_LEGACY_ANIMATIONS
                Matrix4* matrices = mBatchOwner->useBoneWorldMatrices() ? mBoneWorldMatrices : mBoneMatrices;
                const Mesh::IndexMap *indexMap = mBatchOwner->_getIndexToBoneMap();
                Mesh::IndexMap::const_iterator itor = indexMap->begin();
                Mesh::IndexMap::const_iterator end  = indexMap->end();

                while( itor != end )
                    *xform++ = matrices[*itor++];
#else
                const Mesh::IndexMap *indexMap = mBatchOwner->_getIndexToBoneMap();
                Mesh::IndexMap::const_iterator itor = indexMap->begin();
                Mesh::IndexMap::const_iterator end  = indexMap->end();

                while( itor != end )
                    mSkeletonInstance->_getBoneFullTransform(*itor++).store( xform++ );
#endif
                retVal = indexMap->size();
            }
        }
        else
        {
            if( mSkeletonInstance )
                retVal = mBatchOwner->_getIndexToBoneMap()->size();

            std::fill_n( xform, retVal, Matrix4::ZEROAFFINE );
        }

        return retVal;
    }
    //-----------------------------------------------------------------------
    size_t InstancedEntity::getTransforms3x4( float *xform ) const
    {
        size_t retVal;
        //When not attached, returns zero matrix to avoid rendering this one, not identity
        if( isVisible() && isInScene() )
        {
            if( !mSkeletonInstance )
            {
                const Matrix4& mat = _getParentNodeFullTransform();
                for( int i=0; i<3; ++i )
                {
                    Real const *row = mat[i];
                    for( int j=0; j<4; ++j )
                        *xform++ = static_cast<float>( *row++ );
                }

                retVal = 12;
            }
            else
            {
#ifdef OGRE_LEGACY_ANIMATIONS
                Matrix4* matrices = mBatchOwner->useBoneWorldMatrices() ? mBoneWorldMatrices : mBoneMatrices;

                const Mesh::IndexMap *indexMap = mBatchOwner->_getIndexToBoneMap();
                Mesh::IndexMap::const_iterator itor = indexMap->begin();
                Mesh::IndexMap::const_iterator end  = indexMap->end();

                while( itor != end )
                {
                    const Matrix4 &mat = matrices[*itor++];
                    for( int i=0; i<3; ++i )
                    {
                        Real const *row = mat[i];
                        for( int j=0; j<4; ++j )
                            *xform++ = static_cast<float>( *row++ );
                    }
                }
#else
                const Mesh::IndexMap *indexMap = mBatchOwner->_getIndexToBoneMap();
                Mesh::IndexMap::const_iterator itor = indexMap->begin();
                Mesh::IndexMap::const_iterator end  = indexMap->end();

                while( itor != end )
                    mSkeletonInstance->_getBoneFullTransform(*itor++).store4x3( xform++ );
#endif

                retVal = indexMap->size() * 4 * 3;
            }
        }
        else
        {
            if( mSkeletonInstance )
                retVal = mBatchOwner->_getIndexToBoneMap()->size() * 3 * 4;
            else
                retVal = 12;
            
            std::fill_n( xform, retVal, 0.0f );
        }

        return retVal;
    }
    //-----------------------------------------------------------------------
    bool InstancedEntity::findVisible( Camera *camera ) const
    {
        //Object is active
        bool retVal = isInScene();
        if (retVal) 
        {
            //check object is explicitly visible
            retVal = isVisible();
        }

        return retVal;
    }
    //-----------------------------------------------------------------------
    void InstancedEntity::createSkeletonInstance()
    {
        //Is mesh skeletally animated?
        if( mBatchOwner->_getMeshRef()->hasSkeleton() &&
            !mBatchOwner->_getMeshRef()->getOldSkeleton().isNull() &&
            mBatchOwner->_supportsSkeletalAnimation() )
        {
#ifdef OGRE_LEGACY_ANIMATIONS
            mSkeletonInstance = OGRE_NEW OldSkeletonInstance( mBatchOwner->_getMeshRef()->getOldSkeleton() );
            mSkeletonInstance->load();

            mBoneMatrices       = static_cast<Matrix4*>(OGRE_MALLOC_SIMD( sizeof(Matrix4) *
                                                                    mSkeletonInstance->getNumBones(),
                                                                    MEMCATEGORY_ANIMATION));
            if (mBatchOwner->useBoneWorldMatrices())
            {
                mBoneWorldMatrices  = static_cast<Matrix4*>(OGRE_MALLOC_SIMD( sizeof(Matrix4) *
                                                                    mSkeletonInstance->getNumBones(),
                                                                    MEMCATEGORY_ANIMATION));
            }

            mAnimationState = OGRE_NEW AnimationStateSet();
            mBatchOwner->_getMeshRef()->_initAnimationState( mAnimationState );

            if( mParentNode )
                mBatchOwner->_addAnimatedInstance( this );
#else
            const SkeletonDef *skeletonDef = mBatchOwner->_getMeshRef()->getSkeleton().get();
            SceneManager *sceneManager = mBatchOwner->_getManager();
            mSkeletonInstance = sceneManager->createSkeletonInstance( skeletonDef );

            if( mBatchOwner->_supportsSkeletalAnimation() != InstanceBatch::SKELETONS_LUT )
                mSkeletonInstance->setParentNode( mParentNode );
#endif
        }
    }
    //-----------------------------------------------------------------------
    void InstancedEntity::destroySkeletonInstance()
    {
        if( mSkeletonInstance )
        {
            //Tell the ones sharing skeleton with us to use their own
            //sharing partners will remove themselves from notifyUnlink
            while( mSharingPartners.empty() == false )
            {
                mSharingPartners.front()->stopSharingTransform();
            }
            mSharingPartners.clear();

#ifndef OGRE_LEGACY_ANIMATIONS
            SceneManager *sceneManager = mBatchOwner->_getManager();
            sceneManager->destroySkeletonInstance( mSkeletonInstance );
#else
            OGRE_DELETE mSkeletonInstance;
            OGRE_DELETE mAnimationState;
            OGRE_FREE_SIMD( mBoneMatrices, MEMCATEGORY_ANIMATION );
            OGRE_FREE_SIMD( mBoneWorldMatrices, MEMCATEGORY_ANIMATION );

            mSkeletonInstance   = 0;
            mAnimationState     = 0;
            mBoneMatrices       = 0;
            mBoneWorldMatrices  = 0;
#endif
        }
    }
    //-----------------------------------------------------------------------
    void InstancedEntity::stopSharingTransformAsSlave( bool notifyMaster )
    {
        unlinkTransform( notifyMaster );
        createSkeletonInstance();
    }
    //-----------------------------------------------------------------------
    void InstancedEntity::unlinkTransform( bool notifyMaster )
    {
        if( mSharedTransformEntity )
        {
            //Tell our master we're no longer his slave
            if( notifyMaster )
                mSharedTransformEntity->notifyUnlink( this );
            mBatchOwner->_markTransformSharingDirty();

            mSkeletonInstance   = 0;
#ifdef OGRE_LEGACY_ANIMATIONS
            mAnimationState     = 0;
            mBoneMatrices       = 0;
            mBoneWorldMatrices  = 0;
#endif
            mSharedTransformEntity = 0;
        }
    }
    //-----------------------------------------------------------------------
    void InstancedEntity::notifyUnlink( const InstancedEntity *slave )
    {
        //Find the slave and remove it
        InstancedEntityVec::iterator itor = mSharingPartners.begin();
        InstancedEntityVec::iterator end  = mSharingPartners.end();
        while( itor != end )
        {
            if( *itor == slave )
            {
                std::swap(*itor,mSharingPartners.back());
                mSharingPartners.pop_back();
                break;
            }

            ++itor;
        }
    }
    //-----------------------------------------------------------------------
    const AxisAlignedBox& InstancedEntity::getBoundingBox(void) const
    {
        //TODO: Add attached objects (TagPoints) to the bbox
        return mBatchOwner->_getMeshReference()->getBounds();
    }
    //-----------------------------------------------------------------------
    Real InstancedEntity::getSquaredViewDepth( const Camera* cam ) const
    {
        return mParentNode->_getDerivedPosition().squaredDistance(cam->getDerivedPosition());
    }
    //-----------------------------------------------------------------------
    void InstancedEntity::_notifyStaticDirty(void) const
    {
        assert( mBatchOwner->isStatic() );
        mBatchOwner->_notifyStaticDirty();
    }
    //-----------------------------------------------------------------------
    void InstancedEntity::_notifyAttached( Node* parent )
    {
        bool different = (parent != mParentNode);

        if( different )
        {
#ifdef OGRE_LEGACY_ANIMATIONS
            if( parent )
            {
                //Check we're skeletally animated and actual owners of our transform
                if( !mSharedTransformEntity && mSkeletonInstance )
                    mBatchOwner->_addAnimatedInstance( this );
            }
            else
            {
                mBatchOwner->_removeAnimatedInstance( this );
            }
#else
            //Don't notify our skeleton instance when sharing, as we have to work in local space.
            if( mSkeletonInstance && !mSharedTransformEntity &&
                mBatchOwner->_supportsSkeletalAnimation() != InstanceBatch::SKELETONS_LUT )
            {
                mSkeletonInstance->setParentNode( parent );
            }
#endif

            if( isStatic() )
                _notifyStaticDirty();
        }

        MovableObject::_notifyAttached( parent );
    }
#ifndef OGRE_LEGACY_ANIMATIONS
    //-----------------------------------------------------------------------
    void InstancedEntity::_notifyParentNodeMemoryChanged(void)
    {
        if( mSkeletonInstance && !mSharedTransformEntity &&
            mBatchOwner->_supportsSkeletalAnimation() != InstanceBatch::SKELETONS_LUT )
        {
            mSkeletonInstance->setParentNode( mSkeletonInstance->getParentNode() );
        }
    }
#else
    //-----------------------------------------------------------------------
    AnimationState* InstancedEntity::getAnimationState(const String& name) const
    {
        if (!mAnimationState)
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Entity is not animated",
                "InstancedEntity::getAnimationState");
        }

        return mAnimationState->getAnimationState(name);
    }
    //-----------------------------------------------------------------------
    AnimationStateSet* InstancedEntity::getAllAnimationStates(void) const
    {
        return mAnimationState;
    }
    //-----------------------------------------------------------------------
    bool InstancedEntity::_updateAnimation(void)
    {
        //Probably this is triggered by a missing call to mBatchOwner->_removeAnimatedInstance
        assert( !mSharedTransformEntity && "Updating the animation of an entity with shared animation" );
        /*if (mSharedTransformEntity)
        {
            return mSharedTransformEntity->_updateAnimation();
        }
        else
        {*/
            const bool animationDirty =
                (mFrameAnimationLastUpdated != mAnimationState->getDirtyFrameNumber()) ||
                (mSkeletonInstance->getManualBonesDirty());

            if( animationDirty || mBatchOwner->useBoneWorldMatrices())
            {
                mSkeletonInstance->setAnimationState( *mAnimationState );
                mSkeletonInstance->_getBoneMatrices( mBoneMatrices );

                // Cache last parent transform for next frame use too.
                if (mBatchOwner->useBoneWorldMatrices())
                {
                    OptimisedUtil::getImplementation()->concatenateAffineMatrices(
                                                    mParentNode->_getFullTransform(),
                                                    mBoneMatrices,
                                                    mBoneWorldMatrices,
                                                    mSkeletonInstance->getNumBones() );
                }
                
                mFrameAnimationLastUpdated = mAnimationState->getDirtyFrameNumber();

                return true;
            }
        //}

        return false;
    }
#endif
    //---------------------------------------------------------------------------
    void InstancedEntity::setInUse( bool used )
    {
        mInUse = used;
        if( !used )
            setVisible( false );
    }
    //---------------------------------------------------------------------------
    void InstancedEntity::setCustomParam( unsigned char idx, const Vector4 &newParam )
    {
        mBatchOwner->_setCustomParam( this, idx, newParam );
    }
    //---------------------------------------------------------------------------
    const Vector4& InstancedEntity::getCustomParam( unsigned char idx )
    {
        return mBatchOwner->_getCustomParam( this, idx );
    }
}
}
