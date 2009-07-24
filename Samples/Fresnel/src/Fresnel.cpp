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
/*
-----------------------------------------------------------------------------
Filename:    Fresnel.cpp
Description: Fresnel reflections and refractions
-----------------------------------------------------------------------------
*/

#include "ExampleApplication.h"
#include "OgreHardwarePixelBuffer.h"
// Hacky globals
Camera* theCam;
Entity* pPlaneEnt;
Ogre::vector<Entity*>::type aboveWaterEnts;
Ogre::vector<Entity*>::type belowWaterEnts;

// Fish!
#define NUM_FISH 30
#define NUM_FISH_WAYPOINTS 10
#define FISH_PATH_LENGTH 200 
#define FISH_SCALE 1.2
AnimationState* fishAnimations[NUM_FISH];
SimpleSpline fishSplines[NUM_FISH];
Vector3 fishLastPosition[NUM_FISH];
SceneNode* fishNodes[NUM_FISH];
Real animTime = 0.0f;


Plane reflectionPlane;


class RefractionTextureListener : public RenderTargetListener
{
public:
    void preRenderTargetUpdate(const RenderTargetEvent& evt)
    {
        // Hide plane and objects above the water
        pPlaneEnt->setVisible(false);
		vector<Entity*>::type::iterator i, iend;
        iend = aboveWaterEnts.end();
        for (i = aboveWaterEnts.begin(); i != iend; ++i)
        {
            (*i)->setVisible(false);
        }

    }
    void postRenderTargetUpdate(const RenderTargetEvent& evt)
    {
        // Show plane and objects above the water
        pPlaneEnt->setVisible(true);
		vector<Entity*>::type::iterator i, iend;
        iend = aboveWaterEnts.end();
        for (i = aboveWaterEnts.begin(); i != iend; ++i)
        {
            (*i)->setVisible(true);
        }
    }

};
class ReflectionTextureListener : public RenderTargetListener
{
public:
    void preRenderTargetUpdate(const RenderTargetEvent& evt)
    {
        // Hide plane and objects below the water
        pPlaneEnt->setVisible(false);
		vector<Entity*>::type::iterator i, iend;
        iend = belowWaterEnts.end();
        for (i = belowWaterEnts.begin(); i != iend; ++i)
        {
            (*i)->setVisible(false);
        }
        theCam->enableReflection(reflectionPlane);

    }
    void postRenderTargetUpdate(const RenderTargetEvent& evt)
    {
        // Show plane and objects below the water
        pPlaneEnt->setVisible(true);
		vector<Entity*>::type::iterator i, iend;
        iend = belowWaterEnts.end();
        for (i = belowWaterEnts.begin(); i != iend; ++i)
        {
            (*i)->setVisible(true);
        }
        theCam->disableReflection();
    }

};

class FresnelFrameListener : public ExampleFrameListener
{
public:

    FresnelFrameListener(RenderWindow* win, Camera* cam)
        : ExampleFrameListener(win, cam, false, false)
    {}
    bool frameRenderingQueued(const FrameEvent &evt)
    {
        animTime += evt.timeSinceLastFrame;
        while (animTime > FISH_PATH_LENGTH)
            animTime -= FISH_PATH_LENGTH;

        for (size_t fish = 0; fish < NUM_FISH; ++fish)
        {
            // Animate the fish
            fishAnimations[fish]->addTime(evt.timeSinceLastFrame*2);
            // Move the fish
            Vector3 newPos = fishSplines[fish].interpolate(animTime / FISH_PATH_LENGTH);
            fishNodes[fish]->setPosition(newPos);
            // Work out the direction
            Vector3 direction = fishLastPosition[fish] - newPos;
            direction.normalise();
			// Test for opposite vectors
			Real d = 1.0f + Vector3::UNIT_X.dotProduct(direction);
			if (fabs(d) < 0.00001)
			{
				// Diametrically opposed vectors
				Quaternion orientation;
				orientation.FromAxes(Vector3::NEGATIVE_UNIT_X, 
					Vector3::UNIT_Y, Vector3::NEGATIVE_UNIT_Z);
				fishNodes[fish]->setOrientation(orientation);
			}
			else
			{
				fishNodes[fish]->setOrientation(
					Vector3::UNIT_X.getRotationTo(direction));
			}
            fishLastPosition[fish] = newPos;

        }



        return ExampleFrameListener::frameRenderingQueued(evt);
    }

};

class FresnelApplication : public ExampleApplication
{
protected:
    RefractionTextureListener mRefractionListener;
    ReflectionTextureListener mReflectionListener;
public:
    FresnelApplication() {
    
    
    }

    ~FresnelApplication() 
    {
    }
protected:
    


