#ifndef __Lighting_H__
#define __Lighting_H__

#include "SdkSample.h"
#include "OgreBillboard.h"

using namespace Ogre;
using namespace OgreBites;

struct OcclusionQueryActivator : public RenderObjectListener
{
    HardwareOcclusionQuery* mActiveQuery = NULL;

    std::map<Renderable*, HardwareOcclusionQuery*> mQueryMap;

    // Event raised when render single object started.
    void notifyRenderSingleObject(Renderable* rend, const Pass* pass, const AutoParamDataSource* source,
            const LightList* pLightList, bool suppressRenderStateChanges) override
    {
        //
        // The following code activates and deactivates the occlusion queries
        // so that the queries only include the rendering of their intended targets
        //

        // Close the last occlusion query
        // Each occlusion query should only last a single rendering
        if (mActiveQuery)
        {
            mActiveQuery->end();
            mActiveQuery = NULL;
        }

        // Open a new occlusion query

        // Check if a the object being rendered needs
        // to be occlusion queried, and by which query instance.
        auto it = mQueryMap.find(rend);
        if(it == mQueryMap.end())
            return;

        // Stop occlusion query until we get the information
        // (may not happen on the same frame they are requested in)
        if(!it->second->resultReady())
            return;

        mActiveQuery = it->second;
        mActiveQuery->begin();
    }
};

class _OgreSampleClassExport Sample_Lighting : public SdkSample
{
    static const uint8 cPriorityMain = 50;
    static const uint8 cPriorityQuery = 51;
    static const uint8 cPriorityLights = 55;

public:

    Sample_Lighting() :
        mLight1BBFlare(NULL),
        mLight1BBQueryArea(NULL),
        mLight1BBQueryVisible(NULL),
        mLight2BBFlare(NULL),
        mLight2BBQueryArea(NULL),
        mLight2BBQueryVisible(NULL),
        mLight1QueryArea(NULL),
        mLight1QueryVisible(NULL),
        mLight2QueryArea(NULL),
        mLight2QueryVisible(NULL)
    {
        mInfo["Title"] = "Lighting";
        mInfo["Description"] = "Shows OGRE's lighting support. Also demonstrates "
            "usage of occlusion queries and automatic time-relative behaviour "
            "using billboards and controllers.";
        mInfo["Thumbnail"] = "thumb_lighting.png";
        mInfo["Category"] = "Lighting";
    }

    bool frameRenderingQueued(const FrameEvent& evt) override
    {
        if (mLight1QueryArea)
        {
            // Modulate the lights according to the query data
            unsigned int lightAreaCount;
            unsigned int lightVisibleCount;
            float ratio;
            
            lightAreaCount = mLight1QueryArea->getLastResult();
            lightVisibleCount = mLight1QueryVisible->getLastResult();
            ratio = float(lightVisibleCount) / float(lightAreaCount);
            mLight1BBFlare->setColour(mTrail->getInitialColour(0) * ratio);

            lightAreaCount = mLight2QueryArea->getLastResult();
            lightVisibleCount = mLight2QueryVisible->getLastResult();
            ratio = float(lightVisibleCount) / float(lightAreaCount);
            mLight2BBFlare->setColour(mTrail->getInitialColour(1) * ratio);
        }

        return SdkSample::frameRenderingQueued(evt);   // don't forget the parent class updates!
    }

protected:

    void setupContent() override
    {
        // Set our camera to orbit around the origin at a suitable distance
        mCameraMan->setStyle(CS_ORBIT);
        mCameraMan->setYawPitchDist(Radian(0), Radian(0), 400);

        mTrayMgr->showCursor();

        // Create an ogre head and place it at the origin
        Entity* head = mSceneMgr->createEntity("Head", "ogrehead.mesh");
        head->setRenderQueueGroup(cPriorityMain);
        mSceneMgr->getRootSceneNode()->attachObject(head);

        setupLights();
    }

