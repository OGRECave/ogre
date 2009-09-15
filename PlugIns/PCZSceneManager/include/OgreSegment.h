/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
