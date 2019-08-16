#pragma once

#include "SdkSample.h"
#include "OgreBillboard.h"

using namespace Ogre;
using namespace OgreBites;

// Main light billboard set
BillboardSet* mBillboardSet = 0;
// Camera used for rendering the shadow map and as light frustum
Camera* mLightCamera = 0;
// Light camera scene node
SceneNode* mLightCameraSN = 0;

// Knot scene node
SceneNode* mKnotSN = 0;

// Current cookie selected
int mCurrentCookie = 0;

class _OgreSampleClassExport Sample_LightShafts : public SdkSample
{
    bool mRotateEnable;
    bool mRotateKnot;

    MaterialPtr mLightShaftsMat;

public:
    // Basic constructor
    Sample_LightShafts() : mRotateEnable(false), mRotateKnot(true)
    {
        mInfo["Title"] = "Light Shafts";
        mInfo["Description"] = "Demonstrates volumetric light shafts";
        mInfo["Category"] = "Lighting";
        mInfo["Thumbnail"] = "thumb_lightshafts.png";
        mInfo["Help"] = "Controls:\n"
                        "C - Show/Hide light frustum\n"
                        "V - Enable/Disable light rotation\n"
                        "B - Enable/Disable knot rotation\n"
                        "N - Change light cookie";
    }

    StringVector getRequiredPlugins() { return {"Cg Program Manager"}; }

    bool frameStarted(const FrameEvent& e)
    {
        // Update light position
        updatePosition(e);

        return SdkSample::frameStarted(e);
    }

    bool keyPressed(const OgreBites::KeyboardEvent& evt)
    {
        switch (evt.keysym.sym)
        {
        case 'c':
            mLightCamera->setVisible(!mLightCamera->getVisible());
            break;
        case 'v':
            mRotateEnable = !mRotateEnable;
            break;
        case 'b':
            mRotateKnot = !mRotateKnot;
            break;
        case 'n':
            mCurrentCookie = (mCurrentCookie + 1) % 3;
            mLightShaftsMat->getTechnique(0)->getPass(0)->getTextureUnitState(1)->setTextureName(
                StringUtil::format("Cookie%d.png", mCurrentCookie));
            break;
        }
        return SdkSample::keyPressed(evt);
    }

    void updatePosition(const FrameEvent& e)
    {
        // Just a simple circular trajectory
        const Real& SimulationTime = Root::getSingleton().getTimer()->getMilliseconds();
        Real Radius = 8;

        if (!mRotateEnable)
        {
            Radius = 0;
        }

        mLightCameraSN->setPosition(Math::Sin(SimulationTime / 1000) * Radius,
                                    mLightCameraSN->getPosition().y,
                                    Math::Cos(SimulationTime / 1000) * Radius);

        // Set the the scene node direction to 0,KnotHeight,0 point
        Vector3 NormalisedDirection =
            (Vector3(0, mKnotSN->getPosition().y, 0) - mLightCameraSN->getPosition()).normalisedCopy();
        mLightCameraSN->setDirection(NormalisedDirection, Node::TS_WORLD);

        if (mRotateKnot)
        {
            mKnotSN->setOrientation(Quaternion(
                Degree(Root::getSingleton().getTimer()->getMilliseconds() / 50), Vector3(0, 1, 0)));
        }
    }

