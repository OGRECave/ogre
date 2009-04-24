# Configure Shadows demo build

set(SOURCE_FILES src/Shadows.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Common/CEGUIRenderer/include)
add_executable(Demo_Shadows WIN32 ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(Demo_Shadows ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${CEGUI_LIBRARIES} ${OGRE_CEGUIRenderer_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Demo_Shadows)

if (SAMPLE_DEPENDENCIES)
  add_dependencies(Demo_Shadows ${SAMPLE_DEPENDENCIES})
endif ()
