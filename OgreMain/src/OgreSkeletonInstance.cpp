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
#include "OgreSkeletonInstance.h"
#include "OgreBone.h"
#include "OgreTagPoint.h"


namespace Ogre {
    //-------------------------------------------------------------------------
    SkeletonInstance::SkeletonInstance(const SkeletonPtr& masterCopy) 
        : Skeleton()
        , mSkeleton(masterCopy)
        , mNextTagPointAutoHandle(0)
    {
    }
    //-------------------------------------------------------------------------
    SkeletonInstance::~SkeletonInstance()
    {
        // have to call this here rather than in Resource destructor
        // since calling virtual methods in base destructors causes crash
        // ...and calling it in Skeleton destructor does not unload
        // SkeletonInstance since it has seized to be by then.
        unload();
    }
    //-------------------------------------------------------------------------
    unsigned short SkeletonInstance::getNumAnimations(void) const
    {
        return mSkeleton->getNumAnimations();
    }
    //-------------------------------------------------------------------------
    Animation* SkeletonInstance::getAnimation(unsigned short index) const
    {
        return mSkeleton->getAnimation(index);
    }
    //-------------------------------------------------------------------------
    Animation* SkeletonInstance::createAnimation(const String& name, Real length)
    {
        return mSkeleton->createAnimation(name, length);
    }
    //-------------------------------------------------------------------------
    Animation* SkeletonInstance::getAnimation(const String& name, 
		const LinkedSkeletonAnimationSource** linker) const
    {
        return mSkeleton->getAnimation(name, linker);
    }
	//-------------------------------------------------------------------------
	Animation* SkeletonInstance::_getAnimationImpl(const String& name, 
		const LinkedSkeletonAnimationSource** linker) const
	{
		return mSkeleton->_getAnimationImpl(name, linker);
	}
    //-------------------------------------------------------------------------
    void SkeletonInstance::removeAnimation(const String& name)
    {
        mSkeleton->removeAnimation(name);
    }
	//-------------------------------------------------------------------------
	void SkeletonInstance::addLinkedSkeletonAnimationSource(const String& skelName, 
		Real scale)
	{
		mSkeleton->addLinkedSkeletonAnimationSource(skelName, scale);
	}
	//-------------------------------------------------------------------------
	void SkeletonInstance::removeAllLinkedSkeletonAnimationSources(void)
	{
		mSkeleton->removeAllLinkedSkeletonAnimationSources();
	}
	//-------------------------------------------------------------------------
	Skeleton::LinkedSkeletonAnimSourceIterator 
	SkeletonInstance::getLinkedSkeletonAnimationSourceIterator(void) const
	{
		return mSkeleton->getLinkedSkeletonAnimationSourceIterator();
	}
	//-------------------------------------------------------------------------
	void SkeletonInstance::_initAnimationState(AnimationStateSet* animSet)
	{
		mSkeleton->_initAnimationState(animSet);
	}
	//-------------------------------------------------------------------------
	void SkeletonInstance::_refreshAnimationState(AnimationStateSet* animSet)
	{
		mSkeleton->_refreshAnimationState(animSet);
	}
    //-------------------------------------------------------------------------
    void SkeletonInstance::cloneBoneAndChildren(Bone* source, Bone* parent)
    {
        Bone* newBone;
        if (source->getName().empty())
        {
            newBone = createBone(source->getHandle());
        }
        else
        {
            newBone = createBone(source->getName(), source->getHandle());
        }
        if (parent == NULL)
        {
            mRootBones.push_back(newBone);
        }
        else
        {
            parent->addChild(newBone);
        }
        newBone->setOrientation(source->getOrientation());
        newBone->setPosition(source->getPosition());
        newBone->setScale(source->getScale());

        // Process children
        Node::ChildNodeIterator it = source->getChildIterator();
        while (it.hasMoreElements())
        {
            cloneBoneAndChildren(static_cast<Bone*>(it.getNext()), newBone);
        }
    }
    //-------------------------------------------------------------------------
    void SkeletonInstance::loadImpl(void)
    {
        mNextAutoHandle = mSkeleton->mNextAutoHandle;
        mNextTagPointAutoHandle = 0;
        // construct self from master
        mBlendState = mSkeleton->mBlendState;
        // Copy bones
        BoneIterator i = mSkeleton->getRootBoneIterator();
        while (i.hasMoreElements())
        {
            Bone* b = i.getNext();
            cloneBoneAndChildren(b, 0);
            b->_update(true, false);
        }
        setBindingPose();
    }
    //-------------------------------------------------------------------------
    void SkeletonInstance::unloadImpl(void)
    {
        Skeleton::unloadImpl();

        // destroy TagPoints
        for (TagPointList::const_iterator it = mActiveTagPoints.begin(); it != mActiveTagPoints.end(); ++it)
        {
            TagPoint* tagPoint = *it;
            // Woohoo! The child object all the same attaching this skeleton instance, but is ok we can just
            // ignore it:
            //   1. The parent node of the tagPoint already deleted by Skeleton::unload(), nothing need to do now
            //   2. And the child object relationship already detached by Entity::~Entity()
            OGRE_DELETE tagPoint;
        }
        mActiveTagPoints.clear();
        for (TagPointList::const_iterator it2 = mFreeTagPoints.begin(); it2 != mFreeTagPoints.end(); ++it2)
        {
            TagPoint* tagPoint = *it2;
            OGRE_DELETE tagPoint;
        }
        mFreeTagPoints.clear();
    }

