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
#ifndef __OgreGLFBORTT_H__
#define __OgreGLFBORTT_H__

#include "OgreGLCopyingRenderTexture.h"
#include "OgreGLContext.h"
#include "OgreGLFrameBufferObject.h"

/// Extra GL constants
#define GL_DEPTH24_STENCIL8_EXT                           0x88F0


namespace Ogre {
    class GLFBOManager;

    /** RenderTexture for GL FBO
    */
    class _OgreGLExport GLFBORenderTexture: public GLRenderTexture
    {
    public:
        GLFBORenderTexture(GLFBOManager *manager, const String &name, const GLSurfaceDesc &target, bool writeGamma, uint fsaa);

        virtual void getCustomAttribute(const String& name, void* pData);

        /// Override needed to deal with multisample buffers
        virtual void swapBuffers();

        /// Override so we can attach the depth buffer to the FBO
        virtual bool attachDepthBuffer( DepthBuffer *depthBuffer );
        virtual void detachDepthBuffer();
        virtual void _detachDepthBuffer();
    protected:
        GLFrameBufferObject mFB;
    };
    
    /** Factory for GL Frame Buffer Objects, and related things.
    */
    class _OgreGLExport GLFBOManager: public GLRTTManager
    {
    public:
        GLFBOManager(bool atimode);
        ~GLFBOManager();
        
        /** Bind a certain render target if it is a FBO. If it is not a FBO, bind the
            main frame buffer.
        */
        void bind(RenderTarget *target);
        
        /** Unbind a certain render target. No-op for FBOs.
        */
        void unbind(RenderTarget *target) {};
        
        /** Get best depth and stencil supported for given internalFormat
        */
        void getBestDepthStencil(PixelFormat internalFormat, GLenum *depthFormat, GLenum *stencilFormat);
        
        /** Create a texture rendertarget object
        */
        virtual GLFBORenderTexture *createRenderTexture(const String &name, 
            const GLSurfaceDesc &target, bool writeGamma, uint fsaa);

        /** Create a multi render target 
        */
        virtual MultiRenderTarget* createMultiRenderTarget(const String & name);
        
        /** Request a render buffer. If format is GL_NONE, return a zero buffer.
        */
        GLSurfaceDesc requestRenderBuffer(GLenum format, uint32 width, uint32 height, uint fsaa);
        /** Get a FBO without depth/stencil for temporary use, like blitting between textures.
        */
        GLuint getTemporaryFBO() { return mTempFBO; }
    private:
        /** Temporary FBO identifier
         */
        GLuint mTempFBO;
        
        /// Buggy ATI driver?
        bool mATIMode;
        
        /** Detect allowed FBO formats */
        void detectFBOFormats();
        GLuint _tryFormat(GLenum depthFormat, GLenum stencilFormat);
        bool _tryPackedFormat(GLenum packedFormat);
        void _createTempFramebuffer(GLuint fmt, GLuint &fb, GLuint &tid);
    };
    

}

#endif
