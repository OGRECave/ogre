# Configure CameraTrack demo build

set(SOURCE_FILES src/CameraTrack.cpp)

add_executable(Demo_CameraTrack WIN32 ${SOURCE_FILES})
target_link_libraries(Demo_CameraTrack ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_CameraTrack)
