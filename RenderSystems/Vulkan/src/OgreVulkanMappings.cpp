/*
  -----------------------------------------------------------------------------
  This source file is part of OGRE
  (Object-oriented Graphics Rendering Engine)
  For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-present Torus Knot Software Ltd

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

#include "OgreVulkanMappings.h"

#include "OgrePixelFormatGpuUtils.h"

namespace Ogre
{
    //-----------------------------------------------------------------------------------
    VkPrimitiveTopology VulkanMappings::get( OperationType opType )
    {
        switch( opType )
        {
            // clang-format off
        case OT_POINT_LIST:     return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case OT_LINE_LIST:      return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case OT_LINE_STRIP:     return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case OT_TRIANGLE_LIST:  return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case OT_TRIANGLE_STRIP: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case OT_TRIANGLE_FAN:   return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
        default:
            return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
            // clang-format on
        }
    }
    //-----------------------------------------------------------------------------------
    VkPolygonMode VulkanMappings::get( PolygonMode polygonMode )
    {
        switch( polygonMode )
        {
            // clang-format off
        case PM_POINTS:     return VK_POLYGON_MODE_POINT;
        case PM_WIREFRAME:  return VK_POLYGON_MODE_LINE;
        case PM_SOLID:      return VK_POLYGON_MODE_FILL;
            // clang-format on
        }
        return VK_POLYGON_MODE_FILL;
    }
    //-----------------------------------------------------------------------------------
    VkCullModeFlags VulkanMappings::get( CullingMode cullMode )
    {
        switch( cullMode )
        {
            // clang-format off
        case CULL_NONE:             return VK_CULL_MODE_NONE;
        case CULL_CLOCKWISE:        return VK_CULL_MODE_BACK_BIT;
        case CULL_ANTICLOCKWISE:    return VK_CULL_MODE_FRONT_BIT;
            // clang-format on
        }
        return VK_CULL_MODE_BACK_BIT;
    }
    //-----------------------------------------------------------------------------------
    VkCompareOp VulkanMappings::get( CompareFunction compareFunc )
    {
        switch( compareFunc )
        {
            // clang-format off
        case NUM_COMPARE_FUNCTIONS: return VK_COMPARE_OP_NEVER;
        case CMPF_ALWAYS_FAIL:      return VK_COMPARE_OP_NEVER;
        case CMPF_ALWAYS_PASS:      return VK_COMPARE_OP_ALWAYS;
        case CMPF_LESS:             return VK_COMPARE_OP_LESS;
        case CMPF_LESS_EQUAL:       return VK_COMPARE_OP_LESS_OR_EQUAL;
        case CMPF_EQUAL:            return VK_COMPARE_OP_EQUAL;
        case CMPF_NOT_EQUAL:        return VK_COMPARE_OP_NOT_EQUAL;
        case CMPF_GREATER_EQUAL:    return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case CMPF_GREATER:          return VK_COMPARE_OP_GREATER;
            // clang-format on
        }
        return VK_COMPARE_OP_NEVER;
    }
    //-----------------------------------------------------------------------------------
    VkStencilOp VulkanMappings::get( StencilOperation stencilOp )
    {
        switch( stencilOp )
        {
            // clang-format off
        case SOP_KEEP:              return VK_STENCIL_OP_KEEP;
        case SOP_ZERO:              return VK_STENCIL_OP_ZERO;
        case SOP_REPLACE:           return VK_STENCIL_OP_REPLACE;
        case SOP_INCREMENT:         return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
        case SOP_DECREMENT:         return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
        case SOP_INCREMENT_WRAP:    return VK_STENCIL_OP_INCREMENT_AND_WRAP;
        case SOP_DECREMENT_WRAP:    return VK_STENCIL_OP_DECREMENT_AND_WRAP;
        case SOP_INVERT:            return VK_STENCIL_OP_INVERT;
            // clang-format on
        }
        return VK_STENCIL_OP_KEEP;
    }
    //-----------------------------------------------------------------------------------
    VkBlendFactor VulkanMappings::get( SceneBlendFactor blendFactor )
    {
        switch( blendFactor )
        {
            // clang-format off
        case SBF_ONE:                       return VK_BLEND_FACTOR_ONE;
        case SBF_ZERO:                      return VK_BLEND_FACTOR_ZERO;
        case SBF_DEST_COLOUR:               return VK_BLEND_FACTOR_DST_COLOR;
        case SBF_SOURCE_COLOUR:             return VK_BLEND_FACTOR_SRC_COLOR;
        case SBF_ONE_MINUS_DEST_COLOUR:     return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case SBF_ONE_MINUS_SOURCE_COLOUR:   return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case SBF_DEST_ALPHA:                return VK_BLEND_FACTOR_DST_ALPHA;
        case SBF_SOURCE_ALPHA:              return VK_BLEND_FACTOR_SRC_ALPHA;
        case SBF_ONE_MINUS_DEST_ALPHA:      return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case SBF_ONE_MINUS_SOURCE_ALPHA:    return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            // clang-format on
        }
        return VK_BLEND_FACTOR_ONE;
    }
    //-----------------------------------------------------------------------------------
    VkBlendOp VulkanMappings::get( SceneBlendOperation blendOp )
    {
        switch( blendOp )
        {
            // clang-format off
        case SBO_ADD:               return VK_BLEND_OP_ADD;
        case SBO_SUBTRACT:          return VK_BLEND_OP_SUBTRACT;
        case SBO_REVERSE_SUBTRACT:  return VK_BLEND_OP_REVERSE_SUBTRACT;
        case SBO_MIN:               return VK_BLEND_OP_MIN;
        case SBO_MAX:               return VK_BLEND_OP_MAX;
            // clang-format on
        }
        return VK_BLEND_OP_ADD;
    }
    //-----------------------------------------------------------------------------------
    VkFormat VulkanMappings::get( VertexElementType vertexElemType )
    {
        switch( vertexElemType )
        {
        case VET_FLOAT1:
            return VK_FORMAT_R32_SFLOAT;
        case VET_FLOAT2:
            return VK_FORMAT_R32G32_SFLOAT;
        case VET_FLOAT3:
            return VK_FORMAT_R32G32B32_SFLOAT;
        case VET_FLOAT4:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        case VET_COLOUR:
        case VET_COLOUR_ARGB:
        case VET_COLOUR_ABGR:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case VET_SHORT2:
            return VK_FORMAT_R16G16_SINT;
        case VET_SHORT4:
            return VK_FORMAT_R16G16B16A16_SINT;
        case VET_UBYTE4:
            return VK_FORMAT_R8G8B8A8_UINT;
        case VET_USHORT2:
            return VK_FORMAT_R16G16_UINT;
        case VET_USHORT4:
            return VK_FORMAT_R16G16B16A16_UINT;
        case VET_INT1:
            return VK_FORMAT_R32_SINT;
        case VET_INT2:
            return VK_FORMAT_R32G32_SINT;
        case VET_INT3:
            return VK_FORMAT_R32G32B32_SINT;
        case VET_INT4:
            return VK_FORMAT_R32G32B32A32_SINT;
        case VET_UINT1:
            return VK_FORMAT_R32_UINT;
        case VET_UINT2:
            return VK_FORMAT_R32G32_UINT;
        case VET_UINT3:
            return VK_FORMAT_R32G32B32_UINT;
        case VET_UINT4:
            return VK_FORMAT_R32G32B32A32_SINT;
        case VET_BYTE4:
            return VK_FORMAT_R8G8B8A8_SINT;
        case VET_BYTE4_SNORM:
            return VK_FORMAT_R8G8B8A8_SNORM;
        case VET_UBYTE4_NORM:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case VET_SHORT2_SNORM:
            return VK_FORMAT_R16G16_SNORM;
        case VET_SHORT4_SNORM:
            return VK_FORMAT_R16G16B16A16_SNORM;
        case VET_USHORT2_NORM:
            return VK_FORMAT_R16G16_UNORM;
        case VET_USHORT4_NORM:
            return VK_FORMAT_R16G16B16A16_UNORM;
        case VET_HALF2:
            return VK_FORMAT_R16G16_SFLOAT;
        case VET_HALF4:
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        case VET_DOUBLE1:
        case VET_DOUBLE2:
        case VET_DOUBLE3:
        case VET_DOUBLE4:
        case VET_USHORT1_DEPRECATED:
        case VET_USHORT3_DEPRECATED:
            return VK_FORMAT_UNDEFINED;
        }

        return VK_FORMAT_UNDEFINED;
    }
    //-----------------------------------------------------------------------------------
    VkFilter VulkanMappings::get( FilterOptions filter )
    {
        switch( filter )
        {
            // clang-format off
        case FO_NONE:                   return VK_FILTER_NEAREST;
        case FO_POINT:                  return VK_FILTER_NEAREST;
        case FO_LINEAR:                 return VK_FILTER_LINEAR;
        case FO_ANISOTROPIC:            return VK_FILTER_LINEAR;
            // clang-format on
        }

        return VK_FILTER_NEAREST;
    }
    //-----------------------------------------------------------------------------------
    VkSamplerMipmapMode VulkanMappings::getMipFilter( FilterOptions filter )
    {
        switch( filter )
        {
            // clang-format off
        case FO_NONE:                   return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        case FO_POINT:                  return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        case FO_LINEAR:                 return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        case FO_ANISOTROPIC:            return VK_SAMPLER_MIPMAP_MODE_LINEAR;
            // clang-format on
        }

        return VK_SAMPLER_MIPMAP_MODE_NEAREST;
    }
    //-----------------------------------------------------------------------------------
    VkSamplerAddressMode VulkanMappings::get( TextureAddressingMode mode )
    {
        switch( mode )
        {
        case TAM_WRAP:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case TAM_MIRROR:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case TAM_CLAMP:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case TAM_BORDER:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        case TAM_UNKNOWN:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        }

        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    }
    //-----------------------------------------------------------------------------------
    VkImageViewType VulkanMappings::get( TextureTypes::TextureTypes textureType )
    {
        switch( textureType )
        {
        // clang-format off
        case TextureTypes::Unknown:         return VK_IMAGE_VIEW_TYPE_2D;
        case TextureTypes::Type1D:          return VK_IMAGE_VIEW_TYPE_1D;
        case TextureTypes::Type1DArray:     return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
        case TextureTypes::Type2D:          return VK_IMAGE_VIEW_TYPE_2D;
        case TextureTypes::Type2DArray:     return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        case TextureTypes::TypeCube:        return VK_IMAGE_VIEW_TYPE_CUBE;
        case TextureTypes::TypeCubeArray:   return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
        case TextureTypes::Type3D:          return VK_IMAGE_VIEW_TYPE_3D;
            // clang-format on
        }
        return VK_IMAGE_VIEW_TYPE_2D;
    }
    //-----------------------------------------------------------------------------------
    VkFormat VulkanMappings::get( PixelFormatGpu pf )
    {
        // clang-format off
        switch( pf )
        {
        case PFG_UNKNOWN:               return VK_FORMAT_UNDEFINED;
        case PFG_RGBA32_FLOAT:          return VK_FORMAT_R32G32B32A32_SFLOAT;
        case PFG_RGBA32_UINT:           return VK_FORMAT_R32G32B32A32_UINT;
        case PFG_RGBA32_SINT:           return VK_FORMAT_R32G32B32A32_SINT;
        case PFG_RGB32_FLOAT:           return VK_FORMAT_R32G32B32_SFLOAT;
        case PFG_RGB32_UINT:            return VK_FORMAT_R32G32B32_UINT;
        case PFG_RGB32_SINT:            return VK_FORMAT_R32G32B32_SINT;
        case PFG_RGBA16_FLOAT:          return VK_FORMAT_R16G16B16A16_SFLOAT;
        case PFG_RGBA16_UNORM:          return VK_FORMAT_R16G16B16A16_UNORM;
        case PFG_RGBA16_UINT:           return VK_FORMAT_R16G16B16A16_UINT;
        case PFG_RGBA16_SNORM:          return VK_FORMAT_R16G16B16A16_SNORM;
        case PFG_RGBA16_SINT:           return VK_FORMAT_R16G16B16A16_SINT;
        case PFG_RG32_FLOAT:            return VK_FORMAT_R32G32_SFLOAT;
        case PFG_RG32_UINT:             return VK_FORMAT_R32G32_UINT;
        case PFG_RG32_SINT:             return VK_FORMAT_R32G32_SINT;
        case PFG_D32_FLOAT_S8X24_UINT:  return VK_FORMAT_D32_SFLOAT_S8_UINT;
        case PFG_R10G10B10A2_UNORM:     return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
        case PFG_R10G10B10A2_UINT:      return VK_FORMAT_A2B10G10R10_UINT_PACK32;
        case PFG_R11G11B10_FLOAT:       return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
        case PFG_RGBA8_UNORM:           return VK_FORMAT_R8G8B8A8_UNORM;
        case PFG_RGBA8_UNORM_SRGB:      return VK_FORMAT_R8G8B8A8_SRGB;
        case PFG_RGBA8_UINT:            return VK_FORMAT_R8G8B8A8_UINT;
        case PFG_RGBA8_SNORM:           return VK_FORMAT_R8G8B8A8_SNORM;
        case PFG_RGBA8_SINT:            return VK_FORMAT_R8G8B8A8_SINT;
        case PFG_RG16_FLOAT:            return VK_FORMAT_R16G16_SFLOAT;
        case PFG_RG16_UNORM:            return VK_FORMAT_R16G16_UNORM;
        case PFG_RG16_UINT:             return VK_FORMAT_R16G16_UINT;
        case PFG_RG16_SNORM:            return VK_FORMAT_R16G16_SNORM;
        case PFG_RG16_SINT:             return VK_FORMAT_R16G16_SINT;
        case PFG_D32_FLOAT:             return VK_FORMAT_D32_SFLOAT;
        case PFG_R32_FLOAT:             return VK_FORMAT_R32_SFLOAT;
        case PFG_R32_UINT:              return VK_FORMAT_R32_UINT;
        case PFG_R32_SINT:              return VK_FORMAT_R32_SINT;
        case PFG_D24_UNORM:             return VK_FORMAT_X8_D24_UNORM_PACK32;
        case PFG_D24_UNORM_S8_UINT:     return VK_FORMAT_D24_UNORM_S8_UINT;
        case PFG_RG8_UNORM:             return VK_FORMAT_R8G8_UNORM;
        case PFG_RG8_UINT:              return VK_FORMAT_R8G8_UINT;
        case PFG_RG8_SNORM:             return VK_FORMAT_R8G8_SNORM;
        case PFG_RG8_SINT:              return VK_FORMAT_R8G8_SINT;
        case PFG_R16_FLOAT:             return VK_FORMAT_R16_SFLOAT;
        case PFG_D16_UNORM:             return VK_FORMAT_D16_UNORM;
        case PFG_R16_UNORM:             return VK_FORMAT_R16_UNORM;
        case PFG_R16_UINT:              return VK_FORMAT_R16_UINT;
        case PFG_R16_SNORM:             return VK_FORMAT_R16_SNORM;
        case PFG_R16_SINT:              return VK_FORMAT_R16_SINT;
        case PFG_R8_UNORM:              return VK_FORMAT_R8_UNORM;
        case PFG_R8_UINT:               return VK_FORMAT_R8_UINT;
        case PFG_R8_SNORM:              return VK_FORMAT_R8_SNORM;
        case PFG_R8_SINT:               return VK_FORMAT_R8_SINT;
        case PFG_R9G9B9E5_SHAREDEXP:    return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
        case PFG_R8G8_B8G8_UNORM:       return VK_FORMAT_B8G8R8G8_422_UNORM;
        case PFG_G8R8_G8B8_UNORM:       return VK_FORMAT_G8B8G8R8_422_UNORM;
        case PFG_BC1_UNORM:             return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        case PFG_BC1_UNORM_SRGB:        return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
        case PFG_BC2_UNORM:             return VK_FORMAT_BC2_UNORM_BLOCK;
        case PFG_BC2_UNORM_SRGB:        return VK_FORMAT_BC2_SRGB_BLOCK;
        case PFG_BC3_UNORM:             return VK_FORMAT_BC3_UNORM_BLOCK;
        case PFG_BC3_UNORM_SRGB:        return VK_FORMAT_BC3_SRGB_BLOCK;
        case PFG_BC4_UNORM:             return VK_FORMAT_BC4_UNORM_BLOCK;
        case PFG_BC4_SNORM:             return VK_FORMAT_BC4_SNORM_BLOCK;
        case PFG_BC5_UNORM:             return VK_FORMAT_BC5_UNORM_BLOCK;
        case PFG_BC5_SNORM:             return VK_FORMAT_BC5_SNORM_BLOCK;
        case PFG_B5G6R5_UNORM:          return VK_FORMAT_B5G6R5_UNORM_PACK16;
        case PFG_B5G5R5A1_UNORM:        return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
        case PFG_BGRA8_UNORM:           return VK_FORMAT_B8G8R8A8_UNORM;
        case PFG_BGRX8_UNORM:           return VK_FORMAT_B8G8R8A8_UNORM;
        case PFG_R10G10B10_XR_BIAS_A2_UNORM:return VK_FORMAT_A2R10G10B10_USCALED_PACK32;
        case PFG_BGRA8_UNORM_SRGB:      return VK_FORMAT_B8G8R8A8_SRGB;
        case PFG_BGRX8_UNORM_SRGB:      return VK_FORMAT_B8G8R8A8_SRGB;
        case PFG_BC6H_UF16:             return VK_FORMAT_BC6H_UFLOAT_BLOCK;
        case PFG_BC6H_SF16:             return VK_FORMAT_BC6H_SFLOAT_BLOCK;
        case PFG_BC7_UNORM:             return VK_FORMAT_BC7_UNORM_BLOCK;
        case PFG_BC7_UNORM_SRGB:        return VK_FORMAT_BC7_SRGB_BLOCK;
        case PFG_B4G4R4A4_UNORM:        return VK_FORMAT_B4G4R4A4_UNORM_PACK16;
        case PFG_PVRTC_RGB2:            return VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG;
        case PFG_PVRTC_RGBA2:           return VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG;
        case PFG_PVRTC_RGB4:            return VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG;
        case PFG_PVRTC_RGBA4:           return VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG;
        case PFG_PVRTC2_2BPP:           return VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG;
        case PFG_PVRTC2_4BPP:           return VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG;
        case PFG_ETC1_RGB8_UNORM:       return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
        case PFG_ETC2_RGB8_UNORM:       return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
        case PFG_ETC2_RGB8_UNORM_SRGB:  return VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
        case PFG_ETC2_RGBA8_UNORM:      return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
        case PFG_ETC2_RGBA8_UNORM_SRGB: return VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;
        case PFG_ETC2_RGB8A1_UNORM:     return VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
        case PFG_ETC2_RGB8A1_UNORM_SRGB:return VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;
        case PFG_EAC_R11_UNORM:         return VK_FORMAT_EAC_R11_UNORM_BLOCK;
        case PFG_EAC_R11_SNORM:         return VK_FORMAT_EAC_R11_SNORM_BLOCK;
        case PFG_EAC_R11G11_UNORM:      return VK_FORMAT_EAC_R11G11_UNORM_BLOCK;
        case PFG_EAC_R11G11_SNORM:      return VK_FORMAT_EAC_R11G11_SNORM_BLOCK;

        case PFG_ASTC_RGBA_UNORM_4X4_LDR:   return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
        case PFG_ASTC_RGBA_UNORM_5X4_LDR:   return VK_FORMAT_ASTC_5x4_UNORM_BLOCK;
        case PFG_ASTC_RGBA_UNORM_5X5_LDR:   return VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
        case PFG_ASTC_RGBA_UNORM_6X5_LDR:   return VK_FORMAT_ASTC_6x5_UNORM_BLOCK;
        case PFG_ASTC_RGBA_UNORM_6X6_LDR:   return VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
        case PFG_ASTC_RGBA_UNORM_8X5_LDR:   return VK_FORMAT_ASTC_8x5_UNORM_BLOCK;
        case PFG_ASTC_RGBA_UNORM_8X6_LDR:   return VK_FORMAT_ASTC_8x6_UNORM_BLOCK;
        case PFG_ASTC_RGBA_UNORM_8X8_LDR:   return VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
        case PFG_ASTC_RGBA_UNORM_10X5_LDR:  return VK_FORMAT_ASTC_10x5_UNORM_BLOCK;
        case PFG_ASTC_RGBA_UNORM_10X6_LDR:  return VK_FORMAT_ASTC_10x6_UNORM_BLOCK;
        case PFG_ASTC_RGBA_UNORM_10X8_LDR:  return VK_FORMAT_ASTC_10x8_UNORM_BLOCK;
        case PFG_ASTC_RGBA_UNORM_10X10_LDR: return VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
        case PFG_ASTC_RGBA_UNORM_12X10_LDR: return VK_FORMAT_ASTC_12x10_UNORM_BLOCK;
        case PFG_ASTC_RGBA_UNORM_12X12_LDR: return VK_FORMAT_ASTC_12x12_UNORM_BLOCK;

        case PFG_ASTC_RGBA_UNORM_4X4_sRGB:  return VK_FORMAT_ASTC_4x4_SRGB_BLOCK;
        case PFG_ASTC_RGBA_UNORM_5X4_sRGB:  return VK_FORMAT_ASTC_5x4_SRGB_BLOCK;
        case PFG_ASTC_RGBA_UNORM_5X5_sRGB:  return VK_FORMAT_ASTC_5x5_SRGB_BLOCK;
        case PFG_ASTC_RGBA_UNORM_6X5_sRGB:  return VK_FORMAT_ASTC_6x5_SRGB_BLOCK;
        case PFG_ASTC_RGBA_UNORM_6X6_sRGB:  return VK_FORMAT_ASTC_6x6_SRGB_BLOCK;
        case PFG_ASTC_RGBA_UNORM_8X5_sRGB:  return VK_FORMAT_ASTC_8x5_SRGB_BLOCK;
        case PFG_ASTC_RGBA_UNORM_8X6_sRGB:  return VK_FORMAT_ASTC_8x6_SRGB_BLOCK;
        case PFG_ASTC_RGBA_UNORM_8X8_sRGB:  return VK_FORMAT_ASTC_8x8_SRGB_BLOCK;
        case PFG_ASTC_RGBA_UNORM_10X5_sRGB: return VK_FORMAT_ASTC_10x5_SRGB_BLOCK;
        case PFG_ASTC_RGBA_UNORM_10X6_sRGB: return VK_FORMAT_ASTC_10x6_SRGB_BLOCK;
        case PFG_ASTC_RGBA_UNORM_10X8_sRGB: return VK_FORMAT_ASTC_10x8_SRGB_BLOCK;
        case PFG_ASTC_RGBA_UNORM_10X10_sRGB:return VK_FORMAT_ASTC_10x10_SRGB_BLOCK;
        case PFG_ASTC_RGBA_UNORM_12X10_sRGB:return VK_FORMAT_ASTC_12x10_SRGB_BLOCK;
        case PFG_ASTC_RGBA_UNORM_12X12_sRGB:return VK_FORMAT_ASTC_12x12_SRGB_BLOCK;

        case PFG_ATC_RGB:
        case PFG_ATC_RGBA_EXPLICIT_ALPHA:
        case PFG_ATC_RGBA_INTERPOLATED_ALPHA:
        case PFG_A8_UNORM:
        case PFG_R1_UNORM:
        case PFG_AYUV:
        case PFG_Y410:
        case PFG_Y416:
        case PFG_NV12:
        case PFG_P010:
        case PFG_P016:
        case PFG_420_OPAQUE:
        case PFG_YUY2:
        case PFG_Y210:
        case PFG_Y216:
        case PFG_NV11:
        case PFG_AI44:
        case PFG_IA44:
        case PFG_P8:
        case PFG_A8P8:
        case PFG_P208:
        case PFG_V208:
        case PFG_V408:
        case PFG_COUNT:
        default:
            return VK_FORMAT_UNDEFINED;
        }
        // clang-format on
    }
    //-----------------------------------------------------------------------------------
    VkImageAspectFlags VulkanMappings::getImageAspect( PixelFormatGpu pf,
                                                       const bool bPreferDepthOverStencil )
    {
        const uint32 pfFlags = PixelFormatGpuUtils::getFlags( pf );

        VkImageAspectFlags retVal = 0;
        if( pfFlags & ( PixelFormatGpuUtils::PFF_DEPTH | PixelFormatGpuUtils::PFF_STENCIL ) )
        {
            if( pfFlags & PixelFormatGpuUtils::PFF_DEPTH )
                retVal = VK_IMAGE_ASPECT_DEPTH_BIT;
            if( pfFlags & PixelFormatGpuUtils::PFF_STENCIL )
            {
                if( !bPreferDepthOverStencil || !( pfFlags & PixelFormatGpuUtils::PFF_DEPTH ) )
                    retVal |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        }
        else
        {
            retVal = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    VkAccessFlags VulkanMappings::get( const TextureGpu *texture )
    {
        VkAccessFlags texAccessFlags = 0;

        if( texture->isTexture() || texture->isUav() )
        {
            texAccessFlags |= VK_ACCESS_SHADER_READ_BIT;
            if( texture->isUav() )
                texAccessFlags |= VK_ACCESS_SHADER_WRITE_BIT;
        }
        if( texture->isRenderToTexture() )
        {
            // texAccessFlags |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
            if( !PixelFormatGpuUtils::isDepth( texture->getPixelFormat() ) )
            {
                texAccessFlags |=
                    VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            }
            else
            {
                texAccessFlags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            }
        }

        return texAccessFlags;
    }
    //-----------------------------------------------------------------------------------
    VkAccessFlags VulkanMappings::get( BufferPackedTypes bufferPackedTypes )
    {
        switch( bufferPackedTypes )
        {
            // clang-format off
        case BP_TYPE_VERTEX:    return VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
        case BP_TYPE_INDEX:     return VK_ACCESS_INDEX_READ_BIT;
        case BP_TYPE_CONST:     return VK_ACCESS_UNIFORM_READ_BIT;
        case BP_TYPE_TEX:       return VK_ACCESS_SHADER_READ_BIT;
        case BP_TYPE_READONLY:  return VK_ACCESS_SHADER_READ_BIT;
        case BP_TYPE_INDIRECT:  return VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
        case BP_TYPE_UAV:       return VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
        case NUM_BUFFER_PACKED_TYPES: return 0;  // Keep compiler happy
            // clang-format on
        }

        return 0;
    }
    //-----------------------------------------------------------------------------------
    VkAccessFlags VulkanMappings::getAccessFlags( ResourceLayout::Layout layout,
                                                  ResourceAccess::ResourceAccess access,
                                                  const TextureGpu *texture, bool bIsDst )
    {
        VkAccessFlags texAccessFlags = 0;

        switch( layout )
        {
        case ResourceLayout::Texture:
            texAccessFlags |= VK_ACCESS_SHADER_READ_BIT;
            break;
        case ResourceLayout::RenderTarget:
        case ResourceLayout::Clear:
            if( !PixelFormatGpuUtils::isDepth( texture->getPixelFormat() ) )
            {
                texAccessFlags |=
                    VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            }
            else
            {
                texAccessFlags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            }
            break;
        case ResourceLayout::RenderTargetReadOnly:
            if( !PixelFormatGpuUtils::isDepth( texture->getPixelFormat() ) )
                texAccessFlags |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
            else
                texAccessFlags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
            break;
        case ResourceLayout::Uav:
            if( access & ResourceAccess::Read )
                texAccessFlags |= VK_ACCESS_SHADER_READ_BIT;
            if( access & ResourceAccess::Write )
                texAccessFlags |= VK_ACCESS_SHADER_WRITE_BIT;
            break;
        case ResourceLayout::CopySrc:
            texAccessFlags |= VK_ACCESS_TRANSFER_READ_BIT;
            break;
        case ResourceLayout::CopyDst:
            texAccessFlags |= VK_ACCESS_TRANSFER_WRITE_BIT;
            break;
        case ResourceLayout::CopyEnd:
            texAccessFlags = 0u;
            break;
        case ResourceLayout::ResolveDest:
            texAccessFlags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;
        case ResourceLayout::PresentReady:
            texAccessFlags = 0u;
            break;
        case ResourceLayout::MipmapGen:
            if( bIsDst )
                texAccessFlags |= VK_ACCESS_TRANSFER_READ_BIT;
            else
                texAccessFlags |= VK_ACCESS_TRANSFER_WRITE_BIT;
            break;
        case ResourceLayout::Undefined:
        case ResourceLayout::NumResourceLayouts:
            break;
        }

        return texAccessFlags;
    }
    //-----------------------------------------------------------------------------------
    VkImageLayout VulkanMappings::get( ResourceLayout::Layout layout, const TextureGpu *texture )
    {
        switch( layout )
        {
        case ResourceLayout::Undefined:
        case ResourceLayout::NumResourceLayouts:
            return VK_IMAGE_LAYOUT_UNDEFINED;
        case ResourceLayout::Texture:
            return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        case ResourceLayout::RenderTarget:
            return PixelFormatGpuUtils::isDepth( texture->getPixelFormat() )
                       ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
                       : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case ResourceLayout::RenderTargetReadOnly:
            return PixelFormatGpuUtils::isDepth( texture->getPixelFormat() )
                       ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
                       : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case ResourceLayout::ResolveDest:
            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case ResourceLayout::Clear:
            return PixelFormatGpuUtils::isDepth( texture->getPixelFormat() )
                       ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
                       : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case ResourceLayout::Uav:
            return VK_IMAGE_LAYOUT_GENERAL;
        case ResourceLayout::CopySrc:
            return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        case ResourceLayout::CopyDst:
            return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        case ResourceLayout::CopyEnd:
            return get( texture->getDefaultLayout( true ), texture );
        case ResourceLayout::MipmapGen:
            return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        case ResourceLayout::PresentReady:
            return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        }

        return VK_IMAGE_LAYOUT_UNDEFINED;
    }
    //-----------------------------------------------------------------------------------
    uint32_t VulkanMappings::getFormatSize( VkFormat format )
    {
        uint32_t result = 0;
        switch( format )
        {
        case VK_FORMAT_UNDEFINED:
            result = 0;
            break;
        case VK_FORMAT_R4G4_UNORM_PACK8:
            result = 1;
            break;
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
            result = 2;
            break;
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
            result = 2;
            break;
        case VK_FORMAT_R5G6B5_UNORM_PACK16:
            result = 2;
            break;
        case VK_FORMAT_B5G6R5_UNORM_PACK16:
            result = 2;
            break;
        case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
            result = 2;
            break;
        case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
            result = 2;
            break;
        case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
            result = 2;
            break;
        case VK_FORMAT_R8_UNORM:
            result = 1;
            break;
        case VK_FORMAT_R8_SNORM:
            result = 1;
            break;
        case VK_FORMAT_R8_USCALED:
            result = 1;
            break;
        case VK_FORMAT_R8_SSCALED:
            result = 1;
            break;
        case VK_FORMAT_R8_UINT:
            result = 1;
            break;
        case VK_FORMAT_R8_SINT:
            result = 1;
            break;
        case VK_FORMAT_R8_SRGB:
            result = 1;
            break;
        case VK_FORMAT_R8G8_UNORM:
            result = 2;
            break;
        case VK_FORMAT_R8G8_SNORM:
            result = 2;
            break;
        case VK_FORMAT_R8G8_USCALED:
            result = 2;
            break;
        case VK_FORMAT_R8G8_SSCALED:
            result = 2;
            break;
        case VK_FORMAT_R8G8_UINT:
            result = 2;
            break;
        case VK_FORMAT_R8G8_SINT:
            result = 2;
            break;
        case VK_FORMAT_R8G8_SRGB:
            result = 2;
            break;
        case VK_FORMAT_R8G8B8_UNORM:
            result = 3;
            break;
        case VK_FORMAT_R8G8B8_SNORM:
            result = 3;
            break;
        case VK_FORMAT_R8G8B8_USCALED:
            result = 3;
            break;
        case VK_FORMAT_R8G8B8_SSCALED:
            result = 3;
            break;
        case VK_FORMAT_R8G8B8_UINT:
            result = 3;
            break;
        case VK_FORMAT_R8G8B8_SINT:
            result = 3;
            break;
        case VK_FORMAT_R8G8B8_SRGB:
            result = 3;
            break;
        case VK_FORMAT_B8G8R8_UNORM:
            result = 3;
            break;
        case VK_FORMAT_B8G8R8_SNORM:
            result = 3;
            break;
        case VK_FORMAT_B8G8R8_USCALED:
            result = 3;
            break;
        case VK_FORMAT_B8G8R8_SSCALED:
            result = 3;
            break;
        case VK_FORMAT_B8G8R8_UINT:
            result = 3;
            break;
        case VK_FORMAT_B8G8R8_SINT:
            result = 3;
            break;
        case VK_FORMAT_B8G8R8_SRGB:
            result = 3;
            break;
        case VK_FORMAT_R8G8B8A8_UNORM:
            result = 4;
            break;
        case VK_FORMAT_R8G8B8A8_SNORM:
            result = 4;
            break;
        case VK_FORMAT_R8G8B8A8_USCALED:
            result = 4;
            break;
        case VK_FORMAT_R8G8B8A8_SSCALED:
            result = 4;
            break;
        case VK_FORMAT_R8G8B8A8_UINT:
            result = 4;
            break;
        case VK_FORMAT_R8G8B8A8_SINT:
            result = 4;
            break;
        case VK_FORMAT_R8G8B8A8_SRGB:
            result = 4;
            break;
        case VK_FORMAT_B8G8R8A8_UNORM:
            result = 4;
            break;
        case VK_FORMAT_B8G8R8A8_SNORM:
            result = 4;
            break;
        case VK_FORMAT_B8G8R8A8_USCALED:
            result = 4;
            break;
        case VK_FORMAT_B8G8R8A8_SSCALED:
            result = 4;
            break;
        case VK_FORMAT_B8G8R8A8_UINT:
            result = 4;
            break;
        case VK_FORMAT_B8G8R8A8_SINT:
            result = 4;
            break;
        case VK_FORMAT_B8G8R8A8_SRGB:
            result = 4;
            break;
        case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
            result = 4;
            break;
        case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
            result = 4;
            break;
        case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
            result = 4;
            break;
        case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
            result = 4;
            break;
        case VK_FORMAT_A8B8G8R8_UINT_PACK32:
            result = 4;
            break;
        case VK_FORMAT_A8B8G8R8_SINT_PACK32:
            result = 4;
            break;
        case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
            result = 4;
            break;
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
            result = 4;
            break;
        case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
            result = 4;
            break;
        case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
            result = 4;
            break;
        case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
            result = 4;
            break;
        case VK_FORMAT_A2R10G10B10_UINT_PACK32:
            result = 4;
            break;
        case VK_FORMAT_A2R10G10B10_SINT_PACK32:
            result = 4;
            break;
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
            result = 4;
            break;
        case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
            result = 4;
            break;
        case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
            result = 4;
            break;
        case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
            result = 4;
            break;
        case VK_FORMAT_A2B10G10R10_UINT_PACK32:
            result = 4;
            break;
        case VK_FORMAT_A2B10G10R10_SINT_PACK32:
            result = 4;
            break;
        case VK_FORMAT_R16_UNORM:
            result = 2;
            break;
        case VK_FORMAT_R16_SNORM:
            result = 2;
            break;
        case VK_FORMAT_R16_USCALED:
            result = 2;
            break;
        case VK_FORMAT_R16_SSCALED:
            result = 2;
            break;
        case VK_FORMAT_R16_UINT:
            result = 2;
            break;
        case VK_FORMAT_R16_SINT:
            result = 2;
            break;
        case VK_FORMAT_R16_SFLOAT:
            result = 2;
            break;
        case VK_FORMAT_R16G16_UNORM:
            result = 4;
            break;
        case VK_FORMAT_R16G16_SNORM:
            result = 4;
            break;
        case VK_FORMAT_R16G16_USCALED:
            result = 4;
            break;
        case VK_FORMAT_R16G16_SSCALED:
            result = 4;
            break;
        case VK_FORMAT_R16G16_UINT:
            result = 4;
            break;
        case VK_FORMAT_R16G16_SINT:
            result = 4;
            break;
        case VK_FORMAT_R16G16_SFLOAT:
            result = 4;
            break;
        case VK_FORMAT_R16G16B16_UNORM:
            result = 6;
            break;
        case VK_FORMAT_R16G16B16_SNORM:
            result = 6;
            break;
        case VK_FORMAT_R16G16B16_USCALED:
            result = 6;
            break;
        case VK_FORMAT_R16G16B16_SSCALED:
            result = 6;
            break;
        case VK_FORMAT_R16G16B16_UINT:
            result = 6;
            break;
        case VK_FORMAT_R16G16B16_SINT:
            result = 6;
            break;
        case VK_FORMAT_R16G16B16_SFLOAT:
            result = 6;
            break;
        case VK_FORMAT_R16G16B16A16_UNORM:
            result = 8;
            break;
        case VK_FORMAT_R16G16B16A16_SNORM:
            result = 8;
            break;
        case VK_FORMAT_R16G16B16A16_USCALED:
            result = 8;
            break;
        case VK_FORMAT_R16G16B16A16_SSCALED:
            result = 8;
            break;
        case VK_FORMAT_R16G16B16A16_UINT:
            result = 8;
            break;
        case VK_FORMAT_R16G16B16A16_SINT:
            result = 8;
            break;
        case VK_FORMAT_R16G16B16A16_SFLOAT:
            result = 8;
            break;
        case VK_FORMAT_R32_UINT:
            result = 4;
            break;
        case VK_FORMAT_R32_SINT:
            result = 4;
            break;
        case VK_FORMAT_R32_SFLOAT:
            result = 4;
            break;
        case VK_FORMAT_R32G32_UINT:
            result = 8;
            break;
        case VK_FORMAT_R32G32_SINT:
            result = 8;
            break;
        case VK_FORMAT_R32G32_SFLOAT:
            result = 8;
            break;
        case VK_FORMAT_R32G32B32_UINT:
            result = 12;
            break;
        case VK_FORMAT_R32G32B32_SINT:
            result = 12;
            break;
        case VK_FORMAT_R32G32B32_SFLOAT:
            result = 12;
            break;
        case VK_FORMAT_R32G32B32A32_UINT:
            result = 16;
            break;
        case VK_FORMAT_R32G32B32A32_SINT:
            result = 16;
            break;
        case VK_FORMAT_R32G32B32A32_SFLOAT:
            result = 16;
            break;
        case VK_FORMAT_R64_UINT:
            result = 8;
            break;
        case VK_FORMAT_R64_SINT:
            result = 8;
            break;
        case VK_FORMAT_R64_SFLOAT:
            result = 8;
            break;
        case VK_FORMAT_R64G64_UINT:
            result = 16;
            break;
        case VK_FORMAT_R64G64_SINT:
            result = 16;
            break;
        case VK_FORMAT_R64G64_SFLOAT:
            result = 16;
            break;
        case VK_FORMAT_R64G64B64_UINT:
            result = 24;
            break;
        case VK_FORMAT_R64G64B64_SINT:
            result = 24;
            break;
        case VK_FORMAT_R64G64B64_SFLOAT:
            result = 24;
            break;
        case VK_FORMAT_R64G64B64A64_UINT:
            result = 32;
            break;
        case VK_FORMAT_R64G64B64A64_SINT:
            result = 32;
            break;
        case VK_FORMAT_R64G64B64A64_SFLOAT:
            result = 32;
            break;
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
            result = 4;
            break;
        case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
            result = 4;
            break;

        default:
            break;
        }
        return result;
    }
    //-----------------------------------------------------------------------------------
    GpuConstantType VulkanMappings::get( SpvOp op )
    {
        switch( op )
        {
        case SpvOpTypeBool:
            return GCT_BOOL1;
        case SpvOpTypeInt:
            return GCT_INT1;
        case SpvOpTypeFloat:
            return GCT_FLOAT1;
        case SpvOpTypeMatrix:
            return GCT_MATRIX_4X4;  // Need to check for actual number of rows and columns
        case SpvOpTypeImage:
            return GCT_SAMPLER2D;  // Need to check for actual sampler dimensions
        case SpvOpTypeSampler:
            return GCT_SAMPLER2D;  // Need to check for actual sampler dimensions
        case SpvOpTypeSampledImage:
            return GCT_SAMPLER2D;  // Need to check for actual sampler dimensions
        case SpvOpTypeArray:
            return GCT_UNKNOWN;
        case SpvOpTypeStruct:
            return GCT_UNKNOWN;
        default:
            return GCT_UNKNOWN;
        }
    }
    //-----------------------------------------------------------------------------------
    VertexElementSemantic VulkanMappings::getHlslSemantic( const char *sem )
    {
        if( strcmp( sem, "input.blendIndices" ) == 0 )
            return VES_BLEND_INDICES;
        if( strcmp( sem, "input.blendWeigth" ) == 0 )
            return VES_BLEND_WEIGHTS;
        if( strcmp( sem, "input.colour" ) == 0 )
            return VES_DIFFUSE;
        //      if( strcmp(sem, "COLOR") == 0 )
        //          return VES_SPECULAR;
        if( strcmp( sem, "input.normal" ) == 0 )
            return VES_NORMAL;
        if( strcmp( sem, "input.vertex" ) == 0 )
            return VES_POSITION;
        if( strncmp( sem, "input.uv", 8 ) == 0 )
            return VES_TEXTURE_COORDINATES;
        if( strcmp( sem, "input.binormal" ) == 0 )
            return VES_BINORMAL;
        if( strcmp( sem, "input.qtangent" ) == 0 || strcmp( sem, "input.tangent" ) == 0 )
            return VES_TANGENT;

        // to keep compiler happy
        return VES_POSITION;
    }
}  // namespace Ogre
