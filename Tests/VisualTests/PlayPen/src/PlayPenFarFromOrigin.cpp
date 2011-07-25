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

PlayPen_FarFromOrigin::PlayPen_FarFromOrigin()
{
	mInfo["Title"] = "PlayPen_FarFromOrigin";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(250);
}
//----------------------------------------------------------------------------

void PlayPen_FarFromOrigin::cleanupContent()
{
	//clearDebugTextureOverlays();
}

void PlayPen_FarFromOrigin::setupContent()
{
	SceneNode* mTestNode[5];
	mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE);
	mSceneMgr->setShadowTextureSettings(1024, 2);
	
	Vector3 offset(100000, 0, 100000);
	//Vector3 offset(0, 0, 0);
	
	mSceneMgr->setAmbientLight(ColourValue(0.1, 0.1, 0.1));
	
	// Directional test
	Light* mLight = mSceneMgr->createLight("MainLight");
	mLight->setType(Light::LT_DIRECTIONAL);
	Vector3 vec(-1,-1,0);
	vec.normalise();
	mLight->setDirection(vec);
	mLight->setDiffuseColour(ColourValue(0.5, 0.5, 1.0));
	
	// Spotlight test
	mLight = mSceneMgr->createLight("SpotLight");
	mLight->setType(Light::LT_SPOTLIGHT);
	mLight->setAttenuation(10000, 1, 0, 0);
	mLight->setDiffuseColour(1.0, 1.0, 0.5);
	
	mTestNode[0] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	mTestNode[0]->setPosition(offset + Vector3(-400,300,1000));
	mTestNode[0]->lookAt(offset, Node::TS_WORLD, Vector3::UNIT_Z);
	mTestNode[0]->attachObject(mLight);
	
	
	mTestNode[1] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	mTestNode[1]->setPosition(offset);
	
	Entity* pEnt;
	pEnt = mSceneMgr->createEntity( "1", "knot.mesh" );
	mTestNode[1]->attachObject( pEnt );
	
	
	mSceneMgr->setSkyBox(true, "Examples/CloudyNoonSkyBox");
	
	
	Plane plane;
	plane.normal = Vector3::UNIT_Y;
	plane.d = 100;
	MeshManager::getSingleton().createPlane("Myplane",
	TRANSIENT_RESOURCE_GROUP, plane,
	2500,2500,10,10,true,1,5,5,Vector3::UNIT_Z);
	Entity* pPlaneEnt;
	pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
	pPlaneEnt->setMaterialName("2 - Default");
	pPlaneEnt->setCastShadows(false);
	mSceneMgr->getRootSceneNode()->createChildSceneNode(offset)->attachObject(pPlaneEnt);
	
	ParticleSystem* pSys2 = mSceneMgr->createParticleSystem("smoke", 
	"Examples/Smoke");
	mTestNode[4] = mSceneMgr->getRootSceneNode()->createChildSceneNode(offset + Vector3(-300, -100, 200));
	mTestNode[4]->attachObject(pSys2);
	
	mCamera->setPosition(offset + Vector3(0, 1000, 500));
	mCamera->lookAt(offset);
	mCamera->setFarClipDistance(10000);
	
	mSceneMgr->setCameraRelativeRendering(true);
	
	FocusedShadowCameraSetup* camSetup = new FocusedShadowCameraSetup();
	mSceneMgr->setShadowCameraSetup(ShadowCameraSetupPtr(camSetup));
	//addTextureShadowDebugOverlay(1, mSceneMgr);
	
}
