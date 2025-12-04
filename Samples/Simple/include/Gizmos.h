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
        mViewport->setBackgroundColour(ColourValue::White);

        // set our camera to orbit around the origin and show cursor
        mCameraMan->setStyle(CS_ORBIT);
        mTrayMgr->showCursor();

        // attach the light to a pivot node
        mLightPivot = mSceneMgr->getRootSceneNode()->createChildSceneNode();

        // create a basic point light with an offset
        Light* light = mSceneMgr->createLight();
        mLightPivot->createChildSceneNode(Vector3(20, 40, 50))->attachObject(light);

        // create our model, give it the shader material, and place it at the origin
        SceneNode* gizmoParent = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        mGizmo = new Gizmo(getSceneManager(), gizmoParent, G_ROTATE);

        // create a checkbox to toggle light movement
        mTranslate = mTrayMgr->createButton(TL_TOPLEFT, "Translate", "Translate");
        mRotate = mTrayMgr->createButton(TL_TOPLEFT, "Rotate", "Rotate");
        mScale = mTrayMgr->createButton(TL_TOPLEFT, "Scale", "Scale");
    }

    bool mouseMoved(const OgreBites::MouseMotionEvent& evt) override
    {


        if (!mDragging)
        {
            if (mCameraMan->mouseMoved(evt))
                return true;
            pickObject(evt.x / float(mWindow->getWidth()),
                            evt.y / float(mWindow->getHeight()));
        }
        else if (mActiveGizmo)
        {
            Ray ray = mCamera->getCameraToViewportRay(
                evt.x / float(mWindow->getWidth()),
                evt.y / float(mWindow->getHeight()));

            // 1. Compute intersection with plane/axis
            Vector3 newPos = mActiveGizmo.computeDrag(ray);

            // 2. Apply to object
            mGizmo->getObject()->setPosition(newPos);

             // // 3. Notify parent / owner object
            // if (mParentObject)
            //     mParentObject->onGizmoMoved(newPos);
        }

        return true;
    }


    bool mousePressed(const MouseButtonEvent& evt) override
    {
        if (evt.button != BUTTON_LEFT)
            return false;

        float nx = float(evt.x) / float(mWindow->getWidth());
        float ny = float(evt.y) / float(mWindow->getHeight());

        auto* picked = pickObject(nx, ny);

        if (picked)
        {
            mDragging = true;
            mActiveGizmo = picked;
            mDragStartRay = Sample::mCamera->getCameraToViewportRay(
                evt.x / float(mWindow->getWidth()),
                evt.y / float(mWindow->getHeight()));

            mInitialObjectPos = mGizmo->getObject()->getPosition();
        }

        return true;
    }

    bool mouseReleased(const MouseButtonEvent& evt) override
    {
        if (evt.button != BUTTON_LEFT)
            return false;

        mDragging = false;
        mActiveGizmo = nullptr;

        return true;
    }

    private:

    Entity* pickObject(float x, float y)
    {
        Ray ray = mCamera->getCameraToViewportRay(x, y);

        mRayQuery->setRay(ray);

        Entity* pickedEntity = nullptr;

        auto& hits = mRayQuery->execute();
        for (auto& h : hits)
        {
            if (!h.movable) continue;

            auto* ent = dynamic_cast<Entity*>(h.movable);
            if (!ent) continue;

            pickedEntity = ent;
            mGizmo->setHighlighted(ent);
            break;
        }
        if (!pickedEntity)
            mGizmo->setHighlighted(nullptr);
        return pickedEntity;
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
    enum ShaderParam { SP_SHININESS = 1, SP_DIFFUSE, SP_SPECULAR };
    bool mDragging = false;
    Entity* mActiveGizmo = nullptr;
    Ray mDragStartRay;
    Vector3 mInitialObjectPos;
    RaySceneQuery* mRayQuery;
    Gizmo* mGizmo;
    SceneNode* mLightPivot;
    Button* mTranslate;
    Button* mRotate;
    Button* mScale;
};
#endif // OGRE_GIZMOS_H