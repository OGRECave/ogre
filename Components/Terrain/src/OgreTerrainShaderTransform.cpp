// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT
#include "OgreTerrainShaderTransform.h"

#include "OgrePass.h"
#include "OgreShaderProgramSet.h"
#include "OgreShaderParameter.h"
#include "OgreMaterialSerializer.h"
#include "OgreShaderGenerator.h"
#include "OgreShaderFunction.h"
#include "OgreShaderProgram.h"
#include "OgreTerrainQuadTreeNode.h"

namespace Ogre {
using namespace RTShader;

/************************************************************************/
/*                                                                      */
/************************************************************************/
String TerrainTransform::Type = "TerrainTransform";

bool TerrainTransform::preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass)
{
    auto terrainAny = srcPass->getUserObjectBindings().getUserAny("Terrain");
    mTerrain = any_cast<const Terrain*>(terrainAny);
    mCompressed = mTerrain->_getUseVertexCompression();
    mAlign = mTerrain->getAlignment();
    return true;
}

void TerrainTransform::updateGpuProgramsParams(Renderable* rend, const Pass* pass,
                                               const AutoParamDataSource* source,
                                               const LightList* pLightList)
{
    if(!mPointTrans || mConstParamsSet)
        return;

    mPointTrans->setGpuParameter(mTerrain->getPointTransform());

    float baseUVScale = 1.0f / (mTerrain->getSize() - 1);
    mBaseUVScale->setGpuParameter(baseUVScale);
    mConstParamsSet = true;
}

//-----------------------------------------------------------------------
bool TerrainTransform::createCpuSubPrograms(ProgramSet* programSet)
{
    static Operand::OpMask heightAxis[3] = {Operand::OPM_Y, Operand::OPM_Z, Operand::OPM_X};

    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Function* vsEntry = vsProgram->getEntryPointFunction();

    auto posType = mCompressed ? GCT_INT2 : GCT_FLOAT4;

    auto wvpMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);

    auto lodMorph = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_CUSTOM, GCT_FLOAT2, Terrain::LOD_MORPH_CUSTOM_PARAM);

    auto positionIn = vsEntry->resolveInputParameter(Parameter::SPC_POSITION_OBJECT_SPACE, posType);
    auto positionOut = vsEntry->resolveOutputParameter(Parameter::SPC_POSITION_PROJECTIVE_SPACE);

    auto delta = vsEntry->resolveInputParameter(Parameter::SPC_TEXTURE_COORDINATE1, GCT_FLOAT2);

    // Add dependency.
    vsProgram->addDependency("FFPLib_Transform");
    vsProgram->addDependency("TerrainTransforms");

    auto stage = vsEntry->getStage(FFP_VS_TRANSFORM);

    if(mCompressed)
    {
        mPointTrans = vsProgram->resolveParameter(GCT_MATRIX_4X4, "pointTrans");
        mBaseUVScale = vsProgram->resolveParameter(GCT_FLOAT1, "baseUVScale");
        auto height = vsEntry->resolveInputParameter(Parameter::SPC_TEXTURE_COORDINATE0, GCT_FLOAT1);
        auto uv = vsEntry->resolveOutputParameter(Parameter::SPC_TEXTURE_COORDINATE0, GCT_FLOAT2);
        auto position = vsEntry->resolveLocalParameter(Parameter::SPC_POSITION_OBJECT_SPACE);

        stage.callFunction("expandVertex",
                       {In(mPointTrans), In(mBaseUVScale), In(positionIn), In(height), Out(position), Out(uv)});
        positionIn = position;
    }

    stage.callFunction("FFP_Transform", wvpMatrix, positionIn, positionOut);
    stage.callFunction("applyLODMorph",
                       {In(delta), In(lodMorph), InOut(positionOut).mask(heightAxis[mAlign])});


    return true;
}

//-----------------------------------------------------------------------
const String& TerrainTransformFactory::getType() const
{
    return TerrainTransform::Type;
}

//-----------------------------------------------------------------------
SubRenderState* TerrainTransformFactory::createInstanceImpl()
{
    return OGRE_NEW TerrainTransform();
}

}