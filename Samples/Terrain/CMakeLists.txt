# Configure Terrain demo build

set(HEADER_FILES include/Terrain.h)
set(SOURCE_FILES src/Terrain.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(Demo_Terrain WIN32 ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(Demo_Terrain ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_Terrain)
