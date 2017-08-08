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

#include "OgreGL3PlusPixelFormat.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreBitwise.h"

#define GL_R8_SNORM                       0x8F94
#define GL_RG8_SNORM                      0x8F95
#define GL_RGB8_SNORM                     0x8F96
#define GL_RGBA8_SNORM                    0x8F97
#define GL_R16_SNORM                      0x8F98
#define GL_RG16_SNORM                     0x8F99
#define GL_RGB16_SNORM                    0x8F9A
#define GL_RGBA16_SNORM                   0x8F9B

namespace Ogre  {

    GLenum GL3PlusPixelUtil::getGLOriginFormat(PixelFormat format)
    {
        switch(format)
        {
        case PF_DEPTH_DEPRECATED:
        case PF_D24_UNORM:
        case PF_D16_UNORM:
        case PF_D32_FLOAT:
            return GL_DEPTH_COMPONENT;
        case PF_D24_UNORM_S8_UINT:
        case PF_D24_UNORM_X8:
        case PF_X24_S8_UINT:
        case PF_D32_FLOAT_X24_S8_UINT:
        case PF_D32_FLOAT_X24_X8:
        case PF_X32_X24_S8_UINT:
            return GL_DEPTH_STENCIL;
        case PF_A8:
        case PF_L8:
        case PF_L16:
        case PF_R8_SNORM:
        case PF_R16_SNORM:
            return GL_RED;
        case PF_R8_UINT:
        case PF_R8_SINT:
        case PF_R16_SINT:
        case PF_R32_SINT:
        case PF_R16_UINT:
        case PF_R32_UINT:
            return GL_RED_INTEGER;
        case PF_BYTE_LA:
        case PF_RG8:
        case PF_FLOAT16_GR:
        case PF_FLOAT32_GR:
        case PF_SHORT_GR:
        case PF_R8G8_SNORM:
        case PF_R16G16_SNORM:
            return GL_RG;
        case PF_R8G8_UINT:
        case PF_R16G16_UINT:
        case PF_R32G32_UINT:
        case PF_R8G8_SINT:
        case PF_R16G16_SINT:
        case PF_R32G32_SINT:
            return GL_RG_INTEGER;
        case PF_R3G3B2:
        case PF_R5G6B5:
        case PF_FLOAT16_RGB:
        case PF_FLOAT32_RGB:
        case PF_SHORT_RGB:
        case PF_R11G11B10_FLOAT:
        case PF_R9G9B9E5_SHAREDEXP:
        case PF_R8G8B8_SNORM:
        case PF_R16G16B16_SNORM:
            return GL_RGB;
        case PF_R8G8B8_UINT:
        case PF_R16G16B16_UINT:
        case PF_R32G32B32_UINT:
        case PF_R8G8B8_SINT:
        case PF_R16G16B16_SINT:
        case PF_R32G32B32_SINT:
            return GL_RGB_INTEGER;
        case PF_A1R5G5B5:
            return GL_BGRA;
        case PF_B5G6R5:
            return GL_RGB; //GL_BGR - not supported for GL_UNSIGNED_SHORT_5_6_5 type in glTexImage2D()   
        case PF_A4R4G4B4:
            return GL_BGRA;
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
            // Formats are in native endian, so R8G8B8 on little endian is
            // BGR, on big endian it is RGB.
        case PF_R8G8B8:
            return GL_RGB;
        case PF_B8G8R8:
            return GL_BGR;
#else
        case PF_R8G8B8:
            return GL_BGR;
        case PF_B8G8R8:
            return GL_RGB;
#endif
        case PF_X8R8G8B8:
        case PF_A8R8G8B8:
            return GL_BGRA;
        case PF_X8B8G8R8:
        case PF_A8B8G8R8:
            return GL_RGBA;
        case PF_B8G8R8A8:
            return GL_BGRA;
        case PF_R8G8B8A8:
        case PF_A2R10G10B10:
        case PF_FLOAT16_RGBA:
        case PF_FLOAT32_RGBA:
        case PF_SHORT_RGBA:
        case PF_R8G8B8A8_SNORM:
        case PF_R16G16B16A16_SNORM:
            return GL_RGBA;
        case PF_R8G8B8A8_SINT:
        case PF_R16G16B16A16_SINT:
        case PF_R32G32B32A32_SINT:
        case PF_R8G8B8A8_UINT:
        case PF_R16G16B16A16_UINT:
        case PF_R32G32B32A32_UINT:
            return GL_RGBA_INTEGER;
        case PF_A2B10G10R10:
            return GL_BGRA;
        case PF_FLOAT16_R:
        case PF_FLOAT32_R:
            return GL_RED;
        case PF_DXT1:
            return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        case PF_DXT3:
            return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        case PF_DXT5:
            return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        case PF_BC4_UNORM:
            return GL_COMPRESSED_RED_RGTC1;
        case PF_BC4_SNORM:
            return GL_COMPRESSED_SIGNED_RED_RGTC1;
        case PF_BC5_UNORM:
            return GL_COMPRESSED_RG_RGTC2;
        case PF_BC5_SNORM:
            return GL_COMPRESSED_SIGNED_RG_RGTC2;
        case PF_BC6H_SF16:
            return GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB;
        case PF_BC6H_UF16:
            return GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB;
        case PF_BC7_UNORM:
            return GL_COMPRESSED_RGBA_BPTC_UNORM_ARB;
        case PF_BC7_UNORM_SRGB:
            return GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB;
        case PF_ETC2_RGB8:
            return GL_COMPRESSED_RGB8_ETC2;
        case PF_ETC2_RGBA8:
            return GL_COMPRESSED_RGBA8_ETC2_EAC;
        case PF_ETC2_RGB8A1:
            return GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;

        default:
            return 0;
        }
    }

