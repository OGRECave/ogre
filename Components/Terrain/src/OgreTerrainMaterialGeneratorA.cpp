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


		mProfiles.push_back(OGRE_NEW SM2Profile(this, "SM2", "Profile for rendering on Shader Model 2 capable cards"));
		// TODO - check hardware capabilities & use fallbacks if required (more profiles needed)
		setActiveProfile("SM2");

	}
	//---------------------------------------------------------------------
	TerrainMaterialGeneratorA::~TerrainMaterialGeneratorA()
	{

	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	TerrainMaterialGeneratorA::SM2Profile::SM2Profile(TerrainMaterialGenerator* parent, const String& name, const String& desc)
		: Profile(parent, name, desc)
		, mShaderGen(0)
		, mLayerNormalMappingEnabled(true)
		, mLayerParallaxMappingEnabled(true)
		, mLayerSpecularMappingEnabled(true)
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
	void TerrainMaterialGeneratorA::SM2Profile::setLayerNormalMappingEnabled(bool enabled)
	{
		if (enabled != mLayerNormalMappingEnabled)
		{
			mLayerNormalMappingEnabled = enabled;
			mParent->_markChanged();
		}
	}
	//---------------------------------------------------------------------
	void TerrainMaterialGeneratorA::SM2Profile::setLayerParallaxMappingEnabled(bool enabled)
	{
		if (enabled != mLayerParallaxMappingEnabled)
		{
			mLayerParallaxMappingEnabled = enabled;
			mParent->_markChanged();
		}
	}
	//---------------------------------------------------------------------
	void TerrainMaterialGeneratorA::SM2Profile::setLayerSpecularMappingEnabled(bool enabled)
	{
		if (enabled != mLayerSpecularMappingEnabled)
		{
			mLayerSpecularMappingEnabled = enabled;
			mParent->_markChanged();
		}
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
		HighLevelGpuProgramPtr vprog = mShaderGen->generateVertexProgram(this, terrain);
		HighLevelGpuProgramPtr fprog = mShaderGen->generateFragmentProgram(this, terrain);

		pass->setVertexProgram(vprog->getName());
		pass->setFragmentProgram(fprog->getName());

		// global normal map
		TextureUnitState* tu = pass->createTextureUnitState();
		tu->setTextureName(terrain->getTerrainNormalMap()->getName());

		// blend maps
		for (uint i = 0; i < terrain->getBlendTextureCount(); ++i)
		{
			tu = pass->createTextureUnitState(terrain->getBlendTextureName(0));
			tu->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
		}

		// layer textures
		for (uint i = 0; i < terrain->getLayerCount(); ++i)
		{
			// diffuse / specular
			tu = pass->createTextureUnitState(terrain->getLayerTextureName(i, 0));
			// normal / height
			tu = pass->createTextureUnitState(terrain->getLayerTextureName(i, 1));
		}

		return mat;

	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	HighLevelGpuProgramPtr 
		TerrainMaterialGeneratorA::SM2Profile::ShaderHelper::generateVertexProgram(
			const SM2Profile* prof, const Terrain* terrain)
	{
		HighLevelGpuProgramPtr ret = createVertexProgram(terrain);

		StringUtil::StrStreamType sourceStr;
		generateVertexProgramSource(prof, terrain, sourceStr);
		ret->setSource(sourceStr.str());
		defaultVpParams(prof, terrain, ret);
#if OGRE_DEBUG_MODE
		LogManager::getSingleton().stream(LML_TRIVIAL) << "*** Terrain Vertex Program: " 
			<< ret->getName() << " ***\n" << ret->getSource() << "\n***   ***";
#endif

		return ret;

	}
	//---------------------------------------------------------------------
	HighLevelGpuProgramPtr 
	TerrainMaterialGeneratorA::SM2Profile::ShaderHelper::generateFragmentProgram(const SM2Profile* prof, const Terrain* terrain)
	{
		HighLevelGpuProgramPtr ret = createFragmentProgram(terrain);

		StringUtil::StrStreamType sourceStr;
		generateFragmentProgramSource(prof, terrain, sourceStr);
		ret->setSource(sourceStr.str());
		defaultFpParams(prof, terrain, ret);

#if OGRE_DEBUG_MODE
		LogManager::getSingleton().stream(LML_TRIVIAL) << "*** Terrain Fragment Program: " 
			<< ret->getName() << " ***\n" << ret->getSource() << "\n***   ***";
#endif

		return ret;
	}
	//---------------------------------------------------------------------
	void TerrainMaterialGeneratorA::SM2Profile::ShaderHelper::generateVertexProgramSource(
		const SM2Profile* prof, const Terrain* terrain, StringUtil::StrStreamType& outStream)
	{
		generateVpHeader(prof, terrain, outStream);

		for (uint i = 0; i < terrain->getLayerCount(); ++i)
			generateVpLayer(prof, terrain, i, outStream);

		generateVpFooter(prof, terrain, outStream);

	}
	//---------------------------------------------------------------------
	void TerrainMaterialGeneratorA::SM2Profile::ShaderHelper::generateFragmentProgramSource(
		const SM2Profile* prof, const Terrain* terrain, StringUtil::StrStreamType& outStream)
	{
		generateFpHeader(prof, terrain, outStream);

		for (uint i = 0; i < terrain->getLayerCount(); ++i)
			generateFpLayer(prof, terrain, i, outStream);

		generateFpFooter(prof, terrain, outStream);
	}
	//---------------------------------------------------------------------
	void TerrainMaterialGeneratorA::SM2Profile::ShaderHelper::defaultVpParams(
		const SM2Profile* prof, const Terrain* terrain, const HighLevelGpuProgramPtr& prog)
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
		const SM2Profile* prof, const Terrain* terrain, const HighLevelGpuProgramPtr& prog)
	{
		GpuProgramParametersSharedPtr params = prog->getDefaultParameters();
		params->setIgnoreMissingParams(true);

		params->setNamedAutoConstant("ambient", GpuProgramParameters::ACT_AMBIENT_LIGHT_COLOUR);
		params->setNamedAutoConstant("lightPosObjSpace", GpuProgramParameters::ACT_LIGHT_POSITION_OBJECT_SPACE, 0);
		params->setNamedAutoConstant("lightDiffuseColour", GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR, 0);
		params->setNamedAutoConstant("lightSpecularColour", GpuProgramParameters::ACT_LIGHT_SPECULAR_COLOUR, 0);
		params->setNamedAutoConstant("eyePosObjSpace", GpuProgramParameters::ACT_CAMERA_POSITION_OBJECT_SPACE);

		uint numUVMul = terrain->getLayerCount() / 4 + 1;
		for (uint i = 0; i < numUVMul; ++i)
		{
			Vector4 uvMul(
				terrain->getLayerUVMultiplier(i * 4), 
				terrain->getLayerUVMultiplier(i * 4 + 1), 
				terrain->getLayerUVMultiplier(i * 4 + 2), 
				terrain->getLayerUVMultiplier(i * 4 + 3) 
				);
			params->setNamedConstant("uvMul" + StringConverter::toString(i), uvMul);
		}
		
	}
	//---------------------------------------------------------------------
	String TerrainMaterialGeneratorA::SM2Profile::ShaderHelper::getChannel(uint idx)
	{
		uint rem = idx % 4;
		switch(rem)
		{
		case 0:
		default:
			return "r";
		case 1:
			return "g";
		case 2:
			return "b";
		case 3:
			return "a";
		};
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	HighLevelGpuProgramPtr
	TerrainMaterialGeneratorA::SM2Profile::ShaderHelperCg::createVertexProgram(const Terrain* terrain)
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
		TerrainMaterialGeneratorA::SM2Profile::ShaderHelperCg::createFragmentProgram(const Terrain* terrain)
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

		ret->setParameter("profiles", "ps_2_x fp40");
		ret->setParameter("entry_point", "main_fp");

		return ret;

	}
	//---------------------------------------------------------------------
	void TerrainMaterialGeneratorA::SM2Profile::ShaderHelperCg::generateVpHeader(
		const SM2Profile* prof, const Terrain* terrain, StringUtil::StrStreamType& outStream)
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
		const SM2Profile* prof, const Terrain* terrain, StringUtil::StrStreamType& outStream)
	{
		// Main header
		outStream << 
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
			"uniform sampler2D globalNormal : register(s0)\n"
			;

		uint currentSamplerIdx = 1;
		// Blend textures - sampler definitions
		for (uint i = 0; i < terrain->getBlendTextureCount(); ++i)
		{
			outStream << ", uniform sampler2D blendTex" << i 
				<< " : register(s" << currentSamplerIdx++ << ")\n";
		}

		// Layer textures - sampler definitions & UV multipliers
		for (uint i = 0; i < terrain->getLayerCount(); ++i)
		{
			outStream << ", uniform sampler2D difftex" << i 
				<< " : register(s" << currentSamplerIdx++ << ")\n";
			outStream << ", uniform sampler2D normtex" << i 
				<< " : register(s" << currentSamplerIdx++ << ")\n";
		}

		// uv multipliers
		uint numUVMultipliers = (terrain->getLayerCount() / 4) + 1;
		for (uint i = 0; i < numUVMultipliers; ++i)
			outStream << ", uniform float4 uvMul" << i << "\n";

		outStream << 
			") : COLOR\n"
			"{\n"
			"	float4 outputCol;\n"

			// base colour
			"	outputCol = float4(0,0,0,1);\n"

			// global normal
			"	float3 normal = expand(tex2D(globalNormal, uv)).rgb;\n"
			// Light - for now, just simple lighting
			"	float3 lightDir = \n"
			"		lightPosObjSpace.xyz -  (position.xyz * lightPosObjSpace.w);\n"

			"	float3 eyeDir = eyePosObjSpace - position.xyz;\n"

			// set up accumulation areas
			"	float3 diffuse = float3(0,0,0);\n"
			"	float specular = 0;\n";

		// set up the blend values
		for (uint i = 0; i < terrain->getBlendTextureCount(); ++i)
		{
			outStream << "	float4 blendTexVal" << i << " = tex2D(blendTex" << i << ", uv);\n";
		}

		if (prof->isLayerNormalMappingEnabled())
		{
			// derive the tangent space basis
			// we do this in the pixel shader because we don't have per-vertex normals
			// because of the LOD, we use a normal map
			// tangent is always +x or -z in object space depending on alignment
			switch(terrain->getAlignment())
			{
			case Terrain::ALIGN_X_Y:
			case Terrain::ALIGN_X_Z:
				outStream << "	float3 tangent = float3(1, 0, 0);\n";
				break;
			case Terrain::ALIGN_Y_Z:
				outStream << "	float3 tangent = float3(0, 0, -1);\n";
				break;
			};

			outStream << "	float3 binormal = normalize(cross(tangent, normal));\n";
			// note, now we need to re-cross to derive tangent again because it wasn't orthonormal
			outStream << "	tangent = normalize(cross(normal, binormal));\n";
			// derive final matrix
			outStream << "	float3x3 TBN = float3x3(tangent, binormal, normal);\n";

			// set up lighting result placeholders for interpolation
			outStream <<  "	float4 litRes, litResLayer;\n";
			outStream << "	float3 TSlightDir, TSeyeDir, TShalfAngle, TSnormal;\n";
			// move 
			outStream << "	TSlightDir = normalize(mul(TBN, lightDir));\n";
			outStream << "	TSeyeDir = normalize(mul(TBN, eyeDir));\n";

		}
		else
		{
			// simple per-pixel lighting with no normal mapping
			outStream << "	lightDir = normalize(lightDir);\n";
			outStream << "	eyeDir = normalize(eyeDir);\n";
			outStream << "	float3 halfAngle = normalize(lightDir + eyeDir);\n";
			outStream << "	float4 litRes = lit(dot(lightDir, normal), dot(halfAngle, normal), 30);\n";

		}


	}
	//---------------------------------------------------------------------
	void TerrainMaterialGeneratorA::SM2Profile::ShaderHelperCg::generateVpLayer(
		const SM2Profile* prof, const Terrain* terrain, uint layer, StringUtil::StrStreamType& outStream)
	{
		// nothing to do
	}
	//---------------------------------------------------------------------
	void TerrainMaterialGeneratorA::SM2Profile::ShaderHelperCg::generateFpLayer(
		const SM2Profile* prof, const Terrain* terrain, uint layer, StringUtil::StrStreamType& outStream)
	{
		uint uvIdx = layer / 4;
		uint blendIdx = (layer-1) / 4;

		// generate UV
		outStream << "	float2 uv" << layer << " = uv * uvMul" << uvIdx 
			<< "." << getChannel(layer) << ";\n";
		// sample diffuse texture
		outStream << "	float4 diffuseSpecTex" << layer 
			<< " = tex2D(difftex" << layer << ", uv" << layer << ");\n";

		// calculate lighting here if normal mapping
		if (prof->isLayerNormalMappingEnabled())
		{
			// access TS normal map
			outStream << "	TSnormal = expand(tex2D(normtex" << layer << ", uv" << layer << "));\n";
			outStream << "	TShalfAngle = normalize(TSlightDir + TSeyeDir);\n";
			outStream << "	litResLayer = lit(dot(TSlightDir, TSnormal), dot(TShalfAngle, TSnormal), 30);\n";
			if (!layer)
				outStream << "	litRes = litResLayer;\n";
			else
				outStream << "	litRes = lerp(litRes, litResLayer, blendTexVal" 
					<< blendIdx << "." << getChannel(layer-1) << ");\n";

		}

		// apply to common
		if (!layer)
		{
			outStream << "	diffuse = diffuseSpecTex0.rgb;\n";
			if (prof->isLayerSpecularMappingEnabled())
				outStream << "	specular = diffuseSpecTex0.a;\n";
		}
		else
		{
			outStream << "	diffuse = lerp(diffuse, diffuseSpecTex" << layer 
				<< ".rgb, blendTexVal" << blendIdx << "." << getChannel(layer-1) << ");\n";
			if (prof->isLayerSpecularMappingEnabled())
				outStream << "	specular = lerp(specular, diffuseSpecTex" << layer 
					<< ".a, blendTexVal" << blendIdx << "." << getChannel(layer-1) << ");\n";

		}
	}
	//---------------------------------------------------------------------
	void TerrainMaterialGeneratorA::SM2Profile::ShaderHelperCg::generateVpFooter(
		const SM2Profile* prof, const Terrain* terrain, StringUtil::StrStreamType& outStream)
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
		const SM2Profile* prof, const Terrain* terrain, StringUtil::StrStreamType& outStream)
	{
		// diffuse lighting
		outStream << "	outputCol.rgb += ambient * diffuse + litRes.y * lightDiffuseColour * diffuse;\n";

		// specular lighting
		if (!prof->isLayerSpecularMappingEnabled())
			outStream << "	specular = 1.0;\n";
		outStream << "	outputCol.rgb += litRes.z * lightSpecularColour * specular;\n";
		// Final return
		outStream << "	return outputCol;\n"
			"}\n"
			;

	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	HighLevelGpuProgramPtr
	TerrainMaterialGeneratorA::SM2Profile::ShaderHelperHLSL::createVertexProgram(const Terrain* terrain)
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
	TerrainMaterialGeneratorA::SM2Profile::ShaderHelperHLSL::createFragmentProgram(const Terrain* terrain)
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

		ret->setParameter("target", "ps_2_x");
		ret->setParameter("entry_point", "main_fp");

		return ret;

	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	HighLevelGpuProgramPtr
	TerrainMaterialGeneratorA::SM2Profile::ShaderHelperGLSL::createVertexProgram(const Terrain* terrain)
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
		TerrainMaterialGeneratorA::SM2Profile::ShaderHelperGLSL::createFragmentProgram(const Terrain* terrain)
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
