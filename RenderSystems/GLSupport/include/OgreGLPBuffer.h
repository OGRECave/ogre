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

#ifndef __GLPBUFFER_H__
#define __GLPBUFFER_H__

#include "OgreGLSupportPrerequisites.h"

namespace Ogre {
    class GLContext;

    /** An off-screen rendering context. These contexts are always RGBA for simplicity, speed and
        convience, but the component format is configurable.
    */
    class _OgreGLExport GLPBuffer
    {
    public:
        GLPBuffer(PixelComponentType format, uint32 width, uint32 height)
            : mFormat(format), mWidth(width), mHeight(height) {}
        virtual ~GLPBuffer() {}
        
        /** Get the GL context that needs to be active to render to this PBuffer.
        */
        virtual GLContext *getContext() = 0;
        
        PixelComponentType getFormat() { return mFormat; }
        uint32 getWidth() { return mWidth; }
        uint32 getHeight() { return mHeight; }
        
    protected:
        PixelComponentType mFormat;
        uint32 mWidth, mHeight;
    };
    
}

#endif // __GLPBRENDERTEXTURE_H__
