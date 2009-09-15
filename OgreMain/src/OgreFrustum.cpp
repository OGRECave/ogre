/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#include "OgreFrustum.h"

#include "OgreMath.h"
#include "OgreMatrix3.h"
#include "OgreSceneNode.h"
#include "OgreSphere.h"
#include "OgreLogManager.h"
#include "OgreException.h"
#include "OgreRoot.h"
#include "OgreCamera.h"
#include "OgreHardwareBufferManager.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreHardwareIndexBuffer.h"
#include "OgreMaterialManager.h"
#include "OgreRenderSystem.h"

namespace Ogre {

    String Frustum::msMovableType = "Frustum";
    const Real Frustum::INFINITE_FAR_PLANE_ADJUST = 0.00001;
    //-----------------------------------------------------------------------
    Frustum::Frustum() : 
        mProjType(PT_PERSPECTIVE), 
        mFOVy(Radian(Math::PI/4.0)), 
        mFarDist(100000.0f), 
        mNearDist(100.0f), 
        mAspect(1.33333333333333f), 
		mOrthoHeight(1000),
        mFrustumOffset(Vector2::ZERO),
        mFocalLength(1.0f),
        mLastParentOrientation(Quaternion::IDENTITY),
        mLastParentPosition(Vector3::ZERO),
        mRecalcFrustum(true), 
        mRecalcView(true), 
        mRecalcFrustumPlanes(true),
        mRecalcWorldSpaceCorners(true),
        mRecalcVertexData(true),
		mCustomViewMatrix(false),
		mCustomProjMatrix(false),
		mFrustumExtentsManuallySet(false),
        mReflect(false), 
        mLinkedReflectPlane(0),
        mObliqueDepthProjection(false), 
        mLinkedObliqueProjPlane(0)
    {
        // Initialise material
        mMaterial = MaterialManager::getSingleton().getByName("BaseWhiteNoLighting");
        
        // Alter superclass members
        mVisible = false;
        mParentNode = 0;

        mLastLinkedReflectionPlane.normal = Vector3::ZERO;
        mLastLinkedObliqueProjPlane.normal = Vector3::ZERO;


        updateView();
        updateFrustum();
    }

    //-----------------------------------------------------------------------
    Frustum::~Frustum()
    {
        // Do nothing
    }

    //-----------------------------------------------------------------------
    void Frustum::setFOVy(const Radian& fov)
    {
        mFOVy = fov;
        invalidateFrustum();
    }

    //-----------------------------------------------------------------------
    const Radian& Frustum::getFOVy(void) const
    {
        return mFOVy;
    }


    //-----------------------------------------------------------------------
    void Frustum::setFarClipDistance(Real farPlane)
    {
        mFarDist = farPlane;
        invalidateFrustum();
    }

    //-----------------------------------------------------------------------
    Real Frustum::getFarClipDistance(void) const
    {
        return mFarDist;
    }

