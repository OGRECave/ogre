/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#ifdef RTSHADER_SYSTEM_BUILD_CORE_SHADERS

namespace Ogre
{
namespace RTShader
{

/************************************************************************/
/*                                                                      */
/************************************************************************/
String GBuffer::Type = "GBuffer";

//-----------------------------------------------------------------------
const String& GBuffer::getType() const { return Type; }

//-----------------------------------------------------------------------
int GBuffer::getExecutionOrder() const { return FFP_LIGHTING; }

bool GBuffer::preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass)
{
    srcPass->getParent()->getParent()->setReceiveShadows(false);
    return true;
}

//-----------------------------------------------------------------------
bool GBuffer::createCpuSubPrograms(ProgramSet* programSet)
{
    Function* psMain = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM)->getMain();

    for(size_t i = 0; i < mOutBuffers.size(); i++)
    {
        auto out =
            psMain->resolveOutputParameter(i == 0 ? Parameter::SPC_COLOR_DIFFUSE : Parameter::SPC_COLOR_SPECULAR);

        switch(mOutBuffers[i])
        {
        case TL_DEPTH:
            addDepthInvocations(programSet, out);
            break;
        case TL_NORMAL_VIEWDEPTH:
            addViewPosInvocations(programSet, out, true);
            OGRE_FALLTHROUGH;
        case TL_NORMAL:
            addNormalInvocations(programSet, out);
            break;
        case TL_VIEWPOS:
            addViewPosInvocations(programSet, out, false);
            break;
        case TL_DIFFUSE_SPECULAR:
            addDiffuseSpecularInvocations(programSet, out);
            break;
        default:
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "unsupported TargetLayout");
        }
    }

    return true;
}

void GBuffer::addViewPosInvocations(ProgramSet* programSet, const ParameterPtr& out, bool depthOnly)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    Function* vsMain = vsProgram->getMain();
    Function* psMain = psProgram->getMain();

    // vertex shader
    auto vstage = vsMain->getStage(FFP_VS_POST_PROCESS);
    auto vsInPosition = vsMain->resolveInputParameter(Parameter::SPC_POSITION_OBJECT_SPACE);
    auto vsOutPos = vsMain->resolveOutputParameter(Parameter::SPC_POSITION_VIEW_SPACE);
    auto worldViewMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLDVIEW_MATRIX);
    vstage.callFunction(FFP_FUNC_TRANSFORM, worldViewMatrix, vsInPosition, vsOutPos);

    // fragment shader
    auto fstage = psMain->getStage(FFP_PS_COLOUR_END);
    auto viewPos = psMain->resolveInputParameter(vsOutPos);

    if(depthOnly)
    {
        auto far = psProgram->resolveParameter(GpuProgramParameters::ACT_FAR_CLIP_DISTANCE);
        fstage.callFunction("FFP_Length", viewPos, Out(out).w());
        fstage.div(In(out).w(), far, Out(out).w());
        return;
    }

    fstage.assign(viewPos, Out(out).xyz());
    fstage.assign(0, Out(out).w());
}

void GBuffer::addDepthInvocations(ProgramSet* programSet, const ParameterPtr& out)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    Function* vsMain = vsProgram->getMain();
    Function* psMain = psProgram->getMain();

    // vertex shader
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

    // fragment shader
    auto fstage = psMain->getStage(FFP_PS_COLOUR_END);
    auto viewPos = psMain->resolveInputParameter(vsOutPos);

    fstage.assign(In(viewPos).z(), Out(out).x());

    if (isD3D9)
    {
        fstage.div(In(out).x(), In(viewPos).w(), Out(out).x());
    }
}

