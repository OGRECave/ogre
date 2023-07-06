/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#include "OgreCompositor.h"
#include "OgreCompositorChain.h"
#include "OgreCompositionTechnique.h"
#include "OgreCompositorInstance.h"
#include "OgreCompositionTargetPass.h"
#include "OgreCompositionPass.h"
#include "OgreCompositorManager.h"
#include "OgreRenderTarget.h"

namespace Ogre {
CompositorChain::CompositorChain(Viewport *vp):
    mViewport(vp),
    mOriginalScene(0),
    mDirty(true),
    mAnyCompositorsEnabled(false),
    mOldLodBias(1.0f)
{
    assert(vp);
    mOldClearEveryFrameBuffers = vp->getClearBuffers();
    vp->addListener(this);

    createOriginalScene();
    vp->getTarget()->addListener(this);
}
//-----------------------------------------------------------------------
CompositorChain::~CompositorChain()
{
    destroyResources();
}
//-----------------------------------------------------------------------
void CompositorChain::destroyResources(void)
{
    clearCompiledState();

    if (mViewport)
    {
        mViewport->getTarget()->removeListener(this);
        mViewport->removeListener(this);
        removeAllCompositors();
        destroyOriginalScene();

        // destory base "original scene" compositor
        CompositorManager::getSingleton().remove(getCompositorName(), ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME);

        mViewport = 0;
    }
}
//-----------------------------------------------------------------------
const String CompositorChain::getCompositorName() const
{
    return StringUtil::format("Ogre/Scene/%zu", (size_t)mViewport);
}
//-----------------------------------------------------------------------
void CompositorChain::createOriginalScene()
{
    /// Create "default" compositor
    /** Compositor that is used to implicitly represent the original
        render in the chain. This is an identity compositor with only an output pass:
    compositor Ogre/Scene
    {
        technique
        {
            target_output
            {
                pass clear
                {
                    /// Clear frame
                }
                pass render_scene
                {
                    visibility_mask 0xFFFFFFFF
                    render_queues SKIES_EARLY SKIES_LATE
                }
            }
        }
    };
    */

    // If two viewports use the same scheme but differ in settings like visibility masks, shadows, etc we don't
    // want compositors to share their technique.  Otherwise both compositors will have to recompile every time they
    // render.  Thus we generate a unique compositor per viewport.
    const String compName = getCompositorName();

    mOriginalSceneScheme = mViewport->getMaterialScheme();
    CompositorPtr scene = CompositorManager::getSingleton().getByName(compName, ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME);
    if (!scene)
    {
        /// Create base "original scene" compositor
        scene = CompositorManager::getSingleton().create(compName, ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME);
        CompositionTargetPass *tp = scene->createTechnique()->getOutputTargetPass();
        auto pass = tp->createPass(CompositionPass::PT_CLEAR);
        pass->setAutomaticColour(true);

        /// Render everything, including skies
        pass = tp->createPass(CompositionPass::PT_RENDERSCENE);
        pass->setFirstRenderQueue(RENDER_QUEUE_BACKGROUND);
        pass->setLastRenderQueue(RENDER_QUEUE_TRANSPARENTS);
        scene->load();
    }
    mOriginalScene = OGRE_NEW CompositorInstance(scene->getSupportedTechnique(), this);
}
//-----------------------------------------------------------------------
void CompositorChain::destroyOriginalScene()
{
    /// Destroy "original scene" compositor instance
    if (mOriginalScene)
    {
        OGRE_DELETE mOriginalScene;
        mOriginalScene = 0;
    }
}

//-----------------------------------------------------------------------
CompositorInstance* CompositorChain::addCompositor(const CompositorPtr& filter, size_t addPosition, const String& scheme)
{


    filter->touch();
    CompositionTechnique *tech = filter->getSupportedTechnique(scheme);
    if(!tech)
    {
        return 0;
    }
    CompositorInstance *t = OGRE_NEW CompositorInstance(tech, this);
    
    if(addPosition == LAST)
        addPosition = mInstances.size();
    else
        assert(addPosition <= mInstances.size() && "Index out of bounds.");
    mInstances.insert(mInstances.begin()+addPosition, t);
    
    mDirty = true;
    mAnyCompositorsEnabled = true;
    return t;
}
//-----------------------------------------------------------------------
void CompositorChain::removeCompositor(size_t index)
{
    if(index == LAST)
        index = mInstances.size() - 1;

    assert (index < mInstances.size() && "Index out of bounds.");
    Instances::iterator i = mInstances.begin() + index;
    OGRE_DELETE *i;
    mInstances.erase(i);
    
    mDirty = true;
}
//-----------------------------------------------------------------------
size_t CompositorChain::getNumCompositors()
{
    return mInstances.size();
}
//-----------------------------------------------------------------------
void CompositorChain::removeAllCompositors()
{
    for (auto *i : mInstances)
    {
        OGRE_DELETE i;
    }
    mInstances.clear();
    
    mDirty = true;
}
//-----------------------------------------------------------------------
void CompositorChain::_removeInstance(CompositorInstance *i)
{
    Instances::iterator it = std::find(mInstances.begin(), mInstances.end(), i);
    assert(it != mInstances.end());
    if(it != mInstances.end())
    {
        mInstances.erase(it);
        OGRE_DELETE i;
    }
}
//-----------------------------------------------------------------------
void CompositorChain::_queuedOperation(CompositorInstance::RenderSystemOperation* op)
{
    mRenderSystemOperations.push_back(op);

}
//-----------------------------------------------------------------------
size_t CompositorChain::getCompositorPosition(const String& name) const
{
    for (auto it = mInstances.begin(); it != mInstances.end(); ++it)
    {
        if ((*it)->getCompositor()->getName() == name) 
        {
            return std::distance(mInstances.begin(), it);
        }
    }
    return NPOS;
}
CompositorInstance *CompositorChain::getCompositor(const String& name) const
{
    size_t idx = getCompositorPosition(name);
    return idx == NPOS ? NULL : mInstances[idx];
}
//-----------------------------------------------------------------------
CompositorChain::InstanceIterator CompositorChain::getCompositors()
{
    return InstanceIterator(mInstances.begin(), mInstances.end());
}
//-----------------------------------------------------------------------
void CompositorChain::setCompositorEnabled(size_t position, bool state)
{
    CompositorInstance* inst = mInstances[position];
    if (!state && inst->getEnabled())
    {
        // If we're disabling a 'middle' compositor in a chain, we have to be
        // careful about textures which might have been shared by non-adjacent
        // instances which have now become adjacent. 
        CompositorInstance* nextInstance = getNextInstance(inst, true);
        if (nextInstance)
        {
            const CompositionTechnique::TargetPasses& tps =
                nextInstance->getTechnique()->getTargetPasses();
            CompositionTechnique::TargetPasses::const_iterator tpit = tps.begin();
            for(;tpit != tps.end(); ++tpit)
            {
                CompositionTargetPass* tp = *tpit;
                if (tp->getInputMode() == CompositionTargetPass::IM_PREVIOUS)
                {
                    if (nextInstance->getTechnique()->getTextureDefinition(tp->getOutputName())->pooled)
                    {
                        // recreate
                        nextInstance->freeResources(false, true);
                        nextInstance->createResources(false);
                    }
                }

            }
        }

    }
    inst->setEnabled(state);
}
//-----------------------------------------------------------------------
static const Quaternion& getCubemapRotation(int i)
{
    static const Quaternion CubemapRotations[6] = {
        Quaternion(Degree(-90), Vector3::UNIT_Y), //+X
        Quaternion(Degree(90), Vector3::UNIT_Y),  //-X
        Quaternion(Degree(90), Vector3::UNIT_X),  //+Y
        Quaternion(Degree(-90), Vector3::UNIT_X), //-Y
        Quaternion::IDENTITY,                     //+Z
        Quaternion(Degree(180), Vector3::UNIT_Y)  //-Z
    };

    return CubemapRotations[i];
}
void CompositorChain::preRenderTargetUpdate(const RenderTargetEvent& evt)
{
    /// Compile if state is dirty
    if(mDirty)
        _compile();

    // Do nothing if no compositors enabled
    if (!mAnyCompositorsEnabled)
    {
        return;
    }


    /// Update dependent render targets; this is done in the preRenderTarget 
    /// and not the preViewportUpdate for a reason: at this time, the
    /// target Rendertarget will not yet have been set as current. 
    /// ( RenderSystem::setViewport(...) ) if it would have been, the rendering
    /// order would be screwed up and problems would arise with copying rendertextures.
    Camera *cam = mViewport->getCamera();
    if (cam)
    {
        cam->getSceneManager()->_setActiveCompositorChain(this);
    }

    /// Iterate over compiled state
    for(auto& op : mCompiledState)
    {
        /// Skip if this is a target that should only be initialised initially
        if(op.onlyInitial && op.hasBeenRendered)
            continue;
        op.hasBeenRendered = true;

        auto vp = op.target->getViewport(0);
        if (!op.cameraOverride.empty())
        {
            SceneManager *sm = cam->getSceneManager();
            cam = sm->getCamera(op.cameraOverride);
            vp->setCamera(cam);
        }

        if (op.alignCameraToFace > -1)
        {
            cam->getParentSceneNode()->setOrientation(getCubemapRotation(op.alignCameraToFace));
        }

        /// Setup and render
        preTargetOperation(op, vp, cam);
        op.target->update();
        postTargetOperation(op, vp, cam);
    }
}
//-----------------------------------------------------------------------
void CompositorChain::postRenderTargetUpdate(const RenderTargetEvent& evt)
{
    Camera *cam = mViewport->getCamera();
    if (cam)
    {
        cam->getSceneManager()->_setActiveCompositorChain(0);
    }
}
//-----------------------------------------------------------------------
void CompositorChain::preViewportUpdate(const RenderTargetViewportEvent& evt)
{
    // Only set up if there is at least one compositor enabled, and it's this viewport
    if(evt.source != mViewport || !mAnyCompositorsEnabled)
        return;

    // set original scene details from viewport
    CompositionPass* pass = mOriginalScene->getTechnique()->getOutputTargetPass()->getPasses()[0];
    CompositionTargetPass* passParent = pass->getParent();
    if (pass->getClearBuffers() != mOldClearEveryFrameBuffers ||
        pass->getClearDepth() != mViewport->getDepthClear() ||
        passParent->getVisibilityMask() != mViewport->getVisibilityMask() ||
        passParent->getMaterialScheme() != mViewport->getMaterialScheme() ||
        passParent->getShadowsEnabled() != mViewport->getShadowsEnabled())
    {
        // recompile if viewport settings are different
        pass->setClearBuffers(mOldClearEveryFrameBuffers);
        pass->setClearDepth(mViewport->getDepthClear());
        passParent->setVisibilityMask(mViewport->getVisibilityMask());
        passParent->setMaterialScheme(mViewport->getMaterialScheme());
        passParent->setShadowsEnabled(mViewport->getShadowsEnabled());
        _compile();
    }

    Camera *cam = mViewport->getCamera();
    if (cam)
    {
        /// Prepare for output operation
        preTargetOperation(mOutputOperation, mViewport, cam);
    }
}
//-----------------------------------------------------------------------
void CompositorChain::preTargetOperation(CompositorInstance::TargetOperation &op, Viewport *vp, Camera *cam)
{
    if (cam)
    {
        SceneManager *sm = cam->getSceneManager();
        /// Set up render target listener
        mOurListener.setOperation(&op, sm, sm->getDestinationRenderSystem());
        mOurListener.notifyViewport(vp);
        /// Register it
        sm->addRenderQueueListener(&mOurListener);
        /// Set whether we find visibles
        mOldFindVisibleObjects = sm->getFindVisibleObjects();
        sm->setFindVisibleObjects(op.findVisibleObjects);
        /// Set LOD bias level
        mOldLodBias = cam->getLodBias();
        cam->setLodBias(cam->getLodBias() * op.lodBias);
    }

    // Set the visibility mask
    mOldVisibilityMask = vp->getVisibilityMask();
    vp->setVisibilityMask(op.visibilityMask);
    /// Set material scheme 
    mOldMaterialScheme = vp->getMaterialScheme();
    vp->setMaterialScheme(op.materialScheme);
    /// Set shadows enabled
    mOldShadowsEnabled = vp->getShadowsEnabled();
    vp->setShadowsEnabled(op.shadowsEnabled);
    /// XXX TODO
    //vp->setClearEveryFrame( true );
    //vp->setOverlaysEnabled( false );
    //vp->setBackgroundColour( op.clearColour );
}
//-----------------------------------------------------------------------
void CompositorChain::postTargetOperation(CompositorInstance::TargetOperation &op, Viewport *vp, Camera *cam)
{
    if (cam)
    {
        SceneManager *sm = cam->getSceneManager();
        /// Unregister our listener
        sm->removeRenderQueueListener(&mOurListener);
        /// Restore default scene and camera settings
        sm->setFindVisibleObjects(mOldFindVisibleObjects);
        cam->setLodBias(mOldLodBias);
    }

    vp->setVisibilityMask(mOldVisibilityMask);
    vp->setMaterialScheme(mOldMaterialScheme);
    vp->setShadowsEnabled(mOldShadowsEnabled);
}
//-----------------------------------------------------------------------
void CompositorChain::postViewportUpdate(const RenderTargetViewportEvent& evt)
{
    // Only tidy up if there is at least one compositor enabled, and it's this viewport
    if(evt.source != mViewport || !mAnyCompositorsEnabled)
        return;

    Camera *cam = mViewport->getCamera();
    postTargetOperation(mOutputOperation, mViewport, cam);
}
//-----------------------------------------------------------------------
void CompositorChain::viewportCameraChanged(Viewport* viewport)
{
    Camera* camera = viewport->getCamera();
    for (auto *i : mInstances)
    {
        i->notifyCameraChanged(camera);
    }
}
//-----------------------------------------------------------------------
void CompositorChain::viewportDimensionsChanged(Viewport* viewport)
{
    for (auto *i : mInstances)
    {
        i->notifyResized();
    }
}
//-----------------------------------------------------------------------
void CompositorChain::viewportDestroyed(Viewport* viewport)
{
    // this chain is now orphaned. tell compositor manager to delete it.
    CompositorManager::getSingleton().removeCompositorChain(viewport);
}
//-----------------------------------------------------------------------
void CompositorChain::clearCompiledState()
{
    for (auto *r : mRenderSystemOperations)
    {
        OGRE_DELETE r;
    }
    mRenderSystemOperations.clear();

    /// Clear compiled state
    mCompiledState.clear();
    mOutputOperation = CompositorInstance::TargetOperation(0);

}
//-----------------------------------------------------------------------
void CompositorChain::_compile()
{
    // remove original scene if it has the wrong material scheme
    if( mOriginalSceneScheme != mViewport->getMaterialScheme() )
    {
        destroyOriginalScene();
        createOriginalScene();
    }

    clearCompiledState();

    bool compositorsEnabled = false;

    // force default scheme so materials for compositor quads will determined correctly
    MaterialManager& matMgr = MaterialManager::getSingleton();
    String prevMaterialScheme = matMgr.getActiveScheme();
    matMgr.setActiveScheme(Root::getSingleton().getRenderSystem()->_getDefaultViewportMaterialScheme());
    
    /// Set previous CompositorInstance for each compositor in the list
    CompositorInstance *lastComposition = mOriginalScene;
    mOriginalScene->mPreviousInstance = 0;
    for(auto *i : mInstances)
    {
        if(i->getEnabled())
        {
            compositorsEnabled = true;
            i->mPreviousInstance = lastComposition;
            lastComposition = i;
        }
    }
    

    /// Compile misc targets
    lastComposition->_compileTargetOperations(mCompiledState);
    
    /// Final target viewport (0)
    mOutputOperation.renderSystemOperations.clear();
    lastComposition->_compileOutputOperation(mOutputOperation);

    // Deal with viewport settings
    if (compositorsEnabled != mAnyCompositorsEnabled)
    {
        mAnyCompositorsEnabled = compositorsEnabled;
        if (mAnyCompositorsEnabled)
        {
            // Save old viewport clearing options
            mOldClearEveryFrameBuffers = mViewport->getClearBuffers();
            // Don't clear anything every frame since we have our own clear ops
            mViewport->setClearEveryFrame(false);
        }
        else
        {
            // Reset clearing options
            mViewport->setClearEveryFrame(mOldClearEveryFrameBuffers > 0, 
                mOldClearEveryFrameBuffers);
        }
    }

    // restore material scheme
    matMgr.setActiveScheme(prevMaterialScheme);

    
    mDirty = false;
}
//-----------------------------------------------------------------------
void CompositorChain::_markDirty()
{
    mDirty = true;
}
//-----------------------------------------------------------------------
Viewport *CompositorChain::getViewport()
{
    return mViewport;
}
//---------------------------------------------------------------------
void CompositorChain::_notifyViewport(Viewport* vp)
{
    if (vp != mViewport)
    {
        if (mViewport != NULL) 
            mViewport->removeListener(this);

        if (vp != NULL) 
            vp->addListener(this);
        
        if (!vp || !mViewport || vp->getTarget() != mViewport->getTarget())
        {
            if(mViewport)
                mViewport->getTarget()->removeListener(this);

            if(vp)
                vp->getTarget()->addListener(this);
        }
        mOurListener.notifyViewport(vp);
        mViewport = vp;
    }   
}
//-----------------------------------------------------------------------
void CompositorChain::RQListener::renderQueueStarted(uint8 id, 
    const String& cameraName, bool& skipThisQueue)
{
    // Skip when not matching viewport
    // shadows update is nested within main viewport update
    if (mSceneManager->getCurrentViewport() != mViewport)
        return;

    flushUpTo(id);
    /// If no one wants to render this queue, skip it
    /// Don't skip the OVERLAY queue because that's handled separately
    if(!mOperation->renderQueues.test(id) && id!=RENDER_QUEUE_OVERLAY)
    {
        skipThisQueue = true;
    }
}
//-----------------------------------------------------------------------
void CompositorChain::RQListener::setOperation(CompositorInstance::TargetOperation *op,SceneManager *sm,RenderSystem *rs)
{
    mOperation = op;
    mSceneManager = sm;
    mRenderSystem = rs;
    currentOp = op->renderSystemOperations.begin();
    lastOp = op->renderSystemOperations.end();
}
//-----------------------------------------------------------------------
void CompositorChain::RQListener::flushUpTo(uint8 id)
{
    /// Process all RenderSystemOperations up to and including render queue id.
    /// Including, because the operations for RenderQueueGroup x should be executed
    /// at the beginning of the RenderQueueGroup render for x.
    while(currentOp != lastOp && currentOp->first <= id)
    {
        currentOp->second->execute(mSceneManager, mRenderSystem);
        ++currentOp;
    }
}
//-----------------------------------------------------------------------
CompositorInstance* CompositorChain::getPreviousInstance(CompositorInstance* curr, bool activeOnly)
{
    bool found = false;
    for(Instances::reverse_iterator i=mInstances.rbegin(); i!=mInstances.rend(); ++i)
    {
        if (found)
        {
            if ((*i)->getEnabled() || !activeOnly)
                return *i;
        }
        else if(*i == curr)
        {
            found = true;
        }
    }

    return 0;
}
//---------------------------------------------------------------------
CompositorInstance* CompositorChain::getNextInstance(CompositorInstance* curr, bool activeOnly)
{
    bool found = false;
    for(auto *i : mInstances)
    {
        if (found)
        {
            if (i->getEnabled() || !activeOnly)
                return i;
        }
        else if(i == curr)
        {
            found = true;
        }
    }

    return 0;
}
//---------------------------------------------------------------------
}

