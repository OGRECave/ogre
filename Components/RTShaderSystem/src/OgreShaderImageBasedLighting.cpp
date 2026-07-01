// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#include "OgreShaderPrecompiledHeaders.h"
#include "OgreRenderTarget.h"
#ifdef RTSHADER_SYSTEM_BUILD_CORE_SHADERS

namespace Ogre
{
namespace RTShader
{

/************************************************************************/
/*                                                                      */
/************************************************************************/
const String SRS_IMAGE_BASED_LIGHTING = "ImageBasedLighting";

//-----------------------------------------------------------------------
const String& ImageBasedLighting::getType() const { return SRS_IMAGE_BASED_LIGHTING; }

//-----------------------------------------------------------------------
int ImageBasedLighting::getExecutionOrder() const { return FFP_LIGHTING + 10; }

bool ImageBasedLighting::setParameter(const String& name, const String& value)
{
    if ((name == "texture" || name == "texture_diffuse") && !value.empty())
    {
        mEnvMapName = value;
        return true;
    }
    else if (name == "luminance")
    {
        mIsLuminanceParamDirty = true;
        return StringConverter::parse(value, mLuminance);
    }
    else if (name == "texture_specular" && !value.empty())
    {
        //If set, the shader will use seperate "diffuse irradiance" and "specular ggx" cubemaps for IBL.
        mEnvMapNameSpecular = value;
        return true;
    }

    //Enables local probes.
    else if (name == "local_probe_sh_atlas" && !value.empty())
    {
        mIsLocalProbesEnabled = true;
        mLocalProbeShAtlas = value;
        return true;
    }

    //Enables masking by vertex colour. Black = global , white = local.
    else if (name == "vertex_colour_probe_mask")
    {
        mUseVertexColourProbeMask = StringConverter::parseBool(value);
        return true;
    }

    return false;
}

void ImageBasedLighting::copyFrom(const SubRenderState& rhs)
{
    const ImageBasedLighting& rhsIBL = static_cast<const ImageBasedLighting&>(rhs);
    mEnvMapName = rhsIBL.mEnvMapName;
    mEnvMapNameSpecular = rhsIBL.mEnvMapNameSpecular;
    mLuminance = rhsIBL.mLuminance;
    mIsLocalProbesEnabled = rhsIBL.mIsLocalProbesEnabled;
    mLocalProbeShAtlas = rhsIBL.mLocalProbeShAtlas;
    mLocalBlendWeight = rhsIBL.mLocalBlendWeight;
    mUseVertexColourProbeMask = rhsIBL.mUseVertexColourProbeMask;
}

bool ImageBasedLighting::preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass)
{
    if (!srcPass->getLightingEnabled())
        return false;

    // requires GLES3
    if (ShaderGenerator::getSingleton().getTargetLanguage() == "glsles" &&
        !GpuProgramManager::getSingleton().isSyntaxSupported("glsl300es"))
        return false;

    // generate with ./cmgen --size=64 --ibl-dfg-multiscatter --ibl-dfg=dfgLUTmultiscatter.dds
    // see https://github.com/google/filament/blob/78554d231947bae965492eb5c47ad24a8d4a426e/filament/CMakeLists.txt#L510
    auto tus = dstPass->createTextureUnitState("dfgLUTmultiscatter.dds");
    tus->setNumMipmaps(0);
    mDfgLUTSamplerIndex = dstPass->getNumTextureUnitStates() - 1;

    tus = dstPass->createTextureUnitState();
    tus->setTextureName(mEnvMapName, TEX_TYPE_CUBE_MAP);
    tus->setHardwareGammaEnabled(true);
    mEnvMapSamplerIndex = dstPass->getNumTextureUnitStates() - 1;

    // Check for dual cubemaps (diffuse irradiance plus specular convoluted). Add second texture if applicable.
    if (!mEnvMapNameSpecular.empty())
    {
        tus = dstPass->createTextureUnitState();
        tus->setTextureName(mEnvMapNameSpecular, TEX_TYPE_CUBE_MAP);
        tus->setHardwareGammaEnabled(true);
        mEnvMapSamplerIndexSpecular = dstPass->getNumTextureUnitStates() - 1;
    }

    //Add local probe spherical harmionics coefficient atlas.
    if (mIsLocalProbesEnabled)
    {
        tus = dstPass->createTextureUnitState();
        tus->setTextureName(mLocalProbeShAtlas, TEX_TYPE_2D);
        tus->setHardwareGammaEnabled(false);
        tus->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
        tus->setTextureFiltering(TFO_NONE);
        tus->setNumMipmaps(0);
        mLocalProbeShAtlasSamplerIndex = dstPass->getNumTextureUnitStates() - 1;
    }

    return true;
}

bool ImageBasedLighting::createCpuSubPrograms(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Function* vsMain = vsProgram->getEntryPointFunction();
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    Function* psMain = psProgram->getEntryPointFunction();

    auto vsOutViewPos = vsMain->resolveOutputParameter(Parameter::SPC_POSITION_VIEW_SPACE);
    auto viewPos = psMain->resolveInputParameter(vsOutViewPos);

    //World position for light probes
    auto vsInObjectPos = vsMain->resolveInputParameter(Parameter::SPC_POSITION_OBJECT_SPACE);

    auto worldMat = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLD_MATRIX);

