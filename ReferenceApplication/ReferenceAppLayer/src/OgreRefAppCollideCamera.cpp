/*
-----------------------------------------------------------------------------
This source file is part of the OGRE Reference Application, a layer built
on top of OGRE(Object-oriented Graphics Rendering Engine)
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
*/
#include "OgreRefAppCollideCamera.h"
#include "OgreRefAppWorld.h"
#include "OgreLogManager.h"


namespace OgreRefApp {

    //-----------------------------------------------------------------------
    CollideCamera::CollideCamera(const String& name): ApplicationObject(name) 
    {
        setUp(name);
        
    }
    //-----------------------------------------------------------------------
    void CollideCamera::setUp(const String& name)
    {
        // Create visual presence
        SceneManager* sm = World::getSingleton().getSceneManager();
        mSceneNode = sm->getRootSceneNode()->createChildSceneNode(name);

        mCamera = sm->createCamera(name);

        mSceneNode->attachObject(mCamera);
        // Add reverse reference (to self!)
        mCamera->setUserObject(this);

        // No mass body (considered static)

        // Create collision proxy, at near dist
        // SpaceID is irrelevant, we're doing our own spacial partitioning
        dSphere* odeSphere = new dSphere(0, mCamera->getNearClipDistance());
        mCollisionProxies.push_back(odeSphere);
        updateCollisionProxies();


    }
    //-----------------------------------------------------------------------
    void CollideCamera::_notifyCollided(SceneQuery::WorldFragment* wf, const CollisionInfo& info) 
    {
        this->translateWorldSpace(info.normal * info.penetrationDepth);

    }
    //-----------------------------------------------------------------------
    void CollideCamera::setOrientation(const Quaternion& orientation)
    {
        // Set on camera
        mCamera->setOrientation(orientation);
    }
    //-----------------------------------------------------------------------
    const Quaternion& CollideCamera::getOrientation(void)
    {
        return mCamera->getOrientation();
    }
    //-----------------------------------------------------------------------
    void CollideCamera::roll(const Radian& angle) 
    {
        mCamera->roll(angle);
    }
    //-----------------------------------------------------------------------
    void CollideCamera::pitch(const Radian& angle) 
    {
        mCamera->pitch(angle);
    }
    //-----------------------------------------------------------------------
    void CollideCamera::yaw(const Radian& angle) 
    {
        mCamera->yaw(angle);
    }
    //-----------------------------------------------------------------------
    void CollideCamera::rotate(const Vector3& axis, const Radian& angle) 
    {
        mCamera->rotate(axis, angle);
    }
    //-----------------------------------------------------------------------
    void CollideCamera::rotate(const Quaternion& q)
    {
        mCamera->rotate(q);
    }
    //-----------------------------------------------------------------------
    void CollideCamera::translate(const Vector3& d)
    {
        // Adjust position by rotation
        Vector3 newTrans = mCamera->getOrientation() * d;
        translateWorldSpace(newTrans);

    }
    //-----------------------------------------------------------------------
    void CollideCamera::setProjectionType(ProjectionType pt) 
    {
        mCamera->setProjectionType(pt);
    }
    //-----------------------------------------------------------------------
    ProjectionType CollideCamera::getProjectionType(void) const 
    {
        return mCamera->getProjectionType();
    }
    //-----------------------------------------------------------------------
    void CollideCamera::setPolygonMode(PolygonMode sd) 
    {
        mCamera->setPolygonMode(sd);
    }
    //-----------------------------------------------------------------------
    PolygonMode CollideCamera::getPolygonMode(void) const 
    {
        return mCamera->getPolygonMode();
    }
    //-----------------------------------------------------------------------
    void CollideCamera::setDirection(Real x, Real y, Real z) 
    {
        mCamera->setDirection(x, y, z);

    }
    //-----------------------------------------------------------------------
    void CollideCamera::setDirection(const Vector3& vec) 
    {
        mCamera->setDirection(vec);
    }
    //-----------------------------------------------------------------------
    Vector3 CollideCamera::getDirection(void) const 
    {
        return mCamera->getDirection();
    }
    //-----------------------------------------------------------------------
    void CollideCamera::lookAt( const Vector3& targetPoint ) 
    {
        mCamera->lookAt(targetPoint);
    }
    //-----------------------------------------------------------------------
    void CollideCamera::lookAt(Real x, Real y, Real z) 
    {
        mCamera->lookAt(x, y, z);
    }
    //-----------------------------------------------------------------------
    void CollideCamera::setFixedYawAxis( bool useFixed, const Vector3& fixedAxis) 
    {
        mCamera->setFixedYawAxis(useFixed, fixedAxis);
    }
    //-----------------------------------------------------------------------
    void CollideCamera::setFOVy(const Radian& fovy)
    {
        mCamera->setFOVy(fovy);
        nearDistChanged();
    }
    //-----------------------------------------------------------------------
    const Radian& CollideCamera::getFOVy(void) const
    {
        return mCamera->getFOVy();
    }
    //-----------------------------------------------------------------------
    void CollideCamera::setNearClipDistance(Real nearDist) 
    {
        mCamera->setNearClipDistance(nearDist);
        nearDistChanged();
    }
    //-----------------------------------------------------------------------
    Real CollideCamera::getNearClipDistance(void) const 
    {
        return mCamera->getNearClipDistance();
    }
    //-----------------------------------------------------------------------
    void CollideCamera::setFarClipDistance(Real farDist) 
    {
        mCamera->setFarClipDistance(farDist);
    }
    //-----------------------------------------------------------------------
    Real CollideCamera::getFarClipDistance(void) const 
    {
        return mCamera->getFarClipDistance();
    }
    //-----------------------------------------------------------------------
    void CollideCamera::setAspectRatio(Real ratio) 
    {
        mCamera->setAspectRatio(ratio);
    }
    //-----------------------------------------------------------------------
    Real CollideCamera::getAspectRatio(void) const 
    {
        return mCamera->getAspectRatio();
    }
    //-----------------------------------------------------------------------
    const Plane& CollideCamera::getFrustumPlane( FrustumPlane plane ) 
    {
        return mCamera->getFrustumPlane(plane);
    }
    //-----------------------------------------------------------------------
    bool CollideCamera::isVisible(const AxisAlignedBox& bound, FrustumPlane* culledBy) 
    {
        return mCamera->isVisible(bound, culledBy);
    }
    //-----------------------------------------------------------------------
    bool CollideCamera::isVisible(const Sphere& bound, FrustumPlane* culledBy) 
    {
        return mCamera->isVisible(bound, culledBy);
    }
    //-----------------------------------------------------------------------
    bool CollideCamera::isVisible(const Vector3& vert, FrustumPlane* culledBy) 
    {
        return mCamera->isVisible(vert, culledBy);
    }
    //-----------------------------------------------------------------------
    void CollideCamera::nearDistChanged(void)
    {
        // Alter the size of the collision proxy to compensate
        CollisionProxyList::iterator i = mCollisionProxies.begin();
        dSphere* sph = static_cast<dSphere*>(*i);
        sph->setRadius(getNearClipDistance());
    }

}

