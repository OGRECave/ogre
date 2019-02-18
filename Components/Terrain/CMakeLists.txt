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
add_definitions(-DOGRE_TERRAIN_EXPORTS ${OGRE_VISIBILITY_FLAGS})

if (OGRE_BUILD_COMPONENT_PAGING)
  list(APPEND HEADER_FILES
    include/OgreTerrainPaging.h
	include/OgreTerrainPagedWorldSection.h
  )
  list(APPEND SOURCE_FILES
	src/OgreTerrainPaging.cpp
	src/OgreTerrainPagedWorldSection.cpp
  )
endif ()

# setup target
add_library(OgreTerrain ${OGRE_COMP_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES} ${PLATFORM_HEADER_FILES} ${PLATFORM_SOURCE_FILES})
set_target_properties(OgreTerrain PROPERTIES VERSION ${OGRE_SOVERSION} SOVERSION ${OGRE_SOVERSION})
target_link_libraries(OgreTerrain PUBLIC OgreMain)

target_include_directories(OgreTerrain PUBLIC 
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  $<INSTALL_INTERFACE:include/OGRE/Terrain>)

if (OGRE_BUILD_COMPONENT_PAGING)
	target_link_libraries(OgreTerrain PUBLIC OgrePaging)
endif ()


# install 
ogre_config_framework(OgreTerrain)
ogre_config_component(OgreTerrain)

install(FILES ${HEADER_FILES}
  DESTINATION include/OGRE/Terrain
)
install(DIRECTORY "${PROJECT_SOURCE_DIR}/Media/Terrain"
  DESTINATION "${OGRE_MEDIA_PATH}/"
)