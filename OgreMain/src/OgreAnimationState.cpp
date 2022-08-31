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
#include "OgreStableHeaders.h"
#include "OgreAnimationState.h"


namespace Ogre 
{

    //---------------------------------------------------------------------
    AnimationState::AnimationState(AnimationStateSet* parent, const AnimationState &rhs)
        : mAnimationName(rhs.mAnimationName)
        , mParent(parent)
        , mTimePos(rhs.mTimePos)
        , mLength(rhs.mLength)
        , mWeight(rhs.mWeight)
        , mEnabled(rhs.mEnabled)
        , mLoop(rhs.mLoop)
  {
        mParent->_notifyDirty();
    }
    //---------------------------------------------------------------------
    AnimationState::AnimationState(const String& animName, 
        AnimationStateSet *parent, Real timePos, Real length, Real weight, 
        bool enabled)
        : mAnimationName(animName)
        , mParent(parent)
        , mTimePos(timePos)
        , mLength(length)
        , mWeight(weight)
        , mEnabled(enabled)
        , mLoop(true)
    {
        mParent->_notifyDirty();
    }
    //---------------------------------------------------------------------
    const String& AnimationState::getAnimationName() const
    {
        return mAnimationName;
    }
    //---------------------------------------------------------------------
    Real AnimationState::getTimePosition(void) const
    {
        return mTimePos;
    }
    //---------------------------------------------------------------------
    void AnimationState::setTimePosition(Real timePos)
    {
        if (timePos != mTimePos)
        {
            mTimePos = timePos;
            if (mLoop)
            {
                // Wrap
                mTimePos = std::fmod(mTimePos, mLength);
                if(mTimePos < 0) mTimePos += mLength;
            }
            else
            {
                // Clamp
                mTimePos = Math::Clamp(mTimePos, Real(0), mLength);
            }

            if (mEnabled)
                mParent->_notifyDirty();
        }

    }
    //---------------------------------------------------------------------
    Real AnimationState::getLength() const
    {
        return mLength;
    }
    //---------------------------------------------------------------------
    void AnimationState::setLength(Real len)
    {
        mLength = len;
    }
    //---------------------------------------------------------------------
    Real AnimationState::getWeight(void) const
    {
        return mWeight;
    }
    //---------------------------------------------------------------------
    void AnimationState::setWeight(Real weight)
    {
        mWeight = weight;

        if (mEnabled)
            mParent->_notifyDirty();
    }
    //---------------------------------------------------------------------
    void AnimationState::addTime(Real offset)
    {
        setTimePosition(mTimePos + offset);
    }
    //---------------------------------------------------------------------
    bool AnimationState::hasEnded(void) const
    {
        return (mTimePos >= mLength && !mLoop);
    }
    //---------------------------------------------------------------------
    bool AnimationState::getEnabled(void) const
    {
        return mEnabled;
    }
    //---------------------------------------------------------------------
    void AnimationState::setEnabled(bool enabled)
    {
        mEnabled = enabled;
        mParent->_notifyAnimationStateEnabled(this, enabled);
    }
    //---------------------------------------------------------------------
    bool AnimationState::operator==(const AnimationState& rhs) const
    {
        if (mAnimationName == rhs.mAnimationName &&
            mEnabled == rhs.mEnabled &&
            mTimePos == rhs.mTimePos &&
            mWeight == rhs.mWeight &&
            mLength == rhs.mLength && 
            mLoop == rhs.mLoop)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    //---------------------------------------------------------------------
    bool AnimationState::operator!=(const AnimationState& rhs) const
    {
        return !(*this == rhs);
    }
    //---------------------------------------------------------------------
    void AnimationState::copyStateFrom(const AnimationState& animState)
    {
        mTimePos = animState.mTimePos;
        mLength = animState.mLength;
        mWeight = animState.mWeight;
        mEnabled = animState.mEnabled;
        mLoop = animState.mLoop;
        mParent->_notifyDirty();

    }
    //---------------------------------------------------------------------
    void AnimationState::setBlendMaskEntry(size_t boneHandle, float weight)
    {
        assert(mBlendMask.size() > boneHandle);
        mBlendMask[boneHandle] = weight;
        if (mEnabled)
            mParent->_notifyDirty();
    }
    //---------------------------------------------------------------------
    void AnimationState::_setBlendMaskData(const float* blendMaskData) 
    {
        assert(!mBlendMask.empty() && "No BlendMask set!");
        // input 0?
        if(!blendMaskData)
        {
            destroyBlendMask();
            return;
        }
        // dangerous memcpy
        memcpy(mBlendMask.data(), blendMaskData, sizeof(float) * mBlendMask.size());
        if (mEnabled)
            mParent->_notifyDirty();
    }
    //---------------------------------------------------------------------
    void AnimationState::_setBlendMask(const BoneBlendMask* blendMask) 
    {
        if(mBlendMask.empty())
        {
            createBlendMask(blendMask->size(), false);
        }
        _setBlendMaskData(blendMask->data());
    }
    //---------------------------------------------------------------------
    void AnimationState::createBlendMask(size_t blendMaskSizeHint, float initialWeight)
    {
        if(mBlendMask.empty())
        {
            if(initialWeight >= 0)
            {
                mBlendMask.resize(blendMaskSizeHint, initialWeight);
            }
            else
            {
                mBlendMask.resize(blendMaskSizeHint);
            }
        }
    }
    //---------------------------------------------------------------------
    void AnimationState::destroyBlendMask()
    {
        mBlendMask.clear();
        mBlendMask.shrink_to_fit();
    }
    //---------------------------------------------------------------------

    //---------------------------------------------------------------------
    AnimationStateSet::AnimationStateSet()
        : mDirtyFrameNumber(std::numeric_limits<unsigned long>::max())
    {
    }
    //---------------------------------------------------------------------
    AnimationStateSet::AnimationStateSet(const AnimationStateSet& rhs)
        : mDirtyFrameNumber(std::numeric_limits<unsigned long>::max())
    {
        // lock rhs
            OGRE_LOCK_MUTEX(rhs.OGRE_AUTO_MUTEX_NAME);

        for (const auto & mAnimationState : rhs.mAnimationStates)
        {
            AnimationState* src = mAnimationState.second;
            mAnimationStates[src->getAnimationName()] = OGRE_NEW AnimationState(this, *src);
        }

        // Clone enabled animation state list
        for (auto src : rhs.mEnabledAnimationStates)
        {
            mEnabledAnimationStates.push_back(getAnimationState(src->getAnimationName()));
        }
    }
    //---------------------------------------------------------------------
    AnimationStateSet::~AnimationStateSet()
    {
        // Destroy
        removeAllAnimationStates();
    }
    //---------------------------------------------------------------------
    void AnimationStateSet::removeAnimationState(const String& name)
    {
            OGRE_LOCK_AUTO_MUTEX;

        AnimationStateMap::iterator i = mAnimationStates.find(name);
        if (i != mAnimationStates.end())
        {
            mEnabledAnimationStates.remove(i->second);

            OGRE_DELETE i->second;
            mAnimationStates.erase(i);
        }
    }
    //---------------------------------------------------------------------
    void AnimationStateSet::removeAllAnimationStates(void)
    {
            OGRE_LOCK_AUTO_MUTEX;

        for (auto & mAnimationState : mAnimationStates)
        {
            OGRE_DELETE mAnimationState.second;
        }
        mAnimationStates.clear();
        mEnabledAnimationStates.clear();
    }
    //---------------------------------------------------------------------
    AnimationState* AnimationStateSet::createAnimationState(const String& name,  
        Real timePos, Real length, Real weight, bool enabled)
    {
            OGRE_LOCK_AUTO_MUTEX;

        AnimationStateMap::iterator i = mAnimationStates.find(name);
        if (i != mAnimationStates.end())
        {
            OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, 
                "State for animation named '" + name + "' already exists.", 
                "AnimationStateSet::createAnimationState");
        }

        AnimationState* newState = OGRE_NEW AnimationState(name, this, timePos, 
            length, weight, enabled);
        mAnimationStates[name] = newState;
        return newState;

    }
    //---------------------------------------------------------------------
    AnimationState* AnimationStateSet::getAnimationState(const String& name) const
    {
            OGRE_LOCK_AUTO_MUTEX;

        AnimationStateMap::const_iterator i = mAnimationStates.find(name);
        if (i == mAnimationStates.end())
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                "No state found for animation named '" + name + "'", 
                "AnimationStateSet::getAnimationState");
        }
        return i->second;
    }
    //---------------------------------------------------------------------
    bool AnimationStateSet::hasAnimationState(const String& name) const
    {
            OGRE_LOCK_AUTO_MUTEX;

        return mAnimationStates.find(name) != mAnimationStates.end();
    }
    //---------------------------------------------------------------------
    AnimationStateIterator AnimationStateSet::getAnimationStateIterator(void)
    {
            OGRE_LOCK_AUTO_MUTEX;
        // returned iterator not threadsafe, noted in header
        return AnimationStateIterator(
            mAnimationStates.begin(), mAnimationStates.end());
    }
    //---------------------------------------------------------------------
    ConstAnimationStateIterator AnimationStateSet::getAnimationStateIterator(void) const
    {
            OGRE_LOCK_AUTO_MUTEX;
        // returned iterator not threadsafe, noted in header
        return ConstAnimationStateIterator(
            mAnimationStates.begin(), mAnimationStates.end());
    }
    //---------------------------------------------------------------------
    void AnimationStateSet::copyMatchingState(AnimationStateSet* target) const
    {
        // lock target
        OGRE_LOCK_MUTEX(target->OGRE_AUTO_MUTEX_NAME);
        // lock source
        OGRE_LOCK_AUTO_MUTEX;

        for (auto& t : target->mAnimationStates) {
            AnimationStateMap::const_iterator iother = mAnimationStates.find(t.first);
            if (iother == mAnimationStates.end()) {
                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "No animation entry found named " + t.first,
                    "AnimationStateSet::copyMatchingState");
            } else {
                t.second->copyStateFrom(*(iother->second));
            }
        }

        // Copy matching enabled animation state list
        target->mEnabledAnimationStates.clear();

        for (auto *src : mEnabledAnimationStates)
        {
            AnimationStateMap::const_iterator itarget = target->mAnimationStates.find(src->getAnimationName());
            if (itarget != target->mAnimationStates.end())
            {
                target->mEnabledAnimationStates.push_back(itarget->second);
            }
        }

        target->mDirtyFrameNumber = mDirtyFrameNumber;
    }
    //---------------------------------------------------------------------
    void AnimationStateSet::_notifyDirty(void)
    {
        OGRE_LOCK_AUTO_MUTEX;
        ++mDirtyFrameNumber;
    }
    //---------------------------------------------------------------------
    void AnimationStateSet::_notifyAnimationStateEnabled(AnimationState* target, bool enabled)
    {
        OGRE_LOCK_AUTO_MUTEX;
        // Remove from enabled animation state list first
        mEnabledAnimationStates.remove(target);

        // Add to enabled animation state list if need
        if (enabled)
        {
            mEnabledAnimationStates.push_back(target);
        }

        // Set the dirty frame number
        _notifyDirty();
    }
    //---------------------------------------------------------------------
    ConstEnabledAnimationStateIterator AnimationStateSet::getEnabledAnimationStateIterator(void) const
    {
        OGRE_LOCK_AUTO_MUTEX;
        // returned iterator not threadsafe, noted in header
        return ConstEnabledAnimationStateIterator(
            mEnabledAnimationStates.begin(), mEnabledAnimationStates.end());
    }

    ControllerValueRealPtr AnimationStateControllerValue::create(AnimationState* targetAnimationState, bool addTime)
    {
        return std::make_shared<AnimationStateControllerValue>(targetAnimationState, addTime);
    }
}

