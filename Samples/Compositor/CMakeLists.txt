# Configure Compositor demo build

set(HEADER_FILES 
  include/Compositor.h
  include/CompositorDemo_FrameListener.h
  include/ItemSelectorViewManager.h
)
set(SOURCE_FILES 
  src/Compositor.cpp
  src/CompositorDemo_FrameListener.cpp
  src/ItemSelectorViewManager.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Common/CEGUIRenderer/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(Demo_Compositor WIN32 ${HEADER_FILES} ${SOURCE_FILES})
ogre_config_build(Demo_Compositor)
target_link_libraries(Demo_Compositor ${OGRE_LIBRARIES} ${CEGUI_LIBRARIES} ${OGRE_CEGUI_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_Compositor)

# Make sure all plugins are built
if (OGRE_BUILD_PLUGIN_OCTREE)
	add_dependencies(Demo_Compositor Plugin_OctreeSceneManager)
endif ()
if (OGRE_BUILD_PLUGIN_BSP)
	add_dependencies(Demo_Compositor Plugin_BSPSceneManager)
endif ()
if (OGRE_BUILD_PLUGIN_CG)
	add_dependencies(Demo_Compositor Plugin_CgProgramManager)
endif (OGRE_BUILD_PLUGIN_CG)
if (OGRE_BUILD_PLUGIN_PFX)
	add_dependencies(Demo_Compositor Plugin_ParticleFX)
endif ()

if (OGRE_BUILD_PLUGIN_PCZ)
	add_dependencies(Demo_Compositor Plugin_PCZSceneManager)
	add_dependencies(Demo_Compositor Plugin_OctreeZone)
endif ()

if (OGRE_BUILD_RENDERSYSTEM_D3D9)
	add_dependencies(Demo_Compositor RenderSystem_Direct3D9)
endif ()
if (OGRE_BUILD_RENDERSYSTEM_D3D10)
	add_dependencies(Demo_Compositor RenderSystem_Direct3D10)
endif ()
if (OGRE_BUILD_RENDERSYSTEM_GL)
	add_dependencies(Demo_Compositor RenderSystem_GL)
endif ()

