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
        mInfo["Description"] = "Shows how to do physically based rendering using a custom material";
        mInfo["Thumbnail"] = "thumb_pbr.png";
        mInfo["Category"] = "Lighting";
    }

protected:
    GpuProgramParametersSharedPtr mParams;
    Entity* mEntity;

    void setupControls()
    {
        int gui_width = 250;

        SelectMenu* objectType = mTrayMgr->createThickSelectMenu(TL_TOPLEFT, "term", "Material", gui_width, 9);
        objectType->addItem("glTF2 Shader");
        objectType->addItem("RTSS Fallback");
        if(GpuProgramManager::getSingleton().isSyntaxSupported("glsl330"))
            objectType->addItem("Filament Shader");
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

        mEntity = mSceneMgr->createEntity("DamagedHelmet.mesh");

        unsigned short src, dst;
        if (!mEntity->getMesh()->suggestTangentVectorBuildParams(VES_TANGENT, src, dst))
        {
            // enforce that we have tangent vectors
            mEntity->getMesh()->buildTangentVectors(VES_TANGENT, src, dst);
        }

        SceneNode* node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        node->attachObject(mEntity);

        mViewport->setBackgroundColour(ColourValue(0.05, 0.05, 0.05));

        MaterialPtr mat = MaterialManager::getSingleton().getByName("DamagedHelmet");
        mParams = mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
    }

    void itemSelected(SelectMenu* menu)
    {
        static const char* materials[] = {"DamagedHelmet", "DamagedHelmet_FFP", "DamagedHelmet_Filament"};
        int n = menu->getSelectionIndex();

        mEntity->setMaterialName(materials[n]);
    }
        
    void checkBoxToggled(CheckBox* box)
    {
        mParams->setNamedConstant("u_ScaleIBLAmbient", Vector4(Real(box->isChecked())));
    }
};

#endif
