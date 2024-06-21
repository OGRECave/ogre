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

#include "OgreMetalMappings.h"
#include "OgreMetalDevice.h"

namespace Ogre
{
    MTLPixelFormat MetalMappings::getPixelFormat( PixelFormat pf, bool isGamma )
    {
        switch( pf )
        {
        case PF_L8:                     return MTLPixelFormatR8Unorm;
        case PF_L16:                    return MTLPixelFormatR16Unorm;
        case PF_A8:                     return MTLPixelFormatA8Unorm;
        case PF_BYTE_LA:                return MTLPixelFormatInvalid;
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        case PF_R5G6B5:                 return MTLPixelFormatB5G6R5Unorm;
        case PF_B5G6R5:                 return MTLPixelFormatB5G6R5Unorm;
#endif
        case PF_R3G3B2:                 return MTLPixelFormatInvalid;
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        case PF_A4R4G4B4:               return MTLPixelFormatABGR4Unorm;
        case PF_A1R5G5B5:               return MTLPixelFormatBGR5A1Unorm;
#endif

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
#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
        case PF_DXT1:                   return !isGamma ? MTLPixelFormatBC1_RGBA : MTLPixelFormatBC1_RGBA_sRGB;
        case PF_DXT2:                   return !isGamma ? MTLPixelFormatBC2_RGBA : MTLPixelFormatBC2_RGBA_sRGB;
        case PF_DXT3:                   return !isGamma ? MTLPixelFormatBC2_RGBA : MTLPixelFormatBC2_RGBA_sRGB;
        case PF_DXT4:                   return !isGamma ? MTLPixelFormatBC3_RGBA : MTLPixelFormatBC3_RGBA_sRGB;
        case PF_DXT5:                   return !isGamma ? MTLPixelFormatBC3_RGBA : MTLPixelFormatBC3_RGBA_sRGB;
#endif
        case PF_FLOAT16_R:              return MTLPixelFormatR16Float;
        case PF_FLOAT16_RGB:            return MTLPixelFormatRGBA16Float;
        case PF_FLOAT16_RGBA:           return MTLPixelFormatRGBA16Float;
        case PF_FLOAT32_R:              return MTLPixelFormatR32Float;
        case PF_FLOAT32_RGB:            return MTLPixelFormatRGBA32Float;
        case PF_FLOAT32_RGBA:           return MTLPixelFormatRGBA32Float;
        case PF_FLOAT16_GR:             return MTLPixelFormatRG16Float;
        case PF_FLOAT32_GR:             return MTLPixelFormatRG32Float;
        case PF_SHORT_RGBA:             return MTLPixelFormatRGBA16Snorm;
        case PF_SHORT_GR:               return MTLPixelFormatRG16Snorm;
        case PF_SHORT_RGB:              return MTLPixelFormatRGBA16Snorm;
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        case PF_PVRTC_RGB2:             return !isGamma ? MTLPixelFormatPVRTC_RGB_2BPP  : MTLPixelFormatPVRTC_RGB_2BPP_sRGB;
        case PF_PVRTC_RGBA2:            return !isGamma ? MTLPixelFormatPVRTC_RGBA_2BPP : MTLPixelFormatPVRTC_RGBA_2BPP_sRGB;
        case PF_PVRTC_RGB4:             return !isGamma ? MTLPixelFormatPVRTC_RGB_4BPP  : MTLPixelFormatPVRTC_RGB_4BPP_sRGB;
        case PF_PVRTC_RGBA4:            return !isGamma ? MTLPixelFormatPVRTC_RGBA_4BPP : MTLPixelFormatPVRTC_RGBA_4BPP_sRGB;
#endif
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
#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
        case PF_BC4_UNORM:              return MTLPixelFormatBC4_RUnorm;
        case PF_BC4_SNORM:              return MTLPixelFormatBC4_RSnorm;
        case PF_BC5_UNORM:              return MTLPixelFormatBC5_RGUnorm;
        case PF_BC5_SNORM:              return MTLPixelFormatBC5_RGSnorm;
        case PF_BC6H_UF16:              return MTLPixelFormatBC6H_RGBUfloat;
        case PF_BC6H_SF16:              return MTLPixelFormatBC6H_RGBFloat;
        case PF_BC7_UNORM:              return MTLPixelFormatBC7_RGBAUnorm;
#endif
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        case PF_R8:                     return !isGamma ? MTLPixelFormatR8Unorm : MTLPixelFormatR8Unorm_sRGB;
        case PF_RG8:                    return !isGamma ? MTLPixelFormatRG8Unorm : MTLPixelFormatRG8Unorm_sRGB;
#else
        case PF_R8:                     return MTLPixelFormatR8Unorm;
        case PF_RG8:                    return MTLPixelFormatRG8Unorm;
#endif
        case PF_R8_SNORM:               return MTLPixelFormatR8Snorm;
        case PF_R8G8_SNORM:             return MTLPixelFormatRG8Snorm;
        case PF_R8G8B8_SNORM:           return MTLPixelFormatRGBA8Snorm;
        case PF_R8G8B8A8_SNORM:         return MTLPixelFormatRGBA8Snorm;
        case PF_R16_SNORM:              return MTLPixelFormatR16Snorm;
        case PF_R16G16_SNORM:           return MTLPixelFormatRG16Snorm;
        case PF_R16G16B16_SNORM:        return MTLPixelFormatRGBA16Snorm;
        case PF_R16G16B16A16_SNORM:     return MTLPixelFormatRGBA16Snorm;
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        case PF_ETC1_RGB8:              return !isGamma ? MTLPixelFormatETC2_RGB8 : MTLPixelFormatETC2_RGB8_sRGB;
        case PF_ETC2_RGB8:              return !isGamma ? MTLPixelFormatETC2_RGB8 : MTLPixelFormatETC2_RGB8_sRGB;
        case PF_ETC2_RGBA8:             return !isGamma ? MTLPixelFormatEAC_RGBA8 : MTLPixelFormatEAC_RGBA8_sRGB;
        case PF_ETC2_RGB8A1:            return !isGamma ? MTLPixelFormatETC2_RGB8A1 : MTLPixelFormatETC2_RGB8A1_sRGB;
#endif
        case PF_ATC_RGB:                        return MTLPixelFormatInvalid;
        case PF_ATC_RGBA_EXPLICIT_ALPHA:        return MTLPixelFormatInvalid;
        case PF_ATC_RGBA_INTERPOLATED_ALPHA:    return MTLPixelFormatInvalid;

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        case PF_ASTC_RGBA_4X4_LDR:      return MTLPixelFormatASTC_4x4_LDR;
        case PF_ASTC_RGBA_5X4_LDR:      return MTLPixelFormatASTC_5x4_LDR;
        case PF_ASTC_RGBA_5X5_LDR:      return MTLPixelFormatASTC_5x5_LDR;
        case PF_ASTC_RGBA_6X5_LDR:      return MTLPixelFormatASTC_6x5_LDR;
        case PF_ASTC_RGBA_6X6_LDR:      return MTLPixelFormatASTC_6x6_LDR;
        case PF_ASTC_RGBA_8X5_LDR:      return MTLPixelFormatASTC_8x5_LDR;
        case PF_ASTC_RGBA_8X6_LDR:      return MTLPixelFormatASTC_8x6_LDR;
        case PF_ASTC_RGBA_8X8_LDR:      return MTLPixelFormatASTC_8x8_LDR;
        case PF_ASTC_RGBA_10X5_LDR:     return MTLPixelFormatASTC_10x5_LDR;
        case PF_ASTC_RGBA_10X6_LDR:     return MTLPixelFormatASTC_10x6_LDR;
        case PF_ASTC_RGBA_10X8_LDR:     return MTLPixelFormatASTC_10x8_LDR;
        case PF_ASTC_RGBA_10X10_LDR:    return MTLPixelFormatASTC_10x10_LDR;
        case PF_ASTC_RGBA_12X10_LDR:    return MTLPixelFormatASTC_12x10_LDR;
        case PF_ASTC_RGBA_12X12_LDR:    return MTLPixelFormatASTC_12x12_LDR;
#endif

#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
        case PF_DEPTH24_STENCIL8:             return MTLPixelFormatDepth24Unorm_Stencil8;
#else
        case PF_DEPTH24_STENCIL8:             return MTLPixelFormatDepth32Float_Stencil8;
#endif
#if 0
        case PF_X24_S8_UINT:            return MTLPixelFormatStencil8;
#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
        case PF_D24_UNORM:              return MTLPixelFormatDepth24Unorm_Stencil8;
#else
        case PF_D24_UNORM:              return MTLPixelFormatDepth32Float;
#endif
        case PF_D32_FLOAT_X24_S8_UINT:  return MTLPixelFormatDepth32Float_Stencil8;
        case PF_D32_FLOAT_X24_X8:       return MTLPixelFormatDepth32Float_Stencil8;
        case PF_X32_X24_S8_UINT:        return MTLPixelFormatStencil8;
#endif

        case PF_DEPTH16:              return MTLPixelFormatDepth32Float;
        case PF_DEPTH32:              return MTLPixelFormatDepth32Float;
        case PF_DEPTH32F:              return MTLPixelFormatDepth32Float;

        default:
        case PF_COUNT:                  return MTLPixelFormatInvalid;
        }
    }
    //-----------------------------------------------------------------------------------
    void MetalMappings::getDepthStencilFormat( MetalDevice *device, PixelFormat pf,
                                               MTLPixelFormat &outDepth, MTLPixelFormat &outStencil )
    {
        MTLPixelFormat depthFormat = MTLPixelFormatInvalid;
        MTLPixelFormat stencilFormat = MTLPixelFormatInvalid;

        switch( pf )
        {
        case PF_DEPTH24_STENCIL8:
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
            depthFormat = MTLPixelFormatDepth32Float;
            stencilFormat = MTLPixelFormatStencil8;
#else
            if( device->mDevice.depth24Stencil8PixelFormatSupported )
            {
                depthFormat = MTLPixelFormatDepth24Unorm_Stencil8;
                stencilFormat = MTLPixelFormatDepth24Unorm_Stencil8;
            }
            else
            {
                depthFormat = MTLPixelFormatDepth32Float_Stencil8;
                stencilFormat = MTLPixelFormatDepth32Float_Stencil8;
            }
#endif
            // keep stencil off for now
            stencilFormat = MTLPixelFormatInvalid;
            break;
#if 0
        case PF_D32_FLOAT:
        case PF_D32_FLOAT_X24_X8:
            depthFormat = MTLPixelFormatDepth32Float;
            break;
        case PF_D32_FLOAT_X24_S8_UINT:
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
            depthFormat = MTLPixelFormatDepth32Float;
            stencilFormat = MTLPixelFormatStencil8;
#else
            depthFormat = MTLPixelFormatDepth32Float_Stencil8;
            stencilFormat = MTLPixelFormatDepth32Float_Stencil8;
#endif
            break;
        case PF_X32_X24_S8_UINT:
            stencilFormat = MTLPixelFormatStencil8;
            break;
#endif
        case PF_DEPTH16:
        case PF_DEPTH32:
        case PF_DEPTH32F:
            depthFormat = MTLPixelFormatDepth32Float;
            break;
        default:
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "PixelFormat '" + PixelUtil::getFormatName( pf ) +
                         "' is not a valid depth buffer format",
                         "MetalRenderSystem::_createDepthBufferFor" );
        }

