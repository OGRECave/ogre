/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2006  Torus Knot Software Ltd
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

#include "OgreStableHeaders.h"
#include "OgreCommon.h"
#include "OgreSceneManager.h"
#include "OgreLight.h"
#include "OgreShadowCameraSetup.h"
#include "OgreCamera.h"
#include "OgreViewport.h"


namespace Ogre 
{
	/// Default constructor
	DefaultShadowCameraSetup::DefaultShadowCameraSetup()  {}
	
	/// Destructor
	DefaultShadowCameraSetup::~DefaultShadowCameraSetup() {}
	
	/// Default shadow camera setup implementation
	void DefaultShadowCameraSetup::getShadowCamera (const SceneManager *sm, const Camera *cam, 
		const Viewport *vp, const Light *light, Camera *texCam, size_t iteration) const
	{
		Vector3 pos, dir;

		// reset custom view / projection matrix in case already set
		texCam->setCustomViewMatrix(false);
		texCam->setCustomProjectionMatrix(false);
		texCam->setNearClipDistance(light->_deriveShadowNearClipDistance(cam));
		texCam->setFarClipDistance(light->_deriveShadowFarClipDistance(cam));

		// get the shadow frustum's far distance
		Real shadowDist = light->getShadowFarDistance();
		if (!shadowDist)
		{
			// need a shadow distance, make one up
			shadowDist = cam->getNearClipDistance() * 300;
		}
		Real shadowOffset = shadowDist * (sm->getShadowDirLightTextureOffset());

		// Directional lights 
		if (light->getType() == Light::LT_DIRECTIONAL)
		{
			// set up the shadow texture
			// Set ortho projection
			texCam->setProjectionType(PT_ORTHOGRAPHIC);
			// set ortho window so that texture covers far dist
			texCam->setOrthoWindow(shadowDist * 2, shadowDist * 2);

			// Calculate look at position
			// We want to look at a spot shadowOffset away from near plane
			// 0.5 is a litle too close for angles
			Vector3 target = cam->getDerivedPosition() + 
				(cam->getDerivedDirection() * shadowOffset);

			// Calculate direction, which same as directional light direction
			dir = - light->getDerivedDirection(); // backwards since point down -z
			dir.normalise();

			// Calculate position
			// We want to be in the -ve direction of the light direction
			// far enough to project for the dir light extrusion distance
			pos = target + dir * sm->getShadowDirectionalLightExtrusionDistance();

			// Round local x/y position based on a world-space texel; this helps to reduce
			// jittering caused by the projection moving with the camera
			// Viewport is 2 * near clip distance across (90 degree fov)
			Real worldTexelSize = (texCam->getNearClipDistance() * 20) / vp->getActualWidth();
			pos.x -= fmod(pos.x, worldTexelSize);
			pos.y -= fmod(pos.y, worldTexelSize);
			pos.z -= fmod(pos.z, worldTexelSize);
		}
		// Spotlight
		else if (light->getType() == Light::LT_SPOTLIGHT)
		{
			// Set perspective projection
			texCam->setProjectionType(PT_PERSPECTIVE);
			// set FOV slightly larger than the spotlight range to ensure coverage
			Radian fovy = light->getSpotlightOuterAngle()*1.2;
			// limit angle
			if (fovy.valueDegrees() > 175)
				fovy = Degree(175);
			texCam->setFOVy(fovy);

			// Calculate position, which same as spotlight position
			pos = light->getDerivedPosition();

			// Calculate direction, which same as spotlight direction
			dir = - light->getDerivedDirection(); // backwards since point down -z
			dir.normalise();
		}
		// Point light
		else
		{
			// Set perspective projection
			texCam->setProjectionType(PT_PERSPECTIVE);
			// Use 120 degree FOV for point light to ensure coverage more area
			texCam->setFOVy(Degree(120));

			// Calculate look at position
			// We want to look at a spot shadowOffset away from near plane
			// 0.5 is a litle too close for angles
			Vector3 target = cam->getDerivedPosition() + 
				(cam->getDerivedDirection() * shadowOffset);

			// Calculate position, which same as point light position
			pos = light->getDerivedPosition();

			dir = (pos - target); // backwards since point down -z
			dir.normalise();
		}

		// Finally set position
		texCam->setPosition(pos);

		// Calculate orientation based on direction calculated above
		/*
		// Next section (camera oriented shadow map) abandoned
		// Always point in the same direction, if we don't do this then
		// we get 'shadow swimming' as camera rotates
		// As it is, we get swimming on moving but this is less noticeable

		// calculate up vector, we want it aligned with cam direction
		Vector3 up = cam->getDerivedDirection();
		// Check it's not coincident with dir
		if (up.dotProduct(dir) >= 1.0f)
		{
		// Use camera up
		up = cam->getUp();
		}
		*/
		Vector3 up = Vector3::UNIT_Y;
		// Check it's not coincident with dir
		if (Math::Abs(up.dotProduct(dir)) >= 1.0f)
		{
			// Use camera up
			up = Vector3::UNIT_Z;
		}
		// cross twice to rederive, only direction is unaltered
		Vector3 left = dir.crossProduct(up);
		left.normalise();
		up = dir.crossProduct(left);
		up.normalise();
		// Derive quaternion from axes
		Quaternion q;
		q.FromAxes(left, up, dir);
		texCam->setOrientation(q);
	}


}
