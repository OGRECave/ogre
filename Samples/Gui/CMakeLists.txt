# Configure Gui demo build

set(SOURCE_FILES src/Gui.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Common/CEGUIRenderer/include)
add_executable(Demo_Gui WIN32 ${SOURCE_FILES})
ogre_config_build(Demo_Gui)
target_link_libraries(Demo_Gui ${OGRE_LIBRARIES} ${CEGUI_LIBRARIES} ${OGRE_CEGUI_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_Gui)

# Make sure all plugins are built
if (OGRE_BUILD_PLUGIN_OCTREE)
	add_dependencies(Demo_Gui Plugin_OctreeSceneManager)
endif ()
if (OGRE_BUILD_PLUGIN_BSP)
	add_dependencies(Demo_Gui Plugin_BSPSceneManager)
endif ()
if (OGRE_BUILD_PLUGIN_CG)
	add_dependencies(Demo_Gui Plugin_CgProgramManager)
endif (OGRE_BUILD_PLUGIN_CG)
if (OGRE_BUILD_PLUGIN_PFX)
	add_dependencies(Demo_Gui Plugin_ParticleFX)
endif ()

if (OGRE_BUILD_PLUGIN_PCZ)
	add_dependencies(Demo_Gui Plugin_PCZSceneManager)
	add_dependencies(Demo_Gui Plugin_OctreeZone)
endif ()

if (OGRE_BUILD_RENDERSYSTEM_D3D9)
	add_dependencies(Demo_Gui RenderSystem_Direct3D9)
endif ()
if (OGRE_BUILD_RENDERSYSTEM_D3D10)
	add_dependencies(Demo_Gui RenderSystem_Direct3D10)
endif ()
if (OGRE_BUILD_RENDERSYSTEM_GL)
	add_dependencies(Demo_Gui RenderSystem_GL)
endif ()

