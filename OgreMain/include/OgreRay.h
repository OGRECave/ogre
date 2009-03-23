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
#ifndef __Ray_H_
#define __Ray_H_

// Precompiler options
#include "OgrePrerequisites.h"

#include "OgreVector3.h"
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
		};

		/** Tests whether this ray intersects the given plane. 
		@returns A pair structure where the first element indicates whether
			an intersection occurs, and if true, the second element will
			indicate the distance along the ray at which it intersects. 
			This can be converted to a point in space by calling getPoint().
		*/
		std::pair<bool, Real> intersects(const Plane& p) const
		{
			return Math::intersects(*this, p);
		}
        /** Tests whether this ray intersects the given plane bounded volume. 
        @returns A pair structure where the first element indicates whether
        an intersection occurs, and if true, the second element will
        indicate the distance along the ray at which it intersects. 
        This can be converted to a point in space by calling getPoint().
        */
        std::pair<bool, Real> intersects(const PlaneBoundedVolume& p) const
        {
            return Math::intersects(*this, p.planes, p.outside == Plane::POSITIVE_SIDE);
        }
		/** Tests whether this ray intersects the given sphere. 
		@returns A pair structure where the first element indicates whether
			an intersection occurs, and if true, the second element will
			indicate the distance along the ray at which it intersects. 
			This can be converted to a point in space by calling getPoint().
		*/
		std::pair<bool, Real> intersects(const Sphere& s) const
		{
			return Math::intersects(*this, s);
		}
		/** Tests whether this ray intersects the given box. 
		@returns A pair structure where the first element indicates whether
			an intersection occurs, and if true, the second element will
			indicate the distance along the ray at which it intersects. 
			This can be converted to a point in space by calling getPoint().
		*/
		std::pair<bool, Real> intersects(const AxisAlignedBox& box) const
		{
			return Math::intersects(*this, box);
		}

    };
	/** @} */
	/** @} */

}
#endif
