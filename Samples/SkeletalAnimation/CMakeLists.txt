# Configure SkeletalAnimation demo build

set(HEADER_FILES include/SkeletalAnimation.h)
set(SOURCE_FILES src/SkeletalAnimation.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(Demo_SkeletalAnimation WIN32 ${HEADER_FILES} ${SOURCE_FILES})
ogre_config_build(Demo_SkeletalAnimation)
target_link_libraries(Demo_SkeletalAnimation ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_SkeletalAnimation)
