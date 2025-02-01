#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

######################################################################
# OGRE BUILD SYSTEM
# Welcome to the CMake build system for OGRE.
# This is the main file where we prepare the general build environment
# and provide build configuration options.
######################################################################

cmake_minimum_required(VERSION 3.13.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_VS_PLATFORM_NAME STREQUAL "Tegra-Android")
	set(CMAKE_LINK_LIBRARY_FLAG "")
endif()

# Use relative paths
# This is mostly to reduce path size for command-line limits on windows
if(WIN32)
  # This seems to break Xcode projects so definitely don't enable on Apple builds
  set(CMAKE_USE_RELATIVE_PATHS true)
  set(CMAKE_SUPPRESS_REGENERATION true)
endif()

# prefer GLVND
if(POLICY CMP0072)
  cmake_policy(SET CMP0072 NEW)
endif()
if(POLICY CMP0078)
  cmake_policy(SET CMP0078 NEW)
endif()
if(POLICY CMP0086)
  cmake_policy(SET CMP0086 NEW)
endif()

if (APPLE AND NOT ANDROID AND NOT EMSCRIPTEN AND NOT ANDROID_PLATFORM)
  SET(CMAKE_SIZEOF_VOID_P 4)

  if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16.0")
    enable_language(OBJC)
    enable_language(OBJCXX)
  endif()

  if(POLICY CMP0068)
    cmake_policy(SET CMP0068 NEW)  # CMake 3.9+: `RPATH` settings on macOS do not affect `install_name`.
  endif()

  if (NOT CMAKE_OSX_ARCHITECTURES)
    set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "") # target the common case
  endif()

  # otherwise apple defines a macro named check that conflicts with boost
  add_definitions(-D__ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES=0)
endif ()

project(OGRE VERSION 14.3.4)

# extra version info
set(OGRE_VERSION_SUFFIX "")
set(OGRE_VERSION_NAME "Tsathoggua")

# legacy defines
set(OGRE_SOVERSION ${OGRE_VERSION_MAJOR}.${OGRE_VERSION_MINOR})
set(OGRE_VERSION_DASH_SEPARATED "${OGRE_VERSION_MAJOR}-${OGRE_VERSION_MINOR}-${OGRE_VERSION_PATCH}")


# Include necessary submodules
list(APPEND CMAKE_MODULE_PATH
  "${PROJECT_SOURCE_DIR}/CMake"
  "${PROJECT_SOURCE_DIR}/CMake/Utils"
  "${PROJECT_SOURCE_DIR}/CMake/Packages"
)

include(CMakeDependentOption)
include(MacroLogFeature)
include(OgreConfigTargets)
set(OGRE_TEMPLATES_DIR "${PROJECT_SOURCE_DIR}/CMake/Templates")
set(OGRE_WORK_DIR ${PROJECT_BINARY_DIR})


#####################################################################
# Set up the basic build environment
#####################################################################

