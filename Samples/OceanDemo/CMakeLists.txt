# Configure OceanDemo demo build

set(HEADER_FILES include/OceanDemo.h include/MaterialControls.h)
set(SOURCE_FILES src/OceanDemo.cpp src/MaterialControls.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Common/CEGUIRenderer/include)
add_executable(Demo_OceanDemo WIN32 ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})
ogre_config_build(Demo_OceanDemo)
target_link_libraries(Demo_OceanDemo ${OGRE_LIBRARIES} ${CEGUI_LIBRARIES} ${OGRE_CEGUI_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_OceanDemo)

add_dependencies(Demo_OceanDemo ${SAMPLE_DEPENDENCIES})
