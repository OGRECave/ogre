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

    bool frameRenderingQueued(const FrameEvent& evt) override
    {
        mPivot->yaw(Radian(evt.timeSinceLastFrame));      // spin the fishy around the cube mapped one
        mFishSwim->addTime(evt.timeSinceLastFrame * 3);   // make the fishy swim
        return SdkSample::frameRenderingQueued(evt);      // don't forget the parent updates!
    }

protected:

    void setupContent() override
    {
        mSceneMgr->setSkyDome(true, "Examples/CloudySky");

        // setup some basic lighting for our scene
        mSceneMgr->setAmbientLight(ColourValue(0.3, 0.3, 0.3));
        mSceneMgr->getRootSceneNode()
            ->createChildSceneNode(Vector3(20, 80, 50))
            ->attachObject(mSceneMgr->createLight());

        createCubeMap();

        // create an ogre head, give it the dynamic cube map material, and place it at the origin
        mHead = mSceneMgr->createEntity("CubeMappedHead", "ogrehead.mesh");
        mHead->setMaterialName("Examples/DynamicCubeMap");
        mHead->setVisibilityFlags(0xF); // hide from reflection
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
        // use compositors for easy referencing in material
        CompositorManager::getSingleton().addCompositor(mViewport, "CubeMap");
        CompositorManager::getSingleton().setCompositorEnabled(mViewport, "CubeMap", true);

        // create the camera used to render to our cubemap
        Camera* cubeCamera = mSceneMgr->createCamera("CubeMapCamera");
        cubeCamera->setFOVy(Degree(90));
        cubeCamera->setAspectRatio(1);
        cubeCamera->setNearClipDistance(5);

        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(cubeCamera);
    }

    void cleanupContent() override
    {
        MeshManager::getSingleton().remove("floor", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    }

    Entity* mHead;
    SceneNode* mPivot;
    AnimationState* mFishSwim;
};

#endif
