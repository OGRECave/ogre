######################################################################
# OGRE SAMPLES BUILD SYSTEM
######################################################################

cmake_minimum_required(VERSION 2.6)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)
cmake_policy(SET CMP0003 NEW)

project(OGRE3D)

# Include necessary submodules
set(CMAKE_MODULE_PATH 
  "${OGRE3D_SOURCE_DIR}/../cmake"
  "${OGRE3D_SOURCE_DIR}/../../../lib/cmake"
)
include(OgreInstall)
set(CMAKE_PREFIX_PATH 
  "${OGRE3D_SOURCE_DIR}/../"
  "${OGRE3D_SOURCE_DIR}/../../../"
)


#####################################################################
# Set up the basic build environment
#####################################################################

# Set compiler specific build flags
if (CMAKE_COMPILER_IS_GNUCXX)
  add_definitions(-msse)
endif ()
if (MSVC)
  add_definitions(/fp:fast)
endif ()

# Find dependencies
find_package(OGRE REQUIRED)
find_package(OIS REQUIRED)
find_package(CEGUI)

include_directories(
  ${OGRE_INCLUDE_DIRS}
  ${OGRE_PCZ_INCLUDE_DIRS}
  ${OGRE_PCZ_OCTREE_INCLUDE_DIRS}
  ${OIS_INCLUDE_DIRS}
  ${CEGUI_INCLUDE_DIRS}
  ${OGRE_CEGUI_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/Common/include
)

# Specify build paths
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OGRE3D_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OGRE3D_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OGRE3D_BINARY_DIR}/bin")


# Configure Samples build

add_subdirectory(BezierPatch)
add_subdirectory(BSP)
add_subdirectory(CameraTrack)
add_subdirectory(CelShading)
add_subdirectory(CubeMapping)
add_subdirectory(DeferredShading)
add_subdirectory(Dot3Bump)
add_subdirectory(DynTex)
add_subdirectory(EnvMapping)
add_subdirectory(Fresnel)
add_subdirectory(Grass)
add_subdirectory(Isosurf)
add_subdirectory(Lighting)
add_subdirectory(ParticleFX)
add_subdirectory(ParticleGS)
if (NOT OGRE_PCZ_FOUND OR NOT OGRE_PCZ_OCTREE_FOUND)
  message(STATUS "The PCZ SceneManager or the OctreeZone plugin is not available")
  message(STATUS "Skipping PCZTestApp")
else ()
  add_subdirectory(PCZTestApp)
endif ()
add_subdirectory(RenderToTexture)
add_subdirectory(SkeletalAnimation)
add_subdirectory(SkyBox)
add_subdirectory(SkyDome)
add_subdirectory(SkyPlane)
add_subdirectory(Smoke)
add_subdirectory(Terrain)
add_subdirectory(TextureFX)
add_subdirectory(Transpacency)
add_subdirectory(VolumeTex)
add_subdirectory(Water)

if (NOT CEGUI_FOUND)
  message(STATUS "Could not find dependency: CEGUI")
  message(STATUS "Skipping GUI samples build")
elseif (NOT OGRE_CEGUI_FOUND)
  message(STATUS "CEGUI OGRE renderer was not found")
  message(STATUS "Skipping GUI samples build")
else ()
  add_subdirectory(Compositor)
  add_subdirectory(FacialAnimation)
  add_subdirectory(Gui)
  add_subdirectory(Instancing)
  add_subdirectory(OceanDemo)
  add_subdirectory(Shadows)
endif ()
