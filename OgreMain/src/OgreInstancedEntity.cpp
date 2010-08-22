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
#include "OgreStableHeaders.h"
#include "OgreInstancedEntity.h"
#include "OgreInstanceBatch.h"
#include "OgreSkeletonInstance.h"
#include "OgreAnimationState.h"
#include "OgreOptimisedUtil.h"
#include "OgreSceneNode.h"
#include "OgreException.h"

namespace Ogre
{
	InstancedEntity::InstancedEntity( InstanceBatch *batchOwner, uint32 instanceID ) :
				m_batchOwner( batchOwner ),
				m_instanceID( instanceID ),
				m_skeletonInstance( 0 ),
				mAnimationState( 0 ),
				mLastParentXform( Matrix4::ZERO ),
				mFrameAnimationLastUpdated( std::numeric_limits<unsigned long>::max() )
	{
		mName = batchOwner->getName() + "/InstancedEntity_" + StringConverter::toString(m_instanceID);

		//Is mesh skeletally animated?
		if( m_batchOwner->_getMeshRef()->hasSkeleton() &&
			!m_batchOwner->_getMeshRef()->getSkeleton().isNull() )
		{
			m_skeletonInstance = OGRE_NEW SkeletonInstance( m_batchOwner->_getMeshRef()->getSkeleton() );
			m_skeletonInstance->load();

			mBoneMatrices		= static_cast<Matrix4*>(OGRE_MALLOC_SIMD( sizeof(Matrix4) *
																	m_skeletonInstance->getNumBones(),
																	MEMCATEGORY_ANIMATION));
			mBoneWorldMatrices	= static_cast<Matrix4*>(OGRE_MALLOC_SIMD( sizeof(Matrix4) *
																	m_skeletonInstance->getNumBones(),
																	MEMCATEGORY_ANIMATION));

			mAnimationState = OGRE_NEW AnimationStateSet();
			m_batchOwner->_getMeshRef()->_initAnimationState( mAnimationState );
		}
	}

	InstancedEntity::~InstancedEntity()
	{
		if( m_skeletonInstance )
		{
			OGRE_DELETE m_skeletonInstance;
			OGRE_DELETE mAnimationState;
			OGRE_FREE_SIMD( mBoneMatrices, MEMCATEGORY_ANIMATION );
			OGRE_FREE_SIMD( mBoneWorldMatrices, MEMCATEGORY_ANIMATION );
		}
	}

	const String& InstancedEntity::getMovableType(void) const
	{
		static String sType = "InstancedEntity";
		return sType;
	}
	//-----------------------------------------------------------------------
	size_t InstancedEntity::getTransforms( Matrix4 *xform )
	{
		size_t retVal = 1;

		//When not attached, returns zero matrix to avoid rendering this one, not identity
		if( mParentNode )
		{
			if( !m_skeletonInstance )
				*xform = mParentNode->_getFullTransform();
			else
			{
				const Mesh::IndexMap *indexMap = m_batchOwner->_getIndexToBoneMap();
				Mesh::IndexMap::const_iterator itor = indexMap->begin();
				Mesh::IndexMap::const_iterator end  = indexMap->end();

				while( itor != end )
					*xform++ = mBoneWorldMatrices[*itor++];

				retVal = indexMap->size();
			}
		}
		else
		{
			if( m_skeletonInstance )
				retVal = m_skeletonInstance->getNumBones();

			std::fill_n( xform, retVal, Matrix4::ZERO );
		}

		return retVal;
	}
	//-----------------------------------------------------------------------
	bool InstancedEntity::findVisible( Camera *camera )
	{
		//Object is explicitly visible and attached to a Node
		bool retVal = isVisible() & (mParentNode != 0);

		//Object's bounding box is viewed by the camera
		const SceneNode *parentSceneNode = getParentSceneNode();
		if( parentSceneNode && camera )
			retVal &= camera->isVisible( parentSceneNode->_getWorldAABB() );

		return retVal;
	}
	//-----------------------------------------------------------------------
    const AxisAlignedBox& InstancedEntity::getBoundingBox(void) const
    {
		//TODO: Add attached objects (TagPoints) to the bbox
		return m_batchOwner->_getMeshReference()->getBounds();
    }

	//-----------------------------------------------------------------------
    Real InstancedEntity::getBoundingRadius(void) const
	{
		Real rad = m_batchOwner->_getMeshReference()->getBoundingSphereRadius();
        // Scale by largest scale factor
        if(mParentNode )
        {
            const Vector3& s = mParentNode->_getDerivedScale();
			rad *=  std::max( Math::Abs(s.x), std::max( Math::Abs(s.y), Math::Abs(s.z) ) );
        }

		return rad;
	}
	//-----------------------------------------------------------------------
	Real InstancedEntity::getSquaredViewDepth( const Camera* cam ) const
	{
		Real retVal = std::numeric_limits<Real>::infinity();

		if( mParentNode )
			retVal = mParentNode->getSquaredViewDepth( cam );

		return retVal;
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
	void InstancedEntity::_updateAnimation(void)
	{
		const bool animationDirty =
            (mFrameAnimationLastUpdated != mAnimationState->getDirtyFrameNumber()) ||
            (m_skeletonInstance->getManualBonesDirty());

		if( animationDirty || mLastParentXform != _getParentNodeFullTransform() )
        {
			m_skeletonInstance->setAnimationState( *mAnimationState );
			m_skeletonInstance->_getBoneMatrices( mBoneMatrices );

			// Cache last parent transform for next frame use too.
			mLastParentXform = _getParentNodeFullTransform();
			OptimisedUtil::getImplementation()->concatenateAffineMatrices(
												mLastParentXform,
												mBoneMatrices,
												mBoneWorldMatrices,
												m_skeletonInstance->getNumBones() );

			mFrameAnimationLastUpdated = mAnimationState->getDirtyFrameNumber();
		}
	}
}
