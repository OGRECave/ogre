/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2006  Torus Knot Software Ltd
Copyright (c) 2006 Matthias Fink, netAllied GmbH <matthias.fink@web.de>								
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
#include "OgreShadowCameraSetupPSSM.h"
#include "OgreSceneManager.h"
#include "OgreCamera.h"

namespace Ogre
{
	//---------------------------------------------------------------------
	PSSMShadowCameraSetup::PSSMShadowCameraSetup()
		: mSplitPadding(1.0f)
	{
		calculateSplitPoints(3, 100, 100000);
		setOptimalAdjustFactor(0, 5);
		setOptimalAdjustFactor(1, 1);
		setOptimalAdjustFactor(2, 0);


	}
	//---------------------------------------------------------------------
	PSSMShadowCameraSetup::~PSSMShadowCameraSetup()
	{
	}
	//---------------------------------------------------------------------
	void PSSMShadowCameraSetup::calculateSplitPoints(size_t splitCount, Real nearDist, Real farDist, Real lambda)
	{
		if (splitCount < 2)
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Cannot specify less than 2 splits", 
			"PSSMShadowCameraSetup::calculateSplitPoints");

		mSplitPoints.resize(splitCount + 1);
		mOptimalAdjustFactors.resize(splitCount);
		mSplitCount = splitCount;

		mSplitPoints[0] = nearDist;
		for (size_t i = 1; i < mSplitCount; i++)
		{
			Real fraction = (Real)i / (Real)mSplitCount;
			Real splitPoint = lambda * nearDist * Math::Pow(farDist / nearDist, fraction) +
				(1.0 - lambda) * (nearDist + fraction * (farDist - nearDist));

			mSplitPoints[i] = splitPoint;
		}
		mSplitPoints[splitCount] = farDist;

	}
	//---------------------------------------------------------------------
	void PSSMShadowCameraSetup::setSplitPoints(const SplitPointList& newSplitPoints)
	{
		if (newSplitPoints.size() < 3) // 3, not 2 since splits + 1 points
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Cannot specify less than 2 splits", 
			"PSSMShadowCameraSetup::setSplitPoints");
		mSplitCount = newSplitPoints.size() - 1;
		mSplitPoints = newSplitPoints;
		mOptimalAdjustFactors.resize(mSplitCount);
	}
	//---------------------------------------------------------------------
	void PSSMShadowCameraSetup::setOptimalAdjustFactor(size_t splitIndex, Real factor)
	{
		if (splitIndex >= mOptimalAdjustFactors.size())
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Split index out of range", 
			"PSSMShadowCameraSetup::setOptimalAdjustFactor");
		mOptimalAdjustFactors[splitIndex] = factor;
		
	}
	//---------------------------------------------------------------------
	Real PSSMShadowCameraSetup::getOptimalAdjustFactor() const
	{
		// simplifies the overriding of the LiSPSM opt adjust factor use
		return mOptimalAdjustFactors[mCurrentIteration];
	}
	//---------------------------------------------------------------------
	void PSSMShadowCameraSetup::getShadowCamera(const Ogre::SceneManager *sm, const Ogre::Camera *cam,
		const Ogre::Viewport *vp, const Ogre::Light *light, Ogre::Camera *texCam, size_t iteration) const
	{
		// apply the right clip distance.
		Real nearDist = mSplitPoints[iteration];
		Real farDist = mSplitPoints[iteration + 1];

		// Add a padding factor to internal distances so that the connecting split point will not have bad artifacts.
		if (iteration > 0)
		{
			nearDist -= mSplitPadding;
		}
		if (iteration < mSplitCount - 1)
		{
			farDist += mSplitPadding;
		}

		mCurrentIteration = iteration;

		// Ouch, I know this is hacky, but it's the easiest way to re-use LiSPSM / Focussed
		// functionality right now without major changes
		Camera* _cam = const_cast<Camera*>(cam);
		Real oldNear = _cam->getNearClipDistance();
		Real oldFar = _cam->getFarClipDistance();
		_cam->setNearClipDistance(nearDist);
		_cam->setFarClipDistance(farDist);

		LiSPSMShadowCameraSetup::getShadowCamera(sm, cam, vp, light, texCam, iteration);

		// restore near/far
		_cam->setNearClipDistance(oldNear);
		_cam->setFarClipDistance(oldFar);


	}
}
