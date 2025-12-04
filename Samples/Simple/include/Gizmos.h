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

    bool mouseMoved(const MouseMotionEvent& evt) override
    {
        float nx = float(evt.x) / float(mWindow->getWidth());
        float ny = float(evt.y) / float(mWindow->getHeight());

        // If we are dragging, DO NOT pick again — only continue the drag
        if (mGizmo->isDragging())
        {
            Ogre::Ray ray = mCamera->getCameraToViewportRay(nx, ny);
            mGizmo->computeDrag(ray, mCamera->getDerivedDirection());
            return true;  // handled
        }

        // Not dragging → hover highlighting
        Ogre::Entity* hover = pickObject(nx, ny);
        mGizmo->setHighlighted(hover);

        // Allow camera to still move/orbit/etc.
        mCameraMan->mouseMoved(evt);

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
            mGizmo->startDrag(picked, mCamera->getCameraToViewportRay(
                evt.x / float(mWindow->getWidth()),
                evt.y / float(mWindow->getHeight())));
        }

        return true;
    }

    bool mouseReleased(const MouseButtonEvent& evt) override
    {
        if (evt.button != BUTTON_LEFT)
            return false;

        mGizmo->stopDrag();

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
    Vector3 mInitialObjectPos;
    RaySceneQuery* mRayQuery;
    Gizmo* mGizmo;
    SceneNode* mLightPivot;
    Button* mTranslate;
    Button* mRotate;
    Button* mScale;
};
#endif // OGRE_GIZMOS_H