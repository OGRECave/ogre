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
#include "OgreStableHeaders.h"

#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"
#include "OgreStringConverter.h"

#include "OgreGLRenderSystem.h"

#include "OgreOSXRenderTexture.h"
#include "OgreOSXCarbonContext.h"

#include <OpenGL/gl.h>
#define GL_EXT_texture_env_combine 1
#include <OpenGL/glext.h>
#include <OpenGL/glu.h>
#include <AGL/agl.h>

namespace Ogre
{
	OSXPBuffer::OSXPBuffer( PixelComponentType format, uint32 width, uint32 height ) : GLPBuffer( format, width, height ), mContext( NULL )
	{
		LogManager::getSingleton().logMessage( "OSXPBuffer::OSXPBuffer()" );
		createPBuffer();
		// Create context
		//mContext = OGRE_NEW OSXCarbonContext(mAGLContext);
    }
	
	OSXPBuffer::~OSXPBuffer()
	{
		LogManager::getSingleton().logMessage( "OSXPBuffer::~OSXPBuffer()" );
		OGRE_DELETE mContext;
		destroyPBuffer();
	}

	GLContext* OSXPBuffer::getContext()
	{
		LogManager::getSingleton().logMessage( "OSXPBuffer::getContext()" );
		return mContext;
	}
	
	void OSXPBuffer::createPBuffer()
	{
		LogManager::getSingleton().logMessage( "OSXPBuffer::createPBuffer()" );
		
		GLint attrib[] = { AGL_NO_RECOVERY, GL_TRUE, AGL_ACCELERATED, GL_TRUE, AGL_RGBA, AGL_NONE };
		AGLPixelFormat pixelFormat = aglChoosePixelFormat(NULL, 0, attrib);
		mAGLContext = aglCreateContext(pixelFormat, NULL);
		
		//mAGLContext = aglGetCurrentContext();
		aglCreatePBuffer( mWidth, mHeight, GL_TEXTURE_2D, GL_RGBA, 0, &mPBuffer );
		
		GLint vs = aglGetVirtualScreen( mAGLContext );
		aglSetPBuffer( mAGLContext, mPBuffer, 0, 0, vs );
        mContext = OGRE_NEW OSXCarbonContext(mAGLContext, pixelFormat);
	}
	
	void OSXPBuffer::destroyPBuffer()
	{
		LogManager::getSingleton().logMessage( "OSXPBuffer::destroyPBuffer()" );
		aglDestroyPBuffer( mPBuffer );
	}
}

