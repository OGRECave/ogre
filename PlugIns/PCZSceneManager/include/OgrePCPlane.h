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
OgrePCPlane.h  -  Portal Culling Plane

Specialized Plane that is customized for working with the PCZSceneManager. 
Each Plane has a pointer to the Portal which was used to create it
Portal Culling Planes are created from one side of a portal and the
origin of a camera.
   
-----------------------------------------------------------------------------
begin                : Mon Feb 26 2007
author               : Eric Cha
email                : ericc@xenopi.com
Code Style Update	 :
-----------------------------------------------------------------------------
*/

#ifndef PC_PLANE_H
#define PC_PLANE_H

#include <OgrePlane.h>
#include "OgrePCZPrerequisites.h"

namespace Ogre
{
    class PortalBase;


    class _OgrePCZPluginExport PCPlane : public Plane
    {
    public:
        /** Standard constructor */
        PCPlane();
		/** Alternative constructor */
		PCPlane (const Plane & plane);
        /** Alternative constructor */
        PCPlane (const Vector3& rkNormal, const Vector3& rkPoint);
        /** Alternative constructor */
        PCPlane (const Vector3& rkPoint0, const Vector3& rkPoint1, const Vector3& rkPoint2);
		/** Copy from an Ogre Plane */
		void setFromOgrePlane(Plane & ogrePlane);

        /** Standard destructor */
        ~PCPlane();

        /** Returns the Portal that was used to create this plane
        */
        PortalBase* getPortal()
        {
            return mPortal;
        };

        /** Sets the Portal that was used to create this plane
        */
        void setPortal(PortalBase* o)
        {
            mPortal = o;
        };


    protected:

        ///Portal used to create this plane.
        PortalBase *mPortal;
    };

}


#endif
