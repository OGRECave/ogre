#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

############################################################
# Paging optional component
############################################################

PROJECT(OgreTerrain)

# define header and source files for the library
set (HEADER_FILES
    include/OgreTerrain.h
    include/OgreTerrainGroup.h
	include/OgreTerrainLayerBlendMap.h
	include/OgreTerrainMaterialGenerator.h
	include/OgreTerrainMaterialGeneratorA.h
	include/OgreTerrainPaging.h
	include/OgreTerrainPagedWorldSection.h
	include/OgreTerrainPrerequisites.h
    include/OgreTerrainQuadTreeNode.h
)

set (SOURCE_FILES
	src/OgreTerrain.cpp
    src/OgreTerrainGroup.cpp
	src/OgreTerrainLayerBlendMap.cpp
	src/OgreTerrainMaterialGenerator.cpp
	src/OgreTerrainMaterialGeneratorA.cpp
	src/OgreTerrainPaging.cpp
	src/OgreTerrainPagedWorldSection.cpp
	src/OgreTerrainQuadTreeNode.cpp
)


# Add needed definitions 
add_definitions(-DOGRE_TERRAIN_EXPORTS -D_MT -D_USRDLL)

if (OGRE_BUILD_COMPONENT_PAGING)
  set (HEADER_FILES ${HEADER_FILES}
	include/OgreTerrainPaging.h
  )
  set (SOURCE_FILES ${SOURCE_FILES}
	src/OgreTerrainPaging.cpp
  )
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Paging/include)
endif ()

# include headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${OGRE_SOURCE_DIR}/OgreMain/include)

# setup target
add_library(OgreTerrain STATIC ${HEADER_FILES} ${SOURCE_FILES} ${PLATFORM_HEADER_FILES} ${PLATFORM_SOURCE_FILES})
set_target_properties(OgreTerrain PROPERTIES VERSION ${OGRE_VERSION} SOVERSION ${OGRE_VERSION_MAJOR})
target_link_libraries(OgreTerrain OgreMain)
if (OGRE_BUILD_COMPONENT_PAGING)
	target_link_libraries(OgreTerrain OgrePaging)
endif ()
if (OGRE_CONFIG_THREADS)
  target_link_libraries(OgreTerrain ${Boost_LIBRARIES})
endif ()


# install 
ogre_config_component(OgreTerrain)
install(FILES ${HEADER_FILES}
  DESTINATION include/OGRE/Terrain
)

