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
	mGroupOrder((int)FFP_VS_TRANSFORM - 10)
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
	return FFP_TRANSFORM - 10;
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
bool HardwareSkinning::preAddToRenderState(RenderState* renderState, Pass* srcPass, Pass* dstPass)
{
	return 
		(mBoneCount != 0) && (mBoneCount <= 256) &&
		(mWeightCount != 0) && (mWeightCount <= 4) &&
		((mAllowStateChange == true) ||
		((srcPass->hasVertexProgram() == true) &&
		(srcPass->getVertexProgram()->isSkeletalAnimationIncluded() == true)));
}

//-----------------------------------------------------------------------
bool HardwareSkinning::resolveParameters(ProgramSet* programSet)
{
	
	Program* vsProgram = programSet->getCpuVertexProgram();
	Function* vsMain = vsProgram->getEntryPointFunction();

	vsProgram->setSkeletalAnimationIncluded(true);
	
	//input parameters
	mParamInPosition = vsMain->resolveInputParameter(Parameter::SPS_POSITION, 0, Parameter::SPC_POSITION_OBJECT_SPACE, GCT_FLOAT4);	
	mParamInNormal = vsMain->resolveInputParameter(Parameter::SPS_NORMAL, 0, Parameter::SPC_NORMAL_OBJECT_SPACE, GCT_FLOAT3);	
	mParamInBiNormal = vsMain->resolveInputParameter(Parameter::SPS_BINORMAL, 0, Parameter::SPC_BINORMAL, GCT_FLOAT3);	
	mParamInTangent = vsMain->resolveInputParameter(Parameter::SPS_TANGENT, 0, Parameter::SPC_TANGENT, GCT_FLOAT3);	
	mParamInIndices = vsMain->resolveInputParameter(Parameter::SPS_BLEND_INDICES, 0, Parameter::SPC_UNKNOWN, GCT_FLOAT4);
	mParamInWeights = vsMain->resolveInputParameter(Parameter::SPS_BLEND_WEIGHTS, 0, Parameter::SPC_UNKNOWN, GCT_FLOAT4);
	mParamInWorldMatrices = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_WORLD_MATRIX_ARRAY_3x4, 0, mBoneCount);
	mParamInInvWorldMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_INVERSE_WORLD_MATRIX, 0);
	
	mParamTempFloat4 = vsMain->resolveLocalParameter(Parameter::SPS_UNKNOWN, -1, "TempVal4", GCT_FLOAT4);	
	mParamAggregFloat4 = vsMain->resolveLocalParameter(Parameter::SPS_UNKNOWN, -1, "TempAggreg4", GCT_FLOAT4);	
	mParamTempFloat3 = vsMain->resolveLocalParameter(Parameter::SPS_UNKNOWN, -1, "TempVal3", GCT_FLOAT3);	
	mParamAggregFloat3 = vsMain->resolveLocalParameter(Parameter::SPS_UNKNOWN, -1, "TempAggreg3", GCT_FLOAT3);	
	
	//check if parameter retrival went well
	bool isValid = 
		(mParamInPosition.get() != NULL) &&
		(mParamInNormal.get() != NULL) &&
		(mParamInBiNormal.get() != NULL) &&
		(mParamInTangent.get() != NULL) &&
		(mParamInIndices.get() != NULL) &&
		(mParamInWeights.get() != NULL) &&
		(mParamInWorldMatrices.get() != NULL) &&
		(mParamInInvWorldMatrix.get() != NULL) &&
		(mParamTempFloat4.get() != NULL) &&
		(mParamAggregFloat4.get() != NULL) &&
		(mParamTempFloat3.get() != NULL) &&
		(mParamAggregFloat3.get() != NULL);
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
	FunctionInvocation* curFuncInvocation = NULL;	
	int internalCounter = 0;

	//set functions to calculate world position
	for(int i = 0 ; i < getWeightCount() ; ++i)
	{
		addIndexedPositionWeight(vsMain, i, internalCounter);
	}

	//update back the original position relative to the object
	curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_TRANSFORM, mGroupOrder, internalCounter++); 								
	curFuncInvocation->pushOperand(mParamInInvWorldMatrix, Operand::OPS_IN);
	curFuncInvocation->pushOperand(mParamAggregFloat4, Operand::OPS_IN);
	curFuncInvocation->pushOperand(mParamInPosition, Operand::OPS_OUT);
	vsMain->addAtomInstace(curFuncInvocation);	

	addNormalRelatedCalculations(vsMain, mParamInNormal, internalCounter);
	addNormalRelatedCalculations(vsMain, mParamInBiNormal, internalCounter);
	addNormalRelatedCalculations(vsMain, mParamInTangent, internalCounter);
	return true;
}

//-----------------------------------------------------------------------
void HardwareSkinning::addNormalRelatedCalculations(Function* vsMain,
								ParameterPtr& pNormalParam, 
								int& funcCounter)
{
	//set functions to calculate world normal
	for(int i = 0 ; i < getWeightCount() ; ++i)
	{
		addIndexedNormalRelatedWeight(vsMain, pNormalParam, i, funcCounter);
	}

	//update back the original position relative to the object
	FunctionInvocation* curFuncInvocation;
	curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_TRANSFORM, mGroupOrder, funcCounter++); 								
	curFuncInvocation->pushOperand(mParamInInvWorldMatrix, Operand::OPS_IN);
	curFuncInvocation->pushOperand(mParamAggregFloat3, Operand::OPS_IN);
	curFuncInvocation->pushOperand(pNormalParam, Operand::OPS_OUT);
	vsMain->addAtomInstace(curFuncInvocation);	

}

