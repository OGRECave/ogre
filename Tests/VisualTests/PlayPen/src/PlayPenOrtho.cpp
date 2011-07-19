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

PlayPen_Ortho::PlayPen_Ortho()
{
	mInfo["Title"] = "PlayPen_Ortho";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(250);
}
//----------------------------------------------------------------------------

void PlayPen_Ortho::setupContent()
{
	SceneNode* mTestNode[3];
	
	// Set ambient light
	mSceneMgr->setAmbientLight(ColourValue(0.0, 0.0, 0.0));
	
	// Create a point light
	Light* l = mSceneMgr->createLight("MainLight");
	l->setPosition(800,600,0);
	
	mTestNode[0] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	SceneNode* mLightNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	
	Entity* pEnt = mSceneMgr->createEntity( "3", "knot.mesh" );
	mTestNode[1] = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(-200, 0, -200));
	mTestNode[1]->attachObject( pEnt );
	
	pEnt = mSceneMgr->createEntity( "4", "knot.mesh" );
	mTestNode[2] = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(100, 0, 200));
	mTestNode[2]->attachObject( pEnt );
	
	
	mSceneMgr->setSkyBox(true, "Examples/CloudyNoonSkyBox");
	
	
	Plane plane;
	plane.normal = Vector3::UNIT_Y;
	plane.d = 100;
	MeshManager::getSingleton().createPlane("Myplane",
	ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane,
	1500,1500,10,10,true,1,5,5,Vector3::UNIT_Z);
	Entity* pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
	pPlaneEnt->setMaterialName("2 - Default");
	pPlaneEnt->setCastShadows(false);
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);
	
	mCamera->setFixedYawAxis(false);
	mCamera->setProjectionType(PT_ORTHOGRAPHIC);
	mCamera->setPosition(0,10000,0);
	mCamera->lookAt(0,0,0);
	mCamera->setNearClipDistance(1000);
	
}
