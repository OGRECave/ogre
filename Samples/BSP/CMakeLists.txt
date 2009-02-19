# Configure BSP demo build

set(SOURCE_FILES src/BSP.cpp)

add_executable(Demo_BSP WIN32 ${SOURCE_FILES})
target_link_libraries(Demo_BSP ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_BSP)
