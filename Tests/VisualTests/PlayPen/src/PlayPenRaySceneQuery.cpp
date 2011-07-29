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

PlayPen_RaySceneQuery::PlayPen_RaySceneQuery()
{
	mInfo["Title"] = "PlayPen_RaySceneQuery";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(10);
}
//----------------------------------------------------------------------------

bool PlayPen_RaySceneQuery::frameStarted(const FrameEvent& evt)
{
	rayQuery->setRay(mCamera->getCameraToViewportRay(0.5, 0.5));
	RaySceneQueryResult& results = rayQuery->execute();
	for (RaySceneQueryResult::iterator mov = results.begin();
			mov != results.end(); ++mov)
	{
		if (mov->movable)
		{
			if (mov->movable->getMovableType() == "Entity")
			{
				Entity* ent = static_cast<Entity*>(mov->movable);
				ent->setMaterialName("Examples/RustySteel");
			}
		}
	}
	return true;
}
//----------------------------------------------------------------------------

void PlayPen_RaySceneQuery::setupContent()
{
	mCamera->setPosition(0,0,500);
	mCamera->lookAt(0,0,0);

	// Set ambient light
	mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));
	
	// Create a point light
	Light* l = mSceneMgr->createLight("MainLight");
	l->setType(Light::LT_DIRECTIONAL);
	l->setDirection(-Vector3::UNIT_Y);
	
	// Create a set of random balls
	Entity* ent = mSceneMgr->createEntity("Ball", "sphere.mesh");
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ent);
	createRandomEntityClones(ent, 100, Vector3(-1000,-1000,-1000), Vector3(1000,1000,1000), mSceneMgr);

	// stick one at the origin so one will always be hit
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(
		mSceneMgr->createEntity("Ball_origin", "sphere.mesh"));
	
	rayQuery = mSceneMgr->createRayQuery(
	mCamera->getCameraToViewportRay(0.5, 0.5));
	rayQuery->setSortByDistance(true, 1);
	
	bool val = true;
	mSceneMgr->setOption("ShowOctree", &val);

	//mCamera->setFarClipDistance(0);
}
