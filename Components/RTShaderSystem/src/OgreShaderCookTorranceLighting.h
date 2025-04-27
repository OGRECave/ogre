// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT
#ifndef _ShaderCookTorranceLighting_
#define _ShaderCookTorranceLighting_

#include "OgreScriptCompiler.h"
#include "OgreShaderPrerequisites.h"
#ifdef RTSHADER_SYSTEM_BUILD_CORE_SHADERS
#include "OgreShaderExPerPixelLighting.h"
#include "OgreLight.h"
#include "OgreCommon.h"
#include "OgreShaderFFPRenderState.h"

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

class CookTorranceLighting : public SubRenderState
{
public:
    CookTorranceLighting();

    const String& getType() const override;

    int getExecutionOrder() const override { return FFP_LIGHTING; }

    void copyFrom(const SubRenderState& rhs) override;

    bool preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass) override;

    /**
    Return the metallic-roughness map texture name.
    */
    const String& getMetalRoughnessMapName() const { return mMetalRoughnessMapName; }

    bool setParameter(const String& name, const String& value) override;

    bool createCpuSubPrograms(ProgramSet* programSet) override;

private:
    String mMetalRoughnessMapName;
    int mLightCount;
    uint8 mMRMapSamplerIndex;
    int8 mLtcLUT1SamplerIndex;
};

class CookTorranceLightingFactory : public SubRenderStateFactory
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
