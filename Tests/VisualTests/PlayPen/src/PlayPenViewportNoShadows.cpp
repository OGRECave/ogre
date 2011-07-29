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

PlayPen_ViewportNoShadows::PlayPen_ViewportNoShadows()
{
	mInfo["Title"] = "PlayPen_ViewportNoShadows";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(250);
}
//----------------------------------------------------------------------------

void PlayPen_ViewportNoShadows::setupContent()
{
	mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_ADDITIVE);
	
	// Setup lighting
	mSceneMgr->setAmbientLight(ColourValue(0.2, 0.2, 0.2));
	Light* light = mSceneMgr->createLight("MainLight");
	light->setType(Light::LT_DIRECTIONAL);
	Vector3 dir(-1, -1, 0.5);
	dir.normalise();
	light->setDirection(dir);
	
	// Create a skydome
	//mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8);
	
	// Create a floor plane mesh
	Plane plane(Vector3::UNIT_Y, 0.0);
	MeshManager::getSingleton().createPlane(
	"FloorPlane", TRANSIENT_RESOURCE_GROUP,
	plane, 200000, 200000, 20, 20, true, 1, 500, 500, Vector3::UNIT_Z);
	
	
	// Add a floor to the scene
	Entity* entity = mSceneMgr->createEntity("floor", "FloorPlane");
	entity->setMaterialName("Examples/RustySteel");
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(entity);
	entity->setCastShadows(false);
	
	// Add the mandatory ogre head
	entity = mSceneMgr->createEntity("head", "ogrehead.mesh");
	mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(0.0, 10.0, 0.0))->attachObject(entity);
	
	// Position and orient the camera
	mCamera->setPosition(-100.0, 50.0, 90.0);
	mCamera->lookAt(0.0, 10.0, -35.0);
	
	// Add an additional viewport on top of the other one
	Viewport* pip = mWindow->addViewport(mCamera, 1, 0.7, 0.0, 0.3, 0.3);
	pip->setShadowsEnabled(false);
	
}
