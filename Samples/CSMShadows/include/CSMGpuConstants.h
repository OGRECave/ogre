// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
/*  Copyright 2010-2012 Matthew Paul Reid
 */
#ifndef CSM_GPU_CONSTANTS_H
#define CSM_GPU_CONSTANTS_H

#include "Ogre.h"

namespace Ogre
{

class CSMGpuConstants : public ShadowTextureListener
{
public:
    CSMGpuConstants(size_t cascadeCount);
    void shadowTextureCasterPreViewProj(Light* light, Camera* camera, size_t iteration) override;

private:
    Ogre::GpuSharedParametersPtr mParamsScaleBias;

    Ogre::Matrix4 mFirstCascadeViewMatrix;
    Ogre::Real mFirstCascadeCamWidth;
    Ogre::Real mViewRange;
};

} // namespace Ogre

#endif // CSM_GPU_CONSTANTS_H
