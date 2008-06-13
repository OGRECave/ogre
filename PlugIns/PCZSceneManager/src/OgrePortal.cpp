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
Portal.cpp  -  
-----------------------------------------------------------------------------
begin                : Tue Feb 20 2007
author               : Eric Cha
email                : ericc@xenopi.com
current TODO's       : none known
-----------------------------------------------------------------------------
*/

#include "OgrePortal.h"
#include "OgrePCZSceneNode.h"
#include "OgreSphere.h"
#include "OgreCapsule.h"
#include "OgreSegment.h"
#include "OgreRay.h"
#include "OgrePCZone.h"   // need access to real zone class 

using namespace Ogre;

Portal::Portal(const String & name, const PORTAL_TYPE type)
{
	mType = type,
	mName = name;
    mTargetZone = 0;
	mCurrentHomeZone = 0;
	mNewHomeZone = 0;
	mTargetPortal = 0;
	mNode = 0;
	mRadius = 0.0;
	mDirection = Vector3::UNIT_Z;
	mLocalsUpToDate = false;
	// set prevWorldTransform to a zero'd out matrix
	prevWorldTransform = Matrix4::ZERO;
	// default to open
	mOpen = true;
	switch (mType)
	{
	default:
	case PORTAL_TYPE_QUAD:
		mCorners = new Vector3[4];
		mDerivedCorners = new Vector3[4];
		break;
	case PORTAL_TYPE_AABB:
		mCorners = new Vector3[2];
		mDerivedCorners = new Vector3[2];
		break;
	case PORTAL_TYPE_SPHERE:
		mCorners = new Vector3[2];
		mDerivedCorners = new Vector3[2];
		break;
	}
}

Portal::~Portal()
{
	if (mCorners)
		delete [] mCorners;
	mCorners = 0;
	if (mDerivedCorners)
		delete [] mDerivedCorners;
	mDerivedCorners = 0;
}

// Set the SceneNode the Portal is associated with
void Portal::setNode( SceneNode * sn )
{
    mNode = sn ;
	mLocalsUpToDate = false;
}
// Set the 1st Zone the Portal connects to
void Portal::setTargetZone( PCZone * z )
{
    mTargetZone = z ;
}
// Set the zone this portal is in.
void Portal::setCurrentHomeZone( PCZone * z)
{
	// do this here since more than one function calls setCurrentHomeZone
	// also _addPortal is abstract, so easier to do it here.
	if(z)
	{
		// inform old zone of portal change.
		if(mCurrentHomeZone)  
		{
			mCurrentHomeZone->setPortalsUpdated(true);
		}
		z->setPortalsUpdated(true);   // inform new zone of portal change
	}
	mCurrentHomeZone = z;
}

// Set the zone this portal should be moved to
void Portal::setNewHomeZone( PCZone * z)
{
	mNewHomeZone = z;
}

// Set the Portal the Portal connects to
void Portal::setTargetPortal( Portal * p )
{
    mTargetPortal = p ;
}
// Set the local coordinates of one of the portal corners    
void Portal::setCorner( int index, Vector3 & pt)
{
    mCorners[index] = pt ;
	mLocalsUpToDate = false;
}
/** Set the local coordinates of all of the portal corners
*/
// NOTE: there are 4 corners if the portal is a quad type
//       there are 2 corners if the portal is an AABB type (min corner & max corner)
//       there are 2 corners if the portal is a sphere type (center and point on sphere)
void Portal::setCorners( Vector3 * corners )
{
	switch (mType)
	{
	default:
	case PORTAL_TYPE_QUAD:
		mCorners[0] = corners[0];
		mCorners[1] = corners[1];
		mCorners[2] = corners[2];
		mCorners[3] = corners[3];
		break;
	case PORTAL_TYPE_AABB:
		mCorners[0] = corners[0]; // minimum corner
		mCorners[1] = corners[1]; // maximum corner (opposite from min corner)
		break;
	case PORTAL_TYPE_SPHERE:
		mCorners[0] = corners[0]; // center point
		mCorners[1] = corners[1]; // point on sphere surface
		break;
	}
	mLocalsUpToDate = false;
}

