# Configure ParticleGS demo build

set(SOURCE_FILES 
  src/ParticleGS.cpp
  src/ProceduralManualObject.cpp
  src/ProceduralManualObject.h
  src/RandomTools.cpp
  src/RandomTools.h
)

add_executable(Demo_ParticleGS WIN32 ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(Demo_ParticleGS ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Demo_ParticleGS)

if (SAMPLE_DEPENDENCIES)
  add_dependencies(Demo_ParticleGS ${SAMPLE_DEPENDENCIES})
endif ()
