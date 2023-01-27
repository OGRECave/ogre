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
#include "OgreMeshManager.h"

#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS

namespace Ogre {

namespace RTShader {

/************************************************************************/
/*                                                                      */
/************************************************************************/
HardwareSkinningTechnique::HardwareSkinningTechnique() :
    mBoneCount(0),
    mWeightCount(0),
    mCorrectAntipodalityHandling(false),
    mScalingShearingSupport(false),
    mDoBoneCalculations(false),
    mObjSpaceBones(MeshManager::getBonesUseObjectSpace())
{
}

//-----------------------------------------------------------------------
HardwareSkinningTechnique::~HardwareSkinningTechnique()
{
}

//-----------------------------------------------------------------------
void HardwareSkinningTechnique::setHardwareSkinningParam(ushort boneCount, ushort weightCount, bool correctAntipodalityHandling, bool scalingShearingSupport)
{
    mBoneCount = std::min<ushort>(boneCount, OGRE_MAX_NUM_BONES);
    mWeightCount = std::min<ushort>(weightCount, 4);
    mCorrectAntipodalityHandling = correctAntipodalityHandling;
    mScalingShearingSupport = scalingShearingSupport;
}

bool HardwareSkinningTechnique::setParameter(const String& name, const String& value)
{
    if(name == "max_bone_count")
    {
        uint num = 0;
        StringConverter::parse(value, num);
        mBoneCount = std::min<ushort>(num, OGRE_MAX_NUM_BONES);
        return num && num < OGRE_MAX_NUM_BONES;
    }
    else if(name == "weight_count")
    {
        uint num = 0;
        StringConverter::parse(value, num);
        mWeightCount = std::min<ushort>(num, 4);
        return num && num < 5;
    }
    else if(name == "correct_antipodality")
    {
        return StringConverter::parse(value, mCorrectAntipodalityHandling);
    }
    else if(name == "scale_shearing")
    {
        return StringConverter::parse(value, mScalingShearingSupport);
    }

    return false;
}

//-----------------------------------------------------------------------
void HardwareSkinningTechnique::setDoBoneCalculations(bool doBoneCalculations)
{
    mDoBoneCalculations = doBoneCalculations;
}

//-----------------------------------------------------------------------
ushort HardwareSkinningTechnique::getBoneCount()
{
    return mBoneCount;
}

//-----------------------------------------------------------------------
ushort HardwareSkinningTechnique::getWeightCount()
{
    return mWeightCount;
}

//-----------------------------------------------------------------------
bool HardwareSkinningTechnique::hasCorrectAntipodalityHandling()
{
    return mCorrectAntipodalityHandling;
}

//-----------------------------------------------------------------------
bool HardwareSkinningTechnique::hasScalingShearingSupport()
{
    return mScalingShearingSupport;
}
//-----------------------------------------------------------------------
bool HardwareSkinningTechnique::resolveDependencies(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    vsProgram->addDependency(FFP_LIB_TRANSFORM);
    if(mDoBoneCalculations)
    {
        vsProgram->addDependency("SGXLib_DualQuaternion");
        vsProgram->addPreprocessorDefines(StringUtil::format("BONE_COUNT=%d", mBoneCount));
        vsProgram->addPreprocessorDefines(StringUtil::format("WEIGHT_COUNT=%d", mWeightCount));
        if(mCorrectAntipodalityHandling)
            vsProgram->addPreprocessorDefines("CORRECT_ANTIPODALITY");
    }

    return true;
}
bool HardwareSkinningTechnique::resolveParameters(ProgramSet* programSet)
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

    //output param
    mParamOutPositionProj = vsMain->resolveOutputParameter(Parameter::SPC_POSITION_PROJECTIVE_SPACE);

    if (mDoBoneCalculations == true)
    {
        if (ShaderGenerator::getSingleton().getTargetLanguage() == "hlsl")
        {
            //set hlsl shader to use row-major matrices instead of column-major.
            //it enables the use of 3x4 matrices in hlsl shader instead of full 4x4 matrix.
            vsProgram->setUseColumnMajorMatrices(false);
        }

        //input parameters
        mParamInIndices = vsMain->resolveInputParameter(Parameter::SPC_BLEND_INDICES);
        mParamInWeights = vsMain->resolveInputParameter(Parameter::SPC_BLEND_WEIGHTS);
        if(!mObjSpaceBones)
            mParamInInvWorldMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_INVERSE_WORLD_MATRIX);

        resolveParameters(vsProgram);
    }

    mParamInWorldViewProjMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);
    return true;
}
//-----------------------------------------------------------------------
bool HardwareSkinningTechnique::addFunctionInvocations(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Function* vsMain = vsProgram->getEntryPointFunction();

    auto stage = vsMain->getStage(FFP_VS_TRANSFORM);
    //add functions to calculate position data object and projective space
    if (mDoBoneCalculations)
    {
        addPositionCalculations(stage);
        // update back the original position relative to the object
        if (!mObjSpaceBones)
            stage.callFunction(FFP_FUNC_TRANSFORM, mParamInInvWorldMatrix, mParamInPosition, mParamInPosition);
    }

    // update from object to projective space
    stage.callFunction(FFP_FUNC_TRANSFORM, mParamInWorldViewProjMatrix, mParamInPosition, mParamOutPositionProj);

    //add functions to calculate normal and normal related data in world and object space
    if (mDoBoneCalculations && mDoLightCalculations)
    {
        addNormalRelatedCalculations(stage);
        // update back the original position relative to the object
        if (!mObjSpaceBones)
            stage.callFunction(FFP_FUNC_TRANSFORM, mParamInInvWorldMatrix, mParamInNormal, mParamInNormal);
    }
    //addNormalRelatedCalculations(vsMain, mParamInTangent, mParamLocalTangentWorld, internalCounter);
    //addNormalRelatedCalculations(vsMain, mParamInBiNormal, mParamLocalBinormalWorld, internalCounter);
    return true;
}
//-----------------------------------------------------------------------
void HardwareSkinningTechnique::copyFrom(const HardwareSkinningTechnique* hardSkin)
{
    mWeightCount = hardSkin->mWeightCount;
    mBoneCount = hardSkin->mBoneCount;
    mDoBoneCalculations = hardSkin->mDoBoneCalculations;
    mCorrectAntipodalityHandling = hardSkin->mCorrectAntipodalityHandling;
    mScalingShearingSupport = hardSkin->mScalingShearingSupport;
    mObjSpaceBones = hardSkin->mObjSpaceBones;
}

}
}

#endif


