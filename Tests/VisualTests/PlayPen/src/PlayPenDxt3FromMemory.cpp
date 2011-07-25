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

PlayPen_Dxt3FromMemory::PlayPen_Dxt3FromMemory()
{
	mInfo["Title"] = "PlayPen_Dxt3FromMemory";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(250);
}
//----------------------------------------------------------------------------

void PlayPen_Dxt3FromMemory::setupContent()
{
	Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("Tests");
	
	DataStreamPtr stream = ResourceGroupManager::getSingleton().openResource("ogreborderUp_dxt3.dds");
	// manually load into image
	Image img;
	img.load(stream, "dds");
	TextureManager::getSingleton().loadImage("testdxtfrommem", TRANSIENT_RESOURCE_GROUP, img);
	
	
	
	MaterialPtr mat = MaterialManager::getSingleton().create("testdxt", 
	TRANSIENT_RESOURCE_GROUP);
	Pass* p = mat->getTechnique(0)->getPass(0);
	p->setLightingEnabled(false);
	p->setCullingMode(CULL_NONE);
	p->setSceneBlending(SBT_TRANSPARENT_ALPHA);
	p->setAlphaRejectSettings(CMPF_GREATER, 128);
	mat->setReceiveShadows(false);
	TextureUnitState* t = p->createTextureUnitState("testdxtfrommem");
	t->setTextureScale(0.5,0.5);
	Entity *e = mSceneMgr->createEntity("Plane", SceneManager::PT_PLANE);
	e->setMaterialName(mat->getName());
	SceneNode* n = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	n->setPosition(-50, 0, 35);
	n->yaw(Degree(90));
	n->attachObject(e);
	mWindow->getViewport(0)->setBackgroundColour(ColourValue::Red);
	
	mCamera->setPosition(0,0,300);
	mCamera->lookAt(Vector3::ZERO);
	
}