if (NOT CMAKE_BUILD_TYPE)
  # CMake defaults to leaving CMAKE_BUILD_TYPE empty. This screws up
  # differentiation between debug and release builds.
  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build, options are: None (CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif ()

# determine Ogre version numbers
message(STATUS "Configuring OGRE ${OGRE_VERSION}")
# Configure version file for use by scripts
configure_file("${OGRE_TEMPLATES_DIR}/version.txt.in" "${PROJECT_BINARY_DIR}/version.txt" @ONLY)

# determine if we are compiling for a 32bit or 64bit system
include(CheckTypeSize)
CHECK_TYPE_SIZE("void*" OGRE_PTR_SIZE BUILTIN_TYPES_ONLY)
if (OGRE_PTR_SIZE EQUAL 8)
  set(OGRE_PLATFORM_X64 TRUE)
else ()
  set(OGRE_PLATFORM_X64 FALSE)
endif ()

if (WIN32)
  # Create debug libraries with _d postfix
  set(CMAKE_DEBUG_POSTFIX "_d")
endif ()

# Set compiler specific build flags
if (NOT ANDROID AND NOT EMSCRIPTEN AND UNIX OR MINGW)
  if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86|X86|amd64|AMD64")
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag(-msse OGRE_GCC_HAS_SSE)
    if (OGRE_GCC_HAS_SSE)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse")
    endif ()
  endif()
endif()

if(UNIX)
  # This is a set of sensible warnings that provide meaningful output
  set(OGRE_WARNING_FLAGS "-Wall -Winit-self -Wcast-qual -Wwrite-strings -Wextra -Wundef -Wmissing-declarations -Wno-unused-parameter -Wshadow -Wno-missing-field-initializers -Wno-long-long")
  if (EMSCRIPTEN)
      set(OGRE_WARNING_FLAGS "${OGRE_WARNING_FLAGS} -Wno-warn-absolute-paths")
  endif ()
  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(OGRE_WARNING_FLAGS "${OGRE_WARNING_FLAGS} -Wno-missing-braces")
  else()
    set(OGRE_WARNING_FLAGS "${OGRE_WARNING_FLAGS} -Wsuggest-override")
  endif()
  set(CMAKE_CXX_FLAGS "${OGRE_WARNING_FLAGS} ${CMAKE_CXX_FLAGS}")
endif ()
if (MSVC)
  add_definitions(-D_MT -D_USRDLL)
  # MSVC does not like Ogre::Singleton (header pragma is enough for MSVC2015+ though)
  add_definitions(/wd4661)
  if (CMAKE_BUILD_TOOL STREQUAL "nmake")
    # set variable to state that we are using nmake makefiles
	set(NMAKE TRUE)
  endif ()

  # Enable intrinsics on MSVC in debug mode
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Oi")
  if (MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 8)
    # Visual Studio bails out on debug builds in 64bit mode unless
	# this flag is set...
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /bigobj")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /bigobj")
  endif ()

  if (OGRE_PROJECT_FOLDERS)  
    # Turn on the ability to create folders to organize projects (.vcproj)
    # It creates "CMakePredefinedTargets" folder by default and adds CMake
    # defined projects like INSTALL.vcproj and ZERO_CHECK.vcproj
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
  endif()

  option(OGRE_BUILD_MSVC_MP "Enable build with multiple processes in Visual Studio" TRUE)
  if(OGRE_BUILD_MSVC_MP)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  endif ()
  option(OGRE_BUILD_MSVC_ZM "Add /Zm256 compiler option to fix PCH errors on Visual Studio 2013" TRUE)
  if(OGRE_BUILD_MSVC_ZM)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zm256")
  endif ()
endif ()
if (MINGW)
  add_definitions(-D_WIN32_WINNT=0x0501)
  if( CMAKE_SIZEOF_VOID_P EQUAL 4 )
    # set architecture to i686, since otherwise some versions of MinGW can't link
    # the atomic primitives
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=i686")
  endif ()
endif ()

include(GenerateExportHeader)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

