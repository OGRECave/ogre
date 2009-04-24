# Configure CubeMapping demo build

set(HEADER_FILES include/CubeMapping.h)
set(SOURCE_FILES src/CubeMapping.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(Demo_CubeMapping WIN32 ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(Demo_CubeMapping ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Demo_CubeMapping)

if (SAMPLE_DEPENDENCIES)
  add_dependencies(Demo_CubeMapping ${SAMPLE_DEPENDENCIES})
endif ()