    auto vsOutWorldPos = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, 7,
                                                        Parameter::SPC_POSITION_WORLD_SPACE, GCT_FLOAT3);

    auto worldPos = psMain->resolveInputParameter(vsOutWorldPos);

    auto stage = vsMain->getStage(FFP_VS_TRANSFORM + 1);

    stage.callFunction(FFP_FUNC_TRANSFORM, worldMat, vsInObjectPos, vsOutWorldPos);

    auto pixel = psMain->getLocalParameter("pixel");
    mLuminanceParam = psProgram->resolveParameter(GCT_FLOAT1, "luminance");
    mBlendWeightParam = NULL;

    if (!pixel)
    {
        LogManager::getSingleton().logError("image_based_lighting must be used with the metal_roughness SRS");
        return true;
    }

    auto viewNormal = psMain->getLocalParameter(Parameter::SPC_NORMAL_VIEW_SPACE); // allow normal map injection
    if(!viewNormal)
    {
        auto vsOutNormal = vsMain->resolveOutputParameter(Parameter::SPC_NORMAL_VIEW_SPACE);
        viewNormal = psMain->resolveInputParameter(vsOutNormal);
    }

    psProgram->addDependency("RTSLib_IBL");

    auto outDiffuse = psMain->resolveOutputParameter(Parameter::SPC_COLOR_DIFFUSE);

    auto dfgLUTSampler = psProgram->resolveParameter(GCT_SAMPLER2D, "dfgLUTSampler", mDfgLUTSamplerIndex);
    auto iblEnvSampler = psProgram->resolveParameter(GCT_SAMPLERCUBE, "iblEnvSampler", mEnvMapSamplerIndex);
    Ogre::RTShader::UniformParameterPtr iblEnvSpecularSampler = NULL;
    Ogre::RTShader::UniformParameterPtr iblEnvSize = NULL;

    // Add secondary cubemap if applicable.
    if (mEnvMapNameSpecular != "")
    {
        psProgram->addPreprocessorDefines("HAS_DUAL_TEXTURE");
        iblEnvSpecularSampler = psProgram->resolveParameter(GCT_SAMPLERCUBE, "iblEnvSpecularSampler", mEnvMapSamplerIndexSpecular);
        iblEnvSize = psProgram->resolveParameter(GpuProgramParameters::ACT_TEXTURE_SIZE, mEnvMapSamplerIndexSpecular);
    }
    else
    {
        iblEnvSize = psProgram->resolveParameter(GpuProgramParameters::ACT_TEXTURE_SIZE, mEnvMapSamplerIndex);
    }

    //Add required local probe parameters and texture, if local probes are enabled.
    ParameterPtr psInputVertexColour;
    ParameterPtr probeMaskParam;

    if (mIsLocalProbesEnabled && mUseVertexColourProbeMask)
    {
        // Feed through vertex colours as they are used for probe masking.
        // Black = exterior/global, white = interior/local.
        ParameterPtr vsInputVertexColour = vsMain->resolveInputParameter(Parameter::SPC_COLOR_DIFFUSE);

        ParameterPtr vsOutputVertexColour = vsMain->resolveOutputParameter(Parameter::SPC_COLOR_DIFFUSE);

        psInputVertexColour = psMain->resolveInputParameter(vsOutputVertexColour);

        auto vsColourStage = vsMain->getStage(FFP_VS_COLOUR);
        vsColourStage.assign(vsInputVertexColour, vsOutputVertexColour);

        psProgram->addPreprocessorDefines("HAS_VERTEX_COLOUR_PROBE_MASK");
    }

    if (mIsLocalProbesEnabled)
    {

        //Define has local probes
        psProgram->addPreprocessorDefines("HAS_LOCAL_PROBE");

        //SH Coefficients texture atlas. Row = probe ID. 9 px wide for 9 SH Coeffs.
        mLocalProbeShAtlasSampler = psProgram->resolveParameter(GCT_SAMPLER2D, "iblLocalShAtlasSampler", mLocalProbeShAtlasSamplerIndex);

        mShAtlasSizeParam = psProgram->resolveParameter(GpuProgramParameters::ACT_TEXTURE_SIZE, mLocalProbeShAtlasSamplerIndex);

        //Local probe 1
        mLocalProbeParams1_1 = psProgram->resolveParameter(GCT_FLOAT4, "IBL_LocalProbe_1_1");   //x, y, z = position. w = probe ID for SH atlas reference.
        mLocalProbeParams1_2 = psProgram->resolveParameter(GCT_FLOAT4, "IBL_LocalProbe_1_2");   //x, y, z = box extent. w = blend distance.
        mLocalProbeParams1_3 = psProgram->resolveParameter(GCT_FLOAT4, "IBL_LocalProbe_1_3");   //Quaternoin for orientation of probe

        // Local probe 2        
        mLocalProbeParams2_1 = psProgram->resolveParameter(GCT_FLOAT4, "IBL_LocalProbe_2_1");
        mLocalProbeParams2_2 = psProgram->resolveParameter(GCT_FLOAT4, "IBL_LocalProbe_2_2");
        mLocalProbeParams2_3 = psProgram->resolveParameter(GCT_FLOAT4, "IBL_LocalProbe_2_3");

        // Local probe 3        
        mLocalProbeParams3_1 = psProgram->resolveParameter(GCT_FLOAT4, "IBL_LocalProbe_3_1");
        mLocalProbeParams3_2 = psProgram->resolveParameter(GCT_FLOAT4, "IBL_LocalProbe_3_2");
        mLocalProbeParams3_3 = psProgram->resolveParameter(GCT_FLOAT4, "IBL_LocalProbe_3_3");

        // Local probe 4
        mLocalProbeParams4_1 = psProgram->resolveParameter(GCT_FLOAT4, "IBL_LocalProbe_4_1");
        mLocalProbeParams4_2 = psProgram->resolveParameter(GCT_FLOAT4, "IBL_LocalProbe_4_2");
        mLocalProbeParams4_3 = psProgram->resolveParameter(GCT_FLOAT4, "IBL_LocalProbe_4_3");
    }

    auto invViewMat = psProgram->resolveParameter(GpuProgramParameters::ACT_INVERSE_VIEW_MATRIX);

    auto fstage = psMain->getStage(FFP_PS_PBR_LIGHTING_BEGIN + 5);

    //Add second texture if set.
    if (mIsLocalProbesEnabled)
    {
        if (mUseVertexColourProbeMask)
        {
            fstage.callFunction("evaluateIBL", {InOut(pixel),
                                                In(viewNormal),
                                                In(viewPos),
                                                In(worldPos),
                                                In(invViewMat),
                                                In(dfgLUTSampler),
                                                In(iblEnvSampler),
                                                In(iblEnvSpecularSampler),
                                                In(iblEnvSize).w(),
                                                In(mLuminanceParam),

                                                In(mLocalProbeShAtlasSampler),
                                                In(mShAtlasSizeParam).xy(),

                                                In(mLocalProbeParams1_1),
                                                In(mLocalProbeParams1_2),
                                                In(mLocalProbeParams1_3),

                                                In(mLocalProbeParams2_1),
                                                In(mLocalProbeParams2_2),
                                                In(mLocalProbeParams2_3),

                                                In(mLocalProbeParams3_1),
                                                In(mLocalProbeParams3_2),
                                                In(mLocalProbeParams3_3),

                                                In(mLocalProbeParams4_1),
                                                In(mLocalProbeParams4_2),
                                                In(mLocalProbeParams4_3),

                                                In(psInputVertexColour).x(), // NEW

                                                InOut(outDiffuse).xyz()});
        }
        else
        {
            fstage.callFunction("evaluateIBL", {InOut(pixel),
                                                In(viewNormal),
                                                In(viewPos),
                                                In(worldPos),
                                                In(invViewMat),
                                                In(dfgLUTSampler),
                                                In(iblEnvSampler),
                                                In(iblEnvSpecularSampler),
                                                In(iblEnvSize).w(),
                                                In(mLuminanceParam),

                                                In(mLocalProbeShAtlasSampler),
                                                In(mShAtlasSizeParam).xy(),

                                                In(mLocalProbeParams1_1),
                                                In(mLocalProbeParams1_2),
                                                In(mLocalProbeParams1_3),

                                                In(mLocalProbeParams2_1),
                                                In(mLocalProbeParams2_2),
                                                In(mLocalProbeParams2_3),

                                                In(mLocalProbeParams3_1),
                                                In(mLocalProbeParams3_2),
                                                In(mLocalProbeParams3_3),

                                                In(mLocalProbeParams4_1),
                                                In(mLocalProbeParams4_2),
                                                In(mLocalProbeParams4_3),

                                                InOut(outDiffuse).xyz()});
        }
    }
    else
    {
        //No local probes
        if (mEnvMapNameSpecular == "")
        {       
            //No local, no specular - old behaviour
            fstage.callFunction("evaluateIBL",
                    {InOut(pixel), In(viewNormal), In(viewPos), In(invViewMat), In(dfgLUTSampler),
                        In(iblEnvSampler), In(iblEnvSize).w(), In(mLuminanceParam), InOut(outDiffuse).xyz()});
        }
        else
        {
            //No local, specular edition.
            fstage.callFunction("evaluateIBL", {InOut(pixel), In(viewNormal), In(viewPos), In(invViewMat),
                                                In(dfgLUTSampler), In(iblEnvSampler), In(iblEnvSpecularSampler),
                                                In(iblEnvSize).w(), In(mLuminanceParam), InOut(outDiffuse).xyz()});
        }
    }
    return true;
}

