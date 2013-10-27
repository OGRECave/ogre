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

#ifndef __SkeletonDef_H__
#define __SkeletonDef_H__

#include "OgreSkeletonAnimation.h"
#include "OgreSkeletonAnimationDef.h"
#include "OgreIdString.h"
#include "OgreRawPtr.h"

namespace Ogre
{
	class SkeletonDef : public MovableAlloc
	{
		friend class SkeletonInstance;
	public:
		struct BoneData
		{
			size_t		index;
			size_t		parent;
			Vector3		vPos;
			Quaternion	qRot;
			Vector3		vScale;
			String		name;
			uint8		bInheritOrientation:1;
			uint8		bInheritScale:1;

			BoneData( size_t _index, size_t _parent, const Vector3 &_pos, const Quaternion &_rot,
						const Vector3 &_scale, const String &_name,
						bool _inheritOrientation, bool _inheritScale ) :
				index( _index ),
				parent( _parent ),
				vPos( _pos ),
				qRot( _rot ),
				vScale( _scale ),
				name( _name ),
				bInheritOrientation( _inheritOrientation ),
				bInheritScale( _inheritScale )
			{
			}
		};
		typedef vector<BoneData>::type BoneDataVec;

		struct DepthLevelInfo
		{
			size_t	firstBoneIndex;
			size_t	numBonesInLevel;
			DepthLevelInfo() : firstBoneIndex( -1 ), numBonesInLevel( 0 ) {}
		};

		typedef vector<DepthLevelInfo>::type DepthLevelInfoVec;

	protected:
		typedef map<IdString, size_t>::type BoneNameMap;

		BoneDataVec				mBones;
		BoneNameMap				mBoneIndexByName;
		SkeletonAnimationDefVec mAnimationDefs;

		RawSimdUniquePtr<KfTransform, MEMCATEGORY_ANIMATION> mBindPose;
		RawSimdUniquePtr<KfTransform, MEMCATEGORY_ANIMATION> mReverseBindPose;

		DepthLevelInfoVec		mDepthLevelInfoVec;

		/// Cached for SkeletonInstance::mUnusedNodes
		size_t					mNumUnusedSlots;

		vector<list<size_t>::type>::type mBonesPerDepth;

	public:
		/** Constructs this Skeleton based on the old format's Skeleton. The frameRate parameter
			indicates at which framerate it was recorded (i.e. 15fps, 25fps) so that all keyframe
			time values end up rounded.
		@remarks
			If the framerate information has been lost, set it to 1.
		*/
		SkeletonDef( const Skeleton *originalSkeleton, Real frameRate );

		const BoneDataVec& getBones(void) const							{ return mBones; }
		const SkeletonAnimationDefVec& getAnimationDefs(void) const		{ return mAnimationDefs; }
		const DepthLevelInfoVec& getDepthLevelInfo(void) const			{ return mDepthLevelInfoVec; }
		const KfTransform * RESTRICT_ALIAS getBindPose(void) const		{ return mBindPose.get(); }

		/** Returns the total number of bone blocks to reach the given level. i.e On SSE2,
			If the skeleton has 1 root node, 3 children, and 5 children of children;
			then the total number of blocks is 1 + 1 + 2 = 4
		@param numLevels
			Level depth to reach. Must be in range [0; mDepthLevelInfoVec.size]
		*/
		size_t getNumberOfBoneBlocks( size_t numLevels ) const;
	};
}

#endif
