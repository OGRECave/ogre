// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT
#include "OgreTerrainRTShaderSRS.h"

#include "OgrePass.h"
#include "OgreShaderProgramSet.h"
#include "OgreShaderParameter.h"
#include "OgreMaterialSerializer.h"
#include "OgreShaderGenerator.h"
#include "OgreShaderFunction.h"
#include "OgreShaderProgram.h"
#include "OgreTerrainQuadTreeNode.h"
#include "OgreTextureManager.h"

namespace Ogre {
using namespace RTShader;

/************************************************************************/
/*                                                                      */
/************************************************************************/
String TerrainTransform::Type = "TerrainTransform";

bool TerrainTransform::preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass)
{
    auto terrainAny = srcPass->getUserObjectBindings().getUserAny("Terrain");
    mTerrain = any_cast<const Terrain*>(terrainAny);
    mCompressed = mTerrain->_getUseVertexCompression();
    mAlign = mTerrain->getAlignment();
    return true;
}

void TerrainTransform::updateParams()
{
    if(!mCompressed)
        return;

    mPointTrans->setGpuParameter(mTerrain->getPointTransform());

    float baseUVScale = 1.0f / (mTerrain->getSize() - 1);
    mBaseUVScale->setGpuParameter(baseUVScale);
}

//-----------------------------------------------------------------------
bool TerrainTransform::createCpuSubPrograms(ProgramSet* programSet)
{
    static Operand::OpMask heightAxis[3] = {Operand::OPM_Y, Operand::OPM_Z, Operand::OPM_X};

    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Function* vsEntry = vsProgram->getEntryPointFunction();

    auto posType = mCompressed ? GCT_INT2 : GCT_FLOAT4;

    auto wvpMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);

    auto lodMorph = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_CUSTOM, GCT_FLOAT2, Terrain::LOD_MORPH_CUSTOM_PARAM);

    auto positionIn = vsEntry->resolveInputParameter(Parameter::SPC_POSITION_OBJECT_SPACE, posType);
    auto positionOut = vsEntry->resolveOutputParameter(Parameter::SPC_POSITION_PROJECTIVE_SPACE);
    auto uv = vsEntry->resolveOutputParameter(Parameter::SPC_TEXTURE_COORDINATE0, GCT_FLOAT2);
    auto delta = vsEntry->resolveInputParameter(Parameter::SPC_TEXTURE_COORDINATE1, GCT_FLOAT2);

    // Add dependency.
    vsProgram->addDependency("FFPLib_Transform");
    vsProgram->addDependency("TerrainTransforms");

    auto stage = vsEntry->getStage(FFP_VS_TRANSFORM);

    if(mCompressed)
    {
        mPointTrans = vsProgram->resolveParameter(GCT_MATRIX_4X4, "pointTrans");
        mBaseUVScale = vsProgram->resolveParameter(GCT_FLOAT1, "baseUVScale");
        auto height = vsEntry->resolveInputParameter(Parameter::SPC_TEXTURE_COORDINATE0, GCT_FLOAT1);
        auto position = vsEntry->resolveLocalParameter(Parameter::SPC_POSITION_OBJECT_SPACE);

        stage.callFunction("expandVertex",
                       {In(mPointTrans), In(mBaseUVScale), In(positionIn), In(height), Out(position), Out(uv)});
        positionIn = position;
    }
    else
    {
        auto uvin = vsEntry->resolveInputParameter(Parameter::SPC_TEXTURE_COORDINATE0, GCT_FLOAT2);
        stage.assign(In(uvin).xy(), uv);
    }

    stage.callFunction("FFP_Transform", wvpMatrix, positionIn, positionOut);
    stage.callFunction("applyLODMorph", {In(delta), In(lodMorph), InOut(positionOut).mask(heightAxis[mAlign])});

    return true;
}

String TerrainSurface::Type = "TerrainSurface";

bool TerrainSurface::setParameter(const String& name, const String& value)
{
    if(name == "for_composite_map")
    {
        return StringConverter::parse(value, mForCompositeMap);
    }
    else if (name == "use_parallax_mapping")
    {
        return StringConverter::parse(value, mUseParallaxMapping);
    }
    else if (name == "use_specular_mapping")
    {
        return StringConverter::parse(value, mUseSpecularMapping);
    }
    else if (name == "use_normal_mapping")
    {
        return StringConverter::parse(value, mUseNormalMapping);
    }

    return false;
}

