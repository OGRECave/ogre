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
#include "OgreStableHeaders.h"

#include "OgreRenderSystemCapabilities.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreException.h"

namespace Ogre {

	//-----------------------------------------------------------------------
	RenderSystemCapabilities::RenderSystemCapabilities()
		: mVendor(GPU_UNKNOWN)
		, mNumWorldMatrices(0)
		, mNumTextureUnits(0)
		, mStencilBufferBitDepth(0)
		, mNumVertexBlendMatrices(0)
		, mNumMultiRenderTargets(1)
		, mNonPOW2TexturesLimited(false)
	{

		for(int i = 0; i < CAPS_CATEGORY_COUNT; i++)
		{
			mCapabilities[i] = 0;
		}
		mCategoryRelevant[CAPS_CATEGORY_COMMON] = true;
		mCategoryRelevant[CAPS_CATEGORY_COMMON_2] = true;
		// each rendersystem should enable these
		mCategoryRelevant[CAPS_CATEGORY_D3D9] = false;
		mCategoryRelevant[CAPS_CATEGORY_GL] = false;


	}
	//-----------------------------------------------------------------------
	RenderSystemCapabilities::~RenderSystemCapabilities()
	{
	}
	//-----------------------------------------------------------------------
	void RenderSystemCapabilities::log(Log* pLog)
	{
		pLog->logMessage("RenderSystem capabilities");
		pLog->logMessage("-------------------------");
		pLog->logMessage("RenderSystem Name: " + getRenderSystemName());
		pLog->logMessage("GPU Vendor: " + vendorToString(getVendor()));
		pLog->logMessage("Device Name: " + getDeviceName());
		pLog->logMessage("Driver Version: " + getDriverVersion().toString());
		pLog->logMessage(" * Fixed function pipeline: " 
			+ StringConverter::toString(hasCapability(RSC_FIXED_FUNCTION), true));
		pLog->logMessage(
			" * Hardware generation of mipmaps: "
			+ StringConverter::toString(hasCapability(RSC_AUTOMIPMAP), true));
		pLog->logMessage(
			" * Texture blending: "
			+ StringConverter::toString(hasCapability(RSC_BLENDING), true));
		pLog->logMessage(
			" * Anisotropic texture filtering: "
			+ StringConverter::toString(hasCapability(RSC_ANISOTROPY), true));
		pLog->logMessage(
			" * Dot product texture operation: "
			+ StringConverter::toString(hasCapability(RSC_DOT3), true));
		pLog->logMessage(
			" * Cube mapping: "
			+ StringConverter::toString(hasCapability(RSC_CUBEMAPPING), true));
		pLog->logMessage(
			" * Hardware stencil buffer: "
			+ StringConverter::toString(hasCapability(RSC_HWSTENCIL), true));
		if (hasCapability(RSC_HWSTENCIL))
		{
			pLog->logMessage(
				"   - Stencil depth: "
				+ StringConverter::toString(getStencilBufferBitDepth()));
			pLog->logMessage(
				"   - Two sided stencil support: "
				+ StringConverter::toString(hasCapability(RSC_TWO_SIDED_STENCIL), true));
			pLog->logMessage(
				"   - Wrap stencil values: "
				+ StringConverter::toString(hasCapability(RSC_STENCIL_WRAP), true));
		}
		pLog->logMessage(
			" * Hardware vertex / index buffers: "
			+ StringConverter::toString(hasCapability(RSC_VBO), true));
		pLog->logMessage(
			" * Vertex programs: "
			+ StringConverter::toString(hasCapability(RSC_VERTEX_PROGRAM), true));
		pLog->logMessage(
             " * Number of floating-point constants for vertex programs: "
             + StringConverter::toString(mVertexProgramConstantFloatCount));
		pLog->logMessage(
             " * Number of integer constants for vertex programs: "
             + StringConverter::toString(mVertexProgramConstantIntCount));
		pLog->logMessage(
             " * Number of boolean constants for vertex programs: "
             + StringConverter::toString(mVertexProgramConstantBoolCount));
		pLog->logMessage(
			" * Fragment programs: "
			+ StringConverter::toString(hasCapability(RSC_FRAGMENT_PROGRAM), true));
		pLog->logMessage(
             " * Number of floating-point constants for fragment programs: "
             + StringConverter::toString(mFragmentProgramConstantFloatCount));
		pLog->logMessage(
             " * Number of integer constants for fragment programs: "
             + StringConverter::toString(mFragmentProgramConstantIntCount));
		pLog->logMessage(
             " * Number of boolean constants for fragment programs: "
             + StringConverter::toString(mFragmentProgramConstantBoolCount));
		pLog->logMessage(
			" * Geometry programs: "
			+ StringConverter::toString(hasCapability(RSC_GEOMETRY_PROGRAM), true));
		pLog->logMessage(
             " * Number of floating-point constants for geometry programs: "
             + StringConverter::toString(mGeometryProgramConstantFloatCount));
		pLog->logMessage(
             " * Number of integer constants for geometry programs: "
             + StringConverter::toString(mGeometryProgramConstantIntCount));
		pLog->logMessage(
             " * Number of boolean constants for geometry programs: "
             + StringConverter::toString(mGeometryProgramConstantBoolCount));
		String profileList = "";
		for(ShaderProfiles::iterator iter = mSupportedShaderProfiles.begin(), end = mSupportedShaderProfiles.end();
			iter != end; ++iter)
		{
			profileList += " " + *iter;
		}
		pLog->logMessage(" * Supported Shader Profiles:" + profileList);

		pLog->logMessage(
			" * Texture Compression: "
			+ StringConverter::toString(hasCapability(RSC_TEXTURE_COMPRESSION), true));
		if (hasCapability(RSC_TEXTURE_COMPRESSION))
		{
			pLog->logMessage(
				"   - DXT: "
				+ StringConverter::toString(hasCapability(RSC_TEXTURE_COMPRESSION_DXT), true));
			pLog->logMessage(
				"   - VTC: "
				+ StringConverter::toString(hasCapability(RSC_TEXTURE_COMPRESSION_VTC), true));
			pLog->logMessage(
                 "   - PVRTC: "
                 + StringConverter::toString(hasCapability(RSC_TEXTURE_COMPRESSION_PVRTC), true));
		}

		pLog->logMessage(
			" * Scissor Rectangle: "
			+ StringConverter::toString(hasCapability(RSC_SCISSOR_TEST), true));
		pLog->logMessage(
			" * Hardware Occlusion Query: "
			+ StringConverter::toString(hasCapability(RSC_HWOCCLUSION), true));
		pLog->logMessage(
			" * User clip planes: "
			+ StringConverter::toString(hasCapability(RSC_USER_CLIP_PLANES), true));
		pLog->logMessage(
			" * VET_UBYTE4 vertex element type: "
			+ StringConverter::toString(hasCapability(RSC_VERTEX_FORMAT_UBYTE4), true));
		pLog->logMessage(
			" * Infinite far plane projection: "
			+ StringConverter::toString(hasCapability(RSC_INFINITE_FAR_PLANE), true));
		pLog->logMessage(
			" * Hardware render-to-texture: "
			+ StringConverter::toString(hasCapability(RSC_HWRENDER_TO_TEXTURE), true));
		pLog->logMessage(
			" * Floating point textures: "
			+ StringConverter::toString(hasCapability(RSC_TEXTURE_FLOAT), true));
		pLog->logMessage(
			" * Non-power-of-two textures: "
			+ StringConverter::toString(hasCapability(RSC_NON_POWER_OF_2_TEXTURES), true)
			+ (mNonPOW2TexturesLimited ? " (limited)" : ""));
		pLog->logMessage(
			" * Volume textures: "
			+ StringConverter::toString(hasCapability(RSC_TEXTURE_3D), true));
		pLog->logMessage(
			" * Multiple Render Targets: "
			+ StringConverter::toString(mNumMultiRenderTargets));
		pLog->logMessage(
			"   - With different bit depths: " + StringConverter::toString(hasCapability(RSC_MRT_DIFFERENT_BIT_DEPTHS), true));
		pLog->logMessage(
			" * Point Sprites: "
			+ StringConverter::toString(hasCapability(RSC_POINT_SPRITES), true));
		pLog->logMessage(
			" * Extended point parameters: "
			+ StringConverter::toString(hasCapability(RSC_POINT_EXTENDED_PARAMETERS), true));
		pLog->logMessage(
			" * Max Point Size: "
			+ StringConverter::toString(mMaxPointSize));
		pLog->logMessage(
			" * Vertex texture fetch: "
			+ StringConverter::toString(hasCapability(RSC_VERTEX_TEXTURE_FETCH), true));
		pLog->logMessage(
             " * Number of world matrices: "
             + StringConverter::toString(mNumWorldMatrices));
		pLog->logMessage(
             " * Number of texture units: "
             + StringConverter::toString(mNumTextureUnits));
		pLog->logMessage(
             " * Stencil buffer depth: "
             + StringConverter::toString(mStencilBufferBitDepth));
		pLog->logMessage(
             " * Number of vertex blend matrices: "
             + StringConverter::toString(mNumVertexBlendMatrices));
		if (hasCapability(RSC_VERTEX_TEXTURE_FETCH))
		{
			pLog->logMessage(
				"   - Max vertex textures: "
				+ StringConverter::toString(mNumVertexTextureUnits));
			pLog->logMessage(
				"   - Vertex textures shared: "
				+ StringConverter::toString(mVertexTextureUnitsShared, true));

		}
		pLog->logMessage(
			" * Render to Vertex Buffer : "
			+ StringConverter::toString(hasCapability(RSC_HWRENDER_TO_VERTEX_BUFFER), true));

		if (mCategoryRelevant[CAPS_CATEGORY_GL])
		{
			pLog->logMessage(
				" * GL 1.5 without VBO workaround: "
				+ StringConverter::toString(hasCapability(RSC_GL1_5_NOVBO), true));

			pLog->logMessage(
				" * Frame Buffer objects: "
				+ StringConverter::toString(hasCapability(RSC_FBO), true));
			pLog->logMessage(
				" * Frame Buffer objects (ARB extension): "
				+ StringConverter::toString(hasCapability(RSC_FBO_ARB), true));
			pLog->logMessage(
				" * Frame Buffer objects (ATI extension): "
				+ StringConverter::toString(hasCapability(RSC_FBO_ATI), true));
			pLog->logMessage(
				" * PBuffer support: "
				+ StringConverter::toString(hasCapability(RSC_PBUFFER), true));
			pLog->logMessage(
				" * GL 1.5 without HW-occlusion workaround: "
				+ StringConverter::toString(hasCapability(RSC_GL1_5_NOHWOCCLUSION), true));
		}

		if (mCategoryRelevant[CAPS_CATEGORY_D3D9])
		{
			pLog->logMessage(
				" * DirectX per stage constants: "
				+ StringConverter::toString(hasCapability(RSC_PERSTAGECONSTANT), true));
		}

	}
	//---------------------------------------------------------------------
	StringVector RenderSystemCapabilities::msGPUVendorStrings;
	//---------------------------------------------------------------------
	GPUVendor RenderSystemCapabilities::vendorFromString(const String& vendorString)
	{
		initVendorStrings();
		GPUVendor ret = GPU_UNKNOWN;
		String cmpString = vendorString;
		StringUtil::toLowerCase(cmpString);
		for (int i = 0; i < GPU_VENDOR_COUNT; ++i)
		{
			// case insensitive (lower case)
			if (msGPUVendorStrings[i] == cmpString)
			{
				ret = static_cast<GPUVendor>(i);
				break;
			}
		}

		return ret;
		
	}
	//---------------------------------------------------------------------
	String RenderSystemCapabilities::vendorToString(GPUVendor v)
	{
		initVendorStrings();
		return msGPUVendorStrings[v];
	}
	//---------------------------------------------------------------------
	void RenderSystemCapabilities::initVendorStrings()
	{
		if (msGPUVendorStrings.empty())
		{
			// Always lower case!
			msGPUVendorStrings.resize(GPU_VENDOR_COUNT);
			msGPUVendorStrings[GPU_UNKNOWN] = "unknown";
			msGPUVendorStrings[GPU_NVIDIA] = "nvidia";
			msGPUVendorStrings[GPU_ATI] = "ati";
			msGPUVendorStrings[GPU_INTEL] = "intel";
			msGPUVendorStrings[GPU_3DLABS] = "3dlabs";
			msGPUVendorStrings[GPU_S3] = "s3";
			msGPUVendorStrings[GPU_MATROX] = "matrox";
			msGPUVendorStrings[GPU_SIS] = "sis";
			msGPUVendorStrings[GPU_IMAGINATION_TECHNOLOGIES] = "imagination technologies";
			msGPUVendorStrings[GPU_APPLE] = "apple";    // iPhone Simulator
		}
	}

};
