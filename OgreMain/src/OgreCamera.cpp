/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#include "OgreStableHeaders.h"
#include "OgreCamera.h"

#include "OgreMath.h"
#include "OgreMatrix3.h"
#include "OgreSceneManager.h"
#include "OgreSceneNode.h"
#include "OgreAxisAlignedBox.h"
#include "OgreSphere.h"
#include "OgreLogManager.h"
#include "OgreException.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"

namespace Ogre {

    String Camera::msMovableType = "Camera";
    //-----------------------------------------------------------------------
    Camera::Camera( const String& name, SceneManager* sm)
        : mName( name ),
		mSceneMgr(sm),
		mOrientation(Quaternion::IDENTITY),
		mPosition(Vector3::ZERO),
		mSceneDetail(PM_SOLID),
		mAutoTrackTarget(0),
		mAutoTrackOffset(Vector3::ZERO),
		mSceneLodFactor(1.0f),
		mSceneLodFactorInv(1.0f),
		mWindowSet(false),
		mLastViewport(0),
		mAutoAspectRatio(false),
		mCullFrustum(0),
		mUseRenderingDistance(true),
		mLodCamera(0)
    {

        // Reasonable defaults to camera params
        mFOVy = Radian(Math::PI/4.0);
        mNearDist = 100.0f;
        mFarDist = 100000.0f;
        mAspect = 1.33333333333333f;
        mProjType = PT_PERSPECTIVE;
        setFixedYawAxis(true);    // Default to fixed yaw, like freelook since most people expect this

        invalidateFrustum();
        invalidateView();

        // Init matrices
        mViewMatrix = Matrix4::ZERO;
        mProjMatrixRS = Matrix4::ZERO;

        mParentNode = 0;

        // no reflection
        mReflect = false;

        mVisible = false;

    }

    //-----------------------------------------------------------------------
    Camera::~Camera()
    {
        // Do nothing
    }

    //-----------------------------------------------------------------------
    SceneManager* Camera::getSceneManager(void) const
    {
        return mSceneMgr;
    }
    //-----------------------------------------------------------------------
    const String& Camera::getName(void) const
    {
        return mName;
    }


    //-----------------------------------------------------------------------
    void Camera::setPolygonMode(PolygonMode sd)
    {
        mSceneDetail = sd;
    }

    //-----------------------------------------------------------------------
    PolygonMode Camera::getPolygonMode(void) const
    {
        return mSceneDetail;
    }

    //-----------------------------------------------------------------------
    void Camera::setPosition(Real x, Real y, Real z)
    {
        mPosition.x = x;
        mPosition.y = y;
        mPosition.z = z;
        invalidateView();
    }

    //-----------------------------------------------------------------------
    void Camera::setPosition(const Vector3& vec)
    {
        mPosition = vec;
        invalidateView();
    }

    //-----------------------------------------------------------------------
    const Vector3& Camera::getPosition(void) const
    {
        return mPosition;
    }

    //-----------------------------------------------------------------------
    void Camera::move(const Vector3& vec)
    {
        mPosition = mPosition + vec;
        invalidateView();
    }

    //-----------------------------------------------------------------------
    void Camera::moveRelative(const Vector3& vec)
    {
        // Transform the axes of the relative vector by camera's local axes
        Vector3 trans = mOrientation * vec;

        mPosition = mPosition + trans;
        invalidateView();
    }

    //-----------------------------------------------------------------------
    void Camera::setDirection(Real x, Real y, Real z)
    {
        setDirection(Vector3(x,y,z));
    }