bool TerrainSurface::preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass)
{
    mTerrain = any_cast<const Terrain*>(srcPass->getUserObjectBindings().getUserAny("Terrain"));

    SamplerPtr clampSampler = TextureManager::getSingleton().createSampler();
    clampSampler->setAddressingMode(TAM_CLAMP);

    auto tu = dstPass->createTextureUnitState();
    tu->setTexture(mTerrain->getTerrainNormalMap());
    tu->setSampler(clampSampler);

    if (auto cm = mTerrain->getGlobalColourMap())
    {
        tu = dstPass->createTextureUnitState();
        tu->setTexture(cm);
        tu->setSampler(clampSampler);
    }

    if(auto lm = mTerrain->getLightmap())
    {
        tu = dstPass->createTextureUnitState();
        tu->setTexture(lm);
        tu->setSampler(clampSampler);
    }

    for(auto bt : mTerrain->getBlendTextures())
    {
        tu = srcPass->createTextureUnitState();
        tu->setTexture(bt);
        tu->setSampler(clampSampler);
    }

    mUVMul.resize((mTerrain->getLayerCount() + 3) / 4); // integer ceil

    mUseNormalMapping = mUseNormalMapping && !mTerrain->getLayerTextureName(0, 1).empty();
    for (int i = 0; i < mTerrain->getLayerCount(); ++i)
    {
        srcPass->createTextureUnitState(mTerrain->getLayerTextureName(i, 0));
        if (mUseNormalMapping)
            srcPass->createTextureUnitState(mTerrain->getLayerTextureName(i, 1));
    }

    return true;
}

void TerrainSurface::updateParams()
{
    for (size_t i = 0; i < mUVMul.size(); i++)
    {
        Vector4 uvMul(mTerrain->getLayerUVMultiplier(i + 0), mTerrain->getLayerUVMultiplier(i + 1),
                      mTerrain->getLayerUVMultiplier(i + 2), mTerrain->getLayerUVMultiplier(i + 3));
        mUVMul[i]->setGpuParameter(uvMul);
    }
}

static Operand::OpMask channel[4] = {Operand::OPM_X, Operand::OPM_Y, Operand::OPM_Z, Operand::OPM_W};

