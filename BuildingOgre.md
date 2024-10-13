# Guide to building OGRE {#building-ogre}

@tableofcontents

Ogre uses [CMake](https://cmake.org/) as its build system on all supported platforms.
This guide will explain to you how to use CMake to build Ogre from source. You need a CMake version >= 3.10.

What is CMake?
-------------------

CMake is a cross-platform build system - or perhaps more accurately a
build configurator. It is a program which, from a set of CMake scripts,
creates a native build system for your platform that allows you to
build Ogre.
The build process is configurable via CMake. Ogre provides several
options which you can use to customise your build.

Preparing the build environment
------------------------------------

You should now create a build directory for Ogre somewhere outside
Ogre's sources. This is the directory where CMake will create the
build system for your chosen platform and compiler, and this is also
where the Ogre libraries will be compiled. This way, the Ogre source
dir stays clean, and you can have multiple build directories (e.g. for Android and for Linux) all
working from the same Ogre source.

Getting dependencies
--------------------

By default ogre will build the recommended dependencies automatically when you run cmake configure the first time.
Ogre will install the dependencies into the subfolder `Dependencies` in the build dir by default. You can configure it by setting `OGRE_DEPENDENCIES_DIR` in cmake.

@note As the dependencies are built during the *configure* stage of CMake, you must specify the desired `CMAKE_BUILD_TYPE` via command-line. Changing the value in the CMake GUI will have no effect.

If you would rather use system wide libraries set `OGRE_BUILD_DEPENDENCIES=OFF`.
On windows, you would then point `OGRE_DEPENDENCIES_DIR` to a common dependencies folder for all of your middleware projects. Inside this directory you must have the subdirectories bin, lib and include
where you place .dll, .lib and header files of the dependencies, respectively.

For manually building the dependencies, please refer to the list below and get a source package from the website, then build it according to its documentation.

### Linux

On linux you additionally need the following system headers to build the GL, GL3+, GLES2 & Vulkan RenderSystems:

@par Ubuntu

```py
sudo apt-get install libgles2-mesa-dev libvulkan-dev glslang-dev
# with OGRE_USE_WAYLAND=OFF
sudo apt-get install libxrandr-dev
# with OGRE_USE_WAYLAND=ON
sudo apt-get install libwayland-dev libwayland-egl1 libegl-dev

# Optional dependencies
sudo apt-get install libsdl2-dev doxygen
```

@par Fedora

```py
sudo dnf install mesa-libGL-devel mesa-libEGL-devel mesa-vulkan-devel glslang-devel
# with OGRE_USE_WAYLAND=OFF
sudo dnf install libXrandr-devel
# with OGRE_USE_WAYLAND=ON
sudo dnf install pkgconfig wayland-devel egl-wayland

# Optional dependencies
sudo dnf install SDL2-devel doxygen
```

The optional dependencies will enable input handling and allow building the documentation.

### Recommended dependencies

* pugixml: https://github.com/zeux/pugixml
* SDL: https://www.libsdl.org/
* zlib: http://www.zlib.net
* freetype: http://www.freetype.org
* googletest: https://github.com/google/googletest

### Optional dependencies

* DirectX SDK: http://msdn.microsoft.com/en-us/directx/
* Vulkan SDK: https://vulkan.lunarg.com/
* Doxygen: http://doxygen.org
* Remotery: https://github.com/Celtoys/Remotery
* SWIG: http://www.swig.org/
* %Assimp: https://www.assimp.org/
* Wayland: https://wayland.freedesktop.org/
* Rust: https://www.rust-lang.org/
* Bullet: https://pybullet.org/wordpress/

### Deprecated dependencies

* FreeImage: http://freeimage.sourceforge.net
* Cg: http://developer.nvidia.com/object/cg_toolkit.html
* OpenEXR: http://www.openexr.com

Running CMake {#running-cmake}
-------------

Now start the program cmake-gui by either typing the name in a console
or selecting it from the start menu. In the field *Where is the source
code* enter the path to the Ogre source directory (the directory which
contains this file). In the field *Where to build the binaries* enter
the path to the build directory you created.
Hit *Configure*. A dialogue will appear asking you to select a generator.

Check the [CMake documentation](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html) for details on which one is appropriate for your platform and compiler.
@note on OSX, you must to use the Xcode generator to get a proper SampleBrowser .app bundle.

Click *Finish*. CMake will now gather some information about your
build environment and try to locate the dependencies. It will then show
a list of build options. You can adjust the settings to your liking;
- unchecking any of the `OGRE_BUILD_XXX` options will disable that
particular component/ plugin from being built
- `OGRE_CONFIG_XXX` on the other hand allows you to configure Core features e.g. threading or zip file support.
- `OGRE_CONFIG_NODE_INHERIT_TRANSFORM` enables shearing and non-uniform scaling for Ogre::SceneNode. This requires slightly more storage and computation time.
- `OGRE_CONFIG_ENABLE_MESHLOD` If enabled, LOD levels from *.mesh files are used to reduce triangle count. If disabled, they are skipped at loading. Reducing submesh size and entity size.
- `OGRE_PROFILING` add profiling instrumentation the ogre library.
- `OGRE_PROFILING_REMOTERY_PATH` if set, Remotery is used for profiling instead of the Ogre internal profiler.
- `OGRE_ASSERT_MODE` allows you to to disable all runtime assertion exceptions or turn them into calls to `std::abort`.
- `OGRE_RESOURCEMANGER_STRICT` allows you to turn on resource lookup related quirks for pre ogre 1.10 compatibility.
- `OGRE_NODELESS_POSITIONING` allows to use Lights and Cameras without attaching them to nodes (only for legacy code).
- `OGRE_GLSUPPORT_USE_EGL` use EGL instead of GLX/ WGL for GL RenderSystems. This is required for Wayland support.
- `OGRE_USE_WAYLAND` will use Wayland window system instead of X11 on Linux.

Once you are satisfied, hit
*Configure* again and then click on *Generate*. CMake will then create
the build system for you.

Building
--------

Go to your chosen build directory. CMake has generated a build system for
you which you will now use to build Ogre. If you are using Visual Studio,
you should find the file OGRE.sln. Open it and compile the target
*BUILD_ALL*. Similarly you will find an Xcode project to build Ogre
on MacOS.

If you rather want to trigger the build form a console, then cd to your build directory and call the appropriate make program as

    cmake --build . --config Release

to start the build process.

@note on multi-config generators, notably Xcode and MSVC, `--config` must match the `CMAKE_BUILD_TYPE` specified when building the dependencies. This does not apply if you manually provide the dependencies as release *and* debug.

If you have doxygen installed and CMake picked it up, then there will
be an additional build target called *OgreDoc* which you can optionally build.
This will freshly generate the API documentation for Ogre's classes from the header files. In Visual Studio, just select and build the target *OgreDoc*, on Linux type:

     make OgreDoc


Installing {#installing-sdk}
----------

Once the build is complete, the libraries and headers must be copied to a clean location.
This is required if you want to reference %Ogre in your own projects with CMake.
In Visual Studio, just select and build the target *INSTALL*. When using the command line, type:

    cmake --build . --config Release --target install

For Makefile based generators, type:

    make install  # (or sudo make install, if root privileges are required)

On Linux Ogre will be installed to `/usr/local` by default. On Windows this will create the folder `sdk` inside your build directory and copy all the
required libraries there. You can change the install location by changing the variable `CMAKE_INSTALL_PREFIX` in CMake.

Installing and building via vcpkg
---------------------------------
You can download and install ogre using the [vcpkg](https://github.com/Microsoft/vcpkg) dependency manager:
```
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
./vcpkg integrate install
vcpkg install ogre
```
The ogre port in vcpkg is kept up to date by Microsoft team members and community contributors. If the version is out of date, please [create an issue or pull request](https://github.com/Microsoft/vcpkg) on the vcpkg repository.

# Cross-Compiling

Android
-------

To build Ogre for Android, you need to specify the android cross toolchain to cmake as

    cmake -DCMAKE_TOOLCHAIN_FILE=path/to/android-ndk/build/cmake/android.toolchain.cmake -DANDROID_NDK=path/to/android-ndk .

this will build the core Ogre libraries. Additionally it will create gradle projects `OgreJNI` for using Java bindings and `SampleBrowserNDK` for the C++ only Sample Browser.

You can now import these projects in Android Studio or manually trigger the APK creation by changing into the project folders and running

    gradle assembleRelease

WebAssembly / Emscripten
------------------------
Install the Emscripten SDK (see full documentation on [www.emscripten.org](https://emscripten.org/docs/getting_started/downloads.html)), and make sure
that the environment variables are correctly set (eg. run `source <emsdk_path>/emsdk_env.sh` before attempting to build)

Run cmake in cross compile mode using emscripten as following:

    mkdir build-wasm
    emcmake cmake .. -DCMAKE_BUILD_TYPE=Release
    emmake make

NB: to simplify the process, 'emcmake' and 'emmake' wrappers are used. These tools are provided by Emscripten to correctly setup the cross compilation environment

This will not build the full SampleBrowser, but just a minimal Sample. The resulting `EmscriptenSample.html` will be placed in `${CMAKE_BINARY_DIR}/bin/`.

To prevent any cross-origin issues, start a local webserver as `python3 -m http.server 8000` and visit http://localhost:8000.

iOS OS
------

To build Ogre for iOS, you need to specify the ios cross toolchain to cmake as

    cmake -DCMAKE_TOOLCHAIN_FILE=CMake/toolchain/ios.toolchain.xcode.cmake -DIOS_PLATFORM=SIMULATOR -G Xcode .


Unfortunately, you will now have to do a few manual steps to
make the generated build system work properly.

A Xcode project has now been generated in the build directory, so
to start the Ogre build, open OGRE.xcodeproj and build as usual.
To run samples on your device you will need to have a valid iOS Developer
certificate installed.  For each sample, double click on target in the Groups &
Files list.  Ensure that a valid identity is selected in the Code Signing Identity
drop menu.
Also, because we can't tell CMake what Xcode project format you want, you will
have to change it yourself.  Open the Project Menu, choose Edit Project Settings.
Click on the General tab in the settings window.  Change Project Format to
Xcode 3.1-compatible.
And another thing.  You will need to manually set the Bundle Identifier property of
the Info.plist file to match the App ID of the chosen code signing identity.
This can be done from the Target Properties panel.  It must match the bundle
identifier of a valid developer certificate if you are building for devices.

WinRT / UWP
------------------------------

You need Windows 8.0 or later, Windows 10 is recommended.

Visual Studio 2015 is recommended as it is bundled with Universal 10.0.240.0, WinStore 8.0/8.1 and WinPhone 8.0/8.1 SDKs.

Dependencies for Win32 and for WinRT must be located in
separate folders. Cg is not supported.

    cmake.exe -G "Visual Studio 15 2017" -DCMAKE_SYSTEM_NAME=WindowsStore -DCMAKE_SYSTEM_VERSION=10.0 ..

Select SampleBrowser as the start up project and run.

@par Notes

1. The code and generated CMake solution should be on local NTFS drive,
and can't be on a network drive, including VMWare shared folders - or
you will get a errors when you will try to compile/link/run resulting exe.

2. Ogre uses d3dcompiler_xx.dll to compile shaders, and WinStore and
WinPhone 8.1 projects can use it without restriction as it is part of OS.
But WinStore and WinPhone 8.0 applications should load already compiled
shaders from cache, as d3dcompiler_xx.dll is available to them only during
development. Therefore to avoid necessity to deploy d3dcompiler_xx.dll
during development (additional configurations with d3dcompiler_xx.dll)
and generation of such cache - use version 8.1 of these platforms.

3. Running WinPhone emulator in OS running under VMware:
We were able to run the emulation and debug without an issue by using two
steps: (a) Under the settings of the VM > CPU, make sure you have the
option to pass-through the Intel VT-x/EPT feature, (b) Edit the config
file .vmx and add the parameter: hypervisor.cpuid.v0 = "FALSE"
All versions of Visual Studio 2012 have a window refresh issue when running
in VMware and the window is maximized, the solution is just to change the
size of the Visual Studio window to be less the the screen width and height.
