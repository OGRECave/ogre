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
#ifndef _ShaderExDualQuaternionSkinning_
#define _ShaderExDualQuaternionSkinning_

#include "OgreShaderPrerequisites.h"

#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
#include "OgreShaderExHardwareSkinningTechnique.h"

namespace Ogre {
namespace RTShader {

/** \addtogroup Optional
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

/** Implement a sub render state which performs dual quaternion hardware skinning.
    This sub render state uses bone matrices converted to dual quaternions and adds calculations
    to transform the points and normals using their associated dual quaternions.
*/
class DualQuaternionSkinning : public HardwareSkinningTechnique
{
    bool resolveParameters(Program* vsProgram) override;
    void addPositionCalculations(const FunctionStageRef& stage) override;
    void addNormalRelatedCalculations(const FunctionStageRef& stage) override;

    UniformParameterPtr mParamInScaleShearMatrices;
    ParameterPtr mParamBlendS;
    ParameterPtr mParamBlendDQ;

    ParameterPtr mParamTempFloat3x3;
    ParameterPtr mParamTempFloat3x4;
};

} // namespace RTShader
} // namespace Ogre

#endif // RTSHADER_SYSTEM_BUILD_EXT_SHADERS
#endif // _ShaderExDualQuaternionSkinning_

