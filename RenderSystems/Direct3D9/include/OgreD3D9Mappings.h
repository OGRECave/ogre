/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#ifndef __D3D9MAPPINGS_H__
#define __D3D9MAPPINGS_H__

#include "OgreD3D9Prerequisites.h"
#include "OgreCommon.h"
#include "OgreLight.h"
#include "OgreMaterial.h"
#include "OgreRenderSystem.h"
#include "OgreHardwareBuffer.h"
#include "OgreHardwareIndexBuffer.h"

namespace Ogre 
{
	class D3D9Mappings
	{
	public:
		/// enum identifying D3D9 tex. types
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

		/// enum identifying D3D9 filter usage type
		enum eD3DFilterUsage
		{
			/// min filter
			D3D_FUSAGE_MIN,
			/// mag filter
			D3D_FUSAGE_MAG,
			/// mip filter
			D3D_FUSAGE_MIP
		};

		/// return a D3D9 equivalent for a Ogre ShadeOptions value
		static DWORD get(ShadeOptions so);
		/// return a D3D9 equivalent for a Ogre LightTypes value
		static D3DLIGHTTYPE get(Ogre::Light::LightTypes lightType);
		/// return a D3D9 equivalent for a Ogre TexCoordCalsMethod value
		static DWORD get(TexCoordCalcMethod m, const D3DCAPS9& caps);
		/// return a D3D9 equivalent for a Ogre TextureAddressingMode value
		static D3DTEXTUREADDRESS get(TextureUnitState::TextureAddressingMode tam, const D3DCAPS9& devCaps);
		/// return a D3D9 equivalent for a Ogre LayerBlendType value
		static D3DTEXTURESTAGESTATETYPE get(LayerBlendType lbt);
		/// return a D3D9 equivalent for a Ogre LayerBlendOperationEx value
		static DWORD get(LayerBlendOperationEx lbo, const D3DCAPS9& devCaps);
		/// return a D3D9 equivalent for a Ogre LayerBlendSource value
		static DWORD get(LayerBlendSource lbs, bool perStageConstants);
		/// return a D3D9 equivalent for a Ogre SceneBlendFactor value
		static D3DBLEND get(SceneBlendFactor sbf);
		/// return a D3D9 equivlalent for a Ogre SceneBlendOperation value
		static D3DBLENDOP get(SceneBlendOperation sbo);
		/// return a D3D9 equivalent for a Ogre CompareFunction value
		static DWORD get(CompareFunction cf);
		/// return a D3D9 equivalent for a Ogre CillingMode value
		static DWORD get(CullingMode cm, bool flip);
		/// return a D3D9 equivalent for a Ogre FogMode value
		static D3DFOGMODE get(FogMode fm);
		/// return a D3D9 equivalent for a Ogre PolygonMode value
		static D3DFILLMODE get(PolygonMode level);
		/// return a D3D9 equivalent for a Ogre StencilOperation value
		static DWORD get(StencilOperation op, bool invert = false);
		/// return a D3D9 state type for Ogre FilterType value
		static D3DSAMPLERSTATETYPE get(FilterType ft);
		/// return a D3D9 filter option for Ogre FilterType & FilterOption value
		static DWORD get(FilterType ft, FilterOptions fo, const D3DCAPS9& devCaps, eD3DTexType texType);
		/// return the D3DtexType equivalent of a Ogre tex. type
		static eD3DTexType get(TextureType ogreTexType);
        /// return the combination of D3DUSAGE values for Ogre buffer usage
        static DWORD get(HardwareBuffer::Usage usage);
        /// Get lock options
        static DWORD get(HardwareBuffer::LockOptions options, HardwareBuffer::Usage usage);
        /// Get index type
        static D3DFORMAT get(HardwareIndexBuffer::IndexType itype);
		/// Get vertex data type
		static D3DDECLTYPE get(VertexElementType vType);
		/// Get vertex semantic
		static D3DDECLUSAGE get(VertexElementSemantic sem);
        // Convert matrix to D3D style
        static 	D3DXMATRIX makeD3DXMatrix( const Matrix4& mat );
        // Convert matrix from D3D style
        static Matrix4 convertD3DXMatrix( const D3DXMATRIX& mat );

		/// utility method, convert D3D9 pixel format to Ogre pixel format
		static PixelFormat _getPF(D3DFORMAT d3dPF);
		/// utility method, convert Ogre pixel format to D3D9 pixel format
		static D3DFORMAT _getPF(PixelFormat ogrePF);
		/// utility method, find closest Ogre pixel format that D3D9 can support
		static PixelFormat _getClosestSupportedPF(PixelFormat ogrePF);
	};
}
#endif
