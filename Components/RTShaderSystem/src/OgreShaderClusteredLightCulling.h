#ifndef _ShaderExFroxelClustered_
#define _ShaderExFroxelClustered_

#include "OgreShaderPrerequisites.h"
#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
#include "OgreShaderSubRenderState.h"
#include "OgreShaderFFPRenderState.h"
#include "OgreShaderParameter.h"

namespace Ogre {
namespace RTShader {

/** \addtogroup Optional
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

/** Clustered (froxel) light culling.

Culls the global light list against the froxel grid and publishes the resulting
per-fragment light list as the local parameter @c lights of type @c FroxelLights.
Lighting sub render states
*/
class ClusteredLightCulling : public SubRenderState
{
public:
    const String& getType() const override;
    /// must run before any lighting SRS so the light list local exists
    int getExecutionOrder() const override { return FFP_LIGHTING - 1; }

    void copyFrom(const SubRenderState& rhs) override;
    bool setParameter(const String& name, const String& value) override;
    bool preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass) override;

protected:
    bool createCpuSubPrograms(ProgramSet* programSet) override;

    int mLightCount = 0;
    bool mDebugVisualisation = false;
};

/// A factory that enables creation of ClusteredLightCulling instances.
class ClusteredLightCullingFactory : public SubRenderStateFactory
{
public:
    const String& getType() const override;
    SubRenderState* createInstance(const ScriptProperty& prop, Pass* pass,
                                   SGScriptTranslator* translator) override;
    void writeInstance(MaterialSerializer* ser, SubRenderState* subRenderState, Pass* srcPass,
                       Pass* dstPass) override;

protected:
    SubRenderState* createInstanceImpl() override { return OGRE_NEW ClusteredLightCulling; }
};

/** @} */
/** @} */

}
}

#endif
#endif