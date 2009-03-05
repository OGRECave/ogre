# Configure Smoke demo build

set(HEADER_FILES include/Smoke.h)
set(SOURCE_FILES src/Smoke.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(Demo_Smoke WIN32 ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})
ogre_config_build(Demo_Smoke)
target_link_libraries(Demo_Smoke ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_Smoke)

add_dependencies(Demo_Smoke ${SAMPLE_DEPENDENCIES})