    void setupLights()
    {
        
        mSceneMgr->setAmbientLight(ColourValue(0.1, 0.1, 0.1));  // Dim ambient lighting

        // Create a ribbon trail that our lights will leave behind
        NameValuePairList params;
        params["numberOfChains"] = "2";
        params["maxElements"] = "80";
        mTrail = (RibbonTrail*)mSceneMgr->createMovableObject("RibbonTrail", &params);
        mSceneMgr->getRootSceneNode()->attachObject(mTrail);
        mTrail->setMaterialName("Examples/LightRibbonTrail");
        mTrail->setTrailLength(400);
        mTrail->setRenderQueueGroup(cPriorityLights);
        
        // Create the occlusion queries to be used in this sample
        RenderSystem* renderSystem = Ogre::Root::getSingleton().getRenderSystem();
        mLight1QueryArea = renderSystem->createHardwareOcclusionQuery();
        mLight1QueryVisible = renderSystem->createHardwareOcclusionQuery();
        mLight2QueryArea = renderSystem->createHardwareOcclusionQuery();
        mLight2QueryVisible = renderSystem->createHardwareOcclusionQuery();

        if (!mLight1QueryArea)
        {
            LogManager::getSingleton().logWarning("Sample_Lighting - hardware occlusion query not available");
        }
        
        // Create the materials to be used by the objects used fo the occlusion query
        MaterialPtr matBase = MaterialManager::getSingleton().getDefaultMaterial(false);
        MaterialPtr matQueryArea = matBase->clone("QueryArea");
        matQueryArea->setDepthWriteEnabled(false);
        matQueryArea->setColourWriteEnabled(false);
        matQueryArea->setDepthCheckEnabled(false); // Not occluded by objects
        MaterialPtr matQueryVisible = matBase->clone("QueryVisible");
        matQueryVisible->setDepthWriteEnabled(false);
        matQueryVisible->setColourWriteEnabled(false);
        matQueryVisible->setDepthCheckEnabled(true); // Occluded by objects

        SceneNode* node;
        Animation* anim;
        NodeAnimationTrack* track;
        Light* light;
        BillboardSet* bbs;
        
        // Create a light node
        node = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(50, 30, 0));

        // Create a 14 second animation with spline interpolation
        anim = mSceneMgr->createAnimation("Path1", 14);
        anim->setInterpolationMode(Animation::IM_SPLINE);

        track = anim->createNodeTrack(1, node);  // Create a node track for our animation

        // Enter keyframes for our track to define a path for the light to follow
        track->createNodeKeyFrame(0)->setTranslate(Vector3(50, 30, 0));
        track->createNodeKeyFrame(2)->setTranslate(Vector3(100, -30, 0));
        track->createNodeKeyFrame(4)->setTranslate(Vector3(120, -80, 150));
        track->createNodeKeyFrame(6)->setTranslate(Vector3(30, -80, 50));
        track->createNodeKeyFrame(8)->setTranslate(Vector3(-50, 30, -50));
        track->createNodeKeyFrame(10)->setTranslate(Vector3(-150, -20, -100));
        track->createNodeKeyFrame(12)->setTranslate(Vector3(-50, -30, 0));
        track->createNodeKeyFrame(14)->setTranslate(Vector3(50, 30, 0));

        auto& controllerMgr = ControllerManager::getSingleton();
        // Create an animation state from the animation and enable it
        auto animState = mSceneMgr->createAnimationState("Path1");
        animState->setEnabled(true);
        controllerMgr.createFrameTimePassthroughController(AnimationStateControllerValue::create(animState, true));

        // Set initial settings for the ribbon mTrail and add the light node
        mTrail->setInitialColour(0, 1.0, 0.8, 0);
        mTrail->setColourChange(0, 0.5, 0.5, 0.5, 0.5);
        mTrail->setInitialWidth(0, 5);
        mTrail->addNode(node);
        

        // Attach a light with the same colour to the light node
        light = mSceneMgr->createLight();
        light->setDiffuseColour(mTrail->getInitialColour(0));
        node->attachObject(light);

        // Attach a flare with the same colour to the light node
        bbs = mSceneMgr->createBillboardSet(1);
        mLight1BBFlare = bbs->createBillboard(Vector3::ZERO, mTrail->getInitialColour(0));
        bbs->setMaterialName("Examples/Flare");
        bbs->setRenderQueueGroup(cPriorityLights);
        node->attachObject(bbs);

        if (mLight1QueryArea)
        {
            // Attach a billboard which will be used to get a relative area occupied by the light
            mLight1BBQueryArea = mSceneMgr->createBillboardSet(1);
            mLight1BBQueryArea->setDefaultDimensions(10,10);
            mLight1BBQueryArea->createBillboard(Vector3::ZERO);
            mLight1BBQueryArea->setMaterial(matQueryArea);
            mLight1BBQueryArea->setRenderQueueGroup(cPriorityQuery);
            node->attachObject(mLight1BBQueryArea);

            // Attach a billboard which will be used to get the visible area occupied by the light
            mLight1BBQueryVisible = mSceneMgr->createBillboardSet(1);
            mLight1BBQueryVisible->setDefaultDimensions(10,10);
            mLight1BBQueryVisible->createBillboard(Vector3::ZERO);
            mLight1BBQueryVisible->setMaterial(matQueryVisible);
            mLight1BBQueryVisible->setRenderQueueGroup(cPriorityQuery);
            node->attachObject(mLight1BBQueryVisible);
        }

