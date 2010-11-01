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
#include "OgreShaderExHardwareSkinning.h"
#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
#include "OgreShaderFFPRenderState.h"
#include "OgreShaderProgram.h"
#include "OgreShaderParameter.h"
#include "OgreShaderProgramSet.h"

namespace Ogre {
namespace RTShader {

/************************************************************************/
/*                                                                      */
/************************************************************************/
String HardwareSkinning::Type = "SGX_HardwareSkinning";

HardwareSkinning::HardwareSkinning() :
	mBoneCount(0),
	mWeightCount(0),
	mAllowStateChange(false),
	mDoBoneCalculations(false),
	mCreator(NULL)
{
}

//-----------------------------------------------------------------------
const String& HardwareSkinning::getType() const
{
	return Type;
}


//-----------------------------------------------------------------------
int	HardwareSkinning::getExecutionOrder() const
{
	return FFP_TRANSFORM;
}

//-----------------------------------------------------------------------
void HardwareSkinning::setHardwareSkinningParam(ushort boneCount, ushort weightCount)
{
	mBoneCount = std::min<ushort>(boneCount,256);
	mWeightCount = std::min<ushort>(weightCount,4);
}

//-----------------------------------------------------------------------
ushort HardwareSkinning::getBoneCount()
{
	return mBoneCount;
}

//-----------------------------------------------------------------------
ushort HardwareSkinning::getWeightCount()
{
	return mWeightCount;
}

//-----------------------------------------------------------------------
bool HardwareSkinning::preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass)
{
	mDoBoneCalculations =  
		(mBoneCount != 0) && (mBoneCount <= 256) &&
		(mWeightCount != 0) && (mWeightCount <= 4) &&
		((mAllowStateChange == true) ||
		((srcPass->hasVertexProgram() == true) &&
		(srcPass->getVertexProgram()->isSkeletalAnimationIncluded() == true)));

	if ((mDoBoneCalculations) && (mCreator))
	{
		//update the receiver and caster materials
		if (dstPass->getParent()->getShadowCasterMaterial().isNull())
		{
			dstPass->getParent()->setShadowCasterMaterial(
				mCreator->getCustomShadowCasterMaterial(mWeightCount - 1));
		}

		if (dstPass->getParent()->getShadowReceiverMaterial().isNull())
		{
			dstPass->getParent()->setShadowReceiverMaterial(
				mCreator->getCustomShadowReceiverMaterial(mWeightCount - 1));
		}
	}

	return true;
}

//-----------------------------------------------------------------------
bool HardwareSkinning::resolveParameters(ProgramSet* programSet)
{
	
	Program* vsProgram = programSet->getCpuVertexProgram();
	Function* vsMain = vsProgram->getEntryPointFunction();

	vsProgram->setSkeletalAnimationIncluded(true);
	
	//
	// get the parameters we need whether we are doing bone calculations or not
	//
	// Note: in order to be consistent we will always output position, normal,
	// tangent and binormal in both object and world space. And output position
	// in projective space to cover the responsibility of the transform stage

	//input param
	mParamInPosition = vsMain->resolveInputParameter(Parameter::SPS_POSITION, 0, Parameter::SPC_POSITION_OBJECT_SPACE, GCT_FLOAT4);	
	mParamInNormal = vsMain->resolveInputParameter(Parameter::SPS_NORMAL, 0, Parameter::SPC_NORMAL_OBJECT_SPACE, GCT_FLOAT3);	
	mParamInBiNormal = vsMain->resolveInputParameter(Parameter::SPS_BINORMAL, 0, Parameter::SPC_BINORMAL_OBJECT_SPACE, GCT_FLOAT3);	
	mParamInTangent = vsMain->resolveInputParameter(Parameter::SPS_TANGENT, 0, Parameter::SPC_TANGENT_OBJECT_SPACE, GCT_FLOAT3);	
	
	//local param
	mParamLocalPositionWorld = vsMain->resolveLocalParameter(Parameter::SPS_POSITION, 0, Parameter::SPC_POSITION_WORLD_SPACE, GCT_FLOAT4);	
	mParamLocalNormalWorld = vsMain->resolveLocalParameter(Parameter::SPS_NORMAL, 0, Parameter::SPC_NORMAL_WORLD_SPACE, GCT_FLOAT3);	
	mParamLocalTangentWorld = vsMain->resolveLocalParameter(Parameter::SPS_TANGENT, 0, Parameter::SPC_TANGENT_WORLD_SPACE, GCT_FLOAT3);	
	mParamLocalBinormalWorld = vsMain->resolveLocalParameter(Parameter::SPS_BINORMAL, 0, Parameter::SPC_BINORMAL_WORLD_SPACE, GCT_FLOAT3);	
	
	//output param
	mParamOutPositionProj = vsMain->resolveOutputParameter(Parameter::SPS_POSITION, 0, Parameter::SPC_POSITION_PROJECTIVE_SPACE, GCT_FLOAT4);	
	
	//check if parameter retrival went well
	bool isValid = 
		(mParamInPosition.get() != NULL) &&
		(mParamInNormal.get() != NULL) &&
		(mParamInBiNormal.get() != NULL) &&
		(mParamInTangent.get() != NULL) &&
		(mParamLocalPositionWorld.get() != NULL) &&
		(mParamLocalNormalWorld.get() != NULL) &&
		(mParamLocalTangentWorld.get() != NULL) &&
		(mParamLocalBinormalWorld.get() != NULL) &&
		(mParamOutPositionProj.get() != NULL);

	
	if (mDoBoneCalculations == true)
	{
		//input parameters
		mParamInNormal = vsMain->resolveInputParameter(Parameter::SPS_NORMAL, 0, Parameter::SPC_NORMAL_OBJECT_SPACE, GCT_FLOAT3);	
		mParamInBiNormal = vsMain->resolveInputParameter(Parameter::SPS_BINORMAL, 0, Parameter::SPC_BINORMAL_OBJECT_SPACE, GCT_FLOAT3);	
		mParamInTangent = vsMain->resolveInputParameter(Parameter::SPS_TANGENT, 0, Parameter::SPC_TANGENT_OBJECT_SPACE, GCT_FLOAT3);	
		mParamInIndices = vsMain->resolveInputParameter(Parameter::SPS_BLEND_INDICES, 0, Parameter::SPC_UNKNOWN, GCT_FLOAT4);
		mParamInWeights = vsMain->resolveInputParameter(Parameter::SPS_BLEND_WEIGHTS, 0, Parameter::SPC_UNKNOWN, GCT_FLOAT4);
		mParamInWorldMatrices = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_WORLD_MATRIX_ARRAY_3x4, 0, mBoneCount);
		mParamInInvWorldMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_INVERSE_WORLD_MATRIX, 0);
		mParamInViewProjMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_VIEWPROJ_MATRIX, 0);
		
		mParamTempFloat4 = vsMain->resolveLocalParameter(Parameter::SPS_UNKNOWN, -1, "TempVal4", GCT_FLOAT4);	
		mParamTempFloat3 = vsMain->resolveLocalParameter(Parameter::SPS_UNKNOWN, -1, "TempVal3", GCT_FLOAT3);	
			
		//check if parameter retrival went well
		isValid &= 
			(mParamInIndices.get() != NULL) &&
			(mParamInWeights.get() != NULL) &&
			(mParamInWorldMatrices.get() != NULL) &&
			(mParamInViewProjMatrix.get() != NULL) &&
			(mParamInInvWorldMatrix.get() != NULL) &&
			(mParamTempFloat4.get() != NULL) &&
			(mParamTempFloat3.get() != NULL);
	}
	else
	{
		mParamInWorldMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_WORLD_MATRIX, 0);
		mParamInWorldViewProjMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX, 0);
	
		//check if parameter retrival went well
		isValid &= 
			(mParamInWorldMatrix.get() != NULL) &&
			(mParamInWorldViewProjMatrix.get() != NULL);
	}
	return isValid;
}

