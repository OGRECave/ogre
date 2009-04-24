# Configure Bezier demo build

set(HEADER_FILES include/Bezier.h)
set(SOURCE_FILES src/Bezier.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(Demo_Bezier WIN32 ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES} )

target_link_libraries(Demo_Bezier ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Demo_Bezier)

if (SAMPLE_DEPENDENCIES)
  add_dependencies(Demo_Bezier ${SAMPLE_DEPENDENCIES})
endif ()

