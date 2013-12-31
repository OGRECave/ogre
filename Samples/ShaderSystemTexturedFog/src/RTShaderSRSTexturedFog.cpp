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
#include "RTShaderSRSTexturedFog.h"
#include "OgreShaderFFPRenderState.h"
#include "OgreShaderProgram.h"
#include "OgreShaderParameter.h"
#include "OgreShaderProgramSet.h"
#include "OgreGpuProgram.h"
#include "OgrePass.h"
#include "OgreShaderGenerator.h"

#define FFP_FUNC_PIXELFOG_POSITION_DEPTH						"FFP_PixelFog_PositionDepth"

using namespace Ogre;
using namespace RTShader;
/************************************************************************/
/*                                                                      */
/************************************************************************/
String RTShaderSRSTexturedFog::Type = "TexturedFog";

//-----------------------------------------------------------------------
RTShaderSRSTexturedFog::RTShaderSRSTexturedFog(RTShaderSRSTexturedFogFactory* factory)
{
	mFactory = factory;
	mFogMode				= FOG_NONE;
	mPassOverrideParams		= false;
}

//-----------------------------------------------------------------------
const String& RTShaderSRSTexturedFog::getType() const
{
	return Type;
}


//-----------------------------------------------------------------------
int	RTShaderSRSTexturedFog::getExecutionOrder() const
{
	return FFP_FOG;
}
//-----------------------------------------------------------------------
void RTShaderSRSTexturedFog::updateGpuProgramsParams(Renderable* rend, Pass* pass, const AutoParamDataSource* source, 
									 const LightList* pLightList)
{	
	if (mFogMode == FOG_NONE)
		return;

	FogMode fogMode;
	Real newFogStart, newFogEnd, newFogDensity;

	//Check if this is an overlay element if so disable fog 
	if ((rend->getUseIdentityView() == true) && (rend->getUseIdentityProjection() == true))
	{
		fogMode			= FOG_NONE;
		newFogStart		= 100000000;
		newFogEnd		= 200000000;
		newFogDensity	= 0;
	}
	else
	{
		if (mPassOverrideParams)
		{
			fogMode			= pass->getFogMode();
			newFogStart		= pass->getFogStart();
			newFogEnd		= pass->getFogEnd();
			newFogDensity	= pass->getFogDensity();
		}
		else
		{
			SceneManager* sceneMgr = ShaderGenerator::getSingleton().getActiveSceneManager();
		
			fogMode			= sceneMgr->getFogMode();
			newFogStart		= sceneMgr->getFogStart();
			newFogEnd		= sceneMgr->getFogEnd();
			newFogDensity	= sceneMgr->getFogDensity();				
		}
	}

	// Set fog properties.
	setFogProperties(fogMode, newFogStart, newFogEnd, newFogDensity);

	mFogParams->setGpuParameter(mFogParamsValue);
}

