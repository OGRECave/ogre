# Configure RenderToTexture demo build

set(SOURCE_FILES src/RenderToTexture.cpp)

add_executable(Demo_RenderToTexture ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(Demo_RenderToTexture ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Demo_RenderToTexture)

if (SAMPLE_DEPENDENCIES)
  add_dependencies(Demo_RenderToTexture ${SAMPLE_DEPENDENCIES})
endif ()
