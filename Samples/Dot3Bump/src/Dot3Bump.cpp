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
        Dot3Bump.cpp
    \brief
        Specialisation of OGRE's framework application to show the
        dotproduct blending operation and normalization cube map usage
		for achieving bump mapping effect
	\par
		Tangent space computations made through the guide of the
		tutorial on bump mapping from http://users.ox.ac.uk/~univ1234
		author : paul.baker@univ.ox.ac.uk
**/

#include "ExampleApplication.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

// entities we'll use
#define NUM_ENTITIES 3
Entity* mEntities[NUM_ENTITIES];
String mEntityMeshes[NUM_ENTITIES] = 
{
    "athene.mesh",
    "knot.mesh",
    "ogrehead.mesh"
};
size_t mCurrentEntity = 0;

// Lights
#define NUM_LIGHTS 3

// the light
Light *mLights[NUM_LIGHTS];
// billboards for lights
BillboardSet* mLightFlareSets[NUM_LIGHTS];
Billboard* mLightFlares[NUM_LIGHTS];
// Positions for lights
Vector3 mLightPositions[NUM_LIGHTS] = 
{
	Vector3(300,0,0),
	Vector3(-300,50,0),
	Vector3(0, -300, -100)
};
// Base orientations of the lights 
Radian mLightRotationAngles[NUM_LIGHTS] = { Degree(0), Degree(30), Degree(75) };
Vector3 mLightRotationAxes[NUM_LIGHTS] = {
    Vector3::UNIT_X, 
    Vector3::UNIT_Z,
    Vector3::UNIT_Y
};
// Rotation speed for lights, degrees per second
Real mLightSpeeds[NUM_LIGHTS] = { 30, 10, 50};

// Colours for the lights
ColourValue mDiffuseLightColours[NUM_LIGHTS] =
{
	ColourValue(1, 1, 1),
	ColourValue(1, 0, 0),
	ColourValue(1, 1, 0.5)
};
ColourValue mSpecularLightColours[NUM_LIGHTS] =
{
	ColourValue(1, 1, 1),
	ColourValue(1, 0.8, 0.8),
	ColourValue(1, 1, 0.8)
};
// Which lights are enabled
bool mLightState[NUM_LIGHTS] = 
{
	true,
	true,
	false
};
// The materials
#define NUM_MATERIALS 6
String mMaterialNames[NUM_ENTITIES][NUM_MATERIALS] = 
{
    // athene
    { "Examples/Athene/NormalMapped",
    "Examples/Athene/NormalMappedSpecular",
    "Examples/Athene/NormalMapped",
	"Examples/ShowUV",
	"Examples/ShowNormals",
	"Examples/ShowTangents" },
    // knot
    { "Examples/BumpMapping/MultiLight",
	"Examples/BumpMapping/MultiLightSpecular",
    "Examples/OffsetMapping/Specular",
	"Examples/ShowUV",
	"Examples/ShowNormals",
	"Examples/ShowTangents" },
    // ogre head
    { "Examples/BumpMapping/MultiLight",
    "Examples/BumpMapping/MultiLightSpecular",
    "Examples/OffsetMapping/Specular",
	"Examples/ShowUV",
	"Examples/ShowNormals",
	"Examples/ShowTangents" }
};
size_t mCurrentMaterial = 0;

// the scene node of the entity
SceneNode *mMainNode;
// the light nodes
SceneNode* mLightNodes[NUM_LIGHTS];
// the light node pivots
SceneNode* mLightPivots[NUM_LIGHTS];

OverlayElement* mObjectInfo;
OverlayElement* mMaterialInfo;
OverlayElement* mInfo;

#define KEY_PRESSED(_key,_timeDelay, _macro) \
{ \
    if (mKeyboard->isKeyDown(_key) && timeDelay <= 0) \
    { \
		timeDelay = _timeDelay; \
        _macro ; \
    } \
}



// Event handler to add ability to change material
class Dp3_Listener : public ExampleFrameListener
{
public:
    Dp3_Listener(RenderWindow* win, Camera* cam)
        : ExampleFrameListener(win, cam)
    {
    }

