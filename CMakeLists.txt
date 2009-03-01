######################################################################
# OGRE BUILD SYSTEM
# Welcome to the CMake build system for OGRE.
# This is the main file where we prepare the general build environment
# and provide build configuration options.
######################################################################

cmake_minimum_required(VERSION 2.6)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)
cmake_policy(SET CMP0003 NEW)

project(OGRE)

# Include necessary submodules
set(CMAKE_MODULE_PATH 
  "${OGRE_SOURCE_DIR}/CMake"
  "${OGRE_SOURCE_DIR}/CMake/Utils" 
  "${OGRE_SOURCE_DIR}/CMake/Packages"
)
include(CMakeDependentOption)
include(MacroLogFeature)
include(OgreInstall)
include(OgreConfigBuild)
include(PreprocessorUtils)
set(OGRE_TEMPLATES_DIR "${OGRE_SOURCE_DIR}/CMake/Templates")
set(OGRE_WORK_DIR ${OGRE_BINARY_DIR})


#####################################################################
# Set up the basic build environment
#####################################################################

# determine Ogre version numbers
include(OgreGetVersion)
ogre_get_version(${OGRE_SOURCE_DIR}/OgreMain/include/OgrePrerequisites.h)

# Create debug libraries with _d postfix
set(CMAKE_DEBUG_POSTFIX "_d")

# Set compiler specific build flags
if (CMAKE_COMPILER_IS_GNUCXX)
  add_definitions(-msse)
endif ()
if (MSVC)
  add_definitions(/fp:fast)
endif ()

# Add OgreMain include path
include_directories("${OGRE_SOURCE_DIR}/OgreMain/include")
include_directories("${OGRE_BINARY_DIR}/include")

# Find dependencies
include(Dependencies)
# definitions for samples
set(OGRE_LIBRARIES OgreMain) 
set(OGRE_CEGUI_LIBRARIES CEGUIOgreRenderer)
set(OGRE_PCZ_LIBRARIES Plugin_PCZSceneManager)
set(OGRE_PCZ_OCTREE_LIBRARIES Plugin_OctreeZone)

# Specify build paths
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OGRE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OGRE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OGRE_BINARY_DIR}/bin")


######################################################################
# Provide user options to customise the build process
######################################################################

# Customise what to build
option(OGRE_STATIC "Static build" FALSE)
cmake_dependent_option(OGRE_BUILD_RENDERSYSTEM_D3D9 "Build Direct3D9 RenderSystem" TRUE "WIN32;DirectX_FOUND" FALSE)
cmake_dependent_option(OGRE_BUILD_RENDERSYSTEM_D3D10 "Build Direct3D10 RenderSystem" TRUE "WIN32;DirectX_FOUND" FALSE)
cmake_dependent_option(OGRE_BUILD_RENDERSYSTEM_GL "Build OpenGL RenderSystem" TRUE "OPENGL_FOUND" FALSE)
option(OGRE_BUILD_PLUGIN_BSP "Build BSP SceneManager plugin" TRUE)
option(OGRE_BUILD_PLUGIN_OCTREE "Build Octree SceneManager plugin" TRUE)
option(OGRE_BUILD_PLUGIN_PCZ "Build PCZ SceneManager plugin" TRUE)
option(OGRE_BUILD_PLUGIN_PFX "Build ParticleFX plugin" TRUE)
cmake_dependent_option(OGRE_BUILD_PLUGIN_CG "Build Cg plugin" TRUE "Cg_FOUND" FALSE)
cmake_dependent_option(OGRE_BUILD_CEGUIRENDERER "Build CEGUI renderer" TRUE "CEGUI_FOUND" FALSE)
option(OGRE_BUILD_SAMPLES "Build Ogre demos" TRUE)
option(OGRE_BUILD_TOOLS "Build the command-line tools" TRUE)
# Configure the build
#option(OGRE_CONFIG_PROFILING "Enable Ogre profiling code" FALSE)
#set(OGRE_CONFIG_ASSERTMODE 0 CACHE STRING 
#"Set Ogre assertion mode. Possible values:
#  0 - Regular asserts in debug, none in release
#  1 - Regular asserts in debug, exceptions in release
#  2 - Exceptions in debug and release"
#)
option(OGRE_CONFIG_DOUBLE "Use doubles instead of floats in Ogre" FALSE)
set(OGRE_CONFIG_ALLOCATOR 2 CACHE STRING
"Specify the memory allocator to use. Possible values:
  1 - Standard allocator
  2 - nedmalloc
  3 - User-provided allocator"
)
option(OGRE_CONFIG_CONTAINERS_USE_CUSTOM_ALLOCATOR "STL containers in Ogre use the custom allocator" TRUE)
option(OGRE_CONFIG_STRING_USE_CUSTOM_ALLOCATOR "Ogre String uses the custom allocator" FALSE)
option(OGRE_CONFIG_MEMTRACK_DEBUG "Enable Ogre's memory tracker in debug mode" FALSE)
option(OGRE_CONFIG_MEMTRACK_RELEASE "Enable Ogre's memory tracker in release mode" FALSE)
if (Boost_FOUND)
  set(OGRE_CONFIG_THREADS 0 CACHE STRING 
  "Enable Ogre thread support for background loading. Possible values:
    0 - Threading off.
    1 - Full background loading.
    2 - Background resource preparation."
  )
else ()
  set(OGRE_CONFIG_THREADS 0)
endif ()
cmake_dependent_option(OGRE_CONFIG_DISABLE_FREEIMAGE "Don't build FreeImage codec. If you use this option, you need to provide your own image handling codecs." FALSE "FreeImage_FOUND" TRUE)
option(OGRE_CONFIG_DISABLE_DDS "Don't build DDS codec." FALSE)
option(OGRE_CONFIG_NEW_COMPILERS "Use the new script compilers." TRUE)
# Customise what to install
option(OGRE_INSTALL_SAMPLES "Install Ogre demos." FALSE)
option(OGRE_INSTALL_DOCS "Install documentation." FALSE)
option(OGRE_INSTALL_MEDIA "Install media files." FALSE)
option(OGRE_INSTALL_SAMPLES_SOURCE "Install samples source files." FALSE)




###################################################################
# configure global build settings based on selected build options
###################################################################
include(ConfigureBuild)


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
  add_subdirectory(Samples/Common/CEGUIRenderer)
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
