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
#ifndef _PixelFormat_H__
#define _PixelFormat_H__

#include "OgrePrerequisites.h"
#include "OgreCommon.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Image
    *  @{
    */
    /** The pixel format used for images, textures, and render surfaces */
    enum PixelFormat
    {
        /// Unknown pixel format.
        PF_UNKNOWN = 0,
        /// 8-bit pixel format, all bits luminance.
        PF_L8 = 1,
        PF_BYTE_L = PF_L8,
        /// 16-bit pixel format, all bits luminance.
        PF_L16 = 2,
        PF_SHORT_L = PF_L16,
        /// 8-bit pixel format, all bits alpha.
        PF_A8 = 3,
        PF_BYTE_A = PF_A8,
        /// 8-bit pixel format, 4 bits alpha, 4 bits luminance.
        PF_A4L4 = 4,
        /// 2 byte pixel format, 1 byte luminance, 1 byte alpha
        PF_BYTE_LA = 5,
        /// 16-bit pixel format, 5 bits red, 6 bits green, 5 bits blue.
        PF_R5G6B5 = 6,
        /// 16-bit pixel format, 5 bits red, 6 bits green, 5 bits blue.
        PF_B5G6R5 = 7,
        /// 8-bit pixel format, 2 bits blue, 3 bits green, 3 bits red.
        PF_R3G3B2 = 31,
        /// 16-bit pixel format, 4 bits for alpha, red, green and blue.
        PF_A4R4G4B4 = 8,
        /// 16-bit pixel format, 5 bits for blue, green, red and 1 for alpha.
        PF_A1R5G5B5 = 9,
        /// 24-bit pixel format, 8 bits for red, green and blue.
        PF_R8G8B8 = 10,
        /// 24-bit pixel format, 8 bits for blue, green and red.
        PF_B8G8R8 = 11,
        /// 32-bit pixel format, 8 bits for alpha, red, green and blue.
        PF_A8R8G8B8 = 12,
        /// 32-bit pixel format, 8 bits for blue, green, red and alpha.
        PF_A8B8G8R8 = 13,
        /// 32-bit pixel format, 8 bits for blue, green, red and alpha.
        PF_B8G8R8A8 = 14,
        /// 32-bit pixel format, 8 bits for red, green, blue and alpha.
        PF_R8G8B8A8 = 28,
        /// 32-bit pixel format, 8 bits for red, 8 bits for green, 8 bits for blue
        /// like PF_A8R8G8B8, but alpha will get discarded
        PF_X8R8G8B8 = 26,
        /// 32-bit pixel format, 8 bits for blue, 8 bits for green, 8 bits for red
        /// like PF_A8B8G8R8, but alpha will get discarded
        PF_X8B8G8R8 = 27,
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
        /// 3 byte pixel format, 1 byte for red, 1 byte for green, 1 byte for blue
        PF_BYTE_RGB = PF_R8G8B8,
        /// 3 byte pixel format, 1 byte for blue, 1 byte for green, 1 byte for red
        PF_BYTE_BGR = PF_B8G8R8,
        /// 4 byte pixel format, 1 byte for blue, 1 byte for green, 1 byte for red and one byte for alpha
        PF_BYTE_BGRA = PF_B8G8R8A8,
        /// 4 byte pixel format, 1 byte for red, 1 byte for green, 1 byte for blue, and one byte for alpha
        PF_BYTE_RGBA = PF_R8G8B8A8,
#else
        /// 3 byte pixel format, 1 byte for red, 1 byte for green, 1 byte for blue
        PF_BYTE_RGB = PF_B8G8R8,
        /// 3 byte pixel format, 1 byte for blue, 1 byte for green, 1 byte for red
        PF_BYTE_BGR = PF_R8G8B8,
        /// 4 byte pixel format, 1 byte for blue, 1 byte for green, 1 byte for red and one byte for alpha
        PF_BYTE_BGRA = PF_A8R8G8B8,
        /// 4 byte pixel format, 1 byte for red, 1 byte for green, 1 byte for blue, and one byte for alpha
        PF_BYTE_RGBA = PF_A8B8G8R8,
#endif        
        /// 32-bit pixel format, 2 bits for alpha, 10 bits for red, green and blue.
        PF_A2R10G10B10 = 15,
        /// 32-bit pixel format, 10 bits for blue, green and red, 2 bits for alpha.
        PF_A2B10G10R10 = 16,
        /// DDS (DirectDraw Surface) DXT1 format
        PF_DXT1 = 17,
        /// DDS (DirectDraw Surface) DXT2 format
        PF_DXT2 = 18,
        /// DDS (DirectDraw Surface) DXT3 format
        PF_DXT3 = 19,
        /// DDS (DirectDraw Surface) DXT4 format
        PF_DXT4 = 20,
        /// DDS (DirectDraw Surface) DXT5 format
        PF_DXT5 = 21,
        /// 16-bit pixel format, 16 bits (float) for red
        PF_FLOAT16_R = 32,
        /// 48-bit pixel format, 16 bits (float) for red, 16 bits (float) for green, 16 bits (float) for blue
        PF_FLOAT16_RGB = 22,
        /// 64-bit pixel format, 16 bits (float) for red, 16 bits (float) for green, 16 bits (float) for blue, 16 bits (float) for alpha
        PF_FLOAT16_RGBA = 23,
        // 32-bit pixel format, 32 bits (float) for red
        PF_FLOAT32_R = 33,
        /// 96-bit pixel format, 32 bits (float) for red, 32 bits (float) for green, 32 bits (float) for blue
        PF_FLOAT32_RGB = 24,
        /// 128-bit pixel format, 32 bits (float) for red, 32 bits (float) for green, 32 bits (float) for blue, 32 bits (float) for alpha
        PF_FLOAT32_RGBA = 25,
        /// 32-bit, 2-channel s10e5 floating point pixel format, 16-bit green, 16-bit red
        PF_FLOAT16_GR = 35,
        /// 64-bit, 2-channel floating point pixel format, 32-bit green, 32-bit red
        PF_FLOAT32_GR = 36,
        /// Depth texture format
        PF_DEPTH_DEPRECATED = 29,
        /// 64-bit pixel format, 16 bits for red, green, blue and alpha
        PF_SHORT_RGBA = 30,
        /// 32-bit pixel format, 16-bit green, 16-bit red
        PF_SHORT_GR = 34,
        /// 48-bit pixel format, 16 bits for red, green and blue
        PF_SHORT_RGB = 37,
        /// PVRTC (PowerVR) RGB 2 bpp
        PF_PVRTC_RGB2 = 38,
        /// PVRTC (PowerVR) RGBA 2 bpp
        PF_PVRTC_RGBA2 = 39,
        /// PVRTC (PowerVR) RGB 4 bpp
        PF_PVRTC_RGB4 = 40,
        /// PVRTC (PowerVR) RGBA 4 bpp
        PF_PVRTC_RGBA4 = 41,
        /// PVRTC (PowerVR) Version 2, 2 bpp
        PF_PVRTC2_2BPP = 42,
        /// PVRTC (PowerVR) Version 2, 4 bpp
        PF_PVRTC2_4BPP = 43,
        /// 32-bit pixel format, 11 bits (float) for red, 11 bits (float) for green, 10 bits (float) for blue
        PF_R11G11B10_FLOAT = 44,
        /// 8-bit pixel format, 8 bits red (unsigned int).
        PF_R8_UINT = 45,
        /// 16-bit pixel format, 8 bits red (unsigned int), 8 bits blue (unsigned int).
        PF_R8G8_UINT = 46,
        /// 24-bit pixel format, 8 bits red (unsigned int), 8 bits blue (unsigned int), 8 bits green (unsigned int).
        PF_R8G8B8_UINT = 47,
        /// 32-bit pixel format, 8 bits red (unsigned int), 8 bits blue (unsigned int), 8 bits green (unsigned int), 8 bits alpha (unsigned int).
        PF_R8G8B8A8_UINT = 48,
        /// 16-bit pixel format, 16 bits red (unsigned int).
        PF_R16_UINT = 49,
        /// 32-bit pixel format, 16 bits red (unsigned int), 16 bits blue (unsigned int).
        PF_R16G16_UINT = 50,
        /// 48-bit pixel format, 16 bits red (unsigned int), 16 bits blue (unsigned int), 16 bits green (unsigned int).
        PF_R16G16B16_UINT = 51,
        /// 64-bit pixel format, 16 bits red (unsigned int), 16 bits blue (unsigned int), 16 bits green (unsigned int), 16 bits alpha (unsigned int).
        PF_R16G16B16A16_UINT = 52,
        /// 32-bit pixel format, 32 bits red (unsigned int).
        PF_R32_UINT = 53,
        /// 64-bit pixel format, 32 bits red (unsigned int), 32 bits blue (unsigned int).
        PF_R32G32_UINT = 54,
        /// 96-bit pixel format, 32 bits red (unsigned int), 32 bits blue (unsigned int), 32 bits green (unsigned int).
        PF_R32G32B32_UINT = 55,
        /// 128-bit pixel format, 32 bits red (unsigned int), 32 bits blue (unsigned int), 32 bits green (unsigned int), 32 bits alpha (unsigned int).
        PF_R32G32B32A32_UINT = 56,
        /// 8-bit pixel format, 8 bits red (signed int).
        PF_R8_SINT = 57,
        /// 16-bit pixel format, 8 bits red (signed int), 8 bits blue (signed int).
        PF_R8G8_SINT = 58,
        /// 24-bit pixel format, 8 bits red (signed int), 8 bits blue (signed int), 8 bits green (signed int).
        PF_R8G8B8_SINT = 59,
        /// 32-bit pixel format, 8 bits red (signed int), 8 bits blue (signed int), 8 bits green (signed int), 8 bits alpha (signed int).
        PF_R8G8B8A8_SINT = 60,
        /// 16-bit pixel format, 16 bits red (signed int).
        PF_R16_SINT = 61,
        /// 32-bit pixel format, 16 bits red (signed int), 16 bits blue (signed int).
        PF_R16G16_SINT = 62,
        /// 48-bit pixel format, 16 bits red (signed int), 16 bits blue (signed int), 16 bits green (signed int).
        PF_R16G16B16_SINT = 63,
        /// 64-bit pixel format, 16 bits red (signed int), 16 bits blue (signed int), 16 bits green (signed int), 16 bits alpha (signed int).
        PF_R16G16B16A16_SINT = 64,
        /// 32-bit pixel format, 32 bits red (signed int).
        PF_R32_SINT = 65,
        /// 64-bit pixel format, 32 bits red (signed int), 32 bits blue (signed int).
        PF_R32G32_SINT = 66,
        /// 96-bit pixel format, 32 bits red (signed int), 32 bits blue (signed int), 32 bits green (signed int).
        PF_R32G32B32_SINT = 67,
        /// 128-bit pixel format, 32 bits red (signed int), 32 bits blue (signed int), 32 bits green (signed int), 32 bits alpha (signed int).
        PF_R32G32B32A32_SINT = 68,
        /// 32-bit pixel format, 9 bits for blue, green, red plus a 5 bit exponent.
        PF_R9G9B9E5_SHAREDEXP = 69,
        /// DDS (DirectDraw Surface) BC4 format (unsigned normalised)
        PF_BC4_UNORM = 70,
        /// DDS (DirectDraw Surface) BC4 format (signed normalised)
        PF_BC4_SNORM = 71,
        /// DDS (DirectDraw Surface) BC5 format (unsigned normalised)
        PF_BC5_UNORM = 72,
        /// DDS (DirectDraw Surface) BC5 format (signed normalised)
        PF_BC5_SNORM = 73,
        /// DDS (DirectDraw Surface) BC6H format (unsigned 16 bit float)
        PF_BC6H_UF16 = 74,
        /// DDS (DirectDraw Surface) BC6H format (signed 16 bit float)
        PF_BC6H_SF16 = 75,
        /// DDS (DirectDraw Surface) BC7 format (unsigned normalised)
        PF_BC7_UNORM = 76,
        /// DDS (DirectDraw Surface) BC7 format (unsigned normalised sRGB)
        PF_BC7_UNORM_SRGB = 77,
        /// 8-bit pixel format, all bits red.
        PF_R8 = 78,
        /// 16-bit pixel format, 8 bits red, 8 bits green.
        PF_RG8 = 79,
        /// 8-bit pixel format, 8 bits red (signed normalised int).
        PF_R8_SNORM = 80,
        /// 16-bit pixel format, 8 bits red (signed normalised int), 8 bits blue (signed normalised int).
        PF_R8G8_SNORM = 81,
        /// 24-bit pixel format, 8 bits red (signed normalised int), 8 bits blue (signed normalised int), 8 bits green (signed normalised int).
        PF_R8G8B8_SNORM = 82,
        /// 32-bit pixel format, 8 bits red (signed normalised int), 8 bits blue (signed normalised int), 8 bits green (signed normalised int), 8 bits alpha (signed normalised int).
        PF_R8G8B8A8_SNORM = 83,
        /// 16-bit pixel format, 16 bits red (signed normalised int).
        PF_R16_SNORM = 84,
        /// 32-bit pixel format, 16 bits red (signed normalised int), 16 bits blue (signed normalised int).
        PF_R16G16_SNORM = 85,
        /// 48-bit pixel format, 16 bits red (signed normalised int), 16 bits blue (signed normalised int), 16 bits green (signed normalised int).
        PF_R16G16B16_SNORM = 86,
        /// 64-bit pixel format, 16 bits red (signed normalised int), 16 bits blue (signed normalised int), 16 bits green (signed normalised int), 16 bits alpha (signed normalised int).
        PF_R16G16B16A16_SNORM = 87,
        /// ETC1 (Ericsson Texture Compression)
        PF_ETC1_RGB8 = 88,
        /// ETC2 (Ericsson Texture Compression)
        PF_ETC2_RGB8 = 89,
        /// ETC2 (Ericsson Texture Compression)
        PF_ETC2_RGBA8 = 90,
        /// ETC2 (Ericsson Texture Compression)
        PF_ETC2_RGB8A1 = 91,
        /// ATC (AMD_compressed_ATC_texture)
        PF_ATC_RGB = 92,
        /// ATC (AMD_compressed_ATC_texture)
        PF_ATC_RGBA_EXPLICIT_ALPHA = 93,
        /// ATC (AMD_compressed_ATC_texture)
        PF_ATC_RGBA_INTERPOLATED_ALPHA = 94,

        /// ASTC (ARM Adaptive Scalable Texture Compression RGBA, block size 4x4)
        PF_ASTC_RGBA_4X4_LDR = 95,
        /// ASTC (ARM Adaptive Scalable Texture Compression RGBA, block size 5x4)
        PF_ASTC_RGBA_5X4_LDR = 96,
        /// ASTC (ARM Adaptive Scalable Texture Compression RGBA, block size 5x5)
        PF_ASTC_RGBA_5X5_LDR = 97,
        /// ASTC (ARM Adaptive Scalable Texture Compression RGBA, block size 6x5)
        PF_ASTC_RGBA_6X5_LDR = 98,
        /// ASTC (ARM Adaptive Scalable Texture Compression RGBA, block size 6x6)
        PF_ASTC_RGBA_6X6_LDR = 99,
        /// ASTC (ARM Adaptive Scalable Texture Compression RGBA, block size 8x5)
        PF_ASTC_RGBA_8X5_LDR = 100,
        /// ASTC (ARM Adaptive Scalable Texture Compression RGBA, block size 8x6)
        PF_ASTC_RGBA_8X6_LDR = 101,
        /// ASTC (ARM Adaptive Scalable Texture Compression RGBA, block size 8x8)
        PF_ASTC_RGBA_8X8_LDR = 102,
        /// ASTC (ARM Adaptive Scalable Texture Compression RGBA, block size 10x5)
        PF_ASTC_RGBA_10X5_LDR = 103,
        /// ASTC (ARM Adaptive Scalable Texture Compression RGBA, block size 10x6)
        PF_ASTC_RGBA_10X6_LDR = 104,
        /// ASTC (ARM Adaptive Scalable Texture Compression RGBA, block size 10x8)
        PF_ASTC_RGBA_10X8_LDR = 105,
        /// ASTC (ARM Adaptive Scalable Texture Compression RGBA, block size 10x10)
        PF_ASTC_RGBA_10X10_LDR = 106,
        /// ASTC (ARM Adaptive Scalable Texture Compression RGBA, block size 12x10)
        PF_ASTC_RGBA_12X10_LDR = 107,
        /// ASTC (ARM Adaptive Scalable Texture Compression RGBA, block size 12x12)
        PF_ASTC_RGBA_12X12_LDR = 108,
        /// ASTC (ARM Adaptive Scalable Texture Compression 8 bits each sRGB 8 bits alpha, block size 4x4)
        PF_ASTC_SRGB8A8_4X4_LDR = 109,
        /// ASTC (ARM Adaptive Scalable Texture Compression 8 bits each sRGB 8 bits alpha, block size 5x4)
        PF_ASTC_SRGB8A8_5X4_LDR = 110,
        /// ASTC (ARM Adaptive Scalable Texture Compression 8 bits each sRGB 8 bits alpha, block size 5x5)
        PF_ASTC_SRGB8A8_5X5_LDR = 111,
        /// ASTC (ARM Adaptive Scalable Texture Compression 8 bits each sRGB 8 bits alpha, block size 6x5)
        PF_ASTC_SRGB8A8_6X5_LDR = 112,
        /// ASTC (ARM Adaptive Scalable Texture Compression 8 bits each sRGB 8 bits alpha, block size 6x6)
        PF_ASTC_SRGB8A8_6X6_LDR = 113,
        /// ASTC (ARM Adaptive Scalable Texture Compression 8 bits each sRGB 8 bits alpha, block size 8x5)
        PF_ASTC_SRGB8A8_8X5_LDR = 114,
        /// ASTC (ARM Adaptive Scalable Texture Compression 8 bits each sRGB 8 bits alpha, block size 8x6)
        PF_ASTC_SRGB8A8_8X6_LDR = 115,
        /// ASTC (ARM Adaptive Scalable Texture Compression 8 bits each sRGB 8 bits alpha, block size 8x8)
        PF_ASTC_SRGB8A8_8X8_LDR = 116,
        /// ASTC (ARM Adaptive Scalable Texture Compression 8 bits each sRGB 8 bits alpha, block size 10x5)
        PF_ASTC_SRGB8A8_10X5_LDR = 117,
        /// ASTC (ARM Adaptive Scalable Texture Compression 8 bits each sRGB 8 bits alpha, block size 10x6)
        PF_ASTC_SRGB8A8_10X6_LDR = 118,
        /// ASTC (ARM Adaptive Scalable Texture Compression 8 bits each sRGB 8 bits alpha, block size 10x8)
        PF_ASTC_SRGB8A8_10X8_LDR = 119,
        /// ASTC (ARM Adaptive Scalable Texture Compression 8 bits each sRGB 8 bits alpha, block size 10x10)
        PF_ASTC_SRGB8A8_10X10_LDR = 120,
        /// ASTC (ARM Adaptive Scalable Texture Compression 8 bits each sRGB 8 bits alpha, block size 12x10)
        PF_ASTC_SRGB8A8_12X10_LDR = 121,
        /// ASTC (ARM Adaptive Scalable Texture Compression 8 bits each sRGB 8 bits alpha, block size 12x12)
        PF_ASTC_SRGB8A8_12X12_LDR = 122,

        /// Depth texture format. 24 bits for depth, 8 bits for stencil.
        /// The following formats are just reinterpretations of the same depth buffer:
        ///  24-bit normalized uint depth and 8-bit stencil
        ///     * PF_D24_UNORM_S8_UINT
        ///     * PF_D24_UNORM_X8
        ///     * PF_X24_S8_UINT
        /// 24-bit normalized uint depth
        ///     * PF_D24_UNORM
        ///  16-bit normalized uint depth
        ///     * PF_D16_UNORM
        ///  32-bit floating point depth
        ///     * PF_D32_FLOAT
        ///  32-bit floating point depth & 8-bit stencil (+24 unused bits)
        ///     * PF_D32_FLOAT_X24_S8_UINT
        ///     * PF_D32_FLOAT_X24_X8
        ///     * PF_X32_X24_S8_UINT
        ///
        /// This means that e.g. a PF_D24_UNORM_X8 and a PF_D24_UNORM_S8_UINT
        /// may or may not internally point to the same depth buffer. Use depth buffer
        /// pools if you want a guarantee that they use different buffers.
        /// Last but not least, not all GPUs support all these formats.
        PF_D24_UNORM_S8_UINT = 123,
        /// Depth texture format. 24 bits for depth.
        PF_D24_UNORM_X8 = 124,
        /// Depth texture format. 8 bits for stencil.
        PF_X24_S8_UINT = 125,
        /// Depth texture format 24 bits for depth.
        PF_D24_UNORM = 126,
        /// Depth texture format. 16 bits for depth.
        PF_D16_UNORM = 127,
        /// Depth texture format. 32 bits for depth.
        PF_D32_FLOAT = 128,
        /// Depth texture format. 32 bits for depth. 8 bits for stencil
        PF_D32_FLOAT_X24_S8_UINT = 129,
        /// Depth texture format. 32 bits for depth.
        PF_D32_FLOAT_X24_X8 = 130,
        /// Depth texture format. 8 bits for stencil
        PF_X32_X24_S8_UINT = 131,

        /// Dummy, used for UAV-only rendering. D3D11 calls it
        /// Target-independent rasterization / UAVOnlyRenderingForcedSampleCount
        /// OpenGL is under GL_ARB_framebuffer_no_attachments
        PF_NULL = 132,

        // Number of pixel formats currently defined
        PF_COUNT = 133
    };
    typedef vector<PixelFormat>::type PixelFormatList;

    /**
     * Flags defining some on/off properties of pixel formats
     */
    enum PixelFormatFlags {
        /// This format has an alpha channel
        PFF_HASALPHA        = 0x00000001,      
        /** This format is compressed. This invalidates the values in elemBytes,
            elemBits and the bit counts as these might not be fixed in a compressed format. */
        PFF_COMPRESSED    = 0x00000002,
        /// This is a floating point format
        PFF_FLOAT           = 0x00000004,         
        /// This is a depth format (for depth textures)
        PFF_DEPTH           = 0x00000008,
        /** Format is in native endian. Generally true for the 16, 24 and 32 bits
            formats which can be represented as machine integers. */
        PFF_NATIVEENDIAN    = 0x00000010,
        /** This is an intensity format instead of a RGB one. The luminance
            replaces R,G and B. (but not A) */
        PFF_LUMINANCE       = 0x00000020,
        /// This is an integer format
        PFF_INTEGER         = 0x00000040,
        /// This integer format is signed.
        PFF_SIGNED         = 0x00000080
    };
    
    /** Pixel component format */
    enum PixelComponentType
    {
        PCT_BYTE = 0,    /// Byte per component (8 bit fixed 0.0..1.0)
        PCT_SHORT = 1,   /// Short per component (16 bit fixed 0.0..1.0))
        PCT_FLOAT16 = 2, /// 16 bit float per component
        PCT_FLOAT32 = 3, /// 32 bit float per component
        PCT_SINT = 4,   /// Signed integer per component
        PCT_UINT = 5,   /// Unsigned integer per component
        PCT_COUNT = 6    /// Number of pixel types
    };

    /**
     * Some utility functions for packing and unpacking pixel data
     */
    class _OgreExport PixelUtil {
    public:
        /** Returns the size in bytes of an element of the given pixel format.
         @return
               The size in bytes of an element. See Remarks.
         @remarks
               Passing PF_UNKNOWN will result in returning a size of 0 bytes.
        */
        static size_t getNumElemBytes( PixelFormat format );

        /** Returns the size in bits of an element of the given pixel format.
          @return
               The size in bits of an element. See Remarks.
           @remarks
               Passing PF_UNKNOWN will result in returning a size of 0 bits.
        */
        static size_t getNumElemBits( PixelFormat format );

        /** Returns the maximum number of mipmaps given the resolution
            e.g. at 4x4 there's 2 mipmaps. At 1x1 there's 0 mipmaps.
        @return
            Mip count.
        */
        static uint8 getMaxMipmapCount( uint32 maxResolution );
        static uint8 getMaxMipmapCount( uint32 width, uint32 height );
        static uint8 getMaxMipmapCount( uint32 width, uint32 height, uint32 depth );

        /// WARNING: numMipmaps includes the mip 0. That means numMipmaps > 0,
        /// and that you should use texture->getNumMipmaps + 1u
        static size_t calculateSizeBytes( uint32 width, uint32 height, uint32 depth,
                                          uint32 slices, PixelFormat format, uint8 numMipmaps );

        /** Returns the size in memory of a region with the given extents and pixel
            format with consecutive memory layout.
            @param width
                The width of the area
            @param height
                The height of the area
            @param depth
                The depth of the area
            @param format
                The format of the area
            @return
                The size in bytes
            @remarks
                In case that the format is non-compressed, this simply returns
                width*height*depth*PixelUtil::getNumElemBytes(format). In the compressed
                case, this does serious magic.
        */
        static size_t getMemorySize(uint32 width, uint32 height, uint32 depth, PixelFormat format);

        /** Returns the minimum width for block compressed schemes. ie. DXT1 compresses in blocks
            of 4x4 pixels. A texture with a width of 2 is just padded to 4.
            When building UV atlases composed of already compressed data being stitched together,
            the block size is very important to know as the resolution of the individual textures
            must be a multiple of this size.
         @remarks
            If the format is not compressed, returns 1.
         @par
            The function can return a value of 0 (as happens with PVRTC & ETC1 compression); this is
            because although they may compress in blocks (i.e. PVRTC uses a 4x4 or 8x4 block), this
            information is useless as the compression scheme doesn't have isolated blocks (modifying
            a single pixel can change the binary data of the entire stream) making it useless for
            subimage sampling or creating UV atlas.
         @param format
            The format to query for. Can be compressed or not.
         @param apiStrict
            When true, obeys the rules of most APIs (i.e. ETC1 can't update subregions according to
            GLES specs). When false, becomes more practical if manipulating by hand (i.e. ETC1's
            subregions can be updated just fine by @bulkCompressedSubregion)
         @return
            The width of compression block, in pixels. Can be 0 (see remarks). If format is not
            compressed, returns 1.
        */
        static uint32 getCompressedBlockWidth( PixelFormat format, bool apiStrict=true );

        /// @See getCompressedBlockWidth
        static uint32 getCompressedBlockHeight( PixelFormat format, bool apiStrict=true );
        
        /** Returns the property flags for this pixel format
          @return
               A bitfield combination of PFF_HASALPHA, PFF_ISCOMPRESSED,
               PFF_FLOAT, PFF_DEPTH, PFF_NATIVEENDIAN, PFF_LUMINANCE
          @remarks
               This replaces the separate functions for formatHasAlpha, formatIsFloat, ...
        */
        static unsigned int getFlags( PixelFormat format );

        /** Shortcut method to determine if the format has an alpha component */
        static bool hasAlpha(PixelFormat format);
        /** Shortcut method to determine if the format is floating point */
        static bool isFloatingPoint(PixelFormat format);
        /** Shortcut method to determine if the format is integer */
        static bool isInteger(PixelFormat format);
        /** Shortcut method to determine if the format is signed */
        static bool isSigned(PixelFormat format);
        /** Shortcut method to determine if the format is compressed */
        static bool isCompressed(PixelFormat format);
        /** Shortcut method to determine if the format is a depth format. */
        static bool isDepth(PixelFormat format);
        /** Shortcut method to determine if the format is in native endian format. */
        static bool isNativeEndian(PixelFormat format);
        /** Shortcut method to determine if the format is a luminance format. */
        static bool isLuminance(PixelFormat format);
        
        /** Return whether a certain image extent is valid for this image format.
            @param width
                The width of the area
            @param height
                The height of the area
            @param depth
                The depth of the area
            @param format
                The format of the area
            @remarks For non-compressed formats, this is always true. For DXT formats,
            only sizes with a width and height multiple of 4 and depth 1 are allowed.
        */
        static bool isValidExtent(size_t width, size_t height, size_t depth, PixelFormat format);

        /** Gives the number of bits (RGBA) for a format. See remarks.          
          @remarks      For non-colour formats (dxt, depth) this returns [0,0,0,0].
        */
        static void getBitDepths(PixelFormat format, int rgba[4]);

        /** Gives the masks for the R, G, B and A component
          @note         Only valid for native endian formats
        */
        static void getBitMasks(PixelFormat format, uint64 rgba[4]);

        /** Gives the bit shifts for R, G, B and A component
        @note           Only valid for native endian formats
        */
        static void getBitShifts(PixelFormat format, unsigned char rgba[4]);

        /** Gets the name of an image format
        */
        static String getFormatName(PixelFormat srcformat);

        /** Returns whether the format can be packed or unpacked with the packColour()
        and unpackColour() functions. This is generally not true for compressed and
        depth formats as they are special. It can only be true for formats with a
        fixed element size.
          @return 
               true if yes, otherwise false
        */
        static bool isAccessible(PixelFormat srcformat);
        
        /** Returns the component type for a certain pixel format. Returns PCT_BYTE
            in case there is no clear component type like with compressed formats.
            This is one of PCT_BYTE, PCT_SHORT, PCT_FLOAT16, PCT_FLOAT32.
        */
        static PixelComponentType getComponentType(PixelFormat fmt);
        
        /** Returns the component count for a certain pixel format. Returns 3(no alpha) or 
            4 (has alpha) in case there is no clear component type like with compressed formats.
         */
        static size_t getComponentCount(PixelFormat fmt);

        /** Gets the format from given name.
            @param  name            The string of format name
            @param  accessibleOnly  If true, non-accessible format will treat as invalid format,
                                    otherwise, all supported format are valid.
            @param  caseSensitive   Should be set true if string match should use case sensitivity.
            @return                The format match the format name, or PF_UNKNOWN if is invalid name.
        */
        static PixelFormat getFormatFromName(const String& name, bool accessibleOnly = false, bool caseSensitive = false);

        /** Gets the BNF expression of the pixel-formats.
            @note                   The string returned by this function is intended to be used as a BNF expression
                                    to work with Compiler2Pass.
            @param  accessibleOnly  If true, only accessible pixel format will take into account, otherwise all
                                    pixel formats list in PixelFormat enumeration will being returned.
            @return                A string contains the BNF expression.
        */
        static String getBNFExpressionOfPixelFormats(bool accessibleOnly = false);

        /** Returns the similar format but acoording with given bit depths.
            @param fmt      The original foamt.
            @param integerBits Preferred bit depth (pixel bits) for integer pixel format.
                            Available values: 0, 16 and 32, where 0 (the default) means as it is.
            @param floatBits Preferred bit depth (channel bits) for float pixel format.
                            Available values: 0, 16 and 32, where 0 (the default) means as it is.
            @return        The format that similar original format with bit depth according
                            with preferred bit depth, or original format if no conversion occurring.
        */
        static PixelFormat getFormatForBitDepths(PixelFormat fmt, ushort integerBits, ushort floatBits);

        /** Pack a colour value to memory
            @param colour   The colour
            @param pf       Pixelformat in which to write the colour
            @param dest     Destination memory location
        */
        static void packColour(const ColourValue &colour, const PixelFormat pf,  void* dest);
        /** Pack a colour value to memory
            @param r,g,b,a  The four colour components, range 0.0f to 1.0f
                            (an exception to this case exists for floating point pixel
                            formats, which don't clamp to 0.0f..1.0f)
            @param pf       Pixelformat in which to write the colour
            @param dest     Destination memory location
        */
        static void packColour(const uint8 r, const uint8 g, const uint8 b, const uint8 a, const PixelFormat pf,  void* dest);
         /** Pack a colour value to memory
            @param r,g,b,a  The four colour components, range 0.0f to 1.0f
                            (an exception to this case exists for floating point pixel
                            formats, which don't clamp to 0.0f..1.0f)
            @param pf       Pixelformat in which to write the colour
            @param dest     Destination memory location
        */
        static void packColour(const float r, const float g, const float b, const float a, const PixelFormat pf,  void* dest);

        /** Unpack a colour value from memory
            @param colour   The colour is returned here
            @param pf       Pixelformat in which to read the colour
            @param src      Source memory location
        */
        static void unpackColour(ColourValue *colour, PixelFormat pf,  const void* src);
        /** Unpack a colour value from memory
            @param r        The red channel is returned here (as byte)
            @param g        The blue channel is returned here (as byte)
            @param b        The green channel is returned here (as byte)
            @param a        The alpha channel is returned here (as byte)
            @param pf       Pixelformat in which to read the colour
            @param src      Source memory location
            @remarks    This function returns the colour components in 8 bit precision,
                this will lose precision when coming from PF_A2R10G10B10 or floating
                point formats.  
        */
        static void unpackColour(uint8 *r, uint8 *g, uint8 *b, uint8 *a, PixelFormat pf,  const void* src);
        /** Unpack a colour value from memory
            @param r        The red channel is returned here (as float)
            @param g        The blue channel is returned here (as float)
            @param b        The green channel is returned here (as float)
            @param a        The alpha channel is returned here (as float)
            @param pf       Pixelformat in which to read the colour
            @param src      Source memory location
        */
        static void unpackColour(float *r, float *g, float *b, float *a, PixelFormat pf,  const void* src); 
        
        /** Convert consecutive pixels from one format to another. No dithering or filtering is being done. 
            Converting from RGB to luminance takes the R channel.  In case the source and destination format match,
            just a copy is done.
            @param  src         Pointer to source region
            @param  srcFormat   Pixel format of source region
            @param  dst         Pointer to destination region
            @param  dstFormat   Pixel format of destination region
         */
        static void bulkPixelConversion(void *src, PixelFormat srcFormat, void *dst, PixelFormat dstFormat, unsigned int count);

        /** Convert pixels from one format to another. No dithering or filtering is being done. Converting
            from RGB to luminance takes the R channel. 
            @param  src         PixelBox containing the source pixels, pitches and format
            @param  dst         PixelBox containing the destination pixels, pitches and format
            @remarks The source and destination boxes must have the same
            dimensions. In case the source and destination format match, a plain copy is done.
        */
        static void bulkPixelConversion(const PixelBox &src, const PixelBox &dst);

        /** Converts the input source to either PF_R8G8_SNORM or PF_BYTE_LA.
            dst must be one of either formats.
            @param  src         PixelBox containing the source pixels, pitches and format
            @param  dst         PixelBox containing the destination pixels, pitches and format
            @remarks The source and destination boxes must have the same dimensions.
        */
        static void convertForNormalMapping(const PixelBox &src, const PixelBox &dst);

        /** Emplaces the binary compressed data from src into a subregion of dst.
        @param  src
            PixelBox containing the source pixels, pitches and format.
            Data must be consecutive
        @param  dst
            PixelBox containing the destination pixels, pitches and format.
            Data must be consecutive
        @param dstRegion
            The region on dst where src will be emplaced. dstRegion's resolution must
            match that of src. dstRegion must be within dst's bounds.
        @remarks
            The source and destination must have the same the same format.
        @par
            Each compression format may enforce different requirements. Most notably
            the subregions' bounds must be aligned to a certain boundary (usually to
            multiples of 4). If these requirements aren't met, an exception will be
            thrown.
        @par
            Some formats (i.e. PVRTC) don't support subregions at all, and thus
            an exception will be thrown.
        @par
            @See getCompressedBlockWidth
        */
        static void bulkCompressedSubregion( const PixelBox &src, const PixelBox &dst,
                                             const Box &dstRegion );

        /** Flips pixels inplace in vertical direction.
            @param  box         PixelBox containing pixels, pitches and format
            @remarks Non consecutive pixel boxes are supported.
         */
        static void bulkPixelVerticalFlip(const PixelBox &box);
    };

    class _OgreExport PixelFormatToShaderType
    {
    public:
        /** Converts a PixelFormat into its equivalent layout for image variables (GLSL)
            or its equivalent for D3D11/12 variables (HLSL). Used mostly with UAVs.
        @param pixelFormat
            Pixel format to convert.
        @return
            String for the shader to use "as is". If the Pixel Format doesn't have
            a shader equivalent (i.e. depth formats), a null pointer is returned.
            The validity of the pointer lasts as long as the RenderSystem remains
            loaded.
        */
        virtual const char* getPixelFormatType( PixelFormat pixelFormat ) const = 0;
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
