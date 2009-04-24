# Configure Grass demo build

set(SOURCE_FILES src/Grass.cpp)

add_executable(Demo_Grass WIN32 ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(Demo_Grass ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Demo_Grass)

if (SAMPLE_DEPENDENCIES)
  add_dependencies(Demo_Grass ${SAMPLE_DEPENDENCIES})
endif ()