    //-----------------------------------------------------------------------
    void Camera::setDirection(const Vector3& vec)
    {
        // Do nothing if given a zero vector
        // (Replaced assert since this could happen with auto tracking camera and
        //  camera passes through the lookAt point)
        if (vec == Vector3::ZERO) return;

        // Remember, camera points down -Z of local axes!
        // Therefore reverse direction of direction vector before determining local Z
        Vector3 zAdjustVec = -vec;
        zAdjustVec.normalise();


        if( mYawFixed )
        {
            Vector3 xVec = mYawFixedAxis.crossProduct( zAdjustVec );
            xVec.normalise();

            Vector3 yVec = zAdjustVec.crossProduct( xVec );
            yVec.normalise();

            mOrientation.FromAxes( xVec, yVec, zAdjustVec );
        }
        else
        {

            // Get axes from current quaternion
            Vector3 axes[3];
            updateView();
            mRealOrientation.ToAxes(axes);
            Quaternion rotQuat;
            if ( (axes[2]+zAdjustVec).squaredLength() <  0.00005f) 
            {
                // Oops, a 180 degree turn (infinite possible rotation axes)
                // Default to yaw i.e. use current UP
                rotQuat.FromAngleAxis(Radian(Math::PI), axes[1]);
            }
            else
            {
                // Derive shortest arc to new direction
                rotQuat = axes[2].getRotationTo(zAdjustVec);

            }
            mOrientation = rotQuat * mOrientation;
        }

        // transform to parent space
        if (mParentNode)
        {
            mOrientation =
                mParentNode->_getDerivedOrientation().Inverse() * mOrientation;
        }

        // TODO If we have a fixed yaw axis, we mustn't break it by using the
        // shortest arc because this will sometimes cause a relative yaw
        // which will tip the camera

        invalidateView();

    }

    //-----------------------------------------------------------------------
    Vector3 Camera::getDirection(void) const
    {
        // Direction points down -Z by default
        return mOrientation * -Vector3::UNIT_Z;
    }

    //-----------------------------------------------------------------------
    Vector3 Camera::getUp(void) const
    {
        return mOrientation * Vector3::UNIT_Y;
    }

    //-----------------------------------------------------------------------
    Vector3 Camera::getRight(void) const
    {
        return mOrientation * Vector3::UNIT_X;
    }

    //-----------------------------------------------------------------------
    void Camera::lookAt(const Vector3& targetPoint)
    {
        updateView();
        this->setDirection(targetPoint - mRealPosition);
    }

    //-----------------------------------------------------------------------
    void Camera::lookAt( Real x, Real y, Real z )
    {
        Vector3 vTemp( x, y, z );
        this->lookAt(vTemp);
    }

    //-----------------------------------------------------------------------
    void Camera::roll(const Radian& angle)
    {
        // Rotate around local Z axis
        Vector3 zAxis = mOrientation * Vector3::UNIT_Z;
        rotate(zAxis, angle);

        invalidateView();
    }

    //-----------------------------------------------------------------------
    void Camera::yaw(const Radian& angle)
    {
        Vector3 yAxis;

        if (mYawFixed)
        {
            // Rotate around fixed yaw axis
            yAxis = mYawFixedAxis;
        }
        else
        {
            // Rotate around local Y axis
            yAxis = mOrientation * Vector3::UNIT_Y;
        }

        rotate(yAxis, angle);

        invalidateView();
    }

    //-----------------------------------------------------------------------
    void Camera::pitch(const Radian& angle)
    {
        // Rotate around local X axis
        Vector3 xAxis = mOrientation * Vector3::UNIT_X;
        rotate(xAxis, angle);

        invalidateView();

    }

    //-----------------------------------------------------------------------
    void Camera::rotate(const Vector3& axis, const Radian& angle)
    {
        Quaternion q;
        q.FromAngleAxis(angle,axis);
        rotate(q);
    }

    //-----------------------------------------------------------------------
    void Camera::rotate(const Quaternion& q)
    {
        // Note the order of the mult, i.e. q comes after

		// Normalise the quat to avoid cumulative problems with precision
		Quaternion qnorm = q;
		qnorm.normalise();
        mOrientation = qnorm * mOrientation;

        invalidateView();

    }

