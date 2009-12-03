/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
-----------------------------------------------------------------------------
*/
#ifndef __Terrain_H__
#define __Terrain_H__

#include "SdkSample.h"
#include "OgreTerrain.h"
#include "OgreTerrainGroup.h"
#include "OgreTerrainQuadTreeNode.h"
#include "OgreTerrainMaterialGeneratorA.h"

#define TERRAIN_FILE_PREFIX String("testTerrain")
#define TERRAIN_FILE_SUFFIX String("dat")
#define TERRAIN_WORLD_SIZE 12000.0f
#define TERRAIN_SIZE 513

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_Terrain : public SdkSample
{
public:

	Sample_Terrain()
		: mTerrainGroup(0)
		, mFly(false)
		, mFallVelocity(0)
		, mMode(MODE_NORMAL)
		, mLayerEdit(1)
		, mBrushSizeTerrainSpace(0.02)
		, mUpdateCountDown(0)
		, mTerrainPos(1000,0,5000)
		, mTerrainsImported(false)

	{
		mInfo["Title"] = "Terrain";
		mInfo["Description"] = "Demonstrates use of the terrain rendering plugin.";
		mInfo["Thumbnail"] = "thumb_terrain.png";
		mInfo["Category"] = "Unsorted";
		mInfo["Help"] = "Left click and drag anywhere in the scene to look around. Let go again to show "
			"cursor and access widgets. Use WASD keys to move. Use +/- keys when in edit mode to change content.";

		// Update terrain at max 20fps
		mUpdateRate = 1.0 / 20.0;

	}

    void testCapabilities(const RenderSystemCapabilities* caps)
	{
        if (!caps->hasCapability(RSC_VERTEX_PROGRAM) || !caps->hasCapability(RSC_FRAGMENT_PROGRAM))
        {
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your graphics card does not support vertex or fragment shaders, "
                        "so you cannot run this sample. Sorry!", "TerrainSample::testCapabilities");
        }
	}
    
	StringVector getRequiredPlugins()
	{
		StringVector names;
		return names;
	}

	void doTerrainModify(Terrain* terrain, const Vector3& centrepos, Real timeElapsed)
	{
		Vector3 tsPos;
		terrain->getTerrainPosition(centrepos, &tsPos);
#if OGRE_PLATFORM != OGRE_PLATFORM_IPHONE
		if (mKeyboard->isKeyDown(OIS::KC_EQUALS) || mKeyboard->isKeyDown(OIS::KC_MINUS))
		{
			switch(mMode)
			{
			case MODE_EDIT_HEIGHT:
				{
					// we need point coords
					Real terrainSize = (terrain->getSize() - 1);
					long startx = (tsPos.x - mBrushSizeTerrainSpace) * terrainSize;
					long starty = (tsPos.y - mBrushSizeTerrainSpace) * terrainSize;
					long endx = (tsPos.x + mBrushSizeTerrainSpace) * terrainSize;
					long endy= (tsPos.y + mBrushSizeTerrainSpace) * terrainSize;
					startx = std::max(startx, 0L);
					starty = std::max(starty, 0L);
					endx = std::min(endx, (long)terrainSize);
					endy = std::min(endy, (long)terrainSize);
					for (long y = starty; y <= endy; ++y)
					{
						for (long x = startx; x <= endx; ++x)
						{
							Real tsXdist = (x / terrainSize) - tsPos.x;
							Real tsYdist = (y / terrainSize)  - tsPos.y;

							Real weight = std::min((Real)1.0, 
								Math::Sqrt(tsYdist * tsYdist + tsXdist * tsXdist) / Real(0.5 * mBrushSizeTerrainSpace));
							weight = 1.0 - (weight * weight);

							float addedHeight = weight * 250.0 * timeElapsed;
							float newheight;
							if (mKeyboard->isKeyDown(OIS::KC_EQUALS))
								newheight = terrain->getHeightAtPoint(x, y) + addedHeight;
							else
								newheight = terrain->getHeightAtPoint(x, y) - addedHeight;
							terrain->setHeightAtPoint(x, y, newheight);
							if (mUpdateCountDown == 0)
								mUpdateCountDown = mUpdateRate;

						}
					}
				}
				break;
			case MODE_EDIT_BLEND:
				{
					TerrainLayerBlendMap* layer = terrain->getLayerBlendMap(mLayerEdit);
					// we need image coords
					Real imgSize = terrain->getLayerBlendMapSize();
					long startx = (tsPos.x - mBrushSizeTerrainSpace) * imgSize;
					long starty = (tsPos.y - mBrushSizeTerrainSpace) * imgSize;
					long endx = (tsPos.x + mBrushSizeTerrainSpace) * imgSize;
					long endy= (tsPos.y + mBrushSizeTerrainSpace) * imgSize;
					startx = std::max(startx, 0L);
					starty = std::max(starty, 0L);
					endx = std::min(endx, (long)imgSize);
					endy = std::min(endy, (long)imgSize);
					for (long y = starty; y <= endy; ++y)
					{
						for (long x = startx; x <= endx; ++x)
						{
							Real tsXdist = (x / imgSize) - tsPos.x;
							Real tsYdist = (y / imgSize)  - tsPos.y;

							Real weight = std::min((Real)1.0, 
								Math::Sqrt(tsYdist * tsYdist + tsXdist * tsXdist) / Real(0.5 * mBrushSizeTerrainSpace));
							weight = 1.0 - (weight * weight);

							float paint = weight * timeElapsed;
							size_t imgY = imgSize - y;
							float val;
							if (mKeyboard->isKeyDown(OIS::KC_EQUALS))
								val = layer->getBlendValue(x, imgY) + paint;
							else
								val = layer->getBlendValue(x, imgY) - paint;
							val = Math::Clamp(val, 0.0f, 1.0f);
							layer->setBlendValue(x, imgY, val);
							layer->update();

						}
					}
				}
				break;



			};

		}
#endif

	}
    bool frameRenderingQueued(const FrameEvent& evt)
    {
		if (mMode != MODE_NORMAL)
		{
			// fire ray
			Ray ray; 
			//ray = mCamera->getCameraToViewportRay(0.5, 0.5);
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

				for (TerrainGroup::TerrainList::iterator ti = terrainList.begin();
					ti != terrainList.end(); ++ti)
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
			Vector3 camPos = mCamera->getPosition();
			Ray ray;
			ray.setOrigin(Vector3(camPos.x, 10000, camPos.z));
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
				mCamera->setPosition(camPos.x, newy, camPos.z);
				
			}

		}

		if (mUpdateCountDown > 0)
		{
			mUpdateCountDown -= evt.timeSinceLastFrame;
			if (mUpdateCountDown <= 0)
			{
				mTerrainGroup->update();
				mUpdateCountDown = 0;

			}

		}

		if (mTerrainGroup->isDerivedDataUpdateInProgress())
		{
			mInfoLabel->setCaption("Computing textures, patience...");
			mTrayMgr->moveWidgetToTray(mInfoLabel, TL_TOP, 0);
			mInfoLabel->show();
		}
		else
		{
			mTrayMgr->removeWidgetFromTray(mInfoLabel);
			mInfoLabel->hide();
			if (mTerrainsImported)
			{
				saveTerrains(false);
				mTerrainsImported = false;
			}
		}




		return SdkSample::frameRenderingQueued(evt);  // don't forget the parent updates!
    }

	void saveTerrains(bool onlyIfModified)
	{
		mTerrainGroup->saveAllTerrains(onlyIfModified);
	}

	bool keyPressed (const OIS::KeyEvent &e)
	{
#if OGRE_PLATFORM != OGRE_PLATFORM_IPHONE
		switch (e.key)
		{
		case OIS::KC_S:
			// CTRL-S to save
			if (mKeyboard->isKeyDown(OIS::KC_LCONTROL) || mKeyboard->isKeyDown(OIS::KC_RCONTROL))
			{
				saveTerrains(true);
			}
			else
				return SdkSample::keyPressed(e);
			break;
		case OIS::KC_F10:
			// dump
			{
				TerrainGroup::TerrainIterator ti = mTerrainGroup->getTerrainIterator();
				while (ti.hasMoreElements())
				{
					uint32 tkey = ti.peekNextKey();
					TerrainGroup::TerrainSlot* ts = ti.getNext();
					if (ts->instance && ts->instance->isLoaded())
					{
						ts->instance->_dumpTextures("terrain_" + StringConverter::toString(tkey), ".png");
					}


				}
			}
			break;
		default:
			return SdkSample::keyPressed(e);
		}
#endif

		return true;
	}

