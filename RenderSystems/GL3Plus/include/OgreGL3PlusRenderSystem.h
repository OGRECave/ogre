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
#include "OgreRenderWindow.h"
#include "OgreGLRenderSystemCommon.h"
#include "OgreGL3PlusStateCacheManager.h"

#include <array>

namespace Ogre {
    /** \addtogroup RenderSystems RenderSystems
    *  @{
    */
    /** \defgroup GL3Plus GL3Plus
    * Implementation of GL 3 as a rendering system.
    *  @{
    */
    class GLSLShaderManager;
    class GLSLProgram;
    class GLSLProgramManager;
    class HardwareBufferManager;

    /**
       Implementation of GL 3 as a rendering system.
    */
    class _OgreGL3PlusExport GL3PlusRenderSystem : public GLRenderSystemCommon
    {
        friend class GL3PlusSampler;
    private:
        typedef std::unordered_map<GLenum, GLuint>  BindBufferMap;

        /// Last min & mip filtering options, so we can combine them
        FilterOptions mMinFilter;
        FilterOptions mMipFilter;

        /// Holds texture type settings for every stage
        GLenum mTextureTypes[OGRE_MAX_TEXTURE_LAYERS];

        GLint mLargestSupportedAnisotropy;

        /// Store last depth write state
        bool mDepthWrite;

        /// Store last stencil mask state
        uint32 mStencilWriteMask;

        // statecaches are per context
        GL3PlusStateCacheManager* mStateCacheManager;

        GLSLProgramManager* mProgramManager;
        HighLevelGpuProgramFactory* mGLSLShaderFactory;
        HighLevelGpuProgramFactory* mSPIRVShaderFactory;
        HardwareBufferManager* mHardwareBufferManager;

        /** These variables are used for caching RenderSystem state.
            They are cached because OpenGL state changes can be quite expensive,
            which is especially important on mobile or embedded systems.
        */
        GLenum mActiveTextureUnit;
        BindBufferMap mActiveBufferMap;

        /// Check if the GL system has already been initialised
        bool mGLInitialised;

        bool mSeparateShaderObjectsEnabled;

#if OGRE_NO_QUAD_BUFFER_STEREO == 0
		/// @copydoc RenderSystem::setDrawBuffer
		virtual bool setDrawBuffer(ColourBufferType colourBuffer);
#endif

        std::array<GLSLShader*, GPT_COUNT> mCurrentShader;

        GLenum getBlendMode(SceneBlendFactor ogreBlend) const;

        void bindVertexElementToGpu(const VertexElement& elem,
                                    const HardwareVertexBufferSharedPtr& vertexBuffer,
                                    const size_t vertexStart) override;
        /** Initialises GL extensions, must be done AFTER the GL context has been
            established.
        */
        void initialiseExtensions();
    public:
        // Default constructor / destructor
        GL3PlusRenderSystem();
        ~GL3PlusRenderSystem();

        // ----------------------------------
        // Overridden RenderSystem functions
        // ----------------------------------

        const String& getName(void) const override;

        void _initialise() override;

        void initConfigOptions() override;

        RenderSystemCapabilities* createRenderSystemCapabilities() const override;

        void initialiseFromRenderSystemCapabilities(RenderSystemCapabilities* caps, RenderTarget* primary) override;

        void shutdown(void) override;

        /// @copydoc RenderSystem::_createRenderWindow
        RenderWindow* _createRenderWindow(const String &name, unsigned int width, unsigned int height,
                                          bool fullScreen, const NameValuePairList *miscParams = 0) override;

        /// @copydoc RenderSystem::_createDepthBufferFor
        DepthBuffer* _createDepthBufferFor( RenderTarget *renderTarget ) override;

        /// @copydoc RenderSystem::createMultiRenderTarget
        MultiRenderTarget * createMultiRenderTarget(const String & name) override;


        void destroyRenderWindow(const String& name) override;

