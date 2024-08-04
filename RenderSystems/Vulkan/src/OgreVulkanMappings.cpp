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

#include "OgrePixelFormat.h"

namespace Ogre
{
    //-----------------------------------------------------------------------------------
    VkPrimitiveTopology VulkanMappings::get( RenderOperation::OperationType opType )
    {
        switch( opType )
        {
            // clang-format off
        case RenderOperation::OT_POINT_LIST:     return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case RenderOperation::OT_LINE_LIST:      return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case RenderOperation::OT_LINE_STRIP:     return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case RenderOperation::OT_TRIANGLE_LIST:  return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case RenderOperation::OT_TRIANGLE_STRIP: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case RenderOperation::OT_TRIANGLE_FAN:   return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
        case RenderOperation::OT_LINE_LIST_ADJ:  return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
        case RenderOperation::OT_LINE_STRIP_ADJ: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
        case RenderOperation::OT_TRIANGLE_LIST_ADJ:  return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
        case RenderOperation::OT_TRIANGLE_STRIP_ADJ: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
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
        case VET_SHORT1:
            return VK_FORMAT_R16_SINT;
        case VET_SHORT2:
            return VK_FORMAT_R16G16_SINT;
        case VET_SHORT3:
            return VK_FORMAT_R16G16B16_SINT;
        case VET_SHORT4:
            return VK_FORMAT_R16G16B16A16_SINT;
        case VET_UBYTE4:
            return VK_FORMAT_R8G8B8A8_UINT;
        case VET_USHORT1:
            return VK_FORMAT_R16_UINT;
        case VET_USHORT2:
            return VK_FORMAT_R16G16_UINT;
        case VET_USHORT3:
            return VK_FORMAT_R16G16B16_UINT;
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
        case VET_BYTE4_NORM:
            return VK_FORMAT_R8G8B8A8_SNORM;
        case VET_UBYTE4_NORM:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case VET_SHORT2_NORM:
            return VK_FORMAT_R16G16_SNORM;
        case VET_SHORT4_NORM:
            return VK_FORMAT_R16G16B16A16_SNORM;
        case VET_USHORT2_NORM:
            return VK_FORMAT_R16G16_UNORM;
        case VET_USHORT4_NORM:
            return VK_FORMAT_R16G16B16A16_UNORM;
        case VET_HALF1:
            return VK_FORMAT_R16_SFLOAT;
        case VET_HALF2:
            return VK_FORMAT_R16G16_SFLOAT;
        case VET_HALF3:
            return VK_FORMAT_R16G16B16_SFLOAT;
        case VET_HALF4:
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        case VET_INT_10_10_10_2_NORM:
            return VK_FORMAT_A2B10G10R10_SNORM_PACK32;
        case _DETAIL_SWAP_RB:
        case VET_DOUBLE1:
        case VET_DOUBLE2:
        case VET_DOUBLE3:
        case VET_DOUBLE4:
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
        }

        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    }
    //-----------------------------------------------------------------------------------
    VkImageViewType VulkanMappings::get( TextureType textureType )
    {
        switch( textureType )
        {
        // clang-format off
        case TEX_TYPE_1D:          return VK_IMAGE_VIEW_TYPE_1D;
        //case TextureTypes::Type1DArray:     return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
        case TEX_TYPE_2D:          return VK_IMAGE_VIEW_TYPE_2D;
        case TEX_TYPE_2D_ARRAY:     return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        case TEX_TYPE_CUBE_MAP:        return VK_IMAGE_VIEW_TYPE_CUBE;
        //case TextureTypes::TypeCubeArray:   return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
        case TEX_TYPE_3D:          return VK_IMAGE_VIEW_TYPE_3D;
            // clang-format on
        case TEX_TYPE_EXTERNAL_OES:
            break;
        }
        return VK_IMAGE_VIEW_TYPE_2D;
    }
    //-----------------------------------------------------------------------------------
    VkFormat VulkanMappings::get( PixelFormat pf, bool hwGamma )
    {
        // clang-format off
        switch( pf )
        {
        case PF_FLOAT32_RGBA:          return VK_FORMAT_R32G32B32A32_SFLOAT;
        case PF_R32G32B32A32_UINT:           return VK_FORMAT_R32G32B32A32_UINT;
        case PF_R32G32B32A32_SINT:           return VK_FORMAT_R32G32B32A32_SINT;
        case PF_FLOAT32_RGB:           return VK_FORMAT_R32G32B32_SFLOAT;
        case PF_R32G32B32_UINT:            return VK_FORMAT_R32G32B32_UINT;
        case PF_R32G32B32_SINT:            return VK_FORMAT_R32G32B32_SINT;
        case PF_FLOAT16_RGBA:          return VK_FORMAT_R16G16B16A16_SFLOAT;
        //case PF_R16G16B16A16_UNORM:          return VK_FORMAT_R16G16B16A16_UNORM;
        case PF_R16G16B16A16_UINT:           return VK_FORMAT_R16G16B16A16_UINT;
        case PF_R16G16B16A16_SNORM:          return VK_FORMAT_R16G16B16A16_SNORM;
        case PF_R16G16B16A16_SINT:           return VK_FORMAT_R16G16B16A16_SINT;
        //case PF_FLOAT32_RG:            return VK_FORMAT_R32G32_SFLOAT;
        case PF_R32G32_UINT:             return VK_FORMAT_R32G32_UINT;
        case PF_R32G32_SINT:             return VK_FORMAT_R32G32_SINT;
        //case PF_D32_FLOAT_S8X24_UINT:  return VK_FORMAT_D32_SFLOAT_S8_UINT;
        case PF_A2B10G10R10:     return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
        //case PF_R10G10B10A2_UINT:      return VK_FORMAT_A2B10G10R10_UINT_PACK32;
        case PF_R11G11B10_FLOAT:       return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
        case PF_X8B8G8R8:
        case PF_A8B8G8R8:           return hwGamma ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
        //case PF_A8B8G8R8_UINT:            return VK_FORMAT_R8G8B8A8_UINT;
        //case PF_A8B8G8R8_SNORM:           return VK_FORMAT_R8G8B8A8_SNORM;
        //case PF_A8B8G8R8_SINT:            return VK_FORMAT_R8G8B8A8_SINT;
        case PF_FLOAT16_GR:            return VK_FORMAT_R16G16_SFLOAT;
        //case PF_R16G16_UNORM:            return VK_FORMAT_R16G16_UNORM;
        case PF_R16G16_UINT:             return VK_FORMAT_R16G16_UINT;
        case PF_R16G16_SNORM:            return VK_FORMAT_R16G16_SNORM;
        case PF_R16G16_SINT:             return VK_FORMAT_R16G16_SINT;
        case PF_DEPTH32:
        case PF_DEPTH32F:              return VK_FORMAT_D32_SFLOAT;
        case PF_FLOAT32_R:             return VK_FORMAT_R32_SFLOAT;
        case PF_R32_UINT:              return VK_FORMAT_R32_UINT;
        case PF_R32_SINT:              return VK_FORMAT_R32_SINT;
        //case PF_D24_UNORM:             return VK_FORMAT_X8_D24_UNORM_PACK32;
        case PF_DEPTH24_STENCIL8:            return VK_FORMAT_D32_SFLOAT_S8_UINT; // VK_FORMAT_D24_UNORM_S8_UINT not supported on AMD
        case PF_BYTE_LA:
        case PF_RG8:                   return VK_FORMAT_R8G8_UNORM;
        case PF_R8G8_UINT:              return VK_FORMAT_R8G8_UINT;
        case PF_R8G8_SNORM:             return VK_FORMAT_R8G8_SNORM;
        case PF_R8G8_SINT:              return VK_FORMAT_R8G8_SINT;
        case PF_FLOAT16_R:             return VK_FORMAT_R16_SFLOAT;
        case PF_DEPTH16:             return VK_FORMAT_D16_UNORM;
        //case PF_R16_UNORM:             return VK_FORMAT_R16_UNORM;
        case PF_R16_UINT:              return VK_FORMAT_R16_UINT;
        case PF_R16_SNORM:             return VK_FORMAT_R16_SNORM;
        case PF_R16_SINT:              return VK_FORMAT_R16_SINT;
        case PF_A8:
        case PF_L8:
        case PF_R8:                    return VK_FORMAT_R8_UNORM;
        case PF_R8_UINT:               return VK_FORMAT_R8_UINT;
        case PF_R8_SNORM:              return VK_FORMAT_R8_SNORM;
        case PF_R8_SINT:               return VK_FORMAT_R8_SINT;
        case PF_R9G9B9E5_SHAREDEXP:    return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
        case PF_DXT1:                  return hwGamma ? VK_FORMAT_BC1_RGBA_SRGB_BLOCK : VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        case PF_DXT2:                  return hwGamma ? VK_FORMAT_BC1_RGBA_SRGB_BLOCK : VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        case PF_DXT3:                  return hwGamma ? VK_FORMAT_BC2_SRGB_BLOCK : VK_FORMAT_BC2_UNORM_BLOCK;
        case PF_DXT4:                  return hwGamma ? VK_FORMAT_BC2_SRGB_BLOCK : VK_FORMAT_BC2_UNORM_BLOCK;
        case PF_DXT5:                  return hwGamma ? VK_FORMAT_BC3_SRGB_BLOCK : VK_FORMAT_BC3_UNORM_BLOCK;
#if 0
        case PF_R8G8_B8G8_UNORM:       return VK_FORMAT_B8G8R8G8_422_UNORM;
        case PF_G8R8_G8B8_UNORM:       return VK_FORMAT_G8B8G8R8_422_UNORM;
#endif
        case PF_BC4_UNORM:             return VK_FORMAT_BC4_UNORM_BLOCK;
        case PF_BC4_SNORM:             return VK_FORMAT_BC4_SNORM_BLOCK;
        case PF_BC5_UNORM:             return VK_FORMAT_BC5_UNORM_BLOCK;
        case PF_BC5_SNORM:             return VK_FORMAT_BC5_SNORM_BLOCK;
        case PF_R5G6B5:                return VK_FORMAT_R5G6B5_UNORM_PACK16;
        case PF_B5G6R5:                return VK_FORMAT_B5G6R5_UNORM_PACK16;
        case PF_A1R5G5B5:              return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
        case PF_A8R8G8B8:              return hwGamma ? VK_FORMAT_B8G8R8A8_SRGB : VK_FORMAT_B8G8R8A8_UNORM;
        case PF_X8R8G8B8:              return hwGamma ? VK_FORMAT_B8G8R8A8_SRGB : VK_FORMAT_B8G8R8A8_UNORM;
        //case PF_R10G10B10_XR_BIAS_A2_UNORM:return VK_FORMAT_A2R10G10B10_USCALED_PACK32;
        case PF_BC6H_UF16:             return VK_FORMAT_BC6H_UFLOAT_BLOCK;
        case PF_BC6H_SF16:             return VK_FORMAT_BC6H_SFLOAT_BLOCK;
        case PF_BC7_UNORM:             return VK_FORMAT_BC7_UNORM_BLOCK;
        case PF_A4R4G4B4:              return VK_FORMAT_B4G4R4A4_UNORM_PACK16;
        case PF_ETC1_RGB8:             return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
        case PF_ETC2_RGB8:             return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
        case PF_ETC2_RGBA8:            return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
        case PF_ETC2_RGB8A1:           return VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
#if 0
        case PF_EAC_R11_UNORM:         return VK_FORMAT_EAC_R11_UNORM_BLOCK;
        case PF_EAC_R11_SNORM:         return VK_FORMAT_EAC_R11_SNORM_BLOCK;
        case PF_EAC_R11G11_UNORM:      return VK_FORMAT_EAC_R11G11_UNORM_BLOCK;
        case PF_EAC_R11G11_SNORM:      return VK_FORMAT_EAC_R11G11_SNORM_BLOCK;
#endif

        case PF_ASTC_RGBA_4X4_LDR:   return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
        case PF_ASTC_RGBA_5X4_LDR:   return VK_FORMAT_ASTC_5x4_UNORM_BLOCK;
        case PF_ASTC_RGBA_5X5_LDR:   return VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
        case PF_ASTC_RGBA_6X5_LDR:   return VK_FORMAT_ASTC_6x5_UNORM_BLOCK;
        case PF_ASTC_RGBA_6X6_LDR:   return VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
        case PF_ASTC_RGBA_8X5_LDR:   return VK_FORMAT_ASTC_8x5_UNORM_BLOCK;
        case PF_ASTC_RGBA_8X6_LDR:   return VK_FORMAT_ASTC_8x6_UNORM_BLOCK;
        case PF_ASTC_RGBA_8X8_LDR:   return VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
        case PF_ASTC_RGBA_10X5_LDR:  return VK_FORMAT_ASTC_10x5_UNORM_BLOCK;
        case PF_ASTC_RGBA_10X6_LDR:  return VK_FORMAT_ASTC_10x6_UNORM_BLOCK;
        case PF_ASTC_RGBA_10X8_LDR:  return VK_FORMAT_ASTC_10x8_UNORM_BLOCK;
        case PF_ASTC_RGBA_10X10_LDR: return VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
        case PF_ASTC_RGBA_12X10_LDR: return VK_FORMAT_ASTC_12x10_UNORM_BLOCK;
        case PF_ASTC_RGBA_12X12_LDR: return VK_FORMAT_ASTC_12x12_UNORM_BLOCK;
        case PF_ATC_RGB:
        case PF_ATC_RGBA_EXPLICIT_ALPHA:
        case PF_ATC_RGBA_INTERPOLATED_ALPHA:
        // PVRTC requires asking for extension VK_IMG_format_pvrtc before using
        // VK_FORMAT_PVRTC* family of enums.
        //
        // However:
        //  1. This extension is deprecated.
        //  2. We have no way of testing the extension/functionality works and won't crash if Ogre
        //     ever runs on PVRTC-enabled drivers. The oldest / most popular PVRTC GPU we can get is
        //     the PowerVR GE8320 and it doesn't expose this extension.
        //
        // PVRTC on Vulkan is dead.
        case PF_PVRTC_RGB2:
        case PF_PVRTC_RGBA2:
        case PF_PVRTC_RGB4:
        case PF_PVRTC_RGBA4:
        case PF_PVRTC2_2BPP:
        case PF_PVRTC2_4BPP:
        default:
            return VK_FORMAT_UNDEFINED;
        }
        // clang-format on
    }
    //-----------------------------------------------------------------------------------
    VkImageAspectFlags VulkanMappings::getImageAspect(PixelFormat pf, const bool bPreferDepthOverStencil)
    {
        int ret = PixelUtil::isDepth(pf) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        if (pf == PF_DEPTH24_STENCIL8)
            ret |= VK_IMAGE_ASPECT_STENCIL_BIT;
        return ret;
    }
    //-----------------------------------------------------------------------------------
    VkAccessFlags VulkanMappings::get( const Texture *texture )
    {
        VkAccessFlags texAccessFlags = 0;

        if( texture->getUsage() & TU_UNORDERED_ACCESS )
        {
            texAccessFlags |= VK_ACCESS_SHADER_READ_BIT;
            if( texture->getUsage() & TU_UNORDERED_ACCESS )
                texAccessFlags |= VK_ACCESS_SHADER_WRITE_BIT;
        }
        if( texture->getUsage() & TU_RENDERTARGET )
        {
            // texAccessFlags |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
            if( !PixelUtil::isDepth( texture->getFormat() ) )
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
}  // namespace Ogre
