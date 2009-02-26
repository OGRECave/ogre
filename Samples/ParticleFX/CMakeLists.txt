# Configure ParticleFX demo build

set(HEADER_FILES include/ParticleFX.h)
set(SOURCE_FILES src/ParticleFX.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(Demo_ParticleFX WIN32 ${HEADER_FILES} ${SOURCE_FILES})
ogre_config_build(Demo_ParticleFX)
target_link_libraries(Demo_ParticleFX ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_ParticleFX)
