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
#ifndef __PlaneBoundedVolume_H_
#define __PlaneBoundedVolume_H_

// Precompiler options
#include "OgrePrerequisites.h"
#include "OgreAxisAlignedBox.h"
#include "OgreSphere.h"
#include "OgreMath.h"
#include "OgrePlane.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Math
	*  @{
	*/
	/** Represents a convex volume bounded by planes.
    */
    class _OgreExport PlaneBoundedVolume
    {
    public:
        typedef vector<Plane>::type PlaneList;
        /// Publicly accessible plane list, you can modify this direct
        PlaneList planes;
        Plane::Side outside;

        PlaneBoundedVolume() :outside(Plane::NEGATIVE_SIDE) {}
        /** Constructor, determines which side is deemed to be 'outside' */
        PlaneBoundedVolume(Plane::Side theOutside) 
            : outside(theOutside) {}

        /** Intersection test with AABB
        @remarks May return false positives but will never miss an intersection.
        */
        inline bool intersects(const AxisAlignedBox& box) const
        {
            if (box.isNull()) return false;
            if (box.isInfinite()) return true;

            // Get centre of the box
            Vector3 centre = box.getCenter();
            // Get the half-size of the box
            Vector3 halfSize = box.getHalfSize();
            
            PlaneList::const_iterator i, iend;
            iend = planes.end();
            for (i = planes.begin(); i != iend; ++i)
            {
                const Plane& plane = *i;

                Plane::Side side = plane.getSide(centre, halfSize);
                if (side == outside)
                {
                    // Found a splitting plane therefore return not intersecting
                    return false;
                }
            }

            // couldn't find a splitting plane, assume intersecting
            return true;

        }
        /** Intersection test with Sphere
        @remarks May return false positives but will never miss an intersection.
        */
        inline bool intersects(const Sphere& sphere) const
        {
            PlaneList::const_iterator i, iend;
            iend = planes.end();
            for (i = planes.begin(); i != iend; ++i)
            {
                const Plane& plane = *i;

                // Test which side of the plane the sphere is
                Real d = plane.getDistance(sphere.getCenter());
                // Negate d if planes point inwards
                if (outside == Plane::NEGATIVE_SIDE) d = -d;

                if ( (d - sphere.getRadius()) > 0)
                    return false;
            }

            return true;

        }

        /** Intersection test with a Ray
        @return std::pair of hit (bool) and distance
        @remarks May return false positives but will never miss an intersection.
        */
        inline std::pair<bool, Real> intersects(const Ray& ray)
        {
            return Math::intersects(ray, planes, outside == Plane::POSITIVE_SIDE);
        }

    };

    typedef vector<PlaneBoundedVolume>::type PlaneBoundedVolumeList;

	/** @} */
	/** @} */

}

#include "OgreHeaderSuffix.h"

#endif

