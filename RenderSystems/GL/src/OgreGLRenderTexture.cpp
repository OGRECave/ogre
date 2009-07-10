/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#include "OgreGLRenderTexture.h"
#include "OgreGLPixelFormat.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreRoot.h"
#include "OgreGLHardwarePixelBuffer.h"

namespace Ogre {


//-----------------------------------------------------------------------------

template<> GLRTTManager* Singleton<GLRTTManager>::ms_Singleton = 0;
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
        if(name=="TARGET")
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
        target->getCustomAttribute("TARGET", &surface);
        if(surface.buffer)
            static_cast<GLTextureBuffer*>(surface.buffer)->copyFromFramebuffer(surface.zoffset);
    }
	//---------------------------------------------------------------------------------------------

}

