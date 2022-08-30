#ifndef __CameraTrack_H__
#define __CameraTrack_H__

#include "SdkSample.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_CameraTrack : public SdkSample
{
public:

    Sample_CameraTrack()
    {
        mInfo["Title"] = "Camera Tracking";
        mInfo["Description"] = "An example of using AnimationTracks to make a node smoothly follow "
            "a predefined path with spline interpolation. Also uses the auto-tracking feature of the camera.";
        mInfo["Thumbnail"] = "thumb_camtrack.png";
        mInfo["Category"] = "Unsorted";
    }

protected:

    void setupContent() override
    {
        // setup some basic lighting for our scene
        mSceneMgr->setAmbientLight(ColourValue(0.3, 0.3, 0.3));
        mSceneMgr->getRootSceneNode()
            ->createChildSceneNode(Vector3(20, 80, 50))
            ->attachObject(mSceneMgr->createLight());
        
        mSceneMgr->setSkyBox(true, "Examples/MorningSkyBox");

        // create an ogre head entity and attach it to a node
        Entity* head = mSceneMgr->createEntity("Head", "ogrehead.mesh");
        SceneNode* headNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        headNode->attachObject(head);

        mCameraMan->setStyle(CS_MANUAL);   // we will be controlling the camera ourselves, so disable the camera man
        mCameraNode->setAutoTracking(true, headNode);   // make the camera face the head
        mCameraNode->setFixedYawAxis(true);
        
        // set up a 10 second animation for our camera, using spline interpolation for nice curves
        Animation* anim = mSceneMgr->createAnimation("CameraTrack", 10);
        anim->setInterpolationMode(Animation::IM_SPLINE);

        // create a track to animate the camera's node
        NodeAnimationTrack* track = anim->createNodeTrack(0, mCameraNode);

        // create keyframes for our track
        track->createNodeKeyFrame(0)->setTranslate(Vector3(200, 0, 0));
        track->createNodeKeyFrame(2.5)->setTranslate(Vector3(0, -50, 100));
        track->createNodeKeyFrame(5)->setTranslate(Vector3(-500, 100, 0));
        track->createNodeKeyFrame(7.5)->setTranslate(Vector3(0, 200, -300));
        track->createNodeKeyFrame(10)->setTranslate(Vector3(200, 0, 0));

        // create a new animation state to track this
        auto animState = mSceneMgr->createAnimationState("CameraTrack");
        animState->setEnabled(true);

        auto& controllerMgr = ControllerManager::getSingleton();
        controllerMgr.createFrameTimePassthroughController(AnimationStateControllerValue::create(animState, true));
    }
};

#endif
