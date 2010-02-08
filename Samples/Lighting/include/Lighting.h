#ifndef __Lighting_H__
#define __Lighting_H__

#include "SdkSample.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_Lighting : public SdkSample
{
public:

	Sample_Lighting()
	{
		mInfo["Title"] = "Lighting";
		mInfo["Description"] = "Shows OGRE's lighting support. Also demonstrates automatic "
			"time-relative behaviour using billboards and controllers.";
		mInfo["Thumbnail"] = "thumb_lighting.png";
		mInfo["Category"] = "Lighting";
	}

    bool frameRenderingQueued(const FrameEvent& evt)
    {
		// move the lights along their paths
		mGreenLightAnimState->addTime(evt.timeSinceLastFrame);
		mYellowLightAnimState->addTime(evt.timeSinceLastFrame);

		return SdkSample::frameRenderingQueued(evt);   // don't forget the parent class updates!
    }

protected:

    void testCapabilities( const RenderSystemCapabilities* caps )
    {
        if (Ogre::Root::getSingletonPtr()->getRenderSystem()->getName().find("OpenGL ES") != String::npos)
        {
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "This sample uses 1D textures which are not supported in OpenGL ES, "
                "so you cannot run this sample. Sorry!", "Sample_Lighting::testCapabilities");
        }
    }

	void setupContent()
	{
		// set our camera to orbit around the origin at a suitable distance
		mCameraMan->setStyle(CS_ORBIT);
		mCameraMan->setYawPitchDist(Radian(0), Radian(0), 400);

		mTrayMgr->showCursor();

		// create an ogre head and place it at the origin
		Entity* head = mSceneMgr->createEntity("Head", "ogrehead.mesh");
		mSceneMgr->getRootSceneNode()->attachObject(head);

		setupLights();
	}

	void setupLights()
	{
		mSceneMgr->setAmbientLight(ColourValue(0.1, 0.1, 0.1));  // dim ambient lighting

		// create a ribbon trail that our lights will leave behind
		NameValuePairList params;
		params["numberOfChains"] = "2";
		params["maxElements"] = "80";
		RibbonTrail* trail = (RibbonTrail*)mSceneMgr->createMovableObject("RibbonTrail", &params);
		mSceneMgr->getRootSceneNode()->attachObject(trail);
		trail->setMaterialName("Examples/LightRibbonTrail");
		trail->setTrailLength(400);

		SceneNode* node;
		Animation* anim;
		NodeAnimationTrack* track;
		Light* light;
		BillboardSet* bbs;
		
		// create a light node
		node = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(50, 30, 0));

		// create a 14 second animation with spline interpolation
		anim = mSceneMgr->createAnimation("Path1", 14);
		anim->setInterpolationMode(Animation::IM_SPLINE);

		track = anim->createNodeTrack(1, node);  // create a node track for our animation

		// enter keyframes for our track to define a path for the light to follow
		track->createNodeKeyFrame(0)->setTranslate(Vector3(50, 30, 0));
		track->createNodeKeyFrame(2)->setTranslate(Vector3(100, -30, 0));
		track->createNodeKeyFrame(4)->setTranslate(Vector3(120, -80, 150));
		track->createNodeKeyFrame(6)->setTranslate(Vector3(30, -80, 50));
		track->createNodeKeyFrame(8)->setTranslate(Vector3(-50, 30, -50));
		track->createNodeKeyFrame(10)->setTranslate(Vector3(-150, -20, -100));
		track->createNodeKeyFrame(12)->setTranslate(Vector3(-50, -30, 0));
		track->createNodeKeyFrame(14)->setTranslate(Vector3(50, 30, 0));

		// create an animation state from the animation and enable it
		mYellowLightAnimState = mSceneMgr->createAnimationState("Path1");
		mYellowLightAnimState->setEnabled(true);

		// set initial settings for the ribbon trail and add the light node
		trail->setInitialColour(0, 1.0, 0.8, 0);
		trail->setColourChange(0, 0.5, 0.5, 0.5, 0.5);
		trail->setInitialWidth(0, 5);
		trail->addNode(node);

		// attach a light with the same colour to the light node
		light = mSceneMgr->createLight();
		light->setDiffuseColour(trail->getInitialColour(0));
		node->attachObject(light);

		// attach a flare with the same colour to the light node
		bbs = mSceneMgr->createBillboardSet(1);
		bbs->createBillboard(Vector3::ZERO, trail->getInitialColour(0));
		bbs->setMaterialName("Examples/Flare");
		node->attachObject(bbs);
		
		// create a second light node
		node = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(-50, 100, 0));

		// create a 10 second animation with spline interpolation
		anim = mSceneMgr->createAnimation("Path2", 10);
		anim->setInterpolationMode(Animation::IM_SPLINE);

		track = anim->createNodeTrack(1, node);  // create a node track for our animation

		// enter keyframes for our track to define a path for the light to follow
		track->createNodeKeyFrame(0)->setTranslate(Vector3(-50, 100, 0));
		track->createNodeKeyFrame(2)->setTranslate(Vector3(-100, 150, -30));
		track->createNodeKeyFrame(4)->setTranslate(Vector3(-200, 0, 40));
		track->createNodeKeyFrame(6)->setTranslate(Vector3(0, -150, 70));
		track->createNodeKeyFrame(8)->setTranslate(Vector3(50, 0, 30));
		track->createNodeKeyFrame(10)->setTranslate(Vector3(-50, 100, 0));

		// create an animation state from the animation and enable it
		mGreenLightAnimState = mSceneMgr->createAnimationState("Path2");
		mGreenLightAnimState->setEnabled(true);

		// set initial settings for the ribbon trail and add the light node
		trail->setInitialColour(1, 0.0, 1.0, 0.4);
		trail->setColourChange(1, 0.5, 0.5, 0.5, 0.5);
		trail->setInitialWidth(1, 5);
		trail->addNode(node);

		// attach a light with the same colour to the light node
		light = mSceneMgr->createLight();
		light->setDiffuseColour(trail->getInitialColour(1));
		node->attachObject(light);

		// attach a flare with the same colour to the light node
		bbs = mSceneMgr->createBillboardSet(1);
		bbs->createBillboard(Vector3::ZERO, trail->getInitialColour(1));
		bbs->setMaterialName("Examples/Flare");
		node->attachObject(bbs);
	}

	AnimationState* mGreenLightAnimState;
	AnimationState* mYellowLightAnimState;
};

#endif