    //-----------------------------------------------------------------------
    bool Camera::isViewOutOfDate(void) const
    {
        // Overridden from Frustum to use local orientation / position offsets
        // Attached to node?
        if (mParentNode != 0)
        {
            if (mRecalcView ||
                mParentNode->_getDerivedOrientation() != mLastParentOrientation ||
                mParentNode->_getDerivedPosition() != mLastParentPosition)
            {
                // Ok, we're out of date with SceneNode we're attached to
                mLastParentOrientation = mParentNode->_getDerivedOrientation();
                mLastParentPosition = mParentNode->_getDerivedPosition();
                mRealOrientation = mLastParentOrientation * mOrientation;
                mRealPosition = (mLastParentOrientation * mPosition) + mLastParentPosition;
                mRecalcView = true;
                mRecalcWindow = true;
            }
        }
        else
        {
            // Rely on own updates
            mRealOrientation = mOrientation;
            mRealPosition = mPosition;
        }

        // Deriving reflection from linked plane?
        if (mReflect && mLinkedReflectPlane && 
            !(mLastLinkedReflectionPlane == mLinkedReflectPlane->_getDerivedPlane()))
        {
            mReflectPlane = mLinkedReflectPlane->_getDerivedPlane();
            mReflectMatrix = Math::buildReflectionMatrix(mReflectPlane);
            mLastLinkedReflectionPlane = mLinkedReflectPlane->_getDerivedPlane();
            mRecalcView = true;
            mRecalcWindow = true;
        }

        // Deriving reflected orientation / position
        if (mRecalcView)
        {
            if (mReflect)
            {
                // Calculate reflected orientation, use up-vector as fallback axis.
				Vector3 dir = mRealOrientation * Vector3::NEGATIVE_UNIT_Z;
				Vector3 rdir = dir.reflect(mReflectPlane.normal);
                Vector3 up = mRealOrientation * Vector3::UNIT_Y;
				mDerivedOrientation = dir.getRotationTo(rdir, up) * mRealOrientation;

                // Calculate reflected position.
                mDerivedPosition = mReflectMatrix.transformAffine(mRealPosition);
            }
            else
            {
                mDerivedOrientation = mRealOrientation;
                mDerivedPosition = mRealPosition;
            }
        }

        return mRecalcView;

    }

    // -------------------------------------------------------------------
    void Camera::invalidateView() const
    {
        mRecalcWindow = true;
        Frustum::invalidateView();
    }
    // -------------------------------------------------------------------
    void Camera::invalidateFrustum(void) const
    {
        mRecalcWindow = true;
        Frustum::invalidateFrustum();
    }
    //-----------------------------------------------------------------------
    void Camera::_renderScene(Viewport *vp, bool includeOverlays)
    {

        mSceneMgr->_renderScene(this, vp, includeOverlays);
    }


    //-----------------------------------------------------------------------
    std::ostream& operator<<( std::ostream& o, const Camera& c )
    {
        o << "Camera(Name='" << c.mName << "', pos=" << c.mPosition;
        Vector3 dir(c.mOrientation*Vector3(0,0,-1));
        o << ", direction=" << dir << ",near=" << c.mNearDist;
        o << ", far=" << c.mFarDist << ", FOVy=" << c.mFOVy.valueDegrees();
        o << ", aspect=" << c.mAspect << ", ";
        o << ", xoffset=" << c.mFrustumOffset.x << ", yoffset=" << c.mFrustumOffset.y;
        o << ", focalLength=" << c.mFocalLength << ", ";
        o << "NearFrustumPlane=" << c.mFrustumPlanes[FRUSTUM_PLANE_NEAR] << ", ";
        o << "FarFrustumPlane=" << c.mFrustumPlanes[FRUSTUM_PLANE_FAR] << ", ";
        o << "LeftFrustumPlane=" << c.mFrustumPlanes[FRUSTUM_PLANE_LEFT] << ", ";
        o << "RightFrustumPlane=" << c.mFrustumPlanes[FRUSTUM_PLANE_RIGHT] << ", ";
        o << "TopFrustumPlane=" << c.mFrustumPlanes[FRUSTUM_PLANE_TOP] << ", ";
        o << "BottomFrustumPlane=" << c.mFrustumPlanes[FRUSTUM_PLANE_BOTTOM];
        o << ")";

        return o;
    }

    //-----------------------------------------------------------------------
    void Camera::setFixedYawAxis(bool useFixed, const Vector3& fixedAxis)
    {
        mYawFixed = useFixed;
        mYawFixedAxis = fixedAxis;
    }

    //-----------------------------------------------------------------------
    void Camera::_notifyRenderedFaces(unsigned int numfaces)
    {
        mVisFacesLastRender = numfaces;
    }

    //-----------------------------------------------------------------------
    void Camera::_notifyRenderedBatches(unsigned int numbatches)
    {
        mVisBatchesLastRender = numbatches;
    }

