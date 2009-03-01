# Configure RenderToTexture demo build

set(SOURCE_FILES src/RenderToTexture.cpp)

add_executable(Demo_RenderToTexture ${SOURCE_FILES})
ogre_config_build(Demo_RenderToTexture)
target_link_libraries(Demo_RenderToTexture ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_RenderToTexture)

# Make sure all plugins are built
if (OGRE_BUILD_PLUGIN_OCTREE)
	add_dependencies(Demo_RenderToTexture Plugin_OctreeSceneManager)
endif ()
if (OGRE_BUILD_PLUGIN_BSP)
	add_dependencies(Demo_RenderToTexture Plugin_BSPSceneManager)
endif ()
if (OGRE_BUILD_PLUGIN_CG)
	add_dependencies(Demo_RenderToTexture Plugin_CgProgramManager)
endif (OGRE_BUILD_PLUGIN_CG)
if (OGRE_BUILD_PLUGIN_PFX)
	add_dependencies(Demo_RenderToTexture Plugin_ParticleFX)
endif ()

if (OGRE_BUILD_PLUGIN_PCZ)
	add_dependencies(Demo_RenderToTexture Plugin_PCZSceneManager)
	add_dependencies(Demo_RenderToTexture Plugin_OctreeZone)
endif ()

if (OGRE_BUILD_RENDERSYSTEM_D3D9)
	add_dependencies(Demo_RenderToTexture RenderSystem_Direct3D9)
endif ()
if (OGRE_BUILD_RENDERSYSTEM_D3D10)
	add_dependencies(Demo_RenderToTexture RenderSystem_Direct3D10)
endif ()
if (OGRE_BUILD_RENDERSYSTEM_GL)
	add_dependencies(Demo_RenderToTexture RenderSystem_GL)
endif ()