//-----------------------------------------------------------------------
bool HardwareSkinning::resolveDependencies(ProgramSet* programSet)
{
	Program* vsProgram = programSet->getCpuVertexProgram();
	vsProgram->addDependency(FFP_LIB_COMMON);
	vsProgram->addDependency(FFP_LIB_TRANSFORM);
	
	return true;
}

//-----------------------------------------------------------------------
bool HardwareSkinning::addFunctionInvocations(ProgramSet* programSet)
{
	
	Program* vsProgram = programSet->getCpuVertexProgram();
	Function* vsMain = vsProgram->getEntryPointFunction();
	int internalCounter = 0;

	//add functions to calculate position data in world, object and projective space
	addPositionCalculations(vsMain, internalCounter);
	
	//add functions to calculate normal and normal related data in world and object space
	addNormalRelatedCalculations(vsMain, mParamInNormal, mParamLocalNormalWorld, internalCounter);
	addNormalRelatedCalculations(vsMain, mParamInTangent, mParamLocalTangentWorld, internalCounter);
	addNormalRelatedCalculations(vsMain, mParamInBiNormal, mParamLocalBinormalWorld, internalCounter);
	return true;
}

//-----------------------------------------------------------------------
void HardwareSkinning::addPositionCalculations(Function* vsMain, int& funcCounter)
{
	FunctionInvocation* curFuncInvocation = NULL;	
	
	if (mDoBoneCalculations == true)
	{
		//set functions to calculate world position
		for(int i = 0 ; i < getWeightCount() ; ++i)
		{
			addIndexedPositionWeight(vsMain, i, funcCounter);
		}
		
		//update back the original position relative to the object
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_TRANSFORM, FFP_VS_TRANSFORM, funcCounter++); 								
		curFuncInvocation->pushOperand(mParamInInvWorldMatrix, Operand::OPS_IN);
		curFuncInvocation->pushOperand(mParamLocalPositionWorld, Operand::OPS_IN);
		curFuncInvocation->pushOperand(mParamInPosition, Operand::OPS_OUT);
		vsMain->addAtomInstance(curFuncInvocation);	

		//update the projective position thereby filling the transform stage role
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_TRANSFORM, FFP_VS_TRANSFORM, funcCounter++); 								
		curFuncInvocation->pushOperand(mParamInViewProjMatrix, Operand::OPS_IN);
		curFuncInvocation->pushOperand(mParamLocalPositionWorld, Operand::OPS_IN);
		curFuncInvocation->pushOperand(mParamOutPositionProj, Operand::OPS_OUT);
		vsMain->addAtomInstance(curFuncInvocation);	
	}
	else
	{
		//update from object to world space
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_TRANSFORM, FFP_VS_TRANSFORM, funcCounter++); 								
		curFuncInvocation->pushOperand(mParamInWorldMatrix, Operand::OPS_IN);
		curFuncInvocation->pushOperand(mParamInPosition, Operand::OPS_IN);
		curFuncInvocation->pushOperand(mParamLocalPositionWorld, Operand::OPS_OUT);
		vsMain->addAtomInstance(curFuncInvocation);	

		//update from object to projective space
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_TRANSFORM, FFP_VS_TRANSFORM, funcCounter++); 								
		curFuncInvocation->pushOperand(mParamInWorldViewProjMatrix, Operand::OPS_IN);
		curFuncInvocation->pushOperand(mParamInPosition, Operand::OPS_IN);
		curFuncInvocation->pushOperand(mParamOutPositionProj, Operand::OPS_OUT);
		vsMain->addAtomInstance(curFuncInvocation);	
	}
}

