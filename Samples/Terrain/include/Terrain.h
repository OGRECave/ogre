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
#include "OgreTerrainQuadTreeNode.h"

#define TERRAIN_FILE "testTerrain.dat"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_Terrain : public SdkSample
{
public:

	Sample_Terrain()
		: mTerrain(0)
		, mFly(false)
		, mMode(MODE_NORMAL)
		, mLayerEdit(1)
		, mBrushSizeTerrainSpace(0.02)
		, mUpdateCountDown(0)
		, mTerrainPos(1000,0,5000)

	{
		mInfo["Title"] = "Terrain";
		mInfo["Description"] = "Demonstrates use of the terrain rendering plugin.";
		mInfo["Thumbnail"] = "thumb_terrain.png";
		mInfo["Category"] = "Unsorted";

		// Update terrain at max 20fps
		mUpdateRate = 1.0 / 20.0;

	}

    void testCapabilities(const RenderSystemCapabilities* caps)
	{
        if (!caps->hasCapability(RSC_VERTEX_PROGRAM) || !caps->hasCapability(RSC_FRAGMENT_PROGRAM))
        {
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your graphics card does not support vertext or fragment shaders, "
                        "so you cannot run this sample. Sorry!", "TerrainSample::testCapabilities");
        }
	}
    
	StringVector getRequiredPlugins()
	{
		StringVector names;
		return names;
	}

    bool frameRenderingQueued(const FrameEvent& evt)
    {
		if (mMode != MODE_NORMAL)
		{
			// fire ray
			Ray ray; 
			ray = mCamera->getCameraToViewportRay(0.5, 0.5);
			std::pair<bool, Vector3> rayResult = mTerrain->rayIntersects(ray);
			if (rayResult.first)
			{
				mEditMarker->setVisible(true);
				mEditNode->setPosition(rayResult.second);

				Vector3 tsPos;
				mTerrain->getTerrainPosition(rayResult.second, &tsPos);
#if OGRE_PLATFORM != OGRE_PLATFORM_IPHONE
				if (mKeyboard->isKeyDown(OIS::KC_EQUALS) || mKeyboard->isKeyDown(OIS::KC_MINUS))
				{
					switch(mMode)
					{
					case MODE_EDIT_HEIGHT:
						{
							// we need point coords
							Real terrainSize = (mTerrain->getSize() - 1);
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

									float addedHeight = weight * 250.0 * evt.timeSinceLastFrame;
									float newheight;
									if (mKeyboard->isKeyDown(OIS::KC_EQUALS))
										newheight = mTerrain->getHeightAtPoint(x, y) + addedHeight;
									else
										newheight = mTerrain->getHeightAtPoint(x, y) - addedHeight;
									mTerrain->setHeightAtPoint(x, y, newheight);
									if (mUpdateCountDown == 0)
										mUpdateCountDown = mUpdateRate;

								}
							}
						}
						break;
					case MODE_EDIT_BLEND:
						{
							TerrainLayerBlendMap* layer = mTerrain->getLayerBlendMap(mLayerEdit);
							// we need image coords
							Real imgSize = mTerrain->getLayerBlendMapSize();
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

									float paint = weight * evt.timeSinceLastFrame;
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
			else
			{
				mEditMarker->setVisible(false);
			}
		}

		switch(mMode)
		{
		case MODE_NORMAL:
			// TODO - new tray
			//mDebugText = "";
			break;
		case MODE_EDIT_HEIGHT:
			// TODO - new tray
			//mDebugText = "Height Edit Mode";
			break;
		case MODE_EDIT_BLEND:
			// TODO - new tray
			//mDebugText = "Blend Edit Mode";
			break;
        case MODE_COUNT:
            // TODO - new tray
            //mDebugText = "";
            break;
        default:
            break;
		}

		if (!mFly)
		{
			// clamp to terrain
			Ray ray;
			ray.setOrigin(mCamera->getPosition());
			ray.setDirection(Vector3::NEGATIVE_UNIT_Y);

			std::pair<bool, Vector3> rayResult = mTerrain->rayIntersects(ray);
			if (rayResult.first)
			{
				mCamera->setPosition(mCamera->getPosition().x, 
					rayResult.second.y + 30, 
					mCamera->getPosition().z);
			}

		}

		if (mUpdateCountDown > 0)
		{
			mUpdateCountDown -= evt.timeSinceLastFrame;
			if (mUpdateCountDown <= 0)
			{
				mTerrain->update();
				mUpdateCountDown = 0;
			}
		}

		return SdkSample::frameRenderingQueued(evt);  // don't forget the parent updates!
    }

	bool keyPressed (const OIS::KeyEvent &e)
	{
#if OGRE_PLATFORM != OGRE_PLATFORM_IPHONE
		switch (e.key)
		{
		case OIS::KC_RETURN:
			mFly = !mFly;
			break;
		case OIS::KC_SPACE:
			mMode = (Mode)((mMode + 1) & MODE_COUNT);
			break;
		case OIS::KC_S:
			// CTRL-S to save
			if (mKeyboard->isKeyDown(OIS::KC_LCONTROL) || mKeyboard->isKeyDown(OIS::KC_RCONTROL))
				mTerrain->save(TERRAIN_FILE);
		default:
			return SdkSample::keyPressed(e);
		}
#endif

		return true;
	}

protected:

	Terrain* mTerrain;
	bool mFly;
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


	Terrain* createTerrain()
	{
		Terrain* terrain = OGRE_NEW Terrain(mSceneMgr);
		Image img;
		img.load("terrain.png", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		//img.load("terrain_flattened.png", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		//img.load("terrain_onehill.png", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

		Terrain::ImportData imp;
		imp.inputImage = &img;
		imp.terrainSize = 513;
		imp.worldSize = 8000;
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
		terrain->prepare(imp);
		terrain->load();

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

		terrain->freeTemporaryResources();

		return terrain;

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

		mCamera->setPosition(mTerrainPos + Vector3(-1000,300,1000));
		mCamera->lookAt(mTerrainPos);
		mCamera->setNearClipDistance(5);
		mCamera->setFarClipDistance(50000);

		if (mRoot->getRenderSystem()->getCapabilities()->hasCapability(RSC_INFINITE_FAR_PLANE))
        {
            mCamera->setFarClipDistance(0);   // enable infinite far clip distance if we can
        }
	}

	void setupContent()
	{
		mEditMarker = mSceneMgr->createEntity("editMarker", "sphere.mesh");
		mEditNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		mEditNode->attachObject(mEditMarker);
		mEditNode->setScale(0.05, 0.05, 0.05);

		mTrayMgr->showCursor();

		mCameraMan->setTopSpeed(50);

		MaterialManager::getSingleton().setDefaultTextureFiltering(TFO_ANISOTROPIC);
		MaterialManager::getSingleton().setDefaultAnisotropy(7);

		mSceneMgr->setFog(FOG_LINEAR, ColourValue(0.7, 0.7, 0.8), 0, 2000, 10000);

		LogManager::getSingleton().setLogDetail(LL_BOREME);

		Vector3 lightdir(0.55, -0.3, 0.75);
		lightdir.normalise();


		TerrainGlobalOptions::setMaxPixelError(8);
		// testing composite map
		TerrainGlobalOptions::setCompositeMapDistance(3000);
		//TerrainGlobalOptions::setUseRayBoxDistanceCalculation(true);
		//TerrainGlobalOptions::getDefaultMaterialGenerator()->setDebugLevel(1);
		//TerrainGlobalOptions::setLightMapSize(256);



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

		try
		{
			mTerrain = OGRE_NEW Terrain(mSceneMgr);
			mTerrain->load(TERRAIN_FILE);
		}
		catch (Exception&)
		{
			OGRE_DELETE mTerrain;
			mTerrain = 0;
		}
		if (!mTerrain)
			mTerrain = createTerrain();

		//addTextureDebugOverlay(TextureManager::getSingleton().getByName(mTerrain->getTerrainNormalMap()->getName()), 0);

		//mWindow->getViewport(0)->setBackgroundColour(ColourValue::Blue);

		// Testing
		mTerrain->setPosition(mTerrainPos);

		/*
		// create a few entities on the terrain
		for (int i = 0; i < 20; ++i)
		{
			Entity* e = mSceneMgr->createEntity("ninja.mesh");
			Real x = mTerrainPos.x + Math::RangeRandom(-2500, 2500);
			Real z = mTerrainPos.z + Math::RangeRandom(-2500, 2500);
			Real y = mTerrain->getHeightAtWorldPosition(Vector3(x, 0, z));
			mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(x, y, z))->attachObject(e);
		}
		*/



		mSceneMgr->setSkyBox(true, "Examples/CloudyNoonSkyBox");


	}

};

#endif