    void setupContent(void)
    {
        auto statusPanel = mTrayMgr->createParamsPanel(TL_TOPLEFT, "HelpMessage", 200, {"Help"});
        statusPanel->setParamValue("Help", "H / F1");

        mCameraMan->setStyle(OgreBites::CS_ORBIT);
        mCameraMan->setYawPitchDist(Degree(0), Degree(15), 30);

        mTrayMgr->showCursor();

        // Set some camera params
        mCamera->setNearClipDistance(1);

        // add a little ambient lighting
        mSceneMgr->setAmbientLight(ColourValue(0.1, 0.1, 0.1));

        // Set up light 0
        Light* mLight0 = mSceneMgr->createLight("Light0");
        mLight0->setType(Light::LT_SPOTLIGHT);
        mLight0->setDiffuseColour(0.9, 0.9, 0.9);
        mLight0->setSpecularColour(1, 1, 1);
        mLight0->setSpotlightRange(Degree(17.5f), Degree(22.5f));

        mLightShaftsMat = MaterialManager::getSingleton().getByName("LightShafts");

        // Set up our light camera

        mLightCameraSN = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        mLightCameraSN->setPosition(0, 20, 0);
        mLightCameraSN->setDirection(0, -1, 0);

        mLightCameraSN->attachObject(mLight0);

        // Create our billboardset for volumetric rendering
        mBillboardSet = mSceneMgr->createBillboardSet("LightBillboardSet", 1);
        mBillboardSet->setMaterial(mLightShaftsMat);
        mBillboardSet->setBillboardRotationType(BBR_VERTEX);
        mBillboardSet->setCastShadows(false);
        mLightCameraSN->attachObject(mBillboardSet);

        // Creating a RRT for depth/shadow map
        createLightCameraRTT();

        // Create a rush of billboards according to the frustum of the camera(mLightCamera)
        // After it, we can use the lightcamera/billboards scenenode like a light projector
        createLightShafts(mBillboardSet, mLightCamera, 100);

        // Set a floor plane
        MeshManager::getSingleton().createPlane("FloorPlaneMesh", RGN_DEFAULT, Plane(Vector3::UNIT_Y, 0),
                                                250, 250, 100, 100, true, 1, 15, 15, Vector3::UNIT_Z);

        Entity* pPlaneEnt = mSceneMgr->createEntity("Plane", "FloorPlaneMesh");
        pPlaneEnt->setMaterialName("Examples/Rockwall");
        pPlaneEnt->setCastShadows(false);
        SceneNode* pPlaneNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        pPlaneNode->attachObject(pPlaneEnt);
        pPlaneNode->setPosition(0, -20, 0);

        // Set a knot
        Entity* mKnot = mSceneMgr->createEntity("Knot", "knot.mesh");
        mKnotSN = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        mKnotSN->attachObject(mKnot);
        mKnotSN->setScale(0.0225, 0.0225, 0.0225);
    }

    bool createLightShafts(BillboardSet* billboard, Camera* LightCamera, const int& NumberOfPlanes)
    {
        // Calculate the distance between planes
        float DistanceBetweenPlanes =
            (LightCamera->getFarClipDistance() - LightCamera->getNearClipDistance()) / NumberOfPlanes;

        // Get frustum corners to calculate near/far planes dimensions
        const Vector3* FrustumCorners = LightCamera->getWorldSpaceCorners();

        // Calcule near and far planes dimensions
        float NearWidth = (FrustumCorners[0] - FrustumCorners[1]).length(),
              NearHeigth = (FrustumCorners[1] - FrustumCorners[2]).length(),
              FarWidth = (FrustumCorners[4] - FrustumCorners[5]).length(),
              FarHeigth = (FrustumCorners[5] - FrustumCorners[6]).length();

        // Now width/heigth setp
        float WidthStep = (FarWidth - NearWidth) / NumberOfPlanes,
              HeigthStep = (FarHeigth - NearHeigth) / NumberOfPlanes;

        // Add billboards
        Billboard* CurrentBB = 0;
        for (int k = 0; k < NumberOfPlanes; k++)
        {
            CurrentBB = billboard->createBillboard(
                Vector3(0, 0, -LightCamera->getNearClipDistance() - k * DistanceBetweenPlanes),
                ColourValue::White);
            CurrentBB->setDimensions(NearWidth + k * WidthStep, NearHeigth + k * HeigthStep);
        }

        return true;
    }

    void createLightCameraRTT()
    {
        mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE);
        mSceneMgr->setShadowTextureSettings(256, 1, PF_BYTE_RGBA);
        mSceneMgr->setShadowTextureCasterMaterial(MaterialManager::getSingleton().getByName("LightShaftsDepth"));

        // Creat a texture for use as rtt
        TexturePtr LightCameraRTT = mSceneMgr->getShadowTexture(0);
        RenderTarget* RT_Texture = LightCameraRTT->getBuffer()->getRenderTarget();

        Viewport* RT_Texture_Viewport = RT_Texture->getViewport(0);
        mLightCamera = RT_Texture_Viewport->getCamera();

        // Not forget to set near+far distance in materials
        mLightCamera->setNearClipDistance(8);
        mLightCamera->setFarClipDistance(40);

        mLightCamera->setDebugDisplayEnabled(true);
    }

    void cleanupContent()
    {
        MeshManager::getSingleton().remove("FloorPlaneMesh", RGN_DEFAULT);
    }
};