void ImageBasedLighting::updateGpuProgramsParams(Renderable* rend, const Pass* pass, const AutoParamDataSource* source, const LightList* ll)
{
    if (mIsLuminanceParamDirty)
    {
        mLuminanceParam->setGpuParameter(mLuminance);
        mIsLuminanceParamDirty = false;
    }

    //Update local probe data if enabled
    if (mIsLocalProbesEnabled && rend)
    {
        Vector4 probeData;

        //Local probe 1
        probeData = Vector4(0, 0, 0, -1); // Default to -1/no probe.
        if (rend->hasCustomParameter(0))
        {
            probeData = rend->getCustomParameter(0);
        }
        mLocalProbeParams1_1->setGpuParameter(probeData);

        probeData = Vector4(1, 1, 1, 0);
        if (rend->hasCustomParameter(1))
        {
            probeData = rend->getCustomParameter(1);
        }
        mLocalProbeParams1_2->setGpuParameter(probeData);

        probeData = Vector4(0, 0, 0, 1);
        if (rend->hasCustomParameter(2))
        {
            probeData = rend->getCustomParameter(2);
        }
        mLocalProbeParams1_3->setGpuParameter(probeData);

        // Local probe 2
        probeData = Vector4(0, 0, 0, -1); // Default to -1/no probe.
        if (rend->hasCustomParameter(3))
        {
            probeData = rend->getCustomParameter(3);
        }
        mLocalProbeParams2_1->setGpuParameter(probeData);

        probeData = Vector4(1, 1, 1, 0);
        if (rend->hasCustomParameter(4))
        {
            probeData = rend->getCustomParameter(4);
        }
        mLocalProbeParams2_2->setGpuParameter(probeData);

        probeData = Vector4(0, 0, 0, 1);
        if (rend->hasCustomParameter(5))
        {
            probeData = rend->getCustomParameter(5);
        }
        mLocalProbeParams2_3->setGpuParameter(probeData);

        // Local probe 3
        probeData = Vector4(0, 0, 0, -1); // Default to -1/no probe.
        if (rend->hasCustomParameter(6))
        {
            probeData = rend->getCustomParameter(6);
        }
        mLocalProbeParams3_1->setGpuParameter(probeData);

        probeData = Vector4(1, 1, 1, 0);
        if (rend->hasCustomParameter(7))
        {
            probeData = rend->getCustomParameter(7);
        }
        mLocalProbeParams3_2->setGpuParameter(probeData);

        probeData = Vector4(0, 0, 0, 1);
        if (rend->hasCustomParameter(8))
        {
            probeData = rend->getCustomParameter(8);
        }
        mLocalProbeParams3_3->setGpuParameter(probeData);

        // Local probe 4
        probeData = Vector4(0, 0, 0, -1); // Default to -1/no probe.
        if (rend->hasCustomParameter(9))
        {
            probeData = rend->getCustomParameter(9);
        }
        mLocalProbeParams4_1->setGpuParameter(probeData);

        probeData = Vector4(1, 1, 1, 0);
        if (rend->hasCustomParameter(10))
        {
            probeData = rend->getCustomParameter(10);
        }
        mLocalProbeParams4_2->setGpuParameter(probeData);

        probeData = Vector4(0, 0, 0, 1);
        if (rend->hasCustomParameter(11))
        {
            probeData = rend->getCustomParameter(11);
        }
        mLocalProbeParams4_3->setGpuParameter(probeData);
    }
}


