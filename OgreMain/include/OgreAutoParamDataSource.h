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
#ifndef __AutoParamDataSource_H_
#define __AutoParamDataSource_H_

#include "OgrePrerequisites.h"
#include "OgreCommon.h"
#include "OgreMatrix4.h"
#include "OgreVector4.h"
#include "OgreLight.h"
#include "OgreColourValue.h"

namespace Ogre {

	// forward decls
	struct VisibleObjectsBoundsInfo;
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Materials
	*  @{
	*/


    /** This utility class is used to hold the information used to generate the matrices
    and other information required to automatically populate GpuProgramParameters.
    @remarks
        This class exercises a lazy-update scheme in order to avoid having to update all
        the information a GpuProgramParameters class could possibly want all the time. 
        It relies on the SceneManager to update it when the base data has changed, and
        will calculate concatenated matrices etc only when required, passing back precalculated
        matrices when they are requested more than once when the underlying information has
        not altered.
    */
	class _OgreExport AutoParamDataSource : public SceneMgtAlloc
    {
    protected:
		const Light& getLight(size_t index) const;
        mutable Matrix4 mWorldMatrix[256];
        mutable size_t mWorldMatrixCount;
        mutable const Matrix4* mWorldMatrixArray;
        mutable Matrix4 mWorldViewMatrix;
        mutable Matrix4 mViewProjMatrix;
        mutable Matrix4 mWorldViewProjMatrix;
        mutable Matrix4 mInverseWorldMatrix;
        mutable Matrix4 mInverseWorldViewMatrix;
        mutable Matrix4 mInverseViewMatrix;
        mutable Matrix4 mInverseTransposeWorldMatrix;
        mutable Matrix4 mInverseTransposeWorldViewMatrix;
		mutable Vector4 mCameraPosition;
        mutable Vector4 mCameraPositionObjectSpace;
        mutable Matrix4 mTextureViewProjMatrix[OGRE_MAX_SIMULTANEOUS_LIGHTS];
		mutable Matrix4 mTextureWorldViewProjMatrix[OGRE_MAX_SIMULTANEOUS_LIGHTS];
		mutable Matrix4 mSpotlightViewProjMatrix[OGRE_MAX_SIMULTANEOUS_LIGHTS];
		mutable Matrix4 mSpotlightWorldViewProjMatrix[OGRE_MAX_SIMULTANEOUS_LIGHTS];
		mutable Vector4 mShadowCamDepthRanges[OGRE_MAX_SIMULTANEOUS_LIGHTS];
        mutable Matrix4 mViewMatrix;
        mutable Matrix4 mProjectionMatrix;
		mutable Real mDirLightExtrusionDistance;
		mutable Vector4 mLodCameraPosition;
		mutable Vector4 mLodCameraPositionObjectSpace;

        mutable bool mWorldMatrixDirty;
        mutable bool mViewMatrixDirty;
        mutable bool mProjMatrixDirty;
        mutable bool mWorldViewMatrixDirty;
        mutable bool mViewProjMatrixDirty;
        mutable bool mWorldViewProjMatrixDirty;
        mutable bool mInverseWorldMatrixDirty;
        mutable bool mInverseWorldViewMatrixDirty;
        mutable bool mInverseViewMatrixDirty;
        mutable bool mInverseTransposeWorldMatrixDirty;
        mutable bool mInverseTransposeWorldViewMatrixDirty;
		mutable bool mCameraPositionDirty;
        mutable bool mCameraPositionObjectSpaceDirty;
        mutable bool mTextureViewProjMatrixDirty[OGRE_MAX_SIMULTANEOUS_LIGHTS];
		mutable bool mTextureWorldViewProjMatrixDirty[OGRE_MAX_SIMULTANEOUS_LIGHTS];
		mutable bool mSpotlightViewProjMatrixDirty[OGRE_MAX_SIMULTANEOUS_LIGHTS];
		mutable bool mSpotlightWorldViewProjMatrixDirty[OGRE_MAX_SIMULTANEOUS_LIGHTS];
		mutable bool mShadowCamDepthRangesDirty[OGRE_MAX_SIMULTANEOUS_LIGHTS];
		mutable ColourValue mAmbientLight;
        mutable ColourValue mFogColour;
        mutable Vector4 mFogParams;
        mutable int mPassNumber;
		mutable Vector4 mSceneDepthRange;
		mutable bool mSceneDepthRangeDirty;
		mutable bool mLodCameraPositionDirty;
		mutable bool mLodCameraPositionObjectSpaceDirty;

        const Renderable* mCurrentRenderable;
        const Camera* mCurrentCamera;
		bool mCameraRelativeRendering;
		Vector3 mCameraRelativePosition;
        const LightList* mCurrentLightList;
        const Frustum* mCurrentTextureProjector[OGRE_MAX_SIMULTANEOUS_LIGHTS];
        const RenderTarget* mCurrentRenderTarget;
        const Viewport* mCurrentViewport;
		const SceneManager* mCurrentSceneManager;
		const VisibleObjectsBoundsInfo* mMainCamBoundsInfo;
        const Pass* mCurrentPass;

        Light mBlankLight;
    public:
        AutoParamDataSource();
        virtual ~AutoParamDataSource();
        /** Updates the current renderable */
        virtual void setCurrentRenderable(const Renderable* rend);
        /** Sets the world matrices, avoid query from renderable again */
        virtual void setWorldMatrices(const Matrix4* m, size_t count);
        /** Updates the current camera */
        virtual void setCurrentCamera(const Camera* cam, bool useCameraRelative);
        /** Sets the light list that should be used, and it's base index from the global list */
        virtual void setCurrentLightList(const LightList* ll);
        /** Sets the current texture projector for a index */
        virtual void setTextureProjector(const Frustum* frust, size_t index);
        /** Sets the current render target */
        virtual void setCurrentRenderTarget(const RenderTarget* target);
        /** Sets the current viewport */
        virtual void setCurrentViewport(const Viewport* viewport);
		/** Sets the shadow extrusion distance to be used for point lights. */
		virtual void setShadowDirLightExtrusionDistance(Real dist);
		/** Sets the main camera's scene bounding information */
		virtual void setMainCamBoundsInfo(VisibleObjectsBoundsInfo* info);
		/** Set the current scene manager for enquiring on demand */
		virtual void setCurrentSceneManager(const SceneManager* sm);
        /** Sets the current pass */
        virtual void setCurrentPass(const Pass* pass);



        virtual const Matrix4& getWorldMatrix(void) const;
        virtual const Matrix4* getWorldMatrixArray(void) const;
        virtual size_t getWorldMatrixCount(void) const;
        virtual const Matrix4& getViewMatrix(void) const;
        virtual const Matrix4& getViewProjectionMatrix(void) const;
        virtual const Matrix4& getProjectionMatrix(void) const;
        virtual const Matrix4& getWorldViewProjMatrix(void) const;
        virtual const Matrix4& getWorldViewMatrix(void) const;
        virtual const Matrix4& getInverseWorldMatrix(void) const;
        virtual const Matrix4& getInverseWorldViewMatrix(void) const;
        virtual const Matrix4& getInverseViewMatrix(void) const;
        virtual const Matrix4& getInverseTransposeWorldMatrix(void) const;
        virtual const Matrix4& getInverseTransposeWorldViewMatrix(void) const;
        virtual const Vector4& getCameraPosition(void) const;
        virtual const Vector4& getCameraPositionObjectSpace(void) const;
		virtual const Vector4& getLodCameraPosition(void) const;
		virtual const Vector4& getLodCameraPositionObjectSpace(void) const;
		virtual bool hasLightList() const { return mCurrentLightList != 0; }
        /** Get the light which is 'index'th closest to the current object */        
		virtual float getLightNumber(size_t index) const;
		virtual float getLightCount() const;
		virtual float getLightCastsShadows(size_t index) const;
		virtual const ColourValue& getLightDiffuseColour(size_t index) const;
		virtual const ColourValue& getLightSpecularColour(size_t index) const;
		virtual const ColourValue getLightDiffuseColourWithPower(size_t index) const;
		virtual const ColourValue getLightSpecularColourWithPower(size_t index) const;
		virtual const Vector3& getLightPosition(size_t index) const;
		virtual Vector4 getLightAs4DVector(size_t index) const;
		virtual const Vector3& getLightDirection(size_t index) const;
		virtual Real getLightPowerScale(size_t index) const;
		virtual Vector4 getLightAttenuation(size_t index) const;
		virtual Vector4 getSpotlightParams(size_t index) const;
		virtual void setAmbientLightColour(const ColourValue& ambient);
		virtual const ColourValue& getAmbientLightColour(void) const;
        virtual const ColourValue& getSurfaceAmbientColour(void) const;
        virtual const ColourValue& getSurfaceDiffuseColour(void) const;
        virtual const ColourValue& getSurfaceSpecularColour(void) const;
        virtual const ColourValue& getSurfaceEmissiveColour(void) const;
        virtual Real getSurfaceShininess(void) const;
        virtual ColourValue getDerivedAmbientLightColour(void) const;
        virtual ColourValue getDerivedSceneColour(void) const;
        virtual void setFog(FogMode mode, const ColourValue& colour, Real expDensity, Real linearStart, Real linearEnd);
        virtual const ColourValue& getFogColour(void) const;
        virtual const Vector4& getFogParams(void) const;
        virtual const Matrix4& getTextureViewProjMatrix(size_t index) const;
		virtual const Matrix4& getTextureWorldViewProjMatrix(size_t index) const;
		virtual const Matrix4& getSpotlightViewProjMatrix(size_t index) const;
		virtual const Matrix4& getSpotlightWorldViewProjMatrix(size_t index) const;
        virtual const Matrix4& getTextureTransformMatrix(size_t index) const;
        virtual const RenderTarget* getCurrentRenderTarget(void) const;
        virtual const Renderable* getCurrentRenderable(void) const;
        virtual const Pass* getCurrentPass(void) const;
        virtual Vector4 getTextureSize(size_t index) const;
        virtual Vector4 getInverseTextureSize(size_t index) const;
        virtual Vector4 getPackedTextureSize(size_t index) const;
		virtual Real getShadowExtrusionDistance(void) const;
		virtual const Vector4& getSceneDepthRange() const;
		virtual const Vector4& getShadowSceneDepthRange(size_t index) const;
		virtual const ColourValue& getShadowColour() const;
		virtual Matrix4 getInverseViewProjMatrix(void) const;
		virtual Matrix4 getInverseTransposeViewProjMatrix() const;
		virtual Matrix4 getTransposeViewProjMatrix() const;
		virtual Matrix4 getTransposeViewMatrix() const;
        virtual Matrix4 getInverseTransposeViewMatrix() const;
		virtual Matrix4 getTransposeProjectionMatrix() const;
		virtual Matrix4 getInverseProjectionMatrix() const;
		virtual Matrix4 getInverseTransposeProjectionMatrix() const;
		virtual Matrix4 getTransposeWorldViewProjMatrix() const;
		virtual Matrix4 getInverseWorldViewProjMatrix() const;
		virtual Matrix4 getInverseTransposeWorldViewProjMatrix() const;
		virtual Matrix4 getTransposeWorldViewMatrix() const;
		virtual Matrix4 getTransposeWorldMatrix() const;
        virtual Real getTime(void) const;
		virtual Real getTime_0_X(Real x) const;
		virtual Real getCosTime_0_X(Real x) const;
		virtual Real getSinTime_0_X(Real x) const;
		virtual Real getTanTime_0_X(Real x) const;
		virtual Vector4 getTime_0_X_packed(Real x) const;
		virtual Real getTime_0_1(Real x) const;
		virtual Real getCosTime_0_1(Real x) const;
		virtual Real getSinTime_0_1(Real x) const;
		virtual Real getTanTime_0_1(Real x) const;
		virtual Vector4 getTime_0_1_packed(Real x) const;
		virtual Real getTime_0_2Pi(Real x) const;
		virtual Real getCosTime_0_2Pi(Real x) const;
		virtual Real getSinTime_0_2Pi(Real x) const;
		virtual Real getTanTime_0_2Pi(Real x) const;
		virtual Vector4 getTime_0_2Pi_packed(Real x) const;
        virtual Real getFrameTime(void) const;
		virtual Real getFPS() const;
		virtual Real getViewportWidth() const;
		virtual Real getViewportHeight() const;
		virtual Real getInverseViewportWidth() const;
		virtual Real getInverseViewportHeight() const;
		virtual Vector3 getViewDirection() const;
		virtual Vector3 getViewSideVector() const;
		virtual Vector3 getViewUpVector() const;
		virtual Real getFOV() const;
		virtual Real getNearClipDistance() const;
		virtual Real getFarClipDistance() const;
        virtual int getPassNumber(void) const;
        virtual void setPassNumber(const int passNumber);
        virtual void incPassNumber(void);
		virtual void updateLightCustomGpuParameter(const GpuProgramParameters::AutoConstantEntry& constantEntry, GpuProgramParameters *params) const;
    };
	/** @} */
	/** @} */
}

#endif
