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

PlayPen_SkeletalAnimation::PlayPen_SkeletalAnimation()
{
	mInfo["Title"] = "PlayPen_SkeletalAnimation";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(250);
}
//----------------------------------------------------------------------------

void PlayPen_SkeletalAnimation::setupContent()
{
	// Set ambient light
	mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));
	//mWindow->getViewport(0)->setBackgroundColour(ColourValue::White);
	
	
	
	Entity *ent = mSceneMgr->createEntity("robot", "robot.mesh");
	//ent->setDisplaySkeleton(true);
	// Uncomment the below to test software skinning
	ent->setMaterialName("Examples/Rocky");
	// Add entity to the scene node
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ent);
	Ogre::AnimationState* mAnimState = ent->getAnimationState("Walk");
	mAnimState->setEnabled(true);
	
	// Give it a little ambience with lights
	Light* l;
	l = mSceneMgr->createLight("BlueLight");
	l->setPosition(-200,-80,-100);
	l->setDiffuseColour(0.5, 0.5, 1.0);
	
	l = mSceneMgr->createLight("GreenLight");
	l->setPosition(0,0,-100);
	l->setDiffuseColour(0.5, 1.0, 0.5);
	
	// Position the camera
	mCamera->setPosition(200,50,0);
	mCamera->lookAt(0,50,0);
	
	// Report whether hardware skinning is enabled or not
	/*Technique* t = ent->getSubEntity(0)->getTechnique();
	Pass* p = t->getPass(0);
	OverlayElement* guiDbg = OverlayManager::getSingleton().getOverlayElement("Core/DebugText");
	if (p->hasVertexProgram() && 
	p->getVertexProgram()->isSkeletalAnimationIncluded())
	{
		guiDbg->setCaption("Hardware skinning is enabled");
	}
	else
	{
		guiDbg->setCaption("Software skinning is enabled");
	}*/
	
	mAnimStateList.push_back(mAnimState);
}
