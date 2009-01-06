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
OgrePCZFrustum.h  -  PCZ Supplemental Culling Frustum

This isn't really a traditional "frustum", but more a collection of
extra culling planes used by the PCZ Scene Manager for supplementing
the camera culling and light zone culling by creating extra culling
planes from visible portals.  Since portals are 4 sided, the extra 
culling planes tend to form frustums (pyramids) but nothing in the 
code really assumes that the culling planes are frustums.  They are
just treated as planes.  

The "originPlane" is a culling plane which passes through the origin
point specified.  It is used to cull portals which are close to, but
behind the camera view.  (the nature of the culling routine doesn't
give correct results if you just use the "near" plane of the standard
camera frustum (unless that near plane distance is 0.0, but that is
highly not recommended for other reasons having to do with having a 
legal view frustum).
---------------------------------------------------------------------------
begin                : Mon May 29 2007
author               : Eric Cha
email                : ericc@xenopi.com
Code Style Update	 :

---------------------------------------------------------------------------
*/

#ifndef PCZ_FRUSTUM_H
#define PCZ_FRUSTUM_H

#include "OgrePCPlane.h"
#include "OgrePCZPrerequisites.h"
#include "OgreFrustum.h"

namespace Ogre
{
    class Portal;

    typedef list< PCPlane * >::type PCPlaneList;

    /** Specialized frustum shaped culling volume that has culling planes created from portals 
    */
    
    class _OgrePCZPluginExport PCZFrustum 
    {
    public:
        // visibility types
		enum Visibility
		{
			NONE,
			PARTIAL,
			FULL
		};

        /** Standard constructor */
        PCZFrustum();
        /** Standard destructor */
        ~PCZFrustum();

		/* isVisible function for aabb */
		bool isVisible( const AxisAlignedBox &bound) const;
		/* isVisible function for sphere */
		bool isVisible( const Sphere &bound) const;
        /* isVisible() function for portals */
        bool isVisible (Portal * portal);
        /* more detailed check for visibility of an AABB */
        PCZFrustum::Visibility getVisibility(const AxisAlignedBox & bound);

        // calculate  culling planes from portal and Frustum 
        // origin and add to list of  culling planes
        int addPortalCullingPlanes(Portal * portal);
        // remove  culling planes created from the given portal
        void removePortalCullingPlanes(Portal *portal);
		// remove all  culling planes
		void removeAllCullingPlanes(void);
        // set the origin value
        void setOrigin(const Vector3 & newOrigin) {mOrigin = newOrigin;}
        // set the origin plane
        void setOriginPlane(const Vector3 &rkNormal, const Vector3 &rkPoint);
        // tell the frustum whether or not to use the originplane
        void setUseOriginPlane(bool yesno) {mUseOriginPlane = yesno;}
		// get an unused PCPlane from the CullingPlane Reservoir
		PCPlane * getUnusedCullingPlane(void);

		/// Set the projection type of this PCZFrustum.
		inline void setProjectionType(ProjectionType projType)
		{ mProjType = projType; }
		/// Get the projection type of this PCZFrustum.
		inline ProjectionType getProjectionType() const
		{ return mProjType; }

    protected:
        Vector3     mOrigin;
        Plane       mOriginPlane;
        bool        mUseOriginPlane;
        PCPlaneList mActiveCullingPlanes;
		PCPlaneList mCullingPlaneReservoir;
		ProjectionType mProjType;

    };

}


#endif
