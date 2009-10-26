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
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_CONSTRUCT, FFP_VS_COLOUR, internalCounter++);
		curFuncInvocation->getParameterList().push_back("1.0");
		curFuncInvocation->getParameterList().push_back("1.0");
		curFuncInvocation->getParameterList().push_back("1.0");
		curFuncInvocation->getParameterList().push_back("1.0");
		curFuncInvocation->getParameterList().push_back(vsDiffuse->getName());
		vsMain->addAtomInstace(curFuncInvocation);
	}

	if (mVSOutputDiffuse.get() != NULL)
	{
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_ASSIGN, FFP_VS_COLOUR, internalCounter++);
		curFuncInvocation->getParameterList().push_back(vsDiffuse->getName());
		curFuncInvocation->getParameterList().push_back(mVSOutputDiffuse->getName());
		vsMain->addAtomInstace(curFuncInvocation);
	}
	
	if (mVSInputSpecular.get() != NULL)
	{
		vsSpecular = mVSInputSpecular;		
	}
	else
	{
		vsSpecular = vsMain->resolveLocalParameter(Parameter::SPS_COLOR, 1, Parameter::SPC_COLOR_SPECULAR, GCT_FLOAT4);

		curFuncInvocation = new FunctionInvocation(FFP_FUNC_CONSTRUCT, FFP_VS_COLOUR, internalCounter++);
		curFuncInvocation->getParameterList().push_back("0.0");
		curFuncInvocation->getParameterList().push_back("0.0");
		curFuncInvocation->getParameterList().push_back("0.0");
		curFuncInvocation->getParameterList().push_back("0.0");
		curFuncInvocation->getParameterList().push_back(vsSpecular->getName());
		vsMain->addAtomInstace(curFuncInvocation);
	}

	if (mVSOutputSpecular.get() != NULL)
	{
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_ASSIGN, FFP_VS_COLOUR, internalCounter++);
		curFuncInvocation->getParameterList().push_back(vsSpecular->getName());
		curFuncInvocation->getParameterList().push_back(mVSOutputSpecular->getName());
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
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_CONSTRUCT, FFP_PS_COLOUR_BEGIN, internalCounter++);
		curFuncInvocation->getParameterList().push_back("1.0");
		curFuncInvocation->getParameterList().push_back("1.0");
		curFuncInvocation->getParameterList().push_back("1.0");
		curFuncInvocation->getParameterList().push_back("1.0");
		curFuncInvocation->getParameterList().push_back(psDiffuse->getName());
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
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_CONSTRUCT, FFP_PS_COLOUR_BEGIN, internalCounter++);
		curFuncInvocation->getParameterList().push_back("0.0");
		curFuncInvocation->getParameterList().push_back("0.0");
		curFuncInvocation->getParameterList().push_back("0.0");
		curFuncInvocation->getParameterList().push_back("0.0");
		curFuncInvocation->getParameterList().push_back(psSpecular->getName());
		psMain->addAtomInstace(curFuncInvocation);
	}

	// Assign diffuse colour.
	if (mPSOutputDiffuse.get() != NULL)
	{	
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_ASSIGN, FFP_PS_COLOUR_BEGIN, internalCounter++);
		curFuncInvocation->getParameterList().push_back(psDiffuse->getName());
		curFuncInvocation->getParameterList().push_back(mPSOutputDiffuse->getName());		
		psMain->addAtomInstace(curFuncInvocation);
	}

	// Assign specular colour.
	if (mPSOutputSpecular.get() != NULL)
	{
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_ASSIGN, FFP_PS_COLOUR_BEGIN, internalCounter++);
		curFuncInvocation->getParameterList().push_back(psSpecular->getName());
		curFuncInvocation->getParameterList().push_back(mPSOutputSpecular->getName());		
		psMain->addAtomInstace(curFuncInvocation);
	}

	// Add specular to out colour.
	internalCounter = 0;
	if (mPSOutputDiffuse.get() != NULL && psSpecular.get() != NULL)
	{
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_ADD, FFP_PS_COLOUR_END, internalCounter++);
		curFuncInvocation->getParameterList().push_back(mPSOutputDiffuse->getName() + ".xyz");
		curFuncInvocation->getParameterList().push_back(psSpecular->getName() + ".xyz");
		curFuncInvocation->getParameterList().push_back(mPSOutputDiffuse->getName() + ".xyz");		
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
uint32 FFPColour::getHashCode()
{	
	uint32 hashCode   = 0;
				
	sh_hash_combine(hashCode, SubRenderState::getHashCode());
	sh_hash_combine(hashCode, mResolveStageFlags);
	
	return hashCode;
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
SubRenderState*	FFPColourFactory::createInstanceImpl()
{
	return new FFPColour;
}


}
}

