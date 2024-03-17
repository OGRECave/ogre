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

#ifndef GL_EXT_texture_compression_s3tc
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT   0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT  0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT  0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT  0x83F3
#endif

#ifndef GL_EXT_texture_sRGB
#define GL_SRGB_EXT                       0x8C40
#define GL_SRGB8_EXT                      0x8C41
#define GL_SRGB_ALPHA_EXT                 0x8C42
#define GL_SRGB8_ALPHA8_EXT               0x8C43
#define GL_SLUMINANCE_ALPHA_EXT           0x8C44
#define GL_SLUMINANCE8_ALPHA8_EXT         0x8C45
#define GL_SLUMINANCE_EXT                 0x8C46
#define GL_SLUMINANCE8_EXT                0x8C47
#define GL_COMPRESSED_SRGB_EXT            0x8C48
#define GL_COMPRESSED_SRGB_ALPHA_EXT      0x8C49
#define GL_COMPRESSED_SLUMINANCE_EXT      0x8C4A
#define GL_COMPRESSED_SLUMINANCE_ALPHA_EXT 0x8C4B
#define GL_COMPRESSED_SRGB_S3TC_DXT1_EXT  0x8C4C
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT 0x8C4D
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT 0x8C4E
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT 0x8C4F
#endif


namespace Ogre  {

    struct GLPixelFormatDescription {
        GLenum format;
        GLenum type;
        GLenum internalFormat;
    };

