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

#ifndef __SkeletonAnimation_H__
#define __SkeletonAnimation_H__

#include "OgreSkeletonTrack.h"
#include "OgreIdString.h"

#include "OgreRawPtr.h"

#include "OgreHeaderPrefix.h"

namespace Ogre
{
    class SkeletonAnimationDef;
    class SkeletonInstance;

    typedef vector<KeyFrameRigVec::const_iterator>::type KnownKeyFramesVec;

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Animation
    *  @{
    */

    /// Represents the instance of a Skeletal animation based on its definition
    class _OgreExport SkeletonAnimation : public MovableAlloc
    {
        SkeletonAnimationDef const  *mDefinition;
    protected:
        RawSimdUniquePtr<ArrayReal, MEMCATEGORY_ANIMATION> mBoneWeights;
        Real                    mCurrentFrame;
    public:
        Real                    mFrameRate;     // Playback framerate
        Real                    mWeight;
        FastArray<size_t> const *mSlotStarts;   // One per parent depth level
        bool                    mLoop;
        bool                    mEnabled;
        SkeletonInstance        *mOwner;
    protected:
        IdString                mName;

        /// One per track
        KnownKeyFramesVec       mLastKnownKeyFrames;

    public:
        SkeletonAnimation( const SkeletonAnimationDef *definition, const FastArray<size_t> *slotStarts,
                            SkeletonInstance *owner );

        /// Internal function that initializes a lot of structures that can't be done in the
        /// constructor due to how SkeletonInstance is created/pushed in a vector.
        /// If you're not an Ogre dev, don't call this directly.
        void _initialize(void);

        /** Plays the animation forward (or backwards if negative)
        @param time
            Time to advance, in seconds
        */
        void addTime( Real time )                                   { addFrame( time * mFrameRate ); }

        /** Plays the animation forward (or backwards if negative)
        @param frames
            Frames to advance, in frames
        */
        void addFrame( Real frames );

        /** Sets the animation to a particular time.
        @param time
            Time to set to, in seconds
        */
        void setTime( Real time )                                   { setFrame( time * mFrameRate ); }

        /** Sets the animation to a particular frame.
        @param frames
            Frame to set to, in frames
        */
        void setFrame( Real frame );

        /// Gets the current animation time, in seconds. Prefer using getCurrentFrame
        Real getCurrentTime(void) const                      { return mCurrentFrame / mFrameRate; }

        /// Gets the current animation frame, in frames.
        Real getCurrentFrame(void) const                     { return mCurrentFrame; }

        /// Gets the frame count.
        Real getNumFrames(void) const;

        /// Gets animation length, in seconds.
        Real getDuration(void) const;

        IdString getName(void) const                                { return mName; }

        /** Loop setting. Looped animations will wrap back to zero when reaching the animation length
            or go back to the animation length if playing backwards.
            Non-looped animations will stop at the animation length (or at 0 if backwards) but won't
            be disabled.
        */
        void setLoop( bool bLoop )                                  { mLoop = bLoop; }

        /** Returns current loop setting. @See setLoop.
        */
        bool getLoop(void) const                                    { return mLoop; }

        /** Sets the per-bone weight to a particular bone. Useful for fine control
            over animation strength on a set of nodes (i.e. an arm)
        @remarks
            By default all bone weights are set to 1.0
        @param boneName
            The name of the bone to set. If this animation doesn't affect that bone (or the
            name is invalid) this function does nothing.
        @param weight
            Weight to apply to this particular bone. Note that the animation multiplies this
            value against the global mWeight to obtain the final weight.
            Normal range is between [0; 1] but not necessarily.
        */
        void setBoneWeight( IdString boneName, Real weight );

        /** Gets the current per-bone weight of a particular bone.
        @param boneName
            The name of the bone to get. If this animation doesn't affect that bone (or the
            name is invalid) this function returns 0.
        @return
            The weight of the specified bone. 0 if not found.
        */
        Real getBoneWeight( IdString boneName ) const;

        /** Gets a pointer current per-bone weight of a particular bone. Useful if you intend
            to have read/write access to this value very often.
        @remarks
            If returnPtr is the return value to bone[0], do not assume that returnPtr+1
            affects bone[1] or even any other bone. Doing so the behavior is underfined
            and most likely you could be affecting the contents of other SkeletonInstances.
        @param boneName
            The name of the bone to get. If this animation doesn't affect that bone (or the
            name is invalid) this function returns a null pointer.
        @return
            The pointer to the bone weight of the specified bone. Null pointer if not found.
        */
        Real* getBoneWeightPtr( IdString boneName );

        /// Enables or disables this animation. A disabled animation won't be processed at all.
        void setEnabled( bool bEnable );
        bool getEnabled(void) const                                 { return mEnabled; }

        void _applyAnimation( const TransformArray &boneTransforms );

        void _swapBoneWeightsUniquePtr( RawSimdUniquePtr<ArrayReal, MEMCATEGORY_ANIMATION>
                                        &inOutBoneWeights );

        const SkeletonAnimationDef* getDefinition(void) const       { return mDefinition; }
    };

    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif
