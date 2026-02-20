#ifndef OGRE_GIZMOS_H
#define OGRE_GIZMOS_H

#include "OgreGizmos.h"
#include "SdkSample.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_Gizmos : public SdkSample
{
    public:

    Sample_Gizmos()
    {
        mInfo["Title"] = "Gizmos";
        mInfo["Description"] = "A demo of gizmos for manipulating a scene.";
        mInfo["Thumbnail"] = "thumb_cel.png";
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
        mLightPivot->createChildSceneNode(Vector3f(20, 40, 50))->attachObject(light);

        // create our model, give it the shader material, and place it at the origin
        Entity *ent = mSceneMgr->createEntity("Head", "ogrehead.mesh");
        Entity *ent2 = mSceneMgr->createEntity("Head2", "ogrehead.mesh");
        Entity *ent3 = mSceneMgr->createEntity("Head3", "ogrehead.mesh");
        auto node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        auto node2 = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        auto node3 = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        node->attachObject(ent);
        node2->attachObject(ent2);
        node3->attachObject(ent3);
        node->setScale(Vector3f{0.10,0.10, 0.10});
        node2->setScale(Vector3f{0.20,0.20, 0.20});
        node3->setScale(Vector3f{0.40,0.40, 0.40});
        node->setPosition(-25, 0, 0);
        node2->setPosition(-5, 0, 0);
        node3->setPosition(25, 0, 0);
        mSelectedEnt = ent;
        mGizmo = new Gizmo(mSceneMgr, node, G_TRANSLATE);
        mCameraGizmo = new CameraGizmo(mCameraNode, mCameraMan.get());

        // create a checkbox to toggle light movement
        mTranslate = mTrayMgr->createButton(TL_TOPLEFT, "Translate", "Translate");
        mRotate = mTrayMgr->createButton(TL_TOPLEFT, "Rotate", "Rotate");
        mScale = mTrayMgr->createButton(TL_TOPLEFT, "Scale", "Scale");
    }

    bool frameRenderingQueued(const FrameEvent& evt) override
    {
        mCameraGizmo->updateOrientation();

        return true;
    }

    bool mouseMoved(const MouseMotionEvent& evt) override
    {
        float nx = float(evt.x) / float(mWindow->getWidth());
        float ny = float(evt.y) / float(mWindow->getHeight());
        if (mGizmo->isDragging())
        {
            Ray ray = mCamera->getCameraToViewportRay(nx, ny);
            mGizmo->computeDrag(ray, mCamera->getDerivedDirection());
            return true;
        }

        if (mCameraMan != nullptr && mCameraMan->mouseMoved(evt)) return true;
        Ray ray = mCamera->getCameraToViewportRay(nx, ny);
        mRayQuery->setRay(ray);
        mGizmo->pickAxis(ray);
        if (nx >= 0.80f && nx <= 1.0f &&
               ny >= 0.00f && ny <= 0.20f)
        {
            mCameraGizmo->pickFace(nx, ny);
        }
        return true;
    }

    bool mousePressed(const MouseButtonEvent& evt) override
    {
        if (evt.button == BUTTON_LEFT)
        {
            float nx = evt.x / float(mWindow->getWidth());
            float ny = evt.y / float(mWindow->getHeight());

            Ray ray = mCamera->getCameraToViewportRay(nx, ny);
            mRayQuery->setRay(ray);
            if (mGizmo->pickAxis(ray))
            {
                mGizmo->startDrag(mCamera->getCameraToViewportRay(nx, ny), mCamera->getDerivedDirection());
                return true;
            }
            if (nx >= 0.80f && nx <= 1.0f &&
                   ny >= 0.00f && ny <= 0.20f)
            {
                mCameraGizmo->snapCamera(mCameraGizmo->pickFace(nx, ny));
            }
            selectEntity(pickEntity());
            if (mCameraMan->mousePressed(evt))
            {
                return true;
            }
        }
        return false;
    }

    bool mouseReleased(const MouseButtonEvent& evt) override
    {
        if (evt.button == BUTTON_LEFT)
        {
            if (mGizmo->isDragging())
            {
                mGizmo->stopDrag();
                return true;
            }
        }
        if (mCameraMan->mouseReleased(evt))
        {
            return true;
        }
        return false;
    }

private:

    Entity* pickEntity()
    {
        auto& hits = mRayQuery->execute();
        for (auto& h : hits)
        {
            if (!h.movable) continue;

            auto* ent = dynamic_cast<Entity*>(h.movable);
            if (!ent) continue;

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

    void hoverEntity(Entity* ent)
    {
        if (ent == mHoveredEnt) return;
        if (mGizmo->isGizmoEntity(ent)) return;
        mHoveredEnt->setDebugDisplayEnabled(false);
        mHoveredEnt = ent;
        mHoveredEnt->setDebugDisplayEnabled(true);
    }

    void buttonHit( Button* button ) override
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

    // custom shader parameter bindings
    RaySceneQuery* mRayQuery;
    Gizmo* mGizmo;
    CameraGizmo* mCameraGizmo;
    Entity* mSelectedEnt;
    Entity* mHoveredEnt;
    SceneNode* mLightPivot;
    Button* mTranslate;
    Button* mRotate;
    Button* mScale;
};
#endif // OGRE_GIZMOS_H