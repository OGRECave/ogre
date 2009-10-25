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
#include "OgreShaderFFPFog.h"
#include "OgreShaderFFPRenderState.h"
#include "OgreShaderProgram.h"
#include "OgreShaderParameter.h"
#include "OgreShaderProgramSet.h"
#include "OgreGpuProgram.h"
#include "OgrePass.h"
#include "OgreShaderGenerator.h"

namespace Ogre {
namespace RTShader {

/************************************************************************/
/*                                                                      */
/************************************************************************/
String FFPFog::Type = "FFP_Fog";

//-----------------------------------------------------------------------
FFPFog::FFPFog()
{
	mFogMode				= FOG_NONE;
	mCalcMode				= CM_PER_VERTEX;	
	mWorldViewProjMatrix	= NULL;	
	mFogParams				= NULL;	
	mFogColour				= NULL;	
	mVSInPos				= NULL;
	mVSOutFogFactor			= NULL;	
	mPSInFogFactor			= NULL;
	mVSOutDepth				= NULL;
	mPSInDepth				= NULL;
	mPSOutDiffuse			= NULL;
	mPassOverrideParams		= false;
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
	
	sh_hash_combine(hashCode, SubRenderState::getHashCode());
	sh_hash_combine(hashCode, mFogMode);
	sh_hash_combine(hashCode, mCalcMode);
	sh_hash_combine(hashCode, mPassOverrideParams);
	
	return hashCode;
}

//-----------------------------------------------------------------------
void FFPFog::updateGpuProgramsParams(Renderable* rend, Pass* pass, const AutoParamDataSource* source, 
									 const LightList* pLightList)
{	
	if (mFogMode == FOG_NONE)
		return;

	GpuProgramParametersSharedPtr psGpuParams = pass->getFragmentProgramParameters();

	FogMode fogMode;
	ColourValue newFogColour;
	Real newFogStart, newFogEnd, newFogDensity;

	if (mPassOverrideParams)
	{
		fogMode			= pass->getFogMode();
		newFogColour	= pass->getFogColour();
		newFogStart		= pass->getFogStart();
		newFogEnd		= pass->getFogEnd();
		newFogDensity	= pass->getFogDensity();
	}
	else
	{
		SceneManager* sceneMgr = ShaderGenerator::getSingleton().getSceneManager();
		
		fogMode			= sceneMgr->getFogMode();
		newFogColour	= sceneMgr->getFogColour();
		newFogStart		= sceneMgr->getFogStart();
		newFogEnd		= sceneMgr->getFogEnd();
		newFogDensity	= sceneMgr->getFogDensity();				
	}

	// Set fog properties.
	setFogProperties(mFogMode, newFogColour, newFogStart, newFogEnd, newFogDensity);

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
	mVSInPos = vsMain->resolveInputParameter(Parameter::SPS_POSITION, 0, Parameter::SPC_POSITION_OBJECT_SPACE, GCT_FLOAT4);
	if (mVSInPos == NULL)
		return false;

	
	// Resolve fog colour.
	mFogColour = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_GLOBAL, "gFogColor");
	if (mFogColour == NULL)
		return false;
		

	// Resolve pixel shader output diffuse color.
	mPSOutDiffuse = psMain->resolveOutputParameter(Parameter::SPS_COLOR, 0, Parameter::SPC_COLOR_DIFFUSE, GCT_FLOAT4);
	if (mPSOutDiffuse == NULL)	
		return false;
	
	// Per pixel fog.
	if (mCalcMode == CM_PER_PIXEL)
	{
		// Resolve fog params.		
		mFogParams = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_GLOBAL, "gFogParams");
		if (mFogParams == NULL)
			return false;


		// Resolve vertex shader output depth.		
		mVSOutDepth = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1, 
			Parameter::SPC_DEPTH_VIEW_SPACE,
			GCT_FLOAT1);
		if (NULL == mVSOutDepth)
			return false;


		// Resolve pixel shader input depth.
		mPSInDepth = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, mVSOutDepth->getIndex(), 
			mVSOutDepth->getContent(),
			GCT_FLOAT1);
		if (NULL == mPSInDepth)
			return false;		
		
	}

	// Per vertex fog.
	else
	{		
		// Resolve fog params.		
		mFogParams = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_GLOBAL, "gFogParams");
		if (mFogParams == NULL)
			return false;
		
											
		// Resolve vertex shader output fog factor.		
		mVSOutFogFactor = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1, 
			Parameter::SPC_UNKNOWN,
			GCT_FLOAT1);
		if (NULL == mVSOutFogFactor)
			return false;
		
	
		// Resolve pixel shader input fog factor.
		mPSInFogFactor = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, mVSOutFogFactor->getIndex(), 
			mVSOutFogFactor->getContent(),
			GCT_FLOAT1);
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
bool FFPFog::preAddToRenderState(RenderState* renderState, Pass* srcPass, Pass* dstPass)
{	
	FogMode fogMode;
	ColourValue newFogColour;
	Real newFogStart, newFogEnd, newFogDensity;

	if (srcPass->getFogOverride())
	{
		fogMode			= srcPass->getFogMode();
		newFogColour	= srcPass->getFogColour();
		newFogStart		= srcPass->getFogStart();
		newFogEnd		= srcPass->getFogEnd();
		newFogDensity	= srcPass->getFogDensity();
		mPassOverrideParams = true;
	}
	else
	{
		SceneManager* sceneMgr = ShaderGenerator::getSingleton().getSceneManager();
		
		if (sceneMgr == NULL)
		{
			fogMode			= FOG_NONE;
			newFogColour	= ColourValue::White;
			newFogStart		= 0.0;
			newFogEnd		= 0.0;
			newFogDensity	= 0.0;
		}
		else
		{
			fogMode			= sceneMgr->getFogMode();
			newFogColour	= sceneMgr->getFogColour();
			newFogStart		= sceneMgr->getFogStart();
			newFogEnd		= sceneMgr->getFogEnd();
			newFogDensity	= sceneMgr->getFogDensity();
			mPassOverrideParams = false;
		}		
	}

	// Set fog properties.
	setFogProperties(fogMode, newFogColour, newFogStart, newFogEnd, newFogDensity);
	
	
	// Override scene fog since it will happen in shader.
	dstPass->setFog(true, FOG_NONE, newFogColour, newFogDensity, newFogStart, newFogEnd);	

	return true;
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

