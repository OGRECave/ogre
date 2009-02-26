######################################################################
# OGRE BUILD SYSTEM
# Welcome to the CMake build system for OGRE.
# This is the main file where we prepare the general build environment
# and provide build configuration options.
######################################################################

cmake_minimum_required(VERSION 2.6)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)
cmake_policy(SET CMP0003 NEW)

project(OGRE3D)

# Include necessary submodules
set(CMAKE_MODULE_PATH 
  "${OGRE3D_SOURCE_DIR}/CMake"
  "${OGRE3D_SOURCE_DIR}/CMake/Utils" 
  "${OGRE3D_SOURCE_DIR}/CMake/Packages"
)
include(MacroLogFeature)
include(OgreInstall)
include(OgreConfigBuild)
set(OGRE_TEMPLATES_DIR "${OGRE3D_SOURCE_DIR}/CMake/Templates")


######################################################################
# Provide user options to customise the build process
######################################################################

# Customise what to build
option(OGRE_STATIC "Static build (keep LGPL restrictions in mind!)" FALSE)
if (WIN32)
  option(OGRE_BUILD_RENDERSYSTEM_D3D9 "Build Direct3D9 RenderSystem" TRUE)
  option(OGRE_BUILD_RENDERSYSTEM_D3D10 "Build Direct3D10 RenderSystem" TRUE)
endif (WIN32)
option(OGRE_BUILD_RENDERSYSTEM_GL "Build OpenGL RenderSystem" TRUE)
option(OGRE_BUILD_PLUGIN_BSP "Build BSP SceneManager plugin" TRUE)
option(OGRE_BUILD_PLUGIN_OCTREE "Build Octree SceneManager plugin" TRUE)
option(OGRE_BUILD_PLUGIN_PCZ "Build PCZ SceneManager plugin" TRUE)
option(OGRE_BUILD_PLUGIN_PFX "Build ParticleFX plugin" TRUE)
option(OGRE_BUILD_PLUGIN_CG "Build Cg plugin" TRUE)
option(OGRE_BUILD_CEGUIRENDERER "Build CEGUI renderer" TRUE)
option(OGRE_BUILD_SAMPLES "Build Ogre demos" TRUE)
option(OGRE_BUILD_TOOLS "Build the command-line tools" TRUE)
# Configure the build
option(OGRE_CONFIG_PROFILING "Enable Ogre profiling code" FALSE)
set(OGRE_CONFIG_ASSERTMODE 0 CACHE STRING 
"Set Ogre assertion mode. Possible values:
  0 - Regular asserts in debug, none in release
  1 - Regular asserts in debug, exceptions in release
  2 - Exceptions in debug and release"
)
option(OGRE_CONFIG_DOUBLE "Use doubles instead of floats in Ogre" FALSE)
set(OGRE_CONFIG_ALLOCATOR 2 CACHE STRING
"Specify the memory allocator to use. Possible values:
  1 - Standard allocator
  2 - nedmalloc
  3 - User-provided allocator"
)
option(OGRE_CONFIG_MEMTRACK_DEBUG "Enable Ogre's memory tracker in debug mode" FALSE)
option(OGRE_CONFIG_MEMTRACK_RELEASE "Enable Ogre's memory tracker in release mode" FALSE)
set(OGRE_CONFIG_THREADS 0 CACHE STRING 
"Enable Ogre thread support for background loading. Possible values:
  0 - Threading off.
  1 - Full background loading.
  2 - Background resource preparation."
)
option(OGRE_CONFIG_DISABLE_FREEIMAGE "Don't build FreeImage codec. If you use this option, you need to provide your own image handling codecs." FALSE)
option(OGRE_CONFIG_DISABLE_DDS "Don't build DDS codec." FALSE)
option(OGRE_CONFIG_NEW_COMPILERS "Use the new script compilers." TRUE)
# Customise what to install
option(OGRE_INSTALL_DOCS "Install documentation." TRUE)
option(OGRE_INSTALL_MEDIA "Install media files." FALSE)
option(OGRE_INSTALL_SAMPLES_SOURCE "Install samples source files." FALSE)


#####################################################################
# Set up the basic build environment
#####################################################################

# include Ogre version numbers
include(OgreVersion)

# Create debug libraries with _d postfix
set(CMAKE_DEBUG_POSTFIX "_d")

# Set compiler specific build flags
if (CMAKE_COMPILER_IS_GNUCXX)
  add_definitions(-msse)
endif ()
if (MSVC)
  add_definitions(/fp:fast)
endif ()

# Add include directory for automatically generated header files
include_directories("${OGRE3D_BINARY_DIR}/include")
# Add OgreMain include path
include_directories("${OGRE3D_SOURCE_DIR}/OgreMain/include")

# Find dependencies
include(Dependencies)
# definitions for samples
set(OGRE_LIBRARIES OgreMain) 
set(OGRE_CEGUI_LIBRARIES CEGUIOgreRenderer)
set(OGRE_PCZ_LIBRARIES Plugin_PCZSceneManager)
set(OGRE_PCZ_OCTREE_LIBRARIES Plugin_OctreeZone)

# Specify build paths
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OGRE3D_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OGRE3D_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OGRE3D_BINARY_DIR}/bin")


###################################################################
# configure global build settings based on selected build options
###################################################################
if (OGRE_STATIC)
  set(OGRE_LIB_TYPE STATIC)
else ()
  set(OGRE_LIB_TYPE SHARED)
endif ()
if (OGRE_CONFIG_THREADS)
  if (NOT Boost_FOUND)
    message(SEND_ERROR "Could not find dependency: boost-thread. Can't enable threading")
    set(OGRE_CONFIG_THREADS 0)
  else()
    if (UNIX)
      add_definitions(-pthread)
    endif()
  endif()
endif()

# Configure OgreConfig.h and OgrePrerequisites.h files
include(GenerateHeaders)


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

# Setup CEGUI renderer
if (OGRE_BUILD_CEGUIRENDERER)
  if (NOT CEGUI_FOUND)
    message(STATUS "Could not find dependency: CEGUI")
    message(STATUS "Skipping CEGUI renderer build")
    set(OGRE_BUILD_CEGUIRENDERER 0)
  else ()
    add_subdirectory(Samples/Common/CEGUIRenderer)
  endif ()
endif ()

# Setup samples
add_subdirectory(Samples) 

# Setup command-line tools
if (OGRE_BUILD_TOOLS)
  add_subdirectory(Tools)
endif ()

# Install documentation
add_subdirectory(Docs)

# Install media files
if (OGRE_INSTALL_MEDIA)
  add_subdirectory(Samples/Media)
endif ()

# Install CMake modules
add_subdirectory(CMake)

# Install dependencies
add_subdirectory(Dependencies)

# Provide CPack packaging target
include(Packaging)
