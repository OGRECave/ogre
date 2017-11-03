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

#include "Math/Array/OgreObjectMemoryManager.h"

namespace Ogre {

    // forward decls
    class CompositorShadowNode;
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
    class _OgreExport AutoParamDataSource : public SceneMgtAlignedAlloc
    {
    protected:
        const Light& getLight(size_t index) const;
        OGRE_SIMD_ALIGNED_DECL( mutable Matrix4, mWorldMatrix[256] );
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
        mutable ColourValue mAmbientLight[2];
        mutable Vector3 mAmbientLightHemisphereDir;
        mutable ColourValue mFogColour;
        mutable Vector4 mFogParams;
        mutable int mPassNumber;
        mutable Vector4 mSceneDepthRange;
        mutable bool mSceneDepthRangeDirty;
        mutable bool mLodCameraPositionDirty;
        mutable bool mLodCameraPositionObjectSpaceDirty;

        const Renderable* mCurrentRenderable;
        const Camera* mCurrentCamera;
        const LightList* mCurrentLightList;
        const Frustum* mCurrentTextureProjector[OGRE_MAX_SIMULTANEOUS_LIGHTS];
        const RenderTarget* mCurrentRenderTarget;
        const Viewport* mCurrentViewport;
        const SceneManager* mCurrentSceneManager;
        const Pass* mCurrentPass;
        const HlmsComputeJob *mCurrentJob;
        const CompositorShadowNode *mCurrentShadowNode;
        vector<Real>::type          mNullPssmSplitPoint;
        vector<Real>::type          mNullPssmBlendPoint;

        ObjectMemoryManager mObjectMemoryManager;
        NodeMemoryManager *mNodeMemoryManager;
        SceneNode *mBlankLightNode;
        Light mBlankLight;
    public:
        AutoParamDataSource();
        virtual ~AutoParamDataSource();
        /** Updates the current renderable */
         void setCurrentRenderable(const Renderable* rend);
        /** Sets the world matrices, avoid query from renderable again */
         void setWorldMatrices(const Matrix4* m, size_t count);
        /** Updates the current camera */
         void setCurrentCamera(const Camera* cam);
        /** Sets the light list that should be used, and it's base index from the global list */
         void setCurrentLightList(const LightList* ll);
        /** Sets the current texture projector for a index */
         void setTextureProjector(const Frustum* frust, size_t index);
        /** Sets the current viewport */
         void setCurrentViewport(const Viewport* viewport);
        /** Sets the shadow extrusion distance to be used for point lights. */
         void setShadowDirLightExtrusionDistance(Real dist);
        /** Set the current scene manager for enquiring on demand */
         void setCurrentSceneManager(const SceneManager* sm);
        /** Sets the current pass */
         void setCurrentPass(const Pass* pass);
         void setCurrentJob(const HlmsComputeJob* job);
         void setCurrentShadowNode(const CompositorShadowNode *sn);

         const Camera* getCurrentCamera() const;


         const Matrix4& getWorldMatrix(void) const;
         const Matrix4* getWorldMatrixArray(void) const;
         size_t getWorldMatrixCount(void) const;
         const Matrix4& getViewMatrix(void) const;
         const Matrix4& getViewProjectionMatrix(void) const;
         const Matrix4& getProjectionMatrix(void) const;
         const Matrix4& getWorldViewProjMatrix(void) const;
         const Matrix4& getWorldViewMatrix(void) const;
         const Matrix4& getInverseWorldMatrix(void) const;
         const Matrix4& getInverseWorldViewMatrix(void) const;
         const Matrix4& getInverseViewMatrix(void) const;
         const Matrix4& getInverseTransposeWorldMatrix(void) const;
         const Matrix4& getInverseTransposeWorldViewMatrix(void) const;
         const Vector4& getCameraPosition(void) const;
         const Vector4& getCameraPositionObjectSpace(void) const;
         const Vector4& getLodCameraPosition(void) const;
         const Vector4& getLodCameraPositionObjectSpace(void) const;
         bool hasLightList() const { return mCurrentLightList != 0; }
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
         Vector4 getLightAttenuation(size_t index) const;
         Vector4 getSpotlightParams(size_t index) const;
         void setAmbientLightColour( const ColourValue hemispheres[2],
                                     const Vector3 &hemisphereDir );
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
         const Vector4& getFogParams(void) const;
         const Matrix4& getTextureViewProjMatrix(size_t index) const;
         const Matrix4& getTextureWorldViewProjMatrix(size_t index) const;
         const Matrix4& getSpotlightViewProjMatrix(size_t index) const;
         const Matrix4& getSpotlightWorldViewProjMatrix(size_t index) const;
         const Matrix4& getTextureTransformMatrix(size_t index) const;
         const vector<Real>::type& getPssmSplits( size_t shadowMapIdx ) const;
         const vector<Real>::type& getPssmBlends( size_t shadowMapIdx ) const;
         Real getPssmFade( size_t shadowMapIdx ) const;
         const RenderTarget* getCurrentRenderTarget(void) const;
         const Renderable* getCurrentRenderable(void) const;
         const Pass* getCurrentPass(void) const;
         const HlmsComputeJob* getCurrentJob(void) const;
         Vector4 getTextureSize(size_t index) const;
         Vector4 getInverseTextureSize(size_t index) const;
         Vector4 getPackedTextureSize(size_t index) const;
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
         Vector4 getTime_0_X_packed(Real x) const;
         Real getTime_0_1(Real x) const;
         Real getCosTime_0_1(Real x) const;
         Real getSinTime_0_1(Real x) const;
         Real getTanTime_0_1(Real x) const;
         Vector4 getTime_0_1_packed(Real x) const;
         Real getTime_0_2Pi(Real x) const;
         Real getCosTime_0_2Pi(Real x) const;
         Real getSinTime_0_2Pi(Real x) const;
         Real getTanTime_0_2Pi(Real x) const;
         Vector4 getTime_0_2Pi_packed(Real x) const;
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

		 const Light& _getBlankLight(void) const		{ return mBlankLight; }
    };
    /** @} */
    /** @} */
}

#endif
