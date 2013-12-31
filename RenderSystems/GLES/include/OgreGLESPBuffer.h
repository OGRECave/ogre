/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
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

#ifndef __GLESPBuffer_H__
#define __GLESPBuffer_H__

#include "OgreGLESPrerequisites.h"

namespace Ogre {

    /** An off-screen rendering context. These contexts are always RGBA for simplicity, speed and
        convience, but the component format is configurable.
    */
    class GLESContext;
    class _OgreGLESExport GLESPBuffer
    {
        public:
            GLESPBuffer(PixelComponentType format, size_t width, size_t height);
            virtual ~GLESPBuffer();

            PixelComponentType getFormat() { return mFormat; }
            size_t getWidth() { return mWidth; }
            size_t getHeight() { return mHeight; }

            /** Get PBuffer component format for an OGRE pixel format.
             */
            static PixelComponentType getPixelComponentType(PixelFormat fmt);
        protected:
            PixelComponentType mFormat;
            size_t mWidth, mHeight;
    };
}

#endif
