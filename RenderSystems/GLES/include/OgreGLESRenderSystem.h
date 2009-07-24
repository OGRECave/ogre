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

#include "OgreRenderSystem.h"


namespace Ogre {
    class GLESContext;
    class GLESSupport;
    class GLESRTTManager;
    class GLESGpuProgramManager;
    class HardwareBufferManager;

    /**
      Implementation of GL as a rendering system.
     */
    class _OgrePrivate GLESRenderSystem : public RenderSystem
    {
        private:

            /** Array of up to 8 lights, indexed as per API
                Note that a null value indicates a free slot
              */ 
            #define MAX_LIGHTS 8
            Light* mLights[MAX_LIGHTS];
            unsigned short mCurrentLights;

            /// View matrix to set world against
            Matrix4 mViewMatrix;
            Matrix4 mWorldMatrix;
            Matrix4 mTextureMatrix;

            /// Last min & mip filtering options, so we can combine them
            FilterOptions mMinFilter;
            FilterOptions mMipFilter;

            /// What texture coord set each texture unit is using
            size_t mTextureCoordIndex[OGRE_MAX_TEXTURE_LAYERS];

            /// Number of fixed-function texture units
            unsigned short mFixedFunctionTextureUnits;

            /// Store last colour write state
            bool mColourWrite[4];

            /// Store last depth write state
            bool mDepthWrite;

            /// Store last stencil mask state
            uint32 mStencilMask;

            GLfloat mAutoTextureMatrix[16];

            bool mUseAutoTextureMatrix;
            size_t mTextureCount;

            bool mTextureEnabled;

            /// GL support class, used for creating windows etc.
            GLESSupport *mGLSupport;

            /* The main GL context - main thread only */
            GLESContext *mMainContext;

            /* The current GL context  - main thread only */
            GLESContext *mCurrentContext;
            GLESGpuProgramManager *mGpuProgramManager;
            HardwareBufferManager* mHardwareBufferManager;

            /** Manager object for creating render textures.
                Direct render to texture via GL_OES_framebuffer_object is preferable 
                to pbuffers, which depend on the GL support used and are generally 
                unwieldy and slow. However, FBO support for stencil buffers is poor.
              */
            GLESRTTManager *mRTTManager;

            /// Check if the GL system has already been initialised
            bool mGLInitialised;

            GLuint getCombinedMinMipFilter(void) const;

            GLint getTextureAddressingMode(TextureUnitState::TextureAddressingMode tam) const;
            GLint getBlendMode(SceneBlendFactor ogreBlend) const;
            void makeGLMatrix(GLfloat gl_matrix[16], const Matrix4& m);
            void setGLLight(size_t index, Light* lt);

            /// Internal method to set pos / direction of a light
            void setGLLightPositionDirection(Light* lt, GLenum lightindex);
            void setLights();

        protected:

        public:
            // Default constructor / destructor
            GLESRenderSystem();
            virtual ~GLESRenderSystem();

            // ----------------------------------
            // Overridden RenderSystem functions
            // ----------------------------------
            /** See
              RenderSystem
             */
            const String& getName(void) const;
            /** See
              RenderSystem
             */
            ConfigOptionMap& getConfigOptions(void);
            /** See
              RenderSystem
             */
            void setConfigOption(const String &name, const String &value);
            /** See
              RenderSystem
             */
            String validateConfigOptions(void);
            /** See
              RenderSystem
             */
            RenderWindow* _initialise(bool autoCreateWindow, const String& windowTitle = "OGRE Render NativeWindowType");
            /** See
              RenderSystem
             */
            virtual RenderSystemCapabilities* createRenderSystemCapabilities() const;
            /** See
              RenderSystem
             */
            void initialiseFromRenderSystemCapabilities(RenderSystemCapabilities* caps, RenderTarget* primary);
            /** See
              RenderSystem
             */
            void reinitialise(void); // Used if settings changed mid-rendering
            /** See
              RenderSystem
             */
            void shutdown(void);
            /** See
              RenderSystem
             */
            void setAmbientLight(float r, float g, float b);
            /** See
              RenderSystem
             */
            void setShadingType(ShadeOptions so);
            /** See
              RenderSystem
             */
            void setLightingEnabled(bool enabled);

