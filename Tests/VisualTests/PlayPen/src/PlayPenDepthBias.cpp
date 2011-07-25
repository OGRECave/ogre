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

PlayPen_DepthBias::PlayPen_DepthBias()
{
	mInfo["Title"] = "PlayPen_DepthBias";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(250);
}
//----------------------------------------------------------------------------

void PlayPen_DepthBias::setupContent()
{
	Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("Tests");
	
	mSceneMgr->setAmbientLight(ColourValue::White);
	
	MaterialPtr mat = MaterialManager::getSingleton().create("mat1", 
	TRANSIENT_RESOURCE_GROUP);
	Pass* p = mat->getTechnique(0)->getPass(0);
	p->createTextureUnitState("BumpyMetal.jpg");
	
	const String meshName("cube.mesh"); 
	Entity* entity = mSceneMgr->createEntity("base", meshName);
	entity->setMaterialName("mat1");
	mSceneMgr->getRootSceneNode()->attachObject(entity);
	
	
	entity = mSceneMgr->createEntity("base2", meshName);
	entity->setMaterialName("Examples/SphereMappedRustySteel");
	SceneNode* n = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	n->setPosition(-30, 0, 0);
	n->yaw(Degree(45));
	n->attachObject(entity);
	
	for (size_t i = 0; i <= 6;++i)
	{
		String name("decal");
		name += StringConverter::toString(i);
		
		MaterialPtr pMat = static_cast<MaterialPtr>(MaterialManager::getSingleton().create(
		name, TRANSIENT_RESOURCE_GROUP));
		
		pMat->getTechnique(0)->getPass(0)->setLightingEnabled(false);
		pMat->getTechnique(0)->getPass(0)->setAlphaRejectSettings(CMPF_GREATER_EQUAL, 128);
		pMat->getTechnique(0)->getPass(0)->setDepthBias(i);
		pMat->getTechnique(0)->getPass(0)->createTextureUnitState(name + ".png");
		
		entity = mSceneMgr->createEntity(name, meshName);
		entity->setMaterialName(name);
		mSceneMgr->getRootSceneNode()->attachObject(entity);
	}
	
	
	
	mCamera->setPosition(0,0,200);
	mCamera->lookAt(Vector3::ZERO);
	
	
}
