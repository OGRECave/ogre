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

#ifndef __GLES2PixelFormat_H__
#define __GLES2PixelFormat_H__

#include "OgreGLES2Prerequisites.h"
#include "OgrePixelFormat.h"

namespace Ogre {
    /**
    * Class to do pixel format mapping between GL and OGRE
    */
    class _OgreGLES2Export GLES2PixelUtil
    {
        public:
            /// use sized pixel formats. (to be used with GLES3)
            static void useSizedFormats();

            /** Takes the OGRE pixel format and returns the appropriate GL one
                @returns a GLenum describing the format, or 0 if there is no exactly matching 
                one (and conversion is needed)
            */
            static GLenum getGLOriginFormat(PixelFormat mFormat);

            /** Takes the OGRE pixel format and returns type that must be provided
                to GL as data type for reading it into the GPU
                @returns a GLenum describing the data type, or 0 if there is no exactly matching 
                one (and conversion is needed)
            */
            static GLenum getGLOriginDataType(PixelFormat mFormat);

            /**    Takes the OGRE pixel format and returns the type that must be provided
                to GL as internal format. GL_NONE if no match exists.
            @param mFormat The pixel format
            @param hwGamma Whether a hardware gamma-corrected version is requested
            */
            static GLenum getGLInternalFormat(PixelFormat mFormat, bool hwGamma = false);

            /**    Function to get the closest matching OGRE format to an internal GL format. To be
                precise, the format will be chosen that is most efficient to transfer to the card 
                without losing precision.
                @remarks It is valid for this function to always return PF_A8R8G8B8.
            */
            static PixelFormat getClosestOGREFormat(GLenum fmt);
    };
}

#endif
