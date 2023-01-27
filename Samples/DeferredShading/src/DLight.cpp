/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
-----------------------------------------------------------------------------
*/

#include "DLight.h"

#include <cmath>

#include "OgreHardwareBufferManager.h"
#include "OgreCamera.h"
#include "OgreSceneNode.h"
#include "OgreLight.h"
#include "GeomUtils.h"
#include "LightMaterialGenerator.h"
#include "OgreTechnique.h"
#include "OgreSceneManager.h"
#include "OgreShadowCameraSetup.h"

#define ENABLE_BIT(mask, flag) (mask) |= (flag)
#define DISABLE_BIT(mask, flag) (mask) &= ~(flag)

using namespace Ogre;
//-----------------------------------------------------------------------
DLight::DLight(MaterialGenerator *sys, Ogre::Light* parentLight):
    mParentLight(parentLight), bIgnoreWorld(false), mGenerator(sys), mPermutation(0)
{
    // Set up geometry
    // Allocate render operation
    mRenderOp.operationType = RenderOperation::OT_TRIANGLE_LIST;
    mRenderOp.indexData = 0;
    mRenderOp.vertexData = 0;
    mRenderOp.useIndexes = true;

    updateFromParent();
}
//-----------------------------------------------------------------------
DLight::~DLight()
{
    // need to release IndexData and vertexData created for renderable
    delete mRenderOp.indexData;
    delete mRenderOp.vertexData;
}
//-----------------------------------------------------------------------
void DLight::setAttenuation(float c, float b, float a)
{
    // Set Attenuation parameter to shader
    //setCustomParameter(3, Vector4(c, b, a, 0));
    float outerRadius = mParentLight->getAttenuationRange();
    /// There is attenuation? Set material accordingly
    if(c != 1.0f || b != 0.0f || a != 0.0f)
    {
        ENABLE_BIT(mPermutation, LightMaterialGenerator::MI_ATTENUATED);
        if (mParentLight->getType() == Light::LT_POINT)
        {
            //// Calculate radius from Attenuation
            int threshold_level = 10;// difference of 10-15 levels deemed unnoticeable
            float threshold = 1.0f/((float)threshold_level/256.0f); 

            //// Use quadratic formula to determine outer radius
            c = c-threshold;
            float d=std::sqrt(b*b-4*a*c);
            outerRadius = (-2*c)/(b+d);
            outerRadius *= 1.2;
        }
    }
    else
    {
        DISABLE_BIT(mPermutation,LightMaterialGenerator::MI_ATTENUATED);
    }
    
    rebuildGeometry(outerRadius);
}
//-----------------------------------------------------------------------
void DLight::setSpecularColour(const ColourValue &col)
{
    //setCustomParameter(2, Vector4(col.r, col.g, col.b, col.a));
    /// There is a specular component? Set material accordingly
    
    if(col.r != 0.0f || col.g != 0.0f || col.b != 0.0f)
        ENABLE_BIT(mPermutation,LightMaterialGenerator::MI_SPECULAR);
    else
        DISABLE_BIT(mPermutation,LightMaterialGenerator::MI_SPECULAR);
        
}
//-----------------------------------------------------------------------
void DLight::rebuildGeometry(float radius)
{
    //Disable all 3 bits
    DISABLE_BIT(mPermutation, LightMaterialGenerator::MI_POINT);
    DISABLE_BIT(mPermutation, LightMaterialGenerator::MI_SPOTLIGHT);
    DISABLE_BIT(mPermutation, LightMaterialGenerator::MI_DIRECTIONAL);

    switch (mParentLight->getType())
    {
    case Light::LT_DIRECTIONAL:
        createRectangle2D();
        ENABLE_BIT(mPermutation,LightMaterialGenerator::MI_DIRECTIONAL);
        break;
    case Light::LT_POINT:
        /// XXX some more intelligent expression for rings and segments
        createSphere(radius, 10, 10);
        ENABLE_BIT(mPermutation,LightMaterialGenerator::MI_POINT);
        break;
    case Light::LT_SPOTLIGHT:
        Real height = mParentLight->getAttenuationRange();
        Radian coneRadiusAngle = mParentLight->getSpotlightOuterAngle() / 2;
        Real rad = Math::Tan(coneRadiusAngle) * height;
        createCone(rad, height, 20);
        ENABLE_BIT(mPermutation,LightMaterialGenerator::MI_SPOTLIGHT);
        break;
    }   
}
//-----------------------------------------------------------------------
void DLight::createRectangle2D()
{
    /// XXX this RenderOp should really be re-used between DLight objects,
    /// not generated every time
    delete mRenderOp.vertexData; 
    delete mRenderOp.indexData; 

    mRenderOp.vertexData = new VertexData();
    mRenderOp.indexData = 0;

    GeomUtils::createQuad(mRenderOp.vertexData);

    mRenderOp.operationType = RenderOperation::OT_TRIANGLE_STRIP; 
    mRenderOp.useIndexes = false; 

    // Set bounding
    setBoundingBox(AxisAlignedBox(-10000,-10000,-10000,10000,10000,10000));
    mRadius = 15000;
    bIgnoreWorld = true;
}
//-----------------------------------------------------------------------
void DLight::createSphere(float radius, int nRings, int nSegments)
{
    delete mRenderOp.vertexData; 
    delete mRenderOp.indexData;
    mRenderOp.operationType = RenderOperation::OT_TRIANGLE_LIST;
    mRenderOp.indexData = new IndexData();
    mRenderOp.vertexData = new VertexData();
    mRenderOp.useIndexes = true;

    GeomUtils::createSphere(mRenderOp.vertexData, mRenderOp.indexData
        , radius
        , nRings, nSegments
        , false // no normals
        , false // no texture coordinates
        );

    // Set bounding box and sphere
    setBoundingBox( AxisAlignedBox( Vector3(-radius, -radius, -radius), Vector3(radius, radius, radius) ) );
    mRadius = radius;
    bIgnoreWorld = false;
}
//-----------------------------------------------------------------------
void DLight::createCone(float radius, float height, int nVerticesInBase)
{
    delete mRenderOp.vertexData;
    delete mRenderOp.indexData;
    mRenderOp.operationType = RenderOperation::OT_TRIANGLE_LIST;
    mRenderOp.indexData = new IndexData();
    mRenderOp.vertexData = new VertexData();
    mRenderOp.useIndexes = true;

    GeomUtils::createCone(mRenderOp.vertexData, mRenderOp.indexData
        , radius
        , height, nVerticesInBase);

    // Set bounding box and sphere
    setBoundingBox( AxisAlignedBox( 
            Vector3(-radius, 0, -radius), 
            Vector3(radius, height, radius) ) );

    mRadius = radius;
    bIgnoreWorld = false;
}
//-----------------------------------------------------------------------
Real DLight::getBoundingRadius(void) const
{
    return mRadius;
}
//-----------------------------------------------------------------------
Real DLight::getSquaredViewDepth(const Camera* cam) const
{
    if(bIgnoreWorld)
    {
        return 0.0f;
    }
    else
    {
        Vector3 dist = cam->getDerivedPosition() - getParentSceneNode()->_getDerivedPosition();
        return dist.squaredLength();
    }
}
//-----------------------------------------------------------------------
const MaterialPtr& DLight::getMaterial(void) const
{
    return mGenerator->getMaterial(mPermutation);
}
//-----------------------------------------------------------------------
void DLight::getWorldTransforms(Matrix4* xform) const
{
    if (mParentLight->getType() == Light::LT_SPOTLIGHT)
    {
        Quaternion quat = Vector3::UNIT_Y.getRotationTo(mParentLight->getDerivedDirection());
        xform->makeTransform(mParentLight->getDerivedPosition(),
            Vector3::UNIT_SCALE, quat);
    }
    else
    {
        xform->makeTransform(mParentLight->getDerivedPosition(),
            Vector3::UNIT_SCALE, Quaternion::IDENTITY);
    }
    
}
//-----------------------------------------------------------------------
void DLight::updateFromParent()
{
    //TODO : Don't do this unless something changed
    setAttenuation(mParentLight->getAttenuationConstant(), 
        mParentLight->getAttenuationLinear(), mParentLight->getAttenuationQuadric());   
    setSpecularColour(mParentLight->getSpecularColour());

    if (getCastChadows())
    {
        ENABLE_BIT(mPermutation,LightMaterialGenerator::MI_SHADOW_CASTER);
    }
    else
    {
        DISABLE_BIT(mPermutation, LightMaterialGenerator::MI_SHADOW_CASTER);
    }
}
//-----------------------------------------------------------------------
bool DLight::isCameraInsideLight(Ogre::Camera* camera)
{
    switch (mParentLight->getType())
    {
    case Ogre::Light::LT_DIRECTIONAL:
        return false;
    case Ogre::Light::LT_POINT:
        {
        Ogre::Real distanceFromLight = camera->getDerivedPosition()
            .distance(mParentLight->getDerivedPosition());
        //Small epsilon fix to account for the fact that we aren't a true sphere.
        return distanceFromLight <= mRadius + camera->getNearClipDistance() + 0.1; 
        }
    case Ogre::Light::LT_SPOTLIGHT:
        {
        Ogre::Vector3 lightPos = mParentLight->getDerivedPosition();
        Ogre::Vector3 lightDir = mParentLight->getDerivedDirection();
        Ogre::Radian attAngle = mParentLight->getSpotlightOuterAngle();
        
        //Extend the analytic cone's radius by the near clip range by moving its tip accordingly.
        //Some trigonometry needed here.
        Ogre::Vector3 clipRangeFix = -lightDir * (camera->getNearClipDistance() / Ogre::Math::Tan(attAngle/2));
        lightPos = lightPos + clipRangeFix;
    
        Ogre::Vector3 lightToCamDir = camera->getDerivedPosition() - lightPos;
        Ogre::Real distanceFromLight = lightToCamDir.normalise();

        Ogre::Real cosAngle = lightToCamDir.dotProduct(lightDir);
        Ogre::Radian angle = Ogre::Math::ACos(cosAngle);
        //Check whether we will see the cone from our current POV.
        return (distanceFromLight <= (mParentLight->getAttenuationRange() / cosAngle + clipRangeFix.length()))
            && (angle <= attAngle);
        }
    default:
        //Please the compiler
        return false;
    }
}
//-----------------------------------------------------------------------
bool DLight::getCastChadows() const
{
    return 
        mParentLight->_getManager()->isShadowTechniqueInUse() &&
        mParentLight->getCastShadows() && 
        (mParentLight->getType() == Light::LT_DIRECTIONAL || mParentLight->getType() == Light::LT_SPOTLIGHT);
}
//-----------------------------------------------------------------------
void DLight::updateFromCamera(Ogre::Camera* camera)
{
    //Set shader params
    const Ogre::MaterialPtr& mat = getMaterial();
    if (!mat->isLoaded()) 
    {
        mat->load();
    }
    Ogre::Technique* tech = mat->getBestTechnique();
    Ogre::Vector3 farCorner = camera->getViewMatrix(true) * camera->getWorldSpaceCorners()[4];

    for (unsigned short i=0; i<tech->getNumPasses(); i++) 
    {
        Ogre::Pass* pass = tech->getPass(i);
        // get the vertex shader parameters
        Ogre::GpuProgramParametersSharedPtr params = pass->getVertexProgramParameters();
        // set the camera's far-top-right corner
        if (params->_findNamedConstantDefinition("farCorner"))
            params->setNamedConstant("farCorner", farCorner);
        
        params = pass->getFragmentProgramParameters();
        if (params->_findNamedConstantDefinition("farCorner"))
            params->setNamedConstant("farCorner", farCorner);

        //If inside light geometry, render back faces with CMPF_GREATER, otherwise normally
        if (mParentLight->getType() == Ogre::Light::LT_DIRECTIONAL)
        {
            pass->setCullingMode(Ogre::CULL_CLOCKWISE);
            pass->setDepthCheckEnabled(false);
        }
        else
        {
            pass->setDepthCheckEnabled(true);
            if (isCameraInsideLight(camera))
            {
                pass->setCullingMode(Ogre::CULL_ANTICLOCKWISE);
                pass->setDepthFunction(Ogre::CMPF_GREATER_EQUAL);
            }
            else
            {
                pass->setCullingMode(Ogre::CULL_CLOCKWISE);
                pass->setDepthFunction(Ogre::CMPF_LESS_EQUAL);
            }
        }

        SceneNode dummyNode(NULL);
        Camera shadowCam("ShadowCameraSetupCam", 0);
        dummyNode.attachObject(&shadowCam);
        shadowCam._notifyViewport(camera->getViewport());
        SceneManager* sm = mParentLight->_getManager();
        sm->getShadowCameraSetup()->getShadowCamera(sm, 
            camera, camera->getViewport(), mParentLight, &shadowCam, 0);
            
        //Get the shadow camera position
        if (params->_findNamedConstantDefinition("shadowCamPos")) 
        {
            params->setNamedConstant("shadowCamPos", shadowCam.getDerivedPosition());
        }
        if (params->_findNamedConstantDefinition("shadowFarClip"))
        {
            params->setNamedConstant("shadowFarClip", shadowCam.getFarClipDistance());
        }

    }
}
