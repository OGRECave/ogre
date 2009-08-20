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
#include "OgreShaderFFPFog.h"
#include "OgreShaderFFPRenderState.h"
#include "OgreShaderProgram.h"
#include "OgreShaderParameter.h"
#include "OgreShaderProgramSet.h"
#include "OgreGpuProgram.h"
#include "OgrePass.h"

namespace Ogre {
namespace CRTShader {

/************************************************************************/
/*                                                                      */
/************************************************************************/
String FFPFog::Type = "FFP_Fog";

//-----------------------------------------------------------------------
FFPFog::FFPFog()
{
	mFogMode				= FOG_NONE;
	mCalcMode				= CM_PER_PIXEL;	
	mWorldViewProjMatrix	= NULL;	
	mFogParams				= NULL;	
	mFogColour				= NULL;	
	mVSInPos				= NULL;
	mVSOutFogFactor			= NULL;	
	mPSInFogFactor			= NULL;
	mVSOutDepth				= NULL;
	mPSInDepth				= NULL;
	mPSOutDiffuse			= NULL;
}

//-----------------------------------------------------------------------
FFPFog::~FFPFog()
{

}

//-----------------------------------------------------------------------
const String& FFPFog::getType() const
{
	return Type;
}


//-----------------------------------------------------------------------
int	FFPFog::getExecutionOrder() const
{
	return FFP_FOG;
}

//-----------------------------------------------------------------------
uint32 FFPFog::getHashCode()
{
	uint32 hashCode = 0;
	
	boost::hash_combine(hashCode, __super::getHashCode());
	boost::hash_combine(hashCode, mFogMode);
	boost::hash_combine(hashCode, mCalcMode);

	return hashCode;
}

//-----------------------------------------------------------------------
void FFPFog::updateGpuProgramsParams(Renderable* rend, Pass* pass, const AutoParamDataSource* source, 
									 const LightList* pLightList)
{	
	if (mFogMode == FOG_NONE)
		return;

	GpuProgramParametersSharedPtr psGpuParams = pass->getFragmentProgramParameters();

	// Per pixel fog.
	if (mCalcMode == CM_PER_PIXEL)
	{
		psGpuParams->setNamedConstant(mFogParams->getName(), mFogParamsValue);
	}

	// Per vertex fog.
	else
	{
		GpuProgramParametersSharedPtr vsGpuParams = pass->getVertexProgramParameters();
				
		vsGpuParams->setNamedConstant(mFogParams->getName(), mFogParamsValue);	
	}

	psGpuParams->setNamedConstant(mFogColour->getName(), mFogColourValue);

}

//-----------------------------------------------------------------------
bool FFPFog::resolveParameters(ProgramSet* programSet)
{
	if (mFogMode == FOG_NONE)
		return true;

	Program* vsProgram = programSet->getCpuVertexProgram();
	Program* psProgram = programSet->getCpuFragmentProgram();
	Function* vsMain = vsProgram->getEntryPointFunction();
	Function* psMain = psProgram->getEntryPointFunction();


	// Resolve world view matrix.
	mWorldViewProjMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX, 0);
	if (mWorldViewProjMatrix == NULL)
		return false;
	
	// Resolve vertex shader input position.
	mVSInPos = vsMain->resolveInputParameter(Parameter::SPS_POSITION, 0, GCT_FLOAT4);
	if (mVSInPos == NULL)
		return false;

	
	// Resolve fog colour.
	mFogColour = psProgram->resolveParameter(GCT_FLOAT4, -1, "gFogColor");
	if (mFogColour == NULL)
		return false;
		

	// Resolve pixel shader output diffuse color.
	mPSOutDiffuse = psMain->resolveOutputParameter(Parameter::SPS_COLOR, 0, GCT_FLOAT4);
	if (mPSOutDiffuse == NULL)	
		return false;
	
	// Per pixel fog.
	if (mCalcMode == CM_PER_PIXEL)
	{
		// Resolve fog params.		
		mFogParams = psProgram->resolveParameter(GCT_FLOAT4, -1, "gFogParams");
		if (mFogParams == NULL)
			return false;


		// Resolve vertex shader output depth.		
		mVSOutDepth = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1, GCT_FLOAT1);
		if (NULL == mVSOutDepth)
			return false;


		// Resolve pixel shader input depth.
		mPSInDepth = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, mVSOutDepth->getIndex(), GCT_FLOAT1);
		if (NULL == mPSInDepth)
			return false;		
		
	}

	// Per vertex fog.
	else
	{		
		// Resolve fog params.		
		mFogParams = vsProgram->resolveParameter(GCT_FLOAT4, -1, "gFogParams");
		if (mFogParams == NULL)
			return false;
		
											
		// Resolve vertex shader output fog factor.		
		mVSOutFogFactor = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1, GCT_FLOAT1);
		if (NULL == mVSOutFogFactor)
			return false;
		
	
		// Resolve pixel shader input fog factor.
		mPSInFogFactor = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, mVSOutFogFactor->getIndex(), GCT_FLOAT1);
		if (NULL == mPSInFogFactor)
			return false;			
	}


	return true;
}

//-----------------------------------------------------------------------
bool FFPFog::resolveDependencies(ProgramSet* programSet)
{
	if (mFogMode == FOG_NONE)
		return true;

	Program* vsProgram = programSet->getCpuVertexProgram();
	Program* psProgram = programSet->getCpuFragmentProgram();

	vsProgram->addDependency(FFP_LIB_FOG);
	psProgram->addDependency(FFP_LIB_COMMON);

	// Per pixel fog.
	if (mCalcMode == CM_PER_PIXEL)
	{
		psProgram->addDependency(FFP_LIB_FOG);

	}	

	return true;
}

