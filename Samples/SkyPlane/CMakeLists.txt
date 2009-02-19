# Configure SkyPlane demo build

set(HEADER_FILES include/SkyPlane.h)
set(SOURCE_FILES src/SkyPlane.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(Demo_SkyPlane WIN32 ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(Demo_SkyPlane ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_SkyPlane)
