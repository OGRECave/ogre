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

#ifndef __AnimationState_H__
#define __AnimationState_H__

#include "OgrePrerequisites.h"

#include "OgreCommon.h"
#include "OgreController.h"
#include "OgreControllerManager.h"
#include "Threading/OgreThreadHeaders.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {

    template <typename T> class MapIterator;
    template <typename T> class ConstMapIterator;
    template <typename T> class ConstVectorIterator;

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Animation
    *  @{
    */

    /** Represents the state of an animation and the weight of its influence. 

        Other classes can hold instances of this class to store the state of any animations
        they are using.
    */
    class _OgreExport AnimationState : public AnimationAlloc
    {
    public:

        /// Typedef for an array of float values used as a bone blend mask
        typedef std::vector<float> BoneBlendMask;

        /** Normal constructor with all params supplied
            @param
                animName The name of this state.
            @param
                parent The parent AnimationStateSet that this state will belong to.
            @param
                timePos The position, in seconds, where this state will begin.
            @param
                length The length, in seconds, of this animation state.
            @param
                weight Weight to apply the animation state with.
            @param
                enabled Whether the animation state is enabled.
        */
        AnimationState(const String& animName, AnimationStateSet *parent, 
            Real timePos, Real length, Real weight = 1.0, bool enabled = false);
        /// Constructor to copy from an existing state with new parent
        AnimationState(AnimationStateSet* parent, const AnimationState &rhs);
        
        /// Gets the name of the animation to which this state applies
        const String& getAnimationName() const;
        /// Gets the time position for this animation
        Real getTimePosition(void) const;
        /// Sets the time position for this animation
        void setTimePosition(Real timePos);
        /// Gets the total length of this animation (may be shorter than whole animation)
        Real getLength() const;
        /// Sets the total length of this animation (may be shorter than whole animation)
        void setLength(Real len);
        /// Gets the weight (influence) of this animation
        Real getWeight(void) const;
        /// Sets the weight (influence) of this animation
        void setWeight(Real weight);
        /** Modifies the time position, adjusting for animation length
        @param offset The amount of time, in seconds, to extend the animation.

        This method loops at the edges if animation looping is enabled.
        */
        void addTime(Real offset);

        /// Returns true if the animation has reached the end and is not looping
        bool hasEnded(void) const;

        /// Returns true if this animation is currently enabled
        bool getEnabled(void) const;
        /// Sets whether this animation is enabled
        void setEnabled(bool enabled);

        /// Equality operator
        bool operator==(const AnimationState& rhs) const;
        /// Inequality operator
        bool operator!=(const AnimationState& rhs) const;

        /** Sets whether or not an animation loops at the start and end of
            the animation if the time continues to be altered.
        */
        void setLoop(bool loop) { mLoop = loop; }
        /// Gets whether or not this animation loops            
        bool getLoop(void) const { return mLoop; }
     
        /** Copies the states from another animation state, preserving the animation name
        (unlike operator=) but copying everything else.
        @param animState Reference to animation state which will use as source.
        */
        void copyStateFrom(const AnimationState& animState);

        /// Get the parent animation state set
        AnimationStateSet* getParent(void) const { return mParent; }

      /** @brief Create a new blend mask with the given number of entries
       *
       * In addition to assigning a single weight value to a skeletal animation,
       * it may be desirable to assign animation weights per bone using a 'blend mask'.
       *
       * @param blendMaskSizeHint 
       *   The number of bones of the skeleton owning this AnimationState.
       * @param initialWeight
       *   The value all the blend mask entries will be initialised with (negative to skip initialisation)
       */
      void createBlendMask(size_t blendMaskSizeHint, float initialWeight = 1.0f);
      /// Destroy the currently set blend mask
      void destroyBlendMask();

      /** @brief Set the blend mask
       *
       * @par The size of the array should match the number of entries the
       *      blend mask was created with.
       *
       * @par Stick to the setBlendMaskEntry method if you don't know exactly what you're doing.
       */
      void _setBlendMask(const BoneBlendMask* blendMask);
      /// Get the current blend mask
      const BoneBlendMask* getBlendMask() const {return &mBlendMask;}
      /// Return whether there is currently a valid blend mask set
      bool hasBlendMask() const {return !mBlendMask.empty();}
      /// Set the weight for the bone identified by the given handle
      void setBlendMaskEntry(size_t boneHandle, float weight);
      /// Get the weight for the bone identified by the given handle
      inline float getBlendMaskEntry(size_t boneHandle) const
      {
          assert(mBlendMask.size() > boneHandle);
          return mBlendMask[boneHandle];
      }
    private:
        /** @brief Set the blend mask data (might be dangerous)
         *
         * @par The size of the array should match the number of entries the
         *      blend mask was created with.
         *
         * @par Stick to the setBlendMaskEntry method if you don't know exactly what you're doing.
         */
        void _setBlendMaskData(const float* blendMaskData);
        /// The blend mask (containing per bone weights)
        BoneBlendMask mBlendMask;

        String mAnimationName;
        AnimationStateSet* mParent;
        Real mTimePos;
        Real mLength;
        Real mWeight;
        bool mEnabled;
        bool mLoop;

    };

    // A map of animation states
    typedef std::map<String, AnimationState*> AnimationStateMap;
    typedef MapIterator<AnimationStateMap> AnimationStateIterator;
    typedef ConstMapIterator<AnimationStateMap> ConstAnimationStateIterator;
    // A list of enabled animation states
    typedef std::list<AnimationState*> EnabledAnimationStateList;
    typedef ConstVectorIterator<EnabledAnimationStateList> ConstEnabledAnimationStateIterator;

    /** Class encapsulating a set of AnimationState objects.
    */
    class _OgreExport AnimationStateSet : public AnimationAlloc
    {
    public:
        /// Mutex, public for external locking if needed
            OGRE_AUTO_MUTEX;
        /// Create a blank animation state set
        AnimationStateSet();
        /// Create an animation set by copying the contents of another
        AnimationStateSet(const AnimationStateSet& rhs);

        ~AnimationStateSet();

        /** Create a new AnimationState instance. 
        @param animName The name of the animation
        @param timePos Starting time position
        @param length Length of the animation to play
        @param weight Weight to apply the animation with 
        @param enabled Whether the animation is enabled
        */
        AnimationState* createAnimationState(const String& animName,  
            Real timePos, Real length, Real weight = 1.0, bool enabled = false);
        /// Get an animation state by the name of the animation
        AnimationState* getAnimationState(const String& name) const;
        /// Tests if state for the named animation is present
        bool hasAnimationState(const String& name) const;
        /// Remove animation state with the given name
        void removeAnimationState(const String& name);
        /// Remove all animation states
        void removeAllAnimationStates(void);

        /** Get an iterator over all the animation states in this set.
        @deprecated use getAnimationStates()
        */
        AnimationStateIterator getAnimationStateIterator(void);
        /** Get an iterator over all the animation states in this set.
        @deprecated use getAnimationStates()
        */
        OGRE_DEPRECATED ConstAnimationStateIterator getAnimationStateIterator(void) const;

        /** Get all the animation states in this set.
        @note
            This method is not threadsafe,
            you will need to manually lock the public mutex on this
            class to ensure thread safety if you need it.
        */
        const AnimationStateMap& getAnimationStates() const {
            return mAnimationStates;
        }

        /// Copy the state of any matching animation states from this to another
        void copyMatchingState(AnimationStateSet* target) const;
        /// Set the dirty flag and dirty frame number on this state set
        void _notifyDirty(void);
        /// Get the latest animation state been altered frame number
        unsigned long getDirtyFrameNumber(void) const { return mDirtyFrameNumber; }

        /// Internal method respond to enable/disable an animation state
        void _notifyAnimationStateEnabled(AnimationState* target, bool enabled);
        /// Tests if exists enabled animation state in this set
        bool hasEnabledAnimationState(void) const { return !mEnabledAnimationStates.empty(); }
        /** Get an iterator over all the enabled animation states in this set
        @deprecated use getEnabledAnimationStates()
        */
        OGRE_DEPRECATED ConstEnabledAnimationStateIterator getEnabledAnimationStateIterator(void) const;

        /** Get an iterator over all the enabled animation states in this set
        @note
            The iterator returned from this method is not threadsafe,
            you will need to manually lock the public mutex on this
            class to ensure thread safety if you need it.
        */
        const EnabledAnimationStateList& getEnabledAnimationStates() const {
            return mEnabledAnimationStates;
        }

    private:
        unsigned long mDirtyFrameNumber;
        AnimationStateMap mAnimationStates;
        EnabledAnimationStateList mEnabledAnimationStates;

    };

    /** ControllerValue wrapper class for AnimationState.

        In Azathoth and earlier, AnimationState was a ControllerValue but this
        actually causes memory problems since Controllers delete their values
        automatically when there are no further references to them, but AnimationState
        is deleted explicitly elsewhere so this causes double-free problems.
        This wrapper acts as a bridge and it is this which is destroyed automatically.
    */
    class _OgreExport AnimationStateControllerValue : public ControllerValue<float>
    {
    private:
        AnimationState* mTargetAnimationState;
        bool mAddTime;
    public:
        /// @deprecated use create instead
        AnimationStateControllerValue(AnimationState* targetAnimationState, bool addTime = false)
            : mTargetAnimationState(targetAnimationState), mAddTime(addTime) {}

        /**
         * create an instance of this class
         * @param targetAnimationState
         * @param addTime if true, increment time instead of setting to an absolute position
         */
        static ControllerValueRealPtr create(AnimationState* targetAnimationState, bool addTime = false);

        /** ControllerValue implementation. */
        float getValue(void) const override
        {
            return mTargetAnimationState->getTimePosition() / mTargetAnimationState->getLength();
        }

        /** ControllerValue implementation. */
        void setValue(float value) override
        {
            if(mAddTime)
                mTargetAnimationState->addTime(value);
            else
                mTargetAnimationState->setTimePosition(value * mTargetAnimationState->getLength());
        }
    };

    /** @} */   
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif

