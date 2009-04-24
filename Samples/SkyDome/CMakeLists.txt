# Configure SkyDome demo build

set(HEADER_FILES include/SkyDome.h)
set(SOURCE_FILES src/SkyDome.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(Demo_SkyDome WIN32 ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(Demo_SkyDome ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Demo_SkyDome)

if (SAMPLE_DEPENDENCIES)
  add_dependencies(Demo_SkyDome ${SAMPLE_DEPENDENCIES})
endif ()
