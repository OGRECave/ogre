# Configure SkyDome demo build

set(HEADER_FILES include/SkyDome.h)
set(SOURCE_FILES src/SkyDome.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(Demo_SkyDome WIN32 ${HEADER_FILES} ${SOURCE_FILES})
ogre_config_build(Demo_SkyDome)
target_link_libraries(Demo_SkyDome ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_SkyDome)
