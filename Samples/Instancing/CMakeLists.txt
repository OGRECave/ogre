# Configure Instancing demo build

set(HEADER_FILES include/Instancing.h)
set(SOURCE_FILES src/Instancing.cpp) 

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Common/CEGUIRenderer/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(Demo_Instancing WIN32 ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(Demo_Instancing ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${CEGUI_LIBRARIES} ${OGRE_CEGUIRenderer_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Demo_Instancing)

if (SAMPLE_DEPENDENCIES)
  add_dependencies(Demo_Instancing ${SAMPLE_DEPENDENCIES})
endif ()

