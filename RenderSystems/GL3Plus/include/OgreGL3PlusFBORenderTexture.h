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
#ifndef __OgreGL3PlusFBORTT_H__
#define __OgreGL3PlusFBORTT_H__

#include "OgreGLRenderTexture.h"
#include "OgreGLContext.h"
#include "OgreGL3PlusFrameBufferObject.h"

namespace Ogre {
    class GL3PlusFBOManager;
    class GL3PlusRenderBuffer;
    class GL3PlusStateCacheManager;

    /** RenderTexture for GL FBO
     */
    class _OgreGL3PlusExport GL3PlusFBORenderTexture : public GLRenderTexture
    {
    public:
        GL3PlusFBORenderTexture(GL3PlusFBOManager *manager, const String &name, const GLSurfaceDesc &target, bool writeGamma, uint fsaa);

        void getCustomAttribute(const String& name, void* pData) override;

        /// Override needed to deal with multisample buffers
        void swapBuffers() override;

        /// Override so we can attach the depth buffer to the FBO
        bool attachDepthBuffer( DepthBuffer *depthBuffer ) override;
        void detachDepthBuffer() override;
        void _detachDepthBuffer() override;

        GLContext* getContext() const override { return mFB.getContext(); }
        GLFrameBufferObjectCommon* getFBO() override { return &mFB; }
    protected:
        GL3PlusFrameBufferObject mFB;
    };

    /** Factory for GL Frame Buffer Objects, and related things.
     */
    class _OgreGL3PlusExport GL3PlusFBOManager: public GL3PlusRTTManager
    {
    public:
        GL3PlusFBOManager(GL3PlusRenderSystem* renderSystem);
        ~GL3PlusFBOManager();

        /** Get best depth and stencil supported for given internalFormat
         */
        void getBestDepthStencil(PixelFormat internalFormat, GLenum *depthFormat, GLenum *stencilFormat) override;

        /** Create a texture rendertarget object
         */
        GL3PlusFBORenderTexture *createRenderTexture(const String &name,
                                                             const GLSurfaceDesc &target, bool writeGamma, uint fsaa) override;

        GLSurfaceDesc createNewRenderBuffer(unsigned format, uint32 width, uint32 height, uint fsaa) override;

        GL3PlusStateCacheManager* getStateCacheManager();
    private:
        GL3PlusRenderSystem* mRenderSystem;

        /** Detect allowed FBO formats */
        void detectFBOFormats();
        GLuint _tryFormat(GLenum depthFormat, GLenum stencilFormat);
        bool _tryPackedFormat(GLenum packedFormat);
        void _createTempFramebuffer(GLuint internalFormat, GLuint fmt, GLenum dataType, GLuint &fb, GLuint &tid);
    };
}

#endif