// calculate the local direction of the portal from the corners
void Portal::calcDirectionAndRadius(void)
{
	Vector3 radiusVector;
	Vector3 side1, side2;

	switch (mType)
	{
	default:
	case PORTAL_TYPE_QUAD:
		// first calculate local direction
		side1 = mCorners[1] - mCorners[0];
		side2 = mCorners[2] - mCorners[0];
		mDirection = side1.crossProduct(side2);
		mDirection.normalise();
		// calculate local cp
		mLocalCP = Vector3::ZERO;
		for (int i=0;i<4;i++)
		{
			mLocalCP += mCorners[i];
		}
		mLocalCP *= 0.25f;
		// then calculate radius
		radiusVector = mCorners[0] - mLocalCP;
		mRadius = radiusVector.length();
		break;
	case PORTAL_TYPE_AABB:
		// "direction" is is either pointed inward or outward and is set by user, not calculated.
		// calculate local cp
		mLocalCP = Vector3::ZERO;
		for (int i=0;i<2;i++)
		{
			mLocalCP += mCorners[i];
		}
		mLocalCP *= 0.5f;
		// for radius, use distance from corner to center point
		// this gives the radius of a sphere that encapsulates the aabb
		radiusVector = mCorners[0] - mLocalCP;
		mRadius = radiusVector.length();
		break;
	case PORTAL_TYPE_SPHERE:
		// "direction" is is either pointed inward or outward and is set by user, not calculated.
		// local CP is same as corner point 0
		mLocalCP = mCorners[0];
		// since corner1 is point on sphere, radius is simply corner1 - center point
		radiusVector = mCorners[1] - mLocalCP;
		mRadius = radiusVector.length();
		break;
	}
	mDerivedSphere.setRadius(mRadius);
	// locals are now up to date
	mLocalsUpToDate = true;
}

