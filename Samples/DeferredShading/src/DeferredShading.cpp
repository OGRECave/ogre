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

#include "OgreHighLevelGpuProgram.h"
#include "OgreHighLevelGpuProgramManager.h"

using namespace Ogre;

/// XXX make this a .compositor script
void createPostFilters()
{
	/** Postfilter for rendering to fat render target. Excludes skies, backgrounds and other unwanted
		objects.
	*/
	CompositorPtr comp7 = CompositorManager::getSingleton().create(
				"DeferredShading/Fat", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
			);
	{
		CompositionTechnique *t = comp7->createTechnique();
		{
			CompositionTargetPass *tp = t->getOutputTargetPass();
			tp->setInputMode(CompositionTargetPass::IM_NONE);
			tp->setVisibilityMask(DeferredShadingSystem::SceneVisibilityMask);
			/// Clear
			{	CompositionPass *pass = tp->createPass();
				pass->setType(CompositionPass::PT_CLEAR);
				pass->setClearColour(ColourValue(0,0,0,0));
			}
			/// Render geometry
			{	CompositionPass *pass = tp->createPass();
				pass->setType(CompositionPass::PT_RENDERSCENE);
				pass->setFirstRenderQueue(RENDER_QUEUE_1);
				pass->setLastRenderQueue(RENDER_QUEUE_9);
			}
		}
	}
	/** Postfilter doing full deferred shading with two lights in one pass
	*/
	CompositorPtr comp = CompositorManager::getSingleton().create(
				"DeferredShading/Single", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
			);
	{
		CompositionTechnique *t = comp->createTechnique();
		{
			CompositionTargetPass *tp = t->getOutputTargetPass();
			tp->setInputMode(CompositionTargetPass::IM_NONE);
			tp->setVisibilityMask(DeferredShadingSystem::PostVisibilityMask);
			/// Render skies
			{	CompositionPass *pass = tp->createPass();
				pass->setType(CompositionPass::PT_RENDERSCENE);
				pass->setFirstRenderQueue(RENDER_QUEUE_SKIES_EARLY);
				pass->setLastRenderQueue(RENDER_QUEUE_SKIES_EARLY);
			}
			/// Render ambient pass
			{	CompositionPass *pass = tp->createPass();
				pass->setType(CompositionPass::PT_RENDERQUAD);
				pass->setMaterialName("DeferredShading/Post/Single");
				pass->setIdentifier(1);
			}
			/// Render overlayed geometry
			{	CompositionPass *pass = tp->createPass();
				pass->setType(CompositionPass::PT_RENDERSCENE);
				pass->setFirstRenderQueue(RENDER_QUEUE_1);
				pass->setLastRenderQueue(RENDER_QUEUE_9);
			}
		}
	}
	/** Postfilter doing full deferred shading with an ambient pass and multiple light passes
	*/
	CompositorPtr comp2 = CompositorManager::getSingleton().create(
				"DeferredShading/Multi", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
			);
	{
		CompositionTechnique *t = comp2->createTechnique();
		{
			CompositionTargetPass *tp = t->getOutputTargetPass();
			tp->setInputMode(CompositionTargetPass::IM_NONE);
			tp->setVisibilityMask(DeferredShadingSystem::PostVisibilityMask);
			/// Render skies
			{	CompositionPass *pass = tp->createPass();
				pass->setType(CompositionPass::PT_RENDERSCENE);
				pass->setFirstRenderQueue(RENDER_QUEUE_SKIES_EARLY);
				pass->setLastRenderQueue(RENDER_QUEUE_SKIES_EARLY);
			}
			/// Render ambient pass
			{	CompositionPass *pass = tp->createPass();
				pass->setType(CompositionPass::PT_RENDERQUAD);
				pass->setMaterialName("DeferredShading/Post/Multi");
				pass->setIdentifier(1);
			}
			/// Render overlayed geometry
			{
				CompositionPass *pass = tp->createPass();
				pass->setType(CompositionPass::PT_RENDERSCENE);
				pass->setFirstRenderQueue(RENDER_QUEUE_1);
				pass->setLastRenderQueue(RENDER_QUEUE_9);
			}
		}
	}	
	/** Postfilter that shows the normal channel
	*/
	CompositorPtr comp3 = CompositorManager::getSingleton().create(
				"DeferredShading/ShowNormal", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
			);
	{
		CompositionTechnique *t = comp3->createTechnique();
		{
			CompositionTargetPass *tp = t->getOutputTargetPass();
			tp->setInputMode(CompositionTargetPass::IM_NONE);
			{	CompositionPass *pass = tp->createPass();
				pass->setType(CompositionPass::PT_RENDERQUAD);
				pass->setMaterialName("DeferredShading/Post/ShowNormal");
				pass->setIdentifier(1);
			}
		}
	}	
	/** Postfilter that shows the depth and specular channel
	*/
	CompositorPtr comp4 = CompositorManager::getSingleton().create(
				"DeferredShading/ShowDepthSpecular", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
			);
	{
		CompositionTechnique *t = comp4->createTechnique();
		{
			CompositionTargetPass *tp = t->getOutputTargetPass();
			tp->setInputMode(CompositionTargetPass::IM_NONE);
			{	CompositionPass *pass = tp->createPass();
				pass->setType(CompositionPass::PT_RENDERQUAD);
				pass->setMaterialName("DeferredShading/Post/ShowDS");
				pass->setIdentifier(1);
			}
		}
	}	
	/** Postfilter that shows the depth and specular channel
	*/
	CompositorPtr comp5 = CompositorManager::getSingleton().create(
				"DeferredShading/ShowColour", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
			);
	{
		CompositionTechnique *t = comp5->createTechnique();
		{
			CompositionTargetPass *tp = t->getOutputTargetPass();
			tp->setInputMode(CompositionTargetPass::IM_NONE);
			{	CompositionPass *pass = tp->createPass();
				pass->setType(CompositionPass::PT_RENDERQUAD);
				pass->setMaterialName("DeferredShading/Post/ShowColour");
				pass->setIdentifier(1);
			}
		}
	}
}


