# Configure ParticleFX demo build

set(HEADER_FILES include/ParticleFX.h)
set(SOURCE_FILES src/ParticleFX.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(Demo_ParticleFX WIN32 ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(Demo_ParticleFX ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Demo_ParticleFX)

add_dependencies(Demo_ParticleFX ${SAMPLE_DEPENDENCIES})
