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

PlayPen_StencilShadowsMixedOpSubMeshes::PlayPen_StencilShadowsMixedOpSubMeshes()
{
	mInfo["Title"] = "PlayPen_StencilShadowsMixedOpSubMeshes";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(20);
}
//----------------------------------------------------------------------------

void PlayPen_StencilShadowsMixedOpSubMeshes::setupContent()
{
	//mSceneMgr->setShadowTechnique(SHADOWTYPE_STENCIL_MODULATIVE);
	mSceneMgr->setShadowTechnique(SHADOWTYPE_STENCIL_ADDITIVE);
	//mSceneMgr->setShowDebugShadows(true);
	mSceneMgr->setShadowDirectionalLightExtrusionDistance(1000);
	
	//mSceneMgr->setShadowFarDistance(800);
	// Set ambient light
	mSceneMgr->setAmbientLight(ColourValue(0.0, 0.0, 0.0));
	
	Light* mLight;

	// Point light
	//if(pointLight)
	//{
		mLight = mSceneMgr->createLight("MainLight");
		mLight->setPosition(-400,400,-300);
		mLight->setDiffuseColour(0.9, 0.9, 1);
		mLight->setSpecularColour(0.9, 0.9, 1);
		mLight->setAttenuation(6000,1,0.001,0);
	//}
	// Directional light
	//if (directionalLight)
	//{
		/*mLight = mSceneMgr->createLight("Light2");
		Vector3 dir(-1,-1,0);
		dir.normalise();
		mLight->setType(Light::LT_DIRECTIONAL);
		mLight->setDirection(dir);
		mLight->setDiffuseColour(1, 1, 0.8);
		mLight->setSpecularColour(1, 1, 1);*/
	//}
	
	mSceneMgr->setSkyBox(true, "Examples/CloudyNoonSkyBox");
	
	
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
	
	//mCamera->setPosition(180, 34, 223);
	//mCamera->setOrientation(Quaternion(0.7265, -0.2064, 0.6304, 0.1791));
	
	
	ManualObject* man = mSceneMgr->createManualObject("testMO_");
	man->begin("2 - Default");
	man->position(0, 200, 0);
	man->position(0, 50, 100);
	man->position(100, 50, -100);
	man->position(-100, 50, -100);
	man->triangle(0, 1, 2);
	man->triangle(0, 2, 3);
	man->triangle(0, 3, 1);
	man->end();
	man->begin("2 - Default", RenderOperation::OT_LINE_STRIP);
	man->position(0, 200, 0);
	man->position(50, 250, 0);
	man->position(200, 300, 0);
	man->index(0);
	man->index(1);
	man->index(2);
	man->end();
	MeshPtr msh = man->convertToMesh("testMO.mesh_2");
	
	Entity* e = mSceneMgr->createEntity("34", "testMO.mesh_2");
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(e);
	
	mCamera->setPosition(100,320,600);
	mCamera->lookAt(0,120,0);
	
}
