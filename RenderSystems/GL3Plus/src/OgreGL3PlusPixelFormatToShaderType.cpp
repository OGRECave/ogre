/*
  -----------------------------------------------------------------------------
  This source file is part of OGRE
  (Object-oriented Graphics Rendering Engine)
  For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2016 Torus Knot Software Ltd

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

#include "OgreGL3PlusPixelFormatToShaderType.h"

namespace Ogre
{
    const char* GL3PlusPixelFormatToShaderType::getPixelFormatType( PixelFormat pixelFormat ) const
    {
        switch( pixelFormat )
        {
        //UNORM formats
        case PF_L8:
        case PF_A8:
        case PF_R8:
            return "r8";
        case PF_L16:
            return "r16";
        case PF_RG8:
        case PF_BYTE_LA:
            return "rg8";
        case PF_SHORT_GR:
            return "rg16";
        case PF_R8G8B8:
        case PF_B8G8R8:
        case PF_A8R8G8B8:
        case PF_A8B8G8R8:
        case PF_B8G8R8A8:
        case PF_R8G8B8A8:
        case PF_X8R8G8B8:
        case PF_X8B8G8R8:
            return "rgba8";
        case PF_A2R10G10B10:
        case PF_A2B10G10R10:
            return "rgb10_a2";
        case PF_SHORT_RGB:
        case PF_SHORT_RGBA:
            return "rgba16";

        //SNORM formats
        case PF_R8_SNORM:
            return "r8_snorm";
        case PF_R16_SNORM:
            return "r16_snorm";
        case PF_R8G8_SNORM:
            return "rg8_snorm";
        case PF_R16G16_SNORM:
            return "rg16_snorm";
        case PF_R8G8B8_SNORM:
        case PF_R8G8B8A8_SNORM:
            return "rgba8_snorm";
        case PF_R16G16B16_SNORM:
        case PF_R16G16B16A16_SNORM:
            return "rgba16_snorm";

        //SINT formats
        case PF_R8_SINT:
            return "r8i";
        case PF_R16_SINT:
            return "r16i";
        case PF_R32_SINT:
            return "r32i";
        case PF_R8G8_SINT:
            return "rg8i";
        case PF_R16G16_SINT:
            return "rg16i";
        case PF_R32G32_SINT:
            return "rg32i";
        case PF_R8G8B8_SINT:
        case PF_R8G8B8A8_SINT:
            return "rgba8i";
        case PF_R16G16B16_SINT:
        case PF_R16G16B16A16_SINT:
            return "rgba16i";
        case PF_R32G32B32_SINT:
        case PF_R32G32B32A32_SINT:
            return "rgba32i";

        //UINT formats
        case PF_R8_UINT:
            return "r8ui";
        case PF_R16_UINT:
            return "r16ui";
        case PF_R32_UINT:
            return "r32ui";
        case PF_R8G8_UINT:
            return "rg8ui";
        case PF_R16G16_UINT:
            return "rg16ui";
        case PF_R32G32_UINT:
            return "rg32ui";
        case PF_R8G8B8_UINT:
        case PF_R8G8B8A8_UINT:
            return "rgba8ui";
//        case PF_R10G10B10A2_UINT:
//            return "rgb10_a2ui";
        case PF_R16G16B16_UINT:
        case PF_R16G16B16A16_UINT:
            return "rgba16ui";
        case PF_R32G32B32_UINT:
        case PF_R32G32B32A32_UINT:
            return "rgba32ui";

        //Pure floating point
        case PF_FLOAT16_R:
            return "r16f";
        case PF_FLOAT32_R:
            return "r32f";
        case PF_FLOAT16_GR:
            return "rg16f";
        case PF_FLOAT32_GR:
            return "rg32f";
        case PF_R11G11B10_FLOAT:
            return "r11f_g11f_b10f";
        case PF_FLOAT16_RGB:
        case PF_FLOAT16_RGBA:
            return "rgba16f";
        case PF_FLOAT32_RGB:
        case PF_FLOAT32_RGBA:
            return "rgba32f";
        default:
            return 0;
        }

        return 0;
    }
}
