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
PCZCamera.cpp  -  description
-----------------------------------------------------------------------------
begin                : Wed Feb 21 2007
author               : Eric Cha
email                : ericc@xenopi.com
-----------------------------------------------------------------------------
*/

#include <OgreMath.h>
#include <OgreAxisAlignedBox.h>
#include <OgreRoot.h>

#include "OgreSceneNode.h"
#include "OgrePCZCamera.h"
#include "OgrePCZFrustum.h"
#include "OgrePortal.h"

namespace Ogre
{
    PCZCamera::PCZCamera( const String& name, SceneManager* sm ) : Camera( name, sm )
    {
		mBox.setExtents(-0.1, -0.1, -0.1, 0.1, 0.1, 0.1);
        mExtraCullingFrustum.setUseOriginPlane(true);
    }

    PCZCamera::~PCZCamera()
    {
    }

	const AxisAlignedBox& PCZCamera::getBoundingBox(void) const
	{
		return mBox;
	}

    // this version checks against extra culling planes
    bool PCZCamera::isVisible( const AxisAlignedBox &bound, FrustumPlane *culledBy) const 
    {
        // Null boxes always invisible
        if ( bound.isNull() )
            return false;

        // Make any pending updates to the calculated frustum planes
        updateFrustumPlanes();

		// check extra culling planes
        bool extraResults;
        extraResults = mExtraCullingFrustum.isVisible(bound);
        if (!extraResults)
        {
            return false;
        }

		// check "regular" camera frustum
		bool regcamresults = Camera::isVisible(bound, culledBy);

		if (!regcamresults)
		{
			// culled by regular culling planes
			return regcamresults;
		}


		return true;
   }

	/* A 'more detailed' check for visibility of an AAB.  This function returns
	  none, partial, or full for visibility of the box.  This is useful for 
	  stuff like Octree leaf culling */
	PCZCamera::Visibility PCZCamera::getVisibility( const AxisAlignedBox &bound )
	{

		// Null boxes always invisible
		if ( bound.isNull() )
			return NONE;

		// Get centre of the box
		Vector3 centre = bound.getCenter();
		// Get the half-size of the box
		Vector3 halfSize = bound.getHalfSize();

		bool all_inside = true;

		for ( int plane = 0; plane < 6; ++plane )
		{

			// Skip far plane if infinite view frustum
			if (plane == FRUSTUM_PLANE_FAR && mFarDist == 0)
				continue;

			// This updates frustum planes and deals with cull frustum
			Plane::Side side = getFrustumPlane(plane).getSide(centre, halfSize);
			if(side == Plane::NEGATIVE_SIDE) return NONE;
			// We can't return now as the box could be later on the negative side of a plane.
			if(side == Plane::BOTH_SIDE) 
					all_inside = false;
		}
        
        switch(mExtraCullingFrustum.getVisibility(bound))
        {
        case PCZFrustum::NONE:
            return NONE;
        case PCZFrustum::PARTIAL:
            return PARTIAL;
        case PCZFrustum::FULL:
            break;
        }

		if ( all_inside )
			return FULL;
		else
			return PARTIAL;

	}

