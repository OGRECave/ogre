#ifndef __Transparency_H__
#define __Transparency_H__

#include "SdkSample.h"

using namespace Ogre;
using namespace OgreBites;

/* NOTE: This sample simply displays an object with a transparent material. The really relevant stuff
is all in the material script itself. You won't find anything even vaguely related to transparency in
this source code. Check out the Examples/WaterStream material in Examples.material. */

class _OgreSampleClassExport Sample_Transparency : public SdkSample
{
public:

    Sample_Transparency()
    {
        mInfo["Title"] = "Transparency";
        mInfo["Description"] = "Demonstrates the use of transparent materials (or scene blending).";
        mInfo["Thumbnail"] = "thumb_trans.png";
        mInfo["Category"] = "Lighting";

        addScreenshotFrame(25);
    }

    bool frameRenderingQueued(const FrameEvent& evt) override
    {
        Real theta = ControllerManager::getSingleton().getElapsedTime();

        // this is the equation for a PQ torus knot
        Ogre::Real r = 28 * (2 + Math::Sin(theta * 3 / 2 + 0.2));
        Ogre::Real x = r * Math::Cos(theta);
        Ogre::Real y = r * Math::Sin(theta);
        Ogre::Real z = 60 * Math::Cos(theta * 3 / 2 + 0.2);

        Vector3 lastPos = mFishNode->getPosition();   // save fishy's last position
        mFishNode->setPosition(x, y, z);              // set fishy's new position

        // set fishy's direction based on the change in position
        mFishNode->setDirection(mFishNode->getPosition() - lastPos, Node::TS_PARENT, Vector3::NEGATIVE_UNIT_X);

        mFishSwim->addTime(evt.timeSinceLastFrame * 5);   // update fishy's swimming animation

        return SdkSample::frameRenderingQueued(evt);   // don't forget the parent class updates!
    }

protected:

    void checkBoxToggled(CheckBox* box) override
    {
        auto& cm = CompositorManager::getSingleton();
        cm.setCompositorEnabled(mViewport, "WBOIT", box->isChecked());

        if(box->isChecked())
        {
            mWaterStream->setMaterialName("Examples/WaterStream/OIT");
        }
        else
        {
            mWaterStream->setMaterialName("Examples/WaterStream");
        }
    }

    void setupContent() override
    {
#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
        // Need RTSS for WBOIT
        mViewport->setMaterialScheme(MSN_SHADERGEN);
        MaterialManager::getSingleton().setActiveScheme(mViewport->getMaterialScheme());
#endif
        mSceneMgr->setSkyBox(true, "Examples/TrippySkyBox");

        mCameraMan->setStyle(CS_ORBIT);
        mCameraMan->setYawPitchDist(Radian(0), Radian(0), 300); // set camera's starting position

        mSceneMgr->getRootSceneNode()
            ->createChildSceneNode(Vector3(20, 80, 50))
            ->attachObject(mSceneMgr->createLight());  // add basic point light

        // create a torus knot model, give it the translucent texture, and attach it to the origin
        mWaterStream = mSceneMgr->createEntity("Knot", "knot.mesh");
        mSceneMgr->getRootSceneNode()->attachObject(mWaterStream);

        // create a fishy and enable its swimming animation
        auto ent = mSceneMgr->createEntity("Fish", "fish.mesh");
        mFishSwim = ent->getAnimationState("swim");
        mFishSwim->setEnabled(true);
        
        // create a scene node, attach fishy to it, and scale it by a factor of 2
        mFishNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        mFishNode->attachObject(ent);
        mFishNode->setScale(2, 2, 2);

        mWaterStream->setMaterialName("Examples/WaterStream");
        if(!mTrayMgr)
            return;

        // OIT compositor (disabled)
        auto& cm = CompositorManager::getSingleton();
        cm.addCompositor(mViewport, "WBOIT");

        // GUI
        mTrayMgr->showCursor();

        auto oitMat = Ogre::MaterialManager::getSingleton().getByName("Examples/WaterStream/OIT");
        oitMat->load();
        if (oitMat->getBestTechnique()->getSchemeName() != mViewport->getMaterialScheme())
            return;

        mTrayMgr->createCheckBox(TL_TOPLEFT, "OIT", "Order Independent Transparency")->setChecked(false, true);
    }

    Entity* mWaterStream;
    SceneNode* mFishNode;
    AnimationState* mFishSwim;
};

#endif
