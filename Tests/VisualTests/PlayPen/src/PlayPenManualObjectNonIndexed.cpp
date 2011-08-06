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

PlayPen_ManualObjectNonIndexed::PlayPen_ManualObjectNonIndexed()
{
	mInfo["Title"] = "PlayPen_ManualObjectNonIndexed";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(10);
}
//----------------------------------------------------------------------------

void PlayPen_ManualObjectNonIndexed::setupContent()
{
	mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));
	Vector3 dir(-1, -1, 0.5);
	dir.normalise();
	Light* l = mSceneMgr->createLight("light1");
	l->setType(Light::LT_DIRECTIONAL);
	l->setDirection(dir);
	
	Plane plane;
	plane.normal = Vector3::UNIT_Y;
	plane.d = 100;
	MeshManager::getSingleton().createPlane("Myplane",
	TRANSIENT_RESOURCE_GROUP, plane,
	1500,1500,10,10,true,1,5,5,Vector3::UNIT_Z);
	Entity* pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
	pPlaneEnt->setMaterialName("2 - Default");
	pPlaneEnt->setCastShadows(false);
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);
	
	ManualObject* man = static_cast<ManualObject*>(
	mSceneMgr->createMovableObject("test", ManualObjectFactory::FACTORY_TYPE_NAME));
	
	man->begin("Examples/OgreLogo");
	// Define a 40x40 plane, non-indexed
	man->position(-20, 20, 20);
	man->normal(0, 0, 1);
	man->textureCoord(0, 0);
	
	man->position(-20, -20, 20);
	man->normal(0, 0, 1);
	man->textureCoord(0, 1);
	
	man->position(20, 20, 20);
	man->normal(0, 0, 1);
	man->textureCoord(1, 0);
	
	man->position(-20, -20, 20);
	man->normal(0, 0, 1);
	man->textureCoord(0, 1);
	
	man->position(20, -20, 20);
	man->normal(0, 0, 1);
	man->textureCoord(1, 1);
	
	man->position(20, 20, 20);
	man->normal(0, 0, 1);
	man->textureCoord(1, 0);
	
	man->end();
	
	man->begin("Examples/BumpyMetal");
	
	// Define a 40x40 plane, non-indexed
	man->position(-20, 20, 20);
	man->normal(0, 1, 0);
	man->textureCoord(0, 0);
	
	man->position(20, 20, 20);
	man->normal(0, 1, 0);
	man->textureCoord(0, 1);
	
	man->position(20, 20, -20);
	man->normal(0, 1, 0);
	man->textureCoord(1, 1);
	
	man->position(20, 20, -20);
	man->normal(0, 1, 0);
	man->textureCoord(1, 1);
	
	man->position(-20, 20, -20);
	man->normal(0, 1, 0);
	man->textureCoord(1, 0);
	
	man->position(-20, 20, 20);
	man->normal(0, 1, 0);
	man->textureCoord(0, 0);
	
	man->end();
	
	
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(man);
	mCamera->setPosition(100,100,100);
	mCamera->lookAt(0,0,0);
	
}