    GLenum GL3PlusPixelUtil::getGLOriginDataType(PixelFormat format)
    {
        switch(format)
        {
        case PF_BYTE_LA:
        case PF_A8:
        case PF_L8:
        case PF_R8G8B8:
        case PF_B8G8R8:
        case PF_RG8:
            return GL_UNSIGNED_BYTE;
        case PF_R8_SNORM:
        case PF_R8G8_SNORM:
        case PF_R8G8B8_SNORM:
        case PF_R8G8B8A8_SNORM:
        case PF_R16_SNORM:
        case PF_R16G16_SNORM:
        case PF_R16G16B16_SNORM:
        case PF_R16G16B16A16_SNORM:
            return GL_BYTE;
        case PF_R3G3B2:
            return GL_UNSIGNED_BYTE_3_3_2;
        case PF_A1R5G5B5:
            return GL_UNSIGNED_SHORT_1_5_5_5_REV;
        case PF_R5G6B5:
        case PF_B5G6R5:
            return GL_UNSIGNED_SHORT_5_6_5;
        case PF_A4R4G4B4:
            return GL_UNSIGNED_SHORT_4_4_4_4_REV;
        case PF_L16:
            return GL_UNSIGNED_SHORT;
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
        case PF_X8B8G8R8:
        case PF_A8B8G8R8:
            return GL_UNSIGNED_INT_8_8_8_8_REV;
        case PF_X8R8G8B8:
        case PF_A8R8G8B8:
            return GL_UNSIGNED_INT_8_8_8_8_REV;
        case PF_B8G8R8A8:
            return GL_UNSIGNED_BYTE;
        case PF_R8G8B8A8:
            return GL_UNSIGNED_BYTE;
#else
        case PF_X8B8G8R8:
        case PF_A8B8G8R8:
            return GL_UNSIGNED_BYTE;
        case PF_X8R8G8B8:
        case PF_A8R8G8B8:
            return GL_UNSIGNED_BYTE;
        case PF_B8G8R8A8:
            return GL_UNSIGNED_INT_8_8_8_8;
        case PF_R8G8B8A8:
            return GL_UNSIGNED_INT_8_8_8_8;
#endif
        case PF_A2R10G10B10:
            return GL_UNSIGNED_INT_2_10_10_10_REV;
        case PF_A2B10G10R10:
            return GL_UNSIGNED_INT_2_10_10_10_REV;
        case PF_FLOAT16_R:
        case PF_FLOAT16_GR:
        case PF_FLOAT16_RGB:
        case PF_FLOAT16_RGBA:
            return GL_HALF_FLOAT;
        case PF_FLOAT32_R:
        case PF_FLOAT32_GR:
        case PF_FLOAT32_RGB:
        case PF_FLOAT32_RGBA:
        case PF_D32_FLOAT:
            return GL_FLOAT;
        case PF_SHORT_RGBA:
        case PF_SHORT_RGB:
        case PF_SHORT_GR:
            return GL_UNSIGNED_SHORT;
        case PF_R9G9B9E5_SHAREDEXP:
            return GL_UNSIGNED_INT_5_9_9_9_REV;
        case PF_R11G11B10_FLOAT:
            return GL_UNSIGNED_INT_10F_11F_11F_REV;
        case PF_R8_UINT:
        case PF_R8G8_UINT:
        case PF_R8G8B8_UINT:
        case PF_R8G8B8A8_UINT:
            return GL_UNSIGNED_BYTE;
        case PF_R16_UINT:
        case PF_R16G16_UINT:
        case PF_R16G16B16_UINT:
        case PF_R16G16B16A16_UINT:
            return GL_UNSIGNED_SHORT;
        case PF_R32_UINT:
        case PF_R32G32_UINT:
        case PF_R32G32B32_UINT:
        case PF_R32G32B32A32_UINT:
        case PF_DEPTH_DEPRECATED:
        case PF_D24_UNORM:
        case PF_D16_UNORM:
            return GL_UNSIGNED_INT;
        case PF_D24_UNORM_S8_UINT:
        case PF_D24_UNORM_X8:
        case PF_X24_S8_UINT:
            return GL_UNSIGNED_INT_24_8;
        case PF_R8_SINT:
        case PF_R8G8_SINT:
        case PF_R8G8B8_SINT:
        case PF_R8G8B8A8_SINT:
            return GL_BYTE;
        case PF_R16_SINT:
        case PF_R16G16_SINT:
        case PF_R16G16B16_SINT:
        case PF_R16G16B16A16_SINT:
            return GL_SHORT;
        case PF_R32_SINT:
        case PF_R32G32_SINT:
        case PF_R32G32B32_SINT:
        case PF_R32G32B32A32_SINT:
            return GL_INT;
        case PF_D32_FLOAT_X24_S8_UINT:
        case PF_X32_X24_S8_UINT:
        case PF_D32_FLOAT_X24_X8:
            return GL_FLOAT_32_UNSIGNED_INT_24_8_REV;

        default:
            return 0;
        }
    }