        // Create a second light node
        node = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(-50, 100, 0));

        // Create a 10 second animation with spline interpolation
        anim = mSceneMgr->createAnimation("Path2", 10);
        anim->setInterpolationMode(Animation::IM_SPLINE);

        track = anim->createNodeTrack(1, node);  // Create a node track for our animation

        // Enter keyframes for our track to define a path for the light to follow
        track->createNodeKeyFrame(0)->setTranslate(Vector3(-50, 100, 0));
        track->createNodeKeyFrame(2)->setTranslate(Vector3(-100, 150, -30));
        track->createNodeKeyFrame(4)->setTranslate(Vector3(-200, 0, 40));
        track->createNodeKeyFrame(6)->setTranslate(Vector3(0, -150, 70));
        track->createNodeKeyFrame(8)->setTranslate(Vector3(50, 0, 30));
        track->createNodeKeyFrame(10)->setTranslate(Vector3(-50, 100, 0));

        // Create an animation state from the animation and enable it
        animState = mSceneMgr->createAnimationState("Path2");
        animState->setEnabled(true);
        controllerMgr.createFrameTimePassthroughController(AnimationStateControllerValue::create(animState, true));

        // Set initial settings for the ribbon mTrail and add the light node
        mTrail->setInitialColour(1, 0.0, 1.0, 0.4);
        mTrail->setColourChange(1, 0.5, 0.5, 0.5, 0.5);
        mTrail->setInitialWidth(1, 5);
        mTrail->addNode(node);

        // Attach a light with the same colour to the light node
        light = mSceneMgr->createLight();
        light->setDiffuseColour(mTrail->getInitialColour(1));
        node->attachObject(light);

        // Attach a flare with the same colour to the light node
        bbs = mSceneMgr->createBillboardSet(1);
        mLight2BBFlare = bbs->createBillboard(Vector3::ZERO, mTrail->getInitialColour(1));
        bbs->setMaterialName("Examples/Flare");
        bbs->setRenderQueueGroup(cPriorityLights);
        node->attachObject(bbs);
        
        if (mLight1QueryArea)
        {
            // Attach a billboard which will be used to get a relative area occupied by the light
            mLight2BBQueryArea = mSceneMgr->createBillboardSet(1);
            mLight2BBQueryArea->setDefaultDimensions(10,10);
            mLight2BBQueryArea->createBillboard(Vector3::ZERO);
            mLight2BBQueryArea->setMaterial(matQueryArea);
            mLight2BBQueryArea->setRenderQueueGroup(cPriorityQuery);
            node->attachObject(mLight2BBQueryArea);

            // Attach a billboard which will be used to get the visible area occupied by the light
            mLight2BBQueryVisible = mSceneMgr->createBillboardSet(1);
            mLight2BBQueryVisible->setDefaultDimensions(10,10);
            mLight2BBQueryVisible->createBillboard(Vector3::ZERO);
            mLight2BBQueryVisible->setMaterial(matQueryVisible);
            mLight2BBQueryVisible->setRenderQueueGroup(cPriorityQuery);
            node->attachObject(mLight2BBQueryVisible);

            mOcclusionQueryActivator.mQueryMap[mLight2BBQueryArea] = mLight2QueryArea;
            mOcclusionQueryActivator.mQueryMap[mLight2BBQueryVisible] = mLight2QueryVisible;
            mOcclusionQueryActivator.mQueryMap[mLight1BBQueryArea] = mLight1QueryArea;
            mOcclusionQueryActivator.mQueryMap[mLight1BBQueryVisible] = mLight1QueryVisible;
            // Setup the listener for the occlusion query
            mSceneMgr->addRenderObjectListener(&mOcclusionQueryActivator);
        }
    }

    void cleanupContent() override
    {
        RenderSystem* renderSystem = Ogre::Root::getSingleton().getRenderSystem();
        for (const auto& it : mOcclusionQueryActivator.mQueryMap)
        {
            renderSystem->destroyHardwareOcclusionQuery(it.second);
        }
    }

    RibbonTrail* mTrail;

    Billboard* mLight1BBFlare;
    BillboardSet* mLight1BBQueryArea;
    BillboardSet* mLight1BBQueryVisible;
    Billboard* mLight2BBFlare;
    BillboardSet* mLight2BBQueryArea;
    BillboardSet* mLight2BBQueryVisible;

    HardwareOcclusionQuery* mLight1QueryArea;
    HardwareOcclusionQuery* mLight1QueryVisible;
    HardwareOcclusionQuery* mLight2QueryArea;
    HardwareOcclusionQuery* mLight2QueryVisible;

    OcclusionQueryActivator mOcclusionQueryActivator;

    bool mUseOcclusionQuery;
};

#endif
