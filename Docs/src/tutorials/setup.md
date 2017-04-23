# Setting up an OGRE project {#setup}
@note see BuildingOgre.md for instructions how to build OGRE itself
# CMake Configuration {#cmake}
Ogre uses CMake as its build system. It is recommended that you use it in your project as well.  
Then all you need is to add the following three lines to your project
```php
# specify which version you need
find_package(OGRE 1.10 REQUIRED)
# the search paths
include_directories(${OGRE_INCLUDE_DIRS})
link_directories(${OGRE_LIBRARY_DIRS})
```
These settings include all available components and third party libraries OGRE depends on (e.g. boost) - nothing more to do.

If you installed OGRE in a non-standard path, you will have to set `OGRE_DIR` to the location of `OGREConfig.cmake` so `find_package` can figure out the rest.

For inspecting the detected OGRE installation, the following CMake variables are available
* `OGRE_STATIC` - whether ogre was build as static lib
* `OGRE_${COMPONENT}_FOUND` - ${COMPONENT} is available
* `OGRE_PLUGIN_DIR` - The directory where the OGRE plugins are located
* `OGRE_MEDIA_DIR` - The directory where the OGRE sample media is located
* `OGRE_CONFIG_DIR` - The directory where the OGRE config files are located

# Application skeleton {#skeleton}
The easiest way to get started is the OgreBites Component. It handles Ogre startup/ tear down (including Ogre::Overlay, @ref rtss "RTSS"), input using SDL2 and even includes a @ref trays "Simple GUI System".

This is useful if all you want is to get a Scene with a FPS counter up and running (rapid prototyping).
If available it also uses SDL2 for input - you now just have to implement the callbacks.

To use it, simply derive from OgreBites::ApplicationContext and if you want to get input events from OgreBites::InputListener

```cpp
class MyTestApp : public OgreBites::ApplicationContext, public OgreBites::InputListener
{
    ...
}
```
in the constructor we register ourself as a InputListener
```cpp
MyTestApp() : OgreBites::ApplicationContext("MyTestApp")
{
    addInputListener(this);
}
```
to handle input events, we then override the according method
```cpp
bool MyTestApp::keyPressed(const OgreBites::KeyboardEvent& evt)
{
	if (evt.keysym.sym == SDLK_ESCAPE)
	{
		getRoot()->queueEndRendering();
	}

	return true;
}
```
the interesting part however is the setup method
```cpp
void MyTestApp::setup(void)
{
    // do not forget to call the base first
	OgreBites::ApplicationContext::setup();

    // get a pointer to the already created root
	Ogre::Root* root = getRoot();
	Ogre::SceneManager* scnMgr = root->createSceneManager(Ogre::ST_GENERIC);

    // register our scene with the RTSS
	Ogre::RTShader::ShaderGenerator* shadergen = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
	shadergen->addSceneManager(scnMgr);
    
    // without light we would just get a black screen    
    Ogre::Light* light = scnMgr->createLight("MainLight");
    light->setPosition(0, 10, 15);
    
    // also need to tell where we are
    Ogre::SceneNode* camNode = scnMgr->getRootSceneNode()->createChildSceneNode();
    camNode->setPosition(0, 0, 15);
    camNode->lookAt(Ogre::Vector3(0, 0, -1), Ogre::Node::TS_PARENT);
    
    // the actual camera
    Ogre::Camera* cam = scnMgr->createCamera("myCam");
    cam->setNearClipDistance(5); // specific to this sample
    camNode->attachObject(camNode);
    
    // finally something to render
    Ogre::Entity* ent = scnMgr->createEntity("Sinbad.mesh");
    Ogre::SceneNode* node = scnMgr->getRootSceneNode()->createChildSceneNode();
    node->attachObject(ent);
}
```
finally we start everything as
```cpp
int main(void)
{
	MyTestApp app;
	app.initApp();
	app.getRoot()->startRendering();
	app.closeApp();
}
```
OgreBites itself is also a good starting point if you want to do more sophisticated things like embedding OGRE into a Qt application or similar.