    //-----------------------------------------------------------------------
    unsigned int Camera::_getNumRenderedFaces(void) const
    {
        return mVisFacesLastRender;
    }
    //-----------------------------------------------------------------------
    unsigned int Camera::_getNumRenderedBatches(void) const
    {
        return mVisBatchesLastRender;
    }
    //-----------------------------------------------------------------------
    const Quaternion& Camera::getOrientation(void) const
    {
        return mOrientation;
    }

    //-----------------------------------------------------------------------
    void Camera::setOrientation(const Quaternion& q)
    {
        mOrientation = q;
		mOrientation.normalise();
        invalidateView();
    }
    //-----------------------------------------------------------------------
    const Quaternion& Camera::getDerivedOrientation(void) const
    {
        updateView();
        return mDerivedOrientation;
    }
    //-----------------------------------------------------------------------
    const Vector3& Camera::getDerivedPosition(void) const
    {
        updateView();
        return mDerivedPosition;
    }
    //-----------------------------------------------------------------------
    Vector3 Camera::getDerivedDirection(void) const
    {
        // Direction points down -Z
        updateView();
        return mDerivedOrientation * Vector3::NEGATIVE_UNIT_Z;
    }
    //-----------------------------------------------------------------------
    Vector3 Camera::getDerivedUp(void) const
    {
        updateView();
        return mDerivedOrientation * Vector3::UNIT_Y;
    }
    //-----------------------------------------------------------------------
    Vector3 Camera::getDerivedRight(void) const
    {
        updateView();
        return mDerivedOrientation * Vector3::UNIT_X;
    }
    //-----------------------------------------------------------------------
    const Quaternion& Camera::getRealOrientation(void) const
    {
        updateView();
        return mRealOrientation;
    }
    //-----------------------------------------------------------------------
    const Vector3& Camera::getRealPosition(void) const
    {
        updateView();
        return mRealPosition;
    }
    //-----------------------------------------------------------------------
    Vector3 Camera::getRealDirection(void) const
    {
        // Direction points down -Z
        updateView();
        return mRealOrientation * Vector3::NEGATIVE_UNIT_Z;
    }
    //-----------------------------------------------------------------------
    Vector3 Camera::getRealUp(void) const
    {
        updateView();
        return mRealOrientation * Vector3::UNIT_Y;
    }
    //-----------------------------------------------------------------------
    Vector3 Camera::getRealRight(void) const
    {
        updateView();
        return mRealOrientation * Vector3::UNIT_X;
    }
    //-----------------------------------------------------------------------
    const String& Camera::getMovableType(void) const
    {
        return msMovableType;
    }
    //-----------------------------------------------------------------------
    void Camera::setAutoTracking(bool enabled, SceneNode* target, 
        const Vector3& offset)
    {
        if (enabled)
        {
            assert (target != 0 && "target cannot be a null pointer if tracking is enabled");
            mAutoTrackTarget = target;
            mAutoTrackOffset = offset;
        }
        else
        {
            mAutoTrackTarget = 0;
        }
    }
    //-----------------------------------------------------------------------
    void Camera::_autoTrack(void)
    {
        // NB assumes that all scene nodes have been updated
        if (mAutoTrackTarget)
        {
            lookAt(mAutoTrackTarget->_getDerivedPosition() + mAutoTrackOffset);
        }
    }
    //-----------------------------------------------------------------------
	void Camera::setLodBias(Real factor)
	{
		assert(factor > 0.0f && "Bias factor must be > 0!");
		mSceneLodFactor = factor;
		mSceneLodFactorInv = 1.0f / factor;
	}
    //-----------------------------------------------------------------------
	Real Camera::getLodBias(void) const
	{
		return mSceneLodFactor;
	}
    //-----------------------------------------------------------------------
	Real Camera::_getLodBiasInverse(void) const
	{
		return mSceneLodFactorInv;
	}
	//-----------------------------------------------------------------------
	void Camera::setLodCamera(const Camera* lodCam)
	{
		if (lodCam == this)
			mLodCamera = 0;
		else
			mLodCamera = lodCam;
	}
	//---------------------------------------------------------------------
	const Camera* Camera::getLodCamera() const
	{
		return mLodCamera? mLodCamera : this;
	}
    //-----------------------------------------------------------------------
	Ray Camera::getCameraToViewportRay(Real screenX, Real screenY) const
	{
		Ray ret;
		getCameraToViewportRay(screenX, screenY, &ret);
		return ret;
	}
	//---------------------------------------------------------------------
    void Camera::getCameraToViewportRay(Real screenX, Real screenY, Ray* outRay) const
    {
		Matrix4 inverseVP = (getProjectionMatrix() * getViewMatrix(true)).inverse();

		Real nx = (2.0f * screenX) - 1.0f;
		Real ny = 1.0f - (2.0f * screenY);
		Vector3 nearPoint(nx, ny, -1.f);
		// Use midPoint rather than far point to avoid issues with infinite projection
		Vector3 midPoint (nx, ny,  0.0f);

		// Get ray origin and ray target on near plane in world space
		Vector3 rayOrigin, rayTarget;
		
		rayOrigin = inverseVP * nearPoint;
		rayTarget = inverseVP * midPoint;

		Vector3 rayDirection = rayTarget - rayOrigin;
		rayDirection.normalise();

		outRay->setOrigin(rayOrigin);
		outRay->setDirection(rayDirection);
    } 
	//---------------------------------------------------------------------
	PlaneBoundedVolume Camera::getCameraToViewportBoxVolume(Real screenLeft, 
		Real screenTop, Real screenRight, Real screenBottom, bool includeFarPlane)
	{
		PlaneBoundedVolume vol;
		getCameraToViewportBoxVolume(screenLeft, screenTop, screenRight, screenBottom, 
			&vol, includeFarPlane);
		return vol;

	}
	//---------------------------------------------------------------------()
	void Camera::getCameraToViewportBoxVolume(Real screenLeft, 
		Real screenTop, Real screenRight, Real screenBottom, 
		PlaneBoundedVolume* outVolume, bool includeFarPlane)
	{
		outVolume->planes.clear();

		if (mProjType == PT_PERSPECTIVE)
		{

			// Use the corner rays to generate planes
			Ray ul = getCameraToViewportRay(screenLeft, screenTop);
			Ray ur = getCameraToViewportRay(screenRight, screenTop);
			Ray bl = getCameraToViewportRay(screenLeft, screenBottom);
			Ray br = getCameraToViewportRay(screenRight, screenBottom);


			Vector3 normal;
			// top plane
			normal = ul.getDirection().crossProduct(ur.getDirection());
			normal.normalise();
			outVolume->planes.push_back(
				Plane(normal, getDerivedPosition()));

			// right plane
			normal = ur.getDirection().crossProduct(br.getDirection());
			normal.normalise();
			outVolume->planes.push_back(
				Plane(normal, getDerivedPosition()));

			// bottom plane
			normal = br.getDirection().crossProduct(bl.getDirection());
			normal.normalise();
			outVolume->planes.push_back(
				Plane(normal, getDerivedPosition()));

			// left plane
			normal = bl.getDirection().crossProduct(ul.getDirection());
			normal.normalise();
			outVolume->planes.push_back(
				Plane(normal, getDerivedPosition()));

		}
		else
		{
			// ortho planes are parallel to frustum planes

			Ray ul = getCameraToViewportRay(screenLeft, screenTop);
			Ray br = getCameraToViewportRay(screenRight, screenBottom);

			updateFrustumPlanes();
			outVolume->planes.push_back(
				Plane(mFrustumPlanes[FRUSTUM_PLANE_TOP].normal, ul.getOrigin()));
			outVolume->planes.push_back(
				Plane(mFrustumPlanes[FRUSTUM_PLANE_RIGHT].normal, br.getOrigin()));
			outVolume->planes.push_back(
				Plane(mFrustumPlanes[FRUSTUM_PLANE_BOTTOM].normal, br.getOrigin()));
			outVolume->planes.push_back(
				Plane(mFrustumPlanes[FRUSTUM_PLANE_LEFT].normal, ul.getOrigin()));
			

		}

		// near & far plane applicable to both projection types
		outVolume->planes.push_back(getFrustumPlane(FRUSTUM_PLANE_NEAR));
		if (includeFarPlane)
			outVolume->planes.push_back(getFrustumPlane(FRUSTUM_PLANE_FAR));
	}
    // -------------------------------------------------------------------
    void Camera::setWindow (Real Left, Real Top, Real Right, Real Bottom)
    {
        mWLeft = Left;
        mWTop = Top;
        mWRight = Right;
        mWBottom = Bottom;

        mWindowSet = true;
        mRecalcWindow = true;
    }
    // -------------------------------------------------------------------
    void Camera::resetWindow ()
    {
        mWindowSet = false;
    }
    // -------------------------------------------------------------------
    void Camera::setWindowImpl() const
    {
        if (!mWindowSet || !mRecalcWindow)
            return;

        // Calculate general projection parameters
        Real vpLeft, vpRight, vpBottom, vpTop;
        calcProjectionParameters(vpLeft, vpRight, vpBottom, vpTop);

        Real vpWidth = vpRight - vpLeft;
        Real vpHeight = vpTop - vpBottom;

        Real wvpLeft   = vpLeft + mWLeft * vpWidth;
        Real wvpRight  = vpLeft + mWRight * vpWidth;
        Real wvpTop    = vpTop - mWTop * vpHeight;
        Real wvpBottom = vpTop - mWBottom * vpHeight;

        Vector3 vp_ul (wvpLeft, wvpTop, -mNearDist);
        Vector3 vp_ur (wvpRight, wvpTop, -mNearDist);
        Vector3 vp_bl (wvpLeft, wvpBottom, -mNearDist);
        Vector3 vp_br (wvpRight, wvpBottom, -mNearDist);

        Matrix4 inv = mViewMatrix.inverseAffine();

        Vector3 vw_ul = inv.transformAffine(vp_ul);
        Vector3 vw_ur = inv.transformAffine(vp_ur);
        Vector3 vw_bl = inv.transformAffine(vp_bl);
        Vector3 vw_br = inv.transformAffine(vp_br);

		mWindowClipPlanes.clear();
        if (mProjType == PT_PERSPECTIVE)
        {
            Vector3 position = getPositionForViewUpdate();
            mWindowClipPlanes.push_back(Plane(position, vw_bl, vw_ul));
            mWindowClipPlanes.push_back(Plane(position, vw_ul, vw_ur));
            mWindowClipPlanes.push_back(Plane(position, vw_ur, vw_br));
            mWindowClipPlanes.push_back(Plane(position, vw_br, vw_bl));
        }
        else
        {
            Vector3 x_axis(inv[0][0], inv[0][1], inv[0][2]);
            Vector3 y_axis(inv[1][0], inv[1][1], inv[1][2]);
            x_axis.normalise();
            y_axis.normalise();
            mWindowClipPlanes.push_back(Plane( x_axis, vw_bl));
            mWindowClipPlanes.push_back(Plane(-x_axis, vw_ur));
            mWindowClipPlanes.push_back(Plane( y_axis, vw_bl));
            mWindowClipPlanes.push_back(Plane(-y_axis, vw_ur));
        }

        mRecalcWindow = false;

    }
    // -------------------------------------------------------------------
    const std::vector<Plane>& Camera::getWindowPlanes(void) const
    {
        updateView();
        setWindowImpl();
        return mWindowClipPlanes;
    }
    // -------------------------------------------------------------------
    Real Camera::getBoundingRadius(void) const
    {
        // return a little bigger than the near distance
        // just to keep things just outside
        return mNearDist * 1.5;

    }
    //-----------------------------------------------------------------------
    const Vector3& Camera::getPositionForViewUpdate(void) const
    {
        // Note no update, because we're calling this from the update!
        return mRealPosition;
    }
    //-----------------------------------------------------------------------
    const Quaternion& Camera::getOrientationForViewUpdate(void) const
    {
        return mRealOrientation;
    }
    //-----------------------------------------------------------------------
    bool Camera::getAutoAspectRatio(void) const
    {
        return mAutoAspectRatio;
    }
    //-----------------------------------------------------------------------
    void Camera::setAutoAspectRatio(bool autoratio)
    {
        mAutoAspectRatio = autoratio;
    }
	//-----------------------------------------------------------------------
	bool Camera::isVisible(const AxisAlignedBox& bound, FrustumPlane* culledBy) const
	{
		if (mCullFrustum)
		{
			return mCullFrustum->isVisible(bound, culledBy);
		}
		else
		{
			return Frustum::isVisible(bound, culledBy);
		}
	}
	//-----------------------------------------------------------------------
	bool Camera::isVisible(const Sphere& bound, FrustumPlane* culledBy) const
	{
		if (mCullFrustum)
		{
			return mCullFrustum->isVisible(bound, culledBy);
		}
		else
		{
			return Frustum::isVisible(bound, culledBy);
		}
	}
	//-----------------------------------------------------------------------
	bool Camera::isVisible(const Vector3& vert, FrustumPlane* culledBy) const
	{
		if (mCullFrustum)
		{
			return mCullFrustum->isVisible(vert, culledBy);
		}
		else
		{
			return Frustum::isVisible(vert, culledBy);
		}
	}
	//-----------------------------------------------------------------------
	const Vector3* Camera::getWorldSpaceCorners(void) const
	{
		if (mCullFrustum)
		{
			return mCullFrustum->getWorldSpaceCorners();
		}
		else
		{
			return Frustum::getWorldSpaceCorners();
		}
	}
	//-----------------------------------------------------------------------
	const Plane& Camera::getFrustumPlane( unsigned short plane ) const
	{
		if (mCullFrustum)
		{
			return mCullFrustum->getFrustumPlane(plane);
		}
		else
		{
			return Frustum::getFrustumPlane(plane);
		}
	}
	//-----------------------------------------------------------------------
	bool Camera::projectSphere(const Sphere& sphere, 
		Real* left, Real* top, Real* right, Real* bottom) const
	{
		if (mCullFrustum)
		{
			return mCullFrustum->projectSphere(sphere, left, top, right, bottom);
		}
		else
		{
			return Frustum::projectSphere(sphere, left, top, right, bottom);
		}
	}
	//-----------------------------------------------------------------------
	Real Camera::getNearClipDistance(void) const
	{
		if (mCullFrustum)
		{
			return mCullFrustum->getNearClipDistance();
		}
		else
		{
			return Frustum::getNearClipDistance();
		}
	}
	//-----------------------------------------------------------------------
	Real Camera::getFarClipDistance(void) const
	{
		if (mCullFrustum)
		{
			return mCullFrustum->getFarClipDistance();
		}
		else
		{
			return Frustum::getFarClipDistance();
		}
	}
	//-----------------------------------------------------------------------
	const Matrix4& Camera::getViewMatrix(void) const
	{
		if (mCullFrustum)
		{
			return mCullFrustum->getViewMatrix();
		}
		else
		{
			return Frustum::getViewMatrix();
		}
	}
	//-----------------------------------------------------------------------
	const Matrix4& Camera::getViewMatrix(bool ownFrustumOnly) const
	{
		if (ownFrustumOnly)
		{
			return Frustum::getViewMatrix();
		}
		else
		{
			return getViewMatrix();
		}
	}
	//-----------------------------------------------------------------------
	//_______________________________________________________
	//|														|
	//|	getRayForwardIntersect								|
	//|	-----------------------------						|
	//|	get the intersections of frustum rays with a plane	|
	//| of interest.  The plane is assumed to have constant	|
	//| z.  If this is not the case, rays					|
	//| should be rotated beforehand to work in a			|
	//| coordinate system in which this is true.			|
	//|_____________________________________________________|
	//
	std::vector<Vector4> Camera::getRayForwardIntersect(const Vector3& anchor, const Vector3 *dir, Real planeOffset) const
	{
		std::vector<Vector4> res;

		if(!dir)
			return res;

		int infpt[4] = {0, 0, 0, 0}; // 0=finite, 1=infinite, 2=straddles infinity
		Vector3 vec[4];

		// find how much the anchor point must be displaced in the plane's
		// constant variable
		Real delta = planeOffset - anchor.z;

		// now set the intersection point and note whether it is a 
		// point at infinity or straddles infinity
		unsigned int i;
		for (i=0; i<4; i++)
		{
			Real test = dir[i].z * delta;
			if (test == 0.0) {
				vec[i] = dir[i];
				infpt[i] = 1;
			}
			else {
				Real lambda = delta / dir[i].z;
				vec[i] = anchor + (lambda * dir[i]);
				if(test < 0.0)
					infpt[i] = 2;
			}
		}

		for (i=0; i<4; i++)
		{
			// store the finite intersection points
			if (infpt[i] == 0)
				res.push_back(Vector4(vec[i].x, vec[i].y, vec[i].z, 1.0));
			else
			{
				// handle the infinite points of intersection;
				// cases split up into the possible frustum planes 
				// pieces which may contain a finite intersection point
				int nextind = (i+1) % 4;
				int prevind = (i+3) % 4;
				if ((infpt[prevind] == 0) || (infpt[nextind] == 0))
				{
					if (infpt[i] == 1)
						res.push_back(Vector4(vec[i].x, vec[i].y, vec[i].z, 0.0));
					else
					{
						// handle the intersection points that straddle infinity (back-project)
						if(infpt[prevind] == 0) 
						{
							Vector3 temp = vec[prevind] - vec[i];
							res.push_back(Vector4(temp.x, temp.y, temp.z, 0.0));
						}
						if(infpt[nextind] == 0)
						{
							Vector3 temp = vec[nextind] - vec[i];
							res.push_back(Vector4(temp.x, temp.y, temp.z, 0.0));
						}
					}
				} // end if we need to add an intersection point to the list
			} // end if infinite point needs to be considered
		} // end loop over frustun corners

		// we end up with either 0, 3, 4, or 5 intersection points

		return res;
	}

