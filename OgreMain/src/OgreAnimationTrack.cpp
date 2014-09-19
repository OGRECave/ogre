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
#include "OgreAnimationTrack.h"
#include "OgreAnimation.h"
#include "OgreKeyFrame.h"
#include "OgreNode.h"
#include "OgreLogManager.h"
#include "OgreHardwareBufferManager.h"
#include "OgreMesh.h"
#include "OgreException.h"

namespace Ogre {

    namespace {
        // Locally key frame search helper
        struct KeyFrameTimeLess
        {
            bool operator() (const KeyFrame* kf, const KeyFrame* kf2) const
            {
                return kf->getTime() < kf2->getTime();
            }
        };
    }
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    AnimationTrack::AnimationTrack(Animation* parent, unsigned short handle) :
		mParent(parent), mHandle(handle), mListener(0)
    {
    }
    //---------------------------------------------------------------------
    AnimationTrack::~AnimationTrack()
    {
        removeAllKeyFrames();
    }
    //---------------------------------------------------------------------
    unsigned short AnimationTrack::getNumKeyFrames(void) const
    {
        return (unsigned short)mKeyFrames.size();
    }
    //---------------------------------------------------------------------
    KeyFrame* AnimationTrack::getKeyFrame(unsigned short index) const
    {
		// If you hit this assert, then the keyframe index is out of bounds
        assert( index < (ushort)mKeyFrames.size() );

        return mKeyFrames[index];
    }
    //---------------------------------------------------------------------
    Real AnimationTrack::getKeyFramesAtTime(const TimeIndex& timeIndex, KeyFrame** keyFrame1, KeyFrame** keyFrame2,
        unsigned short* firstKeyIndex) const
    {
        // Parametric time
        // t1 = time of previous keyframe
        // t2 = time of next keyframe
        Real t1, t2;

        Real timePos = timeIndex.getTimePos();

        // Find first keyframe after or on current time
        KeyFrameList::const_iterator i;
        if (timeIndex.hasKeyIndex())
        {
            // Global keyframe index available, map to local keyframe index directly.
            assert(timeIndex.getKeyIndex() < mKeyFrameIndexMap.size());
            i = mKeyFrames.begin() + mKeyFrameIndexMap[timeIndex.getKeyIndex()];
#if OGRE_DEBUG_MODE
            KeyFrame timeKey(0, timePos);
            if (i != std::lower_bound(mKeyFrames.begin(), mKeyFrames.end(), &timeKey, KeyFrameTimeLess()))
            {
                OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                    "Optimised key frame search failed",
                    "AnimationTrack::getKeyFramesAtTime");
            }
#endif
        }
        else
        {
            // Wrap time
            Real totalAnimationLength = mParent->getLength();
            assert(totalAnimationLength > 0.0f && "Invalid animation length!");

            if( timePos > totalAnimationLength && totalAnimationLength > 0.0f )
				timePos = fmod( timePos, totalAnimationLength );

            // No global keyframe index, need to search with local keyframes.
            KeyFrame timeKey(0, timePos);
            i = std::lower_bound(mKeyFrames.begin(), mKeyFrames.end(), &timeKey, KeyFrameTimeLess());
        }

        if (i == mKeyFrames.end())
        {
            // There is no keyframe after this time, wrap back to first
            *keyFrame2 = mKeyFrames.front();
            t2 = mParent->getLength() + (*keyFrame2)->getTime();

            // Use last keyframe as previous keyframe
            --i;
        }
        else
        {
            *keyFrame2 = *i;
            t2 = (*keyFrame2)->getTime();

            // Find last keyframe before or on current time
            if (i != mKeyFrames.begin() && timePos < (*i)->getTime())
            {
                --i;
            }
        }

        // Fill index of the first key
        if (firstKeyIndex)
        {
            *firstKeyIndex = static_cast<unsigned short>(std::distance(mKeyFrames.begin(), i));
        }

        *keyFrame1 = *i;

        t1 = (*keyFrame1)->getTime();

