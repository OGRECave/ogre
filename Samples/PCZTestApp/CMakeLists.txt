# Configure PCZTestApp demo build

set(HEADER_FILES include/PCZTestApp.h include/RoomObject.h)
set(SOURCE_FILES src/PCZTestApp.cpp src/RoomObject.cpp)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../../PlugIns/OctreeZone/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../../PlugIns/PCZSceneManager/include
)
add_executable(Demo_PCZTestApp WIN32 ${HEADER_FILES} ${SOURCE_FILES})
ogre_config_build(Demo_PCZTestApp)
target_link_libraries(Demo_PCZTestApp ${OGRE_LIBRARIES} ${OGRE_PCZ_LIBRARIES} ${OGRE_PCZ_OCTREE_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_PCZTestApp)