#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
	virtual bool touchPressed(const OIS::MultiTouchEvent& evt)
	{
		if (mTrayMgr->injectMouseDown(evt)) return true;
		mTrayMgr->hideCursor();  // hide the cursor if user left-clicks in the scene
		return true;
	}
#else
	bool mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
	{
		if (mTrayMgr->injectMouseDown(evt, id)) return true;
		if (id == OIS::MB_Left) mTrayMgr->hideCursor();  // hide the cursor if user left-clicks in the scene
		return true;
	}
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
	virtual bool touchReleased(const OIS::MultiTouchEvent& evt)
	{
		if (mTrayMgr->injectMouseUp(evt)) return true;
		mTrayMgr->showCursor();  // unhide the cursor if user lets go of LMB
		return true;
	}
#else
	bool mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
	{
		if (mTrayMgr->injectMouseUp(evt, id)) return true;
		if (id == OIS::MB_Left) mTrayMgr->showCursor();  // unhide the cursor if user lets go of LMB
		return true;
	}
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
	virtual bool touchMoved(const OIS::MultiTouchEvent& evt)
#else
	virtual bool mouseMoved(const OIS::MouseEvent& evt)
#endif
	{
		if (mTrayMgr->isCursorVisible()) mTrayMgr->injectMouseMove(evt);
		else mCameraMan->injectMouseMove(evt);
		return true;
	}
	void itemSelected(SelectMenu* menu)
	{
		if (menu == mEditMenu)
		{
			mMode = (Mode)mEditMenu->getSelectionIndex();
		}
	}

	void checkBoxToggled(CheckBox* box)
	{
		if (box == mFlyBox)
		{
			mFly = mFlyBox->isChecked();
		}
	}

