/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
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
#include "OgreShadowCameraSetupLiSPSM.h"
#include "OgreRoot.h"
#include "OgreSceneManager.h"
#include "OgreCamera.h"
#include "OgreLight.h"
#include "OgrePlane.h"
#include "OgreConvexBody.h"

namespace Ogre
{


	LiSPSMShadowCameraSetup::LiSPSMShadowCameraSetup(void)
		: mOptAdjustFactor(0.1f)
		, mUseSimpleNOpt(true)
		, mOptAdjustFactorTweak(1.0)
		, mCosCamLightDirThreshold(0.9)
	{
	}
	//-----------------------------------------------------------------------
	LiSPSMShadowCameraSetup::~LiSPSMShadowCameraSetup(void)
	{
	}
	//-----------------------------------------------------------------------
	Matrix4 LiSPSMShadowCameraSetup::calculateLiSPSM(const Matrix4& lightSpace, 
		const PointListBody& bodyB, const PointListBody& bodyLVS,
		const SceneManager& sm, const Camera& cam, const Light& light) const
	{
		// set up bodyB AAB in light space
		AxisAlignedBox bodyBAAB_ls;
		for (size_t i = 0; i < bodyB.getPointCount(); ++i)
		{
			bodyBAAB_ls.merge(lightSpace * bodyB.getPoint(i));
		}

		// near camera point in light space
		const Vector3 e_ls = lightSpace * getNearCameraPoint_ws(cam.getViewMatrix(), bodyLVS);

		// C_start has x and y of e and z from the bodyABB_ls (we look down the negative z axis, so take the maximum z value)
		const Vector3 C_start_ls(e_ls.x, e_ls.y, bodyBAAB_ls.getMaximum().z);

		// calculate the optimal distance between origin and near plane
		Real n_opt;

		if (mUseSimpleNOpt)
			n_opt = calculateNOptSimple(bodyLVS, cam);
		else
			n_opt = calculateNOpt(lightSpace, bodyBAAB_ls, bodyLVS, cam);

		// in case n_opt is null, uniform shadow mapping will be done
		if (n_opt <= 0.0)
		{
			return Matrix4::IDENTITY;
		}

		// calculate the projection center C which is n units behind the near plane of P
		// we look into the negative z direction so add n
		const Vector3 C(C_start_ls + n_opt * Vector3::UNIT_Z);

		// set up a transformation matrix to transform the light space to its new origin
		Matrix4 lightSpaceTranslation(Matrix4::IDENTITY);
		lightSpaceTranslation.setTrans(-C);

		// range from bMin to bMax; d = |B_z_far - B_z_near|
		Real d = Math::Abs(bodyBAAB_ls.getMaximum().z - bodyBAAB_ls.getMinimum().z);

		// set up the LiSPSM perspective transformation
		// build up frustum to map P onto the unit cube with (-1/-1/-1) and (+1/+1/+1)
		Matrix4 P = buildFrustumProjection(-1, 1, -1, 1, n_opt + d, n_opt);

		return P * lightSpaceTranslation;
	}
	//-----------------------------------------------------------------------
	Real LiSPSMShadowCameraSetup::calculateNOpt(const Matrix4& lightSpace, 
		const AxisAlignedBox& bodyBABB_ls, const PointListBody& bodyLVS, 
		const Camera& cam) const
	{
		// get inverse light space matrix
		Matrix4 invLightSpace = lightSpace.inverse();

		// get view matrix
		const Matrix4& viewMatrix = cam.getViewMatrix();

		// calculate z0_ls
		const Vector3 e_ws  = getNearCameraPoint_ws(viewMatrix, bodyLVS);
		const Vector3 z0_ls = calculateZ0_ls(lightSpace, e_ws, bodyBABB_ls.getMaximum().z, cam);

		// z1_ls has the same x and y values as z0_ls and the minimum z values of bodyABB_ls
		const Vector3 z1_ls = Vector3(z0_ls.x, z0_ls.y, bodyBABB_ls.getMinimum().z);

		// world
		const Vector3 z0_ws = invLightSpace * z0_ls;
		const Vector3 z1_ws = invLightSpace * z1_ls;

		// eye
		const Vector3 z0_es = viewMatrix * z0_ws;
		const Vector3 z1_es = viewMatrix * z1_ws;

		const Real z0 = z0_es.z;
		const Real z1 = z1_es.z;

		// check if we have to do uniform shadow mapping
		if ((z0 < 0 && z1 > 0) ||
			(z1 < 0 && z0 > 0))
		{
			// apply uniform shadow mapping
			return 0.0;
		}
		return cam.getNearClipDistance() + Math::Sqrt(z0 * z1) * getOptimalAdjustFactor() * mOptAdjustFactorTweak;
	}
	//-----------------------------------------------------------------------
	Real LiSPSMShadowCameraSetup::calculateNOptSimple(const PointListBody& bodyLVS, 
		const Camera& cam) const
	{
		// get view matrix
		const Matrix4& viewMatrix = cam.getViewMatrix();

		// calculate e_es
		const Vector3 e_ws  = getNearCameraPoint_ws(viewMatrix, bodyLVS);
		const Vector3 e_es = viewMatrix * e_ws;

		// according to the new formula (mainly for directional lights)
		// n_opt = zn + sqrt(z0 * z1);
		// zn is set to Abs(near eye point)
		// z0 is set to the near camera clip distance
		// z1 is set to the far camera clip distance
		return (Math::Abs(e_es.z) + Math::Sqrt(cam.getNearClipDistance() * cam.getFarClipDistance())) * getOptimalAdjustFactor() * mOptAdjustFactorTweak;
	}
	//-----------------------------------------------------------------------
	Vector3 LiSPSMShadowCameraSetup::calculateZ0_ls(const Matrix4& lightSpace, 
		const Vector3& e, Real bodyB_zMax_ls, const Camera& cam) const
	{
		// z0_ls lies on the intersection point between the planes 'bodyB_ls near plane 
		// (z = bodyB_zNear_ls)' and plane with normal UNIT_X where e_ls lies upon (x = e_ls_x) 
		// and the camera's near clipping plane (ls). We are looking towards the negative 
		// z-direction, so bodyB_zNear_ls equals bodyB_zMax_ls.

		const Vector3& camDir = cam.getDerivedDirection();
		const Vector3 e_ls = lightSpace * e;

		// set up a plane with the camera direction as normal and e as a point on the plane
		Plane plane(camDir, e);

		plane = lightSpace * plane;

		// try to intersect plane with a ray from origin V3(e_ls_x, 0.0, bodyB_zNear_ls)T
		// and direction +/- UNIT_Y
		Ray ray(Vector3(e_ls.x, 0.0, bodyB_zMax_ls), Vector3::UNIT_Y);
		std::pair< bool, Real > intersect = ray.intersects(plane);

		// we got an intersection point
		if (intersect.first == true)
		{
			return ray.getPoint(intersect.second);
		}
		else
		{
			// try the other direction
			ray = Ray(Vector3(e_ls.x, 0.0, bodyB_zMax_ls), Vector3::NEGATIVE_UNIT_Y);
			std::pair< bool, Real > intersect = ray.intersects(plane);

			// we got an intersection point
			if (intersect.first == true)
			{
				return ray.getPoint(intersect.second);
			}
			else
			{
				// failure!
				return Vector3(0.0, 0.0, 0.0);
			}
		}
	}
	//-----------------------------------------------------------------------
	Matrix4 LiSPSMShadowCameraSetup::buildFrustumProjection(Real left, Real right, 
		Real bottom, Real top, Real nearf, Real farf) const
	{
		// Changed to nearf because windows defines near and far as a macros
		Real m00 = 2 * nearf / (right - left),
			m02 = (right + left) / (right - left),
			m11 = 2 * nearf / (top - bottom),
			m12 = (top + bottom) / (top - bottom),
			m22 = -(farf + nearf) / (farf - nearf),
			m23 = -2 * farf * nearf / (farf - nearf),
			m32 = -1;

		Matrix4 m(m00, 0.0, m02, 0.0,
			0.0, m11, m12, 0.0,
			0.0, 0.0, m22, m23,
			0.0, 0.0, m32, 0.0);

		return m;
	}
	//-----------------------------------------------------------------------
	void LiSPSMShadowCameraSetup::getShadowCamera (const SceneManager *sm, const Camera *cam, 
		const Viewport *vp, const Light *light, Camera *texCam, size_t iteration) const
	{
		// check availability - viewport not needed
		OgreAssert(sm != NULL, "SceneManager is NULL");
		OgreAssert(cam != NULL, "Camera (viewer) is NULL");
		OgreAssert(light != NULL, "Light is NULL");
		OgreAssert(texCam != NULL, "Camera (texture) is NULL");
		mLightFrustumCameraCalculated = false;


		// calculate standard shadow mapping matrix
		Matrix4 LView, LProj;
		calculateShadowMappingMatrix(*sm, *cam, *light, &LView, &LProj, NULL);
		
		// if the direction of the light and the direction of the camera tend to be parallel,
		// then tweak up the adjust factor
		Real dot = Math::Abs(cam->getDerivedDirection().dotProduct(light->getDerivedDirection()));
		if (dot >= mCosCamLightDirThreshold)
		{
			mOptAdjustFactorTweak = 1.0 + (20 * ((dot - mCosCamLightDirThreshold) / (1.0 - mCosCamLightDirThreshold)) );
		}
		else
		{
			mOptAdjustFactorTweak = 1.0;
		}

		// build scene bounding box
		const VisibleObjectsBoundsInfo& visInfo = sm->getVisibleObjectsBoundsInfo(texCam);
		AxisAlignedBox sceneBB = visInfo.aabb;
		sceneBB.merge(sm->getVisibleObjectsBoundsInfo(cam).receiverAabb);
		sceneBB.merge(cam->getDerivedPosition());

		// in case the sceneBB is empty (e.g. nothing visible to the cam) simply
		// return the standard shadow mapping matrix
		if (sceneBB.isNull())
		{
			texCam->setCustomViewMatrix(true, LView);
			texCam->setCustomProjectionMatrix(true, LProj);
			return;
		}

		// calculate the intersection body B
		mPointListBodyB.reset();
		calculateB(*sm, *cam, *light, sceneBB, &mPointListBodyB);

		// in case the bodyB is empty (e.g. nothing visible to the light or the cam)
		// simply return the standard shadow mapping matrix
		if (mPointListBodyB.getPointCount() == 0)
		{
			texCam->setCustomViewMatrix(true, LView);
			texCam->setCustomProjectionMatrix(true, LProj);
			return;
		}

		// transform to light space: y -> -z, z -> y
		LProj = msNormalToLightSpace * LProj;

		// calculate LVS so it does not need to be calculated twice
		// calculate the body L \cap V \cap S to make sure all returned points are in 
		// front of the camera
		calculateLVS(*sm, *cam, *light, sceneBB, &mPointListBodyLVS);

		// fetch the viewing direction
		const Vector3 viewDir = getLSProjViewDir(LProj * LView, *cam, mPointListBodyLVS);

		// The light space will be rotated in such a way, that the projected light view 
		// always points upwards, so the up-vector is the y-axis (we already prepared the
		// light space for this usage).The transformation matrix is set up with the
		// following parameters:
		// - position is the origin
		// - the view direction is the calculated viewDir
		// - the up vector is the y-axis
		LProj = buildViewMatrix(Vector3::ZERO, viewDir, Vector3::UNIT_Y) * LProj;

		// calculate LiSPSM projection
		LProj = calculateLiSPSM(LProj * LView, mPointListBodyB, mPointListBodyLVS, *sm, *cam, *light) * LProj;

		// map bodyB to unit cube
		LProj = transformToUnitCube(LProj * LView, mPointListBodyB) * LProj;

		// transform from light space to normal space: y -> z, z -> -y
		LProj = msLightSpaceToNormal * LProj;

		// LView = Lv^-1
		// LProj = Switch_{-ls} * FocusBody * P * L_r * Switch_{ls} * L_p
		texCam->setCustomViewMatrix(true, LView);
		texCam->setCustomProjectionMatrix(true, LProj);
	}
	//---------------------------------------------------------------------
	void LiSPSMShadowCameraSetup::setCameraLightDirectionThreshold(Degree angle)
	{
		mCosCamLightDirThreshold = Math::Cos(angle.valueRadians());
	}
	//---------------------------------------------------------------------
	Degree LiSPSMShadowCameraSetup::getCameraLightDirectionThreshold() const
	{
		return Degree(Math::ACos(mCosCamLightDirThreshold));
	}


}