// Calculate the local bounding sphere of the portal from the corner points
Real Portal::getRadius( void )
{
	if (!mLocalsUpToDate)
	{
		calcDirectionAndRadius();
	}
	return mRadius;
}
//Get the coordinates of one of the portal corners
Vector3 & Portal::getCorner( int index)
{
    return mCorners[index];
}
// Get the derived (world) coordinates of a portal corner (assumes they are up-to-date)
Vector3 & Portal::getDerivedCorner( int index) 
{
    return mDerivedCorners[index];
}
// Get the direction of the portal in world coordinates (assumes  it is up-to-date)
Vector3 & Portal::getDerivedDirection( void ) 
{
    return mDerivedDirection;
}
// Get the position (centerpoint) of the portal in world coordinates (assumes  it is up-to-date)
Vector3 & Portal::getDerivedCP( void ) 
{
    return mDerivedCP;
}
// Get the sphere (centered on DerivedCP) of the portal in world coordinates (assumes  it is up-to-date)
Sphere & Portal::getDerivedSphere( void ) 
{
    return mDerivedSphere;
}
// Get the plane of the portal in world coordinates (assumes  it is up-to-date)
Plane & Portal::getDerivedPlane( void ) 
{
    return mDerivedPlane;
}
// Get the previous position (centerpoint) of the portal in world coordinates (assumes  it is up-to-date)
Vector3 & Portal::getPrevDerivedCP( void ) 
{
    return mPrevDerivedCP;
}
// Get the previous plane of the portal in world coordinates (assumes  it is up-to-date)
Plane & Portal::getPrevDerivedPlane( void ) 
{
    return mPrevDerivedPlane;
}
// Update (Calculate) the world spatial values
void Portal::updateDerivedValues(void)
{
	// make sure local values are up to date
	if (!mLocalsUpToDate)
	{
		calcDirectionAndRadius();
	}
	int numCorners = 4;
	if (mType == PORTAL_TYPE_AABB)
		numCorners = 2;
	else if (mType == PORTAL_TYPE_SPHERE)
		numCorners = 2;

	// calculate derived values
	if (mNode)
	{
		if (prevWorldTransform != mNode->_getFullTransform())
		{
            if(mCurrentHomeZone) 
			{
				// inform home zone that a portal has been updated 
				mCurrentHomeZone->setPortalsUpdated(true);   
			}
			// save world transform
			Matrix4 transform = mNode->_getFullTransform();
			Matrix3 rotation;
			// save off the current DerivedCP
			mPrevDerivedCP = mDerivedCP;
			mDerivedCP = transform * mLocalCP;
			mDerivedSphere.setCenter(mDerivedCP);
			switch(mType)
			{
			case PORTAL_TYPE_QUAD:
				for (int i=0;i<numCorners;i++)
				{
					mDerivedCorners[i] =  transform * mCorners[i];
				}
				transform.extract3x3Matrix(rotation);
				mDerivedDirection = rotation * mDirection;
				break;
			case PORTAL_TYPE_AABB:
				{
					AxisAlignedBox aabb;
					aabb.setExtents(mCorners[0], mCorners[1]);
					aabb = mNode->_getWorldAABB();
					//aabb.transform(mNode->_getFullTransform());
					mDerivedCorners[0] = aabb.getMinimum();
					mDerivedCorners[1] = aabb.getMaximum();
					mDerivedDirection = mDirection;
				}
				break;
			case PORTAL_TYPE_SPHERE:
				{
					mDerivedCorners[0] = mDerivedCP;
					mDerivedCorners[1] = transform * mCorners[1];
					mDerivedDirection = mDirection;
				}
				break;
			}
			if (prevWorldTransform != Matrix4::ZERO)
			{
				// save previous calc'd plane
				mPrevDerivedPlane = mDerivedPlane;
				// calc new plane
				mDerivedPlane = Ogre::Plane(mDerivedDirection, mDerivedCP);
				// only update prevWorldTransform if did not move
				// we need to add this conditional to ensure that
				// the portal fully updates when it changes position.
				if (mPrevDerivedPlane == mDerivedPlane &&
					mPrevDerivedCP == mDerivedCP)
				{
					prevWorldTransform = transform;
				}
				mPrevDerivedCP = mDerivedCP;
			}
			else
			{
				// calc new plane
				mDerivedPlane = Ogre::Plane(mDerivedDirection, mDerivedCP);
				// this is first time, so there is no previous, so prev = current.
				mPrevDerivedPlane = mDerivedPlane;
				mPrevDerivedCP = mDerivedCP;
				prevWorldTransform = Matrix4::IDENTITY;
				prevWorldTransform = transform;
			}
		}
	}
	else // no associated node, so just use the local values as derived values
	{
		if (prevWorldTransform != Matrix4::ZERO)
		{
			// save off the current DerivedCP
			mPrevDerivedCP = mDerivedCP;
			mDerivedCP = mLocalCP;
			mDerivedSphere.setCenter(mDerivedCP);
			for (int i=0;i<numCorners;i++)
			{
				mDerivedCorners[i] = mCorners[i];
			}
			mDerivedDirection = mDirection;
			// save previous calc'd plane
			mPrevDerivedPlane = mDerivedPlane;
			// calc new plane
			mDerivedPlane = Ogre::Plane(mDerivedDirection, mDerivedCP);
		}
		else
		{
            if(mCurrentHomeZone) 
			{
				// this case should only happen once 
				mCurrentHomeZone->setPortalsUpdated(true);
			}
			// this is the first time the derived CP has been calculated, so there
			// is no "previous" value, so set previous = current.
			mDerivedCP = mLocalCP;
			mPrevDerivedCP = mDerivedCP;
			mDerivedSphere.setCenter(mDerivedCP);
			for (int i=0;i<numCorners;i++)
			{
				mDerivedCorners[i] = mCorners[i];
			}
			mDerivedDirection = mDirection;
			// calc new plane
			mDerivedPlane = Ogre::Plane(mDerivedDirection, mDerivedCP);
			// this is first time, so there is no previous, so prev = current.
			mPrevDerivedPlane = mDerivedPlane;
			// flag as initialized
			prevWorldTransform = Matrix4::IDENTITY;
		}
	}
}