            /// @copydoc RenderSystem::_createRenderWindow
            RenderWindow* _createRenderWindow(const String &name, unsigned int width, unsigned int height, 
                bool fullScreen, const NameValuePairList *miscParams = 0);

            /// @copydoc RenderSystem::createMultiRenderTarget
            virtual MultiRenderTarget * createMultiRenderTarget(const String & name);

            /** See
              RenderSystem
             */
            void destroyRenderWindow(RenderWindow* pWin);
            /** See
              RenderSystem
             */
            String getErrorDescription(long errorNumber) const;
            /** See
              RenderSystem
             */
            VertexElementType getColourVertexElementType(void) const;
            /** See
              RenderSystem
             */
            void setNormaliseNormals(bool normalise);
    
            // -----------------------------
            // Low-level overridden members
            // -----------------------------
            /** See
             RenderSystem
             */
            void _useLights(const LightList& lights, unsigned short limit);
            /** See
             RenderSystem
             */
            bool areFixedFunctionLightsInViewSpace() const { return true; }
            /** See
             RenderSystem
             */
            void _setWorldMatrix(const Matrix4 &m);
            /** See
             RenderSystem
             */
            void _setViewMatrix(const Matrix4 &m);
            /** See
             RenderSystem
             */
            void _setProjectionMatrix(const Matrix4 &m);
            /** See
             RenderSystem
             */
            void _setSurfaceParams(const ColourValue &ambient,
                    const ColourValue &diffuse, const ColourValue &specular,
                    const ColourValue &emissive, Real shininess,
                    TrackVertexColourType tracking);
            /** See
             RenderSystem
             */
            void _setPointParameters(Real size, bool attenuationEnabled,
                    Real constant, Real linear, Real quadratic, Real minSize, Real maxSize);
            /** See
             RenderSystem
             */
            void _setPointSpritesEnabled(bool enabled);
            /** See
             RenderSystem
             */
            void _setTexture(size_t unit, bool enabled, const TexturePtr &tex);
            /** See
             RenderSystem
             */
            void _setTextureCoordSet(size_t stage, size_t index);
            /** See
             RenderSystem
             */
            void _setTextureCoordCalculation(size_t stage, TexCoordCalcMethod m,
                    const Frustum* frustum = 0);
            /** See
             RenderSystem
             */
            void _setTextureBlendMode(size_t stage, const LayerBlendModeEx& bm);
            /** See
             RenderSystem
             */
            void _setTextureAddressingMode(size_t stage, const TextureUnitState::UVWAddressingMode& uvw);
            /** See
             RenderSystem
             */
            void _setTextureBorderColour(size_t stage, const ColourValue& colour);
            /** See
             RenderSystem
             */
            void _setTextureMipmapBias(size_t unit, float bias);
            /** See
             RenderSystem
             */
            void _setTextureMatrix(size_t stage, const Matrix4& xform);
            /** See
             RenderSystem
             */
            void _setViewport(Viewport *vp);
            /** See
             RenderSystem
             */
            void _beginFrame(void);
            /** See
             RenderSystem
             */
            void _endFrame(void);
            /** See
             RenderSystem
             */
            void _setCullingMode(CullingMode mode);
            /** See
             RenderSystem
             */
            void _setDepthBufferParams(bool depthTest = true, bool depthWrite = true, CompareFunction depthFunction = CMPF_LESS_EQUAL);
            /** See
             RenderSystem
             */
            void _setDepthBufferCheckEnabled(bool enabled = true);
            /** See
             RenderSystem
             */
            void _setDepthBufferWriteEnabled(bool enabled = true);
            /** See
             RenderSystem
             */
            void _setDepthBufferFunction(CompareFunction func = CMPF_LESS_EQUAL);
            /** See
             RenderSystem
             */
            void _setDepthBias(float constantBias, float slopeScaleBias);
            /** See
             RenderSystem
             */
            void _setColourBufferWriteEnabled(bool red, bool green, bool blue, bool alpha);
            /** See
             RenderSystem
             */
            void _setFog(FogMode mode, const ColourValue& colour, Real density, Real start, Real end);
            /** See
             RenderSystem
             */
            void _convertProjectionMatrix(const Matrix4& matrix,
                    Matrix4& dest, bool forGpuProgram = false);
            /** See
             RenderSystem
             */
            void _makeProjectionMatrix(const Radian& fovy, Real aspect, Real nearPlane, Real farPlane,
                    Matrix4& dest, bool forGpuProgram = false);
            /** See
             RenderSystem
             */
            void _makeProjectionMatrix(Real left, Real right, Real bottom, Real top, 
                    Real nearPlane, Real farPlane, Matrix4& dest, bool forGpuProgram = false);
            /** See
             RenderSystem
             */
            void _makeOrthoMatrix(const Radian& fovy, Real aspect, Real nearPlane, Real farPlane,
                    Matrix4& dest, bool forGpuProgram = false);
            /** See
             RenderSystem
             */
            void _applyObliqueDepthProjection(Matrix4& matrix, const Plane& plane, 
                    bool forGpuProgram);
            /** See
             RenderSystem
             */
            void setClipPlane (ushort index, Real A, Real B, Real C, Real D);
            /** See
             RenderSystem
             */
            void enableClipPlane (ushort index, bool enable);
            /** See
             RenderSystem
             */
            void _setPolygonMode(PolygonMode level);
            /** See
             RenderSystem
             */
            void setStencilCheckEnabled(bool enabled);
            /** See
             RenderSystem
             */
            void setStencilBufferParams(CompareFunction func = CMPF_ALWAYS_PASS, 
                    uint32 refValue = 0, uint32 mask = 0xFFFFFFFF,
                    StencilOperation stencilFailOp = SOP_KEEP,
                    StencilOperation depthFailOp = SOP_KEEP,
                    StencilOperation passOp = SOP_KEEP,
                    bool twoSidedOperation = false);
            /** See
             RenderSystem
             */
            void _setTextureUnitFiltering(size_t unit, FilterType ftype, FilterOptions filter);
            /** See
             RenderSystem
             */
            void _setTextureLayerAnisotropy(size_t unit, unsigned int maxAnisotropy);
            /** See
             RenderSystem
             */
            void setVertexDeclaration(VertexDeclaration* decl);
            /** See
             RenderSystem
             */
            void setVertexBufferBinding(VertexBufferBinding* binding);
            /** See
             RenderSystem
             */
            void _render(const RenderOperation& op);
            /** See
             RenderSystem
             */
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

            // ----------------------------------
            // GLRenderSystem specific members
            // ----------------------------------
            /** Returns the main context */
            GLESContext* _getMainContext() { return mMainContext; }
            /** Unregister a render target->context mapping. If the context of target 
             is the current context, change the context to the main context so it
             can be destroyed safely. 
             
             @note This is automatically called by the destructor of 
             GLESContext.
             */
            void _unregisterContext(GLESContext *context);
            /** Switch GL context, dealing with involved internal cached states too
             */
            void _switchContext(GLESContext *context);
            /** One time initialization for the RenderState of a context. Things that
             only need to be set once, like the LightingModel can be defined here.
             */
            void _oneTimeContextInitialization();
            void initialiseContext(RenderWindow* primary);
            /**
             * Set current render target to target, enabling its GL context if needed
             */
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

            /// Internal method for anisotropy validation
            GLfloat _getCurrentAnisotropy(size_t unit);

            void _setSceneBlendingOperation(SceneBlendOperation op);
            void _setSeparateSceneBlendingOperation(SceneBlendOperation op, SceneBlendOperation alphaOp);
    };
}

#endif
