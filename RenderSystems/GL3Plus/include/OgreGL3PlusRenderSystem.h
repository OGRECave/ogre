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
#include "OgreGLSLShader.h"
#include "OgreRenderWindow.h"
#include "OgreGLRenderSystemCommon.h"

namespace Ogre {
    /** \addtogroup RenderSystems RenderSystems
    *  @{
    */
    /** \defgroup GL3Plus GL3Plus
    * Implementation of GL 3 as a rendering system.
    *  @{
    */
    class GL3PlusSupport;
    class GL3PlusRTTManager;
    class GLSLShaderManager;
    class GLSLShaderFactory;
    class HardwareBufferManager;

    /**
       Implementation of GL 3 as a rendering system.
    */
    class _OgreGL3PlusExport GL3PlusRenderSystem : public GLRenderSystemCommon
    {
    private:
        /// Rendering loop control
        bool mStopRendering;

        typedef OGRE_HashMap<GLenum, GLuint>  BindBufferMap;

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

        GLint mLargestSupportedAnisotropy;

        /// Number of fixed-function texture units
        unsigned short mFixedFunctionTextureUnits;

        void initConfigOptions(void);

        /// Store last colour write state
        bool mColourWrite[4];

        /// Store last depth write state
        bool mDepthWrite;

        /// Store last scissor enable state
        bool mScissorsEnabled;

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

        GLSLShaderManager *mShaderManager;
        GLSLShaderFactory* mGLSLShaderFactory;
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

        // check if GL 4.3 is supported
        bool mHasGL43;

        // check if GL 3.2 is supported
        bool mHasGL32;

        // local data members of _render that were moved here to improve performance
        // (save allocations)
        vector<GLuint>::type mRenderAttribsBound;
        vector<GLuint>::type mRenderInstanceAttribsBound;

#if OGRE_NO_QUAD_BUFFER_STEREO == 0
		/// @copydoc RenderSystem::setDrawBuffer
		virtual bool setDrawBuffer(ColourBufferType colourBuffer);
#endif

        /**
            Cache the polygon mode value
        */
        GLenum mPolygonMode;

        GLint getCombinedMinMipFilter(void) const;

        GLSLShader* mCurrentVertexShader;
        GLSLShader* mCurrentFragmentShader;
        GLSLShader* mCurrentGeometryShader;
        GLSLShader* mCurrentHullShader;
        GLSLShader* mCurrentDomainShader;
        GLSLShader* mCurrentComputeShader;

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

        const String& getName(void) const;

        ConfigOptionMap& getConfigOptions(void);

        void setConfigOption(const String &name, const String &value);

        String validateConfigOptions(void);

        RenderWindow* _initialise(bool autoCreateWindow, const String& windowTitle = "OGRE Render Window");

        virtual RenderSystemCapabilities* createRenderSystemCapabilities() const;

        void initialiseFromRenderSystemCapabilities(RenderSystemCapabilities* caps, RenderTarget* primary);

        void reinitialise(void); // Used if settings changed mid-rendering

        void shutdown(void);

        void setAmbientLight(float r, float g, float b) { };   // Not supported

        void setShadingType(ShadeOptions so) { };   // Not supported

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


        void destroyRenderWindow(const String& name);

        String getErrorDescription(long errorNumber) const;

        VertexElementType getColourVertexElementType(void) const;

        void setNormaliseNormals(bool normalise) { };   // Not supported

        // -----------------------------
        // Low-level overridden members
        // -----------------------------

        void _useLights(const LightList& lights, unsigned short limit) { };   // Not supported

        bool areFixedFunctionLightsInViewSpace() const { return true; }

        void _setWorldMatrix(const Matrix4 &m);

        void _setViewMatrix(const Matrix4 &m);

        void _setProjectionMatrix(const Matrix4 &m);

        void _setSurfaceParams(const ColourValue &ambient,
                               const ColourValue &diffuse, const ColourValue &specular,
                               const ColourValue &emissive, Real shininess,
                               TrackVertexColourType tracking) {}

        void _setPointParameters(Real size, bool attenuationEnabled,
                                 Real constant, Real linear, Real quadratic, Real minSize, Real maxSize);

        void _setPointSpritesEnabled(bool enabled);
        /** See
         RenderSystem
         */
        void _setVertexTexture(size_t unit, const TexturePtr &tex);
        /** See
         RenderSystem
         */
        void _setGeometryTexture(size_t unit, const TexturePtr &tex);
        /** See
         RenderSystem
         */
        void _setComputeTexture(size_t unit, const TexturePtr &tex);
        /** See
         RenderSystem
         */
        void _setTesselationHullTexture(size_t unit, const TexturePtr &tex);
        /** See
         RenderSystem
         */
        void _setTesselationDomainTexture(size_t unit, const TexturePtr &tex);

        void _setTexture(size_t unit, bool enabled, const TexturePtr &tex);

        void _setTextureCoordSet(size_t stage, size_t index);

        void _setTextureCoordCalculation(size_t stage, TexCoordCalcMethod m,
                                         const Frustum* frustum = 0) { };   // Not supported

        void _setTextureBlendMode(size_t stage, const LayerBlendModeEx& bm) { };   // Not supported

        void _setTextureAddressingMode(size_t stage, const TextureUnitState::UVWAddressingMode& uvw);

        void _setTextureBorderColour(size_t stage, const ColourValue& colour);

        void _setTextureMipmapBias(size_t unit, float bias);

        void _setTextureMatrix(size_t stage, const Matrix4& xform) { };   // Not supported

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

        void _setFog(FogMode mode, const ColourValue& colour, Real density, Real start, Real end) {}

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
        /** See
            RenderSystem.
        */
        void setStencilBufferParams(CompareFunction func = CMPF_ALWAYS_PASS,
                                    uint32 refValue = 0, uint32 compareMask = 0xFFFFFFFF, uint32 writeMask = 0xFFFFFFFF,
                                    StencilOperation stencilFailOp = SOP_KEEP,
                                    StencilOperation depthFailOp = SOP_KEEP,
                                    StencilOperation passOp = SOP_KEEP,
                    bool twoSidedOperation = false,
                    bool readBackAsTexture = false);

        void _setTextureUnitFiltering(size_t unit, FilterType ftype, FilterOptions filter);

        void _setTextureUnitCompareFunction(size_t unit, CompareFunction function);

        void _setTextureUnitCompareEnabled(size_t unit, bool compare);

        void _setTextureLayerAnisotropy(size_t unit, unsigned int maxAnisotropy);

        void setVertexDeclaration(VertexDeclaration* decl) {}

        void setVertexDeclaration(VertexDeclaration* decl, VertexBufferBinding* binding) {}
        /** See
            RenderSystem.
        */
        void setVertexBufferBinding(VertexBufferBinding* binding) {}

        void _render(const RenderOperation& op);

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

        /** @copydoc RenderTarget::copyContentsToMemory */
        void _copyContentsToMemory(Viewport* vp, const Box& src, const PixelBox &dst, RenderWindow::FrameBuffer buffer);
    };
    /** @} */
    /** @} */
}

#endif