//-----------------------------------------------------------------------
bool RTShaderSRSTexturedFog::resolveParameters(ProgramSet* programSet)
{
	if (mFogMode == FOG_NONE)
		return true;

	Program* vsProgram = programSet->getCpuVertexProgram();
	Program* psProgram = programSet->getCpuFragmentProgram();
	Function* vsMain = vsProgram->getEntryPointFunction();
	Function* psMain = psProgram->getEntryPointFunction();


	// Resolve world view matrix.
	mWorldMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_WORLD_MATRIX, 0);
	if (mWorldMatrix.get() == NULL)
		return false;
	
	// Resolve world view matrix.
	mCameraPos = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_CAMERA_POSITION, 0);
	if (mCameraPos.get() == NULL)
		return false;
	
	// Resolve vertex shader input position.
	mVSInPos = vsMain->resolveInputParameter(Parameter::SPS_POSITION, 0, Parameter::SPC_POSITION_OBJECT_SPACE, GCT_FLOAT4);
	if (mVSInPos.get() == NULL)
		return false;

		// Resolve fog colour.
	mFogColour = psMain->resolveLocalParameter(Parameter::SPS_UNKNOWN, -1, "FogColor", GCT_FLOAT4);
	if (mFogColour.get() == NULL)
		return false;
		
	// Resolve pixel shader output diffuse color.
	mPSOutDiffuse = psMain->resolveOutputParameter(Parameter::SPS_COLOR, 0, Parameter::SPC_COLOR_DIFFUSE, GCT_FLOAT4);
	if (mPSOutDiffuse.get() == NULL)	
		return false;
	
	// Resolve fog params.		
	mFogParams = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_GLOBAL, "gFogParams");
	if (mFogParams.get() == NULL)
		return false;

	// Resolve vertex shader output depth.		
	mVSOutPosView = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1, 
		Parameter::SPC_POSITION_VIEW_SPACE,
		GCT_FLOAT3);
	if (mVSOutPosView.get() == NULL)
		return false;
	
	// Resolve pixel shader input depth.
	mPSInPosView = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, mVSOutPosView->getIndex(), 
		mVSOutPosView->getContent(),
		GCT_FLOAT3);
	if (mPSInPosView.get() == NULL)
		return false;		
	
	// Resolve vertex shader output depth.		
	mVSOutDepth = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1, 
		Parameter::SPC_DEPTH_VIEW_SPACE,
		GCT_FLOAT1);
	if (mVSOutDepth.get() == NULL)
		return false;
	
	// Resolve pixel shader input depth.
	mPSInDepth = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, mVSOutDepth->getIndex(), 
		mVSOutDepth->getContent(),
		GCT_FLOAT1);
	if (mPSInDepth.get() == NULL)
		return false;		

	// Resolve texture sampler parameter.		
	mBackgroundTextureSampler = psProgram->resolveParameter(GCT_SAMPLERCUBE, mBackgroundSamplerIndex, (uint16)GPV_GLOBAL, "FogBackgroundSampler");
	if (mBackgroundTextureSampler.get() == NULL)
		return false;
	
		
	return true;
}

//-----------------------------------------------------------------------
bool RTShaderSRSTexturedFog::resolveDependencies(ProgramSet* programSet)
{
	if (mFogMode == FOG_NONE)
		return true;

	Program* vsProgram = programSet->getCpuVertexProgram();
	Program* psProgram = programSet->getCpuFragmentProgram();

	vsProgram->addDependency(FFP_LIB_FOG);
	psProgram->addDependency(FFP_LIB_COMMON);

	psProgram->addDependency(FFP_LIB_FOG);
	psProgram->addDependency(FFP_LIB_TEXTURING);
	
	return true;
}

//-----------------------------------------------------------------------
bool RTShaderSRSTexturedFog::addFunctionInvocations(ProgramSet* programSet)
{
	if (mFogMode == FOG_NONE)
		return true;

	Program* vsProgram = programSet->getCpuVertexProgram();
	Program* psProgram = programSet->getCpuFragmentProgram();
	Function* vsMain = vsProgram->getEntryPointFunction();
	Function* psMain = psProgram->getEntryPointFunction();
	FunctionInvocation* curFuncInvocation = NULL;	
	int internalCounter;

	internalCounter = 0;
	curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_PIXELFOG_POSITION_DEPTH, FFP_VS_FOG, internalCounter++);
	curFuncInvocation->pushOperand(mWorldMatrix, Operand::OPS_IN);
	curFuncInvocation->pushOperand(mCameraPos, Operand::OPS_IN);
	curFuncInvocation->pushOperand(mVSInPos, Operand::OPS_IN);	
	curFuncInvocation->pushOperand(mVSOutPosView, Operand::OPS_OUT);	
	curFuncInvocation->pushOperand(mVSOutDepth, Operand::OPS_OUT);	
	vsMain->addAtomInstance(curFuncInvocation);		
	
	internalCounter = 0;
	curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_SAMPLE_TEXTURE, FFP_PS_FOG, internalCounter++);
	curFuncInvocation->pushOperand(mBackgroundTextureSampler, Operand::OPS_IN);
	curFuncInvocation->pushOperand(mPSInPosView, Operand::OPS_IN);
	curFuncInvocation->pushOperand(mFogColour, Operand::OPS_OUT);
	psMain->addAtomInstance(curFuncInvocation);


	switch (mFogMode)
	{
	case FOG_LINEAR:
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_PIXELFOG_LINEAR, FFP_PS_FOG, internalCounter++);
		break;
	case FOG_EXP:
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_PIXELFOG_EXP, FFP_PS_FOG, internalCounter++);
		break;
	case FOG_EXP2:
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_PIXELFOG_EXP2, FFP_PS_FOG, internalCounter++);
		break;
       case FOG_NONE:
       default:
           break;
	}

	curFuncInvocation->pushOperand(mPSInDepth, Operand::OPS_IN);
	curFuncInvocation->pushOperand(mFogParams, Operand::OPS_IN);	
	curFuncInvocation->pushOperand(mFogColour, Operand::OPS_IN);		
	curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_IN);
	curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT);
	psMain->addAtomInstance(curFuncInvocation);	
	return true;
}

