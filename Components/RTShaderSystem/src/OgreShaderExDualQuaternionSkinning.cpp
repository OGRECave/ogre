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

#include "OgreShaderPrecompiledHeaders.h"

#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
#define HS_DATA_BIND_NAME "HS_SRS_DATA"


namespace Ogre {

namespace RTShader {

/************************************************************************/
/*                                                                      */
/************************************************************************/
DualQuaternionSkinning::DualQuaternionSkinning() : HardwareSkinningTechnique()
{
}

//-----------------------------------------------------------------------
bool DualQuaternionSkinning::resolveParameters(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
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
    mParamInPosition = vsMain->resolveInputParameter(Parameter::SPC_POSITION_OBJECT_SPACE);

    if(mDoLightCalculations)
        mParamInNormal = vsMain->resolveInputParameter(Parameter::SPC_NORMAL_OBJECT_SPACE);
    //mParamInBiNormal = vsMain->resolveInputParameter(Parameter::SPS_BINORMAL, 0, Parameter::SPC_BINORMAL_OBJECT_SPACE, GCT_FLOAT3);
    //mParamInTangent = vsMain->resolveInputParameter(Parameter::SPS_TANGENT, 0, Parameter::SPC_TANGENT_OBJECT_SPACE, GCT_FLOAT3);

    //local param
    mParamLocalBlendPosition = vsMain->resolveLocalParameter(GCT_FLOAT3, "BlendedPosition");
    mParamLocalNormalWorld = vsMain->resolveLocalParameter(Parameter::SPC_NORMAL_WORLD_SPACE);
    //mParamLocalTangentWorld = vsMain->resolveLocalParameter(Parameter::SPS_TANGENT, 0, Parameter::SPC_TANGENT_WORLD_SPACE, GCT_FLOAT3);
    //mParamLocalBinormalWorld = vsMain->resolveLocalParameter(Parameter::SPS_BINORMAL, 0, Parameter::SPC_BINORMAL_WORLD_SPACE, GCT_FLOAT3);

    //output param
    mParamOutPositionProj = vsMain->resolveOutputParameter(Parameter::SPC_POSITION_PROJECTIVE_SPACE);

    if (mDoBoneCalculations == true)
    {
        //input parameters
        mParamInIndices = vsMain->resolveInputParameter(Parameter::SPC_BLEND_INDICES);
        mParamInWeights = vsMain->resolveInputParameter(Parameter::SPC_BLEND_WEIGHTS);
        mParamInWorldMatrices = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLD_DUALQUATERNION_ARRAY_2x4, mBoneCount);
        mParamInInvWorldMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_INVERSE_WORLD_MATRIX);
        mParamInViewProjMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_VIEWPROJ_MATRIX);
        
        mParamTempWorldMatrix = vsMain->resolveLocalParameter(GCT_MATRIX_2X4, "worldMatrix");
        mParamBlendDQ = vsMain->resolveLocalParameter(GCT_MATRIX_2X4, "blendDQ");
        mParamInitialDQ = vsMain->resolveLocalParameter(GCT_MATRIX_2X4, "initialDQ");

        if (ShaderGenerator::getSingleton().getTargetLanguage() == "hlsl")
        {
            //set hlsl shader to use row-major matrices instead of column-major.
            //it enables the use of auto-bound 3x4 matrices in generated hlsl shader.
            vsProgram->setUseColumnMajorMatrices(false);
        }

        if(mScalingShearingSupport)
        {
            mParamInScaleShearMatrices = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLD_SCALE_SHEAR_MATRIX_ARRAY_3x4, mBoneCount);
            mParamBlendS = vsMain->resolveLocalParameter(GCT_MATRIX_3X4, "blendS");
            mParamTempFloat3x3 = vsMain->resolveLocalParameter(GCT_MATRIX_3X3, "TempVal3x3");
            mParamTempFloat3x4 = vsMain->resolveLocalParameter(GCT_MATRIX_3X4, "TempVal3x4");
        }
        
        mParamTempFloat2x4 = vsMain->resolveLocalParameter(GCT_MATRIX_2X4, "TempVal2x4");
        mParamTempFloat4 = vsMain->resolveLocalParameter(GCT_FLOAT4, "TempVal4");
        mParamTempFloat3 = vsMain->resolveLocalParameter(GCT_FLOAT3, "TempVal3");
    }
    else
    {
        mParamInWorldMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLD_MATRIX);
        mParamInWorldViewProjMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);
    }
    return true;
}

