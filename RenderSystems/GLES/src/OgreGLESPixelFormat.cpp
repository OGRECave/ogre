/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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

#include "OgreGLESPixelFormat.h"

#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreBitwise.h"

/* GL_AMD_compressed_ATC_texture */
#if OGRE_NO_ETC_CODEC == 0 
#	define ATC_RGB_AMD						  							0x8C92
#	define ATC_RGBA_EXPLICIT_ALPHA_AMD		  							0x8C93
#	define ATC_RGBA_INTERPOLATED_ALPHA_AMD	  							0x87EE
#endif


namespace Ogre  {
    GLenum GLESPixelUtil::getGLOriginFormat(PixelFormat mFormat)
    {
        switch (mFormat)
        {
            case PF_A8:
                return GL_ALPHA;

            case PF_L8:
            case PF_L16:
            case PF_FLOAT16_R:
            case PF_FLOAT32_R:
                return GL_LUMINANCE;

            case PF_BYTE_LA:
            case PF_SHORT_GR:
            case PF_FLOAT16_GR:
            case PF_FLOAT32_GR:
                return GL_LUMINANCE_ALPHA;

            // PVRTC compressed formats
#if GL_IMG_texture_compression_pvrtc
            case PF_PVRTC_RGB2:
                return GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
            case PF_PVRTC_RGB4:
                return GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
            case PF_PVRTC_RGBA2:
                return GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
            case PF_PVRTC_RGBA4:
                return GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
#endif                
            case PF_R5G6B5:
            case PF_B5G6R5:
            case PF_R8G8B8:
            case PF_B8G8R8:
                return GL_RGB;

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

#if GL_EXT_texture_compression_dxt1
            case PF_DXT1:
                return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
#endif

#if GL_EXT_texture_compression_s3tc
            case PF_DXT3:
                return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
#endif

#if GL_EXT_texture_compression_s3tc
            case PF_DXT5:
                return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
#endif

#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
            case PF_A1R5G5B5:
                return GL_BGRA;
            case PF_A4R4G4B4:
            case PF_X8R8G8B8:
            case PF_A8R8G8B8:
            case PF_B8G8R8A8:
            case PF_X8B8G8R8:
            case PF_A8B8G8R8:
                return GL_RGBA;
#else
			case PF_X8R8G8B8:
			case PF_A8R8G8B8:
            case PF_A8B8G8R8:
            case PF_B8G8R8A8:
            case PF_A2R10G10B10:
                return GL_BGRA;
            case PF_A4R4G4B4:
            case PF_X8B8G8R8:
			case PF_R8G8B8A8:
            case PF_A2B10G10R10:
                return GL_RGBA;
#endif
            default:
                return 0;
        }
    }

    GLenum GLESPixelUtil::getGLOriginDataType(PixelFormat mFormat)
    {
        switch (mFormat)
        {
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
            default:
                return 0;
        }
    }

    GLenum GLESPixelUtil::getGLInternalFormat(PixelFormat fmt, bool hwGamma)
    {
        switch (fmt)
        {
            case PF_L8:
            case PF_L16:
                return GL_LUMINANCE;

            case PF_A8:
                return GL_ALPHA;

            case PF_BYTE_LA:
                return GL_LUMINANCE_ALPHA;

#if GL_IMG_texture_compression_pvrtc
            case PF_PVRTC_RGB2:
                return GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
            case PF_PVRTC_RGB4:
                return GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
            case PF_PVRTC_RGBA2:
                return GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
            case PF_PVRTC_RGBA4:
                return GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
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
  
#if GL_EXT_texture_compression_dxt1
            case PF_DXT1:
                return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
#endif

#if GL_EXT_texture_compression_s3tc
            case PF_DXT3:
                return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
#endif

#if GL_EXT_texture_compression_s3tc
            case PF_DXT5:
                return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
#endif
  
            case PF_R8G8B8:
            case PF_B8G8R8:
                return GL_RGB;
			case PF_X8B8G8R8:
			case PF_X8R8G8B8:
            case PF_A8R8G8B8:
            case PF_A8B8G8R8:
            case PF_B8G8R8A8:
                return GL_RGBA;
            default:
                return 0;
        }
    }

