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

#ifndef __TerrainTessellation_H__
#define __TerrainTessellation_H__

#define TERRAIN_PAGE_MIN_X 0
#define TERRAIN_PAGE_MIN_Y 0
#define TERRAIN_PAGE_MAX_X 0
#define TERRAIN_PAGE_MAX_Y 0

#include "SdkSample.h"
#include "OgreTerrain.h"
#include "OgreTerrainGroup.h"
#include "OgreTerrainQuadTreeNode.h"
#include "TerrainTessellationMaterialGenerator.h"
#include "OgreTerrainPaging.h"
#include "OgrePageManager.h"
#include "OgreImage.h"
#include "OgreTerrainAutoUpdateLod.h"

#define TERRAIN_FILE_PREFIX String("testTerrain")
#define TERRAIN_FILE_SUFFIX String("dat")
#define TERRAIN_WORLD_SIZE 12000.0f
#define TERRAIN_SIZE 513

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_TerrainTessellation : public SdkSample
{
public:

    Sample_TerrainTessellation()
        : mTerrainGroup(0)
        , mTerrainPaging(0)
        , mPageManager(0)
        //, mFallVelocity(0)
        //, mMode(MODE_NORMAL)
        //, mLayerEdit(1)
        //, mBrushSizeTerrainSpace(0.02)
        //, mHeightUpdateCountDown(0)
        , mTerrainsImported(false)
        , mFly(false)
        , mTerrainPos(1,1,1)
    {
        mInfo["Title"] = "TerrainTessellation";
        mInfo["Description"] = "Sample for terrain tessellation and the use of displacement mapping";
        mInfo["Thumbnail"] = "thumb_tesselation.png";
        mInfo["Category"] = "Unsorted";
        mInfo["Help"] = "Top Left: Multi-frame\nTop Right: Scrolling\nBottom Left: Rotation\nBottom Right: Scaling";
    }

    void testCapabilities(const RenderSystemCapabilities* caps)
    {
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "This sample is not yet finished."
                " Sorry!", "Sample_TerrainTessellation::testCapabilities");
        if (!caps->hasCapability(RSC_TESSELLATION_HULL_PROGRAM) || !caps->hasCapability(RSC_TESSELLATION_DOMAIN_PROGRAM))
        {
            OGRE_EXCEPT(Exception::ERR_INVALID_STATE, "Your graphics card does not support tesselation shaders. Sorry!",
                "Sample_TerrainTessellation:testCapabilities");
        }
        if (!GpuProgramManager::getSingleton().isSyntaxSupported("vs_5_0") &&
            !GpuProgramManager::getSingleton().isSyntaxSupported("hs_5_0") &&
            !GpuProgramManager::getSingleton().isSyntaxSupported("ds_5_0") &&
            !GpuProgramManager::getSingleton().isSyntaxSupported("ps_5_0") &&
            !GpuProgramManager::getSingleton().isSyntaxSupported("hlsl"))
        {
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your card does not support the shader model 5.0 needed for this sample, "
                "so you cannot run this sample. Sorry!", "Sample_TerrainTessellation::testCapabilities");
        }
    }

    bool frameRenderingQueued(const FrameEvent& evt)
    {
        if (!mFly)
        {
            // clamp to terrain
            Vector3 camPos = mCamera->getPosition();
            Ray ray;
            ray.setOrigin(Vector3(camPos.x, mTerrainPos.y + 10000, camPos.z));
            ray.setDirection(Vector3::NEGATIVE_UNIT_Y);

            TerrainGroup::RayResult rayResult = mTerrainGroup->rayIntersects(ray);
            const Real distanceAboveTerrain = 50;
            if (rayResult.hit)
                mCameraNode->setPosition(camPos.x, rayResult.position.y + distanceAboveTerrain, camPos.z);
        }

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
            // disable tessellation
        }
        if (box == mFlyBox)
        {
            mFly = mFlyBox->isChecked();
        }
    }

    void sliderMoved(Slider* slider)
    {
        if (slider->getName() == "tessellationAmount")
        {
            MaterialPtr lMaterialPtr = MaterialManager::getSingleton().getByName( "TerrainTessellation" ).staticCast<Material>();
            lMaterialPtr->getTechnique(0)->getPass(0)->getTessellationHullProgramParameters()->setNamedConstant( "g_tessellationAmount", slider->getValue() );
        }
        if (slider->getName() == "ridgeOctaves")
        {
            MaterialPtr lMaterialPtr = MaterialManager::getSingleton().getByName( "TerrainTessellation" ).staticCast<Material>();
            lMaterialPtr->getTechnique(0)->getPass(0)->getTessellationHullProgramParameters()->setNamedConstant( "g_ridgeOctaves", slider->getValue() );
        }
        if (slider->getName() == "fBmOctaves")
        {
            MaterialPtr lMaterialPtr = MaterialManager::getSingleton().getByName( "TerrainTessellation" ).staticCast<Material>();
            lMaterialPtr->getTechnique(0)->getPass(0)->getTessellationHullProgramParameters()->setNamedConstant( "g_fBmOctaves", slider->getValue() );
        }
        if (slider->getName() == "TwistOctaves")
        {
            MaterialPtr lMaterialPtr = MaterialManager::getSingleton().getByName( "TerrainTessellation" ).staticCast<Material>();
            lMaterialPtr->getTechnique(0)->getPass(0)->getTessellationHullProgramParameters()->setNamedConstant( "g_TwistOctaves", slider->getValue() );
        }
        if (slider->getName() == "detailNoiseScale")
        {
            MaterialPtr lMaterialPtr = MaterialManager::getSingleton().getByName( "TerrainTessellation" ).staticCast<Material>();
            lMaterialPtr->getTechnique(0)->getPass(0)->getTessellationHullProgramParameters()->setNamedConstant( "g_detailNoiseScale", slider->getValue() );
        }
        if (slider->getName() == "targetTrianglesWidth")
        {
            MaterialPtr lMaterialPtr = MaterialManager::getSingleton().getByName( "TerrainTessellation" ).staticCast<Material>();
            lMaterialPtr->getTechnique(0)->getPass(0)->getTessellationHullProgramParameters()->setNamedConstant( "g_targetTrianglesWidth", slider->getValue() );
        }
    }