//-----------------------------------------------------------------------
bool DualQuaternionSkinning::resolveDependencies(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    vsProgram->addDependency(FFP_LIB_COMMON);
    vsProgram->addDependency(FFP_LIB_TRANSFORM);
    if(mDoBoneCalculations)
        vsProgram->addDependency(SGX_LIB_DUAL_QUATERNION);

    return true;
}

//-----------------------------------------------------------------------
bool DualQuaternionSkinning::addFunctionInvocations(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Function* vsMain = vsProgram->getEntryPointFunction();

    //add functions to calculate position data in world, object and projective space
    addPositionCalculations(vsMain);

    //add functions to calculate normal and normal related data in world and object space
    if(mDoLightCalculations)
        addNormalRelatedCalculations(vsMain, mParamInNormal, mParamLocalNormalWorld);
    //addNormalRelatedCalculations(vsMain, mParamInTangent, mParamLocalTangentWorld, internalCounter);
    //addNormalRelatedCalculations(vsMain, mParamInBiNormal, mParamLocalBinormalWorld, internalCounter);

    return true;
}

//-----------------------------------------------------------------------
void DualQuaternionSkinning::addPositionCalculations(Function* vsMain)
{
    auto stage = vsMain->getStage(FFP_VS_TRANSFORM);

    if (mDoBoneCalculations == true)
    {
        if(mScalingShearingSupport)
        {
            //Construct a scaling and shearing matrix based on the blend weights
            for(int i = 0 ; i < getWeightCount() ; ++i)
            {
                //Assign the local param based on the current index of the scaling and shearing matrices
                stage.assign({In(mParamInScaleShearMatrices), At(mParamInIndices).mask(indexToMask(i)),
                              Out(mParamTempFloat3x4)});

                //Calculate the resultant scaling and shearing matrix based on the weights given
                addIndexedPositionWeight(vsMain, i, mParamTempFloat3x4, mParamTempFloat3x4, mParamBlendS);
            }

            //Transform the position based by the scaling and shearing matrix
            stage.callFunction(FFP_FUNC_TRANSFORM, mParamBlendS, mParamInPosition, mParamLocalBlendPosition);
        }
        else
        {
            //Assign the input position to the local blended position
            stage.assign(In(mParamInPosition).xyz(), mParamLocalBlendPosition);
        }
        
        //Set functions to calculate world position
        for(int i = 0 ; i < getWeightCount() ; ++i)
        {
            // Build the dual quaternion matrix
            stage.assign(
                {In(mParamInWorldMatrices), At(mParamInIndices).mask(indexToMask(i)), Out(mParamTempFloat2x4)});

            //Adjust the podalities of the dual quaternions
            if(mCorrectAntipodalityHandling)
            {   
                adjustForCorrectAntipodality(vsMain, i, mParamTempFloat2x4);
            }

            //Calculate the resultant dual quaternion based on the weights given
            addIndexedPositionWeight(vsMain, i, mParamTempFloat2x4, mParamTempFloat2x4, mParamBlendDQ);
        }

        //Normalize the dual quaternion
        stage.callFunction(SGX_FUNC_NORMALIZE_DUAL_QUATERNION, mParamBlendDQ);

        //Calculate the blend position
        stage.callFunction(SGX_FUNC_CALCULATE_BLEND_POSITION, mParamLocalBlendPosition, mParamBlendDQ, mParamTempFloat4);

        //Update from object to projective space
        stage.callFunction(FFP_FUNC_TRANSFORM, mParamInViewProjMatrix, mParamTempFloat4, mParamOutPositionProj);

        //update back the original position relative to the object
        stage.callFunction(FFP_FUNC_TRANSFORM, mParamInInvWorldMatrix, mParamTempFloat4,
                           mParamInPosition);
    }
    else
    {
        //update from object to projective space
        stage.callFunction(FFP_FUNC_TRANSFORM, mParamInWorldViewProjMatrix, mParamInPosition, mParamOutPositionProj);
    }
}

