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

#include "OgreHardwarePixelBuffer.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreMaterialManager.h"

#include "OgreEntity.h"
#include "OgreSubEntity.h"
#include "OgreRoot.h"

#include "OgreCompositor.h"
#include "OgreCompositorManager.h"
#include "OgreCompositorChain.h"
#include "OgreCompositorInstance.h"
#include "OgreCompositionTechnique.h"
#include "OgreCompositionPass.h"
#include "OgreCompositionTargetPass.h"

#include "MLight.h"
#include "LightMaterialGenerator.h"

#include "AmbientLight.h"

#include "OgreHighLevelGpuProgram.h"
#include "OgreHighLevelGpuProgramManager.h"

#include "OgreLogManager.h"

using namespace Ogre;

DeferredShadingSystem::DeferredShadingSystem(
		Viewport *vp, SceneManager *sm,  Camera *cam
	):
	mViewport(vp), mSceneMgr(sm), mCamera(cam),
		mLightMaterialGenerator(0)
{
	for(int i=0; i<DSM_COUNT; ++i)
		mInstance[i]=0;

	createResources();
	createAmbientLight();

	mActive = true;
	mCurrentMode = DSM_COUNT;
	setMode(DSM_SHOWLIT);

	mLightMaterialsDirty=true;

}

DeferredShadingSystem::~DeferredShadingSystem()
{
	// Delete mini lights
	for(set<MLight*>::type::iterator i=mLights.begin(); i!=mLights.end(); ++i)
	{
		delete (*i);
	}
	// Delete the ambient light
	delete mAmbientLight;

	if (mCurrentMode==DSM_SHOWLIT && mInstance[mCurrentMode]->getEnabled())
	{
		RenderTarget* renderTarget = mInstance[mCurrentMode]->getRenderTarget("mrt_output");
		assert(renderTarget);

		LogManager::getSingleton().logMessage("Removing Listener from:");
		LogManager::getSingleton().logMessage(renderTarget->getName());

		renderTarget->removeListener(this);
	}

	CompositorChain *chain = CompositorManager::getSingleton().getCompositorChain(mViewport);
	for(int i=0; i<DSM_COUNT; ++i)
		chain->_removeInstance(mInstance[i]);

	delete mLightMaterialGenerator;
}

void DeferredShadingSystem::setMode(DSMode mode)
{
	assert( 0 <= mode && mode < DSM_COUNT);

	// prevent duplicate setups
	if (mCurrentMode == mode && mInstance[mode]->getEnabled()==mActive)
		return;

	// if the mode is getting disabled 
	// -> we need to remove self as listener only if it was enabled to begin with
	// This should happen before the setEnabled(false) is called
	if (  mCurrentMode == DSM_SHOWLIT
	   && mInstance[mCurrentMode]->getEnabled())
	{
		RenderTarget* renderTarget = mInstance[mCurrentMode]->getRenderTarget("mrt_output");
		assert(renderTarget);

		LogManager::getSingleton().logMessage("Removing Listener from:");
		LogManager::getSingleton().logMessage(renderTarget->getName());

		// remove the listener prior to the texture getting possibly reclaimed
		renderTarget->removeListener(this);
	}

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

	// if some mode got enabled,
	// set self as listener so that the light materials can be set up if dirty. This should happen after the setEnabled(true)
	// is called
	if (  mCurrentMode == DSM_SHOWLIT
	   && mInstance[mCurrentMode]->getEnabled())
	{
		RenderTarget* renderTarget = mInstance[mCurrentMode]->getRenderTarget("mrt_output");
		assert(renderTarget);

		LogManager::getSingleton().logMessage("Adding Listener to:");
		LogManager::getSingleton().logMessage(renderTarget->getName());
		renderTarget->addListener(this);

		// Additionally, mark the light materials as always dirty
		mLightMaterialsDirty = true;
		mDirtyLightList.clear();
		mDirtyLightList = mLights;

		// set up the ambient light here
		setUpAmbientLightMaterial();
	}
}

void DeferredShadingSystem::setActive(bool active)
{
	if (mActive != active)
	{
		mActive = active;
		// mCurrentMode could have changed with a prior call to setMode, so iterate all
		setMode(mCurrentMode);
	}
}