protected:

    void configureTerrainDefaults(Light* l)
    {
        // Configure global
        mTerrainGlobals->setMaxPixelError(8);
        // testing composite map
        mTerrainGlobals->setCompositeMapDistance(3000);
        //mTerrainGlobals->setUseRayBoxDistanceCalculation(true);
        mTerrainGlobals->getDefaultMaterialGenerator()->setLightmapEnabled(false);

        mTerrainGlobals->setCompositeMapAmbient(mSceneMgr->getAmbientLight());
        //mTerrainGlobals->setCompositeMapAmbient(ColourValue::Red);
        mTerrainGlobals->setCompositeMapDiffuse(l->getDiffuseColour());

        // Configure default import settings for if we use imported image
        Terrain::ImportData& defaultimp = mTerrainGroup->getDefaultImportSettings();
        defaultimp.terrainSize = TERRAIN_SIZE;
        defaultimp.worldSize = TERRAIN_WORLD_SIZE;
        defaultimp.inputScale = 600;
        defaultimp.minBatchSize = 33;
        defaultimp.maxBatchSize = 65;
        // textures
        defaultimp.layerList.resize(3);
        defaultimp.layerList[0].worldSize = 100;
        defaultimp.layerList[0].textureNames.push_back("dirt_grayrocky_diffusespecular.dds");
        defaultimp.layerList[0].textureNames.push_back("dirt_grayrocky_normalheight.dds");
        defaultimp.layerList[1].worldSize = 30;
        defaultimp.layerList[1].textureNames.push_back("grass_green-01_diffusespecular.dds");
        defaultimp.layerList[1].textureNames.push_back("grass_green-01_normalheight.dds");
        defaultimp.layerList[2].worldSize = 200;
        defaultimp.layerList[2].textureNames.push_back("growth_weirdfungus-03_diffusespecular.dds");
        defaultimp.layerList[2].textureNames.push_back("growth_weirdfungus-03_normalheight.dds");

        // Init custom materialgenerator
        TerrainMaterialGeneratorPtr terrainMaterialGenerator;

        // Set Ogre Material  with the name "TerrainMaterial" in constructor
        TerrainTessellationMaterialGenerator *terrainMaterial = OGRE_NEW TerrainTessellationMaterialGenerator("Ogre/TerrainTessellation/Terrain");         
        terrainMaterialGenerator.bind( terrainMaterial );  
                       
        mTerrainGlobals->setDefaultMaterialGenerator( terrainMaterialGenerator );
    }

    void setupContent()
    {
        // create our main node to attach our entities to
        mObjectNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        mSceneMgr->setSkyBox(true, "Examples/SpaceSkyBox", 5000);  // set our skybox

        setupLights();
        setupControls();
        
        // set our camera
        mCamera->setFOVy(Ogre::Degree(50.0));
        mCamera->setFOVy(Ogre::Degree(50.0));
        mCamera->setNearClipDistance(0.01f);
        mCamera->lookAt(Ogre::Vector3::ZERO);
        mCameraNode->setPosition(0, 0, 500);
        mCameraMan->setTopSpeed(100);

        setDragLook(true);

        // Set our camera to orbit around the origin at a suitable distance
        mCameraMan->setStyle(CS_ORBIT);
        mCameraMan->setYawPitchDist(Radian(0), Radian(0), 400);

        MaterialManager::getSingleton().setDefaultTextureFiltering(TFO_ANISOTROPIC);
        MaterialManager::getSingleton().setDefaultAnisotropy(7);

        mSceneMgr->setFog(FOG_LINEAR, ColourValue(0.7, 0.7, 0.8), 0, 4000, 10000);

        mTrayMgr->showCursor();
        
        mTerrainGlobals = OGRE_NEW TerrainGlobalOptions();

        mTerrainGroup = OGRE_NEW TerrainGroup(mSceneMgr, Terrain::ALIGN_X_Z, TERRAIN_SIZE, TERRAIN_WORLD_SIZE);
        mTerrainGroup->setFilenameConvention(TERRAIN_FILE_PREFIX, TERRAIN_FILE_SUFFIX);
        mTerrainGroup->setOrigin(mTerrainPos);
        mTerrainGroup->setAutoUpdateLod( TerrainAutoUpdateLodFactory::getAutoUpdateLod(BY_DISTANCE) ); // probably will do it in tessellation stages.
        
        Vector3 lightdir(0.55, -0.3, 0.75);
        lightdir.normalise();

        Light* l = mSceneMgr->createLight("tstLight");
        l->setType(Light::LT_DIRECTIONAL);
        l->setDirection(lightdir);
        l->setDiffuseColour(ColourValue::White);
        l->setSpecularColour(ColourValue(0.4, 0.4, 0.4));

        configureTerrainDefaults(l);

        mTerrainGroup->freeTemporaryResources();
    }

    void unloadResources()
    {

    }

    void setupView()
    {
        SdkSample::setupView();
        // put camera at world center, so that it's difficult to reach the edge
        Vector3 worldCenter(0,0,0);
        mCameraNode->setPosition(mTerrainPos+worldCenter);
        mCamera->lookAt(mTerrainPos);
        mCamera->setNearClipDistance(0.1);
        mCamera->setFarClipDistance(50000);

        mCamera->setFarClipDistance(0);   // enable infinite far clip distance
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

        mTrayMgr->createCheckBox(TL_TOPLEFT, "Wire", "Wire Frame")->setChecked(false, false);
        mTrayMgr->createCheckBox(TL_TOPLEFT, "Tessellation", "Hardware Tessellation")->setChecked(true, false);
        
        mTessellationAmount = mTrayMgr->createThickSlider(TL_TOPLEFT, "tessellationAmount", "Tessellation Amount", 200, 40, 1, 8, 8);
        mTessellationAmount->show();

        mRidgeOctaves = mTrayMgr->createThickSlider(TL_TOPLEFT, "ridgeOctaves", "Ridge Octaves", 200, 40, 0, 15, 15);
        mRidgeOctaves->show();

        mfBmOctaves = mTrayMgr->createThickSlider(TL_TOPLEFT, "fBmOctaves", "fBm Octaves", 200, 40, 0, 15, 15);
        mfBmOctaves->show();

        mTwistOctaves = mTrayMgr->createThickSlider(TL_TOPLEFT, "TwistOctaves", "Twist Octaves", 200, 40, 0, 15, 15);
        mTwistOctaves->show();

        mDetailNoiseScale = mTrayMgr->createThickSlider(TL_TOPLEFT, "detailNoiseScale", "Detail noise scale", 200, 40, 0, 2.0, 200);
        mDetailNoiseScale->show();

        mTargetTrianglesWidth = mTrayMgr->createThickSlider(TL_TOPLEFT, "targetTrianglesWidth", "Target triangles width", 200, 40, 1, 50, 50);
        mTargetTrianglesWidth->show();

        mFlyBox = mTrayMgr->createCheckBox(TL_BOTTOM, "Fly", "Fly");
        mFlyBox->setChecked(false, true);

        // a friendly reminder
        StringVector names;
        names.push_back("Help");
        mTrayMgr->createParamsPanel(TL_TOPLEFT, "Help", 100, names)->setParamValue(0, "H/F1");
    }

    void cleanupContent()
    {
        // clean up properly to avoid interfering with subsequent samples
        if (mPaging)
        {
            OGRE_DELETE mTerrainPaging;
            OGRE_DELETE mPageManager;
        }
        else
            OGRE_DELETE mTerrainGroup;

        OGRE_DELETE mTerrainGlobals;
    }

    SceneNode* mObjectNode;
    Slider*     mTessellationAmount;
    Slider*     mRidgeOctaves;
    Slider*     mfBmOctaves;
    Slider*     mTwistOctaves;
    Slider*     mDetailNoiseScale;
    Slider*     mTargetTrianglesWidth;
    CheckBox* mFlyBox;

    TerrainGlobalOptions* mTerrainGlobals;
    TerrainGroup* mTerrainGroup;
    bool mPaging;
    TerrainPaging* mTerrainPaging;
    PageManager* mPageManager;
    
    bool mTerrainsImported;

    bool mFly;
    Vector3 mTerrainPos;
    
};

#endif