// Adjust the portal so that it is centered and oriented on the given node
// NOTE: This function will move/rotate the node as well!
// NOTE: The node will become the portal's "associated" node (mNode).
void Portal::adjustNodeToMatch(SceneNode *node )
{
	int i;

	// make sure local values are up to date
	if (!mLocalsUpToDate)
	{
		calcDirectionAndRadius();
	}
	// move the parent node to the center point
	node->setPosition(mLocalCP);

	// move the corner points to be relative to the node
	int numCorners = 4;
	if (mType == PORTAL_TYPE_AABB)
		numCorners = 2;
	else if (mType == PORTAL_TYPE_SPHERE)
		numCorners = 2;

	for (i=0;i<numCorners;i++)
	{
		mCorners[i] -= mLocalCP;
	}
	if (mType != PORTAL_TYPE_AABB &&
		mType != PORTAL_TYPE_SPHERE)
	{
		// NOTE: UNIT_Z is the basis for our local direction
		// orient the node to match the direction
		Quaternion q;
		q = Vector3::UNIT_Z.getRotationTo(mDirection);
		node->setOrientation(q);
	}

	// set the node as the portal's associated node
	setNode(node);

	return;
}

// Open a portal (allows scene traversal and crossing)
void Portal::open()
{
	mOpen = true;
}

// Close a portal (disallows scene traversal and crossing)
void Portal::close()
{
	mOpen = false;
}

// Check if a portal intersects an AABB
// NOTE: This check is not exact.
bool Portal::intersects(const AxisAlignedBox & aab)
{
	// Only check if portal is open
	if (mOpen)
	{
		switch(mType)
		{
		case PORTAL_TYPE_QUAD:
			// since ogre doesn't have built in support for a quad, just check
			// if the box intersects both the sphere of the portal and the plane
			// this can result in false positives, but they will be minimal
			if (!aab.intersects(mDerivedSphere))
			{
				return false;
			}
			if (aab.intersects(mDerivedPlane))
			{
				return true;
			}
			break;
		case PORTAL_TYPE_AABB:
			{
				// aab to aab check
				AxisAlignedBox aabb;
				aabb.setExtents(mDerivedCorners[0], mDerivedCorners[1]);
				return (aab.intersects(aabb));
			}
		case PORTAL_TYPE_SPHERE:
			// aab to sphere check
			return (aab.intersects(mDerivedSphere));
		}
	}
	return false; 
}

// Check if a portal intersects a sphere
// NOTE: This check is not exact.
bool Portal::intersects(const Sphere & sphere)
{
	// Only check if portal is open
	if (mOpen)
	{
		switch(mType)
		{
		case PORTAL_TYPE_QUAD:
			// since ogre doesn't have built in support for a quad, just check
			// if the sphere intersects both the sphere of the portal and the plane
			// this can result in false positives, but they will be minimal
			if (!sphere.intersects(mDerivedSphere))
			{
				return false;
			}
			if (sphere.intersects(mDerivedPlane))
			{
				return true;
			}
			break;
		case PORTAL_TYPE_AABB:
			{
				// aab to aab check
				AxisAlignedBox aabb;
				aabb.setExtents(mDerivedCorners[0], mDerivedCorners[1]);
				return (aabb.intersects(sphere));
			}
		case PORTAL_TYPE_SPHERE:
			return (mDerivedSphere.intersects(sphere));
		}
	}
	return false;
}

