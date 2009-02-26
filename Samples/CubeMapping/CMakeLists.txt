# Configure CubeMapping demo build

set(HEADER_FILES include/CubeMapping.h)
set(SOURCE_FILES src/CubeMapping.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(Demo_CubeMapping WIN32 ${HEADER_FILES} ${SOURCE_FILES})
ogre_config_build(Demo_CubeMapping)
target_link_libraries(Demo_CubeMapping ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_CubeMapping)
