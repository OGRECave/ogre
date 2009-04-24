# Configure CelShading demo build

set(SOURCE_FILES src/CelShading.cpp)

add_executable(Demo_CelShading WIN32 ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(Demo_CelShading ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Demo_CelShading)

if (SAMPLE_DEPENDENCIES)
  add_dependencies(Demo_CelShading ${SAMPLE_DEPENDENCIES})
endif ()
