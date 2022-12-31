// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT
#ifndef _ShaderIBL_
#define _ShaderIBL_

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
    float mLuminance = 1.0f;
    String mEnvMapName;
    UniformParameterPtr mLuminanceParam;
    bool mIsLuminanceParamDirty = true;
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

    SubRenderState* createInstance(ScriptCompiler* compiler, PropertyAbstractNode* prop, Pass* pass,
                                   SGScriptTranslator* translator) override;

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
