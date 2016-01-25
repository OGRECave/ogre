# Changes in master but not on Bitbucket

## Tests
* Visual Tests can be built on Android
* Visual Tests can be built without OIS
* Unit Tests are run and enforced on each commit on Linux
* Visual Tests are run on each commit on Linux
* share code with SampleBrowser
* regenerate some test images so they can be loaded with STBImageCodec
* fixed ClearScene Visual Test
* fixed FileSystemArchive tests on Linux & OSX
* fixed RenderSystemCapabilites tests on Linux
* fixed Memory Leak in Unit Test Suite
* Test Suite returns non-null for easy CI integration
* deleted dead PlayPen Tests

## OgreMain
* fix GLSLES Shaders for ShadowVolumeExtrudeProgram
* STBImageCodec: add support for saving PNG files
* fix build on iOS using cross-toolchain/ add instructions
* backed out [bad commits of PRs #588, #577](https://bitbucket.org/sinbad/ogre/pull-requests/599/backed-out-merged-in-liorl-ogre-main-pull/diff#comment-12710605)

## SampleBrowser
* uses SDL2 instead of OIS for input when available (Desktop)
* can be compiled without any input. Start a sample using `./SampleBrowser <SampleNumber>`
* neither SDL2 nor OIS needed for Android anymore
* improved input handling on Android

## RTShaderSystem
* fix FFP_GenerateTexCoord_EnvMap_Reflect on GLSL. Environment Maps are no longer flipped.

## GLNativeSupport (new component)
* Factored out GL Support classes that handle GL context creation
* Defines new GLNativeSupport interface. Platform specific classes (e.g. GLX, WGL) no longer directly accessible.
* Shared between all GL RenderSystems
* allows specifying GL context profile (Core, Compability, ES)
* only one place for fixing GLX/ WGL/ Carbon related bugs
* Dropped 10.000 LOC. Could use SDL2 internally to drop even more

## GLES2: use ES context profile on Desktop ([PR](https://github.com/OGRECave/ogre/pull/183))
* run and test the GLES2/3 RenderSystem on Desktop
* uses GLSupport Module instead of EGL for Extensions/ Context

## EGL in GLNativeSupport (WIP, [PR](https://github.com/OGRECave/ogre/pull/185))
* allow EGL to create "full" GL Contexts as well
* needed for [real headless rendering](http://devblogs.nvidia.com/parallelforall/egl-eye-opengl-visualization-without-x-server/)
* already can start GL3Plus on Linux (however uses Mesa EGL instead of Nvidia)

# Changes on Bitbucket but not in master
* Organize projects into project folders in MSVC projects
* Improved microcode cache
* Direct3D11 Fixes and improvements
