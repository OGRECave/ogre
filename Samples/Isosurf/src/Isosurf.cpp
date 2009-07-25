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
Filename:    IsoSurf.cpp
Description: Demonstrates the use of the geometry shader to tessellate an 
	isosurface using marching tetrahedrons. Partial implementation of cg 
	Isosurf sample from NVIDIA's OpenGL SDK 10 : 
	http://developer.download.nvidia.com/SDK/10/opengl/samples.html
-----------------------------------------------------------------------------
*/

#include "ExampleApplication.h"

#include "ProceduralTools.h"

Entity* tetrahedra;

class TetraHedraShaderListener : public FrameListener
{
	virtual bool frameStarted(const FrameEvent& evt) 
	{ 
		Real seconds = (Real)(Root::getSingleton().getTimer()->getMilliseconds()) / 1000.0;
		Ogre::Pass* renderPass = tetrahedra->getSubEntity(0)->getMaterial()->getTechnique(0)->getPass(0);
		if (renderPass->hasVertexProgram())
		{
			Ogre::Vector4 constParam = Ogre::Vector4(-0.5, 0.0, 0.0, 0.2);
			renderPass->getVertexProgramParameters()->setNamedConstant("Metaballs[0]", constParam);

			Ogre::Vector4 timeParam = Ogre::Vector4(
				0.1 + Ogre::Math::Sin(seconds)*0.5, Ogre::Math::Cos(seconds)*0.5, 0.0, 0.1);
			renderPass->getVertexProgramParameters()->setNamedConstant("Metaballs[1]", timeParam);
		}
		return true; 
	}
};


class IsoSurfApplication : public ExampleApplication
{
public:
    IsoSurfApplication() { 
    }

    ~IsoSurfApplication() {  }
protected:

    // Just override the mandatory create scene method
    void createScene(void)
    {
        // Check capabilities
		const RenderSystemCapabilities* caps = Root::getSingleton().getRenderSystem()->getCapabilities();
        if (!caps->hasCapability(RSC_GEOMETRY_PROGRAM))
        {
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your card does not support geometry programs, so cannot "
                "run this demo. Sorry!", 
                "IsoSurfApplication::createScene");
        }
		
		int maxOutputVertices = caps->getGeometryProgramNumOutputVertices();
		Ogre::LogManager::getSingleton().getDefaultLog()->stream() << 
			"Num output vertices per geometry shader run : " << maxOutputVertices;

		
		mCamera->setPosition(0, 0, -40);
        mCamera->lookAt(0,0,0);
		mCamera->setNearClipDistance(0.1);
		mCamera->setFarClipDistance(100);

		MeshPtr tetrahedraMesh = ProceduralTools::generateTetrahedra();
		//Create tetrahedra and add it to the root scene node
		tetrahedra = mSceneMgr->createEntity("TetrahedraEntity", tetrahedraMesh->getName());
		//tetraHedra->setDebugDisplayEnabled(true);
		Ogre::SceneNode* parentNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		parentNode->attachObject(tetrahedra);
		parentNode->setScale(10,10,10);

		mRoot->addFrameListener(new TetraHedraShaderListener);
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
    IsoSurfApplication app;

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
    IsoSurfApplication app;
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

- (void)applicationWillTerminate:(UIApplication *)application {
    Root::getSingleton().queueEndRendering();
}

- (void)dealloc {
    [super dealloc];
}

@end
#   endif

#endif