    static GLPixelFormatDescription _pixelFormats[] = {
            {GL_NONE},                                           // PF_UNKNOWN
            {GL_RED, GL_UNSIGNED_BYTE, GL_R8},                   // PF_L8
            {GL_RED, GL_UNSIGNED_SHORT, GL_R16},                 // PF_L16
            {GL_RED, GL_UNSIGNED_BYTE, GL_R8},                   // PF_A8
            {GL_RG, GL_UNSIGNED_BYTE, GL_RG8},                   // PF_BYTE_LA
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
            {GL_BGRA_INTEGER, GL_UNSIGNED_INT_2_10_10_10_REV, GL_RGB10_A2UI},  // PF_A2R10G10B10
            {GL_RGBA_INTEGER, GL_UNSIGNED_INT_2_10_10_10_REV, GL_RGB10_A2UI},  // PF_A2B10G10R10
            {GL_NONE, GL_NONE, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT},// PF_DXT1
            {GL_NONE},                                           // PF_DXT2
            {GL_NONE, GL_NONE, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT},// PF_DXT3
            {GL_NONE},                                           // PF_DXT4
            {GL_NONE, GL_NONE, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT},// PF_DXT5
            {GL_RGB, GL_HALF_FLOAT, GL_RGB16F},                  // PF_FLOAT16_RGB
            {GL_RGBA, GL_HALF_FLOAT, GL_RGBA16F},                // PF_FLOAT16_RGBA
            {GL_RGB, GL_FLOAT, GL_RGB32F},                       // PF_FLOAT32_RGB
            {GL_RGBA, GL_FLOAT, GL_RGBA32F},                     // PF_FLOAT32_RGBA
            {GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, GL_RGBA8},    // PF_X8R8G8B8
            {GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, GL_RGBA8},    // PF_X8B8G8R8
            {GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, GL_RGBA8},        // PF_R8G8B8A8
            {GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, GL_DEPTH_COMPONENT16}, // PF_DEPTH16
            {GL_RGBA, GL_UNSIGNED_SHORT, GL_RGBA16},             // PF_SHORT_RGBA
            {GL_RGB, GL_UNSIGNED_BYTE_3_3_2, GL_R3_G3_B2},       // PF_R3G3B2
            {GL_RED, GL_HALF_FLOAT, GL_R16F},                    // PF_FLOAT16_R
            {GL_RED, GL_FLOAT, GL_R32F},                         // PF_FLOAT32_R
            {GL_RG, GL_UNSIGNED_SHORT, GL_RG16},                 // PF_SHORT_GR
            {GL_RG, GL_HALF_FLOAT, GL_RG16F},                    // PF_FLOAT16_GR
            {GL_RG, GL_FLOAT, GL_RG32F},                         // PF_FLOAT32_GR
            {GL_RGB, GL_UNSIGNED_SHORT, GL_RGB16},               // PF_SHORT_RGB
            {GL_NONE},                                           // PF_PVRTC_RGB2
            {GL_NONE},                                           // PF_PVRTC_RGBA2
            {GL_NONE},                                           // PF_PVRTC_RGB4
            {GL_NONE},                                           // PF_PVRTC_RGBA4
            {GL_NONE},                                           // PF_PVRTC2_2BPP
            {GL_NONE},                                           // PF_PVRTC2_4BPP
            {GL_RGB, GL_UNSIGNED_INT_10F_11F_11F_REV, GL_R11F_G11F_B10F}, // PF_R11G11B10_FLOAT
            {GL_RED_INTEGER, GL_UNSIGNED_BYTE, GL_R8UI},         // PF_R8_UINT
            {GL_RG_INTEGER, GL_UNSIGNED_BYTE, GL_RG8UI},         // PF_R8G8_UINT
            {GL_RGB_INTEGER, GL_UNSIGNED_BYTE, GL_RGB8UI},       // PF_R8G8B8_UINT
            {GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, GL_RGBA8UI},     // PF_R8G8B8A8_UINT
            {GL_RED_INTEGER, GL_UNSIGNED_SHORT, GL_R16UI},       // PF_R16_UINT
            {GL_RG_INTEGER, GL_UNSIGNED_SHORT, GL_RG16UI},       // PF_R16G16_UINT
            {GL_RGB_INTEGER, GL_UNSIGNED_SHORT, GL_RGB16UI},     // PF_R16G16B16_UINT
            {GL_RGBA_INTEGER, GL_UNSIGNED_SHORT, GL_RGBA16UI},   // PF_R16G16B16A16_UINT
            {GL_RED_INTEGER, GL_UNSIGNED_INT, GL_R32UI},         // PF_R32_UINT
            {GL_RG_INTEGER, GL_UNSIGNED_INT, GL_RG32UI},         // PF_R32G32_UINT
            {GL_RGB_INTEGER, GL_UNSIGNED_INT, GL_RGB32UI},       // PF_R32G32B32_UINT
            {GL_RGBA_INTEGER, GL_UNSIGNED_INT, GL_RGBA32UI},     // PF_R32G32B32A32_UINT
            {GL_RED_INTEGER, GL_BYTE, GL_R8I},                   // PF_R8_SINT
            {GL_RG_INTEGER, GL_BYTE, GL_RG8I},                   // PF_R8G8_SINT
            {GL_RGB_INTEGER, GL_BYTE, GL_RGB8I},                 // PF_R8G8B8_SINT
            {GL_RGBA_INTEGER, GL_BYTE, GL_RGBA8I},               // PF_R8G8B8A8_SINT
            {GL_RED_INTEGER, GL_SHORT, GL_R16I},                 // PF_R16_SINT
            {GL_RG_INTEGER, GL_SHORT, GL_RG16I},                 // PF_R16G16_SINT
            {GL_RGB_INTEGER, GL_SHORT, GL_RGB16I},               // PF_R16G16B16_SINT
            {GL_RGBA_INTEGER, GL_SHORT, GL_RGBA16I},             // PF_R16G16B16A16_SINT
            {GL_RED_INTEGER, GL_INT, GL_R32I},                   // PF_R32_SINT
            {GL_RG_INTEGER, GL_INT, GL_RG32I},                   // PF_R32G32_SINT
            {GL_RGB_INTEGER, GL_INT, GL_RGB32I},                 // PF_R32G32B32_SINT
            {GL_RGBA_INTEGER, GL_INT, GL_RGBA32I},               // PF_R32G32B32A32_SINT
            {GL_RGB, GL_UNSIGNED_INT_5_9_9_9_REV, GL_RGB9_E5},   // PF_R9G9B9E5_SHAREDEXP
            {GL_NONE, GL_NONE, GL_COMPRESSED_RED_RGTC1},                  // PF_BC4_UNORM
            {GL_NONE, GL_NONE, GL_COMPRESSED_SIGNED_RED_RGTC1},           // PF_BC4_SNORM
            {GL_NONE, GL_NONE, GL_COMPRESSED_RG_RGTC2},                   // PF_BC5_UNORM
            {GL_NONE, GL_NONE, GL_COMPRESSED_SIGNED_RG_RGTC2},            // PF_BC5_SNORM
            {GL_NONE, GL_NONE, GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB},// PF_BC6H_UF16
            {GL_NONE, GL_NONE, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB},  // PF_BC6H_SF16
            {GL_NONE, GL_NONE, GL_COMPRESSED_RGBA_BPTC_UNORM_ARB},        // PF_BC7_UNORM
            {GL_RED, GL_UNSIGNED_BYTE, GL_R8},                   // PF_R8
            {GL_RG, GL_UNSIGNED_BYTE, GL_RG8},                   // PF_RG8
            {GL_RED, GL_UNSIGNED_BYTE, GL_R8_SNORM},             // PF_R8_SNORM
            {GL_RG, GL_UNSIGNED_BYTE, GL_RG8_SNORM},             // PF_RG8_SNORM
            {GL_RGB, GL_UNSIGNED_BYTE, GL_RGB8_SNORM},           // PF_RGB8_SNORM
            {GL_RGBA, GL_UNSIGNED_BYTE, GL_RGBA8_SNORM},         // PF_RGBA8_SNORM
            {GL_RED, GL_UNSIGNED_SHORT, GL_R16_SNORM},           // PF_R16_SNORM
            {GL_RG, GL_UNSIGNED_SHORT, GL_RG16_SNORM},           // PF_RG16_SNORM
            {GL_RGB, GL_UNSIGNED_SHORT, GL_RGB16_SNORM},         // PF_RGB16_SNORM
            {GL_RGBA, GL_UNSIGNED_SHORT, GL_RGBA16_SNORM},       // PF_RGBA16_SNORM
            {GL_NONE},                                           // PF_ETC1_RGB8
            {GL_NONE, GL_NONE, GL_COMPRESSED_RGB8_ETC2},         // PF_ETC2_RGB8
            {GL_NONE, GL_NONE, GL_COMPRESSED_RGBA8_ETC2_EAC},    // PF_ETC2_RGBA8
            {GL_NONE, GL_NONE, GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2},// PF_ETC2_RGB8A1
            {GL_NONE},                                           // PF_ATC_RGB
            {GL_NONE},                                           // PF_ATC_RGBA_EXPLICIT_ALPHA
            {GL_NONE},                                           // PF_ATC_RGBA_INTERPOLATED_ALPHA
            {GL_NONE, GL_NONE, GL_COMPRESSED_RGBA_ASTC_4x4_KHR}, // PF_ASTC_RGBA_4X4_LDR
            {GL_NONE, GL_NONE, GL_COMPRESSED_RGBA_ASTC_5x4_KHR}, // PF_ASTC_RGBA_5X4_LDR
            {GL_NONE, GL_NONE, GL_COMPRESSED_RGBA_ASTC_5x5_KHR}, // PF_ASTC_RGBA_5X5_LDR
            {GL_NONE, GL_NONE, GL_COMPRESSED_RGBA_ASTC_6x5_KHR}, // PF_ASTC_RGBA_6X5_LDR
            {GL_NONE, GL_NONE, GL_COMPRESSED_RGBA_ASTC_6x6_KHR}, // PF_ASTC_RGBA_6X6_LDR
            {GL_NONE, GL_NONE, GL_COMPRESSED_RGBA_ASTC_8x5_KHR}, // PF_ASTC_RGBA_8X5_LDR
            {GL_NONE, GL_NONE, GL_COMPRESSED_RGBA_ASTC_8x6_KHR}, // PF_ASTC_RGBA_8X6_LDR
            {GL_NONE, GL_NONE, GL_COMPRESSED_RGBA_ASTC_8x8_KHR}, // PF_ASTC_RGBA_8X8_LDR
            {GL_NONE, GL_NONE, GL_COMPRESSED_RGBA_ASTC_10x5_KHR},// PF_ASTC_RGBA_10X5_LDR
            {GL_NONE, GL_NONE, GL_COMPRESSED_RGBA_ASTC_10x6_KHR},// PF_ASTC_RGBA_10X6_LDR
            {GL_NONE, GL_NONE, GL_COMPRESSED_RGBA_ASTC_10x8_KHR},// PF_ASTC_RGBA_10X8_LDR
            {GL_NONE, GL_NONE, GL_COMPRESSED_RGBA_ASTC_10x10_KHR},// PF_ASTC_RGBA_10X10_LDR
            {GL_NONE, GL_NONE, GL_COMPRESSED_RGBA_ASTC_12x10_KHR},// PF_ASTC_RGBA_12X10_LDR
            {GL_NONE, GL_NONE, GL_COMPRESSED_RGBA_ASTC_12x12_KHR},// PF_ASTC_RGBA_12X12_LDR
            {GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, GL_DEPTH_COMPONENT32}, // PF_DEPTH32
            {GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_COMPONENT32F}, // PF_DEPTH32F
            {GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, GL_DEPTH24_STENCIL8}, // PF_DEPTH24_STENCIL8
    };