        if (t1 == t2)
        {
            // Same KeyFrame (only one)
            return 0.0;
        }
        else
        {
            return (timePos - t1) / (t2 - t1);
        }
    }
    //---------------------------------------------------------------------
    KeyFrame* AnimationTrack::createKeyFrame(Real timePos)
    {
        KeyFrame* kf = createKeyFrameImpl(timePos);

        // Insert just before upper bound
        KeyFrameList::iterator i =
            std::upper_bound(mKeyFrames.begin(), mKeyFrames.end(), kf, KeyFrameTimeLess());
        mKeyFrames.insert(i, kf);

        _keyFrameDataChanged();
        mParent->_keyFrameListChanged();

        return kf;

    }
    //---------------------------------------------------------------------
    void AnimationTrack::removeKeyFrame(unsigned short index)
    {
		// If you hit this assert, then the keyframe index is out of bounds
        assert( index < (ushort)mKeyFrames.size() );

        KeyFrameList::iterator i = mKeyFrames.begin();

        i += index;

        OGRE_DELETE *i;

        mKeyFrames.erase(i);

        _keyFrameDataChanged();
        mParent->_keyFrameListChanged();


    }
    //---------------------------------------------------------------------
    void AnimationTrack::removeAllKeyFrames(void)
    {
        KeyFrameList::iterator i = mKeyFrames.begin();

        for (; i != mKeyFrames.end(); ++i)
        {
            OGRE_DELETE *i;
        }

        _keyFrameDataChanged();
        mParent->_keyFrameListChanged();

        mKeyFrames.clear();

    }
    //---------------------------------------------------------------------
    void AnimationTrack::_collectKeyFrameTimes(vector<Real>::type& keyFrameTimes)
    {
        for (KeyFrameList::const_iterator i = mKeyFrames.begin(); i != mKeyFrames.end(); ++i)
        {
            Real timePos = (*i)->getTime();

            vector<Real>::type::iterator it =
                std::lower_bound(keyFrameTimes.begin(), keyFrameTimes.end(), timePos);
            if (it == keyFrameTimes.end() || *it != timePos)
            {
                keyFrameTimes.insert(it, timePos);
            }
        }
    }
    //---------------------------------------------------------------------
    void AnimationTrack::_buildKeyFrameIndexMap(const vector<Real>::type& keyFrameTimes)
    {
        // Pre-allocate memory
        mKeyFrameIndexMap.resize(keyFrameTimes.size() + 1);

        size_t i = 0, j = 0;
        while (j <= keyFrameTimes.size())
        {
            mKeyFrameIndexMap[j] = static_cast<ushort>(i);
            while (i < mKeyFrames.size() && mKeyFrames[i]->getTime() <= keyFrameTimes[j])
                ++i;
            ++j;
        }
    }
    //--------------------------------------------------------------------------
    void AnimationTrack::_applyBaseKeyFrame(const KeyFrame*)
    {}
	//---------------------------------------------------------------------
	void AnimationTrack::populateClone(AnimationTrack* clone) const
	{
		for (KeyFrameList::const_iterator i = mKeyFrames.begin();
			i != mKeyFrames.end(); ++i)
		{
			KeyFrame* clonekf = (*i)->_clone(clone);
			clone->mKeyFrames.push_back(clonekf);
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	// Numeric specialisations
	//---------------------------------------------------------------------
	NumericAnimationTrack::NumericAnimationTrack(Animation* parent,
		unsigned short handle)
		: AnimationTrack(parent, handle)
	{
	}
	//---------------------------------------------------------------------
	NumericAnimationTrack::NumericAnimationTrack(Animation* parent,
		unsigned short handle, AnimableValuePtr& target)
		:AnimationTrack(parent, handle), mTargetAnim(target)
	{
	}
	//---------------------------------------------------------------------
	const AnimableValuePtr& NumericAnimationTrack::getAssociatedAnimable(void) const
	{
		return mTargetAnim;
	}
	//---------------------------------------------------------------------
	void NumericAnimationTrack::setAssociatedAnimable(const AnimableValuePtr& val)
	{
		mTargetAnim = val;
	}
	//---------------------------------------------------------------------
	KeyFrame* NumericAnimationTrack::createKeyFrameImpl(Real time)
	{
		return OGRE_NEW NumericKeyFrame(this, time);
	}
	//---------------------------------------------------------------------
	void NumericAnimationTrack::getInterpolatedKeyFrame(const TimeIndex& timeIndex,
		KeyFrame* kf) const
	{
		if (mListener)
		{
			if (mListener->getInterpolatedKeyFrame(this, timeIndex, kf))
				return;
		}

		NumericKeyFrame* kret = static_cast<NumericKeyFrame*>(kf);

        // Keyframe pointers
		KeyFrame *kBase1, *kBase2;
        NumericKeyFrame *k1, *k2;
        unsigned short firstKeyIndex;

        Real t = this->getKeyFramesAtTime(timeIndex, &kBase1, &kBase2, &firstKeyIndex);
		k1 = static_cast<NumericKeyFrame*>(kBase1);
		k2 = static_cast<NumericKeyFrame*>(kBase2);

        if (t == 0.0)
        {
            // Just use k1
            kret->setValue(k1->getValue());
        }
        else
        {
            // Interpolate by t
			AnyNumeric diff = k2->getValue() - k1->getValue();
			kret->setValue(k1->getValue() + diff * t);
        }
	}
	//---------------------------------------------------------------------
	void NumericAnimationTrack::apply(const TimeIndex& timeIndex, Real weight, Real scale)
	{
		applyToAnimable(mTargetAnim, timeIndex, weight, scale);
	}
	//---------------------------------------------------------------------
	void NumericAnimationTrack::applyToAnimable(const AnimableValuePtr& anim, const TimeIndex& timeIndex,
		Real weight, Real scale)
	{
		// Nothing to do if no keyframes or zero weight, scale
		if (mKeyFrames.empty() || !weight || !scale)
			return;

		NumericKeyFrame kf(0, timeIndex.getTimePos());
		getInterpolatedKeyFrame(timeIndex, &kf);
		// add to existing. Weights are not relative, but treated as
		// absolute multipliers for the animation
		AnyNumeric val = kf.getValue() * (weight * scale);

		anim->applyDeltaValue(val);

	}
	//--------------------------------------------------------------------------
	NumericKeyFrame* NumericAnimationTrack::createNumericKeyFrame(Real timePos)
	{
		return static_cast<NumericKeyFrame*>(createKeyFrame(timePos));
	}
	//--------------------------------------------------------------------------
	NumericKeyFrame* NumericAnimationTrack::getNumericKeyFrame(unsigned short index) const
	{
		return static_cast<NumericKeyFrame*>(getKeyFrame(index));
	}
    //---------------------------------------------------------------------
	NumericAnimationTrack* NumericAnimationTrack::_clone(Animation* newParent) const
	{
		NumericAnimationTrack* newTrack = 
			newParent->createNumericTrack(mHandle);
		newTrack->mTargetAnim = mTargetAnim;
		populateClone(newTrack);
		return newTrack;
	}
    //---------------------------------------------------------------------
	//---------------------------------------------------------------------
	// Node specialisations
	//---------------------------------------------------------------------
	NodeAnimationTrack::NodeAnimationTrack(Animation* parent, unsigned short handle)
		: AnimationTrack(parent, handle), mTargetNode(0)
        , mSplines(0), mSplineBuildNeeded(false)
        , mUseShortestRotationPath(true)
	{
	}
	//---------------------------------------------------------------------
	NodeAnimationTrack::NodeAnimationTrack(Animation* parent, unsigned short handle,
		Node* targetNode)
		: AnimationTrack(parent, handle), mTargetNode(targetNode)
        , mSplines(0), mSplineBuildNeeded(false)
        , mUseShortestRotationPath(true)
	{
	}
    //---------------------------------------------------------------------
    NodeAnimationTrack::~NodeAnimationTrack()
    {
        OGRE_DELETE_T(mSplines, Splines, MEMCATEGORY_ANIMATION);
    }
	//---------------------------------------------------------------------
    void NodeAnimationTrack::getInterpolatedKeyFrame(const TimeIndex& timeIndex, KeyFrame* kf) const
    {
		if (mListener)
		{
			if (mListener->getInterpolatedKeyFrame(this, timeIndex, kf))
				return;
		}

		TransformKeyFrame* kret = static_cast<TransformKeyFrame*>(kf);

        // Keyframe pointers
		KeyFrame *kBase1, *kBase2;
        TransformKeyFrame *k1, *k2;
        unsigned short firstKeyIndex;

        Real t = this->getKeyFramesAtTime(timeIndex, &kBase1, &kBase2, &firstKeyIndex);
		k1 = static_cast<TransformKeyFrame*>(kBase1);
		k2 = static_cast<TransformKeyFrame*>(kBase2);

        if (t == 0.0)
        {
            // Just use k1
            kret->setRotation(k1->getRotation());
            kret->setTranslate(k1->getTranslate());
            kret->setScale(k1->getScale());
        }
        else
        {
            // Interpolate by t
            Animation::InterpolationMode im = mParent->getInterpolationMode();
            Animation::RotationInterpolationMode rim =
                mParent->getRotationInterpolationMode();
            Vector3 base;
            switch(im)
            {
            case Animation::IM_LINEAR:
                // Interpolate linearly
                // Rotation
                // Interpolate to nearest rotation if mUseShortestRotationPath set
                if (rim == Animation::RIM_LINEAR)
                {
                    kret->setRotation( Quaternion::nlerp(t, k1->getRotation(),
                        k2->getRotation(), mUseShortestRotationPath) );
                }
                else //if (rim == Animation::RIM_SPHERICAL)
                {
                    kret->setRotation( Quaternion::Slerp(t, k1->getRotation(),
					    k2->getRotation(), mUseShortestRotationPath) );
                }

                // Translation
                base = k1->getTranslate();
                kret->setTranslate( base + ((k2->getTranslate() - base) * t) );

                // Scale
                base = k1->getScale();
                kret->setScale( base + ((k2->getScale() - base) * t) );
                break;

            case Animation::IM_SPLINE:
                // Spline interpolation

                // Build splines if required
                if (mSplineBuildNeeded)
                {
                    buildInterpolationSplines();
                }

                // Rotation, take mUseShortestRotationPath into account
                kret->setRotation( mSplines->rotationSpline.interpolate(firstKeyIndex, t,
					mUseShortestRotationPath) );

                // Translation
                kret->setTranslate( mSplines->positionSpline.interpolate(firstKeyIndex, t) );

                // Scale
                kret->setScale( mSplines->scaleSpline.interpolate(firstKeyIndex, t) );

                break;
            }

        }
    }
    //---------------------------------------------------------------------
    void NodeAnimationTrack::apply(const TimeIndex& timeIndex, Real weight, Real scale)
    {
        applyToNode(mTargetNode, timeIndex, weight, scale);

    }
    //---------------------------------------------------------------------
    Node* NodeAnimationTrack::getAssociatedNode(void) const
    {
        return mTargetNode;
    }
    //---------------------------------------------------------------------
    void NodeAnimationTrack::setAssociatedNode(Node* node)
    {
        mTargetNode = node;
    }
    //---------------------------------------------------------------------
    void NodeAnimationTrack::applyToNode(Node* node, const TimeIndex& timeIndex, Real weight,
		Real scl)
    {
		// Nothing to do if no keyframes or zero weight or no node
		if (mKeyFrames.empty() || !weight || !node)
			return;

        TransformKeyFrame kf(0, timeIndex.getTimePos());
		getInterpolatedKeyFrame(timeIndex, &kf);

		// add to existing. Weights are not relative, but treated as absolute multipliers for the animation
        Vector3 translate = kf.getTranslate() * weight * scl;
		node->translate(translate);

		// interpolate between no-rotation and full rotation, to point 'weight', so 0 = no rotate, 1 = full
        Quaternion rotate;
        Animation::RotationInterpolationMode rim =
            mParent->getRotationInterpolationMode();
        if (rim == Animation::RIM_LINEAR)
        {
            rotate = Quaternion::nlerp(weight, Quaternion::IDENTITY, kf.getRotation(), mUseShortestRotationPath);
        }
        else //if (rim == Animation::RIM_SPHERICAL)
        {
            rotate = Quaternion::Slerp(weight, Quaternion::IDENTITY, kf.getRotation(), mUseShortestRotationPath);
        }
		node->rotate(rotate);

		Vector3 scale = kf.getScale();
		// Not sure how to modify scale for cumulative anims... leave it alone
		//scale = ((Vector3::UNIT_SCALE - kf.getScale()) * weight) + Vector3::UNIT_SCALE;
		if (scale != Vector3::UNIT_SCALE)
		{
            if (scl != 1.0f)
                scale = Vector3::UNIT_SCALE + (scale - Vector3::UNIT_SCALE) * scl;
            else if (weight != 1.0f)
                scale = Vector3::UNIT_SCALE + (scale - Vector3::UNIT_SCALE) * weight;
		}
		node->scale(scale);

    }
    //---------------------------------------------------------------------
    void NodeAnimationTrack::buildInterpolationSplines(void) const
    {
        // Allocate splines if not exists
        if (!mSplines)
        {
            mSplines = OGRE_NEW_T(Splines, MEMCATEGORY_ANIMATION);
        }

        // Cache to register for optimisation
        Splines* splines = mSplines;

        // Don't calc automatically, do it on request at the end
        splines->positionSpline.setAutoCalculate(false);
        splines->rotationSpline.setAutoCalculate(false);
        splines->scaleSpline.setAutoCalculate(false);

        splines->positionSpline.clear();
        splines->rotationSpline.clear();
        splines->scaleSpline.clear();

        KeyFrameList::const_iterator i, iend;
        iend = mKeyFrames.end(); // precall to avoid overhead
        for (i = mKeyFrames.begin(); i != iend; ++i)
        {
			TransformKeyFrame* kf = static_cast<TransformKeyFrame*>(*i);
            splines->positionSpline.addPoint(kf->getTranslate());
            splines->rotationSpline.addPoint(kf->getRotation());
            splines->scaleSpline.addPoint(kf->getScale());
        }

        splines->positionSpline.recalcTangents();
        splines->rotationSpline.recalcTangents();
        splines->scaleSpline.recalcTangents();


        mSplineBuildNeeded = false;
    }

    //---------------------------------------------------------------------
	void NodeAnimationTrack::setUseShortestRotationPath(bool useShortestPath)
	{
		mUseShortestRotationPath = useShortestPath ;
	}

    //---------------------------------------------------------------------
	bool NodeAnimationTrack::getUseShortestRotationPath() const
	{
		return mUseShortestRotationPath ;
	}
    //---------------------------------------------------------------------
    void NodeAnimationTrack::_keyFrameDataChanged(void) const
    {
        mSplineBuildNeeded = true;
    }
    //---------------------------------------------------------------------
	bool NodeAnimationTrack::hasNonZeroKeyFrames(void) const
	{
        KeyFrameList::const_iterator i = mKeyFrames.begin();
        for (; i != mKeyFrames.end(); ++i)
        {
			// look for keyframes which have any component which is non-zero
			// Since exporters can be a little inaccurate sometimes we use a
			// tolerance value rather than looking for nothing
			TransformKeyFrame* kf = static_cast<TransformKeyFrame*>(*i);
			Vector3 trans = kf->getTranslate();
			Vector3 scale = kf->getScale();
			Vector3 axis;
			Radian angle;
			kf->getRotation().ToAngleAxis(angle, axis);
			Real tolerance = 1e-3f;
			if (!trans.positionEquals(Vector3::ZERO, tolerance) ||
				!scale.positionEquals(Vector3::UNIT_SCALE, tolerance) ||
				!Math::RealEqual(angle.valueRadians(), 0.0f, tolerance))
			{
				return true;
			}

		}

		return false;
	}
    //---------------------------------------------------------------------
	void NodeAnimationTrack::optimise(void)
	{
		// Eliminate duplicate keyframes from 2nd to penultimate keyframe
		// NB only eliminate middle keys from sequences of 5+ identical keyframes
		// since we need to preserve the boundary keys in place, and we need
		// 2 at each end to preserve tangents for spline interpolation
		Vector3 lasttrans = Vector3::ZERO;
		Vector3 lastscale = Vector3::ZERO;
		Quaternion lastorientation;
        KeyFrameList::iterator i = mKeyFrames.begin();
		Radian quatTolerance(1e-3f);
		list<unsigned short>::type removeList;
		unsigned short k = 0;
		ushort dupKfCount = 0;
        for (; i != mKeyFrames.end(); ++i, ++k)
        {
			TransformKeyFrame* kf = static_cast<TransformKeyFrame*>(*i);
			Vector3 newtrans = kf->getTranslate();
			Vector3 newscale = kf->getScale();
			Quaternion neworientation = kf->getRotation();
			// Ignore first keyframe; now include the last keyframe as we eliminate
			// only k-2 in a group of 5 to ensure we only eliminate middle keys
			if (i != mKeyFrames.begin() &&
				newtrans.positionEquals(lasttrans) &&
				newscale.positionEquals(lastscale) &&
				neworientation.equals(lastorientation, quatTolerance))
			{
				++dupKfCount;

				// 4 indicates this is the 5th duplicate keyframe
				if (dupKfCount == 4)
				{
					// remove the 'middle' keyframe
					removeList.push_back(k-2);
					--dupKfCount;
				}
			}
			else
			{
				// reset
				dupKfCount = 0;
				lasttrans = newtrans;
				lastscale = newscale;
				lastorientation = neworientation;
			}
		}

		// Now remove keyframes, in reverse order to avoid index revocation
		list<unsigned short>::type::reverse_iterator r = removeList.rbegin();
		for (; r!= removeList.rend(); ++r)
		{
			removeKeyFrame(*r);
		}


	}
	//--------------------------------------------------------------------------
	KeyFrame* NodeAnimationTrack::createKeyFrameImpl(Real time)
	{
		return OGRE_NEW TransformKeyFrame(this, time);
	}
	//--------------------------------------------------------------------------
	TransformKeyFrame* NodeAnimationTrack::createNodeKeyFrame(Real timePos)
	{
		return static_cast<TransformKeyFrame*>(createKeyFrame(timePos));
	}
	//--------------------------------------------------------------------------
	TransformKeyFrame* NodeAnimationTrack::getNodeKeyFrame(unsigned short index) const
	{
		return static_cast<TransformKeyFrame*>(getKeyFrame(index));
	}
    //---------------------------------------------------------------------
	NodeAnimationTrack* NodeAnimationTrack::_clone(Animation* newParent) const
	{
		NodeAnimationTrack* newTrack = 
			newParent->createNodeTrack(mHandle, mTargetNode);
		newTrack->mUseShortestRotationPath = mUseShortestRotationPath;
		populateClone(newTrack);
		return newTrack;
	}
	//--------------------------------------------------------------------------
	void NodeAnimationTrack::_applyBaseKeyFrame(const KeyFrame* b)
	{
		const TransformKeyFrame* base = static_cast<const TransformKeyFrame*>(b);
		
        for (KeyFrameList::iterator i = mKeyFrames.begin(); i != mKeyFrames.end(); ++i)
        {
			TransformKeyFrame* kf = static_cast<TransformKeyFrame*>(*i);
			kf->setTranslate(kf->getTranslate() - base->getTranslate());
			kf->setRotation(base->getRotation().Inverse() * kf->getRotation());
			kf->setScale(kf->getScale() * (Vector3::UNIT_SCALE / base->getScale()));
		}
			
	}
	//--------------------------------------------------------------------------
	VertexAnimationTrack::VertexAnimationTrack(Animation* parent,
		unsigned short handle, VertexAnimationType animType)
		: AnimationTrack(parent, handle)
		, mAnimationType(animType)
	{
	}
	//--------------------------------------------------------------------------
	VertexAnimationTrack::VertexAnimationTrack(Animation* parent, unsigned short handle,
		VertexAnimationType animType, VertexData* targetData, TargetMode target)
		: AnimationTrack(parent, handle)
		, mAnimationType(animType)
		, mTargetVertexData(targetData)
		, mTargetMode(target)
	{
	}
	//--------------------------------------------------------------------------
	VertexMorphKeyFrame* VertexAnimationTrack::createVertexMorphKeyFrame(Real timePos)
	{
		if (mAnimationType != VAT_MORPH)
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
				"Morph keyframes can only be created on vertex tracks of type morph.",
				"VertexAnimationTrack::createVertexMorphKeyFrame");
		}
		return static_cast<VertexMorphKeyFrame*>(createKeyFrame(timePos));
	}
	//--------------------------------------------------------------------------
	VertexPoseKeyFrame* VertexAnimationTrack::createVertexPoseKeyFrame(Real timePos)
	{
		if (mAnimationType != VAT_POSE)
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
				"Pose keyframes can only be created on vertex tracks of type pose.",
				"VertexAnimationTrack::createVertexPoseKeyFrame");
		}
		return static_cast<VertexPoseKeyFrame*>(createKeyFrame(timePos));
	}
	//--------------------------------------------------------------------------
	void VertexAnimationTrack::getInterpolatedKeyFrame(const TimeIndex& timeIndex, KeyFrame* kf) const
	{
		// Only relevant for pose animation
		if (mAnimationType == VAT_POSE)
		{
			// Get keyframes
			KeyFrame *kf1, *kf2;
			Real t = getKeyFramesAtTime(timeIndex, &kf1, &kf2);
			
			VertexPoseKeyFrame* vkfOut = static_cast<VertexPoseKeyFrame*>(kf);
			VertexPoseKeyFrame* vkf1 = static_cast<VertexPoseKeyFrame*>(kf1);
			VertexPoseKeyFrame* vkf2 = static_cast<VertexPoseKeyFrame*>(kf2);
			
			// For each pose reference in key 1, we need to locate the entry in
			// key 2 and interpolate the influence
			const VertexPoseKeyFrame::PoseRefList& poseList1 = vkf1->getPoseReferences();
			const VertexPoseKeyFrame::PoseRefList& poseList2 = vkf2->getPoseReferences();
			for (VertexPoseKeyFrame::PoseRefList::const_iterator p1 = poseList1.begin();
				 p1 != poseList1.end(); ++p1)
			{
				Real startInfluence = p1->influence;
				Real endInfluence = 0;
				// Search for entry in keyframe 2 list (if not there, will be 0)
				for (VertexPoseKeyFrame::PoseRefList::const_iterator p2 = poseList2.begin();
					 p2 != poseList2.end(); ++p2)
				{
					if (p1->poseIndex == p2->poseIndex)
					{
						endInfluence = p2->influence;
						break;
					}
				}
				// Interpolate influence
				Real influence = startInfluence + t*(endInfluence - startInfluence);
				
				vkfOut->addPoseReference(p1->poseIndex, influence);
				
				
			}
			// Now deal with any poses in key 2 which are not in key 1
			for (VertexPoseKeyFrame::PoseRefList::const_iterator p2 = poseList2.begin();
				 p2 != poseList2.end(); ++p2)
			{
				bool found = false;
				for (VertexPoseKeyFrame::PoseRefList::const_iterator p1 = poseList1.begin();
					 p1 != poseList1.end(); ++p1)
				{
					if (p1->poseIndex == p2->poseIndex)
					{
						found = true;
						break;
					}
				}
				if (!found)
				{
					// Need to apply this pose too, scaled from 0 start
					Real influence = t * p2->influence;
					
					vkfOut->addPoseReference(p2->poseIndex, influence);

				}
			} // key 2 iteration
			
		}			
		
		
	}
	//--------------------------------------------------------------------------
	bool VertexAnimationTrack::getVertexAnimationIncludesNormals() const
	{
		if (mAnimationType == VAT_NONE)
			return false;
		
		if (mAnimationType == VAT_MORPH)
		{
			bool normals = false;
			for (KeyFrameList::const_iterator i = mKeyFrames.begin(); i != mKeyFrames.end(); ++i)
			{
				VertexMorphKeyFrame* kf = static_cast<VertexMorphKeyFrame*>(*i);
				bool thisnorm = kf->getVertexBuffer()->getVertexSize() > 12;
				if (i == mKeyFrames.begin())
					normals = thisnorm;
				else
					// Only support normals if ALL keyframes include them
					normals = normals && thisnorm;

			}
			return normals;
		}
		else 
		{
			// needs to derive from Mesh::PoseList, can't tell here
			return false;
		}

				
	}
	//--------------------------------------------------------------------------
	void VertexAnimationTrack::apply(const TimeIndex& timeIndex, Real weight, Real scale)
	{
		applyToVertexData(mTargetVertexData, timeIndex, weight);
	}
	//--------------------------------------------------------------------------
	void VertexAnimationTrack::applyToVertexData(VertexData* data,
		const TimeIndex& timeIndex, Real weight, const PoseList* poseList)
	{
		// Nothing to do if no keyframes or no vertex data
		if (mKeyFrames.empty() || !data)
			return;

		// Get keyframes
		KeyFrame *kf1, *kf2;
		Real t = getKeyFramesAtTime(timeIndex, &kf1, &kf2);

		if (mAnimationType == VAT_MORPH)
		{
			VertexMorphKeyFrame* vkf1 = static_cast<VertexMorphKeyFrame*>(kf1);
			VertexMorphKeyFrame* vkf2 = static_cast<VertexMorphKeyFrame*>(kf2);

			if (mTargetMode == TM_HARDWARE)
			{
				// If target mode is hardware, need to bind our 2 keyframe buffers,
				// one to main pos, one to morph target texcoord
				assert(!data->hwAnimationDataList.empty() &&
					"Haven't set up hardware vertex animation elements!");

				// no use for TempBlendedBufferInfo here btw
				// NB we assume that position buffer is unshared, except for normals
				// VertexDeclaration::getAutoOrganisedDeclaration should see to that
				const VertexElement* posElem =
					data->vertexDeclaration->findElementBySemantic(VES_POSITION);
				// Set keyframe1 data as original position
				data->vertexBufferBinding->setBinding(
					posElem->getSource(), vkf1->getVertexBuffer());
				// Set keyframe2 data as derived
				data->vertexBufferBinding->setBinding(
					data->hwAnimationDataList[0].targetBufferIndex,
					vkf2->getVertexBuffer());
				// save T for use later
				data->hwAnimationDataList[0].parametric = t;

			}
			else
			{
				// If target mode is software, need to software interpolate each vertex

				Mesh::softwareVertexMorph(
					t, vkf1->getVertexBuffer(), vkf2->getVertexBuffer(), data);
			}
		}
		else
		{
			// Pose

			VertexPoseKeyFrame* vkf1 = static_cast<VertexPoseKeyFrame*>(kf1);
			VertexPoseKeyFrame* vkf2 = static_cast<VertexPoseKeyFrame*>(kf2);

			// For each pose reference in key 1, we need to locate the entry in
			// key 2 and interpolate the influence
			const VertexPoseKeyFrame::PoseRefList& poseList1 = vkf1->getPoseReferences();
			const VertexPoseKeyFrame::PoseRefList& poseList2 = vkf2->getPoseReferences();
			for (VertexPoseKeyFrame::PoseRefList::const_iterator p1 = poseList1.begin();
				p1 != poseList1.end(); ++p1)
			{
				Real startInfluence = p1->influence;
				Real endInfluence = 0;
				// Search for entry in keyframe 2 list (if not there, will be 0)
				for (VertexPoseKeyFrame::PoseRefList::const_iterator p2 = poseList2.begin();
					p2 != poseList2.end(); ++p2)
				{
					if (p1->poseIndex == p2->poseIndex)
					{
						endInfluence = p2->influence;
						break;
					}
				}
				// Interpolate influence
				Real influence = startInfluence + t*(endInfluence - startInfluence);
				// Scale by animation weight
				influence = weight * influence;
				// Get pose
				assert (poseList && p1->poseIndex < poseList->size());
				Pose* pose = (*poseList)[p1->poseIndex];
				// apply
				applyPoseToVertexData(pose, data, influence);
			}
			// Now deal with any poses in key 2 which are not in key 1
			for (VertexPoseKeyFrame::PoseRefList::const_iterator p2 = poseList2.begin();
				p2 != poseList2.end(); ++p2)
			{
				bool found = false;
				for (VertexPoseKeyFrame::PoseRefList::const_iterator p1 = poseList1.begin();
					p1 != poseList1.end(); ++p1)
				{
					if (p1->poseIndex == p2->poseIndex)
					{
						found = true;
						break;
					}
				}
				if (!found)
				{
					// Need to apply this pose too, scaled from 0 start
					Real influence = t * p2->influence;
					// Scale by animation weight
					influence = weight * influence;
					// Get pose
					assert (poseList && p2->poseIndex <= poseList->size());
					const Pose* pose = (*poseList)[p2->poseIndex];
					// apply
					applyPoseToVertexData(pose, data, influence);
				}
			} // key 2 iteration
		} // morph or pose animation
	}
	//-----------------------------------------------------------------------------
	void VertexAnimationTrack::applyPoseToVertexData(const Pose* pose,
		VertexData* data, Real influence)
	{
		if (mTargetMode == TM_HARDWARE)
		{
			// Hardware
			// If target mode is hardware, need to bind our pose buffer
			// to a target texcoord
			assert(!data->hwAnimationDataList.empty() &&
				"Haven't set up hardware vertex animation elements!");
			// no use for TempBlendedBufferInfo here btw
			// Set pose target as required
			size_t hwIndex = data->hwAnimDataItemsUsed++;
			// If we try to use too many poses, ignore extras
			if (hwIndex < data->hwAnimationDataList.size())
			{
				VertexData::HardwareAnimationData& animData = data->hwAnimationDataList[hwIndex];
				data->vertexBufferBinding->setBinding(
					animData.targetBufferIndex,
					pose->_getHardwareVertexBuffer(data));
				// save final influence in parametric
				animData.parametric = influence;

			}

		}
		else
		{
			// Software
			Mesh::softwareVertexPoseBlend(influence, pose->getVertexOffsets(), pose->getNormals(), data);
		}

	}
	//--------------------------------------------------------------------------
	VertexMorphKeyFrame* VertexAnimationTrack::getVertexMorphKeyFrame(unsigned short index) const
	{
		if (mAnimationType != VAT_MORPH)
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
				"Morph keyframes can only be created on vertex tracks of type morph.",
				"VertexAnimationTrack::getVertexMorphKeyFrame");
		}

		return static_cast<VertexMorphKeyFrame*>(getKeyFrame(index));
	}
	//--------------------------------------------------------------------------
	VertexPoseKeyFrame* VertexAnimationTrack::getVertexPoseKeyFrame(unsigned short index) const
	{
		if (mAnimationType != VAT_POSE)
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
				"Pose keyframes can only be created on vertex tracks of type pose.",
				"VertexAnimationTrack::getVertexPoseKeyFrame");
		}

		return static_cast<VertexPoseKeyFrame*>(getKeyFrame(index));
	}
	//--------------------------------------------------------------------------
	KeyFrame* VertexAnimationTrack::createKeyFrameImpl(Real time)
	{
		switch(mAnimationType)
		{
		default:
		case VAT_MORPH:
            return OGRE_NEW VertexMorphKeyFrame(this, time);
		case VAT_POSE:
			return OGRE_NEW VertexPoseKeyFrame(this, time);
		};

	}
	//---------------------------------------------------------------------
	bool VertexAnimationTrack::hasNonZeroKeyFrames(void) const
	{
		if (mAnimationType == VAT_MORPH)
		{
			return !mKeyFrames.empty();
		}
		else
		{

			KeyFrameList::const_iterator i = mKeyFrames.begin();
			for (; i != mKeyFrames.end(); ++i)
			{
				// look for keyframes which have a pose influence which is non-zero
				const VertexPoseKeyFrame* kf = static_cast<const VertexPoseKeyFrame*>(*i);
				VertexPoseKeyFrame::ConstPoseRefIterator poseIt
					= kf->getPoseReferenceIterator();
				while (poseIt.hasMoreElements())
				{
					const VertexPoseKeyFrame::PoseRef& poseRef = poseIt.getNext();
					if (poseRef.influence > 0.0f)
						return true;
				}

			}

			return false;
		}
	}
	//---------------------------------------------------------------------
	void VertexAnimationTrack::optimise(void)
	{
		// TODO - remove sequences of duplicate pose references?


	}
    //---------------------------------------------------------------------
	VertexAnimationTrack* VertexAnimationTrack::_clone(Animation* newParent) const
	{
		VertexAnimationTrack* newTrack = 
			newParent->createVertexTrack(mHandle, mAnimationType);
		newTrack->mTargetMode = mTargetMode;
		populateClone(newTrack);
		return newTrack;
	}	
	//--------------------------------------------------------------------------
	void VertexAnimationTrack::_applyBaseKeyFrame(const KeyFrame* b)
	{
		const VertexPoseKeyFrame* base = static_cast<const VertexPoseKeyFrame*>(b);
		
        for (KeyFrameList::iterator i = mKeyFrames.begin(); i != mKeyFrames.end(); ++i)
        {
			VertexPoseKeyFrame* kf = static_cast<VertexPoseKeyFrame*>(*i);
			
			kf->_applyBaseKeyFrame(base);
		}
		
	}
	

}

