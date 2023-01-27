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
#ifndef _ShaderExHardwareSkinningTechnique_
#define _ShaderExHardwareSkinningTechnique_

#include "OgreShaderPrerequisites.h"

#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
#include "OgreShaderFunctionAtom.h"
#include "OgreShaderExHardwareSkinning.h"

namespace Ogre {
namespace RTShader {

/** \addtogroup Optional
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

/** Implement a sub render state which performs hardware skinning.
Meaning, this sub render states adds calculations which multiply
the points and normals by their assigned bone matricies.
*/
class HardwareSkinningTechnique : public RTShaderSystemAlloc
{
// Interface.
public:
    /** Class default constructor */
    HardwareSkinningTechnique();

    virtual ~HardwareSkinningTechnique();

    /**
    @see SubRenderState::copyFrom.
    */
    virtual void copyFrom(const HardwareSkinningTechnique* hardSkin);

    void setHardwareSkinningParam(ushort boneCount, ushort weightCount, bool correctAntipodalityHandling, bool scalingShearingSupport);

    virtual bool setParameter(const String& name, const String& value);

    /**
    Returns the number of bones in the model assigned to the material.
    @see setHardwareSkinningParam()
    */
    ushort getBoneCount();

    /**
    Returns the number of weights/bones affecting a vertex.
    @see setHardwareSkinningParam()
    */
    ushort getWeightCount();

    /**
    Only applicable for dual quaternion skinning.
    @see setHardwareSkinningParam()
    */
    bool hasCorrectAntipodalityHandling();

    /**
    Only applicable for dual quaternion skinning.
    @see setHardwareSkinningParam()
    */
    bool hasScalingShearingSupport();


    void setDoBoneCalculations(bool doBoneCalculations);
    void setDoLightCalculations(bool val) { mDoLightCalculations = val; }

    bool resolveParameters(ProgramSet* programSet);
    bool resolveDependencies(ProgramSet* programSet);

    bool addFunctionInvocations(ProgramSet* programSet);

private:
    /** Adds functions to calculate position data in world, object and projective space */
    virtual void addPositionCalculations(const FunctionStageRef& stage) = 0;

    /** Adds the calculations for calculating a normal related element */
    virtual void addNormalRelatedCalculations(const FunctionStageRef& stage) = 0;

    virtual bool resolveParameters(Program* vsProgram) = 0;
// Attributes.
protected:
    ushort mBoneCount;
    ushort mWeightCount;

    bool mCorrectAntipodalityHandling;
    bool mScalingShearingSupport;

    bool mDoBoneCalculations;
    bool mDoLightCalculations;
    
    bool mObjSpaceBones;

    ParameterPtr mParamInPosition;
    ParameterPtr mParamInNormal;
    //ParameterPtr mParamInBiNormal;
    //ParameterPtr mParamInTangent;
    ParameterPtr mParamInIndices;
    ParameterPtr mParamInWeights;
    UniformParameterPtr mParamInWorldMatrices;
    UniformParameterPtr mParamInInvWorldMatrix;
    UniformParameterPtr mParamInWorldViewProjMatrix;

    ParameterPtr mParamOutPositionProj;
};

class DualQuaternionSkinning;
class LinearSkinning;

/** Implement a sub render state which performs hardware skinning.
Meaning, this sub render states adds calculations which multiply
the points and normals by their assigned bone matricies.
*/
class HardwareSkinning : public SubRenderState
{
public:
    struct SkinningData
    {
        SkinningData()
            : isValid(true), maxBoneCount(0), maxWeightCount(0), skinningType(ST_LINEAR),
              correctAntipodalityHandling(false), scalingShearingSupport(false)
        {
        }

        bool isValid;
        ushort maxBoneCount;
        ushort maxWeightCount;
        SkinningType skinningType;
        bool correctAntipodalityHandling;
        bool scalingShearingSupport;
    };

// Interface.
public:
    /** Class default constructor */
    HardwareSkinning();

    /**
    @see SubRenderState::getType.
    */
    const String& getType() const override;

    /**
    @see SubRenderState::getType.
    */
    int getExecutionOrder() const override;

    /**
    @see SubRenderState::copyFrom.
    */
    void copyFrom(const SubRenderState& rhs) override;

    /**
    Set the hardware skinning parameters.
    @param boneCount The maximum number of bones in the model this material
         is assigned to. Note that this parameter can be higher but not
         lower than the actual number of bones.
    @param weightCount The maximum number of weights/bones affecting a vertex.
    @param skinningType The type of skinning desired.
    @param correctAntipodalityHandling If correct antipodality handling should be utilized (Only applicable for dual quaternion skinning).
    @param scalingShearingSupport If scaling and shearing support should be enabled (Only applicable for dual quaternion skinning).
    */
    bool setParameter(const String& name, const String& value) override;

    /**
    Returns the number of bones in the model assigned to the material.
    @see setHardwareSkinningParam()
    */
    ushort getBoneCount();

    /**
    Returns the number of weights/bones affecting a vertex.
    @see setHardwareSkinningParam()
    */
    ushort getWeightCount();

    /**
    Returns the current skinning type in use.
    @see setHardwareSkinningParam()
     */
    SkinningType getSkinningType();

    /**
    Only applicable for dual quaternion skinning.
    @see setHardwareSkinningParam()
    */
    bool hasCorrectAntipodalityHandling();

    /**
    Only applicable for dual quaternion skinning.
    @see setHardwareSkinningParam()
    */
    bool hasScalingShearingSupport();

    /**
    @see SubRenderState::preAddToRenderState.
    */
    bool preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass) override;

    /**
    Set the factory which created this sub render state
    */
    void _setCreator(const HardwareSkinningFactory* pCreator) { mCreator = pCreator; }

    static String Type;

// Protected methods
protected:
    /**
    @see SubRenderState::resolveParameters.
    */
    bool resolveParameters(ProgramSet* programSet) override;

    /**
    @see SubRenderState::resolveDependencies.
    */
    bool resolveDependencies(ProgramSet* programSet) override;

    /**
    @see SubRenderState::addFunctionInvocations.
    */
    bool addFunctionInvocations(ProgramSet* programSet) override;

    SharedPtr<LinearSkinning> mLinear;
    SharedPtr<DualQuaternionSkinning> mDualQuat;
    SharedPtr<HardwareSkinningTechnique> mActiveTechnique;

    ///The factory which created this sub render state
    const HardwareSkinningFactory* mCreator;
    SkinningType mSkinningType;
};

}
}

#endif
#endif

