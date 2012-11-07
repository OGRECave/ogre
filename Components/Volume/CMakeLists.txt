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
	include/OgreVolumeCSGSource.h
	include/OgreVolumeDualCell.h
	include/OgreVolumeDualGridGenerator.h
	include/OgreVolumeIsoSurface.h
	include/OgreVolumeIsoSurfaceMC.h
	include/OgreVolumeIsoSurfaceTablesMC.h
	include/OgreVolumeMeshBuilder.h
	include/OgreVolumeTextureSource.h
	include/OgreVolumeChunk.h
	include/OgreVolumeOctreeNode.h
	include/OgreVolumeOctreeNodeSplitPolicy.h
	include/OgreVolumeSimplexNoise.h
	include/OgreVolumeSource.h
	include/OgreVolumePrerequisites.h
)
set(SOURCE_FILES
	src/OgreVolumeCacheSource.cpp
	src/OgreVolumeCSGSource.cpp
	src/OgreVolumeDualCell.cpp
	src/OgreVolumeDualGridGenerator.cpp
	src/OgreVolumeIsoSurface.cpp
	src/OgreVolumeIsoSurfaceMC.cpp
	src/OgreVolumeMeshBuilder.cpp
	src/OgreVolumeTextureSource.cpp
	src/OgreVolumeChunk.cpp
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
set_target_properties(OgreVolume PROPERTIES VERSION ${OGRE_SOVERSION})
target_link_libraries(OgreVolume OgreMain)
if (OGRE_CONFIG_THREADS)
  target_link_libraries(OgreVolume ${Boost_LIBRARIES})
endif ()


# install 
ogre_config_component(OgreVolume)

if (APPLE AND NOT OGRE_BUILD_PLATFORM_APPLE_IOS)
    # Set the INSTALL_PATH so that Components can be installed in the application package
    set_target_properties(OgreVolume
       PROPERTIES BUILD_WITH_INSTALL_RPATH 1
       INSTALL_NAME_DIR "@executable_path/../Components"
    )

	# Copy headers into the main Ogre framework
	add_custom_command(TARGET OgreVolume POST_BUILD
	  COMMAND ditto ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)/Ogre.framework/Headers/
	)
endif()

install(FILES ${HEADER_FILES}
  DESTINATION include/OGRE/Volume
)