    //-----------------------------------------------------------------------
    void Frustum::setNearClipDistance(Real nearPlane)
    {
        if (nearPlane <= 0)
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Near clip distance must be greater than zero.",
                "Frustum::setNearClipDistance");
        mNearDist = nearPlane;
        invalidateFrustum();
    }

    //-----------------------------------------------------------------------
    Real Frustum::getNearClipDistance(void) const
    {
        return mNearDist;
    }

    //---------------------------------------------------------------------
    void Frustum::setFrustumOffset(const Vector2& offset)
    {
        mFrustumOffset = offset;
        invalidateFrustum();
    }
    //---------------------------------------------------------------------
    void Frustum::setFrustumOffset(Real horizontal, Real vertical)
    {
        setFrustumOffset(Vector2(horizontal, vertical));
    }
    //---------------------------------------------------------------------
    const Vector2& Frustum::getFrustumOffset() const
    {
        return mFrustumOffset;
    }
    //---------------------------------------------------------------------
    void Frustum::setFocalLength(Real focalLength)
    {
        if (focalLength <= 0)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "Focal length must be greater than zero.",
                "Frustum::setFocalLength");
        }

        mFocalLength = focalLength;
        invalidateFrustum();
    }
    //---------------------------------------------------------------------
    Real Frustum::getFocalLength() const
    {
        return mFocalLength;
    }
    //-----------------------------------------------------------------------
    const Matrix4& Frustum::getProjectionMatrix(void) const
    {

        updateFrustum();

        return mProjMatrix;
    }
    //-----------------------------------------------------------------------
    const Matrix4& Frustum::getProjectionMatrixWithRSDepth(void) const
    {

        updateFrustum();

        return mProjMatrixRSDepth;
    }
    //-----------------------------------------------------------------------
    const Matrix4& Frustum::getProjectionMatrixRS(void) const
    {

        updateFrustum();

        return mProjMatrixRS;
    }
    //-----------------------------------------------------------------------
    const Matrix4& Frustum::getViewMatrix(void) const
    {
        updateView();

        return mViewMatrix;

    }

    //-----------------------------------------------------------------------
    const Plane* Frustum::getFrustumPlanes(void) const
    {
        // Make any pending updates to the calculated frustum planes
        updateFrustumPlanes();

        return mFrustumPlanes;
    }

    //-----------------------------------------------------------------------
    const Plane& Frustum::getFrustumPlane(unsigned short plane) const
    {
        // Make any pending updates to the calculated frustum planes
        updateFrustumPlanes();

        return mFrustumPlanes[plane];

    }

    //-----------------------------------------------------------------------
    bool Frustum::isVisible(const AxisAlignedBox& bound, FrustumPlane* culledBy) const
    {
        // Null boxes always invisible
        if (bound.isNull()) return false;

        // Infinite boxes always visible
        if (bound.isInfinite()) return true;

        // Make any pending updates to the calculated frustum planes
        updateFrustumPlanes();

        // Get centre of the box
        Vector3 centre = bound.getCenter();
        // Get the half-size of the box
        Vector3 halfSize = bound.getHalfSize();

        // For each plane, see if all points are on the negative side
        // If so, object is not visible
        for (int plane = 0; plane < 6; ++plane)
        {
            // Skip far plane if infinite view frustum
            if (plane == FRUSTUM_PLANE_FAR && mFarDist == 0)
                continue;

            Plane::Side side = mFrustumPlanes[plane].getSide(centre, halfSize);
            if (side == Plane::NEGATIVE_SIDE)
            {
                // ALL corners on negative side therefore out of view
                if (culledBy)
                    *culledBy = (FrustumPlane)plane;
                return false;
            }

        }

        return true;
    }

    //-----------------------------------------------------------------------
    bool Frustum::isVisible(const Vector3& vert, FrustumPlane* culledBy) const
    {
        // Make any pending updates to the calculated frustum planes
        updateFrustumPlanes();

        // For each plane, see if all points are on the negative side
        // If so, object is not visible
        for (int plane = 0; plane < 6; ++plane)
        {
            // Skip far plane if infinite view frustum
            if (plane == FRUSTUM_PLANE_FAR && mFarDist == 0)
                continue;

            if (mFrustumPlanes[plane].getSide(vert) == Plane::NEGATIVE_SIDE)
            {
                // ALL corners on negative side therefore out of view
                if (culledBy)
                    *culledBy = (FrustumPlane)plane;
                return false;
            }

        }

        return true;
    }
    //-----------------------------------------------------------------------
    bool Frustum::isVisible(const Sphere& sphere, FrustumPlane* culledBy) const
    {
        // Make any pending updates to the calculated frustum planes
        updateFrustumPlanes();

        // For each plane, see if sphere is on negative side
        // If so, object is not visible
        for (int plane = 0; plane < 6; ++plane)
        {
            // Skip far plane if infinite view frustum
            if (plane == FRUSTUM_PLANE_FAR && mFarDist == 0)
                continue;

            // If the distance from sphere center to plane is negative, and 'more negative' 
            // than the radius of the sphere, sphere is outside frustum
            if (mFrustumPlanes[plane].getDistance(sphere.getCenter()) < -sphere.getRadius())
            {
                // ALL corners on negative side therefore out of view
                if (culledBy)
                    *culledBy = (FrustumPlane)plane;
                return false;
            }

        }

        return true;
    }
	//---------------------------------------------------------------------
	uint32 Frustum::getTypeFlags(void) const
	{
		return SceneManager::FRUSTUM_TYPE_MASK;
	}
    //-----------------------------------------------------------------------
    void Frustum::calcProjectionParameters(Real& left, Real& right, Real& bottom, Real& top) const
    { 
		if (mCustomProjMatrix)
		{
			// Convert clipspace corners to camera space
			Matrix4 invProj = mProjMatrix.inverse();
			Vector3 topLeft(-0.5f, 0.5f, 0.0f);
			Vector3 bottomRight(0.5f, -0.5f, 0.0f);

			topLeft = invProj * topLeft;
			bottomRight = invProj * bottomRight;

			left = topLeft.x;
			top = topLeft.y;
			right = bottomRight.x;
			bottom = bottomRight.y;

		}
		else
		{
			if (mFrustumExtentsManuallySet)
			{
				left = mLeft;
				right = mRight;
				top = mTop;
				bottom = mBottom;
			}
			// Calculate general projection parameters
			else if (mProjType == PT_PERSPECTIVE)
			{
				Radian thetaY (mFOVy * 0.5f);
				Real tanThetaY = Math::Tan(thetaY);
				Real tanThetaX = tanThetaY * mAspect;

				Real nearFocal = mNearDist / mFocalLength;
				Real nearOffsetX = mFrustumOffset.x * nearFocal;
				Real nearOffsetY = mFrustumOffset.y * nearFocal;
				Real half_w = tanThetaX * mNearDist;
				Real half_h = tanThetaY * mNearDist;

				left   = - half_w + nearOffsetX;
				right  = + half_w + nearOffsetX;
				bottom = - half_h + nearOffsetY;
				top    = + half_h + nearOffsetY;

				mLeft = left;
				mRight = right;
				mTop = top;
				mBottom = bottom;
			}
			else
			{
				// Unknown how to apply frustum offset to orthographic camera, just ignore here
				Real half_w = getOrthoWindowWidth() * 0.5;
				Real half_h = getOrthoWindowHeight() * 0.5;

				left   = - half_w;
				right  = + half_w;
				bottom = - half_h;
				top    = + half_h;

				mLeft = left;
				mRight = right;
				mTop = top;
				mBottom = bottom;
			}

		}
    }
	//-----------------------------------------------------------------------
	void Frustum::updateFrustumImpl(void) const
	{
		// Common calcs
		Real left, right, bottom, top;
		calcProjectionParameters(left, right, bottom, top);

		if (!mCustomProjMatrix)
		{

			// The code below will dealing with general projection 
			// parameters, similar glFrustum and glOrtho.
			// Doesn't optimise manually except division operator, so the 
			// code more self-explaining.

			Real inv_w = 1 / (right - left);
			Real inv_h = 1 / (top - bottom);
			Real inv_d = 1 / (mFarDist - mNearDist);

			// Recalc if frustum params changed
			if (mProjType == PT_PERSPECTIVE)
			{
				// Calc matrix elements
				Real A = 2 * mNearDist * inv_w;
				Real B = 2 * mNearDist * inv_h;
				Real C = (right + left) * inv_w;
				Real D = (top + bottom) * inv_h;
				Real q, qn;
				if (mFarDist == 0)
				{
					// Infinite far plane
					q = Frustum::INFINITE_FAR_PLANE_ADJUST - 1;
					qn = mNearDist * (Frustum::INFINITE_FAR_PLANE_ADJUST - 2);
				}
				else
				{
					q = - (mFarDist + mNearDist) * inv_d;
					qn = -2 * (mFarDist * mNearDist) * inv_d;
				}

				// NB: This creates 'uniform' perspective projection matrix,
				// which depth range [-1,1], right-handed rules
				//
				// [ A   0   C   0  ]
				// [ 0   B   D   0  ]
				// [ 0   0   q   qn ]
				// [ 0   0   -1  0  ]
				//
				// A = 2 * near / (right - left)
				// B = 2 * near / (top - bottom)
				// C = (right + left) / (right - left)
				// D = (top + bottom) / (top - bottom)
				// q = - (far + near) / (far - near)
				// qn = - 2 * (far * near) / (far - near)

				mProjMatrix = Matrix4::ZERO;
				mProjMatrix[0][0] = A;
				mProjMatrix[0][2] = C;
				mProjMatrix[1][1] = B;
				mProjMatrix[1][2] = D;
				mProjMatrix[2][2] = q;
				mProjMatrix[2][3] = qn;
				mProjMatrix[3][2] = -1;

				if (mObliqueDepthProjection)
				{
					// Translate the plane into view space

					// Don't use getViewMatrix here, incase overrided by 
					// camera and return a cull frustum view matrix
					updateView();
					Plane plane = mViewMatrix * mObliqueProjPlane;

					// Thanks to Eric Lenyel for posting this calculation 
					// at www.terathon.com

					// Calculate the clip-space corner point opposite the 
					// clipping plane
					// as (sgn(clipPlane.x), sgn(clipPlane.y), 1, 1) and
					// transform it into camera space by multiplying it
					// by the inverse of the projection matrix

					/* generalised version
					Vector4 q = matrix.inverse() * 
					Vector4(Math::Sign(plane.normal.x), 
					Math::Sign(plane.normal.y), 1.0f, 1.0f);
					*/
					Vector4 q;
					q.x = (Math::Sign(plane.normal.x) + mProjMatrix[0][2]) / mProjMatrix[0][0];
					q.y = (Math::Sign(plane.normal.y) + mProjMatrix[1][2]) / mProjMatrix[1][1];
					q.z = -1;
					q.w = (1 + mProjMatrix[2][2]) / mProjMatrix[2][3];

					// Calculate the scaled plane vector
					Vector4 clipPlane4d(plane.normal.x, plane.normal.y, plane.normal.z, plane.d);
					Vector4 c = clipPlane4d * (2 / (clipPlane4d.dotProduct(q)));

					// Replace the third row of the projection matrix
					mProjMatrix[2][0] = c.x;
					mProjMatrix[2][1] = c.y;
					mProjMatrix[2][2] = c.z + 1;
					mProjMatrix[2][3] = c.w; 
				}
			} // perspective
			else if (mProjType == PT_ORTHOGRAPHIC)
			{
				Real A = 2 * inv_w;
				Real B = 2 * inv_h;
				Real C = - (right + left) * inv_w;
				Real D = - (top + bottom) * inv_h;
				Real q, qn;
				if (mFarDist == 0)
				{
					// Can not do infinite far plane here, avoid divided zero only
					q = - Frustum::INFINITE_FAR_PLANE_ADJUST / mNearDist;
					qn = - Frustum::INFINITE_FAR_PLANE_ADJUST - 1;
				}
				else
				{
					q = - 2 * inv_d;
					qn = - (mFarDist + mNearDist)  * inv_d;
				}

				// NB: This creates 'uniform' orthographic projection matrix,
				// which depth range [-1,1], right-handed rules
				//
				// [ A   0   0   C  ]
				// [ 0   B   0   D  ]
				// [ 0   0   q   qn ]
				// [ 0   0   0   1  ]
				//
				// A = 2 * / (right - left)
				// B = 2 * / (top - bottom)
				// C = - (right + left) / (right - left)
				// D = - (top + bottom) / (top - bottom)
				// q = - 2 / (far - near)
				// qn = - (far + near) / (far - near)

				mProjMatrix = Matrix4::ZERO;
				mProjMatrix[0][0] = A;
				mProjMatrix[0][3] = C;
				mProjMatrix[1][1] = B;
				mProjMatrix[1][3] = D;
				mProjMatrix[2][2] = q;
				mProjMatrix[2][3] = qn;
				mProjMatrix[3][3] = 1;
			} // ortho
		} // !mCustomProjMatrix

		RenderSystem* renderSystem = Root::getSingleton().getRenderSystem();
		// API specific
		renderSystem->_convertProjectionMatrix(mProjMatrix, mProjMatrixRS);
		// API specific for Gpu Programs
		renderSystem->_convertProjectionMatrix(mProjMatrix, mProjMatrixRSDepth, true);


		// Calculate bounding box (local)
		// Box is from 0, down -Z, max dimensions as determined from far plane
		// If infinite view frustum just pick a far value
		Real farDist = (mFarDist == 0) ? 100000 : mFarDist;
		// Near plane bounds
		Vector3 min(left, bottom, -farDist);
		Vector3 max(right, top, 0);

		if (mCustomProjMatrix)
		{
			// Some custom projection matrices can have unusual inverted settings
			// So make sure the AABB is the right way around to start with
			Vector3 tmp = min;
			min.makeFloor(max);
			max.makeCeil(tmp);
		}

		if (mProjType == PT_PERSPECTIVE)
		{
			// Merge with far plane bounds
			Real radio = farDist / mNearDist;
			min.makeFloor(Vector3(left * radio, bottom * radio, -farDist));
			max.makeCeil(Vector3(right * radio, top * radio, 0));
		}
		mBoundingBox.setExtents(min, max);

		mRecalcFrustum = false;

		// Signal to update frustum clipping planes
		mRecalcFrustumPlanes = true;
	}
    //-----------------------------------------------------------------------
    void Frustum::updateFrustum(void) const
    {
        if (isFrustumOutOfDate())
        {
			updateFrustumImpl();
        }
    }

    //-----------------------------------------------------------------------
    void Frustum::updateVertexData(void) const
    {
        if (mRecalcVertexData)
        {
            if (mVertexData.vertexBufferBinding->getBufferCount() <= 0)
            {
                // Initialise vertex & index data
                mVertexData.vertexDeclaration->addElement(0, 0, VET_FLOAT3, VES_POSITION);
                mVertexData.vertexCount = 32;
                mVertexData.vertexStart = 0;
                mVertexData.vertexBufferBinding->setBinding( 0,
                    HardwareBufferManager::getSingleton().createVertexBuffer(
                        sizeof(float)*3, 32, HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY) );
            }

            // Note: Even though we can dealing with general projection matrix here,
            //       but because it's incompatibly with infinite far plane, thus, we
            //       still need to working with projection parameters.

            // Calc near plane corners
            Real vpLeft, vpRight, vpBottom, vpTop;
            calcProjectionParameters(vpLeft, vpRight, vpBottom, vpTop);

            // Treat infinite fardist as some arbitrary far value
            Real farDist = (mFarDist == 0) ? 100000 : mFarDist;

            // Calc far palne corners
            Real radio = mProjType == PT_PERSPECTIVE ? farDist / mNearDist : 1;
            Real farLeft = vpLeft * radio;
            Real farRight = vpRight * radio;
            Real farBottom = vpBottom * radio;
            Real farTop = vpTop * radio;

            // Calculate vertex positions (local)
            // 0 is the origin
            // 1, 2, 3, 4 are the points on the near plane, top left first, clockwise
            // 5, 6, 7, 8 are the points on the far plane, top left first, clockwise
            HardwareVertexBufferSharedPtr vbuf = mVertexData.vertexBufferBinding->getBuffer(0);
            float* pFloat = static_cast<float*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));

            // near plane (remember frustum is going in -Z direction)
            *pFloat++ = vpLeft;  *pFloat++ = vpTop;    *pFloat++ = -mNearDist;
            *pFloat++ = vpRight; *pFloat++ = vpTop;    *pFloat++ = -mNearDist;

            *pFloat++ = vpRight; *pFloat++ = vpTop;    *pFloat++ = -mNearDist;
            *pFloat++ = vpRight; *pFloat++ = vpBottom; *pFloat++ = -mNearDist;

            *pFloat++ = vpRight; *pFloat++ = vpBottom; *pFloat++ = -mNearDist;
            *pFloat++ = vpLeft;  *pFloat++ = vpBottom; *pFloat++ = -mNearDist;

            *pFloat++ = vpLeft;  *pFloat++ = vpBottom; *pFloat++ = -mNearDist;
            *pFloat++ = vpLeft;  *pFloat++ = vpTop;    *pFloat++ = -mNearDist;

            // far plane (remember frustum is going in -Z direction)
            *pFloat++ = farLeft;  *pFloat++ = farTop;    *pFloat++ = -farDist;
            *pFloat++ = farRight; *pFloat++ = farTop;    *pFloat++ = -farDist;

            *pFloat++ = farRight; *pFloat++ = farTop;    *pFloat++ = -farDist;
            *pFloat++ = farRight; *pFloat++ = farBottom; *pFloat++ = -farDist;

            *pFloat++ = farRight; *pFloat++ = farBottom; *pFloat++ = -farDist;
            *pFloat++ = farLeft;  *pFloat++ = farBottom; *pFloat++ = -farDist;

            *pFloat++ = farLeft;  *pFloat++ = farBottom; *pFloat++ = -farDist;
            *pFloat++ = farLeft;  *pFloat++ = farTop;    *pFloat++ = -farDist;

            // Sides of the pyramid
            *pFloat++ = 0.0f;    *pFloat++ = 0.0f;   *pFloat++ = 0.0f;
            *pFloat++ = vpLeft;  *pFloat++ = vpTop;  *pFloat++ = -mNearDist;

            *pFloat++ = 0.0f;    *pFloat++ = 0.0f;   *pFloat++ = 0.0f;
            *pFloat++ = vpRight; *pFloat++ = vpTop;    *pFloat++ = -mNearDist;

            *pFloat++ = 0.0f;    *pFloat++ = 0.0f;   *pFloat++ = 0.0f;
            *pFloat++ = vpRight; *pFloat++ = vpBottom; *pFloat++ = -mNearDist;

            *pFloat++ = 0.0f;    *pFloat++ = 0.0f;   *pFloat++ = 0.0f;
            *pFloat++ = vpLeft;  *pFloat++ = vpBottom; *pFloat++ = -mNearDist;

            // Sides of the box

            *pFloat++ = vpLeft;  *pFloat++ = vpTop;  *pFloat++ = -mNearDist;
            *pFloat++ = farLeft;  *pFloat++ = farTop;  *pFloat++ = -farDist;

            *pFloat++ = vpRight; *pFloat++ = vpTop;    *pFloat++ = -mNearDist;
            *pFloat++ = farRight; *pFloat++ = farTop;    *pFloat++ = -farDist;

            *pFloat++ = vpRight; *pFloat++ = vpBottom; *pFloat++ = -mNearDist;
            *pFloat++ = farRight; *pFloat++ = farBottom; *pFloat++ = -farDist;

            *pFloat++ = vpLeft;  *pFloat++ = vpBottom; *pFloat++ = -mNearDist;
            *pFloat++ = farLeft;  *pFloat++ = farBottom; *pFloat++ = -farDist;


            vbuf->unlock();

            mRecalcVertexData = false;
        }
    }

    //-----------------------------------------------------------------------
    bool Frustum::isViewOutOfDate(void) const
    {
        // Attached to node?
        if (mParentNode)
        {
            if (mRecalcView ||
                mParentNode->_getDerivedOrientation() != mLastParentOrientation ||
                mParentNode->_getDerivedPosition() != mLastParentPosition)
            {
                // Ok, we're out of date with SceneNode we're attached to
                mLastParentOrientation = mParentNode->_getDerivedOrientation();
                mLastParentPosition = mParentNode->_getDerivedPosition();
                mRecalcView = true;
            }
        }
        // Deriving reflection from linked plane?
        if (mLinkedReflectPlane && 
            !(mLastLinkedReflectionPlane == mLinkedReflectPlane->_getDerivedPlane()))
        {
            mReflectPlane = mLinkedReflectPlane->_getDerivedPlane();
            mReflectMatrix = Math::buildReflectionMatrix(mReflectPlane);
            mLastLinkedReflectionPlane = mLinkedReflectPlane->_getDerivedPlane();
            mRecalcView = true;
        }

        return mRecalcView;
    }

    //-----------------------------------------------------------------------
    bool Frustum::isFrustumOutOfDate(void) const
    {
        // Deriving custom near plane from linked plane?
        if (mObliqueDepthProjection)
        {
            // Out of date when view out of data since plane needs to be in view space
            if (isViewOutOfDate())
            {
                mRecalcFrustum = true;
            }
            // Update derived plane
            if (mLinkedObliqueProjPlane && 
                !(mLastLinkedObliqueProjPlane == mLinkedObliqueProjPlane->_getDerivedPlane()))
            {
                mObliqueProjPlane = mLinkedObliqueProjPlane->_getDerivedPlane();
                mLastLinkedObliqueProjPlane = mObliqueProjPlane;
                mRecalcFrustum = true;
            }
        }

        return mRecalcFrustum;
    }

    //-----------------------------------------------------------------------
	void Frustum::updateViewImpl(void) const
	{
		// ----------------------
		// Update the view matrix
		// ----------------------

		// Get orientation from quaternion

		if (!mCustomViewMatrix)
		{
			Matrix3 rot;
			const Quaternion& orientation = getOrientationForViewUpdate();
			const Vector3& position = getPositionForViewUpdate();

			mViewMatrix = Math::makeViewMatrix(position, orientation, mReflect? &mReflectMatrix : 0);
		}

		mRecalcView = false;

		// Signal to update frustum clipping planes
		mRecalcFrustumPlanes = true;
		// Signal to update world space corners
		mRecalcWorldSpaceCorners = true;
		// Signal to update frustum if oblique plane enabled,
		// since plane needs to be in view space
		if (mObliqueDepthProjection)
		{
			mRecalcFrustum = true;
		}
	}
	//---------------------------------------------------------------------
	void Frustum::calcViewMatrixRelative(const Vector3& relPos, Matrix4& matToUpdate) const
	{
		Matrix4 matTrans = Matrix4::IDENTITY;
		matTrans.setTrans(relPos);
		matToUpdate = getViewMatrix() * matTrans;

	}
	//-----------------------------------------------------------------------
    void Frustum::updateView(void) const
    {
        if (isViewOutOfDate())
        {
			updateViewImpl();
        }
    }

	//-----------------------------------------------------------------------
	void Frustum::updateFrustumPlanesImpl(void) const
	{
		// -------------------------
		// Update the frustum planes
		// -------------------------
		Matrix4 combo = mProjMatrix * mViewMatrix;

		mFrustumPlanes[FRUSTUM_PLANE_LEFT].normal.x = combo[3][0] + combo[0][0];
		mFrustumPlanes[FRUSTUM_PLANE_LEFT].normal.y = combo[3][1] + combo[0][1];
		mFrustumPlanes[FRUSTUM_PLANE_LEFT].normal.z = combo[3][2] + combo[0][2];
		mFrustumPlanes[FRUSTUM_PLANE_LEFT].d = combo[3][3] + combo[0][3];

		mFrustumPlanes[FRUSTUM_PLANE_RIGHT].normal.x = combo[3][0] - combo[0][0];
		mFrustumPlanes[FRUSTUM_PLANE_RIGHT].normal.y = combo[3][1] - combo[0][1];
		mFrustumPlanes[FRUSTUM_PLANE_RIGHT].normal.z = combo[3][2] - combo[0][2];
		mFrustumPlanes[FRUSTUM_PLANE_RIGHT].d = combo[3][3] - combo[0][3];

		mFrustumPlanes[FRUSTUM_PLANE_TOP].normal.x = combo[3][0] - combo[1][0];
		mFrustumPlanes[FRUSTUM_PLANE_TOP].normal.y = combo[3][1] - combo[1][1];
		mFrustumPlanes[FRUSTUM_PLANE_TOP].normal.z = combo[3][2] - combo[1][2];
		mFrustumPlanes[FRUSTUM_PLANE_TOP].d = combo[3][3] - combo[1][3];

		mFrustumPlanes[FRUSTUM_PLANE_BOTTOM].normal.x = combo[3][0] + combo[1][0];
		mFrustumPlanes[FRUSTUM_PLANE_BOTTOM].normal.y = combo[3][1] + combo[1][1];
		mFrustumPlanes[FRUSTUM_PLANE_BOTTOM].normal.z = combo[3][2] + combo[1][2];
		mFrustumPlanes[FRUSTUM_PLANE_BOTTOM].d = combo[3][3] + combo[1][3];

		mFrustumPlanes[FRUSTUM_PLANE_NEAR].normal.x = combo[3][0] + combo[2][0];
		mFrustumPlanes[FRUSTUM_PLANE_NEAR].normal.y = combo[3][1] + combo[2][1];
		mFrustumPlanes[FRUSTUM_PLANE_NEAR].normal.z = combo[3][2] + combo[2][2];
		mFrustumPlanes[FRUSTUM_PLANE_NEAR].d = combo[3][3] + combo[2][3];

		mFrustumPlanes[FRUSTUM_PLANE_FAR].normal.x = combo[3][0] - combo[2][0];
		mFrustumPlanes[FRUSTUM_PLANE_FAR].normal.y = combo[3][1] - combo[2][1];
		mFrustumPlanes[FRUSTUM_PLANE_FAR].normal.z = combo[3][2] - combo[2][2];
		mFrustumPlanes[FRUSTUM_PLANE_FAR].d = combo[3][3] - combo[2][3];

		// Renormalise any normals which were not unit length
		for(int i=0; i<6; i++ ) 
		{
			Real length = mFrustumPlanes[i].normal.normalise();
			mFrustumPlanes[i].d /= length;
		}

		mRecalcFrustumPlanes = false;
	}
    //-----------------------------------------------------------------------
    void Frustum::updateFrustumPlanes(void) const
    {
        updateView();
        updateFrustum();

        if (mRecalcFrustumPlanes)
        {
			updateFrustumPlanesImpl();
        }
    }
	//-----------------------------------------------------------------------
	void Frustum::updateWorldSpaceCornersImpl(void) const
	{
		Matrix4 eyeToWorld = mViewMatrix.inverseAffine();

		// Note: Even though we can dealing with general projection matrix here,
		//       but because it's incompatibly with infinite far plane, thus, we
		//       still need to working with projection parameters.

		// Calc near plane corners
		Real nearLeft, nearRight, nearBottom, nearTop;
		calcProjectionParameters(nearLeft, nearRight, nearBottom, nearTop);

		// Treat infinite fardist as some arbitrary far value
		Real farDist = (mFarDist == 0) ? 100000 : mFarDist;

		// Calc far palne corners
		Real radio = mProjType == PT_PERSPECTIVE ? farDist / mNearDist : 1;
		Real farLeft = nearLeft * radio;
		Real farRight = nearRight * radio;
		Real farBottom = nearBottom * radio;
		Real farTop = nearTop * radio;

		// near
		mWorldSpaceCorners[0] = eyeToWorld.transformAffine(Vector3(nearRight, nearTop,    -mNearDist));
		mWorldSpaceCorners[1] = eyeToWorld.transformAffine(Vector3(nearLeft,  nearTop,    -mNearDist));
		mWorldSpaceCorners[2] = eyeToWorld.transformAffine(Vector3(nearLeft,  nearBottom, -mNearDist));
		mWorldSpaceCorners[3] = eyeToWorld.transformAffine(Vector3(nearRight, nearBottom, -mNearDist));
		// far
		mWorldSpaceCorners[4] = eyeToWorld.transformAffine(Vector3(farRight,  farTop,     -farDist));
		mWorldSpaceCorners[5] = eyeToWorld.transformAffine(Vector3(farLeft,   farTop,     -farDist));
		mWorldSpaceCorners[6] = eyeToWorld.transformAffine(Vector3(farLeft,   farBottom,  -farDist));
		mWorldSpaceCorners[7] = eyeToWorld.transformAffine(Vector3(farRight,  farBottom,  -farDist));


		mRecalcWorldSpaceCorners = false;
	}
    //-----------------------------------------------------------------------
    void Frustum::updateWorldSpaceCorners(void) const
    {
        updateView();

        if (mRecalcWorldSpaceCorners)
        {
			updateWorldSpaceCornersImpl();
        }

    }

    //-----------------------------------------------------------------------
    Real Frustum::getAspectRatio(void) const
    {
        return mAspect;
    }

    //-----------------------------------------------------------------------
    void Frustum::setAspectRatio(Real r)
    {
        mAspect = r;
        invalidateFrustum();
    }

    //-----------------------------------------------------------------------
    const AxisAlignedBox& Frustum::getBoundingBox(void) const
    {
        return mBoundingBox;
    }
    //-----------------------------------------------------------------------
    void Frustum::_updateRenderQueue(RenderQueue* queue)
    {
		if (mDebugDisplay)
		{
			// Add self 
			queue->addRenderable(this);
		}
    }
    //-----------------------------------------------------------------------
    const String& Frustum::getMovableType(void) const
    {
        return msMovableType;
    }
    //-----------------------------------------------------------------------
	Real Frustum::getBoundingRadius(void) const
	{
        return (mFarDist == 0)? 100000 : mFarDist;
	}
    //-----------------------------------------------------------------------
    const MaterialPtr& Frustum::getMaterial(void) const
    {
        return mMaterial;
    }
    //-----------------------------------------------------------------------
    void Frustum::getRenderOperation(RenderOperation& op) 
    {
        updateVertexData();
        op.operationType = RenderOperation::OT_LINE_LIST;
        op.useIndexes = false;
        op.vertexData = &mVertexData;
    }
    //-----------------------------------------------------------------------
    void Frustum::getWorldTransforms(Matrix4* xform) const 
    {
        if (mParentNode)
            *xform = mParentNode->_getFullTransform();
        else
            *xform = Matrix4::IDENTITY;
    }
    //-----------------------------------------------------------------------
    Real Frustum::getSquaredViewDepth(const Camera* cam) const 
    {
        // Calc from centre
        if (mParentNode)
            return (cam->getDerivedPosition() 
                - mParentNode->_getDerivedPosition()).squaredLength();
        else
            return 0;
    }
    //-----------------------------------------------------------------------
    const LightList& Frustum::getLights(void) const 
    {
        // N/A
        static LightList ll;
        return ll;
    }
    //-----------------------------------------------------------------------
    void Frustum::_notifyCurrentCamera(Camera* cam)
    {
        // Make sure bounding box up-to-date
        updateFrustum();

        MovableObject::_notifyCurrentCamera(cam);
    }

    // -------------------------------------------------------------------
    void Frustum::invalidateFrustum() const
    {
        mRecalcFrustum = true;
        mRecalcFrustumPlanes = true;
        mRecalcWorldSpaceCorners = true;
        mRecalcVertexData = true;
    }
    // -------------------------------------------------------------------
    void Frustum::invalidateView() const
    {
        mRecalcView = true;
        mRecalcFrustumPlanes = true;
        mRecalcWorldSpaceCorners = true;
    }
    // -------------------------------------------------------------------
    const Vector3* Frustum::getWorldSpaceCorners(void) const
    {
        updateWorldSpaceCorners();

        return mWorldSpaceCorners;
    }
    //-----------------------------------------------------------------------
    void Frustum::setProjectionType(ProjectionType pt)
    {
        mProjType = pt;
        invalidateFrustum();
    }

    //-----------------------------------------------------------------------
    ProjectionType Frustum::getProjectionType(void) const
    {
        return mProjType;
    }
    //-----------------------------------------------------------------------
    const Vector3& Frustum::getPositionForViewUpdate(void) const
    {
        return mLastParentPosition;
    }
    //-----------------------------------------------------------------------
    const Quaternion& Frustum::getOrientationForViewUpdate(void) const
    {
        return mLastParentOrientation;
    }
    //-----------------------------------------------------------------------
    void Frustum::enableReflection(const Plane& p)
    {
        mReflect = true;
        mReflectPlane = p;
        mLinkedReflectPlane = 0;
        mReflectMatrix = Math::buildReflectionMatrix(p);
        invalidateView();

    }
    //-----------------------------------------------------------------------
    void Frustum::enableReflection(const MovablePlane* p)
    {
        mReflect = true;
        mLinkedReflectPlane = p;
        mReflectPlane = mLinkedReflectPlane->_getDerivedPlane();
        mReflectMatrix = Math::buildReflectionMatrix(mReflectPlane);
        mLastLinkedReflectionPlane = mLinkedReflectPlane->_getDerivedPlane();
        invalidateView();
    }
    //-----------------------------------------------------------------------
    void Frustum::disableReflection(void)
    {
        mReflect = false;
        mLinkedReflectPlane = 0;
        mLastLinkedReflectionPlane.normal = Vector3::ZERO;
        invalidateView();
    }
    //---------------------------------------------------------------------
    bool Frustum::projectSphere(const Sphere& sphere, 
        Real* left, Real* top, Real* right, Real* bottom) const
    {
		// See http://www.gamasutra.com/features/20021011/lengyel_06.htm
        // Transform light position into camera space

        updateView();
        Vector3 eyeSpacePos = mViewMatrix.transformAffine(sphere.getCenter());

		// initialise
		*left = *bottom = -1.0f;
		*right = *top = 1.0f;

        if (eyeSpacePos.z < 0)
        {
			updateFrustum();
			const Matrix4& projMatrix = getProjectionMatrix();
            Real r = sphere.getRadius();
			Real rsq = r * r;

            // early-exit
            if (eyeSpacePos.squaredLength() <= rsq)
                return false;

			Real Lxz = Math::Sqr(eyeSpacePos.x) + Math::Sqr(eyeSpacePos.z);
			Real Lyz = Math::Sqr(eyeSpacePos.y) + Math::Sqr(eyeSpacePos.z);

			// Find the tangent planes to the sphere
			// XZ first
			// calculate quadratic discriminant: b*b - 4ac
			// x = Nx
			// a = Lx^2 + Lz^2
			// b = -2rLx
			// c = r^2 - Lz^2
			Real a = Lxz;
			Real b = -2.0 * r * eyeSpacePos.x;
			Real c = rsq - Math::Sqr(eyeSpacePos.z);
			Real D = b*b - 4*a*c;

			// two roots?
			if (D > 0)
			{
				Real sqrootD = Math::Sqrt(D);
				// solve the quadratic to get the components of the normal
				Real Nx0 = (-b + sqrootD) / (2 * a);
				Real Nx1 = (-b - sqrootD) / (2 * a);
				
				// Derive Z from this
				Real Nz0 = (r - Nx0 * eyeSpacePos.x) / eyeSpacePos.z;
				Real Nz1 = (r - Nx1 * eyeSpacePos.x) / eyeSpacePos.z;

				// Get the point of tangency
				// Only consider points of tangency in front of the camera
				Real Pz0 = (Lxz - rsq) / (eyeSpacePos.z - ((Nz0 / Nx0) * eyeSpacePos.x));
				if (Pz0 < 0)
				{
					// Project point onto near plane in worldspace
					Real nearx0 = (Nz0 * mNearDist) / Nx0;
					// now we need to map this to viewport coords
					// use projection matrix since that will take into account all factors
					Vector3 relx0 = projMatrix * Vector3(nearx0, 0, -mNearDist);

					// find out whether this is a left side or right side
					Real Px0 = -(Pz0 * Nz0) / Nx0;
					if (Px0 > eyeSpacePos.x)
					{
						*right = std::min(*right, relx0.x);
					}
					else
					{
						*left = std::max(*left, relx0.x);
					}
				}
				Real Pz1 = (Lxz - rsq) / (eyeSpacePos.z - ((Nz1 / Nx1) * eyeSpacePos.x));
				if (Pz1 < 0)
				{
					// Project point onto near plane in worldspace
					Real nearx1 = (Nz1 * mNearDist) / Nx1;
					// now we need to map this to viewport coords
					// use projection matrix since that will take into account all factors
					Vector3 relx1 = projMatrix * Vector3(nearx1, 0, -mNearDist);

					// find out whether this is a left side or right side
					Real Px1 = -(Pz1 * Nz1) / Nx1;
					if (Px1 > eyeSpacePos.x)
					{
						*right = std::min(*right, relx1.x);
					}
					else
					{
						*left = std::max(*left, relx1.x);
					}
				}
			}


			// Now YZ 
			// calculate quadratic discriminant: b*b - 4ac
			// x = Ny
			// a = Ly^2 + Lz^2
			// b = -2rLy
			// c = r^2 - Lz^2
			a = Lyz;
			b = -2.0 * r * eyeSpacePos.y;
			c = rsq - Math::Sqr(eyeSpacePos.z);
			D = b*b - 4*a*c;

			// two roots?
			if (D > 0)
			{
				Real sqrootD = Math::Sqrt(D);
				// solve the quadratic to get the components of the normal
				Real Ny0 = (-b + sqrootD) / (2 * a);
				Real Ny1 = (-b - sqrootD) / (2 * a);

				// Derive Z from this
				Real Nz0 = (r - Ny0 * eyeSpacePos.y) / eyeSpacePos.z;
				Real Nz1 = (r - Ny1 * eyeSpacePos.y) / eyeSpacePos.z;

				// Get the point of tangency
				// Only consider points of tangency in front of the camera
				Real Pz0 = (Lyz - rsq) / (eyeSpacePos.z - ((Nz0 / Ny0) * eyeSpacePos.y));
				if (Pz0 < 0)
				{
					// Project point onto near plane in worldspace
					Real neary0 = (Nz0 * mNearDist) / Ny0;
					// now we need to map this to viewport coords
					// use projection matriy since that will take into account all factors
					Vector3 rely0 = projMatrix * Vector3(0, neary0, -mNearDist);

					// find out whether this is a top side or bottom side
					Real Py0 = -(Pz0 * Nz0) / Ny0;
					if (Py0 > eyeSpacePos.y)
					{
						*top = std::min(*top, rely0.y);
					}
					else
					{
						*bottom = std::max(*bottom, rely0.y);
					}
				}
				Real Pz1 = (Lyz - rsq) / (eyeSpacePos.z - ((Nz1 / Ny1) * eyeSpacePos.y));
				if (Pz1 < 0)
				{
					// Project point onto near plane in worldspace
					Real neary1 = (Nz1 * mNearDist) / Ny1;
					// now we need to map this to viewport coords
					// use projection matriy since that will take into account all factors
					Vector3 rely1 = projMatrix * Vector3(0, neary1, -mNearDist);

					// find out whether this is a top side or bottom side
					Real Py1 = -(Pz1 * Nz1) / Ny1;
					if (Py1 > eyeSpacePos.y)
					{
						*top = std::min(*top, rely1.y);
					}
					else
					{
						*bottom = std::max(*bottom, rely1.y);
					}
				}
			}
        }

        return (*left != -1.0f) || (*top != 1.0f) || (*right != 1.0f) || (*bottom != -1.0f);

    }
    //---------------------------------------------------------------------
    void Frustum::enableCustomNearClipPlane(const MovablePlane* plane)
    {
        mObliqueDepthProjection = true;
        mLinkedObliqueProjPlane = plane;
        mObliqueProjPlane = plane->_getDerivedPlane();
        invalidateFrustum();
    }
    //---------------------------------------------------------------------
    void Frustum::enableCustomNearClipPlane(const Plane& plane)
    {
        mObliqueDepthProjection = true;
        mLinkedObliqueProjPlane = 0;
        mObliqueProjPlane = plane;
        invalidateFrustum();
    }
    //---------------------------------------------------------------------
    void Frustum::disableCustomNearClipPlane(void)
    {
        mObliqueDepthProjection = false;
        mLinkedObliqueProjPlane = 0;
        invalidateFrustum();
    }
    //---------------------------------------------------------------------
	void Frustum::setCustomViewMatrix(bool enable, const Matrix4& viewMatrix)
	{
		mCustomViewMatrix = enable;
		if (enable)
		{
            assert(viewMatrix.isAffine());
			mViewMatrix = viewMatrix;
		}
		invalidateView();
	}
    //---------------------------------------------------------------------
	void Frustum::setCustomProjectionMatrix(bool enable, const Matrix4& projMatrix)
	{
		mCustomProjMatrix = enable;
		if (enable)
		{
			mProjMatrix = projMatrix;
		}
		invalidateFrustum();
	}
	//---------------------------------------------------------------------
	void Frustum::setOrthoWindow(Real w, Real h)
	{
		mOrthoHeight = h;
		mAspect = w / h;
		invalidateFrustum();
	}
	//---------------------------------------------------------------------
	void Frustum::setOrthoWindowHeight(Real h)
	{
		mOrthoHeight = h;
		invalidateFrustum();
	}
	//---------------------------------------------------------------------
	void Frustum::setOrthoWindowWidth(Real w)
	{
		mOrthoHeight = w / mAspect;
		invalidateFrustum();
	}
	//---------------------------------------------------------------------
	Real Frustum::getOrthoWindowHeight() const
	{
		return mOrthoHeight;
	}
	//---------------------------------------------------------------------
	Real Frustum::getOrthoWindowWidth() const
	{
		return mOrthoHeight * mAspect;	
	}
	//---------------------------------------------------------------------
	void Frustum::visitRenderables(Renderable::Visitor* visitor, 
		bool debugRenderables)
	{
		// Only displayed in debug
		if (debugRenderables)
		{
			visitor->visit(this, 0, true);
		}

	}
	//---------------------------------------------------------------------
	void Frustum::setFrustumExtents(Real left, Real right, Real top, Real bottom)
	{
		mFrustumExtentsManuallySet = true;
		mLeft = left;
		mRight = right;
		mTop = top;
		mBottom = bottom;

		invalidateFrustum();
	}
	//---------------------------------------------------------------------
	void Frustum::resetFrustumExtents()
	{
		mFrustumExtentsManuallySet = false;
		invalidateFrustum();
	}
	//---------------------------------------------------------------------
	void Frustum::getFrustumExtents(Real& outleft, Real& outright, Real& outtop, Real& outbottom) const
	{
		updateFrustum();
		outleft = mLeft;
		outright = mRight;
		outtop = mTop;
		outbottom = mBottom;
	}
	//---------------------------------------------------------------------
	PlaneBoundedVolume Frustum::getPlaneBoundedVolume()
	{
		updateFrustumPlanes();

		PlaneBoundedVolume volume;
		volume.planes.push_back(mFrustumPlanes[FRUSTUM_PLANE_NEAR]);
		volume.planes.push_back(mFrustumPlanes[FRUSTUM_PLANE_FAR]);
		volume.planes.push_back(mFrustumPlanes[FRUSTUM_PLANE_BOTTOM]);
		volume.planes.push_back(mFrustumPlanes[FRUSTUM_PLANE_TOP]);
		volume.planes.push_back(mFrustumPlanes[FRUSTUM_PLANE_LEFT]);
		volume.planes.push_back(mFrustumPlanes[FRUSTUM_PLANE_RIGHT]);
		return volume;
	}
	//---------------------------------------------------------------------







} // namespace Ogre
