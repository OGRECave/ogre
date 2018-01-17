#import "OgreController.h"
#import "Ogre.h"

using namespace Ogre;

// Evil Globals ;)
Ogre::SceneManager *mSceneMgr;


@implementation OgreController

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
	std::string mResourcePath = [[[NSBundle mainBundle] resourcePath] cString];
	
	// Create a new root object with the correct paths
	Root *mRoot = new Root(mResourcePath + "/plugins.cfg", mResourcePath + "/ogre.cfg", mResourcePath + "/Ogre.log");
	mRoot->setRenderSystem(mRoot->getAvailableRenderers()->front());

	// use pbuffers not frame buffers because of driver problems
	mRoot->getRenderSystem()->setConfigOption("RTT Preferred Mode", "Copy");

	// Initialise, we do not want an auto created window, as that will create a carbon window
	mRoot->initialise(false);
	
	// Build the param list for a embedded cocoa window...
	NameValuePairList misc;
	misc["externalWindowHandle"] = StringConverter::toString((size_t)ogreView);
	
	// Create the window and load the params
	mRoot->createRenderWindow("OgreCustomNib Demo", 0, 0, false, &misc);
	RenderWindow *mWindow = [ogreView ogreWindow];
	
	mSceneMgr = mRoot->createSceneManager(ST_GENERIC, "MySceneManager");
	
	// Add resource locations -- looking at folders recursively
	 ResourceGroupManager::getSingleton().addResourceLocation(mResourcePath, std::string("FileSystem"), ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, false);
	 ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

	// Create the camera, node & attach camera
	Camera *mCamera = mSceneMgr->createCamera("PlayerCam");
	SceneNode* camNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	camNode->attachObject(mCamera);
	mWindow->addViewport(mCamera);
	
	// Create a light
	mSceneMgr->setAmbientLight(ColourValue(0, 0, 0));
	Light *mainLight = mSceneMgr->createLight("MainLight");

	// Add a object, give it it's own node
	SceneNode *objectNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("OgreNode");
	Entity *ogre = mSceneMgr->createEntity("Ogre", "ogrehead.mesh");
	objectNode->attachObject(ogre);
	objectNode->setPosition(Vector3(0, 0, -150));
	
	// create a timer that causes OGRE to render at 50fps
	NSTimer *renderTimer = [NSTimer scheduledTimerWithTimeInterval:0.02 target:self selector:@selector(renderFrame) userInfo:NULL repeats:YES];
	[[NSRunLoop currentRunLoop] addTimer:renderTimer forMode:NSEventTrackingRunLoopMode];
}

- (void)awakeFromNib
{
	[self willChangeValueForKey:@"diffuseLight"];
	[self willChangeValueForKey:@"specularLight"];
	diffuseLight = [[NSColor whiteColor] retain];
	specularLight = [[NSColor whiteColor] retain];
	[self didChangeValueForKey:@"diffuseLight"];
	[self didChangeValueForKey:@"specularLight"];
}

- (void)setDiffuseLight:(NSColor*)c
{
	[diffuseLight autorelease];
	diffuseLight = [c retain];
	
	NSColor *rgbcolor = [diffuseLight colorUsingColorSpaceName:NSDeviceRGBColorSpace];
	mSceneMgr->getLight("MainLight")->setDiffuseColour([rgbcolor redComponent], [rgbcolor greenComponent], [rgbcolor blueComponent]);
}

- (void)setSpecularLight:(NSColor*)c
{
	[specularLight autorelease];
	specularLight = [c retain];
	
	NSColor *rgbcolor = [specularLight colorUsingColorSpaceName:NSDeviceRGBColorSpace];
	mSceneMgr->getLight("MainLight")->setSpecularColour([rgbcolor redComponent], [rgbcolor greenComponent], [rgbcolor blueComponent]);
}

- (void)renderFrame
{
	Ogre::Root::getSingleton().renderOneFrame();
	mSceneMgr->getSceneNode("OgreNode")->rotate(Vector3(0 ,1 ,0 ), Radian(0.01));
}

@end
