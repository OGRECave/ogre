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
#include "OgreShaderFFPFog.h"
#ifdef RTSHADER_SYSTEM_BUILD_CORE_SHADERS
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
void FFPFog::updateGpuProgramsParams(Renderable* rend, Pass* pass, const AutoParamDataSource* source, 
									 const LightList* pLightList)
{	
	if (mFogMode == FOG_NONE)
		return;

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
		SceneManager* sceneMgr = ShaderGenerator::getSingleton().getActiveSceneManager();
		
		fogMode			= sceneMgr->getFogMode();
		newFogColour	= sceneMgr->getFogColour();
		newFogStart		= sceneMgr->getFogStart();
		newFogEnd		= sceneMgr->getFogEnd();
		newFogDensity	= sceneMgr->getFogDensity();				
	}

	// Set fog properties.
	setFogProperties(fogMode, newFogColour, newFogStart, newFogEnd, newFogDensity);

	// Per pixel fog.
	if (mCalcMode == CM_PER_PIXEL)
	{
		mFogParams->setGpuParameter(mFogParamsValue);
	}

	// Per vertex fog.
	else
	{				
		mFogParams->setGpuParameter(mFogParamsValue);	
	}

	mFogColour->setGpuParameter(mFogColourValue);

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
	bool hasError = false;

	// Resolve world view matrix.
	mWorldViewProjMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX, 0);
	
	// Resolve vertex shader input position.
	mVSInPos = vsMain->resolveInputParameter(Parameter::SPS_POSITION, 0, Parameter::SPC_POSITION_OBJECT_SPACE, GCT_FLOAT4);
	
	// Resolve fog colour.
	mFogColour = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_GLOBAL, "gFogColor");
	
	// Resolve pixel shader output diffuse color.
	mPSOutDiffuse = psMain->resolveOutputParameter(Parameter::SPS_COLOR, 0, Parameter::SPC_COLOR_DIFFUSE, GCT_FLOAT4);
	
	// Per pixel fog.
	if (mCalcMode == CM_PER_PIXEL)
	{
		// Resolve fog params.		
		mFogParams = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_GLOBAL, "gFogParams");
		
		// Resolve vertex shader output depth.		
		mVSOutDepth = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1, 
			Parameter::SPC_DEPTH_VIEW_SPACE,
			GCT_FLOAT1);
		
		// Resolve pixel shader input depth.
		mPSInDepth = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, mVSOutDepth->getIndex(), 
			mVSOutDepth->getContent(),
			GCT_FLOAT1);
		
		hasError |= !(mPSInDepth.get()) || !(mVSOutDepth.get()) || !(mFogParams.get());
	}
	// Per vertex fog.
	else
	{		
		// Resolve fog params.		
		mFogParams = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_GLOBAL, "gFogParams");
		
		// Resolve vertex shader output fog factor.		
		mVSOutFogFactor = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1, 
			Parameter::SPC_UNKNOWN,
			GCT_FLOAT1);
		
		// Resolve pixel shader input fog factor.
		mPSInFogFactor = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, mVSOutFogFactor->getIndex(), 
			mVSOutFogFactor->getContent(),
			GCT_FLOAT1);

		hasError |= !(mPSInFogFactor.get()) || !(mVSOutFogFactor.get()) || !(mFogParams.get());
	}

	hasError |= !(mWorldViewProjMatrix.get()) || !(mVSInPos.get()) || !(mFogColour.get()) || !(mPSOutDiffuse.get());
		
	if (hasError)
	{
		OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, 
				"Not all parameters could be constructed for the sub-render state.",
				"FFPFog::resolveParameters" );
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
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_PIXELFOG_DEPTH, FFP_VS_FOG, internalCounter++);
		curFuncInvocation->pushOperand(mWorldViewProjMatrix, Operand::OPS_IN);
		curFuncInvocation->pushOperand(mVSInPos, Operand::OPS_IN);	
		curFuncInvocation->pushOperand(mVSOutDepth, Operand::OPS_OUT);	
		vsMain->addAtomInstance(curFuncInvocation);		

		internalCounter = 0;
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
		
	}

	// Per vertex fog.
	else
	{
		internalCounter = 0;

		switch (mFogMode)
		{
		case FOG_LINEAR:
			curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_VERTEXFOG_LINEAR, FFP_VS_FOG, internalCounter++);
			break;
		case FOG_EXP:
			curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_VERTEXFOG_EXP, FFP_VS_FOG, internalCounter++);
			break;
		case FOG_EXP2:
			curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_VERTEXFOG_EXP2, FFP_VS_FOG, internalCounter++);
			break;
        case FOG_NONE:
        default:
            break;
		}
			
		curFuncInvocation->pushOperand(mWorldViewProjMatrix, Operand::OPS_IN);
		curFuncInvocation->pushOperand(mVSInPos, Operand::OPS_IN);		
		curFuncInvocation->pushOperand(mFogParams, Operand::OPS_IN);		
		curFuncInvocation->pushOperand(mVSOutFogFactor, Operand::OPS_OUT);
		vsMain->addAtomInstance(curFuncInvocation);		


		internalCounter = 0;

		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_LERP, FFP_PS_FOG, internalCounter++);
		curFuncInvocation->pushOperand(mFogColour, Operand::OPS_IN);
		curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_IN);
		curFuncInvocation->pushOperand(mPSInFogFactor, Operand::OPS_IN);
		curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT);
		psMain->addAtomInstance(curFuncInvocation);	
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
bool FFPFog::preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass)
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
		SceneManager* sceneMgr = ShaderGenerator::getSingleton().getActiveSceneManager();
		
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
		}
		mPassOverrideParams = false;
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
SubRenderState*	FFPFogFactory::createInstance(ScriptCompiler* compiler, 
													PropertyAbstractNode* prop, Pass* pass, SGScriptTranslator* translator)
{
	if (prop->name == "fog_stage")
	{
		if(prop->values.size() >= 1)
		{
			String strValue;

			if(false == SGScriptTranslator::getString(prop->values.front(), &strValue))
			{
				compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
				return NULL;
			}

			if (strValue == "ffp")
			{
				SubRenderState* subRenderState = createOrRetrieveInstance(translator);
				FFPFog* fogSubRenderState = static_cast<FFPFog*>(subRenderState);
				AbstractNodeList::const_iterator it = prop->values.begin();

				if(prop->values.size() >= 2)
				{
					++it;
					if (false == SGScriptTranslator::getString(*it, &strValue))
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
						return NULL;
					}

					if (strValue == "per_vertex")
					{
						fogSubRenderState->setCalcMode(FFPFog::CM_PER_VERTEX);
					}
					else if (strValue == "per_pixel")
					{
						fogSubRenderState->setCalcMode(FFPFog::CM_PER_PIXEL);
					}
				}
				
				return subRenderState;
			}
		}		
	}

	return NULL;
}

//-----------------------------------------------------------------------
void FFPFogFactory::writeInstance(MaterialSerializer* ser, SubRenderState* subRenderState, 
										Pass* srcPass, Pass* dstPass)
{
	ser->writeAttribute(4, "fog_stage");
	ser->writeValue("ffp");

	FFPFog* fogSubRenderState = static_cast<FFPFog*>(subRenderState);


	if (fogSubRenderState->getCalcMode() == FFPFog::CM_PER_VERTEX)
	{
		ser->writeValue("per_vertex");
	}
	else if (fogSubRenderState->getCalcMode() == FFPFog::CM_PER_PIXEL)
	{
		ser->writeValue("per_pixel");
	}
}

//-----------------------------------------------------------------------
SubRenderState*	FFPFogFactory::createInstanceImpl()
{
	return OGRE_NEW FFPFog;
}

}
}

#endif
