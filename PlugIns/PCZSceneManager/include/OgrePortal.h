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
Portal.h  -  Portals are special constructs which which are used to connect 
			 two Zones in a PCZScene.  Portals are defined by 4 coplanr 
             corners and a direction.  Portals are contained within Zones and 
             are essentially "one way" connectors.  Objects and entities can
			 use them to travel to other Zones, but to return, there must be
			 a corresponding Portal which connects back to the original zone
			 from the new zone.

-----------------------------------------------------------------------------
begin                : Thu Feb 22 2007
author               : Eric Cha
email                : ericc@xenopi.com
Code Style Update	 : Apr 5, 2007
-----------------------------------------------------------------------------
*/

#ifndef PORTAL_H
#define PORTAL_H

#include "OgrePCZPrerequisites.h"
#include "OgrePCZSceneNode.h"
#include "OgreAxisAlignedBox.h"
#include "OgreSphere.h"

namespace Ogre
{
    class PCZone;
	class PCZSceneNode;

    /** Portal datastructure for connecting zones.
    @remarks
    */

	class _OgrePCZPluginExport Portal : public SceneCtlAllocatedObject
    {
    public:
		enum PORTAL_TYPE
		{
			PORTAL_TYPE_QUAD,
			PORTAL_TYPE_AABB,
			PORTAL_TYPE_SPHERE,
		};
        Portal(const String &, const PORTAL_TYPE type = PORTAL_TYPE_QUAD);
        virtual ~Portal();