        // -----------------------------
        // Low-level overridden members
        // -----------------------------
        void _setTexture(size_t unit, bool enabled, const TexturePtr &tex) override;

        void _setSampler(size_t unit, Sampler& sampler) override;

        void _setLineWidth(float width) override;

        void _setViewport(Viewport *vp) override;

        void _endFrame(void) override;

        void _setCullingMode(CullingMode mode) override;

        void _setDepthClamp(bool enable) override;

        void _setDepthBufferParams(bool depthTest = true, bool depthWrite = true, CompareFunction depthFunction = CMPF_LESS_EQUAL) override;

        void _setDepthBias(float constantBias, float slopeScaleBias) override;

        void setColourBlendState(const ColourBlendState& state) override;

        void _setPolygonMode(PolygonMode level) override;

        void setStencilState(const StencilState& state) override;

        void _dispatchCompute(const Vector3i& workgroupDim) override;

        void _render(const RenderOperation& op) override;

        void _getDepthStencilFormatFor(PixelFormat internalColourFormat,
                                       uint32* depthFormat,
                                       uint32* stencilFormat) override;

        void setScissorTest(bool enabled, const Rect& rect = Rect()) override;

        void clearFrameBuffer(unsigned int buffers,
                              const ColourValue& colour = ColourValue::Black,
                              float depth = 1.0f, unsigned short stencil = 0) override;
        HardwareOcclusionQuery* createHardwareOcclusionQuery(void) override;

        // ----------------------------------
        // GL3PlusRenderSystem specific members
        // ----------------------------------
        GL3PlusStateCacheManager * _getStateCacheManager() { return mStateCacheManager; }

        /** Create VAO on current context */
        uint32 _createVao() override;
        /** Bind VAO, context should be equal to current context, as VAOs are not shared  */
        void _bindVao(GLContext* context, uint32 vao) override;
        /** Destroy VAO immediately or defer if it was created on other context */
        void _destroyVao(GLContext* context, uint32 vao) override;
        /** Destroy FBO immediately or defer if it was created on other context */
        void _destroyFbo(GLContext* context, uint32 fbo) override;

        /** Unregister a render target->context mapping. If the context of target
            is the current context, change the context to the main context so it
            can be destroyed safely.

            @note This is automatically called by the destructor of
            GL3PlusContext.
        */
        void _unregisterContext(GL3PlusContext *context) override;
        /** Switch GL context, dealing with involved internal cached states too
         */
        void _switchContext(GL3PlusContext *context);
        void _oneTimeContextInitialization() override;
        void initialiseContext(RenderWindow* primary);
        /**
         * Set current render target to target, enabling its GL context if needed
         */
        void _setRenderTarget(RenderTarget *target) override;

        static GLint convertCompareFunction(CompareFunction func);
        static GLint convertStencilOp(StencilOperation op, bool invert = false);

        void bindGpuProgram(GpuProgram* prg) override;
        void unbindGpuProgram(GpuProgramType gptype) override;
        void bindGpuProgramParameters(GpuProgramType gptype, const GpuProgramParametersPtr& params, uint16 mask) override;

        /// @copydoc RenderSystem::_setAlphaRejectSettings
        void _setAlphaRejectSettings( CompareFunction func, unsigned char value, bool alphaToCoverage ) override;

        /// @copydoc RenderSystem::beginProfileEvent
        void beginProfileEvent( const String &eventName ) override;

        /// @copydoc RenderSystem::endProfileEvent
        void endProfileEvent( void ) override;

        /// @copydoc RenderSystem::markProfileEvent
        void markProfileEvent( const String &eventName ) override;

        /** @copydoc RenderTarget::copyContentsToMemory */
        void _copyContentsToMemory(Viewport* vp, const Box& src, const PixelBox &dst, RenderWindow::FrameBuffer buffer) override;
    };
    /** @} */
    /** @} */
}

#endif
