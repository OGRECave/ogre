# Configure PCZTestApp demo build

set(HEADER_FILES include/PCZTestApp.h include/RoomObject.h)
set(SOURCE_FILES src/PCZTestApp.cpp src/RoomObject.cpp)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../../PlugIns/OctreeZone/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../../PlugIns/PCZSceneManager/include
)
add_executable(Demo_PCZTestApp WIN32 ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(Demo_PCZTestApp ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${OGRE_Plugin_PCZSceneManager_LIBRARIES} ${OGRE_Plugin_OctreeZone_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Demo_PCZTestApp)
# need to include plugins directory in RPATH for the demo to be able to find the PCZ libs
set_property(TARGET Demo_PCZTestApp APPEND PROPERTY
  INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib/OGRE
)

if (SAMPLE_DEPENDENCIES)
  add_dependencies(Demo_PCZTestApp ${SAMPLE_DEPENDENCIES})
endif ()