    GLenum GL3PlusPixelUtil::getGLInternalFormat(PixelFormat format, bool hwGamma)
    {
        switch(format)
        {
        case PF_DEPTH_DEPRECATED:
            return GL_DEPTH_COMPONENT16;
        case PF_D24_UNORM_S8_UINT:
        case PF_D24_UNORM_X8:
        case PF_X24_S8_UINT:
            return GL_DEPTH24_STENCIL8;
        case PF_D24_UNORM:
            return GL_DEPTH_COMPONENT24;
        case PF_D16_UNORM:
            return GL_DEPTH_COMPONENT16;
        case PF_D32_FLOAT:
            return GL_DEPTH_COMPONENT32F;
        case PF_D32_FLOAT_X24_S8_UINT:
        case PF_D32_FLOAT_X24_X8:
        case PF_X32_X24_S8_UINT:
            return GL_DEPTH32F_STENCIL8;
        case PF_L8:
        case PF_A8:
            return GL_R8;
        case PF_L16:
            return GL_R16;
        case PF_BYTE_LA:
            return GL_RG8;
        case PF_R3G3B2:
            return GL_R3_G3_B2;
        case PF_A1R5G5B5:
            return GL_RGB5_A1;
        case PF_R5G6B5:
        case PF_B5G6R5:
            return GL_RGB5;
        case PF_A4R4G4B4:
            return GL_RGBA4;
        case PF_R8G8B8:
        case PF_B8G8R8:
        case PF_X8B8G8R8:
        case PF_X8R8G8B8:
            if (hwGamma)
                return GL_SRGB8;
            else
                return GL_RGB8;
        case PF_A8R8G8B8:
        case PF_B8G8R8A8:
        case PF_A8B8G8R8:
        case PF_R8G8B8A8:
            if (hwGamma)
                return GL_SRGB8_ALPHA8;
            else
                return GL_RGBA8;
        case PF_A2R10G10B10:
        case PF_A2B10G10R10:
            return GL_RGB10_A2;
        case PF_FLOAT16_R:
            return GL_R16F;
        case PF_FLOAT16_RGB:
            return GL_RGB16F;
        case PF_FLOAT16_GR:
            return GL_RG16F;
        case PF_FLOAT16_RGBA:
            return GL_RGBA16F;
        case PF_FLOAT32_R:
            return GL_R32F;
        case PF_FLOAT32_GR:
            return GL_RG32F;
        case PF_FLOAT32_RGB:
            return GL_RGB32F;
        case PF_FLOAT32_RGBA:
            return GL_RGBA32F;
        case PF_SHORT_RGBA:
            return GL_RGBA16;
        case PF_SHORT_RGB:
            return GL_RGB16;
        case PF_SHORT_GR:
            return GL_RG16;
        case PF_DXT1:
            if (hwGamma)
                return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;
            else
                return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        case PF_DXT3:
            if (hwGamma)
                return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
            else
                return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        case PF_DXT5:
            if (hwGamma)
                return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
            else
                return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        case PF_R11G11B10_FLOAT:
            return GL_R11F_G11F_B10F;
        case PF_R8_UINT:
            return GL_R8UI;
        case PF_R8G8_UINT:
            return GL_RG8UI;
        case PF_R8G8B8_UINT:
            return GL_RGB8UI;
        case PF_R8G8B8A8_UINT:
            return GL_RGBA8UI;
        case PF_R16_UINT:
            return GL_R16UI;
        case PF_R16G16_UINT:
            return GL_RG16UI;
        case PF_R16G16B16_UINT:
            return GL_RGB16UI;
        case PF_R16G16B16A16_UINT:
            return GL_RGBA16UI;
        case PF_R32_UINT:
            return GL_R32UI;
        case PF_R32G32_UINT:
            return GL_RG32UI;
        case PF_R32G32B32_UINT:
            return GL_RGB32UI;
        case PF_R32G32B32A32_UINT:
            return GL_RGBA32UI;
        case PF_R8_SINT:
            return GL_R8I;
        case PF_R8G8_SINT:
            return GL_RG8I;
        case PF_R8G8B8_SINT:
            return GL_RG8I;
        case PF_R8G8B8A8_SINT:
            return GL_RGB8I;
        case PF_R16_SINT:
            return GL_R16I;
        case PF_R16G16_SINT:
            return GL_RG16I;
        case PF_R16G16B16_SINT:
            return GL_RGB16I;
        case PF_R16G16B16A16_SINT:
            return GL_RGBA16I;
        case PF_R32_SINT:
            return GL_R32I;
        case PF_R32G32_SINT:
            return GL_RG32I;
        case PF_R32G32B32_SINT:
            return GL_RGB32I;
        case PF_R32G32B32A32_SINT:
            return GL_RGBA32I;
        case PF_R9G9B9E5_SHAREDEXP:
            return GL_RGB9_E5;
        case PF_R8_SNORM:
            return GL_R8_SNORM;
        case PF_R8G8_SNORM:
            return GL_RG8_SNORM;
        case PF_R8G8B8_SNORM:
            return GL_RGB8_SNORM;
        case PF_R8G8B8A8_SNORM:
            return GL_RGBA8_SNORM;
        case PF_R16_SNORM:
            return GL_R16_SNORM;
        case PF_R16G16_SNORM:
            return GL_RG16_SNORM;
        case PF_R16G16B16_SNORM:
            return GL_RGB16_SNORM;
        case PF_R16G16B16A16_SNORM:
            return GL_RGBA16_SNORM;
        case PF_BC4_UNORM:
            return GL_COMPRESSED_RED_RGTC1;
        case PF_BC4_SNORM:
            return GL_COMPRESSED_SIGNED_RED_RGTC1;
        case PF_BC5_UNORM:
            return GL_COMPRESSED_RG_RGTC2;
        case PF_BC5_SNORM:
            return GL_COMPRESSED_SIGNED_RG_RGTC2;
        case PF_BC6H_SF16:
            return GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB;
        case PF_BC6H_UF16:
            return GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB;
        case PF_BC7_UNORM:
            return GL_COMPRESSED_RGBA_BPTC_UNORM_ARB;
        case PF_BC7_UNORM_SRGB:
            return GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB;
        case PF_RG8:
            return GL_RG8;
        case PF_ETC2_RGB8:
            return GL_COMPRESSED_RGB8_ETC2;
        case PF_ETC2_RGBA8:
            return GL_COMPRESSED_RGBA8_ETC2_EAC;
        case PF_ETC2_RGB8A1:
            return GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;

        default:
            return GL_NONE;
        }
    }

