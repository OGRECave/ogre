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
#ifndef __Ray_H_
#define __Ray_H_

// Precompiler options
#include "OgrePrerequisites.h"

#include "OgrePlaneBoundedVolume.h"

namespace Ogre {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Math
    *  @{
    */
    /** Representation of a ray in space, i.e. a line with an origin and direction. */
    class _OgreExport Ray
    {
    protected:
        Vector3 mOrigin;
        Vector3 mDirection;
    public:
        Ray():mOrigin(Vector3::ZERO), mDirection(Vector3::UNIT_Z) {}
        Ray(const Vector3& origin, const Vector3& direction)
            :mOrigin(origin), mDirection(direction) {}

        /** Sets the origin of the ray. */
        void setOrigin(const Vector3& origin) {mOrigin = origin;} 
        /** Gets the origin of the ray. */
        const Vector3& getOrigin(void) const {return mOrigin;} 

        /** Sets the direction of the ray. */
        void setDirection(const Vector3& dir) {mDirection = dir;} 
        /** Gets the direction of the ray. */
        const Vector3& getDirection(void) const {return mDirection;} 

        /** Gets the position of a point t units along the ray. */
        Vector3 getPoint(Real t) const { 
            return Vector3(mOrigin + (mDirection * t));
        }
        
        /** Gets the position of a point t units along the ray. */
        Vector3 operator*(Real t) const { 
            return getPoint(t);
        }

        /** Tests whether this ray intersects the given plane. */
        RayTestResult intersects(const Plane& p) const
        {
            Real denom = p.normal.dotProduct(mDirection);
            if (Math::Abs(denom) < std::numeric_limits<Real>::epsilon())
            {
                // Parallel
                return RayTestResult(false, (Real)0);
            }
            else
            {
                Real nom = p.normal.dotProduct(mOrigin) + p.d;
                Real t = -(nom / denom);
                return RayTestResult(t >= 0, (Real)t);
            }
        }
        /** Tests whether this ray intersects the given plane bounded volume. */
        RayTestResult intersects(const PlaneBoundedVolume& p) const
        {
            return Math::intersects(*this, p.planes, p.outside == Plane::POSITIVE_SIDE);
        }
        /** Tests whether this ray intersects the given sphere. */
        RayTestResult intersects(const Sphere& s, bool discardInside = true) const
        {
            // Adjust ray origin relative to sphere center
            Vector3 rayorig = mOrigin - s.getCenter();
            Real radius = s.getRadius();

            // Check origin inside first
            if (rayorig.squaredLength() <= radius*radius && discardInside)
            {
                return RayTestResult(true, (Real)0);
            }

            // Mmm, quadratics
            // Build coeffs which can be used with std quadratic solver
            // ie t = (-b +/- sqrt(b*b + 4ac)) / 2a
            Real a = mDirection.dotProduct(mDirection);
            Real b = 2 * rayorig.dotProduct(mDirection);
            Real c = rayorig.dotProduct(rayorig) - radius*radius;

            // Calc determinant
            Real d = (b*b) - (4 * a * c);
            if (d < 0)
            {
                // No intersection
                return RayTestResult(false, (Real)0);
            }
            else
            {
                // BTW, if d=0 there is one intersection, if d > 0 there are 2
                // But we only want the closest one, so that's ok, just use the
                // '-' version of the solver
                Real t = ( -b - Math::Sqrt(d) ) / (2 * a);
                if (t < 0)
                    t = ( -b + Math::Sqrt(d) ) / (2 * a);
                return RayTestResult(true, t);
            }
        }
        /** Tests whether this ray intersects the given box. */
        RayTestResult intersects(const AxisAlignedBox& box) const
        {
            return Math::intersects(*this, box);
        }

    };

    inline RayTestResult Math::intersects(const Ray& ray, const Plane& plane)
    {
        return ray.intersects(plane);
    }

    inline RayTestResult Math::intersects(const Ray& ray, const Sphere& sphere, bool discardInside)
    {
        return ray.intersects(sphere, discardInside);
    }
    /** @} */
    /** @} */

}
#endif
