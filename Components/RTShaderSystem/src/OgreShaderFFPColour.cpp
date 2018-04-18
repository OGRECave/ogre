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
    mResolveStageFlags  = SF_PS_OUTPUT_DIFFUSE;
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
    bool hasError = false;

    if (mResolveStageFlags & SF_VS_INPUT_DIFFUSE)
        mVSInputDiffuse  = vsMain->resolveInputParameter(Parameter::SPS_COLOR, 0, Parameter::SPC_COLOR_DIFFUSE, GCT_FLOAT4);
    
    if (mResolveStageFlags & SF_VS_INPUT_SPECULAR)
        mVSInputSpecular = vsMain->resolveInputParameter(Parameter::SPS_COLOR, 1, Parameter::SPC_COLOR_SPECULAR, GCT_FLOAT4);
    
    // Resolve VS color outputs if have inputs from vertex stream.
    if (mVSInputDiffuse.get() != NULL || mResolveStageFlags & SF_VS_OUTPUT_DIFFUSE)     
        mVSOutputDiffuse = vsMain->resolveOutputParameter(Parameter::SPS_COLOR, 0, Parameter::SPC_COLOR_DIFFUSE, GCT_FLOAT4);                               

    if (mVSInputSpecular.get() != NULL || mResolveStageFlags & SF_VS_OUTPUT_SPECULAR)       
        mVSOutputSpecular = vsMain->resolveOutputParameter(Parameter::SPS_COLOR, 1, Parameter::SPC_COLOR_SPECULAR, GCT_FLOAT4);         

    // Resolve PS color inputs if have inputs from vertex shader.
    if (mVSOutputDiffuse.get() != NULL || mResolveStageFlags & SF_PS_INPUT_DIFFUSE)     
        mPSInputDiffuse = psMain->resolveInputParameter(Parameter::SPS_COLOR, 0, Parameter::SPC_COLOR_DIFFUSE, GCT_FLOAT4);

    if (mVSOutputSpecular.get() != NULL || mResolveStageFlags & SF_PS_INPUT_SPECULAR)       
        mPSInputSpecular = psMain->resolveInputParameter(Parameter::SPS_COLOR, 1, Parameter::SPC_COLOR_SPECULAR, GCT_FLOAT4);


    // Resolve PS output diffuse color.
    if (mResolveStageFlags & SF_PS_OUTPUT_DIFFUSE)
    {
        mPSOutputDiffuse = psMain->resolveOutputParameter(Parameter::SPS_COLOR, 0, Parameter::SPC_COLOR_DIFFUSE, GCT_FLOAT4);
        hasError |= !(mPSOutputDiffuse.get());
    }

    // Resolve PS output specular color.
    if (mResolveStageFlags & SF_PS_OUTPUT_SPECULAR)
    {
        mPSOutputSpecular = psMain->resolveOutputParameter(Parameter::SPS_COLOR, 1, Parameter::SPC_COLOR_SPECULAR, GCT_FLOAT4);
        hasError |= !(mPSOutputSpecular.get());
    }
    
    
    if (hasError)
    {
        OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, 
                "Not all parameters could be constructed for the sub-render state.",
                "FFPColour::resolveParameters" );
    }
    return true;
}


//-----------------------------------------------------------------------
bool FFPColour::resolveDependencies(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);

    vsProgram->addDependency(FFP_LIB_COMMON);
    psProgram->addDependency(FFP_LIB_COMMON);

    return true;
}

//-----------------------------------------------------------------------
bool FFPColour::addFunctionInvocations(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    Function* vsMain   = vsProgram->getEntryPointFunction();
    Function* psMain   = psProgram->getEntryPointFunction();    
    FunctionInvocation* curFuncInvocation = NULL;   

    
    // Create vertex shader colour invocations.
    ParameterPtr vsDiffuse;
    ParameterPtr vsSpecular;
    if (mVSInputDiffuse)
    {
        vsDiffuse = mVSInputDiffuse;
    }
    else
    {
        vsDiffuse = vsMain->resolveLocalParameter(Parameter::SPS_COLOR, 0, Parameter::SPC_COLOR_DIFFUSE, GCT_FLOAT4);
        vsMain->addAtomAssign(vsDiffuse, ParameterFactory::createConstParam(Vector4(1.0)), FFP_VS_COLOUR);
    }

    if (mVSOutputDiffuse)
    {
        vsMain->addAtomAssign(mVSOutputDiffuse, vsDiffuse, FFP_VS_COLOUR);
    }
    
    if (mVSInputSpecular)
    {
        vsSpecular = mVSInputSpecular;      
    }
    else
    {
        vsSpecular = vsMain->resolveLocalParameter(Parameter::SPS_COLOR, 1, Parameter::SPC_COLOR_SPECULAR, GCT_FLOAT4);
        vsMain->addAtomAssign(vsSpecular, ParameterFactory::createConstParam(Vector4::ZERO), FFP_VS_COLOUR);
    }

    if (mVSOutputSpecular)
    {
        vsMain->addAtomAssign(mVSOutputSpecular, vsSpecular, FFP_VS_COLOUR);
    }
    
    

    // Create fragment shader colour invocations.
    ParameterPtr psDiffuse;
    ParameterPtr psSpecular;
    
    // Handle diffuse colour.
    if (mPSInputDiffuse.get() != NULL)
    {
        psDiffuse = mPSInputDiffuse;                
    }
    else
    {
        psDiffuse = psMain->resolveLocalParameter(Parameter::SPS_COLOR, 0, Parameter::SPC_COLOR_DIFFUSE, GCT_FLOAT4);
        psMain->addAtomAssign(psDiffuse, ParameterFactory::createConstParam(Vector4(1.0)), FFP_PS_COLOUR_BEGIN);
    }

    // Handle specular colour.
    if (mPSInputSpecular)
    {
        psSpecular = mPSInputSpecular;      
    }
    else
    {
        psSpecular = psMain->resolveLocalParameter(Parameter::SPS_COLOR, 1, Parameter::SPC_COLOR_SPECULAR, GCT_FLOAT4);
        psMain->addAtomAssign(psSpecular, ParameterFactory::createConstParam(Vector4::ZERO), FFP_PS_COLOUR_BEGIN);
    }

    // Assign diffuse colour.
    if (mPSOutputDiffuse)
    {   
        psMain->addAtomAssign(mPSOutputDiffuse, psDiffuse, FFP_PS_COLOUR_BEGIN);
    }

    // Assign specular colour.
    if (mPSOutputSpecular)
    {
        psMain->addAtomAssign(mPSOutputSpecular, psSpecular, FFP_PS_COLOUR_BEGIN);
    }

    // Add specular to out colour.
    if (mPSOutputDiffuse.get() != NULL && psSpecular.get() != NULL)
    {
        curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ADD, FFP_PS_COLOUR_END);
        curFuncInvocation->pushOperand(mPSOutputDiffuse, Operand::OPS_IN, Operand::OPM_XYZ);
        curFuncInvocation->pushOperand(psSpecular, Operand::OPS_IN, Operand::OPM_XYZ);
        curFuncInvocation->pushOperand(mPSOutputDiffuse, Operand::OPS_OUT, Operand::OPM_XYZ);
        psMain->addAtomInstance(curFuncInvocation);
    }   

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
