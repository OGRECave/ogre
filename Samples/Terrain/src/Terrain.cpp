/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
LGPL like the rest of the engine.
-----------------------------------------------------------------------------
*/

/**
    @file 
        Terrain.cpp
    @brief
        Shows OGRE's terrain rendering plugin.
*/

#include "ExampleApplication.h"
#include "OgreTerrain.h"
#include "OgreTerrainQuadTreeNode.h"

#define TERRAIN_FILE "testTerrain.dat"

Terrain* mTerrain = 0;

// Event handler to add ability to alter curvature
class TerrainFrameListener : public ExampleFrameListener, public OIS::KeyListener
{
protected:
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
public:
	TerrainFrameListener(RenderWindow* win, Camera* cam)
		: ExampleFrameListener(win, cam, true)
		, mFly(false)
		, mMode(MODE_NORMAL)
		, mLayerEdit(1)
		, mBrushSizeTerrainSpace(0.02)
		, mUpdateCountDown(0)
	{
		// Reduce move speed
		mMoveSpeed = 50;

		// Update terrain at max 20fps
		mUpdateRate = 1.0 / 20.0;

		mKeyboard->setEventCallback(this);

		SceneManager* sm = cam->getSceneManager();
		mEditMarker = sm->createEntity("editMarker", "sphere.mesh");
		mEditNode = sm->getRootSceneNode()->createChildSceneNode();
		mEditNode->attachObject(mEditMarker);
		mEditNode->setScale(0.05, 0.05, 0.05);

	}

	bool keyPressed (const OIS::KeyEvent &e)
	{
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

		}

		return true;
	}

	bool OIS::KeyListener::keyReleased(const OIS::KeyEvent &)
	{
		return true;
	}

	bool processUnbufferedKeyInput(const FrameEvent& evt)
	{
		if (!ExampleFrameListener::processUnbufferedKeyInput(evt))
			return false;

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

			}
			else
			{
				mEditMarker->setVisible(false);
			}
		}

		switch(mMode)
		{
		case MODE_NORMAL:
			mDebugText = "";
			break;
		case MODE_EDIT_HEIGHT:
			mDebugText = "Height Edit Mode";
			break;
		case MODE_EDIT_BLEND:
			mDebugText = "Blend Edit Mode";
			break;

		}

		return true;
	}

	bool frameRenderingQueued(const FrameEvent& evt)
	{
		if(mWindow->isClosed())	return false;

		mSpeedLimit = mMoveScale * evt.timeSinceLastFrame;

		//Need to capture/update each device
		mKeyboard->capture();
		mMouse->capture();
		if( mJoy ) mJoy->capture();

		bool buffJ = (mJoy) ? mJoy->buffered() : true;

		Ogre::Vector3 lastMotion = mTranslateVector;

		//Check if one of the devices is not buffered
		if( !mMouse->buffered() || !mKeyboard->buffered() || !buffJ )
		{
			// one of the input modes is immediate, so setup what is needed for immediate movement
			if (mTimeUntilNextToggle >= 0)
				mTimeUntilNextToggle -= evt.timeSinceLastFrame;

			// Move about 100 units per second
			mMoveScale = mMoveSpeed * evt.timeSinceLastFrame;
			// Take about 10 seconds for full rotation
			mRotScale = mRotateSpeed * evt.timeSinceLastFrame;

			mRotX = 0;
			mRotY = 0;
			mTranslateVector = Ogre::Vector3::ZERO;

		}

		//Check to see which device is not buffered, and handle it
#if OGRE_PLATFORM != OGRE_PLATFORM_IPHONE
		if( processUnbufferedKeyInput(evt) == false )
			return false;
#endif
		if( !mMouse->buffered() )
			if( processUnbufferedMouseInput(evt) == false )
				return false;

		// ramp up / ramp down speed
		if (mTranslateVector == Ogre::Vector3::ZERO)
		{
			// decay (one third speed)
			mCurrentSpeed -= evt.timeSinceLastFrame * 0.3;
			mTranslateVector = lastMotion;
		}
		else
		{
			// ramp up
			mCurrentSpeed += evt.timeSinceLastFrame;

		}
		// Limit motion speed
		if (mCurrentSpeed > 1.0)
			mCurrentSpeed = 1.0;
		if (mCurrentSpeed < 0.0)
			mCurrentSpeed = 0.0;

		mTranslateVector *= mCurrentSpeed;


		if( !mMouse->buffered() || !mKeyboard->buffered() || !buffJ )
			moveCamera();


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

		return true;

	}

};



class TerrainApplication : public ExampleApplication
{
public:
	TerrainApplication() {}

	~TerrainApplication()
	{
	}

protected:

