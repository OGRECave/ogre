# Configure MeshUpgrader

set(SOURCE_FILES 
  src/main.cpp
)

add_executable(OgreMeshUpgrader ${SOURCE_FILES})
ogre_config_build(OgreMeshUpgrader)
target_link_libraries(OgreMeshUpgrader ${OGRE_LIBRARIES})
ogre_install_sample(OgreMeshUpgrader)
