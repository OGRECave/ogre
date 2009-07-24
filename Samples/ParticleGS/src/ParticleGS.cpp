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
Filename:    ParticleGS.cpp
Description: Demonstrates the use of the geometry shader and render to vertex
	buffer to create a particle system that is entirely calculated on the GPU.
	Partial implementation of ParticlesGS example from Microsoft's DirectX 10
	SDK : http://msdn.microsoft.com/en-us/library/bb205329(VS.85).aspx
-----------------------------------------------------------------------------
*/

#include "ExampleApplication.h"
#include "ProceduralManualObject.h"
#include "OgreRenderToVertexBuffer.h"
#include "RandomTools.h"

//#define LOG_GENERATED_BUFFER
Vector3 GRAVITY_VECTOR = Vector3(0, -9.8, 0);
Real demoTime = 0;
ProceduralManualObject* particleSystem;

#ifdef LOG_GENERATED_BUFFER
struct FireworkParticle 
{
	float pos[3];
	float timer;
	float type;
	float vel[3];
};
#endif

class ParticleGSListener : public FrameListener
{
	bool frameStarted(const FrameEvent& evt) 
	{ 
		//Set shader parameters
		GpuProgramParametersSharedPtr geomParams = particleSystem->
			getRenderToVertexBuffer()->getRenderToBufferMaterial()->
			getTechnique(0)->getPass(0)->getGeometryProgramParameters();
		geomParams->setNamedConstant("elapsedTime", evt.timeSinceLastFrame);
		demoTime += evt.timeSinceLastFrame;
		geomParams->setNamedConstant("globalTime", demoTime);
		geomParams->setNamedConstant("frameGravity", GRAVITY_VECTOR * evt.timeSinceLastFrame);
		
		return true; 
	}

	bool frameEnded(const FrameEvent& evt) 
	{ 
#ifdef LOG_GENERATED_BUFFER
		//This will only work if the vertex buffer usage is dynamic (see R2VB implementation)
		LogManager::getSingleton().getDefaultLog()->stream() << 
			"Particle system for frame " <<	Root::getSingleton().getNextFrameNumber();
		RenderOperation renderOp;
		particleSystem->getRenderToVertexBuffer()->getRenderOperation(renderOp);
		const HardwareVertexBufferSharedPtr& vertexBuffer = 
			renderOp.vertexData->vertexBufferBinding->getBuffer(0);
		
		assert(vertexBuffer->getVertexSize() == sizeof(FireworkParticle));
		FireworkParticle* particles = static_cast<FireworkParticle*>
			(vertexBuffer->lock(HardwareBuffer::HBL_READ_ONLY));
		
		for (size_t i=0; i<renderOp.vertexData->vertexCount; i++)
		{
			FireworkParticle& p = particles[i];
			LogManager::getSingleton().getDefaultLog()->stream() <<
				"FireworkParticle " << i+1 << " : " <<
				"Position : " << p.pos[0] << " " << p.pos[1] << " " << p.pos[2] << " , " <<
				"Timer : " << p.timer << " , " <<
				"Type : " << p.type << " , " <<
				"Velocity : " << p.vel[0] << " " << p.vel[1] << " " << p.vel[2];
		}
		
		vertexBuffer->unlock();
#endif
		return true; 
	}
};

class ParticleGSApplication : public ExampleApplication
{
public:
    ParticleGSApplication() { 
    }

    ~ParticleGSApplication() {  }
protected:

	virtual void chooseSceneManager(void)
    {
        // Create the SceneManager, in this case a generic one
        mSceneMgr = mRoot->createSceneManager("DefaultSceneManager", "ExampleSMInstance");
    }

