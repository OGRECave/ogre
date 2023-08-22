#pragma once

#include "SdkSample.h"
#include "SamplePlugin.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_RectLight : public SdkSample
{
    Ogre::Light*   mLight;
public:
    Sample_RectLight()
    {
        mInfo["Title"] = "Rectangular Area Light";
        mInfo["Description"] = "Shows how to use rectangular area lights";
        mInfo["Thumbnail"] = "thumb_rectlight.png";
        mInfo["Category"] = "Lighting";
    }

    bool frameStarted(const FrameEvent& e) override
    {
        mLight->getParentNode()->roll(Degree(e.timeSinceLastFrame * 45));

        return SdkSample::frameStarted(e);
    }

    void setupContent() override
    {
        // Make this viewport work with shader generator
        mViewport->setMaterialScheme(MSN_SHADERGEN);

        auto rcvmat = MaterialManager::getSingleton().getDefaultMaterial()->clone("rcvmat");
        rcvmat->getTechnique(0)->getPass(0)->setSpecular(ColourValue::White * 0.25);
        rcvmat->getTechnique(0)->getPass(0)->setShininess(80);

        auto srcmat = MaterialManager::getSingleton().getDefaultMaterial(false)->clone("lightsrcmat");
        srcmat->getTechnique(0)->getPass(0)->setCullingMode(CULL_ANTICLOCKWISE);

        auto ground = mSceneMgr->createEntity(SceneManager::PT_PLANE);
        ground->setMaterial(rcvmat);
        auto groundnode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        groundnode->pitch(Degree(-90));
        groundnode->attachObject(ground);

        auto sphere = mSceneMgr->createEntity(SceneManager::PT_SPHERE);
        sphere->setMaterial(rcvmat);
        auto spherenode = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(20, 6, 20));
        spherenode->attachObject(sphere);
        spherenode->setScale(Vector3(0.1, 0.1, 0.1));
        spherenode->pitch(Degree(180));

        auto srcsz = 12.0;
        mLight = mSceneMgr->createLight("light", Light::LT_RECTLIGHT);
        mLight->setSourceSize(srcsz, srcsz);
        mLight->setSpotlightRange(Degree(160), Degree(180));

        auto lightnode = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(0, 6, 32));
        lightnode->attachObject(mLight);
        auto lightrect = mSceneMgr->createEntity("lightsrc", SceneManager::PT_PLANE);
        lightrect->setMaterial(srcmat);
        auto lightrectnode = lightnode->createChildSceneNode();
        lightrectnode->setScale(Vector3(srcsz/200, srcsz/200, srcsz/200));
        lightrectnode->attachObject(lightrect);

        mCamera->setNearClipDistance(0.1);
        mCameraMan->setStyle(OgreBites::CS_ORBIT);
        mCameraMan->setTarget(lightnode);
        mCameraMan->setYawPitchDist(Degree(180), Degree(15), 50);

        mTrayMgr->createCheckBox(TL_TOPLEFT, "rectlight", "Rectangular Light")->setChecked(true, false);
        mTrayMgr->showCursor();
    }

    void checkBoxToggled(CheckBox* box) override
    {
        mLight->setType(box->isChecked() ? Light::LT_RECTLIGHT : Light::LT_SPOTLIGHT);
    }
};
