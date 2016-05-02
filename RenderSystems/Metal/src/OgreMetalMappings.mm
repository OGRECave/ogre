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
    MTLPixelFormat MetalMappings::getPixelFormat( PixelFormat pf )
    {
//        switch( pf )
//        {
//        case :
//        }
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
