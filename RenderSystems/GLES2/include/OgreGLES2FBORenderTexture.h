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
#ifndef __OgreGLES2FBORTT_H__
#define __OgreGLES2FBORTT_H__

#include "OgreGLES2FrameBufferObject.h"
#include "OgreGLES2ManagedResource.h"
#include "OgreGLRenderTexture.h"
#include "OgreGLContext.h"

namespace Ogre {
    class GLES2FBOManager;
    class GLES2RenderBuffer;

    /** RenderTexture for GL ES 2 FBO
    */
    class _OgreGLES2Export GLES2FBORenderTexture: public GLRenderTexture MANAGED_RESOURCE
    {
    public:
        GLES2FBORenderTexture(GLES2FBOManager *manager, const String &name, const GLSurfaceDesc &target, bool writeGamma, uint fsaa);
        
        virtual void getCustomAttribute(const String& name, void* pData);

        /// Override needed to deal with multisample buffers
        virtual void swapBuffers();

        /// Override so we can attach the depth buffer to the FBO
        virtual bool attachDepthBuffer( DepthBuffer *depthBuffer );
        virtual void detachDepthBuffer();
        virtual void _detachDepthBuffer();

        GLContext* getContext() const { return mFB.getContext(); }
        GLFrameBufferObjectCommon* getFBO() { return &mFB; }
    protected:
        GLES2FrameBufferObject mFB;
        
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
        /** See AndroidResource. */
        virtual void notifyOnContextLost();
        
        /** See AndroidResource. */
        virtual void notifyOnContextReset();
#endif
    };
    
    /** Factory for GL ES 2 Frame Buffer Objects, and related things.
    */
    class _OgreGLES2Export GLES2FBOManager: public GLRTTManager 
    {
    public:
        GLES2FBOManager();
        ~GLES2FBOManager();
        
        /** Bind a certain render target if it is a FBO. If it is not a FBO, bind the
            main frame buffer.
        */
        void bind(RenderTarget *target);

        /** Get best depth and stencil supported for given internalFormat
        */
        void getBestDepthStencil(PixelFormat internalFormat, uint32 *depthFormat, uint32 *stencilFormat);
        
        /** Create a texture rendertarget object
        */
        virtual GLES2FBORenderTexture *createRenderTexture(const String &name, 
            const GLSurfaceDesc &target, bool writeGamma, uint fsaa);
        
        /** Request a render buffer. If format is GL_NONE, return a zero buffer.
        */
        GLSurfaceDesc requestRenderBuffer(GLenum format, uint32 width, uint32 height, uint fsaa);

        /** Get a FBO without depth/stencil for temporary use, like blitting between textures.
        */
        GLuint getTemporaryFBO() { return mTempFBO; }
        
        /** Detects all supported fbo's and recreates the tempory fbo */
        void _reload();
        
        GLint getMaxFSAASamples() { return mMaxFSAASamples; }

    private:
        // map(format, sizex, sizey) -> [GLSurface*,refcount]
        
        /** Temporary FBO identifier
         */
        GLuint mTempFBO;
        
        GLint mMaxFSAASamples;

        /** Detect allowed FBO formats */
        void detectFBOFormats();
        GLuint _tryFormat(GLenum depthFormat, GLenum stencilFormat);
        bool _tryPackedFormat(GLenum packedFormat);
        void _createTempFramebuffer(GLuint internalFormat, GLuint fmt, GLenum dataType, GLuint &fb, GLuint &tid);
    };
}

#endif
