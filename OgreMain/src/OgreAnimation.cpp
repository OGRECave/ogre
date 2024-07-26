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
#include "OgreAnimation.h"
#include "OgreKeyFrame.h"

namespace Ogre {

    Animation::InterpolationMode Animation::msDefaultInterpolationMode = Animation::IM_LINEAR;
    Animation::RotationInterpolationMode 
        Animation::msDefaultRotationInterpolationMode = Animation::RIM_LINEAR;
    //---------------------------------------------------------------------
    Animation::Animation(const String& name, Real length)
        : mName(name)
        , mLength(length)
        , mInterpolationMode(msDefaultInterpolationMode)
        , mRotationInterpolationMode(msDefaultRotationInterpolationMode)
        , mKeyFrameTimesDirty(false)
        , mUseBaseKeyFrame(false)
        , mBaseKeyFrameTime(0.0f)
        , mBaseKeyFrameAnimationName(BLANKSTRING)
        , mContainer(0)
    {
    }
    //---------------------------------------------------------------------
    Animation::~Animation()
    {
        destroyAllTracks();
    }
    //---------------------------------------------------------------------
    Real Animation::getLength(void) const
    {
        return mLength;
    }
    //---------------------------------------------------------------------
    void Animation::setLength(Real len)
    {
        mLength = len;
    }
    //---------------------------------------------------------------------
    NodeAnimationTrack* Animation::createNodeTrack(unsigned short handle)
    {
        if (hasNodeTrack(handle))
        {
            OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, 
                "Node track with the specified handle " +
                StringConverter::toString(handle) + " already exists",
                "Animation::createNodeTrack");
        }

        NodeAnimationTrack* ret = OGRE_NEW NodeAnimationTrack(this, handle);