//-----------------------------------------------------------------------
const String& ImageBasedLightingFactory::getType() const { return SRS_IMAGE_BASED_LIGHTING; }

//-----------------------------------------------------------------------
SubRenderState* ImageBasedLightingFactory::createInstance(const ScriptProperty& prop, Pass* pass,
                                                          SGScriptTranslator* translator)
{
    if (prop.name != "image_based_lighting" || prop.values.size() < 2)
        return NULL;

    if (prop.values[0] != "texture" && prop.values[0] != "texture_diffuse")
    {
        translator->emitError();
        return NULL;
    }
    auto ret = static_cast<ImageBasedLighting*>(createOrRetrieveInstance(translator));
    ret->setParameter(prop.values[0], prop.values[1]);

    if (prop.values.size() < 4)
        return ret;

    if(prop.values[2] != "luminance")
    {
        translator->emitError();
        return NULL;
    }

    ret->setParameter("luminance", prop.values[3]);

    if (prop.values.size() < 6)
    return ret;

    //If this is set "texture" is then used as a diffuse texture only
    if (prop.values[4] != "texture_specular")
    {
        translator->emitError();
        return NULL;
    }

    ret->setParameter("texture_specular", prop.values[5]);

    if (prop.values.size() < 8)
        return ret;

    //If set then enable local probes and use value for sh coefficients.
    if (prop.values[6] != "local_probe_sh_atlas")
    {
        translator->emitError();
        return NULL;
    }
    ret->setParameter("local_probe_sh_atlas", prop.values[7]);

    if (prop.values.size() < 10)
        return ret;

    if (prop.values[8] != "vertex_colour_probe_mask")
    {
        translator->emitError();
        return NULL;
    }

    ret->setParameter("vertex_colour_probe_mask", prop.values[9]);

    return ret;
}

//-----------------------------------------------------------------------
void ImageBasedLightingFactory::writeInstance(MaterialSerializer* ser, SubRenderState* subRenderState, Pass* srcPass,
                                   Pass* dstPass)
{
    auto ibl = static_cast<ImageBasedLighting*>(subRenderState);
    ser->writeAttribute(4, "image_based_lighting");
    ser->writeValue("texture");
    ser->writeValue(ibl->mEnvMapName);
    ser->writeValue("luminance");
    ser->writeValue(std::to_string(ibl->mLuminance));
}

//-----------------------------------------------------------------------
SubRenderState* ImageBasedLightingFactory::createInstanceImpl() { return OGRE_NEW ImageBasedLighting; }

} // namespace RTShader
} // namespace Ogre

#endif