    GLenum GL3PlusPixelUtil::getClosestGLInternalFormat(PixelFormat format, bool hwGamma)
    {
        GLenum GLformat = getGLInternalFormat(format, hwGamma);
        if (GLformat == GL_NONE)
        {
            if (hwGamma)
                return GL_SRGB8;
            else
                return GL_RGBA8;
        }
        else
            return GLformat;
    }


    GLenum GL3PlusPixelUtil::getGLImageInternalFormat(PixelFormat format)
    {
        switch(format)
        {
        case PF_L8:
        case PF_A8:
            return GL_R8;
        case PF_L16:
            return GL_R16;
        case PF_BYTE_LA:
            return GL_RG8;
        case PF_A8R8G8B8:
        case PF_B8G8R8A8:
        case PF_A8B8G8R8:
        case PF_R8G8B8A8:
            return GL_RGBA8;
        case PF_A2R10G10B10:
        case PF_A2B10G10R10:
            //return GL_RGB10_A2UI;
            return GL_RGB10_A2;
        case PF_FLOAT16_R:
            return GL_R16F;
        case PF_FLOAT16_GR:
            return GL_RG16F;
        case PF_FLOAT16_RGBA:
            return GL_RGBA16F;
        case PF_FLOAT32_R:
            return GL_R32F;
        case PF_FLOAT32_GR:
            return GL_RG32F;
        case PF_FLOAT32_RGBA:
            return GL_RGBA32F;
        case PF_SHORT_RGBA:
            return GL_RGBA16;
        case PF_SHORT_GR:
            return GL_RG16;
        case PF_R11G11B10_FLOAT:
            return GL_R11F_G11F_B10F;
        case PF_R8_UINT:
            return GL_R8UI;
        case PF_R8G8_UINT:
            return GL_RG8UI;
        case PF_R8G8B8A8_UINT:
            return GL_RGBA8UI;
        case PF_R16_UINT:
            return GL_R16UI;
        case PF_R16G16_UINT:
            return GL_RG16UI;
        case PF_R16G16B16A16_UINT:
            return GL_RGBA16UI;
        case PF_R32_UINT:
            return GL_R32UI;
        case PF_R32G32_UINT:
            return GL_RG32UI;
        case PF_R32G32B32A32_UINT:
            return GL_RGBA32UI;
        case PF_R8_SINT:
            return GL_R8I;
        case PF_R8G8_SINT:
            return GL_RG8I;
        case PF_R8G8B8_SINT:
            return GL_RG8I;
        case PF_R16_SINT:
            return GL_R16I;
        case PF_R16G16_SINT:
            return GL_RG16I;
        case PF_R16G16B16A16_SINT:
            return GL_RGBA16I;
        case PF_R32_SINT:
            return GL_R32I;
        case PF_R32G32_SINT:
            return GL_RG32I;
        case PF_R32G32B32A32_SINT:
            return GL_RGBA32I;
        case PF_R8G8_SNORM:
            return GL_RG8_SNORM;
        case PF_R16_SNORM:
            return GL_R16_SNORM;
        case PF_R16G16_SNORM:
            return GL_RG16_SNORM;
        case PF_R16G16B16A16_SNORM:
            return GL_RGBA16_SNORM;
        case PF_R8G8B8A8_SNORM:
            return GL_RGBA8_SNORM;

        default:
            return GL_NONE;
        }
    }


