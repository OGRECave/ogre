/**
*   Modified by: Juan Camilo Acosta Arango (ja0335 )
*   Date: 09-04-2014
**/

#ifndef __PNTriangles_H__
#define __PNTriangles_H__

#include "SdkSample.h"
#include "OgreImage.h"
#include "OgreBillboard.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_PNTriangles : public SdkSample
{
public:

    Sample_PNTriangles()
        : mMoveLights (true)
    {
        mInfo["Title"] = "PNTriangles";
        mInfo["Description"] = "Sample for parametric PN-Triangles tessellation algorithm";
        mInfo["Thumbnail"] = "thumb_tessellation.png";
        mInfo["Category"] = "Unsorted";
        mInfo["Help"] = "Top Left: Multi-frame\nTop Right: Scrolling\nBottom Left: Rotation\nBottom Right: Scaling";
    }

    void testCapabilities(const RenderSystemCapabilities* caps) override
    {
        if (!caps->hasCapability(RSC_TESSELLATION_HULL_PROGRAM) || !caps->hasCapability(RSC_TESSELLATION_DOMAIN_PROGRAM))
        {
            OGRE_EXCEPT(Exception::ERR_INVALID_STATE, "Your graphics card does not support tessellation shaders. Sorry!",
                "Sample_PNTrianglesTessellation:testCapabilities");
        }
        if (!GpuProgramManager::getSingleton().isSyntaxSupported("vs_5_0") &&
            !GpuProgramManager::getSingleton().isSyntaxSupported("hs_5_0") &&
            !GpuProgramManager::getSingleton().isSyntaxSupported("ds_5_0") &&
            !GpuProgramManager::getSingleton().isSyntaxSupported("ps_5_0") &&
            !GpuProgramManager::getSingleton().isSyntaxSupported("hlsl"))
        {
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your card does not support the shader model 5.0 needed for this sample, "
                "so you cannot run this sample. Sorry!", "Sample_PNTrianglesTessellation::testCapabilities");
        }
    }

    bool frameRenderingQueued(const FrameEvent& evt) override
    {
        if (mMoveLights)
        {
            // rotate the light pivots
            mLightPivot1->roll(Degree(evt.timeSinceLastFrame * 10));
            mLightPivot2->roll(Degree(evt.timeSinceLastFrame * 15));
        }

        return SdkSample::frameRenderingQueued(evt);  // don't forget the parent class updates!
    }

    void itemSelected(SelectMenu* menu) override
    {
        if (menu == mMeshMenu)
        {
            // change to the selected entity
            mObjectNode->detachAllObjects();
            mObjectNode->attachObject(mSceneMgr->getEntity(mMeshMenu->getSelectedItem()));

            // remember which material is currently selected
            int index = std::max<int>(0, mMaterialMenu->getSelectionIndex());

            // update the material menu's options
            mMaterialMenu->setItems(mPossibilities[mMeshMenu->getSelectedItem()]);

            mMaterialMenu->selectItem(index);   // select the material with the saved index
        }
        else
        {
            // set the selected material for the active mesh
            ((Entity*)mObjectNode->getAttachedObject(0))->setMaterialName(menu->getSelectedItem());

            if( menu->getSelectionIndex() == 2 )
                mTessellationAmount->show();
            else
                mTessellationAmount->hide();
        }
    }

    void checkBoxToggled(CheckBox* box) override
    {
        if (box->getName() == "Wire")
        {
            if( mCamera->getPolygonMode() == PM_WIREFRAME )
                mCamera->setPolygonMode(PM_SOLID);
            else
                mCamera->setPolygonMode(PM_WIREFRAME);
        }
        else if (StringUtil::startsWith(box->getName(), "Light", false))
        {
            // get the light pivot that corresponds to this checkbox
            SceneNode* pivot = box->getName() == "Light1" ? mLightPivot1 : mLightPivot2;
            // toggle visibility of light and billboard set
            pivot->setVisible(box->isChecked());
        }
        else if (box->getName() == "MoveLights")
        {
            mMoveLights = !mMoveLights;
        }
    }

    void sliderMoved(Slider* slider) override
    {
        if( slider->getName() == "tessellationAmount" )
        {
            MaterialPtr lMaterialPtr = MaterialManager::getSingleton().getByName( mMaterialMenu->getSelectedItem() );
            lMaterialPtr->getTechnique(0)->getPass(0)->getTessellationHullProgramParameters()->setNamedConstant( "g_tessellationAmount", slider->getValue() );
        }
    }

