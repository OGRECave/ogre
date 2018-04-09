/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

#include "OgreStableHeaders.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreRenderTexture.h"
#include "OgreViewport.h"
#include "OgreRectangle2D.h"
#include "OgreShadowCameraSetup.h"

namespace Ogre {
SceneManager::ShadowRenderer::ShadowRenderer(SceneManager* owner) :
mSceneManager(owner),
mShadowTechnique(SHADOWTYPE_NONE),
mShadowColour(ColourValue(0.25, 0.25, 0.25)),
mShadowCasterPlainBlackPass(0),
mShadowReceiverPass(0),
mShadowModulativePass(0),
mShadowDebugPass(0),
mShadowStencilPass(0),
mShadowIndexBufferSize(51200),
mShadowIndexBufferUsedSize(0),
mShadowTextureCustomCasterPass(0),
mShadowTextureCustomReceiverPass(0),
mShadowAdditiveLightClip(false),
mDebugShadows(false),
mShadowMaterialInitDone(false),
mShadowUseInfiniteFarPlane(true),
mShadowDirLightExtrudeDist(10000),
mDefaultShadowFarDist(0),
mDefaultShadowFarDistSquared(0),
mShadowTextureOffset(0.6),
mShadowTextureFadeStart(0.7),
mShadowTextureFadeEnd(0.9)
{
    // set up default shadow camera setup
    mDefaultShadowCameraSetup.reset(new DefaultShadowCameraSetup());

    // init shadow texture count per type.
    mShadowTextureCountPerType[Light::LT_POINT] = 1;
    mShadowTextureCountPerType[Light::LT_DIRECTIONAL] = 1;
    mShadowTextureCountPerType[Light::LT_SPOTLIGHT] = 1;
}

void SceneManager::ShadowRenderer::setShadowColour(const ColourValue& colour)
{
    mShadowColour = colour;

    // Change shadow material setting only when it's prepared,
    // otherwise, it'll set up while preparing shadow materials.
    if (mShadowModulativePass)
    {
        mShadowModulativePass->getFragmentProgramParameters()->setNamedConstant("shadowColor",colour);
    }
}

void SceneManager::ShadowRenderer::render(RenderQueueGroup* group,
                                          QueuedRenderableCollection::OrganisationMode om)
{
    if(mShadowTechnique & SHADOWDETAILTYPE_STENCIL)
    {
        if(mShadowTechnique & SHADOWDETAILTYPE_ADDITIVE)
        {
            // Additive stencil shadows in use
            renderAdditiveStencilShadowedQueueGroupObjects(group, om);
            return;
        }

        // Modulative stencil shadows in use
        renderModulativeStencilShadowedQueueGroupObjects(group, om);
        return;
    }

    // Receiver pass(es)
    if (mShadowTechnique & SHADOWDETAILTYPE_ADDITIVE)
    {
        // Auto-additive
        renderAdditiveTextureShadowedQueueGroupObjects(group, om);
        return;
    }

    // Modulative
    renderModulativeTextureShadowedQueueGroupObjects(group, om);
}
}
