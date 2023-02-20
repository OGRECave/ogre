/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#ifndef __OgreGLRenderSystemCommon_H__
#define __OgreGLRenderSystemCommon_H__

#include "OgreGLSupportPrerequisites.h"
#include "OgreRenderSystem.h"
#include "OgreRenderWindow.h"

// Convenience macro from ARB_vertex_buffer_object spec
#define VBO_BUFFER_OFFSET(i) ((char *)size_t(i))

namespace Ogre {
    class GLContext;
    class GLSLProgramCommon;
    class GLNativeSupport;
    class GLRTTManager;

    class _OgreGLExport GLRenderSystemCommon : public RenderSystem
    {
    protected:
        /* The main GL context - main thread only */
        GLContext* mMainContext;

        /* The current GL context  - main thread only */
        GLContext* mCurrentContext;

        // GL support class, used for creating windows etc.
        GLNativeSupport* mGLSupport;

        // This contains the complete list of supported extensions
        std::set<String> mExtensionList;
        GPUVendor mVendor;

        /** Manager object for creating render textures.
            Direct render to texture via FBO is preferable
            to pbuffers, which depend on the GL support used and are generally
            unwieldy and slow. However, FBO support for stencil buffers is poor.
        */
        GLRTTManager *mRTTManager;

        void initConfigOptions() override;
        void refreshConfig();

        typedef std::list<GLContext*> GLContextList;
        /// List of background thread contexts
        GLContextList mBackgroundContextList;
        OGRE_MUTEX(mThreadInitMutex);

        /** One time initialization for the RenderState of a context. Things that
            only need to be set once, like the LightingModel can be defined here.
        */
        virtual void _oneTimeContextInitialization() = 0;
    public:
        /**
         Specific options:

        | Key |  Default | Description |
        |-----|---------------|---------|
        | Reversed Z-Buffer | false | Use reverse depth buffer to improve depth precision (GL3+ only) |
        | Separate Shader Objects | false | Compile shaders individually instad of using monolithic programs. Better introspection. Allows mixing GLSL and SPIRV shaders (GL3+ only)  |
        | Fixed Pipeline Enabled | true | Use fixed function units where possible. Disable to test migration to shader-only pipeline (GL only) |
        */
        void setConfigOption(const String &name, const String &value) override;

        virtual ~GLRenderSystemCommon() {}

        /** @copydoc RenderTarget::copyContentsToMemory */
        virtual void _copyContentsToMemory(Viewport* vp, const Box& src, const PixelBox& dst,
                                           RenderWindow::FrameBuffer buffer) = 0;

        /** Returns the main context */
        GLContext* _getMainContext() { return mMainContext; }

        /** Returns the current context */
        GLContext* _getCurrentContext() { return mCurrentContext; }

        /**
        * Check if GL Version is supported
        */
        bool hasMinGLVersion(int major, int minor) const;

        /**
        * Check if an extension is available
        */
        bool checkExtension(const String& ext) const;

        /** Unregister a render target->context mapping. If the context of target
            is the current context, change the context to the main context so it
            can be destroyed safely.

            @note This is automatically called by the destructor of
            GLContext.
        */
        virtual void _unregisterContext(GLContext *context) = 0;

        virtual void bindVertexElementToGpu(const VertexElement& elem,
                                            const HardwareVertexBufferSharedPtr& vertexBuffer,
                                            const size_t vertexStart) = 0;

        Real getMinimumDepthInputValue(void) override { return -1.0f; }            // Range [-1.0f, 1.0f]
        Real getMaximumDepthInputValue(void) override { return 1.0f; }             // Range [-1.0f, 1.0f]

        void _convertProjectionMatrix(const Matrix4& matrix, Matrix4& dest, bool) override;

        /// Mimics D3D9RenderSystem::_getDepthStencilFormatFor, if no FBO RTT manager, outputs GL_NONE
        virtual void _getDepthStencilFormatFor(PixelFormat internalColourFormat,
                                               uint32* depthFormat,
                                               uint32* stencilFormat);

        /** Create VAO on current context */
        virtual uint32 _createVao() { return 0; }
        /** Bind VAO, context should be equal to current context, as VAOs are not shared  */
        virtual void _bindVao(GLContext* context, uint32 vao) {}
        /** Destroy VAO immediately or defer if it was created on other context */
        virtual void _destroyVao(GLContext* context, uint32 vao) {}
        /** Destroy FBO immediately or defer if it was created on other context */
        virtual void _destroyFbo(GLContext* context, uint32 fbo) {}
        /** Complete destruction of VAOs and FBOs deferred while creator context was not current */
        void _completeDeferredVaoFboDestruction();

        void registerThread() override;
        void unregisterThread() override;
        void preExtraThreadsStarted() override;
        void postExtraThreadsStarted() override;

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
        virtual void resetRenderer(RenderWindow* pRenderWnd) = 0;
        virtual void notifyOnContextLost() = 0;
#endif
    };
}

#endif
