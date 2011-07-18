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

PlayPen_StaticGeometry::PlayPen_StaticGeometry()
{
	mInfo["Title"] = "PlayPen_StaticGeometry";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(250);
}
//----------------------------------------------------------------------------

void PlayPen_StaticGeometry::setupContent()
{
	
	// Set ambient light
	mSceneMgr->setAmbientLight(ColourValue(0, 0, 0));
	
	// Create a point light
	Light* l = mSceneMgr->createLight("MainLight");
	l->setDiffuseColour(0.4, 0.4, 0.4);
	l->setSpecularColour(ColourValue::White);
	
	SceneNode* animNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	Animation* anim = mSceneMgr->createAnimation("an1", 20);
	anim->setInterpolationMode(Animation::IM_SPLINE);
	NodeAnimationTrack* track = anim->createNodeTrack(1, animNode);
	TransformKeyFrame* kf = track->createNodeKeyFrame(0);
	kf->setTranslate(Vector3(2300, 600, 2300));
	kf = track->createNodeKeyFrame(5);
	kf->setTranslate(Vector3(-2300, 600, 2300));
	kf = track->createNodeKeyFrame(10);
	kf->setTranslate(Vector3(-2300, 600, -2300));
	kf = track->createNodeKeyFrame(15);
	kf->setTranslate(Vector3(2300, 600, -2300));
	kf = track->createNodeKeyFrame(20);
	kf->setTranslate(Vector3(2300, 600, 2300));
	
	//animNode->attachObject(l);
	l->setPosition(0, 600, 0);
	l->setAttenuation(10000, 1, 0, 0);
	
	AnimationState* animState = mSceneMgr->createAnimationState("an1");
	animState->setEnabled(true);
	mAnimStateList.push_back(animState);
	
	
	
	Plane plane;
	plane.normal = Vector3::UNIT_Y;
	plane.d = 0;
	MeshManager::getSingleton().createPlane("Myplane",
	ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane,
	4500,4500,10,10,true,1,5,5,Vector3::UNIT_Z);
	Entity* pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
	pPlaneEnt->setMaterialName("Examples/GrassFloor");
	pPlaneEnt->setCastShadows(false);
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);
	
	Vector3 min(-2000,30,-2000);
	Vector3 max(2000,30,2000);
	
	
	MeshPtr msh = MeshManager::getSingleton().load("ogrehead.mesh", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	msh->buildTangentVectors();
	
	Entity* e = mSceneMgr->createEntity("1", "ogrehead.mesh");
	e->setMaterialName("Examples/BumpMapping/MultiLightSpecular");
	
	StaticGeometry* s = mSceneMgr->createStaticGeometry("bing");
	s->setCastShadows(true);
	s->setRegionDimensions(Vector3(500,500,500));
	for (int i = 0; i < 10; ++i)
	{
		Vector3 pos;
		pos.x = Math::RangeRandom(min.x, max.x);
		pos.y = Math::RangeRandom(min.y, max.y);
		pos.z = Math::RangeRandom(min.z, max.z);
		
		s->addEntity(e, pos);
		Entity* e2 = e->clone("clone" + StringConverter::toString(i));
		mSceneMgr->getRootSceneNode()->createChildSceneNode(pos+Vector3(0,60,0))->attachObject(e2);
		
	}
	
	s->build();
	mCamera->setLodBias(0.5);
	mCamera->setPosition(0,400,1200);
	mCamera->setDirection(0,-0.3f,-1.2f);
	
	//mTestNode[0] = s->getRegionIterator().getNext()->getParentSceneNode();
	
	
	
	
	
	
	
}
