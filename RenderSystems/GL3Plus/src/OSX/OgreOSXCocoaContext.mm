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

#import "OgreOSXCocoaContext.h"

namespace Ogre
{

    CocoaContext::CocoaContext(NSOpenGLContext *context, NSOpenGLPixelFormat *pixelFormat)
      : mBackingWidth(0), mBackingHeight(0), mNSGLContext(context), mNSGLPixelFormat(pixelFormat)
	{
        if(mNSGLPixelFormat)
            [mNSGLPixelFormat retain];
	}

	CocoaContext::~CocoaContext()
	{
        if(mNSGLPixelFormat)
            [mNSGLPixelFormat release];
    }

    void CocoaContext::setCurrent()
	{
		[mNSGLContext makeCurrentContext];
    }

	void CocoaContext::endCurrent()
	{
        [NSOpenGLContext clearCurrentContext]; 
	}

	GL3PlusContext* CocoaContext::clone() const
	{
		NSOpenGLContext *cloneCtx = [[NSOpenGLContext alloc] initWithFormat:mNSGLPixelFormat shareContext:mNSGLContext];
		[cloneCtx copyAttributesFromContext:mNSGLContext withMask:0];
		return OGRE_NEW CocoaContext(cloneCtx, mNSGLPixelFormat);
	}

	NSOpenGLContext* CocoaContext::getContext()
	{
		return mNSGLContext;
    }

	NSOpenGLPixelFormat* CocoaContext::getPixelFormat()
	{
		return mNSGLPixelFormat;
	}
}