protected:

    void setupContent() override
    {
        // create our main node to attach our entities to
        mObjectNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();

        setupModels();
        setupLights();
        setupControls();
        
        // set our camera
        mCamera->setFOVy(Ogre::Degree(50.0));
        mCamera->setFOVy(Ogre::Degree(50.0));
        mCamera->setNearClipDistance(0.01f);
        mCameraNode->lookAt(Ogre::Vector3::ZERO, Ogre::Node::TS_PARENT);
        mCameraNode->setPosition(0, 0, 500);
        

        // Set our camera to orbit around the origin at a suitable distance
        mCameraMan->setStyle(CS_ORBIT);
        mCameraMan->setYawPitchDist(Radian(0), Radian(0), 400);

        mTrayMgr->showCursor();
    }

    void unloadResources() override
    {

    }

    void setupModels()
    {
        StringVector matNames;

        matNames.push_back("Ogre/NoTessellation");
        matNames.push_back("Ogre/TessellationExample");
        matNames.push_back("Ogre/SimpleTessellation");
        //matNames.push_back("Ogre/AdaptiveTessellation");
        matNames.push_back("Ogre/AdaptivePNTrianglesTessellation");
        
        mPossibilities["ogrehead.mesh"] = matNames;
        mPossibilities["knot.mesh"] = matNames;
        mPossibilities["uv_sphere.mesh"] = matNames;

        for (std::map<String, StringVector>::iterator it = mPossibilities.begin(); it != mPossibilities.end(); it++)
        {
            // load each mesh with non-default hardware buffer usage options
            MeshPtr mesh = MeshManager::getSingleton().load(it->first, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);

            // build tangent vectors for our mesh
            mesh->buildTangentVectors();

            // create an entity from the mesh and set the first available material
            Entity* ent = mSceneMgr->createEntity(mesh->getName(), mesh->getName());
            ent->setMaterialName(it->second.front());
        }
    }

    void setupLights()
    {
        mSceneMgr->setAmbientLight(ColourValue::Black); 
        mViewport->setBackgroundColour(ColourValue(0.41f, 0.41f, 0.41f));

        // create pivot nodes
        mLightPivot1 = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        mLightPivot2 = mSceneMgr->getRootSceneNode()->createChildSceneNode();

        mLightPivot1->setPosition(Vector3(200, 0, 0));
        mLightPivot2->setPosition(Vector3(-200, 0, 0));

        mLightPivot1->setDirection(-Vector3::UNIT_X);        
        mLightPivot2->setDirection(Vector3::UNIT_X);

        Light* l;
        BillboardSet* bbs;

        // create white light
        l = mSceneMgr->createLight();
        l->setDiffuseColour(1.0f, 1.0f, 1.0f);
        l->setSpecularColour(1.0f, 1.0f, 1.0f);
        mLightPivot1->attachObject(l);

        // create white flare
        bbs = mSceneMgr->createBillboardSet();
        bbs->setMaterialName("Examples/Flare");
        bbs->createBillboard(200, 0, 0)->setColour(ColourValue::White);
        mLightPivot1->attachObject(bbs);

        // create red light
        l = mSceneMgr->createLight();
        l->setDiffuseColour(1.0f, 0.0f, 0.0f);
        l->setSpecularColour(1.0f, 0.0f, 0.0f);
        mLightPivot2->attachObject(l);

        // create white flare
        bbs = mSceneMgr->createBillboardSet();
        bbs->setMaterialName("Examples/Flare");
        bbs->createBillboard(-200, 0, 0)->setColour(ColourValue::Red);
        mLightPivot2->attachObject(bbs);
    }

    void setupControls()
    {
        mTrayMgr->showCursor();

        // make room for the controls
        mTrayMgr->showLogo(TL_TOPRIGHT);
        mTrayMgr->showFrameStats(TL_TOPRIGHT);
        mTrayMgr->toggleAdvancedFrameStats();

        // create a menu to choose the model displayed
        mMeshMenu = mTrayMgr->createLongSelectMenu(TL_BOTTOM, "Mesh", "Mesh", 370, 290, 10);
        for (std::map<String, StringVector>::iterator it = mPossibilities.begin(); it != mPossibilities.end(); it++)
            mMeshMenu->addItem(it->first);

        // create a menu to choose the material used by the model
        mMaterialMenu = mTrayMgr->createLongSelectMenu(TL_BOTTOM, "Material", "Material", 370, 290, 10);
        
        mTrayMgr->createCheckBox(TL_TOPLEFT, "Wire", "Wire Frame")->setChecked(false, false);
        // create checkboxes to toggle lights
        mTrayMgr->createCheckBox(TL_TOPLEFT, "Light1", "Light A")->setChecked(true, false);
        mTrayMgr->createCheckBox(TL_TOPLEFT, "Light2", "Light B")->setChecked(true, false);
        mTrayMgr->createCheckBox(TL_TOPLEFT, "MoveLights", "Move Lights")->setChecked(true, false);
        
        mTessellationAmount = mTrayMgr->createThickSlider(TL_TOPLEFT, "tessellationAmount", "Tessellation Amount", 200, 40, 1, 8, 8);
        mTessellationAmount->hide();

        // a friendly reminder
        StringVector names;
        names.push_back("Help");
        mTrayMgr->createParamsPanel(TL_TOPLEFT, "Help", 100, names)->setParamValue(0, "H/F1");

        mMeshMenu->selectItem(0);  // select first mesh
    }

    void cleanupContent() override
    {
        // clean up properly to avoid interfering with subsequent samples
        for (std::map<String, StringVector>::iterator it = mPossibilities.begin(); it != mPossibilities.end(); it++)
            MeshManager::getSingleton().unload(it->first, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        mPossibilities.clear();
    }

    std::map<String, StringVector> mPossibilities;
    SceneNode* mObjectNode;
    SceneNode* mLightPivot1;
    SceneNode* mLightPivot2;
    bool mMoveLights;
    SelectMenu* mMeshMenu;
    SelectMenu* mMaterialMenu;
    Slider*     mTessellationAmount;

};

#endif
