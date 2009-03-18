/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
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
PortalBase.h  -  PortalBase is the base class for Portal and AntiPortal.

*/

#ifndef PORTALBASE_H
#define PORTALBASE_H

#include "OgrePCZPrerequisites.h"
#include "OgrePCZSceneNode.h"
#include "OgreMovableObject.h"
#include "OgreAxisAlignedBox.h"
#include "OgreCapsule.h"
#include "OgreSphere.h"

namespace Ogre
{
	class PCZSceneNode;

	/** PortalBase - Base class to Portal and AntiPortal classes. */
	class _OgrePCZPluginExport PortalBase : public MovableObject
	{
	public:
		enum PORTAL_TYPE
		{
			PORTAL_TYPE_QUAD,
			PORTAL_TYPE_AABB,
			PORTAL_TYPE_SPHERE,
		};

		/** Constructor. */
		PortalBase(const String& name, const PORTAL_TYPE type = PORTAL_TYPE_QUAD);

		/** Destructor. */
		virtual ~PortalBase();

		/** Retrieves the axis-aligned bounding box for this object in world coordinates. */
		virtual const AxisAlignedBox& getWorldBoundingBox(bool derive = false) const;
		/** Retrieves the worldspace bounding sphere for this object. */
		virtual const Sphere& getWorldBoundingSphere(bool derive = false) const;

		/** Set the SceneNode the Portal is associated with */
		void setNode(SceneNode* sn);
		/** Set the current home zone of the portal */
		void setCurrentHomeZone(PCZone* zone);
		/** Set the zone this portal should be moved to */
		void setNewHomeZone(PCZone* zone);

		/** Set the local coordinates of one of the portal corners */
		void setCorner(int index, const Vector3& point);
		/** Set the local coordinates of all of the portal corners */
		void setCorners(const Vector3* corners);
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
		/** Calculate the local direction and radius of the portal */
		void calcDirectionAndRadius() const;

		/** get the type of portal */
		const PORTAL_TYPE getType() const {return mType;}
		/** Retrieve the radius of the portal (calculates if necessary for quad portals) */
		Real getRadius() const;

		/** Get the Zone the Portal is currently "in" */
		PCZone* getCurrentHomeZone()
		{ return mCurrentHomeZone; }
		/** Get the Zone the Portal should be moved to */
		PCZone* getNewHomeZone()
		{ return mNewHomeZone; }

		/** Get the coordinates of one of the portal corners in local space */
		const Vector3& getCorner(int index) const
		{ return mCorners[index]; }
		/** Get the direction vector of the portal in local space */
		const Vector3& getDirection() const
		{ return mDirection; }

		/** Get the derived (world) coordinates of one of the portal corners */
		const Vector3& getDerivedCorner(int index) const
		{ return mDerivedCorners[index]; }
		/** Get the direction of the portal in world coordinates */
		const Vector3& getDerivedDirection() const
		{ return mDerivedDirection; }
		/** Get the position (centerpoint) of the portal in world coordinates */
		const Vector3& getDerivedCP() const
		{ return mDerivedCP; }
		/** Get the sphere centered on the derived CP of the portal in world coordinates */
		const Sphere& getDerivedSphere() const
		{ return mDerivedSphere; }
		/** Get the portal plane in world coordinates */
		const Plane& getDerivedPlane() const
		{ return mDerivedPlane; }

		/** Get the previous position (centerpoint) of the portal in world coordinates */
		const Vector3& getPrevDerivedCP() const
		{ return mPrevDerivedCP; }
		/** Get the previous portal plane in world coordinates */
		const Plane& getPrevDerivedPlane() const
		{ return mPrevDerivedPlane; }

		/** Update the derived values */
		void updateDerivedValues() const;
		/** Adjust the portal so that it is centered and oriented on the given node */
		void adjustNodeToMatch(SceneNode* node);
		/** enable the portal */
		void setEnabled(bool value)
		{ mEnabled = value; }
		/** check if portal is enabled */
		bool getEnabled() const {return mEnabled;}
		

		enum PortalIntersectResult
		{
			NO_INTERSECT,
			INTERSECT_NO_CROSS,
			INTERSECT_BACK_NO_CROSS,
			INTERSECT_CROSS
		};
		/** check if portal intersects an aab */
		bool intersects(const AxisAlignedBox& aab);

		/** check if portal intersects an sphere */
		bool intersects(const Sphere& sphere);

		/** check if portal intersects a plane bounded volume */
		bool intersects(const PlaneBoundedVolume& pbv);

		/** check if portal intersects a ray */
		bool intersects(const Ray& ray);

		/** check for intersection between portal & scenenode (also determines
		 * if scenenode crosses over portal
		 */
		PortalIntersectResult intersects(PCZSceneNode* sn);