    /* isVisible() function for portals */
    // NOTE: Everything needs to be updated spatially before this function is
    //       called including portal corners, frustum planes, etc.
    bool PCZCamera::isVisible(PortalBase* portal, FrustumPlane* culledBy) const
    {
		// if portal isn't enabled, it's not visible
		if (!portal->getEnabled()) return false;

        // check the extra frustum first
        if (!mExtraCullingFrustum.isVisible(portal))
        {
            return false;
        }

		// if portal is of type AABB or Sphere, then use simple bound check against planes
		if (portal->getType() == PortalBase::PORTAL_TYPE_AABB)
		{
			AxisAlignedBox aabb;
			aabb.setExtents(portal->getDerivedCorner(0), portal->getDerivedCorner(1));
			return Camera::isVisible(aabb, culledBy);
		}
		else if (portal->getType() == PortalBase::PORTAL_TYPE_SPHERE)
		{
			return Camera::isVisible(portal->getDerivedSphere(), culledBy);
		}

		// only do this check if it's a portal. (anti portal doesn't care about facing)
		if (portal->getTypeFlags() == PortalFactory::FACTORY_TYPE_FLAG)
		{
			// check if the portal norm is facing the camera
			Vector3 cameraToPortal = portal->getDerivedCP() - getDerivedPosition();
			Vector3 portalDirection = portal->getDerivedDirection();
			Real dotProduct = cameraToPortal.dotProduct(portalDirection);
			if ( dotProduct > 0 )
			{
				// portal is faced away from camera 
				return false;
			}
		}

        // check against regular frustum planes
        bool visible_flag;
		if (mCullFrustum)
		{
            // For each frustum plane, see if all points are on the negative side
            // If so, object is not visible
			// NOTE: We skip the NEAR plane (plane #0) because Portals need to
			// be visible no matter how close you get to them.  

            for (int plane = 1; plane < 6; ++plane)
            {
                // set the visible flag to false
                visible_flag = false;
                // Skip far plane if infinite view frustum
                if (plane == FRUSTUM_PLANE_FAR && mFarDist == 0)
                    continue;
                
                // we have to check each corner of the portal
                for (int corner = 0; corner < 4; corner++)
                {
                    Plane::Side side = mCullFrustum->getFrustumPlane(plane).getSide(portal->getDerivedCorner(corner));
                    if (side != Plane::NEGATIVE_SIDE)
                    {
                        visible_flag = true;
						break;
                    }
                }
                // if the visible_flag is still false, then this plane
                // culled all the portal points
                if (visible_flag == false)
                {
                    // ALL corners on negative side therefore out of view
                    if (culledBy)
                        *culledBy = (FrustumPlane)plane;
                    return false;
                }
            }
		}
		else
		{
            // Make any pending updates to the calculated frustum planes
            Frustum::updateFrustumPlanes();

            // For each frustum plane, see if all points are on the negative side
            // If so, object is not visible
			// NOTE: We skip the NEAR plane (plane #0) because Portals need to
			// be visible no matter how close you get to them.
			// BUGBUG: This can cause a false positive situation when a portal is 
			// behind the camera but close.  This could be fixed by having another
			// culling plane at the camera location with normal same as camera direction.
            for (int plane = 1; plane < 6; ++plane)
            {
                // set the visible flag to false
                visible_flag = false;
                // Skip far plane if infinite view frustum
                if (plane == FRUSTUM_PLANE_FAR && mFarDist == 0)
                    continue;
                
                // we have to check each corner of the portal
                for (int corner = 0; corner < 4; corner++)
                {
                    Plane::Side side = mFrustumPlanes[plane].getSide(portal->getDerivedCorner(corner));
                    if (side != Plane::NEGATIVE_SIDE)
                    {
                        visible_flag = true;
						break;
                    }
                }
                // if the visible_flag is still false, then this plane
                // culled all the portal points
                if (visible_flag == false)
                {
                    // ALL corners on negative side therefore out of view
                    if (culledBy)
                        *culledBy = (FrustumPlane)plane;
                    return false;
                }
            }
		}
        // no plane culled all the portal points and the norm
        // was facing the camera, so this portal is visible
        return true;
    }

	/// Sets the type of projection to use (orthographic or perspective).
	void PCZCamera::setProjectionType(ProjectionType pt)
	{
		mExtraCullingFrustum.setProjectionType(pt);
		Camera::setProjectionType(pt);
	}

    /* Update function (currently used for making sure the origin stuff for the
        extra culling frustum is up to date */
    void PCZCamera::update(void)
    {
        // make sure the extra culling frustum origin stuff is up to date
		if (mProjType == PT_PERSPECTIVE)
		//if (!mCustomViewMatrix)
		{
			mExtraCullingFrustum.setUseOriginPlane(true);
			mExtraCullingFrustum.setOrigin(getDerivedPosition());
			mExtraCullingFrustum.setOriginPlane(getDerivedDirection(), getDerivedPosition());
		}
		else
		{
			// In ortho mode, we don't want to cull things behind camera.
			// This helps for back casting which is useful for texture shadow projection on directional light.
			mExtraCullingFrustum.setUseOriginPlane(false);
		}
    }

    // calculate extra culling planes from portal and camera 
    // origin and add to list of extra culling planes
	// NOTE: returns 0 if portal was completely culled by existing planes
	//		 returns > 0 if culling planes are added (# is planes added)
	int PCZCamera::addPortalCullingPlanes(PortalBase* portal)
    {
        // add the extra culling planes from the portal
        return mExtraCullingFrustum.addPortalCullingPlanes(portal);
    }

    // remove extra culling planes created from the given portal
	// NOTE: This should only be used during visibility traversal (backing out of a recursion)
	void PCZCamera::removePortalCullingPlanes(PortalBase* portal)
    {
        mExtraCullingFrustum.removePortalCullingPlanes(portal);
    }

	// remove all extra culling planes
    void PCZCamera::removeAllExtraCullingPlanes(void) 
    {
        mExtraCullingFrustum.removeAllCullingPlanes();
    }

}





