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
    \file 
        Grass.cpp
    \brief
        Specialisation of OGRE's framework application to show the
        use of the StaticGeometry class to create 'baked' instances of
		many meshes, to create effects like grass efficiently.
**/

#include "ExampleApplication.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif


#define KEY_PRESSED(_key,_timeDelay, _macro) \
{ \
    if (mKeyboard->isKeyDown(_key) && timeDelay <= 0) \
    { \
		timeDelay = _timeDelay; \
        _macro ; \
    } \
}

#define GRASS_HEIGHT 300
#define GRASS_WIDTH 250
#define GRASS_MESH_NAME "grassblades"
#define GRASS_MATERIAL "Examples/GrassBlades"
#define OFFSET_PARAM 999

Light* mLight;
SceneNode* mLightNode = 0;
AnimationState* mAnimState = 0;
ColourValue mMinLightColour(0.5, 0.1, 0.0);
ColourValue mMaxLightColour(1.0, 0.6, 0.0);
Real mMinFlareSize = 40;
Real mMaxFlareSize = 80;
StaticGeometry* mStaticGeom;


/** This class 'wibbles' the light and billboard */
class LightWibbler : public ControllerValue<Real>
{
protected:
	Light* mLight;
	Billboard* mBillboard;
	ColourValue mColourRange;
	ColourValue mHalfColour;
	Real mMinSize;
	Real mSizeRange;
	Real intensity;
public:
	LightWibbler(Light* light, Billboard* billboard, const ColourValue& minColour, 
		const ColourValue& maxColour, Real minSize, Real maxSize)
	{
		mLight = light;
		mBillboard = billboard;
		mColourRange.r = (maxColour.r - minColour.r) * 0.5;
		mColourRange.g = (maxColour.g - minColour.g) * 0.5;
		mColourRange.b = (maxColour.b - minColour.b) * 0.5;
		mHalfColour = minColour + mColourRange;
		mMinSize = minSize;
		mSizeRange = maxSize - minSize;
	}

	virtual Real  getValue (void) const
	{
		return intensity;
	}

	virtual void  setValue (Real value)
	{
		intensity = value;

		ColourValue newColour;

		// Attenuate the brightness of the light
		newColour.r = mHalfColour.r + (mColourRange.r * intensity);
		newColour.g = mHalfColour.g + (mColourRange.g * intensity);
		newColour.b = mHalfColour.b + (mColourRange.b * intensity);

		mLight->setDiffuseColour(newColour);
		mBillboard->setColour(newColour);
		// set billboard size
		Real newSize = mMinSize + (intensity * mSizeRange);
		mBillboard->setDimensions(newSize, newSize);

	}
};

class GrassListener : public ExampleFrameListener
{
protected:
	SceneManager* mSceneManager;
	bool mShowBBs;
public:
	GrassListener(RenderWindow* win, Camera* cam, SceneManager* sceneManager)
		: ExampleFrameListener(win, cam), 
		mSceneManager(sceneManager), mShowBBs(false)
	{
	}


	void waveGrass(Real timeElapsed)
	{
		static Real xinc = Math::PI * 0.4;
		static Real zinc = Math::PI * 0.55;
		static Real xpos = Math::RangeRandom(-Math::PI, Math::PI);
		static Real zpos = Math::RangeRandom(-Math::PI, Math::PI);

		xpos += xinc * timeElapsed;
		zpos += zinc * timeElapsed;

		// Update vertex program parameters by binding a value to each renderable
		static Vector4 offset(0,0,0,0);

		StaticGeometry::RegionIterator rit =  mStaticGeom->getRegionIterator();
		while (rit.hasMoreElements())
		{
			StaticGeometry::Region* reg = rit.getNext();

			// a little randomness
			xpos += reg->getCentre().x * 0.001;
			zpos += reg->getCentre().z * 0.001;
			offset.x = Math::Sin(xpos) * 5;
			offset.z = Math::Sin(zpos) * 5;

			StaticGeometry::Region::LODIterator lodit = reg->getLODIterator();
			while (lodit.hasMoreElements())
			{
				StaticGeometry::LODBucket* lod = lodit.getNext();
				StaticGeometry::LODBucket::MaterialIterator matit = 
					lod->getMaterialIterator();
				while (matit.hasMoreElements())
				{
					StaticGeometry::MaterialBucket* mat = matit.getNext();
					StaticGeometry::MaterialBucket::GeometryIterator geomit = 
						mat->getGeometryIterator();
					while (geomit.hasMoreElements())
					{
						StaticGeometry::GeometryBucket* geom = geomit.getNext();
						geom->setCustomParameter(OFFSET_PARAM, offset);

					}
				}
			}
		}

	}

