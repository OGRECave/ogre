// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT
#ifndef _ShaderIBL_
#define _ShaderIBL_

#include "OgreScriptCompiler.h"
#include "OgreShaderPrerequisites.h"
#ifdef RTSHADER_SYSTEM_BUILD_CORE_SHADERS
#include "OgreShaderParameter.h"
#include "OgreShaderSubRenderState.h"

namespace Ogre
{
namespace RTShader
{

/** \addtogroup Optional
 *  @{
 */
/** \addtogroup RTShader
 *  @{
 */

/** Transform sub render state implementation of ImageBasedLighting
 */
class ImageBasedLighting : public SubRenderState
{
    friend class ImageBasedLightingFactory;
    int mDfgLUTSamplerIndex = 0;
    int mEnvMapSamplerIndex = 0;
    int mEnvMapSamplerIndexSpecular = 0;
    int mLocalProbeShAtlasSamplerIndex = 0;
    float mLuminance = 1.0f;
    float mLocalBlendWeight = 0.5f;
    String mEnvMapName;
    String mEnvMapNameSpecular;
    String mLocalProbeShAtlas;

    UniformParameterPtr mLuminanceParam;
    UniformParameterPtr mBlendWeightParam;

    UniformParameterPtr mLocalProbeShAtlasSampler;      //Stores all probe SH coefficients. Row = probe ID. Colums = SH coefficients.
    UniformParameterPtr mShAtlasSizeParam;

    UniformParameterPtr mLocalProbeParams1_1;           //Stores probe position (x, y, z), plus probe ID for SH atlas reference (w).
    UniformParameterPtr mLocalProbeParams1_2;           //Stores probe box extents (x, y, z), plus probe blend distance (w).
    UniformParameterPtr mLocalProbeParams1_3;           //Quaternoin for orientation of probe

    UniformParameterPtr mLocalProbeParams2_1;
    UniformParameterPtr mLocalProbeParams2_2;
    UniformParameterPtr mLocalProbeParams2_3;

    UniformParameterPtr mLocalProbeParams3_1;
    UniformParameterPtr mLocalProbeParams3_2;
    UniformParameterPtr mLocalProbeParams3_3;

    UniformParameterPtr mLocalProbeParams4_1;
    UniformParameterPtr mLocalProbeParams4_2;
    UniformParameterPtr mLocalProbeParams4_3;

    UniformParameterPtr mProbeMaskParam;

    bool mIsLuminanceParamDirty = true;
    bool mUseVertexColourProbeMask = false;
    bool mIsLocalProbesEnabled = false;

public:
    const String& getType() const override;
    int getExecutionOrder() const override;
    bool preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass) override;
    bool createCpuSubPrograms(ProgramSet* programSet) override;
    void copyFrom(const SubRenderState& rhs) override;
    bool setParameter(const String& name, const String& value) override;
    void updateGpuProgramsParams(Renderable* rend, const Pass* pass, const AutoParamDataSource* source,
                                 const LightList* ll) override;
};

class ImageBasedLightingFactory : public SubRenderStateFactory
{
public:
    const String& getType() const override;

    SubRenderState* createInstance(const ScriptProperty& prop, Pass* pass, SGScriptTranslator* translator) override;

    void writeInstance(MaterialSerializer* ser, SubRenderState* subRenderState, Pass* srcPass, Pass* dstPass) override;

protected:
    SubRenderState* createInstanceImpl() override;
};

/** @} */
/** @} */

} // namespace RTShader
} // namespace Ogre

#endif
#endif