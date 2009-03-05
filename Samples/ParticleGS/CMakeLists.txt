# Configure ParticleGS demo build

set(SOURCE_FILES 
  src/ParticleGS.cpp
  src/ProceduralManualObject.cpp
  src/ProceduralManualObject.h
  src/RandomTools.cpp
  src/RandomTools.h
)

add_executable(Demo_ParticleGS WIN32 ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})
ogre_config_build(Demo_ParticleGS)
target_link_libraries(Demo_ParticleGS ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_ParticleGS)

add_dependencies(Demo_ParticleGS ${SAMPLE_DEPENDENCIES})
