/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2009 Torus Knot Software Ltd

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

namespace Ogre  {
	//-----------------------------------------------------------------------------
    GLenum GLPixelUtil::getGLOriginFormat(PixelFormat mFormat)
    {
        switch(mFormat)
        {
			case PF_A8:
				return GL_ALPHA;
            case PF_L8:
                return GL_LUMINANCE;
            case PF_L16:
                return GL_LUMINANCE;
			case PF_BYTE_LA:
				return GL_LUMINANCE_ALPHA;
			case PF_R3G3B2:
				return GL_RGB;
			case PF_A1R5G5B5:
				return GL_BGRA;
			case PF_R5G6B5:
				return GL_RGB;
			case PF_B5G6R5:
				return GL_BGR;
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
				return GL_RGBA;
            case PF_A2R10G10B10:
                return GL_BGRA;
            case PF_A2B10G10R10:
                return GL_RGBA;
			case PF_FLOAT16_R:
                return GL_LUMINANCE;
			case PF_FLOAT16_GR:
				return GL_LUMINANCE_ALPHA;
            case PF_FLOAT16_RGB:
                return GL_RGB;
            case PF_FLOAT16_RGBA:
                return GL_RGBA;
			case PF_FLOAT32_R:
                return GL_LUMINANCE;
			case PF_FLOAT32_GR:
				return GL_LUMINANCE_ALPHA;
            case PF_FLOAT32_RGB:
                return GL_RGB;
            case PF_FLOAT32_RGBA:
                return GL_RGBA;
			case PF_SHORT_RGBA:
				return GL_RGBA;
            case PF_SHORT_RGB:
                return GL_RGB;
			case PF_SHORT_GR:
				return GL_LUMINANCE_ALPHA;
            case PF_DXT1:
                return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
            case PF_DXT3:
                 return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
            case PF_DXT5:
                 return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
            default:
                return 0;
        }
    }
	//----------------------------------------------------------------------------- 
    GLenum GLPixelUtil::getGLOriginDataType(PixelFormat mFormat)
    {
        switch(mFormat)
        {
			case PF_A8:
            case PF_L8:
            case PF_R8G8B8:
            case PF_B8G8R8:
			case PF_BYTE_LA:
                return GL_UNSIGNED_BYTE;
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
                return GL_HALF_FLOAT_ARB;
			case PF_FLOAT32_R:
			case PF_FLOAT32_GR:
            case PF_FLOAT32_RGB:
            case PF_FLOAT32_RGBA:
                return GL_FLOAT;
			case PF_SHORT_RGBA:
            case PF_SHORT_RGB:
			case PF_SHORT_GR:
				return GL_UNSIGNED_SHORT;
            default:
                return 0;
        }
    }
    
    GLenum GLPixelUtil::getGLInternalFormat(PixelFormat mFormat, bool hwGamma)
    {
        switch(mFormat) {
            case PF_L8:
                return GL_LUMINANCE8;
            case PF_L16:
                return GL_LUMINANCE16;
            case PF_A8:
                return GL_ALPHA8;
            case PF_A4L4:
                return GL_LUMINANCE4_ALPHA4;
			case PF_BYTE_LA:
				return GL_LUMINANCE8_ALPHA8;
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
				if (hwGamma)
					return GL_SRGB8_ALPHA8;
				else
					return GL_RGBA8;
            case PF_A2R10G10B10:
            case PF_A2B10G10R10:
                return GL_RGB10_A2;
			case PF_FLOAT16_R:
				return GL_LUMINANCE16F_ARB;
            case PF_FLOAT16_RGB:
                return GL_RGB16F_ARB;
			case PF_FLOAT16_GR: 
				return GL_LUMINANCE_ALPHA16F_ARB;
            case PF_FLOAT16_RGBA:
                return GL_RGBA16F_ARB;
			case PF_FLOAT32_R:
				return GL_LUMINANCE32F_ARB;
			case PF_FLOAT32_GR:
				return GL_LUMINANCE_ALPHA32F_ARB;
            case PF_FLOAT32_RGB:
                return GL_RGB32F_ARB;
            case PF_FLOAT32_RGBA:
                return GL_RGBA32F_ARB;
			case PF_SHORT_RGBA:
				return GL_RGBA16;
            case PF_SHORT_RGB:
                return GL_RGB16;
			case PF_SHORT_GR:
				return GL_LUMINANCE16_ALPHA16;
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
            default:
                return GL_NONE;
        }
    }

    GLenum GLPixelUtil::getClosestGLInternalFormat(PixelFormat mFormat, bool hwGamma)
    {
        GLenum format = getGLInternalFormat(mFormat, hwGamma);
        if(format==GL_NONE)
		{
			if (hwGamma)
				return GL_SRGB8;
			else
				return GL_RGBA8;
		}
        else
            return format;
    }
	
	//----------------------------------------------------------------------------- 	
	PixelFormat GLPixelUtil::getClosestOGREFormat(GLenum fmt)
	{
		switch(fmt) 
		{
		case GL_LUMINANCE8:
			return PF_L8;
		case GL_LUMINANCE16:
			return PF_L16;
		case GL_ALPHA8:
			return PF_A8;
		case GL_LUMINANCE4_ALPHA4:
			// Unsupported by GL as input format, use the byte packed format
			return PF_BYTE_LA;
		case GL_LUMINANCE8_ALPHA8:
			return PF_BYTE_LA;
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
			return PF_A2R10G10B10;
		case GL_RGBA16:
			return PF_SHORT_RGBA;
        case GL_RGB16:
            return PF_SHORT_RGB;
		case GL_LUMINANCE16_ALPHA16:
			return PF_SHORT_GR;
		case GL_LUMINANCE_FLOAT16_ATI:
			return PF_FLOAT16_R;
		case GL_LUMINANCE_ALPHA_FLOAT16_ATI:
			return PF_FLOAT16_GR;
		case GL_LUMINANCE_ALPHA_FLOAT32_ATI:
			return PF_FLOAT32_GR;
		case GL_LUMINANCE_FLOAT32_ATI:
			return PF_FLOAT32_R;
		case GL_RGB_FLOAT16_ATI: // GL_RGB16F_ARB
			return PF_FLOAT16_RGB;
		case GL_RGBA_FLOAT16_ATI:
			return PF_FLOAT16_RGBA;
		case GL_RGB_FLOAT32_ATI:
			return PF_FLOAT32_RGB;
		case GL_RGBA_FLOAT32_ATI:
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
		default:
			return PF_A8R8G8B8;
		};
	}
	//----------------------------------------------------------------------------- 

	size_t GLPixelUtil::getMaxMipmaps(size_t width, size_t height, size_t depth, PixelFormat format)
	{
		size_t count = 0;
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
		
		return count;
	}
    //-----------------------------------------------------------------------------    
    size_t GLPixelUtil::optionalPO2(size_t value)
    {
        const RenderSystemCapabilities *caps = Root::getSingleton().getRenderSystem()->getCapabilities();
        if(caps->hasCapability(RSC_NON_POWER_OF_2_TEXTURES))
            return value;
        else
            return Bitwise::firstPO2From((uint32)value);
    }   

	
};
