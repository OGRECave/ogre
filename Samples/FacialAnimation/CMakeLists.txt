# Configure FacialAnimation demo build

set(SOURCE_FILES src/FacialAnimation.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Common/CEGUIRenderer/include)
add_executable(Demo_FacialAnimation WIN32 ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(Demo_FacialAnimation ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${CEGUI_LIBRARIES} ${OGRE_CEGUIRenderer_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Demo_FacialAnimation)

if (SAMPLE_DEPENDENCIES)
  add_dependencies(Demo_FacialAnimation ${SAMPLE_DEPENDENCIES})
endif ()
