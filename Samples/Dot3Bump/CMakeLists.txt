# Configure Dot3Bump demo build

set(SOURCE_FILES src/Dot3Bump.cpp)

add_executable(Demo_Dot3Bump WIN32 ${SOURCE_FILES} ${RESOURCE_FILES})
ogre_config_build(Demo_Dot3Bump)
target_link_libraries(Demo_Dot3Bump ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_Dot3Bump)

add_dependencies(Demo_Dot3Bump ${SAMPLE_DEPENDENCIES})
