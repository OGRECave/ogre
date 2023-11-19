# Change Log

## v1.10 and newer
See "New and Noteworthy" for each release - e.g. [1.10-Notes.md](1.10-Notes.md)

## v1.9.0 [Ghadamon] (22 November 2013) - MAJOR RELEASE
* [Change log in the Ogre3D wiki](https://wiki.ogre3d.org/tiki-index.php?page=GhadamonNotes)
* [Closed JIRA tickets for Ogre 1.9](https://ogre3d.atlassian.net/issues/?jql=fixVersion%20in%20%28%221.9.0%22%2C%20%221.9.0%20RC1%22%2C%20%221.9.0%20RC2%22%29%20AND%20status%20in%20%28Resolved%2C%20Closed%29%20ORDER%20BY%20key%20DESC)

## v1.8.1 [Byatis] (02 September 2012) - MAINTENANCE RELEASE
<ul>
    <li>[GLES2] Add missing shader for skeletal sample</li>
    <li>Fix crashes in OS X template code.</li>
    <li>[OS X] Resolve some issues with visibility of symbols preventing universal builds.</li>
    <li>[GLES] Improvements to pixel format selection. Reduces conversions at runtime and enables more formats.</li>
    <li>Fix a couple defines in GLEW headers that I mistakenly altered during a batch replace. Resolves issues with FSAA on Windows using OpenGL</li>
    <li>Disabling X11 check for all Apple platforms. Because it may not always be available on the system.</li>
    <li>Fix a bug with OctreeNodes that are not in the scene graph being updated improperly. As reported in viewtopic.php?f=4&t=70617&p=461051</li>
    <li>Patch 3517455 - Rename the COPYING file due to CPack not allowing files without extensions.</li>
    <li>Patch 3526568 - Fix for when a listener object unreferences itself from the listened object while in a callback</li>
    <li>Patch 3525902 - SceneManager - Fix for when a listener object unreferences itself from the listened object while in a callback</li>
    <li>Bug 521 - Copy the flag to control auto edge list building when cloning a mesh.</li>
    <li>Bug 532 - Default values not being returned on invalid input to StringConverter::parse* functions</li>
    <li>Patch 3519819 - Add missing setRenderQueueGroupAndPriority functions to ParticleSystem and BillboardParticleRenderer.</li>
    <li>OS X - Fix the "invalid drawable" error that pops up in the log when a window is created.</li>
    <li>Patch 3324815 - Fix incorrect zip archive behavior. This also fixes a few of our unit tests.</li>
    <li>Add a missing capabilities mapping</li>
    <li>Fix up the Xcode 4 template</li>
    <li>iOS: Fix crash caused by missing GL function pointers with GLES 2 on iOS 4</li>
    <li>Revert part of a change that I made several months ago that was causing problems with texture shadows without edge lists.</li>
    <li>Bug 538 - Support for boost 1.50</li>
    <li>Bug 536 - Freetype 2.4.10</li>
    <li>Fix [3538257]: Listener was never actually removed in SceneManager::removeListener</li>
    <li>added check for icon existance in bool GLXGLSupport
    </li>
    <li>Made SimpleRenderable::setMaterial virtual.
    </li>
    <li>[OS X]  Use the path from the FindCg script to create the framework symlink instead of assuming the prebuilt dependencies are being used.</li>
    <li>Add a missing semicolon that has been causing some compile errors when using boost for threading.</li>
    <li>Moved call to wglShareLists before call to wglMakeCurrent to help Wine emulation</li>
    <li>[D3D9] Catch texture load exception in D3D9Texture::notifyOnDeviceCreate to prevent Ogre from crashing when moved to a new display</li>
    <li>[Papercut] Resolve ambiguity of get/setUserAny in InstanceBatch</li>
    <li>Fix [3545669]: Submesh operation type is not passed to tangent calculator causing triangle strips and fans to have improper tangents</li>
    <li>Fix [3542148] Properly set texture coordinate index in D3D9 if vertex program is used with projective texturing</li>
    <li>Fix [3538259]: Use _tempnam on Windows to generate temporary file name in OgreDeflate.cpp</li>
    <li>Fix [3535939]: In CompositorInstance::getTargetForTex search also for referenced texture name if referencing another compositor's texture</li>
    <li>[3531905] Fix a bug in LOD generation of XSI exporter</li>
    <li>[3531904] Update FindSoftimage.cmake to find Softimage 2013</li>
    <li>Applied and enhanced masterfalcon's patch to prevent BillboardChains from
        updating their vertex buffer except when they are actually being rendered.</li>
    <li>Fixed ParticleSystem's emitted emitters 'sputtering' when emission rate per frame near 1/2.</li>
    <li>Fix [522]: D3D9 / D3D11 destroyRenderTarget caused dangling pointer inside RS's</li>
    <li>Fix [504]: D3D9 Prevent infinite loop when CheckDeviceMultiSampleType fails when
        fsaa is 0</li>
    <li>Fix [454]: D3D9 add more error details when we can not create a vertex shader</li>
    <li>[3484580] Fix a potential crash when loading materials referencing non-existent shadow caster/receiver materials</li>
    <li>Increased size of GLX config dialog to accomodate for additional GL RenderSystem option</li>
    <li>TerrainGroup did not pass along parameters to underlying Terrain in update and updateDerivedData</li>
    <li>Fix [541]: In Rev 3513 the warning texture is created with a invalid context so
        the gl id was 0 - this broke materials without valid textures (if no texture is
        bounded the warning texture is bound)</li>
    <li>Fix for possible endless loop if requesting max number of mipmaps with 0 depth. Affects all GL render systems.</li>
    <li>Fixed -msse flag when building with MinGW.</li>
    <li>[OS X] Fix displaying garbage on first frame when using Cocoa windows.
        Also fixing the "invalid drawable" error.</li>
    <li>Fix incorrect header guard.</li>
    <li>Replace an ugly hack for GCC that was trying to force 16-byte stack alignment in OgreOptimisedUtilSSE, but causes trouble with recent GCC versions</li>
    <li>GLES/GLES2 move error checks into the right place</li>
    <li>Fixed bug: D3D9 can now be compiled in static with MinGW.</li>
    <li>Get PlayPen and VTests running with static builds like iOS. Also some minor code cleanup.</li>
    <li>Patches 3506252 & 3535295 - Profiler enhancements and support for GPU profile events.</li>
    <li>Fix [541]: viewportDestroyed were never called</li>
    <li>Fix a crash when starting/stopping the terrain sample with shadowing enabled. House entities need to be freed.</li>
    <li>Fix [549]: When ogre is compiled with profiler support link dx9 to dx11 so we can
        use D3DPERF_BeginEvent/D3DPERF_EndEvent/D3DPERF_SetMarker
        Fix some profiler crashes</li>
    <li>Bugfix: Instancing LOD distance strategy was considering entities that were not in the scene.
        Bugfix: Instancing LOD distance strategy was using very inaccurate distance formula.</li>
    <li>Bug 545: ScriptLexer is counting \r\n as 2 newlines</li>
    <li>Patch 3505652 - Fix for compositors recompiling every frame</li>
    <li>Patch 3489471 - Entity LOD animation bugfix</li>
    <li>Fixed a bug in the CMakeLists.txt that would cause an error when the user
        influences CMAKE_CXX_FLAGS manually in CMake.</li>
    <li>[GLES2] Attempt to resolve the isAttributeValid crash when shaders are either missing or have failed to compile/link. Also do a little more logging when reporting compile errors.</li>
    <li>Calling removeInstancedEntity on the same InstancedEntity more than once will now raise an exception.</li>
    <li>[iOS] Remove old GCC define to disable thumb and use a compiler flag instead. More reliable across gcc and clang.</li>
    <li>Make it possible to call Viewport::removeListener during viewportDestroyed event</li>
    <li>[OS X] Correctly compute Cocoa window origins and fix repositioning.</li>
    <li>HW-Instancing: Add support for meshes which use a shared vertex buffer</li>
    <li>Fixed the NaCl compile and link for the latest code changed in 1.8</li>
    <li>Fixed texture loading issues in GLES2 RS for NaCl.</li>
    <li>Changed RenderSystems on Windows to interpret externalWindowHandle and parentWindowHandle parameters as size_t instead of unsigned int, which has the wrong size on 64bit platforms</li>
    <li>Fix Unity builds for OSX (SampleBrowser did not compile)</li>
</ul>


## v1.8.0 [Byatis] (25 May 2012) - MAJOR RELEASE
There have been well over 1000 commits since the release of v1.7.0. The following list
shows only the highlights of new features.

<ul>
<li><b>Platform support</b>
    <ul>
        <li><b>iOS</b>
            <ul>
                <li>Use the device's screen size to determine the default screen size on iPhone OS platforms.</li>
                <li>Profiling showed that aligning loops on iPhone would be a good thing.</li>
                <li>iOS: Changing OGRE_PLATFORM_IPHONE to OGRE_PLATFORM_APPLE_IOS</li>
            </ul></li>
        <li><b>NaCl</b>
            <ul>
                <li>Added NaCl platform support</li>
            </ul></li>
    </ul></li>

<li><b>Render systems</b>
    <ul>
        <li><b>GLES2 render system</b>
            <ul>
                <li>Added GLES2 render system.</li>
                <li>Uses the RT shader system for fixed-function emulation.</li>
            </ul></li>
        <li><b>D3D9 render system</b>
            <ul>
                <li>Hardware buffers system memory consumption reduced. Only write only access buffers allocates extra memory.</li>
                <li>Fixed hardware buffer lock crashes when locked with read_only flag.</li>
                <li>Fix hardware buffers crash when used create on active device policy.</li>
                <li>Only default pool hardware buffers are re-created after device reset (Should fix resizing issues)</li>
                <li>Added new config option related to system memory usage when multiple devices involved.</li>
            </ul></li>
        <li><b>D3D11 render system</b>
            <ul>
                <li>Removed the obsolete shader generator (Now that we have the RTSS – we don’t need it anymore).</li>
                <li>A number of the Ogre samples now work with the D3D11 render system.</li>
                <li>Added code to support copy of vertex buffers that where allocated in system memory. (The terrain sample does such a copy).</li>
                <li>DX 11 Stream Out support (along the lines of the existing GL support).</li>
                <li>texture2d-array support added</li>
                <li>Various improvements.</li>
            </ul></li>
    </ul></li>

<li><b>Components</b>
    <ul>
        <li><b>Terrain component</b>
            <ul>
                <li>Patch 2951501: allow changes in terrain size in vertices and world size on the fly</li>
                <li>Support for Terrain vertex compression.</li>
                <li>Terrain now defaults to saving compressed files (can still load uncompressed)</li>
            </ul></li>
        <li><b>Real-Time Shader System (RTSS)</b>
            <ul>
                <li>Added layered blending. Added an entity that shows the feature to the RTSS sample - a rock wall with the OGRE logo blended with the new "overlay" blend.</li>
                <li>Blending modes for GLSL ES.</li>
                <li>Added Hardware skeleton animation in RTSS - A patch by Mattan F.</li>
                <li>Added instanced viewports.</li>
            </ul></li>
    </ul></li>

<li><b>Additional updates (highlights only)</b>
    <ul>
        <li>Fixed/silenced a number of compiler warnings on all platforms.</li>
        <li>Patch 2942638: allow Mesh to have infinite and null AABBs.</li>
        <li>D3D10 render system removed – the system is obsolete and all of its functionality exists in the D3D11 render system.</li>
        <li>Remove TerrainSceneManager - no longer supported, use Components/Terrain instead.</li>
        <li>Patch 2952444: support float1-4, short1-4, colour and ubyte4 in texture coordinates in XML converter</li>
        <li>Patch 2955902: Add Grid3DPageStrategy for space scenes or similar</li>
        <li>Patch 2963105: Add ability to set depth clear value on viewport</li>
        <li>Added DeflateStream for compressing / decompressing regular streams</li>
        <li>Support normals in vertex animation</li>
        <li>MeshUpgrader can now also *downgrade* mesh files with the -V option</li>
        <li>RenderWindows can now be hidden (useful for creating an invisible primary renderwindow).</li>
        <li>Add vsync adjustment options to RenderWindow</li>
        <li>Added new InstanceManager (see http://www.ogre3d.org/forums/viewtopic.php?f=4&t=59902)</li>
        <li>Ogre can now run on Ubuntu in a VirtualBox.</li>
        <li>Added optional shader cache feature: Compiled shaders can be saved to improve loading times.</li>
        <li>Added EmbeddedZipArchiveFactory to OgreZip.</li>
        <li>Fixed a number of papercuts.</li>
        <li>Introduced Unity builds: Enabling OGRE_UNITY_BUILD in CMake will significantly speed up Ogre compile times.
            Still considered experimental, but should be fine.</li>
        <li>Add nograb command line parameter to SampleBrowser to allow debugging with gdb</li>
        <li>Allow skeletal and pose animations to be relative to a keyframe of an animation instead of the binding pose</li>
        <li>GSoC 2011 project: Added VisualTest infrastructure</li>
        <li>GSoC 2011 project: Dual Quaternion skinning</li>
    </ul></li>
</ul>


## v1.7.4 [Cthugha] (14 January 2012) - MAINTENANCE RELEASE
<ul>
<li>iOS: Fix the fix I did for libraries that are included in SDK builds. Use iOS 3.2 when building SDKs</li>
<li>OS X: Add visibility attributes to Cocoa classes so the symbols are exported. Apparently symbol visibility only has an effect for Objective-C on x86_64 targets.</li>
<li>OS X: Fix the OS X build</li>
<li>Bug 443: Small fix for Image::setColourAt</li>
<li>GL: Fix for a couple old bugs that could cause lots of GL errors.  One is an assumption about 2 sided stencil support always being there.  The other related to the wrong enumerations being used for the extension that we are checking.  See <a href="http://www.ogre3d.org/forums/viewtopic.php?f=2&t=44132">relevant forum topic</a>.</li>
<li>Missing D3D9 Software vertex processing fix</li>
<li>Xcode templates update</li>
<li>Adding RTShaderLib files to Xcode templates</li>
<li>OS X: Fix for input issues on Lion.  NSTitledWindowMask is no longer implied when creating a NSWindow.  To fix, we just add it to the style mask that we use.</li>
<li>iOS: The Dynamic Texture sample works.  It should be enabled.</li>
<li>Add a forward slash to the beginning of paths in the Xcode 4 templates to workaround a weird bug in the template wizard that eats up any leading slashes in text fields</li>
<li>GLES: A little cleanup and fixed GL errors when using BGRA textures.</li>
<li>Fix the incorrect average and max FPS entries in SDK trays.</li>
<li>The Xcode 4 templates will need to link to the CoreGraphics framework now.</li>
<li>OS X: FSAA was not being applied to the both Carbon and Cocoa contexts at all.</li>
<li>Updated FindOGRE.cmake to properly set OGRE_${COMPONENT}_BINARY variables with paths to the corresponding DLLs</li>
<li>Updated FindTBB.cmake to correctly identify the version of Visual Studio used</li>
<li>Consider OGRE_LIB_DIRECTORY when setting RPATH for SampleBrowser and OctreeZone plugin</li>
<li>Fix D3D9 capabilities check for cubemaps</li>
<li>Fixed a memory leak in InstancedGeometry</li>
<li>Fix Boost license file location in CMake</li>
<li>Fix definition of element extremes in ogremeshxml.dtd</li>
<li>Have D3D9 rendersystem clean up properly when creating a renderwindow fails</li>
</ul>

## v1.7.3 [Cthugha] (08 May 2011) - MAINTENANCE RELEASE
<ul>
<li>iOS: Remove the animation timer.  Since DisplayLink is used by default now, this only hurts performance of things like input.</li>
<li>Some small changes to ensure that the terrain and paging libs are added to linker flags for SDK sample builds.</li>
<li>Consider weight when scaling in AnimationTracks</li>
<li>Only allow to set custom render capabilities before RenderSystem is fully created</li>
<li>iOS: Fix to prevent absolute paths from being inserted into resources.cfg for iOS to fix running the sample browser on devices.</li>
<li>OS X: The Cocoa view has no use inside the main library.  Moving into the GL Rendersystem where it is actually used.</li>
<li>OS X: Disabling CoreGraphics error checking by default</li>
<li>OS X: 64 bit Cocoa support.  New dependencies are also available to download.  Fixed a few uninitialized variables along the way.  Also updated GLEW to the same version as in default, the older version had some Apple specific bugs that needed to be resolved.</li>
<li>Several fixes for the Xcode templates such as: file permissions for the installer, iOS device orientation.</li>
<li>Adding Cocoa window event handling.  Plus several other fixes for parameter parsing and other things.  Thanks to jdiogo for finding the bugs.</li>
<li>OS X: Build fix when targeting 10.5 or earlier.</li>
<li>OS X: A few fixes for Cocoa windowing.  Now plays nicer with externally created windows.</li>
<li>Clean up several warnings(hidden local variables, unused functions)</li>
<li>iOS: Fix orientation change support.  Use UISupportedInterfaceOrientations in your apps info property list to restrict which orientations are supported by your application.</li>
<li>Fix a couple comment typos</li>
<li>GLES: Disable ENABLE_GL_CHECK, again.</li>
<li>GLES: Fix for images with custom mipmaps.  The dimensions were never being reduced for each mipmap level.  As a side effect, memory usage is also reduced.</li>
<li>iOS: IOKit isn't needed at all and causes link errors with iOS 4.2.</li>
<li>Separate out all the OS X and iOS specific code from SampleBrowser.cpp.  It was getting a bit unruly and difficult to maintain</li>
<li>iOS: Improve orientation support.  Separate EAGLView into its own files.</li>
<li>iOS: Several fixes to the Xcode templates regarding viewport orientation and some cleanup for readability.</li>
<li>Patch 3116577: Plane equality operators.  Also cleaned up some documentation.</li>
<li>Fix a catch-22 that prevented OGRE_BUILD_PLATFORM_IPHONE from showing up in CMake-Gui.</li>
<li>Fixed an incorrect error message in the Terrain component.</li>
<li>Do not offer the Carbon API option in 64-bit Mac builds and default to Cocoa</li>
<li>Allow the retrieval of NSOpenGLContext and NSOpenGLPixelFormat easily in OSXCocoaWindow</li>
<li>Specify the NSOpenGLFPAScreenMask to resolve ambiguity in the pixel format on multiple display systems</li>
<li>In OSXCocoaWindow::createWindowFromExternal, don't force the window to be made key and ordered in front. As an external window, the calling application should have full control over window behaviour.</li>
<li>When creating an external window in Cocoa, don't centre the window (app should be in charge of that)
    Also don't mess with the first responder.</li>
<li>Fix using multiple Cocoa windows with Ogre.
    Previously the window delegate was incorrectly listening in on the events of *every* window, not just the one containing Ogre. This meant if the application had more than one window (Ogre or otherwise), the Ogre windows would get confused with all the events from different windows.
    Fix this by making sure each delegate only attaches to the NSNotifications of the specific window it's concerned with.</li>
<li>Support getCustomAttribute() on Texture, only GL extracts anything interesting so far but more can be added
    This is pretty much essential if you want to get to internal API data for resource sharing without down-casting, which itself requires otherwise unnecessary linking to plugins. It's why we've offered this for RenderWindow etc in the past, I'm not sure why it's never been done for Texture.
    Also support retrieving the FBO ids directly from render targets on GL (already allowed retrieval of FBO struct but again that requires linking)</li>
<li>Added support for spotlight_viewproj_matrix_array GPU parameter</li>
<li>Changed DataStream::getAsString and MemoryDataStream constructors to deal with streams of unknown size</li>
<li>[Papercut] Add destroyRenderTarget function to Ogre::Root</li>
<li>Fix a problem with using some of the lower-level renderable callbacks such as RenderObjectListener to alter shader parameter state - mGpuParamsDirty would not be updated to reflect this and as such things like manual param variances within light iteration loops would not be propagated.
    Allow user to mark GPU params dirty themselves to resolve this.</li>
<li>iOS: Rework some of the sample browser code to shut down properly on iOS</li>
<li>GLES: Use the correct GL type for BGRA textures</li>
<li>OS X: Use correct pixel format attribute name for specifying FSAA in Carbon windows.</li>
<li>GL: Only bind up to the max supported number of render targets since not all implementations support 8.  This prevents a few OpenGL errors.</li>
<li>iOS: Clean up the FSAA/framebuffer code in swapBuffers.  This should resolve issues on iOS 4.1 that have been reported. Bug #384</li>
<li>iOS: Don't search for X11 if building for iOS.  I'm surprised that this hasn't been found until now.  Apparently most devs have the X11 package installed.</li>
<li>iOS: 2 fixes.  The compiler should be g++ instead of gcc and switching the architecture to build for both armv6 and armv7.</li>
<li>OS X: A few CMake fixes to ease building for universal libraries.  Upping the minimum OS to 10.5(it's required for x86_64).  Also updating the list of Boost versions to be current.</li>
<li>Don't apply visibility settings to statically built samples. Fixes linking problems with Xcode 4 and iOS. (Backporting to 1.7)</li>
<li>Remove a GL ES 2 reference in the 1.7 branch</li>
<li>iOS: The meaning of ARCHS_UNIVERSAL_IPHONE_OS changed in Xcode at some point to just armv7.  Changing it to Standard will compile for both armv6 and armv7.</li>
<li>OS X: A few small tweaks for Cocoa windows.  Clearing the framebuffer right away, fixing multisampling for example</li>
<li>iOS: Let's pretend that the iOS simulator doesn't have SSE. (Works around a Xcode 4 bug)</li>
<li>OS X: Add support for 8 FSAA samples</li>
<li>Update the boost versions to look for.</li>
<li>Patch 3153910 - Fix a typo in MovableObject::setRenderQueueGroupAndPriority.  Render queue priority should be set to the priority argument, not the queue ID.</li>
<li>Patch 3221772 - iOS: Fixed bug in setting up the viewport if the lower-left corner is not 0,0.</li>
<li>RTSS: Fix the "Disco" effect in the Shader Sample on OS X.  Thanks to Wolfmanfx!</li>
<li>Fix a documentation spelling error in 2 places</li>
<li>Patch 3046729 - Improvements on previous ProgressiveMesh patch.  "Sometimes it seems to be actually desired to list itself as neighbor, so instead of denying this, we rather make the loop in ProgressiveMesh::PMTriangle::notifyRemoved more robust to these edge conditions."</li>
<li>GLES: Fix using PVR textures</li>
<li>OS X: Fix a crash when switching between windowed and full screen when using the Cocoa interface.</li>
<li>Bug #397: Fix the build with some versions of GCC.</li>
<li>[Papercut] Image getColourAt parameters should be type size_t, not int</li>
<li>373 - [Papercut] Image has getColourAt but not setColourAt</li>
<li>Bug 409 - System freezes in GLPixelUtil::getMaxMipmaps when width or height is 0.  Bug was reported for GL but could affect GL ES as well.</li>
<li>Reformat a little text in an exception so that it follows the format used elsewhere.</li>
<li>Bug 374 - [Papercut] PixelBox should have getColourAt and setColourAt</li>
<li>Bug 365 - [Papercut] void BillBoardSet::setMaterial (const MaterialPtr &material) is missing</li>
<li>Bug 340 - Viewport::clear() saves and re-sets the previous Viewport, even if that Viewport has since been deleted</li>
<li>Bug 344 - Add utility functions to enable/disable skybox/dome/planes instead of destroying and recreating.</li>
<li>Bug 423 - Fix for looking up for texture definitions in very complex compositor setups in getSourceForTex and getTargetForTex</li>
<li>iOS: Explicitly specify the release lib paths so that libraries are always installed to the correct places.  This fixes the problem of duplicate, single architecture libraries in SDK builds.</li>
<li>iOS: Remove -fno-regmove flag to keep Clang from complaining about it.</li>
<li>Update the SDK CMakeList template</li>
<li>Several updates and fixes for the OS X and iOS SDK build scripts</li>
<li>OS X: Fix a long standing issue that often prevented 3 situations: Building with Clang, 64 bit debug builds and linking with Xcode 4.
<li>OS X: Add macAPI option to the config dialog</li>
<li>Added Gentoo install location for Cg to the FindCg.cmake script</li>
<li>Fixed a comment in OgrePixelFormat.h</li>
<li>Modified FindTBB.cmake to cope with TBB 3 paths</li>
<li>Fixed a build error with GCC 4.6</li>
<li>Xcode 4 templates and installer files</li>
<li>iOS: Normalize the case of the word Media in scripts.  Simplifies a little scripting.</li>
</ul>

## v1.7.2 [Cthugha] (03 November 2010) - MAINTENANCE RELEASE
<ul>
<li>Fix bug 240: TextAreaOverlayElement incorrectly loads Font in background thread (OGRE_THREAD_SUPPORT == 2)</li>
<li>Report errors if installable DLLs missing (e.g. cg.dll)<br>
    Make sure OGRE_INSTALL_DEPENDENCIES always true</li>
<li>Fix Cmake error with VS 2010</li>
<li>Fixed a compile bug - in visual studio 2010 - 64 bit we get this error: "LINK : fatal error LNK1210: exceeded internal ILK size limit; link with /INCREMENTAL:NO"</li>
<li>SDK build - added vcxproj ext to the batch file - for VC10 support</li>
<li>Fix a breakage from rev 2133 on systems other than 64-bit VS 2010
    - Cannot pass a blank string to LINK_FLAGS</li>
<li>Fix an infinite loop that could occur with StringUtil::replaceAll if replaceWhat is a substring of replaceWith. Solved by advancing the string position after each replacement.</li>
<li>Make sure the correct resource path is inserted into config files when building the OS X SDK.</li>
<li>Correct the iPhone SDK disc image to use the correct background image file.</li>
<li>Patch 2979571 by moagames : textures referenced with texture_ref can now be also used as render targets. http://www.ogre3d.org/forums/viewtopic.php?f=4&t=56671</li>
<li>Dispose of Carbon window handles when the window object is destroyed. Fixes a problem with old windows remaining visible after the resolution has been changed.</li>
<li>Removed redundant compositor creation call from deferred shading demo.</li>
<li>Allow overriding Rectangle2D's vertex buffer usage, and using this in the compositor's quads. As discussed in http://www.ogre3d.org/forums/viewtopic.php?f=4&t=57656</li>
<li>fixed parsing of matrix types from shared parameter definition</li>
<li>ProgressiveMesh::PMVertex can list itself as a neighbor after collapsing vertices. This leads to a crash in ProgressiveMesh::PMTriangle::notifyRemoved.
    Changed ProgressiveMesh::PMVertex::removeIfNonNeighbor to always remove itself from the neighbor set.</li>
<li>fixed destroySubMesh corrupting the mesh if it contains an edge list</li>
<li>Add OgreOverlayElementFactory.cpp for methods which can't be inlined.</li>
<li>Install ressources according to OGRE_LIB_DIRECTORY .</li>
<li>Support LIB_SUFFIX. Which is the de-facto standard for cmake to support lib64|32 systems.</li>
<li>Removing a few unused variables to lower the number of compiler warnings.</li>
<li>GLES: Fixed a crash on texture creation.  Added buffer mapping support(port from GL Rendersystem).</li>
<li>Adding a few missing header search paths to the Xcode templates.</li>
<li>Adding iPhone 4's native resolution.</li>
<li>Xcode Templates: Use devices' native resolution. Shutdown properly. Add CADisplayLink support.</li>
<li>iPhone: Adding CADisplayLink support to SampleBrowser. Shutdown the app properly on iPhone and disable the loading bar to speed up loading on iOS 4</li>
<li>GL ES: 16 bit texture support.</li>
<li>OS X: When targeting 10.6 only - Fixed an issue with old framebuffer contents being displayed when using non-native resolution full screen contexts. Also fixed a crash when switching resolutions.  Added some error checking to the full screen context creation.</li>
<li>Disabling DisplayLink on iPhone by default, as it can cause input lag sometimes.</li>
<li>Fix a cross-endian bug in StreamSerialiser::calculateChecksum<br>
    Files written on one endian system would fail checksums on another (even though the data can be converted on the fly) because the checksums were calculated on the raw memory of the header data
    Now always calculate checksums in little-endian</li>
<li>Endian fix for FastHash - casting to uint16 is non-portable<br>
    NOTE: this will break checksums if you're on a big-endian machine using files generated before this change.</li>
<li>Fix StreamSerialiser::REVERSE_HEADER_ID which was only reverse for uint16, not the declared uint32</li>
<li>OS X: Reworked windowing to fix setFullscreen and provide more robust GL context management</li>
<li>Adding additional directories to search for the Ogre framework when using a prebuilt SDK with your own projects under OS X</li>
<li>OS X & iOS: Simplify some post build commands by using wildcards. Updating pbxcp commands to exclude .hg directories.</li>
<li>Refreshed iOS SampleBrowser icons and launch images</li>
<li>Adding new iOS higher resolution icons and launch images</li>
<li>Uncomment the use of the HideOnFullScreen attribute for Carbon windows</li>
<li>Adding the using keyword to resolve a set of warnings.  Also removed a switch for a simple if-else.</li>
<li>Very minor cleanup from a bad copy/paste in Xcode template installer code</li>
<li>iOS: Several additions to properly support newer iOS devices including: High resolution screen support, FSAA and Framebuffer discard.  A config key has been added to allow you to manually configure the content scaling factor.  For example, if you only want 720x480 on an iPhone 4, specify contentScalingFactor=1.5.</li>
<li>iOS: Fixed the camera frustum and viewport dimensions if the device is in landscape. With this fix, the FOV and aspect ratio will be correct in all device orientations</li>
<li>Patch 3034005: GLSL writers make wrong assumptions about the precedence of the || && operators (v1-7 applicable part)</li>
<li>Patch 3034009: navigation bug in sample browser</li>
<li>Build fixes for MinGW-w64</li>
<li>(CMake) Properly determine if we are on a 64bit platform</li>
<li>Allow embedding of Dependencies source build into the Ogre source tree</li>
<li>Due to a change made in CMake 2.8.1, Xcode based projects for iPhone would be corrupted.  This fixes the problem.  Also, up the default SDK version to 4.0 since Apple is no longer shipping a 3.0 SDK but still supports targeting it.</li>
<li>LLVM compile fix</li>
<li>Patch 3040518: Fix link warnings when compiling Ogre with -fvisibility-hidden</li>
<li>Updated sed script to work with the CMake 2.8.1+ fix for iOS</li>
<li>iOS: Cache the system version so we don't query it every time we call swapBuffers</li>
<li>Patch 3034010: Reduce a large number of compiler warnings, particularly shadowed local variables</li>
<li>Backing out use of _OgreExport on AllocatedObject members, causes major build problems on VS</li>
<li>SceneManager::renderVisibleObjectsCustomSequence was calling firePostRenderQueues at the start instead of firePreRenderQueues (thanks Lacero)</li>
<li>Fix a script compile error introduced with the warning reduction patch</li>
<li>Fixing iOS project generation. Running fix_linker_paths.sh may no longer be necessary, especially if using CMake 2.8.1 or later.</li>
<li>Use RenderSystem capabilities to only load samples that require shaders if we support a shading language when linking statically.</li>
<li>Bugfix : Scriptcompiler would crash when a specular attribute contained one number.
    As reported in http://www.ogre3d.org/forums/viewtopic.php?f=1&t=59742</li>
<li>Fix a bug in Unix FileSystemLayerImpl if resolving a symlink fails</li>
<li>Fixed animation track exporting to avoid XMLConverter to crash.</li>
<li>Avoid sending tangent related params to XMLConverter when mesh does not have UV data.</li>
<li>Materials will now fallback to Rendering Material mode when custom material template is invalid</li>
<li>Added _tex[#] handlers to material template system allowing the use of indexed base texture binding.</li>
<li>Improved material warnings for better readability.</li>
<li>Updated help document on feature additions.</li>
<li>D3D9RenderSystem: Fixed crash when HLSL compilation failed and trying to log errors but D3DXCompileShader returned a NULL pointer</li>
<li>Fixed nasty memory corruption in OgreInstancedGeometry, when building the GeometryBucket.
    Discussed in http://www.ogre3d.org/forums/viewtopic.php?f=4&t=60432</li>
<li>[3061946] Fix invalid fbConfig check in GL rendersystem</li>
<li>Fix errors generated from RenderSystemCapabilitiesTests due to missing or misnamed capability keywords.</li>
<li>GLES: Use  _OgreGLESExport instead of _OgrePrivate to control symbol visibility for rendersystem classes.</li>
<li>pkg-config can be used to properly build a PCZSceneManager application</li>
<li>Installing OIS dependency didn't work on RelWithDebInfo with MingW</li>
<li>[3059963] Fix a slight copy&paste error in PCZSM</li>
<li>Change required CMake version to >=2.6.2</li>
<li>[3078774] Convert home path to OEM codepage in FileSystemLayerImpl_Win32</li>
<li>[3074121] Fix handling of lights for multiple displays in D3D9 rendersystem.</li>
<li>[3032954] Properly adjust window size in D3D9 RenderSystem to get the desired client size when switching from fullscreen to windowed.</li>
<li>[3067141] Generate user files with debug paths for Visual Studio 2010</li>
<li>Change Manual & API license to Creative Commons ShareAlike v3.0.
    Apparently Debian will only accept the 3.0 license and not 2.x</li>
<li>[3002754] Add OGRE_UNICODE_SUPPORT guard to OgreUTFString.cpp</li>
<li>[3072166] Fixed a crash on exit when SdkSample is initialised before Ogre::Root</li>
<li>[3053689] Terrain sample: Add Numpad +/- as alternative key bindings</li>
<li>Add threading library's include directory to Ogre include directories in FindOGRE.cmake</li>
<li>Correctly check for PBuffer support even if FBO is supported in GL rendersystem</li>
<li>[3043021] Use CMake to make sure that GCC visibility settings are passed along to Xcode.</li>
<li>[3054042] Fix for two frustums attached to one scene node.  Cameras and Frustums should use the name member that they inherit from MoveableObject instead of their own.</li>
<li>[3003231] Specify default values in StringConverter::parse*</li>
<li>D3D9: RenderWindow::adjustWindow needs to take the current window style into consideration, otherwise the produced values can be off.</li>
<li>D3D9: Fixed a bug where RenderWindow and Viewport sizes would not be in sync during a windowResized event.</li>
<li>OS X: Fix use of macAPI in config files to specify Carbon or Cocoa windowing APIs</li>
<li>A fix so ogre can run on Ubuntu that runs in "VirtualBox".<br>
    At the date of this commit - chooseFBConfig was not supported on VirtualBox - http://www.virtualbox.org/ticket/7195<br>
    Now glXGetFBConfigs is uses as an alternative if chooseFBConfig fails.
</li>
<li>iOS: Build script and CMake updates for iOS 4.  Fixes several project creation bugs and includes a warning about CMake 2.8.2 which cannot create usable universal Xcode iOS projects.</li>
<li>Add some error checking to the Xcode template installer to help alleviate installation problems</li>
<li>Update documentation on Bone::setManuallyControlled to mention the option of using AnimationState::setBlendMask to prevent scripted animations applying to the bone.</li>
<li>Patch 3097617: initialise mManager correctly in default hardware buffers.
    Modified slightly to maintain previous interfaces & not break build</li>
<li>Apply patch 3097617 to the GL ES rendersystem too.</li>
<li>[3057434] Fix RibbonTrail reset when attached to a SceneNode not positioned at the origin</li>
<li>iOS: Guard a media dir variable update since we want to preserve the relative paths on iOS</li>
<li>Remove a couple patterns from the Boost header exclude list.  Some headers in these directories are needed indirectly.</li>
<li>Add missing CMake file to list of scripts to install</li>
</ul>

## v1.7.1 [Cthugha] (25 April 2010) - MAINTENANCE RELEASE

<ul>
<li> Terrain: Fix memory leak after updating lightmaps</li>
<li> Terrain: Ensure that imported position is used when preparing from ImportData</li>
<li> Fix calcMorphKeyframeSize in serializer - there has never been an 'isOriginalGeometry' flag!</li>
<li> Safety checks - use !mSkeleton.isNull() instead of hasSkeleton() when accessing the pointer, since hasSkeleton() determines whether the skeleton name is provided, not whether the skeleton has loaded & the pointer is valid</li>
<li> Terrain: Fix a delete that should have been OGRE_DELETE as pointed out by harkon</li>
<li> Focussed shadow camera setup dealt with extrusion of directional light volume incorrectly:
- It clipped the extrusions against the scene AABB, which may exclude casters in sparse scenes where the main camera has stopped seeing them or their shadows for a frame or more
- It should actually have been extruding the receivers AABB clipped to the camera by the shadow far distance
- Focus regions would not be affected because extrusion along the light direction does not widen the focus; in fact the use of the whole scene BB instead of just the receivers BB may have led to worse focus</li>
<li> ArchiveManager's constructor should be in source, not header, to avoid link errors if constructed from outside OgreMain (this fixes tests build on OS X)
Fix some new/deletes in tests that should have been OGRE_NEW/OGRE_DELETE</li>
<li> Mark a few more options as 'Advanced' to de-clutter the main CMake page.
Disabling zip, freeimage, dds etc is definitely an advanced function.</li>
<li> Patch 2981322 - allow OGRE_LIB_DIRECTORY to be set to install in alternate named directories such as lib64 for 64-bit Linuxes that use that convention.</li>
<li> Fix for VC10</li>
<li> Patch 2986452: avoid potential resource group change deadlocks</li>
<li> Patch 2986446: For ACT_LIGHT_POSITION_OBJECT_SPACE for directional lights, we need to use the inverse of the inverse transpose as with ACT_LIGHT_DIRECTION_OBJECT_SPACE, to deal with non-uniform scalings</li>
<li> Patch 2986441: MovableObject's queryLights and _notifyCurrentCamera didn't take account of object scaling when dealing with bounding radius</li>
<li> Patch 2986437: auto-tracking update should be after the re-entrant call to update shadow textures, otherwise old tracking state may be used</li>
<li> Fixed BillboardChain::getNumChainElements when tail < head</li>
<li> Fix loading of mesh versions 1.30 and before
They were picking up LOD loading from latest version instead of fallback in MeshSerializerImpl_v1_4 because of inheritance error</li>
<li> In Mesh::destroySubMesh, fix up SubMesh name map for the removed item</li>
<li> If group name on manual LOD level is blank, use parent mesh</li>
<li> Make PlayPen plugin work on OS X</li>
<li> Fix bug 308 according to user report - don't include _mingw.h in more recent MinGW packages</li>
<li> Fix bug 313: doc build target doesn't handle spaces in directry names</li>
<li> Fix terrain rendering on ATI/GL - no support for fp30/fp40 in latest ATI drivers but arbfp1 works</li>
<li> Terrain morphing was missing from shader when alignment wasn't x/z</li>
<li> Deal with CMAKE_OSX_ARCHITECTURES a little better:
- Selectively remove 64-bit archs (Carbon requirement) instead of manually setting "i386 ppc", so that user can still customise archs for 10.6
- Set the default arch if none is specified (Cmake 2.8.0)</li>
<li> Patch 2977286: Fix support for scene blending operations other than SBO_ADD on Dx9 because caps were not being set</li>
<li> Patch 2979431: Matrix4::makeInverseTransform bug with non-uniform scale</li>
<li> Patch 2978035: D3D9 should send a "DeviceLost" message to app-side listeners before a device is reset</li>
<li> Deferred Shading Demo : Fixed 'is camera inside light' test for spotlights.</li>
<li> Fix problem with picking the right shader generator in TerrainMaterialGeneratorA
- Would not fall back from Cg when Cg not available (bad nesting)
- Can only use ps_2_0 when not parallax mapping</li>
<li> Fix memory leak in SharedPtr when initialised with a null pointer</li>
<li> Ensure libdl exists before linking to it</li>
<li> Fix install of tools PDBs in debug mode</li>
<li> Add test for switching full screen modes without reinitialisation</li>
<li> (Fixed commit) Applied l3fthn4d's compositor (material_scheme & shadows settings) patch as discussed in http://www.ogre3d.org/forums/viewtopic.php?f=4&t=56031</li>
<li> Fix bug 305: Entity::setMaterialName(material, group) calls SubEntity::setMaterialName(material) without group.</li>
<li> Add the rtshader materials directory to resources.cfg</li>
<li> Correct case-sensitivity issue with samples media path</li>
<li> Remove nested parenthesis, not supported on CMake 2.6 (only 2.8)</li>
<li> Explicitly include errno.h.  Otherwise build errors occur if not using boost and threading is turned off.</li>
<li> Patch 2971821: Fix consistency of inclusions of OctreeSceneManager plugin (important when referenced with relative paths)</li>
<li> Patch 2971818: deal with missing MinGW header & make inclusion specific to MinGW</li>
<li> Just make sure meshes are being reloaded in LOD test</li>
<li> Added tests for manual LOD, seems to work</li>
<li> Explicitly note in the manual some directives which are irrelevant when using shaders</li>
<li> Another bad svn:eol-style property</li>
<li> Fix some bad svn:eol-style properties</li>
<li> More Xcode template work.  Cleaned up warnings.  Changed iPhone template to use a NSTimer to make it more reliable at high framerates.  Fixed the installer path.</li>
<li> GLES 1.x patch: fix opengl es 1.1 compilation under windows - ID: 2961527. Thanks Thomas.
http://sourceforge.net/tracker/?func=detail&atid=302997&aid=2961527&group_id=2997</li>
<li> GLES 1.x render system: Replaced a printf with a log entry.</li>
<li> RTSS: Extended the normal map texture unit settings script parsing caps.
Now it can read the filtering type, max anisotropy and mip map bias.</li>
<li> Patch 2965714: CMake Infinite Loop Fix for OS X using makefiles instead of XCode</li>
<li> Patch 2968889: fix ExampleApplication loading config files in debug mode</li>
<li> Fixed some iPhone remnants of the OGRE_CONFIG_DISABLE to OGRE_CONFIG_ENABLE change</li>
<li> Implement missing wireframe and points rendering in GL ES.  Also fixed a typo in a comment.</li>
<li> Doc fix</li>
<li> Fix copy & paste error that could cause crashes when unloading & reinitialising terrain</li>
<li> Fix a couple bugs in the Xcode templates.  Added an option to choose separate locations for OS X and iPhone SDK's.  Cleaned up the installer package a little bit.</li>
<li> Patch 2963406: Deal with negative scale in Entity::getBoundingRadius</li>
<li> Patch 2963372: Matrix4::makeTransform/makeInverseTransform inefficiencies fixed
Reduces complexity of this from 39MUL+30ADD to 18MUL+15ADD</li>
<li> Must set the OGRE_MEDIA_DIR_* to an absolute path for build folder versions</li>
<li> Patch 2959565: fix XSI exporter build</li>
<li> Fix bug 264: problem with Camera::setDirection when using non-fixed yaw and a non-identity parent node.</li>
<li> Fix bug 299: Sequence of light changes can cause crash because of light hash==0</li>
<li> Make sure linking or copying media works in non-SDK builds for OS X and iPhone</li>
<li> Mention that the default material generator requires the Cg plugin</li>
<li> Fix water sample when exiting & reentering it by making sure mesh is removed from MeshManager</li>
<li> Update Xcode templates for 1.7 final release</li>
<li> Profiler: endProfile checks group id against mask.</li>
<li> GpuProgram: Added init in the contractor to mMorphAnimation and mPoseAnimation members.</li>
<li> Fix bug 297: Incorrect use of Boost_USE_STATIC_LIBS causes boost library search to fail when trying to switch to Boost dynamic libs</li>
<li> Changes for building SDK for iPhone</li>
<li> Should delete .exe not .7z when building SDK multiple times</li>
<li> D3D9 Render System: Device multi thread flag is used now only when OGRE_THREAD_SUPPORT == 1. This will save us a few cycles in the other threading modes..</li>
<li> FindOIS.cmake fixes:
- Respect OGRE_DEPENDENCIES_DIR (useful if used outside of OGRE)
- On Win32 define names of the OIS DLLs so they can be copied from wherever they are found</li>
<li> FindOGRE.cmake fixes:
- Use OGRE_DEPENDENCIES_DIR if defined (external dependencies)
- Find media in the OGRE_SOURCE folder correctly
- Use a zip file common to 1.6 and 1.7 to detect media (OgreCore.zip doesn't exist anymore in 1.7+)</li>
<li> Compile fix for people using ExampleFrameListener</li>
<li> RTSS: Fixed Normal map point and spotlight GPU parameters that weren't update correctly.</li>
<li> Minor tweaks to the demos installer
- Nicer defaults
- License display (forced to be RTF unfortunately)</li>
<li> Enable WiX installer interface and allow the user to specify the install directory for the demos
Fix media installing
Add missing samples.cfg</li>
<li> Fixed remaining bugs for MinGW SDK and finished mingwsdk.sh</li>
<li> Fix shortcut errors in WiX setup</li>
<li> Generate WiX config files and custom task for building a demo installer on MSVC
Remove space in OGRE_VERSION before suffix, more trouble than it's worth even for display strings
demomedia.wxi.in was generated using 'heat dir Media -gg -cg OgreMedia -out demomedia.wxi.in -sfrag' on a clean Media export, then replacing SourceDir with Cmake variable</li>
<li> Fixes and enhancements for MinGW SDK generation.</li>
<li> Merged v9841 into tag since urgent change</li>
<li> Merged v9841 into tag since urgent change</li>
<li> In future, use the Windows-mode 7zip self-extractor not the default console-mode one</li>
<li> Final changes for the OS X SDK.  Fixes up all the absolute paths in CMake generated files.</li>
<li> OGRE_VERSION has a whitespace and version suffix appended; this breaks builds if used as the .so version for shared libraries. Use a dedicated OGRE_SOVERSION instead.</li>
<li> RTSS: Fixed Normal map and PSSM3 GPU parameters that weren't update on object switch scenario.</li>

</ul>

## v1.7.0 [Cthugha] (28 February 2010) - MAJOR RELEASE
<a href="#changessince1.7.0RC1">[View changes since RC1]</a>

<ul><li> <b>License changes</b>
<ul><li>OGRE 1.7 is now released under the <a href="http://www.opensource.org/licenses/mit-license.php">MIT License</a></li></ul>
</li><li> <b>Compositor changes</b>
<ul><li> Allow 'pooled' compositor surfaces.
        <ul><li> Separate compositor instances using the same size &amp; format surfaces can share them, saving memory
            </li><li> System detects the compositor instance chaining to avoid problems with interdependence
            </li><li> 'pooled' has to be explicitly enabled on texture definitions, it is not the default because once enabled you cannot necessarily rely on being able to see all the intermediate texture results (because they can be ping-ponging across shared textures); people may be relying on this
            </li></ul>
    </li><li> Manual switching between supported compositor techniques is now possible on the fly
        <ul><li> Compositor techniques can now have 'scheme' names, you can manually switch between supported techniques using the scheme name to drop to different approaches for reasons other than hardware compatibility (performance, alternative tweaks etc)
            </li><li> You can keep &amp; share the textures used by previously active techniques so switching back &amp; forth is faster (must use 'pooled' option for this).
            </li></ul>
    </li><li> Compositors can now specify if they don't want to inherit the FSAA setting from the main target for texture definitions ('no_fsaa')
    </li><li> Compositors can now turn on sRGB gamma conversion per texture definition instead of just inheriting from the main surface ('gamma')
    </li><li> Cross-compositor communication (from <a href="http://www.ogre3d.org/wiki/index.php/SoC2009_Compositor" title="SoC2009 Compositor">SoC2009 Compositor</a>)
        <ul><li> Define a texture as accessible from other locations by the chain_scope or global_scope directive in the texture definition
            </li><li> Reference a texture from a different compositor in the chain (or in general) using the texture_ref directive
            </li></ul>
    </li><li> Compositor &lt;-&gt; code connection improvements (from <a href="http://www.ogre3d.org/wiki/index.php/SoC2009_Compositor" title="SoC2009 Compositor">SoC2009 Compositor</a>)
        <ul><li> It is now possible to define a custom composition pass (instead of just quad/scene/clear) in code and trigger it using the render_custom composition pass type.
            </li><li> You can now link between a compositor and related code (for example, a compositor listener) automatically using CompositorLogics.
            </li></ul>
    </li></ul>
</li><li> <b>SceneManager changes</b> (from <a href="http://www.ogre3d.org/wiki/index.php/SoC2009_Compositor" title="SoC2009 Compositor">SoC2009 Compositor</a>)

<ul><li> It is now possible to pause a frame's rendering mid-way (for example, during a callback called in the middle of the process) to trigger a side render and then resume rendering. See SceneManager::_pauseFrame and SceneManager::_resumeFrame.
    </li><li> Added an option to manually trigger the updating of shadow textures for specific lights. See SceneManager::prepareShadowTextures. The combination of the two new additions allow reusing a single texture for multiple shadowmaps.
    </li></ul>
</li><li> <b>New Sample Browser</b> (from <a href="http://www.ogre3d.org/wiki/index.php/SoC2009_Samples">SoC2009_Samples</a>)
<ul><li> Instead of many separate demos, we now have one integrated sample browser
    </li><li> Samples are pluggable libraries that can be reloaded at runtime without restarting the browser
    </li><li> A simple 'tray' system is used to make sample GUI controls easy to create
    </li></ul>
</li><li> <b>Antialising changes</b>
<ul><li> Support for Coverage Sampled AA (CSAA) - Dx9 &amp; Dx10 only for now
    </li><li> Unified &amp; simplified AA settings
        <ul><li> on Root's config options the setting is now called 'FSAA' in all cases, and consist of samples and a hint string (separated by spaces)
            </li><li> on the miscParams to createRenderWindow you can supply 'FSAA' and 'FSAAHint'. The former is the number of samples, the latter any hinting (e.g. 'Quality')
            </li></ul>
    </li></ul>
</li><li> <b>Light changes</b>
<ul><li> Near/far plane settings for shadow cameras can now be manually configured per light if required
    </li><li> You can now mask lights out per object by calling <i>MovableObject::setLightMask</i> - a renderable object's mask is bitwise and'ed with the light's mask and the light is excluded if the result is 0.
    </li></ul>
</li><li> <b>LOD changes</b> (from <a href="http://www.ogre3d.org/wiki/index.php/SoC2008_LOD">Soc2008_LOD</a>)

<ul><li> LOD no longer has to use distance as a metric for changing LOD levels
    </li><li> LodStrategy can now be set on both Material and Mesh, to either Distance or PixelCount (new strategies can also be added)
    </li></ul>
</li><li> <b>STL container changes</b>
<ul><li> All STL containers now use custom memory allocators
    </li></ul>
</li><li> <b>Profiler changes</b>
<ul><li> Allow milliseconds as well as percentage view - gives a better idea of absolute fluctuations.
    </li><li> Define profiler masks so that profiling can be added to core OGRE but still filtered out by categories (added some simple profiling to test)
    </li><li> Hierarchy of profiles is now inclusive instead of exclusive (children no longer subtract their time from parents). This is more useful in practice when doing breakdowns
    </li><li> Added numerical indicators instead of a scale with 'ticks' since its more useful
    </li></ul>
</li><li> <b>Optimisations</b>
<ul><li> Fixed-function light state is now handled more intelligently, leading to better performance with large numbers of objects
    </li><li> Shader parameters are now updated more selectively, reducing unnecessary updates
    </li></ul>
</li><li> <b>GpuProgramParameters changes</b>

<ul><li> Support added for shared parameter sets which allow you to define / update shader variables for many programs and materials in one place. See <i>GpuProgramManager::createSharedParamerers</i>
    </li><li> Use shared_params, shared_param_named and shared_params_ref in scripts to define and reference shared parameter sets.
    </li><li> Parameters are now automatically migrated when the program they are based on is changed and reloaded; any parameters which still apply are merged into the new parameters
    </li></ul>
</li><li> <b>Archive changes</b>
<ul><li> Archive now supports create and remove of files (FileSystem only implemented for now)
    </li></ul>
</li><li> <b>DataStream changes</b>

<ul><li> Writeable data streams are now supported (FileSystem only implemented for now)
    </li></ul>
</li><li> <b>File handling changes</b>
<ul><li> New class StreamSerialiser is the new way to read &amp; write binary chunk-based formats
    </li></ul>
</li><li> <b>Build changes</b>
<ul><li> <a href="http://www.cmake.org" class="external text" title="http://www.cmake.org" rel="nofollow">Cmake</a> is now used to generate project files, separate explicitly maintained build systems are being removed. See <a href="http://www.ogre3d.org/wiki/index.php/Building_With_CMake" title="Building With CMake">Building With CMake</a>
    </li></ul>
</li><li> <b>RenderWindow changes</b>
<ul><li> miscParams now supports 'vsyncInterval' option, allowing you to sync to a multiple of the refresh rate if you want (and the hardware supports it)
    </li></ul>
</li><li> <b>Viewport changes</b>
<ul><li> Added clear method to manually clear any combination of colour/depth/stencil to a specified value without performing an update.
    </li></ul>
</li><li> <b>Image changes</b>
<ul><li> Added loadTwoImagesAsRGBA and combineTwoImagesAsRGBA to make it easier to construct combined normal/height and diffuse/specular images etc
    </li></ul>
</li><li> <b>New Paging Component</b>
<ul><li> SceneManager-independent, separate optional component (OGRE_HOME/Components/Paging)
    </li><li> Pluggable strategy components to control paging strategy for a section of the scene
    </li><li> Pluggable content components to control paging of content
    </li><li> Plugggable collection components so that paged elements can be composed or selected between in different ways (e.g. paging many LOD levels within a page)
    </li></ul>
</li><li> <b>New Terrain Component</b>
<ul><li> SceneManager-independent, separate optional component (OGRE_HOME/Components/Terrain)
    </li><li> Inherently editable
    </li><li> Hierarchical geometry batching; batch counts reduce at lower LODs as well as vertex count. At the lowest level of detail, the entire terrain page is a single batch.
    </li><li> LOD now adapts in real-time to camera settings (viewport sizes &amp; LOD bias) so you can use the same terrain with multiple views efficiently
    </li><li> Skirts are used instead of stitching to avoid cracks in geometry; this means fewer indexing arrangements &amp; lower overall index buffer usage
    </li><li> Saving &amp; loading of terrain built in, including loading / processing in a background thread
    </li><li> In-built support for splatting layers, configurable sampler inputs and pluggable material generators
    </li><li> Support for generating global normal maps and light maps, in a background thread
    </li></ul>
</li><li> <b>New 'Real Time Shader System' (RTSS) Component</b>
<ul><li> Automatically generate shaders to replace the fixed-function pipeline and to add features such as per-pixel lighting, normal mapping and shadows.
    </li></ul>
</li><li> <b>New Property Component</b>
<ul><li> Separate optional component (OGRE_HOME/Components/Property)
    </li><li> boost::bind based property system to make it easier to expose reflected properties from your objects
    </li></ul>
</li><li> <b>Threading changes</b>
<ul><li> WorkQueue added to accept generalised work items to be executed in multiple background worker threads
    </li><li> WorkQueue starts the number of workers based on hardware, or can be told to start a different number
    </li><li> Main Ogre WorkQueue is in Root::getWorkQueue. You can also subclass WorkQueue and provide your own if you want
    </li><li> ResourceBackgroundQueue now uses WorkQueue instead of using its own queue and can have multiple tasks running at once
    </li><li> New focus on data-driven, task-based parallel execution with separation of GPU and CPU activities
    </li><li> Boost, POCO and Thread Building Blocks supported as threading back-ends (Boost preferred)
    </li></ul>
</li><li> <b>Resource changes</b>
<ul><li> ResourcePool added as a place for other application components to shelve &amp; re-use resources
    </li></ul>
</li><li> <b>Material changes</b>

<ul><li> scene_blend_op and separate_scene_blend_op added to passes, to change the default '+' operator between source &amp; dest blending params
    </li><li> Material listeners can now register to listen to a specific scheme, to allow for none-competing scheme handlers for different schemes.
    </li></ul>
</li><li> <b>iPhone OS port</b>
<ul><li> Synced GLESRenderSystem with GLRenderSystem and added some more extension detection
    </li><li> Added support for ARM architecture and CPU feature detection
    </li><li> iPhone OGRE SDK disc image and sdk package script
    </li></ul>

</li><li> <b>OS X improvements</b>
<ul><li> Moved code and resources from the Mac directory to OgreMain
    </li><li> Eliminated deprecation warnings when targeting Mac OS X 10.5/10.6
    </li><li> General organisation and compatibility improvements
    </li></ul>
</li></ul>

<a name="changessince1.7.0RC1">Changes since 1.7.0 RC1 (highlights only)</a>
<ul>
    <li>Fix GPU-extruded stencil shadows on Dx9</li>
    <li>Allow Terrain to have layers inserted / removed at any point in the stack</li>
    <li>Fix bug 254: DXTn volume texture size calculation crash</li>
    <li>Samples tidied up, and some fixes to unloading behaviour in Compositor, Instancing and VolumeTex samples</li>
    <li>Many fixes to FindOGRE.cmake, now much more reliable and compatible with 1.6 and 1.7. Reference SDKs with OGRE_SDK, or source builds via OGRE_SOURCE and OGRE_BUILD paths.</li>
    <li>New SDKs for Windows and OS X created</li>
    <li>Fill in some missed PVRTC rendersystem caps</li>
    <li>Fix resolving some types in OS X 10.5 SDK</li>
    <li>Added validateConfigOptions() call in Root::restoreConfig(). Solves a crash when moving cfg file from one machine to another.</li>
    <li>RTSS: some refactoring and bug fixing</li>
    <li>Terrain: fixed several bugs</li>
    <li>GLES rendersystem bugfixes</li>
    <li>Fix Codec compile when using OGRE_STRING_USE_CUSTOM_MEMORY_ALLOCATOR</li>
    <li>HashedVector::mListHashDirty was not initialised correctly in a few cases</li>
    <li>Null shadow textures should not be dynamic since that means after a device lost they lost their contents, and there is no handler to manage this. </li>
    <li>Prevent excessive logging when texture load failed in ensureloaded/ensurePrepared</li>
    <li>Windows 64-bit compatibility fixes</li>
    <li>Fix problems when using logical GPU parameter indexes which end up needing more space than originally thought</li>
    <li>Fix a problem with .hdr introduced by new FreeImage</li>
    <li>Fix a bug in ResourceGroupManager::loadResourceGroup when a resource changes group</li>
    <li>Added a Character sample featuring the Sinbad character model.</li>
    <li>Add CMake option corresponding to OGRE_PROFILING</li>
    <li>Fix a large number of problems on Windows with establishing the max size of a window and switching between fullscreen and windowed modes</li>
    <li>WorkQueue::addRequestHandler and WorkQueue::removeRequestHandler no longer block until existing background requests are processed, leading to better parallelism</li>
    <li>Fix no grass displayed on ATI cards on GL in grass demo</li>
    <li>Static build now works including all sample browser plugins</li>
    <li>Fix RGBA colours in custom GLSL attributes</li>
    <li>Reduce compiler warnings</li>
    <li>Lots &amp; lots of other bugfixes and tweaks!</li>
</ul>
