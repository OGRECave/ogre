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
OgrePCPlane.cpp  -  description
-----------------------------------------------------------------------------
begin                : Tue Feb 27 2007
author               : Eric Cha
email                : ericc@xenopi.com
-----------------------------------------------------------------------------
*/

#include "OgrePCPlane.h"
#include "OgrePortal.h"

namespace Ogre
{

    PCPlane::PCPlane() : Plane()
    {
        mPortal = 0;
    }
	PCPlane::PCPlane(const Plane & plane) 
			: Plane(plane)
	{
		mPortal = 0;
	}
    PCPlane::PCPlane(const Vector3& rkNormal, const Vector3& rkPoint) 
            : Plane(rkNormal, rkPoint)
    {
        mPortal = 0;
    }
    PCPlane::PCPlane(const Vector3& rkPoint0, const Vector3& rkPoint1,const Vector3& rkPoint2) 
            : Plane(rkPoint0, rkPoint1, rkPoint2)
    {
        mPortal = 0;
    }
	void PCPlane::setFromOgrePlane(Plane & ogrePlane)
	{
		d = ogrePlane.d;
		normal = ogrePlane.normal;
		mPortal = 0;
	}

    PCPlane::~PCPlane()
    {
		mPortal = 0;
    }

}
