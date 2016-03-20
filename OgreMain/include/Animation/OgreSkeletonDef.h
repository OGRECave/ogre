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

#ifndef __SkeletonDef_H__
#define __SkeletonDef_H__

#include "OgreSkeletonAnimation.h"
#include "OgreSkeletonAnimationDef.h"
#include "OgreIdString.h"
#include "OgreRawPtr.h"

#include "Math/Array/OgreArrayMatrixAf4x3.h"

namespace Ogre
{
    class _OgreExport SkeletonDef : public MovableAlloc
    {
        friend class SkeletonInstance;
    public:
        struct BoneData
        {
            size_t      index;
            size_t      parent;
            Vector3     vPos;
            Quaternion  qRot;
            Vector3     vScale;
            String      name;
            uint8       bInheritOrientation:1;
            uint8       bInheritScale:1;

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
            size_t  firstBoneIndex;
            size_t  numBonesInLevel;
            DepthLevelInfo() : firstBoneIndex( -1 ), numBonesInLevel( 0 ) {}
        };

        typedef vector<DepthLevelInfo>::type DepthLevelInfoVec;

        typedef map<uint32, uint32>::type IndexToIndexMap;
        typedef vector<uint32>::type BoneToSlotVec;

    protected:
        typedef map<IdString, size_t>::type BoneNameMap;

        BoneDataVec             mBones;
        BoneNameMap             mBoneIndexByName;
        SkeletonAnimationDefVec mAnimationDefs;

        /// This maps the slot index to a bone index for mBones[]. @see slotToBlockIdx
        /// Terminology:
        /// Bone Index:
        ///  It's the bone index given by skeleton. i.e. mBones[boneIdx]
        /// Slot Index
        ///  It's the place in SoA slots the bone will end up with, with an index containing
        ///  the parent level in the high 8 bits.
        ///     For example the given bone hierarchy:
        ///          A
        ///        /| |
        ///       / | |
        ///      B  C D
        ///     /
        ///     E
        ///     A is the first root at depth level 0; the index is 0
        ///     B is (1 << 24) | (0 & 0x00FFFFF); because it's the first bone in depth level 1
        ///     C is (1 << 24) | (1 & 0x00FFFFF); because it's the second bone in depth lv 1
        ///     D is (1 << 24) | (2 & 0x00FFFFF); because it's the third bone in depth lv 1
        ///     E is (2 << 24) | (0 & 0x00FFFFF); because it's the first bone in depth lv 2 (child of B)
        ///
        /// Block Index.
        ///  It's the same as slot index, but the slot part (low 24 bits) is divided by ARRAY_PACKED_REALS
        ///  For example, ARRAY_PACKED_REALS = 4, slots 0 1 2 & 3 are in block 0
        ///
        /// Converts Slot Index -> Bone Index
        IndexToIndexMap mSlotToBone;
        /// Converts Bone Index -> Slot Index
        BoneToSlotVec   mBoneToSlot;

        RawSimdUniquePtr<KfTransform, MEMCATEGORY_ANIMATION>        mBindPose;
        RawSimdUniquePtr<ArrayMatrixAf4x3, MEMCATEGORY_ANIMATION>   mReverseBindPose;

        DepthLevelInfoVec       mDepthLevelInfoVec;

        /// Cached for SkeletonInstance::mUnusedNodes
        size_t                  mNumUnusedSlots;

        vector<list<size_t>::type>::type mBonesPerDepth;

        String                  mName;

    public:
        /** Constructs this Skeleton based on the old format's Skeleton. The frameRate parameter
            indicates at which framerate it was recorded (i.e. 15fps, 25fps) so that all keyframe
            time values end up rounded.
        @remarks
            If the framerate information has been lost, set it to 1.
        */
        SkeletonDef( const v1::Skeleton *originalSkeleton, Real frameRate );

        const String& getNameStr(void) const                            { return mName; }

        const BoneDataVec& getBones(void) const                         { return mBones; }
        const SkeletonAnimationDefVec& getAnimationDefs(void) const     { return mAnimationDefs; }
        const DepthLevelInfoVec& getDepthLevelInfo(void) const          { return mDepthLevelInfoVec; }
        const KfTransform * getBindPose(void) const                     { return mBindPose.get(); }
        const RawSimdUniquePtr<ArrayMatrixAf4x3, MEMCATEGORY_ANIMATION>&
                                getReverseBindPose(void) const          { return mReverseBindPose; }
        void getBonesPerDepth( vector<size_t>::type &out ) const;

        /** Returns the total number of bone blocks to reach the given level. i.e On SSE2,
            If the skeleton has 1 root node, 3 children, and 5 children of children;
            then the total number of blocks is 1 + 1 + 2 = 4
        @param numLevels
            Level depth to reach. Must be in range [0; mDepthLevelInfoVec.size]
        */
        size_t getNumberOfBoneBlocks( size_t numLevels ) const;

        /** Converts a "Slot index" to a block index. @see mSlotToBone
            Slot indices contain the information about the bone's depth level in the
            hierarchy, and an index to the exact bone.
            Block indices also contain the depth level, but an index to the start of
            the SIMD block.
        @remarks
            Converting to a "block index" is a lossy process. Information pointing
            to the individual bone is lost; and only the start of the block is
            preserved.
            e.g. if ARRAY_PACKED_REALS = 4, and the slot index is pointing to the
            slot 3; the block index will reset the slot to 0.
            If the slot index is pointing to slot 6, the index will reset to slot 4,
            which is the first in the SIMD block
        @param slotIdx
            The slot index.
        @return
            The block index.
        */
        inline static uint32 slotToBlockIdx( uint32 slotIdx )
        {
            return (slotIdx & 0xFF000000) | ((slotIdx & 0x00FFFFFF) / ARRAY_PACKED_REALS);
        }

        /** Convertes a block index back to a slot index. However the slot points at the
            start of the SIMD block. @see slotToBlockIdx and @see mSlotToBone
        @param blockIdx
            The block index
        @return
            The slot index.
        */
        inline static uint32 blockIdxToSlotStart( uint32 blockIdx )
        {
            return (blockIdx & 0xFF000000) | ((blockIdx & 0x00FFFFFF) * ARRAY_PACKED_REALS);
        }

        /// @see mSlotToBone
        const IndexToIndexMap& getSlotToBone(void) const    { return mSlotToBone; }

        /// @see mBoneToSlot
        const BoneToSlotVec& getBoneToSlot(void) const      { return mBoneToSlot; }
    };
}

#endif
