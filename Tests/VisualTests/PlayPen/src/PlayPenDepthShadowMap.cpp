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

PlayPen_DepthShadowMap::PlayPen_DepthShadowMap()
{
	mInfo["Title"] = "PlayPen_DepthShadowMap";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(10);
}
//----------------------------------------------------------------------------

void PlayPen_DepthShadowMap::setupContent()
{
	mSceneMgr->setShadowTextureCount(1);
	mSceneMgr->setShadowTextureConfig(0, 1024, 1024, PF_FLOAT32_R);
	mSceneMgr->setShadowTextureSelfShadow(true);
	mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_ADDITIVE);
	mSceneMgr->setShadowCasterRenderBackFaces(false);
	
	mSceneMgr->setShadowTextureCasterMaterial("Ogre/DepthShadowmap/Caster/Float");

	LiSPSMShadowCameraSetup *mLiSPSMSetup = new LiSPSMShadowCameraSetup();
	//mLiSPSMSetup->setUseAggressiveFocusRegion(false);
	ShadowCameraSetupPtr mCurrentShadowCameraSetup = ShadowCameraSetupPtr(mLiSPSMSetup);
	//ShadowCameraSetupPtr mCurrentShadowCameraSetup = ShadowCameraSetupPtr(new PlaneOptimalShadowCameraSetup(mPlane));					
	mSceneMgr->setShadowCameraSetup(mCurrentShadowCameraSetup);
	
	// Single light
	Light* l = mSceneMgr->createLight("l1");
	l->setType(Light::LT_SPOTLIGHT);
	//l->setPosition(500, 500, -100);
	l->setPosition(0, 300, 0);
	Vector3 dir = -l->getPosition();
	dir.normalise();
	l->setDirection(dir);
	l->setSpotlightOuterAngle(Degree(40));
	l->setSpotlightInnerAngle(Degree(35));
	
	// ground plane
	MovablePlane movablePlane = MovablePlane(Vector3::UNIT_Y, 0.f);

	MeshManager::getSingleton().createPlane("Myplane",
	TRANSIENT_RESOURCE_GROUP, movablePlane,
	500,500,10,10,true,1,5,5,Vector3::UNIT_Z);
	Entity* pPlaneEnt;
	pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
	pPlaneEnt->setMaterialName("Ogre/DepthShadowmap/Receiver/RockWall");
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);
	
	// box
	ManualObject* man = mSceneMgr->createManualObject("box");
	Real boxsize = 50;
	Real boxsizehalf = boxsize / 2.0;
	man->begin("Ogre/DepthShadowmap/Receiver/Float");
	man->position(-boxsizehalf, 0, boxsizehalf);
	man->position(boxsizehalf, 0, boxsizehalf);
	man->position(boxsizehalf, 0, -boxsizehalf);
	man->position(-boxsizehalf, 0, -boxsizehalf);
	man->position(-boxsizehalf, boxsize, boxsizehalf);
	man->position(boxsizehalf, boxsize, boxsizehalf);
	man->position(boxsizehalf, boxsize, -boxsizehalf);
	man->position(-boxsizehalf, boxsize, -boxsizehalf);
	man->quad(3, 2, 1, 0);
	man->quad(4, 5, 6, 7);
	man->quad(0, 1, 5, 4);
	man->quad(1, 2, 6, 5);
	man->quad(2, 3, 7, 6);
	man->quad(3, 0, 4, 7);
	man->end();
	
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(man);
	
	mCamera->setPosition(150, 100, 150);
	mCamera->lookAt(Vector3::ZERO);
	
	// Create RTT
	//TexturePtr rtt = TextureManager::getSingleton().createManual("rtt1", TRANSIENT_RESOURCE_GROUP, 
	//	TEX_TYPE_2D, 1024, 1024, 1, 0, PF_FLOAT32_R);
	
	
	
}
