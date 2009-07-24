/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
LGPL like the rest of the engine.
-----------------------------------------------------------------------------
*/

/**
\file 
Instancing.cpp
\brief
Shows OGRE's bezier instancing feature
*/

#include "Instancing.h"

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

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
		InstancingApplication app;

		try {
			app.go();
		} catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
			MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL );
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

InstancingListener::InstancingListener(RenderWindow* win, Camera* cam,CEGUI::Renderer* renderer, InstancingApplication*main)
: ExampleFrameListener(win, cam,false,true),
mMain(main),
mRequestShutDown(false),
mLMBDown(false),
mRMBDown(false),
mAvgFrameTime(0.1),
mBurnAmount(0)
{ 
	const GpuProgramManager::SyntaxCodes &syntaxCodes = GpuProgramManager::getSingleton().getSupportedSyntax();
	for (GpuProgramManager::SyntaxCodes::const_iterator iter = syntaxCodes.begin();iter != syntaxCodes.end();++iter)
	{
		LogManager::getSingleton().logMessage("supported syntax : "+(*iter));
	}
	mGUIRenderer=renderer;
	numMesh = 160;
	numRender = 0;
	meshSelected = 0;
	currentGeomOpt = INSTANCE_OPT;
	createCurrentGeomOpt();

	mMouse->setEventCallback(this);
	mKeyboard->setEventCallback(this);

	mGuiAvg   = CEGUI::WindowManager::getSingleton().getWindow("OPAverageFPS");
	mGuiCurr  = CEGUI::WindowManager::getSingleton().getWindow("OPCurrentFPS");
	mGuiBest  = CEGUI::WindowManager::getSingleton().getWindow("OPBestFPS");
	mGuiWorst = CEGUI::WindowManager::getSingleton().getWindow("OPWorstFPS");
	mGuiTris  = CEGUI::WindowManager::getSingleton().getWindow("OPTriCount");
	mGuiDbg   = CEGUI::WindowManager::getSingleton().getWindow("OPDebugMsg");
	mRoot	  = CEGUI::WindowManager::getSingleton().getWindow("root");

	mDebugOverlay->hide();

	timer = new Ogre::Timer();
	mLastTime = timer->getMicroseconds()/1000000.0f;

}
//-----------------------------------------------------------------------
InstancingListener::~InstancingListener()
{
	destroyCurrentGeomOpt();
	delete timer;
}
//-----------------------------------------------------------------------
bool InstancingListener::frameRenderingQueued(const FrameEvent& evt)
{
	burnCPU();
	updateStats();

	if(mRequestShutDown)
		return false;
	const bool returnValue = ExampleFrameListener::frameRenderingQueued(evt);
	// Call default
	return returnValue;
}
//-----------------------------------------------------------------------
void InstancingListener::burnCPU(void)
{
	double mStartTime = timer->getMicroseconds()/1000000.0f; //convert into seconds
	double mCurTime =  mStartTime;
	double mStopTime = mLastTime + mBurnAmount;
	double mCPUUsage;

	while( mCurTime < mStopTime )
	{
		mCurTime = timer->getMicroseconds()/1000000.0f; //convert into seconds
	}

	if( mCurTime - mLastTime > 0.00001f )
		mCPUUsage = (mCurTime - mStartTime) / (mCurTime - mLastTime) * 100.0f;
	else
		mCPUUsage = FLT_MAX;

	mLastTime = timer->getMicroseconds()/1000000.0f; //convert into seconds
	int time = mCPUUsage+0.5f;
	if(mTimeUntilNextToggle<=0)
	{
		mDebugText="remaining for logic:"+ StringConverter::toString(time);
		mTimeUntilNextToggle=1;
	}

}
//-----------------------------------------------------------------------
void InstancingListener::destroyCurrentGeomOpt()
{
	switch(currentGeomOpt)
	{
	case INSTANCE_OPT:destroyInstanceGeom();break;
	case STATIC_OPT:destroyStaticGeom ();break;
	case ENTITY_OPT: destroyEntityGeom ();break;
	}

	assert (numRender == posMatrices.size ());
	for (size_t i = 0; i < numRender; i++)
	{
		delete [] posMatrices[i];
	}
	posMatrices.clear();
}
//-----------------------------------------------------------------------
void InstancingListener::createCurrentGeomOpt()
{
	LogManager::getSingleton().logMessage("geom deleted");
	objectCount=numMesh;
	numRender=1;
	while(objectCount>maxObjectsPerBatch)
	{
		numRender++;
		objectCount-=maxObjectsPerBatch;
	}

	assert (meshSelected < numTypeMeshes);
	MeshPtr m = MeshManager::getSingleton ().getByName (meshes[meshSelected] + ".mesh");
	if (m.isNull ())
	{
		m = MeshManager::getSingleton ().load (meshes[meshSelected] + ".mesh", 
			ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
	}
	const Real radius = m->getBoundingSphereRadius ();

	// could/should print on screen mesh name, 
	//optimisation type, 
	//mesh vertices num, 
	//32 bit or not, 
	//etC..



	posMatrices.resize (numRender);
	posMatrices.reserve (numRender);


	vector <Vector3 *>::type posMatCurr;
	posMatCurr.resize (numRender);
	posMatCurr.reserve (numRender);
	for (size_t i = 0; i < numRender; i++)
	{
		posMatrices[i] = new Vector3[numMesh];
		posMatCurr[i] = posMatrices[i];
	}

	size_t i = 0, j = 0;
	for (size_t p = 0; p < numMesh; p++)
	{
		for (size_t k = 0; k < numRender; k++)
		{
			posMatCurr[k]->x = radius*i;
			posMatCurr[k]->y = k*radius;

			posMatCurr[k]->z = radius*j;
			posMatCurr[k]++;
		}
		if (++j== 10) 
		{
			j = 0;
			i++;
		}

	}
	posMatCurr.clear ();


	switch(currentGeomOpt)
	{
	case INSTANCE_OPT:createInstanceGeom();break;
	case STATIC_OPT:createStaticGeom ();break;
	case ENTITY_OPT: createEntityGeom ();break;
	}
}
//-----------------------------------------------------------------------
void InstancingListener::createInstanceGeom()
{
	if (Root::getSingleton ().getRenderSystem ()->getCapabilities ()->hasCapability (RSC_VERTEX_PROGRAM) == false)
	{
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Your video card doesn't support batching", "Demo_Instance::createScene");
	}

	Entity *ent = mCamera->getSceneManager()->createEntity(meshes[meshSelected], meshes[meshSelected] + ".mesh");	


	renderInstance.reserve(numRender);
	renderInstance.resize(numRender);

	//Load a mesh to read data from.	
	InstancedGeometry* batch = new InstancedGeometry(mCamera->getSceneManager(), 
		meshes[meshSelected] + "s" );
	batch->setCastShadows(true);

	batch->setBatchInstanceDimensions (Vector3(1000000, 1000000, 1000000));
	const size_t batchSize = (numMesh > maxObjectsPerBatch) ? maxObjectsPerBatch :numMesh;
	setupInstancedMaterialToEntity(ent);
	for(size_t i = 0; i < batchSize ; i++)
	{
		batch->addEntity(ent, Vector3::ZERO);
	}
	batch->setOrigin(Vector3::ZERO);

	batch->build();


	for (size_t k = 0; k < numRender-1; k++)
	{
		batch->addBatchInstance();
	}
	InstancedGeometry::BatchInstanceIterator regIt = batch->getBatchInstanceIterator();
	size_t k = 0;
	while (regIt.hasMoreElements ())
	{

		InstancedGeometry::BatchInstance *r = regIt.getNext();

		InstancedGeometry::BatchInstance::InstancedObjectIterator bit = r->getObjectIterator();
		int j = 0;
		while(bit.hasMoreElements())
		{
			InstancedGeometry::InstancedObject* obj = bit.getNext();

			const Vector3 position (posMatrices[k][j]);								
			obj->setPosition(position);
			++j;

		}
		k++;
		
	}
	batch->setVisible(true);
	renderInstance[0] = batch;

	mCamera->getSceneManager()->destroyEntity (ent);
}
void InstancingListener::setupInstancedMaterialToEntity(Entity*ent)
{
	for (Ogre::uint i = 0; i < ent->getNumSubEntities(); ++i)
	{
		SubEntity* se = ent->getSubEntity(i);
		String materialName= se->getMaterialName();
		se->setMaterialName(buildInstancedMaterial(materialName));
	}
}
String InstancingListener::buildInstancedMaterial(const String &originalMaterialName)
{

	// already instanced ?
	if (StringUtil::endsWith (originalMaterialName, "/instanced"))
		return originalMaterialName;

	MaterialPtr originalMaterial = MaterialManager::getSingleton ().getByName (originalMaterialName);

	// if originalMat doesn't exists use "Instancing" material name
	const String instancedMaterialName (originalMaterial.isNull() ? "Instancing" : originalMaterialName + "/Instanced");
	MaterialPtr  instancedMaterial = MaterialManager::getSingleton ().getByName (instancedMaterialName);

	// already exists ?
	if (instancedMaterial.isNull())
	{
		instancedMaterial = originalMaterial->clone(instancedMaterialName);
		instancedMaterial->load();
		Technique::PassIterator pIt = instancedMaterial->getBestTechnique ()->getPassIterator();
		while (pIt.hasMoreElements())
		{

			Pass * const p = pIt.getNext();
			p->setVertexProgram("Instancing", false);
			p->setShadowCasterVertexProgram("InstancingShadowCaster");


		}
	}
	instancedMaterial->load();
	return instancedMaterialName;


}
//-----------------------------------------------------------------------
void InstancingListener::destroyInstanceGeom()
{
	delete renderInstance[0];
	renderInstance.clear();
}
//-----------------------------------------------------------------------
void InstancingListener::createStaticGeom()
{
	Entity *ent = mCamera->getSceneManager()->createEntity(meshes[meshSelected], meshes[meshSelected] + ".mesh");	

	renderStatic.reserve (numRender);
	renderStatic.resize (numRender);

	StaticGeometry* geom = new StaticGeometry (mCamera->getSceneManager(), 
		meshes[meshSelected] + "s");

	geom->setRegionDimensions (Vector3(1000000, 1000000, 1000000));
	size_t k = 0;
	size_t y = 0;
	for (size_t i = 0; i < numMesh; i++)
	{
		if (y==maxObjectsPerBatch)
		{
			y=0;
			k++;
		}
		geom->addEntity (ent, posMatrices[k][y]);
		y++;
	}
	geom->setCastShadows(true);
	geom->build ();
	renderStatic[0] = geom;
	mCamera->getSceneManager ()->destroyEntity (ent);
}
//-----------------------------------------------------------------------
void InstancingListener::destroyStaticGeom()
{

	delete renderStatic[0];

	renderStatic.clear();
}
//-----------------------------------------------------------------------
void InstancingListener::createEntityGeom()
{
	size_t k = 0;
	size_t y = 0;
	renderEntity.reserve (numMesh);
	renderEntity.resize (numMesh);
	nodes.reserve (numMesh);
	nodes.resize (numMesh);

	for (size_t i = 0; i < numMesh; i++)
	{
		if (y==maxObjectsPerBatch)
		{
			y=0;
			k++;
		}
		LogManager::getSingleton().logMessage("marche3");
		nodes[i]=mCamera->getSceneManager()->getRootSceneNode()->createChildSceneNode("node"+StringConverter::toString(i));
		LogManager::getSingleton().logMessage(":"+nodes[i]->getName());
		renderEntity[i]=mCamera->getSceneManager()->createEntity(meshes[meshSelected]+StringConverter::toString(i), meshes[meshSelected] + ".mesh");	
		nodes[i]->attachObject(renderEntity[i]);
		nodes[i]->setPosition(posMatrices[k][y]);

		y++;
	}

}
//-----------------------------------------------------------------------
void InstancingListener::destroyEntityGeom()
{
	size_t i;
	size_t j=0;
	for (i=0;i<numMesh;i++)
	{
		LogManager::getSingleton().logMessage(" " +nodes[i]->getName());
		LogManager::getSingleton().logMessage(StringConverter::toString(j)+":"+StringConverter::toString(j<numMesh));
		String name=nodes[i]->getName();
		mCamera->getSceneManager()->destroySceneNode(name);
		mCamera->getSceneManager()->destroyEntity(renderEntity[i]);
		j++;
	}
}
//-----------------------------------------------------------------------
bool InstancingListener::mouseMoved ( const OIS::MouseEvent &arg )
{

	CEGUI::System::getSingleton().injectMouseMove( arg.state.X.rel, arg.state.Y.rel );
	return true;
}	
//-----------------------------------------------------------------------
bool InstancingListener::mousePressed ( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
	CEGUI::System::getSingleton().injectMouseButtonDown(convertOISMouseButtonToCegui(id));
	return true;
}
//-----------------------------------------------------------------------
bool InstancingListener::mouseReleased (const OIS::MouseEvent &arg, OIS::MouseButtonID id)
{
	CEGUI::System::getSingleton().injectMouseButtonUp(convertOISMouseButtonToCegui(id));
	return true;
}
/*
void InstancingListener::mouseClicked(OIS::MouseEvent* e) {}
void InstancingListener::mouseEntered(OIS::MouseEvent* e) {}
void InstancingListener::mouseExited(OIS::MouseEvent* e) {}*/
void InstancingListener::requestShutdown(void)
{
	mRequestShutDown=true;
}
void InstancingListener::setCurrentGeometryOpt(CurrentGeomOpt opt)
{
	currentGeomOpt=opt;
}
bool InstancingListener::handleMouseMove(const CEGUI::EventArgs& e)
{
	using namespace CEGUI;

	if( mLMBDown)
	{
		// rotate camera
		mRotX += Ogre::Degree(-((const MouseEventArgs&)e).moveDelta.d_x * mAvgFrameTime * 10.0);
		mRotY += Ogre::Degree(-((const MouseEventArgs&)e).moveDelta.d_y * mAvgFrameTime * 10.0);
		mCamera->yaw(mRotX);
		mCamera->pitch(mRotY);
		MouseCursor::getSingleton().setPosition( mLastMousePosition );
	}


	return true;
}
//--------------------------------------------------------------------------
bool InstancingListener::handleMouseButtonUp(const CEGUI::EventArgs& e)
{
	using namespace CEGUI;


	if( ((const MouseEventArgs&)e).button == LeftButton )
	{
		mLMBDown = false;
		MouseCursor::getSingleton().setPosition( mLastMousePosition );
		CEGUI::MouseCursor::getSingleton().show();
	}


	return true;
}

//--------------------------------------------------------------------------
bool InstancingListener::handleMouseButtonDown(const CEGUI::EventArgs& e)
{
	using namespace CEGUI;


	if( ((const MouseEventArgs&)e).button == LeftButton )
	{
		mLMBDown = true;
		mLastMousePosition=CEGUI::MouseCursor::getSingleton().getPosition();
		CEGUI::MouseCursor::getSingleton().hide();
	}

	return true;
}
//--------------------------------------------------------------------------
void InstancingListener::updateStats(void)
{
	static CEGUI::String currFps = "Current FPS: ";
	static CEGUI::String avgFps = "Average FPS: ";
	static CEGUI::String bestFps = "Best FPS: ";
	static CEGUI::String worstFps = "Worst FPS: ";
	static CEGUI::String tris = "Triangle Count: ";


	const Ogre::RenderTarget::FrameStats& stats = mMain->getRenderWindow()->getStatistics();

	mGuiAvg->setText(avgFps + Ogre::StringConverter::toString(stats.avgFPS));
	mGuiCurr->setText(currFps + Ogre::StringConverter::toString(stats.lastFPS));
	mGuiBest->setText(bestFps + Ogre::StringConverter::toString(stats.bestFPS)
		+ " " + Ogre::StringConverter::toString(stats.bestFrameTime)+" ms");
	mGuiWorst->setText(worstFps + Ogre::StringConverter::toString(stats.worstFPS)
		+ " " + Ogre::StringConverter::toString(stats.worstFrameTime)+" ms");

	mGuiTris->setText(tris + Ogre::StringConverter::toString(stats.triangleCount));
	mGuiDbg->setText(mDebugText.c_str());
	mAvgFrameTime = 1.0f/(stats.avgFPS + 1.0f);
	if (mAvgFrameTime > 0.1f) mAvgFrameTime = 0.1f;

}

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
	InstancingApplication app;
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
