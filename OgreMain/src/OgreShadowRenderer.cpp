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
#include "OgreShadowVolumeExtrudeProgram.h"
#include "OgreHighLevelGpuProgram.h"

namespace Ogre {

GpuProgramParametersSharedPtr SceneManager::ShadowRenderer::msInfiniteExtrusionParams;
GpuProgramParametersSharedPtr SceneManager::ShadowRenderer::msFiniteExtrusionParams;

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
mFullScreenQuad(0),
mShadowAdditiveLightClip(false),
mDebugShadows(false),
mShadowMaterialInitDone(false),
mShadowUseInfiniteFarPlane(true),
mShadowDirLightExtrudeDist(10000),
mDefaultShadowFarDist(0),
mDefaultShadowFarDistSquared(0),
mShadowTextureOffset(0.6),
mShadowTextureFadeStart(0.7),
mShadowTextureFadeEnd(0.9),
mShadowTextureSelfShadow(false),
mShadowTextureConfigDirty(true),
mShadowCasterRenderBackFaces(true)
{
    mShadowCasterQueryListener.reset(new ShadowCasterSceneQueryListener(mSceneManager));

    // set up default shadow camera setup
    mDefaultShadowCameraSetup = DefaultShadowCameraSetup::create();

    // init shadow texture count per type.
    mShadowTextureCountPerType[Light::LT_POINT] = 1;
    mShadowTextureCountPerType[Light::LT_DIRECTIONAL] = 1;
    mShadowTextureCountPerType[Light::LT_SPOTLIGHT] = 1;
}

SceneManager::ShadowRenderer::~ShadowRenderer() {}

void SceneManager::ShadowRenderer::setShadowColour(const ColourValue& colour)
{
    mShadowColour = colour;
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
size_t SceneManager::ShadowRenderer::getShadowTexIndex(size_t startLightIndex)
{
    size_t shadowTexIndex = mShadowTextures.size();
    if (mShadowTextureIndexLightList.size() > startLightIndex)
        shadowTexIndex = mShadowTextureIndexLightList[startLightIndex];
    return shadowTexIndex;
}
//-----------------------------------------------------------------------
void SceneManager::ShadowRenderer::renderAdditiveStencilShadowedQueueGroupObjects(
    RenderQueueGroup* pGroup,
    QueuedRenderableCollection::OrganisationMode om)
{
    LightList lightList(1);
    auto visitor = mSceneManager->getQueuedRenderableVisitor();

    for (const auto& pg : pGroup->getPriorityGroups())
    {
        RenderPriorityGroup* pPriorityGrp = pg.second;

        // Sort the queue first
        pPriorityGrp->sort(mSceneManager->mCameraInProgress);

        // Render all the ambient passes first, no light iteration, no lights
        visitor->renderObjects(pPriorityGrp->getSolidsBasic(), om, false, false);
        // Also render any objects which have receive shadows disabled
        visitor->renderObjects(pPriorityGrp->getSolidsNoShadowReceive(), om, true, true);


        // Now iterate per light
        // Iterate over lights, render all volumes to stencil
        for (Light* l : mSceneManager->_getLightsAffectingFrustum())
        {
            // Set light state
            lightList[0] = l;

            // set up scissor, will cover shadow vol and regular light rendering
            ClipResult scissored = mSceneManager->buildAndSetScissor(lightList, mSceneManager->mCameraInProgress);
            ClipResult clipped = CLIPPED_NONE;
            if (mShadowAdditiveLightClip)
                clipped = mSceneManager->buildAndSetLightClip(lightList);

            // skip light if scissored / clipped entirely
            if (scissored == CLIPPED_ALL || clipped == CLIPPED_ALL)
                continue;

            if (l->getCastShadows())
            {
                // Clear stencil
                mDestRenderSystem->clearFrameBuffer(FBT_STENCIL);
                renderShadowVolumesToStencil(l, mSceneManager->mCameraInProgress, false);
                // NB we render where the stencil is equal to zero to render lit areas
                StencilState stencilState;
                stencilState.enabled = true;
                stencilState.compareOp = CMPF_EQUAL;
                mDestRenderSystem->setStencilState(stencilState);
            }

            // render lighting passes for this light
            visitor->renderObjects(pPriorityGrp->getSolidsDiffuseSpecular(), om, false, false, &lightList);

            // Reset stencil params
            mDestRenderSystem->setStencilState(StencilState());

            if (scissored == CLIPPED_SOME)
                mSceneManager->resetScissor();
            if (clipped == CLIPPED_SOME)
                mSceneManager->resetLightClip();

        }// for each light


        // Now render decal passes, no need to set lights as lighting will be disabled
        visitor->renderObjects(pPriorityGrp->getSolidsDecal(), om, false, false);


    }// for each priority

    // Iterate again - variable name changed to appease gcc.
    for (const auto& pg : pGroup->getPriorityGroups())
    {
        RenderPriorityGroup* pPriorityGrp = pg.second;

        // Do unsorted transparents
        visitor->renderObjects(pPriorityGrp->getTransparentsUnsorted(), om, true, true);
        // Do transparents (always descending sort)
        visitor->renderObjects(pPriorityGrp->getTransparents(),
            QueuedRenderableCollection::OM_SORT_DESCENDING, true, true);

    }// for each priority


}
//-----------------------------------------------------------------------
void SceneManager::ShadowRenderer::renderModulativeStencilShadowedQueueGroupObjects(
    RenderQueueGroup* pGroup,
    QueuedRenderableCollection::OrganisationMode om)
{
    /* For each light, we need to render all the solids from each group,
    then do the modulative shadows, then render the transparents from
    each group.
    Now, this means we are going to reorder things more, but that it required
    if the shadows are to look correct. The overall order is preserved anyway,
    it's just that all the transparents are at the end instead of them being
    interleaved as in the normal rendering loop.
    */
    // Iterate through priorities
    auto visitor = mSceneManager->getQueuedRenderableVisitor();
    for (const auto& pg : pGroup->getPriorityGroups())
    {
        RenderPriorityGroup* pPriorityGrp = pg.second;

        // Sort the queue first
        pPriorityGrp->sort(mSceneManager->mCameraInProgress);

        // Do (shadowable) solids
        visitor->renderObjects(pPriorityGrp->getSolidsBasic(), om, true, true);
    }

    // Override auto param ambient to force vertex programs to use shadow colour
    ColourValue currAmbient = mSceneManager->getAmbientLight();
    mSceneManager->setAmbientLight(mShadowColour);

    // Iterate over lights, render all volumes to stencil
    for (Light* l : mSceneManager->_getLightsAffectingFrustum())
    {
        if (l->getCastShadows())
        {
            // Clear stencil
            mDestRenderSystem->clearFrameBuffer(FBT_STENCIL);
            renderShadowVolumesToStencil(l, mSceneManager->mCameraInProgress, true);
            // render full-screen shadow modulator for all lights
            mSceneManager->_setPass(mShadowModulativePass);
            // NB we render where the stencil is not equal to zero to render shadows, not lit areas
            StencilState stencilState;
            stencilState.enabled = true;
            stencilState.compareOp = CMPF_NOT_EQUAL;
            mDestRenderSystem->setStencilState(stencilState);
            mSceneManager->renderSingleObject(mFullScreenQuad, mShadowModulativePass, false, false);
            // Reset stencil params
            mDestRenderSystem->setStencilState(StencilState());
        }

    }// for each light

    // Restore ambient light
    mSceneManager->setAmbientLight(currAmbient);

    // Iterate again - variable name changed to appease gcc.
    for (const auto& pg : pGroup->getPriorityGroups())
    {
        RenderPriorityGroup* pPriorityGrp = pg.second;

        // Do non-shadowable solids
        visitor->renderObjects(pPriorityGrp->getSolidsNoShadowReceive(), om, true, true);

    }// for each priority


    // Iterate again - variable name changed to appease gcc.
    for (const auto& pg : pGroup->getPriorityGroups())
    {
        RenderPriorityGroup* pPriorityGrp = pg.second;

        // Do unsorted transparents
        visitor->renderObjects(pPriorityGrp->getTransparentsUnsorted(), om, true, true);
        // Do transparents (always descending sort)
        visitor->renderObjects(pPriorityGrp->getTransparents(),
            QueuedRenderableCollection::OM_SORT_DESCENDING, true, true);

    }// for each priority

}
//-----------------------------------------------------------------------
void SceneManager::ShadowRenderer::renderTextureShadowCasterQueueGroupObjects(
    RenderQueueGroup* pGroup,
    QueuedRenderableCollection::OrganisationMode om)
{
    // This is like the basic group render, except we skip all transparents
    // and we also render any non-shadowed objects
    // Note that non-shadow casters will have already been eliminated during
    // _findVisibleObjects

    // Iterate through priorities

    // Override auto param ambient to force vertex programs and fixed function to
    ColourValue currAmbient = mSceneManager->getAmbientLight();
    if (mShadowTechnique & SHADOWDETAILTYPE_ADDITIVE)
    {
        // Use simple black / white mask if additive
        mSceneManager->setAmbientLight(ColourValue::Black);
    }
    else
    {
        // Use shadow colour as caster colour if modulative
        mSceneManager->setAmbientLight(mShadowColour);
    }

    auto visitor = mSceneManager->getQueuedRenderableVisitor();
    for (const auto& pg : pGroup->getPriorityGroups())
    {
        RenderPriorityGroup* pPriorityGrp = pg.second;

        // Sort the queue first
        pPriorityGrp->sort(mSceneManager->mCameraInProgress);

        // Do solids, override light list incase any vertex programs use them
        visitor->renderObjects(pPriorityGrp->getSolidsBasic(), om, false, false);
        visitor->renderObjects(pPriorityGrp->getSolidsNoShadowReceive(), om, false, false);
        // Do unsorted transparents that cast shadows
        visitor->renderObjects(pPriorityGrp->getTransparentsUnsorted(), om, false, false, nullptr, true);
        // Do transparents that cast shadows
        visitor->renderObjects(pPriorityGrp->getTransparents(), QueuedRenderableCollection::OM_SORT_DESCENDING, false,
                               false, nullptr, true);

    }// for each priority

    // reset ambient light
    mSceneManager->setAmbientLight(currAmbient);
}
//-----------------------------------------------------------------------
void SceneManager::ShadowRenderer::renderModulativeTextureShadowedQueueGroupObjects(
    RenderQueueGroup* pGroup,
    QueuedRenderableCollection::OrganisationMode om)
{
    /* For each light, we need to render all the solids from each group,
    then do the modulative shadows, then render the transparents from
    each group.
    Now, this means we are going to reorder things more, but that it required
    if the shadows are to look correct. The overall order is preserved anyway,
    it's just that all the transparents are at the end instead of them being
    interleaved as in the normal rendering loop.
    */
    // Iterate through priorities
    auto visitor = mSceneManager->getQueuedRenderableVisitor();
    for (const auto& pg : pGroup->getPriorityGroups())
    {
        RenderPriorityGroup* pPriorityGrp = pg.second;

        // Sort the queue first
        pPriorityGrp->sort(mSceneManager->mCameraInProgress);

        // Do solids
        visitor->renderObjects(pPriorityGrp->getSolidsBasic(), om, true, true);
        visitor->renderObjects(pPriorityGrp->getSolidsNoShadowReceive(), om, true, true);
    }


    // Iterate over lights, render received shadows
    // only perform this if we're in the 'normal' render stage, to avoid
    // doing it during the render to texture
    if (mSceneManager->mIlluminationStage == IRS_NONE)
    {
        mSceneManager->mIlluminationStage = IRS_RENDER_RECEIVER_PASS;

        LightList::const_iterator i, iend;
        iend = mSceneManager->_getLightsAffectingFrustum().end();

        size_t si = 0;
        for (i = mSceneManager->_getLightsAffectingFrustum().begin();
             i != iend && si < mShadowTextures.size(); ++i)
        {
            Light* l = *i;

            if (!l->getCastShadows())
                continue;

            // Get camera for current shadow texture
            Camera *cam = mShadowTextures[si]->getBuffer()->getRenderTarget()->getViewport(0)->getCamera();
            // Hook up receiver texture
            Pass* targetPass = mShadowTextureCustomReceiverPass ?
                    mShadowTextureCustomReceiverPass : mShadowReceiverPass;

            // if this light is a spotlight, we need to add the spot fader layer
            // BUT not if using a custom projection matrix, since then it will be
            // inappropriately shaped most likely
            if (l->getType() == Light::LT_SPOTLIGHT && !cam->isCustomProjectionMatrixEnabled())
            {
                // remove all TUs except 0 & 1
                // (only an issue if additive shadows have been used)
                while(targetPass->getNumTextureUnitStates() > 2)
                    targetPass->removeTextureUnitState(2);

                TextureUnitState* t = NULL;
                // Add spot fader if not present already
                if (targetPass->getNumTextureUnitStates() == 2 &&
                    targetPass->getTextureUnitState(1)->_getTexturePtr() == mSpotFadeTexture)
                {
                    // Just set
                    t = targetPass->getTextureUnitState(1);
                }
                else
                {
                    // Remove any non-conforming spot layers
                    while(targetPass->getNumTextureUnitStates() > 1)
                        targetPass->removeTextureUnitState(1);

                    t = targetPass->createTextureUnitState();
                    t->setTexture(mSpotFadeTexture);
                    t->setColourOperation(LBO_ADD);
                    t->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
                }

                t->setProjectiveTexturing(!targetPass->hasVertexProgram(), cam);
                mSceneManager->mAutoParamDataSource->setTextureProjector(cam, 1);
            }
            else
            {
                // remove all TUs except 0 including spot
                while(targetPass->getNumTextureUnitStates() > 1)
                    targetPass->removeTextureUnitState(1);

            }

            // account for the RTSS
            if (auto betterTechnique = targetPass->getParent()->getParent()->getBestTechnique())
            {
                targetPass = betterTechnique->getPass(0);
            }

            TextureUnitState* texUnit = targetPass->getTextureUnitState(0);
            // clamp to border colour in case this is a custom material
            texUnit->setSampler(mBorderSampler);
            resolveShadowTexture(texUnit, si, 0);

            // Set lighting / blending modes
            targetPass->setSceneBlending(SBF_DEST_COLOUR, SBF_ZERO);
            targetPass->setLightingEnabled(false);

            targetPass->_load();

            // Fire pre-receiver event
            fireShadowTexturesPreReceiver(l, cam);

            renderTextureShadowReceiverQueueGroupObjects(pGroup, om);

            ++si;

        } // for each light

        mSceneManager->mIlluminationStage = IRS_NONE;

    }

    // Iterate again - variable name changed to appease gcc.
    for (const auto& pg : pGroup->getPriorityGroups())
    {
        RenderPriorityGroup* pPriorityGrp = pg.second;

        // Do unsorted transparents
        visitor->renderObjects(pPriorityGrp->getTransparentsUnsorted(), om, true, true);
        // Do transparents (always descending)
        visitor->renderObjects(pPriorityGrp->getTransparents(),
            QueuedRenderableCollection::OM_SORT_DESCENDING, true, true);

    }// for each priority

}
//-----------------------------------------------------------------------
void SceneManager::ShadowRenderer::renderAdditiveTextureShadowedQueueGroupObjects(
    RenderQueueGroup* pGroup,
    QueuedRenderableCollection::OrganisationMode om)
{
    LightList lightList(1);
    auto visitor = mSceneManager->getQueuedRenderableVisitor();

    for (const auto& pg : pGroup->getPriorityGroups())
    {
        RenderPriorityGroup* pPriorityGrp = pg.second;

        // Sort the queue first
        pPriorityGrp->sort(mSceneManager->mCameraInProgress);

        // Render all the ambient passes first, no light iteration, no lights
        visitor->renderObjects(pPriorityGrp->getSolidsBasic(), om, false, false);
        // Also render any objects which have receive shadows disabled
        visitor->renderObjects(pPriorityGrp->getSolidsNoShadowReceive(), om, true, true);


        // only perform this next part if we're in the 'normal' render stage, to avoid
        // doing it during the render to texture
        if (mSceneManager->mIlluminationStage == IRS_NONE)
        {
            // Iterate over lights, render masked
            size_t si = 0;

            for (Light* l : mSceneManager->_getLightsAffectingFrustum())
            {
                if (l->getCastShadows() && si < mShadowTextures.size())
                {
                    // Hook up receiver texture
                    Pass* targetPass = mShadowTextureCustomReceiverPass ?
                        mShadowTextureCustomReceiverPass : mShadowReceiverPass;

                    // account for the RTSS
                    if (auto betterTechnique = targetPass->getParent()->getParent()->getBestTechnique())
                    {
                        targetPass = betterTechnique->getPass(0);
                    }

                    TextureUnitState* texUnit = targetPass->getTextureUnitState(0);
                    // clamp to border colour in case this is a custom material
                    texUnit->setSampler(mBorderSampler);
                    resolveShadowTexture(texUnit, si, 0);

                    // Remove any spot fader layer
                    if (targetPass->getNumTextureUnitStates() > 1 &&
                        targetPass->getTextureUnitState(1)->getTextureName() == "spot_shadow_fade.dds")
                    {
                        // remove spot fader layer (should only be there if
                        // we previously used modulative shadows)
                        targetPass->removeTextureUnitState(1);
                    }
                    // Set lighting / blending modes
                    targetPass->setSceneBlending(SBF_ONE, SBF_ONE);
                    targetPass->setLightingEnabled(true);
                    targetPass->_load();

                    // increment shadow texture since used
                    ++si;

                    mSceneManager->mIlluminationStage = IRS_RENDER_RECEIVER_PASS;

                }
                else
                {
                    mSceneManager->mIlluminationStage = IRS_NONE;
                }

                // render lighting passes for this light
                lightList[0] = l;

                // set up light scissoring, always useful in additive modes
                ClipResult scissored = mSceneManager->buildAndSetScissor(lightList, mSceneManager->mCameraInProgress);
                ClipResult clipped = CLIPPED_NONE;
                if(mShadowAdditiveLightClip)
                    clipped = mSceneManager->buildAndSetLightClip(lightList);
                // skip if entirely clipped
                if(scissored == CLIPPED_ALL || clipped == CLIPPED_ALL)
                    continue;

                visitor->renderObjects(pPriorityGrp->getSolidsDiffuseSpecular(), om, false, false, &lightList);
                if (scissored == CLIPPED_SOME)
                    mSceneManager->resetScissor();
                if (clipped == CLIPPED_SOME)
                    mSceneManager->resetLightClip();

            }// for each light

            mSceneManager->mIlluminationStage = IRS_NONE;

            // Now render decal passes, no need to set lights as lighting will be disabled
            visitor->renderObjects(pPriorityGrp->getSolidsDecal(), om, false, false);

        }


    }// for each priority

    // Iterate again - variable name changed to appease gcc.
    for (const auto& pg : pGroup->getPriorityGroups())
    {
        RenderPriorityGroup* pPriorityGrp = pg.second;

        // Do unsorted transparents
        visitor->renderObjects(pPriorityGrp->getTransparentsUnsorted(), om, true, true);
        // Do transparents (always descending sort)
        visitor->renderObjects(pPriorityGrp->getTransparents(),
            QueuedRenderableCollection::OM_SORT_DESCENDING, true, true);

    }// for each priority

}
//-----------------------------------------------------------------------
void SceneManager::ShadowRenderer::renderTextureShadowReceiverQueueGroupObjects(
    RenderQueueGroup* pGroup,
    QueuedRenderableCollection::OrganisationMode om)
{
    // Iterate through priorities

    // Override auto param ambient to force vertex programs to go full-bright
    ColourValue currAmbient = mSceneManager->getAmbientLight();
    mSceneManager->setAmbientLight(ColourValue::White);
    auto visitor = mSceneManager->getQueuedRenderableVisitor();

    for (const auto& pg : pGroup->getPriorityGroups())
    {
        RenderPriorityGroup* pPriorityGrp = pg.second;

        // Do solids, override light list incase any vertex programs use them
        visitor->renderObjects(pPriorityGrp->getSolidsBasic(), om, false, false);

        // Don't render transparents or passes which have shadow receipt disabled

    }// for each priority

    // reset ambient
    mSceneManager->setAmbientLight(currAmbient);
}
//---------------------------------------------------------------------
void SceneManager::ShadowRenderer::ensureShadowTexturesCreated()
{
    if(!mBorderSampler)
    {
        mBorderSampler = TextureManager::getSingleton().createSampler();
        mBorderSampler->setAddressingMode(TAM_BORDER);
        mBorderSampler->setBorderColour(ColourValue::White);
        mBorderSampler->setFiltering(FT_MIP, FO_NONE); // we do not have mips. GLES2 is particularly picky here.
    }

    if (mShadowTextureConfigDirty)
    {
        destroyShadowTextures();
        ShadowTextureManager::getSingleton().getShadowTextures(mShadowTextureConfigList, mShadowTextures);

        // clear shadow cam - light mapping
        mShadowCamLightMapping.clear();

        //Used to get the depth buffer ID setting for each RTT
        size_t __i = 0;

        // Recreate shadow textures
        for (ShadowTextureList::iterator i = mShadowTextures.begin();
            i != mShadowTextures.end(); ++i, ++__i)
        {
            const TexturePtr& shadowTex = *i;

            // Camera names are local to SM
            String camName = shadowTex->getName() + "Cam";
            // Material names are global to SM, make specific
            String matName = shadowTex->getName() + "Mat" + mSceneManager->getName();

            RenderTexture *shadowRTT = shadowTex->getBuffer()->getRenderTarget();

            //Set appropriate depth buffer
            if(!PixelUtil::isDepth(shadowRTT->suggestPixelFormat()))
                shadowRTT->setDepthBufferPool( mShadowTextureConfigList[__i].depthBufferPoolId );

            // Create camera for this texture, but note that we have to rebind
            // in prepareShadowTextures to coexist with multiple SMs
            Camera* cam = mSceneManager->createCamera(camName);
            cam->setAspectRatio((Real)shadowTex->getWidth() / (Real)shadowTex->getHeight());
            mSceneManager->getRootSceneNode()->createChildSceneNode()->attachObject(cam);
            mShadowTextureCameras.push_back(cam);

            // Create a viewport, if not there already
            if (shadowRTT->getNumViewports() == 0)
            {
                // Note camera assignment is transient when multiple SMs
                Viewport *v = shadowRTT->addViewport(cam);
                v->setClearEveryFrame(true);
                // remove overlays
                v->setOverlaysEnabled(false);
            }

            // Don't update automatically - we'll do it when required
            shadowRTT->setAutoUpdated(false);

            // Also create corresponding Material used for rendering this shadow
            MaterialPtr mat = MaterialManager::getSingleton().getByName(matName);
            if (!mat)
            {
                mat = MaterialManager::getSingleton().create(
                    matName, ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME);
            }
            Pass* p = mat->getTechnique(0)->getPass(0);
            if (p->getNumTextureUnitStates() != 1 ||
                p->getTextureUnitState(0)->_getTexturePtr(0) != shadowTex)
            {
                mat->getTechnique(0)->getPass(0)->removeAllTextureUnitStates();
                // create texture unit referring to render target texture
                TextureUnitState* texUnit =
                    p->createTextureUnitState(shadowTex->getName());
                // set projective based on camera
                texUnit->setProjectiveTexturing(!p->hasVertexProgram(), cam);
                // clamp to border colour
                texUnit->setSampler(mBorderSampler);
                mat->touch();

            }

            // insert dummy camera-light combination
            mShadowCamLightMapping[cam] = 0;

            // Get null shadow texture
            if (mShadowTextureConfigList.empty())
            {
                mNullShadowTexture.reset();
            }
            else
            {
                mNullShadowTexture = ShadowTextureManager::getSingleton().getNullShadowTexture(
                    mShadowTextureConfigList[0].format);
            }


        }
        mShadowTextureConfigDirty = false;
    }

}
//---------------------------------------------------------------------
void SceneManager::ShadowRenderer::destroyShadowTextures(void)
{

    ShadowTextureList::iterator i, iend;
    iend = mShadowTextures.end();
    for (i = mShadowTextures.begin(); i != iend; ++i)
    {
        TexturePtr &shadowTex = *i;

        // Cleanup material that references this texture
        String matName = shadowTex->getName() + "Mat" + mSceneManager->getName();
        MaterialPtr mat = MaterialManager::getSingleton().getByName(matName);
        if (mat)
        {
            // manually clear TUS to ensure texture ref released
            mat->getTechnique(0)->getPass(0)->removeAllTextureUnitStates();
            MaterialManager::getSingleton().remove(mat->getHandle());
        }

    }

    for (auto cam : mShadowTextureCameras)
    {
        mSceneManager->getRootSceneNode()->removeAndDestroyChild(cam->getParentSceneNode());
        // Always destroy camera since they are local to this SM
        mSceneManager->destroyCamera(cam);
    }
    mShadowTextures.clear();
    mShadowTextureCameras.clear();

    // set by render*TextureShadowedQueueGroupObjects
    mSceneManager->mAutoParamDataSource->setTextureProjector(NULL, 0);

    // Will destroy if no other scene managers referencing
    ShadowTextureManager::getSingleton().clearUnused();

    mShadowTextureConfigDirty = true;
}
//---------------------------------------------------------------------
void SceneManager::ShadowRenderer::prepareShadowTextures(Camera* cam, Viewport* vp, const LightList* lightList)
{
    // create shadow textures if needed
    ensureShadowTexturesCreated();

    // Determine far shadow distance
    Real shadowDist = mDefaultShadowFarDist;
    if (!shadowDist)
    {
        // need a shadow distance, make one up
        shadowDist = cam->getNearClipDistance() * 300;
    }
    Real shadowOffset = shadowDist * mShadowTextureOffset;
    // Precalculate fading info
    Real shadowEnd = shadowDist + shadowOffset;
    Real fadeStart = shadowEnd * mShadowTextureFadeStart;
    Real fadeEnd = shadowEnd * mShadowTextureFadeEnd;
    // Additive lighting should not use fogging, since it will overbrighten; use border clamp
    if ((mShadowTechnique & SHADOWDETAILTYPE_ADDITIVE) == 0)
    {
        // set fogging to hide the shadow edge
        mShadowReceiverPass->setFog(true, FOG_LINEAR, ColourValue::White, 0, fadeStart, fadeEnd);
    }
    else
    {
        // disable fogging explicitly
        mShadowReceiverPass->setFog(true, FOG_NONE);
    }

    // Iterate over the lights we've found, max out at the limit of light textures
    // Note that the light sorting must now place shadow casting lights at the
    // start of the light list, therefore we do not need to deal with potential
    // mismatches in the light<->shadow texture list any more

    LightList::const_iterator i, iend;
    ShadowTextureList::iterator si, siend;
    CameraList::iterator ci;
    iend = lightList->end();
    siend = mShadowTextures.end();
    ci = mShadowTextureCameras.begin();
    mShadowTextureIndexLightList.clear();
    size_t shadowTextureIndex = 0;
    for (i = lightList->begin(), si = mShadowTextures.begin(); i != iend && si != siend; ++i)
    {
        Light* light = *i;

        // skip light if shadows are disabled
        if (!light->getCastShadows())
            continue;

        // texture iteration per light.
        size_t textureCountPerLight = mShadowTextureCountPerType[light->getType()];
        for (size_t j = 0; j < textureCountPerLight && si != siend; ++j)
        {
            TexturePtr &shadowTex = *si;
            RenderTarget *shadowRTT = shadowTex->getBuffer()->getRenderTarget();
            Viewport *shadowView = shadowRTT->getViewport(0);
            Camera *texCam = *ci;
            // rebind camera, incase another SM in use which has switched to its cam
            shadowView->setCamera(texCam);

            // Associate main view camera as LOD camera
            texCam->setLodCamera(cam);
            // set base
            if (light->getType() != Light::LT_POINT)
                texCam->getParentSceneNode()->setDirection(light->getDerivedDirection(), Node::TS_WORLD);
            if (light->getType() != Light::LT_DIRECTIONAL)
                texCam->getParentSceneNode()->setPosition(light->getDerivedPosition());

            // Use the material scheme of the main viewport
            // This is required to pick up the correct shadow_caster_material and similar properties.
            shadowView->setMaterialScheme(vp->getMaterialScheme());

            // Set the viewport visibility flags
            shadowView->setVisibilityMask(light->getLightMask() & vp->getVisibilityMask());

            // update shadow cam - light mapping
            ShadowCamLightMapping::iterator camLightIt = mShadowCamLightMapping.find( texCam );
            assert(camLightIt != mShadowCamLightMapping.end());
            camLightIt->second = light;

            if (!light->getCustomShadowCameraSetup())
                mDefaultShadowCameraSetup->getShadowCamera(mSceneManager, cam, vp, light, texCam, j);
            else
                light->getCustomShadowCameraSetup()->getShadowCamera(mSceneManager, cam, vp, light, texCam, j);

            // Setup background colour
            shadowView->setBackgroundColour(ColourValue::White);

            // Fire shadow caster update, callee can alter camera settings
            fireShadowTexturesPreCaster(light, texCam, j);

            // Update target
            shadowRTT->update();

            ++si; // next shadow texture
            ++ci; // next camera
        }

        // set the first shadow texture index for this light.
        mShadowTextureIndexLightList.push_back(shadowTextureIndex);
        shadowTextureIndex += textureCountPerLight;
    }

    fireShadowTexturesUpdated(std::min(lightList->size(), mShadowTextures.size()));

    ShadowTextureManager::getSingleton().clearUnused();

}
//---------------------------------------------------------------------
void SceneManager::ShadowRenderer::renderShadowVolumesToStencil(const Light* light,
    const Camera* camera, bool calcScissor)
{
    // Get the shadow caster list
    const ShadowCasterList& casters = findShadowCastersForLight(light, camera);
    // Check there are some shadow casters to render
    if (casters.empty())
    {
        // No casters, just do nothing
        return;
    }

    // Add light to internal list for use in render call
    LightList lightList;
    // const_cast is forgiveable here since we pass this const
    lightList.push_back(const_cast<Light*>(light));

    // Set up scissor test (point & spot lights only)
    ClipResult scissored = CLIPPED_NONE;
    if (calcScissor)
    {
        scissored = mSceneManager->buildAndSetScissor(lightList, camera);
        if (scissored == CLIPPED_ALL)
            return; // nothing to do
    }

    mDestRenderSystem->unbindGpuProgram(GPT_FRAGMENT_PROGRAM);

    // Can we do a 2-sided stencil?
    bool stencil2sided = false;
    if (mDestRenderSystem->getCapabilities()->hasCapability(RSC_TWO_SIDED_STENCIL) &&
        mDestRenderSystem->getCapabilities()->hasCapability(RSC_STENCIL_WRAP))
    {
        // enable
        stencil2sided = true;
    }

    // Do we have access to vertex programs?
    bool extrudeInSoftware = true;
    bool finiteExtrude = !mShadowUseInfiniteFarPlane;
    if (const auto& fprog = mShadowStencilPass->getFragmentProgram())
    {
        extrudeInSoftware = false;
        // attach the appropriate extrusion vertex program
        // Note we never unset it because support for vertex programs is constant
        mShadowStencilPass->setGpuProgram(GPT_VERTEX_PROGRAM,
                                          ShadowVolumeExtrudeProgram::get(light->getType(), finiteExtrude), false);
        // Set params
        if (finiteExtrude)
        {
            mShadowStencilPass->setVertexProgramParameters(msFiniteExtrusionParams);
        }
        else
        {
            mShadowStencilPass->setVertexProgramParameters(msInfiniteExtrusionParams);
        }
        if (mDebugShadows)
        {
            mShadowDebugPass->setGpuProgram(GPT_VERTEX_PROGRAM,
                                            ShadowVolumeExtrudeProgram::get(light->getType(), finiteExtrude), false);

            // Set params
            if (finiteExtrude)
            {
                mShadowDebugPass->setVertexProgramParameters(msFiniteExtrusionParams);
            }
            else
            {
                mShadowDebugPass->setVertexProgramParameters(msInfiniteExtrusionParams);
            }
        }

        mSceneManager->bindGpuProgram(mShadowStencilPass->getVertexProgram()->_getBindingDelegate());
        mSceneManager->bindGpuProgram(fprog->_getBindingDelegate());
    }
    else
    {
        mDestRenderSystem->unbindGpuProgram(GPT_VERTEX_PROGRAM);
    }
    if (mDestRenderSystem->getCapabilities()->hasCapability(RSC_GEOMETRY_PROGRAM))
    {
        mDestRenderSystem->unbindGpuProgram(GPT_GEOMETRY_PROGRAM);
    }

    mDestRenderSystem->_setAlphaRejectSettings(mShadowStencilPass->getAlphaRejectFunction(),
        mShadowStencilPass->getAlphaRejectValue(), mShadowStencilPass->isAlphaToCoverageEnabled());

    // Turn off colour writing and depth writing
    ColourBlendState disabled;
    disabled.writeR = disabled.writeG = disabled.writeB = disabled.writeA = false;
    mDestRenderSystem->setColourBlendState(disabled);
    mDestRenderSystem->_disableTextureUnitsFrom(0);
    mDestRenderSystem->_setDepthBufferParams(true, false, CMPF_LESS);

    // Figure out the near clip volume
    const PlaneBoundedVolume& nearClipVol =
        light->_getNearClipVolume(camera);

    // Now iterate over the casters and render
    ShadowCasterList::const_iterator si, siend;
    siend = casters.end();


    // Now iterate over the casters and render
    for (si = casters.begin(); si != siend; ++si)
    {
        ShadowCaster* caster = *si;
        bool zfailAlgo = camera->isCustomNearClipPlaneEnabled();
        unsigned long flags = 0;

        // Calculate extrusion distance
        Real extrudeDist = mShadowDirLightExtrudeDist;
        if (light->getType() != Light::LT_DIRECTIONAL)
        {
            // we have to limit shadow extrusion to avoid cliping by far clip plane 
            extrudeDist = std::min(caster->getPointExtrusionDistance(light), mShadowDirLightExtrudeDist); 
            // Set autoparams for finite point light extrusion
            mSceneManager->mAutoParamDataSource->setShadowPointLightExtrusionDistance(extrudeDist);
        }

        Real darkCapExtrudeDist = extrudeDist;
        if (!extrudeInSoftware && !finiteExtrude)
        {
            // hardware extrusion, to infinity (and beyond!)
            flags |= SRF_EXTRUDE_TO_INFINITY;
            darkCapExtrudeDist = mShadowDirLightExtrudeDist;
        }

        // Determine whether zfail is required
        if (zfailAlgo || nearClipVol.intersects(caster->getWorldBoundingBox()))
        {
            // We use zfail for this object only because zfail
            // compatible with zpass algorithm
            zfailAlgo = true;
            // We need to include the light and / or dark cap
            // But only if they will be visible
            if(camera->isVisible(caster->getLightCapBounds()))
            {
                flags |= SRF_INCLUDE_LIGHT_CAP;
            }
            // zfail needs dark cap
            // UNLESS directional lights using hardware extrusion to infinity
            // since that extrudes to a single point
            if(!((flags & SRF_EXTRUDE_TO_INFINITY) &&
                light->getType() == Light::LT_DIRECTIONAL) &&
                camera->isVisible(caster->getDarkCapBounds(*light, darkCapExtrudeDist)))
            {
                flags |= SRF_INCLUDE_DARK_CAP;
            }
        }
        else
        {
            // In zpass we need a dark cap if
            // 1: infinite extrusion on point/spotlight sources in modulative shadows
            //    mode, since otherwise in areas where there is no depth (skybox)
            //    the infinitely projected volume will leave a dark band
            // 2: finite extrusion on any light source since glancing angles
            //    can peek through the end and shadow objects behind incorrectly
            if ((flags & SRF_EXTRUDE_TO_INFINITY) &&
                light->getType() != Light::LT_DIRECTIONAL &&
                (mShadowTechnique & SHADOWDETAILTYPE_MODULATIVE) &&
                camera->isVisible(caster->getDarkCapBounds(*light, darkCapExtrudeDist)))
            {
                flags |= SRF_INCLUDE_DARK_CAP;
            }
            else if (!(flags & SRF_EXTRUDE_TO_INFINITY) &&
                camera->isVisible(caster->getDarkCapBounds(*light, darkCapExtrudeDist)))
            {
                flags |= SRF_INCLUDE_DARK_CAP;
            }

        }

        if(extrudeInSoftware) // convert to flag
            flags |= SRF_EXTRUDE_IN_SOFTWARE;

        // Get shadow renderables
        const ShadowRenderableList& shadowRenderables = caster->getShadowVolumeRenderableList(
            light, mShadowIndexBuffer, mShadowIndexBufferUsedSize, extrudeDist, flags);

        // Render a shadow volume here
        //  - if we have 2-sided stencil, one render with no culling
        //  - otherwise, 2 renders, one with each culling method and invert the ops
        setShadowVolumeStencilState(false, zfailAlgo, stencil2sided);
        renderShadowVolumeObjects(shadowRenderables, mShadowStencilPass, &lightList, flags,
            false, zfailAlgo, stencil2sided);
        if (!stencil2sided)
        {
            // Second pass
            setShadowVolumeStencilState(true, zfailAlgo, false);
            renderShadowVolumeObjects(shadowRenderables, mShadowStencilPass, &lightList, flags,
                true, zfailAlgo, false);
        }

        // Do we need to render a debug shadow marker?
        if (mDebugShadows)
        {
            // reset stencil & colour ops
            mDestRenderSystem->setStencilState(StencilState());

            auto shadowColour = mShadowColour;
            mShadowColour = zfailAlgo ? ColourValue(0.7, 0.0, 0.2) : ColourValue(0.0, 0.7, 0.2);
            mSceneManager->_setPass(mShadowDebugPass);
            renderShadowVolumeObjects(shadowRenderables, mShadowDebugPass, &lightList, flags,
                true, false, false);
            mDestRenderSystem->setColourBlendState(disabled);
            mDestRenderSystem->_setDepthBufferParams(true, false, CMPF_LESS);
            mShadowColour = shadowColour;
        }
    }

    // revert stencil state
    mDestRenderSystem->setStencilState(StencilState());

    mDestRenderSystem->unbindGpuProgram(GPT_VERTEX_PROGRAM);

    if (scissored == CLIPPED_SOME)
    {
        // disable scissor test
        mSceneManager->resetScissor();
    }

}
//---------------------------------------------------------------------
void SceneManager::ShadowRenderer::renderShadowVolumeObjects(const ShadowCaster::ShadowRenderableList& shadowRenderables,
                                             Pass* pass,
                                             const LightList *manualLightList,
                                             unsigned long flags,
                                             bool secondpass, bool zfail, bool twosided)
{
    // ----- SHADOW VOLUME LOOP -----
    // Render all shadow renderables with same stencil operations
    for (ShadowRenderable* sr : shadowRenderables)
    {
        // omit hidden renderables
        if (sr->isVisible())
        {
            // render volume, including dark and (maybe) light caps
            mSceneManager->renderSingleObject(sr, pass, false, false, manualLightList);

            // optionally render separate light cap
            if (sr->isLightCapSeparate() && (flags & SRF_INCLUDE_LIGHT_CAP))
            {
                ShadowRenderable* lightCap = sr->getLightCapRenderable();
                assert(lightCap && "Shadow renderable is missing a separate light cap renderable!");

                // We must take care with light caps when we could 'see' the back facing
                // triangles directly:
                //   1. The front facing light caps must render as always fail depth
                //      check to avoid 'depth fighting'.
                //   2. The back facing light caps must use normal depth function to
                //      avoid break the standard depth check
                //
                // TODO:
                //   1. Separate light caps rendering doesn't need for the 'closed'
                //      mesh that never touch the near plane, because in this instance,
                //      we couldn't 'see' any back facing triangles directly. The
                //      'closed' mesh must determinate by edge list builder.
                //   2. There still exists 'depth fighting' bug with coplane triangles
                //      that has opposite facing. This usually occur when use two side
                //      material in the modeling tools and the model exporting tools
                //      exporting double triangles to represent this model. This bug
                //      can't fixed in GPU only, there must has extra work on edge list
                //      builder and shadow volume generater to fix it.
                //
                if (twosided)
                {
                    // select back facing light caps to render
                    mDestRenderSystem->_setCullingMode(CULL_ANTICLOCKWISE);
                    mSceneManager->mPassCullingMode = CULL_ANTICLOCKWISE;
                    // use normal depth function for back facing light caps
                    mSceneManager->renderSingleObject(lightCap, pass, false, false, manualLightList);

                    // select front facing light caps to render
                    mDestRenderSystem->_setCullingMode(CULL_CLOCKWISE);
                    mSceneManager->mPassCullingMode = CULL_CLOCKWISE;
                    // must always fail depth check for front facing light caps
                    mDestRenderSystem->_setDepthBufferParams(true, false, CMPF_ALWAYS_FAIL);
                    mSceneManager->renderSingleObject(lightCap, pass, false, false, manualLightList);

                    // reset depth function
                    mDestRenderSystem->_setDepthBufferParams(true, false, CMPF_LESS);
                    // reset culling mode
                    mDestRenderSystem->_setCullingMode(CULL_NONE);
                    mSceneManager->mPassCullingMode = CULL_NONE;
                }
                else if ((secondpass || zfail) && !(secondpass && zfail))
                {
                    // use normal depth function for back facing light caps
                    mSceneManager->renderSingleObject(lightCap, pass, false, false, manualLightList);
                }
                else
                {
                    // must always fail depth check for front facing light caps
                    mDestRenderSystem->_setDepthBufferParams(true, false, CMPF_ALWAYS_FAIL);
                    mSceneManager->renderSingleObject(lightCap, pass, false, false, manualLightList);

                    // reset depth function
                    mDestRenderSystem->_setDepthBufferParams(true, false, CMPF_LESS);
                }
            }
        }
    }
}
//---------------------------------------------------------------------
void SceneManager::ShadowRenderer::setShadowVolumeStencilState(bool secondpass, bool zfail, bool twosided)
{
    // Determinate the best stencil operation
    StencilOperation incrOp, decrOp;
    if (mDestRenderSystem->getCapabilities()->hasCapability(RSC_STENCIL_WRAP))
    {
        incrOp = SOP_INCREMENT_WRAP;
        decrOp = SOP_DECREMENT_WRAP;
    }
    else
    {
        incrOp = SOP_INCREMENT;
        decrOp = SOP_DECREMENT;
    }

    // First pass, do front faces if zpass
    // Second pass, do back faces if zpass
    // Invert if zfail
    // this is to ensure we always increment before decrement
    // When two-sided stencil, always pass front face stencil
    // operation parameters and the inverse of them will happen
    // for back faces
    StencilState stencilState;
    stencilState.enabled = true;
    stencilState.compareOp = CMPF_ALWAYS_PASS; // always pass stencil check
    stencilState.twoSidedOperation = twosided;
    if ( !twosided && ((secondpass || zfail) && !(secondpass && zfail)) )
    {
        mSceneManager->mPassCullingMode = twosided? CULL_NONE : CULL_ANTICLOCKWISE;
        stencilState.depthFailOp = zfail ? incrOp : SOP_KEEP; // back face depth fail
        stencilState.depthStencilPassOp = zfail ? SOP_KEEP : decrOp; // back face pass
    }
    else
    {
        mSceneManager->mPassCullingMode = twosided? CULL_NONE : CULL_CLOCKWISE;
        stencilState.depthFailOp = zfail ? decrOp : SOP_KEEP; // front face depth fail
        stencilState.depthStencilPassOp = zfail ? SOP_KEEP : incrOp; // front face pass
    }
    mDestRenderSystem->setStencilState(stencilState);
    mDestRenderSystem->_setCullingMode(mSceneManager->mPassCullingMode);

}
void SceneManager::ShadowRenderer::setShadowTextureCasterMaterial(const MaterialPtr& mat)
{
    if(!mat) {
        mShadowTextureCustomCasterPass = 0;
        return;
    }

    mat->load();
    if (!mat->getBestTechnique())
    {
        // unsupported
        mShadowTextureCustomCasterPass = 0;
    }
    else
    {
        OgreAssert(!mat->getTechnique(0)->getPasses().empty(), "technique 0 has no passes");
        mShadowTextureCustomCasterPass = mat->getTechnique(0)->getPass(0);
    }
}
//---------------------------------------------------------------------
void SceneManager::ShadowRenderer::setShadowTextureReceiverMaterial(const MaterialPtr& mat)
{
    if(!mat) {
        mShadowTextureCustomReceiverPass = 0;
        return;
    }

    mat->load();
    if (!mat->getBestTechnique())
    {
        // unsupported
        mShadowTextureCustomReceiverPass = 0;
    }
    else
    {
        OgreAssert(!mat->getTechnique(0)->getPasses().empty(), "technique 0 has no passes");
        mShadowTextureCustomReceiverPass = mat->getTechnique(0)->getPass(0);
    }
}
//---------------------------------------------------------------------
void SceneManager::ShadowRenderer::setShadowTechnique(ShadowTechnique technique)
{
    mShadowTechnique = technique;
    if (mShadowTechnique & SHADOWDETAILTYPE_STENCIL)
    {
        // Firstly check that we  have a stencil
        // Otherwise forget it
        if (!mDestRenderSystem->getCapabilities()->hasCapability(RSC_HWSTENCIL))
        {
            LogManager::getSingleton().logWarning(
                "Stencil shadows were requested, but this device does not "
                "have a hardware stencil. Shadows disabled.");
            mShadowTechnique = SHADOWTYPE_NONE;
        }
        else if (!mShadowIndexBuffer)
        {
            // Create an estimated sized shadow index buffer
            mShadowIndexBuffer = HardwareBufferManager::getSingleton().
                createIndexBuffer(HardwareIndexBuffer::IT_16BIT,
                mShadowIndexBufferSize,
                HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE,
                false);
            // tell all meshes to prepare shadow volumes
            MeshManager::getSingleton().setPrepareAllMeshesForShadowVolumes(true);
        }
    }

    if (mShadowTechnique == SHADOWTYPE_TEXTURE_MODULATIVE && !mSpotFadeTexture)
        mSpotFadeTexture = TextureManager::getSingleton().load("spot_shadow_fade.dds", RGN_INTERNAL);

    if ((mShadowTechnique & SHADOWDETAILTYPE_TEXTURE) == 0)
    {
        // Destroy shadow textures to optimise resource usage
        destroyShadowTextures();
        mSpotFadeTexture.reset();
    }
    else
    {
        // assure no custom shadow matrix is used accidentally in case we switch
        // from a custom shadow mapping type to a non-custom (uniform shadow mapping)
        for ( Camera* texCam : mShadowTextureCameras)
        {
            texCam->setCustomViewMatrix(false);
            texCam->setCustomProjectionMatrix(false);
        }
    }

    if(mShadowTechnique)
        initShadowVolumeMaterials();
}
//---------------------------------------------------------------------
void SceneManager::ShadowRenderer::initShadowVolumeMaterials()
{
    /* This should have been set in the SceneManager constructor, but if you
       created the SceneManager BEFORE the Root object, you will need to call
       SceneManager::_setDestinationRenderSystem manually.
     */
    OgreAssert( mDestRenderSystem, "no RenderSystem");

    if (mShadowMaterialInitDone)
        return;

    if (!mShadowDebugPass)
    {
        ShadowVolumeExtrudeProgram::initialise();
        MaterialPtr matDebug = MaterialManager::getSingleton().getByName("Ogre/Debug/ShadowVolumes");
        mShadowDebugPass = matDebug->getTechnique(0)->getPass(0);
        msInfiniteExtrusionParams = mShadowDebugPass->getVertexProgramParameters();
    }

    if (!mShadowStencilPass)
    {
        MaterialPtr matStencil = MaterialManager::getSingleton().getByName("Ogre/StencilShadowVolumes");
        mShadowStencilPass = matStencil->getTechnique(0)->getPass(0);
        msFiniteExtrusionParams = mShadowStencilPass->getVertexProgramParameters();
    }

    if (!mShadowModulativePass)
    {
        MaterialPtr matModStencil = MaterialManager::getSingleton().getByName("Ogre/StencilShadowModulationPass");
        matModStencil->load();
        mShadowModulativePass = matModStencil->getTechnique(0)->getPass(0);
    }

    // Also init full screen quad while we're at it
    if (!mFullScreenQuad)
    {
        mFullScreenQuad = mSceneManager->createScreenSpaceRect();
    }

    // Also init shadow caster material for texture shadows
    if (!mShadowCasterPlainBlackPass)
    {
        MaterialPtr matPlainBlack = MaterialManager::getSingleton().getByName("Ogre/TextureShadowCaster");
        matPlainBlack->load();
        mShadowCasterPlainBlackPass = matPlainBlack->getTechnique(0)->getPass(0);
    }

    if (!mShadowReceiverPass)
    {
        MaterialPtr matShadRec = MaterialManager::getSingleton().getByName("Ogre/TextureShadowReceiver", RGN_INTERNAL);
        if (!matShadRec)
        {
            matShadRec = MaterialManager::getSingleton().create("Ogre/TextureShadowReceiver", RGN_INTERNAL);
            mShadowReceiverPass = matShadRec->getTechnique(0)->getPass(0);
            // Don't set lighting and blending modes here, depends on additive / modulative
            TextureUnitState* t = mShadowReceiverPass->createTextureUnitState();
            t->setProjectiveTexturing(true, NULL); // will be set later, but the RTSS needs to know about this
        }
        else
        {
            mShadowReceiverPass = matShadRec->getTechnique(0)->getPass(0);
        }
    }

    mShadowMaterialInitDone = true;
}
const Pass* SceneManager::ShadowRenderer::deriveShadowCasterPass(const Pass* pass)
{
    if (mShadowTechnique & SHADOWDETAILTYPE_TEXTURE)
    {
        Pass* retPass;
        if (pass->getParent()->getShadowCasterMaterial())
        {
            return pass->getParent()->getShadowCasterMaterial()->getBestTechnique()->getPass(0);
        }
        else
        {
            retPass = mShadowTextureCustomCasterPass ?
                mShadowTextureCustomCasterPass : mShadowCasterPlainBlackPass;
        }


        // Special case alpha-blended passes
        if ((pass->getSourceBlendFactor() == SBF_SOURCE_ALPHA &&
            pass->getDestBlendFactor() == SBF_ONE_MINUS_SOURCE_ALPHA)
            || pass->getAlphaRejectFunction() != CMPF_ALWAYS_PASS)
        {
            // Alpha blended passes must retain their transparency
            retPass->setAlphaRejectSettings(pass->getAlphaRejectFunction(),
                pass->getAlphaRejectValue());
            retPass->setSceneBlending(pass->getSourceBlendFactor(), pass->getDestBlendFactor());
            retPass->getParent()->getParent()->setTransparencyCastsShadows(true);

            // So we allow the texture units, but override the colour functions
            // Copy texture state, shift up one since 0 is shadow texture
            unsigned short origPassTUCount = pass->getNumTextureUnitStates();
            for (unsigned short t = 0; t < origPassTUCount; ++t)
            {
                TextureUnitState* tex;
                if (retPass->getNumTextureUnitStates() <= t)
                {
                    tex = retPass->createTextureUnitState();
                }
                else
                {
                    tex = retPass->getTextureUnitState(t);
                }
                // copy base state
                (*tex) = *(pass->getTextureUnitState(t));
                // override colour function
                tex->setColourOperationEx(LBX_SOURCE1, LBS_MANUAL, LBS_CURRENT,
                        mShadowTechnique & SHADOWDETAILTYPE_ADDITIVE ? ColourValue::Black : mShadowColour);

            }
            // Remove any extras
            while (retPass->getNumTextureUnitStates() > origPassTUCount)
            {
                retPass->removeTextureUnitState(origPassTUCount);
            }

        }
        else
        {
            // reset
            retPass->setSceneBlending(SBT_REPLACE);
            retPass->setAlphaRejectFunction(CMPF_ALWAYS_PASS);
            while (retPass->getNumTextureUnitStates() > 0)
            {
                retPass->removeTextureUnitState(0);
            }
        }

        // give the RTSS a chance to generate a better technique
        retPass->getParent()->getParent()->load();

        Technique* btech = retPass->getParent()->getParent()->getBestTechnique();
        if( btech )
        {
            retPass = btech->getPass(0);
        }

        // Propagate culling modes
        retPass->setCullingMode(pass->getCullingMode());
        retPass->setManualCullingMode(pass->getManualCullingMode());

        return retPass;
    }
    else
    {
        return pass;
    }

}
//---------------------------------------------------------------------
const Pass* SceneManager::ShadowRenderer::deriveShadowReceiverPass(const Pass* pass)
{

    if (mShadowTechnique & SHADOWDETAILTYPE_TEXTURE)
    {
        if (pass->getParent()->getShadowReceiverMaterial())
        {
            return pass->getParent()->getShadowReceiverMaterial()->getBestTechnique()->getPass(0);
        }

        Pass* retPass = mShadowTextureCustomReceiverPass ? mShadowTextureCustomReceiverPass : mShadowReceiverPass;

        unsigned short keepTUCount;
        // If additive, need lighting parameters & standard programs
        if (mShadowTechnique & SHADOWDETAILTYPE_ADDITIVE)
        {
            retPass->setLightingEnabled(true);
            retPass->setAmbient(pass->getAmbient());
            retPass->setSelfIllumination(pass->getSelfIllumination());
            retPass->setDiffuse(pass->getDiffuse());
            retPass->setSpecular(pass->getSpecular());
            retPass->setShininess(pass->getShininess());
            retPass->setLightMask(pass->getLightMask());

            // We need to keep alpha rejection settings
            retPass->setAlphaRejectSettings(pass->getAlphaRejectFunction(),
                pass->getAlphaRejectValue());
            // Copy texture state, shift up one since 0 is shadow texture
            unsigned short origPassTUCount = pass->getNumTextureUnitStates();
            for (unsigned short t = 0; t < origPassTUCount; ++t)
            {
                unsigned short targetIndex = t+1;
                TextureUnitState* tex;
                if (retPass->getNumTextureUnitStates() <= targetIndex)
                {
                    tex = retPass->createTextureUnitState();
                }
                else
                {
                    tex = retPass->getTextureUnitState(targetIndex);
                }
                (*tex) = *(pass->getTextureUnitState(t));
                // If programmable, have to adjust the texcoord sets too
                // D3D insists that texcoordsets match tex unit in programmable mode
                if (retPass->hasVertexProgram())
                    tex->setTextureCoordSet(targetIndex);
            }
            keepTUCount = origPassTUCount + 1;
        }// additive lighting
        else
        {
            // need to keep spotlight fade etc
            keepTUCount = retPass->getNumTextureUnitStates();
        }

        // re-/set light iteration settings
        retPass->setIteratePerLight(pass->getIteratePerLight(), pass->getRunOnlyForOneLightType(),
                                    pass->getOnlyLightType());

        // Remove any extra texture units
        while (retPass->getNumTextureUnitStates() > keepTUCount)
        {
            retPass->removeTextureUnitState(keepTUCount);
        }

        // give the RTSS a chance to generate a better technique
        retPass->getParent()->getParent()->load();

        Technique* btech = retPass->getParent()->getParent()->getBestTechnique();
        if( btech )
        {
            retPass = btech->getPass(0);
        }

        return retPass;
    }
    else
    {
        return pass;
    }

}
//---------------------------------------------------------------------
const VisibleObjectsBoundsInfo&
SceneManager::ShadowRenderer::getShadowCasterBoundsInfo( const Light* light, size_t iteration ) const
{
    static VisibleObjectsBoundsInfo nullBox;

    // find light
    unsigned int foundCount = 0;
    ShadowCamLightMapping::const_iterator it;
    for ( it = mShadowCamLightMapping.begin() ; it != mShadowCamLightMapping.end(); ++it )
    {
        if ( it->second == light )
        {
            if (foundCount == iteration)
            {
                // search the camera-aab list for the texture cam
                auto camIt = mSceneManager->mCamVisibleObjectsMap.find( it->first );

                if ( camIt == mSceneManager->mCamVisibleObjectsMap.end() )
                {
                    return nullBox;
                }
                else
                {
                    return camIt->second;
                }
            }
            else
            {
                // multiple shadow textures per light, keep searching
                ++foundCount;
            }
        }
    }

    // AAB not available
    return nullBox;
}
//---------------------------------------------------------------------
void SceneManager::ShadowRenderer::setShadowIndexBufferSize(size_t size)
{
    if (mShadowIndexBuffer && size != mShadowIndexBufferSize)
    {
        // re-create shadow buffer with new size
        mShadowIndexBuffer = HardwareBufferManager::getSingleton().
            createIndexBuffer(HardwareIndexBuffer::IT_16BIT,
            size,
            HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE,
            false);
    }
    mShadowIndexBufferSize = size;
    mShadowIndexBufferUsedSize = 0;
}
//---------------------------------------------------------------------
void SceneManager::ShadowRenderer::setShadowTextureConfig(size_t shadowIndex, unsigned short width,
    unsigned short height, PixelFormat format, unsigned short fsaa, uint16 depthBufferPoolId )
{
    ShadowTextureConfig conf;
    conf.width = width;
    conf.height = height;
    conf.format = format;
    conf.fsaa = fsaa;
    conf.depthBufferPoolId = depthBufferPoolId;

    setShadowTextureConfig(shadowIndex, conf);


}
//---------------------------------------------------------------------
void SceneManager::ShadowRenderer::setShadowTextureConfig(size_t shadowIndex,
    const ShadowTextureConfig& config)
{
    if (shadowIndex >= mShadowTextureConfigList.size())
    {
        OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
            "shadowIndex out of bounds",
            "SceneManager::setShadowTextureConfig");
    }
    mShadowTextureConfigList[shadowIndex] = config;

    mShadowTextureConfigDirty = true;
}
//---------------------------------------------------------------------
void SceneManager::ShadowRenderer::setShadowTextureSize(unsigned short size)
{
    // default all current
    for (ShadowTextureConfigList::iterator i = mShadowTextureConfigList.begin();
        i != mShadowTextureConfigList.end(); ++i)
    {
        if (i->width != size || i->height != size)
        {
            i->width = i->height = size;
            mShadowTextureConfigDirty = true;
        }
    }

}
//---------------------------------------------------------------------
void SceneManager::ShadowRenderer::setShadowTextureCount(size_t count)
{
    // Change size, any new items will need defaults
    if (count != mShadowTextureConfigList.size())
    {
        // if no entries yet, use the defaults
        if (mShadowTextureConfigList.empty())
        {
            mShadowTextureConfigList.resize(count);
        }
        else
        {
            // create new instances with the same settings as the last item in the list
            mShadowTextureConfigList.resize(count, *mShadowTextureConfigList.rbegin());
        }
        mShadowTextureConfigDirty = true;
    }
}
//---------------------------------------------------------------------
void SceneManager::ShadowRenderer::setShadowTexturePixelFormat(PixelFormat fmt)
{
    for (ShadowTextureConfigList::iterator i = mShadowTextureConfigList.begin();
        i != mShadowTextureConfigList.end(); ++i)
    {
        if (i->format != fmt)
        {
            i->format = fmt;
            mShadowTextureConfigDirty = true;
        }
    }
}
void SceneManager::ShadowRenderer::setShadowTextureFSAA(unsigned short fsaa)
{
    for (ShadowTextureConfigList::iterator i = mShadowTextureConfigList.begin();
                i != mShadowTextureConfigList.end(); ++i)
    {
        if (i->fsaa != fsaa)
        {
            i->fsaa = fsaa;
            mShadowTextureConfigDirty = true;
        }
    }
}
//---------------------------------------------------------------------
void SceneManager::ShadowRenderer::setShadowTextureSettings(unsigned short size,
    unsigned short count, PixelFormat fmt, unsigned short fsaa, uint16 depthBufferPoolId)
{
    setShadowTextureCount(count);
    for (ShadowTextureConfigList::iterator i = mShadowTextureConfigList.begin();
        i != mShadowTextureConfigList.end(); ++i)
    {
        if (i->width != size || i->height != size || i->format != fmt || i->fsaa != fsaa)
        {
            i->width = i->height = size;
            i->format = fmt;
            i->fsaa = fsaa;
            i->depthBufferPoolId = depthBufferPoolId;
            mShadowTextureConfigDirty = true;
        }
    }
}
//---------------------------------------------------------------------
const TexturePtr& SceneManager::ShadowRenderer::getShadowTexture(size_t shadowIndex)
{
    if (shadowIndex >= mShadowTextureConfigList.size())
    {
        OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
            "shadowIndex out of bounds",
            "SceneManager::getShadowTexture");
    }
    ensureShadowTexturesCreated();

