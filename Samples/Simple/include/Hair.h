/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
-----------------------------------------------------------------------------
*/

#ifndef __Hair_H__
#define __Hair_H__

#include "SdkSample.h"
#include "OgreImage.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_Hair : public SdkSample
{
public:

    Sample_Hair()
    {
        mInfo["Title"] = "Hair";
        mInfo["Description"] = "Sample for terrain, water tessellation and the use of displacement mapping";
        mInfo["Thumbnail"] = "thumb_tesselation.png";
        mInfo["Category"] = "Unsorted";
        mInfo["Help"] = "Top Left: Multi-frame\nTop Right: Scrolling\nBottom Left: Rotation\nBottom Right: Scaling";
    }

    void testCapabilities(const RenderSystemCapabilities* caps)
    {
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "This sample is not yet finished."
                " Sorry!", "Sample_Hair::testCapabilities");

        if (!caps->hasCapability(RSC_TESSELLATION_HULL_PROGRAM) || !caps->hasCapability(RSC_TESSELLATION_DOMAIN_PROGRAM))
        {
            OGRE_EXCEPT(Exception::ERR_INVALID_STATE, "Your graphics card does not support tesselation shaders. Sorry!",
                "Sample_Hair:testCapabilities");
        }
        if (!GpuProgramManager::getSingleton().isSyntaxSupported("vs_5_0") &&
            !GpuProgramManager::getSingleton().isSyntaxSupported("hs_5_0") &&
            !GpuProgramManager::getSingleton().isSyntaxSupported("ds_5_0") &&
            !GpuProgramManager::getSingleton().isSyntaxSupported("ps_5_0") &&
            !GpuProgramManager::getSingleton().isSyntaxSupported("hlsl"))
        {
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your card does not support the shader model 5.0 needed for this sample, "
                "so you cannot run this sample. Sorry!", "Sample_Hair::testCapabilities");
        }
    }

    bool frameRenderingQueued(const FrameEvent& evt)
    {
        return SdkSample::frameRenderingQueued(evt);  // don't forget the parent class updates!
    }

    void checkBoxToggled(CheckBox* box)
    {
        if (box->getName() == "Wire")
        {
            if( mCamera->getPolygonMode() == PM_WIREFRAME )
                mCamera->setPolygonMode(PM_SOLID);
            else
                mCamera->setPolygonMode(PM_WIREFRAME);
        }
        if (box->getName() == "PlayAnimation")
        {
            mPlayAnimation = !mPlayAnimation;
        }
        if (box->getName() == "LoopAnimation")
        {
            mLoopAnimation = !mLoopAnimation;
        }
        if (box->getName() == "ShortHair")
        {
            mShortHair = !mShortHair;
            // change mesh / model
        }
        if (box->getName() == "CurlyHair")
        {
            mCurlyHair = !mCurlyHair;
            // change mesh / model
        }
        if (box->getName() == "Shadows")
        {
            mShadows = !mShadows;
            // render or not shadows / using Ogre shadows.
        }
        if (box->getName() == "RenderMStrands")
        {
            mRenderMStrands = !mRenderMStrands;
            
            MaterialPtr lMaterialPtr = MaterialManager::getSingleton().getByName( "Hair" ).staticCast<Material>();
            lMaterialPtr->getTechnique(0)->getPass(0)->getTessellationHullProgramParameters()->setNamedConstant( "g_RenderMStrands", mRenderMStrands );
        }
        if (box->getName() == "RenderSStrands")
        {
            mRenderSStrands = !mRenderSStrands;
            
            MaterialPtr lMaterialPtr = MaterialManager::getSingleton().getByName( "Hair" ).staticCast<Material>();
            lMaterialPtr->getTechnique(0)->getPass(0)->getTessellationHullProgramParameters()->setNamedConstant( "g_RenderSStrands", mRenderSStrands );
        }
        if (box->getName() == "HWTessellation")
        {
            mHWTessellation = !mHWTessellation;
            
            MaterialPtr lMaterialPtr = MaterialManager::getSingleton().getByName( "Hair" ).staticCast<Material>();
            lMaterialPtr->getTechnique(0)->getPass(0)->getTessellationHullProgramParameters()->setNamedConstant( "g_HWTessellation", mHWTessellation );
        }
        if (box->getName() == "DynamicLOD" && mHWTessellation)
        {
            mDynamicLOD = !mDynamicLOD;
            
            MaterialPtr lMaterialPtr = MaterialManager::getSingleton().getByName( "Hair" ).staticCast<Material>();
            lMaterialPtr->getTechnique(0)->getPass(0)->getTessellationHullProgramParameters()->setNamedConstant( "g_DynamicLOD", mDynamicLOD );
        }
        if (box->getName() == "WindForce")
        {
            mAddWindForce = !mAddWindForce;
        }   
        if (box->getName() == "ComputeShader")
        {
            mComputeShader = !mComputeShader;
        }
        if (box->getName() == "SimulationLOD")
        {
            mSimulationLOD = !mSimulationLOD;
        }
        if (box->getName() == "Simulate")
        {
            mSimulate = !mSimulate;
        }
        if (box->getName() == "ShowCollision")
        {
            mShowCollision = !mShowCollision;
        }
        if (box->getName() == "ShowScene")
        {
            mShowScene = !mShowScene;
        }
    }

    void sliderMoved(Slider* slider)
    {
        if (slider->getName() == "tessellationLOD")
            if (!mDynamicLOD && mHWTessellation)
            {
                MaterialPtr lMaterialPtr = MaterialManager::getSingleton().getByName( "Hair" ).staticCast<Material>();
                lMaterialPtr->getTechnique(0)->getPass(0)->getTessellationHullProgramParameters()->setNamedConstant( "g_ManualLOD", slider->getValue() );
            }
        if (slider->getName() == "HairWidth")
            if (!mDynamicLOD && mHWTessellation)
            {
                MaterialPtr lMaterialPtr = MaterialManager::getSingleton().getByName( "Hair" ).staticCast<Material>();
                lMaterialPtr->getTechnique(0)->getPass(0)->getTessellationHullProgramParameters()->setNamedConstant( "g_HairWidth", slider->getValue() );
            }
        if (slider->getName()=="LODRate")
        {
            MaterialPtr lMaterialPtr = MaterialManager::getSingleton().getByName( "Hair" ).staticCast<Material>();
            lMaterialPtr->getTechnique(0)->getPass(0)->getTessellationHullProgramParameters()->setNamedConstant( "g_LODRate", slider->getValue() );
        }
        if (slider->getName()=="WindStrength")
            if (mAddWindForce)
            {
                MaterialPtr lMaterialPtr = MaterialManager::getSingleton().getByName( "Hair" ).staticCast<Material>();
                lMaterialPtr->getTechnique(0)->getPass(0)->getTessellationHullProgramParameters()->setNamedConstant( "g_WindStrength", slider->getValue() );
            }
    }

