#include "OgreLogManager.h"
#include "OgreShaderPrecompiledHeaders.h"
#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS

namespace Ogre {
namespace RTShader {

const String SRS_CLUSTERED_LIGHT_CULLING = "SGX_FroxelClustered";

//-----------------------------------------------------------------------
const String& ClusteredLightCulling::getType() const { return SRS_CLUSTERED_LIGHT_CULLING; }

//-----------------------------------------------------------------------
void ClusteredLightCulling::copyFrom(const SubRenderState& rhs)
{
    const auto& rhsFroxel = static_cast<const ClusteredLightCulling&>(rhs);
    mDebugVisualisation = rhsFroxel.mDebugVisualisation;
}

//-----------------------------------------------------------------------
bool ClusteredLightCulling::setParameter(const String& name, const String& value)
{
    if (name == "debug")
        return StringConverter::parse(value, mDebugVisualisation);

    return false;
}

//-----------------------------------------------------------------------
bool ClusteredLightCulling::preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass)
{
    if (!srcPass->getLightingEnabled())
        return false;

    // must match the light count used by the lighting SRS, so the
    // ACT_LIGHT_POSITION_VIEW_SPACE_ARRAY parameter is shared instead of duplicated
    mLightCount = renderState->getLightCount();

	if (srcPass->getIteratePerLight())
	{
		mLightCount = srcPass->getLightCountPerIteration();
	}

	if(srcPass->getMaxSimultaneousLights() == 0)
	{
		mLightCount = 0;
	}

    return mLightCount > 0;
}

bool ClusteredLightCulling::createCpuSubPrograms(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);

    psProgram->addDependency("RTSLib_Froxels");

    if(mDebugVisualisation)
        psProgram->addPreprocessorDefines("DEBUG_FROXELS");

    Function* vsMain = vsProgram->getEntryPointFunction();
    Function* psMain = psProgram->getEntryPointFunction();

    auto froxelData = GpuProgramManager::getSingleton().getSharedParameters("OgreFroxels");
    psProgram->addSharedParameters(froxelData);

    auto tileParams = psProgram->resolveParameter(GpuProgramParameters::ACT_FROXEL_TILE_PARAMS);
    auto depthParams = psProgram->resolveParameter(GpuProgramParameters::ACT_FROXEL_DEPTH_PARAMS);
    auto lightPositions =
        psProgram->resolveParameter(GpuProgramParameters::ACT_LIGHT_POSITION_VIEW_SPACE_ARRAY, mLightCount);

    // fragment position in projective space (provided by SRS_TRANSFORM)
    auto vsOutPos = vsMain->getOutputParameter(Parameter::SPC_POSITION_PROJECTIVE_SPACE);
    auto fragCoord = psMain->resolveInputParameter(vsOutPos);

    auto lightList = psMain->resolveLocalStructParameter("FroxelLights", "lights");

    // runs before the lighting stage (FFP_PS_COLOUR_BEGIN + 1)
    auto stage = psMain->getStage(FFP_PS_COLOUR_BEGIN);

    std::vector<Operand> args = {In(fragCoord).xyz(), In(tileParams), In(depthParams),
                                 In(lightPositions),  At(0),          Out(lightList)};
    if(mDebugVisualisation)
        args.push_back(InOut(psProgram->resolveParameter(GpuProgramParameters::ACT_DERIVED_SCENE_COLOUR)).xyz());
    stage.callFunction("getFroxelLights", args);
    return true;
}

//-----------------------------------------------------------------------
const String& ClusteredLightCullingFactory::getType() const { return SRS_CLUSTERED_LIGHT_CULLING; }

//-----------------------------------------------------------------------
SubRenderState* ClusteredLightCullingFactory::createInstance(const ScriptProperty& prop, Pass* pass,
                                                       SGScriptTranslator* translator)
{
    if (prop.name != "light_clustering" || prop.values.empty())
        return NULL;

    if (prop.values[0] != "froxel")
        return NULL;

    auto ret = createOrRetrieveInstance(translator);

    for (auto it = prop.values.begin() + 1; it != prop.values.end(); ++it)
    {
        if (!ret->setParameter(*it, "true"))
            translator->emitError(*it);
    }

    return ret;
}

//-----------------------------------------------------------------------
void ClusteredLightCullingFactory::writeInstance(MaterialSerializer* ser, SubRenderState* subRenderState,
                                           Pass* srcPass, Pass* dstPass)
{
    ser->writeAttribute(4, "light_clustering");
    ser->writeValue("froxel");
}

}
}

#endif