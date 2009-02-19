# Configure CelShading demo build

set(SOURCE_FILES src/CelShading.cpp)

add_executable(Demo_CelShading WIN32 ${SOURCE_FILES})
target_link_libraries(Demo_CelShading ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_CelShading)
