// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT
#include "OgreShaderPrecompiledHeaders.h"
#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS

namespace Ogre
{
namespace RTShader
{

/************************************************************************/
/*                                                                      */
/************************************************************************/
const String SRS_COOK_TORRANCE_LIGHTING = "CookTorranceLighting";

//-----------------------------------------------------------------------
CookTorranceLighting::CookTorranceLighting()
    : mLightCount(0), mTexCoordSet(0), mMRMapSamplerIndex(0), mLtcLUT1SamplerIndex(-1)
{
}

//-----------------------------------------------------------------------
const String& CookTorranceLighting::getType() const { return SRS_COOK_TORRANCE_LIGHTING; }
//-----------------------------------------------------------------------
bool CookTorranceLighting::createCpuSubPrograms(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Function* vsMain = vsProgram->getEntryPointFunction();
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    Function* psMain = psProgram->getEntryPointFunction();

    vsProgram->addDependency(FFP_LIB_TRANSFORM);

    psProgram->addDependency(FFP_LIB_TRANSFORM);
    psProgram->addDependency(FFP_LIB_TEXTURING);
    psProgram->addDependency("SGXLib_CookTorrance");
    psProgram->addPreprocessorDefines(StringUtil::format("USE_LINEAR_COLOURS,LIGHT_COUNT=%d", mLightCount));
    psProgram->setUseLinearColours(true);

    // Resolve texture coordinates.
    auto uvSet = Parameter::SPC_TEXTURE_COORDINATE0 + mTexCoordSet;
    auto vsOutTexcoord = vsMain->getOutputParameter(uvSet, GCT_FLOAT2); // allow override by others
    ParameterPtr vsInTexcoord;
    if(!vsOutTexcoord)
    {
        vsInTexcoord = vsMain->resolveInputParameter(uvSet, GCT_FLOAT2);
        vsOutTexcoord = vsMain->resolveOutputParameter(uvSet, GCT_FLOAT2);
    }
    auto psInTexcoord = psMain->resolveInputParameter(vsOutTexcoord);

    // resolve view position
    auto vsInPosition = vsMain->getLocalParameter(Parameter::SPC_POSITION_OBJECT_SPACE);
    if (!vsInPosition)
        vsInPosition = vsMain->resolveInputParameter(Parameter::SPC_POSITION_OBJECT_SPACE);
    auto vsOutViewPos = vsMain->resolveOutputParameter(Parameter::SPC_POSITION_VIEW_SPACE);
    auto viewPos = psMain->resolveInputParameter(vsOutViewPos);
    auto worldViewMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLDVIEW_MATRIX);

    // Resolve normal.
    auto viewNormal = psMain->getLocalParameter(Parameter::SPC_NORMAL_VIEW_SPACE);
    ParameterPtr vsInNormal, vsOutNormal;

    if (!viewNormal)
    {
        // Resolve input vertex shader normal.
        vsInNormal = vsMain->resolveInputParameter(Parameter::SPC_NORMAL_OBJECT_SPACE);

        // Resolve output vertex shader normal.
        vsOutNormal = vsMain->resolveOutputParameter(Parameter::SPC_NORMAL_VIEW_SPACE);

        // Resolve input pixel shader normal.
        viewNormal = psMain->resolveInputParameter(vsOutNormal);
    }

    // resolve light params
    auto outDiffuse = psMain->resolveOutputParameter(Parameter::SPC_COLOR_DIFFUSE);
    auto outSpecular = psMain->resolveLocalParameter(Parameter::SPC_COLOR_SPECULAR);

    // insert after texturing
    auto vstage = vsMain->getStage(FFP_PS_COLOUR_BEGIN + 1);
    auto fstage = psMain->getStage(FFP_PS_PBR_LIGHTING_BEGIN);

    // Forward texture coordinates
    if(vsInTexcoord)
        vstage.assign(vsInTexcoord, vsOutTexcoord);
    vstage.callFunction(FFP_FUNC_TRANSFORM, worldViewMatrix, vsInPosition, vsOutViewPos);

