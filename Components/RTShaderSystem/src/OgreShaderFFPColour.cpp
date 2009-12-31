/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2009 Torus Knot Software Ltd
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
#ifdef RTSHADER_SYSTEM_BUILD_CORE_SHADERS
#include "OgreShaderFFPColour.h"
#include "OgreShaderFFPRenderState.h"
#include "OgreShaderProgram.h"
#include "OgreShaderParameter.h"
#include "OgreShaderProgramSet.h"

namespace Ogre {
namespace RTShader {

/************************************************************************/
/*                                                                      */
/************************************************************************/
String FFPColour::Type = "FFP_Colour";

//-----------------------------------------------------------------------
FFPColour::FFPColour()
{
	mResolveStageFlags	= SF_PS_OUTPUT_DIFFUSE;
}

//-----------------------------------------------------------------------
const String& FFPColour::getType() const
{
	return Type;
}


//-----------------------------------------------------------------------
int	FFPColour::getExecutionOrder() const
{
	return FFP_COLOUR;
}

//-----------------------------------------------------------------------
bool FFPColour::resolveParameters(ProgramSet* programSet)
{
	Program* vsProgram = programSet->getCpuVertexProgram();
	Program* psProgram = programSet->getCpuFragmentProgram();
	Function* vsMain   = vsProgram->getEntryPointFunction();
	Function* psMain   = psProgram->getEntryPointFunction();	

	const ShaderParameterList& vsInputParams = vsMain->getInputParameters();
	const ShaderParameterList& psInputParams = psMain->getInputParameters();

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
		if (mPSOutputDiffuse.get() == NULL)
			return false;
	}

	// Resolve PS output specular color.
	if (mResolveStageFlags & SF_PS_OUTPUT_SPECULAR)
	{
		mPSOutputSpecular = psMain->resolveOutputParameter(Parameter::SPS_COLOR, 1, Parameter::SPC_COLOR_SPECULAR, GCT_FLOAT4);
		if (mPSOutputSpecular.get() == NULL)
			return false;
	}
	


	return true;
}


//-----------------------------------------------------------------------
bool FFPColour::resolveDependencies(ProgramSet* programSet)
{
	Program* vsProgram = programSet->getCpuVertexProgram();
	Program* psProgram = programSet->getCpuFragmentProgram();

	vsProgram->addDependency(FFP_LIB_COMMON);
	psProgram->addDependency(FFP_LIB_COMMON);

	return true;
}

//-----------------------------------------------------------------------
bool FFPColour::addFunctionInvocations(ProgramSet* programSet)
{
	Program* vsProgram = programSet->getCpuVertexProgram();
	Program* psProgram = programSet->getCpuFragmentProgram();
	Function* vsMain   = vsProgram->getEntryPointFunction();
	Function* psMain   = psProgram->getEntryPointFunction();	
	FunctionInvocation* curFuncInvocation = NULL;	
	int internalCounter;

	
	// Create vertex shader colour invocations.
	ParameterPtr vsDiffuse;
	ParameterPtr vsSpecular;
	internalCounter = 0;	
	if (mVSInputDiffuse.get() != NULL)
	{
		vsDiffuse = mVSInputDiffuse;
	}
	else
	{
		vsDiffuse = vsMain->resolveLocalParameter(Parameter::SPS_COLOR, 0, Parameter::SPC_COLOR_DIFFUSE, GCT_FLOAT4);
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_CONSTRUCT, FFP_VS_COLOUR, internalCounter++);
		curFuncInvocation->pushOperand(ParameterFactory::createConstParamFloat(1.0), Operand::OPS_IN);
		curFuncInvocation->pushOperand(ParameterFactory::createConstParamFloat(1.0), Operand::OPS_IN);
		curFuncInvocation->pushOperand(ParameterFactory::createConstParamFloat(1.0), Operand::OPS_IN);
		curFuncInvocation->pushOperand(ParameterFactory::createConstParamFloat(1.0), Operand::OPS_IN);
		curFuncInvocation->pushOperand(vsDiffuse, Operand::OPS_OUT);
		vsMain->addAtomInstace(curFuncInvocation);
	}