        /** Set the SceneNode the Portal is associated with
        */
        void setNode( SceneNode * );
        /** Set the Zone the Portal targets (connects to)
        */
        void setTargetZone( PCZone * );
		/** Set the current home zone of the portal
		*/
		void setCurrentHomeZone( PCZone * );
		/** Set the zone this portal should be moved to
		*/
		void setNewHomeZone( PCZone * );
		/** Set the target portal pointer
		*/
		void setTargetPortal( Portal * );
        /** Set the local coordinates of one of the portal corners
        */
        void setCorner( int , Vector3 & );
        /** Set the local coordinates of all of the portal corners
        */
        void setCorners( Vector3 * );
		/** Set the "inward/outward norm" direction of AAB or SPHERE portals
		    NOTE: UNIT_Z = "outward" norm, NEGATIVE_UNIT_Z = "inward" norm
			NOTE: Remember, Portal norms always point towards the zone they are "in".
		*/
		void setDirection(const Vector3 &d)
		{
			switch (mType)
			{
			default:
			case PORTAL_TYPE_QUAD:
				OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
					"Cannot setDirection on a Quad type portal", 
					"Portal::setDirection");
				break;
			case PORTAL_TYPE_AABB:
			case PORTAL_TYPE_SPHERE:
				if (d != Vector3::UNIT_Z &&
					d != Vector3::NEGATIVE_UNIT_Z)
				{
					OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
						"Valid parameters are Vector3::UNIT_Z or Vector3::NEGATIVE_UNIT_Z", 
						"Portal::setDirection");
					return;
				}
				mDirection = d;
				break;
			}
		}
		/* Calculate the local direction and radius of the portal
		*/
		void calcDirectionAndRadius( void );
		/* get the type of portal 
		*/
		const PORTAL_TYPE getType(void) const {return mType;}
		/* Returns the name of the portal
		*/
		const String & getName(void) const { return mName; }
		/* Get the scene node (if any) this portal is associated with 
		*/
		SceneNode * getNode() {return mNode;}
        /** Retrieve the radius of the portal (calculates if necessary for quad portals)
        */
		Real getRadius( void );
        /** Get the Zone the Portal connects to
        */
        PCZone * getTargetZone() {return mTargetZone;}
		/** Get the Zone the Portal is currently "in" 
		*/
		PCZone * getCurrentHomeZone() {return mCurrentHomeZone;}
        /** Get the Zone the Portal should be moved to
        */
        PCZone * getNewHomeZone() {return mNewHomeZone;}
		/** Get the connected portal (if any)
		*/
		Portal * getTargetPortal() {return mTargetPortal;}
        /** Get the coordinates of one of the portal corners in local space
        */
        Vector3 & getCorner( int );
		/** Get the direction vector of the portal in local space 
		*/
		Vector3 & getDirection() {return mDirection;}
        /** Get the derived (world) coordinates of one of the portal corners 
        */
        Vector3 & getDerivedCorner( int ) ;
        /** Get the direction of the portal in world coordinates
        */
        Vector3 & getDerivedDirection( void ) ;
        /** Get the position (centerpoint) of the portal in world coordinates
        */
        Vector3 & getDerivedCP( void ) ;
		/** Get the sphere centered on the derived CP of the portal in world coordinates
		*/
		Sphere & getDerivedSphere( void );
		/* Get the portal plane in world coordinates
		*/
		Plane & getDerivedPlane(void) ;
        /** Get the previous position (centerpoint) of the portal in world coordinates
        */
        Vector3 & getPrevDerivedCP( void ) ;
		/* Get the previous portal plane in world coordinates
		*/
		Plane & getPrevDerivedPlane(void) ;
        /** Update the derived values
        */
        void updateDerivedValues(void);
		/* Adjust the portal so that it is centered and oriented on the given node
		*/
		void adjustNodeToMatch(SceneNode *);
		/* open the portal */
		void open();
		/* close the portal */
		void close();
		/* check if portal is open */
		bool isOpen() {return mOpen;}
		

		enum PortalIntersectResult
		{
			NO_INTERSECT,
			INTERSECT_NO_CROSS,
			INTERSECT_BACK_NO_CROSS,
			INTERSECT_CROSS
		};
        /* check if portal intersects an aab
        */
        bool intersects(const AxisAlignedBox & aab);

        /* check if portal intersects an sphere
        */
        bool intersects(const Sphere & sphere);

        /* check if portal intersects a plane bounded volume
        */
        bool intersects(const PlaneBoundedVolume & pbv);

        /* check if portal intersects a ray
        */
        bool intersects(const Ray & ray);

		/* check for intersection between portal & scenenode (also determines
		 * if scenenode crosses over portal
		 */
		PortalIntersectResult intersects(PCZSceneNode *);
		/* check if portal crossed over portal
		 */
		bool crossedPortal(Portal *);
		/* check if portal touches another portal
		*/
		bool closeTo(Portal *);

    protected:
		// Type of portal (quad, aabb, or sphere)
		PORTAL_TYPE mType;
		// Name (identifier) for the Portal - must be unique
		String mName;
		/// SceneNode (if any) this portal is attached to
		SceneNode * mNode;
        ///connected Zone
        PCZone * mTargetZone;
		/// Zone this portal is currently owned by (in)
		PCZone * mCurrentHomeZone;
		///zone to transfer this portal to
		PCZone * mNewHomeZone;
		///Matching Portal in the target zone (usually in same world space 
        // as this portal, but pointing the opposite direction)
		Portal * mTargetPortal;
        /// Corners of the portal - coordinates are relative to the sceneNode
		// NOTE: there are 4 corners if the portal is a quad type
		//       there are 2 corners if the portal is an AABB type
		//       there are 2 corners if the portal is a sphere type (center and point on sphere)
        Vector3 * mCorners;
		/// Direction ("Norm") of the portal - 
		// NOTE: For a Quad portal, determined by the 1st 3 corners.
		// NOTE: For AABB & SPHERE portals, we only have "inward" or "outward" cases.
		//       To indicate "outward", the Direction is UNIT_Z
		//		 to indicate "inward", the Direction is NEGATIVE_UNIT_Z
		Vector3 mDirection;
		/// Radius of the sphere enclosing the portal 
		// NOTE: For aabb portals, this value is the distance from the center of the aab to a corner
		Real mRadius;
		// Local Centerpoint of the portal
		Vector3 mLocalCP;
        /// Derived (world coordinates) Corners of the portal
		// NOTE: there are 4 corners if the portal is a quad type
		//       there are 2 corners if the portal is an AABB type (min corner & max corner)
		//       there are 2 corners if the portal is a sphere type (center and point on sphere)
        Vector3 * mDerivedCorners;
		/// Derived (world coordinates) direction of the portal
		// NOTE: Only applicable for a Quad portal
		Vector3 mDerivedDirection;
		/// Derived (world coordinates) of portal (center point)
		Vector3 mDerivedCP;
		/// Sphere of the portal centered on the derived CP
		Sphere mDerivedSphere;
		/// Derived (world coordinates) Plane of the portal
		// NOTE: Only applicable for a Quad portal
		Plane mDerivedPlane;
		/// Previous frame portal cp (in world coordinates)
		Vector3 mPrevDerivedCP;
		/// Previous frame derived plane 
		// NOTE: Only applicable for a Quad portal
		Plane mPrevDerivedPlane;
		/// flag indicating whether or not local values are up-to-date
		bool mLocalsUpToDate;
		// previous world transform
		Matrix4 prevWorldTransform;
		// flag open or closed
		bool mOpen;
    };

}

#endif


