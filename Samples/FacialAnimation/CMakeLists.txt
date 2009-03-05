# Configure FacialAnimation demo build

set(SOURCE_FILES src/FacialAnimation.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Common/CEGUIRenderer/include)
add_executable(Demo_FacialAnimation WIN32 ${SOURCE_FILES} ${RESOURCE_FILES})
ogre_config_build(Demo_FacialAnimation)
target_link_libraries(Demo_FacialAnimation ${OGRE_LIBRARIES} ${CEGUI_LIBRARIES} ${OGRE_CEGUI_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_FacialAnimation)

add_dependencies(Demo_FacialAnimation ${SAMPLE_DEPENDENCIES})