// Check if a portal intersects a plane bounded volume
// NOTE: This check is not exact.
// NOTE: UNTESTED as of 5/30/07 (EC)
bool Portal::intersects(const PlaneBoundedVolume & pbv)
{
	// Only check if portal is open
	if (mOpen)
	{
		switch(mType)
		{
		case PORTAL_TYPE_QUAD:
			{
				// first check sphere of the portal
				if (!pbv.intersects(mDerivedSphere))
				{
					return false;
				}
				// if the portal corners are all outside one of the planes of the pbv, 
				// then the portal does not intersect the pbv. (this can result in 
				// some false positives, but it's the best I can do for now)
				PlaneList::const_iterator it = pbv.planes.begin();
				while (it != pbv.planes.end())
				{
					const Plane& plane = *it;
					// check if all 4 corners of the portal are on negative side of the pbv
					bool allOutside = true;
					for (int i=0;i<4;i++)
					{
						if (plane.getSide(mDerivedCorners[i]) != pbv.outside)
						{
							allOutside = false;
						}
					}
					if (allOutside)
					{
						return false;
					}
					it++;
				};
			}
			break;
		case PORTAL_TYPE_AABB:
			{
				AxisAlignedBox aabb;
				aabb.setExtents(mDerivedCorners[0], mDerivedCorners[1]);
				if (!pbv.intersects(aabb))
				{
					return false;
				}
			}
			break;
		case PORTAL_TYPE_SPHERE:
			if (!pbv.intersects(mDerivedSphere))
			{
				return false;
			}
			break;
		}
	}
	return false;
}

// Check if a portal intersects a ray
// NOTE: Kinda using my own invented routine here for quad portals... Better do a lot of testing!
bool Portal::intersects(const Ray & ray )
{
	// Only check if portal is open
	if (mOpen)
	{
		if (mType == PORTAL_TYPE_QUAD)
		{
			// since ogre doesn't have built in support for a quad, I'm going to first
			// find the intersection point (if any) of the ray and the portal plane.  Then
			// using the intersection point, I take the cross product of each side of the portal
			// (0,1,intersect), (1,2, intersect), (2,3, intersect), and (3,0,intersect).  If
			// all 4 cross products have vectors pointing in the same direction, then the
			// intersection point is within the portal, otherwise it is outside.

			std::pair<bool, Real> result = ray.intersects(mDerivedPlane);

			if (result.first == true)
			{
				// the ray intersects the plane, now walk around the edges 
				Vector3 isect = ray.getPoint(result.second);
				Vector3 cross, vect1, vect2;
				Vector3 cross2, vect3, vect4;
				vect1 = mDerivedCorners[1] - mDerivedCorners[0];
				vect2 = isect - mDerivedCorners[0];
				cross = vect1.crossProduct(vect2);
				vect3 = mDerivedCorners[2] - mDerivedCorners[1];
				vect4 = isect - mDerivedCorners[1];
				cross2 = vect3.crossProduct(vect4);
				if (cross.dotProduct(cross2) < 0)
				{
					return false;
				}
				vect1 = mDerivedCorners[3] - mDerivedCorners[2];
				vect2 = isect - mDerivedCorners[2];
				cross = vect1.crossProduct(vect2);
				if (cross.dotProduct(cross2) < 0)
				{
					return false;
				}
				vect1 = mDerivedCorners[0] - mDerivedCorners[3];
				vect2 = isect - mDerivedCorners[3];
				cross = vect1.crossProduct(vect2);
				if (cross.dotProduct(cross2) < 0)
				{
					return false;
				}
				// all cross products pointing same way, so intersect
				// must be on the inside of the portal!
				return true;
			}

			return false;
		}
		else if (mType == PORTAL_TYPE_AABB)
		{
			AxisAlignedBox aabb;
			aabb.setExtents(mDerivedCorners[0], mDerivedCorners[1]);
			std::pair<bool, Real> result = ray.intersects(aabb);
			return result.first;
		}
		else // sphere
		{
			std::pair<bool, Real> result = ray.intersects(mDerivedSphere);
			return result.first;
		}
	}
	return false;
}