	virtual void chooseSceneManager(void)
	{
		// Get the SceneManager, in this case a generic one
		mSceneMgr = mRoot->createSceneManager("TerrainSceneManager");
	}

	virtual void createCamera(void)
	{
		// Create the camera
		mCamera = mSceneMgr->createCamera("PlayerCam");

		// Position it at 500 in Z direction
		mCamera->setPosition(Vector3(128,25,128));
		// Look back along -Z
		mCamera->lookAt(Vector3(0,0,-300));
		mCamera->setNearClipDistance( 5 );
		mCamera->setFarClipDistance( 50000 );

	}
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

	// Just override the mandatory create scene method
	void createScene(void)
	{
		MaterialManager::getSingleton().setDefaultTextureFiltering(TFO_ANISOTROPIC);
		MaterialManager::getSingleton().setDefaultAnisotropy(7);

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

		bool saveTerrain = false;
		try
		{
			mTerrain = OGRE_NEW Terrain(mSceneMgr);
			mTerrain->load(TERRAIN_FILE);
		}
		catch (Exception& e)
		{
			OGRE_DELETE mTerrain;
			mTerrain = 0;
		}
		if (!mTerrain)
			mTerrain = createTerrain();

		//addTextureDebugOverlay(TextureManager::getSingleton().getByName(mTerrain->getTerrainNormalMap()->getName()), 0);

		//mWindow->getViewport(0)->setBackgroundColour(ColourValue::Blue);

		// Testing
		Vector3 terrainPos(1000,0,5000);
		mTerrain->setPosition(terrainPos);

		/*
		// create a few entities on the terrain
		for (int i = 0; i < 20; ++i)
		{
		Entity* e = mSceneMgr->createEntity("ninja.mesh");
		Real x = terrainPos.x + Math::RangeRandom(-2500, 2500);
		Real z = terrainPos.z + Math::RangeRandom(-2500, 2500);
		Real y = mTerrain->getHeightAtWorldPosition(Vector3(x, 0, z));
		mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(x, y, z))->attachObject(e);
		}
		*/



		mCamera->setPosition(terrainPos + Vector3(-1000,300,1000));
		mCamera->lookAt(terrainPos);
		mCamera->setNearClipDistance(5);
		mCamera->setFarClipDistance(50000);

		mSceneMgr->setSkyBox(true, "Examples/CloudyNoonSkyBox");

		//addTextureDebugOverlay(mTerrain->getCompositeMap()->getName(), 0);



	}
	// Create new frame listener
	void createFrameListener(void)
	{
		mFrameListener= new TerrainFrameListener(mWindow, mCamera);
		mRoot->addFrameListener(mFrameListener);
	}

};

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main(int argc, char *argv[])
#endif
{
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
        NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
        int retVal = UIApplicationMain(argc, argv, @"UIApplication", @"AppDelegate");
        [pool release];
        return retVal;
#else
    // Create application object
    TerrainApplication app;

    try {
        app.go();
    } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        std::cerr << "An exception has occured: " <<
            e.getFullDescription().c_str() << std::endl;
#endif
    }

    return 0;
#endif
}

#ifdef __cplusplus
}
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
#   ifdef __OBJC__
@interface AppDelegate : NSObject <UIApplicationDelegate>
{
}

- (void)go;

@end

@implementation AppDelegate

- (void)go {
    // Create application object
    TerrainApplication app;
    try {
        app.go();
    } catch( Ogre::Exception& e ) {
        std::cerr << "An exception has occured: " <<
        e.getFullDescription().c_str() << std::endl;
    }
}

- (void)applicationDidFinishLaunching:(UIApplication *)application {
    // Hide the status bar
    [[UIApplication sharedApplication] setStatusBarHidden:YES];

    // Create a window
    UIWindow *window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];

    // Create an image view
    UIImageView *imageView = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"Default.png"]];
    [window addSubview:imageView];
    
    // Create an indeterminate status indicator
    UIActivityIndicatorView *indicator = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleWhite];
    [indicator setFrame:CGRectMake(150, 280, 20, 20)];
    [indicator startAnimating];
    [window addSubview:indicator];
    
    // Display our window
    [window makeKeyAndVisible];
    
    // Clean up
    [imageView release];
    [indicator release];

    [NSThread detachNewThreadSelector:@selector(go) toTarget:self withObject:nil];
}

- (void)applicationWillTerminate:(UIApplication *)application {
    Root::getSingleton().queueEndRendering();
}

//- (void)applicationWillResignActive:(UIApplication *)application
//{
//    // Pause FrameListeners and rendering
//}
//
//- (void)applicationDidBecomeActive:(UIApplication *)application
//{
//    // Resume FrameListeners and rendering
//}

- (void)dealloc {
    [super dealloc];
}

@end
#   endif

#endif
