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

#include "OgreGLPixelFormat.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreBitwise.h"

#ifndef GL_HALF_FLOAT
#define GL_HALF_FLOAT                     0x140B
#endif

namespace Ogre  {

    struct GLPixelFormatDescription {
        GLenum format;
        GLenum type;
        GLenum internalFormat;
    };

    static GLPixelFormatDescription _pixelFormats[int(PF_COUNT)] = {
            {GL_NONE},                                           // PF_UNKNOWN
            {GL_LUMINANCE, GL_UNSIGNED_BYTE, GL_LUMINANCE8},     // PF_L8
            {GL_LUMINANCE, GL_UNSIGNED_SHORT, GL_LUMINANCE16},   // PF_L16
            {GL_ALPHA, GL_UNSIGNED_BYTE, GL_ALPHA8},             // PF_A8
            {GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, GL_LUMINANCE8_ALPHA8},// PF_BYTE_LA
            {GL_RGB, GL_UNSIGNED_SHORT_5_6_5, GL_RGB5},          // PF_R5G6B5
            {GL_BGR, GL_UNSIGNED_SHORT_5_6_5, GL_RGB5},          // PF_B5G6R5
            {GL_BGRA, GL_UNSIGNED_SHORT_4_4_4_4_REV, GL_RGBA4},  // PF_A4R4G4B4
            {GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, GL_RGB5_A1},// PF_A1R5G5B5
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
            {GL_RGB, GL_UNSIGNED_BYTE, GL_RGB8},                 // PF_R8G8B8
            {GL_BGR, GL_UNSIGNED_BYTE, GL_RGB8},                 // PF_B8G8R8
#else
            {GL_BGR, GL_UNSIGNED_BYTE, GL_RGB8},                 // PF_R8G8B8
            {GL_RGB, GL_UNSIGNED_BYTE, GL_RGB8},                 // PF_B8G8R8
#endif
            {GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, GL_RGBA8},    // PF_A8R8G8B8
            {GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, GL_RGBA8},    // PF_A8B8G8R8
            {GL_BGRA, GL_UNSIGNED_INT_8_8_8_8, GL_RGBA8},        // PF_B8G8R8A8
            {GL_NONE},                                           // PF_A2R10G10B10
            {GL_NONE},                                           // PF_A2B10G10R10
            {GL_NONE, GL_NONE, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT},// PF_DXT1
            {GL_NONE},                                           // PF_DXT2
            {GL_NONE, GL_NONE, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT},// PF_DXT3
            {GL_NONE},                                           // PF_DXT4
            {GL_NONE, GL_NONE, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT},// PF_DXT5
            {GL_RGB, GL_HALF_FLOAT, GL_RGB16F_ARB},              // PF_FLOAT16_RGB
            {GL_RGBA, GL_HALF_FLOAT, GL_RGBA16F_ARB},            // PF_FLOAT16_RGBA
            {GL_RGB, GL_FLOAT, GL_RGB32F_ARB},                   // PF_FLOAT32_RGB
            {GL_RGBA, GL_FLOAT, GL_RGBA32F_ARB},                 // PF_FLOAT32_RGBA
            {GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, GL_RGBA8},    // PF_X8R8G8B8
            {GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, GL_RGBA8},    // PF_X8B8G8R8
            {GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, GL_RGBA8},        // PF_R8G8B8A8
            {GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, GL_DEPTH_COMPONENT16}, // PF_DEPTH16
            {GL_RGBA, GL_UNSIGNED_SHORT, GL_RGBA16},             // PF_SHORT_RGBA
            {GL_RGB, GL_UNSIGNED_BYTE_3_3_2, GL_R3_G3_B2},       // PF_R3G3B2
            {GL_LUMINANCE, GL_HALF_FLOAT, GL_LUMINANCE16F_ARB},  // PF_FLOAT16_R
            {GL_LUMINANCE, GL_FLOAT, GL_LUMINANCE32F_ARB},       // PF_FLOAT32_R
            {GL_LUMINANCE_ALPHA, GL_UNSIGNED_SHORT, GL_LUMINANCE16_ALPHA16},// PF_SHORT_GR
            {GL_LUMINANCE_ALPHA, GL_HALF_FLOAT, GL_LUMINANCE_ALPHA16F_ARB}, // PF_FLOAT16_GR
            {GL_LUMINANCE_ALPHA, GL_FLOAT, GL_LUMINANCE_ALPHA32F_ARB},      // PF_FLOAT32_GR
            {GL_RGB, GL_UNSIGNED_SHORT, GL_RGB16},               // PF_SHORT_RGB
            // limited format support: the rest is PF_NONE
    };

    //-----------------------------------------------------------------------------
    GLenum GLPixelUtil::getGLOriginFormat(PixelFormat pf)
    {
        return _pixelFormats[pf].format;
    }
    //----------------------------------------------------------------------------- 
    GLenum GLPixelUtil::getGLOriginDataType(PixelFormat pf)
    {
        return _pixelFormats[pf].type;
    }
    
    GLenum GLPixelUtil::getGLInternalFormat(PixelFormat pf, bool hwGamma)
    {
        GLenum ret = _pixelFormats[pf].internalFormat;

        if(!hwGamma)
            return ret;

        switch(ret)
        {
        case GL_RGB8:
            return GL_SRGB8;
        case GL_RGBA8:
            return GL_SRGB8_ALPHA8;
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
            return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
            return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
            return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
        default:
            return ret;
        }
    }
    
    //-----------------------------------------------------------------------------     
    PixelFormat GLPixelUtil::getClosestOGREFormat(GLenum format)
    {
        switch(format)
        {
        case GL_DEPTH_COMPONENT24:
        case GL_DEPTH_COMPONENT32:
        case GL_DEPTH_COMPONENT32F:
        case GL_DEPTH_COMPONENT:
            return PF_DEPTH16;
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
            return PF_DXT1;
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
            return PF_DXT3;
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
            return PF_DXT5;
        case GL_SRGB8:
        case GL_RGB8:  // prefer native endian byte format
            return PF_BYTE_RGB;
        case GL_SRGB8_ALPHA8:
        case GL_RGBA8:  // prefer native endian byte format
            return PF_BYTE_RGBA;
        };

        for(int pf = 0; pf < PF_COUNT; pf++) {
            if(_pixelFormats[pf].internalFormat == format)
                return (PixelFormat)pf;
        }

        return PF_BYTE_RGBA;
    }
    //-----------------------------------------------------------------------------    
    uint32 GLPixelUtil::optionalPO2(uint32 value)
    {
        const RenderSystemCapabilities *caps = Root::getSingleton().getRenderSystem()->getCapabilities();
        if(caps->hasCapability(RSC_NON_POWER_OF_2_TEXTURES))
            return value;
        else
            return Bitwise::firstPO2From(value);
    }   

    
}
