#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

############################################################
# Volume optional component
############################################################

PROJECT(OgreVolume)

# define header and source files for the library
set (HEADER_FILES
	include/OgreVolumeCacheSource.h
	include/OgreVolumeChunk.h
	include/OgreVolumeChunkHandler.h
	include/OgreVolumeCSGSource.h
	include/OgreVolumeDualGridGenerator.h
	include/OgreVolumeGridSource.h
	include/OgreVolumeHalfFloatGridSource.h
	include/OgreVolumeIsoSurface.h
	include/OgreVolumeIsoSurfaceMC.h
	include/OgreVolumeIsoSurfaceTablesMC.h
	include/OgreVolumeMeshBuilder.h
	include/OgreVolumeTextureSource.h
	include/OgreVolumeOctreeNode.h
	include/OgreVolumeOctreeNodeSplitPolicy.h
	include/OgreVolumeSimplexNoise.h
	include/OgreVolumeSource.h
	include/OgreVolumePrerequisites.h
)
set(SOURCE_FILES
	src/OgreVolumeCacheSource.cpp
	src/OgreVolumeChunk.cpp
	src/OgreVolumeChunkHandler.cpp
	src/OgreVolumeCSGSource.cpp
	src/OgreVolumeDualGridGenerator.cpp
	src/OgreVolumeGridSource.cpp
	src/OgreVolumeHalfFloatGridSource.cpp
	src/OgreVolumeIsoSurface.cpp
	src/OgreVolumeIsoSurfaceMC.cpp
	src/OgreVolumeMeshBuilder.cpp
	src/OgreVolumeTextureSource.cpp
	src/OgreVolumeOctreeNode.cpp
	src/OgreVolumeOctreeNodeSplitPolicy.cpp
	src/OgreVolumeSimplexNoise.cpp
	src/OgreVolumeSource.cpp
)


# Add needed definitions 
add_definitions(-DOGRE_VOLUME_EXPORTS -D_MT -D_USRDLL)

# include headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${OGRE_SOURCE_DIR}/OgreMain/include)

# setup target
ogre_add_library(OgreVolume ${OGRE_COMP_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES} ${PLATFORM_HEADER_FILES} ${PLATFORM_SOURCE_FILES})
set_target_properties(OgreVolume PROPERTIES VERSION ${OGRE_SOVERSION} SOVERSION ${OGRE_SOVERSION})
target_link_libraries(OgreVolume OgreMain)
if (OGRE_CONFIG_THREADS)
  target_link_libraries(OgreVolume ${OGRE_THREAD_LIBRARIES})
endif ()


# install 
ogre_config_framework(OgreVolume)
ogre_config_component(OgreVolume)

install(FILES ${HEADER_FILES}
  DESTINATION include/OGRE/Volume
)

