// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
/*  Copyright 2010-2012 Matthew Paul Reid
*/
#include "CSMGpuConstants.h"

namespace Ogre
{

CSMGpuConstants::CSMGpuConstants(size_t cascadeCount)
{
    mParamsScaleBias =
        GpuProgramManager::getSingleton().getSharedParameters("params_shadowMatrixScaleBias");
    for (size_t i = 1; i < cascadeCount; i++)
    {
        mParamsScaleBias->addConstantDefinition("texMatrixScaleBias" + StringConverter::toString(i),
                                                GCT_FLOAT4);
    }
}

void CSMGpuConstants::shadowTextureCasterPreViewProj(Light* light, Camera* texCam, size_t index)
{
    if (index == 0)
    {
        mFirstCascadeViewMatrix = texCam->getViewMatrix();
        mFirstCascadeCamWidth = texCam->getOrthoWindowWidth();
        mViewRange = texCam->getFarClipDistance() - texCam->getNearClipDistance();
    }
    else
    {
        Matrix4 mat0 = mFirstCascadeViewMatrix;
        Matrix4 mat1 = texCam->getViewMatrix();

        Real offsetX = mat1[0][3] - mat0[0][3];
        Real offsetY = mat1[1][3] - mat0[1][3];
        Real offsetZ = mat1[2][3] - mat0[2][3];

        Real width0 = mFirstCascadeCamWidth;
        Real width1 = texCam->getOrthoWindowWidth();

        Real oneOnWidth = 1.0f / width0;
        Real offCenter = width1 / (2.0f * width0) - 0.5;

        RenderSystem* rs = Root::getSingletonPtr()->getRenderSystem();
        float depthRange = Math::Abs(rs->getMinimumDepthInputValue() - rs->getMaximumDepthInputValue());

        Vector4 result;
        result.x = offsetX * oneOnWidth + offCenter;
        result.y = -offsetY * oneOnWidth + offCenter;
        result.z = -depthRange * offsetZ / mViewRange;
        result.w = width0 / width1;

        mParamsScaleBias->setNamedConstant("texMatrixScaleBias" + StringConverter::toString(index), result);
    }
}

} // namespace Ogre
