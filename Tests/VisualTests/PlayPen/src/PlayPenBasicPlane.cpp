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

PlayPen_BasicPlane::PlayPen_BasicPlane()
{
	mInfo["Title"] = "PlayPen_BasicPlane";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(10);
}
//----------------------------------------------------------------------------

void PlayPen_BasicPlane::setupContent()
{
	/*
	// Create a light
	Light* l = mSceneMgr->createLight("MainLight");
	// Accept default settings: point light, white diffuse, just set position
	// NB I could attach the light to a SceneNode if I wanted it to move automatically with
	//  other objects, but I don't
	l->setPosition(20,80,50);
	*/
	
	// Create a point light
	Light* l = mSceneMgr->createLight("MainLight");
	l->setType(Light::LT_DIRECTIONAL);
	l->setDirection(-Vector3::UNIT_Y);
	Entity *ent;
	
	// Define a floor plane mesh
	Plane p;
	p.normal = Vector3::UNIT_Y;
	p.d = 200;
	MeshManager::getSingleton().createPlane("FloorPlane",
	TRANSIENT_RESOURCE_GROUP,
	p,2000,2000,1,1,true,1,5,5,Vector3::UNIT_Z);
	
	// Create an entity (the floor)
	ent = mSceneMgr->createEntity("floor", "FloorPlane");
	ent->setMaterialName("Examples/RustySteel");
	
	mSceneMgr->getRootSceneNode()->attachObject(ent);
	
	Entity* sphereEnt = mSceneMgr->createEntity("ogre", "ogrehead.mesh");
	
	SceneNode* mRootNode = mSceneMgr->getRootSceneNode();
	SceneNode* node = mSceneMgr->createSceneNode();
	node->attachObject(sphereEnt);
	mRootNode->addChild(node);
	
}
