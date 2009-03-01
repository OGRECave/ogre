# Configure PCZTestApp demo build

set(HEADER_FILES include/PCZTestApp.h include/RoomObject.h)
set(SOURCE_FILES src/PCZTestApp.cpp src/RoomObject.cpp)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../../PlugIns/OctreeZone/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../../PlugIns/PCZSceneManager/include
)
add_executable(Demo_PCZTestApp WIN32 ${HEADER_FILES} ${SOURCE_FILES})
ogre_config_build(Demo_PCZTestApp)
target_link_libraries(Demo_PCZTestApp ${OGRE_LIBRARIES} ${OGRE_PCZ_LIBRARIES} ${OGRE_PCZ_OCTREE_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_PCZTestApp)

# Make sure all plugins are built
if (OGRE_BUILD_PLUGIN_OCTREE)
	add_dependencies(Demo_PCZTestApp Plugin_OctreeSceneManager)
endif ()
if (OGRE_BUILD_PLUGIN_BSP)
	add_dependencies(Demo_PCZTestApp Plugin_BSPSceneManager)
endif ()
if (OGRE_BUILD_PLUGIN_CG)
	add_dependencies(Demo_PCZTestApp Plugin_CgProgramManager)
endif (OGRE_BUILD_PLUGIN_CG)
if (OGRE_BUILD_PLUGIN_PFX)
	add_dependencies(Demo_PCZTestApp Plugin_ParticleFX)
endif ()

if (OGRE_BUILD_PLUGIN_PCZ)
	add_dependencies(Demo_PCZTestApp Plugin_PCZSceneManager)
	add_dependencies(Demo_PCZTestApp Plugin_OctreeZone)
endif ()

if (OGRE_BUILD_RENDERSYSTEM_D3D9)
	add_dependencies(Demo_PCZTestApp RenderSystem_Direct3D9)
endif ()
if (OGRE_BUILD_RENDERSYSTEM_D3D10)
	add_dependencies(Demo_PCZTestApp RenderSystem_Direct3D10)
endif ()
if (OGRE_BUILD_RENDERSYSTEM_GL)
	add_dependencies(Demo_PCZTestApp RenderSystem_GL)
endif ()

