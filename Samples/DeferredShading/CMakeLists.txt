# Configure DeferredShading demo build

set(HEADER_FILES 
  include/AmbientLight.h
  include/DeferredShading.h
  include/GeomUtils.h
  include/LightMaterialGenerator.h
  include/MaterialGenerator.h
  include/MLight.h
)
set(SOURCE_FILES 
  src/AmbientLight.cpp
  src/DeferredShading.cpp
  src/DeferredShadingDemo.cpp
  src/GeomUtils.cpp
  src/LightMaterialGenerator.cpp
  src/MaterialGenerator.cpp
  src/MLight.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(Demo_DeferredShading ${HEADER_FILES} ${SOURCE_FILES})
ogre_config_build(Demo_DeferredShading)
target_link_libraries(Demo_DeferredShading ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_DeferredShading)

# Make sure all plugins are built
if (OGRE_BUILD_PLUGIN_OCTREE)
	add_dependencies(Demo_DeferredShading Plugin_OctreeSceneManager)
endif ()
if (OGRE_BUILD_PLUGIN_BSP)
	add_dependencies(Demo_DeferredShading Plugin_BSPSceneManager)
endif ()
if (OGRE_BUILD_PLUGIN_CG)
	add_dependencies(Demo_DeferredShading Plugin_CgProgramManager)
endif (OGRE_BUILD_PLUGIN_CG)
if (OGRE_BUILD_PLUGIN_PFX)
	add_dependencies(Demo_DeferredShading Plugin_ParticleFX)
endif ()

if (OGRE_BUILD_PLUGIN_PCZ)
	add_dependencies(Demo_DeferredShading Plugin_PCZSceneManager)
	add_dependencies(Demo_DeferredShading Plugin_OctreeZone)
endif ()

if (OGRE_BUILD_RENDERSYSTEM_D3D9)
	add_dependencies(Demo_DeferredShading RenderSystem_Direct3D9)
endif ()
if (OGRE_BUILD_RENDERSYSTEM_D3D10)
	add_dependencies(Demo_DeferredShading RenderSystem_Direct3D10)
endif ()
if (OGRE_BUILD_RENDERSYSTEM_GL)
	add_dependencies(Demo_DeferredShading RenderSystem_GL)
endif ()

