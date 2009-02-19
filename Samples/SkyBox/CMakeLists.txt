# Configure SkyBox demo build

set(HEADER_FILES include/SkyBox.h)
set(SOURCE_FILES src/SkyBox.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(Demo_SkyBox WIN32 ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(Demo_SkyBox ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_SkyBox)
