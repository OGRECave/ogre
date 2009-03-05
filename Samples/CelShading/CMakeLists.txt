# Configure CelShading demo build

set(SOURCE_FILES src/CelShading.cpp)

add_executable(Demo_CelShading WIN32 ${SOURCE_FILES} ${RESOURCE_FILES})
ogre_config_build(Demo_CelShading)
target_link_libraries(Demo_CelShading ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_CelShading)

add_dependencies(Demo_CelShading ${SAMPLE_DEPENDENCIES})
