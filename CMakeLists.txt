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

cmake_minimum_required(VERSION 2.8.0)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)
cmake_policy(SET CMP0003 NEW)

if(CMAKE_VS_PLATFORM_NAME STREQUAL "Tegra-Android")
	set(CMAKE_LINK_LIBRARY_FLAG "")
endif()

# CMake 2.8.2 has a bug that creates unusable Xcode projects when using ARCHS_STANDARD_32_BIT
# to specify both armv6 and armv7.
if(OGRE_BUILD_PLATFORM_APPLE_IOS AND (CMAKE_VERSION VERSION_EQUAL 2.8.2) AND (CMAKE_GENERATOR STREQUAL "Xcode"))
	message(FATAL_ERROR "CMake 2.8.2 cannot create compatible Xcode projects for iOS, please download the latest version or an older release from http://www.cmake.org/files/")
endif()

# Use relative paths
# This is mostly to reduce path size for command-line limits on windows
if(WIN32)
  # This seems to break Xcode projects so definitely don't enable on Apple builds
  set(CMAKE_USE_RELATIVE_PATHS true)
  set(CMAKE_SUPPRESS_REGENERATION true)
endif()

if (APPLE AND NOT ANDROID)
  if(CMAKE_VERSION VERSION_LESS 3.0.0)
    include(CMakeForceCompiler)
    CMAKE_FORCE_C_COMPILER(clang GNU)
    CMAKE_FORCE_CXX_COMPILER(clang++ GNU)
  endif()
  SET(CMAKE_SIZEOF_VOID_P 4)
  set(CMAKE_XCODE_ATTRIBUTE_GCC_VERSION "com.apple.compilers.llvm.clang.1_0")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
endif ()


project(OGRE)

# Include necessary submodules
set(CMAKE_MODULE_PATH
  "${OGRE_SOURCE_DIR}/CMake"
  "${OGRE_SOURCE_DIR}/CMake/Utils"
  "${OGRE_SOURCE_DIR}/CMake/Packages"
)

if(ANDROID)
  set(CMAKE_FIND_ROOT_PATH ${OGRE_DEPENDENCIES_DIR} "${CMAKE_FIND_ROOT_PATH}")
endif()

include(CMakeDependentOption)
include(CheckCXXCompilerFlag)
include(MacroLogFeature)
include(OgreAddTargets)
include(OgreConfigTargets)
include(PreprocessorUtils)
set(OGRE_TEMPLATES_DIR "${OGRE_SOURCE_DIR}/CMake/Templates")
set(OGRE_WORK_DIR ${OGRE_BINARY_DIR})


#####################################################################
# Set up the basic build environment
#####################################################################

if (NOT CMAKE_BUILD_TYPE)
  # CMake defaults to leaving CMAKE_BUILD_TYPE empty. This screws up
  # differentiation between debug and release builds.
  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build, options are: None (CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif ()

# determine Ogre version numbers
include(OgreGetVersion)
ogre_get_version(${OGRE_SOURCE_DIR}/OgreMain/include/OgrePrerequisites.h)
message(STATUS "Configuring OGRE ${OGRE_VERSION}")
# Configure version file for use by scripts
configure_file("${OGRE_TEMPLATES_DIR}/version.txt.in" "${OGRE_BINARY_DIR}/version.txt" @ONLY)

# determine if we are compiling for a 32bit or 64bit system
include(CheckTypeSize)
CHECK_TYPE_SIZE("void*" OGRE_PTR_SIZE BUILTIN_TYPES_ONLY)
if (OGRE_PTR_SIZE EQUAL 8)
  set(OGRE_PLATFORM_X64 TRUE)
else ()
  set(OGRE_PLATFORM_X64 FALSE)
endif ()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_COMPILER_IS_CLANGXX 1)
endif ()

if (NOT APPLE)
  # Create debug libraries with _d postfix
  set(CMAKE_DEBUG_POSTFIX "_d")
endif ()

