# Configure Gui demo build

set(SOURCE_FILES src/Gui.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Common/CEGUIRenderer/include)
add_executable(Demo_Gui WIN32 ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(Demo_Gui ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${CEGUI_LIBRARIES} ${OGRE_CEGUIRenderer_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Demo_Gui)

if (SAMPLE_DEPENDENCIES)
  add_dependencies(Demo_Gui ${SAMPLE_DEPENDENCIES})
endif ()
