#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

######################################################################
# OGRE SAMPLES BUILD SYSTEM
######################################################################

cmake_minimum_required(VERSION 2.6)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)
cmake_policy(SET CMP0003 NEW)

project(OGRE)

# Find actual Ogre root
if (WIN32)
  get_filename_component(OGRE_ROOT_DIR "${OGRE_SOURCE_DIR}/../" ABSOLUTE)
  set(OGRE_TEMPLATES_DIR "${OGRE_ROOT_DIR}/cmake")
  set(OGRE_WORK_DIR "${OGRE_ROOT_DIR}")
  set(CMAKE_MODULE_PATH "${OGRE_ROOT_DIR}/cmake")
elseif (UNIX)
  get_filename_component(OGRE_ROOT_DIR "${OGRE_SOURCE_DIR}/../../../" ABSOLUTE)
  set(OGRE_TEMPLATES_DIR "${OGRE_ROOT_DIR}/lib/OGRE/cmake")
  set(OGRE_WORK_DIR "${OGRE_ROOT_DIR}")
  set(CMAKE_MODULE_PATH "${OGRE_ROOT_DIR}/lib/OGRE/cmake")
endif ()

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  #  By default, install to the Ogre SDK root
  SET(CMAKE_INSTALL_PREFIX
   "${OGRE_ROOT_DIR}" CACHE PATH "OGRE install prefix" FORCE
  )
endif ()

# Include necessary submodules
include(OgreConfigTargets)
set(CMAKE_PREFIX_PATH "${OGRE_ROOT_DIR}")
set(OGRE_INSTALL_SAMPLES TRUE)

set(OGRE_STATIC @OGRE_STATIC@)


#####################################################################
# Set up the basic build environment
#####################################################################

# Set compiler specific build flags
if (CMAKE_COMPILER_IS_GNUCXX)
  add_definitions(-msse)
endif ()
if (OGRE_BUILD_PLATFORM_IPHONE)
  remove_definitions(-msse)
  add_definitions(-fno-regmove)
endif ()
if (MSVC)
  add_definitions(/fp:fast)
endif ()

# Find dependencies
find_package(OGRE REQUIRED)
find_package(OIS REQUIRED)

include_directories(
  ${OGRE_INCLUDE_DIRS}
  ${OGRE_Plugin_PCZSceneManager_INCLUDE_DIRS}
  ${OGRE_Plugin_OctreeZone_INCLUDE_DIRS}
  ${OIS_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/Common/include
)

if (OGRE_STATIC)
  # need to link against all plugins
  if (OGRE_Plugin_BSPSceneManager_FOUND)
    include_directories(${OGRE_Plugin_BSPSceneManager_INCLUDE_DIRS})
    set(OGRE_PLUGIN_LIBRARIES ${OGRE_PLUGIN_LIBRARIES} ${OGRE_Plugin_BSPSceneManager_LIBRARIES})
  endif ()
  if (OGRE_Plugin_CgProgramManager_FOUND)
    include_directories(${OGRE_Plugin_CgProgramManager_INCLUDE_DIRS})
    set(OGRE_PLUGIN_LIBRARIES ${OGRE_PLUGIN_LIBRARIES} ${OGRE_Plugin_CgProgramManager_LIBRARIES})
  endif ()
  if (OGRE_Plugin_OctreeSceneManager_FOUND)
    include_directories(${OGRE_Plugin_OctreeSceneManager_INCLUDE_DIRS})
    set(OGRE_PLUGIN_LIBRARIES ${OGRE_PLUGIN_LIBRARIES} ${OGRE_Plugin_OctreeSceneManager_LIBRARIES})
  endif ()
  if (OGRE_Plugin_ParticleFX_FOUND)
    include_directories(${OGRE_Plugin_ParticleFX_INCLUDE_DIRS})
    set(OGRE_PLUGIN_LIBRARIES ${OGRE_PLUGIN_LIBRARIES} ${OGRE_Plugin_ParticleFX_LIBRARIES})
  endif ()
  if (OGRE_RenderSystem_Direct3D9_FOUND)
    include_directories(${OGRE_RenderSystem_Direct3D9_INCLUDE_DIRS})
    set(OGRE_PLUGIN_LIBRARIES ${OGRE_PLUGIN_LIBRARIES} ${OGRE_RenderSystem_Direct3D9_LIBRARIES})
  endif ()
  if (OGRE_RenderSystem_Direct3D10_FOUND)
    include_directories(${OGRE_RenderSystem_Direct3D10_INCLUDE_DIRS})
    set(OGRE_PLUGIN_LIBRARIES ${OGRE_PLUGIN_LIBRARIES} ${OGRE_RenderSystem_Direct3D10_LIBRARIES})
  endif ()
  if (OGRE_RenderSystem_GL_FOUND)
    include_directories(${OGRE_RenderSystem_GL_INCLUDE_DIRS})
    set(OGRE_PLUGIN_LIBRARIES ${OGRE_PLUGIN_LIBRARIES} ${OGRE_RenderSystem_GL_LIBRARIES})
  endif ()
endif ()

# Configure Samples build
add_subdirectory(BezierPatch)
if (OGRE_Plugin_BSPSceneManager_FOUND)
  add_subdirectory(BSP)
endif ()
add_subdirectory(CameraTrack)
add_subdirectory(CubeMapping)
add_subdirectory(DeferredShading)
add_subdirectory(DynTex)
add_subdirectory(EnvMapping)
add_subdirectory(Grass)
add_subdirectory(Lighting)
if (OGRE_Paging_FOUND)
  add_subdirectory(Paging)
endif ()
if (OGRE_Plugin_ParticleFX_FOUND)
  add_subdirectory(ParticleFX)
endif ()
if (OGRE_Plugin_PCZSceneManager_FOUND AND OGRE_Plugin_OctreeZone_FOUND)
  add_subdirectory(PCZTestApp)
endif ()
add_subdirectory(RenderToTexture)
add_subdirectory(SkeletalAnimation)
add_subdirectory(SkyBox)
add_subdirectory(SkyDome)
add_subdirectory(SkyPlane)
add_subdirectory(Smoke)
if (OGRE_Plugin_OctreeSceneManager_FOUND)
  add_subdirectory(Terrain)
endif ()
add_subdirectory(TextureFX)
add_subdirectory(Transpacency)

# Require vertex and fragment shaders
add_subdirectory(CelShading)
add_subdirectory(Dot3Bump)
add_subdirectory(Fresnel)

# Require geometry shaders
add_subdirectory(Isosurf)
add_subdirectory(ParticleGS)

# Requires 3D textures
add_subdirectory(VolumeTex)

# Requires cube mapping
add_subdirectory(Water)

add_subdirectory(Compositor)
add_subdirectory(FacialAnimation)
add_subdirectory(Gui)
add_subdirectory(Instancing)
add_subdirectory(OceanDemo)
add_subdirectory(Shadows)