# Set compiler specific build flags
if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX)
  if (EMSCRIPTEN)
    set(OGRE_GCC_HAS_SSE FALSE)
  else()
    check_cxx_compiler_flag(-msse OGRE_GCC_HAS_SSE)
  endif()
  if (OGRE_GCC_HAS_SSE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse -msse2")
  endif ()
  # This is a set of sensible warnings that provide meaningful output
  set(OGRE_WARNING_FLAGS "-Wall -Winit-self -Wno-overloaded-virtual -Wcast-qual -Wwrite-strings -Wextra -Wno-unused-parameter -Wshadow -Wno-missing-field-initializers -Wno-long-long")
  if (EMSCRIPTEN)
      set(OGRE_WARNING_FLAGS "${OGRE_WARNING_FLAGS} -Wno-warn-absolute-paths")
  elseif (NOT APPLE AND NOT CMAKE_COMPILER_IS_CLANGXX)
      set(OGRE_WARNING_FLAGS "${OGRE_WARNING_FLAGS} -Wno-unused-but-set-parameter")
  endif ()
  set(CMAKE_CXX_FLAGS "${OGRE_WARNING_FLAGS} ${CMAKE_CXX_FLAGS}")

  set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG=1" )
endif ()
if (MSVC)
  if (CMAKE_BUILD_TOOL STREQUAL "nmake")
    # set variable to state that we are using nmake makefiles
	set(NMAKE TRUE)
  endif ()
  if (CMAKE_BUILD_TOOL STREQUAL "jom")
	# set variable to state that we are using jom makefiles
	set(JOM TRUE)
  endif ()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fp:fast")
  if (NOT OGRE_PLATFORM_X64)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE2")
  endif()
  # Enable intrinsics on MSVC in debug mode
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Oi")
  if (CMAKE_CL_64)
    # Visual Studio bails out on debug builds in 64bit mode unless
	# this flag is set...
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /bigobj")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /bigobj")
  else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fp:fast")
  endif ()
  if (OGRE_UNITY_BUILD)
    # object files can get large with Unity builds
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
  endif ()
  if (MSVC_VERSION GREATER 1500 OR MSVC_VERSION EQUAL 1500)
    option(OGRE_BUILD_MSVC_MP "Enable build with multiple processes in Visual Studio" TRUE)
  else()
    set(OGRE_BUILD_MSVC_MP FALSE CACHE BOOL "Compiler option /MP requires at least Visual Studio 2008 (VS9) or newer" FORCE)
  endif()
  if(OGRE_BUILD_MSVC_MP)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  endif ()
  if(MSVC_VERSION GREATER 1400 OR MSVC_VERSION EQUAL 1400)
    option(OGRE_BUILD_MSVC_ZM "Add /Zm256 compiler option to Visual Studio to fix PCH errors" TRUE)
  else()
    set(OGRE_BUILD_MSVC_ZM FALSE CACHE BOOL "Compiler option /Zm256 requires at least Visual Studio 2005 (VS8) or newer" FORCE)
  endif()
  if(OGRE_BUILD_MSVC_ZM)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zm256")
  endif ()
endif ()
if (MINGW)
  add_definitions(-D_WIN32_WINNT=0x0500)
  if( CMAKE_SIZEOF_VOID_P EQUAL 4 )
    # set architecture to i686, since otherwise some versions of MinGW can't link
    # the atomic primitives
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=i686")
  endif ()
  # Fpermissive required to let some "non-standard" casting operations in the plugins pass
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
  # disable this optimisation because it breaks release builds (reason unknown)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-tree-slp-vectorize")
  # Ignore some really annoying warnings which also happen in dependencies
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=cast-qual -Wno-unused-local-typedefs")
endif ()

if ((CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX) AND NOT MINGW)
  # Test for GCC visibility
  include(CheckCXXCompilerFlag)
  check_cxx_compiler_flag(-fvisibility=hidden OGRE_GCC_VISIBILITY)
  if (OGRE_GCC_VISIBILITY)
    # determine gcc version
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion
      OUTPUT_VARIABLE OGRE_GCC_VERSION)
    message(STATUS "Detected g++ ${OGRE_GCC_VERSION}")
    message(STATUS "Enabling GCC visibility flags")
    set(OGRE_GCC_VISIBILITY_FLAGS "-DOGRE_GCC_VISIBILITY -fvisibility=hidden")
    set(XCODE_ATTRIBUTE_GCC_SYMBOLS_PRIVATE_EXTERN "YES")

    # check if we can safely add -fvisibility-inlines-hidden
    string(TOLOWER "${CMAKE_BUILD_TYPE}" OGRE_BUILD_TYPE)
    if (OGRE_BUILD_TYPE STREQUAL "debug" AND OGRE_GCC_VERSION VERSION_LESS "4.2")
      message(STATUS "Skipping -fvisibility-inlines-hidden due to possible bug in g++ < 4.2")
    else ()
      if (APPLE AND NOT OGRE_BUILD_PLATFORM_APPLE_IOS)
        message(STATUS "Skipping -fvisibility-inlines-hidden due to linker issues")
        set(XCODE_ATTRIBUTE_GCC_INLINES_ARE_PRIVATE_EXTERN[arch=x86_64] "YES")
      else()
        set(OGRE_GCC_VISIBILITY_FLAGS "${OGRE_GCC_VISIBILITY_FLAGS} -fvisibility-inlines-hidden")
        set(XCODE_ATTRIBUTE_GCC_INLINES_ARE_PRIVATE_EXTERN "YES")
      endif()
    endif ()
  endif (OGRE_GCC_VISIBILITY)

  # Fix x64 issues on Linux
  if(OGRE_PLATFORM_X64 AND NOT APPLE)
    add_definitions(-fPIC)
  endif()
endif ((CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX) AND NOT MINGW)

# determine system endianess
if (MSVC OR ANDROID OR EMSCRIPTEN)
  # This doesn't work on VS 2010
  # MSVC only builds for intel anyway
  set(OGRE_TEST_BIG_ENDIAN FALSE)
else()
  include(TestBigEndian)
  test_big_endian(OGRE_TEST_BIG_ENDIAN)
endif()

