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

#include "OgreGLFBOMultiRenderTarget.h"
#include "OgreGLPixelFormat.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreRoot.h"
#include "OgreGLHardwarePixelBuffer.h"

namespace Ogre {

    GLFBOMultiRenderTarget::GLFBOMultiRenderTarget(GLFBOManager *manager, const String &name):
        MultiRenderTarget(name),
        fbo(manager, 0 /* TODO: multisampling on MRTs? */)
    {
    }

    GLFBOMultiRenderTarget::~GLFBOMultiRenderTarget()
    {
    }

    void GLFBOMultiRenderTarget::bindSurfaceImpl(size_t attachment, RenderTexture *target)
    {
        /// Check if the render target is in the rendertarget->FBO map
        auto fbobj = dynamic_cast<GLRenderTarget*>(target)->getFBO();
        assert(fbobj);
        fbo.bindSurface(attachment, fbobj->getSurface(0));

        // Set width and height
        mWidth = fbo.getWidth();
        mHeight = fbo.getHeight();
    }

    void GLFBOMultiRenderTarget::unbindSurfaceImpl(size_t attachment)
    {
        fbo.unbindSurface(attachment);

        // Set width and height
        mWidth = fbo.getWidth();
        mHeight = fbo.getHeight();
    }

    void GLFBOMultiRenderTarget::getCustomAttribute( const String& name, void *pData )
    {
        if( name == GLRenderTexture::CustomAttributeString_FBO )
        {
            *static_cast<GLFrameBufferObject **>(pData) = &fbo;
        }
    }
    //-----------------------------------------------------------------------------
    bool GLFBOMultiRenderTarget::attachDepthBuffer( DepthBuffer *depthBuffer )
    {
        bool result;
        if( (result = MultiRenderTarget::attachDepthBuffer( depthBuffer )) )
            fbo.attachDepthBuffer( depthBuffer );

        return result;
    }
    //-----------------------------------------------------------------------------
    void GLFBOMultiRenderTarget::detachDepthBuffer()
    {
        fbo.detachDepthBuffer();
        MultiRenderTarget::detachDepthBuffer();
    }
    //-----------------------------------------------------------------------------
    void GLFBOMultiRenderTarget::_detachDepthBuffer()
    {
        fbo.detachDepthBuffer();
        MultiRenderTarget::_detachDepthBuffer();
    }
}
