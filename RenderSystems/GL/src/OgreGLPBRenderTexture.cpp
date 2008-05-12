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
#include "OgreGLPBRenderTexture.h"
#include "OgreGLContext.h"
#include "OgreGLPixelFormat.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreRoot.h"
#include "OgreGLHardwarePixelBuffer.h"

namespace Ogre {
//-----------------------------------------------------------------------------  
    GLPBuffer::GLPBuffer(PixelComponentType format, size_t width, size_t height):
        mFormat(format),
        mWidth(width),
        mHeight(height)
    {
    }
    GLPBuffer::~GLPBuffer()
    {
    }

//-----------------------------------------------------------------------------  
    GLPBRenderTexture::GLPBRenderTexture(GLPBRTTManager *manager, const String &name, 
		const GLSurfaceDesc &target, bool writeGamma, uint fsaa):
        GLRenderTexture(name, target, writeGamma, fsaa),
        mManager(manager)
    {
        mPBFormat = PixelUtil::getComponentType(target.buffer->getFormat());
        
        mManager->requestPBuffer(mPBFormat, mWidth, mHeight);
    }
    GLPBRenderTexture::~GLPBRenderTexture()
    {
        // Release PBuffer
        mManager->releasePBuffer(mPBFormat);
    }
    void GLPBRenderTexture::getCustomAttribute(const String& name, void* pData)
    {
        if(name=="TARGET")
        {
			GLSurfaceDesc &target = *static_cast<GLSurfaceDesc*>(pData);
			target.buffer = static_cast<GLHardwarePixelBuffer*>(mBuffer);
			target.zoffset = mZOffset;
        }
        else if(name=="GLCONTEXT")
        {
            // Get PBuffer for our internal format
            *static_cast<GLContext**>(pData) = mManager->getContextFor(mPBFormat, mWidth, mHeight);
        }
    }
//-----------------------------------------------------------------------------  
    GLPBRTTManager::GLPBRTTManager(GLSupport *support, RenderTarget *mainwindow):
        mSupport(support),
		mMainWindow(mainwindow),
		mMainContext(0)
    {
		mMainWindow->getCustomAttribute("GLCONTEXT", &mMainContext);
    }  
    GLPBRTTManager::~GLPBRTTManager()
    {
        // Delete remaining PBuffers
        for(size_t x=0; x<PCT_COUNT; ++x)
        {
            delete mPBuffers[x].pb;
        }
    }

    RenderTexture *GLPBRTTManager::createRenderTexture(const String &name, 
		const GLSurfaceDesc &target, bool writeGamma, uint fsaa)
    {
        return new GLPBRenderTexture(this, name, target, writeGamma, fsaa);
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
        target->getCustomAttribute("TARGET", &surface);
        if(surface.buffer)
            static_cast<GLTextureBuffer*>(surface.buffer)->copyFromFramebuffer(surface.zoffset);
    }
    
    void GLPBRTTManager::requestPBuffer(PixelComponentType ctype, size_t width, size_t height)
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
    
    GLContext *GLPBRTTManager::getContextFor(PixelComponentType ctype, size_t width, size_t height)
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

