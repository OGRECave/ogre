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

#include "OgreShaderExLinearSkinning.h"
#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
#include "OgreShaderFFPRenderState.h"
#include "OgreShaderProgram.h"
#include "OgreShaderParameter.h"
#include "OgreShaderProgramSet.h"
#include "OgreEntity.h"
#include "OgreSubEntity.h"
#include "OgreMaterial.h"
#include "OgreSubMesh.h"
#include "OgreShaderGenerator.h"

#define HS_DATA_BIND_NAME "HS_SRS_DATA"


namespace Ogre {

namespace RTShader {


/************************************************************************/
/*                                                                      */
/************************************************************************/
LinearSkinning::LinearSkinning() : HardwareSkinningTechnique()
{
}

//-----------------------------------------------------------------------
bool LinearSkinning::resolveParameters(ProgramSet* programSet)
{

	Program* vsProgram = programSet->getCpuVertexProgram();
	Function* vsMain = vsProgram->getEntryPointFunction();

	//if needed mark this vertex program as hardware skinned
	if (mDoBoneCalculations == true)
	{
		vsProgram->setSkeletalAnimationIncluded(true);
	}

	//
	// get the parameters we need whether we are doing bone calculations or not
	//
	// Note: in order to be consistent we will always output position, normal,
	// tangent and binormal in both object and world space. And output position
	// in projective space to cover the responsibility of the transform stage

	//input param
	mParamInPosition = vsMain->resolveInputParameter(Parameter::SPS_POSITION, 0, Parameter::SPC_POSITION_OBJECT_SPACE, GCT_FLOAT4);
	mParamInNormal = vsMain->resolveInputParameter(Parameter::SPS_NORMAL, 0, Parameter::SPC_NORMAL_OBJECT_SPACE, GCT_FLOAT3);
	//mParamInBiNormal = vsMain->resolveInputParameter(Parameter::SPS_BINORMAL, 0, Parameter::SPC_BINORMAL_OBJECT_SPACE, GCT_FLOAT3);
	//mParamInTangent = vsMain->resolveInputParameter(Parameter::SPS_TANGENT, 0, Parameter::SPC_TANGENT_OBJECT_SPACE, GCT_FLOAT3);

	//local param
	mParamLocalPositionWorld = vsMain->resolveLocalParameter(Parameter::SPS_POSITION, 0, Parameter::SPC_POSITION_WORLD_SPACE, GCT_FLOAT4);
	mParamLocalNormalWorld = vsMain->resolveLocalParameter(Parameter::SPS_NORMAL, 0, Parameter::SPC_NORMAL_WORLD_SPACE, GCT_FLOAT3);
	//mParamLocalTangentWorld = vsMain->resolveLocalParameter(Parameter::SPS_TANGENT, 0, Parameter::SPC_TANGENT_WORLD_SPACE, GCT_FLOAT3);
	//mParamLocalBinormalWorld = vsMain->resolveLocalParameter(Parameter::SPS_BINORMAL, 0, Parameter::SPC_BINORMAL_WORLD_SPACE, GCT_FLOAT3);

	//output param
	mParamOutPositionProj = vsMain->resolveOutputParameter(Parameter::SPS_POSITION, 0, Parameter::SPC_POSITION_PROJECTIVE_SPACE, GCT_FLOAT4);

	//check if parameter retrival went well
	bool isValid =
		(mParamInPosition.get() != NULL) &&
		(mParamInNormal.get() != NULL) &&
		//(mParamInBiNormal.get() != NULL) &&
		//(mParamInTangent.get() != NULL) &&
		(mParamLocalPositionWorld.get() != NULL) &&
		(mParamLocalNormalWorld.get() != NULL) &&
		//(mParamLocalTangentWorld.get() != NULL) &&
		//(mParamLocalBinormalWorld.get() != NULL) &&
		(mParamOutPositionProj.get() != NULL);


	if (mDoBoneCalculations == true)
	{
		if (ShaderGenerator::getSingleton().getTargetLanguage() == "hlsl")
		{
			//set hlsl shader to use row-major matrices instead of column-major.
			//it enables the use of 3x4 matrices in hlsl shader instead of full 4x4 matrix.
			vsProgram->setUseColumnMajorMatrices(false);
		}

		//input parameters
		mParamInNormal = vsMain->resolveInputParameter(Parameter::SPS_NORMAL, 0, Parameter::SPC_NORMAL_OBJECT_SPACE, GCT_FLOAT3);
		//mParamInBiNormal = vsMain->resolveInputParameter(Parameter::SPS_BINORMAL, 0, Parameter::SPC_BINORMAL_OBJECT_SPACE, GCT_FLOAT3);
		//mParamInTangent = vsMain->resolveInputParameter(Parameter::SPS_TANGENT, 0, Parameter::SPC_TANGENT_OBJECT_SPACE, GCT_FLOAT3);
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
bool LinearSkinning::resolveDependencies(ProgramSet* programSet)
{
	Program* vsProgram = programSet->getCpuVertexProgram();
	vsProgram->addDependency(FFP_LIB_COMMON);
	vsProgram->addDependency(FFP_LIB_TRANSFORM);

	return true;
}

//-----------------------------------------------------------------------
bool LinearSkinning::addFunctionInvocations(ProgramSet* programSet)
{

	Program* vsProgram = programSet->getCpuVertexProgram();
	Function* vsMain = vsProgram->getEntryPointFunction();
	int internalCounter = 0;

	//add functions to calculate position data in world, object and projective space
	addPositionCalculations(vsMain, internalCounter);

	//add functions to calculate normal and normal related data in world and object space
	addNormalRelatedCalculations(vsMain, mParamInNormal, mParamLocalNormalWorld, internalCounter);
	//addNormalRelatedCalculations(vsMain, mParamInTangent, mParamLocalTangentWorld, internalCounter);
	//addNormalRelatedCalculations(vsMain, mParamInBiNormal, mParamLocalBinormalWorld, internalCounter);
	return true;
}

//-----------------------------------------------------------------------
void LinearSkinning::addPositionCalculations(Function* vsMain, int& funcCounter)
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
void LinearSkinning::addNormalRelatedCalculations(Function* vsMain,
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
void LinearSkinning::addIndexedPositionWeight(Function* vsMain,
								int index, int& funcCounter)
{
	Operand::OpMask indexMask = indexToMask(index);

	FunctionInvocation* curFuncInvocation;

	//multiply position with world matrix and put into temporary param
	curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_TRANSFORM, FFP_VS_TRANSFORM, funcCounter++);
	curFuncInvocation->pushOperand(mParamInWorldMatrices, Operand::OPS_IN);
	curFuncInvocation->pushOperand(mParamInIndices, Operand::OPS_IN,  indexMask, 1);
	curFuncInvocation->pushOperand(mParamInPosition, Operand::OPS_IN);
	curFuncInvocation->pushOperand(mParamTempFloat4, Operand::OPS_OUT, Operand::OPM_XYZ);
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
		curFuncInvocation->pushOperand(mParamTempFloat4, Operand::OPS_IN);
		curFuncInvocation->pushOperand(mParamLocalPositionWorld, Operand::OPS_IN);
		curFuncInvocation->pushOperand(mParamLocalPositionWorld, Operand::OPS_OUT);
		vsMain->addAtomInstance(curFuncInvocation);
	}
}


//-----------------------------------------------------------------------
void LinearSkinning::addIndexedNormalRelatedWeight(Function* vsMain,
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
	curFuncInvocation->pushOperand(mParamTempFloat3, Operand::OPS_OUT);
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
		curFuncInvocation->pushOperand(mParamTempFloat3, Operand::OPS_IN);
		curFuncInvocation->pushOperand(pNormalWorldRelatedParam, Operand::OPS_IN);
		curFuncInvocation->pushOperand(pNormalWorldRelatedParam, Operand::OPS_OUT);
		vsMain->addAtomInstance(curFuncInvocation);
	}
}

}
}

#endif