        outDepth = depthFormat;
        outStencil = stencilFormat;
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
    MTLStencilOperation MetalMappings::get( StencilOperation op )
    {
        switch( op )
        {
        case SOP_KEEP:              return MTLStencilOperationKeep;
        case SOP_ZERO:              return MTLStencilOperationZero;
        case SOP_REPLACE:           return MTLStencilOperationReplace;
        case SOP_INCREMENT:         return MTLStencilOperationIncrementClamp;
        case SOP_DECREMENT:         return MTLStencilOperationDecrementClamp;
        case SOP_INCREMENT_WRAP:    return MTLStencilOperationIncrementWrap;
        case SOP_DECREMENT_WRAP:    return MTLStencilOperationDecrementWrap;
        case SOP_INVERT:            return MTLStencilOperationInvert;
        }
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
        case CMPF_GREATER_EQUAL:        return MTLCompareFunctionGreaterEqual;
        case CMPF_GREATER:              return MTLCompareFunctionGreater;
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
        //case VET_BYTE4_SNORM:           return MTLVertexFormatChar4Normalized;
        case VET_UBYTE4_NORM:           return MTLVertexFormatUChar4Normalized;
        //case VET_SHORT2_SNORM:          return MTLVertexFormatShort2Normalized;
        //case VET_SHORT4_SNORM:          return MTLVertexFormatShort4Normalized;
        case VET_USHORT2_NORM:          return MTLVertexFormatUShort2Normalized;
        case VET_USHORT4_NORM:          return MTLVertexFormatUShort4Normalized;
        case VET_HALF1:                 return MTLVertexFormatHalf;
        case VET_HALF2:                 return MTLVertexFormatHalf2;
        case VET_HALF3:                 return MTLVertexFormatHalf3;
        case VET_HALF4:                 return MTLVertexFormatHalf4;
        case VET_INT_10_10_10_2_NORM:   return MTLVertexFormatInt1010102Normalized;

        case VET_DOUBLE1:
        case VET_DOUBLE2:
        case VET_DOUBLE3:
        case VET_DOUBLE4:
        case VET_USHORT1:
        case VET_USHORT3:
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
    //-----------------------------------------------------------------------------------
    MTLVertexFormat MetalMappings::dataTypeToVertexFormat( MTLDataType dataType )
    {
        switch( dataType )
        {
        case MTLDataTypeNone:           return MTLVertexFormatInvalid;
        case MTLDataTypeStruct:         return MTLVertexFormatInvalid;
        case MTLDataTypeArray:          return MTLVertexFormatInvalid;

        case MTLDataTypeFloat:          return MTLVertexFormatFloat;
        case MTLDataTypeFloat2:         return MTLVertexFormatFloat2;
        case MTLDataTypeFloat3:         return MTLVertexFormatFloat3;
        case MTLDataTypeFloat4:         return MTLVertexFormatFloat4;

        case MTLDataTypeFloat2x2:       return MTLVertexFormatInvalid;
        case MTLDataTypeFloat2x3:       return MTLVertexFormatInvalid;
        case MTLDataTypeFloat2x4:       return MTLVertexFormatInvalid;

        case MTLDataTypeFloat3x2:       return MTLVertexFormatInvalid;
        case MTLDataTypeFloat3x3:       return MTLVertexFormatInvalid;
        case MTLDataTypeFloat3x4:       return MTLVertexFormatInvalid;

        case MTLDataTypeFloat4x2:       return MTLVertexFormatInvalid;
        case MTLDataTypeFloat4x3:       return MTLVertexFormatInvalid;
        case MTLDataTypeFloat4x4:       return MTLVertexFormatInvalid;

        case MTLDataTypeHalf:           return MTLVertexFormatHalf2;
        case MTLDataTypeHalf2:          return MTLVertexFormatHalf2;
        case MTLDataTypeHalf3:          return MTLVertexFormatHalf3;
        case MTLDataTypeHalf4:          return MTLVertexFormatHalf4;

        case MTLDataTypeHalf2x2:        return MTLVertexFormatInvalid;
        case MTLDataTypeHalf2x3:        return MTLVertexFormatInvalid;
        case MTLDataTypeHalf2x4:        return MTLVertexFormatInvalid;

        case MTLDataTypeHalf3x2:        return MTLVertexFormatInvalid;
        case MTLDataTypeHalf3x3:        return MTLVertexFormatInvalid;
        case MTLDataTypeHalf3x4:        return MTLVertexFormatInvalid;

        case MTLDataTypeHalf4x2:        return MTLVertexFormatInvalid;
        case MTLDataTypeHalf4x3:        return MTLVertexFormatInvalid;
        case MTLDataTypeHalf4x4:        return MTLVertexFormatInvalid;

        case MTLDataTypeInt:            return MTLVertexFormatInt;
        case MTLDataTypeInt2:           return MTLVertexFormatInt2;
        case MTLDataTypeInt3:           return MTLVertexFormatInt3;
        case MTLDataTypeInt4:           return MTLVertexFormatInt4;

        case MTLDataTypeUInt:           return MTLVertexFormatUInt;
        case MTLDataTypeUInt2:          return MTLVertexFormatUInt2;
        case MTLDataTypeUInt3:          return MTLVertexFormatUInt3;
        case MTLDataTypeUInt4:          return MTLVertexFormatUInt4;

        case MTLDataTypeShort:          return MTLVertexFormatShort2;
        case MTLDataTypeShort2:         return MTLVertexFormatShort2;
        case MTLDataTypeShort3:         return MTLVertexFormatShort3;
        case MTLDataTypeShort4:         return MTLVertexFormatShort4;

        case MTLDataTypeUShort:         return MTLVertexFormatUShort2;
        case MTLDataTypeUShort2:        return MTLVertexFormatUShort2;
        case MTLDataTypeUShort3:        return MTLVertexFormatUShort3;
        case MTLDataTypeUShort4:        return MTLVertexFormatUShort4;

        case MTLDataTypeChar:           return MTLVertexFormatChar2;
        case MTLDataTypeChar2:          return MTLVertexFormatChar2;
        case MTLDataTypeChar3:          return MTLVertexFormatChar3;
        case MTLDataTypeChar4:          return MTLVertexFormatChar4;

        case MTLDataTypeUChar:          return MTLVertexFormatUChar2;
        case MTLDataTypeUChar2:         return MTLVertexFormatUChar2;
        case MTLDataTypeUChar3:         return MTLVertexFormatUChar3;
        case MTLDataTypeUChar4:         return MTLVertexFormatUChar4;

        case MTLDataTypeBool:           return MTLVertexFormatFloat;
        case MTLDataTypeBool2:          return MTLVertexFormatFloat2;
        case MTLDataTypeBool3:          return MTLVertexFormatFloat3;
        case MTLDataTypeBool4:          return MTLVertexFormatFloat4;
        default: OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "unhandled case");
        }
    }
    //-----------------------------------------------------------------------------------
    GpuConstantType MetalMappings::get( MTLDataType dataType )
    {
        switch( dataType )
        {
        case MTLDataTypeNone:           return GCT_UNKNOWN;
        case MTLDataTypeStruct:         return GCT_UNKNOWN;
        case MTLDataTypeArray:          return GCT_UNKNOWN;

        case MTLDataTypeFloat:          return GCT_FLOAT1;
        case MTLDataTypeFloat2:         return GCT_FLOAT2;
        case MTLDataTypeFloat3:         return GCT_FLOAT3;
        case MTLDataTypeFloat4:         return GCT_FLOAT4;

        case MTLDataTypeFloat2x2:       return GCT_MATRIX_2X2;
        case MTLDataTypeFloat2x3:       return GCT_MATRIX_2X3;
        case MTLDataTypeFloat2x4:       return GCT_MATRIX_2X4;

        case MTLDataTypeFloat3x2:       return GCT_MATRIX_3X2;
        case MTLDataTypeFloat3x3:       return GCT_MATRIX_3X3;
        case MTLDataTypeFloat3x4:       return GCT_MATRIX_3X4;

        case MTLDataTypeFloat4x2:       return GCT_MATRIX_4X2;
        case MTLDataTypeFloat4x3:       return GCT_MATRIX_4X3;
        case MTLDataTypeFloat4x4:       return GCT_MATRIX_4X4;

        case MTLDataTypeHalf:           return GCT_UNKNOWN;
        case MTLDataTypeHalf2:          return GCT_UNKNOWN;
        case MTLDataTypeHalf3:          return GCT_UNKNOWN;
        case MTLDataTypeHalf4:          return GCT_UNKNOWN;

        case MTLDataTypeHalf2x2:        return GCT_UNKNOWN;
        case MTLDataTypeHalf2x3:        return GCT_UNKNOWN;
        case MTLDataTypeHalf2x4:        return GCT_UNKNOWN;

        case MTLDataTypeHalf3x2:        return GCT_UNKNOWN;
        case MTLDataTypeHalf3x3:        return GCT_UNKNOWN;
        case MTLDataTypeHalf3x4:        return GCT_UNKNOWN;

        case MTLDataTypeHalf4x2:        return GCT_UNKNOWN;
        case MTLDataTypeHalf4x3:        return GCT_UNKNOWN;
        case MTLDataTypeHalf4x4:        return GCT_UNKNOWN;

        case MTLDataTypeInt:            return GCT_INT1;
        case MTLDataTypeInt2:           return GCT_INT2;
        case MTLDataTypeInt3:           return GCT_INT3;
        case MTLDataTypeInt4:           return GCT_INT4;

        case MTLDataTypeUInt:           return GCT_UINT1;
        case MTLDataTypeUInt2:          return GCT_UINT2;
        case MTLDataTypeUInt3:          return GCT_UINT3;
        case MTLDataTypeUInt4:          return GCT_UINT4;

        case MTLDataTypeShort:          return GCT_UNKNOWN;
        case MTLDataTypeShort2:         return GCT_UNKNOWN;
        case MTLDataTypeShort3:         return GCT_UNKNOWN;
        case MTLDataTypeShort4:         return GCT_UNKNOWN;

        case MTLDataTypeUShort:         return GCT_UNKNOWN;
        case MTLDataTypeUShort2:        return GCT_UNKNOWN;
        case MTLDataTypeUShort3:        return GCT_UNKNOWN;
        case MTLDataTypeUShort4:        return GCT_UNKNOWN;

        case MTLDataTypeChar:           return GCT_UNKNOWN;
        case MTLDataTypeChar2:          return GCT_UNKNOWN;
        case MTLDataTypeChar3:          return GCT_UNKNOWN;
        case MTLDataTypeChar4:          return GCT_UNKNOWN;

        case MTLDataTypeUChar:          return GCT_UNKNOWN;
        case MTLDataTypeUChar2:         return GCT_UNKNOWN;
        case MTLDataTypeUChar3:         return GCT_UNKNOWN;
        case MTLDataTypeUChar4:         return GCT_UNKNOWN;

        case MTLDataTypeBool:           return GCT_BOOL1;
        case MTLDataTypeBool2:          return GCT_BOOL2;
        case MTLDataTypeBool3:          return GCT_BOOL3;
        case MTLDataTypeBool4:          return GCT_BOOL4;
        default: OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "unhandled case");
        }
    }
}
