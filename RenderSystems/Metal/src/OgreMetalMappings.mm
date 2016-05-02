/*
  -----------------------------------------------------------------------------
  This source file is part of OGRE
  (Object-oriented Graphics Rendering Engine)
  For the latest info, see http://www.ogre3d.org

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

#include "OgreMetalMappings.h"
#include "OgreHlmsDatablock.h"

namespace Ogre
{
    MTLPixelFormat MetalMappings::getPixelFormat( PixelFormat pf, bool isGamma )
    {
        switch( pf )
        {
        case PF_L8:                     return MTLPixelFormatR8Unorm;
        case PF_L16:                    return MTLPixelFormatR16Unorm;
        case PF_A8:                     return MTLPixelFormatA8Unorm;
        case PF_A4L4:                   return MTLPixelFormatInvalid;
        case PF_BYTE_LA:                return MTLPixelFormatInvalid;
        case PF_R5G6B5:                 return MTLPixelFormatB5G6R5Unorm;
        case PF_B5G6R5:                 return MTLPixelFormatB5G6R5Unorm;
        case PF_R3G3B2:                 return MTLPixelFormatInvalid;
        case PF_A4R4G4B4:               return MTLPixelFormatABGR4Unorm;
        case PF_A1R5G5B5:               return MTLPixelFormatBGR5A1Unorm;

        case PF_X8R8G8B8:
        case PF_A8R8G8B8:
        case PF_B8G8R8A8:
            return !isGamma ? MTLPixelFormatBGRA8Unorm : MTLPixelFormatBGRA8Unorm_sRGB;

        case PF_R8G8B8:
        case PF_B8G8R8:
        case PF_X8B8G8R8:
        case PF_A8B8G8R8:
        case PF_R8G8B8A8:
            return !isGamma ? MTLPixelFormatRGBA8Unorm : MTLPixelFormatRGBA8Unorm_sRGB;

        case PF_A2R10G10B10:            return MTLPixelFormatRGB10A2Unorm;
        case PF_A2B10G10R10:            return MTLPixelFormatRGB10A2Unorm;
        case PF_DXT1:                   return !isGamma ? MTLPixelFormatBC1_RGBA : MTLPixelFormatBC1_RGBA_sRGB;
        case PF_DXT2:                   return !isGamma ? MTLPixelFormatBC2_RGBA : MTLPixelFormatBC2_RGBA_sRGB;
        case PF_DXT3:                   return !isGamma ? MTLPixelFormatBC2_RGBA : MTLPixelFormatBC2_RGBA_sRGB;
        case PF_DXT4:                   return !isGamma ? MTLPixelFormatBC3_RGBA : MTLPixelFormatBC3_RGBA_sRGB;
        case PF_DXT5:                   return !isGamma ? MTLPixelFormatBC3_RGBA : MTLPixelFormatBC3_RGBA_sRGB;
        case PF_FLOAT16_R:              return MTLPixelFormatR16Float;
        case PF_FLOAT16_RGB:            return MTLPixelFormatRGBA16Float;
        case PF_FLOAT16_RGBA:           return MTLPixelFormatRGBA16Float;
        case PF_FLOAT32_R:              return MTLPixelFormatR32Float;
        case PF_FLOAT32_RGB:            return MTLPixelFormatRGBA32Float;
        case PF_FLOAT32_RGBA:           return MTLPixelFormatRGBA32Float;
        case PF_FLOAT16_GR:             return MTLPixelFormatRG16Float;
        case PF_FLOAT32_GR:             return MTLPixelFormatRG32Float;
        case PF_DEPTH_DEPRECATED:       return MTLPixelFormatInvalid;
        case PF_SHORT_RGBA:             return MTLPixelFormatRGBA16Snorm;
        case PF_SHORT_GR:               return MTLPixelFormatRG16Snorm;
        case PF_SHORT_RGB:              return MTLPixelFormatRGBA16Snorm;
        case PF_PVRTC_RGB2:             return !isGamma ? MTLPixelFormatPVRTC_RGB_2BPP  : MTLPixelFormatPVRTC_RGB_2BPP_sRGB;
        case PF_PVRTC_RGBA2:            return !isGamma ? MTLPixelFormatPVRTC_RGBA_2BPP : MTLPixelFormatPVRTC_RGBA_2BPP_sRGB;
        case PF_PVRTC_RGB4:             return !isGamma ? MTLPixelFormatPVRTC_RGB_4BPP  : MTLPixelFormatPVRTC_RGB_4BPP_sRGB;
        case PF_PVRTC_RGBA4:            return !isGamma ? MTLPixelFormatPVRTC_RGBA_4BPP : MTLPixelFormatPVRTC_RGBA_4BPP_sRGB;
        case PF_PVRTC2_2BPP:            return MTLPixelFormatInvalid;
        case PF_PVRTC2_4BPP:            return MTLPixelFormatInvalid;
        case PF_R11G11B10_FLOAT:        return MTLPixelFormatRG11B10Float;
        case PF_R8_UINT:                return MTLPixelFormatR8Uint;
        case PF_R8G8_UINT:              return MTLPixelFormatRG8Uint;
        case PF_R8G8B8_UINT:            return MTLPixelFormatRGBA8Uint;
        case PF_R8G8B8A8_UINT:          return MTLPixelFormatRGBA8Uint;
        case PF_R16_UINT:               return MTLPixelFormatR16Uint;
        case PF_R16G16_UINT:            return MTLPixelFormatRG16Uint;
        case PF_R16G16B16_UINT:         return MTLPixelFormatRGBA16Uint;
        case PF_R16G16B16A16_UINT:      return MTLPixelFormatRGBA16Uint;
        case PF_R32_UINT:               return MTLPixelFormatR32Uint;
        case PF_R32G32_UINT:            return MTLPixelFormatRG32Uint;
        case PF_R32G32B32_UINT:         return MTLPixelFormatRGBA32Uint;
        case PF_R32G32B32A32_UINT:      return MTLPixelFormatRGBA32Uint;
        case PF_R8_SINT:                return MTLPixelFormatR8Sint;
        case PF_R8G8_SINT:              return MTLPixelFormatRG8Sint;
        case PF_R8G8B8_SINT:            return MTLPixelFormatRGBA8Sint;
        case PF_R8G8B8A8_SINT:          return MTLPixelFormatRGBA8Sint;
        case PF_R16_SINT:               return MTLPixelFormatR16Sint;
        case PF_R16G16_SINT:            return MTLPixelFormatRG16Sint;
        case PF_R16G16B16_SINT:         return MTLPixelFormatRGBA16Sint;
        case PF_R16G16B16A16_SINT:      return MTLPixelFormatRGBA16Sint;
        case PF_R32_SINT:               return MTLPixelFormatR32Sint;
        case PF_R32G32_SINT:            return MTLPixelFormatRG32Sint;
        case PF_R32G32B32_SINT:         return MTLPixelFormatRGBA32Sint;
        case PF_R32G32B32A32_SINT:      return MTLPixelFormatRGBA32Sint;
        case PF_R9G9B9E5_SHAREDEXP:     return MTLPixelFormatRGB9E5Float;
        case PF_BC4_UNORM:              return MTLPixelFormatBC4_RUnorm;
        case PF_BC4_SNORM:              return MTLPixelFormatBC4_RSnorm;
        case PF_BC5_UNORM:              return MTLPixelFormatBC5_RGUnorm;
        case PF_BC5_SNORM:              return MTLPixelFormatBC5_RGSnorm;
        case PF_BC6H_UF16:              return MTLPixelFormatBC6H_RGBUfloat;
        case PF_BC6H_SF16:              return MTLPixelFormatBC6H_RGBFloat;
        case PF_BC7_UNORM:              return MTLPixelFormatBC7_RGBAUnorm;
        case PF_BC7_UNORM_SRGB:         return MTLPixelFormatBC7_RGBAUnorm_sRGB;
        case PF_R8:                     return !isGamma ? MTLPixelFormatR8Unorm : MTLPixelFormatR8Unorm_sRGB;
        case PF_RG8:                    return !isGamma ? MTLPixelFormatRG8Unorm : MTLPixelFormatRG8Unorm_sRGB;
        case PF_R8_SNORM:               return MTLPixelFormatR8Snorm;
        case PF_R8G8_SNORM:             return MTLPixelFormatRG8Snorm;
        case PF_R8G8B8_SNORM:           return MTLPixelFormatRGBA8Snorm;
        case PF_R8G8B8A8_SNORM:         return MTLPixelFormatRGBA8Snorm;
        case PF_R16_SNORM:              return MTLPixelFormatR16Snorm;
        case PF_R16G16_SNORM:           return MTLPixelFormatRG16Snorm;
        case PF_R16G16B16_SNORM:        return MTLPixelFormatRGBA16Snorm;
        case PF_R16G16B16A16_SNORM:     return MTLPixelFormatRGBA16Snorm;
        case PF_ETC1_RGB8:              return !isGamma ? MTLPixelFormatETC2_RGB8 : MTLPixelFormatETC2_RGB8_sRGB;
        case PF_ETC2_RGB8:              return !isGamma ? MTLPixelFormatETC2_RGB8 : MTLPixelFormatETC2_RGB8_sRGB;
        case PF_ETC2_RGBA8:             return !isGamma ? MTLPixelFormatEAC_RGBA8 : MTLPixelFormatEAC_RGBA8_sRGB;
        case PF_ETC2_RGB8A1:            return !isGamma ? MTLPixelFormatETC2_RGB8A1 : MTLPixelFormatETC2_RGB8A1_sRGB;
        case PF_ATC_RGB:                        return MTLPixelFormatInvalid;
        case PF_ATC_RGBA_EXPLICIT_ALPHA:        return MTLPixelFormatInvalid;
        case PF_ATC_RGBA_INTERPOLATED_ALPHA:    return MTLPixelFormatInvalid;

        //TODO: PF_D24_UNORM_S8_UINT is normally treated as "guaranteed default" in Ogre, but
        //in Metal, the guaranteed is MTLPixelFormatDepth32Float. Should we account this here?
        case PF_D24_UNORM_S8_UINT:      return MTLPixelFormatDepth24Unorm_Stencil8;
        case PF_D24_UNORM_X8:           return MTLPixelFormatDepth24Unorm_Stencil8;
        case PF_X24_S8_UINT:            return MTLPixelFormatStencil8;
        case PF_D24_UNORM:              return MTLPixelFormatDepth24Unorm_Stencil8;
        case PF_D16_UNORM:              return MTLPixelFormatDepth32Float;
        case PF_D32_FLOAT:              return MTLPixelFormatDepth32Float;
        case PF_D32_FLOAT_X24_S8_UINT:  return MTLPixelFormatDepth32Float_Stencil8;
        case PF_D32_FLOAT_X24_X8:       return MTLPixelFormatDepth32Float_Stencil8;
        case PF_X32_X24_S8_UINT:        return MTLPixelFormatStencil8;

        case PF_NULL:                   return MTLPixelFormatInvalid;
        default:
        case PF_COUNT:                  return MTLPixelFormatInvalid;
        }
    }
    //-----------------------------------------------------------------------------------
    MTLBlendFactor MetalMappings::get( SceneBlendFactor op )
    {
        switch( op )
        {
        case SBF_ONE:                   return MTLBlendFactorOne;
        case SBF_ZERO:                  return MTLBlendFactorZero;
        case SBF_DEST_COLOUR:           return MTLBlendFactorDestinationColor;
        case SBF_SOURCE_COLOUR:         return MTLBlendFactorSourceColor;
        case SBF_ONE_MINUS_DEST_COLOUR: return MTLBlendFactorOneMinusDestinationColor;
        case SBF_ONE_MINUS_SOURCE_COLOUR:return MTLBlendFactorOneMinusSourceColor;
        case SBF_DEST_ALPHA:            return MTLBlendFactorDestinationAlpha;
        case SBF_SOURCE_ALPHA:          return MTLBlendFactorSourceAlpha;
        case SBF_ONE_MINUS_DEST_ALPHA:  return MTLBlendFactorOneMinusDestinationAlpha;
        case SBF_ONE_MINUS_SOURCE_ALPHA:return MTLBlendFactorOneMinusSourceAlpha;
        }
    }
    //-----------------------------------------------------------------------------------
    MTLBlendOperation MetalMappings::get( SceneBlendOperation op )
    {
        switch( op )
        {
        case SBO_ADD:                   return MTLBlendOperationAdd;
        case SBO_SUBTRACT:              return MTLBlendOperationSubtract;
        case SBO_REVERSE_SUBTRACT:      return MTLBlendOperationReverseSubtract;
        case SBO_MIN:                   return MTLBlendOperationMin;
        case SBO_MAX:                   return MTLBlendOperationMax;
        }
    }
    //-----------------------------------------------------------------------------------
    MTLColorWriteMask MetalMappings::get( uint8 mask )
    {
        return ((mask & HlmsBlendblock::BlendChannelRed)   << (3u - 0u)) |
               ((mask & HlmsBlendblock::BlendChannelGreen) << (2u - 1u)) |
               ((mask & HlmsBlendblock::BlendChannelBlue)  >> (2u - 1u)) |
               ((mask & HlmsBlendblock::BlendChannelAlpha) >> (3u - 0u));
    }
    //-----------------------------------------------------------------------------------
    MTLCompareFunction MetalMappings::get( CompareFunction cmp )
    {
        switch( cmp )
        {
        case CMPF_ALWAYS_FAIL:          return MTLCompareFunctionNever;
        case CMPF_ALWAYS_PASS:          return MTLCompareFunctionAlways;
        case CMPF_LESS:                 return MTLCompareFunctionLess;
        case CMPF_LESS_EQUAL:           return MTLCompareFunctionLessEqual;
        case CMPF_EQUAL:                return MTLCompareFunctionEqual;
        case CMPF_NOT_EQUAL:            return MTLCompareFunctionNotEqual;
        case CMPF_GREATER_EQUAL:        return MTLCompareFunctionGreater;
        case CMPF_GREATER:              return MTLCompareFunctionGreaterEqual;
        case NUM_COMPARE_FUNCTIONS:
            assert( false ); //Should never hit.
            return MTLCompareFunctionAlways;
        }
    }
    //-----------------------------------------------------------------------------------
    MTLVertexFormat MetalMappings::get( VertexElementType vertexElemType )
    {
        switch( vertexElemType )
        {
        case VET_FLOAT1:                return MTLVertexFormatFloat;
        case VET_FLOAT2:                return MTLVertexFormatFloat2;
        case VET_FLOAT3:                return MTLVertexFormatFloat3;
        case VET_FLOAT4:                return MTLVertexFormatFloat4;
        case VET_SHORT2:                return MTLVertexFormatShort2;
        case VET_SHORT4:                return MTLVertexFormatShort4;
        case VET_UBYTE4:                return MTLVertexFormatUChar4;
        case VET_USHORT2:               return MTLVertexFormatUShort2;
        case VET_USHORT4:               return MTLVertexFormatUShort4;
        case VET_INT1:                  return MTLVertexFormatInt;
        case VET_INT2:                  return MTLVertexFormatInt2;
        case VET_INT3:                  return MTLVertexFormatInt3;
        case VET_INT4:                  return MTLVertexFormatInt4;
        case VET_UINT1:                 return MTLVertexFormatUInt;
        case VET_UINT2:                 return MTLVertexFormatUInt2;
        case VET_UINT3:                 return MTLVertexFormatUInt3;
        case VET_UINT4:                 return MTLVertexFormatUInt4;
        case VET_BYTE4:                 return MTLVertexFormatChar4;
        case VET_BYTE4_SNORM:           return MTLVertexFormatChar4Normalized;
        case VET_UBYTE4_NORM:           return MTLVertexFormatUChar4Normalized;
        case VET_SHORT2_SNORM:          return MTLVertexFormatShort2Normalized;
        case VET_SHORT4_SNORM:          return MTLVertexFormatShort4Normalized;
        case VET_USHORT2_NORM:          return MTLVertexFormatUShort2Normalized;
        case VET_USHORT4_NORM:          return MTLVertexFormatUShort4Normalized;
        case VET_HALF2:                 return MTLVertexFormatHalf2;
        case VET_HALF4:                 return MTLVertexFormatHalf4;

        case VET_COLOUR:
        case VET_COLOUR_ARGB:
        case VET_COLOUR_ABGR:
            return MTLVertexFormatUChar4Normalized;

        case VET_DOUBLE1:
        case VET_DOUBLE2:
        case VET_DOUBLE3:
        case VET_DOUBLE4:
        case VET_USHORT1_DEPRECATED:
        case VET_USHORT3_DEPRECATED:
        default:
            return MTLVertexFormatInvalid;
        }
    }
    //-----------------------------------------------------------------------------------
    MTLSamplerMinMagFilter MetalMappings::get( FilterOptions filter )
    {
        switch( filter )
        {
        case FO_NONE:                   return MTLSamplerMinMagFilterNearest;
        case FO_POINT:                  return MTLSamplerMinMagFilterNearest;
        case FO_LINEAR:                 return MTLSamplerMinMagFilterLinear;
        case FO_ANISOTROPIC:            return MTLSamplerMinMagFilterLinear;
        }
    }
    //-----------------------------------------------------------------------------------
    MTLSamplerMipFilter MetalMappings::getMipFilter( FilterOptions filter )
    {
        switch( filter )
        {
        case FO_NONE:                   return MTLSamplerMipFilterNotMipmapped;
        case FO_POINT:                  return MTLSamplerMipFilterNearest;
        case FO_LINEAR:                 return MTLSamplerMipFilterLinear;
        case FO_ANISOTROPIC:            return MTLSamplerMipFilterLinear;
        }
    }
    //-----------------------------------------------------------------------------------
    MTLSamplerAddressMode MetalMappings::get( TextureAddressingMode mode )
    {
        switch( mode )
        {
        case TAM_WRAP:                  return MTLSamplerAddressModeRepeat;
        case TAM_MIRROR:                return MTLSamplerAddressModeMirrorRepeat;
        case TAM_CLAMP:                 return MTLSamplerAddressModeClampToEdge;
        //Not supported. Get the best next thing.
        case TAM_BORDER:                return MTLSamplerAddressModeClampToEdge;
        default:                        return MTLSamplerAddressModeClampToEdge;
        }
    }
}
