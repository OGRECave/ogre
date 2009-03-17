# Configure Fresnel demo build

set(SOURCE_FILES src/Fresnel.cpp)

add_executable(Demo_Fresnel WIN32 ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(Demo_Fresnel ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Demo_Fresnel)

add_dependencies(Demo_Fresnel ${SAMPLE_DEPENDENCIES})
