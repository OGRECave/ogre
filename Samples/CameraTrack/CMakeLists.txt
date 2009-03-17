# Configure CameraTrack demo build

set(SOURCE_FILES src/CameraTrack.cpp)

add_executable(Demo_CameraTrack WIN32 ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(Demo_CameraTrack ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Demo_CameraTrack)

add_dependencies(Demo_CameraTrack ${SAMPLE_DEPENDENCIES})
