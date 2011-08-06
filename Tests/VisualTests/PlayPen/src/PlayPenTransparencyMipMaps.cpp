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

PlayPen_TransparencyMipMaps::PlayPen_TransparencyMipMaps()
{
	mInfo["Title"] = "PlayPen_TransparencyMipMaps";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(10);
}
//----------------------------------------------------------------------------

void PlayPen_TransparencyMipMaps::setupContent()
{
	MaterialPtr mat = MaterialManager::getSingleton().create("test", 
	TRANSIENT_RESOURCE_GROUP);
	// known png with alpha
	Pass* pass = mat->getTechnique(0)->getPass(0);
	pass->createTextureUnitState("sdk_logo.png");
	pass->setSceneBlending(SBT_TRANSPARENT_ALPHA);
	// alpha blend
	pass->setDepthWriteEnabled(false);
	
	// alpha reject
	//pass->setDepthWriteEnabled(true);
	//pass->setAlphaRejectSettings(CMPF_LESS, 128);
	
	// Define a floor plane mesh
	Plane p;
	p.normal = Vector3::UNIT_Y;
	p.d = 200;
	MeshManager::getSingleton().createPlane("FloorPlane",
	TRANSIENT_RESOURCE_GROUP,
	p,2000,2000,1,1,true,1,5,5,Vector3::UNIT_Z);
	
	// Create an entity (the floor)
	Entity* ent = mSceneMgr->createEntity("floor", "FloorPlane");
	ent->setMaterialName("test");
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ent);
	
	mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8);
	mSceneMgr->setAmbientLight(ColourValue::White);
	
	
	{
		
		Real alphaLevel = 0.5f;
		MaterialPtr alphamat = MaterialManager::getSingleton().create("testy", 
		TRANSIENT_RESOURCE_GROUP);
		Pass* pass = alphamat->getTechnique(0)->getPass(0);
		pass->setSceneBlending(SBT_TRANSPARENT_ALPHA);
		pass->setDepthWriteEnabled(false);
		TextureUnitState* t = pass->createTextureUnitState();
		t->setAlphaOperation(LBX_SOURCE1, LBS_MANUAL, LBS_CURRENT, alphaLevel);
		
		ent = mSceneMgr->createEntity("asd", "ogrehead.mesh");
		ent->setMaterialName("testy");
		mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ent);
		
	}

	mCamera->setPosition(0,0,1000);
	
}
