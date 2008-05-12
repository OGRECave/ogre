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
#include "OgreMovablePlane.h"
#include "OgreNode.h"

namespace Ogre {

    String MovablePlane::msMovableType = "MovablePlane";
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    MovablePlane::MovablePlane(const String& name) : Plane(), MovableObject(name),
        mLastTranslate(Vector3::ZERO), 
        mLastRotate(Quaternion::IDENTITY),
        mDirty(true)
    {
    }
    //-----------------------------------------------------------------------
    MovablePlane::MovablePlane (const Plane& rhs) : Plane(rhs), 
        mLastTranslate(Vector3::ZERO), mLastRotate(Quaternion::IDENTITY), 
        mDirty(true)
    {
    }
    //-----------------------------------------------------------------------
    MovablePlane::MovablePlane (const Vector3& rkNormal, Real fConstant)
        : Plane (rkNormal, fConstant), mLastTranslate(Vector3::ZERO), 
        mLastRotate(Quaternion::IDENTITY), mDirty(true)
    {
    }
    //-----------------------------------------------------------------------
    MovablePlane::MovablePlane (const Vector3& rkNormal, const Vector3& rkPoint)
        : Plane(rkNormal, rkPoint), mLastTranslate(Vector3::ZERO), 
        mLastRotate(Quaternion::IDENTITY), mDirty(true)
    {
    }
    //-----------------------------------------------------------------------
    MovablePlane::MovablePlane (const Vector3& rkPoint0, const Vector3& rkPoint1,
        const Vector3& rkPoint2)
        : Plane(rkPoint0, rkPoint1, rkPoint2), mLastTranslate(Vector3::ZERO), 
        mLastRotate(Quaternion::IDENTITY), mDirty(true)
    {
    }
    //-----------------------------------------------------------------------
    const Plane& MovablePlane::_getDerivedPlane(void) const
    {
        if (mParentNode)
        {
            if (mDirty ||
                !(mParentNode->_getDerivedOrientation() == mLastRotate &&
                mParentNode->_getDerivedPosition() == mLastTranslate))
            {
                mLastRotate = mParentNode->_getDerivedOrientation();
                mLastTranslate = mParentNode->_getDerivedPosition();
                // Rotate normal
                mDerivedPlane.normal = mLastRotate * normal;
                // d remains the same in rotation, since rotation happens first
                mDerivedPlane.d = d;
                // Add on the effect of the translation (project onto new normal)
                mDerivedPlane.d -= mDerivedPlane.normal.dotProduct(mLastTranslate);

                mDirty = false;

            }
        }
        else
        {
            return *this;
        }

        return mDerivedPlane;
    }
    //-----------------------------------------------------------------------
    const String& MovablePlane::getMovableType(void) const
    {
        return msMovableType;
    }
}