/* Test if a scene node intersected a portal during the last time delta 
	* (from last frame time to current frame time).  This function checks
	* if the node "crossed over" the portal also.
*/
Portal::PortalIntersectResult Portal::intersects(PCZSceneNode * pczsn)
{
	// Only check if portal is open
	if (mOpen)
	{
		if (pczsn == mNode)
		{
			// ignore the scene node if it is the node the portal is associated with
			return Portal::NO_INTERSECT;
		}
		// most complicated case - if the portal is a quad:
		if (mType == PORTAL_TYPE_QUAD)
		{
			// the node is modeled as a line segment (prevPostion to currentPosition)
			// intersection test is then between the capsule and the line segment.
			Segment nodeSegment;
			nodeSegment.set(pczsn->getPrevPosition(), pczsn->_getDerivedPosition());

			// we model the portal as a line swept sphere (mPrevDerivedCP to mDerivedCP).
			Capsule portalCapsule;
			portalCapsule.set(mPrevDerivedCP, mDerivedCP, mRadius);

			if (portalCapsule.intersects(nodeSegment))
			{
				// the portal intersected the node at some time from last frame to this frame. 
				// Now check if node "crossed" the portal
				// a crossing occurs if the "side" of the final position of the node compared
				// to the final position of the portal is negative AND the initial position
				// of the node compared to the initial position of the portal is non-negative
				if (mDerivedPlane.getSide(pczsn->_getDerivedPosition()) == Plane::NEGATIVE_SIDE &&
					mPrevDerivedPlane.getSide(pczsn->getPrevPosition()) != Plane::NEGATIVE_SIDE)
				{
					// safety check - make sure the node has at least one dimension which is
					// small enough to fit through the portal! (avoid the "elephant fitting 
					// through a mouse hole" case)
					Vector3 nodeHalfVector = pczsn->_getWorldAABB().getHalfSize();
					Vector3 portalBox = Vector3(mRadius, mRadius, mRadius);
					portalBox.makeFloor(nodeHalfVector);
					if (portalBox.x < mRadius)
					{
						// crossing occurred!
						return Portal::INTERSECT_CROSS;
					}
				}
			}
			// there was no crossing of the portal by the node, but it might be touching
			// the portal.  We check for this by checking the bounding box of the node vs.
			// the sphere of the portal
			if (mDerivedSphere.intersects(pczsn->_getWorldAABB()) &&
				mDerivedPlane.getSide(pczsn->_getWorldAABB()) == Plane::BOTH_SIDE )
			{
				// intersection but no crossing
				// note this means that the node is CURRENTLY touching the portal.
				if (mDerivedPlane.getSide(pczsn->_getDerivedPosition()) != Plane::NEGATIVE_SIDE)
				{
					// the node is on the positive (front) or exactly on the CP of the portal
					return Portal::INTERSECT_NO_CROSS;
				}
				else
				{
					// the node is on the negative (back) side of the portal - it might be in the wrong zone!
					return Portal::INTERSECT_BACK_NO_CROSS;
				}
			}
			// no intersection CURRENTLY.  (there might have been an intersection
			// during the time between last frame and this frame, but it wasn't a portal
			// crossing, and it isn't touching anymore, so it doesn't matter.
			return Portal::NO_INTERSECT;
		}
		else if (mType == PORTAL_TYPE_AABB)
		{
			// for aabb's we check if the center point went from being inside to being outside
			// the aabb (or vice versa) for crossing.  
			AxisAlignedBox aabb;
			aabb.setExtents(mDerivedCorners[0], mDerivedCorners[1]);
			//bool previousInside = aabb.contains(pczsn->getPrevPosition());
			bool currentInside = aabb.contains(pczsn->_getDerivedPosition());
			if (mDirection == Vector3::UNIT_Z)
			{
				// portal norm is "outward" pointing, look for going from outside to inside
				//if (previousInside == false &&
				if (currentInside == true)
				{
					return Portal::INTERSECT_CROSS;
				}
			}
			else
			{
				// portal norm is "inward" pointing, look for going from inside to outside
				//if (previousInside == true &&
				if (currentInside == false)
				{
					return Portal::INTERSECT_CROSS;
				}
			}
			// doesn't cross, but might be touching.  This is a little tricky because we only
			// care if the node aab is NOT fully contained in the portal aabb because we consider
			// the surface of the portal aabb the actual 'portal'.  First, check to see if the 
			// aab of the node intersects the aabb portal
			if (aabb.intersects(pczsn->_getWorldAABB()))
			{
				// now check if the intersection between the two is not the same as the
				// full node aabb, if so, then this means that the node is not fully "contained"
				// which is what we are looking for.
				AxisAlignedBox overlap = aabb.intersection(pczsn->_getWorldAABB());
				if (overlap != pczsn->_getWorldAABB())
				{
					return Portal::INTERSECT_NO_CROSS;
				}
			}
			return Portal::NO_INTERSECT;
		}
		else
		{
			// for spheres we check if the center point went from being inside to being outside
			// the sphere surface (or vice versa) for crossing.  
			//Real previousDistance2 = mPrevDerivedCP.squaredDistance(pczsn->getPrevPosition());
			Real currentDistance2 = mDerivedCP.squaredDistance(pczsn->_getDerivedPosition());
			Real mRadius2 = mRadius * mRadius;
			if (mDirection == Vector3::UNIT_Z)
			{
				// portal norm is "outward" pointing, look for going from outside to inside 
				//if (previousDistance2 >= mRadius2 &&
				if (currentDistance2 < mRadius2)
				{
					return Portal::INTERSECT_CROSS;
				}
			}
			else
			{
				// portal norm is "inward" pointing, look for going from inside to outside
				//if (previousDistance2 < mRadius2 &&
				if (currentDistance2 >= mRadius2)
				{
					return Portal::INTERSECT_CROSS;
				}
			}
			// no crossing, but might be touching - check distance 
			if (Math::Sqrt(Math::Abs(mRadius2 - currentDistance2)) <= mRadius)
			{
				return Portal::INTERSECT_NO_CROSS;
			}
			return Portal::NO_INTERSECT;
		}
	}
	return Portal::NO_INTERSECT; 

}