//-----------------------------------------------------------------------
bool TerrainSurface::createCpuSubPrograms(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    Function* psMain = psProgram->getMain();
    Function* vsMain = vsProgram->getMain();

    psProgram->addDependency("FFPLib_Transform");
    psProgram->addDependency("SGXLib_NormalMap");
    psProgram->addDependency("SGXLib_IntegratedPSSM");
    psProgram->addDependency("TerrainSurface");

    if(mUseNormalMapping)
        psProgram->addPreprocessorDefines("TERRAIN_NORMAL_MAPPING");

    auto uvVS = vsMain->resolveOutputParameter(Parameter::SPC_TEXTURE_COORDINATE0, GCT_FLOAT2);

    if (mForCompositeMap)
    {
        auto uvIn = vsMain->resolveInputParameter(Parameter::SPC_TEXTURE_COORDINATE0, GCT_FLOAT2);
        // forward texcoords in VS
        vsMain->getStage(FFP_VS_TEXTURING).assign(uvIn, uvVS);
    }

    ParameterPtr viewPos;
    if (mUseNormalMapping && mUseParallaxMapping)
    {
        psProgram->addPreprocessorDefines("TERRAIN_PARALLAX_MAPPING");
        // assuming: lighting stage computed this
        auto vsOutViewPos = vsMain->resolveOutputParameter(Parameter::SPC_POSITION_VIEW_SPACE);
        viewPos = psMain->resolveInputParameter(vsOutViewPos);
    }

    auto uvPS = psMain->resolveInputParameter(uvVS);
    uvPS->setHighP(true);

    for(auto& uvMul : mUVMul)
    {
        uvMul = psProgram->resolveParameter(GCT_FLOAT4, "uvMul");
    }

    int texUnit = 0;
    auto globalNormal = psProgram->resolveParameter(GCT_SAMPLER2D, "globalNormal", texUnit++);

    ParameterPtr globalColourMap;
    if (mTerrain->getGlobalColourMap())
        globalColourMap = psProgram->resolveParameter(GCT_SAMPLER2D, "globalColour", texUnit++);

    ParameterPtr lightMap;
    if(mTerrain->getLightmap())
        lightMap = psProgram->resolveParameter(GCT_SAMPLER2D, "lightMap", texUnit++);

    auto normal = psMain->resolveLocalParameter(Parameter::SPC_NORMAL_VIEW_SPACE);
    auto ITMat = psProgram->resolveParameter(GpuProgramParameters::ACT_NORMAL_MATRIX);

    auto diffuse = psMain->resolveLocalParameter(Parameter::SPC_COLOR_DIFFUSE);
    auto diffuseSpec = psMain->resolveLocalParameter(GCT_FLOAT4, "diffuseSpec");
    auto TSnormal = psMain->resolveLocalParameter(GCT_FLOAT3, "TSnormal");
    auto texTmp = psMain->resolveLocalParameter(GCT_FLOAT4, "texTmp");

    auto outDiffuse = psMain->resolveOutputParameter(Parameter::SPC_COLOR_DIFFUSE);

    auto stage = psMain->getStage(FFP_PS_COLOUR_BEGIN);
    stage.assign(Vector4(1), outDiffuse); // FFPColour logic
    stage.callFunction("SGX_FetchNormal", globalNormal, uvPS, normal);
    stage.callFunction("FFP_Transform", ITMat, normal, normal);

    auto psSpecular = psMain->resolveLocalParameter(Parameter::SPC_COLOR_SPECULAR);
    stage.assign(Vector4::ZERO, psSpecular);

    std::vector<ParameterPtr> blendWeights;
    for(auto bt : mTerrain->getBlendTextures())
    {
        auto weight = psMain->resolveLocalParameter(GCT_FLOAT4, StringUtil::format("blendWeight%d", texUnit));
        auto blendTex = psProgram->resolveParameter(GCT_SAMPLER2D, "blendTex", texUnit++);
        stage.sampleTexture(blendTex, uvPS, weight);
        blendWeights.push_back(weight);
    }

    stage.assign(Vector4::ZERO, diffuseSpec);
    stage.assign(Vector3(0, 0, 1), TSnormal);
    for (int l = 0; l < mTerrain->getLayerCount(); ++l)
    {
        auto blendWeight = l == 0 ? In(1.0f) : In(blendWeights[(l - 1) / 4]).mask(channel[(l - 1) % 4]);
        auto difftex = psProgram->resolveParameter(GCT_SAMPLER2D, "difftex", texUnit++);
        std::vector<Operand> args = {blendWeight, In(uvPS), In(mUVMul[l/4]).mask(channel[l % 4])};
        if (mUseNormalMapping)
        {
            if (mUseParallaxMapping)
            {
                args.push_back(In(viewPos));
                args.push_back(In(Vector2(0.03, -0.04)));
            }
            auto normtex = psProgram->resolveParameter(GCT_SAMPLER2D, "normtex", texUnit++);
            args.push_back(In(normtex));
            args.push_back(Out(TSnormal));
        }
        args.push_back(In(difftex));
        args.push_back(Out(diffuseSpec));
        stage.callFunction("blendTerrainLayer", {args});
    }

    if(mUseNormalMapping)
        stage.callFunction("transformToTS", {In(TSnormal), In(ITMat), InOut(normal)});

    // fake vertexcolour input for TVC_SPECULAR
    if(mUseSpecularMapping)
        stage.mul(In(diffuseSpec).w(), Vector4(1), diffuse);

    if(lightMap)
    {
        auto shadowFactor = psMain->resolveLocalParameter(GCT_FLOAT1, "lShadowFactor");
        stage = psMain->getStage(FFP_PS_COLOUR_BEGIN - 1); // before the PSSM stage
        stage.assign(1, shadowFactor);

        stage = psMain->getStage(FFP_PS_COLOUR_BEGIN + 1); // after the PSSM stage
        stage.callFunction("getShadowFactor", {In(lightMap), In(uvPS), InOut(shadowFactor)});
    }

    if(globalColourMap)
    {
        stage = psMain->getStage(FFP_PS_COLOUR_BEGIN + 2); // after lighting calculations
        stage.sampleTexture(globalColourMap, uvPS, texTmp);
        stage.mul(In(diffuseSpec).xyz(), In(texTmp).xyz(), Out(diffuseSpec).xyz());
    }

    stage = psMain->getStage(FFP_PS_TEXTURING);
    stage.mul(diffuseSpec, outDiffuse, outDiffuse);

    // FFPColour logic
    psMain->getStage(FFP_PS_COLOUR_END)
        .add(In(outDiffuse).xyz(), In(psSpecular).xyz(), Out(outDiffuse).xyz());
    return true;
}

//-----------------------------------------------------------------------
const String& TerrainTransformFactory::getType() const
{
    return TerrainTransform::Type;
}
SubRenderState* TerrainTransformFactory::createInstanceImpl()
{
    return OGRE_NEW TerrainTransform();
}

const String& TerrainSurfaceFactory::getType() const
{
    return TerrainSurface::Type;
}
SubRenderState* TerrainSurfaceFactory::createInstanceImpl()
{
    return OGRE_NEW TerrainSurface();
}

}