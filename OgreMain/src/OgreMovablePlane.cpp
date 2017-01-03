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
