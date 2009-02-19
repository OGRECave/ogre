# Configure Bezier demo build

set(HEADER_FILES include/Bezier.h)
set(SOURCE_FILES src/Bezier.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(Demo_Bezier WIN32 ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(Demo_Bezier ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_Bezier)
