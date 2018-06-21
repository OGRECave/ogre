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
#include "OgreGL3PlusStateCacheManager.h"

namespace Ogre {
    /** \addtogroup RenderSystems RenderSystems
    *  @{
    */
    /** \defgroup GL3Plus GL3Plus
    * Implementation of GL 3 as a rendering system.
    *  @{
    */
    class GLSLShaderManager;
    class GLSLShaderFactory;
    class GLSLProgram;
    class HardwareBufferManager;

    /**
       Implementation of GL 3 as a rendering system.
    */
    class _OgreGL3PlusExport GL3PlusRenderSystem : public GLRenderSystemCommon
    {
    private:
        /// Rendering loop control
        bool mStopRendering;

        typedef std::unordered_map<GLenum, GLuint>  BindBufferMap;

        /// Last min & mip filtering options, so we can combine them
        FilterOptions mMinFilter;
        FilterOptions mMipFilter;

        /// Holds texture type settings for every stage
        GLenum mTextureTypes[OGRE_MAX_TEXTURE_LAYERS];

        GLint mLargestSupportedAnisotropy;

        void initConfigOptions(void);

        /// Store last colour write state
        bool mColourWrite[4];

        /// Store last depth write state
        bool mDepthWrite;

        /// Store last scissor enable state
        bool mScissorsEnabled;

        /// Store scissor box
        int mScissorBox[4];

        /// Store last stencil mask state
        uint32 mStencilWriteMask;

        /// GL support class, used for creating windows etc.
        GL3PlusSupport *mGLSupport;

        typedef std::list<GL3PlusContext*> GL3PlusContextList;
        /// List of background thread contexts
        GL3PlusContextList mBackgroundContextList;

        // statecaches are per context
        GL3PlusStateCacheManager* mStateCacheManager;

        GLSLShaderManager *mShaderManager;
        GLSLShaderFactory* mGLSLShaderFactory;
        HardwareBufferManager* mHardwareBufferManager;

        /** Manager object for creating render textures.
            Direct render to texture via FBO is preferable
            to pbuffers, which depend on the GL support used and are generally
            unwieldy and slow. However, FBO support for stencil buffers is poor.
        */
        GLRTTManager *mRTTManager;

        /** These variables are used for caching RenderSystem state.
            They are cached because OpenGL state changes can be quite expensive,
            which is especially important on mobile or embedded systems.
        */
        GLenum mActiveTextureUnit;
        BindBufferMap mActiveBufferMap;

        /// Check if the GL system has already been initialised
        bool mGLInitialised;

#if OGRE_NO_QUAD_BUFFER_STEREO == 0
		/// @copydoc RenderSystem::setDrawBuffer
		virtual bool setDrawBuffer(ColourBufferType colourBuffer);
#endif

        GLint getCombinedMinMipFilter(void) const;

        GLSLShader* mCurrentVertexShader;
        GLSLShader* mCurrentFragmentShader;
        GLSLShader* mCurrentGeometryShader;
        GLSLShader* mCurrentHullShader;
        GLSLShader* mCurrentDomainShader;
        GLSLShader* mCurrentComputeShader;

        GLint getTextureAddressingMode(TextureUnitState::TextureAddressingMode tam) const;
        GLenum getBlendMode(SceneBlendFactor ogreBlend) const;

        void bindVertexElementToGpu(const VertexElement& elem,
                                    const HardwareVertexBufferSharedPtr& vertexBuffer,
                                    const size_t vertexStart);

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

        void shutdown(void);

        /// @copydoc RenderSystem::_createRenderWindow
        RenderWindow* _createRenderWindow(const String &name, unsigned int width, unsigned int height,
                                          bool fullScreen, const NameValuePairList *miscParams = 0);

        /// @copydoc RenderSystem::_createRenderWindows
        bool _createRenderWindows(const RenderWindowDescriptionList& renderWindowDescriptions,
                                  RenderWindowList& createdWindows);

        /// @copydoc RenderSystem::_createDepthBufferFor
        DepthBuffer* _createDepthBufferFor( RenderTarget *renderTarget );

        /// Mimics D3D9RenderSystem::_getDepthStencilFormatFor, if no FBO RTT manager, outputs GL_NONE
        void _getDepthStencilFormatFor( PixelFormat internalColourFormat, GLenum *depthFormat,
                                        GLenum *stencilFormat );

        /// @copydoc RenderSystem::createMultiRenderTarget
        virtual MultiRenderTarget * createMultiRenderTarget(const String & name);


        void destroyRenderWindow(const String& name);

        // -----------------------------
        // Low-level overridden members
        // -----------------------------

        bool areFixedFunctionLightsInViewSpace() const { return true; }

        void _setTexture(size_t unit, bool enabled, const TexturePtr &tex);

        void _setTextureAddressingMode(size_t stage, const TextureUnitState::UVWAddressingMode& uvw);

        void _setTextureBorderColour(size_t stage, const ColourValue& colour);

        void _setTextureMipmapBias(size_t unit, float bias);

        void _setLineWidth(float width);

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

        void _render(const RenderOperation& op);

        void setScissorTest(bool enabled, size_t left = 0, size_t top = 0, size_t right = 800, size_t bottom = 600);

        void clearFrameBuffer(unsigned int buffers,
                              const ColourValue& colour = ColourValue::Black,
                              Real depth = 1.0f, unsigned short stencil = 0);
        HardwareOcclusionQuery* createHardwareOcclusionQuery(void);
        OGRE_MUTEX(mThreadInitMutex);
        void registerThread();
        void unregisterThread();
        void preExtraThreadsStarted();
        void postExtraThreadsStarted();
        void setClipPlanesImpl(const Ogre::PlaneList& planeList);
        GL3PlusSupport* getGLSupportRef() { return mGLSupport; }


        // ----------------------------------
        // GL3PlusRenderSystem specific members
        // ----------------------------------
        bool hasMinGLVersion(int major, int minor) const;
        bool checkExtension(const String& ext) const;

        GL3PlusStateCacheManager * _getStateCacheManager() { return mStateCacheManager; }

        /** Create VAO on current context */
        uint32 _createVao();
        /** Bind VAO, context should be equal to current context, as VAOs are not shared  */
        void _bindVao(GLContext* context, uint32 vao);
        /** Destroy VAO immediately or defer if it was created on other context */
        void _destroyVao(GLContext* context, uint32 vao);
        /** Destroy FBO immediately or defer if it was created on other context */
        void _destroyFbo(GLContext* context, uint32 fbo);

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
