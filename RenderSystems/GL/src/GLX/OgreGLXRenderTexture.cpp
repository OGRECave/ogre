/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#include "OgreStableHeaders.h"

#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"
#include "OgreStringConverter.h"

#include "OgreGLRenderSystem.h"

#include "OgreGLXRenderTexture.h"
#include "OgreGLXContext.h"
#include "OgreGLXUtils.h"
#include "OgreGLXGLSupport.h"
#include <iostream>
#include <climits>

namespace Ogre
{
	//-------------------------------------------------------------------------------------------------//
	GLXPBuffer::GLXPBuffer(GLXGLSupport* glsupport, PixelComponentType format, size_t width, size_t height):
		mGLSupport(glsupport), GLPBuffer(format, width, height), mContext(0)
	{
		Display *glDisplay = mGLSupport->getGLDisplay();
		::GLXDrawable glxDrawable = 0;
		::GLXFBConfig fbConfig = 0;
		
		bool isFloat = false;
		int bits = 0;
		
		switch (mFormat)
		{
		case PCT_BYTE:
			bits = 8; 
			break;
			
		case PCT_SHORT:
			bits = 16; 
			break;
			
		case PCT_FLOAT16:
			bits = 16; 
			break;
			
		case PCT_FLOAT32:
			bits = 32; 
			break;
			
		default: 
			break;
		}
		
		int renderAttrib = GLX_RENDER_TYPE;
		int renderValue  = GLX_RGBA_BIT;
		
		if (mFormat == PCT_FLOAT16 || mFormat == PCT_FLOAT32)
		{
			if (GLXEW_NV_float_buffer)
			{
				renderAttrib = GLX_FLOAT_COMPONENTS_NV;
				renderValue  = GL_TRUE;
			}
			
			if (GLXEW_ATI_pixel_format_float)
			{
				renderAttrib = GLX_RENDER_TYPE;
				renderValue  = GLX_RGBA_FLOAT_ATI_BIT;
			}
			
			if (GLXEW_ARB_fbconfig_float)
			{
				renderAttrib = GLX_RENDER_TYPE;
				renderValue  = GLX_RGBA_FLOAT_BIT;
			}
			
			if (renderAttrib == GLX_RENDER_TYPE && renderValue == GLX_RGBA_BIT)
			{
				OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "No support for Floating point PBuffers",  "GLRenderTexture::createPBuffer");
			}
		}
		
		int minAttribs[] = {
			GLX_DRAWABLE_TYPE, GLX_PBUFFER,
			renderAttrib,	  renderValue,
			GLX_DOUBLEBUFFER,  0,
			None
		};
		
		int maxAttribs[] = {
			GLX_RED_SIZE,	  bits,
			GLX_GREEN_SIZE,	bits,
			GLX_BLUE_SIZE,	 bits,
			GLX_ALPHA_SIZE,	bits,
			GLX_STENCIL_SIZE,  INT_MAX,
			None
		};
		
		int pBufferAttribs[] = {
			GLX_PBUFFER_WIDTH,	  mWidth,
			GLX_PBUFFER_HEIGHT,	 mHeight,
			GLX_PRESERVED_CONTENTS, GL_TRUE,
			None
		};
		
		fbConfig = mGLSupport->selectFBConfig(minAttribs, maxAttribs);
		
		glxDrawable = glXCreatePbuffer(glDisplay, fbConfig, pBufferAttribs);
		
		if (! fbConfig || ! glxDrawable) 
		{
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to create Pbuffer", "GLXPBuffer::GLXPBuffer");
		}
		
		GLint fbConfigID;
		GLuint iWidth, iHeight;
		
		glXGetFBConfigAttrib(glDisplay, fbConfig, GLX_FBCONFIG_ID, &fbConfigID);
		glXQueryDrawable(glDisplay, glxDrawable, GLX_WIDTH, &iWidth);
		glXQueryDrawable(glDisplay, glxDrawable, GLX_HEIGHT, &iHeight);
		
		mWidth = iWidth;  
		mHeight = iHeight;
		LogManager::getSingleton().logMessage(LML_NORMAL, "GLXPBuffer::create used final dimensions " + StringConverter::toString(mWidth) + " x " + StringConverter::toString(mHeight));
		LogManager::getSingleton().logMessage("GLXPBuffer::create used FBConfigID " + StringConverter::toString(fbConfigID));
		
		mContext = new GLXContext(mGLSupport, fbConfig, glxDrawable);
	}
	
	//-------------------------------------------------------------------------------------------------//
	GLXPBuffer::~GLXPBuffer()
	{
		glXDestroyPbuffer(mGLSupport->getGLDisplay(), mContext->mDrawable);
		
		delete mContext;
		
		LogManager::getSingleton().logMessage(LML_NORMAL, "GLXPBuffer::PBuffer destroyed");
	}
	
	//-------------------------------------------------------------------------------------------------//
	GLContext *GLXPBuffer::getContext()
	{
		return mContext;
	}
}
