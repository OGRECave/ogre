#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

############################################################
# Terrain optional component
############################################################

PROJECT(OgreTerrain)

# define header and source files for the library
file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

list(REMOVE_ITEM HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/OgreTerrainPagedWorldSection.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/OgreTerrainPaging.h"
)
list(REMOVE_ITEM SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreTerrainPagedWorldSection.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreTerrainPaging.cpp"
)

# Add needed definitions 
add_definitions(-DOGRE_TERRAIN_EXPORTS -D_MT -D_USRDLL)

if (OGRE_BUILD_COMPONENT_PAGING)
  list(APPEND HEADER_FILES
    include/OgreTerrainPaging.h
	include/OgreTerrainPagedWorldSection.h
  )
  list(APPEND SOURCE_FILES
	src/OgreTerrainPaging.cpp
	src/OgreTerrainPagedWorldSection.cpp
  )
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Paging/include)
endif ()

# include headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${OGRE_SOURCE_DIR}/OgreMain/include)

# setup target
ogre_add_library(OgreTerrain ${OGRE_COMP_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES} ${PLATFORM_HEADER_FILES} ${PLATFORM_SOURCE_FILES})
set_target_properties(OgreTerrain PROPERTIES VERSION ${OGRE_SOVERSION} SOVERSION ${OGRE_SOVERSION})
target_link_libraries(OgreTerrain OgreMain)
if (OGRE_BUILD_COMPONENT_PAGING)
	target_link_libraries(OgreTerrain OgrePaging)
endif ()
if (OGRE_CONFIG_THREADS)
  target_link_libraries(OgreTerrain ${OGRE_THREAD_LIBRARIES})
endif ()


# install 
ogre_config_framework(OgreTerrain)
ogre_config_component(OgreTerrain)

install(FILES ${HEADER_FILES}
  DESTINATION include/OGRE/Terrain
)

