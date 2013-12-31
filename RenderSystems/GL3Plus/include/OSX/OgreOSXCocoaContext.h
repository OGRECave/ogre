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
#ifndef __OgreOSXCocoaContext_H__
#define __OgreOSXCocoaContext_H__

#include "OgreGL3PlusContext.h"
#import <AppKit/NSOpenGL.h>

namespace Ogre {

    class _OgreGL3PlusExport CocoaContext : public GL3PlusContext, public GeneralAllocatedObject
    {
    public:
        CocoaContext(NSOpenGLContext *context, NSOpenGLPixelFormat *pixelFormat);

        virtual ~CocoaContext();

        /** See GL3PlusContext */
        virtual void setCurrent();
		/**
         * This is called before another context is made current. By default,
         * nothing is done here.
         */
        virtual void endCurrent();
		/** Create a new context based on the same window/pbuffer as this
			context - mostly useful for additional threads.
		@note The caller is responsible for deleting the returned context.
		*/
		virtual GL3PlusContext* clone() const;

		/** Grab the NSOpenGLContext if it exists */
		NSOpenGLContext* getContext();
		
		/** Grab the NSOpenGLPixelFormat if it exists */
		NSOpenGLPixelFormat* getPixelFormat();
		
        /* The pixel dimensions of the backbuffer */
        GLint mBackingWidth;
        GLint mBackingHeight;

	private:
		NSOpenGLContext* mNSGLContext;
		NSOpenGLPixelFormat *mNSGLPixelFormat;
    };
}

#endif
