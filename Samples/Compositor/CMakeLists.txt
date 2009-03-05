# Configure Compositor demo build

set(HEADER_FILES 
  include/Compositor.h
  include/CompositorDemo_FrameListener.h
  include/ItemSelectorViewManager.h
)
set(SOURCE_FILES 
  src/Compositor.cpp
  src/CompositorDemo_FrameListener.cpp
  src/ItemSelectorViewManager.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Common/CEGUIRenderer/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(Demo_Compositor WIN32 ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})
ogre_config_build(Demo_Compositor)
target_link_libraries(Demo_Compositor ${OGRE_LIBRARIES} ${CEGUI_LIBRARIES} ${OGRE_CEGUI_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_Compositor)

add_dependencies(Demo_Compositor ${SAMPLE_DEPENDENCIES})

