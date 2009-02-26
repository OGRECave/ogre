# Configure Grass demo build

set(SOURCE_FILES src/Grass.cpp)

add_executable(Demo_Grass WIN32 ${SOURCE_FILES})
ogre_config_build(Demo_Grass)
target_link_libraries(Demo_Grass ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_Grass)
