# Configure Tools build
if (APPLE)
  include_directories("${CMAKE_SOURCE_DIR}/Mac/Ogre/include")
endif()

add_subdirectory(XMLConverter)
add_subdirectory(MeshUpgrader)

