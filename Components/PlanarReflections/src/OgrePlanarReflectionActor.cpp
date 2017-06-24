/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2017 Torus Knot Software Ltd

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

#include "OgrePlanarReflectionActor.h"
#include "Math/Array/OgreArrayQuaternion.h"

namespace Ogre
{
    void PlanarReflectionActor::updateArrayActorPlane(void)
    {
        mActorPlane->planeNormals.setFromQuaternion( mOrientation, mIndex );

        Mathlib::Set( mActorPlane->planeNegD[0], -mPlane.d, mIndex );
        Plane p;
        p.redefine( -mOrientation.yAxis(), mCenter + mOrientation.yAxis() * mHalfSize.y );
        Mathlib::Set( mActorPlane->planeNegD[1], -p.d, mIndex );
        p.redefine(  mOrientation.yAxis(), mCenter - mOrientation.yAxis() * mHalfSize.y );
        Mathlib::Set( mActorPlane->planeNegD[2], -p.d, mIndex );
        p.redefine( -mOrientation.xAxis(), mCenter + mOrientation.xAxis() * mHalfSize.x );
        Mathlib::Set( mActorPlane->planeNegD[3], -p.d, mIndex );
        p.redefine(  mOrientation.xAxis(), mCenter - mOrientation.xAxis() * mHalfSize.x );
        Mathlib::Set( mActorPlane->planeNegD[4], -p.d, mIndex );

        mActorPlane->center.setFromVector3( mCenter, mIndex );
        Mathlib::Set( mActorPlane->xyHalfSize[0], mHalfSize.x, mIndex );
        Mathlib::Set( mActorPlane->xyHalfSize[1], mHalfSize.y, mIndex );
    }
    //-----------------------------------------------------------------------------------
    void PlanarReflectionActor::setPlane( const Vector3 &center, const Vector2 &halfSize,
                                          const Quaternion &orientation )
    {
        mCenter = center;
        mHalfSize = halfSize;
        mOrientation = orientation;
        mPlane.redefine( orientation.zAxis(), center );

        updateArrayActorPlane();
    }
    //-----------------------------------------------------------------------------------
    const Vector3& PlanarReflectionActor::getCenter(void) const
    {
        return mCenter;
    }
    //-----------------------------------------------------------------------------------
    const Vector2& PlanarReflectionActor::getHalfSize(void) const
    {
        return mHalfSize;
    }
    //-----------------------------------------------------------------------------------
    const Quaternion& PlanarReflectionActor::getOrientation(void) const
    {
        return mOrientation;
    }
    //-----------------------------------------------------------------------------------
    const Vector3& PlanarReflectionActor::getNormal(void) const
    {
        return mPlane.normal;
    }
    //-----------------------------------------------------------------------------------
    const Plane& PlanarReflectionActor::getPlane(void) const
    {
        return mPlane;
    }
    //-----------------------------------------------------------------------------------
    bool PlanarReflectionActor::hasReservation(void) const
    {
        return mHasReservation;
    }
    //-----------------------------------------------------------------------------------
    uint8 PlanarReflectionActor::getCurrentBoundSlot(void) const
    {
        return mCurrentBoundSlot;
    }
    //-----------------------------------------------------------------------------------
    Real PlanarReflectionActor::getSquaredDistanceTo( const Vector3 &pos ) const
    {
        Vector3 projectedPos = pos - mPlane.normal * mPlane.getDistance( pos );

        Vector3 localPos = mOrientation.Inverse() * (projectedPos - mCenter);
        localPos.makeFloor( Vector3( mHalfSize.x, mHalfSize.y, 0.0f ) );
        localPos.makeCeil( -Vector3( mHalfSize.x, mHalfSize.y, 0.0f ) );

        Vector3 clampedProjPos = mCenter + mOrientation * localPos;
        return clampedProjPos.squaredDistance( pos );
    }
}