//-----------------------------------------------------------------------
void HardwareSkinning::addNormalRelatedCalculations(Function* vsMain,
								ParameterPtr& pNormalRelatedParam, 
								ParameterPtr& pNormalWorldRelatedParam, 
								int& funcCounter)
{
	FunctionInvocation* curFuncInvocation;
	
	if (mDoBoneCalculations == true)
	{
		//set functions to calculate world normal
		for(int i = 0 ; i < getWeightCount() ; ++i)
		{
			addIndexedNormalRelatedWeight(vsMain, pNormalRelatedParam, pNormalWorldRelatedParam, i, funcCounter);
		}

		//update back the original position relative to the object
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_TRANSFORM, FFP_VS_TRANSFORM, funcCounter++); 								
		curFuncInvocation->pushOperand(mParamInInvWorldMatrix, Operand::OPS_IN);
		curFuncInvocation->pushOperand(pNormalWorldRelatedParam, Operand::OPS_IN);
		curFuncInvocation->pushOperand(pNormalRelatedParam, Operand::OPS_OUT);
		vsMain->addAtomInstance(curFuncInvocation);	
	}
	else
	{
		//update from object to world space
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_TRANSFORM, FFP_VS_TRANSFORM, funcCounter++); 								
		curFuncInvocation->pushOperand(mParamInWorldMatrix, Operand::OPS_IN);
		curFuncInvocation->pushOperand(pNormalRelatedParam, Operand::OPS_IN);
		curFuncInvocation->pushOperand(pNormalWorldRelatedParam, Operand::OPS_OUT);
		vsMain->addAtomInstance(curFuncInvocation);	
	}

}

