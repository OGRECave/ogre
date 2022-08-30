#ifndef __Smoke_H__
#define __Smoke_H__

#include "SdkSample.h"
#include "OgreParticleSystem.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_Smoke : public SdkSample
{
public:

    Sample_Smoke()
    {
        mInfo["Title"] = "Smoke";
        mInfo["Description"] = "Demonstrates depth-sorting and texture-animation of particles.";
        mInfo["Thumbnail"] = "thumb_smoke.png";
        mInfo["Category"] = "Effects";
    }

    bool frameRenderingQueued(const FrameEvent& evt) override
    {
        // spin the head around and make it float up and down
        mPivot->setPosition(0, Math::Sin(mRoot->getTimer()->getMilliseconds() / 150.0) * 10, 0);
        mPivot->yaw(Radian(-evt.timeSinceLastFrame * 1.5));
        return SdkSample::frameRenderingQueued(evt);
    }

protected:

    void setupContent() override
    {     

        mSceneMgr->setSkyBox(true, "Examples/EveningSkyBox");

        // dim orange ambient and two bright orange lights to match the skybox
        mSceneMgr->setAmbientLight(ColourValue(0.3, 0.2, 0));
        Light* light = mSceneMgr->createLight();
        mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(2000, 1000, -1000))->attachObject(light);
        light->setDiffuseColour(1, 0.5, 0);
        light = mSceneMgr->createLight();
        mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(-2000, 1000, 1000))->attachObject(light);
        light->setDiffuseColour(1, 0.5, 0);

        mPivot = mSceneMgr->getRootSceneNode()->createChildSceneNode();  // create a pivot node

        // create a child node and attach an ogre head and some smoke to it
        SceneNode* headNode = mPivot->createChildSceneNode(Vector3(100, 0, 0));
        headNode->attachObject(mSceneMgr->createEntity("Head", "ogrehead.mesh"));
        headNode->attachObject(mSceneMgr->createParticleSystem("Smoke", "Examples/Smoke"));

        mCameraNode->setPosition(0, 30, 350);
    }

    SceneNode* mPivot;
};

#endif
