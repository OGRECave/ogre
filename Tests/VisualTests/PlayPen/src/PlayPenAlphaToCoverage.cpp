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

PlayPen_AlphaToCoverage::PlayPen_AlphaToCoverage()
{
	mInfo["Title"] = "PlayPen_AlphaToCoverage";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(250);
}
//----------------------------------------------------------------------------

void PlayPen_AlphaToCoverage::setupContent()
{
	
	MaterialPtr mat = MaterialManager::getSingleton().create("testa2c", 
	TRANSIENT_RESOURCE_GROUP);
	Pass* p = mat->getTechnique(0)->getPass(0);
	p->setAlphaRejectSettings(CMPF_GREATER, 96);
	p->setLightingEnabled(false);
	p->setCullingMode(CULL_NONE);
	p->setAlphaToCoverageEnabled(true);
	TextureUnitState* t = p->createTextureUnitState("leaf.png");
	t->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
	Entity *e = mSceneMgr->createEntity("PlaneA2C", SceneManager::PT_PLANE);
	e->setMaterialName(mat->getName());
	mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(100, 0, 0))->attachObject(e);
	
	
	mat = MaterialManager::getSingleton().create("testnoa2c", 
	TRANSIENT_RESOURCE_GROUP);
	p = mat->getTechnique(0)->getPass(0);
	p->setAlphaRejectSettings(CMPF_GREATER, 96);
	p->setLightingEnabled(false);
	p->setCullingMode(CULL_NONE);
	p->setAlphaToCoverageEnabled(false);
	t = p->createTextureUnitState("leaf.png");
	t->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
	e = mSceneMgr->createEntity("PlaneNoA2C", SceneManager::PT_PLANE);
	e->setMaterialName(mat->getName());
	mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(-100, 0, 0))->attachObject(e);
	
	mat = MaterialManager::getSingleton().create("bg", 
	TRANSIENT_RESOURCE_GROUP);
	p = mat->getTechnique(0)->getPass(0);
	p->setLightingEnabled(false);
	p->setCullingMode(CULL_NONE);
	t = p->createTextureUnitState();
	t->setColourOperationEx(LBX_SOURCE1, LBS_MANUAL, LBS_CURRENT, ColourValue::White);
	e = mSceneMgr->createEntity("PlaneBg", SceneManager::PT_PLANE);
	e->setMaterialName(mat->getName());
	e->setRenderQueueGroup(RENDER_QUEUE_BACKGROUND);
	SceneNode* s = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(0, 0, -10));
	s->setScale(5,5,5);
	s->attachObject(e);
	
	mCamera->setPosition(0,0,300);
	mCamera->lookAt(Vector3::ZERO);
	
}