    // transform normal in VS
    if (vsOutNormal)
    {
        auto worldViewITMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_NORMAL_MATRIX);
        vstage.callBuiltin("mul", worldViewITMatrix, vsInNormal, vsOutNormal);
    }

    // add the lighting computation
    auto mrparams = psMain->resolveLocalParameter(GCT_FLOAT2, "metalRoughness");
    if(!mMetalRoughnessMapName.empty())
    {
        auto metalRoughnessSampler =
            psProgram->resolveParameter(GCT_SAMPLER2D, "metalRoughnessSampler", mMRMapSamplerIndex);
        auto mrSample = psMain->resolveLocalParameter(GCT_FLOAT4, "mrSample");
        // Roughness is stored in the 'g' channel, metallic is stored in the 'b' channel.
        // This layout intentionally reserves the 'r' channel for (optional) occlusion map data
        fstage.sampleTexture(metalRoughnessSampler, psInTexcoord, mrSample);
        fstage.assign(In(mrSample).mask(Operand::OPM_YZ), mrparams);
    }
    else
    {
        auto specular = psProgram->resolveParameter(GpuProgramParameters::ACT_SURFACE_SPECULAR_COLOUR);
        fstage.assign(In(specular).xy(), mrparams);
    }

    auto sceneCol = psProgram->resolveParameter(GpuProgramParameters::ACT_DERIVED_SCENE_COLOUR);
    auto litResult = psMain->resolveLocalParameter(GCT_FLOAT4, "litResult");
    auto diffuse = psProgram->resolveParameter(GpuProgramParameters::ACT_SURFACE_DIFFUSE_COLOUR);
    auto baseColor = psMain->resolveLocalParameter(GCT_FLOAT3, "baseColor");

    auto pixelParams = psMain->resolveLocalStructParameter("PixelParams", "pixel");

    fstage.mul(In(diffuse).xyz(), In(outDiffuse).xyz(), baseColor);
    fstage.assign(Vector3(0), Out(outDiffuse).xyz());
    fstage.mul(In(diffuse).w(), In(outDiffuse).w(), Out(outDiffuse).w()); // forward alpha

    fstage.callFunction("PBR_MakeParams", {In(baseColor), In(mrparams), InOut(pixelParams)});

    fstage = psMain->getStage(FFP_PS_PBR_LIGHTING_END);
    if(mLightCount > 0)
    {
        auto lightPos = psProgram->resolveParameter(GpuProgramParameters::ACT_LIGHT_POSITION_VIEW_SPACE_ARRAY, mLightCount);
        auto lightDiffuse = psProgram->resolveParameter(GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR_POWER_SCALED_ARRAY, mLightCount);
        auto pointParams = psProgram->resolveParameter(GpuProgramParameters::ACT_LIGHT_ATTENUATION_ARRAY, mLightCount);
        auto spotParams = psProgram->resolveParameter(GpuProgramParameters::ACT_SPOTLIGHT_PARAMS_ARRAY, mLightCount);
        auto lightDirView = psProgram->resolveParameter(GpuProgramParameters::ACT_LIGHT_DIRECTION_VIEW_SPACE_ARRAY, mLightCount);

        std::vector<Operand> params = {In(viewNormal),       In(viewPos),     In(sceneCol),          In(lightPos),
                                       In(lightDiffuse),     In(pointParams), In(lightDirView),      In(spotParams),
                                       In(pixelParams),      InOut(outDiffuse).xyz()};

        if(mLtcLUT1SamplerIndex > -1)
        {
            auto ltcLUT1 = psProgram->resolveParameter(GCT_SAMPLER2D, "ltcLUT1Sampler", mLtcLUT1SamplerIndex);
            auto ltcLUT2 = psProgram->resolveParameter(GCT_SAMPLER2D, "ltcLUT2Sampler", mLtcLUT1SamplerIndex + 1);
            params.insert(params.begin(), {In(ltcLUT1), In(ltcLUT2)});
            psProgram->addPreprocessorDefines("HAVE_AREA_LIGHTS");
        }

        if (auto shadowFactor = psMain->getLocalParameter("lShadowFactor"))
        {
            params.insert(params.begin(), In(shadowFactor));
        }

        fstage.callFunction("PBR_Lights", params);
    }

