/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#include "OgreStableHeaders.h"
#include "OgrePixelFormat.h"
#include "OgreBitwise.h"
#include "OgreColourValue.h"
#include "OgreException.h"


namespace {
#include "OgrePixelConversions.h"
};

namespace Ogre {

    //-----------------------------------------------------------------------
    /**
    * A record that describes a pixel format in detail.
    */
    struct PixelFormatDescription {
        /* Name of the format, as in the enum */
        const char *name;
        /* Number of bytes one element (colour value) takes. */
        unsigned char elemBytes;
        /* Pixel format flags, see enum PixelFormatFlags for the bit field
        * definitions
        */
        uint32 flags;
        /** Component type
         */
        PixelComponentType componentType;
        /** Component count
         */
        unsigned char componentCount;
        /* Number of bits for red(or luminance), green, blue, alpha
        */
        unsigned char rbits,gbits,bbits,abits; /*, ibits, dbits, ... */

        /* Masks and shifts as used by packers/unpackers */
        uint32 rmask, gmask, bmask, amask;
        unsigned char rshift, gshift, bshift, ashift;
    };
    //-----------------------------------------------------------------------
    /** Pixel format database */
    PixelFormatDescription _pixelFormats[PF_COUNT] = {
	//-----------------------------------------------------------------------
        {"PF_UNKNOWN",
            /* Bytes per element */
            0,
            /* Flags */
            0,
            /* Component type and count */
            PCT_BYTE, 0,
            /* rbits, gbits, bbits, abits */
            0, 0, 0, 0,
            /* Masks and shifts */
            0, 0, 0, 0, 0, 0, 0, 0
        },
	//-----------------------------------------------------------------------
        {"PF_L8",
        /* Bytes per element */
        1,
        /* Flags */
        PFF_LUMINANCE | PFF_NATIVEENDIAN,
        /* Component type and count */
        PCT_BYTE, 1,
        /* rbits, gbits, bbits, abits */
        8, 0, 0, 0,
        /* Masks and shifts */
        0xFF, 0, 0, 0, 0, 0, 0, 0
        },
	//-----------------------------------------------------------------------
        {"PF_L16",
        /* Bytes per element */
        2,
        /* Flags */
        PFF_LUMINANCE | PFF_NATIVEENDIAN,
        /* Component type and count */
        PCT_SHORT, 1,
        /* rbits, gbits, bbits, abits */
        16, 0, 0, 0,
        /* Masks and shifts */
        0xFFFF, 0, 0, 0, 0, 0, 0, 0
        },
	//-----------------------------------------------------------------------
        {"PF_A8",
        /* Bytes per element */
        1,
        /* Flags */
        PFF_HASALPHA | PFF_NATIVEENDIAN,
        /* Component type and count */
        PCT_BYTE, 1,
        /* rbits, gbits, bbits, abits */
        0, 0, 0, 8,
        /* Masks and shifts */
        0, 0, 0, 0xFF, 0, 0, 0, 0
        },
	//-----------------------------------------------------------------------
        {"PF_A4L4",
        /* Bytes per element */
        1,
        /* Flags */
        PFF_HASALPHA | PFF_LUMINANCE | PFF_NATIVEENDIAN,
        /* Component type and count */
        PCT_BYTE, 2,
        /* rbits, gbits, bbits, abits */
        4, 0, 0, 4,
        /* Masks and shifts */
        0x0F, 0, 0, 0xF0, 0, 0, 0, 4
        },
	//-----------------------------------------------------------------------
        {"PF_BYTE_LA",
        /* Bytes per element */
        2,
        /* Flags */
        PFF_HASALPHA | PFF_LUMINANCE,
        /* Component type and count */
        PCT_BYTE, 2,
        /* rbits, gbits, bbits, abits */
        8, 0, 0, 8,
        /* Masks and shifts */
        0,0,0,0,0,0,0,0
        },
	//-----------------------------------------------------------------------
        {"PF_R5G6B5",
        /* Bytes per element */
        2,
        /* Flags */
        PFF_NATIVEENDIAN,
        /* Component type and count */
        PCT_BYTE, 3,
        /* rbits, gbits, bbits, abits */
        5, 6, 5, 0,
        /* Masks and shifts */
        0xF800, 0x07E0, 0x001F, 0,
        11, 5, 0, 0
        },
	//-----------------------------------------------------------------------
		{"PF_B5G6R5",
        /* Bytes per element */
        2,
        /* Flags */
        PFF_NATIVEENDIAN,
        /* Component type and count */
        PCT_BYTE, 3,
        /* rbits, gbits, bbits, abits */
        5, 6, 5, 0,
        /* Masks and shifts */
        0x001F, 0x07E0, 0xF800, 0,
        0, 5, 11, 0
        },
	//-----------------------------------------------------------------------
        {"PF_A4R4G4B4",
        /* Bytes per element */
        2,
        /* Flags */
        PFF_HASALPHA | PFF_NATIVEENDIAN,
        /* Component type and count */
        PCT_BYTE, 4,
        /* rbits, gbits, bbits, abits */
        4, 4, 4, 4,
        /* Masks and shifts */
        0x0F00, 0x00F0, 0x000F, 0xF000,
        8, 4, 0, 12
        },
	//-----------------------------------------------------------------------
        {"PF_A1R5G5B5",
        /* Bytes per element */
        2,
        /* Flags */
        PFF_HASALPHA | PFF_NATIVEENDIAN,
        /* Component type and count */
        PCT_BYTE, 4,
        /* rbits, gbits, bbits, abits */
        5, 5, 5, 1,
        /* Masks and shifts */
        0x7C00, 0x03E0, 0x001F, 0x8000,
        10, 5, 0, 15,
        },
	//-----------------------------------------------------------------------
        {"PF_R8G8B8",
        /* Bytes per element */
        3,  // 24 bit integer -- special
        /* Flags */
        PFF_NATIVEENDIAN,
        /* Component type and count */
        PCT_BYTE, 3,
        /* rbits, gbits, bbits, abits */
        8, 8, 8, 0,
        /* Masks and shifts */
        0xFF0000, 0x00FF00, 0x0000FF, 0,
        16, 8, 0, 0
        },
	//-----------------------------------------------------------------------
        {"PF_B8G8R8",
        /* Bytes per element */
        3,  // 24 bit integer -- special
        /* Flags */
        PFF_NATIVEENDIAN,
        /* Component type and count */
        PCT_BYTE, 3,
        /* rbits, gbits, bbits, abits */
        8, 8, 8, 0,
        /* Masks and shifts */
        0x0000FF, 0x00FF00, 0xFF0000, 0,
        0, 8, 16, 0
        },
	//-----------------------------------------------------------------------
        {"PF_A8R8G8B8",
        /* Bytes per element */
        4,
        /* Flags */
        PFF_HASALPHA | PFF_NATIVEENDIAN,
        /* Component type and count */
        PCT_BYTE, 4,
        /* rbits, gbits, bbits, abits */
        8, 8, 8, 8,
        /* Masks and shifts */
        0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000,
        16, 8, 0, 24
        },
	//-----------------------------------------------------------------------
        {"PF_A8B8G8R8",
        /* Bytes per element */
        4,
        /* Flags */
        PFF_HASALPHA | PFF_NATIVEENDIAN,
        /* Component type and count */
        PCT_BYTE, 4,
        /* rbits, gbits, bbits, abits */
        8, 8, 8, 8,
        /* Masks and shifts */
        0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000,
        0, 8, 16, 24,
        },
	//-----------------------------------------------------------------------
        {"PF_B8G8R8A8",
        /* Bytes per element */
        4,
        /* Flags */
        PFF_HASALPHA | PFF_NATIVEENDIAN,
        /* Component type and count */
        PCT_BYTE, 4,
        /* rbits, gbits, bbits, abits */
        8, 8, 8, 8,
        /* Masks and shifts */
        0x0000FF00, 0x00FF0000, 0xFF000000, 0x000000FF,
        8, 16, 24, 0
        },
	//-----------------------------------------------------------------------
        {"PF_A2R10G10B10",
        /* Bytes per element */
        4,
        /* Flags */
        PFF_HASALPHA | PFF_NATIVEENDIAN,
        /* Component type and count */
        PCT_BYTE, 4,
        /* rbits, gbits, bbits, abits */
        10, 10, 10, 2,
        /* Masks and shifts */
        0x3FF00000, 0x000FFC00, 0x000003FF, 0xC0000000,
        20, 10, 0, 30
        },
	//-----------------------------------------------------------------------
        {"PF_A2B10G10R10",
        /* Bytes per element */
        4,
        /* Flags */
        PFF_HASALPHA | PFF_NATIVEENDIAN,
        /* Component type and count */
        PCT_BYTE, 4,
        /* rbits, gbits, bbits, abits */
        10, 10, 10, 2,
        /* Masks and shifts */
        0x000003FF, 0x000FFC00, 0x3FF00000, 0xC0000000,
        0, 10, 20, 30
        },
	//-----------------------------------------------------------------------
        {"PF_DXT1",
        /* Bytes per element */
        0,
        /* Flags */
        PFF_COMPRESSED | PFF_HASALPHA,
        /* Component type and count */
        PCT_BYTE, 3, // No alpha
        /* rbits, gbits, bbits, abits */
        0, 0, 0, 0,
        /* Masks and shifts */
        0, 0, 0, 0, 0, 0, 0, 0
        },
	//-----------------------------------------------------------------------
        {"PF_DXT2",
        /* Bytes per element */
        0,
        /* Flags */
        PFF_COMPRESSED | PFF_HASALPHA,
        /* Component type and count */
        PCT_BYTE, 4,
        /* rbits, gbits, bbits, abits */
        0, 0, 0, 0,
        /* Masks and shifts */
        0, 0, 0, 0, 0, 0, 0, 0
        },
	//-----------------------------------------------------------------------
        {"PF_DXT3",
        /* Bytes per element */
        0,
        /* Flags */
        PFF_COMPRESSED | PFF_HASALPHA,
        /* Component type and count */
        PCT_BYTE, 4,
        /* rbits, gbits, bbits, abits */
        0, 0, 0, 0,
        /* Masks and shifts */
        0, 0, 0, 0, 0, 0, 0, 0
        },
	//-----------------------------------------------------------------------
        {"PF_DXT4",
        /* Bytes per element */
        0,
        /* Flags */
        PFF_COMPRESSED | PFF_HASALPHA,
        /* Component type and count */
        PCT_BYTE, 4,
        /* rbits, gbits, bbits, abits */
        0, 0, 0, 0,
        /* Masks and shifts */
        0, 0, 0, 0, 0, 0, 0, 0
        },
	//-----------------------------------------------------------------------
        {"PF_DXT5",
        /* Bytes per element */
        0,
        /* Flags */
        PFF_COMPRESSED | PFF_HASALPHA,
        /* Component type and count */
        PCT_BYTE, 4,
        /* rbits, gbits, bbits, abits */
        0, 0, 0, 0,
        /* Masks and shifts */
        0, 0, 0, 0, 0, 0, 0, 0
        },
	//-----------------------------------------------------------------------
        {"PF_FLOAT16_RGB",
        /* Bytes per element */
        6,
        /* Flags */
        PFF_FLOAT,
        /* Component type and count */
        PCT_FLOAT16, 3,
        /* rbits, gbits, bbits, abits */
        16, 16, 16, 0,
        /* Masks and shifts */
        0, 0, 0, 0, 0, 0, 0, 0
        },
	//-----------------------------------------------------------------------
        {"PF_FLOAT16_RGBA",
        /* Bytes per element */
        8,
        /* Flags */
        PFF_FLOAT | PFF_HASALPHA,
        /* Component type and count */
        PCT_FLOAT16, 4,
        /* rbits, gbits, bbits, abits */
        16, 16, 16, 16,
        /* Masks and shifts */
        0, 0, 0, 0, 0, 0, 0, 0
        },
	//-----------------------------------------------------------------------
        {"PF_FLOAT32_RGB",
        /* Bytes per element */
        12,
        /* Flags */
        PFF_FLOAT,
        /* Component type and count */
        PCT_FLOAT32, 3,
        /* rbits, gbits, bbits, abits */
        32, 32, 32, 0,
        /* Masks and shifts */
        0, 0, 0, 0, 0, 0, 0, 0
        },
	//-----------------------------------------------------------------------
        {"PF_FLOAT32_RGBA",
        /* Bytes per element */
        16,
        /* Flags */
        PFF_FLOAT | PFF_HASALPHA,
        /* Component type and count */
        PCT_FLOAT32, 4,
        /* rbits, gbits, bbits, abits */
        32, 32, 32, 32,
        /* Masks and shifts */
        0, 0, 0, 0, 0, 0, 0, 0
        },
	//-----------------------------------------------------------------------
        {"PF_X8R8G8B8",
        /* Bytes per element */
        4,
        /* Flags */
        PFF_NATIVEENDIAN,
        /* Component type and count */
        PCT_BYTE, 3,
        /* rbits, gbits, bbits, abits */
        8, 8, 8, 0,
        /* Masks and shifts */
        0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000,
        16, 8, 0, 24
        },
	//-----------------------------------------------------------------------
        {"PF_X8B8G8R8",
        /* Bytes per element */
        4,
        /* Flags */
        PFF_NATIVEENDIAN,
        /* Component type and count */
        PCT_BYTE, 3,
        /* rbits, gbits, bbits, abits */
        8, 8, 8, 0,
        /* Masks and shifts */
        0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000,
        0, 8, 16, 24
        },
	//-----------------------------------------------------------------------
        {"PF_R8G8B8A8",
        /* Bytes per element */
        4,
        /* Flags */
        PFF_HASALPHA | PFF_NATIVEENDIAN,
        /* Component type and count */
        PCT_BYTE, 4,
        /* rbits, gbits, bbits, abits */
        8, 8, 8, 8,
        /* Masks and shifts */
        0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF,
        24, 16, 8, 0
        },
	//-----------------------------------------------------------------------
		{"PF_DEPTH",
        /* Bytes per element */
        4,
        /* Flags */
        PFF_DEPTH,
        /* Component type and count */
        PCT_FLOAT32, 1, // ?
        /* rbits, gbits, bbits, abits */
        0, 0, 0, 0,
        /* Masks and shifts */
		0, 0, 0, 0, 0, 0, 0, 0
        },
	//-----------------------------------------------------------------------
		{"PF_SHORT_RGBA",
		/* Bytes per element */
        8,
        /* Flags */
        PFF_HASALPHA,
        /* Component type and count */
        PCT_SHORT, 4,
        /* rbits, gbits, bbits, abits */
        16, 16, 16, 16,
        /* Masks and shifts */
		0, 0, 0, 0, 0, 0, 0, 0
        },
	//-----------------------------------------------------------------------
        {"PF_R3G3B2",
        /* Bytes per element */
        1,
        /* Flags */
        PFF_NATIVEENDIAN,
        /* Component type and count */
        PCT_BYTE, 3,
        /* rbits, gbits, bbits, abits */
        3, 3, 2, 0,
        /* Masks and shifts */
        0xE0, 0x1C, 0x03, 0,
        5, 2, 0, 0
        },
	//-----------------------------------------------------------------------
        {"PF_FLOAT16_R",
        /* Bytes per element */
        2,
        /* Flags */
        PFF_FLOAT,
        /* Component type and count */
        PCT_FLOAT16, 1,
        /* rbits, gbits, bbits, abits */
        16, 0, 0, 0,
        /* Masks and shifts */
        0, 0, 0, 0, 0, 0, 0, 0
        },
	//-----------------------------------------------------------------------
        {"PF_FLOAT32_R",
        /* Bytes per element */
        4,
        /* Flags */
        PFF_FLOAT,
        /* Component type and count */
        PCT_FLOAT32, 1,
        /* rbits, gbits, bbits, abits */
        32, 0, 0, 0,
        /* Masks and shifts */
        0, 0, 0, 0, 0, 0, 0, 0
        },
	//-----------------------------------------------------------------------
        {"PF_SHORT_GR",
        /* Bytes per element */
        4,
        /* Flags */
        PFF_NATIVEENDIAN,
        /* Component type and count */
        PCT_SHORT, 2,
        /* rbits, gbits, bbits, abits */
        16, 16, 0, 0,
        /* Masks and shifts */
        0x0000FFFF, 0xFFFF0000, 0, 0, 
		0, 16, 0, 0
        },
	//-----------------------------------------------------------------------
        {"PF_FLOAT16_GR",
        /* Bytes per element */
        4,
        /* Flags */
        PFF_FLOAT,
        /* Component type and count */
        PCT_FLOAT16, 2,
        /* rbits, gbits, bbits, abits */
        16, 16, 0, 0,
        /* Masks and shifts */
        0, 0, 0, 0, 0, 0, 0, 0
        },
	//-----------------------------------------------------------------------
        {"PF_FLOAT32_GR",
        /* Bytes per element */
        8,
        /* Flags */
        PFF_FLOAT,
        /* Component type and count */
        PCT_FLOAT32, 2,
        /* rbits, gbits, bbits, abits */
        32, 32, 0, 0,
        /* Masks and shifts */
        0, 0, 0, 0, 0, 0, 0, 0
        },
	//-----------------------------------------------------------------------
		{"PF_SHORT_RGB",
		/* Bytes per element */
        6,
        /* Flags */
        0,
        /* Component type and count */
        PCT_SHORT, 3,
        /* rbits, gbits, bbits, abits */
        16, 16, 16, 0,
        /* Masks and shifts */
		0, 0, 0, 0, 0, 0, 0, 0
        },
    };
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
		PixelBox rval(def, format, ((uint8*)data)
			+ ((def.left-left)*elemSize)
			+ ((def.top-top)*rowPitch*elemSize)
			+ ((def.front-front)*slicePitch*elemSize)
		);