/* This function check if *this* portal "crossed over" the other portal.
*/
bool Portal::crossedPortal(Portal * otherPortal)
{
	// Only check if portal is open
	if (otherPortal->isOpen())
	{
		// we model both portals as line swept spheres (mPrevDerivedCP to mDerivedCP).
		// intersection test is then between the capsules.
		// BUGBUG! This routine needs to check for case where one or both objects
		//         don't move - resulting in simple sphere tests
		// BUGBUG! If one (or both) portals are aabb's this is REALLY not accurate.
		Capsule portalCapsule, otherPortalCapsule;
		portalCapsule.set( mPrevDerivedCP, mDerivedCP, mRadius);

		otherPortalCapsule.set(otherPortal->getPrevDerivedCP(), 
							   otherPortal->getDerivedCP(),
							   otherPortal->getRadius());

		if (portalCapsule.intersects(otherPortalCapsule))
		{
			// the portal intersected the other portal at some time from last frame to this frame. 
			// Now check if this portal "crossed" the other portal
			switch (otherPortal->getType())
			{
			case PORTAL_TYPE_QUAD:
				// a crossing occurs if the "side" of the final position of this portal compared
				// to the final position of the other portal is negative AND the initial position
				// of this portal compared to the initial position of the other portal is non-negative
				// NOTE: This function assumes that this portal is the smaller portal potentially crossing
				//       over the otherPortal which is larger.
				if (otherPortal->getDerivedPlane().getSide(mDerivedCP) == Plane::NEGATIVE_SIDE &&
					otherPortal->getPrevDerivedPlane().getSide(mPrevDerivedCP) != Plane::NEGATIVE_SIDE)
				{
					// crossing occurred!
					return true;
				}
				break;
			case PORTAL_TYPE_AABB:
				{
					// for aabb's we check if the center point went from being inside to being outside
					// the aabb (or vice versa) for crossing.  
					AxisAlignedBox aabb;
					aabb.setExtents(otherPortal->getDerivedCorner(0), otherPortal->getDerivedCorner(1));
					//bool previousInside = aabb.contains(mPrevDerivedCP);
					bool currentInside = aabb.contains(mDerivedCP);
					if (otherPortal->getDerivedDirection() == Vector3::UNIT_Z)
					{
						// portal norm is "outward" pointing, look for going from outside to inside
						//if (previousInside == false &&
						if (currentInside == true)
						{
							return true;
						}
					}
					else
					{
						// portal norm is "inward" pointing, look for going from inside to outside
						//if (previousInside == true &&
						if (currentInside == false)
						{
							return true;
						}
					}
				}
				break;
			case PORTAL_TYPE_SPHERE:
				{
					// for spheres we check if the center point went from being inside to being outside
					// the sphere surface (or vice versa) for crossing.  
					//Real previousDistance2 = mPrevDerivedCP.squaredDistance(otherPortal->getPrevDerivedCP());
					Real currentDistance2 = mDerivedCP.squaredDistance(otherPortal->getDerivedCP());
					Real mRadius2 = Math::Sqr(otherPortal->getRadius());
					if (otherPortal->getDerivedDirection() == Vector3::UNIT_Z)
					{
						// portal norm is "outward" pointing, look for going from outside to inside 
						//if (previousDistance2 >= mRadius2 &&
						if (currentDistance2 < mRadius2)
						{
							return true;
						}
					}
					else
					{
						// portal norm is "inward" pointing, look for going from inside to outside
						//if (previousDistance2 < mRadius2 &&
						if (currentDistance2 >= mRadius2)
						{
							return true;
						}
					}
				}
				break;
			}
		}
	}
	// there was no crossing of the portal by this portal. It might be touching
	// the other portal (but we don't care currently)
	return false;
}

