# Configure Shadows demo build

set(SOURCE_FILES src/Shadows.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Common/CEGUIRenderer/include)
add_executable(Demo_Shadows WIN32 ${SOURCE_FILES})
ogre_config_build(Demo_Shadows)
target_link_libraries(Demo_Shadows ${OGRE_LIBRARIES} ${CEGUI_LIBRARIES} ${OGRE_CEGUI_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_Shadows)
