# Configure SkyDome demo build

set(HEADER_FILES include/SkyDome.h)
set(SOURCE_FILES src/SkyDome.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(Demo_SkyDome WIN32 ${HEADER_FILES} ${SOURCE_FILES})
ogre_config_build(Demo_SkyDome)
target_link_libraries(Demo_SkyDome ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_SkyDome)

# Make sure all plugins are built
if (OGRE_BUILD_PLUGIN_OCTREE)
	add_dependencies(Demo_SkyDome Plugin_OctreeSceneManager)
endif ()
if (OGRE_BUILD_PLUGIN_BSP)
	add_dependencies(Demo_SkyDome Plugin_BSPSceneManager)
endif ()
if (OGRE_BUILD_PLUGIN_CG)
	add_dependencies(Demo_SkyDome Plugin_CgProgramManager)
endif (OGRE_BUILD_PLUGIN_CG)
if (OGRE_BUILD_PLUGIN_PFX)
	add_dependencies(Demo_SkyDome Plugin_ParticleFX)
endif ()

if (OGRE_BUILD_PLUGIN_PCZ)
	add_dependencies(Demo_SkyDome Plugin_PCZSceneManager)
	add_dependencies(Demo_SkyDome Plugin_OctreeZone)
endif ()

if (OGRE_BUILD_RENDERSYSTEM_D3D9)
	add_dependencies(Demo_SkyDome RenderSystem_Direct3D9)
endif ()
if (OGRE_BUILD_RENDERSYSTEM_D3D10)
	add_dependencies(Demo_SkyDome RenderSystem_Direct3D10)
endif ()
if (OGRE_BUILD_RENDERSYSTEM_GL)
	add_dependencies(Demo_SkyDome RenderSystem_GL)
endif ()