//-----------------------------------------------------------------------
void HardwareSkinning::addIndexedPositionWeight(Function* vsMain, 
								int index, int& funcCounter)
{
	Operand::OpMask indexMask = indexToMask(index);
	
	FunctionInvocation* curFuncInvocation;

	//multiply position with world matrix and put into temporary param
	curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_TRANSFORM, FFP_VS_TRANSFORM, funcCounter++); 								
	curFuncInvocation->pushOperand(mParamInWorldMatrices, Operand::OPS_IN);
	curFuncInvocation->pushOperand(mParamInIndices, Operand::OPS_IN,  indexMask, 1);
	curFuncInvocation->pushOperand(mParamInPosition, Operand::OPS_IN);	
	curFuncInvocation->pushOperand(mParamTempFloat4, Operand::OPS_OUT, Operand::OPM_X | Operand::OPM_Y | Operand::OPM_Z);	
	vsMain->addAtomInstance(curFuncInvocation);	

	//set w value of temporary param to 1
	curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ASSIGN, FFP_VS_TRANSFORM, funcCounter++); 								
	curFuncInvocation->pushOperand(ParameterFactory::createConstParamFloat(1.0f), Operand::OPS_IN);
	curFuncInvocation->pushOperand(mParamTempFloat4, Operand::OPS_OUT, Operand::OPM_W);	
	vsMain->addAtomInstance(curFuncInvocation);	

	//multiply temporary param with  weight
	curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_MODULATE, FFP_VS_TRANSFORM, funcCounter++); 								
	curFuncInvocation->pushOperand(mParamTempFloat4, Operand::OPS_IN);	
	curFuncInvocation->pushOperand(mParamInWeights, Operand::OPS_IN, indexMask);
	curFuncInvocation->pushOperand(mParamTempFloat4, Operand::OPS_OUT);	
	vsMain->addAtomInstance(curFuncInvocation);	

	//check if on first iteration
	if (index == 0)
	{
		//set the local param as the value of the world param
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ASSIGN, FFP_VS_TRANSFORM, funcCounter++); 								
		curFuncInvocation->pushOperand(mParamTempFloat4, Operand::OPS_IN);
		curFuncInvocation->pushOperand(mParamLocalPositionWorld, Operand::OPS_OUT);	
		vsMain->addAtomInstance(curFuncInvocation);	
	}
	else
	{
		//add the local param as the value of the world param
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ADD, FFP_VS_TRANSFORM, funcCounter++); 								
		curFuncInvocation->pushOperand(mParamLocalPositionWorld, Operand::OPS_IN);	
		curFuncInvocation->pushOperand(mParamLocalPositionWorld, Operand::OPS_OUT);	
		curFuncInvocation->pushOperand(mParamTempFloat4, Operand::OPS_IN);
		vsMain->addAtomInstance(curFuncInvocation);	
	}
}