	if (mVSOutputDiffuse.get() != NULL)
	{
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ASSIGN, FFP_VS_COLOUR, internalCounter++);
		curFuncInvocation->pushOperand(vsDiffuse, Operand::OPS_IN);
		curFuncInvocation->pushOperand(mVSOutputDiffuse, Operand::OPS_OUT);
		vsMain->addAtomInstace(curFuncInvocation);
	}
	
	if (mVSInputSpecular.get() != NULL)
	{
		vsSpecular = mVSInputSpecular;		
	}
	else
	{
		vsSpecular = vsMain->resolveLocalParameter(Parameter::SPS_COLOR, 1, Parameter::SPC_COLOR_SPECULAR, GCT_FLOAT4);

		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_CONSTRUCT, FFP_VS_COLOUR, internalCounter++);
		curFuncInvocation->pushOperand(ParameterFactory::createConstParamFloat(0.0), Operand::OPS_IN);
		curFuncInvocation->pushOperand(ParameterFactory::createConstParamFloat(0.0), Operand::OPS_IN);
		curFuncInvocation->pushOperand(ParameterFactory::createConstParamFloat(0.0), Operand::OPS_IN);
		curFuncInvocation->pushOperand(ParameterFactory::createConstParamFloat(0.0), Operand::OPS_IN);
		curFuncInvocation->pushOperand(vsSpecular, Operand::OPS_OUT);
		vsMain->addAtomInstace(curFuncInvocation);
	}

	if (mVSOutputSpecular.get() != NULL)
	{
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ASSIGN, FFP_VS_COLOUR, internalCounter++);
		curFuncInvocation->pushOperand(vsSpecular, Operand::OPS_IN);
		curFuncInvocation->pushOperand(mVSOutputSpecular, Operand::OPS_OUT);
		vsMain->addAtomInstace(curFuncInvocation);
	}
	
	

	// Create fragment shader colour invocations.
	ParameterPtr psDiffuse;
	ParameterPtr psSpecular;
	internalCounter = 0;
	
	// Handle diffuse colour.
	if (mPSInputDiffuse.get() != NULL)
	{
		psDiffuse = mPSInputDiffuse;				
	}
	else
	{
		psDiffuse = psMain->resolveLocalParameter(Parameter::SPS_COLOR, 0, Parameter::SPC_COLOR_DIFFUSE, GCT_FLOAT4);
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_CONSTRUCT, FFP_PS_COLOUR_BEGIN, internalCounter++);
		curFuncInvocation->pushOperand(ParameterFactory::createConstParamFloat(1.0), Operand::OPS_IN);
		curFuncInvocation->pushOperand(ParameterFactory::createConstParamFloat(1.0), Operand::OPS_IN);
		curFuncInvocation->pushOperand(ParameterFactory::createConstParamFloat(1.0), Operand::OPS_IN);
		curFuncInvocation->pushOperand(ParameterFactory::createConstParamFloat(1.0), Operand::OPS_IN);
		curFuncInvocation->pushOperand(psDiffuse, Operand::OPS_OUT);
		psMain->addAtomInstace(curFuncInvocation);
	}

	// Handle specular colour.
	if (mPSInputSpecular.get() != NULL)
	{
		psSpecular = mPSInputSpecular;		
	}
	else
	{
		psSpecular = psMain->resolveLocalParameter(Parameter::SPS_COLOR, 1, Parameter::SPC_COLOR_SPECULAR, GCT_FLOAT4);
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_CONSTRUCT, FFP_PS_COLOUR_BEGIN, internalCounter++);
		curFuncInvocation->pushOperand(ParameterFactory::createConstParamFloat(0.0), Operand::OPS_IN);
		curFuncInvocation->pushOperand(ParameterFactory::createConstParamFloat(0.0), Operand::OPS_IN);
		curFuncInvocation->pushOperand(ParameterFactory::createConstParamFloat(0.0), Operand::OPS_IN);
		curFuncInvocation->pushOperand(ParameterFactory::createConstParamFloat(0.0), Operand::OPS_IN);
		curFuncInvocation->pushOperand(psSpecular, Operand::OPS_OUT);
		psMain->addAtomInstace(curFuncInvocation);
	}

	// Assign diffuse colour.
	if (mPSOutputDiffuse.get() != NULL)
	{	
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ASSIGN, FFP_PS_COLOUR_BEGIN, internalCounter++);
		curFuncInvocation->pushOperand(psDiffuse, Operand::OPS_IN);
		curFuncInvocation->pushOperand(mPSOutputDiffuse, Operand::OPS_OUT);		
		psMain->addAtomInstace(curFuncInvocation);
	}

	// Assign specular colour.
	if (mPSOutputSpecular.get() != NULL)
	{
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ASSIGN, FFP_PS_COLOUR_BEGIN, internalCounter++);
		curFuncInvocation->pushOperand(psSpecular, Operand::OPS_IN);
		curFuncInvocation->pushOperand(mPSOutputSpecular, Operand::OPS_OUT);		
		psMain->addAtomInstace(curFuncInvocation);
	}

	// Add specular to out colour.
	internalCounter = 0;
	if (mPSOutputDiffuse.get() != NULL && psSpecular.get() != NULL)
	{
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ADD, FFP_PS_COLOUR_END, internalCounter++);
		curFuncInvocation->pushOperand(mPSOutputDiffuse, Operand::OPS_IN,(Operand::OPM_X | Operand::OPM_Y | Operand::OPM_Z));
		curFuncInvocation->pushOperand(psSpecular, Operand::OPS_IN,(Operand::OPM_X | Operand::OPM_Y | Operand::OPM_Z));
		curFuncInvocation->pushOperand(mPSOutputDiffuse, Operand::OPS_OUT,(Operand::OPM_X | Operand::OPM_Y | Operand::OPM_Z));
		psMain->addAtomInstace(curFuncInvocation);
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
bool FFPColour::preAddToRenderState(RenderState* renderState, Pass* srcPass, Pass* dstPass)
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
SubRenderState*	FFPColourFactory::createInstance(ScriptCompiler* compiler, 
													PropertyAbstractNode* prop, Pass* pass)
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
				return SubRenderStateFactory::createInstance();
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
SubRenderState*	FFPColourFactory::createInstanceImpl()
{
	return OGRE_NEW FFPColour;
}


}
}

#endif