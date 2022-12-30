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
#ifndef __PixelFormatDescriptions_H__
#define __PixelFormatDescriptions_H__

namespace Ogre {
    //-----------------------------------------------------------------------
    /**
     * A record that describes a pixel format in detail.
     */
    struct PixelFormatDescription {
        /* Name of the format, as in the enum */
        const String name;
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
        unsigned char rbits, gbits, bbits, abits; /*, ibits, dbits, ... */

        /* Masks and shifts as used by packers/unpackers */
        uint64 rmask, gmask, bmask, amask;
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
            PFF_HASALPHA,
            /* Component type and count */
            PCT_BYTE, 1,
            /* rbits, gbits, bbits, abits */
            0, 0, 0, 8,
            /* Masks and shifts */
            0, 0, 0, 0xFF, 0, 0, 0, 0
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
            0xFF,0,0,0xFF00,0,0,0,8
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
            PFF_COMPRESSED,
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
        {"PF_DEPTH16",
            /* Bytes per element */
            2,
            /* Flags */
            PFF_LUMINANCE | PFF_DEPTH | PFF_NATIVEENDIAN,
            /* Component type and count */
            PCT_SHORT, 1,
            /* rbits, gbits, bbits, abits */
            16, 0, 0, 0,
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
        //-----------------------------------------------------------------------
        {"PF_PVRTC_RGB2",
            /* Bytes per element */
            0,
            /* Flags */
            PFF_COMPRESSED,
            /* Component type and count */
            PCT_BYTE, 3,
            /* rbits, gbits, bbits, abits */
            0, 0, 0, 0,
            /* Masks and shifts */
            0, 0, 0, 0, 0, 0, 0, 0
        },
        //-----------------------------------------------------------------------
        {"PF_PVRTC_RGBA2",
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
        {"PF_PVRTC_RGB4",
            /* Bytes per element */
            0,
            /* Flags */
            PFF_COMPRESSED,
            /* Component type and count */
            PCT_BYTE, 3,
            /* rbits, gbits, bbits, abits */
            0, 0, 0, 0,
            /* Masks and shifts */
            0, 0, 0, 0, 0, 0, 0, 0
        },
        //-----------------------------------------------------------------------
        {"PF_PVRTC_RGBA4",
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
        {"PF_PVRTC2_2BPP",
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
        {"PF_PVRTC2_4BPP",
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
        {"PF_R11G11B10_FLOAT",
            /* Bytes per element */
            4,
            /* Flags */
            PFF_FLOAT,
            /* Component type and count */
            PCT_FLOAT32, 1,
            /* rbits, gbits, bbits, abits */
            11, 11, 10, 0,
            /* Masks and shifts */
            0xFFC00000, 0x03FF800, 0x000007FF, 0,
            24, 16, 8, 0
        },
        //-----------------------------------------------------------------------
        {"PF_R8_UINT",
            /* Bytes per element */
            1,
            /* Flags */
            PFF_INTEGER | PFF_NATIVEENDIAN,
            /* Component type and count */
            PCT_UINT, 1,
            /* rbits, gbits, bbits, abits */
            8, 0, 0, 0,
            /* Masks and shifts */
            0xFF, 0, 0, 0,
            0, 0, 0, 0
        },
        //-----------------------------------------------------------------------
        {"PF_R8G8_UINT",
            /* Bytes per element */
            2,
            /* Flags */
            PFF_INTEGER | PFF_NATIVEENDIAN,
            /* Component type and count */
            PCT_UINT, 2,
            /* rbits, gbits, bbits, abits */
            8, 8, 0, 0,
            /* Masks and shifts */
            0xFF00, 0x00FF, 0, 0,
            8, 0, 0, 0
        },
        //-----------------------------------------------------------------------
        {"PF_R8G8B8_UINT",
            /* Bytes per element */
            3,
            /* Flags */
            PFF_INTEGER | PFF_NATIVEENDIAN,
            /* Component type and count */
            PCT_UINT, 3,
            /* rbits, gbits, bbits, abits */
            8, 8, 8, 0,
            /* Masks and shifts */
            0xFF0000, 0x00FF00, 0x0000FF, 0,
            16, 8, 0, 0
        },
        //-----------------------------------------------------------------------
        {"PF_R8G8B8A8_UINT",
            /* Bytes per element */
            4,
            /* Flags */
            PFF_INTEGER | PFF_HASALPHA | PFF_NATIVEENDIAN,
            /* Component type and count */
            PCT_UINT, 4,
            /* rbits, gbits, bbits, abits */
            8, 8, 8, 8,
            /* Masks and shifts */
            0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF,
            24, 16, 8, 0
        },
        //-----------------------------------------------------------------------
        {"PF_R16_UINT",
            /* Bytes per element */
            2,
            /* Flags */
            PFF_INTEGER | PFF_NATIVEENDIAN,
            /* Component type and count */
            PCT_UINT, 1,
            /* rbits, gbits, bbits, abits */
            16, 0, 0, 0,
            /* Masks and shifts */
            0xFFFF, 0, 0, 0,
            0, 0, 0, 0
        },
        //-----------------------------------------------------------------------
        {"PF_R16G16_UINT",
            /* Bytes per element */
            4,
            /* Flags */
            PFF_INTEGER | PFF_NATIVEENDIAN,
            /* Component type and count */
            PCT_UINT, 2,
            /* rbits, gbits, bbits, abits */
            16, 16, 0, 0,
            /* Masks and shifts */
            0xFFFF0000, 0x0000FFFF, 0, 0,
            16, 0, 0, 0
        },
        //-----------------------------------------------------------------------
        {"PF_R16G16B16_UINT",
            /* Bytes per element */
            6,
            /* Flags */
            PFF_INTEGER | PFF_NATIVEENDIAN,
            /* Component type and count */
            PCT_UINT, 3,
            /* rbits, gbits, bbits, abits */
            16, 16, 16, 0,
            /* Masks and shifts */
            0xFFFF00000000ULL, 0x0000FFFF0000ULL, 0x00000000FFFFULL, 0,
            32, 16, 0, 0
        },
        //-----------------------------------------------------------------------
        {"PF_R16G16B16A16_UINT",
            /* Bytes per element */
            8,
            /* Flags */
            PFF_INTEGER | PFF_HASALPHA | PFF_NATIVEENDIAN,
            /* Component type and count */
            PCT_UINT, 4,
            /* rbits, gbits, bbits, abits */
            16, 16, 16, 16,
            /* Masks and shifts */
            0xFFFF000000000000ULL, 0x0000FFFF00000000ULL, 0x00000000FFFF0000ULL, 0x000000000000FFFFULL,
            48, 32, 16, 0
        },
        //-----------------------------------------------------------------------
        {"PF_R32_UINT",
            /* Bytes per element */
            4,
            /* Flags */
            PFF_INTEGER | PFF_NATIVEENDIAN,
            /* Component type and count */
            PCT_UINT, 1,
            /* rbits, gbits, bbits, abits */
            32, 0, 0, 0,
            /* Masks and shifts */
            0xFFFFFFFF, 0, 0, 0,
            0, 0, 0, 0
        },
        //-----------------------------------------------------------------------
        {"PF_R32G32_UINT",
            /* Bytes per element */
            8,
            /* Flags */
            PFF_INTEGER | PFF_NATIVEENDIAN,
            /* Component type and count */
            PCT_UINT, 2,
            /* rbits, gbits, bbits, abits */
            32, 32, 0, 0,
            /* Masks and shifts */
            0xFFFFFFFF00000000ULL, 0xFFFFFFFFULL, 0, 0,
            32, 0, 0, 0
        },
        //-----------------------------------------------------------------------
        {"PF_R32G32B32_UINT",
            /* Bytes per element */
            12,
            /* Flags */
            PFF_INTEGER | PFF_NATIVEENDIAN,
            /* Component type and count */
            PCT_UINT, 3,
            /* rbits, gbits, bbits, abits */
            32, 32, 32, 0,
            /* Masks and shifts */
            0, 0, 0, 0,
            64, 32, 0, 0
        },
        //-----------------------------------------------------------------------
        {"PF_R32G32B32A32_UINT",
            /* Bytes per element */
            16,
            /* Flags */
            PFF_INTEGER | PFF_HASALPHA | PFF_NATIVEENDIAN,
            /* Component type and count */
            PCT_UINT, 4,
            /* rbits, gbits, bbits, abits */
            32, 32, 32, 32,
            /* Masks and shifts */
            0, 0, 0, 0,
            96, 64, 32, 0
        },
        //-----------------------------------------------------------------------
        {"PF_R8_SINT",
            /* Bytes per element */
            1,
            /* Flags */
            PFF_INTEGER | PFF_NATIVEENDIAN,
            /* Component type and count */
            PCT_SINT, 1,
            /* rbits, gbits, bbits, abits */
            8, 0, 0, 0,
            /* Masks and shifts */
            0xFF, 0, 0, 0,
            0, 0, 0, 0
        },
        //-----------------------------------------------------------------------
        {"PF_R8G8_SINT",
            /* Bytes per element */
            2,
            /* Flags */
            PFF_INTEGER | PFF_NATIVEENDIAN,
            /* Component type and count */
            PCT_SINT, 2,
            /* rbits, gbits, bbits, abits */
            8, 8, 0, 0,
            /* Masks and shifts */
            0xFF00, 0x00FF, 0, 0,
            8, 0, 0, 0
        },
        //-----------------------------------------------------------------------
        {"PF_R8G8B8_SINT",
            /* Bytes per element */
            3,
            /* Flags */
            PFF_INTEGER | PFF_NATIVEENDIAN,
            /* Component type and count */
            PCT_SINT, 3,
            /* rbits, gbits, bbits, abits */
            8, 8, 8, 0,
            /* Masks and shifts */
            0xFF0000, 0x00FF00, 0x0000FF, 0,
            16, 8, 0, 0
        },
        //-----------------------------------------------------------------------
        {"PF_R8G8B8A8_SINT",
            /* Bytes per element */
            4,
            /* Flags */
            PFF_INTEGER | PFF_HASALPHA | PFF_NATIVEENDIAN,
            /* Component type and count */
            PCT_SINT, 4,
            /* rbits, gbits, bbits, abits */
            8, 8, 8, 8,
            /* Masks and shifts */
            0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF,
            24, 16, 8, 0
        },
        //-----------------------------------------------------------------------
        {"PF_R16_SINT",
            /* Bytes per element */
            2,
            /* Flags */
            PFF_INTEGER | PFF_NATIVEENDIAN,
            /* Component type and count */
            PCT_SINT, 1,
            /* rbits, gbits, bbits, abits */
            16, 0, 0, 0,
            /* Masks and shifts */
            0xFFFF, 0, 0, 0,
            0, 0, 0, 0
        },
        //-----------------------------------------------------------------------
        {"PF_R16G16_SINT",
            /* Bytes per element */
            4,
            /* Flags */
            PFF_INTEGER | PFF_NATIVEENDIAN,
            /* Component type and count */
            PCT_SINT, 2,
            /* rbits, gbits, bbits, abits */
            16, 16, 0, 0,
            /* Masks and shifts */
            0xFFFF0000, 0x0000FFFF, 0, 0,
            16, 0, 0, 0
        },
        //-----------------------------------------------------------------------
        {"PF_R16G16B16_SINT",
            /* Bytes per element */
            6,
            /* Flags */
            PFF_INTEGER | PFF_NATIVEENDIAN,
            /* Component type and count */
            PCT_SINT, 3,
            /* rbits, gbits, bbits, abits */
            16, 16, 16, 0,
            /* Masks and shifts */
            0xFFFF00000000ULL, 0x0000FFFF0000ULL, 0x00000000FFFFULL, 0,
            32, 16, 0, 0
        },
        //-----------------------------------------------------------------------
        {"PF_R16G16B16A16_SINT",
            /* Bytes per element */
            8,
            /* Flags */
            PFF_INTEGER | PFF_HASALPHA | PFF_NATIVEENDIAN,
            /* Component type and count */
            PCT_SINT, 4,
            /* rbits, gbits, bbits, abits */
            16, 16, 16, 16,
            /* Masks and shifts */
            0xFFFF000000000000ULL, 0x0000FFFF00000000ULL, 0x00000000FFFF0000ULL, 0x000000000000FFFFULL,
            48, 32, 16, 0
        },
        //-----------------------------------------------------------------------
        {"PF_R32_SINT",
            /* Bytes per element */
            4,
            /* Flags */
            PFF_INTEGER | PFF_NATIVEENDIAN,
            /* Component type and count */
            PCT_SINT, 1,
            /* rbits, gbits, bbits, abits */
            32, 0, 0, 0,
            /* Masks and shifts */
            0xFFFFFFFF, 0, 0, 0,
            0, 0, 0, 0
        },
        //-----------------------------------------------------------------------
        {"PF_R32G32_SINT",
            /* Bytes per element */
            8,
            /* Flags */
            PFF_INTEGER | PFF_NATIVEENDIAN,
            /* Component type and count */
            PCT_SINT, 2,
            /* rbits, gbits, bbits, abits */
            32, 32, 0, 0,
            /* Masks and shifts */
            0xFFFFFFFF00000000ULL, 0xFFFFFFFFULL, 0, 0,
            32, 0, 0, 0
        },
        //-----------------------------------------------------------------------
        {"PF_R32G32B32_SINT",
            /* Bytes per element */
            12,
            /* Flags */
            PFF_INTEGER | PFF_NATIVEENDIAN,
            /* Component type and count */
            PCT_SINT, 3,
            /* rbits, gbits, bbits, abits */
            32, 32, 32, 0,
            /* Masks and shifts */
            0, 0, 0, 0,
            64, 32, 0, 0
        },
        //-----------------------------------------------------------------------
        {"PF_R32G32B32A32_SINT",
            /* Bytes per element */
            16,
            /* Flags */
            PFF_INTEGER | PFF_HASALPHA | PFF_NATIVEENDIAN,
            /* Component type and count */
            PCT_SINT, 4,
            /* rbits, gbits, bbits, abits */
            32, 32, 32, 32,
            /* Masks and shifts */
            0, 0, 0, 0,
            96, 64, 32, 0
        },
        //-----------------------------------------------------------------------
        {"PF_R9G9B9E5_SHAREDEXP",
            /* Bytes per element */
            4,
            /* Flags */
            PFF_NATIVEENDIAN,
            /* Component type and count */
            PCT_BYTE, 4,
            /* rbits, gbits, bbits, abits */
            9, 9, 9, 0,
            /* Masks and shifts */
            0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF,
            24, 16, 8, 0
        },
        //-----------------------------------------------------------------------
        {"PF_BC4_UNORM",
            /* Bytes per element */
            0,
            /* Flags */
            PFF_COMPRESSED,
            /* Component type and count */
            PCT_BYTE, 1, // Red only
            /* rbits, gbits, bbits, abits */
            0, 0, 0, 0,
            /* Masks and shifts */
            0, 0, 0, 0, 0, 0, 0, 0
        },
        //-----------------------------------------------------------------------
        {"PF_BC4_SNORM",
            /* Bytes per element */
            0,
            /* Flags */
            PFF_COMPRESSED,
            /* Component type and count */
            PCT_BYTE, 1, // Red only
            /* rbits, gbits, bbits, abits */
            0, 0, 0, 0,
            /* Masks and shifts */
            0, 0, 0, 0, 0, 0, 0, 0
        },
        //-----------------------------------------------------------------------
        {"PF_BC5_UNORM",
            /* Bytes per element */
            0,
            /* Flags */
            PFF_COMPRESSED,
            /* Component type and count */
            PCT_BYTE, 2, // Red-Green only
            /* rbits, gbits, bbits, abits */
            0, 0, 0, 0,
            /* Masks and shifts */
            0, 0, 0, 0, 0, 0, 0, 0
        },
        //-----------------------------------------------------------------------
        {"PF_BC5_SNORM",
            /* Bytes per element */
            0,
            /* Flags */
            PFF_COMPRESSED,
            /* Component type and count */
            PCT_BYTE, 2, // Red-Green only
            /* rbits, gbits, bbits, abits */
            0, 0, 0, 0,
            /* Masks and shifts */
            0, 0, 0, 0, 0, 0, 0, 0
        },
        //-----------------------------------------------------------------------
        {"PF_BC6H_UF16",
            /* Bytes per element */
            0,
            /* Flags */
            PFF_COMPRESSED,
            /* Component type and count */
            PCT_BYTE, 3,
            /* rbits, gbits, bbits, abits */
            0, 0, 0, 0,
            /* Masks and shifts */
            0, 0, 0, 0, 0, 0, 0, 0
        },
        //-----------------------------------------------------------------------
        {"PF_BC6H_SF16",
            /* Bytes per element */
            0,
            /* Flags */
            PFF_COMPRESSED,
            /* Component type and count */
            PCT_BYTE, 3,
            /* rbits, gbits, bbits, abits */
            0, 0, 0, 0,
            /* Masks and shifts */
            0, 0, 0, 0, 0, 0, 0, 0
        },
        //-----------------------------------------------------------------------
        {"PF_BC7_UNORM",
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
        {"PF_R8",
            /* Bytes per element */
            1,
            /* Flags */
            PFF_NATIVEENDIAN,
            /* Component type and count */
            PCT_BYTE, 1,
            /* rbits, gbits, bbits, abits */
            8, 0, 0, 0,
            /* Masks and shifts */
            0xFF, 0, 0, 0,
            0, 0, 0, 0
        },
        //-----------------------------------------------------------------------
        {"PF_R8G8",
            /* Bytes per element */
            2,
            /* Flags */
            PFF_NATIVEENDIAN,
            /* Component type and count */
            PCT_BYTE, 2,
            /* rbits, gbits, bbits, abits */
            8, 8, 0, 0,
            /* Masks and shifts */
            0xFF0000, 0x00FF00, 0, 0,
            8, 0, 0, 0
        },
        //-----------------------------------------------------------------------
        {"PF_R8_SNORM",
            /* Bytes per element */
            1,
            /* Flags */
            PFF_NATIVEENDIAN,
            /* Component type and count */
            PCT_BYTE, 1,
            /* rbits, gbits, bbits, abits */
            8, 0, 0, 0,
            /* Masks and shifts */
            0xFF, 0, 0, 0,
            0, 0, 0, 0
        },
        //-----------------------------------------------------------------------
        {"PF_R8G8_SNORM",
            /* Bytes per element */
            2,
            /* Flags */
            PFF_NATIVEENDIAN,
            /* Component type and count */
            PCT_BYTE, 2,
            /* rbits, gbits, bbits, abits */
            8, 8, 0, 0,
            /* Masks and shifts */
            0xFF00, 0x00FF, 0, 0,
            8, 0, 0, 0
        },
        //-----------------------------------------------------------------------
        {"PF_R8G8B8_SNORM",
            /* Bytes per element */
            3,
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
        {"PF_R8G8B8A8_SNORM",
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
        {"PF_R16_SNORM",
            /* Bytes per element */
            2,
            /* Flags */
            PFF_NATIVEENDIAN,
            /* Component type and count */
            PCT_BYTE, 1,
            /* rbits, gbits, bbits, abits */
            16, 0, 0, 0,
            /* Masks and shifts */
            0xFFFF, 0, 0, 0,
            0, 0, 0, 0
        },
        //-----------------------------------------------------------------------
        {"PF_R16G16_SNORM",
            /* Bytes per element */
            4,
            /* Flags */
            PFF_NATIVEENDIAN,
            /* Component type and count */
            PCT_BYTE, 2,
            /* rbits, gbits, bbits, abits */
            16, 16, 0, 0,
            /* Masks and shifts */
            0xFFFF0000, 0x0000FFFF, 0, 0,
            16, 0, 0, 0
        },
        //-----------------------------------------------------------------------
        {"PF_R16G16B16_SNORM",
            /* Bytes per element */
            6,
            /* Flags */
            PFF_NATIVEENDIAN,
            /* Component type and count */
            PCT_BYTE, 3,
            /* rbits, gbits, bbits, abits */
            16, 16, 16, 0,
            /* Masks and shifts */
            0xFFFF00000000ULL, 0x0000FFFF0000ULL, 0x00000000FFFFULL, 0,
            32, 16, 0, 0
        },
        //-----------------------------------------------------------------------
        {"PF_R16G16B16A16_SNORM",
            /* Bytes per element */
            8,
            /* Flags */
            PFF_HASALPHA | PFF_NATIVEENDIAN,
            /* Component type and count */
            PCT_BYTE, 4,
            /* rbits, gbits, bbits, abits */
            16, 16, 16, 16,
            /* Masks and shifts */
            0xFFFF000000000000ULL, 0x0000FFFF00000000ULL, 0x00000000FFFF0000ULL, 0x000000000000FFFFULL,
            48, 32, 16, 0
        },
        
        //-----------------------------------------------------------------------
        {"PF_ETC1_RGB8",
            /* Bytes per element */
            0,
            /* Flags */
            PFF_COMPRESSED,
            /* Component type and count */
            PCT_BYTE, 3,
            /* rbits, gbits, bbits, abits */
            0, 0, 0, 0,
            /* Masks and shifts */
            0, 0, 0, 0, 0, 0, 0, 0
        },
        //-----------------------------------------------------------------------
        {"PF_ETC2_RGB8",
            /* Bytes per element */
            0,
            /* Flags */
            PFF_COMPRESSED,
            /* Component type and count */
            PCT_BYTE, 3,
            /* rbits, gbits, bbits, abits */
            0, 0, 0, 0,
            /* Masks and shifts */
            0, 0, 0, 0, 0, 0, 0, 0
        },
        //-----------------------------------------------------------------------
        {"PF_ETC2_RGBA8",
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
        {"PF_ETC2_RGB8A1",
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
        {"PF_ATC_RGB",
            /* Bytes per element */
            0,
            /* Flags */
            PFF_COMPRESSED,
            /* Component type and count */
            PCT_BYTE, 3,
            /* rbits, gbits, bbits, abits */
            0, 0, 0, 0,
            /* Masks and shifts */
            0, 0, 0, 0, 0, 0, 0, 0
        },
        //-----------------------------------------------------------------------
        {"PF_ATC_RGBA_EXPLICIT_ALPHA",
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
        {"PF_ATC_RGBA_INTERPOLATED_ALPHA",
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
        {"PF_ASTC_RGBA_4X4_LDR",
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
        {"PF_ASTC_RGBA_5X4_LDR",
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
        {"PF_ASTC_RGBA_5X5_LDR",
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
        {"PF_ASTC_RGBA_6X5_LDR",
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
        {"PF_ASTC_RGBA_6X6_LDR",
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
        {"PF_ASTC_RGBA_8X5_LDR",
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
        {"PF_ASTC_RGBA_8X6_LDR",
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
        {"PF_ASTC_RGBA_8X8_LDR",
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
        {"PF_ASTC_RGBA_10X5_LDR",
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
        {"PF_ASTC_RGBA_10X6_LDR",
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
        {"PF_ASTC_RGBA_10X8_LDR",
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
        {"PF_ASTC_RGBA_10X10_LDR",
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
        {"PF_ASTC_RGBA_12X10_LDR",
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
        {"PF_ASTC_RGBA_12X12_LDR",
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
        {"PF_DEPTH32",
            /* Bytes per element */
            4,
            /* Flags */
            PFF_LUMINANCE | PFF_DEPTH | PFF_NATIVEENDIAN,
            /* Component type and count */
            PCT_UINT, 1,
            /* rbits, gbits, bbits, abits */
            32, 0, 0, 0,
            /* Masks and shifts */
            0, 0, 0, 0, 0, 0, 0, 0
        },
        //-----------------------------------------------------------------------
        {"PF_DEPTH32F",
            /* Bytes per element */
            4,
            /* Flags */
            PFF_LUMINANCE | PFF_DEPTH | PFF_NATIVEENDIAN,
            /* Component type and count */
            PCT_FLOAT32, 1,
            /* rbits, gbits, bbits, abits */
            32, 0, 0, 0,
            /* Masks and shifts */
            0, 0, 0, 0, 0, 0, 0, 0
        },
        {"PF_DEPTH24_STENCIL8",
            /* Bytes per element */
            4,
            /* Flags */
            PFF_LUMINANCE | PFF_DEPTH | PFF_NATIVEENDIAN,
            /* Component type and count */
            PCT_UINT, 1,
            /* rbits, gbits, bbits, abits */
            24, 0, 0, 0,
            /* Masks and shifts */
            0, 0, 0, 0, 0, 0, 0, 0
        },
    };
    /** @} */
    /** @} */

}

#endif
