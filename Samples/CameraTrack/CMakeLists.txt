# Configure CameraTrack demo build

set(SOURCE_FILES src/CameraTrack.cpp)

add_executable(Demo_CameraTrack WIN32 ${SOURCE_FILES})
ogre_config_build(Demo_CameraTrack)
target_link_libraries(Demo_CameraTrack ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_CameraTrack)

# Make sure all plugins are built
if (OGRE_BUILD_PLUGIN_OCTREE)
	add_dependencies(Demo_CameraTrack Plugin_OctreeSceneManager)
endif ()
if (OGRE_BUILD_PLUGIN_BSP)
	add_dependencies(Demo_CameraTrack Plugin_BSPSceneManager)
endif ()
if (OGRE_BUILD_PLUGIN_CG)
	add_dependencies(Demo_CameraTrack Plugin_CgProgramManager)
endif (OGRE_BUILD_PLUGIN_CG)
if (OGRE_BUILD_PLUGIN_PFX)
	add_dependencies(Demo_CameraTrack Plugin_ParticleFX)
endif ()

if (OGRE_BUILD_PLUGIN_PCZ)
	add_dependencies(Demo_CameraTrack Plugin_PCZSceneManager)
	add_dependencies(Demo_CameraTrack Plugin_OctreeZone)
endif ()

if (OGRE_BUILD_RENDERSYSTEM_D3D9)
	add_dependencies(Demo_CameraTrack RenderSystem_Direct3D9)
endif ()
if (OGRE_BUILD_RENDERSYSTEM_D3D10)
	add_dependencies(Demo_CameraTrack RenderSystem_Direct3D10)
endif ()
if (OGRE_BUILD_RENDERSYSTEM_GL)
	add_dependencies(Demo_CameraTrack RenderSystem_GL)
endif ()

