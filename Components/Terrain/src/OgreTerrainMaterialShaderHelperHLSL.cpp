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
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreHighLevelGpuProgram.h"
#include "OgreTerrain.h"
#include "OgreShadowCameraSetupPSSM.h"

namespace Ogre
{
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    HighLevelGpuProgramPtr
    TerrainMaterialGeneratorA::SM2Profile::ShaderHelperHLSL::createVertexProgram(
        const SM2Profile* prof, const Terrain* terrain, TechniqueType tt)
    {
        HighLevelGpuProgramManager& mgr = HighLevelGpuProgramManager::getSingleton();
        String progName = getVertexProgramName(prof, terrain, tt);

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
        // creating Shader Model 4.0 type hlsl shader fol DirectX11 Render System
        if (prof->_isSM4Available())
            ret->setParameter("target", "vs_4_0");
        //else if (prof->_isSM3Available())
        //  ret->setParameter("target", "vs_3_0");
        //else
        //  ret->setParameter("target", "vs_2_0");
        ret->setParameter("entry_point", "main_vp");

        return ret;

    }
    //---------------------------------------------------------------------
    HighLevelGpuProgramPtr
    TerrainMaterialGeneratorA::SM2Profile::ShaderHelperHLSL::createFragmentProgram(
        const SM2Profile* prof, const Terrain* terrain, TechniqueType tt)
    {
        HighLevelGpuProgramManager& mgr = HighLevelGpuProgramManager::getSingleton();
        String progName = getFragmentProgramName(prof, terrain, tt);

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
        // creating Shader Model 4.0 type hlsl shader fol DirectX11 Render System
        if (prof->_isSM4Available())
            ret->setParameter("target", "ps_4_0");
        //else if (prof->_isSM3Available())
        //  ret->setParameter("target", "ps_3_0");
        //else
        //  ret->setParameter("target", "ps_2_x");
        ret->setParameter("entry_point", "main_fp");

        return ret;

    }
    //---------------------------------------------------------------------
    void TerrainMaterialGeneratorA::SM2Profile::ShaderHelperHLSL::generateVpHeader(
        const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringStream& outStream)
    {
        // generate some structures needed for SM4 before the header

        // vertex shader output, pixel shader input struct.
        outStream << 
            "struct v2p\n"
            "{\n";

        outStream <<
            "float4 oPos : SV_POSITION;\n"
            "float4 oPosObj : TEXCOORD0; \n";

        uint texCoordSet = 1;
        outStream <<
            "float4 oUVMisc : TEXCOORD" << texCoordSet++ <<"; // xy = uv, z = camDepth\n";

        // uv multipliers
        uint maxLayers = prof->getMaxLayers(terrain);
        uint numLayers = std::min(maxLayers, static_cast<uint>(terrain->getLayerCount()));
        uint numUVMultipliers = (numLayers / 4);
        if (numLayers % 4)
            ++numUVMultipliers;

        // layer UV's premultiplied, packed as xy/zw
        uint numUVSets = numLayers / 2;
        if (numLayers % 2)
            ++numUVSets;
        if (tt != LOW_LOD)
        {
            for (uint i = 0; i < numUVSets; ++i)
            {
                outStream <<
                    "float4 oUV" << i << " : TEXCOORD" << texCoordSet++ << ";\n";
            }
        }

        if (prof->getParent()->getDebugLevel() && tt != RENDER_COMPOSITE_MAP)
        {
            outStream << "float2 lodInfo : TEXCOORD" << texCoordSet++ << ";\n";
        }

        bool fog = terrain->getSceneManager()->getFogMode() != FOG_NONE && tt != RENDER_COMPOSITE_MAP;
        if (fog)
        {
            outStream << "float fogVal : COLOR;\n";
        }
        
        uint numTextures = 1;
        if (prof->getReceiveDynamicShadowsPSSM())
        {
            numTextures = prof->getReceiveDynamicShadowsPSSM()->getSplitCount();
        }
        for (uint i = 0; i < numTextures; ++i)
        {
            outStream <<
                "float4 oLightSpacePos" << i << " : TEXCOORD" << texCoordSet++ << " ;\n";
        }

        outStream << "};\n";
        //output/input structure finished

        outStream << 
            "v2p main_vp(\n";
        bool compression = terrain->_getUseVertexCompression() && tt != RENDER_COMPOSITE_MAP;
        if (compression)
        {
            outStream << 
                "float2 posIndex : POSITION,\n"
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
            "uniform matrix worldMatrix,\n"
            "uniform matrix viewProjMatrix,\n"
            "uniform float2   lodMorph,\n"; // morph amount, morph LOD target

        if (compression)
        {
            outStream << 
                "uniform matrix   posIndexToObjectSpace,\n"
                "uniform float    baseUVScale,\n";
        }

        for (uint i = 0; i < numUVMultipliers - 1; ++i)
            outStream << "uniform float4 uvMul_" << i << ", \n";
        outStream << "uniform float4 uvMul_" << numUVMultipliers - 1 << " \n";

        if (fog)
        {
            outStream <<
                ", uniform float4 fogParams\n";
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
                "   float4 pos;\n"
                "   pos = mul(posIndexToObjectSpace, float4(posIndex, height, 1));\n"
                "   float2 uv = float2(posIndex.x * baseUVScale, 1.0 - (posIndex.y * baseUVScale));\n";
        }
        outStream <<
            "   float4 worldPos = mul(worldMatrix, pos);\n"
            "   v2p output;\n"
            "   output.oPosObj = pos;\n";

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
                "   float toMorph = -min(0, sign(delta.y - lodMorph.y));\n";
            // this will either be 1 (morph) or 0 (don't morph)
            if (prof->getParent()->getDebugLevel())
            {
                // x == LOD level (-1 since value is target level, we want to display actual)
                outStream << "output.lodInfo.x = (lodMorph.y - 1) / " << terrain->getNumLodLevels() << ";\n";
                // y == LOD morph
                outStream << "output.lodInfo.y = toMorph * lodMorph.x;\n";
            }

            // morph
            switch (terrain->getAlignment())
            {
            case Terrain::ALIGN_X_Y:
                outStream << "  worldPos.z += delta.x * toMorph * lodMorph.x;\n";
                break;
            case Terrain::ALIGN_X_Z:
                outStream << "  worldPos.y += delta.x * toMorph * lodMorph.x;\n";
                break;
            case Terrain::ALIGN_Y_Z:
                outStream << "  worldPos.x += delta.x * toMorph * lodMorph.x;\n";
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
                    "   output.oUV" << i << ".xy = " << " uv.xy * uvMul_" << uvMulIdx << "." << getChannel(layer) << ";\n";
                outStream <<
                    "   output.oUV" << i << ".zw = " << " uv.xy * uvMul_" << uvMulIdx << "." << getChannel(layer+1) << ";\n";
                
            }

        }
    }
    //---------------------------------------------------------------------
    void TerrainMaterialGeneratorA::SM2Profile::ShaderHelperHLSL::generateVpLayer(
        const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, uint layer, StringStream& outStream)
    {
        // nothing to do
    }
    //---------------------------------------------------------------------
    void TerrainMaterialGeneratorA::SM2Profile::ShaderHelperHLSL::generateVpFooter(
        const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringStream& outStream)
    {

        outStream << 
            "   output.oPos = mul(viewProjMatrix, worldPos);\n"
            "   output.oUVMisc.xy = uv.xy;\n";

        bool fog = terrain->getSceneManager()->getFogMode() != FOG_NONE && tt != RENDER_COMPOSITE_MAP;
        if (fog)
        {
            if (terrain->getSceneManager()->getFogMode() == FOG_LINEAR)
            {
                outStream <<
                    "   output.fogVal = saturate((output.oPos.z - fogParams.y) * fogParams.w);\n";
            }
            else
            {
                outStream <<
                    "   output.fogVal = 1 - saturate(1 / (exp(output.oPos.z * fogParams.x)));\n";
            }
        }
        
        if (prof->isShadowingEnabled(tt, terrain))
            generateVpDynamicShadows(prof, terrain, tt, outStream);

        outStream << 
            "return output;"
            "}\n";

    }
    //---------------------------------------------------------------------
    uint TerrainMaterialGeneratorA::SM2Profile::ShaderHelperHLSL::generateVpDynamicShadowsParams(
        uint texCoord, const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringStream& outStream)
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
                ", uniform matrix texViewProjMatrix" << i << " \n";
            if (prof->getReceiveDynamicShadowsDepth())
            {
                outStream << 
                    ", uniform float4 depthRange" << i << " // x = min, y = max, z = range, w = 1/range \n";
            }
        }

        return texCoord;

    }
    //---------------------------------------------------------------------
    void TerrainMaterialGeneratorA::SM2Profile::ShaderHelperHLSL::generateVpDynamicShadows(
        const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringStream& outStream)
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
                "   output.oLightSpacePos" << i << " = mul(texViewProjMatrix" << i << ", worldPos); \n";
            if (prof->getReceiveDynamicShadowsDepth())
            {
                // make linear
                outStream <<
                    "output.oLightSpacePos" << i << ".z = (output.oLightSpacePos" << i << ".z - depthRange" << i << ".x) * depthRange" << i << ".w;\n";

            }
        }


        if (prof->getReceiveDynamicShadowsPSSM())
        {
            outStream <<
                "   // pass cam depth\n"
                "   output.oUVMisc.z = output.oPos.z;\n";
        }

    }
    //---------------------------------------------------------------------
    void TerrainMaterialGeneratorA::SM2Profile::ShaderHelperHLSL::generateFpHeader(
        const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringStream& outStream)
    {
        // adding samplerstate for shadowmapping
        outStream <<
            "SamplerState g_SamLinear\n"
            "{\n"
            "   Filter = MIN_MAG_MIP_LINEAR;\n"
            "   AddressU = Wrap;\n"
            "   AddressV = Wrap;\n"
            "};\n"
            "SamplerState g_SamClamp\n"
            "{\n"
            "   Filter = MIN_MAG_MIP_LINEAR;\n"
            "   AddressU = Clamp;\n"
            "   AddressV = Clamp;\n"
            "};\n"
            ;

        // Main header
        outStream << 
            // helpers
            "float4 expand(float4 v)\n"
            "{ \n"
            "   return v * 2 - 1;\n"
            "}\n\n\n";

        if (prof->isShadowingEnabled(tt, terrain))
            generateFpDynamicShadowsHelpers(prof, terrain, tt, outStream);

        // vertex shader output, pixel shader input struct.
        outStream << 
            "struct v2p\n"
            "{\n";

        outStream <<
            "float4 oPos : SV_POSITION;\n"
            "float4 oPosObj : TEXCOORD0; \n";

        uint texCoordSet = 1;
        outStream <<
            "float4 oUVMisc : TEXCOORD" << texCoordSet++ <<"; // xy = uv, z = camDepth\n";

        // uv multipliers
        uint maxLayers = prof->getMaxLayers(terrain);
        uint numLayers = std::min(maxLayers, static_cast<uint>(terrain->getLayerCount()));
        uint numBlendTextures = std::min(terrain->getBlendTextureCount(maxLayers), terrain->getBlendTextureCount());
        uint numUVMultipliers = (numLayers / 4);
        if (numLayers % 4)
            ++numUVMultipliers;

        // layer UV's premultiplied, packed as xy/zw
        uint numUVSets = numLayers / 2;
        if (numLayers % 2)
            ++numUVSets;
        if (tt != LOW_LOD)
        {
            for (uint i = 0; i < numUVSets; ++i)
            {
                outStream <<
                    "float4 oUV" << i << " : TEXCOORD" << texCoordSet++ << ";\n";
            }
        }

        if (prof->getParent()->getDebugLevel() && tt != RENDER_COMPOSITE_MAP)
        {
            outStream << "float2 lodInfo : TEXCOORD" << texCoordSet++ << ";\n";
        }

        bool fog = terrain->getSceneManager()->getFogMode() != FOG_NONE && tt != RENDER_COMPOSITE_MAP;
        if (fog)
        {
            outStream << "float fogVal : COLOR;\n";
        }
        
        uint numTextures = 1;
        if (prof->getReceiveDynamicShadowsPSSM())
        {
            numTextures = prof->getReceiveDynamicShadowsPSSM()->getSplitCount();
        }
        for (uint i = 0; i < numTextures; ++i)
        {
            outStream <<
                "float4 oLightSpacePos" << i << " : TEXCOORD" << texCoordSet++ << " ;\n";
        }

        outStream << "};\n";
        //output/input structure finished

        uint currentSamplerIdx = 0;
        if (tt == LOW_LOD)
        {
            // single composite map covers all the others below
            outStream << 
                "Texture2D compositeMap : register(s" << currentSamplerIdx++ << ");\n";
        }
        else
        {
            outStream << 
                "Texture2D globalNormal : register(s" << currentSamplerIdx++ << ");\n";


            if (terrain->getGlobalColourMapEnabled() && prof->isGlobalColourMapEnabled())
            {
                outStream << "Texture2D globalColourMap : register(s" 
                    << currentSamplerIdx++ << ");\n";
            }
            if (prof->isLightmapEnabled())
            {
                outStream << "Texture2D lightMap : register(s" 
                    << currentSamplerIdx++ << ");\n";
            }
            // Blend textures - sampler definitions
            for (uint i = 0; i < numBlendTextures; ++i)
            {
                outStream << "Texture2D blendTex" << i 
                    << " : register(s" << currentSamplerIdx++ << ");\n";
            }

            // Layer textures - sampler definitions & UV multipliers
            for (uint i = 0; i < numLayers; ++i)
            {
                outStream << "Texture2D difftex" << i 
                    << " : register(s" << currentSamplerIdx++ << ");\n";
                outStream << "Texture2D normtex" << i 
                    << " : register(s" << currentSamplerIdx++ << ");\n";
            }
        }

        outStream << 
            "float4 main_fp(\n"
            "v2p input,\n";

        if (fog)
        {
            outStream <<
                "uniform float3 fogColour, \n";
        }

        outStream <<
            // Only 1 light supported in this version
            // deferred shading profile / generator later, ok? :)
            "uniform float3 ambient,\n"
            "uniform float4 lightPosObjSpace,\n"
            "uniform float3 lightDiffuseColour,\n"
            "uniform float3 lightSpecularColour,\n"
            "uniform float3 eyePosObjSpace,\n"
            // pack scale, bias and specular
            "uniform float4 scaleBiasSpecular\n";

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
            ") : SV_Target\n"
            "{\n"
            "   float4 outputCol;\n"
            "   float shadow = 1.0;\n"
            "   float2 uv = input.oUVMisc.xy;\n"
            // base colour
            "   outputCol = float4(0.0,0.0,0.0,1.0);\n";

        if (tt != LOW_LOD)
        {
            outStream << 
                // global normal
                "   float3 normal = expand(globalNormal.Sample(g_SamClamp, uv)).rgb;\n";

        }

        outStream <<
            "   float3 lightDir = \n"
            "       lightPosObjSpace.xyz -  (input.oPosObj.xyz * lightPosObjSpace.w);\n"
            "   float3 eyeDir = eyePosObjSpace - input.oPosObj.xyz;\n"

            // set up accumulation areas
            "   float3 diffuse = float3(0,0,0);\n"
            "   float specular = 0;\n";


        if (tt == LOW_LOD)
        {
            // we just do a single calculation from composite map
            outStream <<
                "   float4 composite = compositeMap.Sample(g_SamClamp, uv);\n"
                "   diffuse = composite.rgb;\n";
            // TODO - specular; we'll need normals for this!
        }
        else
        {
            // set up the blend values
            for (uint i = 0; i < numBlendTextures; ++i)
            {
                outStream << "  float4 blendTexVal" << i << " = blendTex" << i << ".Sample(g_SamClamp, uv);\n";
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
                    outStream << "  float3 tangent = float3(1, 0, 0);\n";
                    break;
                case Terrain::ALIGN_Y_Z:
                    outStream << "  float3 tangent = float3(0, 0, -1);\n";
                    break;
                };

                outStream << "  float3 binormal = normalize(cross(tangent, normal));\n";
                // note, now we need to re-cross to derive tangent again because it wasn't orthonormal
                outStream << "  tangent = normalize(cross(normal, binormal));\n";
                // derive final matrix
                outStream << "  float3x3 TBN = float3x3(tangent, binormal, normal);\n";

                // set up lighting result placeholders for interpolation
                outStream <<  " float4 litRes, litResLayer;\n";
                outStream << "  float3 TSlightDir, TSeyeDir, TShalfAngle, TSnormal;\n";
                if (prof->isLayerParallaxMappingEnabled())
                    outStream << "  float displacement;\n";
                // move 
                outStream << "  TSlightDir = normalize(mul(TBN, lightDir));\n";
                outStream << "  TSeyeDir = normalize(mul(TBN, eyeDir));\n";

            }
            else
            {
                // simple per-pixel lighting with no normal mapping
                outStream << "  lightDir = normalize(lightDir);\n";
                outStream << "  eyeDir = normalize(eyeDir);\n";
                outStream << "  float3 halfAngle = normalize(lightDir + eyeDir);\n";
                outStream << "  float4 litRes = lit(dot(lightDir, normal), dot(halfAngle, normal), scaleBiasSpecular.z);\n";

            }
        }


    }
    //---------------------------------------------------------------------
    void TerrainMaterialGeneratorA::SM2Profile::ShaderHelperHLSL::generateFpLayer(
        const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, uint layer, StringStream& outStream)
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
        outStream << "  float2 uv" << layer << " = input.oUV" << uvIdx << uvChannels << ";\n";

        // calculate lighting here if normal mapping
        if (prof->isLayerNormalMappingEnabled())
        {
            if (prof->isLayerParallaxMappingEnabled() && tt != RENDER_COMPOSITE_MAP)
            {
                // modify UV - note we have to sample an extra time
                outStream << "  displacement = normtex" << layer << ".Sample(g_SamLinear, uv" << layer << ").a\n"
                    "       * scaleBiasSpecular.x + scaleBiasSpecular.y;\n";
                outStream << "  uv" << layer << " += TSeyeDir.xy * displacement;\n";
            }

            // access TS normal map
            outStream << "  TSnormal = expand(normtex" << layer << ".Sample(g_SamLinear, uv" << layer << ")).rgb;\n";
            outStream << "  TShalfAngle = normalize(TSlightDir + TSeyeDir);\n";
            outStream << "  litResLayer = lit(dot(TSlightDir, TSnormal), dot(TShalfAngle, TSnormal), scaleBiasSpecular.z);\n";
            if (!layer)
                outStream << "  litRes = litResLayer;\n";
            else
                outStream << "  litRes = lerp(litRes, litResLayer, " << blendWeightStr << ");\n";

        }

        // sample diffuse texture
        outStream << "  float4 diffuseSpecTex" << layer 
            << " = difftex" << layer << ".Sample(g_SamLinear, uv" << layer << ");\n";

        // apply to common
        if (!layer)
        {
            outStream << "  diffuse = diffuseSpecTex0.rgb;\n";
            if (prof->isLayerSpecularMappingEnabled())
                outStream << "  specular = diffuseSpecTex0.a;\n";
        }
        else
        {
            outStream << "  diffuse = lerp(diffuse, diffuseSpecTex" << layer 
                << ".rgb, " << blendWeightStr << ");\n";
            if (prof->isLayerSpecularMappingEnabled())
                outStream << "  specular = lerp(specular, diffuseSpecTex" << layer 
                    << ".a, " << blendWeightStr << ");\n";

        }

        // End early-out
        /* Disable - causing some issues even when trying to force the use of texldd
        if (layer && prof->_isSM3Available())
            outStream << "  } // early-out blend value\n";
        */
    }
    //---------------------------------------------------------------------
    void TerrainMaterialGeneratorA::SM2Profile::ShaderHelperHLSL::generateFpFooter(
        const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringStream& outStream)
    {

        if (tt == LOW_LOD)
        {
            if (prof->isShadowingEnabled(tt, terrain))
            {
                generateFpDynamicShadows(prof, terrain, tt, outStream);
                outStream << 
                    "   outputCol.rgb = diffuse * rtshadow;\n";
            }
            else
            {
                outStream << 
                    "   outputCol.rgb = diffuse;\n";
            }
        }
        else
        {
            if (terrain->getGlobalColourMapEnabled() && prof->isGlobalColourMapEnabled())
            {
                // sample colour map and apply to diffuse
                outStream << "  diffuse *= globalColourMap.Sample(g_SamClamp, uv).rgb;\n";
            }
            if (prof->isLightmapEnabled())
            {
                // sample lightmap
                outStream << "  shadow = lightMap.Sample(g_SamClamp, uv).r;\n";
            }

            if (prof->isShadowingEnabled(tt, terrain))
            {
                generateFpDynamicShadows(prof, terrain, tt, outStream);
            }

            // diffuse lighting
            outStream << "  outputCol.rgb += ambient.rgb * diffuse + litRes.y * lightDiffuseColour * diffuse * shadow;\n";

            // specular default
            if (!prof->isLayerSpecularMappingEnabled())
                outStream << "  specular = 1.0;\n";

            if (tt == RENDER_COMPOSITE_MAP)
            {
                // Lighting embedded in alpha
                outStream <<
                    "   outputCol.a = shadow;\n";

            }
            else
            {
                // Apply specular
                outStream << "  outputCol.rgb += litRes.z * lightSpecularColour * specular * shadow;\n";

                if (prof->getParent()->getDebugLevel())
                {
                    outStream << "  outputCol.rg += input.lodInfo.xy;\n";
                }
            }
        }

        bool fog = terrain->getSceneManager()->getFogMode() != FOG_NONE && tt != RENDER_COMPOSITE_MAP;
        if (fog)
        {
            outStream << "  outputCol.rgb = lerp(outputCol.rgb, fogColour, input.fogVal);\n";
        }

        // Final return
        outStream << "  return outputCol;\n"
            << "}\n";

    }
    //---------------------------------------------------------------------
    void TerrainMaterialGeneratorA::SM2Profile::ShaderHelperHLSL::generateFpDynamicShadowsHelpers(
        const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringStream& outStream)
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
            "   return float4(uv.xy + offset * invMapSize * uv.w, uv.z, uv.w); \n"
            "} \n";

        // adding samplerstate for shadowmapping
        outStream <<
            "SamplerState ShadowSampler\n"
            "{\n"
            "   Filter = MIN_MAG_MIP_LINEAR;\n"
            "   AddressU = Wrap;\n"
            "   AddressV = Wrap;\n"
            "};\n";

        if (prof->getReceiveDynamicShadowsDepth())
        {
            outStream << 
                "float calcDepthShadow(Texture2D shadowMap, float4 uv, float invShadowMapSize) \n"
                "{ \n"
                "   // 4-sample PCF \n"
                    
                "   float shadow = 0.0; \n"
                "   float offset = (NUM_SHADOW_SAMPLES_1D/2 - 0.5) * SHADOW_FILTER_SCALE; \n"
                "   for (float y = -offset; y <= offset; y += SHADOW_FILTER_SCALE) \n"
                "       for (float x = -offset; x <= offset; x += SHADOW_FILTER_SCALE) \n"
                "       { \n"
                "           float4 newUV = offsetSample(uv, float2(x, y), invShadowMapSize);\n"
                "           // manually project and assign derivatives \n"
                "           // to avoid gradient issues inside loops \n"
                "           newUV = newUV / newUV.w; \n"
                "           float depth = shadowMap.Sample(ShadowSampler, newUV.xy, 1, 1).x; \n"
                "           if (depth >= 1 || depth >= uv.z)\n"
                "               shadow += 1.0;\n"
                "       } \n"

                "   shadow /= SHADOW_SAMPLES; \n"

                "   return shadow; \n"
                "} \n";
        }
        else
        {
            outStream <<
                "float calcSimpleShadow(Texture2D shadowMap, float4 shadowMapPos) \n"
                "{ \n"
                "   // return tex2Dproj(shadowMap,shadowMapPos); "
                "   return shadowMap.Sample(ShadowSampler, shadowMapPos.xy / shadowMapPos.w).x; \n"
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

            outStream << "\n    ";
            for (uint i = 0; i < numTextures; ++i)
                outStream << "Texture2D shadowMap" << i << ", ";
            outStream << "\n    ";
            for (uint i = 0; i < numTextures; ++i)
                outStream << "float4 lsPos" << i << ", ";
            if (prof->getReceiveDynamicShadowsDepth())
            {
                outStream << "\n    ";
                for (uint i = 0; i < numTextures; ++i)
                    outStream << "float invShadowmapSize" << i << ", ";
            }
            outStream << "\n"
                "   float4 pssmSplitPoints, float camDepth) \n"
                "{ \n"
                "   float shadow; \n"
                "   // calculate shadow \n";
            
            for (uint i = 0; i < numTextures; ++i)
            {
                if (!i)
                    outStream << "  if (camDepth <= pssmSplitPoints." << ShaderHelper::getChannel(i) << ") \n";
                else if (i < numTextures - 1)
                    outStream << "  else if (camDepth <= pssmSplitPoints." << ShaderHelper::getChannel(i) << ") \n";
                else
                    outStream << "  else \n";

                outStream <<
                    "   { \n";
                if (prof->getReceiveDynamicShadowsDepth())
                {
                    outStream <<
                        "       shadow = calcDepthShadow(shadowMap" << i << ", lsPos" << i << ", invShadowmapSize" << i << "); \n";
                }
                else
                {
                    outStream <<
                        "       shadow = calcSimpleShadow(shadowMap" << i << ", lsPos" << i << "); \n";
                }
                outStream <<
                    "   } \n";

            }

            outStream <<
                "   return shadow; \n"
                "} \n\n\n";
        }


    }
    //---------------------------------------------------------------------
    void TerrainMaterialGeneratorA::SM2Profile::ShaderHelperHLSL::generateFpDynamicShadowsParams(
        uint* texCoord, uint* sampler, const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringStream& outStream)
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
                ", uniform sampler2D shadowMap" << i << " : register(s" << *sampler << ") \n";
            *sampler = *sampler + 1;
            if (prof->getReceiveDynamicShadowsDepth())
            {
                outStream <<
                    ", uniform float inverseShadowmapSize" << i << " \n";
            }
        }

    }
    //---------------------------------------------------------------------
    void TerrainMaterialGeneratorA::SM2Profile::ShaderHelperHLSL::generateFpDynamicShadows(
        const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringStream& outStream)
    {
        if (prof->getReceiveDynamicShadowsPSSM())
        {
            uint numTextures = prof->getReceiveDynamicShadowsPSSM()->getSplitCount();
            outStream << 
                "   float camDepth = input.oUVMisc.z;\n";

            if (prof->getReceiveDynamicShadowsDepth())
            {
                outStream << 
                    "   float rtshadow = calcPSSMDepthShadow(";
            }
            else
            {
                outStream << 
                    "   float rtshadow = calcPSSMSimpleShadow(";
            }
            for (uint i = 0; i < numTextures; ++i)
                outStream << "shadowMap" << i << ", ";
            outStream << "\n        ";

            for (uint i = 0; i < numTextures; ++i)
                outStream << "input.oLightSpacePos" << i << ", ";
            if (prof->getReceiveDynamicShadowsDepth())
            {
                outStream << "\n        ";
                for (uint i = 0; i < numTextures; ++i)
                    outStream << "inverseShadowmapSize" << i << ", ";
            }
            outStream << "\n" <<
                "       pssmSplitPoints, camDepth);\n";

        }
        else
        {
            if (prof->getReceiveDynamicShadowsDepth())
            {
                outStream <<
                    "   float rtshadow = calcDepthShadow(shadowMap0, input.oLightSpacePos0, inverseShadowmapSize0);";
            }
            else
            {
                outStream <<
                    "   float rtshadow = calcSimpleShadow(shadowMap0, input.LightSpacePos0);";
            }
        }

        outStream << 
            "   shadow = min(shadow, rtshadow);\n";
        
    }

}
