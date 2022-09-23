# Setting up an OGRE project {#setup}
@note see @ref building-ogre for instructions how to build OGRE itself

@tableofcontents

# CMake Configuration {#cmake}
Ogre uses CMake as its build system. It is strongly recommended that you use it in your project as well.
Then, all you need is to add the following lines to your project
@snippet Samples/Tutorials/CMakeLists.txt discover_ogre
These settings already include any third party libraries the Components depends on (e.g. SDL) - nothing more to do.
Alternatively use `${OGRE_LIBRARIES}` to link against all available OGRE components.

@note If you built OGRE statically, you also need `find_package` the used third-party libraries. e.g. `find_package(ZLIB)`

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

The main class is OgreBites::ApplicationContext which manages the lifetime of everything else.
In the constructor we set our application name. The ogre configuration files will be stored in a system dependent location specific to our app. Then @c initApp will initialise all components and create a window for us.
@snippet Samples/Tutorials/Bootstrap.cpp constructor

the interesting part however is creation of the actual scene
@note The code is explained in detail in the Tutorial @ref tut_FirstScene

@snippet Samples/Tutorials/Bootstrap.cpp setup

to handle input events, we then override the according method
@snippet Samples/Tutorials/Bootstrap.cpp key_handler

finally we start everything as
@snippet Samples/Tutorials/Bootstrap.cpp main

OgreBites itself is also a good starting point if you need more control over the Camera or the Window creation.
For instance to render into an externally created Window. Fore Qt interop, there is OgreBites::ApplicationContextQt.

@note You can find the full code of the above example at 
* `Samples/Tutorials/Bootstrap.cpp` for C++
* `Samples/Python/sample.py` for Python
* `Samples/AndroidJNI/MainActivity.java` for Java (Android)
* `Samples/Csharp/example.cs` for C\#

@see Ogre::FileSystemLayer::getConfigFilePath
@see Ogre::Root::renderOneFrame
@see Ogre::RenderSystem::_createRenderWindow
@see Ogre::RenderSystem::preExtraThreadsStarted

# Running your App {#setupRunning}

%Ogre is divided into two library groups: main libraries and plugins.

@par Main libraries
The main library group contains the @c OgreMain library itself (named @c OgreMain.dll or @c libOgreMain.so depending on your platform) and the component libraries that rely on it.
@par
On Linux you will typically install these into `/usr/local/` which is automatically searched by the linker, so nothing more to do.
On Windows however, you will have to either add the `sdk/bin` folder to `PATH` or copy your executable into `sdk/bin`.

@par Plugins
The second group of shared libraries are the plugins. %Ogre pushes a good portion of its functionality into plugins so that they may be turned on or off easily at runtime. The core plugins that are included with %Ogre have names that start with @c "Plugin_" and @c "Codec_". You can also write your own plugins.
@par
%Ogre also uses plugins for the different render systems (such as OpenGL, DirectX, etc). These plugins start with @c "RenderSystem_".

On Windows, the library and configuration files for %Ogre can be found in the @c bin folder of the SDK.
On Unix they are split into @c share/OGRE for configuration files, @c lib/ for libraries and  @c lib/OGRE for Plugins.

## Configuration Files

%Ogre uses several configuration files (\*.cfg) in the INI format. They control things like which plugins are loaded and where your application will search for resource files. We will briefly introduce you to each of these files. You'll slowly read more about them as you progress through the tutorials as well.

These files are searched in [a set of predefined locations as described here](@ref Ogre::FileSystemLayer::getConfigFilePath). Alternatively, you can set the @c OGRE_CONFIG_DIR environment variable for a custom configuration file location.

@attention The above sample code must find @c plugins.cfg and @c resources.cfg to function properly.

### plugins.cfg

This file tells %Ogre which plugins to load. You modify this file when you want to load a different set of plugins. It is often most useful to simply "comment out" lines instead of removing them, because you never know when a stroke of inspiration will mean you want to reload some unused plugins. Here is some sample content:

```py
# Plugin=RenderSystem_Direct3D9
# Plugin=RenderSystem_Direct3D11
Plugin=RenderSystem_GL
```

We have the DirectX systems commented out, and an active line for OpenGL. On a windows system, you may have this reversed. You can see why it might be helpful not to delete unused lines, because then you have to try and remember whether it was @c RenderSystem_OpenGL or @c RenderSystem_GL.

You can also decide where %Ogre looks for plugins by changing the @c PluginFolder variable. You can use both absolute and relative paths. Relative paths are resolved relative to the location of @c plugins.cfg.

For example, if you have built %Ogre from source on a linux machine, then you will need a line like this at the beginning of your file:

```
PluginFolder=/usr/local/lib/OGRE
```

By default, %Ogre would have been looking in the same directory where the @c plugins.cfg is located, which is sufficient on Windows.

Additionally, you can use the @c OGRE_PLUGIN_DIR environment variable to override the value of @c PluginFolder.

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