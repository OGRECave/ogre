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

// render queues
#define RENDER_QUEUE_OUTLINE_GLOW_OBJECTS	RENDER_QUEUE_MAIN + 1
#define RENDER_QUEUE_OUTLINE_GLOW_GLOWS		RENDER_QUEUE_MAIN + 2
#define RENDER_QUEUE_FULL_GLOW_ALPHA_GLOW	RENDER_QUEUE_MAIN + 3
#define RENDER_QUEUE_FULL_GLOW_GLOW			RENDER_QUEUE_MAIN + 4
#define LAST_STENCIL_OP_RENDER_QUEUE		RENDER_QUEUE_FULL_GLOW_GLOW

// stencil values
#define STENCIL_VALUE_FOR_OUTLINE_GLOW 1
#define STENCIL_VALUE_FOR_FULL_GLOW 2
#define STENCIL_FULL_MASK 0xFFFFFFFF

// a Render queue listener to change the stencil mode
class StencilOpQueueListener : public Ogre::RenderQueueListener 
{ 
public: 
	virtual void renderQueueStarted(Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& skipThisInvocation) 
	{ 
		if (queueGroupId == RENDER_QUEUE_OUTLINE_GLOW_OBJECTS) // outline glow object 
		{ 
			Ogre::RenderSystem * rendersys = Ogre::Root::getSingleton().getRenderSystem(); 

			rendersys->clearFrameBuffer(Ogre::FBT_STENCIL); 
			rendersys->setStencilCheckEnabled(true); 
			rendersys->setStencilBufferParams(Ogre::CMPF_ALWAYS_PASS,
				STENCIL_VALUE_FOR_OUTLINE_GLOW, STENCIL_FULL_MASK, 
				Ogre::SOP_KEEP,Ogre::SOP_KEEP,Ogre::SOP_REPLACE,false);       
		} 
		if (queueGroupId == RENDER_QUEUE_OUTLINE_GLOW_GLOWS)  // outline glow
		{ 
			Ogre::RenderSystem * rendersys = Ogre::Root::getSingleton().getRenderSystem(); 
			rendersys->setStencilCheckEnabled(true); 
			rendersys->setStencilBufferParams(Ogre::CMPF_NOT_EQUAL,
				STENCIL_VALUE_FOR_OUTLINE_GLOW, STENCIL_FULL_MASK, 
				Ogre::SOP_KEEP,Ogre::SOP_KEEP,Ogre::SOP_REPLACE,false);       
		} 
		if (queueGroupId == RENDER_QUEUE_FULL_GLOW_ALPHA_GLOW)  // full glow - alpha glow
		{ 
			Ogre::RenderSystem * rendersys = Ogre::Root::getSingleton().getRenderSystem(); 
			rendersys->setStencilCheckEnabled(true); 
			rendersys->setStencilBufferParams(Ogre::CMPF_ALWAYS_PASS,
				STENCIL_VALUE_FOR_FULL_GLOW,STENCIL_FULL_MASK, 
				Ogre::SOP_KEEP,Ogre::SOP_KEEP,Ogre::SOP_REPLACE,false);       
		} 

		if (queueGroupId == RENDER_QUEUE_FULL_GLOW_GLOW)  // full glow - glow
		{ 
			Ogre::RenderSystem * rendersys = Ogre::Root::getSingleton().getRenderSystem(); 
			rendersys->setStencilCheckEnabled(true); 
			rendersys->setStencilBufferParams(Ogre::CMPF_EQUAL,
				STENCIL_VALUE_FOR_FULL_GLOW,STENCIL_FULL_MASK, 
				Ogre::SOP_KEEP,Ogre::SOP_KEEP,Ogre::SOP_ZERO,false);       
		} 

	} 

	virtual void renderQueueEnded(Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& repeatThisInvocation) 
	{ 
		if (( queueGroupId == LAST_STENCIL_OP_RENDER_QUEUE )

			) 
		{ 
			Ogre::RenderSystem * rendersys = Ogre::Root::getSingleton().getRenderSystem(); 
			rendersys->setStencilCheckEnabled(false); 
			rendersys->setStencilBufferParams(); 
		} 
	} 

}; 
//---------------------------------------------------------------------------

