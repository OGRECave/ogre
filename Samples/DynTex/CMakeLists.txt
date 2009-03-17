# Configure DynTex demo build

set(SOURCE_FILES src/DynTex.cpp)

add_executable(Demo_DynTex WIN32 ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(Demo_DynTex ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Demo_DynTex)

add_dependencies(Demo_DynTex ${SAMPLE_DEPENDENCIES})
