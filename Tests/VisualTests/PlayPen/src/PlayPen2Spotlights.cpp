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

PlayPen_2Spotlights::PlayPen_2Spotlights()
{
	mInfo["Title"] = "PlayPen_2Spotlights";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(250);
}
//----------------------------------------------------------------------------

void PlayPen_2Spotlights::setupContent()
{
	SceneNode* mTestNode[5];
	mSceneMgr->setAmbientLight(ColourValue(0.3, 0.3, 0.3));
	
	Light* mLight = mSceneMgr->createLight("MainLight");
	// Spotlight test
	mLight->setType(Light::LT_SPOTLIGHT);
	mLight->setDiffuseColour(1.0, 0.0, 0.8);
	mLight->setSpotlightRange(Degree(30), Degree(40));
	mTestNode[0] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	mTestNode[0]->setPosition(800,600,0);
	mTestNode[0]->lookAt(Vector3(800,0,0), Node::TS_WORLD, Vector3::UNIT_Z);
	mTestNode[0]->attachObject(mLight);
	
	mTestNode[1] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	mLight = mSceneMgr->createLight("AnotherLight");
	// Spotlight test
	mLight->setType(Light::LT_SPOTLIGHT);
	mLight->setDiffuseColour(0, 1.0, 0.8);
	mLight->setSpotlightRange(Degree(30), Degree(40));
	mTestNode[1] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	mTestNode[1]->setPosition(0,600,800);
	mTestNode[1]->lookAt(Vector3(0,0,800), Node::TS_WORLD, Vector3::UNIT_Z);
	mTestNode[1]->attachObject(mLight);
	
	
	Plane plane;
	plane.normal = Vector3::UNIT_Y;
	plane.d = 100;
	MeshManager::getSingleton().createPlane("Myplane",
	TRANSIENT_RESOURCE_GROUP, plane,
	3500,3500,100,100,true,1,5,5,Vector3::UNIT_Z);
	Entity* pPlaneEnt;
	pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
	pPlaneEnt->setMaterialName("2 - Default");
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);

	mCamera->setPosition(-600,300,-600);
	mCamera->lookAt(300,0,300);
	
}
