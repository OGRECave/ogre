/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
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
#include "OgreTerrainMaterialGeneratorA.h"
#include "OgreTerrain.h"
#include "OgreMaterialManager.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreTextureUnitState.h"
#include "OgreGpuProgramManager.h"
#include "OgreHighLevelGpuProgramManager.h"

namespace Ogre
{
	//---------------------------------------------------------------------
	TerrainMaterialGeneratorA::TerrainMaterialGeneratorA()
	{
		// define the layers
		// We expect terrain textures to have no alpha, so we use the alpha channel
		// in the albedo texture to store specular reflection
		// similarly we double-up the normal and height (for parallax)
		mLayerDecl.samplers.push_back(TerrainLayerSampler("albedo_specular", PF_BYTE_RGBA));
		mLayerDecl.samplers.push_back(TerrainLayerSampler("normal_height", PF_BYTE_RGBA));
		
		mLayerDecl.elements.push_back(
			TerrainLayerSamplerElement(0, TLSS_ALBEDO, 0, 3));
		mLayerDecl.elements.push_back(
			TerrainLayerSamplerElement(0, TLSS_SPECULAR, 3, 1));
		mLayerDecl.elements.push_back(
			TerrainLayerSamplerElement(1, TLSS_NORMAL, 0, 3));
		mLayerDecl.elements.push_back(
			TerrainLayerSamplerElement(1, TLSS_HEIGHT, 3, 1));


		mProfiles.push_back(OGRE_NEW SM2Profile("SM2", "Profile for rendering on Shader Model 2 capable cards"));
		// TODO - check hardware capabilities & use fallbacks if required (more profiles needed)
		setActiveProfile("SM2");

	}
	//---------------------------------------------------------------------
	TerrainMaterialGeneratorA::~TerrainMaterialGeneratorA()
	{

	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	TerrainMaterialGeneratorA::SM2Profile::SM2Profile(const String& name, const String& desc)
		: Profile(name, desc)
		, mShaderGen(0)
	{

	}
	//---------------------------------------------------------------------
	TerrainMaterialGeneratorA::SM2Profile::~SM2Profile()
	{
		OGRE_DELETE mShaderGen;
	}	
	//---------------------------------------------------------------------
	void TerrainMaterialGeneratorA::SM2Profile::requestOptions(Terrain* terrain)
	{
		terrain->_setMorphRequired(true);
		terrain->_setNormalMapRequired(true);

		// TODO - shadow options
	}
	//---------------------------------------------------------------------
	MaterialPtr TerrainMaterialGeneratorA::SM2Profile::generate(const Terrain* terrain)
	{
		// re-use old material if exists
		MaterialPtr mat = terrain->_getMaterial();
		if (mat.isNull())
		{
			MaterialManager& matMgr = MaterialManager::getSingleton();

			// it's important that the names are deterministic for a given terrain, so
			// use the terrain pointer as an ID
			const String& matName = terrain->getMaterialName();
			mat = matMgr.getByName(matName);
			if (mat.isNull())
			{
				mat = matMgr.create(matName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
			}
		}

		// clear everything
		mat->removeAllTechniques();

		Technique* tech = mat->createTechnique();

		// TODO - determine the number of passes to generate
		Pass* pass = tech->createPass();

		GpuProgramManager& gmgr = GpuProgramManager::getSingleton();
		HighLevelGpuProgramManager& hmgr = HighLevelGpuProgramManager::getSingleton();
		if (!mShaderGen)
		{
			if (hmgr.isLanguageSupported("cg"))
				mShaderGen = OGRE_NEW ShaderHelperCg();
			else if (hmgr.isLanguageSupported("hlsl"))
				mShaderGen = OGRE_NEW ShaderHelperHLSL();
			else if (hmgr.isLanguageSupported("glsl"))
				mShaderGen = OGRE_NEW ShaderHelperGLSL();
			else
			{
				// todo
			}
		}
		HighLevelGpuProgramPtr vprog = mShaderGen->generateVertexProgram(terrain, 0);
		HighLevelGpuProgramPtr fprog = mShaderGen->generateFragmentProgram(terrain, 0);

		pass->setVertexProgram(vprog->getName());
		pass->setFragmentProgram(fprog->getName());

		// global normal map
		TextureUnitState* tu = pass->createTextureUnitState();
		tu->setTextureName(terrain->getTerrainNormalMap()->getName());

		// blend maps
		// TODO - determine the actual number
		tu = pass->createTextureUnitState();
		tu->setTextureName(terrain->getBlendTextureName(0));


		// TODO - determine the number of texture units
		// TODO - map in normal & specular
		tu = pass->createTextureUnitState();
		tu->setTextureName(terrain->getLayerTextureName(0, 0));
		tu = pass->createTextureUnitState();
		tu->setTextureName(terrain->getLayerTextureName(1, 0));

		return mat;

	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	HighLevelGpuProgramPtr 
		TerrainMaterialGeneratorA::SM2Profile::ShaderHelper::generateVertexProgram(const Terrain* terrain, uint8 startLayer)
	{
		HighLevelGpuProgramPtr ret = createVertexProgram(terrain, startLayer);

		StringUtil::StrStreamType sourceStr;
		generateVertexProgramSource(terrain, startLayer, sourceStr);
		ret->setSource(sourceStr.str());
		defaultVpParams(terrain, startLayer, ret);
#if OGRE_DEBUG_MODE
		LogManager::getSingleton().stream(LML_TRIVIAL) << "*** Terrain Vertex Program: " 
			<< ret->getName() << " ***\n" << ret->getSource() << "\n***   ***";
#endif

		return ret;

	}
	//---------------------------------------------------------------------
	HighLevelGpuProgramPtr 
	TerrainMaterialGeneratorA::SM2Profile::ShaderHelper::generateFragmentProgram(const Terrain* terrain, uint8 startLayer)
	{
		HighLevelGpuProgramPtr ret = createFragmentProgram(terrain, startLayer);

		StringUtil::StrStreamType sourceStr;
		generateFragmentProgramSource(terrain, startLayer, sourceStr);
		ret->setSource(sourceStr.str());
		defaultFpParams(terrain, startLayer, ret);

#if OGRE_DEBUG_MODE
		LogManager::getSingleton().stream(LML_TRIVIAL) << "*** Terrain Fragment Program: " 
			<< ret->getName() << " ***\n" << ret->getSource() << "\n***   ***";
#endif

		return ret;
	}
	//---------------------------------------------------------------------
	void TerrainMaterialGeneratorA::SM2Profile::ShaderHelper::generateVertexProgramSource(
		const Terrain* terrain, uint8 startLayer, StringUtil::StrStreamType& outStream)
	{
		generateVpHeader(terrain, startLayer, outStream);

		// TODO
		//generateVpLayer(terrain, layer, outStream);

		generateVpFooter(terrain, startLayer, outStream);

	}
	//---------------------------------------------------------------------
	void TerrainMaterialGeneratorA::SM2Profile::ShaderHelper::generateFragmentProgramSource(
		const Terrain* terrain, uint8 startLayer, StringUtil::StrStreamType& outStream)
	{
		generateFpHeader(terrain, startLayer, outStream);

		// TODO
		//generateFpLayer(terrain, layer, outStream);

		generateFpFooter(terrain, startLayer, outStream);
	}
	//---------------------------------------------------------------------
	void TerrainMaterialGeneratorA::SM2Profile::ShaderHelper::defaultVpParams(
		const Terrain* terrain, uint8 startLayer, const HighLevelGpuProgramPtr& prog)
	{
		GpuProgramParametersSharedPtr params = prog->getDefaultParameters();
		params->setIgnoreMissingParams(true);
		params->setNamedAutoConstant("worldMatrix", GpuProgramParameters::ACT_WORLD_MATRIX);
		params->setNamedAutoConstant("viewProjMatrix", GpuProgramParameters::ACT_VIEWPROJ_MATRIX);
		params->setNamedAutoConstant("lodMorph", GpuProgramParameters::ACT_CUSTOM, 
			Terrain::LOD_MORPH_CUSTOM_PARAM);

	}
	//---------------------------------------------------------------------
	void TerrainMaterialGeneratorA::SM2Profile::ShaderHelper::defaultFpParams(
		const Terrain* terrain, uint8 startLayer, const HighLevelGpuProgramPtr& prog)
	{
		GpuProgramParametersSharedPtr params = prog->getDefaultParameters();
		params->setIgnoreMissingParams(true);

		params->setNamedAutoConstant("ambient", GpuProgramParameters::ACT_AMBIENT_LIGHT_COLOUR);
		params->setNamedAutoConstant("lightPosObjSpace", GpuProgramParameters::ACT_LIGHT_POSITION_OBJECT_SPACE, 0);
		params->setNamedAutoConstant("lightDiffuseColour", GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR, 0);
		params->setNamedAutoConstant("lightSpecularColour", GpuProgramParameters::ACT_LIGHT_SPECULAR_COLOUR, 0);
		params->setNamedAutoConstant("eyePosObjSpace", GpuProgramParameters::ACT_CAMERA_POSITION_OBJECT_SPACE);

		// TODO: temp hack remove!
		Vector4 uvMul(terrain->getLayerUVMultiplier(0), terrain->getLayerUVMultiplier(1), 0, 0);
		params->setNamedConstant("uvMul", uvMul);
		
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	HighLevelGpuProgramPtr
	TerrainMaterialGeneratorA::SM2Profile::ShaderHelperCg::createVertexProgram(const Terrain* terrain, uint8 startLayer)
	{
		HighLevelGpuProgramManager& mgr = HighLevelGpuProgramManager::getSingleton();
		String progName = terrain->getMaterialName() + "/sm2/vp";

		HighLevelGpuProgramPtr ret = mgr.getByName(progName);
		if (ret.isNull())
		{
			ret = mgr.createProgram(progName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
				"cg", GPT_VERTEX_PROGRAM);
		}
		else
		{
			ret->unload();
		}

		ret->setParameter("profiles", "vs_2_0 arbvp1");
		ret->setParameter("entry_point", "main_vp");

		return ret;

	}
	//---------------------------------------------------------------------
	HighLevelGpuProgramPtr
		TerrainMaterialGeneratorA::SM2Profile::ShaderHelperCg::createFragmentProgram(const Terrain* terrain, uint8 startLayer)
	{
		HighLevelGpuProgramManager& mgr = HighLevelGpuProgramManager::getSingleton();
		String progName = terrain->getMaterialName() + "/sm2/fp";

		HighLevelGpuProgramPtr ret = mgr.getByName(progName);
		if (ret.isNull())
		{
			ret = mgr.createProgram(progName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
				"cg", GPT_FRAGMENT_PROGRAM);
		}
		else
		{
			ret->unload();
		}

		ret->setParameter("profiles", "ps_2_0 arbfp1");
		ret->setParameter("entry_point", "main_fp");

		return ret;

	}
	//---------------------------------------------------------------------
	void TerrainMaterialGeneratorA::SM2Profile::ShaderHelperCg::generateVpHeader(
		const Terrain* terrain, uint8 startLayer, StringUtil::StrStreamType& outStream)
	{
		static const String vpHeader = 
			"void main_vp(\n"
			"float4 pos : POSITION,\n"
			"float4 uv  : TEXCOORD0,\n" // u,v, lodDelta, lodThreshold
			
			"uniform float4x4 worldMatrix,\n"
			"uniform float4x4 viewProjMatrix,\n"
			"uniform float2   lodMorph,\n" // morph amount, morph LOD target
			
			"out float4 oPos : POSITION,\n"
			"out float2 oUV	 : TEXCOORD0, \n"
			"out float4 oPosObj : TEXCOORD1 \n"

			")\n"
			"{\n"
			"	float4 worldPos = mul(worldMatrix, pos);\n"
			"	oPosObj = pos;"

			// determine whether to apply the LOD morph to this vertex
			// we store the deltas against all vertices so we only want to apply 
			// the morph to the ones which would disappear. The target LOD which is
			// being morphed to is stored in lodMorph.y, and the LOD at which 
			// the vertex should be morphed is stored in uv.w. If we subtract
			// the former from the latter, and arrange to only morph if the
			// result is negative (it will only be -1 in fact, since after that
			// the vertex will never be indexed), we will achieve our aim.
			// sign(vertexLOD - targetLOD) == -1 is to morph
			"	float toMorph = -min(0, sign(uv.w - lodMorph.y));\n"
			// this will either be 1 (morph) or 0 (don't morph)

			;

		outStream << vpHeader;

		// morph
		switch (terrain->getAlignment())
		{
		case Terrain::ALIGN_X_Y:
			break;
		case Terrain::ALIGN_X_Z:
			outStream << "worldPos.y += uv.z * toMorph * lodMorph.x;\n";
			break;
		case Terrain::ALIGN_Y_Z:
			break;
		};
			


	}
	//---------------------------------------------------------------------
	void TerrainMaterialGeneratorA::SM2Profile::ShaderHelperCg::generateFpHeader(
		const Terrain* terrain, uint8 startLayer, StringUtil::StrStreamType& outStream)
	{
		static const String fpHeader = 
			// helpers
			"float4 expand(float4 v)\n"
			"{ \n"
			"	return v * 2 - 1;\n"
			"}\n\n\n"

			"float4 main_fp(\n"
			"float2 uv : TEXCOORD0,\n"
			"float4 position : TEXCOORD1,\n"
			
			// Only 1 light supported in this version
			// deferred shading profile / generator later, ok? :)
			"uniform float4 ambient,\n"
			"uniform float4 lightPosObjSpace,\n"
			"uniform float3 lightDiffuseColour,\n"
			"uniform float3 lightSpecularColour,\n"
			"uniform float3 eyePosObjSpace,\n"
			// TODO - remove this - iterate instead
			"uniform float4 uvMul,\n"
			"uniform sampler2D globalNormal : register(s0),\n"
			"uniform sampler2D blendTex0 : register(s1)\n"

			", uniform sampler2D tex0 : register(s2)\n"
			", uniform sampler2D tex1 : register(s3)\n"

			") : COLOR\n"
			"{\n"
			"	float4 outputCol;\n"
			// ambient colour is universal
			"	outputCol = ambient;\n"

			// global normal
			"	float3 normal = expand(tex2D(globalNormal, uv)).rgb;\n"
			// Light - for now, just simple lighting
			"	float3 lightDir = normalize( \n"
			"	lightPosObjSpace.xyz -  (position.xyz * lightPosObjSpace.w));\n"
			

			// for now, just single lighting result (no normal mapping)
			"	float3 eyeDir = eyePosObjSpace - position.xyz;\n"
			"	float3 halfAngle = normalize(lightDir + eyeDir);\n"
			"	float4 litRes = lit(dot(lightDir, normal), dot(halfAngle, normal), 30);\n"

			// TODO - remove this - iterate instead
			"	float2 uv0 = uv * uvMul.x;\n"
			"	float3 diffuseTex0 = tex2D(tex0, uv0).rgb;\n"
			"	float2 uv1 = uv * uvMul.y;\n"
			"	float3 diffuseTex1 = tex2D(tex1, uv0).rgb;\n"

			// Blend 
			"	float4 blend1to4 = tex2D(blendTex0, uv);\n"
			"	float3 diffuseTex = diffuseTex0;\n"
			"	diffuseTex = lerp(diffuseTex, diffuseTex1, blend1to4.x);\n"
			// diffuse
			"	outputCol.rgb += litRes.y * lightDiffuseColour * diffuseTex;\n"
			// specular
			"	outputCol.rgb += litRes.z * lightSpecularColour;\n"
			;

		outStream << fpHeader;

	}
	//---------------------------------------------------------------------
	void TerrainMaterialGeneratorA::SM2Profile::ShaderHelperCg::generateVpLayer(
		const Terrain* terrain, uint8 layer, StringUtil::StrStreamType& outStream)
	{
		// TODO

	}
	//---------------------------------------------------------------------
	void TerrainMaterialGeneratorA::SM2Profile::ShaderHelperCg::generateFpLayer(
		const Terrain* terrain, uint8 layer, StringUtil::StrStreamType& outStream)
	{
		// TODO
	}
	//---------------------------------------------------------------------
	void TerrainMaterialGeneratorA::SM2Profile::ShaderHelperCg::generateVpFooter(
		const Terrain* terrain, uint8 startLayer, StringUtil::StrStreamType& outStream)
	{
		static const String vpFooter = 
			"	oPos = mul(viewProjMatrix, worldPos);\n"
			"	oUV = uv.xy;\n"
			"}\n"
			;

		outStream << vpFooter;

	}
	//---------------------------------------------------------------------
	void TerrainMaterialGeneratorA::SM2Profile::ShaderHelperCg::generateFpFooter(
		const Terrain* terrain, uint8 startLayer, StringUtil::StrStreamType& outStream)
	{
		static const String fpFooter = 
			"	return outputCol;\n"
			"}\n"
			;

		outStream << fpFooter;
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	HighLevelGpuProgramPtr
	TerrainMaterialGeneratorA::SM2Profile::ShaderHelperHLSL::createVertexProgram(const Terrain* terrain, uint8 startLayer)
	{
		HighLevelGpuProgramManager& mgr = HighLevelGpuProgramManager::getSingleton();
		String progName = terrain->getMaterialName() + "/sm2/vp";

		HighLevelGpuProgramPtr ret = mgr.getByName(progName);
		if (ret.isNull())
		{
			ret = mgr.createProgram(progName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
				"hlsl", GPT_VERTEX_PROGRAM);
		}
		else
		{
			ret->unload();
		}

		ret->setParameter("target", "vs_2_0");
		ret->setParameter("entry_point", "main_vp");

		return ret;

	}
	//---------------------------------------------------------------------
	HighLevelGpuProgramPtr
	TerrainMaterialGeneratorA::SM2Profile::ShaderHelperHLSL::createFragmentProgram(const Terrain* terrain, uint8 startLayer)
	{
		HighLevelGpuProgramManager& mgr = HighLevelGpuProgramManager::getSingleton();
		String progName = terrain->getMaterialName() + "/sm2/fp";

		HighLevelGpuProgramPtr ret = mgr.getByName(progName);
		if (ret.isNull())
		{
			ret = mgr.createProgram(progName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
				"hlsl", GPT_FRAGMENT_PROGRAM);
		}
		else
		{
			ret->unload();
		}

		ret->setParameter("target", "ps_2_0");
		ret->setParameter("entry_point", "main_fp");

		return ret;

	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	HighLevelGpuProgramPtr
	TerrainMaterialGeneratorA::SM2Profile::ShaderHelperGLSL::createVertexProgram(const Terrain* terrain, uint8 startLayer)
	{
		HighLevelGpuProgramManager& mgr = HighLevelGpuProgramManager::getSingleton();
		String progName = terrain->getMaterialName() + "/sm2/vp";

		HighLevelGpuProgramPtr ret = mgr.getByName(progName);
		if (ret.isNull())
		{
			ret = mgr.createProgram(progName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
				"glsl", GPT_VERTEX_PROGRAM);
		}
		else
		{
			ret->unload();
		}

		return ret;

	}
	//---------------------------------------------------------------------
	HighLevelGpuProgramPtr
		TerrainMaterialGeneratorA::SM2Profile::ShaderHelperGLSL::createFragmentProgram(const Terrain* terrain, uint8 startLayer)
	{
		HighLevelGpuProgramManager& mgr = HighLevelGpuProgramManager::getSingleton();
		String progName = terrain->getMaterialName() + "/sm2/fp";

		HighLevelGpuProgramPtr ret = mgr.getByName(progName);
		if (ret.isNull())
		{
			ret = mgr.createProgram(progName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
				"glsl", GPT_FRAGMENT_PROGRAM);
		}
		else
		{
			ret->unload();
		}

		return ret;

	}


}
