#ifndef __Island_H__
#define __Island_H__

#include "SdkSample.h"
#include "OgreImage.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_Island : public SdkSample
{
public:

    Sample_Island()
    {
        mInfo["Title"] = "Island";
        mInfo["Description"] = "Sample for terrain, water tessellation and the use of displacement mapping";
        mInfo["Thumbnail"] = "thumb_tesselation.png";
        mInfo["Category"] = "Unsorted";
        mInfo["Help"] = "Top Left: Multi-frame\nTop Right: Scrolling\nBottom Left: Rotation\nBottom Right: Scaling";
    }

    void testCapabilities(const RenderSystemCapabilities* caps)
    {
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "This sample is not yet finished."
                " Sorry!", "Sample_Island::testCapabilities");
        if (!caps->hasCapability(RSC_TESSELLATION_PROGRAM))
        {
            OGRE_EXCEPT(Exception::ERR_INVALID_STATE, "Your graphics card does not support tesselation shaders. Sorry!",
                "Sample_Island:testCapabilities");
        }
        if (!GpuProgramManager::getSingleton().isSyntaxSupported("vs_5_0") &&
            !GpuProgramManager::getSingleton().isSyntaxSupported("hs_5_0") &&
            !GpuProgramManager::getSingleton().isSyntaxSupported("ds_5_0") &&
            !GpuProgramManager::getSingleton().isSyntaxSupported("ps_5_0") &&
            !GpuProgramManager::getSingleton().isSyntaxSupported("hlsl"))
        {
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your card does not support the shader model 5.0 needed for this sample, "
                "so you cannot run this sample. Sorry!", "Sample_Island::testCapabilities");
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
        if (box->getName() == "Tessellation")
        {
            g_UseDynamicLOD = !g_UseDynamicLOD;
            
            MaterialPtr lMaterialPtr = MaterialManager::getSingleton().getByName( "Island" ).staticCast<Material>();
            lMaterialPtr->getTechnique(0)->getPass(0)->getTessellationHullProgramParameters()->setNamedConstant( "g_UseDynamicLOD", g_UseDynamicLOD );
        }
        if (box->getName() == "FrustumCull")
        {
            g_FrustumCullInHS = !g_FrustumCullInHS;
            
            MaterialPtr lMaterialPtr = MaterialManager::getSingleton().getByName( "Island" ).staticCast<Material>();
            lMaterialPtr->getTechnique(0)->getPass(0)->getTessellationHullProgramParameters()->setNamedConstant( "g_FrustumCullInHS", g_FrustumCullInHS );
        }
        if (box->getName() == "RenderRefraction")
        {
            g_RenderCaustics = !g_RenderCaustics;
            
            MaterialPtr lMaterialPtr = MaterialManager::getSingleton().getByName( "Island" ).staticCast<Material>();
            lMaterialPtr->getTechnique(0)->getPass(0)->getTessellationHullProgramParameters()->setNamedConstant( "g_RenderCaustics", g_RenderCaustics );
        }
    }

    void sliderMoved(Slider* slider)
    {
        if (slider->getName() == "tessellationLOD")
        {
            MaterialPtr lMaterialPtr = MaterialManager::getSingleton().getByName( "Island" ).staticCast<Material>();
            lMaterialPtr->getTechnique(0)->getPass(0)->getTessellationHullProgramParameters()->setNamedConstant( "g_DynamicTessFactor", slider->getValue() );
        }
        if (slider->getName() == "tessellationFactor")
        {
            MaterialPtr lMaterialPtr = MaterialManager::getSingleton().getByName( "Island" ).staticCast<Material>();
            lMaterialPtr->getTechnique(0)->getPass(0)->getTessellationHullProgramParameters()->setNamedConstant( "g_StaticTessFactor", slider->getValue() );        
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
        mTrayMgr->createCheckBox(TL_TOPLEFT, "Tessellation", "Use Dynamic Tessellation LOD")->setChecked(true, false);
        mTrayMgr->createCheckBox(TL_TOPLEFT, "FrustumCull", "Use Frustum Cull in HS")->setChecked(true, false);
        mTrayMgr->createCheckBox(TL_TOPLEFT, "RenderRefraction", "Render Refraction Caustics")->setChecked(true, false);
        
        mTessellationLOD = mTrayMgr->createThickSlider(TL_TOPLEFT, "tessellationLOD", "Dynamic tessellation LOD", 200, 40, 1, 100, 100);
        mTessellationLOD->show();

        mTessellationFactor = mTrayMgr->createThickSlider(TL_TOPLEFT, "tessellationFactor", "Static tessellation factor", 200, 40, 1, 64, 64);
        mTessellationFactor->show();

        // a friendly reminder
        StringVector names;
        names.push_back("Help");
        mTrayMgr->createParamsPanel(TL_TOPLEFT, "Help", 100, names)->setParamValue(0, "H/F1");
        
        g_UseDynamicLOD = true;
        g_FrustumCullInHS = true;
        g_RenderCaustics = true;
    }

    void cleanupContent()
    {
        // clean up properly to avoid interfering with subsequent samples
    }

    SceneNode* mObjectNode;
    Slider*     mTessellationLOD;
    Slider*     mTessellationFactor;
    bool g_UseDynamicLOD;
    bool g_FrustumCullInHS;
    bool g_RenderCaustics;
};

#endif