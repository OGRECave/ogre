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
#include "OgreShaderPrecompiledHeaders.h"
#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS

#define SGX_LIB_INTEGRATEDPSSM                      "SGXLib_IntegratedPSSM"

namespace Ogre {
namespace RTShader {

/************************************************************************/
/*                                                                      */
/************************************************************************/
String IntegratedPSSM3::Type = "SGX_IntegratedPSSM3";
const String SRS_INTEGRATED_PSSM3 = "SGX_IntegratedPSSM3";
const String SRS_SHADOW_MAPPING = "SGX_IntegratedPSSM3";

//-----------------------------------------------------------------------
IntegratedPSSM3::IntegratedPSSM3()
{
    mPCFxSamples = 2;
    mUseTextureCompare = false;
    mUseColourShadows = false;
    mDebug = false;
    mIsD3D9 = false;
    mShadowTextureParamsList.resize(1); // normal single texture depth shadowmapping
    mMultiLightCount = 1;
}

//-----------------------------------------------------------------------
int IntegratedPSSM3::getExecutionOrder() const
{
    return FFP_LIGHTING - 1;
}

//-----------------------------------------------------------------------
void IntegratedPSSM3::updateGpuProgramsParams(Renderable* rend, const Pass* pass,
                                             const AutoParamDataSource* source,
                                             const LightList* pLightList)
{
    if (mMultiLightCount > 1)
        return;

    Vector4 vSplitPoints;

    for(size_t i = 0; i < mShadowTextureParamsList.size() - 1; i++)
    {
        vSplitPoints[i] = mShadowTextureParamsList[i].mMaxRange;
    }
    vSplitPoints[3] = mShadowTextureParamsList.back().mMaxRange;

    const Matrix4& proj = source->getProjectionMatrix();

    for(int i = 0; i < 4; i++)
    {
        auto tmp = proj * Vector4(0, 0, -vSplitPoints[i], 1);
        vSplitPoints[i] = tmp[2] / tmp[3];
    }


    mPSSplitPoints->setGpuParameter(vSplitPoints);

}

//-----------------------------------------------------------------------
void IntegratedPSSM3::copyFrom(const SubRenderState& rhs)
{
    const IntegratedPSSM3& rhsPssm= static_cast<const IntegratedPSSM3&>(rhs);

    mPCFxSamples = rhsPssm.mPCFxSamples;
    mUseTextureCompare = rhsPssm.mUseTextureCompare;
    mUseColourShadows = rhsPssm.mUseColourShadows;
    mDebug = rhsPssm.mDebug;
    mMultiLightCount = rhsPssm.mMultiLightCount;
    mShadowTextureParamsList.resize(rhsPssm.mShadowTextureParamsList.size());

    ShadowTextureParamsConstIterator itSrc = rhsPssm.mShadowTextureParamsList.begin();
    for (auto& p : mShadowTextureParamsList)
    {
        p.mMaxRange = itSrc->mMaxRange;
        ++itSrc;
    }
}

//-----------------------------------------------------------------------
bool IntegratedPSSM3::preAddToRenderState(const RenderState* renderState,
                                         Pass* srcPass, Pass* dstPass)
{
    if (!srcPass->getParent()->getParent()->getReceiveShadows() ||
        renderState->getLightCount() == 0)
        return false;

    mIsD3D9 = ShaderGenerator::getSingleton().getTargetLanguage() == "hlsl" &&
              !GpuProgramManager::getSingleton().isSyntaxSupported("vs_4_0_level_9_1");

    PixelFormat shadowTexFormat = PF_UNKNOWN;
    const auto& configs = ShaderGenerator::getSingleton().getActiveSceneManager()->getShadowTextureConfigList();
    if (!configs.empty())
        shadowTexFormat = configs[0].format; // assume first texture is representative
    mUseTextureCompare = PixelUtil::isDepth(shadowTexFormat) && !mIsD3D9;
    mUseColourShadows = PixelUtil::getComponentType(shadowTexFormat) == PCT_BYTE; // use colour shadowmaps for byte textures

    if(mMultiLightCount > 1)
        mShadowTextureParamsList.resize(mMultiLightCount);

    auto shadowSampler = TextureManager::getSingleton().getSampler(mUseTextureCompare ? "Ogre/DepthShadowSampler"
                                                                                      : "Ogre/ShadowSampler");
    for (auto& p : mShadowTextureParamsList)
    {
        TextureUnitState* curShadowTexture = dstPass->createTextureUnitState();
        curShadowTexture->setContentType(TextureUnitState::CONTENT_SHADOW);
        curShadowTexture->setSampler(shadowSampler);
        p.mTextureSamplerIndex = dstPass->getNumTextureUnitStates() - 1;
    }

    return true;
}

//-----------------------------------------------------------------------
void IntegratedPSSM3::setSplitPoints(const SplitPointList& newSplitPoints)
{
    OgreAssert(newSplitPoints.size() <= 5, "at most 5 split points are supported");

    mShadowTextureParamsList.resize(newSplitPoints.size() - 1);

    for (size_t i = 1; i < newSplitPoints.size(); ++i)
    {
        mShadowTextureParamsList[i - 1].mMaxRange = newSplitPoints[i];
    }
}

bool IntegratedPSSM3::setParameter(const String& name, const String& value)
{
    if(name == "debug")
    {
        return StringConverter::parse(value, mDebug);
    }
    else if (name == "filter")
    {
        if(value == "pcf4")
            mPCFxSamples = 2;
        else if(value == "pcf16")
            mPCFxSamples = 4;
        else
            return false;

        return true;
    }
    else if (name == "light_count")
    {
        mMultiLightCount = StringConverter::parseInt(value);
        return true;
    }

    return false;
}

void IntegratedPSSM3::setParameter(const String& name, const Any& value)
{
    if(name == "split_points")
    {
        setSplitPoints(any_cast<SplitPointList>(value));
        return;
    }
    else if (name == "debug")
    {
        mDebug = any_cast<bool>(value);
        return;
    }

    SubRenderState::setParameter(name, value);
}

//-----------------------------------------------------------------------
bool IntegratedPSSM3::resolveParameters(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    Function* vsMain = vsProgram->getEntryPointFunction();
    Function* psMain = psProgram->getEntryPointFunction();

    // Get input position parameter.
    mVSInPos = vsMain->getLocalParameter(Parameter::SPC_POSITION_OBJECT_SPACE);
    if(!mVSInPos)
        mVSInPos = vsMain->getInputParameter(Parameter::SPC_POSITION_OBJECT_SPACE);

    // Get output position parameter.
    mVSOutPos = vsMain->getOutputParameter(Parameter::SPC_POSITION_PROJECTIVE_SPACE);

    if (mIsD3D9)
    {
        mVSOutPos = vsMain->resolveOutputParameter(Parameter::SPC_UNKNOWN, GCT_FLOAT4);
    }

    // Resolve input depth parameter.
    mPSInDepth = psMain->resolveInputParameter(mVSOutPos);

    // Resolve computed local shadow colour parameter.
    mPSLocalShadowFactor = psMain->resolveLocalParameter(GCT_FLOAT1, "lShadowFactor", mMultiLightCount);

    // Resolve computed local shadow colour parameter.
    mPSSplitPoints = psProgram->resolveParameter(GCT_FLOAT4, "pssm_split_points");

    int lightIndex = 0;

    for (auto& p : mShadowTextureParamsList)
    {
        p.mWorldViewProjMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_TEXTURE_WORLDVIEWPROJ_MATRIX, lightIndex);
        p.mVSOutLightPosition = vsMain->resolveOutputParameter(Parameter::Content(Parameter::SPC_POSITION_LIGHT_SPACE0 + lightIndex));
        p.mPSInLightPosition = psMain->resolveInputParameter(p.mVSOutLightPosition);
        auto stype = mUseTextureCompare ? GCT_SAMPLER2DSHADOW : GCT_SAMPLER2D;
        p.mTextureSampler = psProgram->resolveParameter(stype, "shadow_map", p.mTextureSamplerIndex);
        p.mInvTextureSize = psProgram->resolveParameter(GpuProgramParameters::ACT_INVERSE_TEXTURE_SIZE, p.mTextureSamplerIndex);
        ++lightIndex;
    }

