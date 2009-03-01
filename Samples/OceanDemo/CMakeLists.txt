# Configure OceanDemo demo build

set(HEADER_FILES include/OceanDemo.h include/MaterialControls.h)
set(SOURCE_FILES src/OceanDemo.cpp src/MaterialControls.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Common/CEGUIRenderer/include)
add_executable(Demo_OceanDemo WIN32 ${HEADER_FILES} ${SOURCE_FILES})
ogre_config_build(Demo_OceanDemo)
target_link_libraries(Demo_OceanDemo ${OGRE_LIBRARIES} ${CEGUI_LIBRARIES} ${OGRE_CEGUI_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_OceanDemo)

# Make sure all plugins are built
if (OGRE_BUILD_PLUGIN_OCTREE)
	add_dependencies(Demo_OceanDemo Plugin_OctreeSceneManager)
endif ()
if (OGRE_BUILD_PLUGIN_BSP)
	add_dependencies(Demo_OceanDemo Plugin_BSPSceneManager)
endif ()
if (OGRE_BUILD_PLUGIN_CG)
	add_dependencies(Demo_OceanDemo Plugin_CgProgramManager)
endif (OGRE_BUILD_PLUGIN_CG)
if (OGRE_BUILD_PLUGIN_PFX)
	add_dependencies(Demo_OceanDemo Plugin_ParticleFX)
endif ()

if (OGRE_BUILD_PLUGIN_PCZ)
	add_dependencies(Demo_OceanDemo Plugin_PCZSceneManager)
	add_dependencies(Demo_OceanDemo Plugin_OctreeZone)
endif ()

if (OGRE_BUILD_RENDERSYSTEM_D3D9)
	add_dependencies(Demo_OceanDemo RenderSystem_Direct3D9)
endif ()
if (OGRE_BUILD_RENDERSYSTEM_D3D10)
	add_dependencies(Demo_OceanDemo RenderSystem_Direct3D10)
endif ()
if (OGRE_BUILD_RENDERSYSTEM_GL)
	add_dependencies(Demo_OceanDemo RenderSystem_GL)
endif ()

