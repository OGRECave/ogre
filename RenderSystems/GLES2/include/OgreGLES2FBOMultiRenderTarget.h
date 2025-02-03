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

#ifndef __GLES2MULTIRENDERTARGET_H__
#define __GLES2MULTIRENDERTARGET_H__

#include "OgreGLES2FrameBufferObject.h"

namespace Ogre {
    
    class GLES2FBOManager;

    /** MultiRenderTarget for GL ES 2.x.
    */
    class _OgreGLES2Export GLES2FBOMultiRenderTarget : public MultiRenderTarget, public GLRenderTarget
    {
    public:
        GLES2FBOMultiRenderTarget(GLES2FBOManager *manager, const String &name);
        ~GLES2FBOMultiRenderTarget();

        void getCustomAttribute( const String& name, void *pData ) override;
        GLContext* getContext() const override { return fbo.getContext(); }
        GLFrameBufferObjectCommon* getFBO() override { return &fbo; }

        bool requiresTextureFlipping() const override { return true; }

        /// Override so we can attach the depth buffer to the FBO
        bool attachDepthBuffer( DepthBuffer *depthBuffer ) override;
        void _detachDepthBuffer() override;
    private:
        void bindSurfaceImpl(size_t attachment, RenderTexture *target) override;
        void unbindSurfaceImpl(size_t attachment) override;
        GLES2FrameBufferObject fbo;
    };

}

#endif // __GLES2MULTIRENDERTARGET_H__