    if (!(mVSInPos.get()) || !(mVSOutPos.get()))
    {
        OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Not all parameters could be constructed for the sub-render state.");
    }

    return true;
}

//-----------------------------------------------------------------------
bool IntegratedPSSM3::resolveDependencies(ProgramSet* programSet)
{
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    psProgram->addDependency(SGX_LIB_INTEGRATEDPSSM);

    psProgram->addPreprocessorDefines(StringUtil::format("PSSM_NUM_SPLITS=%zu,PCF_XSAMPLES=%.1f,SHADOWLIGHT_COUNT=%d",
                                                         mShadowTextureParamsList.size(), mPCFxSamples, mMultiLightCount));

    if(mDebug)
        psProgram->addPreprocessorDefines("DEBUG_PSSM");

    if(mUseTextureCompare)
        psProgram->addPreprocessorDefines("PSSM_SAMPLE_CMP");

    if(mUseColourShadows)
        psProgram->addPreprocessorDefines("PSSM_SAMPLE_COLOUR");

    return true;
}

//-----------------------------------------------------------------------
bool IntegratedPSSM3::addFunctionInvocations(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Function* vsMain = vsProgram->getEntryPointFunction();
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);

    // Add vertex shader invocations.
    if (false == addVSInvocation(vsMain, FFP_VS_TEXTURING + 1))
        return false;

