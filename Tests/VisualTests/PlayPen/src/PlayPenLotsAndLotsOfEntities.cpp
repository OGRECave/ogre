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


void createRandomEntityClones(Entity* ent, size_t cloneCount, 
	const Vector3& min, const Vector3& max, SceneManager* mgr)
{
	Entity *cloneEnt;
	for (size_t n = 0; n < cloneCount; ++n)
	{
		// Create a new node under the root
		SceneNode* node = mgr->createSceneNode();
		// Random translate
		Vector3 nodePos;
		nodePos.x = Math::RangeRandom(min.x, max.x);
		nodePos.y = Math::RangeRandom(min.y, max.y);
		nodePos.z = Math::RangeRandom(min.z, max.z);
		node->setPosition(nodePos);
		mgr->getRootSceneNode()->addChild(node);
		cloneEnt = ent->clone(ent->getName() + "_clone" + StringConverter::toString(n));
		// Attach to new node
		node->attachObject(cloneEnt);

	}
}

PlayPen_LotsAndLotsOfEntities::PlayPen_LotsAndLotsOfEntities()
{
	mInfo["Title"] = "PlayPen_LotsAndLotsOfEntities";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(3);// only go a couple frames, this one is slow (and static)
}
//----------------------------------------------------------------------------

void PlayPen_LotsAndLotsOfEntities::setupContent()
{
	// Set ambient light
	mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));
	
	// Create a point light
	Light* l = mSceneMgr->createLight("MainLight");
	l->setType(Light::LT_DIRECTIONAL);
	l->setDirection(-Vector3::UNIT_Y);
	
	// Create a set of random balls
	Entity* ent = mSceneMgr->createEntity("Ball", "cube.mesh");
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ent);
	createRandomEntityClones(ent, 3000, Vector3(-1000,-1000,-1000), Vector3(1000,1000,1000), mSceneMgr);
	
	//bool val = true;
	//mSceneMgr->setOption("ShowOctree", &val);
	
	mCamera->setPosition(0,0, -4000);
	mCamera->lookAt(Vector3::ZERO);
	
	// enable the profiler
	//Profiler* prof = Profiler::getSingletonPtr();
	//if (prof)
	//prof->setEnabled(true);
	
	
}
