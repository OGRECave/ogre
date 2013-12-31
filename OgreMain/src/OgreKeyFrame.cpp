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

#include "OgreKeyFrame.h"
#include "OgreAnimationTrack.h"
#include "OgreHardwareBufferManager.h"

namespace Ogre
{
    //---------------------------------------------------------------------
    KeyFrame::KeyFrame(const AnimationTrack* parent, Real time) 
        : mTime(time), mParentTrack(parent)
    {
    }
	//---------------------------------------------------------------------
	KeyFrame* KeyFrame::_clone(AnimationTrack* newParent) const
	{
		return OGRE_NEW KeyFrame(newParent, mTime);
	}
	//---------------------------------------------------------------------
	NumericKeyFrame::NumericKeyFrame(const AnimationTrack* parent, Real time)
		:KeyFrame(parent, time)
	{
	}
	//---------------------------------------------------------------------
	const AnyNumeric& NumericKeyFrame::getValue(void) const
	{
		return mValue;
	}
	//---------------------------------------------------------------------
	void NumericKeyFrame::setValue(const AnyNumeric& val)
	{
		mValue = val;
	}
    //---------------------------------------------------------------------
	KeyFrame* NumericKeyFrame::_clone(AnimationTrack* newParent) const
	{
		NumericKeyFrame* newKf = OGRE_NEW NumericKeyFrame(newParent, mTime);
		newKf->mValue = mValue;
		return newKf;
	}
    //---------------------------------------------------------------------
	TransformKeyFrame::TransformKeyFrame(const AnimationTrack* parent, Real time)
		:KeyFrame(parent, time), mTranslate(Vector3::ZERO), 
		mScale(Vector3::UNIT_SCALE), mRotate(Quaternion::IDENTITY) 
	{
	}
	//---------------------------------------------------------------------
    void TransformKeyFrame::setTranslate(const Vector3& trans)
    {
        mTranslate = trans;
        if (mParentTrack)
            mParentTrack->_keyFrameDataChanged();
    }
    //---------------------------------------------------------------------
    const Vector3& TransformKeyFrame::getTranslate(void) const
    {
        return mTranslate;
    }
    //---------------------------------------------------------------------
    void TransformKeyFrame::setScale(const Vector3& scale)
    {
        mScale = scale;
        if (mParentTrack)
            mParentTrack->_keyFrameDataChanged();
    }
    //---------------------------------------------------------------------
    const Vector3& TransformKeyFrame::getScale(void) const
    {
        return mScale;
    }
    //---------------------------------------------------------------------
    void TransformKeyFrame::setRotation(const Quaternion& rot)
    {
        mRotate = rot;
        if (mParentTrack)
            mParentTrack->_keyFrameDataChanged();
    }
    //---------------------------------------------------------------------
    const Quaternion& TransformKeyFrame::getRotation(void) const
    {
        return mRotate;
    }
    //---------------------------------------------------------------------
	KeyFrame* TransformKeyFrame::_clone(AnimationTrack* newParent) const
	{
		TransformKeyFrame* newKf = OGRE_NEW TransformKeyFrame(newParent, mTime);
		newKf->mTranslate = mTranslate;
		newKf->mScale = mScale;
		newKf->mRotate = mRotate;
		return newKf;
	}
	//---------------------------------------------------------------------
	VertexMorphKeyFrame::VertexMorphKeyFrame(const AnimationTrack* parent, Real time)
		: KeyFrame(parent, time)
	{
	}
	//---------------------------------------------------------------------
	void VertexMorphKeyFrame::setVertexBuffer(const HardwareVertexBufferSharedPtr& buf)
	{
		mBuffer = buf;
	}
	//---------------------------------------------------------------------
	const HardwareVertexBufferSharedPtr& 
	VertexMorphKeyFrame::getVertexBuffer(void) const
	{
		return mBuffer;
	}
    //---------------------------------------------------------------------
	KeyFrame* VertexMorphKeyFrame::_clone(AnimationTrack* newParent) const
	{
		VertexMorphKeyFrame* newKf = OGRE_NEW VertexMorphKeyFrame(newParent, mTime);
		newKf->mBuffer = mBuffer;
		return newKf;
	}	
	//---------------------------------------------------------------------
	VertexPoseKeyFrame::VertexPoseKeyFrame(const AnimationTrack* parent, Real time)
		:KeyFrame(parent, time)
	{
	}
	//---------------------------------------------------------------------
	void VertexPoseKeyFrame::addPoseReference(ushort poseIndex, Real influence)
	{
		mPoseRefs.push_back(PoseRef(poseIndex, influence));
	}
	//---------------------------------------------------------------------
	void VertexPoseKeyFrame::updatePoseReference(ushort poseIndex, Real influence)
	{
		for (PoseRefList::iterator i = mPoseRefs.begin(); i != mPoseRefs.end(); ++i)
		{
			if (i->poseIndex == poseIndex)
			{
				i->influence = influence;
				return;
			}
		}
		// if we got here, we didn't find it
		addPoseReference(poseIndex, influence);

	}
	//---------------------------------------------------------------------
	void VertexPoseKeyFrame::removePoseReference(ushort poseIndex)
	{
		for (PoseRefList::iterator i = mPoseRefs.begin(); i != mPoseRefs.end(); ++i)
		{
			if (i->poseIndex == poseIndex)
			{
				mPoseRefs.erase(i);
				return;
			}
		}
	}
	//---------------------------------------------------------------------
	void VertexPoseKeyFrame::removeAllPoseReferences(void)
	{
		mPoseRefs.clear();
	}
	//---------------------------------------------------------------------
	const VertexPoseKeyFrame::PoseRefList& 
	VertexPoseKeyFrame::getPoseReferences(void) const
	{
		return mPoseRefs;
	}
	//---------------------------------------------------------------------
	VertexPoseKeyFrame::PoseRefIterator 
	VertexPoseKeyFrame::getPoseReferenceIterator(void)
	{
		return PoseRefIterator(mPoseRefs.begin(), mPoseRefs.end());
	}
	//---------------------------------------------------------------------
	VertexPoseKeyFrame::ConstPoseRefIterator 
	VertexPoseKeyFrame::getPoseReferenceIterator(void) const
	{
		return ConstPoseRefIterator(mPoseRefs.begin(), mPoseRefs.end());
	}
    //---------------------------------------------------------------------
	KeyFrame* VertexPoseKeyFrame::_clone(AnimationTrack* newParent) const
	{
		VertexPoseKeyFrame* newKf = OGRE_NEW VertexPoseKeyFrame(newParent, mTime);
		// By-value copy ok
		newKf->mPoseRefs = mPoseRefs;
		return newKf;
	}	
	//---------------------------------------------------------------------
	void VertexPoseKeyFrame::_applyBaseKeyFrame(const VertexPoseKeyFrame* base)
	{
		// We subtract the matching pose influences in the base keyframe from the
		// influences in this keyframe
		for (PoseRefList::iterator i = mPoseRefs.begin(); i != mPoseRefs.end(); ++i)
		{
			PoseRef& myPoseRef = *i;
			
			VertexPoseKeyFrame::ConstPoseRefIterator basePoseIt = base->getPoseReferenceIterator();
			Real baseInfluence = 0.0f;
			while (basePoseIt.hasMoreElements())
			{
				const VertexPoseKeyFrame::PoseRef& basePoseRef = basePoseIt.getNext();
				if (basePoseRef.poseIndex == myPoseRef.poseIndex)
				{
					baseInfluence = basePoseRef.influence;
					break;
				}
			}
			
			myPoseRef.influence -= baseInfluence;
			
		}
		
	}


}