		rval.rowPitch = rowPitch;
		rval.slicePitch = slicePitch;
		rval.format = format;

		return rval;
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
	size_t PixelUtil::getMemorySize(size_t width, size_t height, size_t depth, PixelFormat format)
	{
		if(isCompressed(format))
		{
			switch(format)
			{
				// DXT formats work by dividing the image into 4x4 blocks, then encoding each
				// 4x4 block with a certain number of bytes. DXT can only be used on 2D images.
				case PF_DXT1:
					assert(depth == 1);
					return ((width+3)/4)*((height+3)/4)*8;
				case PF_DXT2:
				case PF_DXT3:
				case PF_DXT4:
				case PF_DXT5:
					assert(depth == 1);
					return ((width+3)/4)*((height+3)/4)*16;
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
	void PixelUtil::getBitMasks(PixelFormat format, uint32 rgba[4])
    {
        const PixelFormatDescription &des = getDescriptionFor(format);
        rgba[0] = des.rmask;
        rgba[1] = des.gmask;
        rgba[2] = des.bmask;
        rgba[3] = des.amask;
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
        typedef std::multimap<String::size_type, String> FormatNameMap;
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
				((uint16*)dest)[0] = Bitwise::floatToFixed(r, 16);
                ((uint16*)dest)[1] = Bitwise::floatToFixed(g, 16);
                ((uint16*)dest)[2] = Bitwise::floatToFixed(b, 16);
                break;
			case PF_SHORT_RGBA:
				((uint16*)dest)[0] = Bitwise::floatToFixed(r, 16);
                ((uint16*)dest)[1] = Bitwise::floatToFixed(g, 16);
                ((uint16*)dest)[2] = Bitwise::floatToFixed(b, 16);
                ((uint16*)dest)[3] = Bitwise::floatToFixed(a, 16);
				break;
			case PF_BYTE_LA:
				((uint8*)dest)[0] = Bitwise::floatToFixed(r, 8);
                ((uint8*)dest)[1] = Bitwise::floatToFixed(a, 8);
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
                *r = *g = *b = Bitwise::fixedToFixed(
                    (value & des.rmask)>>des.rshift, des.rbits, 8);
            }
            else
            {
                *r = Bitwise::fixedToFixed((value & des.rmask)>>des.rshift, des.rbits, 8);
                *g = Bitwise::fixedToFixed((value & des.gmask)>>des.gshift, des.gbits, 8);
                *b = Bitwise::fixedToFixed((value & des.bmask)>>des.bshift, des.bbits, 8);
            }
            if(des.flags & PFF_HASALPHA)
            {
                *a = Bitwise::fixedToFixed((value & des.amask)>>des.ashift, des.abits, 8);
            }
            else
            {
                *a = 255; // No alpha, default a component to full
            }
        } else {
            // Do the operation with the more generic floating point
            float rr, gg, bb, aa;
            unpackColour(&rr,&gg,&bb,&aa, pf, src);
            *r = Bitwise::floatToFixed(rr, 8);
            *g = Bitwise::floatToFixed(gg, 8);
            *b = Bitwise::floatToFixed(bb, 8);
            *a = Bitwise::floatToFixed(aa, 8);
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
                *r = *g = *b = ((float*)src)[0];
                *a = 1.0f;
                break;
			case PF_FLOAT32_GR:
				*g = ((float*)src)[0];
				*r = *b = ((float*)src)[1];
				*a = 1.0f;
				break;
            case PF_FLOAT32_RGB:
                *r = ((float*)src)[0];
                *g = ((float*)src)[1];
                *b = ((float*)src)[2];
                *a = 1.0f;
                break;
            case PF_FLOAT32_RGBA:
                *r = ((float*)src)[0];
                *g = ((float*)src)[1];
                *b = ((float*)src)[2];
                *a = ((float*)src)[3];
                break;
            case PF_FLOAT16_R:
                *r = *g = *b = Bitwise::halfToFloat(((uint16*)src)[0]);
                *a = 1.0f;
                break;
			case PF_FLOAT16_GR:
				*g = Bitwise::halfToFloat(((uint16*)src)[0]);
				*r = *b = Bitwise::halfToFloat(((uint16*)src)[1]);
				*a = 1.0f;
				break;
            case PF_FLOAT16_RGB:
                *r = Bitwise::halfToFloat(((uint16*)src)[0]);
                *g = Bitwise::halfToFloat(((uint16*)src)[1]);
                *b = Bitwise::halfToFloat(((uint16*)src)[2]);
                *a = 1.0f;
                break;
            case PF_FLOAT16_RGBA:
                *r = Bitwise::halfToFloat(((uint16*)src)[0]);
                *g = Bitwise::halfToFloat(((uint16*)src)[1]);
                *b = Bitwise::halfToFloat(((uint16*)src)[2]);
                *a = Bitwise::halfToFloat(((uint16*)src)[3]);
                break;
			case PF_SHORT_RGB:
				*r = Bitwise::fixedToFloat(((uint16*)src)[0], 16);
                *g = Bitwise::fixedToFloat(((uint16*)src)[1], 16);
				*b = Bitwise::fixedToFloat(((uint16*)src)[2], 16);
				*a = 1.0f;
				break;
			case PF_SHORT_RGBA:
				*r = Bitwise::fixedToFloat(((uint16*)src)[0], 16);
                *g = Bitwise::fixedToFloat(((uint16*)src)[1], 16);
				*b = Bitwise::fixedToFloat(((uint16*)src)[2], 16);
				*a = Bitwise::fixedToFloat(((uint16*)src)[3], 16);
				break;
			case PF_BYTE_LA:
				*r = *g = *b = Bitwise::fixedToFloat(((uint8*)src)[0], 8);
				*a = Bitwise::fixedToFloat(((uint8*)src)[1], 8);
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
        /*uint8 *srcptr = static_cast<uint8*>(src.data)
            + (src.left + src.top * src.rowPitch + src.front * src.slicePitch) * srcPixelSize;
        uint8 *dstptr = static_cast<uint8*>(dst.data)
            + (dst.left + dst.top * dst.rowPitch + dst.front * dst.slicePitch) * dstPixelSize;
		*/
		uint8 *srcptr = static_cast<uint8*>(src.data), *dstptr = static_cast<uint8*>(dst.data);

        // Calculate pitches+skips in bytes
        const size_t srcRowSkipBytes = src.getRowSkip()*srcPixelSize;
        const size_t srcSliceSkipBytes = src.getSliceSkip()*srcPixelSize;
        const size_t dstRowSkipBytes = dst.getRowSkip()*dstPixelSize;
        const size_t dstSliceSkipBytes = dst.getSliceSkip()*dstPixelSize;

        // The brute force fallback
        float r,g,b,a;
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

}
