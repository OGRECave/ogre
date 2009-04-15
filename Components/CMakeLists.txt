# Configure Components build
if (APPLE)
  include_directories("${CMAKE_SOURCE_DIR}/Mac/Ogre/include")
endif()

if (OGRE_BUILD_COMPONENT_PAGING)
  add_subdirectory(Paging)
endif ()

if (OGRE_BUILD_COMPONENT_PROPERTY)
  add_subdirectory(Property)
endif ()

if (OGRE_BUILD_COMPONENT_TERRAIN)
  add_subdirectory(Terrain)
endif ()
