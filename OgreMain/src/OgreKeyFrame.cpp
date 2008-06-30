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


}

