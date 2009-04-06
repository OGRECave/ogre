/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
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

#ifndef __GLESRenderSystem_H__
#define __GLESRenderSystem_H__

#include "OgreGLESPrerequisites.h"

#include "OgrePlatform.h"
#include "OgreRenderSystem.h"
#include "OgreVector4.h"
#include "OgreHardwareBufferManager.h"
#include "OgreLight.h"

#define MAX_LIGHTS 8

namespace Ogre {
    class GLESContext;
    class GLESSupport;
    class GLESRTTManager;
    class GLESGpuProgramManager;

    /**
      Implementation of GL as a rendering system.
     */
    class _OgrePrivate GLESRenderSystem : public RenderSystem
    {
        private:
            GLESSupport *mGLSupport;
            GLESContext *mMainContext;
            GLESContext *mCurrentContext;
            GLESGpuProgramManager *mGpuProgramManager;
            HardwareBufferManager* mHardwareBufferManager;

            GLESRTTManager *mRTTManager;
            bool mGLInitialized;

            Light* mLights[MAX_LIGHTS];
            unsigned short mCurrentLights;

            bool mColourWrite[4];
            bool mDepthWrite;
            uint32 mStencilMask;

            Matrix4 mViewMatrix;
            Matrix4 mWorldMatrix;
            Matrix4 mTextureMatrix;

            GLfloat mAutoTextureMatrix[16];
            FilterOptions mMinFilter;
            FilterOptions mMipFilter;
            bool mUseAutoTextureMatrix;
            size_t mTextureCount;

            bool mTextureEnabled;
            size_t mTextureCoordIndex[OGRE_MAX_TEXTURE_LAYERS];
            unsigned short mFixedFunctionTextureUnits;

            GLuint getCombinedMinMipFilter(void) const;

            GLint getTextureAddressingMode(TextureUnitState::TextureAddressingMode tam) const;
            GLint getBlendMode(SceneBlendFactor ogreBlend) const;
            void makeGLMatrix(GLfloat gl_matrix[16], const Matrix4& m);
            void setGLLight(size_t index, Light* lt);
            void setGLLightPositionDirection(Light* lt, GLenum lightindex);
            void setLights();

        protected:

        public:
            // Default constructor / destructor
            GLESRenderSystem();
            virtual ~GLESRenderSystem();

