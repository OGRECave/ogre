# Configure OctreeZone SceneManager build

set (HEADER_FILES
  include/OgreHeightmapTerrainZonePageSource.h
  include/OgreOctreeZone.h
  include/OgreOctreeZoneOctree.h
  include/OgreOctreeZonePlugin.h
  include/OgreOctreeZonePrerequisites.h
  include/OgreTerrainVertexProgram.h
  include/OgreTerrainZone.h
  include/OgreTerrainZonePage.h
  include/OgreTerrainZonePageSource.h
  include/OgreTerrainZonePrerequisites.h
  include/OgreTerrainZoneRenderable.h
)

set (SOURCE_FILES
  src/OgreHeightmapTerrainZonePageSource.cpp
  src/OgreOctreeZone.cpp
  src/OgreOctreeZoneDll.cpp
  src/OgreOctreeZoneOctree.cpp
  src/OgreOctreeZonePlugin.cpp
  src/OgreTerrainVertexProgram.cpp
  src/OgreTerrainZone.cpp
  src/OgreTerrainZonePage.cpp
  src/OgreTerrainZonePageSource.cpp
  src/OgreTerrainZoneRenderable.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../PCZSceneManager/include)
add_definitions(-D_USRDLL)

add_library(Plugin_OctreeZone ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})
ogre_config_build(Plugin_OctreeZone)
target_link_libraries(Plugin_OctreeZone OgreMain Plugin_PCZSceneManager)

if (NOT OGRE_STATIC)
  set_property(TARGET Plugin_OctreeZone PROPERTY
    COMPILE_DEFINITIONS OGRE_OCTREEZONEPLUGIN_EXPORTS OGRE_TERRAINZONEPLUGIN_EXPORTS)
endif ()
set_target_properties(Plugin_OctreeZone PROPERTIES PREFIX "")

ogre_install_plugin(Plugin_OctreeZone)
install(FILES ${HEADER_FILES} DESTINATION include/OGRE/PCZ)
