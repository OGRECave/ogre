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
const String SRS_WBOIT = "WBOIT";

//-----------------------------------------------------------------------
const String& WBOIT::getType() const { return SRS_WBOIT; }

//-----------------------------------------------------------------------
int WBOIT::getExecutionOrder() const { return FFP_POST_PROCESS; }

bool WBOIT::preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass)
{
    dstPass->setTransparentSortingEnabled(false);
    dstPass->setSeparateSceneBlending(SBF_ONE, SBF_ONE, SBF_ZERO, SBF_ONE_MINUS_SOURCE_ALPHA);
    return true;
}

bool WBOIT::createCpuSubPrograms(ProgramSet* programSet)
{
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    psProgram->addDependency("SGXLib_WBOIT");

    Function* vsMain = programSet->getCpuProgram(GPT_VERTEX_PROGRAM)->getMain();
    Function* psMain = psProgram->getMain();

    auto vsOutPos = vsMain->resolveOutputParameter(Parameter::SPC_POSITION_PROJECTIVE_SPACE);

    bool isD3D9 = ShaderGenerator::getSingleton().getTargetLanguage() == "hlsl" &&
                  !GpuProgramManager::getSingleton().isSyntaxSupported("vs_4_0_level_9_1");

    if (isD3D9)
    {
        auto vstage = vsMain->getStage(FFP_VS_POST_PROCESS);
        auto vsPos = vsMain->resolveOutputParameter(Parameter::SPC_UNKNOWN, GCT_FLOAT4);
        vstage.assign(vsOutPos, vsPos);
        std::swap(vsOutPos, vsPos);
    }

    auto viewPos = psMain->resolveInputParameter(vsOutPos);

    auto accum = psMain->resolveOutputParameter(Parameter::SPC_COLOR_DIFFUSE);
    auto revealage = psMain->resolveOutputParameter(Parameter::SPC_COLOR_SPECULAR);

    auto stage = psMain->getStage(FFP_PS_POST_PROCESS);

    if (isD3D9)
    {
        stage.div(viewPos, In(viewPos).w(), viewPos);
    }

    stage.callFunction("SGX_WBOIT", {In(viewPos).z(), InOut(accum), Out(revealage)});

    return true;
}

//-----------------------------------------------------------------------
const String& WBOITFactory::getType() const { return SRS_WBOIT; }

//-----------------------------------------------------------------------
SubRenderState* WBOITFactory::createInstance(const ScriptProperty& prop, Pass* pass, SGScriptTranslator* translator)
{
    if (prop.name != "weighted_blended_oit")
        return NULL;

    bool val;
    if(!StringConverter::parse(prop.values[0], val) || !val)
    {
        return NULL;
    }

    auto ret = static_cast<WBOIT*>(createOrRetrieveInstance(translator));
    return ret;
}

//-----------------------------------------------------------------------
void WBOITFactory::writeInstance(MaterialSerializer* ser, SubRenderState* subRenderState, Pass* srcPass,
                                   Pass* dstPass)
{
    ser->writeAttribute(4, "weighted_blended_oit");
    ser->writeValue("true");
}

//-----------------------------------------------------------------------
SubRenderState* WBOITFactory::createInstanceImpl() { return OGRE_NEW WBOIT; }

} // namespace RTShader
} // namespace Ogre

#endif
