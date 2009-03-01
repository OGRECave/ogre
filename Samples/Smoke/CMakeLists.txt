# Configure Smoke demo build

set(HEADER_FILES include/Smoke.h)
set(SOURCE_FILES src/Smoke.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(Demo_Smoke WIN32 ${HEADER_FILES} ${SOURCE_FILES})
ogre_config_build(Demo_Smoke)
target_link_libraries(Demo_Smoke ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_Smoke)

# Make sure all plugins are built
if (OGRE_BUILD_PLUGIN_OCTREE)
	add_dependencies(Demo_Smoke Plugin_OctreeSceneManager)
endif ()
if (OGRE_BUILD_PLUGIN_BSP)
	add_dependencies(Demo_Smoke Plugin_BSPSceneManager)
endif ()
if (OGRE_BUILD_PLUGIN_CG)
	add_dependencies(Demo_Smoke Plugin_CgProgramManager)
endif (OGRE_BUILD_PLUGIN_CG)
if (OGRE_BUILD_PLUGIN_PFX)
	add_dependencies(Demo_Smoke Plugin_ParticleFX)
endif ()

if (OGRE_BUILD_PLUGIN_PCZ)
	add_dependencies(Demo_Smoke Plugin_PCZSceneManager)
	add_dependencies(Demo_Smoke Plugin_OctreeZone)
endif ()

if (OGRE_BUILD_RENDERSYSTEM_D3D9)
	add_dependencies(Demo_Smoke RenderSystem_Direct3D9)
endif ()
if (OGRE_BUILD_RENDERSYSTEM_D3D10)
	add_dependencies(Demo_Smoke RenderSystem_Direct3D10)
endif ()
if (OGRE_BUILD_RENDERSYSTEM_GL)
	add_dependencies(Demo_Smoke RenderSystem_GL)
endif ()

