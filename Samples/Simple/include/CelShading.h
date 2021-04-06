#ifndef __CelShading_H__
#define __CelShading_H__

#include "SdkSample.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_CelShading : public SdkSample
{
public:

    Sample_CelShading()
    {
        mInfo["Title"] = "Cel-shading";
        mInfo["Description"] = "A demo of cel-shaded graphics using vertex & fragment programs.";
        mInfo["Thumbnail"] = "thumb_cel.png";
        mInfo["Category"] = "Lighting";
    }

    void testCapabilities(const RenderSystemCapabilities* caps)
    {
        requireMaterial("Examples/CelShading");
    }

    bool frameRenderingQueued(const FrameEvent& evt)
    {
        // make the light revolve around our model if and only if the check box is checked
        if (mMoveLight->isChecked()) mLightPivot->yaw(Degree(evt.timeSinceLastFrame * 30));

        return SdkSample::frameRenderingQueued(evt);  // don't forget the parent class updates!
    }

protected:

    void setupContent()
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
        Entity *ent = mSceneMgr->createEntity("Head", "ogrehead.mesh");
        ent->setMaterialName("Examples/CelShading");
        mSceneMgr->getRootSceneNode()->attachObject(ent);

        /* We set the same material for all parts of the head, but use custom shader parameters to set the
        colours for each part. See Examples-Advanced.material for how these are bound to GPU parameters. */

        SubEntity* sub;
        
        sub = ent->getSubEntity(0);    // eyes
        sub->setCustomParameter(SP_SHININESS, Vector4(35, 0, 0, 0));
        sub->setCustomParameter(SP_DIFFUSE, Vector4(1, 0.3, 0.3, 1));
        sub->setCustomParameter(SP_SPECULAR, Vector4(1, 0.6, 0.6, 1));

        sub = ent->getSubEntity(1);    // skin
        sub->setCustomParameter(SP_SHININESS, Vector4(10, 0, 0, 0));
        sub->setCustomParameter(SP_DIFFUSE, Vector4(0, 0.5, 0, 1));
        sub->setCustomParameter(SP_SPECULAR, Vector4(0.3, 0.5, 0.3, 1));

        sub = ent->getSubEntity(2);    // earring
        sub->setCustomParameter(SP_SHININESS, Vector4(25, 0, 0, 0));
        sub->setCustomParameter(SP_DIFFUSE, Vector4(1, 1, 0, 1));
        sub->setCustomParameter(SP_SPECULAR, Vector4(1, 1, 0.7, 1));

        sub = ent->getSubEntity(3);    // teeth
        sub->setCustomParameter(SP_SHININESS, Vector4(20, 0, 0, 0));
        sub->setCustomParameter(SP_DIFFUSE, Vector4(1, 1, 0.7, 1));
        sub->setCustomParameter(SP_SPECULAR, Vector4(1, 1, 1, 1));

        // create a check box to toggle light movement
        mMoveLight = mTrayMgr->createCheckBox(TL_TOPLEFT, "MoveLight", "Move Light");
        mMoveLight->setChecked(true);
    }

    // custom shader parameter bindings
    enum ShaderParam { SP_SHININESS = 1, SP_DIFFUSE, SP_SPECULAR };

    SceneNode* mLightPivot;
    CheckBox* mMoveLight;
};

#endif
