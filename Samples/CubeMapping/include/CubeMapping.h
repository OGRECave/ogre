#ifndef __CubeMapping_H__
#define __CubeMapping_H__

#include "SdkSample.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_CubeMapping : public SdkSample, public RenderTargetListener
{
public:

	Sample_CubeMapping()
	{
		mInfo["Title"] = "Cube Mapping";
		mInfo["Description"] = "Demonstrates the cube mapping feature where a wrap-around environment is reflected "
			"off of an object. Uses render-to-texture to create dynamic cubemaps.";
		mInfo["Thumbnail"] = "thumb_cubemap.png";
		mInfo["Category"] = "Unsorted";
	}

	void testCapabilities(const RenderSystemCapabilities* caps)
	{
        if (!caps->hasCapability(RSC_CUBEMAPPING))
        {
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your graphics card does not support cube mapping, "
				"so you cannot run this sample. Sorry!", "CubeMappingSample::testCapabilities");
        }
	}

    bool frameRenderingQueued(const FrameEvent& evt)
    {
		mPivot->yaw(Radian(evt.timeSinceLastFrame));      // spin the fishy around the cube mapped one
		mFishSwim->addTime(evt.timeSinceLastFrame * 3);   // make the fishy swim
		return SdkSample::frameRenderingQueued(evt);      // don't forget the parent updates!
    }

    void preRenderTargetUpdate(const RenderTargetEvent& evt)
    {
        mHead->setVisible(false);  // hide the head

		// point the camera in the right direction based on which face of the cubemap this is
		mCubeCamera->setOrientation(Quaternion::IDENTITY);
		if (evt.source == mTargets[0]) mCubeCamera->yaw(Degree(-90));
		else if (evt.source == mTargets[1]) mCubeCamera->yaw(Degree(90));
		else if (evt.source == mTargets[2]) mCubeCamera->pitch(Degree(90));
		else if (evt.source == mTargets[3]) mCubeCamera->pitch(Degree(-90));
		else if (evt.source == mTargets[5]) mCubeCamera->yaw(Degree(180));
    }

    void postRenderTargetUpdate(const RenderTargetEvent& evt)
    {
        mHead->setVisible(true);  // unhide the head
    }

protected:

	void setupContent()
	{
		mSceneMgr->setSkyDome(true, "Examples/CloudySky");

		// setup some basic lighting for our scene
        mSceneMgr->setAmbientLight(ColourValue(0.3, 0.3, 0.3));
        mSceneMgr->createLight()->setPosition(20, 80, 50);

		createCubeMap();

		// create an ogre head, give it the dynamic cube map material, and place it at the origin
		mHead = mSceneMgr->createEntity("CubeMappedHead", "ogrehead.mesh");
		mHead->setMaterialName("Examples/DynamicCubeMap");
		mSceneMgr->getRootSceneNode()->attachObject(mHead);

		mPivot = mSceneMgr->getRootSceneNode()->createChildSceneNode();  // create a pivot node

		Entity* fish = mSceneMgr->createEntity("Fish", "fish.mesh");
		mFishSwim = fish->getAnimationState("swim");
		mFishSwim->setEnabled(true);

		// create a child node at an offset and attach a regular ogre head and a nimbus to it
		SceneNode* node = mPivot->createChildSceneNode(Vector3(-60, 10, 0));
		node->setScale(7, 7, 7);
		node->yaw(Degree(90));
		node->attachObject(fish);

		// create a floor mesh resource
		MeshManager::getSingleton().createPlane("floor", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			Plane(Vector3::UNIT_Y, -30), 1000, 1000, 10, 10, true, 1, 8, 8, Vector3::UNIT_Z);

		// create a floor entity, give it a material, and place it at the origin
        Entity* floor = mSceneMgr->createEntity("Floor", "floor");
        floor->setMaterialName("Examples/BumpyMetal");
        mSceneMgr->getRootSceneNode()->attachObject(floor);

		// set our camera to orbit around the head and show cursor
		mCameraMan->setStyle(CS_ORBIT);
		mTrayMgr->showCursor();
	}

	void createCubeMap()
	{
		// create the camera used to render to our cubemap
		mCubeCamera = mSceneMgr->createCamera("CubeMapCamera");
		mCubeCamera->setFOVy(Degree(90));
		mCubeCamera->setAspectRatio(1);
		mCubeCamera->setFixedYawAxis(false);
		mCubeCamera->setNearClipDistance(5);

		// create our dynamic cube map texture
		TexturePtr tex = TextureManager::getSingleton().createManual("dyncubemap",
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_CUBE_MAP, 128, 128, 0, PF_R8G8B8, TU_RENDERTARGET);

		// assign our camera to all 6 render targets of the texture (1 for each direction)
		for (unsigned int i = 0; i < 6; i++)
		{
			mTargets[i] = tex->getBuffer(i)->getRenderTarget();
			mTargets[i]->addViewport(mCubeCamera)->setOverlaysEnabled(false);
			mTargets[i]->addListener(this);
		}
	}

	void cleanupContent()
	{
		MeshManager::getSingleton().remove("floor");
		TextureManager::getSingleton().remove("dyncubemap");
	}

	Entity* mHead;
	Camera* mCubeCamera;
	RenderTarget* mTargets[6];
	SceneNode* mPivot;
	AnimationState* mFishSwim;
};

#endif
