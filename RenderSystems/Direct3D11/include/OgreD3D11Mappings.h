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
#ifndef __D3D11MAPPINGS_H__
#define __D3D11MAPPINGS_H__

#include "OgreD3D11Prerequisites.h"
#include "OgreTextureUnitState.h"
#include "OgreRenderSystem.h"
#include "OgreHardwareIndexBuffer.h"

namespace Ogre 
{
    class _OgreD3D11Export D3D11Mappings
	{
	public:
		/// return a D3D11 equivalent for a Ogre TextureAddressingMode value
		static D3D11_TEXTURE_ADDRESS_MODE get(TextureAddressingMode tam);
		/// return a D3D11 equivalent for a Ogre SceneBlendFactor value
		static D3D11_BLEND get(SceneBlendFactor sbf, bool forAlpha);
		/// return a D3D11 equivalent for a Ogre SceneBlendOperation value
		static D3D11_BLEND_OP get(SceneBlendOperation sbo);
		/// return a D3D11 equivalent for a Ogre CompareFunction value
		static D3D11_COMPARISON_FUNC get(CompareFunction cf);
		/// return a D3D11 equivalent for a Ogre CillingMode value
		static D3D11_CULL_MODE get(CullingMode cm, bool flip = false);
		/// return a D3D11 equivalent for a Ogre PolygonMode value
		static D3D11_FILL_MODE get(PolygonMode level);
		/// return a D3D11 equivalent for a Ogre StencilOperation value
		static D3D11_STENCIL_OP get(StencilOperation op, bool invert = false);
		/// return a D3D11 state type for Ogre FilterOption min/mag/mip values
		static D3D11_FILTER get(const FilterOptions minification, const FilterOptions magnification, const FilterOptions mips, const bool comparison = false);
		/// Get lock options
		static D3D11_MAP get(HardwareBuffer::LockOptions options, HardwareBuffer::Usage usage);
		/// Get index type
		static DXGI_FORMAT getFormat(HardwareIndexBuffer::IndexType itype);
		/// Get vertex data type
		static DXGI_FORMAT get(VertexElementType vType);
		/// Get vertex semantic
		static LPCSTR get(VertexElementSemantic sem);
		static VertexElementSemantic get(LPCSTR sem);

		/// utility method, generates Ogre PixelBox using usual parameters and dataPtr/rowPitch/slicePitch from D3D11_MAPPED_SUBRESOURCE
		static PixelBox getPixelBoxWithMapping(D3D11_BOX extents, DXGI_FORMAT pixelFormat, const D3D11_MAPPED_SUBRESOURCE& mapping);
		/// utility method, applies dataPtr/rowPitch/slicePitch from D3D11_MAPPED_SUBRESOURCE to Ogre PixelBox
		static void setPixelBoxMapping(PixelBox& box, const D3D11_MAPPED_SUBRESOURCE& mapping);

		/// utility method, convert D3D11 pixel format to Ogre pixel format
		static PixelFormat _getPF(DXGI_FORMAT d3dPF);
		/// utility method, convert Ogre pixel format to D3D11 pixel format
		static DXGI_FORMAT _getPF(PixelFormat ogrePF);
		/// utility method, optionally maps plain format to _SRGB counterparts
		static DXGI_FORMAT _getGammaFormat(DXGI_FORMAT format, bool appendSRGB);
		static bool _isBinaryCompressedFormat(DXGI_FORMAT d3dPF);

		static D3D11_USAGE _getUsage(HardwareBuffer::Usage usage);
		static D3D11_USAGE _getUsage(TextureUsage usage) { return _getUsage(static_cast<HardwareBuffer::Usage>(usage)); }
		static UINT _getAccessFlags(HardwareBuffer::Usage usage);
		static UINT _getAccessFlags(TextureUsage usage) { return _getAccessFlags(static_cast<HardwareBuffer::Usage>(usage)); }
		static bool _isDynamic(HardwareBuffer::Usage usage);
		static bool _isDynamic(TextureUsage usage) { return _isDynamic(static_cast<HardwareBuffer::Usage>(usage)); }

		/// utility method, find closest Ogre pixel format that D3D11 can support
		static PixelFormat _getClosestSupportedPF(PixelFormat ogrePF);

		static TextureType _getTexType(D3D11_SRV_DIMENSION type);

		static UINT _getTextureBindFlags(DXGI_FORMAT format, TextureUsage usage);
		static UINT _getTextureMiscFlags(UINT bindflags, TextureType textype, TextureUsage usage);
	};
}
#endif
