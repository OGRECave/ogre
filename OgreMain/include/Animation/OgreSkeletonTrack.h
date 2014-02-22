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

#ifndef __SkeletonTrack_H__
#define __SkeletonTrack_H__

#include "OgrePrerequisites.h"
#include "Math/Array/OgreArrayQuaternion.h"
#include "Math/Array/OgreKfTransform.h"

namespace Ogre
{
    class KfTransformArrayMemoryManager;

    struct KeyFrameRig
    {
        Real mFrame;
        Real mInvNextFrameDistance; // 1.0f / (KeyFrameRig[1].mFrame - KeyFrameRig[0].mFrame)

        // SoA variable. Packs posrotscale posrotscale ...
        KfTransform * RESTRICT_ALIAS mBoneTransform;
    };

    typedef vector<KeyFrameRig>::type KeyFrameRigVec;

    typedef FastArray<BoneTransform> TransformArray;

    class _OgreExport SkeletonTrack : public AnimationAlloc
    {
    protected:
        /// There is one entry per each parent level
        KeyFrameRigVec      mKeyFrameRigs;
        Real                mNumFrames;

        uint32              mBoneBlockIdx;

        /** Number of SIMD slots used by all @see KeyFrameRig::mBoneTransform.
            When this value is <= (ARRAY_PACKED_REALS / 2); then we repeat the transforms
            to the next slots. i.e: 
            mUsedSlots = 2;
            mBoneTransform[0] = Bone Transform A
            mBoneTransform[1] = Bone Transform B
            mBoneTransform[2] = mBoneTransform[0]
            mBoneTransform[3] = mBoneTransform[1]

            mUsedSlots = 1;
            mBoneTransform[0] = Bone Transform A
            mBoneTransform[1] = mBoneTransform[0]
            mBoneTransform[2] = mBoneTransform[0]
            mBoneTransform[3] = mBoneTransform[0]
        */
        uint32              mUsedSlots;

        KfTransformArrayMemoryManager *mLocalMemoryManager;

    public:
        SkeletonTrack( uint32 boneBlockIdx, KfTransformArrayMemoryManager *kfTransformMemoryManager );
        ~SkeletonTrack();

        void setNumKeyFrame( size_t numKeyFrames );

        void addKeyFrame( Real timestamp, Real frameRate );
        void setKeyFrameTransform( Real frame, uint32 slot, const Vector3 &vPos,
                                    const Quaternion &qRot, const Vector3 vScale );

        uint32 getBoneBlockIdx(void) const                      { return mBoneBlockIdx; }
        size_t getUsedSlots(void) const                         { return mUsedSlots; }
        void _setMaxUsedSlot( uint32 slot )
                                        { mUsedSlots = std::max( slot+1, mUsedSlots ); }

        const KeyFrameRigVec& getKeyFrames(void) const          { return mKeyFrameRigs; }
        KeyFrameRigVec& _getKeyFrames(void)                     { return mKeyFrameRigs; }

        inline void getKeyFrameRigAt( KeyFrameRigVec::const_iterator &inOutPrevFrame,
                                        KeyFrameRigVec::const_iterator &outNextFrame,
                                        Real frame ) const;

        /** Applies the interpolated keyframe at the given frame to all bone
            transformations that are animated by this Track.
        @param inOutLastKnownKeyFrame [in/out]
            Hint to this system on where to start next. If unknown (i.e. first call) set
            it to mKeyFrameRigs.begin()
            As output it will be set to the current keyframe, and should be used as input
            for the next call to keep keyframe searchs in constant time.
        @param frame [in]
            Frame number, should be in range [0; mNumFrames]; if below zero the result
            will look like clamped to zero, if above mNumFrames, it result will look
            like clamped to mNumFrames
        @param perBoneWeights [in]
            A list of per bone weights. The size of the array must be 1 (or more)
        @param KfTransforms [out]
            An array with all bone transformations, sorted by parent level.
            The key frames are only applied to affected bones.
        */
        void applyKeyFrameRigAt( KeyFrameRigVec::const_iterator &inOutLastKnownKeyFrame, float frame,
                                 ArrayReal animWeight, const ArrayReal * RESTRICT_ALIAS perBoneWeights,
                                 const TransformArray &KfTransforms ) const;

        /** Takes all KeyFrames and repeats the KfTransforms for every unused slot by a pattern
            based on the number of used slots. Only useful when
            mUsedSlots <= (ARRAY_PACKED_REALS >> 1). Otherwise it does nothing.
        */
        void _bakeUnusedSlots(void);
    };

    typedef vector<SkeletonTrack>::type SkeletonTrackVec;
}

#endif
