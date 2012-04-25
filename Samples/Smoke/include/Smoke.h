#ifndef __Smoke_H__
#define __Smoke_H__

#include "SdkSample.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_Smoke : public SdkSample
{
public:

	Sample_Smoke()
	{
		mInfo["Title"] = "Smoke";
		mInfo["Description"] = "Demonstrates depth-sorting of particles in particle systems.";
		mInfo["Thumbnail"] = "thumb_smoke.png";
		mInfo["Category"] = "Effects";
	}

	bool frameRenderingQueued(const FrameEvent& evt)
	{
		// spin the head around and make it float up and down
		mPivot->setPosition(0, Math::Sin(mRoot->getTimer()->getMilliseconds() / 150.0) * 10, 0);
		mPivot->yaw(Radian(-evt.timeSinceLastFrame * 1.5));
		return SdkSample::frameRenderingQueued(evt);
	}

protected:

	void setupContent()
	{     

		mSceneMgr->setSkyBox(true, "Examples/EveningSkyBox");

		// dim orange ambient and two bright orange lights to match the skybox
		mSceneMgr->setAmbientLight(ColourValue(0.3, 0.2, 0));
		Light* light = mSceneMgr->createLight();
		light->setPosition(2000, 1000, -1000);
		light->setDiffuseColour(1, 0.5, 0);
		light = mSceneMgr->createLight();
        light->setPosition(-2000, 1000, 1000);
		light->setDiffuseColour(1, 0.5, 0);

		mPivot = mSceneMgr->getRootSceneNode()->createChildSceneNode();  // create a pivot node

		// create a child node and attach an ogre head and some smoke to it
		SceneNode* headNode = mPivot->createChildSceneNode(Vector3(100, 0, 0));
		headNode->attachObject(mSceneMgr->createEntity("Head", "ogrehead.mesh"));
        headNode->attachObject(mSceneMgr->createParticleSystem("Smoke", "Examples/Smoke"));

		mCamera->setPosition(0, 30, 350);
	}

	SceneNode* mPivot;
};

#endif
