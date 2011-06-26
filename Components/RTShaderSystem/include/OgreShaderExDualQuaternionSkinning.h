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
#ifndef _ShaderExDualQuaternionSkinning_
#define _ShaderExDualQuaternionSkinning_

#include "OgreShaderPrerequisites.h"
#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
#include "OgreShaderSubRenderState.h"
#include "OgreShaderParameter.h"
#include "OgreRenderSystem.h"
#include "OgreShaderFunctionAtom.h"
#include "OgreShaderExHardwareSkinning.h"

#define HS_MAX_WEIGHT_COUNT 4
namespace Ogre {
namespace RTShader {

/** \addtogroup Core
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

#define SGX_LIB_DUAL_QUATERNION				"SGXLib_DualQuaternion"
#define SGX_FUNC_BLEND_WEIGHT				"SGX_BlendWeight"
#define SGX_FUNC_ANTIPODALITY_ADJUSTMENT		"SGX_AntipodalityAdjustment"
#define SGX_FUNC_CALCULATE_BLEND_POSITION		"SGX_CalculateBlendPosition"
#define SGX_FUNC_CALCULATE_BLEND_NORMAL			"SGX_CalculateBlendNormal"
#define SGX_FUNC_NORMALIZE_DUAL_QUATERNION		"SGX_NormalizeDualQuaternion"
#define SGX_FUNC_ADJOINT_TRANSPOSE_MATRIX		"SGX_AdjointTransposeMatrix"

/** Implement a sub render state which performs hardware skinning.
Meaning, this sub render states adds calculations which multiply
the points and normals by their assigned bone matricies.
*/
class _OgreRTSSExport DualQuaternionSkinning : public HardwareSkinningTechnique
{
// Interface.
public:
	/** Class default constructor */
	DualQuaternionSkinning();

	/**
	@see SubRenderState::preAddToRenderState.
	*/
	virtual bool preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass);

// Protected methods
protected:
	/**
	@see SubRenderState::resolveParameters.
	*/
	virtual bool resolveParameters (ProgramSet* programSet);

	/**
	@see SubRenderState::resolveDependencies.
	*/
	virtual bool resolveDependencies (ProgramSet* programSet);

	/**
	@see SubRenderState::addFunctionInvocations.
	*/
	virtual bool addFunctionInvocations (ProgramSet* programSet);

	/** Adds functions to calculate position data in world, object and projective space */
	void addPositionCalculations(Function* vsMain, int& funcCounter);

	void adjustForCorrectAntipodality(Function* vsMain, int index, int& funcCounter);

	/** Adds the weight of a given position for a given index */
	void addIndexedPositionWeight(Function* vsMain, int index, UniformParameterPtr& pPositionRelatedParam,
				      ParameterPtr& pPositionTempParameter, ParameterPtr& pPositionRelatedOutputParam, int& funcCounter);
	
	/** Adds the calculations for calculating a normal related element */
	void addNormalRelatedCalculations(Function* vsMain,
						ParameterPtr& pNormalRelatedParam,
						ParameterPtr& pNormalWorldRelatedParam,
						int& funcCounter);

protected:
	UniformParameterPtr mParamInScaleShearMatrices;
	ParameterPtr mParamBlendS;
	ParameterPtr mParamBlendDQ;
	
	ParameterPtr mParamTempFloat2x4;
	ParameterPtr mParamTempFloat3x3;
	ParameterPtr mParamTempFloat3x4;
};

}
}

#endif
#endif

