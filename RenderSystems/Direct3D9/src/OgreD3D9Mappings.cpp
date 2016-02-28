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
#include "OgreD3D9Mappings.h"
#include "OgreString.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"
#include "OgreException.h"

namespace Ogre 
{
    //---------------------------------------------------------------------
    DWORD D3D9Mappings::get(ShadeOptions so)
    {
        switch( so )
        {
        case SO_FLAT:
            return D3DSHADE_FLAT;
        case SO_GOURAUD:
            return D3DSHADE_GOURAUD;
        case SO_PHONG:
            return D3DSHADE_PHONG;
        }
        return 0;
    }
    //---------------------------------------------------------------------
    D3DLIGHTTYPE D3D9Mappings::get(Ogre::Light::LightTypes lightType)
    {
        switch( lightType )
        {
        case Light::LT_POINT:
            return D3DLIGHT_POINT;
        case Light::LT_DIRECTIONAL:
            return D3DLIGHT_DIRECTIONAL;
        case Light::LT_SPOTLIGHT:
            return D3DLIGHT_SPOT;
        }
        return D3DLIGHT_FORCE_DWORD;
    }
    //---------------------------------------------------------------------
    DWORD D3D9Mappings::get(TexCoordCalcMethod m, const D3DCAPS9& caps)
    {
        switch( m )
        {
        case TEXCALC_NONE:
            return D3DTSS_TCI_PASSTHRU;
        case TEXCALC_ENVIRONMENT_MAP_REFLECTION:
            return D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR;
        case TEXCALC_ENVIRONMENT_MAP_PLANAR:
            if (caps.VertexProcessingCaps & D3DVTXPCAPS_TEXGEN_SPHEREMAP)
            {
                // Use sphere map if available
                return D3DTSS_TCI_SPHEREMAP;
            }
            else
            {
                // If not, fall back on camera space reflection vector which isn't as good
                return D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR;
            }
        case TEXCALC_ENVIRONMENT_MAP_NORMAL:
            return D3DTSS_TCI_CAMERASPACENORMAL;
        case TEXCALC_ENVIRONMENT_MAP:
            if (caps.VertexProcessingCaps & D3DVTXPCAPS_TEXGEN_SPHEREMAP)
            {
                // Use sphere map if available
                return D3DTSS_TCI_SPHEREMAP;
            }
            else
            {
                // If not, fall back on camera space normal which isn't as good
                return D3DTSS_TCI_CAMERASPACENORMAL;
            }
        case TEXCALC_PROJECTIVE_TEXTURE:
            return D3DTSS_TCI_CAMERASPACEPOSITION;
        }
        return 0;
    }
    //---------------------------------------------------------------------
    D3DTEXTUREADDRESS D3D9Mappings::get(TextureUnitState::TextureAddressingMode tam, const D3DCAPS9& devCaps)
    {
        switch( tam )
        {
        case TextureUnitState::TAM_WRAP:
            return D3DTADDRESS_WRAP;
        case TextureUnitState::TAM_MIRROR:
            return D3DTADDRESS_MIRROR;
        case TextureUnitState::TAM_CLAMP:
            return D3DTADDRESS_CLAMP;
        case TextureUnitState::TAM_BORDER:
            if (devCaps.TextureAddressCaps & D3DPTADDRESSCAPS_BORDER)
                return D3DTADDRESS_BORDER;
            else
                return D3DTADDRESS_CLAMP;
        }
        return D3DTADDRESS_FORCE_DWORD;
    }
    //---------------------------------------------------------------------
    D3DTEXTURESTAGESTATETYPE D3D9Mappings::get(LayerBlendType lbt)
    {
        switch( lbt )
        {
        case LBT_COLOUR:
            return D3DTSS_COLOROP;
        case LBT_ALPHA:
            return D3DTSS_ALPHAOP;
        }
        return  D3DTSS_FORCE_DWORD;
    }
    //---------------------------------------------------------------------
    DWORD D3D9Mappings::get(LayerBlendSource lbs, bool perStageConstants)
    {
        switch( lbs )
        {
        case LBS_CURRENT:
            return D3DTA_CURRENT;
        case LBS_TEXTURE:
            return D3DTA_TEXTURE;
        case LBS_DIFFUSE:
            return D3DTA_DIFFUSE;
        case LBS_SPECULAR:
            return D3DTA_SPECULAR;
        case LBS_MANUAL:
            return perStageConstants ? D3DTA_CONSTANT : D3DTA_TFACTOR;
        }
        return 0;
    }
    //---------------------------------------------------------------------
    DWORD D3D9Mappings::get(LayerBlendOperationEx lbo, const D3DCAPS9& devCaps)
    {
        switch( lbo )
        {
        case LBX_SOURCE1:
            return D3DTOP_SELECTARG1;
        case LBX_SOURCE2:
            return D3DTOP_SELECTARG2;
        case LBX_MODULATE:
            return D3DTOP_MODULATE;
        case LBX_MODULATE_X2:
            return D3DTOP_MODULATE2X;
        case LBX_MODULATE_X4:
            return D3DTOP_MODULATE4X;
        case LBX_ADD:
            return D3DTOP_ADD;
        case LBX_ADD_SIGNED:
            return D3DTOP_ADDSIGNED;
        case LBX_ADD_SMOOTH:
            return D3DTOP_ADDSMOOTH;
        case LBX_SUBTRACT:
            return D3DTOP_SUBTRACT;
        case LBX_BLEND_DIFFUSE_ALPHA:
            return D3DTOP_BLENDDIFFUSEALPHA;
        case LBX_BLEND_TEXTURE_ALPHA:
            return D3DTOP_BLENDTEXTUREALPHA;
        case LBX_BLEND_CURRENT_ALPHA:
            return D3DTOP_BLENDCURRENTALPHA;
        case LBX_BLEND_MANUAL:
            return D3DTOP_BLENDFACTORALPHA;
        case LBX_DOTPRODUCT:
            if (devCaps.TextureOpCaps & D3DTEXOPCAPS_DOTPRODUCT3)
                return D3DTOP_DOTPRODUCT3;
            else
                return D3DTOP_MODULATE;
        case LBX_BLEND_DIFFUSE_COLOUR:
            if (devCaps.TextureOpCaps & D3DTEXOPCAPS_LERP)
                return D3DTOP_LERP;
            else
                return D3DTOP_MODULATE;
        }
        return 0;
    }
    //---------------------------------------------------------------------
    D3DBLEND D3D9Mappings::get(SceneBlendFactor sbf)
    {
        switch( sbf )
        {
        case SBF_ONE:
            return D3DBLEND_ONE;
        case SBF_ZERO:
            return D3DBLEND_ZERO;
        case SBF_DEST_COLOUR:
            return D3DBLEND_DESTCOLOR;
        case SBF_SOURCE_COLOUR:
            return D3DBLEND_SRCCOLOR;
        case SBF_ONE_MINUS_DEST_COLOUR:
            return D3DBLEND_INVDESTCOLOR;
        case SBF_ONE_MINUS_SOURCE_COLOUR:
            return D3DBLEND_INVSRCCOLOR;
        case SBF_DEST_ALPHA:
            return D3DBLEND_DESTALPHA;
        case SBF_SOURCE_ALPHA:
            return D3DBLEND_SRCALPHA;
        case SBF_ONE_MINUS_DEST_ALPHA:
            return D3DBLEND_INVDESTALPHA;
        case SBF_ONE_MINUS_SOURCE_ALPHA:
            return D3DBLEND_INVSRCALPHA;
        }
        return D3DBLEND_FORCE_DWORD;
    }
    //---------------------------------------------------------------------
    D3DBLENDOP D3D9Mappings::get(Ogre::SceneBlendOperation sbo)
    {
        switch(sbo)
        {
        case SBO_ADD:
            return D3DBLENDOP_ADD;
        case SBO_SUBTRACT:
            return D3DBLENDOP_SUBTRACT;
        case SBO_REVERSE_SUBTRACT:
            return D3DBLENDOP_REVSUBTRACT;
        case SBO_MIN:
            return D3DBLENDOP_MIN;
        case SBO_MAX:
            return D3DBLENDOP_MAX;
        }

        return D3DBLENDOP_FORCE_DWORD;
    }
    //---------------------------------------------------------------------
    DWORD D3D9Mappings::get(CompareFunction cf)
    {
        switch( cf )
        {
        case CMPF_ALWAYS_FAIL:
            return D3DCMP_NEVER;
        case CMPF_ALWAYS_PASS:
            return D3DCMP_ALWAYS;
        case CMPF_LESS:
            return D3DCMP_LESS;
        case CMPF_LESS_EQUAL:
            return D3DCMP_LESSEQUAL;
        case CMPF_EQUAL:
            return D3DCMP_EQUAL;
        case CMPF_NOT_EQUAL:
            return D3DCMP_NOTEQUAL;
        case CMPF_GREATER_EQUAL:
            return D3DCMP_GREATEREQUAL;
        case CMPF_GREATER:
            return D3DCMP_GREATER;
        };
        return 0;
    }
    //---------------------------------------------------------------------
    DWORD D3D9Mappings::get(CullingMode cm, bool flip)
    {
        switch( cm )
        {
        case CULL_NONE:
            return D3DCULL_NONE;
        case CULL_CLOCKWISE:
            if( flip )
                return D3DCULL_CCW;
            else
                return D3DCULL_CW;
        case CULL_ANTICLOCKWISE:
            if( flip )
                return D3DCULL_CW;
            else
                return D3DCULL_CCW;
        }
        return 0;
    }
    //---------------------------------------------------------------------
    D3DFOGMODE D3D9Mappings::get(FogMode fm)
    {
        switch( fm )
        {
        case FOG_EXP:
            return D3DFOG_EXP;
        case FOG_EXP2:
            return D3DFOG_EXP2;
        case FOG_LINEAR:
            return D3DFOG_LINEAR;
        }
        return D3DFOG_FORCE_DWORD;
    }
    //---------------------------------------------------------------------
    D3DFILLMODE D3D9Mappings::get(PolygonMode level)
    {
        switch(level)
        {
        case PM_POINTS:
            return D3DFILL_POINT;
        case PM_WIREFRAME:
            return D3DFILL_WIREFRAME;
        case PM_SOLID:
            return D3DFILL_SOLID;
        }
        return D3DFILL_FORCE_DWORD;
    }
    //---------------------------------------------------------------------
    DWORD D3D9Mappings::get(StencilOperation op, bool invert)
    {
        switch(op)
        {
        case SOP_KEEP:
            return D3DSTENCILOP_KEEP;
        case SOP_ZERO:
            return D3DSTENCILOP_ZERO;
        case SOP_REPLACE:
            return D3DSTENCILOP_REPLACE;
        case SOP_INCREMENT:
            return invert? D3DSTENCILOP_DECRSAT : D3DSTENCILOP_INCRSAT;
        case SOP_DECREMENT:
            return invert? D3DSTENCILOP_INCRSAT : D3DSTENCILOP_DECRSAT;
        case SOP_INCREMENT_WRAP:
            return invert? D3DSTENCILOP_DECR : D3DSTENCILOP_INCR;
        case SOP_DECREMENT_WRAP:
            return invert? D3DSTENCILOP_INCR : D3DSTENCILOP_DECR;
        case SOP_INVERT:
            return D3DSTENCILOP_INVERT;
        }
        return 0;
    }
    //---------------------------------------------------------------------
    D3DSAMPLERSTATETYPE D3D9Mappings::get(FilterType ft)
    {
        switch (ft)
        {
        case FT_MIN:
            return D3DSAMP_MINFILTER;
            break;
        case FT_MAG:
            return D3DSAMP_MAGFILTER;
            break;
        case FT_MIP:
            return D3DSAMP_MIPFILTER;
            break;
        }

        // to keep compiler happy
        return D3DSAMP_MINFILTER;
    }
    //---------------------------------------------------------------------
    DWORD D3D9Mappings::get(FilterType ft, FilterOptions fo, const D3DCAPS9& devCaps, 
        eD3DTexType texType)
    {
        // Assume normal
        DWORD capsType = devCaps.TextureFilterCaps;

        switch( texType )
        {
        case D3D_TEX_TYPE_NORMAL:
            capsType = devCaps.TextureFilterCaps;
            break;
        case D3D_TEX_TYPE_CUBE:
            capsType = devCaps.CubeTextureFilterCaps;
            break;
        case D3D_TEX_TYPE_VOLUME:
            capsType = devCaps.VolumeTextureFilterCaps;
            break;
        }

        switch (ft)
        {
        case FT_MIN:
            switch( fo )
            {
                // NOTE: Fall through if device doesn't support requested type
            case FO_ANISOTROPIC:
                if( capsType & D3DPTFILTERCAPS_MINFANISOTROPIC )
                {
                    return D3DTEXF_ANISOTROPIC;
                    break;
                }
            case FO_LINEAR:
                if( capsType & D3DPTFILTERCAPS_MINFLINEAR )
                {
                    return D3DTEXF_LINEAR;
                    break;
                }
            case FO_POINT:
            case FO_NONE:
                return D3DTEXF_POINT;
                break;
            }
            break;
        case FT_MAG:
            switch( fo )
            {
            // NOTE: Fall through if device doesn't support requested type
            case FO_ANISOTROPIC:
                if( capsType & D3DPTFILTERCAPS_MAGFANISOTROPIC )
                {
                    return D3DTEXF_ANISOTROPIC;
                    break;
                }
            case FO_LINEAR:
                if( capsType & D3DPTFILTERCAPS_MAGFLINEAR )
                {
                    return D3DTEXF_LINEAR;
                    break;
                }
            case FO_POINT:
            case FO_NONE:
                return D3DTEXF_POINT;
                break;
            }
            break;
        case FT_MIP:
            switch( fo )
            {
            case FO_ANISOTROPIC:
            case FO_LINEAR:
                if( capsType & D3DPTFILTERCAPS_MIPFLINEAR )
                {
                    return D3DTEXF_LINEAR;
                    break;
                }
            case FO_POINT:
                if( capsType & D3DPTFILTERCAPS_MIPFPOINT )
                {
                    return D3DTEXF_POINT;
                    break;
                }
            case FO_NONE:
                return D3DTEXF_NONE;
                break;
            }
            break;
        }

        // should never get here
        return 0;

    }
    //---------------------------------------------------------------------
    D3D9Mappings::eD3DTexType D3D9Mappings::get(TextureType ogreTexType)
    {
        switch( ogreTexType )
        {
        case TEX_TYPE_1D :
        case TEX_TYPE_2D :
            return D3D9Mappings::D3D_TEX_TYPE_NORMAL;
        case TEX_TYPE_CUBE_MAP :
            return D3D9Mappings::D3D_TEX_TYPE_CUBE;
        case TEX_TYPE_3D :
            return D3D9Mappings::D3D_TEX_TYPE_VOLUME;
        }
        return D3D9Mappings::D3D_TEX_TYPE_NONE;
    }
    //---------------------------------------------------------------------
    DWORD D3D9Mappings::get(HardwareBuffer::Usage usage)
    {
        DWORD ret = 0;
        if (usage & HardwareBuffer::HBU_DYNAMIC)
        {
#if OGRE_D3D_MANAGE_BUFFERS
            // Only add the dynamic flag for default pool, and
            // we use default pool when buffer is discardable
            if (usage & HardwareBuffer::HBU_DISCARDABLE)
                ret |= D3DUSAGE_DYNAMIC;
#else
            ret |= D3DUSAGE_DYNAMIC;
#endif
        }
        if (usage & HardwareBuffer::HBU_WRITE_ONLY)
        {
            ret |= D3DUSAGE_WRITEONLY;
        }
        return ret;
    }
    //---------------------------------------------------------------------
    DWORD D3D9Mappings::get(HardwareBuffer::LockOptions options, HardwareBuffer::Usage usage)
    {
        DWORD ret = 0;
        if (options == HardwareBuffer::HBL_DISCARD)
        {
#if OGRE_D3D_MANAGE_BUFFERS
            // Only add the discard flag for dynamic usgae and default pool
            if ((usage & HardwareBuffer::HBU_DYNAMIC) &&
                (usage & HardwareBuffer::HBU_DISCARDABLE))
                ret |= D3DLOCK_DISCARD;
#else
            // D3D doesn't like discard or no_overwrite on non-dynamic buffers
            if (usage & HardwareBuffer::HBU_DYNAMIC)
                ret |= D3DLOCK_DISCARD;
#endif
        }
        if (options == HardwareBuffer::HBL_READ_ONLY)
        {
            // D3D debug runtime doesn't like you locking managed buffers readonly
            // when they were created with write-only (even though you CAN read
            // from the software backed version)
            if (!(usage & HardwareBuffer::HBU_WRITE_ONLY))
                ret |= D3DLOCK_READONLY;

        }
        if (options == HardwareBuffer::HBL_NO_OVERWRITE)
        {
#if OGRE_D3D_MANAGE_BUFFERS
            // Only add the nooverwrite flag for dynamic usgae and default pool
            if ((usage & HardwareBuffer::HBU_DYNAMIC) &&
                (usage & HardwareBuffer::HBU_DISCARDABLE))
                ret |= D3DLOCK_NOOVERWRITE;
#else
            // D3D doesn't like discard or no_overwrite on non-dynamic buffers
            if (usage & HardwareBuffer::HBU_DYNAMIC)
                ret |= D3DLOCK_NOOVERWRITE;
#endif 
        }

        return ret;
    }
    //---------------------------------------------------------------------
    D3DFORMAT D3D9Mappings::get(HardwareIndexBuffer::IndexType itype)
    {
        if (itype == HardwareIndexBuffer::IT_32BIT)
        {
            return D3DFMT_INDEX32;
        }
        else
        {
            return D3DFMT_INDEX16;
        }
    }
    //---------------------------------------------------------------------
    D3DDECLTYPE D3D9Mappings::get(VertexElementType vType)
    {
        switch (vType)
        {
        case VET_COLOUR:
        case VET_COLOUR_ABGR:
        case VET_COLOUR_ARGB:
            return D3DDECLTYPE_D3DCOLOR;
            break;
        case VET_FLOAT1:
            return D3DDECLTYPE_FLOAT1;
            break;
        case VET_FLOAT2:
            return D3DDECLTYPE_FLOAT2;
            break;
        case VET_FLOAT3:
            return D3DDECLTYPE_FLOAT3;
            break;
        case VET_FLOAT4:
            return D3DDECLTYPE_FLOAT4;
            break;
        case VET_SHORT2:
            return D3DDECLTYPE_SHORT2;
            break;
        case VET_SHORT4:
            return D3DDECLTYPE_SHORT4;
            break;
        case VET_UBYTE4:
            return D3DDECLTYPE_UBYTE4;
            break;
        }
        // to keep compiler happy
        return D3DDECLTYPE_FLOAT3;
    }
    //---------------------------------------------------------------------
    D3DDECLUSAGE D3D9Mappings::get(VertexElementSemantic sem)
    {
        switch (sem)
        {
        case VES_BLEND_INDICES:
            return D3DDECLUSAGE_BLENDINDICES;
            break;
        case VES_BLEND_WEIGHTS:
            return D3DDECLUSAGE_BLENDWEIGHT;
            break;
        case VES_DIFFUSE:
            return D3DDECLUSAGE_COLOR; // NB index will differentiate
            break;
        case VES_SPECULAR:
            return D3DDECLUSAGE_COLOR; // NB index will differentiate
            break;
        case VES_NORMAL:
            return D3DDECLUSAGE_NORMAL;
            break;
        case VES_POSITION:
            return D3DDECLUSAGE_POSITION;
            break;
        case VES_TEXTURE_COORDINATES:
            return D3DDECLUSAGE_TEXCOORD;
            break;
        case VES_BINORMAL:
            return D3DDECLUSAGE_BINORMAL;
            break;
        case VES_TANGENT:
            return D3DDECLUSAGE_TANGENT;
            break;
        }
        // to keep compiler happy
        return D3DDECLUSAGE_POSITION;
    }
    //---------------------------------------------------------------------
    D3DXMATRIX D3D9Mappings::makeD3DXMatrix( const Matrix4& mat )
    {
        // Transpose matrix
        // D3D9 uses row vectors i.e. V*M
        // Ogre, OpenGL and everything else uses column vectors i.e. M*V
        return D3DXMATRIX(
            mat[0][0], mat[1][0], mat[2][0], mat[3][0],
            mat[0][1], mat[1][1], mat[2][1], mat[3][1],
            mat[0][2], mat[1][2], mat[2][2], mat[3][2],
            mat[0][3], mat[1][3], mat[2][3], mat[3][3]);
    }
    //---------------------------------------------------------------------
    Matrix4 D3D9Mappings::convertD3DXMatrix( const D3DXMATRIX& mat )
    {
        return Matrix4(
            mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
            mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
            mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
            mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]);
    }
    /****************************************************************************************/
    PixelFormat D3D9Mappings::_getPF(D3DFORMAT d3dPF)
    {
        switch(d3dPF)
        {
        case D3DFMT_A8:
            return PF_A8;
        case D3DFMT_L8:
            return PF_L8;
        case D3DFMT_L16:
            return PF_L16;
        case D3DFMT_A4L4:
            return PF_A4L4;
        case D3DFMT_A8L8:
            return PF_BYTE_LA;  // Assume little endian here
        case D3DFMT_R3G3B2:
            return PF_R3G3B2;
        case D3DFMT_A1R5G5B5:
            return PF_A1R5G5B5;
        case D3DFMT_A4R4G4B4:
            return PF_A4R4G4B4;
        case D3DFMT_R5G6B5:
            return PF_R5G6B5;
        case D3DFMT_R8G8B8:
            return PF_R8G8B8;
        case D3DFMT_X8R8G8B8:
            return PF_X8R8G8B8;
        case D3DFMT_A8R8G8B8:
            return PF_A8R8G8B8;
        case D3DFMT_X8B8G8R8:
            return PF_X8B8G8R8;
        case D3DFMT_A8B8G8R8:
            return PF_A8B8G8R8;
        case D3DFMT_A2R10G10B10:
            return PF_A2R10G10B10;
        case D3DFMT_A2B10G10R10:
           return PF_A2B10G10R10;
        case D3DFMT_R16F:
            return PF_FLOAT16_R;
        case D3DFMT_A16B16G16R16F:
            return PF_FLOAT16_RGBA;
        case D3DFMT_R32F:
            return PF_FLOAT32_R;
        case D3DFMT_G32R32F:
            return PF_FLOAT32_GR;
        case D3DFMT_A32B32G32R32F:
            return PF_FLOAT32_RGBA;
        case D3DFMT_G16R16F:
            return PF_FLOAT16_GR;
        case D3DFMT_A16B16G16R16:
            return PF_SHORT_RGBA;
        case D3DFMT_G16R16:
            return PF_SHORT_GR;
        case D3DFMT_DXT1:
            return PF_DXT1;
        case D3DFMT_DXT2:
            return PF_DXT2;
        case D3DFMT_DXT3:
            return PF_DXT3;
        case D3DFMT_DXT4:
            return PF_DXT4;
        case D3DFMT_DXT5:
            return PF_DXT5;
        default:
            return PF_UNKNOWN;
        }
    }
    /****************************************************************************************/
    D3DFORMAT D3D9Mappings::_getPF(PixelFormat ogrePF)
    {
        switch(ogrePF)
        {
        case PF_L8:
            return D3DFMT_L8;
        case PF_L16:
            return D3DFMT_L16;
        case PF_A8:
            return D3DFMT_A8;
        case PF_A4L4:
            return D3DFMT_A4L4;
        case PF_BYTE_LA:
            return D3DFMT_A8L8; // Assume little endian here
        case PF_R3G3B2:
            return D3DFMT_R3G3B2;
        case PF_A1R5G5B5:
            return D3DFMT_A1R5G5B5;
        case PF_R5G6B5:
            return D3DFMT_R5G6B5;
        case PF_A4R4G4B4:
            return D3DFMT_A4R4G4B4;
        case PF_R8G8B8:
            return D3DFMT_R8G8B8;
        case PF_A8R8G8B8:
            return D3DFMT_A8R8G8B8;
        case PF_A8B8G8R8:
            return D3DFMT_A8B8G8R8;
        case PF_X8R8G8B8:
            return D3DFMT_X8R8G8B8;
        case PF_X8B8G8R8:
            return D3DFMT_X8B8G8R8;
        case PF_A2B10G10R10:
            return D3DFMT_A2B10G10R10;
        case PF_A2R10G10B10:
            return D3DFMT_A2R10G10B10;
        case PF_FLOAT16_R:
            return D3DFMT_R16F;
        case PF_FLOAT16_GR:
            return D3DFMT_G16R16F;
        case PF_FLOAT16_RGBA:
            return D3DFMT_A16B16G16R16F;
        case PF_FLOAT32_R:
            return D3DFMT_R32F;
        case PF_FLOAT32_GR:
            return D3DFMT_G32R32F;
        case PF_FLOAT32_RGBA:
            return D3DFMT_A32B32G32R32F;
        case PF_SHORT_RGBA:
            return D3DFMT_A16B16G16R16;
        case PF_SHORT_GR:
            return D3DFMT_G16R16;
        case PF_DXT1:
            return D3DFMT_DXT1;
        case PF_DXT2:
            return D3DFMT_DXT2;
        case PF_DXT3:
            return D3DFMT_DXT3;
        case PF_DXT4:
            return D3DFMT_DXT4;
        case PF_DXT5:
            return D3DFMT_DXT5;
        case PF_UNKNOWN:
        default:
            return D3DFMT_UNKNOWN;
        }
    }
    /****************************************************************************************/
    PixelFormat D3D9Mappings::_getClosestSupportedPF(PixelFormat ogrePF)
    {
        if (_getPF(ogrePF) != D3DFMT_UNKNOWN)
        {
            return ogrePF;
        }
        switch(ogrePF)
        {
        case PF_B5G6R5:
            return PF_R5G6B5;
        case PF_B8G8R8:
            return PF_R8G8B8;
        case PF_B8G8R8A8:
            return PF_A8R8G8B8;
        case PF_SHORT_RGB:
            return PF_SHORT_RGBA;
        case PF_FLOAT16_RGB:
            return PF_FLOAT16_RGBA;
        case PF_FLOAT32_RGB:
            return PF_FLOAT32_RGBA;
        case PF_UNKNOWN:
        default:
            return PF_A8R8G8B8;
        }
    }

}
