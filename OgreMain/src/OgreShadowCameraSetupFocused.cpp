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
#include "OgreShadowCameraSetupFocused.h"
#include "OgreRoot.h"
#include "OgreSceneManager.h"
#include "OgreCamera.h"
#include "OgreLight.h"
#include "OgrePlane.h"
#include "OgreConvexBody.h"
#include "OgreLogManager.h"


namespace Ogre
{
    FocusedShadowCameraSetup::FocusedShadowCameraSetup(void)
    {
    }
    //-----------------------------------------------------------------------
    FocusedShadowCameraSetup::~FocusedShadowCameraSetup(void)
    {
    }
    /*void FocusedShadowCameraSetup::calculateShadowMappingMatrix(const SceneManager& sm,
        const Camera& cam, const Light& light, Matrix4 *out_view, Matrix4 *out_proj, 
        Camera *out_cam) const
    {
        // get the shadow frustum's far distance
        Real shadowDist = light.getShadowFarDistance();
        if (!shadowDist)
        {
            // need a shadow distance, make one up
            shadowDist = cam.getNearClipDistance() * 3000;
        }
        Real shadowOffset = shadowDist * sm.getShadowDirLightTextureOffset();


        if (light.getType() == Light::LT_DIRECTIONAL)
        {
            // generate view matrix if requested
            if (out_view != NULL)
            {
                *out_view = buildViewMatrix(cam.getDerivedPosition(),
                                            light.getDerivedDirection(), 
                                            cam.getDerivedUp());
            }

            // generate projection matrix if requested
            if (out_proj != NULL)
            {
                *out_proj = Matrix4::getScale(1, 1, -1);
                // *out_proj = Matrix4::IDENTITY;
            }

            // set up camera if requested
            if (out_cam != NULL)
            {
                out_cam->setProjectionType(PT_ORTHOGRAPHIC);
                out_cam->setDirection(light.getDerivedDirection());
                out_cam->setPosition(cam.getDerivedPosition());
                out_cam->setFOVy(Degree(90));
                out_cam->setNearClipDistance(shadowOffset);
            }
        }
        else if (light.getType() == Light::LT_POINT)
        {
            const Vector3 lightDerivedPos( light.getParentNode()->_getDerivedPosition() );
            // target analogue to the default shadow textures
            // Calculate look at position
            // We want to look at a spot shadowOffset away from near plane
            // 0.5 is a little too close for angles
            Vector3 target = cam.getDerivedPosition() + 
                (cam.getDerivedDirection() * shadowOffset);
            Vector3 lightDir = target - lightDerivedPos;
            lightDir.normalise();

            // generate view matrix if requested
            if (out_view != NULL)
                *out_view = buildViewMatrix( lightDerivedPos, lightDir, cam.getDerivedUp());

            // generate projection matrix if requested
            if (out_proj != NULL)
            {
                // set FOV to 120 degrees
                mTempFrustum->setFOVy(Degree(120));

                mTempFrustum->setNearClipDistance(light._deriveShadowNearClipDistance(&cam));
                mTempFrustum->setFarClipDistance(light._deriveShadowFarClipDistance(&cam));

                *out_proj = mTempFrustum->getProjectionMatrix();
            }

            // set up camera if requested
            if (out_cam != NULL)
            {
                out_cam->setProjectionType(PT_PERSPECTIVE);
                out_cam->setDirection(lightDir);
                out_cam->setPosition(lightDerivedPos);
                out_cam->setFOVy(Degree(120));
                out_cam->setNearClipDistance(light._deriveShadowNearClipDistance(&cam));
                out_cam->setFarClipDistance(light._deriveShadowFarClipDistance(&cam));
            }
        }
        else if (light.getType() == Light::LT_SPOTLIGHT)
        {
            const Vector3 lightDerivedPos( light.getParentNode()->_getDerivedPosition() );
            // generate view matrix if requested
            if (out_view != NULL)
            {
                *out_view = buildViewMatrix( lightDerivedPos,
                                            light.getDerivedDirection(), 
                                            cam.getDerivedUp());
            }

            // generate projection matrix if requested
            if (out_proj != NULL)
            {
                // set FOV slightly larger than spotlight range
                mTempFrustum->setFOVy(Ogre::Math::Clamp<Radian>(light.getSpotlightOuterAngle() * 1.2, Radian(0), Radian(Math::PI/2.0f)));

                mTempFrustum->setNearClipDistance(light._deriveShadowNearClipDistance(&cam));
                mTempFrustum->setFarClipDistance(light._deriveShadowFarClipDistance(&cam));

                *out_proj = mTempFrustum->getProjectionMatrix();
            }

            // set up camera if requested
            if (out_cam != NULL)
            {
                out_cam->setProjectionType(PT_PERSPECTIVE);
                out_cam->setDirection(light.getDerivedDirection());
                out_cam->setPosition(lightDerivedPos);
                out_cam->setFOVy(Ogre::Math::Clamp<Radian>(light.getSpotlightOuterAngle() * 1.2, Radian(0), Radian(Math::PI/2.0f)));
                out_cam->setNearClipDistance(light._deriveShadowNearClipDistance(&cam));
                out_cam->setFarClipDistance(light._deriveShadowFarClipDistance(&cam));
            }
        }
    }*/
    //-----------------------------------------------------------------------
    void FocusedShadowCameraSetup::getShadowCamera( const SceneManager *sm, const Camera *cam,
                                                    const Light *light, Camera *texCam, size_t iteration,
                                                    const Vector2 &viewportRealSize ) const
    {
        // check availability - viewport not needed
        OgreAssert(sm != NULL, "SceneManager is NULL");
        OgreAssert(cam != NULL, "Camera (viewer) is NULL");
        OgreAssert(light != NULL, "Light is NULL");
        OgreAssert(texCam != NULL, "Camera (texture) is NULL");

        if( light->getType() != Light::LT_DIRECTIONAL )
        {
            DefaultShadowCameraSetup::getShadowCamera( sm, cam, light, texCam,
                                                       iteration, viewportRealSize );
            return;
        }

        texCam->setNearClipDistance(light->_deriveShadowNearClipDistance(cam));
        texCam->setFarClipDistance(light->_deriveShadowFarClipDistance(cam));

        const AxisAlignedBox &casterBox = sm->getCurrentCastersBox();

        //Will be overriden, but not always (in case we early out to use uniform shadows)
        mMaxDistance = casterBox.getMinimum().distance( casterBox.getMaximum() );

        // in case the casterBox is empty (e.g. there are no casters) simply
        // return the standard shadow mapping matrix
        if( casterBox.isNull() )
        {
            texCam->setProjectionType( PT_ORTHOGRAPHIC );
            //Anything will do, there are no casters. But we must ensure depth of the receiver
            //doesn't become negative else a shadow square will appear (i.e. "the sun is below the floor")
            const Real farDistance = Ogre::min( cam->getFarClipDistance(),
                                                light->getShadowFarDistance() );
            texCam->setPosition( cam->getDerivedPosition() -
                                 light->getDerivedDirection() * farDistance );
            texCam->setOrthoWindow( 1, 1 );
            texCam->setNearClipDistance( 1.0f );
            texCam->setFarClipDistance( 1.1f );

            texCam->getWorldAabbUpdated();

            mMinDistance = 1.0f;
            mMaxDistance = 1.1f;
            return;
        }

        const Node *lightNode = light->getParentNode();
        const Real farDistance= Ogre::min( cam->getFarClipDistance(), light->getShadowFarDistance() );
        const Quaternion scalarLightSpaceToWorld( lightNode->_getDerivedOrientation() );
        const Quaternion scalarWorldToLightSpace( scalarLightSpaceToWorld.Inverse() );
        ArrayQuaternion worldToLightSpace;
        worldToLightSpace.setAll( scalarWorldToLightSpace );

        ArrayVector3 vMinBounds( Mathlib::MAX_POS, Mathlib::MAX_POS, Mathlib::MAX_POS );
        ArrayVector3 vMaxBounds( Mathlib::MAX_NEG, Mathlib::MAX_NEG, Mathlib::MAX_NEG );

        #define NUM_ARRAY_VECTORS (8 + ARRAY_PACKED_REALS - 1) / ARRAY_PACKED_REALS

        //Take the 8 camera frustum's corners, transform to
        //light space, and compute its AABB in light space
        ArrayVector3 corners[NUM_ARRAY_VECTORS];
        cam->getCustomWorldSpaceCorners( corners, farDistance );

        for( size_t i=0; i<NUM_ARRAY_VECTORS; ++i )
        {
            ArrayVector3 lightSpacePoint = worldToLightSpace * corners[i];
            vMinBounds.makeFloor( lightSpacePoint );
            vMaxBounds.makeCeil( lightSpacePoint );
        }

        Vector3 vMinCamFrustumLS = vMinBounds.collapseMin();
        Vector3 vMaxCamFrustumLS = vMaxBounds.collapseMax();

        Vector3 casterAabbCornersLS[8];
        for( size_t i=0; i<8; ++i )
        {
            casterAabbCornersLS[i] = scalarWorldToLightSpace *
                                    casterBox.getCorner( static_cast<AxisAlignedBox::CornerEnum>( i ) );
        }

        ConvexBody convexBody;
        convexBody.define( casterAabbCornersLS );

        Plane p;
        p.redefine( Vector3::NEGATIVE_UNIT_X, vMinCamFrustumLS );
        convexBody.clip( p );
        p.redefine( Vector3::UNIT_X, vMaxCamFrustumLS );
        convexBody.clip( p );
        p.redefine( Vector3::NEGATIVE_UNIT_Y, vMinCamFrustumLS );
        convexBody.clip( p );
        p.redefine( Vector3::UNIT_Y, vMaxCamFrustumLS );
        convexBody.clip( p );
        p.redefine( Vector3::NEGATIVE_UNIT_Z, vMinCamFrustumLS );
        convexBody.clip( p );

        Vector3 vMin( std::numeric_limits<Real>::max(), std::numeric_limits<Real>::max(),
                      std::numeric_limits<Real>::max() );
        Vector3 vMax( -std::numeric_limits<Real>::max(), -std::numeric_limits<Real>::max(),
                      -std::numeric_limits<Real>::max() );

        for( size_t i=0; i<convexBody.getPolygonCount(); ++i )
        {
            const Polygon& polygon = convexBody.getPolygon( i );

            for( size_t j=0; j<polygon.getVertexCount(); ++j )
            {
                const Vector3 &point = polygon.getVertex( j );
                vMin.makeFloor( point );
                vMax.makeCeil( point );
            }
        }

        if( vMin > vMax )
        {
            //There are no casters that will affect the viewing frustum
            //(or something went wrong with the clipping).
            //Rollback to something valid
            vMin = vMinCamFrustumLS;
            vMax = vMaxCamFrustumLS;

            //Add some padding to prevent negative depth (i.e. "the sun is below the floor")
            vMax.z += 5.0f; // Backwards is towards +Z!
        }

        vMin.z = Ogre::min( vMin.z, vMinCamFrustumLS.z );

        //Some padding
        vMax += 0.05f;
        vMin -= 0.05f;

        const float zPadding = 2.0f;

        texCam->setProjectionType( PT_ORTHOGRAPHIC );
        Vector3 shadowCameraPos = (vMin + vMax) * 0.5f;
        shadowCameraPos.z       = vMax.z + zPadding; // Backwards is towards +Z!
        //Go back from light space to world space
        shadowCameraPos = scalarLightSpaceToWorld * shadowCameraPos;
        texCam->setPosition( shadowCameraPos );
        texCam->setOrthoWindow( (vMax.x - vMin.x), (vMax.y - vMin.y) );

        mMinDistance = 1.0f;
        mMaxDistance = vMax.z - vMin.z + zPadding; //We just went backwards, we need to enlarge our depth
        texCam->setNearClipDistance( mMinDistance );
        texCam->setFarClipDistance( mMaxDistance );

        //Update the AABB. Note: Non-shadow caster cameras are forbidden to change mid-render
        texCam->getWorldAabbUpdated();
    }
}
