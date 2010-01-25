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
#ifndef __D3D10MAPPINGS_H__
#define __D3D10MAPPINGS_H__

#include "OgreD3D10Prerequisites.h"
#include "OgreTextureUnitState.h"
#include "OgreRenderSystem.h"

namespace Ogre 
{
	class D3D10Mappings
	{
	public:
		/// enum identifying D3D10 tex. types
		enum eD3DTexType
		{
			/// standard texture
			D3D_TEX_TYPE_NORMAL,
			/// cube texture
			D3D_TEX_TYPE_CUBE,
			/// volume texture
			D3D_TEX_TYPE_VOLUME,
			/// just to have it...
			D3D_TEX_TYPE_NONE
		};

		/// enum identifying D3D10 filter usage type
		enum eD3DFilterUsage
		{
			/// min filter
			D3D_FUSAGE_MIN,
			/// mag filter
			D3D_FUSAGE_MAG,
			/// mip filter
			D3D_FUSAGE_MIP
		};

		/// return a D3D10 equivalent for a Ogre ShadeOptions value
		//static DWORD get(ShadeOptions so);
		/// return a D3D10 equivalent for a Ogre LightTypes value
		//static D3DLIGHTTYPE get(Light::LightTypes lightType);
		/// return a D3D10 equivalent for a Ogre TexCoordCalsMethod value
		//static DWORD get(TexCoordCalcMethod m, const D3DCAPS9& caps);
		/// return a D3D10 equivalent for a Ogre TextureAddressingMode value
		static D3D10_TEXTURE_ADDRESS_MODE get(TextureUnitState::TextureAddressingMode tam);
		/// return a D3D10 equivalent for a Ogre LayerBlendType value
		static D3D10_BLEND get(LayerBlendType lbt);
		/// return a D3D10 equivalent for a Ogre LayerBlendOperationEx value
		//static DWORD get(LayerBlendOperationEx lbo, const D3DCAPS9& devCaps);
		/// return a D3D10 equivalent for a Ogre LayerBlendSource value
		//static DWORD get(LayerBlendSource lbs);
		/// return a D3D10 equivalent for a Ogre SceneBlendFactor value
		static D3D10_BLEND get(SceneBlendFactor sbf);
		/// return a D3D10 equivalent for a Ogre CompareFunction value
		static D3D10_COMPARISON_FUNC get(CompareFunction cf);
		/// return a D3D10 equivalent for a Ogre CillingMode value
		static D3D10_CULL_MODE get(CullingMode cm, bool flip = false);
		/// return a D3D10 equivalent for a Ogre FogMode value
		//static D3DFOGMODE get(FogMode fm);
		/// return a D3D10 equivalent for a Ogre PolygonMode value
		static D3D10_FILL_MODE get(PolygonMode level);
		/// return a D3D10 equivalent for a Ogre StencilOperation value
		static D3D10_STENCIL_OP get(StencilOperation op, bool invert = false);
		/// return a D3D10 state type for Ogre FilterType value
		static DWORD get(FilterType ft);
		static D3D10_FILTER get(const FilterOptions minification, const FilterOptions magnification, const FilterOptions mips, const bool comparison = false);
		/// return a D3D10 filter option for Ogre FilterType & FilterOption value
		//static DWORD get(FilterType ft, FilterOptions fo, const D3DCAPS9& devCaps, eD3DTexType texType);
		/// return the D3DtexType equivalent of a Ogre tex. type
		static eD3DTexType get(TextureType ogreTexType);
		/// return the combination of D3DUSAGE values for Ogre buffer usage
		static DWORD get(HardwareBuffer::Usage usage);
		/// Get lock options
		static D3D10_MAP get(HardwareBuffer::LockOptions options, HardwareBuffer::Usage usage);
		/// Get index type
		static DXGI_FORMAT get(HardwareIndexBuffer::IndexType itype);
		static UINT getByteWidth(HardwareIndexBuffer::IndexType itype);
		static DXGI_FORMAT getFormat(HardwareIndexBuffer::IndexType itype);
		/// Get vertex data type
		static DXGI_FORMAT get(VertexElementType vType);
		/// Get vertex semantic
		static LPCSTR get(VertexElementSemantic sem,unsigned short index);
		/// Get dx10 color
		static void get(const ColourValue& inColour, float * outColour );


		// Convert matrix to D3D style
		static 	D3DXMATRIX makeD3DXMatrix( const Matrix4& mat );
		// Convert matrix from D3D style
		static Matrix4 convertD3DXMatrix( const D3DXMATRIX& mat );

		/// utility method, convert D3D10 pixel format to Ogre pixel format
		static PixelFormat _getPF(DXGI_FORMAT d3dPF);
		/// utility method, convert Ogre pixel format to D3D10 pixel format
		static DXGI_FORMAT _getPF(PixelFormat ogrePF);
		//
		static D3D10_USAGE _getUsage(HardwareBuffer::Usage mUsage);
		static D3D10_USAGE _getUsage(int mUsage);
		static UINT _getAccessFlags(HardwareBuffer::Usage mUsage);
		static UINT _getAccessFlags(int mUsage);
		static bool _isDynamic(HardwareBuffer::Usage mUsage);
		/// utility method, find closest Ogre pixel format that D3D10 can support
		static PixelFormat _getClosestSupportedPF(PixelFormat ogrePF);
	};
}
#endif
