# Configure Isosurf demo build

set(SOURCE_FILES 
  src/Isosurf.cpp
  src/ProceduralTools.cpp
)

add_executable(Demo_Isosurf WIN32 ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(Demo_Isosurf ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Demo_Isosurf)

if (SAMPLE_DEPENDENCIES)
  add_dependencies(Demo_Isosurf ${SAMPLE_DEPENDENCIES})
endif ()
