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
