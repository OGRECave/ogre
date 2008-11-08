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
PCZCamera.h  -  description
-----------------------------------------------------------------------------
begin                : Wed Feb 21 2007
author               : Eric Cha
email                : ericc@xenopi.com
Code Style Update	 :
-----------------------------------------------------------------------------
*/

#ifndef PCZCAMERA_H
#define PCZCAMERA_H

#include <OgreCamera.h>
#include "OgrePCPlane.h"
#include "OgrePortal.h"
#include "OgrePCZFrustum.h"
#include "OgrePCZPrerequisites.h"

namespace Ogre
{
    #define MAX_EXTRA_CULLING_PLANES    40

    class PCZone;


    /** Specialized viewpoint from which an PCZone Scene can be rendered.
    @remarks
    */

    class _OgrePCZPluginExport PCZCamera : public Camera
    {
    public:
		/** Visibility types */
		enum Visibility
		{
			NONE,
			PARTIAL,
			FULL
		};

        /* Standard constructor */
        PCZCamera( const String& name, SceneManager* sm );
        /* Standard destructor */
        ~PCZCamera();

        /** Overridden: Retrieves the local axis-aligned bounding box for this object.
            @remarks
                This bounding box is in local coordinates.
        */
        virtual const AxisAlignedBox& getBoundingBox(void) const;

		/* Overridden isVisible function for aabb */
		virtual bool isVisible( const AxisAlignedBox &bound, FrustumPlane *culledBy=0) const;

        /* isVisible() function for portals */
        bool isVisible (Portal * portal, FrustumPlane *culledBy = 0);

        /** Returns the visiblity of the box
        */
        bool isVisibile( const AxisAlignedBox &bound );

        /** Returns the detailed visiblity of the box
		*/
		PCZCamera::Visibility getVisibility( const AxisAlignedBox &bound );

		/// Sets the type of projection to use (orthographic or perspective).
		void setProjectionType(ProjectionType pt);

        /* Update function (currently used for making sure the origin stuff for the
           extra culling frustum is up to date */
        void update(void);

        // calculate extra culling planes from portal and camera 
        // origin and add to list of extra culling planes
        int addPortalCullingPlanes(Portal * portal);
        // remove extra culling planes created from the given portal
        void removePortalCullingPlanes(Portal *portal);
		// remove all extra culling planes
        void removeAllExtraCullingPlanes(void);
    protected:
		AxisAlignedBox mBox;
        PCZFrustum mExtraCullingFrustum;
    };

}

#endif
