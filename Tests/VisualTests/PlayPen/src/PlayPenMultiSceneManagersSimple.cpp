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

PlayPen_MultiSceneManagersSimple::PlayPen_MultiSceneManagersSimple()
{
	mInfo["Title"] = "PlayPen_MultiSceneManagersSimple";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(10);
}
//----------------------------------------------------------------------------

void PlayPen_MultiSceneManagersSimple::setupContent()
{
	// Create a secondary scene manager with it's own camera
	SceneManager* sm2 = Root::getSingleton().createSceneManager(ST_GENERIC);
	Camera* camera2 = sm2->createCamera("cam2");
	camera2->setPosition(0,0,-500);
	camera2->lookAt(Vector3::ZERO);
	Entity* ent = sm2->createEntity("knot2", "knot.mesh");
	sm2->getRootSceneNode()->createChildSceneNode()->attachObject(ent);
	Light* l = sm2->createLight("l2");
	l->setPosition(100,50,-100);
	l->setDiffuseColour(ColourValue::Green);
	sm2->setAmbientLight(ColourValue(0.2, 0.2, 0.2));
	
	Viewport* vp = mWindow->addViewport(camera2, 1, 0.67, 0, 0.33, 0.25);
	vp->setOverlaysEnabled(false);
	vp->setBackgroundColour(ColourValue(1,0,0));
	
	// Use original SM for normal scene
	ent = mSceneMgr->createEntity("head", "ogrehead.mesh");
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ent);
	l = mSceneMgr->createLight("l2"); // note same name, will work since different SM
	l->setPosition(100,50,-100);
	l->setDiffuseColour(ColourValue::Red);
	mSceneMgr->setAmbientLight(ColourValue(0.2, 0.2, 0.2));
	mCamera->setPosition(0,0,500);
	
	
}