    return mShadowTextures[shadowIndex];
}

void SceneManager::ShadowRenderer::resolveShadowTexture(TextureUnitState* tu, size_t shadowIndex, size_t shadowTexUnitIndex) const
{
    Camera* cam = NULL;
    TexturePtr shadowTex;
    if (shadowIndex < mShadowTextures.size())
    {
        shadowTex = mShadowTextures[shadowIndex];
        // Hook up projection frustum
        cam = shadowTex->getBuffer()->getRenderTarget()->getViewport(0)->getCamera();
        // Enable projective texturing if fixed-function, but also need to
        // disable it explicitly for program pipeline.
        tu->setProjectiveTexturing(!tu->getParent()->hasVertexProgram(), cam);
    }
    else
    {
        // Use fallback 'null' shadow texture
        // no projection since all uniform colour anyway
        shadowTex = mNullShadowTexture;
        tu->setProjectiveTexturing(false);
    }
    mSceneManager->mAutoParamDataSource->setTextureProjector(cam, shadowTexUnitIndex);
    tu->_setTexturePtr(shadowTex);
}

//---------------------------------------------------------------------
bool SceneManager::ShadowRenderer::ShadowCasterSceneQueryListener::queryResult(
    MovableObject* object)
{
    if (object->getCastShadows() && object->isVisible() &&
        mSceneMgr->isRenderQueueToBeProcessed(object->getRenderQueueGroup()) &&
        // objects need an edge list to cast shadows (shadow volumes only)
        ((mSceneMgr->getShadowTechnique() & SHADOWDETAILTYPE_TEXTURE) ||
        ((mSceneMgr->getShadowTechnique() & SHADOWDETAILTYPE_STENCIL) && object->hasEdgeList())
        )
       )
    {
        if (mFarDistSquared)
        {
            // Check object is within the shadow far distance
            Real dist =  object->getParentNode()->getSquaredViewDepth(mCamera);
            Real radius = object->getBoundingRadiusScaled();
            if (dist - (radius * radius) > mFarDistSquared)
            {
                // skip, beyond max range
                return true;
            }
        }

        // If the object is in the frustum, we can always see the shadow
        if (mCamera->isVisible(object->getWorldBoundingBox()))
        {
            mCasterList->push_back(object);
            return true;
        }

        // Otherwise, object can only be casting a shadow into our view if
        // the light is outside the frustum (or it's a directional light,
        // which are always outside), and the object is intersecting
        // on of the volumes formed between the edges of the frustum and the
        // light
        if (!mIsLightInFrustum || mLight->getType() == Light::LT_DIRECTIONAL)
        {
            // Iterate over volumes
            PlaneBoundedVolumeList::const_iterator i, iend;
            iend = mLightClipVolumeList->end();
            for (i = mLightClipVolumeList->begin(); i != iend; ++i)
            {
                if (i->intersects(object->getWorldBoundingBox()))
                {
                    mCasterList->push_back(object);
                    return true;
                }

            }

        }
    }
    return true;
}
//---------------------------------------------------------------------
bool SceneManager::ShadowRenderer::ShadowCasterSceneQueryListener::queryResult(
    SceneQuery::WorldFragment* fragment)
{
    // don't deal with world geometry
    return true;
}
//---------------------------------------------------------------------
const SceneManager::ShadowRenderer::ShadowCasterList&
SceneManager::ShadowRenderer::findShadowCastersForLight(const Light* light, const Camera* camera)
{
    mShadowCasterList.clear();

    if (light->getType() == Light::LT_DIRECTIONAL)
    {
        // Basic AABB query encompassing the frustum and the extrusion of it
        AxisAlignedBox aabb;
        const Vector3* corners = camera->getWorldSpaceCorners();
        Vector3 min, max;
        Vector3 extrude = light->getDerivedDirection() * -mShadowDirLightExtrudeDist;
        // do first corner
        min = max = corners[0];
        min.makeFloor(corners[0] + extrude);
        max.makeCeil(corners[0] + extrude);
        for (size_t c = 1; c < 8; ++c)
        {
            min.makeFloor(corners[c]);
            max.makeCeil(corners[c]);
            min.makeFloor(corners[c] + extrude);
            max.makeCeil(corners[c] + extrude);
        }
        aabb.setExtents(min, max);

        if (!mShadowCasterAABBQuery)
            mShadowCasterAABBQuery.reset(mSceneManager->createAABBQuery(aabb));
        else
            mShadowCasterAABBQuery->setBox(aabb);
        // Execute, use callback
        mShadowCasterQueryListener->prepare(false,
            &(light->_getFrustumClipVolumes(camera)),
            light, camera, &mShadowCasterList, light->getShadowFarDistanceSquared());
        mShadowCasterAABBQuery->execute(mShadowCasterQueryListener.get());


    }
    else
    {
        Sphere s(light->getDerivedPosition(), light->getAttenuationRange());
        // eliminate early if camera cannot see light sphere
        if (camera->isVisible(s))
        {
            if (!mShadowCasterSphereQuery)
                mShadowCasterSphereQuery.reset(mSceneManager->createSphereQuery(s));
            else
                mShadowCasterSphereQuery->setSphere(s);

            // Determine if light is inside or outside the frustum
            bool lightInFrustum = camera->isVisible(light->getDerivedPosition());
            const PlaneBoundedVolumeList* volList = 0;
            if (!lightInFrustum)
            {
                // Only worth building an external volume list if
                // light is outside the frustum
                volList = &(light->_getFrustumClipVolumes(camera));
            }

            // Execute, use callback
            mShadowCasterQueryListener->prepare(lightInFrustum,
                volList, light, camera, &mShadowCasterList, light->getShadowFarDistanceSquared());
            mShadowCasterSphereQuery->execute(mShadowCasterQueryListener.get());

        }

    }


    return mShadowCasterList;
}
//---------------------------------------------------------------------
void SceneManager::ShadowRenderer::fireShadowTexturesUpdated(size_t numberOfShadowTextures)
{
    ListenerList listenersCopy = mListeners;
    ListenerList::iterator i, iend;

    iend = listenersCopy.end();
    for (i = listenersCopy.begin(); i != iend; ++i)
    {
        (*i)->shadowTexturesUpdated(numberOfShadowTextures);
    }
}
//---------------------------------------------------------------------
void SceneManager::ShadowRenderer::fireShadowTexturesPreCaster(Light* light, Camera* camera, size_t iteration)
{
    auto listenersCopy = mListeners;
    for (auto l : listenersCopy)
    {
        l->shadowTextureCasterPreViewProj(light, camera, iteration);
    }
}
//---------------------------------------------------------------------
void SceneManager::ShadowRenderer::fireShadowTexturesPreReceiver(Light* light, Frustum* f)
{
    ListenerList listenersCopy = mListeners;
    ListenerList::iterator i, iend;

    iend = listenersCopy.end();
    for (i = listenersCopy.begin(); i != iend; ++i)
    {
        (*i)->shadowTextureReceiverPreViewProj(light, f);
    }
}
void SceneManager::ShadowRenderer::sortLightsAffectingFrustum(LightList& lightList)
{
    if ((mShadowTechnique & SHADOWDETAILTYPE_TEXTURE) == 0)
        return;
    // Sort the lights if using texture shadows, since the first 'n' will be
    // used to generate shadow textures and we should pick the most appropriate

    // Allow a Listener to override light sorting
    // Reverse iterate so last takes precedence
    bool overridden = false;
    ListenerList listenersCopy = mListeners;
    for (ListenerList::reverse_iterator ri = listenersCopy.rbegin();
        ri != listenersCopy.rend(); ++ri)
    {
        overridden = (*ri)->sortLightsAffectingFrustum(lightList);
        if (overridden)
            break;
    }
    if (!overridden)
    {
        // default sort (stable to preserve directional light ordering
        std::stable_sort(lightList.begin(), lightList.end(), lightsForShadowTextureLess());
    }
}
}
