/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd
Copyright (c) 2006 Matthias Fink, netAllied GmbH <matthias.fink@web.de>                             

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
*/

#include "OgreStableHeaders.h"
#include "OgreShadowCameraSetupPSSM.h"
#include "OgreCamera.h"

namespace Ogre
{
    //---------------------------------------------------------------------
    PSSMShadowCameraSetup::PSSMShadowCameraSetup()
        : mSplitPadding(1.0f), mCurrentIteration(0)
    {
        calculateSplitPoints(3, 100, 100000);
    }
    //---------------------------------------------------------------------
    PSSMShadowCameraSetup::~PSSMShadowCameraSetup()
    {
    }
    //---------------------------------------------------------------------
    void PSSMShadowCameraSetup::calculateSplitPoints(uint splitCount, Real nearDist, Real farDist, Real lambda, Real blend, Real fade)
    {
        if (splitCount < 2)
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Cannot specify less than 2 splits", 
            "PSSMShadowCameraSetup::calculateSplitPoints");

        mSplitPoints.resize(splitCount + 1);
        mSplitBlendPoints.resize(blend == 0.0f ? 0 : splitCount - 1);
        mSplitCount = splitCount;

        mSplitPoints[0] = nearDist;
        for (size_t i = 1; i < mSplitCount; i++)
        {
            Real fraction = (Real)i / (Real)mSplitCount;
            Real splitPoint = lambda * nearDist * Math::Pow(farDist / nearDist, fraction) +
                (1.0f - lambda) * (nearDist + fraction * (farDist - nearDist));

            mSplitPoints[i] = splitPoint;

            if (blend != 0.0f)
            {
                mSplitBlendPoints[i - 1] = Math::lerp(mSplitPoints[i], mSplitPoints[i - 1], blend);
            }
        }
        mSplitPoints[splitCount] = farDist;

        if (fade == 0.0f)
        {
            mSplitFadePoint = 0.0f;
        }
        else
        {
            mSplitFadePoint = Math::lerp(mSplitPoints[mSplitCount], mSplitPoints[mSplitCount - 1], fade);
        }
    }
    //---------------------------------------------------------------------
    void PSSMShadowCameraSetup::setSplitPoints(const SplitPointList& newSplitPoints, Real blend, Real fade)
    {
        if (newSplitPoints.size() < 3) // 3, not 2 since splits + 1 points
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Cannot specify less than 2 splits", 
            "PSSMShadowCameraSetup::setSplitPoints");
        mSplitCount = static_cast<uint>(newSplitPoints.size() - 1);
        mSplitPoints = newSplitPoints;

        if (blend == 0.0f)
        {
            mSplitBlendPoints.resize(0);
        }
        else
        {
            mSplitBlendPoints.resize(mSplitCount - 1);
            for (size_t i = 1; i < mSplitCount; i++)
            {
                mSplitBlendPoints[i - 1] = Math::lerp(mSplitPoints[i], mSplitPoints[i - 1], blend);
            }
        }

        if (fade == 0.0f)
        {
            mSplitFadePoint = 0.0f;
        }
        else
        {
            mSplitFadePoint = Math::lerp(mSplitPoints[mSplitCount], mSplitPoints[mSplitCount - 1], fade);
        }
    }
    //---------------------------------------------------------------------
    void PSSMShadowCameraSetup::getShadowCamera( const Ogre::SceneManager *sm, const Ogre::Camera *cam,
                                                 const Ogre::Light *light, Ogre::Camera *texCam,
                                                 size_t iteration,
                                                 const Vector2 &viewportRealSize ) const
    {
        // apply the right clip distance.
        Real nearDist = mSplitPoints[iteration];
        Real farDist = mSplitPoints[iteration + 1];

        // Add a padding factor to internal distances so that the connecting split point will not have bad artifacts.
        if (iteration > 0)
        {
            nearDist -= mSplitPadding;
            nearDist = std::max( nearDist, mSplitPoints[0] );
        }
        if (iteration < mSplitCount - 1)
        {
            farDist += mSplitPadding;
        }

        mCurrentIteration = iteration;

        // Ouch, I know this is hacky, but it's the easiest way to re-use LiSPSM / Focused
        // functionality right now without major changes
        Camera* _cam = const_cast<Camera*>(cam);
        Real oldNear = _cam->getNearClipDistance();
        Real oldFar = _cam->getFarClipDistance();
        _cam->setNearClipDistance(nearDist);
        _cam->setFarClipDistance(farDist);

        FocusedShadowCameraSetup::getShadowCamera( sm, cam,  light, texCam,
                                                   iteration, viewportRealSize );

        // restore near/far
        _cam->setNearClipDistance(oldNear);
        _cam->setFarClipDistance(oldFar);


    }
}
