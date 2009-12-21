/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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

namespace Ogre 
{
	//---------------------------------------------------------------------
	/*DWORD D3D11Mappings::get(ShadeOptions so)
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
	}*/
	//---------------------------------------------------------------------
	/*D3DLIGHTTYPE D3D11Mappings::get(Light::LightTypes lightType)
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
	}*/
	//---------------------------------------------------------------------
	/*DWORD D3D11Mappings::get(TexCoordCalcMethod m, const D3DCAPS9& caps)
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
	}*/
	//---------------------------------------------------------------------
	D3D11_TEXTURE_ADDRESS_MODE D3D11Mappings::get(TextureUnitState::TextureAddressingMode tam)
	{
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
		return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
	}
	//---------------------------------------------------------------------
	D3D11_BLEND D3D11Mappings::get(LayerBlendType lbt)
	{
		switch( lbt )
		{
		case LBT_COLOUR:
			return D3D11_BLEND_SRC_COLOR;
		case LBT_ALPHA:
			return D3D11_BLEND_SRC_ALPHA;
		}
		return  D3D11_BLEND_ZERO;
	}
	//---------------------------------------------------------------------
	/*DWORD D3D11Mappings::get(LayerBlendSource lbs)
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
	return D3DTA_TFACTOR;
	}
	return 0;
	}*/
	//---------------------------------------------------------------------
	/*DWORD D3D11Mappings::get(LayerBlendOperationEx lbo, const D3DCAPS9& devCaps)
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
	}*/
	//---------------------------------------------------------------------
	D3D11_BLEND D3D11Mappings::get(SceneBlendFactor sbf)
	{
		switch( sbf )
		{
		case SBF_ONE:
			return D3D11_BLEND_ONE;
		case SBF_ZERO:
			return D3D11_BLEND_ZERO;
		case SBF_DEST_COLOUR:
			return D3D11_BLEND_DEST_COLOR;
		case SBF_SOURCE_COLOUR:
			return D3D11_BLEND_SRC_COLOR;
		case SBF_ONE_MINUS_DEST_COLOUR:
			return D3D11_BLEND_INV_DEST_COLOR;
		case SBF_ONE_MINUS_SOURCE_COLOUR:
			return D3D11_BLEND_INV_SRC_COLOR;
		case SBF_DEST_ALPHA:
			return D3D11_BLEND_DEST_ALPHA;
		case SBF_SOURCE_ALPHA:
			return D3D11_BLEND_SRC_ALPHA;
		case SBF_ONE_MINUS_DEST_ALPHA:
			return D3D11_BLEND_INV_DEST_ALPHA;
		case SBF_ONE_MINUS_SOURCE_ALPHA:
			return D3D11_BLEND_INV_SRC_ALPHA;
		}
		return D3D11_BLEND_ZERO;
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
			if( flip )
				return D3D11_CULL_FRONT;
			else
				return D3D11_CULL_BACK;
		case CULL_ANTICLOCKWISE:
			if( flip )
				return D3D11_CULL_BACK;
			else
				return D3D11_CULL_FRONT;
		}
		return D3D11_CULL_NONE;
	}
	//---------------------------------------------------------------------
	/*D3DFOGMODE D3D11Mappings::get(FogMode fm)
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
	}*/
	//---------------------------------------------------------------------
	D3D11_FILL_MODE D3D11Mappings::get(PolygonMode level)
	{
		switch(level)
		{
		case PM_POINTS:
			return D3D11_FILL_SOLID; // this will done in a geometry shader like in the FixedFuncEMU sample  and the shader needs solid
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
		// D3D11_STENCIL_OP
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
	DWORD D3D11Mappings::get(FilterType ft)
	{
		switch (ft)
		{
		case FT_MIN:
			return D3D11_MIN_FILTER_SHIFT;
			break;
		case FT_MAG:
			return D3D11_MAG_FILTER_SHIFT;
			break;
		case FT_MIP:
			return D3D11_MIP_FILTER_SHIFT;
			break;
		}

		// to keep compiler happy
		return D3D11_MIP_FILTER_SHIFT;
	}
	//---------------------------------------------------------------------
	D3D11_FILTER D3D11Mappings::get(const FilterOptions min, const FilterOptions mag, const FilterOptions mip, const bool comparison)
	{

		D3D11_FILTER res;
#define MARGE_FOR_SWITCH(_comparison_, _min_ , _mag_, _mip_ ) ((_comparison_ << 16) | (_min_ << 8) | (_mag_ << 4) | (_mip_))

		switch((MARGE_FOR_SWITCH(comparison, min, mag, mip)))
		{
		case MARGE_FOR_SWITCH(true, FO_POINT, FO_POINT, FO_POINT):
			res = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
			break;
		case MARGE_FOR_SWITCH(true, FO_POINT, FO_POINT, FO_LINEAR):
			res = D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
			break;
		case MARGE_FOR_SWITCH(true, FO_POINT, FO_LINEAR, FO_POINT):
			res = D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
			break;
		case MARGE_FOR_SWITCH(true, FO_POINT, FO_LINEAR, FO_LINEAR):
			res = D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
			break;
		case MARGE_FOR_SWITCH(true, FO_LINEAR, FO_POINT, FO_POINT):
			res = D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
			break;
		case MARGE_FOR_SWITCH(true, FO_LINEAR, FO_POINT, FO_LINEAR):
			res = D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
			break;		
		case MARGE_FOR_SWITCH(true, FO_LINEAR, FO_LINEAR, FO_POINT):
			res = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
			break;
		case MARGE_FOR_SWITCH(true, FO_LINEAR, FO_LINEAR, FO_LINEAR):
			res = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
			break;
		case MARGE_FOR_SWITCH(true, FO_ANISOTROPIC, FO_ANISOTROPIC, FO_ANISOTROPIC):
			res = D3D11_FILTER_COMPARISON_ANISOTROPIC;
			break;

		case MARGE_FOR_SWITCH(false, FO_POINT, FO_POINT, FO_POINT):
			res = D3D11_FILTER_MIN_MAG_MIP_POINT;
			break;
		case MARGE_FOR_SWITCH(false, FO_POINT, FO_POINT, FO_LINEAR):
			res = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
			break;
		case MARGE_FOR_SWITCH(false, FO_POINT, FO_LINEAR, FO_POINT):
			res = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
			break;
		case MARGE_FOR_SWITCH(false, FO_POINT, FO_LINEAR, FO_LINEAR):
			res = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
			break;
		case MARGE_FOR_SWITCH(false, FO_LINEAR, FO_POINT, FO_POINT):
			res = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
			break;
		case MARGE_FOR_SWITCH(false, FO_LINEAR, FO_POINT, FO_LINEAR):
			res = D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
			break;		
		case MARGE_FOR_SWITCH(false, FO_LINEAR, FO_LINEAR, FO_POINT):
			res = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			break;
		case MARGE_FOR_SWITCH(false, FO_LINEAR, FO_LINEAR, FO_LINEAR):
			res = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			break;
		case MARGE_FOR_SWITCH(false, FO_ANISOTROPIC, FO_ANISOTROPIC, FO_ANISOTROPIC):
			res = D3D11_FILTER_ANISOTROPIC;
			break;
		default:
			res = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		}	
		
		return res;
	}
	//---------------------------------------------------------------------
	/*DWORD D3D11Mappings::get(FilterType ft, FilterOptions fo, const D3DCAPS9& devCaps, 
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

	}*/
	//---------------------------------------------------------------------
	D3D11Mappings::eD3DTexType D3D11Mappings::get(TextureType ogreTexType)
	{
		switch( ogreTexType )
		{
		case TEX_TYPE_1D :
		case TEX_TYPE_2D :
			return D3D11Mappings::D3D_TEX_TYPE_NORMAL;
		case TEX_TYPE_CUBE_MAP :
			return D3D11Mappings::D3D_TEX_TYPE_CUBE;
		case TEX_TYPE_3D :
			return D3D11Mappings::D3D_TEX_TYPE_VOLUME;
		}
		return D3D11Mappings::D3D_TEX_TYPE_NONE;
	}
	//---------------------------------------------------------------------
	DWORD D3D11Mappings::get(HardwareBuffer::Usage usage)
	{
		DWORD ret = 0;
		if (usage & HardwareBuffer::HBU_DYNAMIC)
		{
#if OGRE_D3D_MANAGE_BUFFERS
			// Only add the dynamic flag for default pool, and
			// we use default pool when buffer is discardable
			if (usage & HardwareBuffer::HBU_DISCARDABLE)
				ret |= D3D11_USAGE_DYNAMIC;
#else
			ret |= D3D11_USAGE_DYNAMIC;
#endif
		}
		if (usage & HardwareBuffer::HBU_WRITE_ONLY)
		{
			ret |= D3D11_USAGE_DYNAMIC;
		}
		return ret;
	}
	//---------------------------------------------------------------------
	D3D11_MAP D3D11Mappings::get(HardwareBuffer::LockOptions options, HardwareBuffer::Usage usage)
	{
		D3D11_MAP ret = D3D11_MAP_READ_WRITE;
		if (options == HardwareBuffer::HBL_DISCARD)
		{
#if OGRE_D3D_MANAGE_BUFFERS
			// Only add the discard flag for dynamic usgae and default pool
			if ((usage & HardwareBuffer::HBU_DYNAMIC) &&
				(usage & HardwareBuffer::HBU_DISCARDABLE))
				ret = D3D11_MAP_WRITE_DISCARD;
#else
			// D3D doesn't like discard or no_overwrite on non-dynamic buffers
			if (usage & HardwareBuffer::HBU_DYNAMIC)
				ret = D3D11_MAP_WRITE_DISCARD;
#endif
		}
		if (options == HardwareBuffer::HBL_READ_ONLY)
		{
			// D3D debug runtime doesn't like you locking managed buffers readonly
			// when they were created with write-only (even though you CAN read
			// from the software backed version)
			if (!(usage & HardwareBuffer::HBU_WRITE_ONLY))
				ret = D3D11_MAP_READ;

		}
		if (options == HardwareBuffer::HBL_NO_OVERWRITE)
		{
#if OGRE_D3D_MANAGE_BUFFERS
			// Only add the nooverwrite flag for dynamic usgae and default pool
			if ((usage & HardwareBuffer::HBU_DYNAMIC) &&
				(usage & HardwareBuffer::HBU_DISCARDABLE))
				ret = D3D11_MAP_WRITE_NO_OVERWRITE;
#else
			// D3D doesn't like discard or no_overwrite on non-dynamic buffers
			if (usage & HardwareBuffer::HBU_DYNAMIC)
				ret = D3D11_MAP_WRITE_NO_OVERWRITE;
#endif 
		}

		return ret;
	}
	//---------------------------------------------------------------------
	UINT D3D11Mappings::getByteWidth(HardwareIndexBuffer::IndexType itype)
	{
		if (itype == HardwareIndexBuffer::IT_32BIT)
		{
			return sizeof(long);
		}
		else
		{
			return sizeof(short);
		}
	}
	//---------------------------------------------------------------------
	DXGI_FORMAT D3D11Mappings::getFormat(HardwareIndexBuffer::IndexType itype)
	{
		if (itype == HardwareIndexBuffer::IT_32BIT)
		{
			return DXGI_FORMAT_R32_UINT;
		}
		else
		{
			return DXGI_FORMAT_R16_UINT;
		}
	}
	//---------------------------------------------------------------------
	DXGI_FORMAT D3D11Mappings::get(VertexElementType vType)
	{
		switch (vType)
		{
		case VET_COLOUR:
		case VET_COLOUR_ABGR:
		case VET_COLOUR_ARGB:
			return DXGI_FORMAT_R32_UINT;
			break;
		case VET_FLOAT1:
			return DXGI_FORMAT_R32_FLOAT;
			break;
		case VET_FLOAT2:
			return DXGI_FORMAT_R32G32_FLOAT;
			break;
		case VET_FLOAT3:
			return DXGI_FORMAT_R32G32B32_FLOAT;
			break;
		case VET_FLOAT4:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
			break;
		case VET_SHORT2:
			return DXGI_FORMAT_R16G16_SINT;
			break;
		case VET_SHORT4:
			return DXGI_FORMAT_R16G16B16A16_SINT;
			break;
		case VET_UBYTE4:
			return DXGI_FORMAT_R8G8B8A8_UINT;
			break;
		}
		// to keep compiler happy
		return DXGI_FORMAT_R32G32B32_FLOAT;
	}
	//---------------------------------------------------------------------
	LPCSTR D3D11Mappings::get(VertexElementSemantic sem,unsigned short index)
	{
		// todo - add to ogre - POSITIONT and PSIZE ("Transformed vertex position" and "Point size")
		switch (sem)	
		{
		case VES_BLEND_INDICES:
			return "BLENDINDICES";
			break;
		case VES_BLEND_WEIGHTS:
			return "BLENDWEIGHT";
			break;
		case VES_DIFFUSE:
			return "COLOR"; // NB index will differentiate
			break;
		case VES_SPECULAR:
			return "COLOR"; // NB index will differentiate
			break;
		case VES_NORMAL:
			return "NORMAL";
			break;
		case VES_POSITION:
			return "POSITION";
			break;
		case VES_TEXTURE_COORDINATES:
			return "TEXCOORD";
			break;
		case VES_BINORMAL:
			return "BINORMAL";
			break;
		case VES_TANGENT:
			return "TANGENT";
			break;
		}
		// to keep compiler happy
		return "POSITION";
	}
	//---------------------------------------------------------------------
	void D3D11Mappings::get(const ColourValue& inColour, float * outColour )
	{
		outColour[0] = inColour.r;
		outColour[1] = inColour.g;
		outColour[2] = inColour.b;
		outColour[3] = inColour.a;	
	}
	//---------------------------------------------------------------------
	PixelFormat D3D11Mappings::_getPF(DXGI_FORMAT d3dPF)
	{
		switch(d3dPF)
		{

		case DXGI_FORMAT_UNKNOWN	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R32G32B32A32_TYPELESS	:
			return PF_FLOAT32_RGBA;
		case DXGI_FORMAT_R32G32B32A32_FLOAT	:
			return PF_FLOAT32_RGBA;
		case DXGI_FORMAT_R32G32B32A32_UINT	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R32G32B32A32_SINT	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R32G32B32_TYPELESS	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R32G32B32_FLOAT	:
			return PF_FLOAT32_RGB;
		case DXGI_FORMAT_R32G32B32_UINT	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R32G32B32_SINT	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R16G16B16A16_TYPELESS	:
			return PF_FLOAT16_RGBA;
		case DXGI_FORMAT_R16G16B16A16_FLOAT	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R16G16B16A16_UNORM	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R16G16B16A16_UINT	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R16G16B16A16_SNORM	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R16G16B16A16_SINT	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R32G32_TYPELESS	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R32G32_FLOAT	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R32G32_UINT	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R32G32_SINT	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R32G8X24_TYPELESS	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R10G10B10A2_TYPELESS	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R10G10B10A2_UNORM	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R10G10B10A2_UINT	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R11G11B10_FLOAT	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R8G8B8A8_TYPELESS	:
			return PF_R8G8B8A8;
		case DXGI_FORMAT_R8G8B8A8_UNORM	:
			return PF_R8G8B8A8;
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R8G8B8A8_UINT	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R8G8B8A8_SNORM	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R8G8B8A8_SINT	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R16G16_TYPELESS	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R16G16_FLOAT	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R16G16_UNORM	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R16G16_UINT	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R16G16_SNORM	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R16G16_SINT	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R32_TYPELESS	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_D32_FLOAT	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R32_FLOAT	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R32_UINT	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R32_SINT	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R24G8_TYPELESS	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_D24_UNORM_S8_UINT	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R8G8_TYPELESS	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R8G8_UNORM	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R8G8_UINT	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R8G8_SNORM	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R8G8_SINT	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R16_TYPELESS	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R16_FLOAT	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_D16_UNORM	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R16_UNORM	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R16_UINT	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R16_SNORM	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R16_SINT	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R8_TYPELESS	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R8_UNORM	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R8_UINT	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R8_SNORM	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R8_SINT	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_A8_UNORM	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R1_UNORM	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R9G9B9E5_SHAREDEXP	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R8G8_B8G8_UNORM	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_G8R8_G8B8_UNORM	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_BC1_TYPELESS	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_BC1_UNORM	:
			return PF_DXT1;
		case DXGI_FORMAT_BC1_UNORM_SRGB	:
			return PF_DXT1;
		case DXGI_FORMAT_BC2_TYPELESS	:
			return PF_DXT2;
		case DXGI_FORMAT_BC2_UNORM	:
			return PF_DXT2;
		case DXGI_FORMAT_BC2_UNORM_SRGB	:
			return PF_DXT2;
		case DXGI_FORMAT_BC3_TYPELESS	:
			return PF_DXT3;
		case DXGI_FORMAT_BC3_UNORM	:
			return PF_DXT3;
		case DXGI_FORMAT_BC3_UNORM_SRGB	:
			return PF_DXT3;
		case DXGI_FORMAT_BC4_TYPELESS	:
			return PF_DXT4;
		case DXGI_FORMAT_BC4_UNORM	:
			return PF_DXT4;
		case DXGI_FORMAT_BC4_SNORM	:
			return PF_DXT4;
		case DXGI_FORMAT_BC5_TYPELESS	:
			return PF_DXT5;
		case DXGI_FORMAT_BC5_UNORM	:
			return PF_DXT5;
		case DXGI_FORMAT_BC5_SNORM	:
			return PF_DXT5;
		case DXGI_FORMAT_B5G6R5_UNORM	:
			return PF_DXT5;
		case DXGI_FORMAT_B5G5R5A1_UNORM	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_B8G8R8A8_UNORM	:
			return PF_UNKNOWN;
		case DXGI_FORMAT_B8G8R8X8_UNORM	:
			return PF_UNKNOWN;
		default:
			return PF_UNKNOWN;
		}
	}
	//---------------------------------------------------------------------
	DXGI_FORMAT D3D11Mappings::_getPF(PixelFormat ogrePF)
	{
		switch(ogrePF)
		{
		case PF_L8:
			return DXGI_FORMAT_R8_UNORM;
		case PF_L16:
			return DXGI_FORMAT_R16_UNORM;
		case PF_A8:
			return DXGI_FORMAT_UNKNOWN;
		case PF_A4L4:
			return DXGI_FORMAT_UNKNOWN;
		case PF_BYTE_LA:
			return DXGI_FORMAT_UNKNOWN; 
		case PF_R3G3B2:
			return DXGI_FORMAT_UNKNOWN;
		case PF_A1R5G5B5:
			return DXGI_FORMAT_UNKNOWN;
		case PF_R5G6B5:
			return DXGI_FORMAT_UNKNOWN;
		case PF_A4R4G4B4:
			return DXGI_FORMAT_UNKNOWN;
		case PF_R8G8B8:
			return DXGI_FORMAT_UNKNOWN;
		case PF_A8R8G8B8:
			return DXGI_FORMAT_UNKNOWN;
		case PF_A8B8G8R8:
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		case PF_X8R8G8B8:
			return DXGI_FORMAT_UNKNOWN;
		case PF_X8B8G8R8:
			return DXGI_FORMAT_UNKNOWN;
		case PF_A2B10G10R10:
			return DXGI_FORMAT_R10G10B10A2_TYPELESS;
		case PF_A2R10G10B10:
			return DXGI_FORMAT_UNKNOWN;
		case PF_FLOAT16_R:
			return DXGI_FORMAT_R16_FLOAT;
		case PF_FLOAT16_RGBA:
			return DXGI_FORMAT_R16G16B16A16_FLOAT;
		case PF_FLOAT32_R:
			return DXGI_FORMAT_R32_FLOAT;
		case PF_FLOAT32_RGBA:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case PF_SHORT_RGBA:
			return DXGI_FORMAT_R16G16B16A16_UNORM;
		case PF_DXT1:
			return DXGI_FORMAT_BC1_UNORM;
		case PF_DXT2:
			return DXGI_FORMAT_BC2_UNORM;
		case PF_DXT3:
			return DXGI_FORMAT_BC3_UNORM;
		case PF_DXT4:
			return DXGI_FORMAT_BC4_UNORM;
		case PF_DXT5:
			return DXGI_FORMAT_BC5_UNORM;
		case PF_UNKNOWN:
		default:
			return DXGI_FORMAT_UNKNOWN;
		}
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
		case PF_FLOAT16_RGB:
			return PF_FLOAT16_RGBA;
		case PF_FLOAT32_RGB:
			return PF_FLOAT32_RGBA;
		case PF_UNKNOWN:
		default:
			return PF_A8B8G8R8;
		}
	}
	//---------------------------------------------------------------------
	D3D11_USAGE D3D11Mappings::_getUsage(HardwareBuffer::Usage mUsage)
	{
		if (_isDynamic(mUsage))
		{
			return D3D11_USAGE_DYNAMIC;
		}
		else
		{
			return D3D11_USAGE_DEFAULT;
		}
	}
	//---------------------------------------------------------------------
	bool D3D11Mappings::_isDynamic(HardwareBuffer::Usage mUsage)
	{
		switch ( mUsage)
		{
		case HardwareBuffer::HBU_DYNAMIC:
		case HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY:
		case HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE:
			return true;
		}

		return false;
	}
	//---------------------------------------------------------------------
	D3D11_USAGE D3D11Mappings::_getUsage(int mUsage)
	{
		return D3D11_USAGE_DEFAULT;
	}
	//---------------------------------------------------------------------
	UINT D3D11Mappings::_getAccessFlags(int mUsage)
	{
		return 0;
	}
	//---------------------------------------------------------------------
	UINT D3D11Mappings::_getAccessFlags(HardwareBuffer::Usage mUsage)
	{
		if (_isDynamic(mUsage))
		{
			return D3D11_CPU_ACCESS_WRITE;
		}
		else
		{
			return 0;
		}
	}
	//---------------------------------------------------------------------
	DXGI_FORMAT D3D11Mappings::get(HardwareIndexBuffer::IndexType itype)
	{
		if (itype == HardwareIndexBuffer::IT_32BIT)
		{
			//AIZTODO: PUT THOSE VALUES AND FIND OUT WHY WE WANT THEM
			return DXGI_FORMAT_R32_UINT;//return D3DFMT_INDEX32;
		}
		else
		{
			return DXGI_FORMAT_R16_UINT;//return D3DFMT_INDEX16;
		}
	}
	//---------------------------------------------------------------------
}
