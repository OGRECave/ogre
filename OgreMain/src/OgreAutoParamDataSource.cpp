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

#include "OgreAutoParamDataSource.h"
#include "OgreRenderable.h"
#include "OgreCamera.h"
#include "OgreRenderTarget.h"
#include "OgreControllerManager.h"
#include "OgreMath.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"

namespace Ogre {
    const Matrix4 PROJECTIONCLIPSPACE2DTOIMAGESPACE_PERSPECTIVE(
        0.5,    0,    0,  0.5, 
        0,   -0.5,    0,  0.5, 
        0,      0,    1,    0,
        0,      0,    0,    1);

    //-----------------------------------------------------------------------------
    AutoParamDataSource::AutoParamDataSource()
        : mWorldMatrixDirty(true),
         mViewMatrixDirty(true),
         mProjMatrixDirty(true),
         mWorldViewMatrixDirty(true),
         mViewProjMatrixDirty(true),
         mWorldViewProjMatrixDirty(true),
         mInverseWorldMatrixDirty(true),
         mInverseWorldViewMatrixDirty(true),
         mInverseViewMatrixDirty(true),
         mInverseTransposeWorldMatrixDirty(true),
         mInverseTransposeWorldViewMatrixDirty(true),
		 mCameraPositionDirty(true),
         mCameraPositionObjectSpaceDirty(true),
		 mSceneDepthRangeDirty(true),
		 mLodCameraPositionDirty(true),
		 mLodCameraPositionObjectSpaceDirty(true),
         mCurrentRenderable(0),
         mCurrentCamera(0), 
		 mCameraRelativeRendering(false),
         mCurrentRenderTarget(0),
         mCurrentViewport(0), 
		 mCurrentSceneManager(0),
		 mMainCamBoundsInfo(0),
         mCurrentPass(0)
    {
        mBlankLight.setDiffuseColour(ColourValue::Black);
        mBlankLight.setSpecularColour(ColourValue::Black);
        mBlankLight.setAttenuation(0,1,0,0);
		for(size_t i = 0; i < OGRE_MAX_SIMULTANEOUS_LIGHTS; ++i)
		{
			mTextureViewProjMatrixDirty[i] = true;
			mTextureWorldViewProjMatrixDirty[i] = true;
			mSpotlightViewProjMatrixDirty[i] = true;
			mSpotlightWorldViewProjMatrixDirty[i] = true;
			mCurrentTextureProjector[i] = 0;
			mShadowCamDepthRangesDirty[i] = false;
		}

    }
    //-----------------------------------------------------------------------------
    AutoParamDataSource::~AutoParamDataSource()
    {
    }
	//-----------------------------------------------------------------------------
    const Light& AutoParamDataSource::getLight(size_t index) const
    {
        // If outside light range, return a blank light to ensure zeroised for program
		if (index < mCurrentLightList->size())
		{
			return *((*mCurrentLightList)[index]);
		}
		else
        {
            return mBlankLight;
        }        
    }
    //-----------------------------------------------------------------------------
    void AutoParamDataSource::setCurrentRenderable(const Renderable* rend)
    {
		mCurrentRenderable = rend;
		mWorldMatrixDirty = true;
        mViewMatrixDirty = true;
        mProjMatrixDirty = true;
		mWorldViewMatrixDirty = true;
        mViewProjMatrixDirty = true;
		mWorldViewProjMatrixDirty = true;
		mInverseWorldMatrixDirty = true;
        mInverseViewMatrixDirty = true;
		mInverseWorldViewMatrixDirty = true;
		mInverseTransposeWorldMatrixDirty = true;
		mInverseTransposeWorldViewMatrixDirty = true;
		mCameraPositionObjectSpaceDirty = true;
		mLodCameraPositionObjectSpaceDirty = true;
		for(size_t i = 0; i < OGRE_MAX_SIMULTANEOUS_LIGHTS; ++i)
		{
			mTextureWorldViewProjMatrixDirty[i] = true;
			mSpotlightWorldViewProjMatrixDirty[i] = true;
		}

    }
    //-----------------------------------------------------------------------------
    void AutoParamDataSource::setCurrentCamera(const Camera* cam, bool useCameraRelative)
    {
        mCurrentCamera = cam;
		mCameraRelativeRendering = useCameraRelative;
		mCameraRelativePosition = cam->getDerivedPosition();
        mViewMatrixDirty = true;
        mProjMatrixDirty = true;
        mWorldViewMatrixDirty = true;
        mViewProjMatrixDirty = true;
        mWorldViewProjMatrixDirty = true;
        mInverseViewMatrixDirty = true;
        mInverseWorldViewMatrixDirty = true;
        mInverseTransposeWorldViewMatrixDirty = true;
        mCameraPositionObjectSpaceDirty = true;
        mCameraPositionDirty = true;
		mLodCameraPositionObjectSpaceDirty = true;
		mLodCameraPositionDirty = true;
    }
    //-----------------------------------------------------------------------------
    void AutoParamDataSource::setCurrentLightList(const LightList* ll)
    {
        mCurrentLightList = ll;
		for(size_t i = 0; i < ll->size() && i < OGRE_MAX_SIMULTANEOUS_LIGHTS; ++i)
		{
			mSpotlightViewProjMatrixDirty[i] = true;
			mSpotlightWorldViewProjMatrixDirty[i] = true;
		}

    }
	//---------------------------------------------------------------------
	float AutoParamDataSource::getLightNumber(size_t index) const
	{
		return static_cast<float>(getLight(index)._getIndexInFrame());
	}
	//-----------------------------------------------------------------------------
	const ColourValue& AutoParamDataSource::getLightDiffuseColour(size_t index) const
	{
		return getLight(index).getDiffuseColour();
	}
	//-----------------------------------------------------------------------------
	const ColourValue& AutoParamDataSource::getLightSpecularColour(size_t index) const
	{
		return getLight(index).getSpecularColour();
	}
	//-----------------------------------------------------------------------------
	const ColourValue AutoParamDataSource::getLightDiffuseColourWithPower(size_t index) const
	{
		const Light& l = getLight(index);
		ColourValue scaled(l.getDiffuseColour());
		Real power = l.getPowerScale();
		// scale, but not alpha
		scaled.r *= power;
		scaled.g *= power;
		scaled.b *= power;
		return scaled;
	}
	//-----------------------------------------------------------------------------
	const ColourValue AutoParamDataSource::getLightSpecularColourWithPower(size_t index) const
	{
		const Light& l = getLight(index);
		ColourValue scaled(l.getSpecularColour());
		Real power = l.getPowerScale();
		// scale, but not alpha
		scaled.r *= power;
		scaled.g *= power;
		scaled.b *= power;
		return scaled;
	}
	//-----------------------------------------------------------------------------
	const Vector3& AutoParamDataSource::getLightPosition(size_t index) const
	{
		return getLight(index).getDerivedPosition(true);
	}
	//-----------------------------------------------------------------------------
	Vector4 AutoParamDataSource::getLightAs4DVector(size_t index) const
	{
		return getLight(index).getAs4DVector(true);
	}
	//-----------------------------------------------------------------------------
	const Vector3& AutoParamDataSource::getLightDirection(size_t index) const
	{
		return getLight(index).getDerivedDirection();
	}
	//-----------------------------------------------------------------------------
	Real AutoParamDataSource::getLightPowerScale(size_t index) const
	{
		return getLight(index).getPowerScale();
	}
	//-----------------------------------------------------------------------------
	Vector4 AutoParamDataSource::getLightAttenuation(size_t index) const
	{
		// range, const, linear, quad
        const Light& l = getLight(index);
		return Vector4(l.getAttenuationRange(),
				       l.getAttenuationConstant(),
					   l.getAttenuationLinear(),
					   l.getAttenuationQuadric());
	}
	//-----------------------------------------------------------------------------
	Vector4 AutoParamDataSource::getSpotlightParams(size_t index) const
	{
		// inner, outer, fallof, isSpot
		const Light& l = getLight(index);
		if (l.getType() == Light::LT_SPOTLIGHT)
		{
			return Vector4(Math::Cos(l.getSpotlightInnerAngle().valueRadians() * 0.5),
						   Math::Cos(l.getSpotlightOuterAngle().valueRadians() * 0.5),
						   l.getSpotlightFalloff(),
						   1.0);
		}
		else
		{
			// Use safe values which result in no change to point & dir light calcs
			// The spot factor applied to the usual lighting calc is 
			// pow((dot(spotDir, lightDir) - y) / (x - y), z)
			// Therefore if we set z to 0.0f then the factor will always be 1
			// since pow(anything, 0) == 1
			// However we also need to ensure we don't overflow because of the division
			// therefore set x = 1 and y = 0 so divisor doesn't change scale
			return Vector4(1.0, 0.0, 0.0, 1.0); // since the main op is pow(.., vec4.z), this will result in 1.0
		}
	}
	//-----------------------------------------------------------------------------
	void AutoParamDataSource::setMainCamBoundsInfo(VisibleObjectsBoundsInfo* info)
	{
		mMainCamBoundsInfo = info;
		mSceneDepthRangeDirty = true;
	}
	//-----------------------------------------------------------------------------
	void AutoParamDataSource::setCurrentSceneManager(const SceneManager* sm)
	{
		mCurrentSceneManager = sm;
	}
    //-----------------------------------------------------------------------------
    void AutoParamDataSource::setWorldMatrices(const Matrix4* m, size_t count)
    {
        mWorldMatrixArray = m;
        mWorldMatrixCount = count;
        mWorldMatrixDirty = false;
    }
    //-----------------------------------------------------------------------------
    const Matrix4& AutoParamDataSource::getWorldMatrix(void) const
    {
        if (mWorldMatrixDirty)
        {
            mWorldMatrixArray = mWorldMatrix;
            mCurrentRenderable->getWorldTransforms(mWorldMatrix);
            mWorldMatrixCount = mCurrentRenderable->getNumWorldTransforms();
			if (mCameraRelativeRendering)
			{
				for (size_t i = 0; i < mWorldMatrixCount; ++i)
				{
					mWorldMatrix[i].setTrans(mWorldMatrix[i].getTrans() - mCameraRelativePosition);
				}
			}
            mWorldMatrixDirty = false;
        }
        return mWorldMatrixArray[0];
    }
    //-----------------------------------------------------------------------------
    size_t AutoParamDataSource::getWorldMatrixCount(void) const
    {
		// trigger derivation
		getWorldMatrix();
        return mWorldMatrixCount;
    }
    //-----------------------------------------------------------------------------
    const Matrix4* AutoParamDataSource::getWorldMatrixArray(void) const
    {
		// trigger derivation
		getWorldMatrix();
        return mWorldMatrixArray;
    }
    //-----------------------------------------------------------------------------
    const Matrix4& AutoParamDataSource::getViewMatrix(void) const
    {
        if (mViewMatrixDirty)
        {
            if (mCurrentRenderable && mCurrentRenderable->getUseIdentityView())
                mViewMatrix = Matrix4::IDENTITY;
            else
			{
                mViewMatrix = mCurrentCamera->getViewMatrix(true);
				if (mCameraRelativeRendering)
				{
					mViewMatrix.setTrans(Vector3::ZERO);
				}

			}
            mViewMatrixDirty = false;
        }
        return mViewMatrix;
    }
    //-----------------------------------------------------------------------------
    const Matrix4& AutoParamDataSource::getViewProjectionMatrix(void) const
    {
        if (mViewProjMatrixDirty)
        {
            mViewProjMatrix = getProjectionMatrix() * getViewMatrix();
            mViewProjMatrixDirty = false;
        }
        return mViewProjMatrix;
    }
    //-----------------------------------------------------------------------------
    const Matrix4& AutoParamDataSource::getProjectionMatrix(void) const
    {
        if (mProjMatrixDirty)
        {
            // NB use API-independent projection matrix since GPU programs
            // bypass the API-specific handedness and use right-handed coords
            if (mCurrentRenderable && mCurrentRenderable->getUseIdentityProjection())
            {
                // Use identity projection matrix, still need to take RS depth into account.
                RenderSystem* rs = Root::getSingleton().getRenderSystem();
                rs->_convertProjectionMatrix(Matrix4::IDENTITY, mProjectionMatrix, true);
            }
            else
            {
                mProjectionMatrix = mCurrentCamera->getProjectionMatrixWithRSDepth();
            }
            if (mCurrentRenderTarget && mCurrentRenderTarget->requiresTextureFlipping())
            {
                // Because we're not using setProjectionMatrix, this needs to be done here
                // Invert transformed y
                mProjectionMatrix[1][0] = -mProjectionMatrix[1][0];
                mProjectionMatrix[1][1] = -mProjectionMatrix[1][1];
                mProjectionMatrix[1][2] = -mProjectionMatrix[1][2];
                mProjectionMatrix[1][3] = -mProjectionMatrix[1][3];
            }
            mProjMatrixDirty = false;
        }
        return mProjectionMatrix;
    }
    //-----------------------------------------------------------------------------
    const Matrix4& AutoParamDataSource::getWorldViewMatrix(void) const
    {
        if (mWorldViewMatrixDirty)
        {
            mWorldViewMatrix = getViewMatrix().concatenateAffine(getWorldMatrix());
            mWorldViewMatrixDirty = false;
        }
        return mWorldViewMatrix;
    }
    //-----------------------------------------------------------------------------
    const Matrix4& AutoParamDataSource::getWorldViewProjMatrix(void) const
    {
        if (mWorldViewProjMatrixDirty)
        {
            mWorldViewProjMatrix = getProjectionMatrix() * getWorldViewMatrix();
            mWorldViewProjMatrixDirty = false;
        }
        return mWorldViewProjMatrix;
    }
    //-----------------------------------------------------------------------------
    const Matrix4& AutoParamDataSource::getInverseWorldMatrix(void) const
    {
        if (mInverseWorldMatrixDirty)
        {
            mInverseWorldMatrix = getWorldMatrix().inverseAffine();
            mInverseWorldMatrixDirty = false;
        }
        return mInverseWorldMatrix;
    }
    //-----------------------------------------------------------------------------
    const Matrix4& AutoParamDataSource::getInverseWorldViewMatrix(void) const
    {
        if (mInverseWorldViewMatrixDirty)
        {
            mInverseWorldViewMatrix = getWorldViewMatrix().inverseAffine();
            mInverseWorldViewMatrixDirty = false;
        }
        return mInverseWorldViewMatrix;
    }
    //-----------------------------------------------------------------------------
    const Matrix4& AutoParamDataSource::getInverseViewMatrix(void) const
    {
        if (mInverseViewMatrixDirty)
        {
            mInverseViewMatrix = getViewMatrix().inverseAffine();
            mInverseViewMatrixDirty = false;
        }
        return mInverseViewMatrix;
    }
    //-----------------------------------------------------------------------------
    const Matrix4& AutoParamDataSource::getInverseTransposeWorldMatrix(void) const
    {
        if (mInverseTransposeWorldMatrixDirty)
        {
            mInverseTransposeWorldMatrix = getInverseWorldMatrix().transpose();
            mInverseTransposeWorldMatrixDirty = false;
        }
        return mInverseTransposeWorldMatrix;
    }
    //-----------------------------------------------------------------------------
    const Matrix4& AutoParamDataSource::getInverseTransposeWorldViewMatrix(void) const
    {
        if (mInverseTransposeWorldViewMatrixDirty)
        {
            mInverseTransposeWorldViewMatrix = getInverseWorldViewMatrix().transpose();
            mInverseTransposeWorldViewMatrixDirty = false;
        }
        return mInverseTransposeWorldViewMatrix;
    }
    //-----------------------------------------------------------------------------
    const Vector4& AutoParamDataSource::getCameraPosition(void) const
    {
        if(mCameraPositionDirty)
        {
            Vector3 vec3 = mCurrentCamera->getDerivedPosition();
            mCameraPosition[0] = vec3[0];
            mCameraPosition[1] = vec3[1];
            mCameraPosition[2] = vec3[2];
            mCameraPosition[3] = 1.0;
            mCameraPositionDirty = false;
        }
        return mCameraPosition;
    }    
    //-----------------------------------------------------------------------------
    const Vector4& AutoParamDataSource::getCameraPositionObjectSpace(void) const
    {
        if (mCameraPositionObjectSpaceDirty)
        {
            mCameraPositionObjectSpace = 
                getInverseWorldMatrix().transformAffine(mCurrentCamera->getDerivedPosition());
            mCameraPositionObjectSpaceDirty = false;
        }
        return mCameraPositionObjectSpace;
    }
	//-----------------------------------------------------------------------------
	const Vector4& AutoParamDataSource::getLodCameraPosition(void) const
	{
		if(mLodCameraPositionDirty)
		{
			Vector3 vec3 = mCurrentCamera->getLodCamera()->getDerivedPosition();
			mLodCameraPosition[0] = vec3[0];
			mLodCameraPosition[1] = vec3[1];
			mLodCameraPosition[2] = vec3[2];
			mLodCameraPosition[3] = 1.0;
			mLodCameraPositionDirty = false;
		}
		return mLodCameraPosition;
	}
	//-----------------------------------------------------------------------------
	const Vector4& AutoParamDataSource::getLodCameraPositionObjectSpace(void) const
	{
		if (mLodCameraPositionObjectSpaceDirty)
		{
			mLodCameraPositionObjectSpace = 
				getInverseWorldMatrix().transformAffine(mCurrentCamera->getLodCamera()->getDerivedPosition());
			mLodCameraPositionObjectSpaceDirty = false;
		}
		return mLodCameraPositionObjectSpace;
	}
    //-----------------------------------------------------------------------------
	void AutoParamDataSource::setAmbientLightColour(const ColourValue& ambient)
	{
		mAmbientLight = ambient;
	}
	//---------------------------------------------------------------------
	float AutoParamDataSource::getLightCount() const
	{
		return static_cast<float>(mCurrentLightList->size());
	}
	//---------------------------------------------------------------------
	int AutoParamDataSource::getLightCastsShadows(size_t index) const
	{
		return static_cast<int>(getLight(index).getCastShadows() ? 1 : 0);
	}
    //-----------------------------------------------------------------------------
	const ColourValue& AutoParamDataSource::getAmbientLightColour(void) const
	{
		return mAmbientLight;
		
	}
    //-----------------------------------------------------------------------------
    void AutoParamDataSource::setCurrentPass(const Pass* pass)
    {
        mCurrentPass = pass;
    }
    //-----------------------------------------------------------------------------
    const Pass* AutoParamDataSource::getCurrentPass(void) const
    {
        return mCurrentPass;
    }
    //-----------------------------------------------------------------------------
    Vector4 AutoParamDataSource::getTextureSize(size_t index) const
    {
        Vector4 size = Vector4(1,1,1,1);

        if (index < mCurrentPass->getNumTextureUnitStates())
        {
            const TexturePtr& tex = mCurrentPass->getTextureUnitState(
				static_cast<unsigned short>(index))->_getTexturePtr();
            if (!tex.isNull())
            {
                size.x = tex->getWidth();
                size.y = tex->getHeight();
                size.z = tex->getDepth();
            }
        }

        return size;
    }
    //-----------------------------------------------------------------------------
    Vector4 AutoParamDataSource::getInverseTextureSize(size_t index) const
    {
        Vector4 size = getTextureSize(index);
        return 1 / size;
    }
    //-----------------------------------------------------------------------------
    Vector4 AutoParamDataSource::getPackedTextureSize(size_t index) const
    {
        Vector4 size = getTextureSize(index);
        return Vector4(size.x, size.y, 1 / size.x, 1 / size.y);
    }
    //-----------------------------------------------------------------------------
    const ColourValue& AutoParamDataSource::getSurfaceAmbientColour(void) const
    {
        return mCurrentPass->getAmbient();
    }
    //-----------------------------------------------------------------------------
    const ColourValue& AutoParamDataSource::getSurfaceDiffuseColour(void) const
    {
        return mCurrentPass->getDiffuse();
    }
    //-----------------------------------------------------------------------------
    const ColourValue& AutoParamDataSource::getSurfaceSpecularColour(void) const
    {
        return mCurrentPass->getSpecular();
    }
    //-----------------------------------------------------------------------------
    const ColourValue& AutoParamDataSource::getSurfaceEmissiveColour(void) const
    {
        return mCurrentPass->getSelfIllumination();
    }
    //-----------------------------------------------------------------------------
    Real AutoParamDataSource::getSurfaceShininess(void) const
    {
        return mCurrentPass->getShininess();
    }
    //-----------------------------------------------------------------------------
    ColourValue AutoParamDataSource::getDerivedAmbientLightColour(void) const
    {
        return getAmbientLightColour() * getSurfaceAmbientColour();
    }
    //-----------------------------------------------------------------------------
    ColourValue AutoParamDataSource::getDerivedSceneColour(void) const
    {
        ColourValue result = getDerivedAmbientLightColour() + getSurfaceEmissiveColour();
        result.a = getSurfaceDiffuseColour().a;
        return result;
    }
    //-----------------------------------------------------------------------------
    void AutoParamDataSource::setFog(FogMode mode, const ColourValue& colour,
        Real expDensity, Real linearStart, Real linearEnd)
    {
        (void)mode; // ignored
        mFogColour = colour;
        mFogParams.x = expDensity;
        mFogParams.y = linearStart;
        mFogParams.z = linearEnd;
        mFogParams.w = linearEnd != linearStart ? 1 / (linearEnd - linearStart) : 0;
    }
    //-----------------------------------------------------------------------------
    const ColourValue& AutoParamDataSource::getFogColour(void) const
    {
        return mFogColour;
    }
    //-----------------------------------------------------------------------------
    const Vector4& AutoParamDataSource::getFogParams(void) const
    {
        return mFogParams;
    }
    //-----------------------------------------------------------------------------
    void AutoParamDataSource::setTextureProjector(const Frustum* frust, size_t index = 0)
    {
		if (index < OGRE_MAX_SIMULTANEOUS_LIGHTS)
		{
			mCurrentTextureProjector[index] = frust;
			mTextureViewProjMatrixDirty[index] = true;
			mTextureWorldViewProjMatrixDirty[index] = true;
			mShadowCamDepthRangesDirty[index] = true;
		}

    }
    //-----------------------------------------------------------------------------
    const Matrix4& AutoParamDataSource::getTextureViewProjMatrix(size_t index) const
    {
		if (index < OGRE_MAX_SIMULTANEOUS_LIGHTS)
		{
			if (mTextureViewProjMatrixDirty[index] && mCurrentTextureProjector[index])
			{
				if (mCameraRelativeRendering)
				{
					// World positions are now going to be relative to the camera position
					// so we need to alter the projector view matrix to compensate
					Matrix4 viewMatrix;
					mCurrentTextureProjector[index]->calcViewMatrixRelative(
						mCurrentCamera->getDerivedPosition(), viewMatrix);
					mTextureViewProjMatrix[index] = 
						PROJECTIONCLIPSPACE2DTOIMAGESPACE_PERSPECTIVE * 
						mCurrentTextureProjector[index]->getProjectionMatrixWithRSDepth() * 
						viewMatrix;
				}
				else
				{
					mTextureViewProjMatrix[index] = 
						PROJECTIONCLIPSPACE2DTOIMAGESPACE_PERSPECTIVE * 
						mCurrentTextureProjector[index]->getProjectionMatrixWithRSDepth() * 
						mCurrentTextureProjector[index]->getViewMatrix();
				}
				mTextureViewProjMatrixDirty[index] = false;
			}
			return mTextureViewProjMatrix[index];
		}
		else
			return Matrix4::IDENTITY;
    }
	//-----------------------------------------------------------------------------
	const Matrix4& AutoParamDataSource::getTextureWorldViewProjMatrix(size_t index) const
	{
		if (index < OGRE_MAX_SIMULTANEOUS_LIGHTS)
		{
			if (mTextureWorldViewProjMatrixDirty[index] && mCurrentTextureProjector[index])
			{
				mTextureWorldViewProjMatrix[index] = 
					getTextureViewProjMatrix(index)	* getWorldMatrix();
				mTextureWorldViewProjMatrixDirty[index] = false;
			}
			return mTextureWorldViewProjMatrix[index];
		}
		else
			return Matrix4::IDENTITY;
	}
	//-----------------------------------------------------------------------------
	const Matrix4& AutoParamDataSource::getSpotlightViewProjMatrix(size_t index) const
	{
		if (index < OGRE_MAX_SIMULTANEOUS_LIGHTS)
		{
			const Light& l = getLight(index);

			if (&l != &mBlankLight && 
				l.getType() == Light::LT_SPOTLIGHT &&
				mSpotlightViewProjMatrixDirty[index])
			{
				Frustum frust;
				SceneNode dummyNode(0);
				dummyNode.attachObject(&frust);

				frust.setProjectionType(PT_PERSPECTIVE);
				frust.setFOVy(l.getSpotlightOuterAngle());
				frust.setAspectRatio(1.0f);
				// set near clip the same as main camera, since they are likely
				// to both reflect the nature of the scene
				frust.setNearClipDistance(mCurrentCamera->getNearClipDistance());
				// Calculate position, which same as spotlight position, in camera-relative coords if required
				dummyNode.setPosition(l.getDerivedPosition(true));
				// Calculate direction, which same as spotlight direction
				Vector3 dir = - l.getDerivedDirection(); // backwards since point down -z
				dir.normalise();
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
				dummyNode.setOrientation(q);

				// The view matrix here already includes camera-relative changes if necessary
				// since they are built into the frustum position
				mSpotlightViewProjMatrix[index] = 
					PROJECTIONCLIPSPACE2DTOIMAGESPACE_PERSPECTIVE * 
					frust.getProjectionMatrixWithRSDepth() * 
					frust.getViewMatrix();

				mSpotlightViewProjMatrixDirty[index] = false;
			}
			return mSpotlightViewProjMatrix[index];
		}
		else
			return Matrix4::IDENTITY;
	}
	//-----------------------------------------------------------------------------
	const Matrix4& AutoParamDataSource::getSpotlightWorldViewProjMatrix(size_t index) const
	{
		if (index < OGRE_MAX_SIMULTANEOUS_LIGHTS)
		{
			const Light& l = getLight(index);

			if (&l != &mBlankLight && 
				l.getType() == Light::LT_SPOTLIGHT &&
				mSpotlightWorldViewProjMatrixDirty[index])
			{
				mSpotlightWorldViewProjMatrix[index] = 
					getSpotlightViewProjMatrix(index) * getWorldMatrix();
				mSpotlightWorldViewProjMatrixDirty[index] = false;
			}
			return mSpotlightWorldViewProjMatrix[index];
		}
		else
			return Matrix4::IDENTITY;
	}
//-----------------------------------------------------------------------------
  const Matrix4& AutoParamDataSource::getTextureTransformMatrix(size_t index) const
  {
    // make sure the current pass is set
    assert(mCurrentPass && "current pass is NULL!");
    // check if there is a texture unit with the given index in the current pass
    if(index < mCurrentPass->getNumTextureUnitStates())
    {
      // texture unit existent, return its currently set transform
      return mCurrentPass->getTextureUnitState(static_cast<unsigned short>(index))->getTextureTransform();
    }
    else
    {
      // no such texture unit, return unity
      return Matrix4::IDENTITY;
    }
  }
    //-----------------------------------------------------------------------------
    void AutoParamDataSource::setCurrentRenderTarget(const RenderTarget* target)
    {
        mCurrentRenderTarget = target;
    }
    //-----------------------------------------------------------------------------
    const RenderTarget* AutoParamDataSource::getCurrentRenderTarget(void) const
    {
        return mCurrentRenderTarget;
    }
    //-----------------------------------------------------------------------------
    void AutoParamDataSource::setCurrentViewport(const Viewport* viewport)
    {
        mCurrentViewport = viewport;
    }
    //-----------------------------------------------------------------------------
	void AutoParamDataSource::setShadowDirLightExtrusionDistance(Real dist)
	{
		mDirLightExtrusionDistance = dist;
	}
    //-----------------------------------------------------------------------------
	Real AutoParamDataSource::getShadowExtrusionDistance(void) const
	{
		const Light& l = getLight(0); // only ever applies to one light at once
		if (l.getType() == Light::LT_DIRECTIONAL)
		{
			// use constant
			return mDirLightExtrusionDistance;
		}
		else
		{
			// Calculate based on object space light distance
			// compared to light attenuation range
			Vector3 objPos = getInverseWorldMatrix().transformAffine(l.getDerivedPosition(true));
			return l.getAttenuationRange() - objPos.length();
		}
	}
    //-----------------------------------------------------------------------------
    const Renderable* AutoParamDataSource::getCurrentRenderable(void) const
    {
        return mCurrentRenderable;
    }
	//-----------------------------------------------------------------------------
	Matrix4 AutoParamDataSource::getInverseViewProjMatrix(void) const
	{
		return this->getViewProjectionMatrix().inverse();
	}
	//-----------------------------------------------------------------------------
	Matrix4 AutoParamDataSource::getInverseTransposeViewProjMatrix(void) const
	{
		return this->getInverseViewProjMatrix().transpose();
	}
	//-----------------------------------------------------------------------------
	Matrix4 AutoParamDataSource::getTransposeViewProjMatrix(void) const
	{
		return this->getViewProjectionMatrix().transpose();
	}
	//-----------------------------------------------------------------------------
	Matrix4 AutoParamDataSource::getTransposeViewMatrix(void) const
	{
		return this->getViewMatrix().transpose();
	}
	//-----------------------------------------------------------------------------
	Matrix4 AutoParamDataSource::getInverseTransposeViewMatrix(void) const
	{
		return this->getInverseViewMatrix().transpose();
	}
	//-----------------------------------------------------------------------------
	Matrix4 AutoParamDataSource::getTransposeProjectionMatrix(void) const
	{
		return this->getProjectionMatrix().transpose();
	}
	//-----------------------------------------------------------------------------
	Matrix4 AutoParamDataSource::getInverseProjectionMatrix(void) const 
	{
		return this->getProjectionMatrix().inverse();
	}
	//-----------------------------------------------------------------------------
	Matrix4 AutoParamDataSource::getInverseTransposeProjectionMatrix(void) const
	{
		return this->getInverseProjectionMatrix().transpose();
	}
	//-----------------------------------------------------------------------------
	Matrix4 AutoParamDataSource::getTransposeWorldViewProjMatrix(void) const
	{
		return this->getWorldViewProjMatrix().transpose();
	}
	//-----------------------------------------------------------------------------
	Matrix4 AutoParamDataSource::getInverseWorldViewProjMatrix(void) const
	{
		return this->getWorldViewProjMatrix().inverse();
	}
	//-----------------------------------------------------------------------------
	Matrix4 AutoParamDataSource::getInverseTransposeWorldViewProjMatrix(void) const
	{
		return this->getInverseWorldViewProjMatrix().transpose();
	}
	//-----------------------------------------------------------------------------
	Matrix4 AutoParamDataSource::getTransposeWorldViewMatrix(void) const
	{
		return this->getWorldViewMatrix().transpose();
	}
	//-----------------------------------------------------------------------------
	Matrix4 AutoParamDataSource::getTransposeWorldMatrix(void) const
	{
		return this->getWorldMatrix().transpose();
	}
    //-----------------------------------------------------------------------------
    Real AutoParamDataSource::getTime(void) const
    {
        return ControllerManager::getSingleton().getElapsedTime();
    }
	//-----------------------------------------------------------------------------
	Real AutoParamDataSource::getTime_0_X(Real x) const
	{
		return fmod(this->getTime(), x);
	}
	//-----------------------------------------------------------------------------
	Real AutoParamDataSource::getCosTime_0_X(Real x) const
	{ 
		return cos(this->getTime_0_X(x)); 
	}
	//-----------------------------------------------------------------------------
	Real AutoParamDataSource::getSinTime_0_X(Real x) const
	{ 
		return sin(this->getTime_0_X(x)); 
	}
	//-----------------------------------------------------------------------------
	Real AutoParamDataSource::getTanTime_0_X(Real x) const
	{ 
		return tan(this->getTime_0_X(x)); 
	}
	//-----------------------------------------------------------------------------
	Vector4 AutoParamDataSource::getTime_0_X_packed(Real x) const
	{
		Real t = this->getTime_0_X(x);
		return Vector4(t, sin(t), cos(t), tan(t));
	}
	//-----------------------------------------------------------------------------
	Real AutoParamDataSource::getTime_0_1(Real x) const
	{ 
		return this->getTime_0_X(x)/x; 
	}
	//-----------------------------------------------------------------------------
	Real AutoParamDataSource::getCosTime_0_1(Real x) const
	{ 
		return cos(this->getTime_0_1(x)); 
	}
	//-----------------------------------------------------------------------------
	Real AutoParamDataSource::getSinTime_0_1(Real x) const
	{ 
		return sin(this->getTime_0_1(x)); 
	}
	//-----------------------------------------------------------------------------
	Real AutoParamDataSource::getTanTime_0_1(Real x) const
	{ 
		return tan(this->getTime_0_1(x)); 
	}
	//-----------------------------------------------------------------------------
	Vector4 AutoParamDataSource::getTime_0_1_packed(Real x) const
	{
		Real t = this->getTime_0_1(x);
		return Vector4(t, sin(t), cos(t), tan(t));
	}
	//-----------------------------------------------------------------------------
	Real AutoParamDataSource::getTime_0_2Pi(Real x) const
	{ 
		return this->getTime_0_X(x)/x*2*Math::PI; 
	}
	//-----------------------------------------------------------------------------
	Real AutoParamDataSource::getCosTime_0_2Pi(Real x) const
	{ 
		return cos(this->getTime_0_2Pi(x)); 
	}
	//-----------------------------------------------------------------------------
	Real AutoParamDataSource::getSinTime_0_2Pi(Real x) const
	{ 
		return sin(this->getTime_0_2Pi(x)); 
	}
	//-----------------------------------------------------------------------------
	Real AutoParamDataSource::getTanTime_0_2Pi(Real x) const
	{ 
		return tan(this->getTime_0_2Pi(x)); 
	}
	//-----------------------------------------------------------------------------
	Vector4 AutoParamDataSource::getTime_0_2Pi_packed(Real x) const
	{
		Real t = this->getTime_0_2Pi(x);
		return Vector4(t, sin(t), cos(t), tan(t));
	}
	//-----------------------------------------------------------------------------
	Real AutoParamDataSource::getFrameTime(void) const
	{
		return ControllerManager::getSingleton().getFrameTimeSource()->getValue();
	}
	//-----------------------------------------------------------------------------
	Real AutoParamDataSource::getFPS() const
	{
		return mCurrentRenderTarget->getLastFPS();
	}
	//-----------------------------------------------------------------------------
	Real AutoParamDataSource::getViewportWidth() const
	{ 
		return mCurrentViewport->getActualWidth(); 
	}
	//-----------------------------------------------------------------------------
	Real AutoParamDataSource::getViewportHeight() const
	{ 
		return mCurrentViewport->getActualHeight(); 
	}
	//-----------------------------------------------------------------------------
	Real AutoParamDataSource::getInverseViewportWidth() const
	{ 
		return 1.0f/mCurrentViewport->getActualWidth(); 
	}
	//-----------------------------------------------------------------------------
	Real AutoParamDataSource::getInverseViewportHeight() const
	{ 
		return 1.0f/mCurrentViewport->getActualHeight(); 
	}
	//-----------------------------------------------------------------------------
	Vector3 AutoParamDataSource::getViewDirection() const
	{
		return mCurrentCamera->getDerivedDirection();
	}
	//-----------------------------------------------------------------------------
	Vector3 AutoParamDataSource::getViewSideVector() const
	{ 
		return mCurrentCamera->getDerivedRight();
	}
	//-----------------------------------------------------------------------------
	Vector3 AutoParamDataSource::getViewUpVector() const
	{ 
		return mCurrentCamera->getDerivedUp();
	}
	//-----------------------------------------------------------------------------
	Real AutoParamDataSource::getFOV() const
	{ 
		return mCurrentCamera->getFOVy().valueRadians(); 
	}
	//-----------------------------------------------------------------------------
	Real AutoParamDataSource::getNearClipDistance() const
	{ 
		return mCurrentCamera->getNearClipDistance(); 
	}
	//-----------------------------------------------------------------------------
	Real AutoParamDataSource::getFarClipDistance() const
	{ 
		return mCurrentCamera->getFarClipDistance(); 
	}
	//-----------------------------------------------------------------------------
    int AutoParamDataSource::getPassNumber(void) const
    {
        return mPassNumber;
    }
	//-----------------------------------------------------------------------------
    void AutoParamDataSource::setPassNumber(const int passNumber)
    {
        mPassNumber = passNumber;
    }
	//-----------------------------------------------------------------------------
    void AutoParamDataSource::incPassNumber(void)
    {
        ++mPassNumber;
    }
	//-----------------------------------------------------------------------------
	const Vector4& AutoParamDataSource::getSceneDepthRange() const
	{
		if (mSceneDepthRangeDirty)
		{
			// calculate depth information
			mSceneDepthRange.x = mMainCamBoundsInfo->minDistance;
			mSceneDepthRange.y = mMainCamBoundsInfo->maxDistance;
			mSceneDepthRange.z = mMainCamBoundsInfo->maxDistance - mMainCamBoundsInfo->minDistance;
			mSceneDepthRange.w = 1.0f / mSceneDepthRange.z;
			mSceneDepthRangeDirty = false;
		}

		return mSceneDepthRange;

	}
	//-----------------------------------------------------------------------------
	const Vector4& AutoParamDataSource::getShadowSceneDepthRange(size_t index) const
	{
		static Vector4 dummy(0, 100000, 100000, 1/100000);

		if (!mCurrentSceneManager->isShadowTechniqueTextureBased())
			return dummy;

		if (index < OGRE_MAX_SIMULTANEOUS_LIGHTS)
		{
			if (mShadowCamDepthRangesDirty[index] && mCurrentTextureProjector[index])
			{
				const VisibleObjectsBoundsInfo& info = 
					mCurrentSceneManager->getVisibleObjectsBoundsInfo(
						(Camera*)mCurrentTextureProjector[index]);

				Real depthRange = info.maxDistance - info.minDistance;
				if (depthRange > std::numeric_limits<Real>::epsilon())
				{
					mShadowCamDepthRanges[index] = Vector4(
						info.minDistance,
						info.maxDistance,
						depthRange,
						1.0f / depthRange);
				}
				else
				{
					mShadowCamDepthRanges[index] = dummy;
				}

				mShadowCamDepthRangesDirty[index] = false;
			}
			return mShadowCamDepthRanges[index];
		}
		else
			return dummy;
	}
	//---------------------------------------------------------------------
	const ColourValue& AutoParamDataSource::getShadowColour() const
	{
		return mCurrentSceneManager->getShadowColour();
	}

}

