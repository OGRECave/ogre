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
#include "OgreTerrainMaterialShaderHelpers.h"
#include "OgreTerrain.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreShadowCameraSetupPSSM.h"
#include "OgreHighLevelGpuProgram.h"
#include "OgreGpuProgramManager.h"

namespace Ogre
{
    //---------------------------------------------------------------------
    ShaderHelperGLSL::ShaderHelperGLSL() : ShaderHelper()
    {
        const auto& hmgr = HighLevelGpuProgramManager::getSingleton();
        for(auto lang : {"glsl", "hlsl", "glsles", "glslang"})
        {
            if(hmgr.isLanguageSupported(lang))
            {
                mLang = lang;
                break;
            }
        }
    }
    //---------------------------------------------------------------------
    HighLevelGpuProgramPtr
    ShaderHelperGLSL::createVertexProgram(
        const SM2Profile* prof, const Terrain* terrain, TechniqueType tt)
    {
        HighLevelGpuProgramManager& mgr = HighLevelGpuProgramManager::getSingleton();
        String progName = getVertexProgramName(prof, terrain, tt);

        HighLevelGpuProgramPtr ret = mgr.getByName(progName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        if (!ret)
        {
            ret = mgr.createProgram(progName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                    mLang, GPT_VERTEX_PROGRAM);
        }
        else
        {
            ret->unload();
        }

        if (prof->getParent()->getDebugLevel())
        {
            ret->setParameter("preprocessor_defines",
                StringUtil::format("TERRAIN_DEBUG,NUM_LODS=%d", terrain->getNumLodLevels()));
        }

        return ret;
    }
    //---------------------------------------------------------------------
    HighLevelGpuProgramPtr
        ShaderHelperGLSL::createFragmentProgram(
            const SM2Profile* prof, const Terrain* terrain, TechniqueType tt)
    {
        HighLevelGpuProgramManager& mgr = HighLevelGpuProgramManager::getSingleton();
        String progName = getFragmentProgramName(prof, terrain, tt);

        HighLevelGpuProgramPtr ret = mgr.getByName(progName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        if (!ret)
        {
            ret = mgr.createProgram(progName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                    mLang, GPT_FRAGMENT_PROGRAM);
        }
        else
        {
            ret->unload();
        }

        if (prof->getReceiveDynamicShadowsPSSM())
        {
            uint numShadowTextures = prof->getReceiveDynamicShadowsPSSM()->getSplitCount();

            auto defines = StringUtil::format("PSSM_NUM_SPLITS=%d", numShadowTextures);
            if (!prof->getReceiveDynamicShadowsDepth())
                defines += ",PSSM_SAMPLE_COLOUR";

            ret->setParameter("preprocessor_defines", defines);
        }

        if(mLang == "hlsl")
        {
            ret->setParameter("enable_backwards_compatibility", "true");
            ret->setParameter("target", "ps_4_0 ps_3_0 ps_2_b");
        }

        return ret;
    }

    //---------------------------------------------------------------------
    void ShaderHelperGLSL::generateVertexProgramSource(const SM2Profile* prof, const Terrain* terrain,
                                                                                   TechniqueType tt, StringStream& outStream)
    {
        outStream << "#include <OgreUnifiedShader.h>\n";
        outStream << "#include <FFPLib_Fog.glsl>\n";
        outStream << "#include <TerrainTransforms.glsl>\n";

        outStream <<
            "OGRE_UNIFORMS_BEGIN\n"
            "uniform mat4 worldMatrix;\n"
            "uniform mat4 viewProjMatrix;\n"
            "uniform vec2 lodMorph;\n"; // morph amount, morph LOD target

        bool compression = terrain->_getUseVertexCompression() && tt != RENDER_COMPOSITE_MAP;
        bool fog = terrain->getSceneManager()->getFogMode() != FOG_NONE && tt != RENDER_COMPOSITE_MAP;

        if (compression)
        {
            outStream <<
                "uniform mat4 posIndexToObjectSpace;\n"
                "uniform float baseUVScale;\n";
        }

        if (fog)
            outStream << "uniform vec4 fogParams;\n";

        uint numShadowTextures = uint(prof->isShadowingEnabled(tt, terrain));
        if (numShadowTextures)
        {
            if (prof->getReceiveDynamicShadowsPSSM())
            {
                numShadowTextures = prof->getReceiveDynamicShadowsPSSM()->getSplitCount();
            }
            for (uint i = 0; i < numShadowTextures; ++i)
            {
                outStream << "uniform mat4 texViewProjMatrix" << i << ";\n";
            }
        }
        outStream << "OGRE_UNIFORMS_END\n";

        outStream << "MAIN_PARAMETERS\n";
        if (compression)
        {
            const char* idx2 = (mLang == "glsl" || mLang == "glsles") ? "vec2" : "ivec2";
            outStream << "IN(" << idx2 << " vertex, POSITION)\n"
                         "IN(float uv0, TEXCOORD0)\n";
        }
        else
        {
            outStream << "IN(vec4 position, POSITION)\n"
                         "IN(vec2 uv0, TEXCOORD0)\n";
        }
        if (tt != RENDER_COMPOSITE_MAP)
            outStream << "IN(vec2 delta, TEXCOORD1)\n"; // lodDelta, lodThreshold

        outStream << "OUT(vec4 oPosObj, TEXCOORD0)\n";

        uint texCoordSet = 1;
        outStream << "OUT(vec4 oUVMisc, TEXCOORD" << texCoordSet++ << ")\n"; // xy = uv, z = camDepth;

        if (prof->getParent()->getDebugLevel() && tt != RENDER_COMPOSITE_MAP)
        {
            outStream << "OUT(vec2 lodInfo, TEXCOORD" << texCoordSet++ << ")\n";
        }

        if (fog)
            outStream << "OUT(float fogVal, COLOR)\n";

        for (uint i = 0; i < numShadowTextures; ++i)
        {
	        outStream << StringUtil::format("OUT(vec4 oLightSpacePos%d, TEXCOORD%d)\n", i, texCoordSet++);
        }

        // check we haven't exceeded texture coordinates
        if (texCoordSet > 8)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Requested options require too many texture coordinate sets! Try reducing the number of layers.");
        }

        outStream <<
            "MAIN_DECLARATION\n"
            "{\n";
        if (compression)
        {
            outStream <<
                "    vec2 _uv0; vec4 position;\n"
                "    expandVertex(posIndexToObjectSpace, baseUVScale, vertex, uv0, position, _uv0);\n"
                "#define uv0 _uv0\n"; // alias uv0 to _uv0
        }
        outStream <<
            "    vec4 worldPos = mul(worldMatrix, position);\n"
            "    oPosObj = position;\n";

        // morph
        if (tt != RENDER_COMPOSITE_MAP)
        {
            static char heightAxis[] = "yzx";
            outStream << "    applyLODMorph(delta, lodMorph, worldPos."
                      << heightAxis[terrain->getAlignment()];

            if (prof->getParent()->getDebugLevel())
                outStream << ", lodInfo";
            outStream << ");\n";
        }

        outStream <<
            "    gl_Position = mul(viewProjMatrix, worldPos);\n"
            "    oUVMisc.xy = uv0.xy;\n";

        if (fog)
        {
            if (terrain->getSceneManager()->getFogMode() == FOG_LINEAR)
            {
                outStream << "    FFP_VertexFog_Linear(gl_Position, fogParams, fogVal);\n";
            }
            else
            {
                outStream << "    FFP_VertexFog_Exp(gl_Position, fogParams, fogVal);\n";
            }
        }

        if (numShadowTextures)
            generateVpDynamicShadows(prof, terrain, tt, outStream);

        outStream << "}\n";
    }
    //---------------------------------------------------------------------
    void ShaderHelperGLSL::generateFpHeader(const SM2Profile* prof, const Terrain* terrain,
                                                                                   TechniqueType tt, StringStream& outStream)
    {
        outStream << "#define USE_OGRE_FROM_FUTURE\n";
        outStream << "#include <OgreUnifiedShader.h>\n";
        // shader libs
        outStream << "#include <SGXLib_NormalMap.glsl>\n";
        outStream << "#include <SGXLib_PerPixelLighting.glsl>\n";
        outStream << "#include <SGXLib_IntegratedPSSM.glsl>\n";

        // UV's premultiplied, packed as xy/zw
        uint maxLayers = prof->getMaxLayers(terrain);
        uint numBlendTextures =
            std::min<uint8>(Terrain::getBlendTextureCount(maxLayers), terrain->getBlendTextures().size());
        uint numLayers = std::min(maxLayers, static_cast<uint>(terrain->getLayerCount()));

        uint currentSamplerIdx = 0;
        outStream << StringUtil::format("SAMPLER2D(globalNormal, %d);\n", currentSamplerIdx++);

        if (terrain->getGlobalColourMapEnabled() && prof->isGlobalColourMapEnabled())
        {
            outStream << StringUtil::format("SAMPLER2D(globalColourMap, %d);\n", currentSamplerIdx++);
        }
        if (prof->isLightmapEnabled())
        {
            outStream << StringUtil::format("SAMPLER2D(lightMap, %d);\n", currentSamplerIdx++);
        }
        // Blend textures - sampler definitions
        for (uint i = 0; i < numBlendTextures; ++i)
        {
            outStream << StringUtil::format("SAMPLER2D(blendTex%d, %d);\n", i, currentSamplerIdx++);
        }

        // Layer textures - sampler definitions & UV multipliers
        for (uint i = 0; i < numLayers; ++i)
        {
            outStream << StringUtil::format("SAMPLER2D(difftex%d, %d);\n", i, currentSamplerIdx++);
            if(prof->isLayerNormalMappingEnabled())
                outStream << StringUtil::format("SAMPLER2D(normtex%d, %d);\n", i, currentSamplerIdx++);
        }

        mShadowSamplerStartHi = currentSamplerIdx;
        uint numShadowTextures = uint(prof->isShadowingEnabled(tt, terrain));
        if (numShadowTextures && prof->getReceiveDynamicShadowsPSSM())
            numShadowTextures = prof->getReceiveDynamicShadowsPSSM()->getSplitCount();
        for (uint i = 0; i < numShadowTextures; ++i)
            outStream << StringUtil::format("SAMPLER2D(shadowMap%d, %d);\n", i, currentSamplerIdx++);

        outStream << "OGRE_UNIFORMS_BEGIN\n";
        // uv multipliers
        uint numUVMultipliers = (numLayers + 3) / 4; // integer ceil
        for (uint i = 0; i < numUVMultipliers; ++i)
            outStream << "uniform vec4 uvMul_" << i << ";\n";

        if (numShadowTextures && prof->getReceiveDynamicShadowsPSSM())
            outStream << "uniform vec4 pssmSplitPoints;\n";
        for (uint i = 0; i < numShadowTextures; ++i)
            outStream << "uniform float inverseShadowmapSize" << i << ";\n";

        // check we haven't exceeded samplers
        OgreAssert(
            currentSamplerIdx < OGRE_MAX_TEXTURE_LAYERS,
            "Requested options require too many texture samplers! Try reducing the number of layers.");

        outStream <<
            // Only 1 light supported in this version
            // deferred shading profile / generator later, ok? :)
            "uniform vec4 lightPosObjSpace;\n"
            "uniform vec3 lightDiffuseColour;\n"
            "uniform vec3 lightSpecularColour;\n"
            "uniform vec3 eyePosObjSpace;\n"
            "uniform vec4 ambient;\n"
            // pack scale, bias and specular
            "uniform vec4 scaleBiasSpecular;\n";

        bool fog = terrain->getSceneManager()->getFogMode() != FOG_NONE && tt != RENDER_COMPOSITE_MAP;

        if (fog)
            outStream << "uniform vec3 fogColour;\n";

        outStream << "OGRE_UNIFORMS_END\n";

        outStream << "MAIN_PARAMETERS\n"
                     "IN(vec4 oPosObj, TEXCOORD0)\n";
        uint texCoordSet = 1;
        outStream << "IN(f32vec4 oUVMisc, TEXCOORD" << texCoordSet++ << ")\n";

        if (prof->getParent()->getDebugLevel() && tt != RENDER_COMPOSITE_MAP)
        {
            outStream << "IN(vec2 lodInfo, TEXCOORD" << texCoordSet++ << ")\n";
        }

        if (fog)
            outStream << "IN(float fogVal, COLOR)\n";

        for (uint i = 0; i < numShadowTextures; ++i)
        {
            outStream << StringUtil::format("IN(vec4 oLightSpacePos%d, TEXCOORD%d)\n", i, texCoordSet++);
        }

        outStream << "MAIN_DECLARATION\n"
            "{\n"
            "    float shadow = 1.0;\n"
            "    f32vec2 uv = oUVMisc.xy;\n"
            // base colour
            "    gl_FragColor = vec4(0,0,0,1);\n";

        outStream <<
            // global normal
        "    vec3 normal;\n"
        "    SGX_FetchNormal(globalNormal, uv, normal);\n";

        outStream <<
            "    vec3 lightDir = \n"
            "        -(lightPosObjSpace.xyz - (oPosObj.xyz * lightPosObjSpace.w));\n"
            "    vec3 eyeDir = eyePosObjSpace - oPosObj.xyz;\n"

            // set up accumulation areas
            "    vec3 diffuse = vec3(0,0,0);\n"
            "    float specular = 0.0;\n";

        // set up the blend values
        for (uint i = 0; i < numBlendTextures; ++i)
        {
            outStream << "    vec4 blendTexVal" << i << " = texture2D(blendTex" << i << ", uv);\n";
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
                    outStream << "    vec3 tangent = vec3(1, 0, 0);\n";
                    break;
                case Terrain::ALIGN_Y_Z:
                    outStream << "    vec3 tangent = vec3(0, 0, -1);\n";
                    break;
            };

            outStream << "    vec3 binormal = normalize(cross(tangent, normal));\n";
            // note, now we need to re-cross to derive tangent again because it wasn't orthonormal
            outStream << "    tangent = normalize(cross(normal, binormal));\n";
            // derive final matrix
            outStream << "    mat3 TBN = mtxFromRows(tangent, binormal, normal);\n";

            // set up lighting result placeholders for interpolation
            outStream << "    vec3 TSnormal;\n";
            if (prof->isLayerParallaxMappingEnabled())
                outStream << "    float displacement;\n";
            // move
            outStream << "    lightDir = normalize(mul(TBN, lightDir));\n";
            outStream << "    eyeDir = normalize(mul(TBN, eyeDir));\n";
        }
        else
        {
            // simple per-pixel lighting with no normal mapping
            outStream << "    lightDir = normalize(lightDir);\n";
            outStream << "    eyeDir = normalize(eyeDir);\n";
        }
        outStream << "    vec3 halfAngle = normalize(lightDir + eyeDir);\n";
    }
    //---------------------------------------------------------------------
    void ShaderHelperGLSL::generateFpLayer(const SM2Profile* prof, const Terrain* terrain,
                                                                                  TechniqueType tt, uint layer, StringStream& outStream)
    {
        uint uvMulIdx = layer / 4;
        uint blendIdx = (layer-1) / 4;
        String blendWeightStr = StringUtil::format("blendTexVal%d.%s", blendIdx, getChannel(layer-1));

        // generate UV
        outStream << "    vec2 uv" << layer << " = mod(uv * uvMul_" << uvMulIdx << "." << getChannel(layer) << ", 1.0);\n";

        // calculate lighting here if normal mapping
        if (prof->isLayerNormalMappingEnabled())
        {
            if (prof->isLayerParallaxMappingEnabled() && tt != RENDER_COMPOSITE_MAP)
            {
                // modify UV - note we have to sample an extra time
                outStream << "    displacement = texture2D(normtex" << layer << ", uv" << layer << ").a\n"
                             "        * scaleBiasSpecular.x + scaleBiasSpecular.y;\n";
                outStream << "    uv" << layer << " += eyeDir.xy * displacement;\n";
            }

            // access TS normal map
            outStream << StringUtil::format("    SGX_FetchNormal(normtex%d, uv%d, TSnormal);\n", layer, layer);

            if (!layer)
                outStream << "    normal = TSnormal;\n";
            else
                outStream << "    normal += TSnormal * " << blendWeightStr << ";\n";

        }

        // sample diffuse texture
        outStream << "    vec4 diffuseSpecTex" << layer
            << " = texture2D(difftex" << layer << ", uv" << layer << ");\n";

        // apply to common
        if (!layer)
        {
            outStream << "    diffuse = diffuseSpecTex0.rgb;\n";
            if (prof->isLayerSpecularMappingEnabled())
                outStream << "    specular = diffuseSpecTex0.a;\n";
        }
        else
        {
            outStream << "    diffuse = mix(diffuse, diffuseSpecTex" << layer
                << ".rgb, " << blendWeightStr << ");\n";
            if (prof->isLayerSpecularMappingEnabled())
                outStream << "    specular = mix(specular, diffuseSpecTex" << layer
                    << ".a, " << blendWeightStr << ");\n";
        }
    }
    //---------------------------------------------------------------------
    void ShaderHelperGLSL::generateFpFooter(const SM2Profile* prof, const Terrain* terrain,
                                                                                   TechniqueType tt, StringStream& outStream)
    {
        if (terrain->getGlobalColourMapEnabled() && prof->isGlobalColourMapEnabled())
        {
            // sample colour map and apply to diffuse
            outStream << "    diffuse *= texture2D(globalColourMap, uv).rgb;\n";
        }
        if (prof->isLightmapEnabled())
        {
            // sample lightmap
            outStream << "    shadow = texture2D(lightMap, uv).r;\n";
        }

        if (prof->isShadowingEnabled(tt, terrain))
        {
            generateFpDynamicShadows(prof, terrain, tt, outStream);
        }

        // specular default
        if (!prof->isLayerSpecularMappingEnabled())
            outStream << "    specular = 1.0;\n";

        if (tt == RENDER_COMPOSITE_MAP)
        {
            outStream << "    SGX_Light_Directional_Diffuse(normal, lightDir, diffuse, gl_FragColor.rgb);\n";
            // Lighting embedded in alpha
            outStream << "    gl_FragColor.a = shadow;\n";
        }
        else
        {
            outStream << "    vec3 specularCol = vec3(0,0,0);\n";
            outStream << "    SGX_Light_Directional_DiffuseSpecular(normal, eyeDir, lightDir, lightDiffuseColour * diffuse, "
                            "lightSpecularColour * specular, scaleBiasSpecular.z, gl_FragColor.rgb, specularCol);\n";

            // Apply specular
            outStream << "    gl_FragColor.rgb += specularCol;\n";
        }
        outStream << "    gl_FragColor.rgb = gl_FragColor.rgb * shadow + ambient.rgb * diffuse;\n";

        if (prof->getParent()->getDebugLevel())
        {
            outStream << "    gl_FragColor.rg += lodInfo.xy;\n";
        }

        bool fog = terrain->getSceneManager()->getFogMode() != FOG_NONE && tt != RENDER_COMPOSITE_MAP;
        if (fog)
        {
            outStream << "    gl_FragColor.rgb = mix(fogColour, gl_FragColor.rgb, fogVal);\n";
        }

        outStream << "}\n";
    }
    //---------------------------------------------------------------------
    void ShaderHelperGLSL::generateVpDynamicShadows(const SM2Profile* prof, const Terrain* terrain,
                                                                                           TechniqueType tt, StringStream& outStream)
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
                "    oLightSpacePos" << i << " = mul(texViewProjMatrix" << i << ", worldPos); \n";
        }

        if (prof->getReceiveDynamicShadowsPSSM())
        {
            outStream <<
                "    // pass cam depth\n"
                "    oUVMisc.z = gl_Position.z;\n";
        }
    }
    //---------------------------------------------------------------------
    void ShaderHelperGLSL::generateFpDynamicShadows(const SM2Profile* prof, const Terrain* terrain,
                                                                                           TechniqueType tt, StringStream& outStream)
    {
        outStream << "    float rtshadow;";
        if (prof->getReceiveDynamicShadowsPSSM())
        {
            uint numTextures = prof->getReceiveDynamicShadowsPSSM()->getSplitCount();
            outStream <<
                "    SGX_ComputeShadowFactor_PSSM3(";
            outStream << "oUVMisc.z, pssmSplitPoints,\n        ";
            for (uint i = 0; i < numTextures; ++i)
            {
                outStream << "oLightSpacePos" << i << ", ";
                outStream << "shadowMap" << i << ", ";
                outStream << "vec2_splat(inverseShadowmapSize" << i << "), ";
                outStream << "\n        ";
            }

            outStream << "        rtshadow);\n";
        }
        else
        {
            outStream <<
                "    SGX_ShadowPCF4(shadowMap0, oLightSpacePos0, vec2_splat(inverseShadowmapSize0), rtshadow);";
        }

        outStream << 
            "    shadow = min(shadow, rtshadow);\n";
    }

}
