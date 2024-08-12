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
#include "OgreD3D11Mappings.h"
#include "OgreD3D11RenderSystem.h"
#include "OgreRoot.h"

namespace Ogre 
{
    //---------------------------------------------------------------------
    D3D11_TEXTURE_ADDRESS_MODE D3D11Mappings::get(TextureAddressingMode tam)
    {
        D3D11RenderSystem* rsys = static_cast<D3D11RenderSystem*>(Root::getSingleton().getRenderSystem());
        if (rsys->_getFeatureLevel() == D3D_FEATURE_LEVEL_9_1)
            return D3D11_TEXTURE_ADDRESS_WRAP;

        //return D3D11_TEXTURE_ADDRESS_WRAP;
        switch( tam )
        {
        case TextureUnitState::TAM_WRAP:
            return D3D11_TEXTURE_ADDRESS_WRAP;
        case TextureUnitState::TAM_MIRROR:
            return D3D11_TEXTURE_ADDRESS_MIRROR;
        case TextureUnitState::TAM_CLAMP:
            return D3D11_TEXTURE_ADDRESS_CLAMP;
        case TextureUnitState::TAM_BORDER:
            return D3D11_TEXTURE_ADDRESS_BORDER;
        }
		return D3D11_TEXTURE_ADDRESS_WRAP;
    }
    //---------------------------------------------------------------------
    D3D11_BLEND D3D11Mappings::get(SceneBlendFactor sbf, bool forAlpha)
    {
        switch( sbf )
        {
        case SBF_ONE:                       return D3D11_BLEND_ONE;
        case SBF_ZERO:                      return D3D11_BLEND_ZERO;
        case SBF_DEST_COLOUR:               return forAlpha ? D3D11_BLEND_DEST_ALPHA : D3D11_BLEND_DEST_COLOR;
        case SBF_SOURCE_COLOUR:             return forAlpha ? D3D11_BLEND_SRC_ALPHA : D3D11_BLEND_SRC_COLOR;
        case SBF_ONE_MINUS_DEST_COLOUR:     return forAlpha ? D3D11_BLEND_INV_DEST_ALPHA : D3D11_BLEND_INV_DEST_COLOR;
        case SBF_ONE_MINUS_SOURCE_COLOUR:   return forAlpha ? D3D11_BLEND_INV_SRC_ALPHA : D3D11_BLEND_INV_SRC_COLOR;
        case SBF_DEST_ALPHA:                return D3D11_BLEND_DEST_ALPHA;
        case SBF_SOURCE_ALPHA:              return D3D11_BLEND_SRC_ALPHA;
        case SBF_ONE_MINUS_DEST_ALPHA:      return D3D11_BLEND_INV_DEST_ALPHA;
        case SBF_ONE_MINUS_SOURCE_ALPHA:    return D3D11_BLEND_INV_SRC_ALPHA;
        }
        return D3D11_BLEND_ZERO;
    }
    //---------------------------------------------------------------------
    D3D11_BLEND_OP D3D11Mappings::get(SceneBlendOperation sbo)
    {
        switch( sbo )
        {
        case SBO_ADD:
            return D3D11_BLEND_OP_ADD;
        case SBO_SUBTRACT:
            return D3D11_BLEND_OP_SUBTRACT;
        case SBO_REVERSE_SUBTRACT:
            return D3D11_BLEND_OP_REV_SUBTRACT;
        case SBO_MIN:
            return D3D11_BLEND_OP_MIN;
        case SBO_MAX:
            return D3D11_BLEND_OP_MAX;
        }
        return D3D11_BLEND_OP_ADD;
    }
    //---------------------------------------------------------------------
    D3D11_COMPARISON_FUNC D3D11Mappings::get(CompareFunction cf)
    {
        switch( cf )
        {
        case CMPF_ALWAYS_FAIL:
            return D3D11_COMPARISON_NEVER;
        case CMPF_ALWAYS_PASS:
            return D3D11_COMPARISON_ALWAYS;
        case CMPF_LESS:
            return D3D11_COMPARISON_LESS;
        case CMPF_LESS_EQUAL:
            return D3D11_COMPARISON_LESS_EQUAL;
        case CMPF_EQUAL:
            return D3D11_COMPARISON_EQUAL;
        case CMPF_NOT_EQUAL:
            return D3D11_COMPARISON_NOT_EQUAL;
        case CMPF_GREATER_EQUAL:
            return D3D11_COMPARISON_GREATER_EQUAL;
        case CMPF_GREATER:
            return D3D11_COMPARISON_GREATER;
        };
        return D3D11_COMPARISON_ALWAYS;
    }
    //---------------------------------------------------------------------
    D3D11_CULL_MODE D3D11Mappings::get(CullingMode cm, bool flip)
    {
        switch( cm )
        {
        case CULL_NONE:
            return D3D11_CULL_NONE;
        case CULL_CLOCKWISE:
			return flip ? D3D11_CULL_FRONT : D3D11_CULL_BACK;
        case CULL_ANTICLOCKWISE:
			return flip ? D3D11_CULL_BACK : D3D11_CULL_FRONT;
        }
        return D3D11_CULL_NONE;
    }
    //---------------------------------------------------------------------
    D3D11_FILL_MODE D3D11Mappings::get(PolygonMode level)
    {
        switch(level)
        {
        case PM_POINTS:
            return D3D11_FILL_SOLID;
        case PM_WIREFRAME:
            return D3D11_FILL_WIREFRAME;
        case PM_SOLID:
            return D3D11_FILL_SOLID;
        }
        return D3D11_FILL_SOLID;
    }
    //---------------------------------------------------------------------
    D3D11_STENCIL_OP D3D11Mappings::get(StencilOperation op, bool invert)
    {
        switch(op)
        {
        case SOP_KEEP:
            return D3D11_STENCIL_OP_KEEP;
        case SOP_ZERO:
            return D3D11_STENCIL_OP_ZERO;
        case SOP_REPLACE:
            return D3D11_STENCIL_OP_REPLACE;
        case SOP_INCREMENT:
            return invert? D3D11_STENCIL_OP_DECR_SAT : D3D11_STENCIL_OP_INCR_SAT;
        case SOP_DECREMENT:
            return invert? D3D11_STENCIL_OP_INCR_SAT : D3D11_STENCIL_OP_DECR_SAT;
        case SOP_INCREMENT_WRAP:
            return invert? D3D11_STENCIL_OP_DECR : D3D11_STENCIL_OP_INCR;
        case SOP_DECREMENT_WRAP:
            return invert? D3D11_STENCIL_OP_INCR : D3D11_STENCIL_OP_DECR;
        case SOP_INVERT:
            return D3D11_STENCIL_OP_INVERT;
        }
        return D3D11_STENCIL_OP_KEEP;
    }
    //---------------------------------------------------------------------
    D3D11_FILTER D3D11Mappings::get(const FilterOptions min, const FilterOptions mag, const FilterOptions mip, const bool comparison)
    {
		// anisotropic means trilinear and anisotropic, handle this case early
		if(min == FO_ANISOTROPIC || mag == FO_ANISOTROPIC || mip == FO_ANISOTROPIC)
			return comparison ? D3D11_FILTER_COMPARISON_ANISOTROPIC : D3D11_FILTER_ANISOTROPIC;

		// FilterOptions::FO_NONE is not supported
#define MERGE_FOR_SWITCH(_comparison_, _min_ , _mag_, _mip_ ) ((_comparison_ ? 8 : 0) | (_min_ == FO_LINEAR ? 4 : 0) | (_mag_ == FO_LINEAR ? 2 : 0) | (_mip_ == FO_LINEAR ? 1 : 0))
		switch((MERGE_FOR_SWITCH(comparison, min, mag, mip)))
        {
		case MERGE_FOR_SWITCH(true, FO_POINT, FO_POINT, FO_POINT):
			return D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
		case MERGE_FOR_SWITCH(true, FO_POINT, FO_POINT, FO_LINEAR):
			return D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
		case MERGE_FOR_SWITCH(true, FO_POINT, FO_LINEAR, FO_POINT):
			return D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
		case MERGE_FOR_SWITCH(true, FO_POINT, FO_LINEAR, FO_LINEAR):
			return D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
		case MERGE_FOR_SWITCH(true, FO_LINEAR, FO_POINT, FO_POINT):
			return D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
		case MERGE_FOR_SWITCH(true, FO_LINEAR, FO_POINT, FO_LINEAR):
			return D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		case MERGE_FOR_SWITCH(true, FO_LINEAR, FO_LINEAR, FO_POINT):
			return D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		case MERGE_FOR_SWITCH(true, FO_LINEAR, FO_LINEAR, FO_LINEAR):
			return D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		case MERGE_FOR_SWITCH(false, FO_POINT, FO_POINT, FO_POINT):
			return D3D11_FILTER_MIN_MAG_MIP_POINT;
		case MERGE_FOR_SWITCH(false, FO_POINT, FO_POINT, FO_LINEAR):
			return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
		case MERGE_FOR_SWITCH(false, FO_POINT, FO_LINEAR, FO_POINT):
			return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
		case MERGE_FOR_SWITCH(false, FO_POINT, FO_LINEAR, FO_LINEAR):
			return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
		case MERGE_FOR_SWITCH(false, FO_LINEAR, FO_POINT, FO_POINT):
			return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
		case MERGE_FOR_SWITCH(false, FO_LINEAR, FO_POINT, FO_LINEAR):
			return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		case MERGE_FOR_SWITCH(false, FO_LINEAR, FO_LINEAR, FO_POINT):
			return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		case MERGE_FOR_SWITCH(false, FO_LINEAR, FO_LINEAR, FO_LINEAR):
			return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        }   
#undef MERGE_FOR_SWITCH

		return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    }
    //---------------------------------------------------------------------
    D3D11_MAP D3D11Mappings::get(HardwareBuffer::LockOptions options, HardwareBuffer::Usage usage)
    {
        D3D11_MAP ret = D3D11_MAP_READ_WRITE;
        if (options == HardwareBuffer::HBL_DISCARD)
        {
            // D3D doesn't like discard or no_overwrite on non-dynamic buffers
            if (usage & HardwareBuffer::HBU_DYNAMIC)
                ret = D3D11_MAP_WRITE_DISCARD;
        }
        if (options == HardwareBuffer::HBL_READ_ONLY)
        {
            // D3D debug runtime doesn't like you locking managed buffers readonly
            // when they were created with write-only (even though you CAN read
            // from the software backed version)
            if (!(usage & HBU_DETAIL_WRITE_ONLY))
                ret = D3D11_MAP_READ;

        }
        if (options == HardwareBuffer::HBL_NO_OVERWRITE)
        {
            // D3D doesn't like discard or no_overwrite on non-dynamic buffers
            if (usage & HardwareBuffer::HBU_DYNAMIC)
                ret = D3D11_MAP_WRITE_NO_OVERWRITE;
        }

        return ret;
    }
    //---------------------------------------------------------------------
    void D3D11Mappings::setPixelBoxMapping(PixelBox& box, const D3D11_MAPPED_SUBRESOURCE& mapping)
    {
        // The main issue - pitches D3D11 are in bytes, but Ogre stores them in elements, therefore conversion is required
        size_t elemSize = PixelUtil::getNumElemBytes(box.format);
        if(elemSize != 0)
        {
            assert(0 == mapping.RowPitch % elemSize);
            assert(0 == mapping.DepthPitch % elemSize);
            box.rowPitch = mapping.RowPitch / elemSize;
            box.slicePitch = mapping.DepthPitch / elemSize;
        }
        else if(PixelUtil::isCompressed(box.format))
        {
            box.rowPitch = box.getWidth();
            box.slicePitch = box.getWidth() * box.getHeight();
        }
        else
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Invalid pixel format", "setPixelBoxMapping");
        }
        box.data = (uchar*)mapping.pData;
    }
    //---------------------------------------------------------------------
    PixelBox D3D11Mappings::getPixelBoxWithMapping(D3D11_BOX extents, DXGI_FORMAT pixelFormat, const D3D11_MAPPED_SUBRESOURCE& mapping)
    {
        PixelBox box(Box(extents.left, extents.top, extents.front, extents.right, extents.bottom, extents.back), _getPF(pixelFormat));
        setPixelBoxMapping(box, mapping);
        return box;
    }
    //---------------------------------------------------------------------
    DXGI_FORMAT D3D11Mappings::getFormat(HardwareIndexBuffer::IndexType itype)
    {
		return itype == HardwareIndexBuffer::IT_32BIT ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
    }
    //---------------------------------------------------------------------
    DXGI_FORMAT D3D11Mappings::get(VertexElementType vType)
    {
        switch (vType)
        {
        // Float32
        case VET_FLOAT1:
            return DXGI_FORMAT_R32_FLOAT;
        case VET_FLOAT2:
            return DXGI_FORMAT_R32G32_FLOAT;
        case VET_FLOAT3:
            return DXGI_FORMAT_R32G32B32_FLOAT;
        case VET_FLOAT4:
            return DXGI_FORMAT_R32G32B32A32_FLOAT;

        // Float16
        case VET_HALF1:
            return DXGI_FORMAT_R16_FLOAT;
        case VET_HALF2:
            return DXGI_FORMAT_R16G16_FLOAT;
        case VET_HALF4:
            return DXGI_FORMAT_R16G16B16A16_FLOAT;

        // Signed short
        case VET_SHORT1:
            return DXGI_FORMAT_R16_SINT;
        case VET_SHORT2:
            return DXGI_FORMAT_R16G16_SINT;
        case VET_SHORT4:
            return DXGI_FORMAT_R16G16B16A16_SINT;
        case VET_SHORT2_NORM:
            return DXGI_FORMAT_R16G16_SNORM;
        case VET_SHORT4_NORM:
            return DXGI_FORMAT_R16G16B16A16_SNORM;

        // Unsigned short
        case VET_USHORT1:
            return DXGI_FORMAT_R16_UINT;
        case VET_USHORT2:
            return DXGI_FORMAT_R16G16_UINT;
        case VET_USHORT4:
            return DXGI_FORMAT_R16G16B16A16_UINT;
        case VET_USHORT2_NORM:
            return DXGI_FORMAT_R16G16_UNORM;
        case VET_USHORT4_NORM:
            return DXGI_FORMAT_R16G16B16A16_UNORM;

        // Signed int
        case VET_INT1:
            return DXGI_FORMAT_R32_SINT;
        case VET_INT2:
            return DXGI_FORMAT_R32G32_SINT;
        case VET_INT3:
            return DXGI_FORMAT_R32G32B32_SINT;
        case VET_INT4:
            return DXGI_FORMAT_R32G32B32A32_SINT;

        // Unsigned int
        case VET_UINT1:
            return DXGI_FORMAT_R32_UINT;
        case VET_UINT2:
            return DXGI_FORMAT_R32G32_UINT;
        case VET_UINT3:
            return DXGI_FORMAT_R32G32B32_UINT;
        case VET_UINT4:
            return DXGI_FORMAT_R32G32B32A32_UINT;
            
        case VET_BYTE4:
            return DXGI_FORMAT_R8G8B8A8_SINT;
        case VET_BYTE4_NORM:
            return DXGI_FORMAT_R8G8B8A8_SNORM;
        case VET_UBYTE4:
            return DXGI_FORMAT_R8G8B8A8_UINT;
        case VET_UBYTE4_NORM:
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        }
        // to keep compiler happy
        return DXGI_FORMAT_R32G32B32_FLOAT;
    }
    //---------------------------------------------------------------------
    LPCSTR D3D11Mappings::get(VertexElementSemantic sem)
    {
        // todo - add to ogre - POSITIONT and PSIZE ("Transformed vertex position" and "Point size")
        switch (sem)    
        {
        case VES_BLEND_INDICES:
            return "BLENDINDICES";
        case VES_BLEND_WEIGHTS:
            return "BLENDWEIGHT";
        case VES_DIFFUSE:
            return "COLOR"; // NB index will differentiate
        case VES_SPECULAR:
            return "COLOR"; // NB index will differentiate
        case VES_NORMAL:
            return "NORMAL";
        case VES_POSITION:
            return "POSITION";
        case VES_TEXTURE_COORDINATES:
            return "TEXCOORD";
        case VES_BINORMAL:
            return "BINORMAL";
        case VES_TANGENT:
            return "TANGENT";
        }
        // to keep compiler happy
        return "";
    }
    //---------------------------------------------------------------------
    PixelFormat D3D11Mappings::_getPF(DXGI_FORMAT d3dPF)
    {
        switch(d3dPF)
        {
        case DXGI_FORMAT_UNKNOWN:                   return PF_UNKNOWN;
        case DXGI_FORMAT_R32G32B32A32_TYPELESS:     return PF_UNKNOWN;
        case DXGI_FORMAT_R32G32B32A32_FLOAT:        return PF_FLOAT32_RGBA;
        case DXGI_FORMAT_R32G32B32A32_UINT:         return PF_UNKNOWN;
        case DXGI_FORMAT_R32G32B32A32_SINT:         return PF_UNKNOWN;
        case DXGI_FORMAT_R32G32B32_TYPELESS:        return PF_UNKNOWN;
        case DXGI_FORMAT_R32G32B32_FLOAT:           return PF_FLOAT32_RGB;
        case DXGI_FORMAT_R32G32B32_UINT:            return PF_UNKNOWN;
        case DXGI_FORMAT_R32G32B32_SINT:            return PF_UNKNOWN;
        case DXGI_FORMAT_R16G16B16A16_TYPELESS:     return PF_UNKNOWN;
        case DXGI_FORMAT_R16G16B16A16_FLOAT:        return PF_FLOAT16_RGBA;
        case DXGI_FORMAT_R16G16B16A16_UNORM:        return PF_SHORT_RGBA;
        case DXGI_FORMAT_R16G16B16A16_UINT:         return PF_UNKNOWN;
        case DXGI_FORMAT_R16G16B16A16_SNORM:        return PF_UNKNOWN;
        case DXGI_FORMAT_R16G16B16A16_SINT:         return PF_UNKNOWN;
        case DXGI_FORMAT_R32G32_TYPELESS:           return PF_UNKNOWN;
        case DXGI_FORMAT_R32G32_FLOAT:              return PF_FLOAT32_GR;
        case DXGI_FORMAT_R32G32_UINT:               return PF_UNKNOWN;
        case DXGI_FORMAT_R32G32_SINT:               return PF_UNKNOWN;
        case DXGI_FORMAT_R32G8X24_TYPELESS:         return PF_UNKNOWN;
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:      return PF_UNKNOWN;
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:  return PF_UNKNOWN;
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:   return PF_UNKNOWN;
        case DXGI_FORMAT_R10G10B10A2_TYPELESS:      return PF_UNKNOWN;
        case DXGI_FORMAT_R10G10B10A2_UNORM:         return PF_A2B10G10R10;
        case DXGI_FORMAT_R10G10B10A2_UINT:          return PF_UNKNOWN;
        case DXGI_FORMAT_R11G11B10_FLOAT:           return PF_R11G11B10_FLOAT;
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:         return PF_UNKNOWN;
        case DXGI_FORMAT_R8G8B8A8_UNORM:            return PF_A8B8G8R8;
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:       return PF_A8B8G8R8;
        case DXGI_FORMAT_R8G8B8A8_UINT:             return PF_UNKNOWN;
        case DXGI_FORMAT_R8G8B8A8_SNORM:            return PF_UNKNOWN;
        case DXGI_FORMAT_R8G8B8A8_SINT:             return PF_UNKNOWN;
        case DXGI_FORMAT_R16G16_TYPELESS:           return PF_UNKNOWN;
        case DXGI_FORMAT_R16G16_FLOAT:              return PF_FLOAT16_GR;
        case DXGI_FORMAT_R16G16_UNORM:              return PF_SHORT_GR;
        case DXGI_FORMAT_R16G16_UINT:               return PF_UNKNOWN;
        case DXGI_FORMAT_R16G16_SNORM:              return PF_UNKNOWN;
        case DXGI_FORMAT_R16G16_SINT:               return PF_R16G16_SINT;
        case DXGI_FORMAT_R32_TYPELESS:              return PF_DEPTH32;
        case DXGI_FORMAT_D32_FLOAT:                 return PF_DEPTH32F;
        case DXGI_FORMAT_R32_FLOAT:                 return PF_FLOAT32_R;
        case DXGI_FORMAT_R32_UINT:                  return PF_UNKNOWN;
        case DXGI_FORMAT_R32_SINT:                  return PF_UNKNOWN;
        case DXGI_FORMAT_R24G8_TYPELESS:            return PF_UNKNOWN;
        case DXGI_FORMAT_D24_UNORM_S8_UINT:         return PF_DEPTH24_STENCIL8;
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:     return PF_UNKNOWN;
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT:      return PF_UNKNOWN;
        case DXGI_FORMAT_R8G8_TYPELESS:             return PF_UNKNOWN;
        case DXGI_FORMAT_R8G8_UNORM:                return PF_UNKNOWN;
        case DXGI_FORMAT_R8G8_UINT:                 return PF_UNKNOWN;
        case DXGI_FORMAT_R8G8_SNORM:                return PF_UNKNOWN;
        case DXGI_FORMAT_R8G8_SINT:                 return PF_UNKNOWN;
        case DXGI_FORMAT_R16_TYPELESS:              return PF_UNKNOWN;
        case DXGI_FORMAT_R16_FLOAT:                 return PF_FLOAT16_R;
        case DXGI_FORMAT_D16_UNORM:                 return PF_DEPTH16;
        case DXGI_FORMAT_R16_UNORM:                 return PF_L16;
        case DXGI_FORMAT_R16_UINT:                  return PF_UNKNOWN;
        case DXGI_FORMAT_R16_SNORM:                 return PF_UNKNOWN;
        case DXGI_FORMAT_R16_SINT:                  return PF_UNKNOWN;
        case DXGI_FORMAT_R8_TYPELESS:               return PF_UNKNOWN;
        case DXGI_FORMAT_R8_UNORM:                  return PF_R8;
        case DXGI_FORMAT_R8_UINT:                   return PF_UNKNOWN;
        case DXGI_FORMAT_R8_SNORM:                  return PF_UNKNOWN;
        case DXGI_FORMAT_R8_SINT:                   return PF_UNKNOWN;
        case DXGI_FORMAT_A8_UNORM:                  return PF_A8;
        case DXGI_FORMAT_R1_UNORM:                  return PF_UNKNOWN;
        case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:        return PF_UNKNOWN;
        case DXGI_FORMAT_R8G8_B8G8_UNORM:           return PF_UNKNOWN;
        case DXGI_FORMAT_G8R8_G8B8_UNORM:           return PF_UNKNOWN;
        case DXGI_FORMAT_BC1_TYPELESS:              return PF_DXT1;
        case DXGI_FORMAT_BC1_UNORM:                 return PF_DXT1;
        case DXGI_FORMAT_BC1_UNORM_SRGB:            return PF_DXT1;
        case DXGI_FORMAT_BC2_TYPELESS:              return PF_DXT3;
        case DXGI_FORMAT_BC2_UNORM:                 return PF_DXT3;
        case DXGI_FORMAT_BC2_UNORM_SRGB:            return PF_DXT3;
        case DXGI_FORMAT_BC3_TYPELESS:              return PF_DXT5;
        case DXGI_FORMAT_BC3_UNORM:                 return PF_DXT5;
        case DXGI_FORMAT_BC3_UNORM_SRGB:            return PF_DXT5;
        case DXGI_FORMAT_BC4_TYPELESS:              return PF_BC4_UNORM;
        case DXGI_FORMAT_BC4_UNORM:                 return PF_BC4_UNORM;
        case DXGI_FORMAT_BC4_SNORM:                 return PF_BC4_SNORM;
        case DXGI_FORMAT_BC5_TYPELESS:              return PF_BC5_UNORM;
        case DXGI_FORMAT_BC5_UNORM:                 return PF_BC5_UNORM;
        case DXGI_FORMAT_BC5_SNORM:                 return PF_BC5_SNORM;
        case DXGI_FORMAT_B5G6R5_UNORM:              return PF_R5G6B5;
        case DXGI_FORMAT_B5G5R5A1_UNORM:            return PF_A1R5G5B5;
        case DXGI_FORMAT_B8G8R8A8_UNORM:            return PF_A8R8G8B8;
        case DXGI_FORMAT_B8G8R8X8_UNORM:            return PF_X8R8G8B8;
        case DXGI_FORMAT_BC6H_TYPELESS:             return PF_BC6H_SF16;
        case DXGI_FORMAT_BC6H_UF16:                 return PF_BC6H_UF16;
        case DXGI_FORMAT_BC6H_SF16:                 return PF_BC6H_SF16;
        case DXGI_FORMAT_BC7_TYPELESS:              return PF_BC7_UNORM;
        case DXGI_FORMAT_BC7_UNORM:                 return PF_BC7_UNORM;
        case DXGI_FORMAT_BC7_UNORM_SRGB:            return PF_BC7_UNORM;

#if defined(_WIN32_WINNT_WIN8) && (_WIN32_WINNT >= _WIN32_WINNT_WIN8) && defined(DXGI_FORMAT_AYUV)
        case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:return PF_UNKNOWN;
        case DXGI_FORMAT_B8G8R8A8_TYPELESS:         return PF_UNKNOWN;
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:       return PF_A8R8G8B8;
        case DXGI_FORMAT_B8G8R8X8_TYPELESS:         return PF_UNKNOWN;
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:       return PF_X8R8G8B8;
        case DXGI_FORMAT_AYUV:                      return PF_UNKNOWN;
        case DXGI_FORMAT_Y410:                      return PF_UNKNOWN;
        case DXGI_FORMAT_Y416:                      return PF_UNKNOWN;
        case DXGI_FORMAT_NV12:                      return PF_UNKNOWN;
        case DXGI_FORMAT_P010:                      return PF_UNKNOWN;
        case DXGI_FORMAT_P016:                      return PF_UNKNOWN;
        case DXGI_FORMAT_420_OPAQUE:                return PF_UNKNOWN;
        case DXGI_FORMAT_YUY2:                      return PF_UNKNOWN;
        case DXGI_FORMAT_Y210:                      return PF_UNKNOWN;
        case DXGI_FORMAT_Y216:                      return PF_UNKNOWN;
        case DXGI_FORMAT_NV11:                      return PF_UNKNOWN;
        case DXGI_FORMAT_AI44:                      return PF_UNKNOWN;
        case DXGI_FORMAT_IA44:                      return PF_UNKNOWN;
        case DXGI_FORMAT_P8:                        return PF_UNKNOWN;
        case DXGI_FORMAT_A8P8:                      return PF_UNKNOWN;
        case DXGI_FORMAT_B4G4R4A4_UNORM:            return PF_A4R4G4B4;
#endif

        default:                                    return PF_UNKNOWN;
        }
    }
    //---------------------------------------------------------------------
    DXGI_FORMAT D3D11Mappings::_getPF(PixelFormat ogrePF)
    {
        // PF_L8 maps to DXGI_FORMAT_R8_UNORM and grayscale textures became "redscale", without green and blue components.
        // This can be fixed by shader modification, but here we can only convert PF_L8 to PF_A8B8G8R8 manually to fix the issue.
        // Note, that you can use PF_R8 to explicitly request "redscale" behavior for grayscale textures, avoiding overhead.
        switch(ogrePF)
        {
        case PF_R8:             return DXGI_FORMAT_R8_UNORM;
        case PF_L16:            return DXGI_FORMAT_R16_UNORM;
        case PF_A8:             return DXGI_FORMAT_A8_UNORM;
        case PF_BYTE_LA:        return DXGI_FORMAT_UNKNOWN; 
        case PF_R3G3B2:         return DXGI_FORMAT_UNKNOWN;
        case PF_A1R5G5B5:       return DXGI_FORMAT_UNKNOWN;
        case PF_R5G6B5:         return DXGI_FORMAT_UNKNOWN;
        case PF_A4R4G4B4:       return DXGI_FORMAT_UNKNOWN;
        case PF_R8G8B8:         return DXGI_FORMAT_UNKNOWN;
        case PF_A8R8G8B8:       return DXGI_FORMAT_B8G8R8A8_UNORM;
        case PF_A8B8G8R8:       return DXGI_FORMAT_R8G8B8A8_UNORM;
        case PF_X8R8G8B8:       return DXGI_FORMAT_B8G8R8X8_UNORM;
        case PF_X8B8G8R8:       return DXGI_FORMAT_UNKNOWN;
        case PF_A2B10G10R10:    return DXGI_FORMAT_R10G10B10A2_TYPELESS;
        case PF_A2R10G10B10:    return DXGI_FORMAT_UNKNOWN;
        case PF_R11G11B10_FLOAT:return DXGI_FORMAT_R11G11B10_FLOAT;
        case PF_FLOAT16_R:      return DXGI_FORMAT_R16_FLOAT;
        case PF_FLOAT16_GR:     return DXGI_FORMAT_R16G16_FLOAT;
        case PF_FLOAT16_RGBA:   return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case PF_FLOAT32_R:      return DXGI_FORMAT_R32_FLOAT;
        case PF_FLOAT32_RGBA:   return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case PF_SHORT_RGBA:     return DXGI_FORMAT_R16G16B16A16_UNORM;
        case PF_DXT1:           return DXGI_FORMAT_BC1_UNORM;
        case PF_DXT2:           return DXGI_FORMAT_BC1_UNORM;
        case PF_DXT3:           return DXGI_FORMAT_BC2_UNORM;
        case PF_DXT4:           return DXGI_FORMAT_BC2_UNORM;
        case PF_DXT5:           return DXGI_FORMAT_BC3_UNORM;
        case PF_BC4_SNORM:      return DXGI_FORMAT_BC4_SNORM;
        case PF_BC4_UNORM:      return DXGI_FORMAT_BC4_UNORM;
        case PF_BC5_SNORM:      return DXGI_FORMAT_BC5_SNORM;
        case PF_BC5_UNORM:      return DXGI_FORMAT_BC5_UNORM;
        case PF_BC6H_UF16:      return DXGI_FORMAT_BC6H_UF16;
        case PF_BC6H_SF16:      return DXGI_FORMAT_BC6H_SF16;
        case PF_BC7_UNORM:      return DXGI_FORMAT_BC7_UNORM;
        case PF_R16G16_SINT:    return DXGI_FORMAT_R16G16_SINT;
        case PF_FLOAT32_GR:     return DXGI_FORMAT_R32G32_FLOAT;
        case PF_DEPTH16:        return DXGI_FORMAT_R32_TYPELESS;
        case PF_DEPTH32:        return DXGI_FORMAT_R32_TYPELESS;
        case PF_DEPTH32F:       return DXGI_FORMAT_R32_TYPELESS;
        case PF_DEPTH24_STENCIL8:     return DXGI_FORMAT_R24G8_TYPELESS;
        default:                return DXGI_FORMAT_UNKNOWN;
        }
    }
    //---------------------------------------------------------------------
    DXGI_FORMAT D3D11Mappings::_getGammaFormat(DXGI_FORMAT format, bool appendSRGB)
    {
        if(appendSRGB)
        {
            switch(format)
            {
            case DXGI_FORMAT_R8G8B8A8_UNORM:       return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            case DXGI_FORMAT_B8G8R8A8_UNORM:       return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
            case DXGI_FORMAT_B8G8R8X8_UNORM:       return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
            case DXGI_FORMAT_BC1_UNORM:            return DXGI_FORMAT_BC1_UNORM_SRGB;
            case DXGI_FORMAT_BC2_UNORM:            return DXGI_FORMAT_BC2_UNORM_SRGB;
            case DXGI_FORMAT_BC3_UNORM:            return DXGI_FORMAT_BC3_UNORM_SRGB;
            case DXGI_FORMAT_BC7_UNORM:            return DXGI_FORMAT_BC7_UNORM_SRGB;
            }
        }
        return format;
    }
    //---------------------------------------------------------------------
    bool D3D11Mappings::_isBinaryCompressedFormat(DXGI_FORMAT d3dPF)
    {
        return
            d3dPF == DXGI_FORMAT_BC1_TYPELESS || d3dPF == DXGI_FORMAT_BC1_UNORM || d3dPF == DXGI_FORMAT_BC1_UNORM_SRGB ||
            d3dPF == DXGI_FORMAT_BC2_TYPELESS || d3dPF == DXGI_FORMAT_BC2_UNORM || d3dPF == DXGI_FORMAT_BC2_UNORM_SRGB ||
            d3dPF == DXGI_FORMAT_BC3_TYPELESS || d3dPF == DXGI_FORMAT_BC3_UNORM || d3dPF == DXGI_FORMAT_BC3_UNORM_SRGB ||
            d3dPF == DXGI_FORMAT_BC4_TYPELESS || d3dPF == DXGI_FORMAT_BC4_UNORM || d3dPF == DXGI_FORMAT_BC4_SNORM ||
            d3dPF == DXGI_FORMAT_BC5_TYPELESS || d3dPF == DXGI_FORMAT_BC5_UNORM || d3dPF == DXGI_FORMAT_BC5_SNORM ||
#if OGRE_PLATFORM == OGRE_PLATFORM_WINRT
            d3dPF == DXGI_FORMAT_BC6H_TYPELESS|| d3dPF == DXGI_FORMAT_BC6H_UF16 || d3dPF == DXGI_FORMAT_BC6H_SF16 || 
            d3dPF == DXGI_FORMAT_BC7_TYPELESS || d3dPF == DXGI_FORMAT_BC7_UNORM || d3dPF == DXGI_FORMAT_BC7_UNORM_SRGB ||
#endif
            0;
    }
    //---------------------------------------------------------------------
    PixelFormat D3D11Mappings::_getClosestSupportedPF(PixelFormat ogrePF)
    {
        if (_getPF(ogrePF) != DXGI_FORMAT_UNKNOWN)
        {
            return ogrePF;
        }
        switch(ogrePF)
        {
        case PF_R8G8B8:
            return PF_X8R8G8B8;
        case PF_FLOAT16_RGB:
            return PF_FLOAT16_RGBA;
        case PF_FLOAT32_RGB:
            return PF_FLOAT32_RGBA;
        case PF_DEPTH16:
            return PF_L16;
        case PF_DEPTH24_STENCIL8:
        case PF_DEPTH32:
        case PF_DEPTH32F:
            return PF_FLOAT32_R;
        case PF_UNKNOWN:
        default:
            return PF_A8B8G8R8;
        }
    }
    //---------------------------------------------------------------------
	D3D11_USAGE D3D11Mappings::_getUsage(HardwareBuffer::Usage usage)
    {
		return _isDynamic(usage) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
    }
    //---------------------------------------------------------------------
	bool D3D11Mappings::_isDynamic(HardwareBuffer::Usage usage)
    {
		return (usage & HardwareBuffer::HBU_DYNAMIC) != 0;
    }
    //---------------------------------------------------------------------
	UINT D3D11Mappings::_getAccessFlags(HardwareBuffer::Usage usage)
    {
		return _isDynamic(usage) ? D3D11_CPU_ACCESS_WRITE : 0;
    }
    //---------------------------------------------------------------------
    TextureType D3D11Mappings::_getTexType(D3D11_SRV_DIMENSION type)
    {
        switch(type)
        {
        case D3D_SRV_DIMENSION_TEXTURE1D:
            return TEX_TYPE_1D;
        case D3D_SRV_DIMENSION_TEXTURE2D:
        case D3D_SRV_DIMENSION_TEXTURE2DMS:
            return TEX_TYPE_2D;
        case D3D_SRV_DIMENSION_TEXTURE2DARRAY:
        case D3D_SRV_DIMENSION_TEXTURE2DMSARRAY:
            return TEX_TYPE_2D_ARRAY;
        case D3D_SRV_DIMENSION_TEXTURE3D:
            return TEX_TYPE_3D;
        case D3D_SRV_DIMENSION_TEXTURECUBE:
            return TEX_TYPE_CUBE_MAP;
        default:
            // unknown
            return static_cast<TextureType>(0);
        }
    }
    //---------------------------------------------------------------------
	UINT D3D11Mappings::_getTextureBindFlags(DXGI_FORMAT format, TextureUsage usage)
    {
		// We mark all textures as render target to be able to use GenerateMips() on it
		// TODO: use DDSTextureLoader way of determining supported formats via CheckFormatSupport() & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN
		// TODO: explore DDSTextureLoader way of generating mips on temporary texture, to avoid D3D11_BIND_RENDER_TARGET flag injection 
		bool isRenderTarget = /*(usage & TU_RENDERTARGET) &&*/ !(usage & TU_DYNAMIC);

		// check for incompatible pixel formats
		if(isRenderTarget)
		{
			switch (format)
			{
			case DXGI_FORMAT_R32G32B32A32_TYPELESS:
			case DXGI_FORMAT_R32G32B32_TYPELESS:
			case DXGI_FORMAT_R32G32B32_FLOAT:
			case DXGI_FORMAT_R32G32B32_UINT:
			case DXGI_FORMAT_R32G32B32_SINT:
			case DXGI_FORMAT_R16G16B16A16_TYPELESS:
			case DXGI_FORMAT_R32G32_TYPELESS:
			case DXGI_FORMAT_R32G8X24_TYPELESS:
			case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
			case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
			case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
			case DXGI_FORMAT_R10G10B10A2_TYPELESS:
			case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
			case DXGI_FORMAT_R8G8B8A8_TYPELESS:
			case DXGI_FORMAT_R16G16_TYPELESS:
			case DXGI_FORMAT_R32_TYPELESS:
			case DXGI_FORMAT_D32_FLOAT:
			case DXGI_FORMAT_R24G8_TYPELESS:
			case DXGI_FORMAT_D24_UNORM_S8_UINT:
			case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
			case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
			case DXGI_FORMAT_R8G8_TYPELESS:
			case DXGI_FORMAT_R16_TYPELESS:
			case DXGI_FORMAT_D16_UNORM:
			case DXGI_FORMAT_R8_TYPELESS:
			case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
			case DXGI_FORMAT_R8G8_B8G8_UNORM:
			case DXGI_FORMAT_G8R8_G8B8_UNORM:
			case DXGI_FORMAT_BC1_TYPELESS:
			case DXGI_FORMAT_BC1_UNORM:
			case DXGI_FORMAT_BC1_UNORM_SRGB:
			case DXGI_FORMAT_BC2_TYPELESS:
			case DXGI_FORMAT_BC2_UNORM:
			case DXGI_FORMAT_BC2_UNORM_SRGB:
			case DXGI_FORMAT_BC3_TYPELESS:
			case DXGI_FORMAT_BC3_UNORM:
			case DXGI_FORMAT_BC3_UNORM_SRGB:
			case DXGI_FORMAT_BC4_TYPELESS:
			case DXGI_FORMAT_BC4_UNORM:
			case DXGI_FORMAT_BC4_SNORM:
			case DXGI_FORMAT_BC5_TYPELESS:
			case DXGI_FORMAT_BC5_UNORM:
			case DXGI_FORMAT_BC5_SNORM:
			case DXGI_FORMAT_B8G8R8A8_TYPELESS:
			case DXGI_FORMAT_B8G8R8X8_TYPELESS:
			case DXGI_FORMAT_BC6H_TYPELESS:
			case DXGI_FORMAT_BC6H_UF16:
			case DXGI_FORMAT_BC6H_SF16:
			case DXGI_FORMAT_BC7_TYPELESS:
			case DXGI_FORMAT_BC7_UNORM:
			case DXGI_FORMAT_BC7_UNORM_SRGB:
				isRenderTarget = false;
			}
		}

        UINT retVal = 0;
        if( !(usage & TU_NOT_SAMPLED) )
            retVal |= D3D11_BIND_SHADER_RESOURCE;

        if( isRenderTarget )
            retVal |= D3D11_BIND_RENDER_TARGET;

        if( usage & TU_UNORDERED_ACCESS )
            retVal |= D3D11_BIND_UNORDERED_ACCESS;

        return retVal;
	}

    UINT D3D11Mappings::_getTextureMiscFlags(UINT bindflags, TextureType textype, TextureUsage usage)
    {
        if(_isDynamic(usage))
            return 0;

        UINT flags = 0;

		if((bindflags & D3D11_BIND_SHADER_RESOURCE) && (bindflags & D3D11_BIND_RENDER_TARGET))
			flags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;

        if(textype == TEX_TYPE_CUBE_MAP)
            flags |= D3D11_RESOURCE_MISC_TEXTURECUBE;

        return flags;
    }
}
