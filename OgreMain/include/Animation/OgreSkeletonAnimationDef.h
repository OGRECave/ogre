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

#ifndef __SkeletonAnimationDef_H__
#define __SkeletonAnimationDef_H__

#include "OgreSkeletonTrack.h"
#include "OgreIdString.h"

namespace Ogre
{
    namespace v1
    {
        class TimeIndex;
    }

    class _OgreExport SkeletonAnimationDef : public AnimationAlloc
    {
        friend class SkeletonAnimation;
    protected:
        SkeletonTrackVec    mTracks;
        /** Number of frames. May not equal the number of keyframes
            (i.e. remain stationary at the end for a long time).
        */
        Real                mNumFrames;
        Real                mOriginalFrameRate;
        /** Converts bone index to consecutive slot (@see SkeletonAnimation::mBoneWeights).
            The parent level depth is in the last 8 bits
        */
        map<IdString, size_t>::type mBoneToWeights;
        String              mName;

        SkeletonDef const   *mSkeletonDef;

        KfTransformArrayMemoryManager *mKfTransformMemoryManager;

        typedef vector<Real>::type TimestampVec;
        typedef map<size_t, TimestampVec>::type TimestampsPerBlock;

        /** Same as @see OldNodeAnimationTrack::getInterpolatedKeyFrame, but doesn't normalize
            the interpolated quaternion, otherwise the rotation speed would be completely
            changed after we create additional keyframes to maintain compatibility within
            the SIMD block.
        */
        static void getInterpolatedUnnormalizedKeyFrame( v1::OldNodeAnimationTrack *oldTrack,
                                                         const v1::TimeIndex& timeIndex,
                                                         v1::TransformKeyFrame* kf );

        /** Allocates enough memory in mKfTransformMemoryManager, creates all the mTracks
            (one per each entry in timestampsByBlock), and allocates all the keyframes
            from each track in a cache friendly manner, according to the usage pattern
            we'll be going to do.
        @param timestampsByBlock
            A map with block index as key; and as value an array of unique time stamps
            (a time stamp = one keyframe)
        @param frameRate
            The original recording framerate.
        */
        void allocateCacheFriendlyKeyframes( const TimestampsPerBlock &timestampsByBlock,
                                             Real frameRate );

    public:
        SkeletonAnimationDef();
        ~SkeletonAnimationDef();

        void setName( const String &name )                              { mName = name; }
        const String& getNameStr(void) const                            { return mName; }
        void _setSkeletonDef( const SkeletonDef *skeletonDef )          { mSkeletonDef = skeletonDef; }

        void build( const v1::Skeleton *skeleton, const v1::Animation *animation, Real frameRate );

        /// Dumps all the tracks in CSV format to the output string argument.
        /// Mostly for debugging purposes. (also easy example to show how to
        /// enumerate all the tracks and get the bones back from its block index)
        void _dumpCsvTracks( String &outText ) const;
    };

    typedef vector<SkeletonAnimationDef>::type SkeletonAnimationDefVec;
}

#endif
