/******************************************************************************
Copyright (c) W.J. van der Laan

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software  and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to use, 
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so, subject 
to the following conditions:

The above copyright notice and this permission notice shall be included in all copies 
or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION 
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,OUT OF OR IN CONNECTION WITH THE 
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/

#include "DeferredShading.h"

#include "OgreConfigFile.h"
#include "OgreStringConverter.h"
#include "OgreException.h"

#include "OgreRoot.h"
#include "OgreRenderSystem.h"

#include "OgreEntity.h"
#include "OgreSubEntity.h"
#include "OgreRoot.h"

#include "OgreCompositor.h"
#include "OgreCompositorManager.h"
#include "OgreCompositorChain.h"
#include "OgreCompositorInstance.h"

#include "OgreLogManager.h"

#include "DeferredLightCP.h"
#include "SSAOLogic.h"
#include "GBufferSchemeHandler.h"
#include "NullSchemeHandler.h"

using namespace Ogre;

const Ogre::uint8 DeferredShadingSystem::PRE_GBUFFER_RENDER_QUEUE = Ogre::RENDER_QUEUE_1;
const Ogre::uint8 DeferredShadingSystem::POST_GBUFFER_RENDER_QUEUE = Ogre::RENDER_QUEUE_8;

DeferredShadingSystem::DeferredShadingSystem(
		Viewport *vp, SceneManager *sm,  Camera *cam
	):
	mSceneMgr(sm), mViewport(vp), mCamera(cam)
{
	sm->setShadowTechnique(SHADOWTYPE_TEXTURE_ADDITIVE);
	sm->setShadowTextureCasterMaterial("DeferredShading/Shadows/Caster");
	mSceneMgr->setShadowFarDistance(150);
	mSceneMgr->setShadowTextureSize(512);
	mSceneMgr->setShadowTextureCount(1);
	mSceneMgr->setShadowTexturePixelFormat(PF_FLOAT16_R);
	mSceneMgr->setShadowDirectionalLightExtrusionDistance(75);
}

void DeferredShadingSystem::initialize()
{
	for(int i=0; i<DSM_COUNT; ++i)
		mInstance[i]=0;

	createResources();
	
	mActive = false;
	
	mSSAO = false;
	mCurrentMode = DSM_SHOWLIT;
	setActive(true);
}

DeferredShadingSystem::~DeferredShadingSystem()
{
	CompositorChain *chain = CompositorManager::getSingleton().getCompositorChain(mViewport);
	for(int i=0; i<DSM_COUNT; ++i)
		chain->_removeInstance(mInstance[i]);
}

void DeferredShadingSystem::setMode(DSMode mode)
{
	assert( 0 <= mode && mode < DSM_COUNT);

	// prevent duplicate setups
	if (mCurrentMode == mode && mInstance[mode]->getEnabled()==mActive)
		return;

	for(int i=0; i<DSM_COUNT; ++i)
	{
		if(i == mode)
		{
			mInstance[i]->setEnabled(mActive);
		}
		else
		{
			mInstance[i]->setEnabled(false);
		}
	}

	mCurrentMode = mode;

	mSSAOInstance->setEnabled(mActive && mSSAO && mCurrentMode == DSM_SHOWLIT);
}

void DeferredShadingSystem::setSSAO(bool ssao)
{
	if (ssao != mSSAO) 
	{
		mSSAO = ssao;
		if (mActive && mCurrentMode == DSM_SHOWLIT)
		{
			mSSAOInstance->setEnabled(ssao);
		}
	}
}
	
bool DeferredShadingSystem::getSSAO() const
{
	return mSSAO;
}
void DeferredShadingSystem::setActive(bool active)
{
	if (mActive != active)
	{
		mActive = active;
		mGBufferInstance->setEnabled(active);

		// mCurrentMode could have changed with a prior call to setMode, so iterate all
		setMode(mCurrentMode);
	}
}

DeferredShadingSystem::DSMode DeferredShadingSystem::getMode(void) const
{
	return mCurrentMode;
}

void DeferredShadingSystem::createResources(void)
{
	CompositorManager &compMan = CompositorManager::getSingleton();

	//Hook up the compositor logic and scheme handlers.
	//This can theoretically happen in a loaded plugin, but in this case the demo contains the code.
	MaterialManager::getSingleton().addListener(new GBufferSchemeHandler, "GBuffer");
	MaterialManager::getSingleton().addListener(new NullSchemeHandler, "NoGBuffer");

	compMan.registerCompositorLogic("SSAOLogic", new SSAOLogic);
	compMan.registerCustomCompositionPass("DeferredLight", new DeferredLightCompositionPass);
	
	// Create the main GBuffer compositor
	mGBufferInstance = compMan.addCompositor(mViewport, "DeferredShading/GBuffer");
	
	// Create filters
	mInstance[DSM_SHOWLIT] = compMan.addCompositor(mViewport, "DeferredShading/ShowLit");
	mInstance[DSM_SHOWNORMALS] = compMan.addCompositor(mViewport, "DeferredShading/ShowNormals");
	mInstance[DSM_SHOWDSP] = compMan.addCompositor(mViewport, "DeferredShading/ShowDepthSpecular");
	mInstance[DSM_SHOWCOLOUR] = compMan.addCompositor(mViewport, "DeferredShading/ShowColour");

	mInstance[DSM_SHOWCOLOUR] = compMan.addCompositor(mViewport, "DeferredShading/ShowColour");
	mSSAOInstance =  compMan.addCompositor(mViewport, "DeferredShading/SSAO");
}


void DeferredShadingSystem::logCurrentMode(void)
{
	if (mActive==false)
	{
		LogManager::getSingleton().logMessage("No Compositor Enabled!");
		return;
	}

	CompositorInstance* ci = mInstance[mCurrentMode];
	assert(ci->getEnabled()==true);

	LogManager::getSingleton().logMessage("Current Mode: ");
	LogManager::getSingleton().logMessage(ci->getCompositor()->getName());
}