// check if portal is close to another portal.  
// Note, both portals are assumed to be stationary
// and DerivedCP is the current position.
// this function is INTENTIONALLY NOT EXACT because
// it is used by PCZSM::connectPortalsToTargetZonesByLocation
// which is a utility function to link up nearby portals
//
bool Portal::closeTo(Portal * otherPortal)
{
	// only portals of the same type can be "close to" each other.
	if (mType != otherPortal->getType())
	{
		return false;
	}
	bool close = false;
	switch(mType)
	{
	default:
	case PORTAL_TYPE_QUAD:
		{
			// quad portals must be within 1/4 sphere of each other
			Sphere quarterSphere1 = mDerivedSphere;
			quarterSphere1.setRadius(quarterSphere1.getRadius()*0.25);
			Sphere quarterSphere2 = otherPortal->getDerivedSphere();
			quarterSphere2.setRadius(quarterSphere2.getRadius()*0.25);
			close = quarterSphere1.intersects(quarterSphere2);
		}
		break;
	case PORTAL_TYPE_AABB:
		// NOTE: AABB's must match perfectly
		if (mDerivedCP == otherPortal->getDerivedCP() &&
			mCorners[0] == otherPortal->getCorner(0) &&
			mCorners[1] == otherPortal->getCorner(1))
		{
			close = true;
		}
		break;
	case PORTAL_TYPE_SPHERE:
		// NOTE: Spheres must match perfectly
		if (mDerivedCP == otherPortal->getDerivedCP() &&
			mRadius == otherPortal->getRadius())
		{
			close = true;
		}
		break;
	}
	return close;
}