DeferredShadingSystem::DSMode DeferredShadingSystem::getMode(void) const
{
	return mCurrentMode;
}

MLight *DeferredShadingSystem::createMLight()
{
	MLight *rv = new MLight(mLightMaterialGenerator);
	mLights.insert(rv);

	if (mCurrentMode==DSM_SHOWLIT)
	{
		mDirtyLightList.insert(rv);
		mLightMaterialsDirty = true;
	}

	return rv;
}

void DeferredShadingSystem::destroyMLight(MLight *m)
{
	mLights.erase(m);
	delete m;
}

void DeferredShadingSystem::createResources(void)
{
	CompositorManager &compMan = CompositorManager::getSingleton();

	// Create lights material generator
	if(Root::getSingleton().getRenderSystem()->getName()=="OpenGL Rendering Subsystem")
		mLightMaterialGenerator = new LightMaterialGenerator("glsl");
	else
		mLightMaterialGenerator = new LightMaterialGenerator("hlsl");

	// Create filters
	mInstance[DSM_SHOWLIT] = compMan.addCompositor(mViewport, "DeferredShading/ShowLit");
	mInstance[DSM_SHOWNORMALS] = compMan.addCompositor(mViewport, "DeferredShading/ShowNormals");
	mInstance[DSM_SHOWDSP] = compMan.addCompositor(mViewport, "DeferredShading/ShowDepthSpecular");
	mInstance[DSM_SHOWCOLOUR] = compMan.addCompositor(mViewport, "DeferredShading/ShowColour");
}

void DeferredShadingSystem::setupLightMaterials(void)
{
	assert( mLightMaterialsDirty 
		&& mCurrentMode == DSM_SHOWLIT
		&& mInstance[mCurrentMode]->getEnabled()==true);

	CompositorInstance* ci = mInstance[mCurrentMode];

	String mrt0 = ci->getTextureInstanceName("mrt_output", 0);
	String mrt1 = ci->getTextureInstanceName("mrt_output", 1);

	for(LightList::iterator it = mDirtyLightList.begin(); it != mDirtyLightList.end(); ++it)
	{
		MLight* light = *it;
		setupMaterial(light->getMaterial(), mrt0, mrt1);
	}

	mLightMaterialsDirty = false;
}

void DeferredShadingSystem::setupMaterial(const MaterialPtr &mat
										  , const String& texName0
										  , const String& texName1)
{
	for(unsigned short i=0; i<mat->getNumTechniques(); ++i)
	{
		Pass *pass = mat->getTechnique(i)->getPass(0);
		pass->getTextureUnitState(0)->setTextureName(texName0);
		pass->getTextureUnitState(1)->setTextureName(texName1);
	}
}

void DeferredShadingSystem::createAmbientLight(void)
{
	mAmbientLight = new AmbientLight;
	mSceneMgr->getRootSceneNode()->attachObject(mAmbientLight);
}

void DeferredShadingSystem::setUpAmbientLightMaterial(void)
{
	assert(mAmbientLight 
		&& mCurrentMode==DSM_SHOWLIT 
		&& mInstance[mCurrentMode]->getEnabled()==true);

	String mrt0 = mInstance[mCurrentMode]->getTextureInstanceName("mrt_output", 0);
	String mrt1 = mInstance[mCurrentMode]->getTextureInstanceName("mrt_output", 1);
	setupMaterial(mAmbientLight->getMaterial(), mrt0, mrt1);
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
		
	if (mCurrentMode==DSM_SHOWLIT)
	{			
		LogManager::getSingleton().logMessage("Current mrt outputs are:");
		LogManager::getSingleton().logMessage(ci->getTextureInstanceName("mrt_output", 0));
		LogManager::getSingleton().logMessage(ci->getTextureInstanceName("mrt_output", 1));
	}
}

void DeferredShadingSystem::preRenderTargetUpdate(const RenderTargetEvent& evt)
{
	if (mLightMaterialsDirty)
	{
		assert(mCurrentMode==DSM_SHOWLIT
			&& mInstance[mCurrentMode]->getEnabled()
			&& (evt.source == mInstance[mCurrentMode]->getRenderTarget("mrt_output"))
			);
		setupLightMaterials();
	}
}
