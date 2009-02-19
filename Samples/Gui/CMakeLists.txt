# Configure Gui demo build

set(SOURCE_FILES src/Gui.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Common/CEGUIRenderer/include)
add_executable(Demo_Gui WIN32 ${SOURCE_FILES})
target_link_libraries(Demo_Gui ${OGRE_LIBRARIES} ${CEGUI_LIBRARIES} ${OGRE_CEGUI_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_Gui)
