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
// This file is based on material originally from:
// Geometric Tools, LLC
// Copyright (c) 1998-2010
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt


#ifndef __Plane_H__
#define __Plane_H__

#include "OgrePrerequisites.h"

#include "OgreVector.h"
#include "OgreAxisAlignedBox.h"

namespace Ogre {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Math
    *  @{
    */
    /** Defines a plane in 3D space.
        @remarks
            A plane is defined in 3D space by the equation
            Ax + By + Cz + D = 0
        @par
            This equates to a vector (the normal of the plane, whose x, y
            and z components equate to the coefficients A, B and C
            respectively), and a constant (D) which is the distance along
            the normal you have to go to move the plane back to the origin.
     */
    class _OgreExport Plane
    {
    public:
        Vector3 normal;
        Real d;

    public:
        /** Default constructor - sets everything to 0.
        */
        Plane() : normal(Vector3::ZERO), d(0.0f) {}
        /** Construct a plane through a normal, and a distance to move the plane along the normal.*/
        Plane(const Vector3& rkNormal, Real fConstant)
        {
            normal = rkNormal;
            d = -fConstant;
        }
        /** Construct a plane using the 4 constants directly **/
        Plane(Real a, Real b, Real c, Real _d) : normal(a, b, c), d(_d) {}
        /// @overload
        explicit Plane(const Vector4& v) : normal(v.xyz()), d(v.w) {}
        Plane(const Vector3& rkNormal, const Vector3& rkPoint)
        {
            redefine(rkNormal, rkPoint);
        }
        Plane(const Vector3& p0, const Vector3& p1, const Vector3& p2)
        {
            redefine(p0, p1, p2);
        }

        /** The "positive side" of the plane is the half space to which the
            plane normal points. The "negative side" is the other half
            space. The flag "no side" indicates the plane itself.
        */
        enum Side
        {
            NO_SIDE,
            POSITIVE_SIDE,
            NEGATIVE_SIDE,
            BOTH_SIDE
        };

        Side getSide(const Vector3& rkPoint) const
        {
            Real fDistance = getDistance(rkPoint);

            if (fDistance < 0.0)
                return Plane::NEGATIVE_SIDE;

            if (fDistance > 0.0)
                return Plane::POSITIVE_SIDE;

            return Plane::NO_SIDE;
        }

        /**
        Returns the side where the alignedBox is. The flag BOTH_SIDE indicates an intersecting box.
        One corner ON the plane is sufficient to consider the box and the plane intersecting.
        */
        Side getSide(const AxisAlignedBox& box) const
        {
            if (box.isNull())
                return NO_SIDE;
            if (box.isInfinite())
                return BOTH_SIDE;

            return getSide(box.getCenter(), box.getHalfSize());
        }

        /** Returns which side of the plane that the given box lies on.
            The box is defined as centre/half-size pairs for effectively.
        @param centre The centre of the box.
        @param halfSize The half-size of the box.
        @return
            POSITIVE_SIDE if the box complete lies on the "positive side" of the plane,
            NEGATIVE_SIDE if the box complete lies on the "negative side" of the plane,
            and BOTH_SIDE if the box intersects the plane.
        */
        Side getSide(const Vector3& centre, const Vector3& halfSize) const
        {
            // Calculate the distance between box centre and the plane
            Real dist = getDistance(centre);

            // Calculate the maximise allows absolute distance for
            // the distance between box centre and plane
            Real maxAbsDist = normal.absDotProduct(halfSize);

            if (dist < -maxAbsDist)
                return NEGATIVE_SIDE;

            if (dist > +maxAbsDist)
                return POSITIVE_SIDE;

            return BOTH_SIDE;
        }

        /** This is a pseudodistance. The sign of the return value is
            positive if the point is on the positive side of the plane,
            negative if the point is on the negative side, and zero if the
            point is on the plane.
            @par
            The absolute value of the return value is the true distance only
            when the plane normal is a unit length vector.
        */
        Real getDistance(const Vector3& rkPoint) const
        {
            return normal.dotProduct(rkPoint) + d;
        }

        /** Redefine this plane based on 3 points. */
        void redefine(const Vector3& p0, const Vector3& p1, const Vector3& p2)
        {
            normal = Math::calculateBasicFaceNormal(p0, p1, p2);
            d = -normal.dotProduct(p0);
        }

        /** Redefine this plane based on a normal and a point. */
        void redefine(const Vector3& rkNormal, const Vector3& rkPoint)
        {
            normal = rkNormal;
            d = -rkNormal.dotProduct(rkPoint);
        }

        /** Project a vector onto the plane. 
        @remarks This gives you the element of the input vector that is perpendicular 
            to the normal of the plane. You can get the element which is parallel
            to the normal of the plane by subtracting the result of this method
            from the original vector, since parallel + perpendicular = original.
        @param v The input vector
        */
        Vector3 projectVector(const Vector3& v) const
        {
            // We know plane normal is unit length, so use simple method
            Matrix3 xform;
            xform[0][0] = 1.0f - normal.x * normal.x;
            xform[0][1] = -normal.x * normal.y;
            xform[0][2] = -normal.x * normal.z;
            xform[1][0] = -normal.y * normal.x;
            xform[1][1] = 1.0f - normal.y * normal.y;
            xform[1][2] = -normal.y * normal.z;
            xform[2][0] = -normal.z * normal.x;
            xform[2][1] = -normal.z * normal.y;
            xform[2][2] = 1.0f - normal.z * normal.z;
            return xform * v;
        }

        /** Normalises the plane.
            @remarks
                This method normalises the plane's normal and the length scale of d
                is as well.
            @note
                This function will not crash for zero-sized vectors, but there
                will be no changes made to their components.
            @return The previous length of the plane's normal.
        */
        Real normalise(void)
        {
            Real fLength = normal.length();

            // Will also work for zero-sized vectors, but will change nothing
            // We're not using epsilons because we don't need to.
            // Read http://www.ogre3d.org/forums/viewtopic.php?f=4&t=61259
            if (fLength > Real(0.0f))
            {
                Real fInvLength = 1.0f / fLength;
                normal *= fInvLength;
                d *= fInvLength;
            }

            return fLength;
        }

        /// Get flipped plane, with same location but reverted orientation
        Plane operator - () const
        {
            return Plane(-(normal.x), -(normal.y), -(normal.z), -d); // not equal to Plane(-normal, -d)
        }

        /// Comparison operator
        bool operator==(const Plane& rhs) const
        {
            return (rhs.d == d && rhs.normal == normal);
        }
        bool operator!=(const Plane& rhs) const
        {
            return (rhs.d != d || rhs.normal != normal);
        }

        friend std::ostream& operator<<(std::ostream& o, const Plane& p)
        {
            o << "Plane(normal=" << p.normal << ", d=" << p.d << ")";
            return o;
        }
    };

    inline Plane operator * (const Matrix4& mat, const Plane& p)
    {
        Plane ret;
        Matrix4 invTrans = mat.inverse().transpose();
        Vector4 v4( p.normal.x, p.normal.y, p.normal.z, p.d );
        v4 = invTrans * v4;
        ret.normal.x = v4.x;
        ret.normal.y = v4.y;
        ret.normal.z = v4.z;
        ret.d = v4.w / ret.normal.normalise();

        return ret;
    }

    inline bool Math::intersects(const Plane& plane, const AxisAlignedBox& box)
    {
        return plane.getSide(box) == Plane::BOTH_SIDE;
    }

    typedef std::vector<Plane> PlaneList;
    /** @} */
    /** @} */

} // namespace Ogre

#endif
