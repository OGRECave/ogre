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

PlayPen_AttachObjectsToBones::PlayPen_AttachObjectsToBones()
{
	mInfo["Title"] = "PlayPen_AttachObjectsToBones";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(120);
}
//----------------------------------------------------------------------------

void PlayPen_AttachObjectsToBones::setupContent()
{
	Entity *ent;
	for (int i = 0; i < 12; ++i)
	{
		ent = mSceneMgr->createEntity("robot" + StringConverter::toString(i), "robot.mesh");
		if (i % 2)
		{
			Entity* ent2 = mSceneMgr->createEntity("plane" + StringConverter::toString(i), "razor.mesh");
			ent->attachObjectToBone("Joint8", ent2);
		}
		else
		{
			ParticleSystem* psys = mSceneMgr->createParticleSystem("psys" + StringConverter::toString(i), "Examples/PurpleFountain");
			psys->getEmitter(0)->setTimeToLive(0.2);
			ent->attachObjectToBone("Joint15", psys);
		}
		// Add entity to the scene node
		mSceneMgr->getRootSceneNode()->createChildSceneNode(
		Vector3(0,0,(i*200)-(12*200/2)))->attachObject(ent);
		
		ent->getParentNode()->yaw(Degree(i * 45));

		AnimationState* animState = ent->getAnimationState("Walk");
		animState->setEnabled(true);
		mAnimStateList.push_back(animState);
	}
	
	
	
	// Give it a little ambience with lights
	Light* l;
	l = mSceneMgr->createLight("BlueLight");
	l->setPosition(-200,-80,-100);
	l->setDiffuseColour(0.5, 0.5, 1.0);
	
	l = mSceneMgr->createLight("GreenLight");
	l->setPosition(0,0,-100);
	l->setDiffuseColour(0.5, 1.0, 0.5);
	
	// Position the camera
	mCamera->setPosition(400,120,500);
	mCamera->lookAt(-50,50,0);
	
	mSceneMgr->setAmbientLight(ColourValue(1,1,1,1));
	//mSceneMgr->showBoundingBoxes(true);
	
}
