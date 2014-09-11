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
#include "OgreSkeletonInstance.h"
#include "OgreAnimationState.h"
#include "OgreOptimisedUtil.h"
#include "OgreSceneNode.h"
#include "OgreStringConverter.h"
#include "OgreCamera.h"
#include "OgreException.h"
#include "OgreNameGenerator.h"

namespace Ogre
{
	NameGenerator InstancedEntity::msNameGenerator("");

	InstancedEntity::InstancedEntity( InstanceBatch *batchOwner, uint32 instanceID, InstancedEntity* sharedTransformEntity ) :
				MovableObject(),
				mInstanceId( instanceID ),
                mInUse( false ),
				mBatchOwner( batchOwner ),
				mAnimationState( 0 ),
				mSkeletonInstance( 0 ),
				mBoneMatrices(0),
				mBoneWorldMatrices(0),
				mFrameAnimationLastUpdated(std::numeric_limits<unsigned long>::max() - 1),
				mSharedTransformEntity( 0 ),
				mTransformLookupNumber(instanceID),
				mPosition(Vector3::ZERO),
				mDerivedLocalPosition(Vector3::ZERO),
				mOrientation(Quaternion::IDENTITY),
				mScale(Vector3::UNIT_SCALE),
				mMaxScaleLocal(1),
				mNeedTransformUpdate(true),
				mNeedAnimTransformUpdate(true),
				mUseLocalTransform(false)

	
	{
		//Use a static name generator to ensure this name stays unique (which may not happen
		//otherwise due to reparenting when defragmenting)
		mName = batchOwner->getName() + "/InstancedEntity_" + StringConverter::toString(mInstanceId) + "/"+
				msNameGenerator.generate();

		if (sharedTransformEntity)
		{
			sharedTransformEntity->shareTransformWith(this);
		}
		else
		{
			createSkeletonInstance();
		}
		updateTransforms();
	}

	InstancedEntity::~InstancedEntity()
	{
		unlinkTransform();
		destroySkeletonInstance();
	}

