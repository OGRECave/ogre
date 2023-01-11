// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#include "OgreShaderPrecompiledHeaders.h"
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
    if (name == "texture" && !value.empty())
    {
        mEnvMapName = value;
        return true;
    }
    else if (name == "luminance")
    {
        mIsLuminanceParamDirty = true;
        return StringConverter::parse(value, mLuminance);
    }

    return false;
}

void ImageBasedLighting::copyFrom(const SubRenderState& rhs)
{
    const ImageBasedLighting& rhsIBL = static_cast<const ImageBasedLighting&>(rhs);
    mEnvMapName = rhsIBL.mEnvMapName;
    mLuminance = rhsIBL.mLuminance;
}

bool ImageBasedLighting::preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass)
{
    if (!srcPass->getLightingEnabled())
        return false;
    // generate with ./cmgen --size=64 --ibl-dfg-multiscatter --ibl-dfg=dfgLUTmultiscatter.dds
    // see https://github.com/google/filament/blob/78554d231947bae965492eb5c47ad24a8d4a426e/filament/CMakeLists.txt#L510
    auto tus = dstPass->createTextureUnitState("dfgLUTmultiscatter.dds");
    tus->setNumMipmaps(0);
    mDfgLUTSamplerIndex = dstPass->getNumTextureUnitStates() - 1;

    tus = dstPass->createTextureUnitState(mEnvMapName);
    tus->setHardwareGammaEnabled(true);
    mEnvMapSamplerIndex = dstPass->getNumTextureUnitStates() - 1;

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

    auto pixel = psMain->getLocalParameter("pixel");
    mLuminanceParam = psProgram->resolveParameter(GCT_FLOAT1, "luminance");

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

    auto iblEnvSize = psProgram->resolveParameter(GpuProgramParameters::ACT_TEXTURE_SIZE, mEnvMapSamplerIndex);
    auto invViewMat = psProgram->resolveParameter(GpuProgramParameters::ACT_INVERSE_VIEW_MATRIX);

    auto fstage = psMain->getStage(FFP_PS_COLOUR_END + 55); // run before CookTorrance evaluation

    fstage.callFunction("evaluateIBL",
                        {InOut(pixel), In(viewNormal), In(viewPos), In(invViewMat), In(dfgLUTSampler), In(iblEnvSampler),
                         In(iblEnvSize).w(), In(mLuminanceParam), InOut(outDiffuse).xyz()});

    return true;
}

void ImageBasedLighting::updateGpuProgramsParams(Renderable* rend, const Pass* pass, const AutoParamDataSource* source, const LightList* ll)
{
    if (mIsLuminanceParamDirty)
    {
        mLuminanceParam->setGpuParameter(mLuminance);
        mIsLuminanceParamDirty = false;
    }
}


//-----------------------------------------------------------------------
const String& ImageBasedLightingFactory::getType() const { return SRS_IMAGE_BASED_LIGHTING; }

//-----------------------------------------------------------------------
SubRenderState* ImageBasedLightingFactory::createInstance(ScriptCompiler* compiler, PropertyAbstractNode* prop, Pass* pass,
                                               SGScriptTranslator* translator)
{
    if (prop->name != "image_based_lighting" || prop->values.size() < 2)
        return NULL;

    auto it = prop->values.begin();
    if((*it++)->getString() != "texture")
    {
        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
        return NULL;
    }
    auto ret = static_cast<ImageBasedLighting*>(createOrRetrieveInstance(translator));
    ret->setParameter("texture", (*it++)->getString());

    if (prop->values.size() < 4)
        return ret;

    if((*it++)->getString() != "luminance")
    {
        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
        return NULL;
    }

    ret->setParameter("luminance", (*it++)->getString());

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