            const String& getName(void) const;
            ConfigOptionMap& getConfigOptions(void);
            void setConfigOption(const String &name, const String &value);
            String validateConfigOptions(void);
            RenderWindow* _initialise(bool autoCreateWindow, const String& windowTitle = "OGRE Render NativeWindowType");
            virtual RenderSystemCapabilities* createRenderSystemCapabilities() const;
            void initialiseFromRenderSystemCapabilities(RenderSystemCapabilities* caps, RenderTarget* primary);
            void reinitialise(void); // Used if settings changed mid-rendering
            void shutdown(void);
            void setAmbientLight(float r, float g, float b);
            void setShadingType(ShadeOptions so);
            void setLightingEnabled(bool enabled);
            RenderWindow* _createRenderWindow(const String &name, unsigned int width, unsigned int height, 
            bool fullScreen, const NameValuePairList *miscParams = 0);
            virtual MultiRenderTarget * createMultiRenderTarget(const String & name); 
            void destroyRenderWindow(RenderWindow* pWin);
            String getErrorDescription(long errorNumber) const;
            VertexElementType getColourVertexElementType(void) const;
            void setNormaliseNormals(bool normalise);
            void _useLights(const LightList& lights, unsigned short limit);
			bool areFixedFunctionLightsInViewSpace() const { return true; }
            void _setWorldMatrix(const Matrix4 &m);
            void _setViewMatrix(const Matrix4 &m);
            void _setProjectionMatrix(const Matrix4 &m);
            void _setSurfaceParams(const ColourValue &ambient,
                const ColourValue &diffuse, const ColourValue &specular,
                const ColourValue &emissive, Real shininess,
                TrackVertexColourType tracking);
            void _setPointParameters(Real size, bool attenuationEnabled,
                Real constant, Real linear, Real quadratic, Real minSize, Real maxSize);
            void _setPointSpritesEnabled(bool enabled);
            void _setTexture(size_t unit, bool enabled, const TexturePtr &tex);
            void _setTextureCoordSet(size_t stage, size_t index);
            void _setTextureCoordCalculation(size_t stage, TexCoordCalcMethod m,
                const Frustum* frustum = 0);
            void _setTextureBlendMode(size_t stage, const LayerBlendModeEx& bm);
            void _setTextureAddressingMode(size_t stage, const TextureUnitState::UVWAddressingMode& uvw);
            void _setTextureBorderColour(size_t stage, const ColourValue& colour);
            void _setTextureMipmapBias(size_t unit, float bias);
            void _setTextureMatrix(size_t stage, const Matrix4& xform);
            void _setViewport(Viewport *vp);
            void _beginFrame(void);
            void _endFrame(void);
            void _setCullingMode(CullingMode mode);
            void _setDepthBufferParams(bool depthTest = true, bool depthWrite = true, CompareFunction depthFunction = CMPF_LESS_EQUAL);
            void _setDepthBufferCheckEnabled(bool enabled = true);
            void _setDepthBufferWriteEnabled(bool enabled = true);
            void _setDepthBufferFunction(CompareFunction func = CMPF_LESS_EQUAL);
            void _setDepthBias(float constantBias, float slopeScaleBias);
            void _setColourBufferWriteEnabled(bool red, bool green, bool blue, bool alpha);
            void _setFog(FogMode mode, const ColourValue& colour, Real density, Real start, Real end);
            void _convertProjectionMatrix(const Matrix4& matrix,
                Matrix4& dest, bool forGpuProgram = false);
            void _makeProjectionMatrix(const Radian& fovy, Real aspect, Real nearPlane, Real farPlane,
                Matrix4& dest, bool forGpuProgram = false);
            void _makeProjectionMatrix(Real left, Real right, Real bottom, Real top, 
                Real nearPlane, Real farPlane, Matrix4& dest, bool forGpuProgram = false);
            void _makeOrthoMatrix(const Radian& fovy, Real aspect, Real nearPlane, Real farPlane,
                Matrix4& dest, bool forGpuProgram = false);
            void _applyObliqueDepthProjection(Matrix4& matrix, const Plane& plane, 
                bool forGpuProgram);
            void setClipPlane (ushort index, Real A, Real B, Real C, Real D);
            void enableClipPlane (ushort index, bool enable);
            void _setPolygonMode(PolygonMode level);
            void setStencilCheckEnabled(bool enabled);
            void setStencilBufferParams(CompareFunction func = CMPF_ALWAYS_PASS, 
                uint32 refValue = 0, uint32 mask = 0xFFFFFFFF,
                StencilOperation stencilFailOp = SOP_KEEP,
                StencilOperation depthFailOp = SOP_KEEP,
                StencilOperation passOp = SOP_KEEP,
                bool twoSidedOperation = false);
            void _setTextureUnitFiltering(size_t unit, FilterType ftype, FilterOptions filter);
            void _setTextureLayerAnisotropy(size_t unit, unsigned int maxAnisotropy);
            void setVertexDeclaration(VertexDeclaration* decl);
            void setVertexBufferBinding(VertexBufferBinding* binding);
            void _render(const RenderOperation& op);
            void setScissorTest(bool enabled, size_t left = 0, size_t top = 0, size_t right = 800, size_t bottom = 600);
            void clearFrameBuffer(unsigned int buffers,
                const ColourValue& colour = ColourValue::Black,
                Real depth = 1.0f, unsigned short stencil = 0);
            HardwareOcclusionQuery* createHardwareOcclusionQuery(void);
            Real getHorizontalTexelOffset(void);
            Real getVerticalTexelOffset(void);
            Real getMinimumDepthInputValue(void);
            Real getMaximumDepthInputValue(void);
            void registerThread();
            void unregisterThread();
            void preExtraThreadsStarted();
            void postExtraThreadsStarted();
            void setClipPlanesImpl(const Ogre::PlaneList& planeList);

            GLESContext* _getMainContext() { return mMainContext; }
            void _unregisterContext(GLESContext *context);
            void _switchContext(GLESContext *context);
            void _oneTimeContextInitialization();
            void initialiseContext(RenderWindow* primary);
            void _setRenderTarget(RenderTarget *target);

            GLint convertCompareFunction(CompareFunction func) const;

            void bindGpuProgram(GpuProgram* prg);
            void unbindGpuProgram(GpuProgramType gptype);
            void bindGpuProgramParameters(GpuProgramType gptype, GpuProgramParametersSharedPtr params, uint16 mask);
            void bindGpuProgramPassIterationParameters(GpuProgramType gptype);

			/// @copydoc RenderSystem::_setSceneBlending
			void _setSceneBlending( SceneBlendFactor sourceFactor, SceneBlendFactor destFactor, SceneBlendOperation op );
			/// @copydoc RenderSystem::_setSeparateSceneBlending
			void _setSeparateSceneBlending( SceneBlendFactor sourceFactor, SceneBlendFactor destFactor, SceneBlendFactor sourceFactorAlpha, SceneBlendFactor destFactorAlpha, SceneBlendOperation op, SceneBlendOperation alphaOp );
			/// @copydoc RenderSystem::_setAlphaRejectSettings
			void _setAlphaRejectSettings( CompareFunction func, unsigned char value, bool alphaToCoverage );
			/// @copydoc RenderSystem::getDisplayMonitorCount
			unsigned int getDisplayMonitorCount() const;

            void _setSceneBlendingOperation(SceneBlendOperation op);
            void _setSeparateSceneBlendingOperation(SceneBlendOperation op, SceneBlendOperation alphaOp);
    };
}

#endif
