# Configure EnvMapping demo build

set(HEADER_FILES include/EnvMapping.h)
set(SOURCE_FILES src/EnvMapping.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(Demo_EnvMapping WIN32 ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(Demo_EnvMapping ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_EnvMapping)
