# Configure CelShading demo build

set(SOURCE_FILES src/CelShading.cpp)

add_executable(Demo_CelShading WIN32 ${SOURCE_FILES})
ogre_config_build(Demo_CelShading)
target_link_libraries(Demo_CelShading ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_CelShading)

# Make sure all plugins are built
if (OGRE_BUILD_PLUGIN_OCTREE)
	add_dependencies(Demo_CelShading Plugin_OctreeSceneManager)
endif ()
if (OGRE_BUILD_PLUGIN_BSP)
	add_dependencies(Demo_CelShading Plugin_BSPSceneManager)
endif ()
if (OGRE_BUILD_PLUGIN_CG)
	add_dependencies(Demo_CelShading Plugin_CgProgramManager)
endif (OGRE_BUILD_PLUGIN_CG)
if (OGRE_BUILD_PLUGIN_PFX)
	add_dependencies(Demo_CelShading Plugin_ParticleFX)
endif ()

if (OGRE_BUILD_PLUGIN_PCZ)
	add_dependencies(Demo_CelShading Plugin_PCZSceneManager)
	add_dependencies(Demo_CelShading Plugin_OctreeZone)
endif ()

if (OGRE_BUILD_RENDERSYSTEM_D3D9)
	add_dependencies(Demo_CelShading RenderSystem_Direct3D9)
endif ()
if (OGRE_BUILD_RENDERSYSTEM_D3D10)
	add_dependencies(Demo_CelShading RenderSystem_Direct3D10)
endif ()
if (OGRE_BUILD_RENDERSYSTEM_GL)
	add_dependencies(Demo_CelShading RenderSystem_GL)
endif ()

