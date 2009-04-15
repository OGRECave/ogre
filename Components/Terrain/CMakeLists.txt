############################################################
# Paging optional component
############################################################

PROJECT(OgreTerrain)

# define header and source files for the library
set (HEADER_FILES
    include/OgreTerrain.h
	include/OgreTerrainPrerequisites.h
)

set (SOURCE_FILES
	src/OgreTerrain.cpp
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
include_directories(${CMAKE_SOURCE_DIR}/OgreMain/include)


# setup target
add_library(OgreTerrain ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES} ${PLATFORM_HEADER_FILES} ${PLATFORM_SOURCE_FILES})
set_target_properties(OgreTerrain PROPERTIES VERSION ${OGRE_VERSION} SOVERSION ${OGRE_VERSION_MAJOR})
target_link_libraries(OgreTerrain OgreMain)
if (OGRE_BUILD_COMPONENT_PAGING)
	target_link_libraries(OgreTerrain OgrePaging)
endif ()
if (OGRE_CONFIG_THREADS)
  target_link_libraries(OgreTerrain ${Boost_LIBRARIES})
endif ()

# install 
ogre_config_lib(OgreTerrain)
install(FILES ${HEADER_FILES}
  DESTINATION include/OGRE/Terrain
)