# Add OgreMain include path
include_directories("${OGRE_SOURCE_DIR}/OgreMain/include")
include_directories("${OGRE_BINARY_DIR}/include")
if (APPLE)
  if (OGRE_BUILD_PLATFORM_APPLE_IOS)
    include_directories("${OGRE_SOURCE_DIR}/OgreMain/include/iOS")

    # Set static early for proper dependency detection
    set(OGRE_STATIC TRUE)
  else ()
    include_directories("${OGRE_SOURCE_DIR}/OgreMain/include/OSX")
  endif ()
endif (APPLE)

# Add platform-specific render system Metal include path
if (APPLE AND OGRE_BUILD_RENDERSYSTEM_METAL)
  if (OGRE_BUILD_PLATFORM_APPLE_IOS)
    include_directories("${OGRE_SOURCE_DIR}/RenderSystems/Metal/include/Windowing/iOS")
  else ()
    include_directories("${OGRE_SOURCE_DIR}/RenderSystems/Metal/include/Windowing/OSX")
  endif ()
endif (APPLE AND OGRE_BUILD_RENDERSYSTEM_METAL)

# Find dependencies
include(Dependencies)
# definitions for samples
set(OGRE_LIBRARIES OgreMain)
set(OGRE_MeshLodGenerator_LIBRARIES OgreMeshLodGenerator)
set(OGRE_Paging_LIBRARIES OgrePaging)
set(OGRE_Terrain_LIBRARIES OgreTerrain)
set(OGRE_Volume_LIBRARIES OgreVolume)

