/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#include <boost/functional/hash/hash.hpp>
#include "OgreShaderFFPColour.h"
#include "OgreShaderFFPRenderState.h"
#include "OgreShaderProgram.h"
#include "OgreShaderParameter.h"
#include "OgreShaderProgramSet.h"

namespace Ogre {
namespace CRTShader {

/************************************************************************/
/*                                                                      */
/************************************************************************/
String FFPColour::Type = "FFP_Colour";

//-----------------------------------------------------------------------
FFPColour::FFPColour()
{
	mVSInputDiffuse		= NULL;
	mVSInputSpecular	= NULL;
	mVSOutputDiffuse	= NULL;
	mVSOutputSpecular	= NULL;
	mPSInputDiffuse		= NULL;
	mPSInputSpecular	= NULL;
	mPSOutputDiffuse	= NULL;
	mPSOutputSpecular	= NULL;
	mResolveStageFlags	= SF_PS_OUTPUT_DIFFUSE;
}

//-----------------------------------------------------------------------
FFPColour::~FFPColour()
{

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
		mVSInputDiffuse  = vsMain->resolveInputParameter(Parameter::SPS_COLOR, 0, GCT_FLOAT4);
	
	if (mResolveStageFlags & SF_VS_INPUT_SPECULAR)
		mVSInputSpecular = vsMain->resolveInputParameter(Parameter::SPS_COLOR, 1, GCT_FLOAT4);
	
	// Resolve VS color outputs if have inputs from vertex stream.
	if (mVSInputDiffuse != NULL || mResolveStageFlags & SF_VS_OUTPUT_DIFFUSE)		
		mVSOutputDiffuse = vsMain->resolveOutputParameter(Parameter::SPS_COLOR, 0, GCT_FLOAT4);								

	if (mVSInputSpecular != NULL || mResolveStageFlags & SF_VS_OUTPUT_SPECULAR)		
		mVSOutputSpecular = vsMain->resolveOutputParameter(Parameter::SPS_COLOR, 1, GCT_FLOAT4);			

	// Resolve PS color inputs if have inputs from vertex shader.
	if (mVSOutputDiffuse != NULL || mResolveStageFlags & SF_PS_INPUT_DIFFUSE)		
		mPSInputDiffuse = psMain->resolveInputParameter(Parameter::SPS_COLOR, 0, GCT_FLOAT4);

	if (mVSOutputSpecular != NULL || mResolveStageFlags & SF_PS_INPUT_SPECULAR)		
		mPSInputSpecular = psMain->resolveInputParameter(Parameter::SPS_COLOR, 1, GCT_FLOAT4);


	// Resolve PS output diffuse color.
	if (mResolveStageFlags & SF_PS_OUTPUT_DIFFUSE)
	{
		mPSOutputDiffuse = psMain->resolveOutputParameter(Parameter::SPS_COLOR, 0, GCT_FLOAT4);
		if (mPSOutputDiffuse == NULL)
			return false;
	}

	// Resolve PS output specular color.
	if (mResolveStageFlags & SF_PS_OUTPUT_SPECULAR)
	{
		mPSOutputSpecular = psMain->resolveOutputParameter(Parameter::SPS_COLOR, 1, GCT_FLOAT4);
		if (mPSOutputSpecular == NULL)
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
	Parameter* vsDiffuse  = NULL;
	Parameter* vsSpecular = NULL;
	internalCounter = 0;	
	if (mVSInputDiffuse != NULL)
	{
		vsDiffuse = mVSInputDiffuse;
	}
	else
	{
		vsDiffuse = vsMain->resolveLocalParameter(Parameter::SPS_COLOR, 0, GCT_FLOAT4, "lDiffuse");
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_CONSTRUCT, FFP_VS_COLOUR, internalCounter++);
		curFuncInvocation->getParameterList().push_back("1.0");
		curFuncInvocation->getParameterList().push_back("1.0");
		curFuncInvocation->getParameterList().push_back("1.0");
		curFuncInvocation->getParameterList().push_back("1.0");
		curFuncInvocation->getParameterList().push_back(vsDiffuse->getName());
		vsMain->addAtomInstace(curFuncInvocation);
	}

	if (mVSOutputDiffuse != NULL)
	{
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_ASSIGN, FFP_VS_COLOUR, internalCounter++);
		curFuncInvocation->getParameterList().push_back(vsDiffuse->getName());
		curFuncInvocation->getParameterList().push_back(mVSOutputDiffuse->getName());
		vsMain->addAtomInstace(curFuncInvocation);
	}
	