//-----------------------------------------------------------------------
void HardwareSkinning::addIndexedNormalRelatedWeight(Function* vsMain,
								ParameterPtr& pNormalParam, 
								ParameterPtr& pNormalWorldRelatedParam, 
								int index, int& funcCounter)
{

	FunctionInvocation* curFuncInvocation;

	Operand::OpMask indexMask = indexToMask(index);
	
	//multiply position with world matrix and put into temporary param
	curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_TRANSFORM, FFP_VS_TRANSFORM, funcCounter++); 								
	curFuncInvocation->pushOperand(mParamInWorldMatrices, Operand::OPS_IN, Operand::OPM_ALL);
	curFuncInvocation->pushOperand(mParamInIndices, Operand::OPS_IN,  indexMask, 1);
	curFuncInvocation->pushOperand(pNormalParam, Operand::OPS_IN);	
	curFuncInvocation->pushOperand(mParamTempFloat3, Operand::OPS_OUT, Operand::OPM_X | Operand::OPM_Y | Operand::OPM_Z);	
	vsMain->addAtomInstance(curFuncInvocation);	

	//multiply temporary param with weight
	curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_MODULATE, FFP_VS_TRANSFORM, funcCounter++); 								
	curFuncInvocation->pushOperand(mParamTempFloat3, Operand::OPS_IN);	
	curFuncInvocation->pushOperand(mParamInWeights, Operand::OPS_IN, indexMask);
	curFuncInvocation->pushOperand(mParamTempFloat3, Operand::OPS_OUT);	
	vsMain->addAtomInstance(curFuncInvocation);	

	//check if on first iteration
	if (index == 0)
	{
		//set the local param as the value of the world normal
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ASSIGN, FFP_VS_TRANSFORM, funcCounter++); 								
		curFuncInvocation->pushOperand(mParamTempFloat3, Operand::OPS_IN);
		curFuncInvocation->pushOperand(pNormalWorldRelatedParam, Operand::OPS_OUT);	
		vsMain->addAtomInstance(curFuncInvocation);	
	}
	else
	{
		//add the local param as the value of the world normal
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ADD, FFP_VS_TRANSFORM, funcCounter++); 								
		curFuncInvocation->pushOperand(pNormalWorldRelatedParam, Operand::OPS_IN);	
		curFuncInvocation->pushOperand(pNormalWorldRelatedParam, Operand::OPS_OUT);	
		curFuncInvocation->pushOperand(mParamTempFloat3, Operand::OPS_IN);
		vsMain->addAtomInstance(curFuncInvocation);	
	}
}

//-----------------------------------------------------------------------
Operand::OpMask HardwareSkinning::indexToMask(int index)
{
	switch(index)
	{
	case 0: return Operand::OPM_X; 
	case 1: return Operand::OPM_Y; 
	case 2: return Operand::OPM_Z; 
	case 3: return Operand::OPM_W; 
	default: OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Illegal value", "HardwareSkinning::indexToMask");
	}
}

//-----------------------------------------------------------------------
void HardwareSkinning::copyFrom(const SubRenderState& rhs)
{
	const HardwareSkinning& hardSkin = static_cast<const HardwareSkinning&>(rhs);
	mWeightCount = hardSkin.mWeightCount;
	mBoneCount = hardSkin.mBoneCount;
	mAllowStateChange = hardSkin.mAllowStateChange;
	mDoBoneCalculations = hardSkin.mDoBoneCalculations;
	mCreator = hardSkin.mCreator;
}

//-----------------------------------------------------------------------
const String& HardwareSkinningFactory::getType() const
{
	return HardwareSkinning::Type;
}

