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

#define SGX_LIB_DUAL_QUATERNION                 "SGXLib_DualQuaternion"
#define SGX_FUNC_CALCULATE_BLEND_POSITION       "SGX_CalculateBlendPosition"
#define SGX_FUNC_CALCULATE_BLEND_NORMAL         "SGX_CalculateBlendNormal"
#define SGX_FUNC_ADJOINT_TRANSPOSE_MATRIX       "SGX_AdjointTransposeMatrix"

namespace Ogre {

namespace RTShader {

/************************************************************************/
/*                                                                      */
/************************************************************************/
bool DualQuaternionSkinning::resolveParameters(Program* vsProgram)
{
    Function* vsMain = vsProgram->getEntryPointFunction();

    mParamInWorldMatrices = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLD_DUALQUATERNION_ARRAY_2x4, mBoneCount);
    mParamBlendDQ = vsMain->resolveLocalParameter(GCT_MATRIX_2X4, "blendDQ");

    if(mScalingShearingSupport)
    {
        mParamInScaleShearMatrices = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLD_SCALE_SHEAR_MATRIX_ARRAY_3x4, mBoneCount);
        mParamBlendS = vsMain->resolveLocalParameter(GCT_MATRIX_3X4, "blendS");
        mParamTempFloat3x3 = vsMain->resolveLocalParameter(GCT_MATRIX_3X3, "TempVal3x3");
    }
    return true;
}
//-----------------------------------------------------------------------
void DualQuaternionSkinning::addPositionCalculations(const FunctionStageRef& stage)
{
    if (mScalingShearingSupport)
    {
        // Construct a scaling and shearing matrix based on the blend weights
        stage.callFunction("blendBonesMat3x4", {In(mParamInScaleShearMatrices), In(mParamInIndices),
                                                In(mParamInWeights), Out(mParamBlendS)});
        // Transform the position based by the scaling and shearing matrix
        stage.callFunction(FFP_FUNC_TRANSFORM, mParamBlendS, mParamInPosition, Out(mParamInPosition).xyz());
    }

    // Calculate the resultant dual quaternion based on the weights given
    stage.callFunction("blendBonesDQ",
                       {In(mParamInWorldMatrices), In(mParamInIndices), In(mParamInWeights), Out(mParamBlendDQ)});

    // Calculate the blend position
    stage.callFunction(SGX_FUNC_CALCULATE_BLEND_POSITION, In(mParamInPosition).xyz(), mParamBlendDQ, mParamInPosition);
}

//-----------------------------------------------------------------------
void DualQuaternionSkinning::addNormalRelatedCalculations(const FunctionStageRef& stage)
{
    if (mScalingShearingSupport)
    {
        // Calculate the adjoint transpose of the blended scaling and shearing matrix
        stage.callFunction(SGX_FUNC_ADJOINT_TRANSPOSE_MATRIX, mParamBlendS, mParamTempFloat3x3);
        // Transform the normal by the adjoint transpose of the blended scaling and shearing matrix
        stage.callBuiltin("mul", mParamTempFloat3x3, mParamInNormal, mParamInNormal);
        // Need to normalize again after transforming the normal
        stage.callBuiltin("normalize", mParamInNormal, mParamInNormal);
    }
    // Transform the normal according to the dual quaternion
    stage.callFunction(SGX_FUNC_CALCULATE_BLEND_NORMAL, mParamInNormal, mParamBlendDQ, mParamInNormal);
}
}
}

#endif



