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

#ifndef __GL3PlusRenderSystem_H__
#define __GL3PlusRenderSystem_H__

#include "OgreGL3PlusPrerequisites.h"

#include "OgreMaterialManager.h"
#include "OgreRenderSystem.h"
#include "OgreGL3PlusGpuProgram.h"

namespace Ogre {
    class GL3PlusContext;
    class GL3PlusSupport;
    class GL3PlusRTTManager;
    class GL3PlusGpuProgramManager;
    class GLSLProgramFactory;
    class GLSLGpuProgram;
    class GLSLLinkProgram;
    class HardwareBufferManager;

    /**
      Implementation of GL 3 as a rendering system.
     */
    class _OgreGL3PlusExport GL3PlusRenderSystem : public RenderSystem
    {
        private:
            /// Rendering loop control
            bool mStopRendering;

            typedef HashMap<GLenum, GLuint>  BindBufferMap;

            /// View matrix to set world against
            Matrix4 mViewMatrix;
            Matrix4 mWorldMatrix;
            Matrix4 mTextureMatrix;

            /// Last min & mip filtering options, so we can combine them
            FilterOptions mMinFilter;
            FilterOptions mMipFilter;

            bool mTextureCompareEnabled;

            /** Used to store the number of mipmaps in the currently bound texture.  This is then
             used to modify the texture unit filtering.
             */
            size_t mTextureMipmapCount;

            /// What texture coord set each texture unit is using
            size_t mTextureCoordIndex[OGRE_MAX_TEXTURE_LAYERS];

            /// Holds texture type settings for every stage
            GLenum mTextureTypes[OGRE_MAX_TEXTURE_LAYERS];

            /// Number of fixed-function texture units
            unsigned short mFixedFunctionTextureUnits;

            void initConfigOptions(void);

            /// Store last colour write state
            bool mColourWrite[4];

            /// Store last depth write state
            bool mDepthWrite;

            /// Store last stencil mask state
            uint32 mStencilWriteMask;

            GLfloat mAutoTextureMatrix[16];

            bool mUseAutoTextureMatrix;

            /// GL support class, used for creating windows etc.
            GL3PlusSupport *mGLSupport;

            /* The main GL context - main thread only */
            GL3PlusContext *mMainContext;

            /* The current GL context  - main thread only */
            GL3PlusContext *mCurrentContext;
        
            typedef list<GL3PlusContext*>::type GL3PlusContextList;
            /// List of background thread contexts
            GL3PlusContextList mBackgroundContextList;

            GL3PlusGpuProgramManager *mGpuProgramManager;
            GLSLProgramFactory* mGLSLProgramFactory;
            HardwareBufferManager* mHardwareBufferManager;

            /** Manager object for creating render textures.
                Direct render to texture via FBO is preferable 
                to pbuffers, which depend on the GL support used and are generally 
                unwieldy and slow. However, FBO support for stencil buffers is poor.
              */
            GL3PlusRTTManager *mRTTManager;

            /** These variables are used for caching RenderSystem state.
                They are cached because OpenGL state changes can be quite expensive,
                which is especially important on mobile or embedded systems.
             */
            GLenum mActiveTextureUnit;
            BindBufferMap mActiveBufferMap;

            /// Check if the GL system has already been initialised
            bool mGLInitialised;

            // local data members of _render that were moved here to improve performance
            // (save allocations)
            vector<GLuint>::type mRenderAttribsBound;
            vector<GLuint>::type mRenderInstanceAttribsBound;

            /** 
                Cache the polygon mode value
             */
            GLenum mPolygonMode;

            GLint getCombinedMinMipFilter(void) const;

            GL3PlusGpuProgram* mCurrentVertexProgram;
            GL3PlusGpuProgram* mCurrentFragmentProgram;
            GL3PlusGpuProgram* mCurrentGeometryProgram;
            GL3PlusGpuProgram* mCurrentHullProgram;
            GL3PlusGpuProgram* mCurrentDomainProgram;
            GL3PlusGpuProgram* mCurrentComputeProgram;

            GLint getTextureAddressingMode(TextureUnitState::TextureAddressingMode tam) const;
            GLenum getBlendMode(SceneBlendFactor ogreBlend) const;

            bool activateGLTextureUnit(size_t unit);
            void bindVertexElementToGpu( const VertexElement &elem, HardwareVertexBufferSharedPtr vertexBuffer,
                                        const size_t vertexStart, 
                                        vector<GLuint>::type &attribsBound,
                                        vector<GLuint>::type &instanceAttribsBound,
                                        bool updateVAO);

        public:
            // Default constructor / destructor
            GL3PlusRenderSystem();
            ~GL3PlusRenderSystem();
        
