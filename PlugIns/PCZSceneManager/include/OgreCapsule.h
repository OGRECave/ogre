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
