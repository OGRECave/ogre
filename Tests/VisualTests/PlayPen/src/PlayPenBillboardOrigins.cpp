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

PlayPen_BillboardOrigins::PlayPen_BillboardOrigins()
{
	mInfo["Title"] = "PlayPen_BillboardOrigins";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(250);
}
//----------------------------------------------------------------------------

void PlayPen_BillboardOrigins::setupContent()
{
	mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));
	Vector3 dir(-1, -1, 0.5);
	dir.normalise();
	Light* l = mSceneMgr->createLight("light1");
	l->setType(Light::LT_DIRECTIONAL);
	l->setDirection(dir);
	
	/*Plane plane;
	plane.normal = Vector3::UNIT_Y;
	plane.d = 0;
	MeshManager::getSingleton().createPlane("Myplane",
	ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane,
	1500,1500,10,10,true,1,5,5,Vector3::UNIT_Z);
	Entity* pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
	pPlaneEnt->setMaterialName("2 - Default");
	pPlaneEnt->setCastShadows(false);
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);*/
	
	BillboardSet* bbs = mSceneMgr->createBillboardSet("1");
	bbs->setDefaultDimensions(50,50);
	bbs->createBillboard(0, 0, 0);
	bbs->setBillboardOrigin(BBO_TOP_LEFT);
	bbs->setMaterialName("2 - Default");
	//bbs->setBillboardType(BBT_ORIENTED_COMMON);
	bbs->setCommonDirection(Vector3::UNIT_Y);
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(bbs);
	
	bbs = mSceneMgr->createBillboardSet("2");
	bbs->setDefaultDimensions(50,50);
	bbs->createBillboard(0, -10, 0);
	bbs->setBillboardOrigin(BBO_CENTER);
	bbs->setMaterialName("Examples/RustySteel");
	//bbs->setBillboardType(BBT_ORIENTED_COMMON);
	bbs->setCommonDirection(Vector3::UNIT_Y);
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(bbs);

	bbs = mSceneMgr->createBillboardSet("3");
	bbs->setDefaultDimensions(50,50);
	bbs->createBillboard(0, -20, 0);
	bbs->setBillboardOrigin(BBO_BOTTOM_RIGHT);
	bbs->setMaterialName("Examples/OgreLogo");
	//bbs->setBillboardType(BBT_ORIENTED_COMMON);
	bbs->setCommonDirection(Vector3::UNIT_Y);
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(bbs);

	mCamera->setPosition(0,160,1);
	mCamera->lookAt(0,0,0);
}
