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
#ifndef __Sphere_H_
#define __Sphere_H_

// Precompiler options
#include "OgrePrerequisites.h"

#include "OgreVector3.h"

namespace Ogre {


    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Math
    *  @{
    */
    /** A sphere primitive, mostly used for bounds checking. 
    @remarks
        A sphere in math texts is normally represented by the function
        x^2 + y^2 + z^2 = r^2 (for sphere's centered on the origin). Ogre stores spheres
        simply as a center point and a radius.
    */
    class _OgreExport Sphere
    {
    protected:
        Real mRadius;
        Vector3 mCenter;
    public:
        /** Standard constructor - creates a unit sphere around the origin.*/
        Sphere() : mRadius(1.0), mCenter(Vector3::ZERO) {}
        /** Constructor allowing arbitrary spheres. 
            @param center The center point of the sphere.
            @param radius The radius of the sphere.
        */
        Sphere(const Vector3& center, Real radius)
            : mRadius(radius), mCenter(center) {}

        /** Returns the radius of the sphere. */
        Real getRadius(void) const { return mRadius; }

        /** Sets the radius of the sphere. */
        void setRadius(Real radius) { mRadius = radius; }

        /** Returns the center point of the sphere. */
        const Vector3& getCenter(void) const { return mCenter; }

        /** Sets the center point of the sphere. */
        void setCenter(const Vector3& center) { mCenter = center; }

        /** Returns whether or not this sphere intersects another sphere. */
        bool intersects(const Sphere& s) const
        {
            return (s.mCenter - mCenter).squaredLength() <=
                Math::Sqr(s.mRadius + mRadius);
        }
        /** Returns whether or not this sphere intersects a box. */
        bool intersects(const AxisAlignedBox& box) const
        {
            return Math::intersects(*this, box);
        }
        /** Returns whether or not this sphere intersects a plane. */
        bool intersects(const Plane& plane) const
        {
            return Math::intersects(*this, plane);
        }
        /** Returns whether or not this sphere intersects a point. */
        bool intersects(const Vector3& v) const
        {
            return ((v - mCenter).squaredLength() <= Math::Sqr(mRadius));
        }
        /** Merges another Sphere into the current sphere */
        void merge(const Sphere& oth)
        {
            Vector3 diff = oth.getCenter() - mCenter;
            Real lengthSq = diff.squaredLength();
            Real radiusDiff = oth.getRadius() - mRadius;
            
            // Early-out
            if (Math::Sqr(radiusDiff) >= lengthSq) 
            {
                // One fully contains the other
                if (radiusDiff <= 0.0f) 
                    return; // no change
                else 
                {
                    mCenter = oth.getCenter();
                    mRadius = oth.getRadius();
                    return;
                }
            }

            Real length = Math::Sqrt(lengthSq);
            Real t = (length + radiusDiff) / (2.0f * length);
            mCenter = mCenter + diff * t;
            mRadius = 0.5f * (length + mRadius + oth.getRadius());
        }
        

    };
    /** @} */
    /** @} */

}

#endif

