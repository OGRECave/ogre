// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#include "OgreStableHeaders.h"
#include "OgreShadowCameraSetup.h"
#include "OgreViewport.h"
#include "OgreHardwarePixelBuffer.h"

namespace Ogre {

SceneManager::TextureShadowRenderer::TextureShadowRenderer(SceneManager* owner) :
mSceneManager(owner),
mShadowCasterPlainBlackPass(0),
mShadowReceiverPass(0),
mShadowTextureCustomCasterPass(0),
mShadowTextureCustomReceiverPass(0),
mDefaultShadowFarDist(0),
mDefaultShadowFarDistSquared(0),
mShadowTextureOffset(0.6),
mShadowTextureFadeStart(0.7),
mShadowTextureFadeEnd(0.9),
mShadowTextureSelfShadow(false),
mShadowTextureConfigDirty(true),
mShadowCasterRenderBackFaces(true)
{
    // set up default shadow camera setup
    mDefaultShadowCameraSetup = DefaultShadowCameraSetup::create();

    mCullCameraSetup = DefaultShadowCameraSetup::create();

    // init shadow texture count per type.
    mShadowTextureCountPerType[Light::LT_POINT] = 1;
    mShadowTextureCountPerType[Light::LT_DIRECTIONAL] = 1;
    mShadowTextureCountPerType[Light::LT_SPOTLIGHT] = 1;
}

SceneManager::TextureShadowRenderer::~TextureShadowRenderer() {}

void SceneManager::TextureShadowRenderer::render(RenderQueueGroup* group,
                                          QueuedRenderableCollection::OrganisationMode om)
{
    // Receiver pass(es)
    if (mSceneManager->isShadowTechniqueAdditive())
    {
        // Auto-additive
        renderAdditiveTextureShadowedQueueGroupObjects(group, om);
        return;
    }

    // Modulative
    renderModulativeTextureShadowedQueueGroupObjects(group, om);
}

size_t SceneManager::TextureShadowRenderer::getShadowTexIndex(size_t startLightIndex)
{
    size_t shadowTexIndex = mShadowTextures.size();
    if (mShadowTextureIndexLightList.size() > startLightIndex)
        shadowTexIndex = mShadowTextureIndexLightList[startLightIndex];
    return shadowTexIndex;
}
//-----------------------------------------------------------------------
void SceneManager::TextureShadowRenderer::renderTextureShadowCasterQueueGroupObjects(
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
    if (mSceneManager->isShadowTechniqueAdditive())
    {
        // Use simple black / white mask if additive
        mSceneManager->setAmbientLight(ColourValue::Black);
    }
    else
    {
        // Use shadow colour as caster colour if modulative
        mSceneManager->setAmbientLight(mSceneManager->getShadowColour());
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
void SceneManager::TextureShadowRenderer::renderModulativeTextureShadowedQueueGroupObjects(
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
}
//-----------------------------------------------------------------------
void SceneManager::TextureShadowRenderer::renderAdditiveTextureShadowedQueueGroupObjects(
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
                if(mSceneManager->getShadowUseLightClipPlanes())
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
}
//-----------------------------------------------------------------------
void SceneManager::TextureShadowRenderer::renderTextureShadowReceiverQueueGroupObjects(
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
void SceneManager::TextureShadowRenderer::ensureShadowTexturesCreated()
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
        for (auto& shadowTex : mShadowTextures)
        {
            // Camera names are local to SM
            String camName = shadowTex->getName() + "Cam";

            RenderTexture *shadowRTT = shadowTex->getBuffer()->getRenderTarget();

            //Set appropriate depth buffer
            if(!PixelUtil::isDepth(shadowRTT->suggestPixelFormat()))
                shadowRTT->setDepthBufferPool( mShadowTextureConfigList[__i].depthBufferPoolId );

            // Create camera for this texture, but note that we have to rebind
            // in prepareShadowTextures to coexist with multiple SMs
            Camera* cam = mSceneManager->createCamera(camName);
            cam->setAspectRatio((Real)shadowTex->getWidth() / (Real)shadowTex->getHeight());
            auto camNode = mSceneManager->getRootSceneNode()->createChildSceneNode();
            camNode->attachObject(cam);
            mShadowTextureCameras.push_back(cam);

            // use separate culling camera, in case a focused shadow setup is used
            // in which case we want to keep the original light frustum for culling
            Camera* cullCam = mSceneManager->createCamera(camName+"/Cull");
            cullCam->setAspectRatio((Real)shadowTex->getWidth() / (Real)shadowTex->getHeight());
            cam->setCullingFrustum(cullCam);
            camNode->attachObject(cullCam);


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
            ++__i;
        }
        mShadowTextureConfigDirty = false;
    }

}
//---------------------------------------------------------------------
void SceneManager::TextureShadowRenderer::destroyShadowTextures(void)
{
    for (auto cam : mShadowTextureCameras)
    {
        mSceneManager->getRootSceneNode()->removeAndDestroyChild(cam->getParentSceneNode());

        // Always destroy camera since they are local to this SM
        if(auto cullcam = dynamic_cast<Camera*>(cam->getCullingFrustum()))
            mSceneManager->destroyCamera(cullcam);

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
void SceneManager::TextureShadowRenderer::prepareShadowTextures(Camera* cam, Viewport* vp, const LightList* lightList)
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
    if (!mSceneManager->isShadowTechniqueAdditive())
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

        mDestRenderSystem->_setDepthClamp(light->getType() == Light::LT_DIRECTIONAL);

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
            {
#ifdef OGRE_NODELESS_POSITIONING
                texCam->getParentSceneNode()->setDirection(light->getDerivedDirection(), Node::TS_WORLD);
#else
                texCam->getParentSceneNode()->setOrientation(light->getParentNode()->_getDerivedOrientation());
#endif
            }
            if (light->getType() != Light::LT_DIRECTIONAL)
                texCam->getParentSceneNode()->setPosition(light->getDerivedPosition());

            // also update culling camera
            auto cullCam = dynamic_cast<Camera*>(texCam->getCullingFrustum());
            cullCam->_notifyViewport(shadowView);
            mCullCameraSetup->getShadowCamera(mSceneManager, cam, vp, light, cullCam, j);

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

        mDestRenderSystem->_setDepthClamp(false);

        // set the first shadow texture index for this light.
        mShadowTextureIndexLightList.push_back(shadowTextureIndex);
        shadowTextureIndex += textureCountPerLight;
    }

    fireShadowTexturesUpdated(std::min(lightList->size(), mShadowTextures.size()));

    ShadowTextureManager::getSingleton().clearUnused();
}


void SceneManager::TextureShadowRenderer::setShadowTextureCasterMaterial(const MaterialPtr& mat)
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
void SceneManager::TextureShadowRenderer::setShadowTextureReceiverMaterial(const MaterialPtr& mat)
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
void SceneManager::TextureShadowRenderer::setShadowTechnique(ShadowTechnique technique)
{
    if (mSceneManager->getShadowTechnique() == SHADOWTYPE_TEXTURE_MODULATIVE && !mSpotFadeTexture)
        mSpotFadeTexture = TextureManager::getSingleton().load("spot_shadow_fade.dds", RGN_INTERNAL);

    if (!mSceneManager->isShadowTechniqueTextureBased())
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

    if(!mSceneManager->getShadowTechnique())
        return;

    // init shadow caster material for texture shadows
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
}


const Pass* SceneManager::TextureShadowRenderer::deriveShadowCasterPass(const Pass* pass)
{
    Pass* retPass;
    if (pass->getParent()->getShadowCasterMaterial())
    {
        auto bestTech = pass->getParent()->getShadowCasterMaterial()->getBestTechnique();
        if (bestTech && bestTech->getNumPasses() > 0) {
            return bestTech->getPass(0);
        }
    }
    retPass = mShadowTextureCustomCasterPass ?
        mShadowTextureCustomCasterPass : mShadowCasterPlainBlackPass;

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
                                      mSceneManager->isShadowTechniqueAdditive() ? ColourValue::Black : mSceneManager->getShadowColour());
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
//---------------------------------------------------------------------
const Pass* SceneManager::TextureShadowRenderer::deriveShadowReceiverPass(const Pass* pass)
{
    if (pass->getParent()->getShadowReceiverMaterial())
    {
        return pass->getParent()->getShadowReceiverMaterial()->getBestTechnique()->getPass(0);
    }

    Pass* retPass = mShadowTextureCustomReceiverPass ? mShadowTextureCustomReceiverPass : mShadowReceiverPass;

    unsigned short keepTUCount;
    // If additive, need lighting parameters & standard programs
    if (mSceneManager->isShadowTechniqueAdditive())
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

const Pass* SceneManager::TextureShadowRenderer::deriveTextureShadowPass(const Pass* pass)
{
    if (mSceneManager->_getCurrentRenderStage() == IRS_RENDER_TO_TEXTURE)
    {
        // Derive a special shadow caster pass from this one
        return deriveShadowCasterPass(pass);
    }

    if (mSceneManager->_getCurrentRenderStage() == IRS_RENDER_RECEIVER_PASS)
    {
        return deriveShadowReceiverPass(pass);
    }

    return pass;
}

//---------------------------------------------------------------------
const VisibleObjectsBoundsInfo&
SceneManager::TextureShadowRenderer::getShadowCasterBoundsInfo( const Light* light, size_t iteration ) const
{
    static VisibleObjectsBoundsInfo nullBox;

    // find light
    unsigned int foundCount = 0;
    for (auto& m : mShadowCamLightMapping)
    {
        if (m.second == light )
        {
            if (foundCount == iteration)
            {
                // search the camera-aab list for the texture cam
                auto camIt = mSceneManager->mCamVisibleObjectsMap.find(m.first);

                if (camIt == mSceneManager->mCamVisibleObjectsMap.end())
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
void SceneManager::TextureShadowRenderer::setShadowTextureConfig(size_t shadowIndex, unsigned short width,
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
void SceneManager::TextureShadowRenderer::setShadowTextureConfig(size_t shadowIndex,
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
void SceneManager::TextureShadowRenderer::setShadowTextureSize(unsigned short size)
{
    // default all current
    for (auto & i : mShadowTextureConfigList)
    {
        if (i.width != size || i.height != size)
        {
            i.width = i.height = size;
            mShadowTextureConfigDirty = true;
        }
    }

}
//---------------------------------------------------------------------
void SceneManager::TextureShadowRenderer::setShadowTextureCount(size_t count)
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
void SceneManager::TextureShadowRenderer::setShadowTexturePixelFormat(PixelFormat fmt)
{
    for (auto & i : mShadowTextureConfigList)
    {
        if (i.format != fmt)
        {
            i.format = fmt;
            mShadowTextureConfigDirty = true;
        }
    }
}
void SceneManager::TextureShadowRenderer::setShadowTextureFSAA(unsigned short fsaa)
{
    for (auto & i : mShadowTextureConfigList)
    {
        if (i.fsaa != fsaa)
        {
            i.fsaa = fsaa;
            mShadowTextureConfigDirty = true;
        }
    }
}
//---------------------------------------------------------------------
void SceneManager::TextureShadowRenderer::setShadowTextureSettings(unsigned short size,
    unsigned short count, PixelFormat fmt, unsigned short fsaa, uint16 depthBufferPoolId)
{
    setShadowTextureCount(count);
    for (auto & i : mShadowTextureConfigList)
    {
        if (i.width != size || i.height != size || i.format != fmt || i.fsaa != fsaa)
        {
            i.width = i.height = size;
            i.format = fmt;
            i.fsaa = fsaa;
            i.depthBufferPoolId = depthBufferPoolId;
            mShadowTextureConfigDirty = true;
        }
    }
}
//---------------------------------------------------------------------
const TexturePtr& SceneManager::TextureShadowRenderer::getShadowTexture(size_t shadowIndex)
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

void SceneManager::TextureShadowRenderer::resolveShadowTexture(TextureUnitState* tu, size_t shadowIndex, size_t shadowTexUnitIndex) const
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

namespace
{
/** Default sorting routine which sorts lights which cast shadows
to the front of a list, sub-sorting by distance.

Since shadow textures are generated from lights based on the
frustum rather than individual objects, a shadow and camera-wise sort is
required to pick the best lights near the start of the list. Up to
the number of shadow textures will be generated from this.
*/
struct lightsForShadowTextureLess
{
    bool operator()(const Light* l1, const Light* l2) const
    {
        if (l1 == l2)
            return false;

        // sort shadow casting lights ahead of non-shadow casting
        if (l1->getCastShadows() != l2->getCastShadows())
        {
            return l1->getCastShadows();
        }

        // otherwise sort by distance (directional lights will have 0 here)
        return l1->tempSquareDist < l2->tempSquareDist;
    }
};
} // namespace

void SceneManager::TextureShadowRenderer::sortLightsAffectingFrustum(LightList& lightList) const
{
    // Sort the lights if using texture shadows, since the first 'n' will be
    // used to generate shadow textures and we should pick the most appropriate

    ListenerList listenersCopy = mListeners; // copy in case of listeners removing themselves

    // Allow a Listener to override light sorting
    // Reverse iterate so last takes precedence
    for (auto ri = listenersCopy.rbegin(); ri != listenersCopy.rend(); ++ri)
    {
        if((*ri)->sortLightsAffectingFrustum(lightList))
            return;
    }

    // default sort (stable to preserve directional light ordering
    std::stable_sort(lightList.begin(), lightList.end(), lightsForShadowTextureLess());
}
//---------------------------------------------------------------------
void SceneManager::TextureShadowRenderer::fireShadowTexturesUpdated(size_t numberOfShadowTextures)
{
    ListenerList listenersCopy = mListeners;

    for (auto *l : listenersCopy)
    {
        l->shadowTexturesUpdated(numberOfShadowTextures);
    }
}
//---------------------------------------------------------------------
void SceneManager::TextureShadowRenderer::fireShadowTexturesPreCaster(Light* light, Camera* camera, size_t iteration)
{
    auto listenersCopy = mListeners;
    for (auto l : listenersCopy)
    {
        l->shadowTextureCasterPreViewProj(light, camera, iteration);
    }
}
//---------------------------------------------------------------------
void SceneManager::TextureShadowRenderer::fireShadowTexturesPreReceiver(Light* light, Frustum* f)
{
    ListenerList listenersCopy = mListeners;
    for (auto *l : listenersCopy)
    {
        l->shadowTextureReceiverPreViewProj(light, f);
    }
}
}