PlayPen_StencilGlow::PlayPen_StencilGlow()
	:mStencilListener(0)
{
    mInfo["Title"] = "PlayPen_StencilGlow";
    mInfo["Description"] = "Tests stencil glow effect";
	addScreenshotFrame(150);
}
//---------------------------------------------------------------------------

PlayPen_StencilGlow::~PlayPen_StencilGlow()
{
	if(mStencilListener)
		delete mStencilListener;
}
//---------------------------------------------------------------------------

void PlayPen_StencilGlow::setupContent(void)
{
	// test media
	Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("Tests");
	
	// Set ambient light
	mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

	// Create a point light
	Light* l = mSceneMgr->createLight("MainLight");
	// Accept default settings: point light, white diffuse, just set position
	// NB I could attach the light to a SceneNode if I wanted it to move automatically with
	//  other objects, but I don't
	l->setPosition(20,80,50);


	// outline glow entity
	Entity *outlineGlowEntity = mSceneMgr->createEntity("outlineGlow", "ogrehead.mesh");
	outlineGlowEntity->setRenderQueueGroup(RENDER_QUEUE_OUTLINE_GLOW_OBJECTS);
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(outlineGlowEntity);


	// outline glow entity actual glow
	Ogre::Entity* actualOutlineGlowEntity = outlineGlowEntity->clone(outlineGlowEntity->getName() + "_glow"); 
	actualOutlineGlowEntity->setRenderQueueGroup(RENDER_QUEUE_OUTLINE_GLOW_GLOWS); 
	actualOutlineGlowEntity->setMaterialName("cg/glow"); 
	Ogre::SceneNode* actualOutlineGlowNode = outlineGlowEntity->getParentSceneNode()->createChildSceneNode("outlineGlowNode"); 
	actualOutlineGlowNode->attachObject(actualOutlineGlowEntity); 


	// normal entity
	Entity *normalOgreEntity = mSceneMgr->createEntity("normalOgreEntity", "ogrehead.mesh");
	Ogre::SceneNode* normalOgreNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	normalOgreNode->attachObject(normalOgreEntity);
	normalOgreNode->setPosition(80, 0, 0);
	 


	// full glow entity
	Entity *fullGlowEntity = mSceneMgr->createEntity("fullGlowEntity", "ogrehead.mesh");
	Ogre::SceneNode* fullGlowNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	fullGlowNode->attachObject(fullGlowEntity);
	fullGlowNode->setPosition(-80, 0, 0);

	// full glow alpha glow
	Ogre::Entity* alphaFullGlowEntity = fullGlowEntity->clone(fullGlowEntity->getName() + "_alphaGlow"); 
	alphaFullGlowEntity->setRenderQueueGroup(RENDER_QUEUE_FULL_GLOW_ALPHA_GLOW); 
	alphaFullGlowEntity->setMaterialName("cg/alpha_glow"); 
	Ogre::SceneNode* alphaFullGlowNode = fullGlowEntity->getParentSceneNode()->createChildSceneNode("fullGlowAlphaNode"); 
	alphaFullGlowNode->attachObject(alphaFullGlowEntity); 

	// full glow alpha glow
	Ogre::Entity* glowFullGlowEntity = fullGlowEntity->clone(fullGlowEntity->getName() + "_glow"); 
	glowFullGlowEntity->setRenderQueueGroup(RENDER_QUEUE_FULL_GLOW_GLOW); 
	glowFullGlowEntity->setMaterialName("cg/no_depth_check_glow"); 
	Ogre::SceneNode* glowFullGlowNode = fullGlowEntity->getParentSceneNode()->createChildSceneNode("fullGlowGlowNode"); 
	glowFullGlowNode->attachObject(glowFullGlowEntity); 

	mCamera->setPosition(0,0,200);
	mCamera->setDirection(0,0,-1);

	mStencilListener = new StencilOpQueueListener();
	mSceneMgr->addRenderQueueListener(mStencilListener);

}
//---------------------------------------------------------------------------

