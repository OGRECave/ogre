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
OgreCapsule.h  -  3D Line-Swept-Sphere class for intersection testing in Ogre3D
Some algorithms based off code from the Wild Magic library by Dave Eberly
-----------------------------------------------------------------------------
begin                : Mon Apr 02 2007
author               : Eric Cha
email                : ericc@xenopi.com
Code Style Update	 :
-----------------------------------------------------------------------------
*/

#ifndef CAPSULE_H
#define CAPSULE_H

#include "OgreSegment.h"

namespace Ogre
{

	class Capsule
	{
	public:
		// construction
		Capsule ();  // uninitialized
		Capsule (const Segment&, Real);

		// set values
		void set(const Vector3& newOrigin, const Vector3& newEnd, Real newRadius);
		void setOrigin(const Vector3& newOrigin);
		void setEndPoint(const Vector3& newEndpoint);
		void setRadius(Real newRadius);

		// intersection tests
		bool intersects(const Capsule&) const;
		bool intersects(const Segment&) const;

		// defining members
		Segment	mSegment;
		Real	mRadius;
	};
}

#endif //CAPSULE3_H
