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
#include "OgreGLPBRenderTexture.h"
#include "OgreGLContext.h"
#include "OgreGLPixelFormat.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreRoot.h"
#include "OgreGLHardwarePixelBuffer.h"
#include "OgreGLNativeSupport.h"

namespace Ogre {
//-----------------------------------------------------------------------------  
    GLPBRenderTexture::GLPBRenderTexture(GLPBRTTManager *manager, const String &name, 
        const GLSurfaceDesc &target, bool writeGamma):
        GLRenderTexture(name, target, writeGamma),
        mManager(manager)
    {
        mPBFormat = PixelUtil::getComponentType(target.buffer->getFormat());
        mDepthBufferPoolId = RBP_NONE; // not poolable
        
        mManager->requestPBuffer(mPBFormat, mWidth, mHeight);
    }
    GLPBRenderTexture::~GLPBRenderTexture()
    {
        // Release PBuffer
        mManager->releasePBuffer(mPBFormat);
    }
    void GLPBRenderTexture::getCustomAttribute(const String& name, void* pData)
    {
        if( name == GLRenderTexture::CustomAttributeString_TARGET )
        {
            GLSurfaceDesc &target = *static_cast<GLSurfaceDesc*>(pData);
            target.buffer = static_cast<GLHardwarePixelBufferCommon*>(mBuffer);
            target.zoffset = mZOffset;
        }
        else if (name == GLRenderTexture::CustomAttributeString_GLCONTEXT )
        {
            // Get PBuffer for our internal format
            *static_cast<GLContext**>(pData) = getContext();
        }
    }

    GLContext* GLPBRenderTexture::getContext() const
    {
        return mManager->getContextFor(mPBFormat, mWidth, mHeight);
    }

//-----------------------------------------------------------------------------  
    GLPBRTTManager::GLPBRTTManager(GLNativeSupport *support, RenderTarget *mainwindow):
        mSupport(support),
        mMainWindow(mainwindow),
        mMainContext(0)
    {
        mMainContext = dynamic_cast<GLRenderTarget*>(mMainWindow)->getContext();
    }  
    GLPBRTTManager::~GLPBRTTManager()
    {
        // Delete remaining PBuffers
        for(auto & mPBuffer : mPBuffers)
        {
            delete mPBuffer.pb;
        }
    }

    RenderTexture *GLPBRTTManager::createRenderTexture(const String &name, 
        const GLSurfaceDesc &target, bool writeGamma)
    {
        return new GLPBRenderTexture(this, name, target, writeGamma);
    }
    
    bool GLPBRTTManager::checkFormat(PixelFormat format) 
    { 
        return true; 
    }

    void GLPBRTTManager::bind(RenderTarget *target)
    {
        // Nothing to do here
        // Binding of context is done by GL subsystem, as contexts are also used for RenderWindows
    }

    void GLPBRTTManager::unbind(RenderTarget *target)
    { 
        // Copy on unbind
        GLSurfaceDesc surface;
        surface.buffer = 0;
        target->getCustomAttribute(GLRenderTexture::CustomAttributeString_TARGET, &surface);
        if(surface.buffer)
            static_cast<GLTextureBuffer*>(surface.buffer)->copyFromFramebuffer(surface.zoffset);
    }
    
    void GLPBRTTManager::requestPBuffer(PixelComponentType ctype, uint32 width, uint32 height)
    {
        //Check size
        if(mPBuffers[ctype].pb)
        {
            if(mPBuffers[ctype].pb->getWidth()<width || mPBuffers[ctype].pb->getHeight()<height)
            {
                // If the current PBuffer is too small, destroy it and create a new one
                delete mPBuffers[ctype].pb;
                mPBuffers[ctype].pb = 0;
            }
        }
        if(!mPBuffers[ctype].pb)
        {
            // Create pbuffer via rendersystem
            mPBuffers[ctype].pb = mSupport->createPBuffer(ctype, width, height);
        }
        
        ++mPBuffers[ctype].refcount;
    }
    
    void GLPBRTTManager::releasePBuffer(PixelComponentType ctype)
    {
        --mPBuffers[ctype].refcount;
        if(mPBuffers[ctype].refcount == 0)
        {
            delete mPBuffers[ctype].pb;
            mPBuffers[ctype].pb = 0;
        }
    }
    
    GLContext *GLPBRTTManager::getContextFor(PixelComponentType ctype, uint32 width, uint32 height)
    {
        // Faster to return main context if the RTT is smaller than the window size
        // and ctype is PCT_BYTE. This must be checked every time because the window might have been resized
        if(ctype == PCT_BYTE)
        {
            if(width <= mMainWindow->getWidth() && height <= mMainWindow->getHeight())
                return mMainContext;
        }
        assert(mPBuffers[ctype].pb);
        return mPBuffers[ctype].pb->getContext();
    }
//---------------------------------------------------------------------------------------------

}

