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

PlayPen_Dxt1FromMemory::PlayPen_Dxt1FromMemory()
{
	mInfo["Title"] = "PlayPen_Dxt1FromMemory";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(250);
}
//----------------------------------------------------------------------------

void PlayPen_Dxt1FromMemory::setupContent()
{
	Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("Tests");
	
	DataStreamPtr stream = ResourceGroupManager::getSingleton().openResource("BumpyMetal_dxt1.dds");
	// manually load into image
	Image img;
	img.load(stream, "dds");
	TextureManager::getSingleton().loadImage("testdxtfrommem", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, img);
	
	
	
	MaterialPtr mat = MaterialManager::getSingleton().create("testdxt", 
	ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	Pass* p = mat->getTechnique(0)->getPass(0);
	p->setLightingEnabled(false);
	p->setCullingMode(CULL_NONE);
	TextureUnitState* t = p->createTextureUnitState("testdxtfrommem");
	Entity *e = mSceneMgr->createEntity("Plane", SceneManager::PT_PLANE);
	e->setMaterialName(mat->getName());
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(e);
	
	mCamera->setPosition(0,0,-300);
	mCamera->lookAt(Vector3::ZERO);
	
}
