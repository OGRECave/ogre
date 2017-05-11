# Setting up an OGRE project {#setup}
@note see BuildingOgre.md for instructions how to build OGRE itself
# CMake Configuration {#cmake}
Ogre uses CMake as its build system. It is recommended that you use it in your project as well.  
Then all you need is to add the following three lines to your project
@snippet Samples/Bootstrap/CMakeLists.txt discover_ogre
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
@snippet Samples/Bootstrap/main.cpp constructor

to handle input events, we then override the according method
@snippet Samples/Bootstrap/main.cpp key_handler

the interesting part however is the setup method
@snippet Samples/Bootstrap/main.cpp setup

finally we start everything as
@snippet Samples/Bootstrap/main.cpp main
@note you can find the full code in `Samples/Bootstrap`.

OgreBites itself is also a good starting point if you want to do more sophisticated things like embedding OGRE into a Qt application or similar.
