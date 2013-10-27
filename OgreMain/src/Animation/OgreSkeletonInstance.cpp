/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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

#include "Animation/OgreSkeletonInstance.h"
#include "Animation/OgreSkeletonDef.h"
#include "Animation/OgreSkeletonAnimationDef.h"

#include "OgreId.h"

#include "OgreOldBone.h"
#include "OgreSkeleton.h"

namespace Ogre
{
	SkeletonInstance::SkeletonInstance( const SkeletonDef *skeletonDef,
										NodeMemoryManager *nodeMemoryManager ) :
			mDefinition( skeletonDef )
	{
		mBones.resize( mDefinition->getBones().size(), Bone( Transform() ) );

		vector<list<size_t>::type>::type::const_iterator itDepth = mDefinition->mBonesPerDepth.begin();
		vector<list<size_t>::type>::type::const_iterator enDepth = mDefinition->mBonesPerDepth.end();

		while( itDepth != enDepth )
		{
			list<size_t>::type::const_iterator itor = itDepth->begin();
			list<size_t>::type::const_iterator end  = itDepth->end();

			while( itor != end )
			{
				Bone *parent = 0;
				size_t parentIdx = mDefinition->mBones[*itor].parent;
				const SkeletonDef::BoneData &boneData = mDefinition->mBones[*itor];

				if( parentIdx != std::numeric_limits<size_t>::max() )
					parent = &mBones[parentIdx];

				mBones[*itor].~Bone();
				new (&mBones[*itor]) Bone( Id::generateNewId<Node>(), (SceneManager*)0,
											nodeMemoryManager, parent, 0 );
				Bone &newBone = mBones[*itor];
				newBone.setPosition( boneData.vPos );
				newBone.setOrientation( boneData.qRot );
				newBone.setScale( boneData.vScale );
				newBone.setInheritOrientation( boneData.bInheritOrientation );
				newBone.setInheritScale( boneData.bInheritScale );
				newBone.setName( boneData.name );
				newBone.mGlobalIndex = *itor;
				
				if( parent )
					parent->_notifyOfChild( &newBone );

				++itor;
			}

			++itDepth;
		}

		{
			const SkeletonDef::DepthLevelInfoVec &depthLevelInfo = mDefinition->getDepthLevelInfo();
			mSlotStarts.reserve( depthLevelInfo.size() );
			mBoneStartTransforms.reserve( depthLevelInfo.size() );

			mManualBones = RawSimdUniquePtr<ArrayReal, MEMCATEGORY_ANIMATION>(
								mDefinition->getNumberOfBoneBlocks( depthLevelInfo.size() ) );
			Real *manualBones = reinterpret_cast<Real*>( mManualBones.get() );

			mUnusedNodes.resize( mDefinition->mNumUnusedSlots, Bone( Transform() ) );

			KfTransform const *reverseBindPose = mDefinition->mReverseBindPose.get();

			SkeletonDef::DepthLevelInfoVec::const_iterator itor = depthLevelInfo.begin();
			SkeletonDef::DepthLevelInfoVec::const_iterator end  = depthLevelInfo.end();

			while( itor != end )
			{
				const Transform &firstBoneTransform = mBones[itor->firstBoneIndex]._getTransform();
				mBoneStartTransforms.push_back( firstBoneTransform );
				mSlotStarts.push_back( firstBoneTransform.mIndex );

				assert( (itor->numBonesInLevel <= (ARRAY_PACKED_REALS >> 1)) ||
						!firstBoneTransform.mIndex );

				if( itor->numBonesInLevel > (ARRAY_PACKED_REALS >> 1) )
				{
					//TODO: Reserve enough space in mUnusedNodes (amount can be cached in SkeletonDef)
					size_t unusedSlots = ARRAY_PACKED_REALS -
											(itor->numBonesInLevel % ARRAY_PACKED_REALS);

					// When x is a multiple of ARRAY_PACKED_REALS, then the formula gives:
					//		ARRAY_PACKED_REALS - x % ARRAY_PACKED_REALS = ARRAY_PACKED_REALS;
					// but we don't need to create any unused node, as the slots are already
					// aligned to the block
					if( unusedSlots != ARRAY_PACKED_REALS )
					{
						for( size_t i=0; i<unusedSlots; ++i )
						{
							//Dummy bones need the right parent so they
							//consume memory from the right depth level
							Bone *parent = 0;
							if( itor != depthLevelInfo.begin() )
								parent = &mBones[itor->firstBoneIndex];

							mUnusedNodes[i].~Bone();
							new (&mUnusedNodes[i]) Bone( Id::generateNewId<Node>(), (SceneManager*)0,
														nodeMemoryManager, parent, 0 );
							Bone &unused = mUnusedNodes[i];
							unused.setName( "Unused" );
							unused.mGlobalIndex = i;

							if( parent )
								parent->_notifyOfChild( &unused );
						}
					}
				}

				//Prepare for default pose, 0.0f for manually animated or a slot that
				//doesn't belong to us, 1.0f when we should apply animation
				size_t slotStart = firstBoneTransform.mIndex;
				size_t remainder = (slotStart + itor->numBonesInLevel) % ARRAY_PACKED_REALS;
				for( size_t i=0; i<slotStart; ++i )
					*manualBones++ = 0.0f;
				for( size_t i=slotStart; i<slotStart + itor->numBonesInLevel; ++i )
					*manualBones++ = 1.0f;
				if( remainder != 0 )
				{
					for( size_t i=0; i<ARRAY_PACKED_REALS - remainder; ++i )
						*manualBones++ = 0.0f;
				}

				//Take advantage that all SceneNodes in mOwner are planar in memory
				Node **bonesPtr = firstBoneTransform.mOwner;
				for( size_t i=slotStart; i<slotStart + itor->numBonesInLevel; ++i )
				{
					assert( dynamic_cast<Bone*>( bonesPtr[i] ) );

					static_cast<Bone*>( bonesPtr[i] )->_setReverseBindPtr( reverseBindPose );
					if( !( (i+1) % ARRAY_PACKED_REALS) )
						++reverseBindPose;
				}

				if( remainder != 0 )
					++reverseBindPose;

				++itor;
			}
		}

		const SkeletonAnimationDefVec &animationDefs = mDefinition->getAnimationDefs();
		mAnimations.reserve( animationDefs.size() );

		SkeletonAnimationDefVec::const_iterator itor = animationDefs.begin();
		SkeletonAnimationDefVec::const_iterator end  = animationDefs.end();

		while( itor != end )
		{
			SkeletonAnimation animation( &(*itor), &mSlotStarts, this );
			mAnimations.push_back( animation );
			mAnimations.back().initialize();
			++itor;
		}
	}
	//-----------------------------------------------------------------------------------
	SkeletonInstance::~SkeletonInstance()
	{
		BoneVec::iterator itor = mBones.begin();
		BoneVec::iterator end  = mBones.end();

		while( itor != end )
		{
			itor->removeAllChildren();
			++itor;
		}

		mAnimations.clear();

		mUnusedNodes.clear(); //LIFO order: These were created last
		mBones.clear();
	}
	//-----------------------------------------------------------------------------------
	void SkeletonInstance::update(void)
	{
		resetToPose();
		ActiveAnimationsVec::iterator itor = mActiveAnimations.begin();
		ActiveAnimationsVec::iterator end  = mActiveAnimations.end();

		while( itor != end )
		{
			(*itor)->_applyAnimation( mBoneStartTransforms );
			++itor;
		}
	}
	//-----------------------------------------------------------------------------------
	void SkeletonInstance::resetToPose(void)
	{
		KfTransform const * RESTRICT_ALIAS bindPose = mDefinition->getBindPose();
		ArrayReal const * RESTRICT_ALIAS manualBones = mManualBones.get();

		SkeletonDef::DepthLevelInfoVec::const_iterator itDepthLevelInfo =
												mDefinition->getDepthLevelInfo().begin();

		TransformArray::iterator itor = mBoneStartTransforms.begin();
		TransformArray::iterator end  = mBoneStartTransforms.end();

		while( itor != end )
		{
			Transform t = *itor;
			for( size_t i=0; i<itDepthLevelInfo->numBonesInLevel; i += ARRAY_PACKED_REALS )
			{
				*t.mPosition = Math::lerp( *t.mPosition, bindPose->mPosition, *manualBones );
				*t.mOrientation = Math::lerp( *t.mOrientation, bindPose->mOrientation, *manualBones );
				*t.mScale = Math::lerp( *t.mScale, bindPose->mScale, *manualBones );
				t.advancePack();

				++bindPose;
				++manualBones;
			}

			++itor;
			++itDepthLevelInfo;
		}
	}
	//-----------------------------------------------------------------------------------
	void SkeletonInstance::setManualBone( SceneNode *bone, bool isManual )
	{
		assert( &mBones[bone->mGlobalIndex] == bone && "The bone doesn't belong to this instance!" );

		uint32 depthLevel = bone->getDepthLevel();
		size_t offset = mDefinition->getNumberOfBoneBlocks( depthLevel );
		SceneNode &firstBone = mBones[mDefinition->getDepthLevelInfo()[depthLevel].firstBoneIndex];

		uintptr_t diff = bone->_getTransform().mParents - firstBone._getTransform().mParents;
		Real *manualBones = reinterpret_cast<Real*>( mManualBones.get() );
		manualBones[diff] = isManual ? 1.0f : 0.0f;
	}
	//-----------------------------------------------------------------------------------
	bool SkeletonInstance::isManualBone( SceneNode *bone )
	{
		assert( &mBones[bone->mGlobalIndex] == bone && "The bone doesn't belong to this instance!" );

		uint32 depthLevel = bone->getDepthLevel();
		size_t offset = mDefinition->getNumberOfBoneBlocks( depthLevel );
		SceneNode &firstBone = mBones[mDefinition->getDepthLevelInfo()[depthLevel].firstBoneIndex];

		uintptr_t diff = bone->_getTransform().mParents - firstBone._getTransform().mParents;
		const Real *manualBones = reinterpret_cast<const Real*>( mManualBones.get() );
		return manualBones[diff] != 0.0f;
	}
	//-----------------------------------------------------------------------------------
	SceneNode* SkeletonInstance::getBone( IdString boneName )
	{
		SkeletonDef::BoneNameMap::const_iterator itor = mDefinition->mBoneIndexByName.find( boneName );

		if( itor == mDefinition->mBoneIndexByName.end() )
		{
			OGRE_EXCEPT( Exception::ERR_ITEM_NOT_FOUND,
							 "Can't find bone with name '" + boneName.getFriendlyText() + "'",
							 "SkeletonInstance::getBone" );
		}

		return &mBones[itor->second];
	}
	//-----------------------------------------------------------------------------------
	bool SkeletonInstance::hasAnimation( IdString name ) const
	{
		SkeletonAnimationVec::const_iterator itor = mAnimations.begin();
		SkeletonAnimationVec::const_iterator end  = mAnimations.end();

		while( itor != end && itor->getName() != name )
			++itor;

		return itor != end;
	}
	//-----------------------------------------------------------------------------------
	SkeletonAnimation* SkeletonInstance::getAnimation( IdString name )
	{
		SkeletonAnimationVec::iterator itor = mAnimations.begin();
		SkeletonAnimationVec::iterator end  = mAnimations.end();

		while( itor != end )
		{
			if( itor->getName() == name )
				return &(*itor);
			++itor;
		}

		OGRE_EXCEPT( Exception::ERR_ITEM_NOT_FOUND,
					 "Can't find animation '" + name.getFriendlyText() + "'",
					 "SkeletonInstance::getAnimation" );
		return 0;
	}
	//-----------------------------------------------------------------------------------
	void SkeletonInstance::_enableAnimation( SkeletonAnimation *animation )
	{
		mActiveAnimations.push_back( animation );
	}
	//-----------------------------------------------------------------------------------
	void SkeletonInstance::_disableAnimation( SkeletonAnimation *animation )
	{
		ActiveAnimationsVec::iterator it = std::find( mActiveAnimations.begin(), mActiveAnimations.end(),
														animation );
		if( it != mActiveAnimations.end() )
			efficientVectorRemove( mActiveAnimations, it );
	}
	//-----------------------------------------------------------------------------------
	void SkeletonInstance::getTransforms( Matrix4 * RESTRICT_ALIAS outTransform,
											const FastArray<unsigned short> &usedBones ) const
	{
		FastArray<unsigned short>::const_iterator itor = usedBones.begin();
		FastArray<unsigned short>::const_iterator end  = usedBones.end();

		while( itor != end )
		{
			*outTransform++ = mBones[*itor]._getFullTransform();
			++itor;
		}
	}
}