//-----------------------------------------------------------------------
void RTShaderSRSTexturedFog::copyFrom(const SubRenderState& rhs)
{
	const RTShaderSRSTexturedFog& rhsFog = static_cast<const RTShaderSRSTexturedFog&>(rhs);

	mFogMode			= rhsFog.mFogMode;
	mFogParamsValue		= rhsFog.mFogParamsValue;
	mFactory			= rhsFog.mFactory;
}

//-----------------------------------------------------------------------
bool RTShaderSRSTexturedFog::preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass)
{	
	if (mFactory == NULL)
		return false;

	FogMode fogMode;
	ColourValue newFogColour;
	Real newFogStart, newFogEnd, newFogDensity;

	if (srcPass->getFogOverride())
	{
		fogMode			= srcPass->getFogMode();
		newFogStart		= srcPass->getFogStart();
		newFogEnd		= srcPass->getFogEnd();
		newFogDensity	= srcPass->getFogDensity();
		mPassOverrideParams = true;
	}
	else
	{
		SceneManager* sceneMgr = ShaderGenerator::getSingleton().getActiveSceneManager();
		
		if (sceneMgr == NULL)
		{
			fogMode			= FOG_NONE;
			newFogStart		= 0.0;
			newFogEnd		= 0.0;
			newFogDensity	= 0.0;
		}
		else
		{
			fogMode			= sceneMgr->getFogMode();
			newFogStart		= sceneMgr->getFogStart();
			newFogEnd		= sceneMgr->getFogEnd();
			newFogDensity	= sceneMgr->getFogDensity();			
		}
		mPassOverrideParams = false;
	}

	// Set fog properties.
	setFogProperties(fogMode, newFogStart, newFogEnd, newFogDensity);
	
	
	// Override scene fog since it will happen in shader.
	dstPass->setFog(true, FOG_NONE, ColourValue::White, newFogDensity, newFogStart, newFogEnd);	

	TextureUnitState* tus = dstPass->createTextureUnitState(mFactory->getBackgroundTextureName());
	tus->setCubicTextureName(mFactory->getBackgroundTextureName(), true);
	mBackgroundSamplerIndex = dstPass->getNumTextureUnitStates() - 1;

	return true;
}

//-----------------------------------------------------------------------
void RTShaderSRSTexturedFog::setFogProperties(FogMode fogMode, 
							 float fogStart, 
							 float fogEnd, 
							 float fogDensity)
{
	mFogMode			= fogMode;
	mFogParamsValue.x 	= fogDensity;
	mFogParamsValue.y 	= fogStart;
	mFogParamsValue.z 	= fogEnd;
	mFogParamsValue.w 	= fogEnd != fogStart ? 1 / (fogEnd - fogStart) : 0;	
}

//-----------------------------------------------------------------------
const String& RTShaderSRSTexturedFogFactory::getType() const
{
	return RTShaderSRSTexturedFog::Type;
}

//-----------------------------------------------------------------------
SubRenderState*	RTShaderSRSTexturedFogFactory::createInstanceImpl()
{
	return OGRE_NEW RTShaderSRSTexturedFog(this);
}
