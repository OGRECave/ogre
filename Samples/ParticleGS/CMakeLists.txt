# Configure ParticleGS demo build

set(SOURCE_FILES 
  src/ParticleGS.cpp
  src/ProceduralManualObject.cpp
  src/ProceduralManualObject.h
  src/RandomTools.cpp
  src/RandomTools.h
)

add_executable(Demo_ParticleGS WIN32 ${HEADER_FILES} ${SOURCE_FILES})
ogre_config_build(Demo_ParticleGS)
target_link_libraries(Demo_ParticleGS ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_ParticleGS)

# Make sure all plugins are built
if (OGRE_BUILD_PLUGIN_OCTREE)
	add_dependencies(Demo_ParticleGS Plugin_OctreeSceneManager)
endif ()
if (OGRE_BUILD_PLUGIN_BSP)
	add_dependencies(Demo_ParticleGS Plugin_BSPSceneManager)
endif ()
if (OGRE_BUILD_PLUGIN_CG)
	add_dependencies(Demo_ParticleGS Plugin_CgProgramManager)
endif (OGRE_BUILD_PLUGIN_CG)
if (OGRE_BUILD_PLUGIN_PFX)
	add_dependencies(Demo_ParticleGS Plugin_ParticleFX)
endif ()

if (OGRE_BUILD_PLUGIN_PCZ)
	add_dependencies(Demo_ParticleGS Plugin_PCZSceneManager)
	add_dependencies(Demo_ParticleGS Plugin_OctreeZone)
endif ()

if (OGRE_BUILD_RENDERSYSTEM_D3D9)
	add_dependencies(Demo_ParticleGS RenderSystem_Direct3D9)
endif ()
if (OGRE_BUILD_RENDERSYSTEM_D3D10)
	add_dependencies(Demo_ParticleGS RenderSystem_Direct3D10)
endif ()
if (OGRE_BUILD_RENDERSYSTEM_GL)
	add_dependencies(Demo_ParticleGS RenderSystem_GL)
endif ()

