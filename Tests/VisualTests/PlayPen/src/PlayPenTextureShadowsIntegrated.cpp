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

PlayPen_TextureShadowsIntegrated::PlayPen_TextureShadowsIntegrated()
{
	mInfo["Title"] = "PlayPen_TextureShadowsIntegrated";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(250);
}
//----------------------------------------------------------------------------

void PlayPen_TextureShadowsIntegrated::cleanupContent()
{
	clearDebugTextureOverlays();
}
//----------------------------------------------------------------------------

void PlayPen_TextureShadowsIntegrated::setupContent()
{
	mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED);
	//mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE);
	MaterialManager::getSingleton().setDefaultTextureFiltering(TFO_ANISOTROPIC);
	MaterialManager::getSingleton().setDefaultAnisotropy(5);
	
	mSceneMgr->setShadowTextureSettings(1024, 2);
	
	mSceneMgr->setAmbientLight(ColourValue::Black);
	Light* l = mSceneMgr->createLight("Spot1");
	l->setType(Light::LT_SPOTLIGHT);
	l->setAttenuation(5000,1,0,0);
	l->setSpotlightRange(Degree(30),Degree(45),1.0f);
	SceneNode* lightNode1 = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	lightNode1->attachObject(l);
	lightNode1->setPosition(400, 250, 500);
	lightNode1->lookAt(Vector3(0,-200,0), Node::TS_WORLD);
	l->setDirection(Vector3::NEGATIVE_UNIT_Z);
	l->setDiffuseColour(0.7, 0.7, 0.5);
	
	l = mSceneMgr->createLight("Spot2");
	l->setAttenuation(5000,1,0,0);
	/* // spotlight */
	l->setType(Light::LT_SPOTLIGHT);
	l->setSpotlightRange(Degree(30),Degree(45),1.0f);
	/**/
	// point
	SceneNode* lightNode2 = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	lightNode2->attachObject(l);
	lightNode2->setPosition(-500, 200, 500);
	lightNode2->lookAt(Vector3(0,-200,0), Node::TS_WORLD);
	l->setDirection(Vector3::NEGATIVE_UNIT_Z);
	/* // directional
	l->setType(Light::LT_DIRECTIONAL);
	Vector3 dir(0.5, -1, 0.5);
	dir.normalise();
	l->setDirection(dir);
	*/
	l->setDiffuseColour(1, 0.2, 0.2);
	
	/*
	// Test spot 3
	l = mSceneMgr->createLight("Spot3");
	l->setType(Light::LT_SPOTLIGHT);
	l->setAttenuation(5000,1,0,0);
	l->setSpotlightRange(Degree(30),Degree(45),1.0f);
	SceneNode* lightNode3 = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	lightNode3->attachObject(l);
	lightNode3->setPosition(700, 250, 500);
	lightNode3->lookAt(Vector3(0,-200,0), Node::TS_WORLD);
	l->setDirection(Vector3::NEGATIVE_UNIT_Z);
	l->setDiffuseColour(0.0, 0.7, 1.0);
	*/
	
	// Create a basic plane to have something in the scene to look at
	Plane plane;
	plane.normal = Vector3::UNIT_Y;
	plane.d = 100;
	MeshPtr msh = MeshManager::getSingleton().createPlane("Myplane",
	ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane,
	4500,4500,100,100,true,1,40,40,Vector3::UNIT_Z);
	msh->buildTangentVectors(VES_TANGENT);
	Entity* pPlaneEnt;
	pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
	//pPlaneEnt->setMaterialName("Examples/OffsetMapping/Specular");
	pPlaneEnt->setMaterialName("Examples/OffsetMapping/IntegratedShadows");
	pPlaneEnt->setCastShadows(false);
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);
	
	pPlaneEnt = mSceneMgr->createEntity( "plane2", "Myplane" );
	//pPlaneEnt->setMaterialName("Examples/OffsetMapping/Specular");
	pPlaneEnt->setMaterialName("Examples/OffsetMapping/IntegratedShadows");
	pPlaneEnt->setCastShadows(false);
	SceneNode* n = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	n->roll(Degree(90));
	n->translate(100,0,0);
	//n->attachObject(pPlaneEnt);
	
	pPlaneEnt = mSceneMgr->createEntity( "plane3", "Myplane" );
	//pPlaneEnt->setMaterialName("Examples/OffsetMapping/Specular");
	pPlaneEnt->setMaterialName("Examples/OffsetMapping/IntegratedShadows");
	pPlaneEnt->setCastShadows(false);
	n = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	n->pitch(Degree(90));
	n->yaw(Degree(-90));
	n->translate(0,0,-100);
	n->attachObject(pPlaneEnt);
	
	mCamera->setPosition(-50, 500, 1000);
	mCamera->lookAt(Vector3(-50,-100,0));
	
	Entity* ent = mSceneMgr->createEntity("athene", "athene.mesh");
	ent->setMaterialName("Examples/Athene/NormalMapped");
	mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(0,-20,0))->attachObject(ent);
	
	addTextureShadowDebugOverlay(2, mSceneMgr);
	
	
	
}
