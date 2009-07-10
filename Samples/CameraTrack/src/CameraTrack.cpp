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
 Filename:    CameraTrack.cpp
 Description: An example of using AnimationTracks to smoothly make a node
 follow a predefined path, with spline interpolation. Also 
 uses the auto tracking ability of the camera.
 -----------------------------------------------------------------------------
 */

#include "ExampleApplication.h"

AnimationState* mAnimState;

// Event handler 
class CameraTrackListener: public ExampleFrameListener
{
protected:
public:
    CameraTrackListener(RenderWindow* win, Camera* cam)
    : ExampleFrameListener(win, cam)
    {
    }
    
    bool frameRenderingQueued(const FrameEvent& evt)
    {
        if( ExampleFrameListener::frameRenderingQueued(evt) == false )
            return false;
        
        mAnimState->addTime(evt.timeSinceLastFrame);
        
        return true;
    }
};

class CameraTrackApplication : public ExampleApplication
{
public:
    CameraTrackApplication() {
        
    }
    
    ~CameraTrackApplication() {  }
    
protected:
    
    // Just override the mandatory create scene method
    void createScene(void)
    {
        
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.2, 0.2, 0.2));
        
        // Create a skydome
        mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8);
        
        // Create a light
        Light* l = mSceneMgr->createLight("MainLight");
        // Accept default settings: point light, white diffuse, just set position
        // NB I could attach the light to a SceneNode if I wanted it to move automatically with
        //  other objects, but I don't
        l->setPosition(20,80,50);
        
        Entity *ent;
        
        // Define a floor plane mesh
        Plane p;
        p.normal = Vector3::UNIT_Y;
        p.d = 200;
        MeshManager::getSingleton().createPlane(
                                                "FloorPlane", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
                                                p, 200000, 200000, 20, 20, true, 1, 50, 50, Vector3::UNIT_Z);
        
        // Create an entity (the floor)
        ent = mSceneMgr->createEntity("floor", "FloorPlane");
        ent->setMaterialName("Examples/RustySteel");
        // Attach to child of root node, better for culling (otherwise bounds are the combination of the 2)
        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ent);
        
        // Add a head, give it it's own node
        SceneNode* headNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        ent = mSceneMgr->createEntity("head", "ogrehead.mesh");
        headNode->attachObject(ent);
        
        // Make sure the camera track this node
        mCamera->setAutoTracking(true, headNode);
        
        // Create the camera node & attach camera
        SceneNode* camNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        camNode->attachObject(mCamera);
        
        // set up spline animation of node
        Animation* anim = mSceneMgr->createAnimation("CameraTrack", 10);
        // Spline it for nice curves
        anim->setInterpolationMode(Animation::IM_SPLINE);
        // Create a track to animate the camera's node
        NodeAnimationTrack* track = anim->createNodeTrack(0, camNode);
        // Setup keyframes
        TransformKeyFrame* key = track->createNodeKeyFrame(0); // startposition
        key = track->createNodeKeyFrame(2.5);
        key->setTranslate(Vector3(500,500,-1000));
        key = track->createNodeKeyFrame(5);
        key->setTranslate(Vector3(-1500,1000,-600));
        key = track->createNodeKeyFrame(7.5);
        key->setTranslate(Vector3(0,-100,0));
        key = track->createNodeKeyFrame(10);
        key->setTranslate(Vector3(0,0,0));
        // Create a new animation state to track this
        mAnimState = mSceneMgr->createAnimationState("CameraTrack");
        mAnimState->setEnabled(true);
        
        // Put in a bit of fog for the hell of it
        mSceneMgr->setFog(FOG_EXP, ColourValue::White, 0.0002);
    }
    
    // Create new frame listener
    void createFrameListener(void)
    {
        mFrameListener= new CameraTrackListener(mWindow, mCamera);
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
        CameraTrackApplication app;
        try {
            app.go();
        } catch( Ogre::Exception& e ) {
            std::cerr << "An exception has occured: " <<
            e.getFullDescription().c_str() << std::endl;
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
    CameraTrackApplication app;
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
