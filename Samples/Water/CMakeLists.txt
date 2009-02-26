# Configure Water demo build

set(HEADER_FILES include/WaterMesh.h)
set(SOURCE_FILES src/Water.cpp src/WaterMesh.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(Demo_Water WIN32 ${HEADER_FILES} ${SOURCE_FILES})
ogre_config_build(Demo_Water)
target_link_libraries(Demo_Water ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_Water)
