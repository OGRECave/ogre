#ifndef __BSP_H__
#define __BSP_H__

#include "SdkSample.h"
#include "OgreFileSystemLayer.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_BSP : public SdkSample
{
 public:

    Sample_BSP()
    {
        mInfo["Title"] = "BSP";
        mInfo["Description"] = "A demo of the indoor, or BSP (Binary Space Partition) scene manager. "
            "Also demonstrates how to load BSP maps from Quake 3.";
        mInfo["Thumbnail"] = "thumb_bsp.png";
        mInfo["Category"] = "Geometry";
    }

    StringVector getRequiredPlugins() override
    {
        StringVector names;
        names.push_back("BSP Scene Manager");
        return names;
    }

 protected:
    void createSceneManager() override
    {
        mSceneMgr = mRoot->createSceneManager("BspSceneManager");   // the BSP scene manager is required for this sample
#ifdef INCLUDE_RTSHADER_SYSTEM
        mShaderGenerator->addSceneManager(mSceneMgr);
#endif
        if(auto overlaySystem = mContext->getOverlaySystem())
            mSceneMgr->addRenderQueueListener(overlaySystem);
    }

    void loadResources()
    {
        /* NOTE: The browser initialises everything at the beginning already, so we use a 0 init proportion.
           If you're not compiling this sample for use with the browser, then leave the init proportion at 0.7. */
        mTrayMgr->showLoadingBar(1, 1, 0);

        // associate the world geometry with the world resource group, and then load the group
        ResourceGroupManager& rgm = ResourceGroupManager::getSingleton();

        // Pick a new resource group so Q3Shader parser is correctly registered
        rgm.setWorldResourceGroupName("BSPWorld");

        rgm.setCustomStagesForResourceGroup("BSPWorld", mSceneMgr->estimateWorldGeometry("maps/oa_rpg3dm2.bsp"));
        rgm.initialiseResourceGroup("BSPWorld");
        rgm.loadResourceGroup("BSPWorld");
        // one would register a ResourceGroupListener for this, if we were not to call it right away
        mSceneMgr->setWorldGeometry("maps/oa_rpg3dm2.bsp");

        mTrayMgr->hideLoadingBar();
    }

    void unloadResources() override
    {
        // unload the map so we don't interfere with subsequent samples
        ResourceGroupManager& rgm = ResourceGroupManager::getSingleton();
        rgm.clearResourceGroup(rgm.getWorldResourceGroupName());
        rgm.setWorldResourceGroupName(ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    }

    void setupContent() override
    {
        loadResources();

        // modify camera for close work
        mCamera->setNearClipDistance(4);
        mCamera->setFarClipDistance(4000);

        // Quake uses the Z axis as the up axis, so make necessary adjustments
        mCameraNode->setFixedYawAxis(true, Vector3::UNIT_Z);
        mCameraNode->pitch(Degree(90));

        // specific for this map
        mCameraNode->setPosition(Vector3(0, 0, 340));

        mCameraMan->setTopSpeed(350);   // make the camera move a bit faster
    }
};

#endif