        mNodeTrackList[handle] = ret;
        return ret;
    }
    //---------------------------------------------------------------------
    NodeAnimationTrack* Animation::createNodeTrack(unsigned short handle, Node* node)
    {
        NodeAnimationTrack* ret = createNodeTrack(handle);

        ret->setAssociatedNode(node);

        return ret;
    }
    //---------------------------------------------------------------------
    unsigned short Animation::getNumNodeTracks(void) const
    {
        return (unsigned short)mNodeTrackList.size();
    }
    //---------------------------------------------------------------------
    bool Animation::hasNodeTrack(unsigned short handle) const
    {
        return (mNodeTrackList.find(handle) != mNodeTrackList.end());
    }
    //---------------------------------------------------------------------
    NodeAnimationTrack* Animation::getNodeTrack(unsigned short handle) const
    {
        NodeTrackList::const_iterator i = mNodeTrackList.find(handle);

        if (i == mNodeTrackList.end())
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                "Cannot find node track with the specified handle " +
                StringConverter::toString(handle),
                "Animation::getNodeTrack");
        }

        return i->second;

    }
    //---------------------------------------------------------------------
    void Animation::destroyNodeTrack(unsigned short handle)
    {
        NodeTrackList::iterator i = mNodeTrackList.find(handle);

        if (i != mNodeTrackList.end())
        {
            OGRE_DELETE i->second;
            mNodeTrackList.erase(i);
            _keyFrameListChanged();
        }
    }
    //---------------------------------------------------------------------
    void Animation::destroyAllNodeTracks(void)
    {
        for (auto& t : mNodeTrackList)
        {
            OGRE_DELETE t.second;
        }
        mNodeTrackList.clear();
        _keyFrameListChanged();
    }
    //---------------------------------------------------------------------
    NumericAnimationTrack* Animation::createNumericTrack(unsigned short handle, const AnimableValuePtr& anim)
    {
        if (hasNumericTrack(handle))
        {
            OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, 
                "Numeric track with the specified handle " +
                StringConverter::toString(handle) + " already exists",
                "Animation::createNumericTrack");
        }

        NumericAnimationTrack* ret = OGRE_NEW NumericAnimationTrack(this, handle, anim);

        mNumericTrackList[handle] = ret;
        return ret;
    }
    //---------------------------------------------------------------------
    unsigned short Animation::getNumNumericTracks(void) const
    {
        return (unsigned short)mNumericTrackList.size();
    }
    //---------------------------------------------------------------------
    bool Animation::hasNumericTrack(unsigned short handle) const
    {
        return (mNumericTrackList.find(handle) != mNumericTrackList.end());
    }
    //---------------------------------------------------------------------
    NumericAnimationTrack* Animation::getNumericTrack(unsigned short handle) const
    {
        NumericTrackList::const_iterator i = mNumericTrackList.find(handle);

        if (i == mNumericTrackList.end())
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                "Cannot find numeric track with the specified handle " +
                StringConverter::toString(handle),
                "Animation::getNumericTrack");
        }

        return i->second;

    }
    //---------------------------------------------------------------------
    void Animation::destroyNumericTrack(unsigned short handle)
    {
        NumericTrackList::iterator i = mNumericTrackList.find(handle);

        if (i != mNumericTrackList.end())
        {
            OGRE_DELETE i->second;
            mNumericTrackList.erase(i);
            _keyFrameListChanged();
        }
    }
    //---------------------------------------------------------------------
    void Animation::destroyAllNumericTracks(void)
    {
        for (auto& t : mNumericTrackList)
        {
            OGRE_DELETE t.second;
        }
        mNumericTrackList.clear();
        _keyFrameListChanged();
    }
    //---------------------------------------------------------------------
    VertexAnimationTrack* Animation::createVertexTrack(unsigned short handle, 
        VertexAnimationType animType)
    {
        if (hasVertexTrack(handle))
        {
            OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, 
                "Vertex track with the specified handle " +
                StringConverter::toString(handle) + " already exists",
                "Animation::createVertexTrack");
        }

        VertexAnimationTrack* ret = OGRE_NEW VertexAnimationTrack(this, handle, animType);

        mVertexTrackList[handle] = ret;
        return ret;

    }
    //---------------------------------------------------------------------
    VertexAnimationTrack* Animation::createVertexTrack(unsigned short handle, 
        VertexData* data, VertexAnimationType animType)
    {
        VertexAnimationTrack* ret = createVertexTrack(handle, animType);

        ret->setAssociatedVertexData(data);

        return ret;
    }
    //---------------------------------------------------------------------
    unsigned short Animation::getNumVertexTracks(void) const
    {
        return (unsigned short)mVertexTrackList.size();
    }
    //---------------------------------------------------------------------
    bool Animation::hasVertexTrack(unsigned short handle) const
    {
        return (mVertexTrackList.find(handle) != mVertexTrackList.end());
    }
    //---------------------------------------------------------------------
    VertexAnimationTrack* Animation::getVertexTrack(unsigned short handle) const
    {
        VertexTrackList::const_iterator i = mVertexTrackList.find(handle);

        if (i == mVertexTrackList.end())
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                "Cannot find vertex track with the specified handle " +
                StringConverter::toString(handle),
                "Animation::getVertexTrack");
        }

        return i->second;

    }
    //---------------------------------------------------------------------
    void Animation::destroyVertexTrack(unsigned short handle)
    {
        VertexTrackList::iterator i = mVertexTrackList.find(handle);

        if (i != mVertexTrackList.end())
        {
            OGRE_DELETE  i->second;
            mVertexTrackList.erase(i);
            _keyFrameListChanged();
        }
    }
    //---------------------------------------------------------------------
    void Animation::destroyAllVertexTracks(void)
    {
        VertexTrackList::iterator i;
        for (i = mVertexTrackList.begin(); i != mVertexTrackList.end(); ++i)
        {
            OGRE_DELETE  i->second;
        }
        mVertexTrackList.clear();
        _keyFrameListChanged();
    }
    //---------------------------------------------------------------------
    void Animation::destroyAllTracks(void)
    {
        destroyAllNodeTracks();
        destroyAllNumericTracks();
        destroyAllVertexTracks();
    }
    //---------------------------------------------------------------------
    const String& Animation::getName(void) const
    {
        return mName;
    }
    //---------------------------------------------------------------------
    void Animation::apply(Real timePos, Real weight, Real scale)
    {
        _applyBaseKeyFrame();

        // Calculate time index for fast keyframe search
        TimeIndex timeIndex = _getTimeIndex(timePos);

        for (auto& i : mNodeTrackList)
        {
            i.second->apply(timeIndex, weight, scale);
        }
        for (auto& j : mNumericTrackList)
        {
            j.second->apply(timeIndex, weight, scale);
        }
        for (auto& k : mVertexTrackList)
        {
            k.second->apply(timeIndex, weight, scale);
        }
    }
    //---------------------------------------------------------------------
    void Animation::applyToNode(Node* node, Real timePos, Real weight, Real scale)
    {
        _applyBaseKeyFrame();

        // Calculate time index for fast keyframe search
        TimeIndex timeIndex = _getTimeIndex(timePos);

        for (auto& t : mNodeTrackList)
        {
            t.second->applyToNode(node, timeIndex, weight, scale);
        }
    }
    //---------------------------------------------------------------------
    void Animation::apply(Skeleton* skel, Real timePos, Real weight, 
        Real scale)
    {
        _applyBaseKeyFrame();

        // Calculate time index for fast keyframe search
        TimeIndex timeIndex = _getTimeIndex(timePos);

        for (auto& t : mNodeTrackList)
        {
            // get bone to apply to 
            Bone* b = skel->getBone(t.first);
            t.second->applyToNode(b, timeIndex, weight, scale);
        }


    }
    //---------------------------------------------------------------------
    void Animation::apply(Skeleton* skel, Real timePos, float weight,
      const AnimationState::BoneBlendMask* blendMask, Real scale)
    {
        _applyBaseKeyFrame();

        // Calculate time index for fast keyframe search
        TimeIndex timeIndex = _getTimeIndex(timePos);

        for (auto& t : mNodeTrackList)
        {
            Bone* b = skel->getBone(t.first);
            t.second->applyToNode(b, timeIndex, (*blendMask)[b->getHandle()] * weight, scale);
        }
    }
    //---------------------------------------------------------------------
    void Animation::apply(Entity* entity, Real timePos, Real weight, 
        bool software, bool hardware)
    {
        _applyBaseKeyFrame();

        // Calculate time index for fast keyframe search
        TimeIndex timeIndex = _getTimeIndex(timePos);

        VertexTrackList::iterator i;
        for (auto& t : mVertexTrackList)
        {
            unsigned short handle = t.first;
            VertexAnimationTrack* track = t.second;

            VertexData* swVertexData;
            VertexData* hwVertexData;
            if (handle == 0)
            {
                // shared vertex data
                swVertexData = entity->_getSoftwareVertexAnimVertexData();
                hwVertexData = entity->_getHardwareVertexAnimVertexData();
                entity->_markBuffersUsedForAnimation();
            }
            else
            {
                // sub entity vertex data (-1)
                SubEntity* s = entity->getSubEntity(handle - 1);
                // Skip this track if subentity is not visible
                if (!s->isVisible())
                    continue;
                swVertexData = s->_getSoftwareVertexAnimVertexData();
                hwVertexData = s->_getHardwareVertexAnimVertexData();
                s->_markBuffersUsedForAnimation();
            }
            // Apply to both hardware and software, if requested
            if (software)
            {
                track->setTargetMode(VertexAnimationTrack::TM_SOFTWARE);
                track->applyToVertexData(swVertexData, timeIndex, weight, 
                    &(entity->getMesh()->getPoseList()));
            }
            if (hardware)
            {
                track->setTargetMode(VertexAnimationTrack::TM_HARDWARE);
                track->applyToVertexData(hwVertexData, timeIndex, weight, 
                    &(entity->getMesh()->getPoseList()));
            }
        }

    }
    //---------------------------------------------------------------------
    void Animation::applyToAnimable(const AnimableValuePtr& anim, Real timePos, Real weight, Real scale)
    {
        _applyBaseKeyFrame();

        // Calculate time index for fast keyframe search
        _getTimeIndex(timePos);

        for (auto& j : mNumericTrackList)
        {
            j.second->applyToAnimable(anim, timePos, weight, scale);
        }
   }
    //---------------------------------------------------------------------
    void Animation::applyToVertexData(VertexData* data, Real timePos, float weight)
    {
        _applyBaseKeyFrame();
        
        // Calculate time index for fast keyframe search
        TimeIndex timeIndex = _getTimeIndex(timePos);

        for (auto& k : mVertexTrackList)
        {
            k.second->applyToVertexData(data, timeIndex, weight);
        }
    }
    //---------------------------------------------------------------------
    void Animation::setInterpolationMode(InterpolationMode im)
    {
        mInterpolationMode = im;
    }
    //---------------------------------------------------------------------
    Animation::InterpolationMode Animation::getInterpolationMode(void) const
    {
        return mInterpolationMode;
    }
    //---------------------------------------------------------------------
    void Animation::setDefaultInterpolationMode(InterpolationMode im)
    {
        msDefaultInterpolationMode = im;
    }
    //---------------------------------------------------------------------
    Animation::InterpolationMode Animation::getDefaultInterpolationMode(void)
    {
        return msDefaultInterpolationMode;
    }
    //---------------------------------------------------------------------
    const Animation::NodeTrackList& Animation::_getNodeTrackList(void) const
    {
        return mNodeTrackList;

    }
    //---------------------------------------------------------------------
    const Animation::NumericTrackList& Animation::_getNumericTrackList(void) const
    {
        return mNumericTrackList;
    }
    //---------------------------------------------------------------------
    const Animation::VertexTrackList& Animation::_getVertexTrackList(void) const
    {
        return mVertexTrackList;
    }
    //---------------------------------------------------------------------
    void Animation::setRotationInterpolationMode(RotationInterpolationMode im)
    {
        mRotationInterpolationMode = im;
    }
    //---------------------------------------------------------------------
    Animation::RotationInterpolationMode Animation::getRotationInterpolationMode(void) const
    {
        return mRotationInterpolationMode;
    }
    //---------------------------------------------------------------------
    void Animation::setDefaultRotationInterpolationMode(RotationInterpolationMode im)
    {
        msDefaultRotationInterpolationMode = im;
    }
    //---------------------------------------------------------------------
    Animation::RotationInterpolationMode Animation::getDefaultRotationInterpolationMode(void)
    {
        return msDefaultRotationInterpolationMode;
    }
    //---------------------------------------------------------------------
    void Animation::optimise(bool discardIdentityNodeTracks)
    {
        optimiseNodeTracks(discardIdentityNodeTracks);
        optimiseVertexTracks();
        
    }
    //-----------------------------------------------------------------------
    void Animation::_collectIdentityNodeTracks(TrackHandleList& tracks) const
    {
        for (auto& t : mNodeTrackList)
        {
            const NodeAnimationTrack* track = t.second;
            if (track->hasNonZeroKeyFrames())
            {
                tracks.erase(t.first);
            }
        }
    }
    //-----------------------------------------------------------------------
    void Animation::_destroyNodeTracks(const TrackHandleList& tracks)
    {
        for (auto t : tracks)
        {
            destroyNodeTrack(t);
        }
    }
    //-----------------------------------------------------------------------
    void Animation::optimiseNodeTracks(bool discardIdentityTracks)
    {
        // Iterate over the node tracks and identify those with no useful keyframes
        std::list<unsigned short> tracksToDestroy;
        for (auto& t : mNodeTrackList)
        {
            NodeAnimationTrack* track = t.second;
            if (discardIdentityTracks && !track->hasNonZeroKeyFrames())
            {
                // mark the entire track for destruction
                tracksToDestroy.push_back(t.first);
            }
            else
            {
                track->optimise();
            }
        }

        // Now destroy the tracks we marked for death
        for(unsigned short& h : tracksToDestroy)
        {
            destroyNodeTrack(h);
        }
    }
    //-----------------------------------------------------------------------
    void Animation::optimiseVertexTracks(void)
    {
        // Iterate over the node tracks and identify those with no useful keyframes
        std::list<unsigned short> tracksToDestroy;
        for (auto& t : mVertexTrackList)
        {
            VertexAnimationTrack* track = t.second;
            if (!track->hasNonZeroKeyFrames())
            {
                // mark the entire track for destruction
                tracksToDestroy.push_back(t.first);
            }
            else
            {
                track->optimise();
            }
        }

        // Now destroy the tracks we marked for death
        for(unsigned short& h : tracksToDestroy)
        {
            destroyVertexTrack(h);
        }
    }
    //-----------------------------------------------------------------------
    Animation* Animation::clone(const String& newName) const
    {
        Animation* newAnim = OGRE_NEW Animation(newName, mLength);
        newAnim->mInterpolationMode = mInterpolationMode;
        newAnim->mRotationInterpolationMode = mRotationInterpolationMode;
        
        // Clone all tracks
        for (auto i : mNodeTrackList)
        {
            i.second->_clone(newAnim);
        }
        for (auto i : mNumericTrackList)
        {
            i.second->_clone(newAnim);
        }
        for (auto i : mVertexTrackList)
        {
            i.second->_clone(newAnim);
        }

        newAnim->_keyFrameListChanged();
        return newAnim;

    }
    //-----------------------------------------------------------------------
    TimeIndex Animation::_getTimeIndex(Real timePos) const
    {
        // Uncomment following statement for work as previous
        //return timePos;

        // Build keyframe time list on demand
        if (mKeyFrameTimesDirty)
        {
            buildKeyFrameTimeList();
        }

        // Wrap time
        Real totalAnimationLength = mLength;

        if( timePos > totalAnimationLength && totalAnimationLength > 0.0f )
            timePos = std::fmod( timePos, totalAnimationLength );

        // Search for global index
        auto it = std::lower_bound(mKeyFrameTimes.begin(), mKeyFrameTimes.end() - 1, timePos);
        return TimeIndex(timePos, static_cast<uint>(std::distance(mKeyFrameTimes.begin(), it)));
    }
    //-----------------------------------------------------------------------
    void Animation::buildKeyFrameTimeList(void) const
    {
        // Clear old keyframe times
        mKeyFrameTimes.clear();

        // Collect all keyframe times from each track
        for (auto& i : mNodeTrackList)
        {
            i.second->_collectKeyFrameTimes(mKeyFrameTimes);
        }
        for (auto& j : mNumericTrackList)
        {
            j.second->_collectKeyFrameTimes(mKeyFrameTimes);
        }
        for (auto& k : mVertexTrackList)
        {
            k.second->_collectKeyFrameTimes(mKeyFrameTimes);
        }

        // Build global index to local index map for each track
        for (auto& i : mNodeTrackList)
        {
            i.second->_buildKeyFrameIndexMap(mKeyFrameTimes);
        }
        for (auto& j : mNumericTrackList)
        {
            j.second->_buildKeyFrameIndexMap(mKeyFrameTimes);
        }
        for (auto& k : mVertexTrackList)
        {
            k.second->_buildKeyFrameIndexMap(mKeyFrameTimes);
        }

        // Reset dirty flag
        mKeyFrameTimesDirty = false;
    }
    //-----------------------------------------------------------------------
    void Animation::setUseBaseKeyFrame(bool useBaseKeyFrame, Real keyframeTime, const String& baseAnimName)
    {
        if (useBaseKeyFrame != mUseBaseKeyFrame ||
            keyframeTime != mBaseKeyFrameTime ||
            baseAnimName != mBaseKeyFrameAnimationName)
        {
            mUseBaseKeyFrame = useBaseKeyFrame;
            mBaseKeyFrameTime = keyframeTime;
            mBaseKeyFrameAnimationName = baseAnimName;
        }
    }
    //-----------------------------------------------------------------------
    bool Animation::getUseBaseKeyFrame() const
    {
        return mUseBaseKeyFrame;
    }
    //-----------------------------------------------------------------------
    Real Animation::getBaseKeyFrameTime() const
    {
        return mBaseKeyFrameTime;
    }
    //-----------------------------------------------------------------------
    const String& Animation::getBaseKeyFrameAnimationName() const
    {
        return mBaseKeyFrameAnimationName;
    }
    //-----------------------------------------------------------------------
    void Animation::_applyBaseKeyFrame()
    {
        if (mUseBaseKeyFrame)
        {
            Animation* baseAnim = this;
            if (!mBaseKeyFrameAnimationName.empty() && mContainer)
                baseAnim = mContainer->getAnimation(mBaseKeyFrameAnimationName);
            
            if (baseAnim)
            {
                for (auto& i : mNodeTrackList)
                {
                    NodeAnimationTrack* track = i.second;
                    
                    NodeAnimationTrack* baseTrack;
                    if (baseAnim == this)
                        baseTrack = track;
                    else
                        baseTrack = baseAnim->getNodeTrack(track->getHandle());
                    
                    TransformKeyFrame kf(baseTrack, mBaseKeyFrameTime);
                    baseTrack->getInterpolatedKeyFrame(baseAnim->_getTimeIndex(mBaseKeyFrameTime), &kf);
                    track->_applyBaseKeyFrame(&kf);
                }
                
                for (auto& i : mVertexTrackList)
                {
                    VertexAnimationTrack* track = i.second;
                    
                    if (track->getAnimationType() == VAT_POSE)
                    {
                        VertexAnimationTrack* baseTrack;
                        if (baseAnim == this)
                            baseTrack = track;
                        else
                            baseTrack = baseAnim->getVertexTrack(track->getHandle());
                        
                        VertexPoseKeyFrame kf(baseTrack, mBaseKeyFrameTime);
                        baseTrack->getInterpolatedKeyFrame(baseAnim->_getTimeIndex(mBaseKeyFrameTime), &kf);
                        track->_applyBaseKeyFrame(&kf);
                        
                    }
                }
                
            }
            
            // Re-base has been done, this is a one-way translation
            mUseBaseKeyFrame = false;
        }
        
    }
    //-----------------------------------------------------------------------
    void Animation::_notifyContainer(AnimationContainer* c)
    {
        mContainer = c;
    }
    //-----------------------------------------------------------------------
    AnimationContainer* Animation::getContainer()
    {
        return mContainer;
    }
}


