# Configure TextureFX demo build

set(HEADER_FILES include/TextureFX.h)
set(SOURCE_FILES src/TextureFX.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(Demo_TextureFX WIN32 ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(Demo_TextureFX ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_TextureFX)
