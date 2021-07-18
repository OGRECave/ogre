// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT
#ifndef _ShaderWBOIT_
#define _ShaderWBOIT_

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

/** Transform sub render state implementation of writing to WBOIT buffers
 */
class WBOIT : public SubRenderState
{
public:
    const String& getType() const override;
    int getExecutionOrder() const override;
    bool preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass) override;
    bool createCpuSubPrograms(ProgramSet* programSet) override;
    void copyFrom(const SubRenderState& rhs) override {}

    static String Type;
};

/**
A factory that enables creation of GBuffer instances.
@remarks Sub class of SubRenderStateFactory
*/
class WBOITFactory : public SubRenderStateFactory
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
