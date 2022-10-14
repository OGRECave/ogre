// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT
#ifndef _ShaderTerrainTransform_
#define _ShaderTerrainTransform_

#include "OgreTerrainPrerequisites.h"

#include "OgreShaderSubRenderState.h"
#include "OgreShaderParameter.h"
#include "OgreShaderFFPRenderState.h"
#include "OgreTerrain.h"

namespace Ogre {

class TerrainTransform : public RTShader::SubRenderState
{
public:
    const String& getType() const override { return Type; }
    int getExecutionOrder() const override { return RTShader::FFP_TRANSFORM; }
    void copyFrom(const SubRenderState& rhs) override {}
    bool createCpuSubPrograms(RTShader::ProgramSet* programSet) override;
    bool preAddToRenderState(const RTShader::RenderState* renderState, Pass* srcPass, Pass* dstPass) override;
    void updateParams();

    static String Type;
private:
    const Terrain* mTerrain;
    bool mCompressed = false;
    Terrain::Alignment mAlign = Terrain::ALIGN_X_Z;
    RTShader::UniformParameterPtr mPointTrans;
    RTShader::UniformParameterPtr mBaseUVScale;
};

class TerrainTransformFactory : public RTShader::SubRenderStateFactory
{
public:
    const String& getType() const override;

protected:
    RTShader::SubRenderState* createInstanceImpl() override;
};
}
#endif
