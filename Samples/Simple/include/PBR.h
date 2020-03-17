#ifndef __Sample_PBR_H__
#define __Sample_PBR_H__

#include "SdkSample.h"
#include "SamplePlugin.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_PBR : public SdkSample
{
public:

    Sample_PBR()
    {
        mInfo["Title"] = "Physically Based Rendering";
        mInfo["Description"] = "Shows how to do physically based rendering using the glTF2.0 reference shaders";
        mInfo["Thumbnail"] = "thumb_pbr.png";
        mInfo["Category"] = "Lighting";
    }

    ~Sample_PBR()
    {
        
    }

protected:
    GpuProgramParametersSharedPtr mParams;

    void setupControls()
    {
        int gui_width = 250;

        SelectMenu* objectType = mTrayMgr->createThickSelectMenu(TL_TOPLEFT, "term", "Term", gui_width, 9);
        objectType->addItem("final");
        objectType->addItem("fresnel (F)");
        objectType->addItem("geometric (G)");
        objectType->addItem("microfacet (D)");
        objectType->addItem("specular (FGD)");
        objectType->addItem("diffuse");
        objectType->addItem("albedo");
        objectType->addItem("metallic");
        objectType->addItem("roughness");
        objectType->selectItem(0, false);

        mTrayMgr->createCheckBox(TL_TOPLEFT, "ibl", "Image Based Lighting", gui_width)->setChecked(true, false);
    }

    void setupContent()
    {
        setupControls();

        mCamera->setNearClipDistance(0.1);
        mCameraMan->setStyle(CS_ORBIT);
        mCameraMan->setYawPitchDist(Degree(-30), Degree(30), 3);
        mTrayMgr->showCursor();

        Light* light = mSceneMgr->createLight();
        light->setDiffuseColour(ColourValue::White);

        mSceneMgr->getRootSceneNode()
            ->createChildSceneNode(Vector3(4, 1, 6))
            ->attachObject(light);

        Entity* ent = mSceneMgr->createEntity("DamagedHelmet", "DamagedHelmet.mesh");
        ent->getMesh()->buildTangentVectors(); // enforce that we have tangent vectors

        SceneNode* node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        node->pitch(Degree(90));
        node->attachObject(ent);

        mViewport->setBackgroundColour(ColourValue(0.05, 0.05, 0.05));

        MaterialPtr mat = MaterialManager::getSingleton().getByName("DamagedHelmet");
        mParams = mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
    }

    void itemSelected(SelectMenu* menu)
    {
        Real mix[8] = {0};
        int n = menu->getSelectionIndex();

        if (n > 0)
            mix[n - 1] = 1;

        mParams->setNamedConstant("u_ScaleFGDSpec", Vector4(mix));
        mParams->setNamedConstant("u_ScaleDiffBaseMR", Vector4(mix + 4));
    }
        
    void checkBoxToggled(CheckBox* box)
    {
        mParams->setNamedConstant("u_ScaleIBLAmbient", Vector4(Real(box->isChecked())));
    }
};

#endif
