#ifndef OGRE_GIZMOS_H
#define OGRE_GIZMOS_H

#include "OgreGizmos.h"
#include "SdkSample.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_Gizmos : public SdkSample, public RenderTargetListener
{
public:
    Sample_Gizmos()
    {
        mInfo["Title"] = "Gizmos";
        mInfo["Description"] = "A demo of gizmos for manipulating a scene.";
        mInfo["Thumbnail"] = "thumb_gizmos.png";
        mInfo["Category"] = "Other";
    }

protected:
    void setupContent() override
    {
        mRayQuery = mSceneMgr->createRayQuery(Ray());
        mRayQuery->setSortByDistance(true);
        mViewport->setBackgroundColour(ColourValue(0.8f, 0.8f, 0.8f, 1.0f));

        // set our camera to orbit around the origin and show cursor
        mCameraMan->setStyle(CS_ORBIT);
        mTrayMgr->showCursor();

        // attach the light to a pivot node
        mLightPivot = mSceneMgr->getRootSceneNode()->createChildSceneNode();

        // create a basic point light with an offset
        Light* light = mSceneMgr->createLight();
        mLightPivot->createChildSceneNode(Vector3(20, 40, 50))->attachObject(light);

        // create our model, give it the shader material, and place it at the origin
        Entity* ent = mSceneMgr->createEntity("Head", "ogrehead.mesh");
        Entity* ent2 = mSceneMgr->createEntity("Head2", "ogrehead.mesh");
        Entity* ent3 = mSceneMgr->createEntity("Head3", "ogrehead.mesh");
        auto node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        auto node2 = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        auto node3 = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        node->attachObject(ent);
        node2->attachObject(ent2);
        node3->attachObject(ent3);
        node->setScale(Vector3{0.10, 0.10, 0.10});
        node2->setScale(Vector3{0.20, 0.20, 0.20});
        node3->setScale(Vector3{0.40, 0.40, 0.40});
        node->setPosition(-25, 0, 0);
        node2->setPosition(-5, 0, 0);
        node3->setPosition(25, 0, 0);
        mSelectedEnt = ent;
        mGizmo = new Gizmo(node, G_TRANSLATE, mCamera, mWindow);
        mCameraGizmo = new CameraGizmo(mCameraNode, mCameraMan.get(), mWindow);

        mInputListenerChain =
            TouchAgnosticInputListenerChain(mWindow, {mTrayMgr.get(), mCameraGizmo, mGizmo, this, mCameraMan.get()});

        // create a checkbox to toggle light movement
        mTranslate = mTrayMgr->createButton(TL_TOPLEFT, "Translate", "Translate");
        mRotate = mTrayMgr->createButton(TL_TOPLEFT, "Rotate", "Rotate");
        mScale = mTrayMgr->createButton(TL_TOPLEFT, "Scale", "Scale");
    }

    bool mousePressed(const MouseButtonEvent& evt) override
    {
        if (evt.button == BUTTON_LEFT)
        {
            float nx = evt.x / float(mWindow->getWidth());
            float ny = evt.y / float(mWindow->getHeight());

            Ray ray = mCamera->getCameraToViewportRay(nx, ny);
            mRayQuery->setRay(ray);

            selectEntity(pickEntity());
            if (mCameraMan->mousePressed(evt))
            {
                return true;
            }
        }
        return SdkSample::mousePressed(evt);
    }

private:
    Entity* pickEntity() const
    {
        auto& hits = mRayQuery->execute();
        for (auto& h : hits)
        {
            if (!h.movable)
                continue;

            auto* ent = dynamic_cast<Entity*>(h.movable);
            if (!ent)
                continue;

            return ent;
        }
        return nullptr;
    }

    void selectEntity(Entity* ent)
    {
        if (ent != nullptr && ent != mSelectedEnt && !mGizmo->isGizmoEntity(ent))
        {
            mSelectedEnt = ent;
            mGizmo->attachTo(ent->getParentSceneNode());
        }
    }

    void buttonHit(Button* button) override
    {
        String name = button->getName();
        if (name == "Translate")
        {
            mGizmo->setMode(G_TRANSLATE);
        }
        else if (name == "Rotate")
        {
            mGizmo->setMode(G_ROTATE);
        }
        else if (name == "Scale")
        {
            mGizmo->setMode(G_SCALE);
        }
    }

    RaySceneQuery* mRayQuery;
    Gizmo* mGizmo;
    CameraGizmo* mCameraGizmo;
    Entity* mSelectedEnt;
    SceneNode* mLightPivot;
    Button* mTranslate;
    Button* mRotate;
    Button* mScale;
};
#endif // OGRE_GIZMOS_H