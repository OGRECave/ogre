# Configure Terrain demo build

set(HEADER_FILES include/Terrain.h)
set(SOURCE_FILES src/Terrain.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(Demo_Terrain WIN32 ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(Demo_Terrain ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Demo_Terrain)

if (SAMPLE_DEPENDENCIES)
  add_dependencies(Demo_Terrain ${SAMPLE_DEPENDENCIES})
endif ()
