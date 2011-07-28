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

PlayPen_ClearScene::PlayPen_ClearScene()
	:mFramesElapsed(0)
{
	mInfo["Title"] = "PlayPen_ClearScene";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(25);
}
//----------------------------------------------------------------------------

bool PlayPen_ClearScene::frameStarted(const Ogre::FrameEvent& evt)
{
	++mFramesElapsed;
	if(mFramesElapsed == 20)
		mSceneMgr->clearScene();
	return true;
}
//----------------------------------------------------------------------------

void PlayPen_ClearScene::setupContent()
{
	// Define a floor plane mesh
	Plane p;
	p.normal = Vector3::UNIT_Y;
	p.d = 200;
	MeshManager::getSingleton().createPlane("FloorPlane",
	TRANSIENT_RESOURCE_GROUP,
	p,200000,200000,20,20,true,1,50,50,Vector3::UNIT_Z);

	Entity* planeEnt;
	planeEnt = mSceneMgr->createEntity( "plane", "FloorPlane" );
	planeEnt->setMaterialName("Examples/Rockwall");
	mSceneMgr->getRootSceneNode()->attachObject(planeEnt);

	mCamera->setPosition(0,500,100);
	mCamera->lookAt(0,0,0);
}
//----------------------------------------------------------------------------
