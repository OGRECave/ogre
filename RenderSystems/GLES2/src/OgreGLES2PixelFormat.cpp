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

#include "OgreGLES2PixelFormat.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreBitwise.h"

namespace Ogre {
	//-----------------------------------------------------------------------------
    GLenum GLES2PixelUtil::getGLOriginFormat(PixelFormat mFormat)
    {
        switch (mFormat)
        {
            case PF_A8:
                return GL_ALPHA;
            case PF_DEPTH:
                return GL_DEPTH_COMPONENT;

#if GL_OES_texture_half_float || GL_EXT_color_buffer_half_float || (OGRE_NO_GLES3_SUPPORT == 0) && OGRE_PLATFORM != OGRE_PLATFORM_NACL
            case PF_FLOAT16_RGB:
            case PF_FLOAT32_RGB:
                return GL_RGB;
            case PF_FLOAT16_RGBA:
            case PF_FLOAT32_RGBA:
                return GL_RGBA;
#endif

#if (OGRE_NO_GLES3_SUPPORT == 0)
            case PF_FLOAT16_R:
            case PF_FLOAT32_R:
            case PF_R8:
                return GL_RED_EXT;
            case PF_FLOAT16_GR:
            case PF_FLOAT32_GR:
            case PF_RG8:
                return GL_RG_EXT;
#else
            case PF_BYTE_LA:
            case PF_SHORT_GR:
                return GL_LUMINANCE_ALPHA;
#endif

#if (GL_EXT_texture_rg && OGRE_PLATFORM != OGRE_PLATFORM_NACL)
            case PF_FLOAT16_R:
            case PF_FLOAT32_R:
            case PF_R8:
                return GL_RED_EXT;

            case PF_FLOAT16_GR:
            case PF_FLOAT32_GR:
            case PF_RG8:
                return GL_RG_EXT;
#endif

            // PVRTC compressed formats
#if GL_IMG_texture_compression_pvrtc && OGRE_PLATFORM != OGRE_PLATFORM_NACL
            case PF_PVRTC_RGB2:
                return GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
            case PF_PVRTC_RGB4:
                return GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
            case PF_PVRTC_RGBA2:
                return GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
            case PF_PVRTC_RGBA4:
                return GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
#endif                
#if GL_IMG_texture_compression_pvrtc2 && OGRE_PLATFORM != OGRE_PLATFORM_NACL
            case PF_PVRTC2_2BPP:
                return GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG;
            case PF_PVRTC2_4BPP:
                return GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG;
#endif

#if OGRE_NO_ETC_CODEC == 0 
#	ifdef GL_OES_compressed_ETC1_RGB8_texture
            case PF_ETC1_RGB8:
                return GL_ETC1_RGB8_OES;
#	endif
#	ifdef GL_AMD_compressed_ATC_texture
			case PF_ATC_RGB:
				return ATC_RGB_AMD;
			case PF_ATC_RGBA_EXPLICIT_ALPHA:
				return ATC_RGBA_EXPLICIT_ALPHA_AMD;
			case PF_ATC_RGBA_INTERPOLATED_ALPHA:
				return ATC_RGBA_INTERPOLATED_ALPHA_AMD;
#	endif
#endif

#if OGRE_NO_GLES3_SUPPORT == 0
            case PF_ETC2_RGB8:
                return GL_COMPRESSED_RGB8_ETC2;
            case PF_ETC2_RGBA8:
                return GL_COMPRESSED_RGBA8_ETC2_EAC;
            case PF_ETC2_RGB8A1:
                return GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
#endif

            case PF_R5G6B5:
            case PF_B5G6R5:
            case PF_R8G8B8:
            case PF_B8G8R8:
            case PF_X8B8G8R8:
            case PF_SHORT_RGB:
                return GL_RGB;

            case PF_X8R8G8B8:
			case PF_A8R8G8B8:
            case PF_A4R4G4B4:
            case PF_A1R5G5B5:
            case PF_B8G8R8A8:
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
                return GL_BGRA_EXT;
#else
                return GL_RGBA;
#endif

            case PF_A8B8G8R8:
			case PF_R8G8B8A8:
            case PF_A2R10G10B10:
            case PF_A2B10G10R10:
            case PF_SHORT_RGBA:
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS && OGRE_NO_GLES3_SUPPORT == 1
                return GL_BGRA_EXT;
#else
                return GL_RGBA;
#endif

            case PF_DXT1:
#if GL_EXT_texture_compression_dxt1
                return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
#endif
            case PF_DXT3:
#if GL_EXT_texture_compression_s3tc
                return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
#endif
            case PF_DXT5:
#if GL_EXT_texture_compression_s3tc
                return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
#endif
#if OGRE_NO_GLES3_SUPPORT == 0
            case PF_L8:
                return GL_RED;
            case PF_R8_UINT:
            case PF_R16_UINT:
            case PF_R32_UINT:
            case PF_R8_SINT:
            case PF_R16_SINT:
            case PF_R32_SINT:
				return GL_RED_INTEGER;
            case PF_R8G8_UINT:
            case PF_R16G16_UINT:
            case PF_R32G32_UINT:
            case PF_R8G8_SINT:
            case PF_R16G16_SINT:
            case PF_R32G32_SINT:
				return GL_RG_INTEGER;
            case PF_R8G8B8_UINT:
            case PF_R16G16B16_UINT:
            case PF_R32G32B32_UINT:
            case PF_R8G8B8_SINT:
            case PF_R16G16B16_SINT:
			case PF_R32G32B32_SINT:
				return GL_RGB_INTEGER;
            case PF_R8G8B8A8_UINT:
            case PF_R16G16B16A16_UINT:
            case PF_R32G32B32A32_UINT:
            case PF_R8G8B8A8_SINT:
            case PF_R16G16B16A16_SINT:
            case PF_R32G32B32A32_SINT:
                return GL_RGBA_INTEGER;
            case PF_R11G11B10_FLOAT:
            case PF_R9G9B9E5_SHAREDEXP:
                return GL_RGB;
            case PF_R8_SNORM:
                return GL_R8_SNORM;
            case PF_R8G8_SNORM:
                return GL_RG8_SNORM;
            case PF_R8G8B8_SNORM:
                return GL_RGB8_SNORM;
            case PF_R8G8B8A8_SNORM:
                return GL_RGBA8_SNORM;
#else
            case PF_L8:
            case PF_L16:
                return GL_LUMINANCE;
#endif
            default:
                return 0;
        }
    }
	//-----------------------------------------------------------------------------
    GLenum GLES2PixelUtil::getGLOriginDataType(PixelFormat mFormat)
    {
        switch (mFormat)
        {
            case PF_DEPTH:
                return GL_UNSIGNED_INT;
            case PF_A8:
            case PF_L8:
            case PF_L16:
            case PF_R8G8B8:
            case PF_B8G8R8:
            case PF_BYTE_LA:
                return GL_UNSIGNED_BYTE;
            case PF_R5G6B5:
            case PF_B5G6R5:
                return GL_UNSIGNED_SHORT_5_6_5;
            case PF_A4R4G4B4:
            case PF_SHORT_RGB:
            case PF_SHORT_RGBA:
				return GL_UNSIGNED_SHORT_4_4_4_4;
            case PF_A1R5G5B5:
                return GL_UNSIGNED_SHORT_5_5_5_1;

#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
            case PF_X8B8G8R8:
            case PF_X8R8G8B8:
            case PF_A8B8G8R8:
            case PF_A8R8G8B8:
                return GL_UNSIGNED_INT_8_8_8_8_REV;
            case PF_B8G8R8A8:
            case PF_R8G8B8A8:
                return GL_UNSIGNED_BYTE;
#else
            case PF_X8B8G8R8:
            case PF_A8B8G8R8:
            case PF_X8R8G8B8:
            case PF_A8R8G8B8:
            case PF_B8G8R8A8:
            case PF_R8G8B8A8:
                return GL_UNSIGNED_BYTE;
#endif

            case PF_FLOAT16_R:
            case PF_FLOAT16_GR:
            case PF_FLOAT16_RGB:
            case PF_FLOAT16_RGBA:
#if (GL_OES_texture_half_float && OGRE_PLATFORM != OGRE_PLATFORM_NACL) || (OGRE_NO_GLES3_SUPPORT == 0)
                return GL_HALF_FLOAT_OES;
#else
                return 0;
#endif
#if (GL_EXT_texture_rg && OGRE_PLATFORM != OGRE_PLATFORM_NACL) || (OGRE_NO_GLES3_SUPPORT == 0)
            case PF_R8:
            case PF_RG8:
                return GL_UNSIGNED_BYTE;
#endif
            case PF_FLOAT32_R:
            case PF_FLOAT32_GR:
            case PF_FLOAT32_RGB:
            case PF_FLOAT32_RGBA:
#if (GL_OES_texture_float && OGRE_PLATFORM != OGRE_PLATFORM_NACL) || (OGRE_NO_GLES3_SUPPORT == 0)
                return GL_FLOAT;
#endif
#if OGRE_NO_GLES3_SUPPORT == 0
            case PF_A2R10G10B10:
                return GL_UNSIGNED_INT_2_10_10_10_REV;
            case PF_A2B10G10R10:
                return GL_UNSIGNED_INT_2_10_10_10_REV;
            case PF_R8_SNORM:
            case PF_R8G8_SNORM:
            case PF_R8G8B8_SNORM:
            case PF_R8G8B8A8_SNORM:
            case PF_R16_SNORM:
            case PF_R16G16_SNORM:
            case PF_R16G16B16_SNORM:
            case PF_R16G16B16A16_SNORM:
            case PF_R8_SINT:
            case PF_R8G8_SINT:
            case PF_R8G8B8_SINT:
            case PF_R8G8B8A8_SINT:
                return GL_BYTE;
            case PF_R8_UINT:
            case PF_R8G8_UINT:
            case PF_R8G8B8_UINT:
            case PF_R8G8B8A8_UINT:
				return GL_UNSIGNED_BYTE;
            case PF_R32_UINT:
            case PF_R32G32_UINT:
            case PF_R32G32B32_UINT:
            case PF_R32G32B32A32_UINT:
                return GL_UNSIGNED_INT;
            case PF_R16_UINT:
            case PF_R16G16_UINT:
            case PF_R16G16B16_UINT:
            case PF_R16G16B16A16_UINT:
				return GL_UNSIGNED_SHORT;
            case PF_R16_SINT:
            case PF_R16G16_SINT:
            case PF_R16G16B16_SINT:
            case PF_R16G16B16A16_SINT:
				return GL_SHORT;
            case PF_R32G32B32_SINT:
            case PF_R32_SINT:
            case PF_R32G32_SINT:
            case PF_R32G32B32A32_SINT:
                return GL_INT;

			case PF_R9G9B9E5_SHAREDEXP:
                return GL_UNSIGNED_INT_5_9_9_9_REV;
            case PF_R11G11B10_FLOAT:
                return GL_UNSIGNED_INT_10F_11F_11F_REV;
#endif
            default:
                return 0;
        }
    }