	bool InstancedEntity::shareTransformWith( InstancedEntity *slave )
	{
		if( !this->mBatchOwner->_getMeshRef()->hasSkeleton() ||
			this->mBatchOwner->_getMeshRef()->getSkeleton().isNull() ||
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

		if( this->mBatchOwner->_getMeshRef()->getSkeleton() !=
			slave->mBatchOwner->_getMeshRef()->getSkeleton() )
		{
			OGRE_EXCEPT( Exception::ERR_INVALID_STATE, "Sharing transforms requires both instanced"
											" entities to have the same skeleton",
											"InstancedEntity::shareTransformWith" );
			return false;
		}

		slave->unlinkTransform();
		slave->destroySkeletonInstance();
		
		slave->mSkeletonInstance	= this->mSkeletonInstance;
		slave->mAnimationState		= this->mAnimationState;
		slave->mBoneMatrices		= this->mBoneMatrices;
		if (mBatchOwner->useBoneWorldMatrices())
		{
			slave->mBoneWorldMatrices	= this->mBoneWorldMatrices;
		}
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
				*xform = mBatchOwner->useBoneWorldMatrices() ? 
						_getParentNodeFullTransform() : Matrix4::IDENTITY;
			}
			else
			{
				Matrix4* matrices = mBatchOwner->useBoneWorldMatrices() ? mBoneWorldMatrices : mBoneMatrices;
				const Mesh::IndexMap *indexMap = mBatchOwner->_getIndexToBoneMap();
				Mesh::IndexMap::const_iterator itor = indexMap->begin();
				Mesh::IndexMap::const_iterator end  = indexMap->end();

				while( itor != end )
					*xform++ = matrices[*itor++];

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
				const Matrix4& mat = mBatchOwner->useBoneWorldMatrices() ? 
					_getParentNodeFullTransform() : Matrix4::IDENTITY;
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

			//Object's bounding box is viewed by the camera
			if( retVal && camera )
				retVal = camera->isVisible(Sphere(_getDerivedPosition(),getBoundingRadius()));
		}

		return retVal;
	}
	//-----------------------------------------------------------------------
	void InstancedEntity::createSkeletonInstance()
	{
		//Is mesh skeletally animated?
		if( mBatchOwner->_getMeshRef()->hasSkeleton() &&
			!mBatchOwner->_getMeshRef()->getSkeleton().isNull() &&
			mBatchOwner->_supportsSkeletalAnimation() )
		{
			mSkeletonInstance = OGRE_NEW SkeletonInstance( mBatchOwner->_getMeshRef()->getSkeleton() );
			mSkeletonInstance->load();

			mBoneMatrices		= static_cast<Matrix4*>(OGRE_MALLOC_SIMD( sizeof(Matrix4) *
																	mSkeletonInstance->getNumBones(),
																	MEMCATEGORY_ANIMATION));
			if (mBatchOwner->useBoneWorldMatrices())
			{
				mBoneWorldMatrices	= static_cast<Matrix4*>(OGRE_MALLOC_SIMD( sizeof(Matrix4) *
																	mSkeletonInstance->getNumBones(),
																	MEMCATEGORY_ANIMATION));
			}

			mAnimationState = OGRE_NEW AnimationStateSet();
			mBatchOwner->_getMeshRef()->_initAnimationState( mAnimationState );
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

			OGRE_DELETE mSkeletonInstance;
			OGRE_DELETE mAnimationState;
			OGRE_FREE_SIMD( mBoneMatrices, MEMCATEGORY_ANIMATION );
			OGRE_FREE_SIMD( mBoneWorldMatrices, MEMCATEGORY_ANIMATION );

			mSkeletonInstance	= 0;
			mAnimationState		= 0;
			mBoneMatrices		= 0;
			mBoneWorldMatrices	= 0;
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

			mSkeletonInstance	= 0;
			mAnimationState		= 0;
			mBoneMatrices		= 0;
			mBoneWorldMatrices	= 0;
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
    Real InstancedEntity::getBoundingRadius(void) const
	{
		return mBatchOwner->_getMeshReference()->getBoundingSphereRadius() * getMaxScaleCoef();
	}
	//-----------------------------------------------------------------------
	Real InstancedEntity::getSquaredViewDepth( const Camera* cam ) const
	{
		return _getDerivedPosition().squaredDistance(cam->getDerivedPosition());
	}
	//-----------------------------------------------------------------------
	void InstancedEntity::_notifyMoved(void)
	{
		markTransformDirty();
		MovableObject::_notifyMoved();
		updateTransforms();
	}

	//-----------------------------------------------------------------------
	void InstancedEntity::_notifyAttached( Node* parent, bool isTagPoint )
	{
		markTransformDirty();
		MovableObject::_notifyAttached( parent, isTagPoint );
		updateTransforms();
	}
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
		if (mSharedTransformEntity)
		{
			return mSharedTransformEntity->_updateAnimation();
		}
		else
		{
			const bool animationDirty =
				(mFrameAnimationLastUpdated != mAnimationState->getDirtyFrameNumber()) ||
				(mSkeletonInstance->getManualBonesDirty());

			if( animationDirty || (mNeedAnimTransformUpdate &&  mBatchOwner->useBoneWorldMatrices()))
			{
				mSkeletonInstance->setAnimationState( *mAnimationState );
				mSkeletonInstance->_getBoneMatrices( mBoneMatrices );

				// Cache last parent transform for next frame use too.
				if (mBatchOwner->useBoneWorldMatrices())
				{
					OptimisedUtil::getImplementation()->concatenateAffineMatrices(
													_getParentNodeFullTransform(),
													mBoneMatrices,
													mBoneWorldMatrices,
													mSkeletonInstance->getNumBones() );
					mNeedAnimTransformUpdate = false;
				}
				
				mFrameAnimationLastUpdated = mAnimationState->getDirtyFrameNumber();

				return true;
			}
		}

		return false;
	}

	//-----------------------------------------------------------------------
	void InstancedEntity::markTransformDirty()
	{
		mNeedTransformUpdate = true;
		mNeedAnimTransformUpdate = true; 
		mBatchOwner->_boundsDirty();
	}

	//---------------------------------------------------------------------------
	void InstancedEntity::setPosition(const Vector3& position, bool doUpdate) 
	{ 
		mPosition = position; 
		mDerivedLocalPosition = position;
		mUseLocalTransform = true;
		markTransformDirty();
		if (doUpdate) updateTransforms();
	} 

	//---------------------------------------------------------------------------
	void InstancedEntity::setOrientation(const Quaternion& orientation, bool doUpdate) 
	{ 
		mOrientation = orientation;  
		mUseLocalTransform = true;
		markTransformDirty();
		if (doUpdate) updateTransforms();
	} 

	//---------------------------------------------------------------------------
	void InstancedEntity::setScale(const Vector3& scale, bool doUpdate) 
	{ 
		mScale = scale; 
		mMaxScaleLocal = std::max<Real>(std::max<Real>(
			Math::Abs(mScale.x), Math::Abs(mScale.y)), Math::Abs(mScale.z)); 
		mUseLocalTransform = true;
		markTransformDirty();
		if (doUpdate) updateTransforms();
	} 

	//---------------------------------------------------------------------------
	Real InstancedEntity::getMaxScaleCoef() const 
	{ 
		if (mParentNode)
		{
			const Ogre::Vector3& parentScale = mParentNode->_getDerivedScale();
			return mMaxScaleLocal * std::max<Real>(std::max<Real>(
				Math::Abs(parentScale.x), Math::Abs(parentScale.y)), Math::Abs(parentScale.z)); 
		}
		return mMaxScaleLocal; 
	}

	//---------------------------------------------------------------------------
	void InstancedEntity::updateTransforms()
	{
		if (mUseLocalTransform && mNeedTransformUpdate)
		{
			if (mParentNode)
			{
				const Vector3& parentPosition = mParentNode->_getDerivedPosition();
				const Quaternion& parentOrientation = mParentNode->_getDerivedOrientation();
				const Vector3& parentScale = mParentNode->_getDerivedScale();
				
				Quaternion derivedOrientation = parentOrientation * mOrientation;
				Vector3 derivedScale = parentScale * mScale;
				mDerivedLocalPosition = parentOrientation * (parentScale * mPosition) + parentPosition;
			
				mFullLocalTransform.makeTransform(mDerivedLocalPosition, derivedScale, derivedOrientation);
			}
			else
			{
				mFullLocalTransform.makeTransform(mPosition,mScale,mOrientation);
			}
			mNeedTransformUpdate = false;
		}
	}

	//---------------------------------------------------------------------------
	void InstancedEntity::setInUse( bool used )
	{
		mInUse = used;
		//Remove the use of local transform if the object is deleted
		mUseLocalTransform &= used;
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