	//_______________________________________________________
	//|														|
	//|	forwardIntersect									|
	//|	-----------------------------						|
	//|	Forward intersect the camera's frustum rays with	|
	//| a specified plane of interest.						|
	//| Note that if the frustum rays shoot out and would	|
	//| back project onto the plane, this means the forward	|
	//| intersection of the frustum would occur at the		|
	//| line at infinity.									|
	//|_____________________________________________________|
	//
	void Camera::forwardIntersect(const Plane& worldPlane, std::vector<Vector4>* intersect3d) const
	{
		if(!intersect3d)
			return;

		Vector3 trCorner = getWorldSpaceCorners()[0];
		Vector3 tlCorner = getWorldSpaceCorners()[1];
		Vector3 blCorner = getWorldSpaceCorners()[2];
		Vector3 brCorner = getWorldSpaceCorners()[3];

		// need some sort of rotation that will bring the plane normal to the z axis
		Plane pval = worldPlane;
		if(pval.normal.z < 0.0)
		{
			pval.normal *= -1.0;
			pval.d *= -1.0;
		}
		Quaternion invPlaneRot = pval.normal.getRotationTo(Vector3::UNIT_Z);

		// get rotated light
		Vector3 lPos = invPlaneRot * getDerivedPosition();
		Vector3 vec[4];
		vec[0] = invPlaneRot * trCorner - lPos;
		vec[1] = invPlaneRot * tlCorner - lPos; 
		vec[2] = invPlaneRot * blCorner - lPos; 
		vec[3] = invPlaneRot * brCorner - lPos; 

		// compute intersection points on plane
		std::vector<Vector4> iPnt = getRayForwardIntersect(lPos, vec, -pval.d);


		// return wanted data
		if(intersect3d) 
		{
			Quaternion planeRot = invPlaneRot.Inverse();
			(*intersect3d).clear();
			for(unsigned int i=0; i<iPnt.size(); i++)
			{
				Vector3 intersection = planeRot * Vector3(iPnt[i].x, iPnt[i].y, iPnt[i].z);
				(*intersect3d).push_back(Vector4(intersection.x, intersection.y, intersection.z, iPnt[i].w));
			}
		}
	}
	//-----------------------------------------------------------------------
	void Camera::synchroniseBaseSettingsWith(const Camera* cam)
	{
		this->setPosition(cam->getPosition());
		this->setProjectionType(cam->getProjectionType());
		this->setOrientation(cam->getOrientation());
		this->setAspectRatio(cam->getAspectRatio());
		this->setNearClipDistance(cam->getNearClipDistance());
		this->setFarClipDistance(cam->getFarClipDistance());
		this->setLodCamera(cam->getLodCamera());
		this->setUseRenderingDistance(cam->getUseRenderingDistance());
		this->setCullingFrustum(cam->getCullingFrustum());


	}


} // namespace Ogre