//-----------------------------------------------------------------------
void HardwareSkinning::addIndexedPositionWeight(Function* vsMain, 
								int index, int& funcCounter)
{
	Operand::OpMask indexMask = indexToMask(index);
	
	FunctionInvocation* curFuncInvocation;

	//multiply position with world matrix and put into temporary param
	curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_TRANSFORM, mGroupOrder, funcCounter++); 								
	curFuncInvocation->pushOperand(mParamInWorldMatrices, Operand::OPS_IN);
	curFuncInvocation->pushOperand(mParamInIndices, Operand::OPS_IN,  indexMask, 1);
	curFuncInvocation->pushOperand(mParamInPosition, Operand::OPS_IN);	
	curFuncInvocation->pushOperand(mParamTempFloat4, Operand::OPS_OUT, Operand::OPM_X | Operand::OPM_Y | Operand::OPM_Z);	
	vsMain->addAtomInstace(curFuncInvocation);	

	//set w value of temporary param to 1
	curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ASSIGN, mGroupOrder, funcCounter++); 								
	curFuncInvocation->pushOperand(ParameterFactory::createConstParamFloat(1.0f), Operand::OPS_IN);
	curFuncInvocation->pushOperand(mParamTempFloat4, Operand::OPS_OUT, Operand::OPM_W);	
	vsMain->addAtomInstace(curFuncInvocation);	

	//multiply temporary param with  weight
	curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_MODULATE, mGroupOrder, funcCounter++); 								
	curFuncInvocation->pushOperand(mParamTempFloat4, Operand::OPS_IN);	
	curFuncInvocation->pushOperand(mParamInWeights, Operand::OPS_IN, indexMask);
	curFuncInvocation->pushOperand(mParamTempFloat4, Operand::OPS_OUT);	
	vsMain->addAtomInstace(curFuncInvocation);	

	//check if on first iteration
	if (index == 0)
	{
		//set the local param as the value of the world param
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ASSIGN, mGroupOrder, funcCounter++); 								
		curFuncInvocation->pushOperand(mParamTempFloat4, Operand::OPS_IN);
		curFuncInvocation->pushOperand(mParamAggregFloat4, Operand::OPS_OUT);	
		vsMain->addAtomInstace(curFuncInvocation);	
	}
	else
	{
		//add the local param as the value of the world param
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ADD, mGroupOrder, funcCounter++); 								
		curFuncInvocation->pushOperand(mParamAggregFloat4, Operand::OPS_IN);	
		curFuncInvocation->pushOperand(mParamTempFloat4, Operand::OPS_IN);
		curFuncInvocation->pushOperand(mParamAggregFloat4, Operand::OPS_OUT);	
		vsMain->addAtomInstace(curFuncInvocation);	
	}
}


//-----------------------------------------------------------------------
void HardwareSkinning::addIndexedNormalRelatedWeight(Function* vsMain,
								ParameterPtr& pNormalParam, 
								int index, int& funcCounter)
{

	FunctionInvocation* curFuncInvocation;

	Operand::OpMask indexMask = indexToMask(index);
	
	//multiply position with world matrix and put into temporary param
	curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_TRANSFORM, mGroupOrder, funcCounter++); 								
	curFuncInvocation->pushOperand(mParamInWorldMatrices, Operand::OPS_IN, Operand::OPM_ALL);
	curFuncInvocation->pushOperand(mParamInIndices, Operand::OPS_IN,  indexMask, 1);
	curFuncInvocation->pushOperand(pNormalParam, Operand::OPS_IN);	
	curFuncInvocation->pushOperand(mParamTempFloat3, Operand::OPS_OUT, Operand::OPM_X | Operand::OPM_Y | Operand::OPM_Z);	
	vsMain->addAtomInstace(curFuncInvocation);	

	//multiply temporary param with weight
	curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_MODULATE, mGroupOrder, funcCounter++); 								
	curFuncInvocation->pushOperand(mParamTempFloat3, Operand::OPS_IN);	
	curFuncInvocation->pushOperand(mParamInWeights, Operand::OPS_IN, indexMask);
	curFuncInvocation->pushOperand(mParamTempFloat3, Operand::OPS_OUT);	
	vsMain->addAtomInstace(curFuncInvocation);	

	//check if on first iteration
	if (index == 0)
	{
		//set the local param as the value of the world normal
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ASSIGN, mGroupOrder, funcCounter++); 								
		curFuncInvocation->pushOperand(mParamTempFloat3, Operand::OPS_IN);
		curFuncInvocation->pushOperand(mParamAggregFloat3, Operand::OPS_OUT);	
		vsMain->addAtomInstace(curFuncInvocation);	
	}
	else
	{
		//add the local param as the value of the world normal
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ADD, mGroupOrder, funcCounter++); 								
		curFuncInvocation->pushOperand(mParamAggregFloat3, Operand::OPS_IN);	
		curFuncInvocation->pushOperand(mParamTempFloat3, Operand::OPS_IN);
		curFuncInvocation->pushOperand(mParamAggregFloat3, Operand::OPS_OUT);	
		vsMain->addAtomInstace(curFuncInvocation);	
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
		bool hasError = true;
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
	return OGRE_NEW HardwareSkinning;
}


}
}

#endif
