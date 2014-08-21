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
#include "OgreStableHeaders.h"
#include "OgrePixelFormat.h"
#include "OgreBitwise.h"
#include "OgreColourValue.h"
#include "OgreException.h"
#include "OgrePixelFormatDescriptions.h"

namespace {
#include "OgrePixelConversions.h"
}

namespace Ogre {

    //-----------------------------------------------------------------------
	size_t PixelBox::getConsecutiveSize() const
	{
		return PixelUtil::getMemorySize(getWidth(), getHeight(), getDepth(), format);
	}
	PixelBox PixelBox::getSubVolume(const Box &def) const
	{
		if(PixelUtil::isCompressed(format))
		{
			if(def.left == left && def.top == top && def.front == front &&
			   def.right == right && def.bottom == bottom && def.back == back)
			{
				// Entire buffer is being queried
				return *this;
			}
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Cannot return subvolume of compressed PixelBuffer", "PixelBox::getSubVolume");
		}
		if(!contains(def))
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Bounds out of range", "PixelBox::getSubVolume");

		const size_t elemSize = PixelUtil::getNumElemBytes(format);
		// Calculate new data origin
		// Notice how we do not propagate left/top/front from the incoming box, since
		// the returned pointer is already offset
		PixelBox rval(def.getWidth(), def.getHeight(), def.getDepth(), format, 
			((uint8*)data) + ((def.left-left)*elemSize)
			+ ((def.top-top)*rowPitch*elemSize)
			+ ((def.front-front)*slicePitch*elemSize)
		);

		rval.rowPitch = rowPitch;
		rval.slicePitch = slicePitch;
		rval.format = format;

		return rval;
	}
    void* PixelBox::getTopLeftFrontPixelPtr() const
    {
        return (uint8*)data + (left + top * rowPitch + front * slicePitch) * PixelUtil::getNumElemBytes(format);
    }
    //-----------------------------------------------------------------------
    /**
    * Directly get the description record for provided pixel format. For debug builds,
    * this checks the bounds of fmt with an assertion.
    */
    static inline const PixelFormatDescription &getDescriptionFor(const PixelFormat fmt)
    {
        const int ord = (int)fmt;
        assert(ord>=0 && ord<PF_COUNT);

        return _pixelFormats[ord];
    }
    //-----------------------------------------------------------------------
    size_t PixelUtil::getNumElemBytes( PixelFormat format )
    {
        return getDescriptionFor(format).elemBytes;
    }
	//-----------------------------------------------------------------------
	size_t PixelUtil::getMemorySize(uint32 width, uint32 height, uint32 depth, PixelFormat format)
	{
		if(isCompressed(format))
		{
			switch(format)
			{
				// DXT formats work by dividing the image into 4x4 blocks, then encoding each
				// 4x4 block with a certain number of bytes. 
				case PF_DXT1:
					return ((width+3)/4)*((height+3)/4)*8 * depth;
				case PF_DXT2:
				case PF_DXT3:
				case PF_DXT4:
				case PF_DXT5:
					return ((width+3)/4)*((height+3)/4)*16 * depth;
                case PF_BC4_SNORM:
                case PF_BC4_UNORM:
                    return ((width+3)/4)*((height+3)/4)*8;
                case PF_BC5_SNORM:
                case PF_BC5_UNORM:
                case PF_BC6H_SF16:
                case PF_BC6H_UF16:
                case PF_BC7_UNORM:
                case PF_BC7_UNORM_SRGB:
                    return ((width+3)/4)*((height+3)/4)*16;

                // Size calculations from the PVRTC OpenGL extension spec
                // http://www.khronos.org/registry/gles/extensions/IMG/IMG_texture_compression_pvrtc.txt
                // Basically, 32 bytes is the minimum texture size.  Smaller textures are padded up to 32 bytes
                case PF_PVRTC_RGB2:
                case PF_PVRTC_RGBA2:
                case PF_PVRTC2_2BPP:
                    return (std::max((int)width, 16) * std::max((int)height, 8) * 2 + 7) / 8;
                case PF_PVRTC_RGB4:
                case PF_PVRTC_RGBA4:
                case PF_PVRTC2_4BPP:
                    return (std::max((int)width, 8) * std::max((int)height, 8) * 4 + 7) / 8;

                case PF_ETC1_RGB8:
                case PF_ETC2_RGB8:
                case PF_ETC2_RGBA8:
                case PF_ETC2_RGB8A1:
                    return ((width * height) >> 1);
                case PF_ATC_RGB:
                    return ((width + 3) / 4) * ((height + 3) / 4) * 8;
                case PF_ATC_RGBA_EXPLICIT_ALPHA:
                case PF_ATC_RGBA_INTERPOLATED_ALPHA:
                    return ((width + 3) / 4) * ((height + 3) / 4) * 16;

				default:
				OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Invalid compressed pixel format",
					"PixelUtil::getMemorySize");
			}
		}
		else
		{
			return width*height*depth*getNumElemBytes(format);
		}
	}
    //-----------------------------------------------------------------------
    size_t PixelUtil::getNumElemBits( PixelFormat format )
    {
        return getDescriptionFor(format).elemBytes * 8;
    }
    //-----------------------------------------------------------------------
    unsigned int PixelUtil::getFlags( PixelFormat format )
    {
        return getDescriptionFor(format).flags;
    }
    //-----------------------------------------------------------------------
    bool PixelUtil::hasAlpha(PixelFormat format)
    {
        return (PixelUtil::getFlags(format) & PFF_HASALPHA) > 0;
    }
    //-----------------------------------------------------------------------
    bool PixelUtil::isFloatingPoint(PixelFormat format)
    {
        return (PixelUtil::getFlags(format) & PFF_FLOAT) > 0;
    }
    //-----------------------------------------------------------------------
    bool PixelUtil::isInteger(PixelFormat format)
    {
        return (PixelUtil::getFlags(format) & PFF_INTEGER) > 0;
    }
    //-----------------------------------------------------------------------
    bool PixelUtil::isCompressed(PixelFormat format)
    {
        return (PixelUtil::getFlags(format) & PFF_COMPRESSED) > 0;
    }
    //-----------------------------------------------------------------------
    bool PixelUtil::isDepth(PixelFormat format)
    {
        return (PixelUtil::getFlags(format) & PFF_DEPTH) > 0;
    }
    //-----------------------------------------------------------------------
    bool PixelUtil::isNativeEndian(PixelFormat format)
    {
        return (PixelUtil::getFlags(format) & PFF_NATIVEENDIAN) > 0;
    }
    //-----------------------------------------------------------------------
    bool PixelUtil::isLuminance(PixelFormat format)
    {
        return (PixelUtil::getFlags(format) & PFF_LUMINANCE) > 0;
    }
    //-----------------------------------------------------------------------
	bool PixelUtil::isValidExtent(size_t width, size_t height, size_t depth, PixelFormat format)
	{
		if(isCompressed(format))
		{
			switch(format)
			{
				case PF_DXT1:
				case PF_DXT2:
				case PF_DXT3:
				case PF_DXT4:
				case PF_DXT5:
                case PF_BC4_SNORM:
                case PF_BC4_UNORM:
                case PF_BC5_SNORM:
                case PF_BC5_UNORM:
                case PF_BC6H_SF16:
                case PF_BC6H_UF16:
                case PF_BC7_UNORM:
                case PF_BC7_UNORM_SRGB:
					return ((width&3)==0 && (height&3)==0 && depth==1);
				default:
					return true;
			}
		}
		else
		{
			return true;
		}
	}
	//-----------------------------------------------------------------------
    void PixelUtil::getBitDepths(PixelFormat format, int rgba[4])
    {
        const PixelFormatDescription &des = getDescriptionFor(format);
        rgba[0] = des.rbits;
        rgba[1] = des.gbits;
        rgba[2] = des.bbits;
        rgba[3] = des.abits;
    }
	//-----------------------------------------------------------------------
	void PixelUtil::getBitMasks(PixelFormat format, uint64 rgba[4])
    {
        const PixelFormatDescription &des = getDescriptionFor(format);
        rgba[0] = des.rmask;
        rgba[1] = des.gmask;
        rgba[2] = des.bmask;
        rgba[3] = des.amask;
    }
	//---------------------------------------------------------------------
	void PixelUtil::getBitShifts(PixelFormat format, unsigned char rgba[4])
	{
		const PixelFormatDescription &des = getDescriptionFor(format);
		rgba[0] = des.rshift;
		rgba[1] = des.gshift;
		rgba[2] = des.bshift;
		rgba[3] = des.ashift;
	}
    //-----------------------------------------------------------------------
    String PixelUtil::getFormatName(PixelFormat srcformat)
    {
        return getDescriptionFor(srcformat).name;
    }
    //-----------------------------------------------------------------------
    bool PixelUtil::isAccessible(PixelFormat srcformat)
    {
        if (srcformat == PF_UNKNOWN)
            return false;
        unsigned int flags = getFlags(srcformat);
        return !((flags & PFF_COMPRESSED) || (flags & PFF_DEPTH));
    }
    //-----------------------------------------------------------------------
    PixelComponentType PixelUtil::getComponentType(PixelFormat fmt)
    {
        const PixelFormatDescription &des = getDescriptionFor(fmt);
        return des.componentType;
    }
    //-----------------------------------------------------------------------
    size_t PixelUtil::getComponentCount(PixelFormat fmt)
    {
        const PixelFormatDescription &des = getDescriptionFor(fmt);
        return des.componentCount;
    }
    //-----------------------------------------------------------------------
    PixelFormat PixelUtil::getFormatFromName(const String& name, bool accessibleOnly, bool caseSensitive)
    {
        String tmp = name;
        if (!caseSensitive)
        {
            // We are stored upper-case format names.
            StringUtil::toUpperCase(tmp);
        }

        for (int i = 0; i < PF_COUNT; ++i)
        {
            PixelFormat pf = static_cast<PixelFormat>(i);
            if (!accessibleOnly || isAccessible(pf))
            {
                if (tmp == getFormatName(pf))
                    return pf;
            }
        }
        return PF_UNKNOWN;
    }
    //-----------------------------------------------------------------------
    String PixelUtil::getBNFExpressionOfPixelFormats(bool accessibleOnly)
    {
        // Collect format names sorted by length, it's required by BNF compiler
        // that similar tokens need longer ones comes first.
        typedef multimap<String::size_type, String>::type FormatNameMap;
        FormatNameMap formatNames;
        for (size_t i = 0; i < PF_COUNT; ++i)
        {
            PixelFormat pf = static_cast<PixelFormat>(i);
            if (!accessibleOnly || isAccessible(pf))
            {
                String formatName = getFormatName(pf);
                formatNames.insert(std::make_pair(formatName.length(), formatName));
            }
        }

        // Populate the BNF expression in reverse order
        String result;
        // Note: Stupid M$ VC7.1 can't dealing operator!= with FormatNameMap::const_reverse_iterator.
        for (FormatNameMap::reverse_iterator j = formatNames.rbegin(); j != formatNames.rend(); ++j)
        {
            if (!result.empty())
                result += " | ";
            result += "'" + j->second + "'";
        }

        return result;
    }
    //-----------------------------------------------------------------------
    PixelFormat PixelUtil::getFormatForBitDepths(PixelFormat fmt, ushort integerBits, ushort floatBits)
    {
        switch (integerBits)
        {
        case 16:
            switch (fmt)
            {
            case PF_R8G8B8:
            case PF_X8R8G8B8:
                return PF_R5G6B5;

            case PF_B8G8R8:
            case PF_X8B8G8R8:
                return PF_B5G6R5;

            case PF_A8R8G8B8:
            case PF_R8G8B8A8:
            case PF_A8B8G8R8:
            case PF_B8G8R8A8:
                return PF_A4R4G4B4;

            case PF_A2R10G10B10:
            case PF_A2B10G10R10:
                return PF_A1R5G5B5;

            default:
                // use original image format
                break;
            }
            break;

        case 32:
            switch (fmt)
            {
            case PF_R5G6B5:
                return PF_X8R8G8B8;

            case PF_B5G6R5:
                return PF_X8B8G8R8;

            case PF_A4R4G4B4:
                return PF_A8R8G8B8;

            case PF_A1R5G5B5:
                return PF_A2R10G10B10;

            default:
                // use original image format
                break;
            }
            break;

        default:
            // use original image format
            break;
        }

        switch (floatBits)
        {
        case 16:
            switch (fmt)
            {
            case PF_FLOAT32_R:
                return PF_FLOAT16_R;

            case PF_FLOAT32_RGB:
                return PF_FLOAT16_RGB;

            case PF_FLOAT32_RGBA:
                return PF_FLOAT16_RGBA;

            default:
                // use original image format
                break;
            }
            break;

        case 32:
            switch (fmt)
            {
            case PF_FLOAT16_R:
                return PF_FLOAT32_R;

            case PF_FLOAT16_RGB:
                return PF_FLOAT32_RGB;

            case PF_FLOAT16_RGBA:
                return PF_FLOAT32_RGBA;

            default:
                // use original image format
                break;
            }
            break;

        default:
            // use original image format
            break;
        }

        return fmt;
    }
    //-----------------------------------------------------------------------
    /*************************************************************************
    * Pixel packing/unpacking utilities
    */
    void PixelUtil::packColour(const ColourValue &colour, const PixelFormat pf,  void* dest)
    {
        packColour(colour.r, colour.g, colour.b, colour.a, pf, dest);
    }
    //-----------------------------------------------------------------------
    void PixelUtil::packColour(const uint8 r, const uint8 g, const uint8 b, const uint8 a, const PixelFormat pf,  void* dest)
    {
        const PixelFormatDescription &des = getDescriptionFor(pf);
        if(des.flags & PFF_NATIVEENDIAN) {
            // Shortcut for integer formats packing
            unsigned int value = ((Bitwise::fixedToFixed(r, 8, des.rbits)<<des.rshift) & des.rmask) |
                ((Bitwise::fixedToFixed(g, 8, des.gbits)<<des.gshift) & des.gmask) |
                ((Bitwise::fixedToFixed(b, 8, des.bbits)<<des.bshift) & des.bmask) |
                ((Bitwise::fixedToFixed(a, 8, des.abits)<<des.ashift) & des.amask);
            // And write to memory
            Bitwise::intWrite(dest, des.elemBytes, value);
        } else {
            // Convert to float
            packColour((float)r/255.0f,(float)g/255.0f,(float)b/255.0f,(float)a/255.0f, pf, dest);
        }
    }
    //-----------------------------------------------------------------------
    void PixelUtil::packColour(const float r, const float g, const float b, const float a, const PixelFormat pf,  void* dest)
    {
        // Catch-it-all here
        const PixelFormatDescription &des = getDescriptionFor(pf);
        if(des.flags & PFF_NATIVEENDIAN) {
            // Do the packing
            //std::cerr << dest << " " << r << " " << g <<  " " << b << " " << a << std::endl;
            const unsigned int value = ((Bitwise::floatToFixed(r, des.rbits)<<des.rshift) & des.rmask) |
                ((Bitwise::floatToFixed(g, des.gbits)<<des.gshift) & des.gmask) |
                ((Bitwise::floatToFixed(b, des.bbits)<<des.bshift) & des.bmask) |
                ((Bitwise::floatToFixed(a, des.abits)<<des.ashift) & des.amask);
            // And write to memory
            Bitwise::intWrite(dest, des.elemBytes, value);
        } else {
            switch(pf)
            {
            case PF_FLOAT32_R:
                ((float*)dest)[0] = r;
                break;
			case PF_FLOAT32_GR:
				((float*)dest)[0] = g;
				((float*)dest)[1] = r;
				break;
            case PF_FLOAT32_RGB:
                ((float*)dest)[0] = r;
                ((float*)dest)[1] = g;
                ((float*)dest)[2] = b;
                break;
            case PF_FLOAT32_RGBA:
                ((float*)dest)[0] = r;
                ((float*)dest)[1] = g;
                ((float*)dest)[2] = b;
                ((float*)dest)[3] = a;
                break;
            case PF_DEPTH:
            case PF_FLOAT16_R:
                ((uint16*)dest)[0] = Bitwise::floatToHalf(r);
                break;
			case PF_FLOAT16_GR:
				((uint16*)dest)[0] = Bitwise::floatToHalf(g);
				((uint16*)dest)[1] = Bitwise::floatToHalf(r);
				break;
            case PF_FLOAT16_RGB:
                ((uint16*)dest)[0] = Bitwise::floatToHalf(r);
                ((uint16*)dest)[1] = Bitwise::floatToHalf(g);
                ((uint16*)dest)[2] = Bitwise::floatToHalf(b);
                break;
            case PF_FLOAT16_RGBA:
                ((uint16*)dest)[0] = Bitwise::floatToHalf(r);
                ((uint16*)dest)[1] = Bitwise::floatToHalf(g);
                ((uint16*)dest)[2] = Bitwise::floatToHalf(b);
                ((uint16*)dest)[3] = Bitwise::floatToHalf(a);
                break;
            case PF_SHORT_RGB:
				((uint16*)dest)[0] = (uint16)Bitwise::floatToFixed(r, 16);
                ((uint16*)dest)[1] = (uint16)Bitwise::floatToFixed(g, 16);
                ((uint16*)dest)[2] = (uint16)Bitwise::floatToFixed(b, 16);
                break;
			case PF_SHORT_RGBA:
				((uint16*)dest)[0] = (uint16)Bitwise::floatToFixed(r, 16);
                ((uint16*)dest)[1] = (uint16)Bitwise::floatToFixed(g, 16);
                ((uint16*)dest)[2] = (uint16)Bitwise::floatToFixed(b, 16);
                ((uint16*)dest)[3] = (uint16)Bitwise::floatToFixed(a, 16);
				break;
			case PF_BYTE_LA:
				((uint8*)dest)[0] = (uint8)Bitwise::floatToFixed(r, 8);
                ((uint8*)dest)[1] = (uint8)Bitwise::floatToFixed(a, 8);
				break;
            default:
                // Not yet supported
                OGRE_EXCEPT(
                    Exception::ERR_NOT_IMPLEMENTED,
                    "pack to "+getFormatName(pf)+" not implemented",
                    "PixelUtil::packColour");
                break;
            }
        }
    }
    //-----------------------------------------------------------------------
    void PixelUtil::unpackColour(ColourValue *colour, PixelFormat pf,  const void* src)
    {
        unpackColour(&colour->r, &colour->g, &colour->b, &colour->a, pf, src);
    }
    //-----------------------------------------------------------------------
    void PixelUtil::unpackColour(uint8 *r, uint8 *g, uint8 *b, uint8 *a, PixelFormat pf,  const void* src)
    {
        const PixelFormatDescription &des = getDescriptionFor(pf);
        if(des.flags & PFF_NATIVEENDIAN) {
            // Shortcut for integer formats unpacking
            const unsigned int value = Bitwise::intRead(src, des.elemBytes);
            if(des.flags & PFF_LUMINANCE)
            {
                // Luminance format -- only rbits used
                *r = *g = *b = (uint8)Bitwise::fixedToFixed(
                    (value & des.rmask)>>des.rshift, des.rbits, 8);
            }
            else
            {
                *r = (uint8)Bitwise::fixedToFixed((value & des.rmask)>>des.rshift, des.rbits, 8);
                *g = (uint8)Bitwise::fixedToFixed((value & des.gmask)>>des.gshift, des.gbits, 8);
                *b = (uint8)Bitwise::fixedToFixed((value & des.bmask)>>des.bshift, des.bbits, 8);
            }
            if(des.flags & PFF_HASALPHA)
            {
                *a = (uint8)Bitwise::fixedToFixed((value & des.amask)>>des.ashift, des.abits, 8);
            }
            else
            {
                *a = 255; // No alpha, default a component to full
            }
        } else {
            // Do the operation with the more generic floating point
            float rr = 0, gg = 0, bb = 0, aa = 0;
            unpackColour(&rr,&gg,&bb,&aa, pf, src);
            *r = (uint8)Bitwise::floatToFixed(rr, 8);
            *g = (uint8)Bitwise::floatToFixed(gg, 8);
            *b = (uint8)Bitwise::floatToFixed(bb, 8);
            *a = (uint8)Bitwise::floatToFixed(aa, 8);
        }
    }
    //-----------------------------------------------------------------------
    void PixelUtil::unpackColour(float *r, float *g, float *b, float *a,
        PixelFormat pf,  const void* src)
    {
        const PixelFormatDescription &des = getDescriptionFor(pf);
        if(des.flags & PFF_NATIVEENDIAN) {
            // Shortcut for integer formats unpacking
            const unsigned int value = Bitwise::intRead(src, des.elemBytes);
            if(des.flags & PFF_LUMINANCE)
            {
                // Luminance format -- only rbits used
                *r = *g = *b = Bitwise::fixedToFloat(
                    (value & des.rmask)>>des.rshift, des.rbits);
            }
            else
            {
                *r = Bitwise::fixedToFloat((value & des.rmask)>>des.rshift, des.rbits);
                *g = Bitwise::fixedToFloat((value & des.gmask)>>des.gshift, des.gbits);
                *b = Bitwise::fixedToFloat((value & des.bmask)>>des.bshift, des.bbits);
            }
            if(des.flags & PFF_HASALPHA)
            {
                *a = Bitwise::fixedToFloat((value & des.amask)>>des.ashift, des.abits);
            }
            else
            {
                *a = 1.0f; // No alpha, default a component to full
            }
        } else {
            switch(pf)
            {
            case PF_FLOAT32_R:
                *r = *g = *b = ((const float*)src)[0];
                *a = 1.0f;
                break;
			case PF_FLOAT32_GR:
				*g = ((const float*)src)[0];
				*r = *b = ((const float*)src)[1];
				*a = 1.0f;
				break;
            case PF_FLOAT32_RGB:
                *r = ((const float*)src)[0];
                *g = ((const float*)src)[1];
                *b = ((const float*)src)[2];
                *a = 1.0f;
                break;
            case PF_FLOAT32_RGBA:
                *r = ((const float*)src)[0];
                *g = ((const float*)src)[1];
                *b = ((const float*)src)[2];
                *a = ((const float*)src)[3];
                break;
            case PF_DEPTH:
            case PF_FLOAT16_R:
                *r = *g = *b = Bitwise::halfToFloat(((const uint16*)src)[0]);
                *a = 1.0f;
                break;
            case PF_FLOAT16_GR:
                *g = Bitwise::halfToFloat(((const uint16*)src)[0]);
                *r = *b = Bitwise::halfToFloat(((const uint16*)src)[1]);
                *a = 1.0f;
                break;
            case PF_FLOAT16_RGB:
                *r = Bitwise::halfToFloat(((const uint16*)src)[0]);
                *g = Bitwise::halfToFloat(((const uint16*)src)[1]);
                *b = Bitwise::halfToFloat(((const uint16*)src)[2]);
                *a = 1.0f;
                break;
            case PF_FLOAT16_RGBA:
                *r = Bitwise::halfToFloat(((const uint16*)src)[0]);
                *g = Bitwise::halfToFloat(((const uint16*)src)[1]);
                *b = Bitwise::halfToFloat(((const uint16*)src)[2]);
                *a = Bitwise::halfToFloat(((const uint16*)src)[3]);
                break;
            case PF_SHORT_RGB:
                *r = Bitwise::fixedToFloat(((const uint16*)src)[0], 16);
                *g = Bitwise::fixedToFloat(((const uint16*)src)[1], 16);
                *b = Bitwise::fixedToFloat(((const uint16*)src)[2], 16);
                *a = 1.0f;
                break;
            case PF_SHORT_RGBA:
                *r = Bitwise::fixedToFloat(((const uint16*)src)[0], 16);
                *g = Bitwise::fixedToFloat(((const uint16*)src)[1], 16);
                *b = Bitwise::fixedToFloat(((const uint16*)src)[2], 16);
                *a = Bitwise::fixedToFloat(((const uint16*)src)[3], 16);
                break;
            case PF_BYTE_LA:
                *r = *g = *b = Bitwise::fixedToFloat(((const uint8*)src)[0], 8);
                *a = Bitwise::fixedToFloat(((const uint8*)src)[1], 8);
                break;
            default:
                // Not yet supported
                OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                    "unpack from "+getFormatName(pf)+" not implemented",
                    "PixelUtil::unpackColour");
                break;
            }
        }
    }
    //-----------------------------------------------------------------------
    /* Convert pixels from one format to another */
    void PixelUtil::bulkPixelConversion(void *srcp, PixelFormat srcFormat,
        void *destp, PixelFormat dstFormat, unsigned int count)
    {
        PixelBox src(count, 1, 1, srcFormat, srcp),
				 dst(count, 1, 1, dstFormat, destp);

        bulkPixelConversion(src, dst);
    }
    //-----------------------------------------------------------------------
    void PixelUtil::bulkPixelConversion(const PixelBox &src, const PixelBox &dst)
    {
        assert(src.getWidth() == dst.getWidth() &&
			   src.getHeight() == dst.getHeight() &&
			   src.getDepth() == dst.getDepth());

		// Check for compressed formats, we don't support decompression, compression or recoding
		if(PixelUtil::isCompressed(src.format) || PixelUtil::isCompressed(dst.format))
		{
			if(src.format == dst.format)
			{
				memcpy(dst.data, src.data, src.getConsecutiveSize());
				return;
			}
			else
			{
				OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
					"This method can not be used to compress or decompress images",
					"PixelUtil::bulkPixelConversion");
			}
		}

        // The easy case
        if(src.format == dst.format) {
            // Everything consecutive?
            if(src.isConsecutive() && dst.isConsecutive())
            {
				memcpy(dst.data, src.data, src.getConsecutiveSize());
                return;
            }

            const size_t srcPixelSize = PixelUtil::getNumElemBytes(src.format);
            const size_t dstPixelSize = PixelUtil::getNumElemBytes(dst.format);
            uint8 *srcptr = static_cast<uint8*>(src.data)
                + (src.left + src.top * src.rowPitch + src.front * src.slicePitch) * srcPixelSize;
            uint8 *dstptr = static_cast<uint8*>(dst.data)
				+ (dst.left + dst.top * dst.rowPitch + dst.front * dst.slicePitch) * dstPixelSize;

            // Calculate pitches+skips in bytes
            const size_t srcRowPitchBytes = src.rowPitch*srcPixelSize;
            //const size_t srcRowSkipBytes = src.getRowSkip()*srcPixelSize;
            const size_t srcSliceSkipBytes = src.getSliceSkip()*srcPixelSize;

            const size_t dstRowPitchBytes = dst.rowPitch*dstPixelSize;
            //const size_t dstRowSkipBytes = dst.getRowSkip()*dstPixelSize;
            const size_t dstSliceSkipBytes = dst.getSliceSkip()*dstPixelSize;

            // Otherwise, copy per row
            const size_t rowSize = src.getWidth()*srcPixelSize;
            for(size_t z=src.front; z<src.back; z++)
            {
                for(size_t y=src.top; y<src.bottom; y++)
                {
					memcpy(dstptr, srcptr, rowSize);
                    srcptr += srcRowPitchBytes;
                    dstptr += dstRowPitchBytes;
                }
                srcptr += srcSliceSkipBytes;
                dstptr += dstSliceSkipBytes;
            }
            return;
        }
		// Converting to PF_X8R8G8B8 is exactly the same as converting to
		// PF_A8R8G8B8. (same with PF_X8B8G8R8 and PF_A8B8G8R8)
		if(dst.format == PF_X8R8G8B8 || dst.format == PF_X8B8G8R8)
		{
			// Do the same conversion, with PF_A8R8G8B8, which has a lot of
			// optimized conversions
			PixelBox tempdst = dst;
			tempdst.format = dst.format==PF_X8R8G8B8?PF_A8R8G8B8:PF_A8B8G8R8;
			bulkPixelConversion(src, tempdst);
			return;
		}
		// Converting from PF_X8R8G8B8 is exactly the same as converting from
		// PF_A8R8G8B8, given that the destination format does not have alpha.
		if((src.format == PF_X8R8G8B8||src.format == PF_X8B8G8R8) && !hasAlpha(dst.format))
		{
			// Do the same conversion, with PF_A8R8G8B8, which has a lot of
			// optimized conversions
			PixelBox tempsrc = src;
			tempsrc.format = src.format==PF_X8R8G8B8?PF_A8R8G8B8:PF_A8B8G8R8;
			bulkPixelConversion(tempsrc, dst);
			return;
		}

