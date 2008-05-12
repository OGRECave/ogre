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

#include "OgreBone.h"
#include "OgreSkeleton.h"

namespace Ogre {

    //---------------------------------------------------------------------
    Bone::Bone(unsigned short handle, Skeleton* creator) 
        : Node(), mHandle(handle), mManuallyControlled(false), mCreator(creator)
    {
    }
    //---------------------------------------------------------------------
    Bone::Bone(const String& name, unsigned short handle, Skeleton* creator) 
        : Node(name), mHandle(handle), mManuallyControlled(false), mCreator(creator)
    {
    }
    //---------------------------------------------------------------------
    Bone::~Bone()
    {
    }
    //---------------------------------------------------------------------
    Bone* Bone::createChild(unsigned short handle, const Vector3& translate, 
        const Quaternion& rotate)
    {
        Bone* retBone = mCreator->createBone(handle);
        retBone->translate(translate);
        retBone->rotate(rotate);
        this->addChild(retBone);
        return retBone;
    }
    //---------------------------------------------------------------------
    Node* Bone::createChildImpl(void)
    {
        return mCreator->createBone();
    }
    //---------------------------------------------------------------------
    Node* Bone::createChildImpl(const String& name)
    {
        return mCreator->createBone(name);
    }
    //---------------------------------------------------------------------
    void Bone::setBindingPose(void)
    {
        setInitialState();

        // Save inverse derived position/scale/orientation, used for calculate offset transform later
        mBindDerivedInversePosition = - _getDerivedPosition();
        mBindDerivedInverseScale = Vector3::UNIT_SCALE / _getDerivedScale();
        mBindDerivedInverseOrientation = _getDerivedOrientation().Inverse();
    }
    //---------------------------------------------------------------------
    void Bone::reset(void)
    {
        resetToInitialState();
    }
    //---------------------------------------------------------------------
    void Bone::setManuallyControlled(bool manuallyControlled) 
	{
        mManuallyControlled = manuallyControlled;
		mCreator->_notifyManualBoneStateChange(this);
    }
    //---------------------------------------------------------------------
    bool Bone::isManuallyControlled() const {
        return mManuallyControlled;
    }
    //---------------------------------------------------------------------
    void Bone::_getOffsetTransform(Matrix4& m) const
    {
        // Combine scale with binding pose inverse scale,
        // NB just combine as equivalent axes, no shearing
        Vector3 scale = _getDerivedScale() * mBindDerivedInverseScale;

        // Combine orientation with binding pose inverse orientation
        Quaternion rotate = _getDerivedOrientation() * mBindDerivedInverseOrientation;

        // Combine position with binding pose inverse position,
        // Note that translation is relative to scale & rotation,
        // so first reverse transform original derived position to
        // binding pose bone space, and then transform to current
        // derived bone space.
        Vector3 translate = _getDerivedPosition() + rotate * (scale * mBindDerivedInversePosition);

        m.makeTransform(translate, scale, rotate);
    }
    //---------------------------------------------------------------------
    unsigned short Bone::getHandle(void) const
    {
        return mHandle;
    }
	//---------------------------------------------------------------------
	void Bone::needUpdate(bool forceParentUpdate)
	{
		Node::needUpdate(forceParentUpdate);

		if (isManuallyControlled())
		{
			// Dirty the skeleton if manually controlled so animation can be updated
			mCreator->_notifyManualBonesDirty();
		}

	}





}

