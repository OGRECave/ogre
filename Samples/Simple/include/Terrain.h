/*
  -----------------------------------------------------------------------------
  This source file is part of OGRE
  (Object-oriented Graphics Rendering Engine)
  For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

  You may use this sample code for anything you like, it is not covered by the
  same license as the rest of the engine.
  -----------------------------------------------------------------------------
*/
#ifndef __Terrain_H__
#define __Terrain_H__

//#define PAGING

#define TERRAIN_PAGE_MIN_X 0
#define TERRAIN_PAGE_MIN_Y 0
#define TERRAIN_PAGE_MAX_X 0
#define TERRAIN_PAGE_MAX_Y 0

#include "OgrePageManager.h"
#include "OgreTerrain.h"
#include "OgreTerrainGroup.h"
#include "OgreTerrainMaterialGeneratorA.h"
#include "OgreTerrainPaging.h"
#include "OgreTerrainQuadTreeNode.h"
#include "SdkSample.h"

#define TERRAIN_FILE_PREFIX String("testTerrain")
#define TERRAIN_FILE_SUFFIX String("dat")
#define TERRAIN_WORLD_SIZE 12000.0f
#define TERRAIN_SIZE 513

#define SHADOWS_IN_LOW_LOD_MATERIAL false

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_Terrain : public SdkSample
{
public:
    Sample_Terrain()
        : mTerrainGlobals(0), mTerrainGroup(0), mTerrainPaging(0), mPageManager(0), mFly(false), mParallaxOcclusion(false),
          mFallVelocity(0), mMode(MODE_NORMAL), mLayerEdit(1), mBrushSizeTerrainSpace(0.02), mHeightUpdateCountDown(0),
          mTerrainPos(1000, 0, 5000), mTerrainsImported(false), mKeyPressed(0)

    {
        mInfo["Title"] = "Terrain";
        mInfo["Description"] = "Demonstrates use of the terrain rendering plugin.";
        mInfo["Thumbnail"] = "thumb_terrain.png";
        mInfo["Category"] = "Environment";
        mInfo["Help"] =
            "Left click and drag anywhere in the scene to look around. Let go again to show "
            "cursor and access widgets. Use WASD keys to move. Use +/- keys when in edit mode to change content.";

        // Update terrain at max 20fps
        mHeightUpdateRate = 1.0 / 20.0;
    }

    void doTerrainModify(Terrain* terrain, const Vector3& centrepos, Real timeElapsed)
    {
        Vector3 tsPos;
        terrain->getTerrainPosition(centrepos, &tsPos);
#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
        if (mKeyPressed == '+' || mKeyPressed == '-' || mKeyPressed == SDLK_KP_PLUS || mKeyPressed == SDLK_KP_MINUS)
        {
            switch (mMode)
            {
            case MODE_EDIT_HEIGHT:
            {
                // we need point coords
                Real terrainSize = (terrain->getSize() - 1);
                int startx = (tsPos.x - mBrushSizeTerrainSpace) * terrainSize;
                int starty = (tsPos.y - mBrushSizeTerrainSpace) * terrainSize;
                int endx = (tsPos.x + mBrushSizeTerrainSpace) * terrainSize;
                int endy = (tsPos.y + mBrushSizeTerrainSpace) * terrainSize;
                startx = std::max(startx, 0);
                starty = std::max(starty, 0);
                endx = std::min(endx, (int)terrainSize);
                endy = std::min(endy, (int)terrainSize);
                for (int y = starty; y <= endy; ++y)
                {
                    for (int x = startx; x <= endx; ++x)
                    {
                        Real tsXdist = (x / terrainSize) - tsPos.x;
                        Real tsYdist = (y / terrainSize) - tsPos.y;

                        Real weight = std::min((Real)1.0, Math::Sqrt(tsYdist * tsYdist + tsXdist * tsXdist) /
                                                              Real(0.5 * mBrushSizeTerrainSpace));
                        weight = 1.0 - (weight * weight);

                        float addedHeight = weight * 250.0 * timeElapsed;
                        float newheight;
                        if (mKeyPressed == '+' || mKeyPressed == SDLK_KP_PLUS)
                            newheight = terrain->getHeightAtPoint(x, y) + addedHeight;
                        else
                            newheight = terrain->getHeightAtPoint(x, y) - addedHeight;
                        terrain->setHeightAtPoint(x, y, newheight);
                    }
                }
                if (mHeightUpdateCountDown == 0)
                    mHeightUpdateCountDown = mHeightUpdateRate;
            }
            break;
            case MODE_EDIT_BLEND:
            {
                TerrainLayerBlendMap* layer = terrain->getLayerBlendMap(mLayerEdit);
                // we need image coords
                Real imgSize = terrain->getLayerBlendMapSize();
                int startx = (tsPos.x - mBrushSizeTerrainSpace) * imgSize;
                int starty = (tsPos.y - mBrushSizeTerrainSpace) * imgSize;
                int endx = (tsPos.x + mBrushSizeTerrainSpace) * imgSize;
                int endy = (tsPos.y + mBrushSizeTerrainSpace) * imgSize;
                startx = std::max(startx, 0);
                starty = std::max(starty, 0);
                endx = std::min(endx, (int)imgSize);
                endy = std::min(endy, (int)imgSize);
                for (int y = starty; y <= endy; ++y)
                {
                    for (int x = startx; x <= endx; ++x)
                    {
                        Real tsXdist = (x / imgSize) - tsPos.x;
                        Real tsYdist = (y / imgSize) - tsPos.y;

                        Real weight = std::min((Real)1.0, Math::Sqrt(tsYdist * tsYdist + tsXdist * tsXdist) /
                                                              Real(0.5 * mBrushSizeTerrainSpace));
                        weight = 1.0 - (weight * weight);

                        float paint = weight * timeElapsed;
                        uint32 imgY = imgSize - y;
                        float val;
                        if (mKeyPressed == '+' || mKeyPressed == SDLK_KP_PLUS)
                            val = layer->getBlendValue(x, imgY) + paint;
                        else
                            val = layer->getBlendValue(x, imgY) - paint;
                        val = Math::Clamp(val, 0.0f, 1.0f);
                        layer->setBlendValue(x, imgY, val);
                    }
                }

                layer->update();
            }
            break;
            case MODE_NORMAL:
            case MODE_COUNT:
                break;
            };
        }
#endif
    }
    bool frameRenderingQueued(const FrameEvent& evt) override
    {
        if (mMode != MODE_NORMAL)
        {
            // fire ray
            Ray ray;
            // ray = mCamera->getCameraToViewportRay(0.5, 0.5);
            ray = mTrayMgr->getCursorRay(mCamera);

            TerrainGroup::RayResult rayResult = mTerrainGroup->rayIntersects(ray);
            if (rayResult.hit)
            {
                mEditMarker->setVisible(true);
                mEditNode->setPosition(rayResult.position);

                // figure out which terrains this affects
                TerrainGroup::TerrainList terrainList;
                Real brushSizeWorldSpace = TERRAIN_WORLD_SIZE * mBrushSizeTerrainSpace;
                Sphere sphere(rayResult.position, brushSizeWorldSpace);
                mTerrainGroup->sphereIntersects(sphere, &terrainList);

                for (TerrainGroup::TerrainList::iterator ti = terrainList.begin(); ti != terrainList.end(); ++ti)
                    doTerrainModify(*ti, rayResult.position, evt.timeSinceLastFrame);
            }
            else
            {
                mEditMarker->setVisible(false);
            }
        }

        if (!mFly)
        {
            // clamp to terrain
            Vector3 camPos = mCameraNode->getPosition();
            Ray ray;
            ray.setOrigin(Vector3(camPos.x, mTerrainPos.y + 10000, camPos.z));
            ray.setDirection(Vector3::NEGATIVE_UNIT_Y);

            TerrainGroup::RayResult rayResult = mTerrainGroup->rayIntersects(ray);
            Real distanceAboveTerrain = 50;
            Real fallSpeed = 300;
            Real newy = camPos.y;
            if (rayResult.hit)
            {
                if (camPos.y > rayResult.position.y + distanceAboveTerrain)
                {
                    mFallVelocity += evt.timeSinceLastFrame * 20;
                    mFallVelocity = std::min(mFallVelocity, fallSpeed);
                    newy = camPos.y - mFallVelocity * evt.timeSinceLastFrame;
                }
                newy = std::max(rayResult.position.y + distanceAboveTerrain, newy);
                mCameraNode->setPosition(camPos.x, newy, camPos.z);
            }
        }

        if (mHeightUpdateCountDown > 0)
        {
            mHeightUpdateCountDown -= evt.timeSinceLastFrame;
            if (mHeightUpdateCountDown <= 0)
            {
                mTerrainGroup->update();
                mHeightUpdateCountDown = 0;
            }
        }

        //! [loading_label]
        if (mTerrainGroup->isDerivedDataUpdateInProgress())
        {
            mTrayMgr->moveWidgetToTray(mInfoLabel, TL_TOP, 0);
            mInfoLabel->show();
            if (mTerrainsImported)
            {
                mInfoLabel->setCaption("Building terrain, please wait...");
            }
            else
            {
                mInfoLabel->setCaption("Updating textures, patience...");
            }
        }
        else
        {
            mTrayMgr->removeWidgetFromTray(mInfoLabel);
            mInfoLabel->hide();
            if (mTerrainsImported)
            {
                // FIXME does not end up in the correct resource group
                // saveTerrains(true);
                mTerrainsImported = false;
            }
        }
        //! [loading_label]

        return SdkSample::frameRenderingQueued(evt); // don't forget the parent updates!
    }

    void saveTerrains(bool onlyIfModified) { mTerrainGroup->saveAllTerrains(onlyIfModified); }

    bool keyReleased(const KeyboardEvent& evt) override
    {
        mKeyPressed = 0;
        return SdkSample::keyReleased(evt);
    }

    bool keyPressed(const KeyboardEvent& e) override
    {
#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
        mKeyPressed = e.keysym.sym;

        switch (e.keysym.sym)
        {
        case 's':
            // CTRL-S to save
            if (e.keysym.mod & KMOD_CTRL)
            {
                saveTerrains(true);
            }
            else
                return SdkSample::keyPressed(e);
            break;
        case SDLK_F10:
            // dump
            {
                for (const auto& ti : mTerrainGroup->getTerrainSlots())
                {
                    TerrainGroup::TerrainSlot* ts = ti.second;
                    if (ts->instance && ts->instance->isLoaded())
                    {
                        ts->instance->_dumpTextures("terrain_" + std::to_string(ti.first), ".png");
                    }
                }
            }
            break;
            /*
              case SDLK_F7:
              // change terrain size
              if (mTerrainGroup->getTerrainSize() == 513)
              mTerrainGroup->setTerrainSize(1025);
              else
              mTerrainGroup->setTerrainSize(513);
              break;
              case SDLK_F8:
              // change terrain world size
              if (mTerrainGroup->getTerrainWorldSize() == TERRAIN_WORLD_SIZE)
              mTerrainGroup->setTerrainWorldSize(TERRAIN_WORLD_SIZE * 2);
              else
              mTerrainGroup->setTerrainWorldSize(TERRAIN_WORLD_SIZE);
              break;
            */
        default:
            return SdkSample::keyPressed(e);
        }
#endif

        return true;
    }

    void itemSelected(SelectMenu* menu) override
    {
        if (menu == mEditMenu)
        {
            mMode = (Mode)mEditMenu->getSelectionIndex();
        }
        else if (menu == mShadowsMenu)
        {
            mShadowMode = (ShadowMode)mShadowsMenu->getSelectionIndex();
            changeShadows();
        }
    }

    void checkBoxToggled(CheckBox* box) override
    {
        if (box == mFlyBox)
        {
            mFly = mFlyBox->isChecked();
        }
        if (box == mParallaxOcclusionBox)
        {
            mParallaxOcclusion = mParallaxOcclusionBox->isChecked();
            changeShadows();
        }
    }

protected:
    TerrainGlobalOptions* mTerrainGlobals;
    TerrainGroup* mTerrainGroup;
    bool mPaging;
    TerrainPaging* mTerrainPaging;
    PageManager* mPageManager;
#ifdef PAGING
    /// This class just pretends to provide prcedural page content to avoid page loading
    class DummyPageProvider : public PageProvider
    {
    public:
        bool prepareProceduralPage(Page* page, PagedWorldSection* section) { return true; }
        bool loadProceduralPage(Page* page, PagedWorldSection* section) { return true; }
        bool unloadProceduralPage(Page* page, PagedWorldSection* section) { return true; }
        bool unprepareProceduralPage(Page* page, PagedWorldSection* section) { return true; }
    };
    DummyPageProvider mDummyPageProvider;
#endif
    bool mFly;
    bool mParallaxOcclusion;
    Real mFallVelocity;
    enum Mode
    {
        MODE_NORMAL = 0,
        MODE_EDIT_HEIGHT = 1,
        MODE_EDIT_BLEND = 2,
        MODE_COUNT = 3
    };
    enum ShadowMode
    {
        SHADOWS_NONE = 0,
        SHADOWS_COLOUR = 1,
        SHADOWS_DEPTH = 2,
        SHADOWS_COUNT = 3
    };
    Mode mMode;
    ShadowMode mShadowMode;
    Ogre::uint8 mLayerEdit;
    Real mBrushSizeTerrainSpace;
    SceneNode* mEditNode;
    Entity* mEditMarker;
    Real mHeightUpdateCountDown;
    Real mHeightUpdateRate;
    Vector3 mTerrainPos;
    SelectMenu* mEditMenu;
    SelectMenu* mShadowsMenu;
    CheckBox* mFlyBox;
    CheckBox* mParallaxOcclusionBox;
    //! [infolabel]
    OgreBites::Label* mInfoLabel = nullptr;
    //! [infolabel]
    bool mTerrainsImported;
    ShadowCameraSetupPtr mPSSMSetup;

    typedef std::list<Entity*> EntityList;
    EntityList mHouseList;

    Keycode mKeyPressed;

    void defineTerrain(long x, long y, bool flat = false)
    {
        // if a file is available, use it
        // if not, generate file from import

        // Usually in a real project you'll know whether the compact terrain data is
        // available or not; I'm doing it this way to save distribution size

        if (flat)
        {
            mTerrainGroup->defineTerrain(x, y, 0.0f);
            return;
        }

        //! [define]
        String filename = mTerrainGroup->generateFilename(x, y);
        if (ResourceGroupManager::getSingleton().resourceExists(mTerrainGroup->getResourceGroup(), filename))
        {
            mTerrainGroup->defineTerrain(x, y);
        }
        else
        {
            Image img;
            getTerrainImage(x % 2 != 0, y % 2 != 0, img);
            mTerrainGroup->defineTerrain(x, y, &img);
            mTerrainsImported = true;
        }
        //! [define]
    }

    void getTerrainImage(bool flipX, bool flipY, Image& img)
    {
        //! [heightmap]
        img.load("terrain.png", mTerrainGroup->getResourceGroup());
        if (flipX)
            img.flipAroundY();
        if (flipY)
            img.flipAroundX();
        //! [heightmap]
    }

    void initBlendMaps(Terrain* terrain)
    {
        //! [blendmap]
        using namespace Ogre;
        TerrainLayerBlendMap* blendMap0 = terrain->getLayerBlendMap(1);
        TerrainLayerBlendMap* blendMap1 = terrain->getLayerBlendMap(2);
        float minHeight0 = 20;
        float fadeDist0 = 15;
        float minHeight1 = 70;
        float fadeDist1 = 15;
        float* pBlend0 = blendMap0->getBlendPointer();
        float* pBlend1 = blendMap1->getBlendPointer();
        for (uint16 y = 0; y < terrain->getLayerBlendMapSize(); ++y)
        {
            for (uint16 x = 0; x < terrain->getLayerBlendMapSize(); ++x)
            {
                Real tx, ty;

                blendMap0->convertImageToTerrainSpace(x, y, &tx, &ty);
                float height = terrain->getHeightAtTerrainPosition(tx, ty);

                *pBlend0++ = Math::saturate((height - minHeight0) / fadeDist0);
                *pBlend1++ = Math::saturate((height - minHeight1) / fadeDist1);
            }
        }
        blendMap0->dirty();
        blendMap1->dirty();
        blendMap0->update();
        blendMap1->update();
        //! [blendmap]
        // set up a colour map
        /*
          if (!terrain->getGlobalColourMapEnabled())
          {
          terrain->setGlobalColourMapEnabled(true);
          Image colourMap;
          colourMap.load("testcolourmap.jpg", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
          terrain->getGlobalColourMap()->loadImage(colourMap);
          }
        */
    }

    void configureTerrainDefaults(Light* l)
    {
        //! [configure_lod]
        mTerrainGlobals->setMaxPixelError(8);
        mTerrainGlobals->setCompositeMapDistance(3000);
        //! [configure_lod]

        // mTerrainGlobals->setUseRayBoxDistanceCalculation(true);
        // mTerrainGlobals->getDefaultMaterialGenerator()->setDebugLevel(1);
        // mTerrainGlobals->setLightMapSize(256);

        TerrainMaterialGeneratorA::SM2Profile* matProfile = static_cast<TerrainMaterialGeneratorA::SM2Profile*>(
            mTerrainGlobals->getDefaultMaterialGenerator()->getActiveProfile());

        // Disable the lightmap for OpenGL ES 2.0. The minimum number of samplers allowed is 8(as opposed to 16 on
        // desktop). Otherwise we will run over the limit by just one. The minimum was raised to 16 in GL ES 3.0.
        if (Ogre::Root::getSingletonPtr()->getRenderSystem()->getCapabilities()->getNumTextureUnits() < 9)
        {
            matProfile->setLightmapEnabled(false);
        }

        // Disable steep parallax by default
        matProfile->setLayerParallaxOcclusionMappingEnabled(false);

        //! [composite_lighting]
        // Important to set these so that the terrain knows what to use for baked (non-realtime) data
        mTerrainGlobals->setLightMapDirection(l->getDerivedDirection());
        mTerrainGlobals->setCompositeMapAmbient(mSceneMgr->getAmbientLight());
        mTerrainGlobals->setCompositeMapDiffuse(l->getDiffuseColour());
        //! [composite_lighting]
        // mTerrainGlobals->setCompositeMapAmbient(ColourValue::Red);

        // Configure default import settings for if we use imported image
        //! [import_settings]
        Ogre::Terrain::ImportData& defaultimp = mTerrainGroup->getDefaultImportSettings();
        defaultimp.inputScale = 600;
        defaultimp.minBatchSize = 33;
        defaultimp.maxBatchSize = 65;
        //! [import_settings]

        //! [tex_from_src]
        Image combined;
        combined.loadTwoImagesAsRGBA("Ground23_col.jpg", "Ground23_spec.png", "General");
        TextureManager::getSingleton().loadImage("Ground23_diffspec", "General", combined);
        //! [tex_from_src]

        //! [textures]
        defaultimp.layerList.resize(3);
        defaultimp.layerList[0].worldSize = 200;
        defaultimp.layerList[0].textureNames.push_back("Ground37_diffspec.dds");
        defaultimp.layerList[0].textureNames.push_back("Ground37_normheight.dds");
        defaultimp.layerList[1].worldSize = 200;
        defaultimp.layerList[1].textureNames.push_back("Ground23_diffspec"); // loaded from memory
        defaultimp.layerList[1].textureNames.push_back("Ground23_normheight.dds");
        defaultimp.layerList[2].worldSize = 400;
        defaultimp.layerList[2].textureNames.push_back("Rock20_diffspec.dds");
        defaultimp.layerList[2].textureNames.push_back("Rock20_normheight.dds");
        //! [textures]
    }

    void addTextureShadowDebugOverlay(TrayLocation loc, size_t num)
    {
        for (size_t i = 0; i < num; ++i)
        {
            TexturePtr shadowTex = mSceneMgr->getShadowTexture(i);
            addTextureDebugOverlay(loc, shadowTex, i);
        }
    }

    void changeShadows() { configureShadows(mShadowMode != SHADOWS_NONE, mShadowMode == SHADOWS_DEPTH); }

    void configureShadows(bool enabled, bool depthShadows)
    {
        auto matProfile = static_cast<TerrainMaterialGeneratorA::SM2Profile*>(
            mTerrainGlobals->getDefaultMaterialGenerator()->getActiveProfile());
        matProfile->setReceiveDynamicShadowsEnabled(false); // force regen if colour/ depth shadows change
        matProfile->setReceiveDynamicShadowsEnabled(enabled);
        matProfile->setReceiveDynamicShadowsLowLod(SHADOWS_IN_LOW_LOD_MATERIAL);

        RTShader::RenderState* schemRenderState = mShaderGenerator->getRenderState(MSN_SHADERGEN);
        if (auto srs = schemRenderState->getSubRenderState(RTShader::SRS_SHADOW_MAPPING))
        {
            schemRenderState->removeSubRenderState(srs);
        }

        if (enabled)
        {
            // General scene setup
            mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED);
            mSceneMgr->setShadowFarDistance(3000);

            // 3 textures per directional light (PSSM)
            mSceneMgr->setShadowTextureCountPerLightType(Ogre::Light::LT_DIRECTIONAL, 3);

            if (!mPSSMSetup)
            {
                // shadow camera setup
                PSSMShadowCameraSetup* pssmSetup = new PSSMShadowCameraSetup();
                pssmSetup->setSplitPadding(mCamera->getNearClipDistance() * 2);
                pssmSetup->calculateSplitPoints(3, mCamera->getNearClipDistance(), mSceneMgr->getShadowFarDistance());
                pssmSetup->setOptimalAdjustFactor(0, 2);
                pssmSetup->setOptimalAdjustFactor(1, 1);
                pssmSetup->setOptimalAdjustFactor(2, 0.5);

                mPSSMSetup.reset(pssmSetup);
            }
            mSceneMgr->setShadowCameraSetup(mPSSMSetup);

            if (depthShadows)
            {
                mSceneMgr->setShadowTextureCount(3);
                mSceneMgr->setShadowTextureConfig(0, 2048, 2048, PF_DEPTH16);
                mSceneMgr->setShadowTextureConfig(1, 1024, 1024, PF_DEPTH16);
                mSceneMgr->setShadowTextureConfig(2, 1024, 1024, PF_DEPTH16);
                mSceneMgr->setShadowTextureSelfShadow(true);
                mSceneMgr->setShadowCasterRenderBackFaces(true);

                auto subRenderState = mShaderGenerator->createSubRenderState(RTShader::SRS_SHADOW_MAPPING);
                subRenderState->setParameter("split_points",
                                             static_cast<PSSMShadowCameraSetup*>(mPSSMSetup.get())->getSplitPoints());
                schemRenderState->addTemplateSubRenderState(subRenderState);
            }
            else
            {
                mSceneMgr->setShadowTextureCount(3);
                mSceneMgr->setShadowTextureConfig(0, 2048, 2048, PF_X8B8G8R8);
                mSceneMgr->setShadowTextureConfig(1, 1024, 1024, PF_X8B8G8R8);
                mSceneMgr->setShadowTextureConfig(2, 1024, 1024, PF_X8B8G8R8);
                mSceneMgr->setShadowTextureSelfShadow(false);
                mSceneMgr->setShadowCasterRenderBackFaces(false);
            }

            matProfile->setReceiveDynamicShadowsPSSM(static_cast<PSSMShadowCameraSetup*>(mPSSMSetup.get()));

            // addTextureShadowDebugOverlay(TL_RIGHT, 3);
        }
        else
        {
            mSceneMgr->setShadowTechnique(SHADOWTYPE_NONE);
        }

        // Update parallax occlusion
        matProfile->setLayerParallaxOcclusionMappingEnabled(mParallaxOcclusion);

        mShaderGenerator->invalidateScheme(MSN_SHADERGEN);
    }

    /*-----------------------------------------------------------------------------
      | Extends setupView to change some initial camera settings for this sample.
      -----------------------------------------------------------------------------*/
    void setupView() override
    {
        SdkSample::setupView();
        // Make this viewport work with shader generator scheme.
        mViewport->setMaterialScheme(MSN_SHADERGEN);

        //! [camera_setup]
        mCameraNode->setPosition(mTerrainPos + Vector3(1683, 50, 2116));
        mCameraNode->lookAt(Vector3(1963, 50, 1660), Node::TS_PARENT);
        mCamera->setNearClipDistance(40); // tight near plane important for shadows
        mCamera->setFarClipDistance(50000);
        //! [camera_setup]

        //! [camera_inf]
        mCamera->setFarClipDistance(0); // enable infinite far clip distance
        //! [camera_inf]
    }

    void setupControls()
    {
        mTrayMgr->showCursor();

        // make room for the controls
        mTrayMgr->showLogo(TL_TOPRIGHT);
        mTrayMgr->showFrameStats(TL_TOPRIGHT);
        mTrayMgr->toggleAdvancedFrameStats();

        //! [infolabel_create]
        mInfoLabel = mTrayMgr->createLabel(TL_TOP, "TInfo", "", 350);
        //! [infolabel_create]

        mEditMenu = mTrayMgr->createLongSelectMenu(TL_BOTTOM, "EditMode", "Edit Mode", 370, 250, 3);
        mEditMenu->addItem("None");
        mEditMenu->addItem("Elevation");
        mEditMenu->addItem("Blend");
        mEditMenu->selectItem(0); // no edit mode

        mFlyBox = mTrayMgr->createCheckBox(TL_BOTTOM, "Fly", "Fly");
        mFlyBox->setChecked(false, false);

        mParallaxOcclusionBox = mTrayMgr->createCheckBox(TL_BOTTOM, "ParallaxOcclusion", "Parallax Occlusion Mapping");
        mParallaxOcclusionBox->setChecked(false, false);

        mShadowsMenu = mTrayMgr->createLongSelectMenu(TL_BOTTOM, "Shadows", "Shadows", 370, 250, 3);
        mShadowsMenu->addItem("None");
        mShadowsMenu->addItem("Colour Shadows");
        mShadowsMenu->addItem("Depth Shadows");
        mShadowsMenu->selectItem(0); // no edit mode

        // a friendly reminder
        StringVector names;
        names.push_back("Help");
        mTrayMgr->createParamsPanel(TL_TOPLEFT, "Help", 100, names)->setParamValue(0, "H/F1");
    }

    void setupContent() override
    {
        //! [global_opts]
        mTerrainGlobals = new Ogre::TerrainGlobalOptions();
        //! [global_opts]

        mEditMarker = mSceneMgr->createEntity("editMarker", "sphere.mesh");
        mEditNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        mEditNode->attachObject(mEditMarker);
        mEditNode->setScale(0.05, 0.05, 0.05);

        setupControls();

        mCameraMan->setTopSpeed(50);

        setDragLook(true);

#if OGRE_PLATFORM != OGRE_PLATFORM_ANDROID
        MaterialManager::getSingleton().setDefaultTextureFiltering(TFO_ANISOTROPIC);
        MaterialManager::getSingleton().setDefaultAnisotropy(8);
#endif

        ColourValue fadeColour(0.7, 0.7, 0.8);
        //! [linear_fog]
        mSceneMgr->setFog(Ogre::FOG_LINEAR, fadeColour, 0, 2000, 10000);
        //! [linear_fog]

        LogManager::getSingleton().setMinLogLevel(LML_TRIVIAL);

        //! [light]
        Ogre::Light* l = mSceneMgr->createLight();
        l->setType(Ogre::Light::LT_DIRECTIONAL);
        l->setDiffuseColour(ColourValue::White);
        l->setSpecularColour(ColourValue(0.4, 0.4, 0.4));

        Ogre::SceneNode* ln = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        ln->setDirection(Vector3(0.55, -0.3, 0.75).normalisedCopy());
        ln->attachObject(l);
        //! [light]
        mSceneMgr->setAmbientLight(ColourValue(0.2, 0.2, 0.2));

        //! [terrain_create]
        mTerrainGroup = new Ogre::TerrainGroup(mSceneMgr, Ogre::Terrain::ALIGN_X_Z, TERRAIN_SIZE, TERRAIN_WORLD_SIZE);
        mTerrainGroup->setFilenameConvention(TERRAIN_FILE_PREFIX, TERRAIN_FILE_SUFFIX);
        mTerrainGroup->setOrigin(mTerrainPos);
        //! [terrain_create]

        configureTerrainDefaults(l);
#ifdef PAGING
        // Paging setup
        mPageManager = OGRE_NEW PageManager();
        // Since we're not loading any pages from .page files, we need a way just
        // to say we've loaded them without them actually being loaded
        mPageManager->setPageProvider(&mDummyPageProvider);
        mPageManager->addCamera(mCamera);
        mTerrainPaging = OGRE_NEW TerrainPaging(mPageManager);
        PagedWorld* world = mPageManager->createWorld();
        mTerrainPaging->createWorldSection(world, mTerrainGroup, 2000, 3000, TERRAIN_PAGE_MIN_X, TERRAIN_PAGE_MIN_Y,
                                           TERRAIN_PAGE_MAX_X, TERRAIN_PAGE_MAX_Y);
#else
        //! [define_loop]
        for (long x = TERRAIN_PAGE_MIN_X; x <= TERRAIN_PAGE_MAX_X; ++x)
            for (long y = TERRAIN_PAGE_MIN_Y; y <= TERRAIN_PAGE_MAX_Y; ++y)
                defineTerrain(x, y);
        // sync load since we want everything in place when we start
        mTerrainGroup->loadAllTerrains(true);
        //! [define_loop]
#endif

        //! [init_blend]
        if (mTerrainsImported)
        {
            for (const auto& ti : mTerrainGroup->getTerrainSlots())
            {
                initBlendMaps(ti.second->instance);
            }
        }

        mTerrainGroup->freeTemporaryResources();
        //! [init_blend]

        // create a few entities on the terrain
        Entity* e = mSceneMgr->createEntity("tudorhouse.mesh");
        Vector3 entPos(mTerrainPos.x + 2043, 0, mTerrainPos.z + 1715);
        Quaternion rot;
        entPos.y = mTerrainGroup->getHeightAtWorldPosition(entPos) + 65.5 + mTerrainPos.y;
        rot.FromAngleAxis(Degree(Math::RangeRandom(-180, 180)), Vector3::UNIT_Y);
        SceneNode* sn = mSceneMgr->getRootSceneNode()->createChildSceneNode(entPos, rot);
        sn->setScale(Vector3(0.12, 0.12, 0.12));
        sn->attachObject(e);
        mHouseList.push_back(e);

        e = mSceneMgr->createEntity("tudorhouse.mesh");
        entPos = Vector3(mTerrainPos.x + 1850, 0, mTerrainPos.z + 1478);
        entPos.y = mTerrainGroup->getHeightAtWorldPosition(entPos) + 65.5 + mTerrainPos.y;
        rot.FromAngleAxis(Degree(Math::RangeRandom(-180, 180)), Vector3::UNIT_Y);
        sn = mSceneMgr->getRootSceneNode()->createChildSceneNode(entPos, rot);
        sn->setScale(Vector3(0.12, 0.12, 0.12));
        sn->attachObject(e);
        mHouseList.push_back(e);

        e = mSceneMgr->createEntity("tudorhouse.mesh");
        entPos = Vector3(mTerrainPos.x + 1970, 0, mTerrainPos.z + 2180);
        entPos.y = mTerrainGroup->getHeightAtWorldPosition(entPos) + 65.5 + mTerrainPos.y;
        rot.FromAngleAxis(Degree(Math::RangeRandom(-180, 180)), Vector3::UNIT_Y);
        sn = mSceneMgr->getRootSceneNode()->createChildSceneNode(entPos, rot);
        sn->setScale(Vector3(0.12, 0.12, 0.12));
        sn->attachObject(e);
        mHouseList.push_back(e);

        //! [skybox]
        mSceneMgr->setSkyBox(true, "Examples/CloudyNoonSkyBox");
        //! [skybox]
    }

    void _shutdown() override
    {
        if (mTerrainPaging)
        {
            OGRE_DELETE mTerrainPaging;
            mTerrainPaging = 0;
            OGRE_DELETE mPageManager;
            mPageManager = 0;
        }
        else if (mTerrainGroup)
        {
            OGRE_DELETE mTerrainGroup;
            mTerrainGroup = 0;
        }

        if (mTerrainGlobals)
        {
            OGRE_DELETE mTerrainGlobals;
            mTerrainGlobals = 0;
        }

        mHouseList.clear();

        SdkSample::_shutdown();
    }
};

#endif