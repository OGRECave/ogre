# Configure RenderToTexture demo build

set(SOURCE_FILES src/RenderToTexture.cpp)

add_executable(Demo_RenderToTexture ${SOURCE_FILES} ${RESOURCE_FILES})
ogre_config_build(Demo_RenderToTexture)
target_link_libraries(Demo_RenderToTexture ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_RenderToTexture)

add_dependencies(Demo_RenderToTexture ${SAMPLE_DEPENDENCIES})
