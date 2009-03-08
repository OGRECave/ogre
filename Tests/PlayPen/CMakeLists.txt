# Configure Bezier demo build

set(HEADER_FILES 
	include/AnimationBlender.h 
	include/StaticPluginLoader.h)
set(SOURCE_FILES 
	src/PlayPen.cpp
	src/AnimationBlender.cpp
	src/WindowEmbedding.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/Samples/Common/include)
add_executable(PlayPen WIN32 ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES} )
ogre_config_build(PlayPen)
target_link_libraries(PlayPen ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(PlayPen)

add_dependencies(PlayPen ${TEST_DEPENDENCIES})

