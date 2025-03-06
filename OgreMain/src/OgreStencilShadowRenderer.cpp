// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#include "OgreStableHeaders.h"
#include "OgreRectangle2D.h"
#include "OgreShadowVolumeExtrudeProgram.h"

namespace Ogre {

GpuProgramParametersSharedPtr SceneManager::StencilShadowRenderer::msInfiniteExtrusionParams;
GpuProgramParametersSharedPtr SceneManager::StencilShadowRenderer::msFiniteExtrusionParams;

typedef std::vector<ShadowCaster*> ShadowCasterList;

/// Inner class to use as callback for shadow caster scene query
class ShadowCasterSceneQueryListener : public SceneQueryListener
{
protected:
    SceneManager* mSceneMgr;
    ShadowCasterList* mCasterList;
    bool mIsLightInFrustum;
    const PlaneBoundedVolumeList* mLightClipVolumeList;
    const Camera* mCamera;
    const Light* mLight;
    Real mFarDistSquared;
public:
    ShadowCasterSceneQueryListener(SceneManager* sm) : mSceneMgr(sm),
        mCasterList(0), mIsLightInFrustum(false), mLightClipVolumeList(0),
        mCamera(0), mFarDistSquared(0) {}
    // Prepare the listener for use with a set of parameters
    void prepare(bool lightInFrustum, const PlaneBoundedVolumeList* lightClipVolumes, const Light* light,
                 const Camera* cam, ShadowCasterList* casterList, Real farDistSquared)
    {
        mCasterList = casterList;
        mIsLightInFrustum = lightInFrustum;
        mLightClipVolumeList = lightClipVolumes;
        mCamera = cam;
        mLight = light;
        mFarDistSquared = farDistSquared;
    }
    bool queryResult(MovableObject* object) override;
};

SceneManager::StencilShadowRenderer::StencilShadowRenderer(SceneManager* owner) :
mSceneManager(owner),
mShadowModulativePass(0),
mShadowDebugPass(0),
mShadowStencilPass(0),
mShadowIndexBufferSize(51200),
mShadowIndexBufferUsedSize(0),
mFullScreenQuad(0),
mShadowAdditiveLightClip(false),
mDebugShadows(false),
mShadowMaterialInitDone(false),
mShadowUseInfiniteFarPlane(true),
mShadowDirLightExtrudeDist(10000)
{
    mShadowCasterQueryListener = std::make_unique<ShadowCasterSceneQueryListener>(mSceneManager);
}

SceneManager::StencilShadowRenderer::~StencilShadowRenderer() {}

void SceneManager::StencilShadowRenderer::render(RenderQueueGroup* group,
                                          QueuedRenderableCollection::OrganisationMode om)
{
    if(mSceneManager->isShadowTechniqueAdditive())
    {
        // Additive stencil shadows in use
        renderAdditiveStencilShadowedQueueGroupObjects(group, om);
        return;
    }

    // Modulative stencil shadows in use
    renderModulativeStencilShadowedQueueGroupObjects(group, om);
}
//-----------------------------------------------------------------------
void SceneManager::StencilShadowRenderer::renderAdditiveStencilShadowedQueueGroupObjects(
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
}
//-----------------------------------------------------------------------
void SceneManager::StencilShadowRenderer::renderModulativeStencilShadowedQueueGroupObjects(
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

    // Do non-shadowable solids
    for (const auto& pg : pGroup->getPriorityGroups())
    {
        visitor->renderObjects(pg.second->getSolidsNoShadowReceive(), om, true, true);
    }
}
//---------------------------------------------------------------------
void SceneManager::StencilShadowRenderer::renderShadowVolumesToStencil(const Light* light,
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
    for (auto *caster : casters)
    {
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
                mSceneManager->isShadowTechniqueModulative() &&
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

            auto shadowColour = mSceneManager->getShadowColour();
            mSceneManager->setShadowColour(zfailAlgo ? ColourValue(0.7, 0.0, 0.2) : ColourValue(0.0, 0.7, 0.2));
            mSceneManager->_setPass(mShadowDebugPass);
            renderShadowVolumeObjects(shadowRenderables, mShadowDebugPass, &lightList, flags,
                true, false, false);
            mDestRenderSystem->setColourBlendState(disabled);
            mDestRenderSystem->_setDepthBufferParams(true, false, CMPF_LESS);
            mSceneManager->setShadowColour(shadowColour);
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
void SceneManager::StencilShadowRenderer::renderShadowVolumeObjects(const ShadowCaster::ShadowRenderableList& shadowRenderables,
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
void SceneManager::StencilShadowRenderer::setShadowVolumeStencilState(bool secondpass, bool zfail, bool twosided)
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
void SceneManager::StencilShadowRenderer::setShadowTechnique(ShadowTechnique technique)
{
    if (!mShadowIndexBuffer)
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

    initShadowVolumeMaterials();
}
//---------------------------------------------------------------------
void SceneManager::StencilShadowRenderer::initShadowVolumeMaterials()
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

    mShadowMaterialInitDone = true;
}
//---------------------------------------------------------------------
void SceneManager::StencilShadowRenderer::setShadowIndexBufferSize(size_t size)
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
bool ShadowCasterSceneQueryListener::queryResult(MovableObject* object)
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
const SceneManager::StencilShadowRenderer::ShadowCasterList&
SceneManager::StencilShadowRenderer::findShadowCastersForLight(const Light* light, const Camera* camera)
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
}
