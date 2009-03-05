# Configure Instancing demo build

set(HEADER_FILES include/Instancing.h)
set(SOURCE_FILES src/Instancing.cpp) 

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Common/CEGUIRenderer/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(Demo_Instancing WIN32 ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})
ogre_config_build(Demo_Instancing)
target_link_libraries(Demo_Instancing ${OGRE_LIBRARIES} ${CEGUI_LIBRARIES} ${OGRE_CEGUI_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_Instancing)

add_dependencies(Demo_Instancing ${SAMPLE_DEPENDENCIES})

