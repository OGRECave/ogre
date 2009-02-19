# Configure Lighting demo build

set(HEADER_FILES include/Lighting.h)
set(SOURCE_FILES src/Lighting.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(Demo_Lighting WIN32 ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(Demo_Lighting ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_Lighting)
