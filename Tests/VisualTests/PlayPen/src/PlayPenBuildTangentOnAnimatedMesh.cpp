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

PlayPen_BuildTangentOnAnimatedMesh::PlayPen_BuildTangentOnAnimatedMesh()
{
	mInfo["Title"] = "PlayPen_BuildTangentOnAnimatedMesh";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(50);
}
//----------------------------------------------------------------------------

void PlayPen_BuildTangentOnAnimatedMesh::setupContent()
{
	SceneNode* mTestNode[2];
	Light* mLight = 0;
	//mSceneMgr->setShadowTextureSize(512);
	//mSceneMgr->setShadowTechnique(SHADOWTYPE_STENCIL_ADDITIVE);
	//mSceneMgr->setShadowFarDistance(1500);
	//mSceneMgr->setShadowColour(ColourValue(0.35, 0.35, 0.35));
	//mSceneMgr->setShadowFarDistance(800);
	// Set ambient light
	mSceneMgr->setAmbientLight(ColourValue(0.3, 0.3, 0.3));
	
	mLight = mSceneMgr->createLight("MainLight");
	
	/*/
	// Directional test
	mLight->setType(Light::LT_DIRECTIONAL);
	Vector3 vec(-1,-1,0);
	vec.normalise();
	mLight->setDirection(vec);
	/*/
	// Point test
	mLight->setType(Light::LT_POINT);
	mLight->setPosition(0, 200, 0);
	//*/
	MeshPtr pMesh = MeshManager::getSingleton().load("ninja.mesh",
	TRANSIENT_RESOURCE_GROUP/*,    
	HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY, 
	HardwareBuffer::HBU_STATIC_WRITE_ONLY, 
	true, true*/); //so we can still read it
	// Build tangent vectors, all our meshes use only 1 texture coordset 
	unsigned short src, dest;
	if (!pMesh->suggestTangentVectorBuildParams(VES_TANGENT, src, dest))
	{
		pMesh->buildTangentVectors(VES_TANGENT, src, dest);
	}
	
	Entity* pEnt = mSceneMgr->createEntity("Ninja", "ninja.mesh");
	
	/*/
	mAnimState = pEnt->getAnimationState("Walk");
	mAnimState->setEnabled(true);
	/*/
	pEnt->getAnimationState("Walk")->setEnabled(true);
	//*/
	mTestNode[1] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	mTestNode[1]->attachObject( pEnt );
	mTestNode[1]->translate(-100,-100,0);
	
	
	Plane plane;
	plane.normal = Vector3::UNIT_Y;
	plane.d = 100;
	MeshManager::getSingleton().createPlane("Myplane",
	TRANSIENT_RESOURCE_GROUP, plane,
	1500,1500,10,10,true,1,5,5,Vector3::UNIT_Z);
	Entity* pPlaneEnt;
	pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
	pPlaneEnt->setMaterialName("2 - Default");
	pPlaneEnt->setCastShadows(false);
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);

	mCamera->setPosition(0,0,400);
}