    // Add pixel shader invocations.
    if (false == addPSInvocation(psProgram, FFP_PS_COLOUR_BEGIN))
        return false;

    return true;
}

//-----------------------------------------------------------------------
bool IntegratedPSSM3::addVSInvocation(Function* vsMain, const int groupOrder)
{
    auto stage = vsMain->getStage(groupOrder);

    if(mIsD3D9)
    {
        auto vsOutPos = vsMain->resolveOutputParameter(Parameter::SPC_POSITION_PROJECTIVE_SPACE);
        stage.assign(vsOutPos, mVSOutPos);
    }

    // Compute world space position.
    for (auto& p : mShadowTextureParamsList)
    {
        stage.callBuiltin("mul", p.mWorldViewProjMatrix, mVSInPos, p.mVSOutLightPosition);
    }

    return true;
}

//-----------------------------------------------------------------------
bool IntegratedPSSM3::addPSInvocation(Program* psProgram, const int groupOrder)
{
    Function* psMain = psProgram->getEntryPointFunction();
    auto stage = psMain->getStage(groupOrder);

    if(mShadowTextureParamsList.size() < 2  || mMultiLightCount > 1)
    {
        for(uchar i = 0; i < mMultiLightCount; ++i)
        {
            ShadowTextureParams& params = mShadowTextureParamsList[i];
            stage.callFunction("SGX_ShadowPCF4",
                               {In(params.mTextureSampler), In(params.mPSInLightPosition),
                                In(params.mInvTextureSize).xy(), Out(mPSLocalShadowFactor), At(i)});
        }
    }
    else
    {
        auto fdepth = psMain->resolveLocalParameter(GCT_FLOAT1, "fdepth");
        if(mIsD3D9)
            stage.div(In(mPSInDepth).z(), In(mPSInDepth).w(), fdepth);
        else
            stage.assign(In(mPSInDepth).z(), fdepth);
        std::vector<Operand> params = {In(fdepth), In(mPSSplitPoints)};

        for(auto& texp : mShadowTextureParamsList)
        {
            params.push_back(In(texp.mPSInLightPosition));
            params.push_back(In(texp.mTextureSampler));
            params.push_back(In(texp.mInvTextureSize).xy());
        }

        params.push_back(Out(mPSLocalShadowFactor));
        params.push_back(At(0));

        if(mDebug)
        {
            auto sceneCol = psProgram->resolveParameter(GpuProgramParameters::ACT_DERIVED_SCENE_COLOUR);
            params.push_back(InOut(sceneCol));
        }

        // Compute shadow factor.
        stage.callFunction("SGX_ComputeShadowFactor_PSSM3", params);
    }

    // shadow factor is applied by lighting stages
    return true;
}

//-----------------------------------------------------------------------
SubRenderState* IntegratedPSSM3Factory::createInstance(ScriptCompiler* compiler,
                                                      PropertyAbstractNode* prop, Pass* pass, SGScriptTranslator* translator)
{
    if (prop->name == "integrated_pssm4")
    {
        compiler->addError(ScriptCompiler::CE_DEPRECATEDSYMBOL, prop->file, prop->line, "integrated_pssm4. Use shadow_mapping instead.");

        SubRenderState* subRenderState = createOrRetrieveInstance(translator);

        auto it = prop->values.begin();
        auto itEnd = prop->values.end();

        if (prop->values.size() >= 4)
        {
            IntegratedPSSM3::SplitPointList splitPointList;
            if(SGScriptTranslator::getVector(it, itEnd, splitPointList, 4))
                subRenderState->setParameter("split_points", splitPointList);

            std::advance(it, 4);
        }

        for (; it != itEnd; ++it)
        {
            const auto& val = (*it)->getString();
            if(val == "debug")
            {
                subRenderState->setParameter("debug", "true");
            }
            else if(val == "pcf16")
            {
                subRenderState->setParameter("filter", "pcf16");
            }
        }

        return subRenderState;
    }

    if (prop->name == "shadow_mapping")
    {
        SubRenderState* subRenderState = createOrRetrieveInstance(translator);

        auto it = prop->values.begin();
        while(it != prop->values.end())
        {
            String paramName = (*it)->getString();
            String paramValue = (*++it)->getString();

            if (!subRenderState->setParameter(paramName, paramValue))
            {
                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line, paramName);
                return subRenderState;
            }
            it++;
        }

        return subRenderState;
    }

    return NULL;
}

//-----------------------------------------------------------------------
SubRenderState* IntegratedPSSM3Factory::createInstanceImpl()
{
    return OGRE_NEW IntegratedPSSM3;
}

}
}

#endif
