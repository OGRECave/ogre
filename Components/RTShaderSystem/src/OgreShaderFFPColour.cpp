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

namespace Ogre {
namespace RTShader {

/************************************************************************/
/*                                                                      */
/************************************************************************/
String FFPColour::Type = "FFP_Colour";

//-----------------------------------------------------------------------
FFPColour::FFPColour()
{
    mResolveStageFlags  = 0;
}

//-----------------------------------------------------------------------
const String& FFPColour::getType() const
{
    return Type;
}


//-----------------------------------------------------------------------
int FFPColour::getExecutionOrder() const
{
    return FFP_COLOUR;
}

//-----------------------------------------------------------------------
bool FFPColour::resolveParameters(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    Function* vsMain   = vsProgram->getEntryPointFunction();
    Function* psMain   = psProgram->getEntryPointFunction();    

    if (mResolveStageFlags & SF_VS_INPUT_DIFFUSE)
        mVSInputDiffuse  = vsMain->resolveInputParameter(Parameter::SPC_COLOR_DIFFUSE);

    // Resolve VS color outputs if have inputs from vertex stream.
    if (mVSInputDiffuse.get() != NULL || mResolveStageFlags & SF_VS_OUTPUT_DIFFUSE)
        mVSOutputDiffuse = vsMain->resolveOutputParameter(Parameter::SPC_COLOR_DIFFUSE);

    if (mResolveStageFlags & SF_VS_OUTPUT_SPECULAR)
        mVSOutputSpecular = vsMain->resolveOutputParameter(Parameter::SPC_COLOR_SPECULAR);

    // Resolve PS color inputs if have inputs from vertex shader.
    if (mVSOutputDiffuse.get() != NULL || mResolveStageFlags & SF_PS_INPUT_DIFFUSE)
        mPSInputDiffuse = psMain->resolveInputParameter(Parameter::SPC_COLOR_DIFFUSE);

    if (mVSOutputSpecular.get() != NULL || mResolveStageFlags & SF_PS_INPUT_SPECULAR)
        mPSInputSpecular = psMain->resolveInputParameter(Parameter::SPC_COLOR_SPECULAR);


    // Resolve PS output diffuse color.
    mPSOutputDiffuse = psMain->resolveOutputParameter(Parameter::SPC_COLOR_DIFFUSE);

    return true;
}


//-----------------------------------------------------------------------
bool FFPColour::resolveDependencies(ProgramSet* programSet)
{
    return true;
}

//-----------------------------------------------------------------------
bool FFPColour::addFunctionInvocations(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    Function* vsMain   = vsProgram->getEntryPointFunction();
    Function* psMain   = psProgram->getEntryPointFunction();    
    
    // Create vertex shader colour invocations.
    ParameterPtr vsDiffuse;
    ParameterPtr vsSpecular;

    auto vsStage = vsMain->getStage(FFP_VS_COLOUR);
    if (mVSInputDiffuse)
    {
        vsDiffuse = mVSInputDiffuse;
    }
    else
    {
        vsDiffuse = vsMain->resolveLocalParameter(Parameter::SPC_COLOR_DIFFUSE);
        vsStage.assign(Vector4(1.0), vsDiffuse);
    }

    if (mVSOutputDiffuse)
    {
        vsStage.assign(vsDiffuse, mVSOutputDiffuse);
    }
    
    vsSpecular = vsMain->resolveLocalParameter(Parameter::SPC_COLOR_SPECULAR);
    vsStage.assign(Vector4::ZERO, vsSpecular);

    if (mVSOutputSpecular)
    {
        vsStage.assign(vsSpecular, mVSOutputSpecular);
    }



    // Create fragment shader colour invocations.
    ParameterPtr psDiffuse;
    ParameterPtr psSpecular;
    auto psStage = psMain->getStage(FFP_PS_COLOUR_BEGIN);

    // Handle diffuse colour.
    if (mPSInputDiffuse.get() != NULL)
    {
        psDiffuse = mPSInputDiffuse;                
    }
    else
    {
        psDiffuse = psMain->resolveLocalParameter(Parameter::SPC_COLOR_DIFFUSE);
        psStage.assign(Vector4(1.0), psDiffuse);
    }

    // Handle specular colour.
    if (mPSInputSpecular)
    {
        psSpecular = mPSInputSpecular;
    }
    else
    {
        psSpecular = psMain->resolveLocalParameter(Parameter::SPC_COLOR_SPECULAR);
        psStage.assign(Vector4::ZERO, psSpecular);
    }

    // Assign diffuse colour.
    psStage.assign(psDiffuse, mPSOutputDiffuse);

    // Add specular to out colour.
    psMain->getStage(FFP_PS_COLOUR_END)
        .add(In(mPSOutputDiffuse).xyz(), In(psSpecular).xyz(), Out(mPSOutputDiffuse).xyz());

    return true;
}


//-----------------------------------------------------------------------
void FFPColour::copyFrom(const SubRenderState& rhs)
{
    const FFPColour& rhsColour = static_cast<const FFPColour&>(rhs);

    setResolveStageFlags(rhsColour.mResolveStageFlags);
}

//-----------------------------------------------------------------------
bool FFPColour::preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass)
{
    TrackVertexColourType trackColour = srcPass->getVertexColourTracking();

    if (trackColour != 0)
        addResolveStageMask(FFPColour::SF_VS_INPUT_DIFFUSE);

    return true;
}

//-----------------------------------------------------------------------
const String& FFPColourFactory::getType() const
{
    return FFPColour::Type;
}

//-----------------------------------------------------------------------
SubRenderState* FFPColourFactory::createInstance(ScriptCompiler* compiler, 
                                                    PropertyAbstractNode* prop, Pass* pass, SGScriptTranslator* translator)
{
    if (prop->name == "colour_stage")
    {
        if(prop->values.size() == 1)
        {
            String modelType;

            if(false == SGScriptTranslator::getString(prop->values.front(), &modelType))
            {
                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                return NULL;
            }

            if (modelType == "ffp")
            {
                return createOrRetrieveInstance(translator);
            }
        }       
    }

    return NULL;
}

//-----------------------------------------------------------------------
void FFPColourFactory::writeInstance(MaterialSerializer* ser, SubRenderState* subRenderState, 
                                        Pass* srcPass, Pass* dstPass)
{
    ser->writeAttribute(4, "colour_stage");
    ser->writeValue("ffp");
}

//-----------------------------------------------------------------------
SubRenderState* FFPColourFactory::createInstanceImpl()
{
    return OGRE_NEW FFPColour;
}


}
}

#endif