//-----------------------------------------------------------------------
SubRenderState*	HardwareSkinningFactory::createInstance(ScriptCompiler* compiler, 
												   PropertyAbstractNode* prop, Pass* pass, SGScriptTranslator* translator)
{
	if (prop->name == "hardware_skinning")
	{
		bool hasError = false;
		uint32 boneCount = 0;
		uint32 weightCount = 0;
		
		if(prop->values.size() >= 2)
		{
			AbstractNodeList::iterator it = prop->values.begin();
			if(false == SGScriptTranslator::getUInt(*it, &boneCount))
				hasError = true;

			++it;
			if(false == SGScriptTranslator::getUInt(*it, &weightCount))
				hasError = true;
		}

		if (hasError == true)
		{
			compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line, "Expected the format: hardware_skinning <bone count> <weight count>.");
			return NULL;
		}
		else
		{
			//create and update the hardware skinning sub render state
			SubRenderState*	subRenderState = SubRenderStateFactory::createInstance();
			HardwareSkinning* hardSkinSrs = static_cast<HardwareSkinning*>(subRenderState);
			hardSkinSrs->setHardwareSkinningParam(boneCount, weightCount);
			
			//hardware skinning was specificaly asked for in the material
			//so allow for change no matter what
			hardSkinSrs->setAllowSkinningStateChange(true);

			return subRenderState;
		}

	}

	return NULL;
}

//-----------------------------------------------------------------------
void HardwareSkinningFactory::writeInstance(MaterialSerializer* ser, SubRenderState* subRenderState, 
									   Pass* srcPass, Pass* dstPass)
{
	ser->writeAttribute(4, "hardware_skinning");
	
	HardwareSkinning* hardSkinSrs = static_cast<HardwareSkinning*>(subRenderState);
	ser->writeValue(StringConverter::toString(hardSkinSrs->getBoneCount()));
	ser->writeValue(StringConverter::toString(hardSkinSrs->getWeightCount()));
}

//-----------------------------------------------------------------------
SubRenderState*	HardwareSkinningFactory::createInstanceImpl()
{
	HardwareSkinning* pSkin = OGRE_NEW HardwareSkinning;
	pSkin->_setCreator(this);
	return pSkin;
}

//-----------------------------------------------------------------------
void HardwareSkinningFactory::setCustomShadowCasterMaterials(const MaterialPtr& caster1Weight, const MaterialPtr& caster2Weight,
											const MaterialPtr& caster3Weight, const MaterialPtr& caster4Weight)
{
	mCustomShadowCasterMaterials[0] = caster1Weight;
	mCustomShadowCasterMaterials[1] = caster2Weight;
	mCustomShadowCasterMaterials[2] = caster3Weight;
	mCustomShadowCasterMaterials[3] = caster4Weight;
}

//-----------------------------------------------------------------------
void HardwareSkinningFactory::setCustomShadowReceiverMaterials(const MaterialPtr& receiver1Weight, const MaterialPtr& receiver2Weight,
											  const MaterialPtr& receiver3Weight, const MaterialPtr& receiver4Weight)
{
	mCustomShadowReceiverMaterials[0] = receiver1Weight;
	mCustomShadowReceiverMaterials[1] = receiver2Weight;
	mCustomShadowReceiverMaterials[2] = receiver3Weight;
	mCustomShadowReceiverMaterials[3] = receiver4Weight;
}

//-----------------------------------------------------------------------
const MaterialPtr& HardwareSkinningFactory::getCustomShadowCasterMaterial(ushort index) const
{
	assert(index < HS_MAX_WEIGHT_COUNT);
	return mCustomShadowCasterMaterials[index];
}

//-----------------------------------------------------------------------
const MaterialPtr& HardwareSkinningFactory::getCustomShadowReceiverMaterial(ushort index) const
{
	assert(index < HS_MAX_WEIGHT_COUNT);
	return mCustomShadowReceiverMaterials[index];
}


}
}

#endif
