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
OgreSegment.h  -  3D Line Segment class for intersection testing in Ogre3D
Some algorithms based off code from the Wild Magic library by Dave Eberly
-----------------------------------------------------------------------------
begin                : Mon Apr 02 2007
author               : Eric Cha
email                : ericc@xenopi.com
Code Style Update	 : Apr 5, 2007
-----------------------------------------------------------------------------
*/

#ifndef SEGMENT_H
#define SEGMENT_H

#include "OgreVector3.h"

namespace Ogre
{
	class Capsule;

	class Segment
	{
	public:
		// The segment is represented as P+t*D, where P is the segment origin,
		// D is a unit-length direction vector and |t| <= e.  The value e is
		// referred to as the extent of the segment.  The end points of the
		// segment are P-e*D and P+e*D.  The user must ensure that the direction
		// vector is unit-length.  The representation for a segment is analogous
		// to that for an oriented bounding box.  P is the center, D is the
		// axis direction, and e is the extent.


		// construction
		Segment ();  // uninitialized
		Segment (const Vector3&, const Vector3&, Real);

		// set values
		void set(const Vector3& newOrigin, const Vector3& newEnd);
		void setOrigin(const Vector3& newOrigin);
		void setEndPoint(const Vector3& newEndpoint);

		// functions to calculate distance to another segment
		Real distance(const Segment& otherSegment) const;
		Real squaredDistance(const Segment& otherSegment) const;

		// intersect check between segment & capsule 
		bool intersects(const Capsule&) const;

		// defining variables
		Vector3 mOrigin;
		Vector3 mDirection;
		Real	mExtent;
	};
}

#endif //SEGMENT_H
