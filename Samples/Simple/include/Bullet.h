#pragma once

#include "SdkSample.h"
#include "OgreBullet.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_Bullet : public SdkSample, public RenderTargetListener
{
    std::unique_ptr<Bullet::DynamicsWorld> mDynWorld;
    std::unique_ptr<Bullet::DebugDrawer> mDbgDraw;

public:
    Sample_Bullet()
    {
        mInfo["Title"] = "Bullet physics integration";
        mInfo["Description"] = "Bullet with Ogre";
        mInfo["Category"] = "Unsorted";
        mInfo["Thumbnail"] = "thumb_bullet.png";
    }

    bool frameStarted(const FrameEvent& evt) override
    {
        mDynWorld->getBtWorld()->stepSimulation(evt.timeSinceLastFrame, 10);
        mDbgDraw->update();

        return true;
    }

    void setupContent(void) override
    {
        mCameraMan->setStyle(OgreBites::CS_ORBIT);
        mCameraMan->setYawPitchDist(Degree(45), Degree(45), 20);

        // without light we would just get a black screen
        Light* light = mSceneMgr->createLight("MainLight");
        mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(0, 10, 15))->attachObject(light);
        mSceneMgr->setAmbientLight(ColourValue(0.7, 0.7, 0.7));

        mCamera->setNearClipDistance(0.05);
        mTrayMgr->showCursor();

        // Player object
        auto player = mSceneMgr->createEntity("ogrehead.mesh");
        auto playerNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(0, 10, 0));
        playerNode->setScale(1.0/50, 1.0/50, 1.0/50);
        playerNode->attachObject(player);

        // Ground
        auto level = mSceneMgr->createEntity("TestLevel_b0.mesh");
        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(level);

        // Physics world and debug drawing
        mDynWorld.reset(new Bullet::DynamicsWorld(Vector3(0, -9.8, 0)));
        mDbgDraw.reset(new Bullet::DebugDrawer(mSceneMgr->getRootSceneNode(), mDynWorld->getBtWorld()));

        mDynWorld->addRigidBody(5, player, Bullet::CT_SPHERE);
        mDynWorld->addRigidBody(0, level, Bullet::CT_TRIMESH);
    }
};