    void flipLightState(size_t i)
    {
        mLightState[i] = !mLightState[i];
        mLights[i]->setVisible(mLightState[i]);
        mLightFlareSets[i]->setVisible(mLightState[i]);
    }
    bool frameRenderingQueued(const FrameEvent& evt)
    {

        if(!ExampleFrameListener::frameRenderingQueued(evt))
            return false;
        
        static Real timeDelay = 0;

        timeDelay -= evt.timeSinceLastFrame;

		// switch meshes
        KEY_PRESSED(OIS::KC_O, 1, 
            mEntities[mCurrentEntity]->setVisible(false); 
            mCurrentEntity = (mCurrentEntity + 1) % NUM_ENTITIES; 
            mEntities[mCurrentEntity]->setVisible(true);
            mEntities[mCurrentEntity]->setMaterialName(mMaterialNames[mCurrentEntity][mCurrentMaterial]);
            mObjectInfo->setCaption("Current: " + mEntityMeshes[mCurrentEntity]);
            mMaterialInfo->setCaption("Current: " + mMaterialNames[mCurrentEntity][mCurrentMaterial]);
        );

		// switch materials
		KEY_PRESSED(OIS::KC_M, 1, 
            mCurrentMaterial = (mCurrentMaterial + 1) % NUM_MATERIALS; 
            mEntities[mCurrentEntity]->setMaterialName(mMaterialNames[mCurrentEntity][mCurrentMaterial]);
            mMaterialInfo->setCaption("Current: " + mMaterialNames[mCurrentEntity][mCurrentMaterial]);
        );

		// enable / disable lights
		KEY_PRESSED(OIS::KC_1, 1, flipLightState(0));
		// switch materials
		KEY_PRESSED(OIS::KC_2, 1, flipLightState(1));
		// switch materials
		KEY_PRESSED(OIS::KC_3, 1, flipLightState(2));

        // animate the lights
        for (size_t i = 0; i < NUM_LIGHTS; ++i)
            mLightPivots[i]->rotate(Ogre::Vector3::UNIT_Z, Degree(mLightSpeeds[i] * evt.timeSinceLastFrame));

		return true;
    }

};

class Dp3_Application : public ExampleApplication
{
public:
    Dp3_Application() {}
	
protected:
	SceneNode *mpObjsNode; // the node wich will hold our entities