//-----------------------------------------------------------------------
bool FFPFog::addFunctionInvocations(ProgramSet* programSet)
{
	if (mFogMode == FOG_NONE)
		return true;

	Program* vsProgram = programSet->getCpuVertexProgram();
	Program* psProgram = programSet->getCpuFragmentProgram();
	Function* vsMain = vsProgram->getEntryPointFunction();
	Function* psMain = psProgram->getEntryPointFunction();
	FunctionInvocation* curFuncInvocation = NULL;	
	int internalCounter;


	// Per pixel fog.
	if (mCalcMode == CM_PER_PIXEL)
	{
		internalCounter = 0;
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_PIXELFOG_DEPTH, FFP_VS_FOG, internalCounter++);
		curFuncInvocation->getParameterList().push_back(mWorldViewProjMatrix->getName());
		curFuncInvocation->getParameterList().push_back(mVSInPos->getName());	
		curFuncInvocation->getParameterList().push_back(mVSOutDepth->getName());	
		vsMain->addAtomInstace(curFuncInvocation);		

		internalCounter = 0;
		switch (mFogMode)
		{
		case FOG_LINEAR:
			curFuncInvocation = new FunctionInvocation(FFP_FUNC_PIXELFOG_LINEAR, FFP_PS_FOG, internalCounter++);
			break;
		case FOG_EXP:
			curFuncInvocation = new FunctionInvocation(FFP_FUNC_PIXELFOG_EXP, FFP_PS_FOG, internalCounter++);
			break;
		case FOG_EXP2:
			curFuncInvocation = new FunctionInvocation(FFP_FUNC_PIXELFOG_EXP2, FFP_PS_FOG, internalCounter++);
			break;
		}

		curFuncInvocation->getParameterList().push_back(mPSInDepth->getName());
		curFuncInvocation->getParameterList().push_back(mFogParams->getName());	
		curFuncInvocation->getParameterList().push_back(mFogColour->getName());		
		curFuncInvocation->getParameterList().push_back(mPSOutDiffuse->getName());
		curFuncInvocation->getParameterList().push_back(mPSOutDiffuse->getName());
		psMain->addAtomInstace(curFuncInvocation);	
		
	}

	// Per vertex fog.
	else
	{
		internalCounter = 0;

		switch (mFogMode)
		{
		case FOG_LINEAR:
			curFuncInvocation = new FunctionInvocation(FFP_FUNC_VERTEXFOG_LINEAR, FFP_VS_FOG, internalCounter++);
			break;
		case FOG_EXP:
			curFuncInvocation = new FunctionInvocation(FFP_FUNC_VERTEXFOG_EXP, FFP_VS_FOG, internalCounter++);
			break;
		case FOG_EXP2:
			curFuncInvocation = new FunctionInvocation(FFP_FUNC_VERTEXFOG_EXP2, FFP_VS_FOG, internalCounter++);
			break;
		}
			
		curFuncInvocation->getParameterList().push_back(mWorldViewProjMatrix->getName());
		curFuncInvocation->getParameterList().push_back(mVSInPos->getName());		
		curFuncInvocation->getParameterList().push_back(mFogParams->getName());		
		curFuncInvocation->getParameterList().push_back(mVSOutFogFactor->getName());
		vsMain->addAtomInstace(curFuncInvocation);		


		internalCounter = 0;

		curFuncInvocation = new FunctionInvocation(FFP_FUNC_LERP, FFP_PS_FOG, internalCounter++);
		curFuncInvocation->getParameterList().push_back(mFogColour->getName());
		curFuncInvocation->getParameterList().push_back(mPSOutDiffuse->getName());
		curFuncInvocation->getParameterList().push_back(mPSInFogFactor->getName());
		curFuncInvocation->getParameterList().push_back(mPSOutDiffuse->getName());
		psMain->addAtomInstace(curFuncInvocation);	
	}



	return true;
}

//-----------------------------------------------------------------------
void FFPFog::copyFrom(const SubRenderState& rhs)
{
	const FFPFog& rhsFog = static_cast<const FFPFog&>(rhs);

	mFogMode			= rhsFog.mFogMode;
	mFogColourValue		= rhsFog.mFogColourValue;
	mFogParamsValue		= rhsFog.mFogParamsValue;

	setCalcMode(rhsFog.mCalcMode);
}

//-----------------------------------------------------------------------
void FFPFog::setFogProperties(FogMode fogMode, 
							 const ColourValue& fogColour, 
							 float fogStart, 
							 float fogEnd, 
							 float fogDensity)
{
	mFogMode			= fogMode;
	mFogColourValue		= fogColour;
	mFogParamsValue.x 	= fogDensity;
	mFogParamsValue.y 	= fogStart;
	mFogParamsValue.z 	= fogEnd;
	mFogParamsValue.w 	= fogEnd != fogStart ? 1 / (fogEnd - fogStart) : 0;	
}

//-----------------------------------------------------------------------
const String& FFPFogFactory::getType() const
{
	return FFPFog::Type;
}

//-----------------------------------------------------------------------
SubRenderState*	FFPFogFactory::createInstanceImpl()
{
	return new FFPFog;
}


}
}

