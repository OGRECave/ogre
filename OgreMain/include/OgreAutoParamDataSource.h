/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2014 Torus Knot Software Ltd

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
#include "OgreLight.h"
#include "OgreSceneNode.h"

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
        mutable Affine3 mWorldMatrix[256];
        mutable size_t mWorldMatrixCount;
        mutable const Affine3* mWorldMatrixArray;
        mutable Affine3 mWorldViewMatrix;
        mutable Matrix4 mViewProjMatrix;
        mutable Matrix4 mWorldViewProjMatrix;
        mutable Affine3 mInverseWorldMatrix;
        mutable Affine3 mInverseWorldViewMatrix;
        mutable Affine3 mInverseViewMatrix;
        mutable Matrix4 mInverseTransposeWorldMatrix;
        mutable Matrix4 mInverseTransposeWorldViewMatrix;
        mutable Vector4 mCameraPosition;
        mutable Vector4 mCameraPositionObjectSpace;
        mutable Matrix4 mTextureViewProjMatrix[OGRE_MAX_SIMULTANEOUS_LIGHTS];
        mutable Matrix4 mTextureWorldViewProjMatrix[OGRE_MAX_SIMULTANEOUS_LIGHTS];
        mutable Matrix4 mSpotlightViewProjMatrix[OGRE_MAX_SIMULTANEOUS_LIGHTS];
        mutable Matrix4 mSpotlightWorldViewProjMatrix[OGRE_MAX_SIMULTANEOUS_LIGHTS];
        mutable Vector4 mShadowCamDepthRanges[OGRE_MAX_SIMULTANEOUS_LIGHTS];
        mutable Affine3 mViewMatrix;
        mutable Matrix4 mProjectionMatrix;
        mutable Real mDirLightExtrusionDistance;
        mutable Real mPointLightExtrusionDistance;
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
        ColourValue mAmbientLight;
        ColourValue mFogColour;
        Vector4f mFogParams;
        Vector4f mPointParams;
        int mPassNumber;
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

        SceneNode mDummyNode;
        Light mBlankLight;
    public:
        AutoParamDataSource();
        /** Updates the current renderable */
        void setCurrentRenderable(const Renderable* rend);
        /** Sets the world matrices, avoid query from renderable again */
        void setWorldMatrices(const Affine3* m, size_t count);
        /** Updates the current camera */
        void setCurrentCamera(const Camera* cam, bool useCameraRelative);
        /** Sets the light list that should be used, and it's base index from the global list */
        void setCurrentLightList(const LightList* ll);
        /** Sets the current texture projector for a index */
        void setTextureProjector(const Frustum* frust, size_t index);
        /** Sets the current render target */
        void setCurrentRenderTarget(const RenderTarget* target);
        /** Sets the current viewport */
        void setCurrentViewport(const Viewport* viewport);
        /** Sets the shadow extrusion distance to be used for dir lights. */
        void setShadowDirLightExtrusionDistance(Real dist);
        /** Sets the shadow extrusion distance to be used for point lights. */
        void setShadowPointLightExtrusionDistance(Real dist);
        /** Sets the main camera's scene bounding information */
        void setMainCamBoundsInfo(VisibleObjectsBoundsInfo* info);
        /** Set the current scene manager for enquiring on demand */
        void setCurrentSceneManager(const SceneManager* sm);
        /** Sets the current pass */
        void setCurrentPass(const Pass* pass);

		/** Returns the current bounded camera */
		const Camera* getCurrentCamera() const;

        const Affine3& getWorldMatrix(void) const;
        const Affine3* getWorldMatrixArray(void) const;
        size_t getWorldMatrixCount(void) const;
        const Affine3& getViewMatrix(void) const;
        const Matrix4& getViewProjectionMatrix(void) const;
        const Matrix4& getProjectionMatrix(void) const;
        const Matrix4& getWorldViewProjMatrix(void) const;
        const Affine3& getWorldViewMatrix(void) const;
        const Affine3& getInverseWorldMatrix(void) const;
        const Affine3& getInverseWorldViewMatrix(void) const;
        const Affine3& getInverseViewMatrix(void) const;
        const Matrix4& getInverseTransposeWorldMatrix(void) const;
        const Matrix4& getInverseTransposeWorldViewMatrix(void) const;
        const Vector4& getCameraPosition(void) const;
        const Vector4& getCameraPositionObjectSpace(void) const;
        const Vector4  getCameraRelativePosition(void) const;
        const Vector4& getLodCameraPosition(void) const;
        const Vector4& getLodCameraPositionObjectSpace(void) const;
        bool hasLightList() const { return mCurrentLightList != 0; }
        /** Get the light which is 'index'th closest to the current object */        
        float getLightNumber(size_t index) const;
        float getLightCount() const;
        float getLightCastsShadows(size_t index) const;
        const ColourValue& getLightDiffuseColour(size_t index) const;
        const ColourValue& getLightSpecularColour(size_t index) const;
        const ColourValue getLightDiffuseColourWithPower(size_t index) const;
        const ColourValue getLightSpecularColourWithPower(size_t index) const;
        Vector3 getLightPosition(size_t index) const;
        Vector4 getLightAs4DVector(size_t index) const;
        Vector3 getLightDirection(size_t index) const;
        Real getLightPowerScale(size_t index) const;
        const Vector4f& getLightAttenuation(size_t index) const;
        Vector4f getSpotlightParams(size_t index) const;
        void setAmbientLightColour(const ColourValue& ambient);
        const ColourValue& getAmbientLightColour(void) const;
        const ColourValue& getSurfaceAmbientColour(void) const;
        const ColourValue& getSurfaceDiffuseColour(void) const;
        const ColourValue& getSurfaceSpecularColour(void) const;
        const ColourValue& getSurfaceEmissiveColour(void) const;
        Real getSurfaceShininess(void) const;
        Real getSurfaceAlphaRejectionValue(void) const;
        ColourValue getDerivedAmbientLightColour(void) const;
        ColourValue getDerivedSceneColour(void) const;
        void setFog(FogMode mode, const ColourValue& colour, Real expDensity, Real linearStart, Real linearEnd);
        const ColourValue& getFogColour(void) const;
        const Vector4f& getFogParams(void) const;
        void setPointParameters(bool attenuation, const Vector4f& params);
        const Vector4f& getPointParams() const;
        const Matrix4& getTextureViewProjMatrix(size_t index) const;
        const Matrix4& getTextureWorldViewProjMatrix(size_t index) const;
        const Matrix4& getSpotlightViewProjMatrix(size_t index) const;
        const Matrix4& getSpotlightWorldViewProjMatrix(size_t index) const;
        const Matrix4& getTextureTransformMatrix(size_t index) const;
        const RenderTarget* getCurrentRenderTarget(void) const;
        const Renderable* getCurrentRenderable(void) const;
        const Pass* getCurrentPass(void) const;
        Vector4f getTextureSize(size_t index) const;
        Vector4f getInverseTextureSize(size_t index) const;
        Vector4f getPackedTextureSize(size_t index) const;
        Real getShadowExtrusionDistance(void) const;
        const Vector4& getSceneDepthRange() const;
        const Vector4& getShadowSceneDepthRange(size_t index) const;
        const ColourValue& getShadowColour() const;
        Matrix4 getInverseViewProjMatrix(void) const;
        Matrix4 getInverseTransposeViewProjMatrix() const;
        Matrix4 getTransposeViewProjMatrix() const;
        Matrix4 getTransposeViewMatrix() const;
        Matrix4 getInverseTransposeViewMatrix() const;
        Matrix4 getTransposeProjectionMatrix() const;
        Matrix4 getInverseProjectionMatrix() const;
        Matrix4 getInverseTransposeProjectionMatrix() const;
        Matrix4 getTransposeWorldViewProjMatrix() const;
        Matrix4 getInverseWorldViewProjMatrix() const;
        Matrix4 getInverseTransposeWorldViewProjMatrix() const;
        Matrix4 getTransposeWorldViewMatrix() const;
        Matrix4 getTransposeWorldMatrix() const;
        Real getTime(void) const;
        Real getTime_0_X(Real x) const;
        Real getCosTime_0_X(Real x) const;
        Real getSinTime_0_X(Real x) const;
        Real getTanTime_0_X(Real x) const;
        Vector4f getTime_0_X_packed(Real x) const;
        Real getTime_0_1(Real x) const;
        Real getCosTime_0_1(Real x) const;
        Real getSinTime_0_1(Real x) const;
        Real getTanTime_0_1(Real x) const;
        Vector4f getTime_0_1_packed(Real x) const;
        Real getTime_0_2Pi(Real x) const;
        Real getCosTime_0_2Pi(Real x) const;
        Real getSinTime_0_2Pi(Real x) const;
        Real getTanTime_0_2Pi(Real x) const;
        Vector4f getTime_0_2Pi_packed(Real x) const;
        Real getFrameTime(void) const;
        Real getFPS() const;
        Real getViewportWidth() const;
        Real getViewportHeight() const;
        Real getInverseViewportWidth() const;
        Real getInverseViewportHeight() const;
        Vector3 getViewDirection() const;
        Vector3 getViewSideVector() const;
        Vector3 getViewUpVector() const;
        Real getFOV() const;
        Real getNearClipDistance() const;
        Real getFarClipDistance() const;
        int getPassNumber(void) const;
        void setPassNumber(const int passNumber);
        void incPassNumber(void);
        void updateLightCustomGpuParameter(const GpuProgramParameters::AutoConstantEntry& constantEntry, GpuProgramParameters *params) const;
    };
    /** @} */
    /** @} */
}

#endif
