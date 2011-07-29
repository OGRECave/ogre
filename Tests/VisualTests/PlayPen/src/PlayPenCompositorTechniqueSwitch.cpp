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
#include "PlayPenTests.h"

PlayPen_CompositorTechniqueSwitch::PlayPen_CompositorTechniqueSwitch()
{
	mInfo["Title"] = "PlayPen_CompositorTechniqueSwitch";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(15);
}
//----------------------------------------------------------------------------

void PlayPen_CompositorTechniqueSwitch::cleanupContent()
{
	CompositorManager::getSingleton().removeCompositorChain(mViewport);
}
//----------------------------------------------------------------------------

bool PlayPen_CompositorTechniqueSwitch::frameStarted(const FrameEvent& evt)
{
	mTimeUntilNextToggle -= evt.timeSinceLastFrame;

	if(mTimeUntilNextToggle <= 0.f)
	{
		++mCompositorIndex;
		mCompositorIndex = mCompositorIndex % mCompositorSchemeList.size();
		mCompositorToSwitch->setScheme(mCompositorSchemeList[mCompositorIndex]);
		mTimeUntilNextToggle = 0.1;		
	}

	return true;
}
//----------------------------------------------------------------------------

void PlayPen_CompositorTechniqueSwitch::setupContent()
{
	mTimeUntilNextToggle = 0.1f;// swap compositors every 10 frames
	CompositorManager& cmgr = CompositorManager::getSingleton();
	CompositorPtr compositor = cmgr.create("testtechswitch", 
	TRANSIENT_RESOURCE_GROUP);
	// technique 1 (Invert)
	CompositionTechnique* ctech1 = compositor->createTechnique();
	CompositionTechnique::TextureDefinition* tdef =	ctech1->createTextureDefinition("rt0");
	tdef->formatList.push_back(PF_A8B8G8R8);
	tdef->width = tdef->height = 0;
	tdef->pooled = true;
	
	CompositionTargetPass* tpass = ctech1->createTargetPass();
	tpass->setOutputName("rt0");
	tpass->setInputMode(CompositionTargetPass::IM_PREVIOUS);
	CompositionTargetPass* tout = ctech1->getOutputTargetPass();
	tout->setInputMode(CompositionTargetPass::IM_NONE);
	CompositionPass* pass = tout->createPass();
	pass->setType(CompositionPass::PT_RENDERQUAD);
	pass->setMaterialName("Ogre/Compositor/Invert");
	pass->setInput(0, "rt0");
	
	// technique 2 (Tiling)
	ctech1 = compositor->createTechnique();
	ctech1->setSchemeName("Tiling");
	tdef =	ctech1->createTextureDefinition("rt0");
	tdef->formatList.push_back(PF_A8B8G8R8);
	tdef->width = tdef->height = 0;
	tdef->pooled = true;
	
	tpass = ctech1->createTargetPass();
	tpass->setOutputName("rt0");
	tpass->setInputMode(CompositionTargetPass::IM_PREVIOUS);
	tout = ctech1->getOutputTargetPass();
	tout->setInputMode(CompositionTargetPass::IM_NONE);
	pass = tout->createPass();
	pass->setType(CompositionPass::PT_RENDERQUAD);
	pass->setMaterialName("Ogre/Compositor/Tiling");
	pass->setInput(0, "rt0");
	
	compositor->load();
	
	Entity* e = mSceneMgr->createEntity("1", "knot.mesh");
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(e);
	mSceneMgr->setSkyBox(true, "Examples/CloudyNoonSkyBox", 1000);
	
	// enable compositor (should pick first technique)
	Viewport* vp = mWindow->getViewport(0);
	
	mCompositorToSwitch = cmgr.addCompositor(vp, compositor->getName());
	mCompositorSchemeList.push_back("");
	mCompositorSchemeList.push_back("Tiling");
	
	cmgr.setCompositorEnabled(vp, compositor->getName(), true);
	
	mCamera->setPosition(0, 0, -300);
	mCamera->lookAt(Vector3::ZERO);
	
	mCompositorIndex = 0;
	
}
