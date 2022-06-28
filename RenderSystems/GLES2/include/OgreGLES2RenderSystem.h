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

#ifndef __GLES2RenderSystem_H__
#define __GLES2RenderSystem_H__

#include "OgreGLES2Prerequisites.h"

#include "OgreMaterialManager.h"
#include "OgreRenderSystem.h"
#include "OgreGLSLESProgram.h"
#include "OgreGLRenderSystemCommon.h"

namespace Ogre {
    /** \addtogroup RenderSystems RenderSystems
    *  @{
    */
    /** \defgroup GLES2 GLES2
    * Implementation of GL ES 2.x as a rendering system.
    *  @{
    */
    class GLES2FBOManager;
    class GLSLESProgramCommon;
    class GLSLESProgramManager;
    class GLSLESProgramFactory;
    class GLES2StateCacheManager;
#if !OGRE_NO_GLES2_CG_SUPPORT
    class GLSLESCgProgramFactory;
#endif
    class HardwareBufferManager;
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
    class GLES2ManagedResourceManager;
#endif
    
    /**
      Implementation of GL ES 2.x as a rendering system.
     */
    class _OgreGLES2Export GLES2RenderSystem : public GLRenderSystemCommon
    {
        private:
            /// Last min & mip filtering options, so we can combine them
            FilterOptions mMinFilter;
            FilterOptions mMipFilter;

            /// Holds texture type settings for every stage
            GLenum mTextureTypes[OGRE_MAX_TEXTURE_LAYERS];

            /// State cache manager which responsible to reduce redundant state changes
            GLES2StateCacheManager* mStateCacheManager;

            GLSLESProgramManager* mProgramManager;
            GLSLESProgramFactory* mGLSLESProgramFactory;
#if !OGRE_NO_GLES2_CG_SUPPORT
            GLSLESCgProgramFactory* mGLSLESCgProgramFactory;
#endif
            HardwareBufferManager* mHardwareBufferManager;

            /// Check if the GL system has already been initialised
            bool mGLInitialised;

            // local data member of _render that were moved here to improve performance
            // (save allocations)
            std::vector<GLuint> mRenderAttribsBound;
            std::vector<GLuint> mRenderInstanceAttribsBound;

            GLenum mPolygonMode;

            GLSLESProgram* mCurrentVertexProgram;
            GLSLESProgram* mCurrentFragmentProgram;

            GLint getTextureAddressingMode(TextureAddressingMode tam) const;
            GLenum getBlendMode(SceneBlendFactor ogreBlend) const;
            void bindVertexElementToGpu(const VertexElement& elem,
                                        const HardwareVertexBufferSharedPtr& vertexBuffer,
                                        const size_t vertexStart);

            /** Initialises GL extensions, must be done AFTER the GL context has been
                established.
            */
            void initialiseExtensions();

            // Mipmap count of the actual bounded texture
            size_t mCurTexMipCount;

        public:
            // Default constructor / destructor
            GLES2RenderSystem();
            virtual ~GLES2RenderSystem();

            // ----------------------------------
            // Overridden RenderSystem functions
            // ----------------------------------

            const String& getName(void) const;

            void _initialise() override;

            virtual RenderSystemCapabilities* createRenderSystemCapabilities() const;

            void initialiseFromRenderSystemCapabilities(RenderSystemCapabilities* caps, RenderTarget* primary);

            void shutdown(void);

            /// @copydoc RenderSystem::_createRenderWindow
            RenderWindow* _createRenderWindow(const String &name, unsigned int width, unsigned int height, 
                bool fullScreen, const NameValuePairList *miscParams = 0);

            /// @copydoc RenderSystem::_createDepthBufferFor
            DepthBuffer* _createDepthBufferFor( RenderTarget *renderTarget );

            /// @copydoc RenderSystem::createMultiRenderTarget
            virtual MultiRenderTarget * createMultiRenderTarget(const String & name);


            void destroyRenderWindow(const String& name);

            // -----------------------------
            // Low-level overridden members
            // -----------------------------
            void _setTexture(size_t unit, bool enabled, const TexturePtr &tex);

            void _setSampler(size_t unit, Sampler& sampler);

            void _setTextureAddressingMode(size_t stage, const Sampler::UVWAddressingMode& uvw);

            void _setLineWidth(float width);

            void _setViewport(Viewport *vp);

            void _endFrame(void);

            void _setCullingMode(CullingMode mode);

            void _setDepthBufferParams(bool depthTest = true, bool depthWrite = true, CompareFunction depthFunction = CMPF_LESS_EQUAL);

            void _setDepthBufferCheckEnabled(bool enabled = true);

            void _setDepthBufferWriteEnabled(bool enabled = true);

            void _setDepthBufferFunction(CompareFunction func = CMPF_LESS_EQUAL);

            void _setDepthBias(float constantBias, float slopeScaleBias);

            void setColourBlendState(const ColourBlendState& state);

            void _setPolygonMode(PolygonMode level);

            void setStencilState(const StencilState& state) override;

            virtual void _setTextureUnitFiltering(size_t unit, FilterOptions minFilter,
                FilterOptions magFilter, FilterOptions mipFilter);              

            void _setTextureUnitFiltering(size_t unit, FilterType ftype, FilterOptions filter);

            void _render(const RenderOperation& op);

            void setScissorTest(bool enabled, const Rect& rect = Rect());

            void clearFrameBuffer(unsigned int buffers,
                const ColourValue& colour = ColourValue::Black,
                float depth = 1.0f, unsigned short stencil = 0);
            HardwareOcclusionQuery* createHardwareOcclusionQuery(void);

            // ----------------------------------
            // GLES2RenderSystem specific members
            // ----------------------------------        
            GLES2StateCacheManager * _getStateCacheManager() { return mStateCacheManager; }
        
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
             GLContext.
             */
            void _unregisterContext(GLContext *context);
            /** Switch GL context, dealing with involved internal cached states too
             */
            void _switchContext(GLContext *context);
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
            void bindGpuProgramParameters(GpuProgramType gptype, const GpuProgramParametersPtr& params, uint16 mask);

            /// @copydoc RenderSystem::_setAlphaRejectSettings
            void _setAlphaRejectSettings( CompareFunction func, unsigned char value, bool alphaToCoverage );

            void _destroyDepthBuffer(RenderTarget* pRenderWnd);
        
            /// @copydoc RenderSystem::beginProfileEvent
            virtual void beginProfileEvent( const String &eventName );
            
            /// @copydoc RenderSystem::endProfileEvent
            virtual void endProfileEvent( void );
            
            /// @copydoc RenderSystem::markProfileEvent
            virtual void markProfileEvent( const String &eventName );

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
            void resetRenderer(RenderWindow* pRenderWnd);
        
            void notifyOnContextLost();

            static GLES2ManagedResourceManager* getResourceManager();
    private:
            static GLES2ManagedResourceManager* mResourceManager;
#endif
            void _copyContentsToMemory(Viewport* vp, const Box& src, const PixelBox& dst, RenderWindow::FrameBuffer buffer);
    };
}

#endif