    //-------------------------------------------------------------------------
    TagPoint* SkeletonInstance::createTagPointOnBone(Bone* bone,
        const Quaternion &offsetOrientation, 
        const Vector3 &offsetPosition)
    {
        TagPoint* ret;
        if (mFreeTagPoints.empty()) {
            ret = OGRE_NEW TagPoint(mNextTagPointAutoHandle++, this);
            mActiveTagPoints.push_back(ret);
        } else {
            ret = mFreeTagPoints.front();
            mActiveTagPoints.splice(
                mActiveTagPoints.end(), mFreeTagPoints, mFreeTagPoints.begin());
            // Initial some members ensure identically behavior, avoiding potential bug.
            ret->setParentEntity(0);
            ret->setChildObject(0);
            ret->setInheritOrientation(true);
            ret->setInheritScale(true);
            ret->setInheritParentEntityOrientation(true);
            ret->setInheritParentEntityScale(true);
        }

        ret->setPosition(offsetPosition);
        ret->setOrientation(offsetOrientation);
        ret->setScale(Vector3::UNIT_SCALE);
        ret->setBindingPose();
        bone->addChild(ret);

        return ret;
    }
    //-------------------------------------------------------------------------
    void SkeletonInstance::freeTagPoint(TagPoint* tagPoint)
    {
        TagPointList::iterator it =
            std::find(mActiveTagPoints.begin(), mActiveTagPoints.end(), tagPoint);
        assert(it != mActiveTagPoints.end());
        if (it != mActiveTagPoints.end())
        {
            if (tagPoint->getParent())
                tagPoint->getParent()->removeChild(tagPoint);

            mFreeTagPoints.splice(mFreeTagPoints.end(), mActiveTagPoints, it);
        }
    }
	//-------------------------------------------------------------------------
	const String& SkeletonInstance::getName(void) const
	{
		// delegate
		return mSkeleton->getName();
	}
	//-------------------------------------------------------------------------
	ResourceHandle SkeletonInstance::getHandle(void) const
	{
		// delegate
		return mSkeleton->getHandle();
	}
	//-------------------------------------------------------------------------
	const String& SkeletonInstance::getGroup(void)
	{
		// delegate
		return mSkeleton->getGroup();
	}


}

