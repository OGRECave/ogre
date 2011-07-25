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

PlayPen_TextureShadowsIntegratedPSSM::PlayPen_TextureShadowsIntegratedPSSM()
{
	mInfo["Title"] = "PlayPen_TextureShadowsIntegratedPSSM";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(250);
}
//----------------------------------------------------------------------------

void PlayPen_TextureShadowsIntegratedPSSM::cleanupContent()
{
	clearDebugTextureOverlays();
}
//----------------------------------------------------------------------------

void PlayPen_TextureShadowsIntegratedPSSM::setupContent()
{
	mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED);
	
	// 3 textures per directional light
	mSceneMgr->setShadowTextureCountPerLightType(Ogre::Light::LT_DIRECTIONAL, 3);
	mSceneMgr->setShadowTextureSettings(512, 3, PF_FLOAT32_R);
	mSceneMgr->setShadowTextureSelfShadow(true);
	// Set up caster material - this is just a standard depth/shadow map caster
	mSceneMgr->setShadowTextureCasterMaterial("PSSM/shadow_caster");
	
	// shadow camera setup
	PSSMShadowCameraSetup* pssmSetup = new PSSMShadowCameraSetup();
	pssmSetup->calculateSplitPoints(3, mCamera->getNearClipDistance(), mCamera->getFarClipDistance());
	pssmSetup->setSplitPadding(10);
	pssmSetup->setOptimalAdjustFactor(0, 2);
	pssmSetup->setOptimalAdjustFactor(1, 1);
	pssmSetup->setOptimalAdjustFactor(2, 0.5);
	
	mSceneMgr->setShadowCameraSetup(ShadowCameraSetupPtr(pssmSetup));
	
	
	mSceneMgr->setAmbientLight(ColourValue(0.3, 0.3, 0.3));
	Light* l = mSceneMgr->createLight("Dir");
	l->setType(Light::LT_DIRECTIONAL);
	Vector3 dir(0.3, -1, 0.2);
	dir.normalise();
	l->setDirection(dir);
	
	
	// Create a basic plane to have something in the scene to look at
	Plane plane;
	plane.normal = Vector3::UNIT_Y;
	plane.d = 100;
	MeshPtr msh = MeshManager::getSingleton().createPlane("Myplane",
	TRANSIENT_RESOURCE_GROUP, plane,
	4500,4500,100,100,true,1,40,40,Vector3::UNIT_Z);
	msh->buildTangentVectors(VES_TANGENT);
	Entity* pPlaneEnt;
	pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
	pPlaneEnt->setMaterialName("PSSM/Plane");
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);
	
	mCamera->setPosition(-50, 500, 1000);
	mCamera->lookAt(Vector3(-50,-100,0));
	
	Entity* ent = mSceneMgr->createEntity("knot", "knot.mesh");
	ent->setMaterialName("PSSM/Knot");
	mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(0,0,0))->attachObject(ent);
	createRandomEntityClones(ent, 20, Vector3(-1000,0,-1000), Vector3(1000,0,1000), mSceneMgr);
	
	
	Vector4 splitPoints;
	const PSSMShadowCameraSetup::SplitPointList& splitPointList = pssmSetup->getSplitPoints();
	for (int i = 0; i < 3; ++i)
	{
		splitPoints[i] = splitPointList[i];
	}
	MaterialPtr mat = MaterialManager::getSingleton().getByName("PSSM/Plane");
	mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->setNamedConstant("pssmSplitPoints", splitPoints);
	mat = MaterialManager::getSingleton().getByName("PSSM/Knot");
	mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->setNamedConstant("pssmSplitPoints", splitPoints);
	
	addTextureShadowDebugOverlay(3, mSceneMgr);
	
	
	
}