    GLenum GL3PlusPixelUtil::getClosestGLImageInternalFormat(PixelFormat format)
    {
        GLenum GLformat = getGLImageInternalFormat(format);
        return (format == GL_NONE ? GL_RGBA8 : GLformat);
    }


    PixelFormat GL3PlusPixelUtil::getClosestOGREFormat(GLenum format)
    {
        switch(format)
        {
        case GL_DEPTH_COMPONENT16:
            return PF_D16_UNORM;
        case GL_DEPTH_COMPONENT24:
            return PF_D24_UNORM;
        case GL_DEPTH_COMPONENT32:
        case GL_DEPTH_COMPONENT32F:
            return PF_D32_FLOAT;
        case GL_DEPTH_COMPONENT:
        case GL_DEPTH24_STENCIL8:
            return PF_D24_UNORM_S8_UINT;
        case GL_DEPTH32F_STENCIL8:
            return PF_D32_FLOAT_X24_S8_UINT;
        case GL_R8:
            return PF_L8;
        case GL_R16:
            return PF_L16;
        case GL_RG: //TODO Is there a better OGRE format?
        case GL_RG8:
            return PF_RG8;
        case GL_R3_G3_B2:
            return PF_R3G3B2;
        case GL_RGB5_A1:
            return PF_A1R5G5B5;
        case GL_RGB5:
            return PF_R5G6B5;
        case GL_RGBA4:
            return PF_A4R4G4B4;
        case GL_RGB8:
        case GL_SRGB8:
            return PF_X8R8G8B8;
        case GL_RGBA8:
        case GL_SRGB8_ALPHA8:
            return PF_A8R8G8B8;
        case GL_RGB10_A2:
        case GL_RGB10_A2UI:
            return PF_A2R10G10B10;
        case GL_RGBA16:
            return PF_SHORT_RGBA;
        case GL_RGB16:
            return PF_SHORT_RGB;
        case GL_RG16:
            return PF_SHORT_GR;
        case GL_R16F:
            return PF_FLOAT16_R;
        case GL_RG16F:
            return PF_FLOAT16_GR;
        case GL_RG32F:
            return PF_FLOAT32_GR;
        case GL_R32F:
            return PF_FLOAT32_R;
        case GL_RGB16F:
            return PF_FLOAT16_RGB;
        case GL_RGBA16F:
            return PF_FLOAT16_RGBA;
        case GL_RGB32F:
            return PF_FLOAT32_RGB;
        case GL_RGBA32F:
            return PF_FLOAT32_RGBA;
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
            return PF_DXT1;
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
            return PF_DXT3;
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
            return PF_DXT5;

        case GL_R11F_G11F_B10F:
            return PF_R11G11B10_FLOAT;
        case GL_R8UI:
            return PF_R8_UINT;
        case GL_RG8UI:
            return PF_R8G8_UINT;
        case GL_RGB8UI:
            return PF_R8G8B8_UINT;
        case GL_RGBA8UI:
            return PF_R8G8B8A8_UINT;
        case GL_R16UI:
            return PF_R16_UINT;
        case GL_RG16UI:
            return PF_R16G16_UINT;
        case GL_RGB16UI:
            return PF_R16G16B16_UINT;
        case GL_RGBA16UI:
            return PF_R16G16B16A16_UINT;
        case GL_R32UI:
            return PF_R32_UINT;
        case GL_RG32UI:
            return PF_R32G32_UINT;
        case GL_RGB32UI:
            return PF_R32G32B32_UINT;
        case GL_RGBA32UI:
            return PF_R32G32B32A32_UINT;
        case GL_R8I:
            return PF_R8_SINT;
        case GL_RG8I:
            return PF_R8G8_SINT;
        case GL_RGB8I:
            return PF_R8G8B8_SINT;
        case GL_RGBA8I:
            return PF_R8G8B8A8_SINT;
        case GL_R16I:
            return PF_R16_SINT;
        case GL_RG16I:
            return PF_R16G16_SINT;
        case GL_RGB16I:
            return PF_R16G16B16_SINT;
        case GL_RGBA16I:
            return PF_R16G16B16A16_SINT;
        case GL_R32I:
            return PF_R32_SINT;
        case GL_RG32I:
            return PF_R32G32_SINT;
        case GL_RGB32I:
            return PF_R32G32B32_SINT;
        case GL_RGBA32I:
            return PF_R32G32B32A32_SINT;
        case GL_RGB9_E5:
            return PF_R9G9B9E5_SHAREDEXP;
        case GL_R8_SNORM:
            return PF_R8_SNORM;
        case GL_RG8_SNORM:
            return PF_R8G8_SNORM;
        case GL_RGB8_SNORM:
            return PF_R8G8B8_SNORM;
        case GL_RGBA8_SNORM:
            return PF_R8G8B8A8_SNORM;
        case GL_R16_SNORM:
            return PF_R16_SNORM;
        case GL_RG16_SNORM:
            return PF_R16G16_SNORM;
        case GL_RGB16_SNORM:
            return PF_R16G16B16_SNORM;
        case GL_RGBA16_SNORM:
            return PF_R16G16B16A16_SNORM;
        case GL_COMPRESSED_RED_RGTC1:
            return PF_BC4_UNORM;
        case GL_COMPRESSED_SIGNED_RED_RGTC1:
            return PF_BC4_SNORM;
        case GL_COMPRESSED_RG_RGTC2:
            return PF_BC5_UNORM;
        case GL_COMPRESSED_SIGNED_RG_RGTC2:
            return PF_BC5_SNORM;
        case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB:
            return PF_BC6H_SF16;
        case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB:
            return PF_BC6H_UF16;
        case GL_COMPRESSED_RGBA_BPTC_UNORM_ARB:
            return PF_BC7_UNORM;
        case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB:
            return PF_BC7_UNORM_SRGB;
        case GL_COMPRESSED_RGB8_ETC2:
            return PF_ETC2_RGB8;
        case GL_COMPRESSED_RGBA8_ETC2_EAC:
            return PF_ETC2_RGBA8;
        case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
            return PF_ETC2_RGB8A1;

        default:
            return PF_A8R8G8B8;
        };
    }


    size_t GL3PlusPixelUtil::getMaxMipmaps(size_t width, size_t height, size_t depth, PixelFormat format)
    {
        size_t count = 0;
        do {
            if(width>1)     width = width/2;
            if(height>1)    height = height/2;
            if(depth>1)     depth = depth/2;
            /*
              NOT needed, compressed formats will have mipmaps up to 1x1
              if(PixelUtil::isValidExtent(width, height, depth, format))
              count ++;
              else
              break;
            */

            count++;
        } while(!(width == 1 && height == 1 && depth == 1));

        return count;
    }

}