void GBuffer::addNormalInvocations(ProgramSet* programSet, const ParameterPtr& out)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Function* vsMain = vsProgram->getMain();
    Function* psMain = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM)->getMain();

    auto fstage = psMain->getStage(FFP_PS_COLOUR_END);
    auto viewNormal = psMain->getLocalParameter(Parameter::SPC_NORMAL_VIEW_SPACE);
    if(!viewNormal)
    {
        // compute vertex shader normal
        auto vstage = vsMain->getStage(FFP_VS_LIGHTING);
        auto vsInNormal = vsMain->resolveInputParameter(Parameter::SPC_NORMAL_OBJECT_SPACE);
        auto vsOutNormal = vsMain->resolveOutputParameter(Parameter::SPC_NORMAL_VIEW_SPACE);
        auto worldViewITMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_NORMAL_MATRIX);
        vstage.callFunction(FFP_FUNC_TRANSFORM, worldViewITMatrix, vsInNormal, vsOutNormal);
        vstage.callFunction(FFP_FUNC_NORMALIZE, vsOutNormal);

        // pass through
        viewNormal = psMain->resolveInputParameter(vsOutNormal);
    }
    fstage.assign(viewNormal, Out(out).xyz());
}

void GBuffer::addDiffuseSpecularInvocations(ProgramSet* programSet, const ParameterPtr& out)
{
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    Function* psMain = psProgram->getMain();

    // set diffuse - TODO vertex color tracking
    auto diffuse = psProgram->resolveParameter(GpuProgramParameters::ACT_SURFACE_DIFFUSE_COLOUR);
    psMain->getStage(FFP_PS_COLOUR_BEGIN + 1).assign(diffuse, out);

    // set shininess
    auto surfaceShininess = psProgram->resolveParameter(GpuProgramParameters::ACT_SURFACE_SHININESS);
    psMain->getStage(FFP_PS_COLOUR_END).assign(surfaceShininess, Out(out).w());
}

//-----------------------------------------------------------------------
void GBuffer::copyFrom(const SubRenderState& rhs)
{
    const GBuffer& rhsTransform = static_cast<const GBuffer&>(rhs);
    mOutBuffers = rhsTransform.mOutBuffers;
}

//-----------------------------------------------------------------------
const String& GBufferFactory::getType() const { return GBuffer::Type; }

static GBuffer::TargetLayout translate(const String& val)
{
    if(val == "depth")
        return GBuffer::TL_DEPTH;
    if(val == "normal")
        return GBuffer::TL_NORMAL;
    if(val == "viewpos")
        return GBuffer::TL_VIEWPOS;
    if(val == "normal_viewdepth")
        return GBuffer::TL_NORMAL_VIEWDEPTH;
    return GBuffer::TL_DIFFUSE_SPECULAR;
}

//-----------------------------------------------------------------------
SubRenderState* GBufferFactory::createInstance(ScriptCompiler* compiler, PropertyAbstractNode* prop, Pass* pass,
                                               SGScriptTranslator* translator)
{
    if (prop->name != "lighting_stage" || prop->values.size() < 2)
        return NULL;

    auto it = prop->values.begin();
    String val;

    if(!SGScriptTranslator::getString(*it++, &val))
    {
        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
        return NULL;
    }

    if (val != "gbuffer")
        return NULL;

    GBuffer::TargetBuffers targets;

    if(!SGScriptTranslator::getString(*it++, &val))
    {
        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
        return NULL;
    }
    targets.push_back(translate(val));

    if(it != prop->values.end())
    {
        if(!SGScriptTranslator::getString(*it++, &val))
        {
            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
            return NULL;
        }

        targets.push_back(translate(val));
    }

    auto ret = static_cast<GBuffer*>(createOrRetrieveInstance(translator));
    ret->setOutBuffers(targets);
    return ret;
}

//-----------------------------------------------------------------------
void GBufferFactory::writeInstance(MaterialSerializer* ser, SubRenderState* subRenderState, Pass* srcPass,
                                   Pass* dstPass)
{
    ser->writeAttribute(4, "lighting_stage");
    ser->writeValue("gbuffer");
    ser->writeValue("depth");
}

//-----------------------------------------------------------------------
SubRenderState* GBufferFactory::createInstanceImpl() { return OGRE_NEW GBuffer; }

} // namespace RTShader
} // namespace Ogre

#endif