protected:

	TerrainGroup* mTerrainGroup;
	bool mFly;
	Real mFallVelocity;
	enum Mode
	{
		MODE_NORMAL = 0,
		MODE_EDIT_HEIGHT = 1,
		MODE_EDIT_BLEND = 2,
		MODE_COUNT = 3
	};
	Mode mMode;
	uint8 mLayerEdit;
	Real mBrushSizeTerrainSpace;
	SceneNode* mEditNode;
	Entity* mEditMarker;
	Real mUpdateCountDown;
	Real mUpdateRate;
	Vector3 mTerrainPos;
	SelectMenu* mEditMenu;
	CheckBox* mFlyBox;
	OgreBites::Label* mInfoLabel;
	bool mTerrainsImported;


	void defineTerrain(long x, long y, bool flat = false)
	{
		// if a file is available, use it
		// if not, generate file from import

		// Usually in a real project you'll know whether the compact terrain data is
		// available or not; I'm doing it this way to save distribution size

		if (flat)
		{
			mTerrainGroup->defineTerrain(x, y, 0.0f);
		}
		else
		{
			String filename = mTerrainGroup->generateFilename(x, y);
			if (ResourceGroupManager::getSingleton().resourceExists(mTerrainGroup->getResourceGroup(), filename))
			{
				mTerrainGroup->defineTerrain(x, y);
			}
			else
			{
				Terrain::ImportData imp;
				createTerrainImportData(x % 2 != 0, y % 2 != 0, imp);
				mTerrainGroup->defineTerrain(x, y, &imp);
				mTerrainsImported = true;
			}

		}
	}

	void createTerrainImportData(bool flipX, bool flipY, Terrain::ImportData& imp)
	{
		Image* img = OGRE_NEW Image();
		img->load("terrain.png", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		if (flipX)
			img->flipAroundY();
		if (flipY)
			img->flipAroundX();

		imp.inputImage = img;
		imp.terrainSize = TERRAIN_SIZE;
		imp.worldSize = TERRAIN_WORLD_SIZE;
		imp.inputScale = 600;
		imp.minBatchSize = 33;
		imp.maxBatchSize = 65;
		// textures
		imp.layerList.resize(3);
		imp.layerList[0].worldSize = 100;
		imp.layerList[0].textureNames.push_back("dirt_grayrocky_diffusespecular.dds");
		imp.layerList[0].textureNames.push_back("dirt_grayrocky_normalheight.dds");
		imp.layerList[1].worldSize = 30;
		imp.layerList[1].textureNames.push_back("grass_green-01_diffusespecular.dds");
		imp.layerList[1].textureNames.push_back("grass_green-01_normalheight.dds");
		imp.layerList[2].worldSize = 200;
		imp.layerList[2].textureNames.push_back("growth_weirdfungus-03_diffusespecular.dds");
		imp.layerList[2].textureNames.push_back("growth_weirdfungus-03_normalheight.dds");

	}

	void initBlendMaps(Terrain* terrain)
	{
		TerrainLayerBlendMap* blendMap0 = terrain->getLayerBlendMap(1);
		TerrainLayerBlendMap* blendMap1 = terrain->getLayerBlendMap(2);
		Real minHeight0 = 70;
		Real fadeDist0 = 40;
		Real minHeight1 = 70;
		Real fadeDist1 = 15;
		float* pBlend0 = blendMap0->getBlendPointer();
		float* pBlend1 = blendMap1->getBlendPointer();
		for (Ogre::uint16 y = 0; y < terrain->getLayerBlendMapSize(); ++y)
		{
			for (Ogre::uint16 x = 0; x < terrain->getLayerBlendMapSize(); ++x)
			{
				Real tx, ty;

				blendMap0->convertImageToTerrainSpace(x, y, &tx, &ty);
				Real height = terrain->getHeightAtTerrainPosition(tx, ty);
				Real val = (height - minHeight0) / fadeDist0;
				val = Math::Clamp(val, (Real)0, (Real)1);
				//*pBlend0++ = val;

				val = (height - minHeight1) / fadeDist1;
				val = Math::Clamp(val, (Real)0, (Real)1);
				*pBlend1++ = val;


			}
		}
		blendMap0->dirty();
		blendMap1->dirty();
		//blendMap0->loadImage("blendmap1.png", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		blendMap0->update();
		blendMap1->update();

		/*
		// set up a colour map
		terrain->setGlobalColourMapEnabled(true);
		Image colourMap;
		colourMap.load("testcolourmap.jpg", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		terrain->getGlobalColourMap()->loadImage(colourMap);
		*/

	}


    void createSceneManager()
    {
		// we're going to need a special terrain scene manager for this sample
        mSceneMgr = mRoot->createSceneManager("TerrainSceneManager");
    }

	/*-----------------------------------------------------------------------------
	| Extends setupView to change some initial camera settings for this sample.
	-----------------------------------------------------------------------------*/
	void setupView()
	{
		SdkSample::setupView();

		mCamera->setPosition(mTerrainPos + Vector3(-1000,50,1000));
		mCamera->lookAt(mTerrainPos);
		mCamera->setNearClipDistance(5);
		mCamera->setFarClipDistance(50000);

		if (mRoot->getRenderSystem()->getCapabilities()->hasCapability(RSC_INFINITE_FAR_PLANE))
        {
            mCamera->setFarClipDistance(0);   // enable infinite far clip distance if we can
        }
	}

	void setupControls()
	{
		mTrayMgr->showCursor();

#ifdef USE_RTSHADER_SYSTEM
		// we don't need this right now (maybe in future!)
		mTrayMgr->removeWidgetFromTray(mRTShaderSystemPanel);
		mRTShaderSystemPanel->hide();
#endif
		// make room for the controls
		mTrayMgr->showLogo(TL_TOPRIGHT);
		mTrayMgr->showFrameStats(TL_TOPRIGHT);
		mTrayMgr->toggleAdvancedFrameStats();

		mInfoLabel = mTrayMgr->createLabel(TL_TOP, "TInfo", "", 350);

		mEditMenu = mTrayMgr->createLongSelectMenu(TL_BOTTOM, "EditMode", "Edit Mode", 370, 250, 3);
		mEditMenu->addItem("None");
		mEditMenu->addItem("Elevation");
		mEditMenu->addItem("Blend");

		mFlyBox = mTrayMgr->createCheckBox(TL_BOTTOM, "Fly", "Fly");
		mFlyBox->setChecked(false, false);

		// a friendly reminder
		StringVector names;
		names.push_back("Help");
		mTrayMgr->createParamsPanel(TL_TOPLEFT, "Help", 100, names)->setParamValue(0, "H/F1");

		mEditMenu->selectItem(0);  // no edit mode
	}

	void setupContent()
	{
		bool blankTerrain = false;
		//blankTerrain = true;

		mEditMarker = mSceneMgr->createEntity("editMarker", "sphere.mesh");
		mEditNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		mEditNode->attachObject(mEditMarker);
		mEditNode->setScale(0.05, 0.05, 0.05);

		setupControls();

		mCameraMan->setTopSpeed(50);

		MaterialManager::getSingleton().setDefaultTextureFiltering(TFO_ANISOTROPIC);
		MaterialManager::getSingleton().setDefaultAnisotropy(7);

		mSceneMgr->setFog(FOG_LINEAR, ColourValue(0.7, 0.7, 0.8), 0, 5000, 14000);

		LogManager::getSingleton().setLogDetail(LL_BOREME);

		Vector3 lightdir(0.55, -0.3, 0.75);
		lightdir.normalise();


		TerrainGlobalOptions::setMaxPixelError(8);
		// testing composite map
		TerrainGlobalOptions::setCompositeMapDistance(3000);
		//TerrainGlobalOptions::setUseRayBoxDistanceCalculation(true);
		//TerrainGlobalOptions::getDefaultMaterialGenerator()->setDebugLevel(1);
		//TerrainGlobalOptions::setLightMapSize(256);
		//static_cast<TerrainMaterialGeneratorA::SM2Profile*>(TerrainGlobalOptions::getDefaultMaterialGenerator()->getActiveProfile())
		//	->setLightmapEnabled(false);



		Light* l = mSceneMgr->createLight("tstLight");
		l->setType(Light::LT_DIRECTIONAL);
		l->setDirection(lightdir);
		l->setDiffuseColour(ColourValue::White);
		l->setSpecularColour(ColourValue(0.4, 0.4, 0.4));

		mSceneMgr->setAmbientLight(ColourValue(0.2, 0.2, 0.2));


		// Important to set these so that the terrain knows what to use for derived (non-realtime) data
		TerrainGlobalOptions::setLightMapDirection(lightdir);
		TerrainGlobalOptions::setCompositeMapAmbient(mSceneMgr->getAmbientLight());
		//TerrainGlobalOptions::setCompositeMapAmbient(ColourValue::Red);
		TerrainGlobalOptions::setCompositeMapDiffuse(l->getDiffuseColour());
		
		mTerrainGroup = OGRE_NEW TerrainGroup(mSceneMgr, Terrain::ALIGN_X_Z, TERRAIN_SIZE, TERRAIN_WORLD_SIZE);
		mTerrainGroup->setFilenameConvention(TERRAIN_FILE_PREFIX, TERRAIN_FILE_SUFFIX);
		mTerrainGroup->setOrigin(mTerrainPos);

		defineTerrain(0, 0, blankTerrain);
		defineTerrain(1, 0, blankTerrain);


		// sync load since we want everything in place when we start
		mTerrainGroup->loadAllTerrains(true);

		if (mTerrainsImported)
		{
			TerrainGroup::TerrainIterator ti = mTerrainGroup->getTerrainIterator();
			while(ti.hasMoreElements())
			{
				Terrain* t = ti.getNext()->instance;
				initBlendMaps(t);
			}
		}

		mTerrainGroup->freeTemporaryResources();


		// create a few entities on the terrain
		for (int i = 0; i < 20; ++i)
		{
			Entity* e = mSceneMgr->createEntity("ninja.mesh");
			Real x = mTerrainPos.x + Math::RangeRandom(-2500, 2500);
			Real z = mTerrainPos.z + Math::RangeRandom(-2500, 2500);
			Real y = mTerrainGroup->getHeightAtWorldPosition(Vector3(x, 0, z));
			mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(x, y, z))->attachObject(e);
		}



		mSceneMgr->setSkyBox(true, "Examples/CloudyNoonSkyBox");


	}

};

#endif
