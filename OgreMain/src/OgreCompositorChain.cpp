/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#include "OgreCompositorChain.h"
#include "OgreCompositionTechnique.h"
#include "OgreCompositorInstance.h"
#include "OgreCompositionTargetPass.h"
#include "OgreCompositionPass.h"
#include "OgreViewport.h"
#include "OgreCamera.h"
#include "OgreRenderTarget.h"
#include "OgreLogManager.h"
#include "OgreCompositorManager.h"
#include "OgreSceneManager.h"
#include "OgreRenderQueueInvocation.h"
#include "OgreMaterialManager.h"

namespace Ogre {
CompositorChain::CompositorChain(Viewport *vp):
    mViewport(vp),
	mOriginalScene(0),
    mDirty(true),
	mAnyCompositorsEnabled(false)
{
	mOldClearEveryFrameBuffers = mViewport->getClearBuffers();
    assert(mViewport);
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
		removeAllCompositors();
		/// Destroy "original scene" compositor instance
		if (mOriginalScene)
		{
			mViewport->getTarget()->removeListener(this);
			OGRE_DELETE mOriginalScene;
			mOriginalScene = 0;
		}
		mViewport = 0;
	}
}
//-----------------------------------------------------------------------
CompositorInstance* CompositorChain::addCompositor(CompositorPtr filter, size_t addPosition, const String& scheme)
{
	// Init on demand
	if (!mOriginalScene)
	{
		mViewport->getTarget()->addListener(this);
		
		/// Create base "original scene" compositor
		CompositorPtr base = CompositorManager::getSingleton().load("Ogre/Scene",
			ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME);
		mOriginalScene = OGRE_NEW CompositorInstance(base->getSupportedTechnique(), this);
	}


	filter->touch();
    CompositionTechnique *tech = filter->getSupportedTechnique(scheme);
	if(!tech)
	{
		/// Warn user
		LogManager::getSingleton().logMessage(
			"CompositorChain: Compositor " + filter->getName() + " has no supported techniques.", LML_CRITICAL
			);
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
    Instances::iterator i, iend;
    iend = mInstances.end();
    for (i = mInstances.begin(); i != iend; ++i)
    {
		OGRE_DELETE *i;
    }
    mInstances.clear();
    
    mDirty = true;
}
//-----------------------------------------------------------------------
void CompositorChain::_removeInstance(CompositorInstance *i)
{
	mInstances.erase(std::find(mInstances.begin(), mInstances.end(), i));
	OGRE_DELETE i;
}
//-----------------------------------------------------------------------
void CompositorChain::_queuedOperation(CompositorInstance::RenderSystemOperation* op)
{
	mRenderSystemOperations.push_back(op);

}
//-----------------------------------------------------------------------
CompositorInstance *CompositorChain::getCompositor(size_t index)
{
    assert (index < mInstances.size() && "Index out of bounds.");
    return mInstances[index];
}
//-----------------------------------------------------------------------
CompositorInstance *CompositorChain::getCompositor(const String& name)
{
	for (Instances::iterator it = mInstances.begin(); it != mInstances.end(); ++it) 
	{
		if ((*it)->getCompositor()->getName() == name) 
		{
			return *it;
		}
	}
	OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Invalid compositor name",
		"CompositorChain::getCompositor");
}
//-----------------------------------------------------------------------
CompositorChain::InstanceIterator CompositorChain::getCompositors()
{
    return InstanceIterator(mInstances.begin(), mInstances.end());
}
//-----------------------------------------------------------------------
void CompositorChain::setCompositorEnabled(size_t position, bool state)
{
	CompositorInstance* inst = getCompositor(position);
	if (!state && inst->getEnabled())
	{
		// If we're disabling a 'middle' compositor in a chain, we have to be
		// careful about textures which might have been shared by non-adjacent
		// instances which have now become adjacent. 
		CompositorInstance* nextInstance = getNextInstance(inst, true);
		if (nextInstance)
		{
			CompositionTechnique::TargetPassIterator tpit = nextInstance->getTechnique()->getTargetPassIterator();
			while(tpit.hasMoreElements())
			{
				CompositionTargetPass* tp = tpit.getNext();
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
	if (!cam)
	{
		return;
	}

    /// Iterate over compiled state
    CompositorInstance::CompiledState::iterator i;
    for(i=mCompiledState.begin(); i!=mCompiledState.end(); ++i)
    {
		/// Skip if this is a target that should only be initialised initially
		if(i->onlyInitial && i->hasBeenRendered)
			continue;
		i->hasBeenRendered = true;
		/// Setup and render
        preTargetOperation(*i, i->target->getViewport(0), cam);
        i->target->update();
        postTargetOperation(*i, i->target->getViewport(0), cam);
    }
}
//-----------------------------------------------------------------------
void CompositorChain::preViewportUpdate(const RenderTargetViewportEvent& evt)
{
	// Only set up if there is at least one compositor enabled, and it's this viewport
    if(evt.source != mViewport || !mAnyCompositorsEnabled)
        return;

	// set original scene details from viewport
	CompositionPass* pass = mOriginalScene->getTechnique()->getOutputTargetPass()->getPass(0);
	CompositionTargetPass* passParent = pass->getParent();
	if (pass->getClearBuffers() != mViewport->getClearBuffers() ||
		pass->getClearColour() != mViewport->getBackgroundColour() ||
		passParent->getVisibilityMask() != mViewport->getVisibilityMask() ||
		passParent->getMaterialScheme() != mViewport->getMaterialScheme() ||
		passParent->getShadowsEnabled() != mViewport->getShadowsEnabled())
	{
		// recompile if viewport settings are different
		pass->setClearBuffers(mViewport->getClearBuffers());
		pass->setClearColour(mViewport->getBackgroundColour());
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
    SceneManager *sm = cam->getSceneManager();
	/// Set up render target listener
	mOurListener.setOperation(&op, sm, sm->getDestinationRenderSystem());
	mOurListener.notifyViewport(vp);
	/// Register it
	sm->addRenderQueueListener(&mOurListener);
	/// Set visiblity mask
	mOldVisibilityMask = sm->getVisibilityMask();
	sm->setVisibilityMask(op.visibilityMask);
	/// Set whether we find visibles
	mOldFindVisibleObjects = sm->getFindVisibleObjects();
	sm->setFindVisibleObjects(op.findVisibleObjects);
    /// Set LOD bias level
    mOldLodBias = cam->getLodBias();
    cam->setLodBias(cam->getLodBias() * op.lodBias);
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
    SceneManager *sm = cam->getSceneManager();
	/// Unregister our listener
	sm->removeRenderQueueListener(&mOurListener);
	/// Restore default scene and camera settings
	sm->setVisibilityMask(mOldVisibilityMask);
	sm->setFindVisibleObjects(mOldFindVisibleObjects);
    cam->setLodBias(mOldLodBias);
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
	if (cam)
	{
		postTargetOperation(mOutputOperation, mViewport, cam);
	}
}
//-----------------------------------------------------------------------
void CompositorChain::viewportRemoved(const RenderTargetViewportEvent& evt)
{
	// check this is the viewport we're attached to (multi-viewport targets)
	if (evt.source == mViewport) 
	{
		// this chain is now orphaned
		// can't delete it since held from outside, but release all resources being used
		destroyResources();
	}

}
//-----------------------------------------------------------------------
void CompositorChain::clearCompiledState()
{
	for (RenderSystemOperations::iterator i = mRenderSystemOperations.begin();
		i != mRenderSystemOperations.end(); ++i)
	{
		OGRE_DELETE *i;
	}
	mRenderSystemOperations.clear();

	/// Clear compiled state
	mCompiledState.clear();
	mOutputOperation = CompositorInstance::TargetOperation(0);

}
//-----------------------------------------------------------------------
void CompositorChain::_compile()
{
	clearCompiledState();

	bool compositorsEnabled = false;

	// force default scheme so materials for compositor quads will determined correctly
	MaterialManager& matMgr = MaterialManager::getSingleton();
	String prevMaterialScheme = matMgr.getActiveScheme();
	matMgr.setActiveScheme(MaterialManager::DEFAULT_SCHEME_NAME);
	
    /// Set previous CompositorInstance for each compositor in the list
    CompositorInstance *lastComposition = mOriginalScene;
	mOriginalScene->mPreviousInstance = 0;
	CompositionPass* pass = mOriginalScene->getTechnique()->getOutputTargetPass()->getPass(0);
	pass->setClearBuffers(mViewport->getClearBuffers());
	pass->setClearColour(mViewport->getBackgroundColour());
    for(Instances::iterator i=mInstances.begin(); i!=mInstances.end(); ++i)
    {
        if((*i)->getEnabled())
        {
			compositorsEnabled = true;
            (*i)->mPreviousInstance = lastComposition;
            lastComposition = (*i);
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
	mViewport = vp;
}
//-----------------------------------------------------------------------
void CompositorChain::RQListener::renderQueueStarted(uint8 id, 
	const String& invocation, bool& skipThisQueue)
{
	// Skip when not matching viewport
	// shadows update is nested within main viewport update
	if (mSceneManager->getCurrentViewport() != mViewport)
		return;

	flushUpTo(id);
	/// If noone wants to render this queue, skip it
	/// Don't skip the OVERLAY queue because that's handled seperately
	if(!mOperation->renderQueues.test(id) && id!=RENDER_QUEUE_OVERLAY)
	{
		skipThisQueue = true;
	}
}
//-----------------------------------------------------------------------
void CompositorChain::RQListener::renderQueueEnded(uint8 id, 
	const String& invocation, bool& repeatThisQueue)
{
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
	for(Instances::iterator i=mInstances.begin(); i!=mInstances.end(); ++i)
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
}

