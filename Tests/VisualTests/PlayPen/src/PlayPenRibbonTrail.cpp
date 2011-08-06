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

PlayPen_RibbonTrail::PlayPen_RibbonTrail()
{
	mInfo["Title"] = "PlayPen_RibbonTrail";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(150);
}
//----------------------------------------------------------------------------

void PlayPen_RibbonTrail::setupContent()
{
	mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));
	Vector3 dir(-1, -1, 0.5);
	dir.normalise();
	Light* l = mSceneMgr->createLight("light1");
	l->setType(Light::LT_DIRECTIONAL);
	l->setDirection(dir);
	
	NameValuePairList pairList;
	pairList["numberOfChains"] = "2";
	pairList["maxElements"] = "80";
	RibbonTrail* trail = static_cast<RibbonTrail*>(
	mSceneMgr->createMovableObject("1", "RibbonTrail", &pairList));
	trail->setMaterialName("Examples/LightRibbonTrail");
	trail->setTrailLength(400);
	//mRibbonTrail = trail;
	
	
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(trail);
	
	// Create 3 nodes for trail to follow
	SceneNode* animNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	animNode->setPosition(0,20,0);
	Animation* anim = mSceneMgr->createAnimation("an1", 10);
	anim->setInterpolationMode(Animation::IM_SPLINE);
	NodeAnimationTrack* track = anim->createNodeTrack(1, animNode);
	TransformKeyFrame* kf = track->createNodeKeyFrame(0);
	kf->setTranslate(Vector3::ZERO);
	kf = track->createNodeKeyFrame(2);
	kf->setTranslate(Vector3(100, 0, 0));
	kf = track->createNodeKeyFrame(4);
	kf->setTranslate(Vector3(200, 0, 300));
	kf = track->createNodeKeyFrame(6);
	kf->setTranslate(Vector3(0, 20, 500));
	kf = track->createNodeKeyFrame(8);
	kf->setTranslate(Vector3(-100, 10, 100));
	kf = track->createNodeKeyFrame(10);
	kf->setTranslate(Vector3::ZERO);
	
	//testremoveNode = animNode;
	
	AnimationState* animState = mSceneMgr->createAnimationState("an1");
	animState->setEnabled(true);
	mAnimStateList.push_back(animState);
	
	trail->addNode(animNode);
	trail->setInitialColour(0, 1.0, 0.8, 0);
	trail->setColourChange(0, 0.5, 0.5, 0.5, 0.5);
	trail->setInitialWidth(0, 5);
	
	// Add light
	Light* l2 = mSceneMgr->createLight("l2");
	l2->setDiffuseColour(trail->getInitialColour(0));
	animNode->attachObject(l2);
	
	// Add billboard
	BillboardSet* bbs = mSceneMgr->createBillboardSet("bb", 1);
	bbs->createBillboard(Vector3::ZERO, trail->getInitialColour(0));
	bbs->setMaterialName("Examples/Flare");
	animNode->attachObject(bbs);
	
	animNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	animNode->setPosition(-50,10,0);
	anim = mSceneMgr->createAnimation("an2", 10);
	anim->setInterpolationMode(Animation::IM_SPLINE);
	track = anim->createNodeTrack(1, animNode);
	kf = track->createNodeKeyFrame(0);
	kf->setTranslate(Vector3::ZERO);
	kf = track->createNodeKeyFrame(2);
	kf->setTranslate(Vector3(-100, 150, -30));
	kf = track->createNodeKeyFrame(4);
	kf->setTranslate(Vector3(-200, 0, 40));
	kf = track->createNodeKeyFrame(6);
	kf->setTranslate(Vector3(0, -150, 70));
	kf = track->createNodeKeyFrame(8);
	kf->setTranslate(Vector3(50, 0, 30));
	kf = track->createNodeKeyFrame(10);
	kf->setTranslate(Vector3::ZERO);
	
	animState = mSceneMgr->createAnimationState("an2");
	animState->setEnabled(true);
	mAnimStateList.push_back(animState);
	
	trail->addNode(animNode);
	trail->setInitialColour(1, 0.0, 1.0, 0.4);
	trail->setColourChange(1, 0.5, 0.5, 0.5, 0.5);
	trail->setInitialWidth(1, 5);
	
	
	// Add light
	l2 = mSceneMgr->createLight("l3");
	l2->setDiffuseColour(trail->getInitialColour(1));
	animNode->attachObject(l2);
	
	// Add billboard
	bbs = mSceneMgr->createBillboardSet("bb2", 1);
	bbs->createBillboard(Vector3::ZERO, trail->getInitialColour(1));
	bbs->setMaterialName("Examples/Flare");
	animNode->attachObject(bbs);
	
	
	mCamera->setPosition(0,0,500);
	//mSceneMgr->showBoundingBoxes(true);
	
}