	if (mVSInputSpecular != NULL)
	{
		vsSpecular = mVSInputSpecular;		
	}
	else
	{
		vsSpecular = vsMain->resolveLocalParameter(Parameter::SPS_COLOR, 1, GCT_FLOAT4, "lSpecular");

		curFuncInvocation = new FunctionInvocation(FFP_FUNC_CONSTRUCT, FFP_VS_COLOUR, internalCounter++);
		curFuncInvocation->getParameterList().push_back("0.0");
		curFuncInvocation->getParameterList().push_back("0.0");
		curFuncInvocation->getParameterList().push_back("0.0");
		curFuncInvocation->getParameterList().push_back("0.0");
		curFuncInvocation->getParameterList().push_back(vsSpecular->getName());
		vsMain->addAtomInstace(curFuncInvocation);
	}

	if (mVSOutputSpecular != NULL)
	{
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_ASSIGN, FFP_VS_COLOUR, internalCounter++);
		curFuncInvocation->getParameterList().push_back(vsSpecular->getName());
		curFuncInvocation->getParameterList().push_back(mVSOutputSpecular->getName());
		vsMain->addAtomInstace(curFuncInvocation);
	}
	
	

	// Create fragment shader colour invocations.
	Parameter* psDiffuse = NULL;
	Parameter* psSpecular = NULL;
	internalCounter = 0;
	
	// Handle diffuse colour.
	if (mPSInputDiffuse != NULL)
	{
		psDiffuse = mPSInputDiffuse;				
	}
	else
	{
		psDiffuse = psMain->resolveLocalParameter(Parameter::SPS_COLOR, 0, GCT_FLOAT4, "lDiffuse");
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_CONSTRUCT, FFP_PS_COLOUR_BEGIN, internalCounter++);
		curFuncInvocation->getParameterList().push_back("1.0");
		curFuncInvocation->getParameterList().push_back("1.0");
		curFuncInvocation->getParameterList().push_back("1.0");
		curFuncInvocation->getParameterList().push_back("1.0");
		curFuncInvocation->getParameterList().push_back(psDiffuse->getName());
		psMain->addAtomInstace(curFuncInvocation);
	}

	// Handle specular colour.
	if (mPSInputSpecular != NULL)
	{
		psSpecular = mPSInputSpecular;		
	}
	else
	{
		psSpecular = psMain->resolveLocalParameter(Parameter::SPS_COLOR, 1, GCT_FLOAT4, "lSpecular");
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_CONSTRUCT, FFP_PS_COLOUR_BEGIN, internalCounter++);
		curFuncInvocation->getParameterList().push_back("0.0");
		curFuncInvocation->getParameterList().push_back("0.0");
		curFuncInvocation->getParameterList().push_back("0.0");
		curFuncInvocation->getParameterList().push_back("0.0");
		curFuncInvocation->getParameterList().push_back(psSpecular->getName());
		psMain->addAtomInstace(curFuncInvocation);
	}

	// Assign diffuse colour.
	if (mPSOutputDiffuse != NULL)
	{	
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_ASSIGN, FFP_PS_COLOUR_BEGIN, internalCounter++);
		curFuncInvocation->getParameterList().push_back(psDiffuse->getName());
		curFuncInvocation->getParameterList().push_back(mPSOutputDiffuse->getName());		
		psMain->addAtomInstace(curFuncInvocation);
	}

	// Assign specular colour.
	if (mPSOutputSpecular != NULL)
	{
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_ASSIGN, FFP_PS_COLOUR_BEGIN, internalCounter++);
		curFuncInvocation->getParameterList().push_back(psSpecular->getName());
		curFuncInvocation->getParameterList().push_back(mPSOutputSpecular->getName());		
		psMain->addAtomInstace(curFuncInvocation);
	}

	// Add specular to out colour.
	internalCounter = 0;
	if (mPSOutputDiffuse != NULL && psSpecular != NULL)
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
				
	boost::hash_combine(hashCode, SubRenderState::getHashCode());
	boost::hash_combine(hashCode, mResolveStageFlags);
	
	return hashCode;
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