    return true;
}

//-----------------------------------------------------------------------
void CookTorranceLighting::copyFrom(const SubRenderState& rhs)
{
    const CookTorranceLighting& rhsLighting = static_cast<const CookTorranceLighting&>(rhs);
    mMetalRoughnessMapName = rhsLighting.mMetalRoughnessMapName;
    mSampler = rhsLighting.mSampler;
    mLightCount = rhsLighting.mLightCount;
    mTexCoordSet = rhsLighting.mTexCoordSet;
}

//-----------------------------------------------------------------------
bool CookTorranceLighting::preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass)
{
    if (!srcPass->getLightingEnabled())
        return false;

    mLightCount = renderState->getLightCount();

    if(renderState->haveAreaLights())
        mLtcLUT1SamplerIndex = ensureLtcLUTPresent(dstPass);

    if(mMetalRoughnessMapName.empty())
        return true;

    dstPass->createTextureUnitState(mMetalRoughnessMapName);
    mMRMapSamplerIndex = dstPass->getNumTextureUnitStates() - 1;

    if(mSampler)
        dstPass->getTextureUnitState(mMRMapSamplerIndex)->setSampler(mSampler);

    return true;
}

bool CookTorranceLighting::setParameter(const String& name, const String& value)
{
    if (name == "texture" && !value.empty())
    {
        mMetalRoughnessMapName = value;
        return true;
    }
    else if (name == "tex_coord_set")
    {
        return StringConverter::parse(value, mTexCoordSet);
    }

    return false;
}

void CookTorranceLighting::setParameter(const String& name, const Any& value)
{
    if (name == "sampler")
    {
        mSampler = any_cast<SamplerPtr>(value);
        return;
    }

    SubRenderState::setParameter(name, value);
}

//-----------------------------------------------------------------------
const String& CookTorranceLightingFactory::getType() const { return SRS_COOK_TORRANCE_LIGHTING; }

//-----------------------------------------------------------------------
SubRenderState* CookTorranceLightingFactory::createInstance(const ScriptProperty& prop, Pass* pass,
                                                            SGScriptTranslator* translator)
{
    if (prop.name == "lighting_stage" && prop.values.size() >= 1)
    {
        // Read light model type.
        if (prop.values[0] != "metal_roughness")
            return NULL;

        auto subRenderState = createOrRetrieveInstance(translator);

        if(prop.values.size() < 3)
            return subRenderState;

        if (prop.values[1] != "texture")
        {
            translator->emitError(prop.values[1]);
            return subRenderState;
        }

        if(!subRenderState->setParameter("texture", prop.values[2]))
            translator->emitError();

        if(prop.values.size() < 4)
            return subRenderState;

        if (auto sampler = TextureManager::getSingleton().getSampler(prop.values[3]))
        {
            subRenderState->setParameter("sampler", sampler);
        }
        else
        {
            translator->emitError();
        }

        if(prop.values.size() < 5)
            return subRenderState;

        if(!subRenderState->setParameter("tex_coord_set", prop.values[4]))
            translator->emitError();

        return subRenderState;
    }

    return NULL;
}

//-----------------------------------------------------------------------
void CookTorranceLightingFactory::writeInstance(MaterialSerializer* ser, SubRenderState* subRenderState, Pass* srcPass,
                                                Pass* dstPass)
{
    auto ctSubRenderState = static_cast<CookTorranceLighting*>(subRenderState);

    ser->writeAttribute(4, "lighting_stage");
    ser->writeValue("metal_roughness");
    if(ctSubRenderState->getMetalRoughnessMapName().empty())
        return;
    ser->writeValue("texture");
    ser->writeValue(ctSubRenderState->getMetalRoughnessMapName());
}

//-----------------------------------------------------------------------
SubRenderState* CookTorranceLightingFactory::createInstanceImpl() { return OGRE_NEW CookTorranceLighting; }

} // namespace RTShader
} // namespace Ogre

#endif
