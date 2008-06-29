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
#include "OgreShadowCameraSetupFocused.h"
#include "OgreRoot.h"
#include "OgreSceneManager.h"
#include "OgreCamera.h"
#include "OgreLight.h"
#include "OgrePlane.h"
#include "OgreLogManager.h"


namespace Ogre
{
	/** transform from normal to light space */
	const Matrix4 FocusedShadowCameraSetup::msNormalToLightSpace(
		1,  0,  0,  0,		// x
		0,  0, -1,  0,		// y
		0,  1,  0,  0,		// z
		0,  0,  0,  1);	// w
	/** transform  from light to normal space */
	const Matrix4 FocusedShadowCameraSetup::msLightSpaceToNormal(
		1,  0,  0,  0,		// x
		0,  0,  1,  0,		// y
		0, -1,  0,  0,		// z
		0,  0,  0,  1);	// w

	FocusedShadowCameraSetup::FocusedShadowCameraSetup(void)
		: mTempFrustum(OGRE_NEW Frustum())
		, mLightFrustumCamera(OGRE_NEW Camera("TEMP LIGHT INTERSECT CAM", NULL))
		, mLightFrustumCameraCalculated(false)
		, mUseAggressiveRegion(true)
	{
		mTempFrustum->setProjectionType(PT_PERSPECTIVE);
	}
	//-----------------------------------------------------------------------
	FocusedShadowCameraSetup::~FocusedShadowCameraSetup(void)
	{
		OGRE_DELETE mTempFrustum;
		OGRE_DELETE mLightFrustumCamera;
	}
	//-----------------------------------------------------------------------
	void FocusedShadowCameraSetup::calculateShadowMappingMatrix(const SceneManager& sm,
		const Camera& cam, const Light& light, Matrix4 *out_view, Matrix4 *out_proj, 
		Camera *out_cam) const
	{
		const Vector3& camDir = cam.getDerivedDirection();

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
					-light.getDerivedDirection(), 
					cam.getDerivedUp());
			}

			// generate projection matrix if requested
			if (out_proj != NULL)
			{
				*out_proj = Matrix4::IDENTITY;
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
			// target analogue to the default shadow textures
			// Calculate look at position
			// We want to look at a spot shadowOffset away from near plane
			// 0.5 is a little too close for angles
			Vector3 target = cam.getDerivedPosition() + 
				(cam.getDerivedDirection() * shadowOffset);
			Vector3 lightDir = target - light.getDerivedPosition();
			lightDir.normalise();

			// generate view matrix if requested
			if (out_view != NULL)
			{
				*out_view = buildViewMatrix(light.getDerivedPosition(), 
					lightDir, 
					cam.getDerivedUp());
			}

			// generate projection matrix if requested
			if (out_proj != NULL)
			{
				// set FOV to 120 degrees
				mTempFrustum->setFOVy(Degree(120));

				// set near clip distance like the camera
				mTempFrustum->setNearClipDistance(cam.getNearClipDistance());

				*out_proj = mTempFrustum->getProjectionMatrix();
			}

			// set up camera if requested
			if (out_cam != NULL)
			{
				out_cam->setProjectionType(PT_PERSPECTIVE);
				out_cam->setDirection(lightDir);
				out_cam->setPosition(light.getDerivedPosition());
				out_cam->setFOVy(Degree(120));
				out_cam->setNearClipDistance(cam.getNearClipDistance());
			}
		}
		else if (light.getType() == Light::LT_SPOTLIGHT)
		{
			// generate view matrix if requested
			if (out_view != NULL)
			{
				*out_view = buildViewMatrix(light.getDerivedPosition(), 
					light.getDerivedDirection(), 
					cam.getDerivedUp());
			}

			// generate projection matrix if requested
			if (out_proj != NULL)
			{
				// set FOV slightly larger than spotlight range
				mTempFrustum->setFOVy(light.getSpotlightOuterAngle() * 1.2);

				// set near clip distance like the camera
				mTempFrustum->setNearClipDistance(cam.getNearClipDistance());

				*out_proj = mTempFrustum->getProjectionMatrix();
			}

			// set up camera if requested
			if (out_cam != NULL)
			{
				out_cam->setProjectionType(PT_PERSPECTIVE);
				out_cam->setDirection(light.getDerivedDirection());
				out_cam->setPosition(light.getDerivedPosition());
				out_cam->setFOVy(light.getSpotlightOuterAngle() * 1.2);
				out_cam->setNearClipDistance(cam.getNearClipDistance());
			}
		}
	}
	//-----------------------------------------------------------------------
	void FocusedShadowCameraSetup::calculateB(const SceneManager& sm, const Camera& cam, 
		const Light& light, const AxisAlignedBox& sceneBB, PointListBody *out_bodyB) const
	{
		OgreAssert(out_bodyB != NULL, "bodyB vertex list is NULL");

		/// perform convex intersection of the form B = ((V \cap S) + l) \cap S \cap L

		// get V
		mBodyB.define(cam);

		if (light.getType() != Light::LT_DIRECTIONAL)
		{
			// clip bodyB with sceneBB
			/* Note, Matthias' original code states this:
			"The procedure ((V \cap S) + l) \cap S \cap L (Wimmer et al.) leads in some 
			cases to disappearing shadows. Valid parts of the scene are clipped, so shadows 
			are partly incomplete. The cause may be the	transformation into light space that 
			is only done for the corner points which may not contain the whole scene afterwards 
			any more. So we fall back to the method of Stamminger et al. (V + l) \cap S \cap L 
			which does not show these anomalies."

			.. leading to the commenting out of the below clip. However, ift makes a major
			difference to the quality of the focus, and so far I haven't noticed
			the clipping issues described. Intuitively I would have thought that
			any clipping issue would be due to the findCastersForLight not being
			quite right, since if the sceneBB includes those there is no reason for
			this clip instruction to omit a genuine shadow caster.

			I have made this a user option since the quality effect is major and
			the clipping problem only seems to occur in some specific cases. 
			*/
			if (mUseAggressiveRegion)
				mBodyB.clip(sceneBB);

			// form a convex hull of bodyB with the light position
			mBodyB.extend(light.getDerivedPosition());

			// clip bodyB with sceneBB
			mBodyB.clip(sceneBB);

			// clip with the light frustum
			// set up light camera to clip with the resulting frustum planes
			if (!mLightFrustumCameraCalculated)
			{
				calculateShadowMappingMatrix(sm, cam, light, NULL, NULL, mLightFrustumCamera);
				mLightFrustumCameraCalculated = true;
			}
			mBodyB.clip(*mLightFrustumCamera);

			// extract bodyB vertices
			out_bodyB->build(mBodyB);

		}
		else
		{
			// clip bodyB with sceneBB
			mBodyB.clip(sceneBB);

			// Also clip based on shadow far distance if appropriate
			Real farDist = light.getShadowFarDistance();
			if (farDist)
			{
				Vector3 pointOnPlane = cam.getDerivedPosition() + 
					(cam.getDerivedDirection() * farDist);
				Plane p(cam.getDerivedDirection(), pointOnPlane);
				mBodyB.clip(p);
			}

			// Extrude the intersection bodyB into the inverted light direction and store 
			// the info in the point list.
			// The sceneBB holds the maximum extent of the extrusion.
			out_bodyB->buildAndIncludeDirection(mBodyB, 
				sceneBB, 
				-light.getDerivedDirection());
		}
	}

	//-----------------------------------------------------------------------
	void FocusedShadowCameraSetup::calculateLVS(const SceneManager& sm, const Camera& cam, 
		const Light& light, const AxisAlignedBox& sceneBB, PointListBody *out_LVS) const
	{
		ConvexBody bodyLVS;

		// init body with view frustum
		bodyLVS.define(cam);

		// clip the body with the light frustum (point + spot)
		// for a directional light the space of the intersected
		// view frustum and sceneBB is always lighted and in front
		// of the viewer.
		if (light.getType() != Light::LT_DIRECTIONAL)
		{
			// clip with the light frustum
			// set up light camera to clip the resulting frustum
			if (!mLightFrustumCameraCalculated)
			{
				calculateShadowMappingMatrix(sm, cam, light, NULL, NULL, mLightFrustumCamera);
				mLightFrustumCameraCalculated = true;
			}
			bodyLVS.clip(*mLightFrustumCamera);
		}

		// clip the body with the scene bounding box
		bodyLVS.clip(sceneBB);

		// extract bodyLVS vertices
		out_LVS->build(bodyLVS);
	}
	//-----------------------------------------------------------------------
	Vector3 FocusedShadowCameraSetup::getLSProjViewDir(const Matrix4& lightSpace, 
		const Camera& cam, const PointListBody& bodyLVS) const
	{
		// goal is to construct a view direction
		// because parallel lines are not parallel any more after perspective projection we have to transform
		// a ray to point us the viewing direction

		// fetch a point near the camera
		const Vector3 e_world = getNearCameraPoint_ws(cam.getViewMatrix(), bodyLVS);

		// plus the direction results in a second point
		const Vector3 b_world = e_world + cam.getDerivedDirection();

		// transformation into light space
		const Vector3 e_ls = lightSpace * e_world;
		const Vector3 b_ls = lightSpace * b_world;

		// calculate the projection direction, which is the subtraction of
		// b_ls from e_ls. The y component is set to 0 to project the view
		// direction into the shadow map plane.
		Vector3 projectionDir(b_ls - e_ls);
		projectionDir.y = 0;

		// deal with Y-only vectors
		return Math::RealEqual(projectionDir.length(), 0.0) ? 
			Vector3::NEGATIVE_UNIT_Z : projectionDir.normalisedCopy();
	}
	//-----------------------------------------------------------------------
	Vector3 FocusedShadowCameraSetup::getNearCameraPoint_ws(const Matrix4& viewMatrix, 
		const PointListBody& bodyLVS) const
	{
		if (bodyLVS.getPointCount() == 0)
			return Vector3(0,0,0);

		Vector3 nearEye = viewMatrix * bodyLVS.getPoint(0),	// for comparison
			nearWorld = bodyLVS.getPoint(0);				// represents the final point

		// store the vertex with the highest z-value which is the nearest point
		for (size_t i = 1; i < bodyLVS.getPointCount(); ++i)
		{
			const Vector3& vWorld = bodyLVS.getPoint(i);

			// comparison is done from the viewer
			Vector3 vEye = viewMatrix * vWorld;

			if (vEye.z > nearEye.z)
			{
				nearEye		= vEye;
				nearWorld	= vWorld;
			}
		}

		return nearWorld;
	}
	//-----------------------------------------------------------------------
	Matrix4 FocusedShadowCameraSetup::transformToUnitCube(const Matrix4& m, 
		const PointListBody& body) const
	{
		// map the transformed body AAB points to the unit cube (-1/-1/-1) / (+1/+1/+1) corners
		AxisAlignedBox aab_trans;

		for (size_t i = 0; i < body.getPointCount(); ++i)
		{
			aab_trans.merge(m * body.getPoint(i));
		}

		Vector3 vMin, vMax;

		vMin = aab_trans.getMinimum();
		vMax = aab_trans.getMaximum();

		const Vector3 trans(-(vMax.x + vMin.x) / (vMax.x - vMin.x),
			-(vMax.y + vMin.y) / (vMax.y - vMin.y),
			-(vMax.z + vMin.z) / (vMax.z - vMin.z));

		const Vector3 scale(2 / (vMax.x - vMin.x),
			2 / (vMax.y - vMin.y),
			2 / (vMax.z - vMin.z));

		Matrix4 mOut(Matrix4::IDENTITY);
		mOut.setTrans(trans);
		mOut.setScale(scale);

		return mOut;
	}
	//-----------------------------------------------------------------------
	Matrix4 FocusedShadowCameraSetup::buildViewMatrix(const Vector3& pos, const Vector3& dir, 
		const Vector3& up) const
	{
		Vector3 xN = dir.crossProduct(up);
		xN.normalise();
		Vector3 upN = xN.crossProduct(dir);
		upN.normalise();

		Matrix4 m(xN.x,		xN.y,		xN.z,		-xN.dotProduct(pos),
			upN.x,		upN.y,		upN.z,		-upN.dotProduct(pos),
			-dir.x,		-dir.y,	-dir.z,	dir.dotProduct(pos),
			0.0,			0.0,		0.0,		1.0
			);

		return m;
	}
	//-----------------------------------------------------------------------
	void FocusedShadowCameraSetup::getShadowCamera (const SceneManager *sm, const Camera *cam, 
		const Viewport *vp, const Light *light, Camera *texCam) const
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

		// build scene bounding box
		const VisibleObjectsBoundsInfo& visInfo = sm->getShadowCasterBoundsInfo(light);
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
		mPointListBodyLVS.reset();
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

		// map bodyB to unit cube
		LProj = transformToUnitCube(LProj * LView, mPointListBodyB) * LProj;

		// transform from light space to normal space: y -> z, z -> -y
		LProj = msLightSpaceToNormal * LProj;

		// set the two custom matrices
		texCam->setCustomViewMatrix(true, LView);
		texCam->setCustomProjectionMatrix(true, LProj);
	}

	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//-----------------------------------------------------------------------
	FocusedShadowCameraSetup::PointListBody::PointListBody()
	{
		// Preallocate some space
		mBodyPoints.reserve(12);
	}
	//-----------------------------------------------------------------------
	FocusedShadowCameraSetup::PointListBody::PointListBody(const ConvexBody& body)
	{
		build(body);
	}
	//-----------------------------------------------------------------------
	FocusedShadowCameraSetup::PointListBody::~PointListBody()
	{
	}
	//-----------------------------------------------------------------------
	void FocusedShadowCameraSetup::PointListBody::merge(const PointListBody& plb)
	{
		size_t size = plb.getPointCount();
		for (size_t i = 0; i < size; ++i)
		{
			this->addPoint(plb.getPoint(i));
		}
	}
	//-----------------------------------------------------------------------
	void FocusedShadowCameraSetup::PointListBody::build(const ConvexBody& body, bool filterDuplicates)
	{
		// erase list
		mBodyPoints.clear();

		// Try to reserve a representative amount of memory
		mBodyPoints.reserve(body.getPolygonCount() * 6);

		// build new list
		for (size_t i = 0; i < body.getPolygonCount(); ++i)
		{
			for (size_t j = 0; j < body.getVertexCount(i); ++j)
			{
				const Vector3 &vInsert = body.getVertex(i, j);

				// duplicates allowed?
				if (filterDuplicates)
				{
					bool bPresent = false;

					for(Polygon::VertexList::iterator vit = mBodyPoints.begin();
						vit != mBodyPoints.end(); ++vit)
					{
						const Vector3& v = *vit;

						if (vInsert.positionEquals(v))
						{
							bPresent = true;
							break;
						}
					}

					if (bPresent == false)
					{
						mBodyPoints.push_back(body.getVertex(i, j));
					}
				}

				// else insert directly
				else
				{
					mBodyPoints.push_back(body.getVertex(i, j));
				}
			}
		}

		// update AAB
		// no points altered, so take body AAB
		mAAB = body.getAABB();
	}
	//-----------------------------------------------------------------------
	void FocusedShadowCameraSetup::PointListBody::buildAndIncludeDirection(
		const ConvexBody& body, const AxisAlignedBox& aabMax, const Vector3& dir)
	{
		// reset point list
		this->reset();

		// intersect the rays formed by the points in the list with the given direction and
		// insert them into the list

		// min/max aab points for comparison
		const Vector3& min = aabMax.getMinimum();
		const Vector3& max = aabMax.getMaximum();

		// assemble the clipping planes
		Plane pl[6];

		// front
		pl[0].redefine(Vector3::UNIT_Z, max);
		// back
		pl[1].redefine(Vector3::NEGATIVE_UNIT_Z, min);
		// left
		pl[2].redefine(Vector3::NEGATIVE_UNIT_X, min);
		// right
		pl[3].redefine(Vector3::UNIT_X, max);
		// bottom
		pl[4].redefine(Vector3::NEGATIVE_UNIT_Y, min);
		// top
		pl[5].redefine(Vector3::UNIT_Y, max);


		const size_t polyCount = body.getPolygonCount();
		for (size_t iPoly = 0; iPoly < polyCount; ++iPoly)
		{

			// store the old inserted point and plane info
			// if the currently processed point hits a different plane than the previous point an 
			// intersection point is calculated that lies on the two planes' intersection edge

			// fetch current polygon
			const Polygon& p = body.getPolygon(iPoly);

			size_t pointCount = p.getVertexCount();
			for (size_t iPoint = 0; iPoint < pointCount ; ++iPoint)
			{
				// base point
				const Vector3& pt = p.getVertex(iPoint);

				// add the base point
				this->addPoint(pt);

				// intersection ray
				Ray ray(pt, dir);

				// intersect with each plane
				for (size_t iPlane = 0; iPlane < 6; ++iPlane)
				{
					std::pair< bool, Real > intersect = ray.intersects(pl[ iPlane ]);

					const Vector3 ptIntersect = ray.getPoint(intersect.second);

					// intersection point must exist (first) and the point distance must be greater than null (second)
					// in case of distance null the intersection point equals the base point
					if (intersect.first && intersect.second > 0)
					{
						if (ptIntersect.x < max.x + 1e-3f && ptIntersect.x > min.x - 1e-3f && 
							ptIntersect.y < max.y + 1e-3f && ptIntersect.y > min.y - 1e-3f && 
							ptIntersect.z < max.z + 1e-3f && ptIntersect.z > min.z - 1e-3f)
						{
							// in case the point lies on the boundary, continue and see if there is another plane that intersects
							if (pt.positionEquals(ptIntersect))
							{
								continue;
							}

							// add intersection point
							this->addPoint(ptIntersect);
						}

					} // if: intersection available

				} // for: plane intersection

			} // for: polygon point iteration

		} // for: polygon iteration
	}
	//-----------------------------------------------------------------------
	const AxisAlignedBox& FocusedShadowCameraSetup::PointListBody::getAAB(void) const
	{
		return mAAB;
	}
	//-----------------------------------------------------------------------	
	void FocusedShadowCameraSetup::PointListBody::addPoint(const Vector3& point)
	{
		// dont check for doubles, simply add
		mBodyPoints.push_back(point);

		// update AAB
		mAAB.merge(point);
	}
	//-----------------------------------------------------------------------
	void FocusedShadowCameraSetup::PointListBody::addAAB(const AxisAlignedBox& aab)
	{
		const Vector3& min = aab.getMinimum();
		const Vector3& max = aab.getMaximum();

		Vector3 currentVertex = min;
		// min min min
		addPoint(currentVertex);

		// min min max
		currentVertex.z = max.z;
		addPoint(currentVertex);

		// min max max
		currentVertex.y = max.y;
		addPoint(currentVertex);

		// min max min
		currentVertex.z = min.z;
		addPoint(currentVertex);

		// max max min
		currentVertex.x = max.x;
		addPoint(currentVertex);

		// max max max
		currentVertex.z = max.z;
		addPoint(currentVertex);

		// max min max
		currentVertex.y = min.y;
		addPoint(currentVertex);

		// max min min
		currentVertex.z = min.z;
		addPoint(currentVertex); 

	}
	//-----------------------------------------------------------------------	
	const Vector3& FocusedShadowCameraSetup::PointListBody::getPoint(size_t cnt) const
	{
		OgreAssert(cnt >= 0 && cnt < getPointCount(), "Search position out of range");

		return mBodyPoints[ cnt ];
	}
	//-----------------------------------------------------------------------	
	size_t FocusedShadowCameraSetup::PointListBody::getPointCount(void) const
	{
		return mBodyPoints.size();
	}
	//-----------------------------------------------------------------------	
	void FocusedShadowCameraSetup::PointListBody::reset(void)
	{
		mBodyPoints.clear();
		mAAB.setNull();
	}

}


