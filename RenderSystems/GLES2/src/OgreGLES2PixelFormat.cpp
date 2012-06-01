/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2012 Torus Knot Software Ltd

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

namespace Ogre  {
	//-----------------------------------------------------------------------------
    GLenum GLES2PixelUtil::getGLOriginFormat(PixelFormat mFormat)
    {
        switch (mFormat)
        {
            case PF_A8:
                return GL_ALPHA;

            case PF_L8:
            case PF_L16:
                return GL_LUMINANCE;
#if GL_OES_texture_half_float
            case PF_FLOAT16_RGB:
            case PF_FLOAT32_RGB:
                return GL_RGB;
            case PF_FLOAT16_RGBA:
            case PF_FLOAT32_RGBA:
                return GL_RGBA;
#endif

#if GL_EXT_texture_rg
            case PF_FLOAT16_R:
            case PF_FLOAT32_R:
            case PF_R8:
                return GL_RED_EXT;

            case PF_FLOAT16_GR:
            case PF_FLOAT32_GR:
            case PF_RG8:
                return GL_RG_EXT;
#endif
            case PF_BYTE_LA:
            case PF_SHORT_GR:
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

			case PF_X8R8G8B8:
			case PF_A8R8G8B8:
            case PF_A8B8G8R8:
            case PF_B8G8R8A8:
            case PF_A2R10G10B10:
                return GL_BGRA;
			case PF_X8B8G8R8:
			case PF_R8G8B8A8:
            case PF_A2B10G10R10:
                return GL_RGBA;
            default:
                return 0;
        }
    }
	//-----------------------------------------------------------------------------
    GLenum GLES2PixelUtil::getGLOriginDataType(PixelFormat mFormat)
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
            case PF_FLOAT16_R:
            case PF_FLOAT16_GR:
            case PF_FLOAT16_RGB:
            case PF_FLOAT16_RGBA:
#if GL_OES_texture_half_float
                return GL_HALF_FLOAT_OES;
#else
                return 0;
#endif
#if GL_EXT_texture_rg
            case PF_R8:
            case PF_RG8:
                return GL_UNSIGNED_BYTE;
#endif
            case PF_FLOAT32_R:
            case PF_FLOAT32_GR:
            case PF_FLOAT32_RGB:
            case PF_FLOAT32_RGBA:
#if GL_OES_texture_float
                return GL_FLOAT;
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
            case PF_R8G8B8:
            case PF_B8G8R8:
            case PF_FLOAT16_RGB:
            case PF_FLOAT32_RGB:
                return GL_RGB;
			case PF_X8B8G8R8:
			case PF_X8R8G8B8:
            case PF_A8R8G8B8:
            case PF_A8B8G8R8:
            case PF_B8G8R8A8:
            case PF_FLOAT16_RGBA:
            case PF_FLOAT32_RGBA:
                return GL_RGBA;

#if GL_EXT_texture_rg
            case PF_FLOAT16_R:
            case PF_FLOAT32_R:
            case PF_R8:
                return GL_RED_EXT;
            case PF_FLOAT16_GR:
            case PF_FLOAT32_GR:
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
	//-----------------------------------------------------------------------------
    PixelFormat GLES2PixelUtil::getClosestOGREFormat(GLenum fmt, GLenum dataType)
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

#if GL_EXT_texture_rg
            case GL_R8_EXT:
                return PF_R8;
            case GL_RG8_EXT:
                return PF_RG8;
#endif
            default:
                return PF_A8R8G8B8;
        };
    }
	//-----------------------------------------------------------------------------
    size_t GLES2PixelUtil::getMaxMipmaps(size_t width, size_t height, size_t depth,
                                      PixelFormat format)
    {
		size_t count = 0;
        if((width > 0) && (height > 0))
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
	//-----------------------------------------------------------------------------
    // TODO: Remove
    size_t GLES2PixelUtil::optionalPO2(size_t value)
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
}