	bool frameRenderingQueued(const FrameEvent& evt)
	{
		if( ExampleFrameListener::frameRenderingQueued(evt) == false )
			return false;

		static Real timeDelay = 0;
		timeDelay -= evt.timeSinceLastFrame;

		if (mAnimState)
			mAnimState->addTime(evt.timeSinceLastFrame);

#if OGRE_PLATFORM != OGRE_PLATFORM_IPHONE
		KEY_PRESSED(OIS::KC_B, 1, 
			mShowBBs = !mShowBBs;
			mSceneManager->showBoundingBoxes(mShowBBs);
			)
#endif
		waveGrass(evt.timeSinceLastFrame);

		return true;
	}
};



class Grass_Application : public ExampleApplication
{
public:
    Grass_Application() {}
	
protected:
	SceneNode *mpObjsNode; // the node which will hold our entities

	void createGrassMesh()
	{
		// Each grass section is 3 planes at 60 degrees to each other
		// Normals point straight up to simulate correct lighting
		MeshPtr msh = MeshManager::getSingleton().createManual(GRASS_MESH_NAME, 
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		SubMesh* sm = msh->createSubMesh();
		sm->useSharedVertices = false;
		sm->vertexData = new VertexData();
		sm->vertexData->vertexStart = 0;
		sm->vertexData->vertexCount = 12;
		VertexDeclaration* dcl = sm->vertexData->vertexDeclaration;
		size_t offset = 0;
		dcl->addElement(0, offset, VET_FLOAT3, VES_POSITION);
		offset += VertexElement::getTypeSize(VET_FLOAT3);
		dcl->addElement(0, offset, VET_FLOAT3, VES_NORMAL);
		offset += VertexElement::getTypeSize(VET_FLOAT3);
		dcl->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES);
		offset += VertexElement::getTypeSize(VET_FLOAT2);

		HardwareVertexBufferSharedPtr vbuf = HardwareBufferManager::getSingleton()
			.createVertexBuffer(
				offset, 12, HardwareBuffer::HBU_STATIC_WRITE_ONLY);
		float* pReal = static_cast<float*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));
		Vector3 baseVec(GRASS_WIDTH/2, 0, 0);
		Vector3 vec = baseVec;
		Quaternion rot;
		rot.FromAngleAxis(Degree(60), Vector3::UNIT_Y);
		int i;
		for (i = 0; i < 3; ++i)
		{
			// position
			*pReal++ = -vec.x;
			*pReal++ = GRASS_HEIGHT;
			*pReal++ = -vec.z;
			// normal
			*pReal++ = 0;
			*pReal++ = 1;
			*pReal++ = 0;
			// uv
			*pReal++ = 0;
			*pReal++ = 0;

			// position
			*pReal++ = vec.x;
			*pReal++ = GRASS_HEIGHT;
			*pReal++ = vec.z;
			// normal
			*pReal++ = 0;
			*pReal++ = 1;
			*pReal++ = 0;
			// uv
			*pReal++ = 1;
			*pReal++ = 0;

			// position
			*pReal++ = -vec.x;
			*pReal++ = 0;
			*pReal++ = -vec.z;
			// normal
			*pReal++ = 0;
			*pReal++ = 1;
			*pReal++ = 0;
			// uv
			*pReal++ = 0;
			*pReal++ = 1;

			// position
			*pReal++ = vec.x;
			*pReal++ = 0;
			*pReal++ = vec.z;
			// normal
			*pReal++ = 0;
			*pReal++ = 1;
			*pReal++ = 0;
			// uv
			*pReal++ = 1;
			*pReal++ = 1;

			vec = rot * vec;
		}
		vbuf->unlock();
		sm->vertexData->vertexBufferBinding->setBinding(0, vbuf);
		sm->indexData->indexCount = 6*3;
		sm->indexData->indexBuffer = HardwareBufferManager::getSingleton()
			.createIndexBuffer(HardwareIndexBuffer::IT_16BIT, 6*3,
				HardwareBuffer::HBU_STATIC_WRITE_ONLY);
		Ogre::uint16* pI = static_cast<Ogre::uint16*>(
			sm->indexData->indexBuffer->lock(HardwareBuffer::HBL_DISCARD));
		for (i = 0; i < 3; ++i)
		{
			int off = i*4;
			*pI++ = 0 + off;
			*pI++ = 3 + off;
			*pI++ = 1 + off;

			*pI++ = 0 + off;
			*pI++ = 2 + off;
			*pI++ = 3 + off;
		}

		sm->indexData->indexBuffer->unlock();

		sm->setMaterialName(GRASS_MATERIAL);
		msh->load();

	}

	void setupLighting()
	{
		// Set ambient light
		mSceneMgr->setAmbientLight(ColourValue::Black);
		// Point light, movable, reddish
		mLight = mSceneMgr->createLight("Light2");
		mLight->setDiffuseColour(mMinLightColour);
		mLight->setSpecularColour(1, 1, 1);
		mLight->setAttenuation(8000,1,0.0005,0);

		// Create light node
		mLightNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(
			"MovingLightNode");
		mLightNode->attachObject(mLight);
		// create billboard set
		BillboardSet* bbs = mSceneMgr->createBillboardSet("lightbbs", 1);
		bbs->setMaterialName("Examples/Flare");
		Billboard* bb = bbs->createBillboard(0,0,0,mMinLightColour);
		// attach
		mLightNode->attachObject(bbs);

		// create controller, after this is will get updated on its own
		ControllerFunctionRealPtr func = ControllerFunctionRealPtr(
			new WaveformControllerFunction(Ogre::WFT_SINE, 0.0, 0.5));
		ControllerManager& contMgr = ControllerManager::getSingleton();
		ControllerValueRealPtr val = ControllerValueRealPtr(
			new LightWibbler(mLight, bb, mMinLightColour, mMaxLightColour, 
			mMinFlareSize, mMaxFlareSize));
		Controller<Real>* controller = contMgr.createController(
			contMgr.getFrameTimeSource(), val, func);

        (void)controller;   // Silence warning

		//mLight->setPosition(Vector3(300,250,-300));
		mLightNode->setPosition(Vector3(300,250,-300));


		// Create a track for the light
		Animation* anim = mSceneMgr->createAnimation("LightTrack", 20);
		// Spline it for nice curves
		anim->setInterpolationMode(Animation::IM_SPLINE);
		// Create a track to animate the camera's node
		NodeAnimationTrack* track = anim->createNodeTrack(0, mLightNode);
		// Setup keyframes
		TransformKeyFrame* key = track->createNodeKeyFrame(0); // A startposition
		key->setTranslate(Vector3(300,550,-300));
		key = track->createNodeKeyFrame(2);//B
		key->setTranslate(Vector3(150,600,-250));
		key = track->createNodeKeyFrame(4);//C
		key->setTranslate(Vector3(-150,650,-100));
		key = track->createNodeKeyFrame(6);//D
		key->setTranslate(Vector3(-400,500,-200));
		key = track->createNodeKeyFrame(8);//E
		key->setTranslate(Vector3(-200,500,-400));
		key = track->createNodeKeyFrame(10);//F
		key->setTranslate(Vector3(-100,450,-200));
		key = track->createNodeKeyFrame(12);//G
		key->setTranslate(Vector3(-100,400,180));
		key = track->createNodeKeyFrame(14);//H
		key->setTranslate(Vector3(0,250,600));
		key = track->createNodeKeyFrame(16);//I
		key->setTranslate(Vector3(100,650,100));
		key = track->createNodeKeyFrame(18);//J
		key->setTranslate(Vector3(250,600,0));
		key = track->createNodeKeyFrame(20);//K == A
		key->setTranslate(Vector3(300,550,-300));
		// Create a new animation state to track this
		mAnimState = mSceneMgr->createAnimationState("LightTrack");
		mAnimState->setEnabled(true);
	}

	void createScene(void)
    {

		mSceneMgr->setSkyBox(true, "Examples/SpaceSkyBox");

		setupLighting();


		Plane plane;
		plane.normal = Vector3::UNIT_Y;
		plane.d = 0;
		MeshManager::getSingleton().createPlane("Myplane",
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane,
			14500,14500,10,10,true,1,50,50,Vector3::UNIT_Z);
		Entity* pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
		pPlaneEnt->setMaterialName("Examples/GrassFloor");
		pPlaneEnt->setCastShadows(false);
		mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);

		Vector3 minV(-2000,0,-2000);
		Vector3 maxV(2000,0,2000);


		createGrassMesh();

		Entity* e = mSceneMgr->createEntity("1", GRASS_MESH_NAME);

		StaticGeometry* s = mSceneMgr->createStaticGeometry("bing");
		s->setRegionDimensions(Vector3(1000,1000,1000));
		// Set the region origin so the centre is at 0 world
		s->setOrigin(Vector3(-500, 500, -500));

		for (int x = -1950; x < 1950; x += 150)
		{
			for (int z = -1950; z < 1950; z += 150)
			{
				Vector3 pos(
					x + Math::RangeRandom(-25, 25), 
					0, 
					z + Math::RangeRandom(-25, 25));
				Quaternion orientation;
				orientation.FromAngleAxis(
					Degree(Math::RangeRandom(0, 359)),
					Vector3::UNIT_Y);
				Vector3 scale(
					1, Math::RangeRandom(0.85, 1.15), 1);
				s->addEntity(e, pos, orientation, scale);
			}

		}

		s->build();
		mStaticGeom = s;

		// Put an Ogre head in the middle
		MeshPtr m = MeshManager::getSingleton().load("ogrehead.mesh", 
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		unsigned short src, dest;
		if (!m->suggestTangentVectorBuildParams(VES_TANGENT, src, dest))
		{
			m->buildTangentVectors(VES_TANGENT, src, dest);
		}
		e = mSceneMgr->createEntity("head", "ogrehead.mesh");
		e->setMaterialName("Examples/OffsetMapping/Specular");
		SceneNode* headNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		headNode->attachObject(e);
		headNode->setScale(7,7,7);
		headNode->setPosition(0,200,0);
		headNode->yaw(Degree(15));
		mCamera->move(Vector3(0,350,0));
	}

    // Create new frame listener
    void createFrameListener(void)
    {
        mFrameListener= new GrassListener(mWindow, mCamera, mSceneMgr);
        mRoot->addFrameListener(mFrameListener);
    }
};

#ifdef __cplusplus
extern "C" {
#endif

#include "OgreErrorDialog.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main(int argc, char **argv)
#endif
{
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
        NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
        int retVal = UIApplicationMain(argc, argv, @"UIApplication", @"AppDelegate");
        [pool release];
        return retVal;
#else
    // Create application object
    Grass_Application app;
    
    try {
        app.go();
    } catch( Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        std::cerr << "An exception has occured: " << e.getFullDescription();
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
    Grass_Application app;
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