	ProceduralManualObject* createProceduralParticleSystem()
	{
		particleSystem = static_cast<ProceduralManualObject*>
			(mSceneMgr->createMovableObject("ParticleGSEntity", ProceduralManualObjectFactory::FACTORY_TYPE_NAME));
		particleSystem->setMaterial("Ogre/ParticleGS/Display");

		//Generate the geometry that will seed the particle system
		ManualObject* particleSystemSeed = mSceneMgr->createManualObject("ParticleSeed");
		//This needs to be the initial launcher particle
		particleSystemSeed->begin("Ogre/ParticleGS/Display", RenderOperation::OT_POINT_LIST);
		particleSystemSeed->position(0,0,0); //Position
		particleSystemSeed->textureCoord(1); //Timer
		particleSystemSeed->textureCoord(0); //Type
		particleSystemSeed->textureCoord(0,0,0); //Velocity
		particleSystemSeed->end();

		//Generate the RenderToBufferObject
		RenderToVertexBufferSharedPtr r2vbObject = 
			HardwareBufferManager::getSingleton().createRenderToVertexBuffer();
		r2vbObject->setRenderToBufferMaterialName("Ogre/ParticleGS/Generate");
		
		//Apply the random texture
		TexturePtr randomTexture = RandomTools::generateRandomVelocityTexture();
		r2vbObject->getRenderToBufferMaterial()->getTechnique(0)->getPass(0)->
			getTextureUnitState("RandomTexture")->setTextureName(
			randomTexture->getName(), randomTexture->getTextureType());

		r2vbObject->setOperationType(RenderOperation::OT_POINT_LIST);
		r2vbObject->setMaxVertexCount(16000);
		r2vbObject->setResetsEveryUpdate(false);
		VertexDeclaration* vertexDecl = r2vbObject->getVertexDeclaration();
		size_t offset = 0;
		offset += vertexDecl->addElement(0, offset, VET_FLOAT3, VES_POSITION).getSize(); //Position
		offset += vertexDecl->addElement(0, offset, VET_FLOAT1, VES_TEXTURE_COORDINATES, 0).getSize(); //Timer
		offset += vertexDecl->addElement(0, offset, VET_FLOAT1, VES_TEXTURE_COORDINATES, 1).getSize(); //Type
		offset += vertexDecl->addElement(0, offset, VET_FLOAT3, VES_TEXTURE_COORDINATES, 2).getSize(); //Velocity
		
		//Bind the two together
		particleSystem->setRenderToVertexBuffer(r2vbObject);
		particleSystem->setManualObject(particleSystemSeed);

		//Set bounds
		AxisAlignedBox aabb;
		aabb.setMinimum(-100,-100,-100);
		aabb.setMaximum(100,100,100);
		particleSystem->setBoundingBox(aabb);
		
		return particleSystem;
	}

    // Just override the mandatory create scene method
    void createScene(void)
    {
        // Check capabilities
		const RenderSystemCapabilities* caps = Root::getSingleton().getRenderSystem()->getCapabilities();
        if (!caps->hasCapability(RSC_GEOMETRY_PROGRAM))
        {
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your card does not support geometry programs, so cannot "
                "run this demo. Sorry!", 
                "ParticleGSApplication::createScene");
        }
		if (!caps->hasCapability(RSC_HWRENDER_TO_VERTEX_BUFFER))
        {
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your card does not support render to vertex buffer, "
				"so cannot run this demo. Sorry!", 
                "ParticleGSApplication::createScene");
        }

		Root::getSingleton().addMovableObjectFactory(new ProceduralManualObjectFactory);
		mRoot->addFrameListener(new ParticleGSListener);
		ProceduralManualObject* particleSystem = createProceduralParticleSystem();

		mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(particleSystem);
		//mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(particleSystem->getManualObject());
		mCamera->setPosition(0,35,-100);
		mCamera->lookAt(0,35,0);
		
		//Add an ogre head to the scene
		SceneNode* ogreHeadSN = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		Entity *ogreHead = mSceneMgr->createEntity("head", "ogrehead.mesh");        
		ogreHeadSN->scale(0.1,0.1,0.1);
		ogreHeadSN->yaw(Degree(180));
		ogreHeadSN->attachObject(ogreHead);
		
		//Add a plane to the scene
		Plane plane;
		plane.normal = Vector3::UNIT_Y;
		plane.d = 100;
		MeshManager::getSingleton().createPlane("Myplane",
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane,
			1500,1500,20,20,true,1,60,60,Vector3::UNIT_Z);
		Entity* pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
		pPlaneEnt->setMaterialName("Examples/Rockwall");
		pPlaneEnt->setCastShadows(false);
		mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(0,95,0))->attachObject(pPlaneEnt);
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
    ParticleGSApplication app;

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
    ParticleGSApplication app;
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