if((CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU") AND NOT OGRE_STATIC)
  set(OGRE_VISIBILITY_FLAGS "-DOGRE_GCC_VISIBILITY") # for legacy headers
endif()

# determine system endianness
if (MSVC OR ANDROID OR EMSCRIPTEN OR APPLE_IOS)
  # This doesn't work on VS 2010
  # MSVC only builds for intel anyway
  # all IOS devices are little endian
  set(OGRE_TEST_BIG_ENDIAN FALSE)
else()
  include(TestBigEndian)
  test_big_endian(OGRE_TEST_BIG_ENDIAN)
endif()

# definitions for samples
set(OGRE_LIBRARIES OgreMain)

# Specify build paths
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
if (WIN32 OR APPLE)
  if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    # We don't want to install in default system location, install is really for the SDK, so call it that
    set(CMAKE_INSTALL_PREFIX
	  "${PROJECT_BINARY_DIR}/sdk" CACHE PATH "OGRE install prefix" FORCE
    )
  endif (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
endif(WIN32 OR APPLE)

# Set up iOS overrides.
if (APPLE_IOS)
  set(CMAKE_EXE_LINKER_FLAGS "-framework OpenGLES -framework Foundation -framework CoreGraphics -framework QuartzCore -framework UIKit")
  set(XCODE_ATTRIBUTE_GCC_UNROLL_LOOPS "YES")
  set(XCODE_ATTRIBUTE_LLVM_VECTORIZE_LOOPS "YES")
  set(XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer")
  set(XCODE_ATTRIBUTE_GCC_PRECOMPILE_PREFIX_HEADER "YES")
  set(OGRE_BUILD_RENDERSYSTEM_GLES2 TRUE CACHE BOOL "Forcing OpenGL ES 2 RenderSystem for iOS" FORCE)
  set(OGRE_STATIC TRUE CACHE BOOL "Forcing static build for iOS" FORCE)
  set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.yourcompany.\${PRODUCT_NAME:rfc1034identifier}")
elseif (ANDROID)
  set(TargetPlatform "Android")
  set(OGRE_PLATFORM OGRE_PLATFORM_ANDROID)
  option(OGRE_BUILD_ANDROID_JNI_SAMPLE "Enable JNI Sample" FALSE)

  set(OGRE_BUILD_RENDERSYSTEM_GLES2 TRUE CACHE BOOL "Forcing OpenGL ES 2 RenderSystem for Android" FORCE)

  set(OGRE_BUILD_PLUGIN_PCZ FALSE CACHE BOOL "Disable pcz on Android" FORCE)
  set(OGRE_BUILD_TOOLS FALSE CACHE BOOL "Disable tools on Android" FORCE)
  set(OGRE_STATIC TRUE CACHE BOOL "Forcing static build for Android" FORCE)

  # workaround for the legacy android toolchain
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(EMSCRIPTEN)
  add_definitions(-DEMSCRIPTEN=1 -D__EMSCRIPTEN__=1)
  set(TargetPlatform "Emscripten")
  set(OGRE_PLATFORM OGRE_PLATFORM_EMSCRIPTEN)

  set(OGRE_BUILD_RENDERSYSTEM_GLES2 TRUE CACHE BOOL "Forcing OpenGL ES 2 RenderSystem for Emscripten" FORCE)
  set(OGRE_BUILD_RENDERSYSTEM_GL FALSE CACHE BOOL "Disable OpenGL RenderSystem for Emscripten" FORCE)

  set(OGRE_BUILD_PLUGIN_STBI TRUE CACHE BOOL "Enable STBIImageCodec on Emscripten (Smaller Footprint)" FORCE)
  set(OGRE_BUILD_PLUGIN_FREEIMAGE FALSE CACHE BOOL "Disable Freeimage on Emscripten (Smaller Footprint)" FORCE)
  set(OGRE_BUILD_PLUGIN_PCZ FALSE CACHE BOOL "Disable pcz on Emscripten" FORCE)
  set(OGRE_BUILD_PLUGIN_BSP FALSE CACHE BOOL "Disable pcz on Emscripten" FORCE)
  set(OGRE_BUILD_TOOLS FALSE CACHE BOOL "Disable tools on Emscripten" FORCE)
  set(OGRE_BUILD_TESTS FALSE CACHE BOOL "Disable tests on Emscripten" FORCE)
  set(OGRE_BUILD_COMPONENT_VOLUME FALSE CACHE BOOL "Disable volume component on Emscripten" FORCE)
  set(OGRE_BUILD_COMPONENT_PAGING FALSE CACHE BOOL "Disable paging component on Emscripten" FORCE)
  set(OGRE_BUILD_COMPONENT_TERRAIN FALSE CACHE BOOL "Disable terrain component on Emscripten" FORCE)
  set(OGRE_STATIC TRUE CACHE BOOL "Forcing static build for Emscripten" FORCE)

  set(OGRE_CONFIG_THREADS "0" CACHE STRING "Threading is unstable on Emscripten" FORCE)
elseif (APPLE AND NOT APPLE_IOS)
  # Make sure that the GLES2 render system is disabled for non-iOS Apple builds
  set(OGRE_BUILD_RENDERSYSTEM_GLES2 FALSE CACHE BOOL "" FORCE)
endif ()

if(OGRE_BUILD_COMPONENT_MESHLODGENERATOR)
  set(OGRE_CONFIG_ENABLE_MESHLOD TRUE CACHE BOOL "Forcing Mesh Lod" FORCE)
endif()

# Find dependencies
include(Dependencies)

######################################################################
# Provide user options to customise the build process
######################################################################

# Customise what to build
option(OGRE_STATIC "Static build" FALSE)
cmake_dependent_option(OGRE_ENABLE_PRECOMPILED_HEADERS "Use precompiled headers to speed up build" TRUE "NOT MINGW; NOT EMSCRIPTEN" FALSE)
set(OGRE_RESOURCEMANAGER_STRICT "2" CACHE STRING 
  "Make ResourceManager strict for faster operation. Possible values:
  0 - OFF search in all groups twice - for case sensitive and insensitive lookup [DEPRECATED]
  1 - PEDANTIC require an explicit resource group. Case sensitive lookup.
  2 - STRICT search in default group if not specified otherwise. Case sensitive lookup.
  ")
set_property(CACHE OGRE_RESOURCEMANAGER_STRICT PROPERTY STRINGS 0 1 2)

option(OGRE_NODELESS_POSITIONING "use Lights and Cameras without attaching them to nodes [DEPRECATED]" FALSE)

cmake_dependent_option(OGRE_BUILD_RENDERSYSTEM_D3D9 "Build Direct3D9 RenderSystem" TRUE "WIN32;DirectX9_FOUND;NOT WINDOWS_STORE;NOT WINDOWS_PHONE" FALSE)
cmake_dependent_option(OGRE_BUILD_RENDERSYSTEM_D3D11 "Build Direct3D11 RenderSystem" TRUE "WIN32;DirectX11_FOUND OR WINDOWS_STORE OR WINDOWS_PHONE" FALSE)
cmake_dependent_option(OGRE_BUILD_RENDERSYSTEM_GL3PLUS "Build OpenGL 3+ RenderSystem" TRUE "OPENGL_FOUND;NOT WINDOWS_STORE;NOT WINDOWS_PHONE" FALSE)
cmake_dependent_option(OGRE_BUILD_RENDERSYSTEM_GL "Build OpenGL RenderSystem" TRUE "OPENGL_FOUND;NOT APPLE_IOS;NOT WINDOWS_STORE;NOT WINDOWS_PHONE" FALSE)
cmake_dependent_option(OGRE_BUILD_RENDERSYSTEM_GLES2 "Build OpenGL ES 2.x RenderSystem" TRUE "OPENGLES2_FOUND;NOT WINDOWS_STORE;NOT WINDOWS_PHONE" FALSE)
cmake_dependent_option(OGRE_BUILD_RENDERSYSTEM_METAL "Build Metal RenderSystem" FALSE "APPLE" FALSE)
option(OGRE_BUILD_RENDERSYSTEM_VULKAN "Build Vulkan RenderSystem" FALSE)
cmake_dependent_option(OGRE_BUILD_PLUGIN_ASSIMP "Build Open Asset Import plugin" TRUE "assimp_FOUND" FALSE)
cmake_dependent_option(OGRE_BUILD_RENDERSYSTEM_TINY "Build Tiny RenderSystem (software-rendering)" FALSE "NOT ANDROID" FALSE)
option(OGRE_BUILD_PLUGIN_BSP "Build BSP SceneManager plugin" TRUE)
option(OGRE_BUILD_PLUGIN_OCTREE "Build Octree SceneManager plugin" TRUE)
option(OGRE_BUILD_PLUGIN_PFX "Build ParticleFX plugin" TRUE)
cmake_dependent_option(OGRE_BUILD_PLUGIN_DOT_SCENE "Build .scene plugin" TRUE "pugixml_FOUND" FALSE)
cmake_dependent_option(OGRE_BUILD_PLUGIN_PCZ "Build PCZ SceneManager plugin" TRUE "" FALSE)
cmake_dependent_option(OGRE_BUILD_COMPONENT_PAGING "Build Paging component" TRUE "" FALSE)
cmake_dependent_option(OGRE_BUILD_COMPONENT_MESHLODGENERATOR "Build MeshLodGenerator component" TRUE "" FALSE)
cmake_dependent_option(OGRE_BUILD_COMPONENT_TERRAIN "Build Terrain component" TRUE "" FALSE)
cmake_dependent_option(OGRE_BUILD_COMPONENT_VOLUME "Build Volume component" TRUE "" FALSE)
cmake_dependent_option(OGRE_BUILD_COMPONENT_PROPERTY "Build Property component" TRUE "" FALSE)
cmake_dependent_option(OGRE_BUILD_PLUGIN_CG "Build Cg plugin" TRUE "Cg_FOUND;NOT APPLE_IOS;NOT WINDOWS_STORE;NOT WINDOWS_PHONE" FALSE)
option(OGRE_BUILD_PLUGIN_GLSLANG "Build glslang plugin" ${OGRE_BUILD_RENDERSYSTEM_VULKAN}) # no way to determine whether glslang is present
option(OGRE_BUILD_COMPONENT_OVERLAY "Build Overlay component" TRUE)

cmake_dependent_option(OGRE_BUILD_COMPONENT_OVERLAY_IMGUI "Include dear imgui in Overlays" TRUE "OGRE_BUILD_COMPONENT_OVERLAY" FALSE)

cmake_dependent_option(OGRE_BUILD_COMPONENT_BITES "Build OgreBites component" TRUE "OGRE_BUILD_COMPONENT_OVERLAY" FALSE)
cmake_dependent_option(OGRE_BUILD_COMPONENT_BULLET "Build Bullet physics component" TRUE "BULLET_FOUND" FALSE)
cmake_dependent_option(OGRE_BITES_STATIC_PLUGINS "Skip plugins.cfg and statically load plugins via OgreBites" FALSE "NOT OGRE_STATIC;OGRE_BUILD_COMPONENT_BITES" FALSE)
cmake_dependent_option(OGRE_BUILD_COMPONENT_PYTHON "Build Python bindings" TRUE "NOT OGRE_STATIC" FALSE)
cmake_dependent_option(OGRE_BUILD_COMPONENT_JAVA "Build Java (JNI) bindings" TRUE "NOT OGRE_STATIC OR ANDROID" FALSE)
cmake_dependent_option(OGRE_BUILD_COMPONENT_CSHARP "Build Csharp bindings" TRUE "NOT OGRE_STATIC" FALSE)
option(OGRE_BUILD_COMPONENT_RTSHADERSYSTEM "Build RTShader System component" TRUE)
cmake_dependent_option(OGRE_BUILD_RTSHADERSYSTEM_SHADERS "Build RTShader System FFP shaders" TRUE "OGRE_BUILD_COMPONENT_RTSHADERSYSTEM" FALSE)

cmake_dependent_option(OGRE_BUILD_SAMPLES "Build Ogre demos" TRUE "OGRE_BUILD_COMPONENT_OVERLAY;OGRE_BUILD_COMPONENT_BITES" FALSE)
cmake_dependent_option(OGRE_BUILD_TOOLS "Build the command-line tools" TRUE "pugixml_FOUND;NOT APPLE_IOS;NOT WINDOWS_STORE;NOT WINDOWS_PHONE" FALSE)
cmake_dependent_option(OGRE_BUILD_XSIEXPORTER "Build the Softimage exporter" FALSE "Softimage_FOUND" FALSE)
cmake_dependent_option(OGRE_BUILD_LIBS_AS_FRAMEWORKS "Build frameworks for libraries on OS X." TRUE "APPLE;NOT OGRE_BUILD_PLATFORM_APPLE_IOS" FALSE)
cmake_dependent_option(OGRE_BUILD_TESTS "Build the unit tests & PlayPen" FALSE "OGRE_BUILD_COMPONENT_BITES" FALSE)
option(OGRE_CONFIG_DOUBLE "Use doubles instead of floats in Ogre" FALSE)
option(OGRE_CONFIG_NODE_INHERIT_TRANSFORM "Tells the node whether it should inherit full transform from it's parent node or derived position, orientation and scale" FALSE)
set(OGRE_CONFIG_THREADS "3" CACHE STRING 
	"Enable Ogre thread safety support for multithreading. Possible values:
	0 - no thread safety. DefaultWorkQueue is not threaded.
	1 - background resource preparation and loading is thread safe. Threaded DefaultWorkQueue. [DEPRECATED]
	2 - only background resource preparation is thread safe. Threaded DefaultWorkQueue. [DEPRECATED]
	3 - no thread safety. Threaded DefaultWorkQueue."
)
set_property(CACHE OGRE_CONFIG_THREADS PROPERTY STRINGS 0 1 2 3)
set(OGRE_CONFIG_THREAD_PROVIDER "std" CACHE STRING
	"Select the library to use for thread support. Possible values:
	boost - Boost thread library. [DEPRECATED]
	poco  - Poco thread library. [DEPRECATED]
	tbb   - ThreadingBuildingBlocks library. [DEPRECATED]
	std   - STL thread library (requires compiler support)."
)
set_property(CACHE OGRE_CONFIG_THREAD_PROVIDER PROPERTY STRINGS boost poco tbb std)
cmake_dependent_option(OGRE_BUILD_PLUGIN_FREEIMAGE "Build FreeImage codec." TRUE "FreeImage_FOUND" FALSE)
cmake_dependent_option(OGRE_BUILD_PLUGIN_EXRCODEC "Build EXR Codec plugin" TRUE "OPENEXR_FOUND;" FALSE)
option(OGRE_BUILD_PLUGIN_STBI "Enable STBI image codec." TRUE)
option(OGRE_BUILD_PLUGIN_RSIMAGE "Enable image-rs codec." FALSE)
option(OGRE_CONFIG_ENABLE_MESHLOD "Enable Mesh Lod." TRUE)
option(OGRE_CONFIG_ENABLE_DDS "Build DDS codec." TRUE)
option(OGRE_CONFIG_ENABLE_PVRTC "Build PVRTC codec." ${APPLE_IOS})
option(OGRE_CONFIG_ENABLE_ETC "Build ETC codec." TRUE)
option(OGRE_CONFIG_ENABLE_ASTC "Build ASTC codec." TRUE)
option(OGRE_CONFIG_ENABLE_QUAD_BUFFER_STEREO "Enable stereoscopic 3D support" FALSE)
option(OGRE_CONFIG_ENABLE_ZIP "Build ZIP archive support. If you disable this option, you cannot use ZIP archives resource locations. The samples won't work." TRUE)
cmake_dependent_option(OGRE_CONFIG_ENABLE_GLES2_CG_SUPPORT "Enable Cg support to ES 2 render system" FALSE "OGRE_BUILD_RENDERSYSTEM_GLES2" FALSE)
cmake_dependent_option(OGRE_CONFIG_ENABLE_GLES2_GLSL_OPTIMISER "Enable GLSL optimiser use in GLES 2 render system" FALSE "OGRE_BUILD_RENDERSYSTEM_GLES2" FALSE)
cmake_dependent_option(OGRE_CONFIG_ENABLE_GL_STATE_CACHE_SUPPORT "Enable OpenGL state cache management" FALSE "OGRE_BUILD_RENDERSYSTEM_GL OR OGRE_BUILD_RENDERSYSTEM_GLES2 OR OGRE_BUILD_RENDERSYSTEM_GL3PLUS" FALSE)
option(OGRE_CONFIG_ENABLE_TBB_SCHEDULER "Enable TBB's scheduler initialisation/shutdown." TRUE)
cmake_dependent_option(OGRE_CONFIG_FILESYSTEM_UNICODE "paths expected to be in UTF-8 and wchar_t file IO routines are used" TRUE "MSVC" FALSE)
# Customise what to install
cmake_dependent_option(OGRE_INSTALL_SAMPLES "Install Ogre demos." TRUE "OGRE_BUILD_SAMPLES" FALSE)
option(OGRE_INSTALL_TOOLS "Install Ogre tools." TRUE)
option(OGRE_INSTALL_DOCS "Install documentation." TRUE)
cmake_dependent_option(OGRE_INSTALL_PDB "Install debug pdb files" TRUE "MSVC;NOT OGRE_STATIC" FALSE)
option(OGRE_PROFILING "Enable internal instrumentation." FALSE)
set(OGRE_PROFILING_REMOTERY_PATH "" CACHE PATH "set this to Remotery/lib to use Remotery instead of the buildin profiler")
cmake_dependent_option(OGRE_CONFIG_STATIC_LINK_CRT "Statically link the MS CRT dlls (msvcrt)" FALSE "MSVC" FALSE)
set(OGRE_LIB_DIRECTORY "lib${LIB_SUFFIX}" CACHE STRING "Install path for libraries, e.g. 'lib64' on some 64-bit Linux distros.")
set(OGRE_BIN_DIRECTORY "bin" CACHE STRING "Install path for binaries")

cmake_dependent_option(OGRE_INSTALL_VSPROPS "Install Visual Studio Property Page." FALSE "MSVC" FALSE)
if (OGRE_INSTALL_VSPROPS)
  configure_file(${OGRE_TEMPLATES_DIR}/OGRE.props.in ${PROJECT_BINARY_DIR}/OGRE.props)
  install(FILES ${PROJECT_BINARY_DIR}/OGRE.props DESTINATION "${CMAKE_INSTALL_PREFIX}")
endif ()

# provide option to install dependencies
include(InstallDependencies)

# determine threading options
include(PrepareThreadingOptions)

# Enable project folders by default if the IDE supportes them
# Hide option from other compilers.
if (MSVC OR CMAKE_GENERATOR MATCHES Xcode)
	option(OGRE_PROJECT_FOLDERS "Organize project into project folders." TRUE)
endif ()

# hide advanced options
mark_as_advanced(
  OGRE_BUILD_RTSHADERSYSTEM_CORE_SHADERS
  OGRE_BUILD_RTSHADERSYSTEM_EXT_SHADERS
  OGRE_CONFIG_DOUBLE
  OGRE_CONFIG_NODE_INHERIT_TRANSFORM
  OGRE_CONFIG_ENABLE_MESHLOD
  OGRE_CONFIG_ENABLE_DDS
  OGRE_CONFIG_ENABLE_PVRTC
  OGRE_CONFIG_ENABLE_ETC
  OGRE_CONFIG_ENABLE_ASTC
  OGRE_CONFIG_ENABLE_ZIP
  OGRE_CONFIG_ENABLE_GL_STATE_CACHE_SUPPORT
  OGRE_CONFIG_ENABLE_GLES2_CG_SUPPORT
  OGRE_CONFIG_ENABLE_GLES2_GLSL_OPTIMISER
  OGRE_CONFIG_ENABLE_TBB_SCHEDULER
  OGRE_PROFILING
  OGRE_CONFIG_STATIC_LINK_CRT
  OGRE_LIB_DIRECTORY
  OGRE_BIN_DIRECTORY
)

###################################################################
# configure global build settings based on selected build options
###################################################################
include(ConfigureBuild)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH};${CMAKE_INSTALL_PREFIX}/${OGRE_LIB_DIRECTORY}")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

###################################################################
# disable way too common compiler warnings on project level
###################################################################
if(MSVC)
	add_definitions( /wd4251 /wd4275 )
endif()

##################################################################
# Now setup targets
##################################################################

# install resource files
include(InstallResources)

# enable PCH support
include(PrecompiledHeader)

# Setup OgreMain project
add_subdirectory(OgreMain)

# Setup RenderSystems
add_subdirectory(RenderSystems)

# Setup Plugins
add_subdirectory(PlugIns)

# Setup Components
add_subdirectory(Components)

# Setup tests (before samples so that PlayPen is included in browser)
if (OGRE_BUILD_TESTS)
  # enable CTest
  ENABLE_TESTING()
  INCLUDE(CTest)
  add_subdirectory(Tests)
endif ()

# Setup samples
add_subdirectory(Samples)

# Setup command-line tools
if (OGRE_BUILD_TOOLS)
  add_subdirectory(Tools)
endif ()

# Setup XSIExporter
if (OGRE_BUILD_XSIEXPORTER)
  add_subdirectory(Tools/XSIExport)
endif ()

# Install documentation
add_subdirectory(Docs)

# Install sample media files
install(DIRECTORY Media/Main DESTINATION "${OGRE_MEDIA_PATH}/")
install(DIRECTORY Media/RTShaderLib DESTINATION "${OGRE_MEDIA_PATH}/")
if (OGRE_INSTALL_SAMPLES)
  install(DIRECTORY Samples/Media/ DESTINATION ${OGRE_MEDIA_PATH})
endif ()

# Provide CPack packaging target
include(Packaging)


# Show feature summary
include(OgreFeatureSummary)
