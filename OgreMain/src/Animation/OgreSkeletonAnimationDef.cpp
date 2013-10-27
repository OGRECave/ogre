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

#include "Animation/OgreSkeletonAnimationDef.h"

#include "Math/Array/OgreMathlib.h"
#include "Math/Array/OgreTransform.h"
#include "Math/Array/OgreKfTransformArrayMemoryManager.h"

#include "OgreAnimation.h"
#include "OgreOldBone.h"
#include "OgreSkeleton.h"

namespace Ogre
{
	SkeletonAnimationDef::SkeletonAnimationDef() :
		mNumFrames( 0 ),
		mKfTransformMemoryManager( 0 ),
		mOriginalFrameRate( 25.0f )
	{
	}
	//-----------------------------------------------------------------------------------
	SkeletonAnimationDef::~SkeletonAnimationDef()
	{
		mTracks.clear();

		if( mKfTransformMemoryManager )
		{
			mKfTransformMemoryManager->destroy();
			delete mKfTransformMemoryManager;
			mKfTransformMemoryManager = 0;
		}
	}
	//-----------------------------------------------------------------------------------
	inline uint32 SkeletonAnimationDef::slotToBlockIdx( uint32 slotIdx ) const
	{
		return (slotIdx & 0xFF000000) | ((slotIdx & 0x00FFFFFF) / ARRAY_PACKED_REALS);
	}
	//-----------------------------------------------------------------------------------
	void SkeletonAnimationDef::build( const Skeleton *skeleton, const Animation *animation, Real frameRate )
	{
		mOriginalFrameRate = frameRate;
		mNumFrames = animation->getLength() * frameRate;

		//Terminology:
		// Bone Index:
		//	It's the bone index given by skeleton. i.e. getBone( boneIdx )
		// Slot Index
		//	It's the place in SoA slots the bone will end up with, with an index containing
		//	the parent level in the high 8 bits.
		//			For example the first root bone is 0
		//			The first bone child of root is (1 << 24) | (0 & 0x00FFFFF)
		//			The second bone child of root is (1 << 24) | (1 & 0x00FFFFF)
		//			The first bone child of child of root is (2 << 24) | (0 & 0x00FFFFF)
		//
		// Block Index.
		//	It's the same as slot index, but the slot part (low 24 bits) is divided by ARRAY_PACKED_REALS
		//	For example, ARRAY_PACKED_REALS = 4, slots 0 1 2 & 3 are in block 0

		//Converts Bone Index -> Slot Index
		vector<uint32>::type boneToSlot;
		boneToSlot.reserve( skeleton->getNumBones() );

		//Converts Slot Index -> Bone Index
		map<uint32, uint32>::type slotToBone;

		for( size_t i=0; i<skeleton->getNumBones(); ++i )
		{
			const OldBone *bone = skeleton->getBone( i );

			size_t depthLevel = 0;
			OldNode const *parentBone = bone;
			while( (parentBone = parentBone->getParent()) )
				++depthLevel;

			size_t offset = 0;
			vector<size_t>::type::const_iterator itor = boneToSlot.begin();
			vector<size_t>::type::const_iterator end  = boneToSlot.end();
			while( itor != end )
			{
				if( (*itor >> 24) == depthLevel )
					++offset;
				++itor;
			}

			//Build the map that lets us know the final slot bone index that will be
			//assigned to this bone (to get the block we still need to divide by ARRAY_PACKED_REALS)
			boneToSlot.push_back( (depthLevel << 24) | (offset & 0x00FFFFFF) );
			slotToBone[boneToSlot.back()] = i;
		}

		//1st Pass: Count the number of keyframes, so we know how
		//much memory to allocate, as we don't listen for resizes.
		//We also build a list of unique keyframe timestamps per block
		//(i.e. merge the keyframes from two bones that the same block)
		Animation::NodeTrackIterator itor = animation->getNodeTrackIterator();
		{
			//Count the number of blocks needed by counting the number of unique keyframes per block.
			//i.e. When ARRAY_PACKED_REALS = 4; if 2 bones are in the same block and have the same
			//keyframe time value, we'll need 4 slots (aka. 1 block). If those 2 bones have two
			//different time values, we'll need 8 slots.
			TimestampVec emptyVec;
			TimestampsPerBlock timestampsByBlock;

			while( itor.hasMoreElements() )
			{
				size_t boneIdx				= itor.peekNextKey();
				NodeAnimationTrack *track	= itor.getNext();

				if( track->getNumKeyFrames() > 0 )
				{
					uint32 slotIdx = boneToSlot[boneIdx];
					uint32 blockIdx = slotToBlockIdx( slotIdx );

					TimestampsPerBlock::iterator itKeyframes = timestampsByBlock.find( blockIdx );
					if( itKeyframes == timestampsByBlock.end() )
					{
						itKeyframes = timestampsByBlock.insert(
											std::make_pair( (size_t)blockIdx, emptyVec ) ).first;
					}

					itKeyframes->second.reserve( track->getNumKeyFrames() );

					for( size_t i=0; i<track->getNumKeyFrames(); ++i )
					{
						Real timestamp = track->getKeyFrame(i)->getTime();
						TimestampVec::iterator it = std::lower_bound( itKeyframes->second.begin(),
																		itKeyframes->second.end(),
																		timestamp );
						if( it == itKeyframes->second.end() || *it != timestamp )
							itKeyframes->second.insert( it, timestamp );
					}

					size_t trackDiff = std::distance( timestampsByBlock.begin(), itKeyframes );
					mBoneToWeights[skeleton->getBone( boneIdx )->getName()] =
										(boneToSlot[boneIdx] & 0xFF000000) |
										(trackDiff * ARRAY_PACKED_REALS + slotIdx) & 0x00FFFFFF;
				}
			}

			//Now that we've built the list of unique keyframes, allocate the mTracks and its keyframes
			allocateCacheFriendlyKeyframes( timestampsByBlock, frameRate );
		}

		//Set now all the transforms the allocated space
		SkeletonTrackVec::iterator itTrack = mTracks.begin();
		SkeletonTrackVec::iterator enTrack = mTracks.end();

		while( itTrack != enTrack )
		{
			KeyFrameRigVec &keyFrames = itTrack->_getKeyFrames();
			KeyFrameRigVec::iterator itKeys = keyFrames.begin();
			KeyFrameRigVec::iterator enKeys = keyFrames.end();

			uint32 blockIdx = itTrack->getBoneBlockIdx();

			while( itKeys != enKeys )
			{
				Real fTime = itKeys->mFrame * frameRate;

				for( size_t i=0; i<ARRAY_PACKED_REALS; ++i )
				{
					uint32 slotIdx = blockIdx + i;

					uint32 boneIdx = -1;
					map<uint32, uint32>::type::const_iterator it = slotToBone.find( slotIdx );
					if( it != slotToBone.end() )
						boneIdx = it->second;

					if( animation->hasNodeTrack( boneIdx ) )
					{
						NodeAnimationTrack *oldTrack = animation->getNodeTrack( boneIdx );

						TransformKeyFrame originalKF( 0, fTime );
						oldTrack->getInterpolatedKeyFrame( animation->_getTimeIndex( fTime ),
															&originalKF );

						itKeys->mBoneTransform->mPosition.setFromVector3( originalKF.getTranslate(), i );
						itKeys->mBoneTransform->mOrientation.setFromQuaternion( originalKF.getRotation(),
																				i );
						itKeys->mBoneTransform->mScale.setFromVector3( originalKF.getScale(), i );
						itTrack->_setMaxUsedSlot( i );
					}
					else
					{
						//This bone is not affected at all by the animation, but is
						//part of the block. Fill with identity transform
						itKeys->mBoneTransform->mPosition.setFromVector3( Vector3::ZERO, i );
						itKeys->mBoneTransform->mOrientation.setFromQuaternion( Quaternion::IDENTITY, i );
						itKeys->mBoneTransform->mScale.setFromVector3( Vector3::UNIT_SCALE, i );
					}
				}

				++itKeys;
			}

			itTrack->_bakeUnusedSlots();

			++itTrack;
		}
	}
	//-----------------------------------------------------------------------------------
	void SkeletonAnimationDef::allocateCacheFriendlyKeyframes(
											const TimestampsPerBlock &timestampsByBlock, Real frameRate )
	{
		assert( !mKfTransformMemoryManager );

		size_t numKeyFrames = 0;
		TimestampsPerBlock::const_iterator itor = timestampsByBlock.begin();
		TimestampsPerBlock::const_iterator end  = timestampsByBlock.end();

		while( itor != end )
		{
			numKeyFrames += itor->second.size();
			++itor;
		}

		mKfTransformMemoryManager = new KfTransformArrayMemoryManager(
											0, numKeyFrames * ARRAY_PACKED_REALS,
											-1, numKeyFrames * ARRAY_PACKED_REALS );
		mKfTransformMemoryManager->initialize();

		mTracks.reserve( timestampsByBlock.size() );

		//1st pass: All first keyframes.
		itor = timestampsByBlock.begin();
		while( itor != end )
		{
			size_t blockIdx = itor->first;
			mTracks.push_back( SkeletonTrack( blockIdx, mKfTransformMemoryManager ) );

			mTracks.back().addKeyFrame( *itor->second.begin(), frameRate );
			++itor;
		}

		vector<size_t>::type keyframesDone; //One per block
		keyframesDone.resize( timestampsByBlock.size(), 1 );

		//2nd pass: The in-between keyframes.
		bool addedAny = true;
		while( addedAny )
		{
			addedAny = false;

			//Keep adding keyframes until we're done with them
			size_t i = 0;
			itor = timestampsByBlock.begin();
			while( itor != end )
			{
				if( keyframesDone[i] < itor->second.size() - 1 )
				{
					//First find if there are key frames (J0) from the next blocks whose
					//next keyframe (J1) is before ours (I0), and add those instead (J0).
					//If not, add ours.
					Real timestampI0 = itor->second[keyframesDone[i]];

					bool subAddedAny = false;
					size_t j = i+1;
					TimestampsPerBlock::const_iterator itor2 = itor;
					++itor2;
					while( itor2 != end )
					{
						if( keyframesDone[j] < itor2->second.size() - 1 )
						{
							Real timestampJ1 = itor2->second[keyframesDone[j] + 1];
							if( timestampJ1 <= timestampI0 )
							{
								Real timestampJ0 = itor2->second[keyframesDone[j]];
								mTracks[j].addKeyFrame( timestampJ0, frameRate );
								++keyframesDone[j];
								addedAny = subAddedAny = true;
							}
						}

						++j;
						++itor2;
					}

					//Don't add ours if a previous track's keyframe (H0) is between
					//our keyframe I0 and our next keyframe I1. We'll add both together
					//in the next iteration
					bool dontAdd = false;
					Real timestampI1 = itor->second[keyframesDone[i] + 1];
					j = 0;
					itor2 = timestampsByBlock.begin();
					while( itor2 != itor && !dontAdd )
					{
						if( keyframesDone[j] < itor2->second.size() - 1 )
						{
							Real timestampJ0 = itor2->second[keyframesDone[j]];
							if( timestampI1 > timestampJ0 && timestampI0 < timestampJ0 )
							{
								dontAdd = true;
								addedAny = true; //There still a reason to iterate again
							}
						}
						++j;
						++itor2;
					}

					if( !subAddedAny && !dontAdd )
					{
						mTracks[i].addKeyFrame( timestampI0, frameRate );
						++keyframesDone[i];
						addedAny = true;
					}
				}

				++i;
				++itor;
			}
		}

		//3rd Pass: Create the last key frames
		size_t i = 0;
		itor = timestampsByBlock.begin();
		while( itor != end )
		{
			if( itor->second.size() > 1 )
			{
				mTracks[i].addKeyFrame( *(itor->second.end()-1), frameRate );
				++keyframesDone[i];
			}
			++i;
			++itor;
		}
	}
}
