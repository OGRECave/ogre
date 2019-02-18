# Setting up an OGRE project {#setup}
@note see @ref building-ogre for instructions how to build OGRE itself
# CMake Configuration {#cmake}
Ogre uses CMake as its build system. It is recommended that you use it in your project as well.  
Then all you need is to add the following lines to your project
@snippet Samples/Tutorials/CMakeLists.txt discover_ogre
These settings already include any third party libraries the Components depends on (e.g. SDL) - nothing more to do.
Alternatively use `${OGRE_LIBRARIES}` to link against all available OGRE components.

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
in the constructor we set our application name. The ogre configuration files will be stored in a system dependant location specific to our app.
@snippet Samples/Tutorials/Bootstrap.cpp constructor

to handle input events, we then override the according method
@snippet Samples/Tutorials/Bootstrap.cpp key_handler

the interesting part however is the setup method
@snippet Samples/Tutorials/Bootstrap.cpp setup
@note The above code is explained in detail in @ref tut_FirstScene.

finally we start everything as
@snippet Samples/Tutorials/Bootstrap.cpp main
@note You can find the full code of the above example at 
* `Samples/Tutorials/Bootstrap.cpp` for C++
* `Samples/Python/bites_sample.py` for Python
* `Samples/AndroidJNI/MainActivity.java` for Java (Android)

OgreBites itself is also a good starting point if you need more control over the Camera or the Window creation.
For instance to render into an existing Qt Window.

@see Ogre::FileSystemLayer::getConfigFilePath
@see Ogre::Root::renderOneFrame
@see Ogre::RenderSystem::_createRenderWindow
@see Ogre::RenderSystem::preExtraThreadsStarted

# Running your App

On Linux you will typically install OGRE into `/usr/local/` which is automatically searched by the linker, so nothing more to do.
On Windows however, you will have to either add the `sdk/bin` folder to `PATH` or copy your executable into `sdk/bin`.