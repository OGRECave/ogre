/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/

#ifndef __AnimationSet_H__
#define __AnimationSet_H__

#include "OgrePrerequisites.h"

#include "OgreString.h"
#include "OgreController.h"
#include "OgreIteratorWrappers.h"

namespace Ogre {

    /** Represents the state of an animation and the weight of it's influence. 
    @remarks
        Other classes can hold instances of this class to store the state of any animations
        they are using.
    */
	class _OgreExport AnimationState : public AnimationAlloc
    {
    public:

      /// typedef for an array of float values used as a bone blend mask
      typedef vector<float>::type BoneBlendMask;

        /// Normal constructor with all params supplied
        AnimationState(const String& animName, AnimationStateSet *parent, 
			Real timePos, Real length, Real weight = 1.0, bool enabled = false);
		/// constructor to copy from an existing state with new parent
		AnimationState(AnimationStateSet* parent, const AnimationState &rhs);
		/** Destructor - is here because class has virtual functions and some compilers 
			would whine if it won't exist.
		*/
		virtual ~AnimationState();
        
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
        @remarks
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
        // Inequality operator
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

      /** @brief create a new blend mask with the given number of entries
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
      /// destroy the currently set blend mask
      void destroyBlendMask();
      /** @brief set the blend mask data (might be dangerous)
       *
       * @par The size of the array should match the number of entries the
       *      blend mask was created with.
       *
       * @par Stick to the setBlendMaskEntry method if you don't know exactly what you're doing.
       */
      void _setBlendMaskData(const float* blendMaskData);
      /** @brief set the blend mask
       *
       * @par The size of the array should match the number of entries the
       *      blend mask was created with.
       *
       * @par Stick to the setBlendMaskEntry method if you don't know exactly what you're doing.
       */
      void _setBlendMask(const BoneBlendMask* blendMask);
      /// get the current blend mask (const version, may be 0) 
      const BoneBlendMask* getBlendMask() const {return mBlendMask;}
      /// return whether there is currently a valid blend mask set
      bool hasBlendMask() const {return mBlendMask != 0;}
      /// set the weight for the bone identified by the given handle
      void setBlendMaskEntry(size_t boneHandle, float weight);
      /// get the weight for the bone identified by the given handle
      inline float getBlendMaskEntry(size_t boneHandle) const
      {
        assert(mBlendMask && mBlendMask->size() > boneHandle);
        return (*mBlendMask)[boneHandle];
      }
    protected:
      /// the blend mask (containing per bone weights)
      BoneBlendMask* mBlendMask;

        String mAnimationName;
		AnimationStateSet* mParent;
        Real mTimePos;
        Real mLength;
        Real mWeight;
        bool mEnabled;
        bool mLoop;

    };

	// A map of animation states
	typedef map<String, AnimationState*>::type AnimationStateMap;
	typedef MapIterator<AnimationStateMap> AnimationStateIterator;
	typedef ConstMapIterator<AnimationStateMap> ConstAnimationStateIterator;
    // A list of enabled animation states
    typedef list<AnimationState*>::type EnabledAnimationStateList;
    typedef ConstVectorIterator<EnabledAnimationStateList> ConstEnabledAnimationStateIterator;

	/** Class encapsulating a set of AnimationState objects.
	*/
	class _OgreExport AnimationStateSet : public AnimationAlloc
	{
	public:
		/// Mutex, public for external locking if needed
		OGRE_AUTO_MUTEX
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
		@note
			The iterator returned from this method is not threadsafe,
			you will need to manually lock the public mutex on this
			class to ensure thread safety if you need it.
		*/
		AnimationStateIterator getAnimationStateIterator(void);
		/** Get an iterator over all the animation states in this set.
		@note
			The iterator returned from this method is not threadsafe,
			you will need to manually lock the public mutex on this
			class to ensure thread safety if you need it.
		*/
		ConstAnimationStateIterator getAnimationStateIterator(void) const;
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
		@note
			The iterator returned from this method is not threadsafe,
			you will need to manually lock the public mutex on this
			class to ensure thread safety if you need it.
		*/
		ConstEnabledAnimationStateIterator getEnabledAnimationStateIterator(void) const;

	protected:
		unsigned long mDirtyFrameNumber;
		AnimationStateMap mAnimationStates;
        EnabledAnimationStateList mEnabledAnimationStates;

	};

	/** ControllerValue wrapper class for AnimationState.
	@remarks
		In Azathoth and earlier, AnimationState was a ControllerValue but this
		actually causes memory problems since Controllers delete their values
		automatically when there are no further references to them, but AnimationState
		is deleted explicitly elsewhere so this causes double-free problems.
		This wrapper acts as a bridge and it is this which is destroyed automatically.
	*/
	class _OgreExport AnimationStateControllerValue : public ControllerValue<Real>
	{
	protected:
		AnimationState* mTargetAnimationState;
	public:
		/** Constructor, pass in the target animation state. */
		AnimationStateControllerValue(AnimationState* targetAnimationState)
			: mTargetAnimationState(targetAnimationState) {}
		/// Destructor (parent already virtual)
		~AnimationStateControllerValue() {}
		/** ControllerValue implementation. */
		Real getValue(void) const;

		/** ControllerValue implementation. */
		void setValue(Real value);

	};


}

#endif

