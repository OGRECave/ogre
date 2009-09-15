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

#include "OgreOSXCGLContext.h"
#include "OgreLogManager.h"

namespace Ogre
{
	
	OSXCGLContext::OSXCGLContext(CGLContextObj cglContext, CGLPixelFormatObj pixelFormat): mCGLContext(cglContext), mPixelFormat(pixelFormat)
	{
	}
    
	OSXCGLContext::~OSXCGLContext()
	{
        CGLClearDrawable(mCGLContext); 
		CGLDestroyContext(mCGLContext);
        
        if(mPixelFormat != NULL)
            CGLDestroyPixelFormat(mPixelFormat);
    }

    void OSXCGLContext::setCurrent()
	{
		CGLSetCurrentContext(mCGLContext);
    }
		
	void OSXCGLContext::endCurrent()
	{
		CGLSetCurrentContext(NULL);
	}
	
	GLContext* OSXCGLContext::clone() const
	{
		CGLContextObj cglCtxShare;
        CGLCreateContext(mPixelFormat, mCGLContext, &cglCtxShare);
		return new OSXCGLContext(cglCtxShare, mPixelFormat);
	}
	
	String OSXCGLContext::getContextType()
	{
		return "CGL";
	}
		
	CGLContextObj OSXCGLContext::getContext()
	{
		return mCGLContext;
	}
}
