# Configure Grass demo build

set(SOURCE_FILES src/Grass.cpp)

add_executable(Demo_Grass WIN32 ${SOURCE_FILES})
ogre_config_build(Demo_Grass)
target_link_libraries(Demo_Grass ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_Grass)

# Make sure all plugins are built
if (OGRE_BUILD_PLUGIN_OCTREE)
	add_dependencies(Demo_Grass Plugin_OctreeSceneManager)
endif ()
if (OGRE_BUILD_PLUGIN_BSP)
	add_dependencies(Demo_Grass Plugin_BSPSceneManager)
endif ()
if (OGRE_BUILD_PLUGIN_CG)
	add_dependencies(Demo_Grass Plugin_CgProgramManager)
endif (OGRE_BUILD_PLUGIN_CG)
if (OGRE_BUILD_PLUGIN_PFX)
	add_dependencies(Demo_Grass Plugin_ParticleFX)
endif ()

if (OGRE_BUILD_PLUGIN_PCZ)
	add_dependencies(Demo_Grass Plugin_PCZSceneManager)
	add_dependencies(Demo_Grass Plugin_OctreeZone)
endif ()

if (OGRE_BUILD_RENDERSYSTEM_D3D9)
	add_dependencies(Demo_Grass RenderSystem_Direct3D9)
endif ()
if (OGRE_BUILD_RENDERSYSTEM_D3D10)
	add_dependencies(Demo_Grass RenderSystem_Direct3D10)
endif ()
if (OGRE_BUILD_RENDERSYSTEM_GL)
	add_dependencies(Demo_Grass RenderSystem_GL)
endif ()