	//-----------------------------------------------------------------------------
    GLenum GLES2PixelUtil::getGLInternalFormat(PixelFormat fmt, bool hwGamma)
    {
        switch (fmt)
        {
            case PF_DEPTH:
                return GL_DEPTH_COMPONENT;
#if GL_IMG_texture_compression_pvrtc && OGRE_PLATFORM != OGRE_PLATFORM_NACL
            case PF_PVRTC_RGB2:
                return GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
            case PF_PVRTC_RGB4:
                return GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
            case PF_PVRTC_RGBA2:
                return GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
            case PF_PVRTC_RGBA4:
                return GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
#endif
#if GL_IMG_texture_compression_pvrtc2 && OGRE_PLATFORM != OGRE_PLATFORM_NACL
            case PF_PVRTC2_2BPP:
                return GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG;
            case PF_PVRTC2_4BPP:
                return GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG;
#endif

#if OGRE_NO_ETC_CODEC == 0 
#	ifdef GL_OES_compressed_ETC1_RGB8_texture
            case PF_ETC1_RGB8:
                return GL_ETC1_RGB8_OES;
#	endif
#	ifdef GL_AMD_compressed_ATC_texture
			case PF_ATC_RGB:
				return ATC_RGB_AMD;
			case PF_ATC_RGBA_EXPLICIT_ALPHA:
				return ATC_RGBA_EXPLICIT_ALPHA_AMD;
			case PF_ATC_RGBA_INTERPOLATED_ALPHA:
				return ATC_RGBA_INTERPOLATED_ALPHA_AMD;
#	endif
#endif

#if OGRE_NO_GLES3_SUPPORT == 0
            case PF_ETC2_RGB8:
                return GL_COMPRESSED_RGB8_ETC2;
            case PF_ETC2_RGBA8:
                return GL_COMPRESSED_RGBA8_ETC2_EAC;
            case PF_ETC2_RGB8A1:
                return GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
#endif

#if OGRE_NO_GLES3_SUPPORT == 0
			case PF_A1R5G5B5:
				return GL_RGB5_A1;
            case PF_R5G6B5:
			case PF_B5G6R5:
                return GL_RGB565;
            case PF_A4R4G4B4:
                return GL_RGBA4;
            case PF_R8G8B8:
            case PF_B8G8R8:
				if (hwGamma)
					return GL_SRGB8;
				else
					return GL_RGB8;
            case PF_A8R8G8B8:
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
                return GL_BGRA8_EXT;
#endif
            case PF_B8G8R8A8:
            case PF_A8B8G8R8:
            case PF_R8G8B8A8:
			case PF_X8B8G8R8:
			case PF_X8R8G8B8:
				if (hwGamma)
					return GL_SRGB8_ALPHA8;
				else
					return GL_RGBA8;
            case PF_A2R10G10B10:
            case PF_A2B10G10R10:
                return GL_RGB10_A2UI;
            case PF_FLOAT32_RGB:
                return GL_RGB32F;
            case PF_FLOAT32_RGBA:
                return GL_RGBA32F;
            case PF_FLOAT16_RGBA:
                return GL_RGBA16F;
            case PF_FLOAT16_RGB:
                return GL_RGB16F;
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
            case PF_L8:
            case PF_A8:
                return GL_R8;
            case PF_L16:
                return GL_R16F_EXT;
#else
            case PF_L8:
            case PF_L16:
                return GL_LUMINANCE;

            case PF_A8:
                return GL_ALPHA;

            case PF_BYTE_LA:
                return GL_LUMINANCE_ALPHA;
            case PF_A8R8G8B8:
            case PF_A8B8G8R8:
            case PF_B8G8R8A8:
            case PF_A1R5G5B5:
            case PF_A4R4G4B4:
            case PF_X8B8G8R8:
            case PF_X8R8G8B8:
            case PF_SHORT_RGBA:
                return GL_RGBA;
            case PF_FLOAT16_RGB:
            case PF_FLOAT32_RGB:
            case PF_R5G6B5:
            case PF_B5G6R5:
            case PF_R8G8B8:
            case PF_B8G8R8:
            case PF_SHORT_RGB:
                return GL_RGB;
            case PF_FLOAT32_RGBA:
            case PF_A2R10G10B10:
            case PF_A2B10G10R10:
#endif

            case PF_A4L4:
            case PF_R3G3B2:
            case PF_SHORT_GR:
			case PF_DXT1:
#if GL_EXT_texture_compression_dxt1
				if (!hwGamma)
					return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
#endif
            case PF_DXT3:
#if GL_EXT_texture_compression_s3tc
				if (!hwGamma)
	                return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
#endif
            case PF_DXT5:
#if GL_EXT_texture_compression_s3tc
				if (!hwGamma)
	                return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
#endif
                
#if (GL_EXT_texture_rg && OGRE_PLATFORM != OGRE_PLATFORM_NACL) || (OGRE_NO_GLES3_SUPPORT == 0)
            case PF_FLOAT16_R:
                return GL_R16F_EXT;
            case PF_FLOAT32_R:
                return GL_R32F_EXT;
            case PF_R8:
                return GL_RED_EXT;
            case PF_FLOAT16_GR:
                return GL_RG16F_EXT;
            case PF_FLOAT32_GR:
                return GL_RG32F_EXT;
            case PF_RG8:
                return GL_RG_EXT;
#endif
            default:
                return GL_NONE;
        }
    }
	//-----------------------------------------------------------------------------
    GLenum GLES2PixelUtil::getClosestGLInternalFormat(PixelFormat mFormat,
                                                   bool hwGamma)
    {
        GLenum format = getGLInternalFormat(mFormat, hwGamma);
        if (format == GL_NONE)
        {
#if OGRE_NO_GLES3_SUPPORT == 0
            if (hwGamma)
                return GL_SRGB8;
            else
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
                return GL_BGRA8_EXT;
#else
                return GL_RGBA8;
#endif
#else
            if (hwGamma)
            {
                // TODO not supported
                return 0;
            }
            else
            {
                return GL_RGBA8_OES;
            }
#endif
        }
        else
        {
            return format;
        }
    }
	//-----------------------------------------------------------------------------
    PixelFormat GLES2PixelUtil::getClosestOGREFormat(GLenum fmt, GLenum dataType)
    {
        switch (fmt)
        {
            case GL_DEPTH_COMPONENT:
            case GL_DEPTH24_STENCIL8_OES:
            case GL_DEPTH_COMPONENT16:
            case GL_DEPTH_COMPONENT24_OES:
            case GL_DEPTH_COMPONENT32_OES:
                return PF_DEPTH;

#if GL_IMG_texture_compression_pvrtc
            case GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG:
                return PF_PVRTC_RGB2;
            case GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG:
                return PF_PVRTC_RGBA2;
            case GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG:
                return PF_PVRTC_RGB4;
            case GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG:
                return PF_PVRTC_RGBA4;
#endif

#if GL_IMG_texture_compression_pvrtc2
            case GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG:
                return PF_PVRTC2_2BPP;
            case GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG:
                return PF_PVRTC2_4BPP;
#endif

#if OGRE_NO_ETC_CODEC == 0 
#	ifdef GL_OES_compressed_ETC1_RGB8_texture
            case GL_ETC1_RGB8_OES:
                return PF_ETC1_RGB8;
#	endif
#	ifdef GL_AMD_compressed_ATC_texture
			case ATC_RGB_AMD:
				return PF_ATC_RGB;
			case ATC_RGBA_EXPLICIT_ALPHA_AMD:
				return PF_ATC_RGBA_EXPLICIT_ALPHA;
			case ATC_RGBA_INTERPOLATED_ALPHA_AMD:
				return PF_ATC_RGBA_INTERPOLATED_ALPHA;
#	endif
#endif

#if OGRE_NO_GLES3_SUPPORT == 0
            case GL_COMPRESSED_RGB8_ETC2:
                return PF_ETC2_RGB8;
            case GL_COMPRESSED_RGBA8_ETC2_EAC:
                return PF_ETC2_RGBA8;
            case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
                return PF_ETC2_RGB8A1;
#endif

            case GL_LUMINANCE:
                return PF_L8;
            case GL_ALPHA:
                return PF_A8;
            case GL_LUMINANCE_ALPHA:
                return PF_BYTE_LA;
                
            case GL_RGB:
            case GL_RGB8_OES:
                switch(dataType)
                {
                    case GL_UNSIGNED_SHORT_5_6_5:
                        return PF_B5G6R5;
                    case GL_HALF_FLOAT_OES:
                        return PF_FLOAT16_RGB;
                    case GL_FLOAT:
                        return PF_FLOAT32_RGB;
                    default:
                        return PF_R8G8B8;
                }
            case GL_RGBA:
            case GL_RGBA8_OES:
                switch(dataType)
                {
                    case GL_UNSIGNED_SHORT_5_5_5_1:
                        return PF_A1R5G5B5;
                    case GL_UNSIGNED_SHORT_4_4_4_4:
                        return PF_A4R4G4B4;
                    case GL_HALF_FLOAT_OES:
                        return PF_FLOAT16_RGBA;
                    case GL_FLOAT:
                        return PF_FLOAT32_RGBA;
                    default:
                        return PF_A8B8G8R8;
                }
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
            case GL_BGRA8_EXT:
            case GL_BGRA_EXT:
                return PF_A8R8G8B8;
#endif

#if GL_EXT_texture_compression_dxt1
            case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
            case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
                return PF_DXT1;
#endif
#if GL_EXT_texture_compression_s3tc
            case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
                return PF_DXT3;
            case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
                return PF_DXT5;
#endif
                
#if (GL_EXT_texture_rg && OGRE_PLATFORM != OGRE_PLATFORM_NACL) || (OGRE_NO_GLES3_SUPPORT == 0)
            case GL_R8_EXT:
                return PF_R8;
            case GL_RG8_EXT:
                return PF_RG8;
#endif
#if OGRE_NO_GLES3_SUPPORT == 0
            case GL_RGB10_A2:
            case GL_RGB10_A2UI:
                return PF_A2R10G10B10;
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
            case GL_SRGB8:
                return PF_X8R8G8B8;
            case GL_SRGB8_ALPHA8:
                return PF_A8R8G8B8;
#endif

#if GL_EXT_color_buffer_half_float || (OGRE_NO_GLES3_SUPPORT == 0)
            case GL_RGBA16F_EXT:
                return PF_FLOAT16_RGBA;
            case GL_RGB16F_EXT:
                return PF_FLOAT16_RGB;
            case GL_RG16F_EXT:
                return PF_FLOAT16_GR;
            case GL_R16F_EXT:
                return PF_FLOAT16_R;
#endif

#if GL_EXT_color_buffer_float || (OGRE_NO_GLES3_SUPPORT == 0)
            case GL_RGBA32F_EXT:
                return PF_FLOAT32_RGBA;
            case GL_RGB32F_EXT:
                return PF_FLOAT32_RGB;
            case GL_RG32F_EXT:
                return PF_FLOAT32_GR;
            case GL_R32F_EXT:
                return PF_FLOAT32_R;
#endif

            default:
                LogManager::getSingleton().logMessage("Unhandled Pixel format: " + StringConverter::toString(fmt));
                return PF_A8B8G8R8;
        };
    }
	//-----------------------------------------------------------------------------
    size_t GLES2PixelUtil::getMaxMipmaps(uint32 width, uint32 height, uint32 depth,
                                      PixelFormat format)
    {
		size_t count = 0;
        if((width > 0) && (height > 0) && (depth > 0))
        {
            do {
                if(width > 1)
                    width = width / 2;
                if(height > 1)
                    height = height / 2;
                if(depth > 1)
                    depth = depth / 2;
                /*
                 NOT needed, compressed formats will have mipmaps up to 1x1
                 if(PixelUtil::isValidExtent(width, height, depth, format))
                 count ++;
                 else
                 break;
                 */
                
                count++;
            } while(!(width == 1 && height == 1 && depth == 1));
        }		
		return count;
    }
	//-----------------------------------------------------------------------------
    // TODO: Remove
    uint32 GLES2PixelUtil::optionalPO2(uint32 value)
    {
        const RenderSystemCapabilities *caps =
            Root::getSingleton().getRenderSystem()->getCapabilities();

        if (caps->hasCapability(RSC_NON_POWER_OF_2_TEXTURES))
        {
            return value;
        }
        else
        {
            return Bitwise::firstPO2From(value);
        }
    }
}