    // Just override the mandatory create scene method
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
                !GpuProgramManager::getSingleton().isSyntaxSupported("ps_2_0") &&
				!GpuProgramManager::getSingleton().isSyntaxSupported("ps_1_4")
				)
            {
                OGRE_EXCEPT(1, "Your card does not support advanced fragment programs, "
                    "so cannot run this demo. Sorry!", 
                "Fresnel::createScene");
            }
        }

        theCam = mCamera;
        theCam->setPosition(-50,125,760);
		theCam->setDirection (0,0,-1);
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

        // Create a point light
        Light* l = mSceneMgr->createLight("MainLight");
        l->setType(Light::LT_DIRECTIONAL);
        l->setDirection(-Vector3::UNIT_Y);

        Entity* pEnt;

        TexturePtr mTexture = TextureManager::getSingleton().createManual( "Refraction", 
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_2D, 
			512, 512, 0, PF_R8G8B8, TU_RENDERTARGET );
        //RenderTexture* rttTex = mRoot->getRenderSystem()->createRenderTexture( "Refraction", 512, 512 );
        RenderTarget *rttTex = mTexture->getBuffer()->getRenderTarget();
		
        {
            Viewport *v = rttTex->addViewport( mCamera );
            MaterialPtr mat = MaterialManager::getSingleton().getByName("Examples/FresnelReflectionRefraction");
            mat->getTechnique(0)->getPass(0)->getTextureUnitState(2)->setTextureName("Refraction");
            v->setOverlaysEnabled(false);
            rttTex->addListener(&mRefractionListener);
        }
        
		mTexture = TextureManager::getSingleton().createManual( "Reflection", 
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_2D, 
			512, 512, 0, PF_R8G8B8, TU_RENDERTARGET );
        //rttTex = mRoot->getRenderSystem()->createRenderTexture( "Reflection", 512, 512 );
        rttTex = mTexture->getBuffer()->getRenderTarget();
        {
            Viewport *v = rttTex->addViewport( mCamera );
            MaterialPtr mat = MaterialManager::getSingleton().getByName("Examples/FresnelReflectionRefraction");
            mat->getTechnique(0)->getPass(0)->getTextureUnitState(1)->setTextureName("Reflection");
            v->setOverlaysEnabled(false);
            rttTex->addListener(&mReflectionListener);
        }
        
        
        // Define a floor plane mesh
        reflectionPlane.normal = Vector3::UNIT_Y;
        reflectionPlane.d = 0;
        MeshManager::getSingleton().createPlane("ReflectPlane",
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            reflectionPlane,
            700,1300,10,10,true,1,3,5,Vector3::UNIT_Z);
        pPlaneEnt = mSceneMgr->createEntity( "plane", "ReflectPlane" );
        pPlaneEnt->setMaterialName("Examples/FresnelReflectionRefraction");
        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);

        
        mSceneMgr->setSkyBox(true, "Examples/CloudyNoonSkyBox");

        // My node to which all objects will be attached
        SceneNode* myRootNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();

		// above water entities
        pEnt = mSceneMgr->createEntity( "RomanBathUpper", "RomanBathUpper.mesh" );
		myRootNode->attachObject(pEnt);        
        aboveWaterEnts.push_back(pEnt);

        pEnt = mSceneMgr->createEntity( "Columns", "Columns.mesh" );
		myRootNode->attachObject(pEnt);        
        aboveWaterEnts.push_back(pEnt);

		Ogre::SceneNode *headNode = myRootNode->createChildSceneNode ();
		pEnt = mSceneMgr->createEntity( "OgreHead", "ogrehead.mesh" );
		pEnt->setMaterialName ("RomanBath/OgreStone");
        headNode->attachObject(pEnt);
		headNode->setPosition(-350,55,130);
		headNode->rotate(Vector3::UNIT_Y, Degree (90));
        aboveWaterEnts.push_back(pEnt);

		// below water entities
		pEnt = mSceneMgr->createEntity( "RomanBathLower", "RomanBathLower.mesh" );
        myRootNode->attachObject(pEnt);
        belowWaterEnts.push_back(pEnt);

		for (size_t fishNo = 0; fishNo < NUM_FISH; ++fishNo)
        {
            pEnt = mSceneMgr->createEntity("fish" + StringConverter::toString(fishNo), "fish.mesh");
            fishNodes[fishNo] = myRootNode->createChildSceneNode();
			fishNodes[fishNo]->setScale(FISH_SCALE, FISH_SCALE, FISH_SCALE);
            fishAnimations[fishNo] = pEnt->getAnimationState("swim");
            fishAnimations[fishNo]->setEnabled(true);
            fishNodes[fishNo]->attachObject(pEnt);
            belowWaterEnts.push_back(pEnt);

            // Generate a random selection of points for the fish to swim to
            fishSplines[fishNo].setAutoCalculate(false);
            Vector3 lastPos = Vector3::ZERO;
            for (size_t waypoint = 0; waypoint < NUM_FISH_WAYPOINTS; ++waypoint)
            {
                Vector3 pos = Vector3(
                    Math::SymmetricRandom() * 270, -10, Math::SymmetricRandom() * 700);
                if (waypoint > 0)
                {
                    // check this waypoint isn't too far, we don't want turbo-fish ;)
                    // since the waypoints are achieved every 5 seconds, half the length
                    // of the pond is ok
                    while ((lastPos - pos).length() > 750)
                    {
                        pos = Vector3(
                            Math::SymmetricRandom() * 270, -10, Math::SymmetricRandom() * 700);
                    }
                }
                fishSplines[fishNo].addPoint(pos);
                lastPos = pos;
            }
            // close the spline
            fishSplines[fishNo].addPoint(fishSplines[fishNo].getPoint(0));
            // recalc
            fishSplines[fishNo].recalcTangents();


        }




    }

    void createFrameListener(void)
    {
        mFrameListener= new FresnelFrameListener(mWindow, mCamera);
        mFrameListener->showDebugOverlay(true);
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
    FresnelApplication app;

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
    FresnelApplication app;
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
