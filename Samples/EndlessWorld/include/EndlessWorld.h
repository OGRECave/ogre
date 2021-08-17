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
#ifndef __EndlessWorld_H__
#define __EndlessWorld_H__

#define ENDLESS_PAGING

// max range for a int16
#define ENDLESS_PAGE_MIN_X (-0x7FFF)
#define ENDLESS_PAGE_MIN_Y (-0x7FFF)
#define ENDLESS_PAGE_MAX_X 0x7FFF
#define ENDLESS_PAGE_MAX_Y 0x7FFF

#include "SdkSample.h"
#include "OgreTerrain.h"
#include "OgreTerrainGroup.h"
#include "OgreTerrainQuadTreeNode.h"
#include "OgreTerrainMaterialGeneratorA.h"
#include "OgreTerrainPagedWorldSection.h"
#include "OgreTerrainAutoUpdateLod.h"
#include "OgreTerrainPaging.h"
#include "OgrePageManager.h"
#include "PerlinNoiseTerrainGenerator.h"

#define ENDLESS_TERRAIN_FILE_PREFIX String("EndlessWorldTerrain")
#define ENDLESS_TERRAIN_FILE_SUFFIX String("dat")
#define TERRAIN_WORLD_SIZE 12000.0f
#define TERRAIN_SIZE 513
#define HOLD_LOD_DISTANCE 3000.0
#define USE_PERLIN_DEFINER 1

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_EndlessWorld : public SdkSample
{
public:

	Sample_EndlessWorld()
		: mTerrainGlobals(0)
        , mTerrainGroup(0)
		, mTerrainPaging(0)
		, mPageManager(0)
		, mPagedWorld(0)
		, mTerrainPagedWorldSection(0)
		, mPerlinNoiseTerrainGenerator(0)
        , mLodStatus(false)
		, mFly(true)
		, mFallVelocity(0)
        , mTerrainPos(0,0,0)
        , mLodInfoOverlay(0)
        , mLodInfoOverlayContainer(0)

	{
		mInfo["Title"] = "Endless World";
		mInfo["Description"] = "Demonstrates use of the terrain plugin with paging option.";
		mInfo["Thumbnail"] = "thumb_terrain.png";
		mInfo["Category"] = "Environment";
		mInfo["Help"] = "Left click and drag anywhere in the scene to look around. Let go again to show "
			"cursor and access widgets. Use WASD keys to move. You can increase/decrease terrains' LOD level using Page Up/Page Down."
			"Use C to generate another random terrain";
	}

    bool frameRenderingQueued(const FrameEvent& evt)
    {
		if (!mFly)
		{
			// clamp to terrain
			Vector3 camPos = mCameraNode->getPosition();
			Ray ray;
			ray.setOrigin(Vector3(camPos.x, mTerrainPos.y + 10000, camPos.z));
			ray.setDirection(Vector3::NEGATIVE_UNIT_Y);

			TerrainGroup::RayResult rayResult = mTerrainGroup->rayIntersects(ray);
			const Real distanceAboveTerrain = 50;
			if (rayResult.hit)
				mCameraNode->setPosition(camPos.x, rayResult.position.y + distanceAboveTerrain, camPos.z);
		}

		if (mTerrainGroup->isDerivedDataUpdateInProgress())
		{
			mTrayMgr->moveWidgetToTray(mInfoLabel, TL_TOP, 0);
			mInfoLabel->show();
			mInfoLabel->setCaption("Building terrain...");
		}
		else
		{
			mTrayMgr->removeWidgetFromTray(mInfoLabel);
			mInfoLabel->hide();
		}

		if (mLodStatus)
		{
			for(LabelList::iterator li = mLodStatusLabelList.begin(); li != mLodStatusLabelList.end(); li++)
			{
				mLodInfoOverlayContainer->_removeChild(*li);
				OverlayManager::getSingleton().destroyOverlayElement(*li);
			}
			mLodStatusLabelList.clear();

			for (const auto& ti : mTerrainGroup->getTerrainSlots())
			{
				Terrain* t = ti.second->instance;
				if (!t)
					continue;

				Vector3 pt = mCamera->getProjectionMatrix() * (mCamera->getViewMatrix() * t->getPosition());
				Real x = (pt.x / 2) + 0.5f;
				Real y = 1 - ((pt.y / 2) + 0.5f);

				String lName = StringConverter::toString(size_t(t))+"/"+"LodInfoLabel";

				OverlayElement *l = OverlayManager::getSingleton().createOverlayElement("TextArea", lName);
				l->setCaption("Target="+StringConverter::toString(t->getTargetLodLevel())+"\nHighest="+
					  StringConverter::toString(t->getHighestLodLoaded())+"\nPrepared="+
					  StringConverter::toString(t->getHighestLodPrepared())
					  );
				l->setPosition(x, y);
				l->setDimensions(0.1, 0.1);  // center text in label and its position
				l->setParameter("font_name", "SdkTrays/Value");
				l->setParameter("char_height", "0.02f");
				l->setColour(ColourValue(1.0,0.0,0.0));

				mLodInfoOverlayContainer->addChild(l);
				mLodStatusLabelList.push_back(l);
			}
		}

		mTerrainGroup->autoUpdateLodAll(false, Real(HOLD_LOD_DISTANCE));
		return SdkSample::frameRenderingQueued(evt);  // don't forget the parent updates!
    }

	bool keyPressed (const KeyboardEvent &e)
	{
#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
		switch (e.keysym.sym)
		{
		case SDLK_PAGEUP:
			{
				mAutoBox->setChecked(false);
				for (const auto& ti : mTerrainGroup->getTerrainSlots())
				{
					if(Terrain* t = ti.second->instance)
						t->increaseLodLevel();
				}
			}
			break;
		case SDLK_PAGEDOWN:
			{
				mAutoBox->setChecked(false);
				for (const auto& ti : mTerrainGroup->getTerrainSlots())
				{
					if(Terrain* t = ti.second->instance)
						t->decreaseLodLevel();
				}
			}
			break;
		// generate new random offset, to make terrains different
		case 'c':
			if(mPerlinNoiseTerrainGenerator)
			{
				// random a new origin point
				mPerlinNoiseTerrainGenerator->randomize();

				// reload all terrains
				for (const auto& ti : mTerrainGroup->getTerrainSlots())
				{
					mTerrainPagedWorldSection->unloadPage(ti.first);
					mTerrainPagedWorldSection->loadPage(ti.first);
				}
			}
			break;
		default:
			return SdkSample::keyPressed(e);
		}
#endif

		return true;
	}

	void checkBoxToggled(CheckBox* box)
	{
		if (box == mFlyBox)
		{
			mFly = mFlyBox->isChecked();
		}
		else if (box == mLodStatusBox)
		{
			mLodStatus = mLodStatusBox->isChecked();
			if (!mLodStatus)
			{
				for(LabelList::iterator li = mLodStatusLabelList.begin(); li != mLodStatusLabelList.end(); li++)
				{
					mLodInfoOverlayContainer->_removeChild(*li);
					OverlayManager::getSingleton().destroyOverlayElement(*li);
				}
				mLodStatusLabelList.clear();
			}
		}
		else if (box == mAutoBox)
		{
			if(mTerrainGroup)
			{
				auto strategy = mAutoBox->isChecked() ? BY_DISTANCE : NONE;
				mTerrainGroup->setAutoUpdateLod(TerrainAutoUpdateLodFactory::getAutoUpdateLod(strategy));
			}
		}
	}

protected:

	TerrainGlobalOptions* mTerrainGlobals;
	TerrainGroup* mTerrainGroup;
	TerrainPaging* mTerrainPaging;
	PageManager* mPageManager;
	PagedWorld* mPagedWorld;
	TerrainPagedWorldSection* mTerrainPagedWorldSection;
	PerlinNoiseTerrainGenerator* mPerlinNoiseTerrainGenerator;
	bool mLodStatus;

	/// This class just pretends to provide procedural page content to avoid page loading
	class DummyPageProvider : public PageProvider
	{
	public:
		bool prepareProceduralPage(Page* page, PagedWorldSection* section) { return true; }
		bool loadProceduralPage(Page* page, PagedWorldSection* section) { return true; }
		bool unloadProceduralPage(Page* page, PagedWorldSection* section) { return true; }
		bool unprepareProceduralPage(Page* page, PagedWorldSection* section) { return true; }
	};
	DummyPageProvider mDummyPageProvider;

	bool mFly;
	Real mFallVelocity;
	Vector3 mTerrainPos;
	CheckBox* mFlyBox;
	OgreBites::Label* mInfoLabel;

	typedef std::list<OverlayElement*> LabelList;
	LabelList mLodStatusLabelList;

	Overlay *mLodInfoOverlay;
	OverlayContainer *mLodInfoOverlayContainer;

	CheckBox *mLodStatusBox;
	CheckBox *mAutoBox;

	void configureTerrainDefaults(Light* l)
	{
		// Configure global
		mTerrainGlobals->setMaxPixelError(8);
		// testing composite map
		mTerrainGlobals->setCompositeMapDistance(3000);
		//mTerrainGlobals->setUseRayBoxDistanceCalculation(true);
		mTerrainGlobals->getDefaultMaterialGenerator()->setLightmapEnabled(false);

		mTerrainGlobals->setCompositeMapAmbient(mSceneMgr->getAmbientLight());
		mTerrainGlobals->setCompositeMapDiffuse(l->getDiffuseColour());
		mTerrainGlobals->setLightMapDirection(l->getDerivedDirection());

		// Configure default import settings for if we use imported image
		Terrain::ImportData& defaultimp = mTerrainGroup->getDefaultImportSettings();
		defaultimp.terrainSize = TERRAIN_SIZE;
		defaultimp.worldSize = TERRAIN_WORLD_SIZE;
		defaultimp.inputScale = 600;
		defaultimp.minBatchSize = 33;
		defaultimp.maxBatchSize = 65;
		// textures
		defaultimp.layerList.resize(1);
		defaultimp.layerList[0].worldSize = 200;
		defaultimp.layerList[0].textureNames.push_back("Ground37_diffspec.dds");
		defaultimp.layerList[0].textureNames.push_back("Ground37_normheight.dds");
	}

	/*-----------------------------------------------------------------------------
	| Extends setupView to change some initial camera settings for this sample.
	-----------------------------------------------------------------------------*/
	void setupView()
	{
		SdkSample::setupView();
		// put camera at world center, so that it's difficult to reach the edge
		Vector3 worldCenter(
			(ENDLESS_PAGE_MAX_X+ENDLESS_PAGE_MIN_X) / 2 * TERRAIN_WORLD_SIZE,
			0,
			-(ENDLESS_PAGE_MAX_Y+ENDLESS_PAGE_MIN_Y) / 2 * TERRAIN_WORLD_SIZE
			);
		mCameraNode->setPosition(mTerrainPos+worldCenter);
		mCameraNode->lookAt(mTerrainPos, Node::TS_PARENT);
		mCamera->setNearClipDistance(0.1);
		mCamera->setFarClipDistance(50000);

		mCamera->setFarClipDistance(0);   // enable infinite far clip distance
	}

	void setupControls()
	{
		mTrayMgr->showCursor();

		// make room for the controls
		mTrayMgr->showLogo(TL_TOPRIGHT);
		mTrayMgr->showFrameStats(TL_TOPRIGHT);
		mTrayMgr->toggleAdvancedFrameStats();

		mInfoLabel = mTrayMgr->createLabel(TL_TOP, "TInfo", "", 350);

		mFlyBox = mTrayMgr->createCheckBox(TL_BOTTOM, "Fly", "Fly");
		mFlyBox->setChecked(false, true);

		mLodStatusBox = mTrayMgr->createCheckBox(TL_BOTTOM, "LODStatus", "LOD Status");
		mLodStatusBox->setChecked(false, true);

		mAutoBox = mTrayMgr->createCheckBox(TL_BOTTOM, "LODAuto", "Auto LOD");
		mAutoBox->setChecked(true, true);

		// a friendly reminder
		StringVector names;
		names.push_back("Help");
		mTrayMgr->createParamsPanel(TL_TOPLEFT, "Help", 100, names)->setParamValue(0, "H/F1");
	}

	class SimpleTerrainDefiner : public TerrainPagedWorldSection::TerrainDefiner
	{
	public:
		virtual void define(TerrainGroup* terrainGroup, long x, long y)
		{
			Image img;
			img.load("terrain.png", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
			if (x % 2)
				img.flipAroundY();
			if (y % 2)
				img.flipAroundX();
			terrainGroup->defineTerrain(x, y, &img);
		}
	};

	void setupContent()
	{
		mTerrainGlobals = OGRE_NEW TerrainGlobalOptions();

		mCameraMan->setTopSpeed(100);

		setDragLook(true);

		MaterialManager::getSingleton().setDefaultTextureFiltering(TFO_ANISOTROPIC);
		MaterialManager::getSingleton().setDefaultAnisotropy(7);

		mSceneMgr->setFog(FOG_LINEAR, ColourValue(0.7, 0.7, 0.8), 0, 4000, 10000);

		LogManager::getSingleton().setMinLogLevel(LML_TRIVIAL);

		Light* l = mSceneMgr->createLight("tstLight");
		l->setType(Light::LT_DIRECTIONAL);
		l->setDiffuseColour(ColourValue::White);
		l->setSpecularColour(ColourValue(0.4, 0.4, 0.4));

	    auto ln = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	    ln->setDirection(Vector3(0.55, -0.3, 0.75).normalisedCopy());
	    ln->attachObject(l);

		mSceneMgr->setAmbientLight(ColourValue(0.2, 0.2, 0.2));

		mTerrainGroup = OGRE_NEW TerrainGroup(mSceneMgr, Terrain::ALIGN_X_Z, TERRAIN_SIZE, TERRAIN_WORLD_SIZE);
		mTerrainGroup->setFilenameConvention(ENDLESS_TERRAIN_FILE_PREFIX, ENDLESS_TERRAIN_FILE_SUFFIX);
		mTerrainGroup->setOrigin(mTerrainPos);
		mTerrainGroup->setAutoUpdateLod(TerrainAutoUpdateLodFactory::getAutoUpdateLod(BY_DISTANCE));

		configureTerrainDefaults(l);

		// Paging setup
		mPageManager = OGRE_NEW PageManager();
		// Since we're not loading any pages from .page files, we need a way just 
		// to say we've loaded them without them actually being loaded
		mPageManager->setPageProvider(&mDummyPageProvider);
		mPageManager->addCamera(mCamera);
		mPageManager->setDebugDisplayLevel(0);
		mTerrainPaging = OGRE_NEW TerrainPaging(mPageManager);
		mPagedWorld = mPageManager->createWorld();
		mTerrainPagedWorldSection = mTerrainPaging->createWorldSection(mPagedWorld, mTerrainGroup, 400, 500, 
			ENDLESS_PAGE_MIN_X, ENDLESS_PAGE_MIN_Y, 
			ENDLESS_PAGE_MAX_X, ENDLESS_PAGE_MAX_Y);

#if USE_PERLIN_DEFINER == 1
		mPerlinNoiseTerrainGenerator = OGRE_NEW PerlinNoiseTerrainGenerator;
		mTerrainPagedWorldSection->setDefiner(mPerlinNoiseTerrainGenerator);
#else
		mTerrainPagedWorldSection->setDefiner(OGRE_NEW SimpleTerrainDefiner);
#endif

		mTerrainGroup->freeTemporaryResources();

		mSceneMgr->setSkyBox(true, "Examples/CloudyNoonSkyBox");

		// setup LOD info overlay
		mLodInfoOverlay = OverlayManager::getSingleton().create("LODInfoOverlay");

		mLodInfoOverlay->setZOrder(10);
		mLodInfoOverlayContainer = (OverlayContainer*)OverlayManager::getSingleton().createOverlayElement("Panel", "LODInfoOverlayPanel");
		mLodInfoOverlayContainer->setDimensions(1.0, 1.0);
		mLodInfoOverlayContainer->setPosition(0.0, 0.0);

		mLodInfoOverlay->add2D(mLodInfoOverlayContainer);
		mLodInfoOverlay->show();

		setupControls();
	}

	void _shutdown()
	{
		if(mTerrainPaging)
		{
			OGRE_DELETE mTerrainPaging;
			mPageManager->removeCamera(mCamera);
			mPageManager->destroyWorld( mPagedWorld );
			OGRE_DELETE mPageManager;
		}

        if(mTerrainGlobals)
            OGRE_DELETE mTerrainGlobals;

        if(mLodInfoOverlay)
            OverlayManager::getSingleton().destroy(mLodInfoOverlay);

        if(mLodInfoOverlayContainer)
        {
            for(LabelList::iterator li = mLodStatusLabelList.begin(); li != mLodStatusLabelList.end(); li++)
            {
                mLodInfoOverlayContainer->_removeChild(*li);
                OverlayManager::getSingleton().destroyOverlayElement(*li);
            }
            mLodStatusLabelList.clear();

            OverlayManager::getSingleton().destroyOverlayElement(mLodInfoOverlayContainer);
        }

		SdkSample::_shutdown();
	}
};

#endif
