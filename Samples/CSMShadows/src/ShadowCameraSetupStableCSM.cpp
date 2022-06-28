// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
/*  Copyright 2010-2012 Matthew Paul Reid
*/
#include "ShadowCameraSetupStableCSM.h"
#include "CSMGpuConstants.h"

namespace Ogre {

StableCSMShadowCameraSetup::StableCSMShadowCameraSetup() :
    mSplitPadding(1.0f)
{
    calculateSplitPoints(3, 100, 100000);
}
//---------------------------------------------------------------------
StableCSMShadowCameraSetup::~StableCSMShadowCameraSetup()
{
}
//---------------------------------------------------------------------
void StableCSMShadowCameraSetup::calculateSplitPoints(size_t cascadeCount, Real firstSplitDist, Real farDist, Real lambda)
{
    OgreAssert(cascadeCount >= 2, "Cannot specify less than 2 cascades");

    mSplitPoints.resize(cascadeCount + 1);
    mCascadeCount = cascadeCount;

    mSplitPoints[0] = 0;
    firstSplitDist = std::max((Real)0.001, firstSplitDist);

    for (size_t i = 1; i <= mCascadeCount; i++)
    {
        Real fraction = (Real)(i-1) / (mCascadeCount-1);
        Real logDist = firstSplitDist * Math::Pow(farDist / firstSplitDist, fraction);
        Real linDist = firstSplitDist + fraction * (farDist - firstSplitDist);
        Real splitPoint = linDist + lambda * (logDist - linDist);

        mSplitPoints[i] = splitPoint;
    }
}
//---------------------------------------------------------------------
void StableCSMShadowCameraSetup::setSplitPoints(const SplitPointList& newSplitPoints)
{
    OgreAssert(newSplitPoints.size() >= 3, "Cannot specify less than 2 splits"); // 3, not 2 since splits + 1 points
    mCascadeCount = newSplitPoints.size() - 1;
    mSplitPoints = newSplitPoints;
}

//---------------------------------------------------------------------
void StableCSMShadowCameraSetup::getShadowCamera(const Ogre::SceneManager *sm, const Ogre::Camera *cam,
    const Ogre::Viewport *vp, const Ogre::Light *light, Ogre::Camera *texCam, size_t iteration) const
{
    texCam->setLodBias(0.25);

    // apply the right clip distance.
    Real nearDist = mSplitPoints[iteration];
    Real farDist = mSplitPoints[iteration + 1];

    // Add a padding factor to internal distances so that the connecting split point will not have bad artifacts.
    if (iteration > 0)
    {
        nearDist -= mSplitPadding;
    }
    if (iteration < mCascadeCount - 1)
    {
        farDist += mSplitPadding;
    }

    getShadowCameraForCascade(sm, cam, vp, light, texCam, iteration, nearDist, farDist);
}



void StableCSMShadowCameraSetup::getShadowCameraForCascade (const SceneManager *sm, const Camera *cam,
    const Viewport *vp, const Light *light, Camera *texCam, size_t iteration,
    Ogre::Real nearSplit, Ogre::Real farSplit) const
{
    OgreAssert(light->getType() == Light::LT_DIRECTIONAL, "only directional lights supported");

    Vector3 pos, dir;
    Matrix3 rot;

    // reset custom view / projection matrix in case already set
    texCam->setCustomViewMatrix(false);
    texCam->setCustomProjectionMatrix(false);

    // set up the shadow texture
    // Set ortho projection
    texCam->setProjectionType(PT_ORTHOGRAPHIC);
    // set easy FOV and near dist so that texture covers far dist
    texCam->setFOVy(Degree(90));

    float nearClip = light->_deriveShadowNearClipDistance(cam);
    float farClip = light->_deriveShadowFarClipDistance();

    texCam->setNearClipDistance(nearClip);
    texCam->setFarClipDistance(farClip);

    // need an actual farClip, so make one up
    if (farClip == 0)
        farClip = nearClip * 3000;

    // Calculate texCam direction, which same as directional light direction
    dir = - light->getDerivedDirection(); // backwards since point down -z
    dir.normalise();


    //get light orientation

    Vector3 up = Vector3::UNIT_Y;
    // Check it's not coincident with dir
    if (Math::Abs(up.dotProduct(dir)) >= 1.0f)
    {
        // Use camera up
        up = Vector3::UNIT_Z;
    }
    rot = Math::lookRotation(dir, up);

    // Find minimum enclosing sphere for view frustum
    // We do this in local space so that we don't have slight precision variation between frames
    float vertical = Math::Tan(cam->getFOVy()*0.5f);
    float horizontal = Math::Tan(cam->getFOVy()*0.5f * cam->getAspectRatio());

    Vector3 topLeftFar;
    topLeftFar.x = horizontal * farSplit;
    topLeftFar.y = vertical * farSplit;
    topLeftFar.z = farSplit;

    Vector3 bottomRightNear;
    bottomRightNear.z = nearSplit;
    bottomRightNear.x = horizontal * bottomRightNear.z;
    bottomRightNear.y = vertical * bottomRightNear.z;

    float dist = (topLeftFar.squaredLength() - bottomRightNear.squaredLength()) / (2 * (topLeftFar.z - bottomRightNear.z));

    if (dist > farSplit)
    {
        dist = farSplit;
    }

    Vector3 localPos(0, 0, -dist); // we have now found the point along frustum center which is equi-distant to the opposing corner positions

    Real diameter = 2.0f * topLeftFar.distance(-localPos);
    pos = cam->getDerivedPosition() + cam->getDerivedOrientation() * localPos;

    diameter *= (Real)1.01; // allow some boundary pixels for filtering etc. TODO - make this a user constant
    pos += dir * 0.5 * (farClip - nearClip); // pull light back so we can see the scene


    //calculate window size
    texCam->setOrthoWindowWidth(diameter);


    // Round local x/y position based on a world-space texel; this helps to reduce
    // jittering caused by the projection moving with the camera
    Real worldTexelSize = (texCam->getOrthoWindowWidth()) / texCam->getViewport()->getActualWidth();

    //convert world space camera position into light space
    Vector3 lightSpacePos = rot.transpose() * pos;

    //snap to nearest texel
    lightSpacePos.x -= fmod(lightSpacePos.x, worldTexelSize);
    lightSpacePos.y -= fmod(lightSpacePos.y, worldTexelSize);

    //convert back to world space
    pos = rot * lightSpacePos;

    // Finally set position
    texCam->getParentNode()->setPosition(pos);
    texCam->getParentNode()->setOrientation(rot);
}

}
