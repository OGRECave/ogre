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

#include "OgreGL3PlusPrerequisites.h"
#include "OgreGLRenderTexture.h"
#include "OgreGLContext.h"

namespace Ogre {
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
        GLFBORenderTexture* createRenderTexture(const String& name, const GLSurfaceDesc& target,
                                                bool writeGamma) override;

        GLHardwarePixelBufferCommon* createNewRenderBuffer(unsigned format, uint32 width, uint32 height,
                                                           uint fsaa) override;

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
