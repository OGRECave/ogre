// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT
#ifndef __TinyDepthBuffer_H__
#define __TinyDepthBuffer_H__

#include "OgreDepthBuffer.h"
#include "OgreImage.h"

namespace Ogre
{
    class TinyDepthBuffer : public DepthBuffer
    {
        Ogre::Image mBuffer;
    public:
        TinyDepthBuffer(uint16 poolId, uint32 width, uint32 height, uint32 fsaa, bool manual)
            : DepthBuffer(poolId, 32, width, height, fsaa, "", manual)
        {
            mBuffer.create(PF_FLOAT32_R, width, height);
        }

        Image* getImage() { return &mBuffer; }
    };
}
#endif
