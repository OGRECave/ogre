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

#include "Animation/OgreSkeletonDef.h"
#include "Animation/OgreSkeletonAnimationDef.h"
#include "Animation/OgreBone.h"
#include "Math/Array/OgreNodeMemoryManager.h"
#include "Math/Array/OgreKfTransformArrayMemoryManager.h"

#include "OgreId.h"

#include "OgreOldBone.h"
#include "OgreSkeleton.h"

namespace Ogre
{
	SkeletonDef::SkeletonDef( const Skeleton *originalSkeleton, Real frameRate ) : mNumUnusedSlots( 0 )
	{
		mBones.reserve( originalSkeleton->getNumBones() );

		//Clone the bone data
		size_t numDepthLevels = 1;
		Skeleton::ConstBoneIterator itor = originalSkeleton->getBoneIteratorConst();
		while( itor.hasMoreElements() )
		{
			OldBone *bone = itor.getNext();
			size_t parentIndex = -1;
			if( bone->getParent() )
			{
				assert( !bone->getParent() || dynamic_cast<OldBone*>( bone->getParent() ) );
				OldBone *parent = static_cast<OldBone*>( bone->getParent() );
				parentIndex = parent->getHandle();

				size_t tmpDepthLevels = 2;
				OldNode *tmpParent = parent;
				while( (tmpParent = tmpParent->getParent()) )
					++tmpDepthLevels;
				numDepthLevels = std::max( numDepthLevels, tmpDepthLevels );
			}

			BoneData boneData( bone->getHandle(), parentIndex, bone->getPosition(),
								bone->getOrientation(), bone->getScale(),
								bone->getName(), bone->getInheritOrientation(),
								bone->getInheritScale() );
			mBoneIndexByName[bone->getName()] = mBones.size();
			mBones.push_back( boneData );
		}

		mBonesPerDepth.resize( numDepthLevels );
		mDepthLevelInfoVec.resize( numDepthLevels );
		for( size_t i=0; i<mBones.size(); ++i )
		{
			size_t currentDepthLevel = 0;
			BoneData const *tmpBone = &mBones[i];
			while( tmpBone->parent != std::numeric_limits<size_t>::max() )
			{
				tmpBone = &mBones[tmpBone->parent];
				++currentDepthLevel;
			}

			DepthLevelInfo &info = mDepthLevelInfoVec[currentDepthLevel];
			info.firstBoneIndex = std::min( i, info.firstBoneIndex );
			++info.numBonesInLevel;

			mBonesPerDepth[currentDepthLevel].push_back( i );
		}

		//Clone the animations
		mAnimationDefs.resize( originalSkeleton->getNumAnimations() );
		for( size_t i=0; i<originalSkeleton->getNumAnimations(); ++i )
		{
			mAnimationDefs[i].setName( originalSkeleton->getAnimation( i )->getName() );
			mAnimationDefs[i].build( originalSkeleton, originalSkeleton->getAnimation( i ), frameRate );
		}

		//Create the bones (just like we would for SkeletonInstance)so we can
		//get derived position/rotation/scale and then calculate its inverse
		NodeMemoryManager nodeMemoryManager;
		vector<Bone>::type boneNodes( mBones.size(), Bone( Transform() ) );
		vector<list<size_t>::type>::type::const_iterator itDepth = mBonesPerDepth.begin();
		vector<list<size_t>::type>::type::const_iterator enDepth = mBonesPerDepth.end();

		while( itDepth != enDepth )
		{
			list<size_t>::type::const_iterator itor = itDepth->begin();
			list<size_t>::type::const_iterator end  = itDepth->end();

			while( itor != end )
			{
				Bone *parent = 0;
				size_t parentIdx = mBones[*itor].parent;
				const BoneData &boneData = mBones[*itor];

				if( parentIdx != std::numeric_limits<size_t>::max() )
					parent = &boneNodes[parentIdx];

				boneNodes[*itor].~Bone();
				new (&boneNodes[*itor]) Bone( Id::generateNewId<Node>(), (SceneManager*)0,
												&nodeMemoryManager, parent, 0 );
				Bone &newBone = boneNodes[*itor];
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

		//Calculate in bulk. Calling getDerivedPositionUpdated & Co like
		//we need would need, has O(N!) complexity.
		for( size_t i=0; i<nodeMemoryManager.getNumDepths(); ++i )
		{
			Transform t;
			const size_t numNodes = nodeMemoryManager.getFirstNode( t, i );
			Node::updateAllTransforms( numNodes, t );
		}

		{
			size_t numBoneBlocks = getNumberOfBoneBlocks( mDepthLevelInfoVec.size() );

			//Create initial bind pose.
			mBindPose = RawSimdUniquePtr<KfTransform, MEMCATEGORY_ANIMATION>( numBoneBlocks );
			RawSimdUniquePtr<KfTransform, MEMCATEGORY_ANIMATION> derivedPosesPtr =
						RawSimdUniquePtr<KfTransform, MEMCATEGORY_ANIMATION>( numBoneBlocks );

			size_t bindPoseIndex = 0;
			KfTransform *bindPose = mBindPose.get();
			KfTransform *derivedPose = derivedPosesPtr.get();

			size_t currentDepthLevel = 0;
			DepthLevelInfoVec::const_iterator itor = mDepthLevelInfoVec.begin();
			DepthLevelInfoVec::const_iterator end  = mDepthLevelInfoVec.end();

			while( itor != end )
			{
				list<size_t>::type::const_iterator itBoneIdx = mBonesPerDepth[currentDepthLevel].begin();
				list<size_t>::type::const_iterator enBoneIdx = mBonesPerDepth[currentDepthLevel].end();
				while( itBoneIdx != enBoneIdx )
				{
					const BoneData &boneData = mBones[*itBoneIdx];
					bindPose->mPosition.setFromVector3( boneData.vPos, bindPoseIndex );
					bindPose->mOrientation.setFromQuaternion( boneData.qRot, bindPoseIndex );
					bindPose->mScale.setFromVector3( boneData.vScale, bindPoseIndex );

					const Bone &derivedBone = boneNodes[*itBoneIdx];
					derivedPose->mPosition.setFromVector3(
										derivedBone._getDerivedPosition(), bindPoseIndex );
					derivedPose->mOrientation.setFromQuaternion(
										derivedBone._getDerivedOrientation(), bindPoseIndex );
					derivedPose->mScale.setFromVector3(
										derivedBone._getDerivedScale(), bindPoseIndex );

					++bindPoseIndex;
					if( bindPoseIndex >= ARRAY_PACKED_REALS )
					{
						bindPoseIndex = 0;
						++bindPose;
						++derivedPose;
					}

					++itBoneIdx;
				}

				if( itor->numBonesInLevel <= (ARRAY_PACKED_REALS >> 1) )
				{
					//Repeat the slots in patterns to save memory when instances get created
					//Do the same in SkeletonTrack::_bakeUnusedSlots
					size_t k=0;
					for( size_t j=itor->numBonesInLevel; j<ARRAY_PACKED_REALS; ++j )
					{
						Vector3 vTmp;
						Quaternion qTmp;
						bindPose->mPosition.getAsVector3( vTmp, k );
						bindPose->mPosition.setFromVector3( vTmp, j );
						bindPose->mOrientation.getAsQuaternion( qTmp, k );
						bindPose->mOrientation.setFromQuaternion( qTmp, j );
						bindPose->mScale.getAsVector3( vTmp, k );
						bindPose->mScale.setFromVector3( vTmp, j );

						derivedPose->mPosition.getAsVector3( vTmp, k );
						derivedPose->mPosition.setFromVector3( vTmp, j );
						derivedPose->mOrientation.getAsQuaternion( qTmp, k );
						derivedPose->mOrientation.setFromQuaternion( qTmp, j );
						derivedPose->mScale.getAsVector3( vTmp, k );
						derivedPose->mScale.setFromVector3( vTmp, j );

						k = (k+1) % itor->numBonesInLevel;
					}

					bindPoseIndex = 0;
					++bindPose;
					++derivedPose;
				}
				else if( bindPoseIndex != 0 )
				{
					//We can't repeat to save memory per instance. But we still need
					//to create a hole so the next depth level starts in the first slot
					//of the next block
					for( ; bindPoseIndex<ARRAY_PACKED_REALS; ++bindPoseIndex )
					{
						bindPose->mPosition.setFromVector3( Vector3::ZERO, bindPoseIndex );
						bindPose->mOrientation.setFromQuaternion( Quaternion::IDENTITY, bindPoseIndex );
						bindPose->mScale.setFromVector3( Vector3::UNIT_SCALE, bindPoseIndex );

						derivedPose->mPosition.setFromVector3( Vector3::ZERO, bindPoseIndex );
						derivedPose->mOrientation.setFromQuaternion( Quaternion::IDENTITY,
																		 bindPoseIndex );
						derivedPose->mScale.setFromVector3( Vector3::UNIT_SCALE, bindPoseIndex );
					}

					bindPoseIndex = 0;
					++bindPose;
					++derivedPose;
				}

				++currentDepthLevel;
				++itor;
			}

			//Now set the reverse of the binding pose (so we can pass the
			//construct the derived transform matrices in the correct space)
			mReverseBindPose = RawSimdUniquePtr<KfTransform, MEMCATEGORY_ANIMATION>( numBoneBlocks );
			KfTransform *reverseBindPose = mReverseBindPose.get();
			derivedPose = derivedPosesPtr.get();
			for( size_t i=0; i<numBoneBlocks; ++i )
			{
				reverseBindPose[i].mPosition	= -derivedPose[i].mPosition;
				reverseBindPose[i].mOrientation	= derivedPose[i].mOrientation.Inverse();
				reverseBindPose[i].mScale		= derivedPose[i].mScale;
				reverseBindPose[i].mScale.inverseLeaveZeroes();
			}
		}

		{
			//Cache the amount of unused slots for the SkeletonInstance
			mNumUnusedSlots = 0;
			SkeletonDef::DepthLevelInfoVec::const_iterator itor = mDepthLevelInfoVec.begin();
			SkeletonDef::DepthLevelInfoVec::const_iterator end  = mDepthLevelInfoVec.end();
			while( itor != end )
			{
				if( itor->numBonesInLevel > (ARRAY_PACKED_REALS >> 1) )
				{
					size_t unusedSlots = ARRAY_PACKED_REALS -
											(itor->numBonesInLevel % ARRAY_PACKED_REALS);
					if( unusedSlots != ARRAY_PACKED_REALS )
						mNumUnusedSlots += unusedSlots;
				}
				++itor;
			}
		}
	}
	//-----------------------------------------------------------------------------------
	size_t SkeletonDef::getNumberOfBoneBlocks( size_t numLevels ) const
	{
		size_t numBlocks = 0;

		DepthLevelInfoVec::const_iterator itor = mDepthLevelInfoVec.begin();
		DepthLevelInfoVec::const_iterator end  = mDepthLevelInfoVec.begin() + numLevels;

		while( itor != end )
		{
			numBlocks += (itor->numBonesInLevel - 1 + ARRAY_PACKED_REALS) / ARRAY_PACKED_REALS;
			++itor;
		}

		return numBlocks;
	}
}
