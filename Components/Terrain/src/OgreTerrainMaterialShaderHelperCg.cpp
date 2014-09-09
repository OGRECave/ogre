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
#include "OgreTerrainMaterialGeneratorA.h"
#include "OgreTerrain.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreShadowCameraSetupPSSM.h"

namespace Ogre
{
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	HighLevelGpuProgramPtr
	TerrainMaterialGeneratorA::SM2Profile::ShaderHelperCg::createVertexProgram(
		const SM2Profile* prof, const Terrain* terrain, TechniqueType tt)
	{
		HighLevelGpuProgramManager& mgr = HighLevelGpuProgramManager::getSingleton();
		String progName = getVertexProgramName(prof, terrain, tt);
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

		ret->setParameter("profiles", "vs_4_0 vs_3_0 vs_2_0 arbvp1");
		ret->setParameter("entry_point", "main_vp");

		return ret;

	}
	//---------------------------------------------------------------------
	HighLevelGpuProgramPtr
		TerrainMaterialGeneratorA::SM2Profile::ShaderHelperCg::createFragmentProgram(
			const SM2Profile* prof, const Terrain* terrain, TechniqueType tt)
	{
		HighLevelGpuProgramManager& mgr = HighLevelGpuProgramManager::getSingleton();
		String progName = getFragmentProgramName(prof, terrain, tt);

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
		
		if(prof->isLayerNormalMappingEnabled() || prof->isLayerParallaxMappingEnabled())
			ret->setParameter("profiles", "ps_4_0 ps_3_0 ps_2_x fp40 arbfp1");
		else
			ret->setParameter("profiles", "ps_4_0 ps_3_0 ps_2_0 fp30 arbfp1");
		ret->setParameter("entry_point", "main_fp");

		return ret;

	}
	//---------------------------------------------------------------------
	void TerrainMaterialGeneratorA::SM2Profile::ShaderHelperCg::generateVpHeader(
		const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringUtil::StrStreamType& outStream)
	{
		outStream << 
			"void main_vp(\n";
		bool compression = terrain->_getUseVertexCompression() && tt != RENDER_COMPOSITE_MAP;
		if (compression)
		{
			const char* idx2 = prof->_isSM4Available() ? "int2" : "float2";
			outStream << 
				idx2 << " posIndex : POSITION,\n"
				"float height  : TEXCOORD0,\n";
		}
		else
		{
			outStream <<
				"float4 pos : POSITION,\n"
				"float2 uv  : TEXCOORD0,\n";

		}
		if (tt != RENDER_COMPOSITE_MAP)
			outStream << "float2 delta  : TEXCOORD1,\n"; // lodDelta, lodThreshold

		outStream << 
			"uniform float4x4 worldMatrix,\n"
			"uniform float4x4 viewProjMatrix,\n"
			"uniform float2   lodMorph,\n"; // morph amount, morph LOD target

		if (compression)
		{
			outStream << 
				"uniform float4x4   posIndexToObjectSpace,\n"
				"uniform float    baseUVScale,\n";
		}
		// uv multipliers
		uint maxLayers = prof->getMaxLayers(terrain);
		uint numLayers = std::min(maxLayers, static_cast<uint>(terrain->getLayerCount()));
		uint numUVMultipliers = (numLayers / 4);
		if (numLayers % 4)
			++numUVMultipliers;
		for (uint i = 0; i < numUVMultipliers; ++i)
			outStream << "uniform float4 uvMul_" << i << ", \n";

		outStream <<
			"out float4 oPos : POSITION,\n"
			"out float4 oPosObj : TEXCOORD0 \n";

		uint texCoordSet = 1;
		outStream <<
			", out float4 oUVMisc : TEXCOORD" << texCoordSet++ <<" // xy = uv, z = camDepth\n";

		// layer UV's premultiplied, packed as xy/zw
		uint numUVSets = numLayers / 2;
		if (numLayers % 2)
			++numUVSets;
		if (tt != LOW_LOD)
		{
			for (uint i = 0; i < numUVSets; ++i)
			{
				outStream <<
					", out float4 oUV" << i << " : TEXCOORD" << texCoordSet++ << "\n";
			}
		}

		if (prof->getParent()->getDebugLevel() && tt != RENDER_COMPOSITE_MAP)
		{
			outStream << ", out float2 lodInfo : TEXCOORD" << texCoordSet++ << "\n";
		}

		bool fog = terrain->getSceneManager()->getFogMode() != FOG_NONE && tt != RENDER_COMPOSITE_MAP;
		if (fog)
		{
			outStream <<
				", uniform float4 fogParams\n"
				", out float fogVal : COLOR\n";
		}

		if (prof->isShadowingEnabled(tt, terrain))
		{
			texCoordSet = generateVpDynamicShadowsParams(texCoordSet, prof, terrain, tt, outStream);
		}

		// check we haven't exceeded texture coordinates
		if (texCoordSet > 8)
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
				"Requested options require too many texture coordinate sets! Try reducing the number of layers.",
				__FUNCTION__);
		}

		outStream <<
			")\n"
			"{\n";
		if (compression)
		{
			outStream <<
				"	float4 pos;\n"
				"	pos = mul(posIndexToObjectSpace, float4(posIndex, height, 1));\n"
				"   float2 uv = float2(posIndex.x * baseUVScale, 1.0 - (posIndex.y * baseUVScale));\n";
		}
		outStream <<
			"	float4 worldPos = mul(worldMatrix, pos);\n"
			"	oPosObj = pos;\n";

		if (tt != RENDER_COMPOSITE_MAP)
		{
			// determine whether to apply the LOD morph to this vertex
			// we store the deltas against all vertices so we only want to apply 
			// the morph to the ones which would disappear. The target LOD which is
			// being morphed to is stored in lodMorph.y, and the LOD at which 
			// the vertex should be morphed is stored in uv.w. If we subtract
			// the former from the latter, and arrange to only morph if the
			// result is negative (it will only be -1 in fact, since after that
			// the vertex will never be indexed), we will achieve our aim.
			// sign(vertexLOD - targetLOD) == -1 is to morph
			outStream << 
				"	float toMorph = -min(0, sign(delta.y - lodMorph.y));\n";
			// this will either be 1 (morph) or 0 (don't morph)
			if (prof->getParent()->getDebugLevel())
			{
				// x == LOD level (-1 since value is target level, we want to display actual)
				outStream << "lodInfo.x = (lodMorph.y - 1) / " << terrain->getNumLodLevels() << ";\n";
				// y == LOD morph
				outStream << "lodInfo.y = toMorph * lodMorph.x;\n";
			}

			// morph
			switch (terrain->getAlignment())
			{
			case Terrain::ALIGN_X_Y:
				outStream << "	worldPos.z += delta.x * toMorph * lodMorph.x;\n";
				break;
			case Terrain::ALIGN_X_Z:
				outStream << "	worldPos.y += delta.x * toMorph * lodMorph.x;\n";
				break;
			case Terrain::ALIGN_Y_Z:
				outStream << "	worldPos.x += delta.x * toMorph * lodMorph.x;\n";
				break;
			};
		}


		// generate UVs
		if (tt != LOW_LOD)
		{
			for (uint i = 0; i < numUVSets; ++i)
			{
				uint layer  =  i * 2;
				uint uvMulIdx = layer / 4;

				outStream <<
					"	oUV" << i << ".xy = " << " uv.xy * uvMul_" << uvMulIdx << "." << getChannel(layer) << ";\n";
				outStream <<
					"	oUV" << i << ".zw = " << " uv.xy * uvMul_" << uvMulIdx << "." << getChannel(layer+1) << ";\n";
				
			}

		}	


	}
	//---------------------------------------------------------------------
	void TerrainMaterialGeneratorA::SM2Profile::ShaderHelperCg::generateFpHeader(
		const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringUtil::StrStreamType& outStream)
	{

		// Main header
		outStream << 
			// helpers
			"float4 expand(float4 v)\n"
			"{ \n"
			"	return v * 2 - 1;\n"
			"}\n\n\n";

		if (prof->isShadowingEnabled(tt, terrain))
			generateFpDynamicShadowsHelpers(prof, terrain, tt, outStream);


		outStream << 
			"float4 main_fp(\n"
			"float4 vertexPos : POSITION,\n"
			"float4 position : TEXCOORD0,\n";

		uint texCoordSet = 1;
		outStream <<
			"float4 uvMisc : TEXCOORD" << texCoordSet++ << ",\n";

		// UV's premultiplied, packed as xy/zw
		uint maxLayers = prof->getMaxLayers(terrain);
		uint numBlendTextures = std::min(terrain->getBlendTextureCount(maxLayers), terrain->getBlendTextureCount());
		uint numLayers = std::min(maxLayers, static_cast<uint>(terrain->getLayerCount()));
		uint numUVSets = numLayers / 2;
		if (numLayers % 2)
			++numUVSets;
		if (tt != LOW_LOD)
		{
			for (uint i = 0; i < numUVSets; ++i)
			{
				outStream <<
					"float4 layerUV" << i << " : TEXCOORD" << texCoordSet++ << ", \n";
			}

		}
		if (prof->getParent()->getDebugLevel() && tt != RENDER_COMPOSITE_MAP)
		{
			outStream << "float2 lodInfo : TEXCOORD" << texCoordSet++ << ", \n";
		}

		bool fog = terrain->getSceneManager()->getFogMode() != FOG_NONE && tt != RENDER_COMPOSITE_MAP;
		if (fog)
		{
			outStream <<
				"uniform float3 fogColour, \n"
				"float fogVal : COLOR,\n";
		}

		uint currentSamplerIdx = 0;

		outStream <<
			// Only 1 light supported in this version
			// deferred shading profile / generator later, ok? :)
			"uniform float3 ambient,\n"
			"uniform float4 lightPosObjSpace,\n"
			"uniform float3 lightDiffuseColour,\n"
			"uniform float3 lightSpecularColour,\n"
			"uniform float3 eyePosObjSpace,\n"
			// pack scale, bias and specular
			"uniform float4 scaleBiasSpecular,\n";

		if (tt == LOW_LOD)
		{
			// single composite map covers all the others below
			outStream << 
				"uniform sampler2D compositeMap : register(s" << currentSamplerIdx++ << ")\n";
		}
		else
		{
			outStream << 
				"uniform sampler2D globalNormal : register(s" << currentSamplerIdx++ << ")\n";


			if (terrain->getGlobalColourMapEnabled() && prof->isGlobalColourMapEnabled())
			{
				outStream << ", uniform sampler2D globalColourMap : register(s" 
					<< currentSamplerIdx++ << ")\n";
			}
			if (prof->isLightmapEnabled())
			{
				outStream << ", uniform sampler2D lightMap : register(s" 
					<< currentSamplerIdx++ << ")\n";
			}
			// Blend textures - sampler definitions
			for (uint i = 0; i < numBlendTextures; ++i)
			{
				outStream << ", uniform sampler2D blendTex" << i 
					<< " : register(s" << currentSamplerIdx++ << ")\n";
			}

			// Layer textures - sampler definitions & UV multipliers
			for (uint i = 0; i < numLayers; ++i)
			{
				outStream << ", uniform sampler2D difftex" << i 
					<< " : register(s" << currentSamplerIdx++ << ")\n";
				outStream << ", uniform sampler2D normtex" << i 
					<< " : register(s" << currentSamplerIdx++ << ")\n";
			}
		}

		if (prof->isShadowingEnabled(tt, terrain))
		{
			generateFpDynamicShadowsParams(&texCoordSet, &currentSamplerIdx, prof, terrain, tt, outStream);
		}

		// check we haven't exceeded samplers
		if (currentSamplerIdx > 16)
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
				"Requested options require too many texture samplers! Try reducing the number of layers.",
				__FUNCTION__);
		}

		outStream << 
			") : COLOR\n"
			"{\n"
			"	float4 outputCol;\n"
			"	float shadow = 1.0;\n"
			"	float2 uv = uvMisc.xy;\n"
			// base colour
			"	outputCol = float4(0,0,0,1);\n";

		if (tt != LOW_LOD)
		{
			outStream << 
				// global normal
				"	float3 normal = expand(tex2D(globalNormal, uv)).rgb;\n";

		}

		outStream <<
			"	float3 lightDir = \n"
			"		lightPosObjSpace.xyz -  (position.xyz * lightPosObjSpace.w);\n"
			"	float3 eyeDir = eyePosObjSpace - position.xyz;\n"

			// set up accumulation areas
			"	float3 diffuse = float3(0,0,0);\n"
			"	float specular = 0;\n";


		if (tt == LOW_LOD)
		{
			// we just do a single calculation from composite map
			outStream <<
				"	float4 composite = tex2D(compositeMap, uv);\n"
				"	diffuse = composite.rgb;\n";
			// TODO - specular; we'll need normals for this!
		}
		else
		{
			// set up the blend values
			for (uint i = 0; i < numBlendTextures; ++i)
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
				if (prof->isLayerParallaxMappingEnabled())
					outStream << "	float displacement;\n";
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
				outStream << "	float4 litRes = lit(dot(lightDir, normal), dot(halfAngle, normal), scaleBiasSpecular.z);\n";

			}
		}


	}
	//---------------------------------------------------------------------
	void TerrainMaterialGeneratorA::SM2Profile::ShaderHelperCg::generateVpLayer(
		const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, uint layer, StringUtil::StrStreamType& outStream)
	{
		// nothing to do
	}
	//---------------------------------------------------------------------
	void TerrainMaterialGeneratorA::SM2Profile::ShaderHelperCg::generateFpLayer(
		const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, uint layer, StringUtil::StrStreamType& outStream)
	{
		uint uvIdx = layer / 2;
		String uvChannels = (layer % 2) ? ".zw" : ".xy";
		uint blendIdx = (layer-1) / 4;
		String blendChannel = getChannel(layer-1);
		String blendWeightStr = String("blendTexVal") + StringConverter::toString(blendIdx) + 
			"." + blendChannel;

		// generate early-out conditional
		/* Disable - causing some issues even when trying to force the use of texldd
		if (layer && prof->_isSM3Available())
			outStream << "  if (" << blendWeightStr << " > 0.0003)\n  { \n";
		*/

		// generate UV
		outStream << "	float2 uv" << layer << " = layerUV" << uvIdx << uvChannels << ";\n";

		// calculate lighting here if normal mapping
		if (prof->isLayerNormalMappingEnabled())
		{
			if (prof->isLayerParallaxMappingEnabled() && tt != RENDER_COMPOSITE_MAP)
			{
				// modify UV - note we have to sample an extra time
				outStream << "	displacement = tex2D(normtex" << layer << ", uv" << layer << ").a\n"
					"		* scaleBiasSpecular.x + scaleBiasSpecular.y;\n";
				outStream << "	uv" << layer << " += TSeyeDir.xy * displacement;\n";
			}

			// access TS normal map
			outStream << "	TSnormal = expand(tex2D(normtex" << layer << ", uv" << layer << ")).rgb;\n";
			outStream << "	TShalfAngle = normalize(TSlightDir + TSeyeDir);\n";
			outStream << "	litResLayer = lit(dot(TSlightDir, TSnormal), dot(TShalfAngle, TSnormal), scaleBiasSpecular.z);\n";
			if (!layer)
				outStream << "	litRes = litResLayer;\n";
			else
				outStream << "	litRes = lerp(litRes, litResLayer, " << blendWeightStr << ");\n";

		}

		// sample diffuse texture
		outStream << "	float4 diffuseSpecTex" << layer 
			<< " = tex2D(difftex" << layer << ", uv" << layer << ");\n";

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
				<< ".rgb, " << blendWeightStr << ");\n";
			if (prof->isLayerSpecularMappingEnabled())
				outStream << "	specular = lerp(specular, diffuseSpecTex" << layer 
					<< ".a, " << blendWeightStr << ");\n";

		}

		// End early-out
		/* Disable - causing some issues even when trying to force the use of texldd
		if (layer && prof->_isSM3Available())
			outStream << "  } // early-out blend value\n";
		*/
	}
	//---------------------------------------------------------------------
	void TerrainMaterialGeneratorA::SM2Profile::ShaderHelperCg::generateVpFooter(
		const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringUtil::StrStreamType& outStream)
	{

		outStream << 
			"	oPos = mul(viewProjMatrix, worldPos);\n"
			"	oUVMisc.xy = uv.xy;\n";

		bool fog = terrain->getSceneManager()->getFogMode() != FOG_NONE && tt != RENDER_COMPOSITE_MAP;
		if (fog)
		{
			if (terrain->getSceneManager()->getFogMode() == FOG_LINEAR)
			{
				outStream <<
					"	fogVal = saturate((oPos.z - fogParams.y) * fogParams.w);\n";
			}
			else
			{
				outStream <<
					"	fogVal = 1 - saturate(1 / (exp(oPos.z * fogParams.x)));\n";
			}
		}
		
		if (prof->isShadowingEnabled(tt, terrain))
			generateVpDynamicShadows(prof, terrain, tt, outStream);

		outStream << 
			"}\n";


	}
	//---------------------------------------------------------------------
	void TerrainMaterialGeneratorA::SM2Profile::ShaderHelperCg::generateFpFooter(
		const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringUtil::StrStreamType& outStream)
	{

		if (tt == LOW_LOD)
		{
			if (prof->isShadowingEnabled(tt, terrain))
			{
				generateFpDynamicShadows(prof, terrain, tt, outStream);
				outStream << 
					"	outputCol.rgb = diffuse * rtshadow;\n";
			}
			else
			{
				outStream << 
					"	outputCol.rgb = diffuse;\n";
			}
		}
		else
		{
			if (terrain->getGlobalColourMapEnabled() && prof->isGlobalColourMapEnabled())
			{
				// sample colour map and apply to diffuse
				outStream << "	diffuse *= tex2D(globalColourMap, uv).rgb;\n";
			}
			if (prof->isLightmapEnabled())
			{
				// sample lightmap
				outStream << "	shadow = tex2D(lightMap, uv).r;\n";
			}

			if (prof->isShadowingEnabled(tt, terrain))
			{
				generateFpDynamicShadows(prof, terrain, tt, outStream);
			}

			// diffuse lighting
			outStream << "	outputCol.rgb += ambient.rgb * diffuse + litRes.y * lightDiffuseColour * diffuse * shadow;\n";

			// specular default
			if (!prof->isLayerSpecularMappingEnabled())
				outStream << "	specular = 1.0;\n";

			if (tt == RENDER_COMPOSITE_MAP)
			{
				// Lighting embedded in alpha
				outStream <<
					"	outputCol.a = shadow;\n";

			}
			else
			{
				// Apply specular
				outStream << "	outputCol.rgb += litRes.z * lightSpecularColour * specular * shadow;\n";

				if (prof->getParent()->getDebugLevel())
				{
					outStream << "	outputCol.rg += lodInfo.xy;\n";
				}
			}
		}

		bool fog = terrain->getSceneManager()->getFogMode() != FOG_NONE && tt != RENDER_COMPOSITE_MAP;
		if (fog)
		{
			outStream << "	outputCol.rgb = lerp(outputCol.rgb, fogColour, fogVal);\n";
		}

		// Final return
		outStream << "	return outputCol;\n"
			<< "}\n";

	}
	//---------------------------------------------------------------------
	void TerrainMaterialGeneratorA::SM2Profile::ShaderHelperCg::generateFpDynamicShadowsHelpers(
		const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringUtil::StrStreamType& outStream)
	{
		// TODO make filtering configurable
		outStream <<
			"// Simple PCF \n"
			"// Number of samples in one dimension (square for total samples) \n"
			"#define NUM_SHADOW_SAMPLES_1D 2.0 \n"
			"#define SHADOW_FILTER_SCALE 1 \n"

			"#define SHADOW_SAMPLES NUM_SHADOW_SAMPLES_1D*NUM_SHADOW_SAMPLES_1D \n"

			"float4 offsetSample(float4 uv, float2 offset, float invMapSize) \n"
			"{ \n"
			"	return float4(uv.xy + offset * invMapSize * uv.w, uv.z, uv.w); \n"
			"} \n";

		if (prof->getReceiveDynamicShadowsDepth())
		{
			outStream << 
				"float calcDepthShadow(sampler2D shadowMap, float4 uv, float invShadowMapSize) \n"
				"{ \n"
				"	// 4-sample PCF \n"
					
				"	float shadow = 0.0; \n"
				"	float offset = (NUM_SHADOW_SAMPLES_1D/2 - 0.5) * SHADOW_FILTER_SCALE; \n"
				"	for (float y = -offset; y <= offset; y += SHADOW_FILTER_SCALE) \n"
				"		for (float x = -offset; x <= offset; x += SHADOW_FILTER_SCALE) \n"
				"		{ \n"
				"			float4 newUV = offsetSample(uv, float2(x, y), invShadowMapSize);\n"
				"			// manually project and assign derivatives \n"
				"			// to avoid gradient issues inside loops \n"
				"			newUV = newUV / newUV.w; \n"
				"			float depth = tex2D(shadowMap, newUV.xy, 1, 1).x; \n"
				"			if (depth >= 1 || depth >= uv.z)\n"
				"				shadow += 1.0;\n"
				"		} \n"

				"	shadow /= SHADOW_SAMPLES; \n"

				"	return shadow; \n"
				"} \n";
		}
		else
		{
			outStream <<
				"float calcSimpleShadow(sampler2D shadowMap, float4 shadowMapPos) \n"
				"{ \n"
				"	return tex2Dproj(shadowMap, shadowMapPos).x; \n"
				"} \n";

		}

		if (prof->getReceiveDynamicShadowsPSSM())
		{
			uint numTextures = prof->getReceiveDynamicShadowsPSSM()->getSplitCount();


			if (prof->getReceiveDynamicShadowsDepth())
			{
				outStream <<
					"float calcPSSMDepthShadow(";
			}
			else
			{
				outStream <<
					"float calcPSSMSimpleShadow(";
			}

			outStream << "\n	";
			for (uint i = 0; i < numTextures; ++i)
				outStream << "sampler2D shadowMap" << i << ", ";
			outStream << "\n	";
			for (uint i = 0; i < numTextures; ++i)
				outStream << "float4 lsPos" << i << ", ";
			if (prof->getReceiveDynamicShadowsDepth())
			{
				outStream << "\n	";
				for (uint i = 0; i < numTextures; ++i)
					outStream << "float invShadowmapSize" << i << ", ";
			}
			outStream << "\n"
				"	float4 pssmSplitPoints, float camDepth) \n"
				"{ \n"
				"	float shadow; \n"
				"	// calculate shadow \n";
			
			for (uint i = 0; i < numTextures; ++i)
			{
				if (!i)
					outStream << "	if (camDepth <= pssmSplitPoints." << ShaderHelper::getChannel(i) << ") \n";
				else if (i < numTextures - 1)
					outStream << "	else if (camDepth <= pssmSplitPoints." << ShaderHelper::getChannel(i) << ") \n";
				else
					outStream << "	else \n";

				outStream <<
					"	{ \n";
				if (prof->getReceiveDynamicShadowsDepth())
				{
					outStream <<
						"		shadow = calcDepthShadow(shadowMap" << i << ", lsPos" << i << ", invShadowmapSize" << i << "); \n";
				}
				else
				{
					outStream <<
						"		shadow = calcSimpleShadow(shadowMap" << i << ", lsPos" << i << "); \n";
				}
				outStream <<
					"	} \n";

			}

			outStream <<
				"	return shadow; \n"
				"} \n\n\n";
		}


	}
	//---------------------------------------------------------------------
	uint TerrainMaterialGeneratorA::SM2Profile::ShaderHelperCg::generateVpDynamicShadowsParams(
		uint texCoord, const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringUtil::StrStreamType& outStream)
	{
		// out semantics & params
		uint numTextures = 1;
		if (prof->getReceiveDynamicShadowsPSSM())
		{
			numTextures = prof->getReceiveDynamicShadowsPSSM()->getSplitCount();
		}
		for (uint i = 0; i < numTextures; ++i)
		{
			outStream <<
				", out float4 oLightSpacePos" << i << " : TEXCOORD" << texCoord++ << " \n" <<
				", uniform float4x4 texViewProjMatrix" << i << " \n";
			if (prof->getReceiveDynamicShadowsDepth())
			{
				outStream <<
					", uniform float4 depthRange" << i << " // x = min, y = max, z = range, w = 1/range \n";
			}
		}

		return texCoord;

	}
	//---------------------------------------------------------------------
	void TerrainMaterialGeneratorA::SM2Profile::ShaderHelperCg::generateVpDynamicShadows(
		const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringUtil::StrStreamType& outStream)
	{
		uint numTextures = 1;
		if (prof->getReceiveDynamicShadowsPSSM())
		{
			numTextures = prof->getReceiveDynamicShadowsPSSM()->getSplitCount();
		}

		// Calculate the position of vertex in light space
		for (uint i = 0; i < numTextures; ++i)
		{
			outStream <<
				"	oLightSpacePos" << i << " = mul(texViewProjMatrix" << i << ", worldPos); \n";
			if (prof->getReceiveDynamicShadowsDepth())
			{
				// make linear
				outStream <<
					"oLightSpacePos" << i << ".z = (oLightSpacePos" << i << ".z - depthRange" << i << ".x) * depthRange" << i << ".w;\n";

			}
		}


		if (prof->getReceiveDynamicShadowsPSSM())
		{
			outStream <<
				"	// pass cam depth\n"
				"	oUVMisc.z = oPos.z;\n";
		}

	}
	//---------------------------------------------------------------------
	void TerrainMaterialGeneratorA::SM2Profile::ShaderHelperCg::generateFpDynamicShadowsParams(
		uint* texCoord, uint* sampler, const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringUtil::StrStreamType& outStream)
	{
		if (tt == HIGH_LOD)
			mShadowSamplerStartHi = *sampler;
		else if (tt == LOW_LOD)
			mShadowSamplerStartLo = *sampler;

		// in semantics & params
		uint numTextures = 1;
		if (prof->getReceiveDynamicShadowsPSSM())
		{
			numTextures = prof->getReceiveDynamicShadowsPSSM()->getSplitCount();
			outStream <<
				", uniform float4 pssmSplitPoints \n";
		}
		for (uint i = 0; i < numTextures; ++i)
		{
			outStream <<
				", float4 lightSpacePos" << i << " : TEXCOORD" << *texCoord << " \n" <<
				", uniform sampler2D shadowMap" << i << " : register(s" << *sampler << ") \n";
			*sampler = *sampler + 1;
			*texCoord = *texCoord + 1;
			if (prof->getReceiveDynamicShadowsDepth())
			{
				outStream <<
					", uniform float inverseShadowmapSize" << i << " \n";
			}
		}

	}
	//---------------------------------------------------------------------
	void TerrainMaterialGeneratorA::SM2Profile::ShaderHelperCg::generateFpDynamicShadows(
		const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringUtil::StrStreamType& outStream)
	{
		if (prof->getReceiveDynamicShadowsPSSM())
		{
			uint numTextures = prof->getReceiveDynamicShadowsPSSM()->getSplitCount();
			outStream << 
				"	float camDepth = uvMisc.z;\n";

			if (prof->getReceiveDynamicShadowsDepth())
			{
				outStream << 
					"	float rtshadow = calcPSSMDepthShadow(";
			}
			else
			{
				outStream << 
					"	float rtshadow = calcPSSMSimpleShadow(";
			}
			for (uint i = 0; i < numTextures; ++i)
				outStream << "shadowMap" << i << ", ";
			outStream << "\n		";

			for (uint i = 0; i < numTextures; ++i)
				outStream << "lightSpacePos" << i << ", ";
			if (prof->getReceiveDynamicShadowsDepth())
			{
				outStream << "\n		";
				for (uint i = 0; i < numTextures; ++i)
					outStream << "inverseShadowmapSize" << i << ", ";
			}
			outStream << "\n" <<
				"		pssmSplitPoints, camDepth);\n";

		}
		else
		{
			if (prof->getReceiveDynamicShadowsDepth())
			{
				outStream <<
					"	float rtshadow = calcDepthShadow(shadowMap0, lightSpacePos0, inverseShadowmapSize0);";
			}
			else
			{
				outStream <<
					"	float rtshadow = calcSimpleShadow(shadowMap0, lightSpacePos0);";
			}
		}

		outStream << 
			"	shadow = min(shadow, rtshadow);\n";
		
	}
}