// NB VC6 can't handle the templates required for optimised conversion, tough
#if OGRE_COMPILER != OGRE_COMPILER_MSVC || OGRE_COMP_VER >= 1300
        // Is there a specialized, inlined, conversion?
        if(doOptimizedConversion(src, dst))
        {
            // If so, good
            return;
        }
#endif

        const size_t srcPixelSize = PixelUtil::getNumElemBytes(src.format);
        const size_t dstPixelSize = PixelUtil::getNumElemBytes(dst.format);
        uint8 *srcptr = static_cast<uint8*>(src.data)
            + (src.left + src.top * src.rowPitch + src.front * src.slicePitch) * srcPixelSize;
        uint8 *dstptr = static_cast<uint8*>(dst.data)
            + (dst.left + dst.top * dst.rowPitch + dst.front * dst.slicePitch) * dstPixelSize;
		
		// Old way, not taking into account box dimensions
		//uint8 *srcptr = static_cast<uint8*>(src.data), *dstptr = static_cast<uint8*>(dst.data);

        // Calculate pitches+skips in bytes
        const size_t srcRowSkipBytes = src.getRowSkip()*srcPixelSize;
        const size_t srcSliceSkipBytes = src.getSliceSkip()*srcPixelSize;
        const size_t dstRowSkipBytes = dst.getRowSkip()*dstPixelSize;
        const size_t dstSliceSkipBytes = dst.getSliceSkip()*dstPixelSize;

        // The brute force fallback
        float r = 0, g = 0, b = 0, a = 1;
        for(size_t z=src.front; z<src.back; z++)
        {
            for(size_t y=src.top; y<src.bottom; y++)
            {
                for(size_t x=src.left; x<src.right; x++)
                {
                    unpackColour(&r, &g, &b, &a, src.format, srcptr);
                    packColour(r, g, b, a, dst.format, dstptr);
                    srcptr += srcPixelSize;
                    dstptr += dstPixelSize;
                }
                srcptr += srcRowSkipBytes;
                dstptr += dstRowSkipBytes;
            }
            srcptr += srcSliceSkipBytes;
            dstptr += dstSliceSkipBytes;
        }
    }
    //-----------------------------------------------------------------------
    void PixelUtil::bulkPixelVerticalFlip(const PixelBox &box)
    {
		// Check for compressed formats, we don't support decompression, compression or recoding
		if(PixelUtil::isCompressed(box.format))
		{
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                        "This method can not be used for compressed formats",
                        "PixelUtil::bulkPixelVerticalFlip");
		}
        
        const size_t pixelSize = PixelUtil::getNumElemBytes(box.format);
        const size_t copySize = (box.right - box.left) * pixelSize;

        // Calculate pitches in bytes
        const size_t rowPitchBytes = box.rowPitch * pixelSize;
        const size_t slicePitchBytes = box.slicePitch * pixelSize;

        uint8 *basesrcptr = static_cast<uint8*>(box.data)
            + (box.left + box.top * box.rowPitch + box.front * box.slicePitch) * pixelSize;
        uint8 *basedstptr = basesrcptr + (box.bottom - box.top - 1) * rowPitchBytes;
        uint8* tmpptr = (uint8*)OGRE_MALLOC_ALIGN(copySize, MEMCATEGORY_GENERAL, false);
        
        // swap rows
        const size_t halfRowCount = (box.bottom - box.top) >> 1;
        for(size_t z = box.front; z < box.back; z++)
        {
            uint8* srcptr = basesrcptr;
            uint8* dstptr = basedstptr;
            for(size_t y = 0; y < halfRowCount; y++)
            {
                // swap rows
                memcpy(tmpptr, dstptr, copySize);
                memcpy(dstptr, srcptr, copySize);
                memcpy(srcptr, tmpptr, copySize);
                srcptr += rowPitchBytes;
                dstptr -= rowPitchBytes;
            }
            basesrcptr += slicePitchBytes;
            basedstptr += slicePitchBytes;
        }
        
        OGRE_FREE_ALIGN(tmpptr, MEMCATEGORY_GENERAL, false);
    }

    ColourValue PixelBox::getColourAt(size_t x, size_t y, size_t z)
    {
        ColourValue cv;

        unsigned char pixelSize = PixelUtil::getNumElemBytes(format);
        size_t pixelOffset = pixelSize * (z * slicePitch + y * rowPitch + x);
        PixelUtil::unpackColour(&cv, format, (unsigned char *)data + pixelOffset);

        return cv;
    }

    void PixelBox::setColourAt(ColourValue const &cv, size_t x, size_t y, size_t z)
    {
        unsigned char pixelSize = PixelUtil::getNumElemBytes(format);
        size_t pixelOffset = pixelSize * (z * slicePitch + y * rowPitch + x);
        PixelUtil::packColour(cv, format, (unsigned char *)data + pixelOffset);
    }

}