protected:

    void setupContent()
    {
        // create our main node to attach our entities to
        mObjectNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();

        mSceneMgr->setSkyBox(true, "Examples/SpaceSkyBox", 5000);  // set our skybox

        setupModels();
        setupLights();
        setupControls();
        
        // set our camera
        mCamera->setFOVy(Ogre::Degree(50.0));
        mCamera->setFOVy(Ogre::Degree(50.0));
        mCamera->setNearClipDistance(0.01f);
        mCamera->lookAt(Ogre::Vector3::ZERO);
        mCameraNode->setPosition(0, 0, 500);
        

        // Set our camera to orbit around the origin at a suitable distance
        mCameraMan->setStyle(CS_ORBIT);
        mCameraMan->setYawPitchDist(Radian(0), Radian(0), 400);

        mTrayMgr->showCursor();
    }

    void unloadResources()
    {

    }

    void setupModels()
    {

    }

    void setupLights()
    {
        mSceneMgr->setAmbientLight(ColourValue::Black); 
        mViewport->setBackgroundColour(ColourValue(0.41f, 0.41f, 0.41f));
    }

    void setupControls()
    {
        mTrayMgr->showCursor();

        // make room for the controls
        mTrayMgr->showLogo(TL_TOPRIGHT);
        mTrayMgr->showFrameStats(TL_TOPRIGHT);
        mTrayMgr->toggleAdvancedFrameStats();
        
        mTrayMgr->createCheckBox(TL_TOPLEFT, "Wire", "Render Wire Frame")->setChecked(false, false);
        mTrayMgr->createCheckBox(TL_TOPLEFT, "PlayAnimation", "Play Animation")->setChecked(false, false);
        mTrayMgr->createCheckBox(TL_TOPLEFT, "LoopAnimation", "Loop Animation")->setChecked(false, false);
        
        // create a menu to choose the model displayed
        mMeshMenu = mTrayMgr->createLongSelectMenu(TL_LEFT, "Mesh", "Mesh", 370, 290, 10);
        for (std::map<String, StringVector>::iterator it = mPossibilities.begin(); it != mPossibilities.end(); it++)
            mMeshMenu->addItem(it->first);
        
        mTrayMgr->createCheckBox(TL_TOPLEFT, "ShortHair", "Short Hair")->setChecked(false, false);
        mTrayMgr->createCheckBox(TL_TOPLEFT, "CurlyHair", "Curly Hair")->setChecked(false, false);
        mTrayMgr->createCheckBox(TL_TOPLEFT, "Shadows", "Shadows")->setChecked(true, false);
        mTrayMgr->createCheckBox(TL_TOPLEFT, "RenderMStrands", "Render M strands")->setChecked(true, false);
        mTrayMgr->createCheckBox(TL_TOPLEFT, "RenderSStrands", "Render S strands")->setChecked(true, false);
        mTrayMgr->createCheckBox(TL_TOPLEFT, "HWTessellation", "HW Tessellation")->setChecked(true, false);
        mTrayMgr->createCheckBox(TL_TOPLEFT, "DynamicLOD", "Dynamic LOD")->setChecked(true, false);
        
        mManualLOD = mTrayMgr->createThickSlider(TL_TOPLEFT, "tessellationLOD", "Manual tessellation LOD", 200, 40, 1, 50, 50);
        mManualLOD->show();
        
        mHairWidth = mTrayMgr->createThickSlider(TL_TOPLEFT, "HairWidth", "Hair Width", 200, 40, 1, 100, 100);
        mHairWidth->show();
        
        mLODRate = mTrayMgr->createThickSlider(TL_TOPLEFT, "LODRate", "LOD Rate", 200, 40, 1, 100, 100);
        mLODRate->show();
        
        mTrayMgr->createCheckBox(TL_TOPLEFT, "WindForce", "Add wind force")->setChecked(true, false);
        
        mWindStrength = mTrayMgr->createThickSlider(TL_TOPLEFT, "WindStrength", "Wind Strength", 200, 40, 0.01, 0.25, 25);
        mWindStrength->show();
        
        mTrayMgr->createCheckBox(TL_TOPLEFT, "ComputeShader", "Compute Shader")->setChecked(true, false);
        mTrayMgr->createCheckBox(TL_TOPLEFT, "SimulationLOD", "SimulationLOD")->setChecked(true, false);
        mTrayMgr->createCheckBox(TL_TOPLEFT, "Simulate", "Simulate")->setChecked(true, false);
        mTrayMgr->createCheckBox(TL_TOPLEFT, "ShowCollision", "Show Collision")->setChecked(false, false);
        mTrayMgr->createCheckBox(TL_TOPLEFT, "ShowScene", "Show Scene")->setChecked(true, false);

        // a friendly reminder
        StringVector names;
        names.push_back("Help");
        mTrayMgr->createParamsPanel(TL_TOPLEFT, "Help", 100, names)->setParamValue(0, "H/F1");
        
        mPlayAnimation = false;
        mLoopAnimation = false;
        mShortHair = false;
        mCurlyHair = false;
        mShadows = true;
        mRenderMStrands = true;
        mRenderSStrands = true;
        mHWTessellation = true;
        mDynamicLOD = true;
        mAddWindForce = true;
        mComputeShader = true;
        mSimulationLOD = true;
        mSimulate = true;
        mShowCollision = false;
        mShowScene = true;      
    }

    void cleanupContent()
    {
        // clean up properly to avoid interfering with subsequent samples
    }

    SelectMenu* mMeshMenu;
    SceneNode* mObjectNode;
    bool mPlayAnimation;
    bool mLoopAnimation;
    
    SelectMenu* mColorMenu;
    std::map<String, StringVector> mPossibilities;
    
    bool mShortHair;
    bool mCurlyHair;
    bool mShadows;
    bool mRenderMStrands;
    bool mRenderSStrands;
    bool mHWTessellation;
    bool mDynamicLOD;
    Slider* mManualLOD;
    Slider* mHairWidth;
    Slider* mLODRate;
    bool mAddWindForce;
    Slider* mWindStrength;
    bool mComputeShader;
    bool mSimulationLOD;
    bool mSimulate;
    bool mShowCollision;
    bool mShowScene;
};

#endif