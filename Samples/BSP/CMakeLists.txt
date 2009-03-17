# Configure BSP demo build

set(SOURCE_FILES src/BSP.cpp)

add_executable(Demo_BSP WIN32 ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(Demo_BSP ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Demo_BSP)

add_dependencies(Demo_BSP ${SAMPLE_DEPENDENCIES})