    GLenum GLESPixelUtil::getClosestGLInternalFormat(PixelFormat mFormat,
                                                   bool hwGamma)
    {
        GLenum format = getGLInternalFormat(mFormat, hwGamma);
        if (format == 0)
        {
            if (hwGamma)
            {
                // TODO not supported
                return 0;
            }
            else
            {
                return GL_RGBA8_OES;
            }
        }
        else
        {
            return format;
        }
    }

    PixelFormat GLESPixelUtil::getClosestOGREFormat(GLenum fmt, GLenum dataType)
    {
        switch (fmt)
        {
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

            case GL_LUMINANCE:
                return PF_L8;
            case GL_ALPHA:
                return PF_A8;
            case GL_LUMINANCE_ALPHA:
                return PF_BYTE_LA;
            
            case GL_RGB:
                switch(dataType)
                {
                    case GL_UNSIGNED_SHORT_5_6_5:
                        return PF_B5G6R5;
                    default:
                        return PF_R8G8B8;
                }
            case GL_RGBA:
                switch(dataType)
                {
                    case GL_UNSIGNED_SHORT_5_5_5_1:
                        return PF_A1R5G5B5;
                    case GL_UNSIGNED_SHORT_4_4_4_4:
                        return PF_A4R4G4B4;
                    default:
                        return PF_A8B8G8R8;
                }
#ifdef GL_BGRA
            case GL_BGRA:
                return PF_A8R8G8B8;
#endif
            case GL_RGB8_OES:
                return PF_X8R8G8B8;
            case GL_RGBA8_OES:
                return PF_A8R8G8B8;

            default:
                return PF_A8R8G8B8;
        };
    }

    size_t GLESPixelUtil::getMaxMipmaps(size_t width, size_t height, size_t depth,
                                      PixelFormat format)
    {
		size_t count = 0;
        if((width > 0) && (height > 0) && (depth > 0))
        {
            do {
                if(width>1)		width = width/2;
                if(height>1)	height = height/2;
                if(depth>1)		depth = depth/2;
                /*
                 NOT needed, compressed formats will have mipmaps up to 1x1
                 if(PixelUtil::isValidExtent(width, height, depth, format))
                 count ++;
                 else
                 break;
                 */
                
                count ++;
            } while(!(width == 1 && height == 1 && depth == 1));
        }		
		return count;
    }

    size_t GLESPixelUtil::optionalPO2(size_t value)
    {
        const RenderSystemCapabilities *caps =
            Root::getSingleton().getRenderSystem()->getCapabilities();

        if (caps->hasCapability(RSC_NON_POWER_OF_2_TEXTURES))
        {
            return value;
        }
        else
        {
            return Bitwise::firstPO2From((uint32)value);
        }
    }

    void GLESPixelUtil::convertToGLformat(const PixelBox &src, const PixelBox &dst)
    {
        // Always need to convert PF_A4R4G4B4, GL expects the colors to be in the 
        // reverse order
        if (dst.format == PF_A4R4G4B4)
        {
            // Convert PF_A4R4G4B4 -> PF_B4G4R4A4
            // Reverse pixel order
            uint16 *srcptr = static_cast<uint16*>(src.data)
			+ (src.left + src.top * src.rowPitch + src.front * src.slicePitch);
            uint16 *dstptr = static_cast<uint16*>(dst.data)
			+ (dst.left + dst.top * dst.rowPitch + dst.front * dst.slicePitch);
            const size_t srcSliceSkip = src.getSliceSkip();
            const size_t dstSliceSkip = dst.getSliceSkip();
            const size_t k = src.right - src.left;
            for(size_t z=src.front; z<src.back; z++) 
            {
                for(size_t y=src.top; y<src.bottom; y++)
                {
                    for(size_t x=0; x<k; x++)
                    {
                        dstptr[x] = ((srcptr[x]&0x000F)<<12) |  // B
                                    ((srcptr[x]&0x00F0)<<4) |   // G
                                    ((srcptr[x]&0x0F00)>>4) |   // R
                                    ((srcptr[x]&0xF000)>>12);   // A
                    }
                    srcptr += src.rowPitch;
                    dstptr += dst.rowPitch;
                }
                srcptr += srcSliceSkip;
                dstptr += dstSliceSkip;
            }    
        }
    }
}