    GLenum GL3PlusPixelUtil::getGLOriginFormat(PixelFormat pf)
    {
        static_assert(sizeof(_pixelFormats)/sizeof(GLPixelFormatDescription) == PF_COUNT, "Did you add a new format?");
        return _pixelFormats[pf].format;
    }

    GLenum GL3PlusPixelUtil::getGLOriginDataType(PixelFormat pf)
    {
        return _pixelFormats[pf].type;
    }

    GLenum GL3PlusPixelUtil::getGLInternalFormat(PixelFormat pf, bool hwGamma)
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
        case GL_COMPRESSED_RGBA_BPTC_UNORM_ARB:
            return GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB;
        case GL_COMPRESSED_RGBA_ASTC_4x4_KHR:
        case GL_COMPRESSED_RGBA_ASTC_5x4_KHR:
        case GL_COMPRESSED_RGBA_ASTC_5x5_KHR:
        case GL_COMPRESSED_RGBA_ASTC_6x5_KHR:
        case GL_COMPRESSED_RGBA_ASTC_6x6_KHR:
        case GL_COMPRESSED_RGBA_ASTC_8x5_KHR:
        case GL_COMPRESSED_RGBA_ASTC_8x6_KHR:
        case GL_COMPRESSED_RGBA_ASTC_8x8_KHR:
        case GL_COMPRESSED_RGBA_ASTC_10x5_KHR:
        case GL_COMPRESSED_RGBA_ASTC_10x6_KHR:
        case GL_COMPRESSED_RGBA_ASTC_10x8_KHR:
        case GL_COMPRESSED_RGBA_ASTC_10x10_KHR:
        case GL_COMPRESSED_RGBA_ASTC_12x10_KHR:
        case GL_COMPRESSED_RGBA_ASTC_12x12_KHR:
            return ret + 0x20; // ASTC SRGBA format offset
        default:
            return ret;
        }
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
        return (GLformat == GL_NONE ? GL_RGBA8 : GLformat);
    }


    PixelFormat GL3PlusPixelUtil::getClosestOGREFormat(GLenum format)
    {
        switch(format)
        {
        case GL_DEPTH_COMPONENT24:
        case GL_DEPTH_COMPONENT:
            return PF_DEPTH16;
        case GL_DEPTH_COMPONENT32:
            return PF_DEPTH32;
        case GL_DEPTH_COMPONENT32F:
            return PF_DEPTH32F;
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
            return PF_DXT1;
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
            return PF_DXT3;
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
            return PF_DXT5;
        case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB:
            return PF_BC7_UNORM;
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR:
            return PixelFormat(int(PF_ASTC_RGBA_4X4_LDR) +
                               (format - GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR));
        case GL_SRGB8:
        case GL_RGB8: // prefer native endian byte format
            return PF_BYTE_RGB;
        case GL_SRGB8_ALPHA8:
        case GL_RGBA8: // prefer native endian byte format
            return PF_BYTE_RGBA;
        };

        for(int pf = 0; pf < PF_COUNT; pf++) {
            if(_pixelFormats[pf].internalFormat == format)
                return (PixelFormat)pf;
        }

        return PF_BYTE_RGBA;
    }
}