DeferredShadingSystem::DeferredShadingSystem(
		Viewport *vp, SceneManager *sm,  Camera *cam
	):
	mSceneMgr(sm), mViewport(vp), mCamera(cam),
		mLightMaterialGenerator(0)
{
	for(int i=0; i<DSM_COUNT; ++i)
		mInstance[i]=0;

	mActive = true;
	mCurrentMode = DSM_MULTIPASS;

	rttTex = 0;

	createPostFilters();

	createResources();
	// Hide post geometry
	mSceneMgr->setVisibilityMask(mSceneMgr->getVisibilityMask() & ~PostVisibilityMask);
	// Default to normal deferred shading mode
	setMode(mCurrentMode);
	setActive(true);
}

DeferredShadingSystem::~DeferredShadingSystem()
{
	// Delete mini lights
	for(std::set<MLight*>::iterator i=mLights.begin(); i!=mLights.end(); ++i)
	{
		delete (*i);
	}

	Ogre::CompositorChain *chain = Ogre::CompositorManager::getSingleton().getCompositorChain(mViewport);
	for(int i=0; i<DSM_COUNT; ++i)
		chain->_removeInstance(mInstance[i]);

	delete mLightMaterialGenerator;
}
void DeferredShadingSystem::setMode(DSMode mode)
{
	for(int i=0; i<DSM_COUNT; ++i)
	{
		if(i == mode)
			mInstance[i]->setEnabled(mActive);
		else
			mInstance[i]->setEnabled(false);
	}
	mCurrentMode = mode;
}
void DeferredShadingSystem::setActive(bool active)
{
	mActive = active;
	setMode(mCurrentMode);
}
void DeferredShadingSystem::createResources(void)
{
	Ogre::CompositorManager &compMan = Ogre::CompositorManager::getSingleton();
	// Create 'fat' render target
	unsigned int width = mViewport->getActualWidth();
	unsigned int height = mViewport->getActualHeight();
	PixelFormat format = PF_FLOAT16_RGBA;
	//PixelFormat format = PF_SHORT_RGBA;

	mTexture0 = TextureManager::getSingleton().createManual("RttTex0", 
		ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_2D, 
		width, height, 0, format, TU_RENDERTARGET );
	mTexture1 = TextureManager::getSingleton().createManual("RttTex1", 
		ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_2D, 
		width, height, 0, format, TU_RENDERTARGET );
	//assert(mTexture0->getFormat() == format);
	//assert(mTexture1->getFormat() == format);
	rttTex = Ogre::Root::getSingleton().getRenderSystem()->createMultiRenderTarget("MRT");
    RenderTexture* rt0 = mTexture0->getBuffer()->getRenderTarget();
    RenderTexture* rt1 = mTexture1->getBuffer()->getRenderTarget();
    rt0->setAutoUpdated(false);
    rt1->setAutoUpdated(false);
	rttTex->bindSurface(0, rt0);
	rttTex->bindSurface(1, rt1);
	rttTex->setAutoUpdated( false );

	// Setup viewport on 'fat' render target
	Viewport* v = rttTex->addViewport( mCamera );
	v->setClearEveryFrame( false );
	v->setOverlaysEnabled( false );
    // Should disable skies for MRT due it's not designed for that, and
    // will causing NVIDIA refusing write anything to other render targets
    // for some reason.
    v->setSkiesEnabled(false);
	v->setBackgroundColour( ColourValue( 0, 0, 0, 0) );
	compMan.addCompositor(v, "DeferredShading/Fat");

	// Create lights material generator
	setupMaterial(MaterialManager::getSingleton().getByName("DeferredShading/LightMaterialQuad"));
	setupMaterial(MaterialManager::getSingleton().getByName("DeferredShading/LightMaterial"));
	if(Root::getSingleton().getRenderSystem()->getName()=="OpenGL Rendering Subsystem")
		mLightMaterialGenerator = new LightMaterialGenerator("glsl");
	else
		mLightMaterialGenerator = new LightMaterialGenerator("hlsl");

	// Create filters
	mInstance[DSM_SINGLEPASS] = compMan.addCompositor(mViewport, "DeferredShading/Single");
	mInstance[DSM_MULTIPASS] = compMan.addCompositor(mViewport, "DeferredShading/Multi");
	mInstance[DSM_SHOWNORMALS] = compMan.addCompositor(mViewport, "DeferredShading/ShowNormal");
	mInstance[DSM_SHOWDSP] = compMan.addCompositor(mViewport, "DeferredShading/ShowDepthSpecular");
	mInstance[DSM_SHOWCOLOUR] = compMan.addCompositor(mViewport, "DeferredShading/ShowColour");

	// Add material setup callback
	for(int i=0; i<DSM_COUNT; ++i)
		mInstance[i]->addListener(this);
}
void DeferredShadingSystem::setupMaterial(const MaterialPtr &mat)
{
	for(unsigned short i=0; i<mat->getNumTechniques(); ++i)
	{
		Pass *pass = mat->getTechnique(i)->getPass(0);
		pass->getTextureUnitState(0)->setTextureName(mTexture0->getName());
		pass->getTextureUnitState(1)->setTextureName(mTexture1->getName());
	}
}

MLight *DeferredShadingSystem::createMLight()
{
	MLight *rv = new MLight(mLightMaterialGenerator);
	rv->setVisibilityFlags(PostVisibilityMask);
	mLights.insert(rv);

	return rv;
}
void DeferredShadingSystem::destroyMLight(MLight *m)
{
	mLights.erase(m);
	delete m;
}

void DeferredShadingSystem::update()
{
	rttTex->update();
}

void DeferredShadingSystem::notifyMaterialSetup(uint32 pass_id, MaterialPtr &mat)
{
	/// Local pass identifier 1 is the render quad pass
	if(pass_id == 1)
	{
		setupMaterial(mat);
	}
}
