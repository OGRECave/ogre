# Configure MeshUpgrader

set(SOURCE_FILES 
  src/main.cpp
)

add_executable(OgreMeshUpgrader ${SOURCE_FILES})
target_link_libraries(OgreMeshUpgrader ${OGRE_LIBRARIES})
ogre_config_tool(OgreMeshUpgrader)