            friend class ShaderGeneratorTechniqueResolverListener;

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
            RenderWindow* _initialise(bool autoCreateWindow, const String& windowTitle = "OGRE Render Window");
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
            void setAmbientLight(float r, float g, float b) { };   // Not supported
            /** See
              RenderSystem
             */
            void setShadingType(ShadeOptions so) { };   // Not supported
            /** See
              RenderSystem
             */
            void setLightingEnabled(bool enabled) { };   // Not supported

            /// @copydoc RenderSystem::_createRenderWindow
            RenderWindow* _createRenderWindow(const String &name, unsigned int width, unsigned int height, 
                bool fullScreen, const NameValuePairList *miscParams = 0);

            /// @copydoc RenderSystem::_createRenderWindows
            bool _createRenderWindows(const RenderWindowDescriptionList& renderWindowDescriptions, 
			RenderWindowList& createdWindows);

            /// @copydoc RenderSystem::_createDepthBufferFor
            DepthBuffer* _createDepthBufferFor( RenderTarget *renderTarget );

            /// Mimics D3D9RenderSystem::_getDepthStencilFormatFor, if no FBO RTT manager, outputs GL_NONE
            void _getDepthStencilFormatFor( GLenum internalColourFormat, GLenum *depthFormat,
                                            GLenum *stencilFormat );

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
            void setNormaliseNormals(bool normalise) { };   // Not supported

            // -----------------------------
            // Low-level overridden members
            // -----------------------------
            /** See
             RenderSystem
             */
            void _useLights(const LightList& lights, unsigned short limit) { };   // Not supported
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
                                   TrackVertexColourType tracking) {}
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
                    const Frustum* frustum = 0) { };   // Not supported
            /** See
             RenderSystem
             */
            void _setTextureBlendMode(size_t stage, const LayerBlendModeEx& bm) { };   // Not supported
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
            void _setTextureMatrix(size_t stage, const Matrix4& xform) { };   // Not supported
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
             RenderSystem.
             */
            void setStencilBufferParams(CompareFunction func = CMPF_ALWAYS_PASS, 
                    uint32 refValue = 0, uint32 compareMask = 0xFFFFFFFF, uint32 writeMask = 0xFFFFFFFF,
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
            void _setTextureUnitCompareFunction(size_t unit, CompareFunction function);
            /** See
             RenderSystem
             */
            void _setTextureUnitCompareEnabled(size_t unit, bool compare);
            /** See
             RenderSystem
             */
            void _setTextureLayerAnisotropy(size_t unit, unsigned int maxAnisotropy);
            /** See
             RenderSystem
             */
            void setVertexDeclaration(VertexDeclaration* decl) {}
            /** See
             RenderSystem
             */
            void setVertexDeclaration(VertexDeclaration* decl, VertexBufferBinding* binding) {}
            /** See
             RenderSystem.
             */
            void setVertexBufferBinding(VertexBufferBinding* binding) {}
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
            Real getHorizontalTexelOffset(void) { return 0.0; }               // No offset in GL
            Real getVerticalTexelOffset(void) { return 0.0; }                 // No offset in GL
            Real getMinimumDepthInputValue(void) { return -1.0f; }            // Range [-1.0f, 1.0f]
            Real getMaximumDepthInputValue(void) { return 1.0f; }             // Range [-1.0f, 1.0f]
            OGRE_MUTEX(mThreadInitMutex);
            void registerThread();
            void unregisterThread();
            void preExtraThreadsStarted();
            void postExtraThreadsStarted();
            void setClipPlanesImpl(const Ogre::PlaneList& planeList);

            // ----------------------------------
            // GL3PlusRenderSystem specific members
            // ----------------------------------
            /** Returns the main context */
            GL3PlusContext* _getMainContext() { return mMainContext; }
            /** Unregister a render target->context mapping. If the context of target 
             is the current context, change the context to the main context so it
             can be destroyed safely. 
             
             @note This is automatically called by the destructor of 
             GL3PlusContext.
             */
            void _unregisterContext(GL3PlusContext *context);
            /** Switch GL context, dealing with involved internal cached states too
             */
            void _switchContext(GL3PlusContext *context);
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
            GLint convertStencilOp(StencilOperation op, bool invert = false) const;

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

            GLenum _getPolygonMode(void) { return mPolygonMode; }

            void _setSceneBlendingOperation(SceneBlendOperation op);
            void _setSeparateSceneBlendingOperation(SceneBlendOperation op, SceneBlendOperation alphaOp);
            /// @copydoc RenderSystem::hasAnisotropicMipMapFilter
            virtual bool hasAnisotropicMipMapFilter() const { return false; }

            /// @copydoc RenderSystem::beginProfileEvent
            virtual void beginProfileEvent( const String &eventName );

            /// @copydoc RenderSystem::endProfileEvent
            virtual void endProfileEvent( void );

            /// @copydoc RenderSystem::markProfileEvent
            virtual void markProfileEvent( const String &eventName );
    };
}

#endif