//-----------------------------------------------------------------------
void DualQuaternionSkinning::addNormalRelatedCalculations(Function* vsMain,
                                ParameterPtr& pNormalRelatedParam,
                                ParameterPtr& pNormalWorldRelatedParam)
{
    auto stage = vsMain->getStage(FFP_VS_TRANSFORM);

    if (mDoBoneCalculations == true)
    {
        if(mScalingShearingSupport)
        {
            //Calculate the adjoint transpose of the blended scaling and shearing matrix
            stage.callFunction(SGX_FUNC_ADJOINT_TRANSPOSE_MATRIX, mParamBlendS, mParamTempFloat3x3);
            //Transform the normal by the adjoint transpose of the blended scaling and shearing matrix
            stage.callFunction(FFP_FUNC_TRANSFORM, mParamTempFloat3x3, pNormalRelatedParam, pNormalRelatedParam);
            //Need to normalize again after transforming the normal
            stage.callFunction(FFP_FUNC_NORMALIZE, pNormalRelatedParam);
        }
        //Transform the normal according to the dual quaternion
        stage.callFunction(SGX_FUNC_CALCULATE_BLEND_NORMAL, pNormalRelatedParam, mParamBlendDQ, pNormalWorldRelatedParam);
        //update back the original position relative to the object
        stage.callFunction(FFP_FUNC_TRANSFORM, mParamInInvWorldMatrix, pNormalWorldRelatedParam, pNormalRelatedParam);
    }
    else
    {
        //update from object to world space
        stage.callFunction(FFP_FUNC_TRANSFORM, mParamInWorldMatrix, pNormalRelatedParam, pNormalWorldRelatedParam);
    }
}

//-----------------------------------------------------------------------
void DualQuaternionSkinning::adjustForCorrectAntipodality(Function* vsMain,
                                int index, const ParameterPtr& pTempWorldMatrix)
{
    auto stage = vsMain->getStage(FFP_VS_TRANSFORM);
    
    //Antipodality doesn't need to be adjusted for dq0 on itself (used as the basis of antipodality calculations)
    if(index > 0)
    {
        //This is the base dual quaternion dq0, which the antipodality calculations are based on
        stage.callFunction(SGX_FUNC_ANTIPODALITY_ADJUSTMENT, mParamInitialDQ, mParamTempFloat2x4, pTempWorldMatrix);
    }
    else if(index == 0)
    {   
        //Set the first dual quaternion as the initial dq
        stage.assign(mParamTempFloat2x4, mParamInitialDQ);
    }
}

//-----------------------------------------------------------------------
void DualQuaternionSkinning::addIndexedPositionWeight(Function* vsMain, int index,
                                ParameterPtr& pWorldMatrix, ParameterPtr& pPositionTempParameter,
                                ParameterPtr& pPositionRelatedOutputParam)
{
    auto stage = vsMain->getStage(FFP_VS_TRANSFORM);

    //multiply position with world matrix and put into temporary param
    stage.mul(In(mParamInWeights).mask(indexToMask(index)), pWorldMatrix, pPositionTempParameter);

    //check if on first iteration
    if (index == 0)
    {
        //set the local param as the value of the world param
        stage.assign(pPositionTempParameter, pPositionRelatedOutputParam);
    }
    else
    {
        //add the local param as the value of the world param
        stage.add(pPositionTempParameter, pPositionRelatedOutputParam, pPositionRelatedOutputParam);
    }
}

}
}

#endif