	void createScene(void)
    {
		// Check prerequisites first
		const RenderSystemCapabilities* caps = Root::getSingleton().getRenderSystem()->getCapabilities();
		if (!caps->hasCapability(RSC_VERTEX_PROGRAM) || !(caps->hasCapability(RSC_FRAGMENT_PROGRAM)))
		{
			OGRE_EXCEPT(1, "Your card does not support vertex and fragment programs, so cannot "
				"run this demo. Sorry!", 
				"Fresnel::createScene");
		}
		else
		{
			if (!GpuProgramManager::getSingleton().isSyntaxSupported("arbfp1") &&
				!GpuProgramManager::getSingleton().isSyntaxSupported("ps_2_0"))
			{
				OGRE_EXCEPT(1, "Your card does not support shader model 2, "
					"so cannot run this demo. Sorry!", 
					"Fresnel::createScene");
			}
		}

        // Set ambient light and fog
        mSceneMgr->setAmbientLight(ColourValue(0.0, 0.0, 0.0));
        /*
		// Define a floor plane mesh
        Plane p;
        p.normal = Vector3::UNIT_Y;
        p.d = 200;
        MeshManager::getSingleton().createPlane("FloorPlane",p,2000,2000,1,1,true,1,5,5,Vector3::UNIT_Z);
        // Create an entity (the floor)
        Entity *floorEnt = mSceneMgr->createEntity("floor", "FloorPlane");
        floorEnt->setMaterialName("Examples/DP3Terrain");
        mSceneMgr->getRootSceneNode()->attachObject(floorEnt);
        */

        mMainNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();

        // Load the meshes with non-default HBU options
		for(int mn = 0; mn < NUM_ENTITIES; mn++) {
			MeshPtr pMesh = MeshManager::getSingleton().load(mEntityMeshes[mn],
                ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,    
                HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY, 
				HardwareBuffer::HBU_STATIC_WRITE_ONLY, 
				true, true); //so we can still read it
            // Build tangent vectors, all our meshes use only 1 texture coordset 
			// Note we can build into VES_TANGENT now (SM2+)
            unsigned short src, dest;
            if (!pMesh->suggestTangentVectorBuildParams(VES_TANGENT, src, dest))
            {
                pMesh->buildTangentVectors(VES_TANGENT, src, dest);
				// Second mode cleans mirrored / rotated UVs but requires quality models
				//pMesh->buildTangentVectors(VES_TANGENT, src, dest, true, true);
            }
            // Create entity
            mEntities[mn] = mSceneMgr->createEntity("Ent" + StringConverter::toString(mn), 
                mEntityMeshes[mn]);
            // Attach to child of root node
    		mMainNode->attachObject(mEntities[mn]);
            // Make invisible, except for index 0
            if (mn == 0)
                mEntities[mn]->setMaterialName(mMaterialNames[mCurrentEntity][mCurrentMaterial]);
            else
                mEntities[mn]->setVisible(false);
		}

        for (unsigned int i = 0; i < NUM_LIGHTS; ++i)
        {
            mLightPivots[i] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
            mLightPivots[i]->rotate(mLightRotationAxes[i], mLightRotationAngles[i]);
            // Create a light, use default parameters
            mLights[i] = mSceneMgr->createLight("Light" + StringConverter::toString(i));
			mLights[i]->setPosition(mLightPositions[i]);
			mLights[i]->setDiffuseColour(mDiffuseLightColours[i]);
			mLights[i]->setSpecularColour(mSpecularLightColours[i]);
			mLights[i]->setVisible(mLightState[i]);
            // Attach light
            mLightPivots[i]->attachObject(mLights[i]);
			// Create billboard for light
			mLightFlareSets[i] = mSceneMgr->createBillboardSet("Flare" + StringConverter::toString(i));
			mLightFlareSets[i]->setMaterialName("Examples/Flare");
			mLightPivots[i]->attachObject(mLightFlareSets[i]);
			mLightFlares[i] = mLightFlareSets[i]->createBillboard(mLightPositions[i]);
			mLightFlares[i]->setColour(mDiffuseLightColours[i]);
			mLightFlareSets[i]->setVisible(mLightState[i]);
        }
        // move the camera a bit right and make it look at the knot
		mCamera->moveRelative(Vector3(50, 0, 20));
		mCamera->lookAt(0, 0, 0);
		// show overlay
		Overlay* pOver = OverlayManager::getSingleton().getByName("Example/DP3Overlay");    
        mObjectInfo = OverlayManager::getSingleton().getOverlayElement("Example/DP3/ObjectInfo");
        mMaterialInfo = OverlayManager::getSingleton().getOverlayElement("Example/DP3/MaterialInfo");
        mInfo = OverlayManager::getSingleton().getOverlayElement("Example/DP3/Info");

        mObjectInfo->setCaption("Current: " + mEntityMeshes[mCurrentEntity]);
        mMaterialInfo->setCaption("Current: " + mMaterialNames[mCurrentEntity][mCurrentMaterial]);
        if (!caps->hasCapability(RSC_FRAGMENT_PROGRAM))
        {
            mInfo->setCaption("NOTE: Light colours and specular highlights are not supported by your card.");
        }
		pOver->show();
	}

    // Create new frame listener
    void createFrameListener(void)
    {
        mFrameListener= new Dp3_Listener(mWindow, mCamera);
        mRoot->addFrameListener(mFrameListener);
    }
};

#ifdef __cplusplus
extern "C" {
#endif

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
    Dp3_Application app;

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
    Dp3_Application app;
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