# Specify build paths
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OGRE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OGRE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OGRE_BINARY_DIR}/bin")
if (WIN32 OR APPLE)
  if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    # We don't want to install in default system location, install is really for the SDK, so call it that
    set(CMAKE_INSTALL_PREFIX
	  "${OGRE_BINARY_DIR}/sdk" CACHE PATH "OGRE install prefix" FORCE
    )
  endif (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
endif(WIN32 OR APPLE)

# Set up iOS overrides.
if (OGRE_BUILD_PLATFORM_APPLE_IOS)
  include_directories("${OGRE_SOURCE_DIR}/OgreMain/include/iOS")

  # Set build variables
  set(XCODE_ATTRIBUTE_SDKROOT iphoneos)
  if(CMAKE_GENERATOR STREQUAL "Unix Makefiles")
    execute_process(COMMAND xcodebuild -version -sdk "${XCODE_ATTRIBUTE_SDKROOT}" Path | head -n 1 OUTPUT_VARIABLE CMAKE_OSX_SYSROOT)
    string(REGEX REPLACE "(\r?\n)+$" "" CMAKE_OSX_SYSROOT "${CMAKE_OSX_SYSROOT}")
  else()
    set(CMAKE_OSX_SYSROOT iphoneos)
  endif()

  set(CMAKE_EXE_LINKER_FLAGS "-framework Foundation -framework CoreGraphics -framework QuartzCore -framework UIKit")
  set( CMAKE_XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES )
  if( OGRE_BUILD_RENDERSYSTEM_METAL )
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework Metal")
  endif()
  set(XCODE_ATTRIBUTE_GCC_UNROLL_LOOPS "YES")
  set(XCODE_ATTRIBUTE_LLVM_VECTORIZE_LOOPS "YES")
  set(XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer")
  set(XCODE_ATTRIBUTE_GCC_PRECOMPILE_PREFIX_HEADER "YES")
  #set(OGRE_BUILD_RENDERSYSTEM_GLES2 TRUE CACHE BOOL "Forcing OpenGL ES 2 RenderSystem for iOS" FORCE)
  set(OGRE_STATIC TRUE CACHE BOOL "Forcing static build for iOS" FORCE)
  set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.yourcompany.\${PRODUCT_NAME:rfc1034identifier}")
  set( CMAKE_XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2" )

  set(CMAKE_OSX_ARCHITECTURES "$(ARCHS_STANDARD_INCLUDING_64_BIT)")
  string (REPLACE "-msse2" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  string (REPLACE "-msse" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
elseif (ANDROID)
  set(TargetPlatform "Android")
  set(OGRE_PLATFORM OGRE_PLATFORM_ANDROID)
  set(OGRE_CONFIG_ENABLE_VIEWPORT_ORIENTATIONMODE FALSE CACHE BOOL "Disable viewport orientation Android" FORCE)
  option(OGRE_BUILD_ANDROID_JNI_SAMPLE "Enable JNI Sample" FALSE)

  if( NOT ANDROID_GLES_ONLY )
   set(OGRE_CONFIG_ENABLE_GLES2_VAO_SUPPORT FALSE CACHE BOOL "Disable VAO on Android" FORCE)
   set(OGRE_BUILD_RENDERSYSTEM_GLES2 TRUE CACHE BOOL "Forcing OpenGL ES 2 RenderSystem for Android" FORCE)
  else()
   set(OGRE_BUILD_COMPONENT_RTSHADERSYSTEM FALSE CACHE BOOL "Disable RTSS component on Android ES1" FORCE)
   set(OGRE_BUILD_COMPONENT_VOLUME FALSE CACHE BOOL "Disable volume component on Android ES1" FORCE)
   set(OGRE_BUILD_COMPONENT_PAGING FALSE CACHE BOOL "Disable paging component on Android ES1" FORCE)
   set(OGRE_BUILD_COMPONENT_TERRAIN FALSE CACHE BOOL "Disable terrain component on Android ES1" FORCE)
  endif()

  set(OGRE_BUILD_TESTS FALSE CACHE BOOL "Disable tests on Android" FORCE)
  set(OGRE_BUILD_TOOLS FALSE CACHE BOOL "Disable tools on Android" FORCE)
  set(OGRE_STATIC TRUE CACHE BOOL "Forcing static build for Android" FORCE)
  set(OGRE_SIMD FALSE CACHE BOOL "Disable SIMD on Android" FORCE)
  set(OGRE_RESTRICT_ALIASING FALSE CACHE BOOL "Disable strict aliasing on Android" FORCE)
  string (REPLACE "-msse2" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
elseif(EMSCRIPTEN)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++11-extensions")
  add_definitions(-DEMSCRIPTEN=1 -D__EMSCRIPTEN__=1)
  set(TargetPlatform "Emscripten")
  set(OGRE_PLATFORM OGRE_PLATFORM_EMSCRIPTEN)

  set(OGRE_BUILD_RENDERSYSTEM_GLES2 TRUE CACHE BOOL "Forcing OpenGL ES 2 RenderSystem for Emscripten" FORCE)

  set(OGRE_CONFIG_ENABLE_STBI TRUE CACHE BOOL "Enable STBIImageCodec on Emscripten (Smaller Footprint)" FORCE)
  set(OGRE_CONFIG_ENABLE_FREEIMAGE FALSE CACHE BOOL "Disable Freeimage on Emscripten (Smaller Footprint)" FORCE)
  set(OGRE_BUILD_TOOLS FALSE CACHE BOOL "Disable tools on Emscripten" FORCE)
  set(OGRE_BUILD_TESTS FALSE CACHE BOOL "Disable tests on Emscripten" FORCE)
  set(OGRE_BUILD_COMPONENT_VOLUME FALSE CACHE BOOL "Disable volume component on Emscripten" FORCE)
  set(OGRE_BUILD_COMPONENT_PAGING FALSE CACHE BOOL "Disable paging component on Emscripten" FORCE)
  set(OGRE_BUILD_COMPONENT_TERRAIN FALSE CACHE BOOL "Disable terrain component on Emscripten" FORCE)
  set(OGRE_CONFIG_ALLOCATOR 1 CACHE STRING "Use default allocator on Emscripten" FORCE)
  set(OGRE_CONFIG_CONTAINERS_USE_CUSTOM_ALLOCATOR FALSE CACHE BOOL "Containers use default allocator on Emscripten" FORCE)
elseif (APPLE AND NOT OGRE_BUILD_PLATFORM_APPLE_IOS)

  set(XCODE_ATTRIBUTE_SDKROOT macosx)
  if(CMAKE_GENERATOR STREQUAL "Unix Makefiles")
    execute_process(COMMAND xcodebuild -version -sdk "${XCODE_ATTRIBUTE_SDKROOT}" Path | head -n 1 OUTPUT_VARIABLE CMAKE_OSX_SYSROOT)
    string(REGEX REPLACE "(\r?\n)+$" "" CMAKE_OSX_SYSROOT "${CMAKE_OSX_SYSROOT}")
  else()
    set(CMAKE_OSX_SYSROOT macosx)
  endif()

  set( CMAKE_XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES )

  if (NOT CMAKE_OSX_ARCHITECTURES)
    if(OGRE_BUILD_RENDERSYSTEM_GL3PLUS)
      if(CMAKE_GENERATOR STREQUAL "Unix Makefiles")
        set(CMAKE_OSX_ARCHITECTURES "${ARCHS_STANDARD_64_BIT}")
      else()
        set(CMAKE_OSX_ARCHITECTURES "${ARCHS_STANDARD_64_BIT}")
      endif()
    else()
      if(CMAKE_GENERATOR STREQUAL "Unix Makefiles")
        set(CMAKE_OSX_ARCHITECTURES "${ARCHS_STANDARD_32_64_BIT}")
      else()
        set(CMAKE_OSX_ARCHITECTURES "${ARCHS_STANDARD_32_64_BIT}")
      endif()
    endif()
  endif()

  # Make sure that the OpenGL render system is selected for non-iOS Apple builds
  set(OGRE_BUILD_RENDERSYSTEM_GLES2 FALSE)
endif ()

if(OGRE_BUILD_COMPONENT_MESHLODGENERATOR)
  set(OGRE_CONFIG_ENABLE_MESHLOD TRUE CACHE BOOL "Forcing Mesh Lod" FORCE)
endif()

if(OGRE_CONFIG_ENABLE_GLES3_SUPPORT)
  set(OGRE_CONFIG_ENABLE_GLES2_VAO_SUPPORT TRUE CACHE BOOL "Forcing VAO support for OpenGL ES 3" FORCE)
endif()

# Enable the PVRTC codec if OpenGL ES is being built
if(OGRE_BUILD_RENDERSYSTEM_GLES2)
  set(OGRE_CONFIG_ENABLE_PVRTC TRUE CACHE BOOL "Forcing PVRTC codec for OpenGL ES" FORCE)
  set(OGRE_CONFIG_ENABLE_ETC TRUE CACHE BOOL "Forcing ETC codec for OpenGL ES" FORCE)
endif()

# Enable the ETC codec if OpenGL 3+ is being built
if(OGRE_BUILD_RENDERSYSTEM_GL3PLUS)
  set(OGRE_CONFIG_ENABLE_ETC TRUE CACHE BOOL "Forcing ETC codec for OpenGL 3+" FORCE)
endif()

if(OGRE_BUILD_RENDERSYSTEM_METAL AND OGRE_BUILD_PLATFORM_APPLE_IOS)
  set(OGRE_CONFIG_ENABLE_ASTC TRUE CACHE BOOL "Forcing ASTC codec for Metal" FORCE)
endif()

######################################################################
# Provide user options to customise the build process
######################################################################

# Customise what to build
cmake_dependent_option(OGRE_STATIC "Static build" FALSE "" TRUE)
cmake_dependent_option(OGRE_BUILD_RENDERSYSTEM_D3D11 "Build Direct3D11 RenderSystem" TRUE "WIN32;DirectX11_FOUND OR WINDOWS_STORE OR WINDOWS_PHONE" FALSE)
cmake_dependent_option(OGRE_BUILD_RENDERSYSTEM_GL3PLUS "Build OpenGL 3+ RenderSystem" TRUE "OPENGL_FOUND; NOT OGRE_BUILD_RENDERSYSTEM_GLES; NOT OGRE_BUILD_RENDERSYSTEM_GLES2;NOT WINDOWS_STORE;NOT WINDOWS_PHONE;NOT OGRE_BUILD_PLATFORM_APPLE_IOS" FALSE)
cmake_dependent_option(OGRE_BUILD_RENDERSYSTEM_GLES "Build OpenGL ES 1.x RenderSystem" FALSE "OPENGLES_FOUND;NOT WINDOWS_STORE;NOT WINDOWS_PHONE" FALSE)
if( APPLE )
    option(OGRE_BUILD_PLATFORM_APPLE_IOS "Build For iOS (Apple only)" FALSE)
    option(OGRE_BUILD_RENDERSYSTEM_METAL "Build Metal RenderSystem (Apple only)" TRUE)
endif()
cmake_dependent_option(OGRE_BUILD_PLATFORM_NACL "Build Ogre for Google's Native Client (NaCl)" FALSE "OPENGLES2_FOUND;NOT WINDOWS_STORE;NOT WINDOWS_PHONE" FALSE)
option(OGRE_BUILD_PLUGIN_PFX "Build ParticleFX plugin" TRUE)

cmake_dependent_option(OGRE_BUILD_COMPONENT_HLMS_PBS_MOBILE
"PBS Stands for Physically Based Shading and it's the default material for most entities and meshes. This is the 'mobile' version for OpenGL ES 2.0.
If you're targetting only GL ES 3.0 and above; you shouldn't use this component and use the non-mobile instead. Although this component may work with
ES 3.0, GL3+ and even DX11 render systems, the mobile version uses uniform variables and limited shader based instancing, whereas non-mobile uses
uniform buffers, texture arrays and true hardware instancing for lower API overhead" TRUE "" FALSE)
cmake_dependent_option(OGRE_BUILD_COMPONENT_HLMS_UNLIT_MOBILE
"Unlit material system supporting texture animations, multiple diffuse texture units (aka multitexturing) with Photoshop-like blend modes. This is the
'mobile' version for OpenGL ES 2.0. If you're targetting only GL ES 3.0 and above; you shouldn't use this component and use the non-mobile instead." TRUE "" FALSE)
cmake_dependent_option(OGRE_BUILD_COMPONENT_HLMS_PBS
"PBS Stands for Physically Based Shading and it's the default material for most entities and meshes. This is the 'desktop' version for OpenGL 3.3+ and D3D11." TRUE "" FALSE)
cmake_dependent_option(OGRE_BUILD_COMPONENT_HLMS_UNLIT
"Unlit material system supporting texture animations, multiple diffuse texture units (aka multitexturing) with Photoshop-like blend modes. This is the
'desktop' version for OpenGL 3.3+ and D3D11." TRUE "" FALSE)
cmake_dependent_option(OGRE_BUILD_COMPONENT_PAGING "Build Paging component" FALSE "" FALSE)
option( OGRE_BUILD_COMPONENT_PLANAR_REFLECTIONS "Component to use planar reflections, can be used by both HlmsPbs & HlmsUnlit" FALSE )
cmake_dependent_option(OGRE_BUILD_COMPONENT_MESHLODGENERATOR "Build MeshLodGenerator component" TRUE "" FALSE)
cmake_dependent_option(OGRE_BUILD_COMPONENT_TERRAIN "Build Terrain component" FALSE "" FALSE)
cmake_dependent_option(OGRE_BUILD_COMPONENT_VOLUME "Build Volume component" FALSE "" FALSE)
cmake_dependent_option(OGRE_BUILD_COMPONENT_PROPERTY "Build Property component" FALSE "Boost_FOUND" FALSE)
cmake_dependent_option(OGRE_BUILD_PLUGIN_CG "Build Cg plugin" FALSE "Cg_FOUND;NOT OGRE_BUILD_PLATFORM_APPLE_IOS" FALSE)
cmake_dependent_option(OGRE_BUILD_COMPONENT_OVERLAY "Build Overlay component" TRUE "FREETYPE_FOUND OR OGRE_BUILD_PLATFORM_WINRT OR OGRE_BUILD_PLATFORM_WINDOWS_PHONE" FALSE)
option(OGRE_BUILD_COMPONENT_RTSHADERSYSTEM "Build RTShader System component" FALSE)
cmake_dependent_option(OGRE_BUILD_RTSHADERSYSTEM_CORE_SHADERS "Build RTShader System FFP core shaders" FALSE "OGRE_BUILD_COMPONENT_RTSHADERSYSTEM" FALSE)
cmake_dependent_option(OGRE_BUILD_RTSHADERSYSTEM_EXT_SHADERS "Build RTShader System extensions shaders" FALSE "OGRE_BUILD_COMPONENT_RTSHADERSYSTEM" FALSE)
cmake_dependent_option(OGRE_BUILD_LIBS_AS_FRAMEWORKS "Build frameworks for libraries on OS X." TRUE "APPLE;NOT OGRE_BUILD_PLATFORM_APPLE_IOS" FALSE)

option(OGRE_BUILD_SAMPLES2 "Build Ogre new demos (2.1)" TRUE)
cmake_dependent_option(OGRE_BUILD_SAMPLES_AS_BUNDLES "Build samples as application bundles on OS X." TRUE "APPLE;OGRE_BUILD_SAMPLES2" FALSE)
cmake_dependent_option(OGRE_BUILD_TOOLS "Build the command-line tools" TRUE "NOT OGRE_BUILD_PLATFORM_APPLE_IOS;NOT WINDOWS_STORE;NOT OGRE_BUILD_PLATFORM_WINDOWS_PHONE" FALSE)
cmake_dependent_option(OGRE_BUILD_XSIEXPORTER "Build the Softimage exporter" FALSE "Softimage_FOUND" FALSE)
option(OGRE_BUILD_TESTS "Build the unit tests & PlayPen" FALSE)
option(OGRE_CONFIG_DOUBLE "Use doubles instead of floats in Ogre" FALSE)

if (WINDOWS_STORE OR WINDOWS_PHONE)
# WinRT can only use the standard allocator
set(OGRE_CONFIG_ALLOCATOR 1 CACHE STRING "Forcing Standard Allocator for WinRT" FORCE)
else ()
set(OGRE_CONFIG_ALLOCATOR 1 CACHE STRING
"Specify the memory allocator to use. Possible values:
  1 - Standard allocator
  2 - nedmalloc - NOT RECOMMENDED - Only useful in WinXP & for debugging mem. corruption - Has known issues see https://github.com/ned14/nedmalloc/issues/15
  3 - User-provided allocator
  4 - nedmalloc with pooling - NOT RECOMMENDED - See nedmalloc issues.
  5 - Debug allocator that will assert on most forms of memory corruption and provides deterministic memory addressing (for easier debugging w/ data breakpoints). Not indended for release or deployment. You may want to tweak OGRE_TRACK_POOL_SIZE in OgreMain/src/OgreMemoryTrackAlloc.cpp."
)
endif ()

set( OGRE_DEBUG_LEVEL_DEBUG 3 CACHE STRING
    "Specify debug level for debug builds:
    0 - None. Disabled. No checks done at all.
    1 - Low. We perform basic assert checks and other non-performance intensive checks.
    2 - Medium. We alter classes to add some debug variables for a bit more thorough checking and also perform checks that may cause some performance hit.
    3 - High. We perform intensive validation without concerns for performance."
)
set( OGRE_DEBUG_LEVEL_RELEASE 0 CACHE STRING
    "Specify debug level for Release, RelWithDebInfo and MinSizeRel builds. See OGRE_MAX_DEBUG_LEVEL_DEBUG" )

cmake_dependent_option(OGRE_CONFIG_CONTAINERS_USE_CUSTOM_ALLOCATOR "STL containers in Ogre use the custom allocator" TRUE "" FALSE)
option(OGRE_CONFIG_STRING_USE_CUSTOM_ALLOCATOR "Ogre String uses the custom allocator" FALSE)
option(OGRE_CONFIG_MEMTRACK_DEBUG "Enable Ogre's memory tracker in debug mode" FALSE)
option(OGRE_CONFIG_MEMTRACK_RELEASE "Enable Ogre's memory tracker in release mode" FALSE)
# determine threading options
include(PrepareThreadingOptions)
cmake_dependent_option(OGRE_CONFIG_ENABLE_FREEIMAGE "Build FreeImage codec. If you disable this option, you need to provide your own image handling codecs." TRUE "FreeImage_FOUND" FALSE)
option(OGRE_CONFIG_ENABLE_STBI "Enable STBI image codec." FALSE)
cmake_dependent_option(OGRE_CONFIG_ENABLE_JSON "Use rapidjson (needed by Hlms JSON materials)" TRUE "Rapidjson_FOUND" FALSE)
option(OGRE_CONFIG_ENABLE_MESHLOD "Enable Mesh Lod." TRUE)
option(OGRE_CONFIG_ENABLE_DDS "Build DDS codec." TRUE)
option(OGRE_CONFIG_ENABLE_PVRTC "Build PVRTC codec." FALSE)
option(OGRE_CONFIG_ENABLE_ETC "Build ETC codec." FALSE)
option(OGRE_CONFIG_ENABLE_ASTC "Build ASTC codec." FALSE)
option(OGRE_CONFIG_ENABLE_QUAD_BUFFER_STEREO "Enable stereoscopic 3D support" FALSE)
cmake_dependent_option(OGRE_CONFIG_ENABLE_ZIP "Build ZIP archive support. If you disable this option, you cannot use ZIP archives resource locations. The samples won't work." TRUE "ZZip_FOUND" FALSE)
option(OGRE_CONFIG_ENABLE_VIEWPORT_ORIENTATIONMODE "Include Viewport orientation mode support." FALSE)
cmake_dependent_option(OGRE_CONFIG_ENABLE_GLES2_CG_SUPPORT "Enable Cg support to ES 2 render system" FALSE "OGRE_BUILD_RENDERSYSTEM_GLES2" FALSE)
cmake_dependent_option(OGRE_CONFIG_ENABLE_GLES2_GLSL_OPTIMISER "Enable GLSL optimiser use in GLES 2 render system" FALSE "OGRE_BUILD_RENDERSYSTEM_GLES2" FALSE)
cmake_dependent_option(OGRE_CONFIG_ENABLE_GLES2_VAO_SUPPORT "Use VertexArrayObjects if possible." FALSE "OGRE_BUILD_RENDERSYSTEM_GLES2" FALSE)
cmake_dependent_option(OGRE_CONFIG_ENABLE_GL_STATE_CACHE_SUPPORT "Enable OpenGL state cache management" FALSE "OGRE_BUILD_RENDERSYSTEM_GL OR OGRE_BUILD_RENDERSYSTEM_GLES OR OGRE_BUILD_RENDERSYSTEM_GLES2 OR OGRE_BUILD_RENDERSYSTEM_GL3PLUS" FALSE)
cmake_dependent_option(OGRE_CONFIG_ENABLE_GLES3_SUPPORT "Enable OpenGL ES 3.x Features" FALSE "OPENGLES3_FOUND;OGRE_BUILD_RENDERSYSTEM_GLES2" FALSE)
option(OGRE_CONFIG_ENABLE_FINE_LIGHT_MASK_GRANULARITY "Forward+ will support setLightMask, for a fine granularity control over which lights affect which objects. Performance impact may vary (may be slower, may be faster if you filter a lot of lights)" FALSE)
option(OGRE_CONFIG_ENABLE_TBB_SCHEDULER "Enable TBB's scheduler initialisation/shutdown." TRUE)
cmake_dependent_option(OGRE_USE_BOOST "Use Boost extensions" FALSE "Boost_FOUND" FALSE)

cmake_dependent_option( OGRE_BUILD_COMPONENT_SCENE_FORMAT "Component to export and import scenes (meshes, textures, items, entities)" FALSE "OGRE_CONFIG_ENABLE_JSON" FALSE )

# Customise what to install
option(OGRE_INSTALL_SAMPLES "Install Ogre demos." TRUE)
option(OGRE_INSTALL_TOOLS "Install Ogre tools." TRUE)
option(OGRE_INSTALL_DOCS "Install documentation." TRUE)
option(OGRE_INSTALL_SAMPLES_SOURCE "Install samples source files." FALSE)
cmake_dependent_option(OGRE_INSTALL_PDB "Install debug pdb files" TRUE "MSVC" FALSE)
cmake_dependent_option(OGRE_FULL_RPATH "Build executables with the full required RPATH to run from their install location." FALSE "NOT WIN32" FALSE)
option(OGRE_LEGACY_ANIMATIONS "Use the skeletal animation from 1.x. It's much slower, but the new system is still experimental" TRUE)
option(OGRE_SIMD_SSE2 "Enable SIMD (Include SSE2 files)." TRUE)
option(OGRE_SIMD_NEON "Enable SIMD (Include NEON files)." TRUE)
option(OGRE_RESTRICT_ALIASING "Restrict aliasing." TRUE)
option(OGRE_IDSTRING_ALWAYS_READABLE "Always keep readable strings on IdString, even in Release builds." FALSE)
cmake_dependent_option(OGRE_CONFIG_STATIC_LINK_CRT "Statically link the MS CRT dlls (msvcrt)" FALSE "MSVC" FALSE)
set(OGRE_LIB_DIRECTORY "lib${LIB_SUFFIX}" CACHE STRING "Install path for libraries, e.g. 'lib64' on some 64-bit Linux distros.")
if (WIN32)
	option(OGRE_INSTALL_VSPROPS "Install Visual Studio Property Page." FALSE)
	if (OGRE_INSTALL_VSPROPS)
		configure_file(${OGRE_TEMPLATES_DIR}/OGRE.props.in ${OGRE_BINARY_DIR}/OGRE.props)
		install(FILES ${OGRE_BINARY_DIR}/OGRE.props DESTINATION "${CMAKE_INSTALL_PREFIX}")
	endif ()
endif ()

# Unity build options
# A Unity build includes all sources files in just a few actual compilation units
# to potentially speed up the compilation.
option(OGRE_UNITY_BUILD "Enable unity build for Ogre." FALSE)
set(OGRE_UNITY_FILES_PER_UNIT "50" CACHE STRING "Number of files per compilation unit in Unity build.")

set( OGRE_PROFILING_PROVIDER "none" CACHE STRING
	"Enable profiling of Ogre for performance analysis. Profiling has a performance
	overhead so you may want to ship your game with this option disabled.
	none - Profiling OFF
	internal - Use internal profiling with on-screen overlays
	remotery - Use Remotery. https://github.com/Celtoys/Remotery
	offline - Use internal profiling that generates a CSV file for offline analysis"
)
option(OGRE_PROFILING_EXHAUSTIVE "When a valid profiler provider is set, includes exhaustive information of Ogre calls to better find culprit of big slowdowns or hitches, particularly why load times are slow. Best used with 'offline' profiler provider" FALSE)

if( NOT Remotery_FOUND AND OGRE_PROFILING_PROVIDER STREQUAL "remotery" )
	message( STATUS "WARNING: Remotery not found! Forcing profiling to disable" )
	set( OGRE_PROFILING_PROVIDER "none" )
endif()

# hide advanced options
mark_as_advanced(
  OGRE_BUILD_RTSHADERSYSTEM_CORE_SHADERS
  OGRE_BUILD_RTSHADERSYSTEM_EXT_SHADERS
  OGRE_CONFIG_DOUBLE
  OGRE_CONFIG_ALLOCATOR
  OGRE_CONFIG_CONTAINERS_USE_CUSTOM_ALLOCATOR
  OGRE_CONFIG_STRING_USE_CUSTOM_ALLOCATOR
  OGRE_CONFIG_MEMTRACK_DEBUG
  OGRE_CONFIG_MEMTRACK_RELEASE
  OGRE_CONFIG_ENABLE_MESHLOD
  OGRE_CONFIG_ENABLE_DDS
  OGRE_CONFIG_ENABLE_FREEIMAGE
  OGRE_CONFIG_ENABLE_PVRTC
  OGRE_CONFIG_ENABLE_ETC
  OGRE_CONFIG_ENABLE_STBI
  OGRE_CONFIG_ENABLE_ASTC
  OGRE_CONFIG_ENABLE_VIEWPORT_ORIENTATIONMODE
  OGRE_CONFIG_ENABLE_ZIP
  OGRE_CONFIG_ENABLE_GL_STATE_CACHE_SUPPORT
  OGRE_CONFIG_ENABLE_GLES2_CG_SUPPORT
  OGRE_CONFIG_ENABLE_GLES2_GLSL_OPTIMISER
  OGRE_CONFIG_ENABLE_GLES2_VAO_SUPPORT
  OGRE_CONFIG_ENABLE_GLES3_SUPPORT
  OGRE_CONFIG_ENABLE_TBB_SCHEDULER
  OGRE_CONFIG_ENABLE_FINE_LIGHT_MASK_GRANULARITY
  OGRE_USE_BOOST
  OGRE_INSTALL_SAMPLES_SOURCE
  OGRE_FULL_RPATH
  OGRE_PROFILING_PROVIDER
  OGRE_CONFIG_STATIC_LINK_CRT
  OGRE_LIB_DIRECTORY
)

###################################################################
# configure global build settings based on selected build options
###################################################################
include(ConfigureBuild)

###################################################################
# disable (useless) compiler warnings on project level
###################################################################
if(MSVC)
	add_definitions( /wd4786 /wd4503 /wd4251 /wd4275 /wd4290 /wd4661 /wd4996 /wd4127 /wd4100)
endif()

##################################################################
# Now setup targets
##################################################################

# install resource files
include(InstallResources)

# Setup OgreMain project
add_subdirectory(OgreMain)

# Setup RenderSystems
add_subdirectory(RenderSystems)

# Setup Plugins
add_subdirectory(PlugIns)

# Setup Components
add_subdirectory(Components)

# Setup tests (before samples so that PlayPen is included in browser)
if (OGRE_BUILD_TESTS AND 0)
  # enable CTest
  ENABLE_TESTING()
  INCLUDE(CTest)
  add_subdirectory(Tests)
endif ()

# Setup samples
add_subdirectory(Samples)

# Add android JNI binding
if(ANDROID AND OGRE_BUILD_ANDROID_JNI_SAMPLE)
  include(toolchain/AndroidJNI)
endif()

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

# Install media files
if (OGRE_INSTALL_SAMPLES OR OGRE_INSTALL_SAMPLES_SOURCE)
  add_subdirectory(Samples/Media)
endif ()

# Install CMake modules
add_subdirectory(CMake)

# Provide CPack packaging target
include(Packaging)


# Show feature summary
include(FeatureSummary)

