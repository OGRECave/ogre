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

    void buttonHit(OgreBites::Button* button) override;

    // custom shader parameter bindings
    enum ShaderParam { SP_SHININESS = 1, SP_DIFFUSE, SP_SPECULAR };

    Gizmo* mGizmo;
    SceneNode* mLightPivot;
    Button* mTranslate;
    Button* mRotate;
    Button* mScale;
};

inline void Sample_Gizmos::buttonHit( OgreBites::Button* button )
{
    Ogre::String name = button->getName();
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
#endif // OGRE_GIZMOS_H