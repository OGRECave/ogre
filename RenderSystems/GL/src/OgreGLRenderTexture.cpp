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
#include "OgreGLRenderTexture.h"
#include "OgreGLPixelFormat.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreRoot.h"
#include "OgreGLHardwarePixelBuffer.h"

namespace Ogre {

    const String GLRenderTexture::CustomAttributeString_FBO = "FBO";
    const String GLRenderTexture::CustomAttributeString_TARGET = "TARGET";
    const String GLRenderTexture::CustomAttributeString_GLCONTEXT = "GLCONTEXT";

//-----------------------------------------------------------------------------

template<> GLRTTManager* Singleton<GLRTTManager>::msSingleton = 0;
    GLRTTManager::~GLRTTManager()
    {
    }
    MultiRenderTarget* GLRTTManager::createMultiRenderTarget(const String & name)
    {
    	OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "MultiRenderTarget can only be used with GL_EXT_framebuffer_object extension", "GLRTTManager::createMultiRenderTarget");
    }
    PixelFormat GLRTTManager::getSupportedAlternative(PixelFormat format)
    {
        if(checkFormat(format))
            return format;
        /// Find first alternative
        PixelComponentType pct = PixelUtil::getComponentType(format);
        switch(pct)
        {
        case PCT_BYTE: format = PF_A8R8G8B8; break;
        case PCT_SHORT: format = PF_SHORT_RGBA; break;
        case PCT_FLOAT16: format = PF_FLOAT16_RGBA; break;
        case PCT_FLOAT32: format = PF_FLOAT32_RGBA; break;
        case PCT_COUNT: break;
        case PCT_SINT: break;
        case PCT_UINT: break;
        }
        if(checkFormat(format))
            return format;
        /// If none at all, return to default
        return PF_A8R8G8B8;
    }
//-----------------------------------------------------------------------------  
    GLRenderTexture::GLRenderTexture(const String &name, const GLSurfaceDesc &target, bool writeGamma, uint fsaa):
        RenderTexture(target.buffer, target.zoffset)
    {
        mName = name;
		mHwGamma = writeGamma;
		mFSAA = fsaa;
    }
    GLRenderTexture::~GLRenderTexture()
    {
    }
//-----------------------------------------------------------------------------  
    GLCopyingRenderTexture::GLCopyingRenderTexture(GLCopyingRTTManager *manager, 
		const String &name, const GLSurfaceDesc &target, bool writeGamma, uint fsaa):
        GLRenderTexture(name, target, writeGamma, fsaa)
    {
    }
    void GLCopyingRenderTexture::getCustomAttribute(const String& name, void* pData)
    {
        if( name == GLRenderTexture::CustomAttributeString_TARGET )
        {
			GLSurfaceDesc &target = *static_cast<GLSurfaceDesc*>(pData);
			target.buffer = static_cast<GLHardwarePixelBuffer*>(mBuffer);
			target.zoffset = mZOffset;
        }
    }
//-----------------------------------------------------------------------------  
    GLCopyingRTTManager::GLCopyingRTTManager()
    {
    }  
    GLCopyingRTTManager::~GLCopyingRTTManager()
    {
    }

    RenderTexture *GLCopyingRTTManager::createRenderTexture(const String &name, const GLSurfaceDesc &target, 
		bool writeGamma, uint fsaa)
    {
        return new GLCopyingRenderTexture(this, name, target, writeGamma, fsaa);
    }
    
    bool GLCopyingRTTManager::checkFormat(PixelFormat format) 
    { 
        return true; 
    }

    void GLCopyingRTTManager::bind(RenderTarget *target)
    {
        // Nothing to do here
    }

    void GLCopyingRTTManager::unbind(RenderTarget *target)
    {
        // Copy on unbind
        GLSurfaceDesc surface;
		surface.buffer = 0;
        target->getCustomAttribute(GLRenderTexture::CustomAttributeString_TARGET, &surface);
        if(surface.buffer)
            static_cast<GLTextureBuffer*>(surface.buffer)->copyFromFramebuffer(surface.zoffset);
    }
	//---------------------------------------------------------------------------------------------

}