		/** check if portal crossed over portal */
		bool crossedPortal(const PortalBase* otherPortal);
		/** check if portal touches another portal */
		bool closeTo(const PortalBase* otherPortal);

		/** @copydoc MovableObject::getBoundingBox. */
		const AxisAlignedBox& getBoundingBox() const;

		/** @copydoc MovableObject::getBoundingRadius. */
		Real getBoundingRadius() const
		{ return getRadius(); }

		/** @copydoc MovableObject::_updateRenderQueue. */
		void _updateRenderQueue(RenderQueue* queue)
		{ /* Draw debug info if needed? */ }

		/** @copydoc MovableObject::visitRenderables. */
		void visitRenderables(Renderable::Visitor* visitor, bool debugRenderables = false)
		{ }

		/** Called when scene node moved. */
		void _notifyMoved()
		{
			updateDerivedValues();
			mWasMoved = true;
		}

		/** Called when attached to a scene node. */
		void _notifyAttached(Node* parent, bool isTagPoint = false)
		{
			MovableObject::_notifyAttached(parent, isTagPoint);
			mDerivedUpToDate = false;
		}

		/** Returns true if portal needs update. */
		bool needUpdate();

		/** Returns an updated capsule of the portal for intersection test. */
		const Capsule& getCapsule() const;

		/** Returns an updated AAB of the portal for intersection test. */
		const AxisAlignedBox& getAAB();

	protected:
		// Type of portal (quad, aabb, or sphere)
		PORTAL_TYPE mType;
		/// Zone this portal is currently owned by (in)
		PCZone * mCurrentHomeZone;
		///zone to transfer this portal to
		PCZone * mNewHomeZone;
		/// Corners of the portal - coordinates are relative to the sceneNode
		// NOTE: there are 4 corners if the portal is a quad type
		//       there are 2 corners if the portal is an AABB type
		//       there are 2 corners if the portal is a sphere type (center and point on sphere)
		Vector3 * mCorners;
		/// Direction ("Norm") of the portal - 
		// NOTE: For a Quad portal, determined by the 1st 3 corners.
		// NOTE: For AABB & SPHERE portals, we only have "inward" or "outward" cases.
		//       To indicate "outward", the Direction is UNIT_Z
		//       to indicate "inward", the Direction is NEGATIVE_UNIT_Z
		mutable Vector3 mDirection;
		/// Radius of the sphere enclosing the portal 
		// NOTE: For aabb portals, this value is the distance from the center of the aab to a corner
		mutable Real mRadius;
		// Local Centerpoint of the portal
		mutable Vector3 mLocalCP;
		/// Derived (world coordinates) Corners of the portal
		// NOTE: there are 4 corners if the portal is a quad type
		//       there are 2 corners if the portal is an AABB type (min corner & max corner)
		//       there are 2 corners if the portal is a sphere type (center and point on sphere)
		Vector3 * mDerivedCorners;
		/// Derived (world coordinates) direction of the portal
		// NOTE: Only applicable for a Quad portal
		mutable Vector3 mDerivedDirection;
		/// Derived (world coordinates) of portal (center point)
		mutable Vector3 mDerivedCP;
		/// Sphere of the portal centered on the derived CP
		mutable Sphere mDerivedSphere;
		/// Derived (world coordinates) Plane of the portal
		// NOTE: Only applicable for a Quad portal
		mutable Plane mDerivedPlane;
		/// Previous frame portal cp (in world coordinates)
		mutable Vector3 mPrevDerivedCP;
		/// Previous frame derived plane 
		// NOTE: Only applicable for a Quad portal
		mutable Plane mPrevDerivedPlane;
		/// flag indicating whether or not local values are up-to-date
		mutable bool mLocalsUpToDate;
		/// flag indicating whether or not derived values are up-to-date
		mutable bool mDerivedUpToDate;
		// previous world transform
		mutable Matrix4 mPrevWorldTransform;
		// flag defining if portal is enabled or disabled.
		bool mEnabled;
		// cache of portal's capsule.
		mutable Capsule mPortalCapsule;
		// cache of portal's AAB that contains the bound of portal movement.
		mutable AxisAlignedBox mPortalAAB;
		// cache of portal's previous AAB.
		mutable AxisAlignedBox mPrevPortalAAB;
		// cache of portal's local AAB.
		mutable AxisAlignedBox mLocalPortalAAB;
		// defined if portal was moved previously.
		mutable bool mWasMoved;
	};

	/** Factory object for creating Portal instances */
	class _OgrePCZPluginExport PortalBaseFactory : public MovableObjectFactory
	{
	protected:
		/** get the portal type from name value pair. */
		PortalBase::PORTAL_TYPE getPortalType(const NameValuePairList* params);

	};

}

#endif
