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

namespace Ogre {

namespace RTShader {


/************************************************************************/
/*                                                                      */
/************************************************************************/
bool LinearSkinning::resolveParameters(Program* vsProgram)
{
    Function* vsMain = vsProgram->getEntryPointFunction();
    mParamInWorldMatrices = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLD_MATRIX_ARRAY_3x4, mBoneCount);
    mParamBlendMat = vsMain->resolveLocalParameter(GCT_MATRIX_3X4, "blendMat");
    return true;
}

//-----------------------------------------------------------------------
void LinearSkinning::addPositionCalculations(const FunctionStageRef& stage)
{
    // Construct a scaling and shearing matrix based on the blend weights
    stage.callFunction("blendBonesMat3x4",
                       {In(mParamInWorldMatrices), In(mParamInIndices), In(mParamInWeights), Out(mParamBlendMat)});

    // multiply position with world matrix
    stage.callFunction(FFP_FUNC_TRANSFORM, mParamBlendMat, mParamInPosition, Out(mParamInPosition).xyz());
    // set w value to 1
    stage.assign(1, Out(mParamInPosition).w());
}

//-----------------------------------------------------------------------
void LinearSkinning::addNormalRelatedCalculations(const FunctionStageRef& stage)
{
    // multiply normal with world matrix and put into temporary param
    stage.callFunction(FFP_FUNC_TRANSFORM, mParamBlendMat, mParamInNormal, mParamInNormal);
}
}
}

#endif
