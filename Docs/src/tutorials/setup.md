# Setting up an OGRE project {#setup}
@note see @ref building-ogre for instructions how to build OGRE itself

@tableofcontents

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
* `Samples/Python/sample.py` for Python
* `Samples/AndroidJNI/MainActivity.java` for Java (Android)
* `Samples/Csharp/example.cs` for C\#

OgreBites itself is also a good starting point if you need more control over the Camera or the Window creation.
For instance to render into an existing Qt Window.

@see Ogre::FileSystemLayer::getConfigFilePath
@see Ogre::Root::renderOneFrame
@see Ogre::RenderSystem::_createRenderWindow
@see Ogre::RenderSystem::preExtraThreadsStarted

# Running your App {#setupRunning}

On Linux you will typically install OGRE into `/usr/local/` which is automatically searched by the linker, so nothing more to do.
On Windows however, you will have to either add the `sdk/bin` folder to `PATH` or copy your executable into `sdk/bin`.

## Configuration Files

Ogre uses several configuration files (\*.cfg). They control things like which plugins are loaded and where your application will search for resource files. We will briefly introduce you to each of these files. You'll slowly read more about them as you progress through the tutorials as well.

You can place these files the same directory as your executable or in any of [the default lookup paths described here](@ref Ogre::FileSystemLayer::getConfigFilePath). Alternatively you can set the @c OGRE_CONFIG_DIR environment variable for the configuration file location. This overrides the step "executable path" in the default lookup order.

@attention %Ogre must find @c plugins.cfg and @c resources.cfg to function properly. Later tutorials will cover more of their use.

### plugins.cfg

This file tells %Ogre which plugins to load. You modify this file when you want to load a different set of plugins. It is often most useful to simply "comment out" lines instead of removing them, because you never know when a stroke of inspiration will mean you want to reload some unused plugins. Here is some sample content:

```py
# Plugin=RenderSystem_Direct3D9
# Plugin=RenderSystem_Direct3D10
# Plugin=RenderSystem_Direct3D11
Plugin=RenderSystem_GL
```

We have the three DirectX systems commented out, and an active line for OpenGL. On a windows system, you may have this reversed. You can see why it might be helpful not to delete unused lines, because then you have to try and remember whether it was RenderSystem_OpenGL or RenderSystem_GL.

You can also decide where %Ogre looks for plugins by changing the @c PluginFolder variable. You can use both absolute and relative paths. Relative paths are resolved relative to the location of @c plugins.cfg. Additionally, you can use the @c OGRE_PLUGIN_DIR environment variable to override the value of @c PluginFolder.

For example, if you have built %Ogre from source on a linux machine, then you will need a line like this at the beginning of your file:

```
PluginFolder=/usr/local/lib/OGRE
```

By default, %Ogre would have been looking in the same directory where the @c plugins.cfg is located, which is sufficient on Windows.

@note %Ogre is aware whether your app is a bundle. Therefore a relative path like `Contents/Frameworks/` will be correctly resolved inside the app bundle on OSX.

### resources.cfg

This file contains a list of the directories %Ogre will use to search for resources. Resources include scripts, meshes, textures, GUI layouts, and others. You can also use both absolute and relative paths in this file, but you still cannot use environment variables. Relative paths are resolved relative to the location of @c resources.cfg. %Ogre will **not** search subdirectories, so you have to manually enter them. Here is an example:

```
[General]
FileSystem=../media
FileSystem=../media/materials/scripts
FileSystem=../media/materials/textures
FileSystem=../media/models
```

Here is an example of a relative path being used and the need to list subdirectories. Including the '../media' directory did not automatically include the '../media/models' directory. This is so that Ogre doesn't get greedy and waste time loading up unneeded resources.

### ogre.cfg

This file is generated by the Render Settings dialog that appears when you run your application. **Do not** distribute this file with your application. This file will be specific to your own setup. This file will contain your choices for things like screen resolution (see Ogre::RenderSystem::setConfigOption). Do not modify this file directly. Change the settings with the dialog and it will be automatically updated.

Depending on the OS, %Ogre will place this config file in [the writable path described here](@ref Ogre::FileSystemLayer::getWritablePath).