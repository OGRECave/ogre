# Configure BSP SceneManager build

set (HEADER_FILES
  include/OgreBspLevel.h
  include/OgreBspNode.h
  include/OgreBspPrerequisites.h
  include/OgreBspResourceManager.h
  include/OgreBspSceneManager.h
  include/OgreBspSceneManagerPlugin.h
  include/OgreBspSceneNode.h
  include/OgreQuake3Level.h
  include/OgreQuake3Shader.h
  include/OgreQuake3ShaderManager.h
  include/OgreQuake3Types.h
)

set (SOURCE_FILES
  src/OgreBspLevel.cpp
  src/OgreBspNode.cpp
  src/OgreBspResourceManager.cpp
  src/OgreBspSceneManager.cpp
  src/OgreBspSceneManagerDll.cpp
  src/OgreBspSceneManagerPlugin.cpp
  src/OgreBspSceneNode.cpp
  src/OgreQuake3Level.cpp
  src/OgreQuake3Shader.cpp
  src/OgreQuake3ShaderManager.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_definitions(-D_USRDLL)

add_library(Plugin_BSPSceneManager ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(Plugin_BSPSceneManager OgreMain)
if (NOT OGRE_STATIC)
  set_target_properties(Plugin_BSPSceneManager PROPERTIES
    COMPILE_DEFINITIONS OGRE_BSPPLUGIN_EXPORTS
  ) 
endif ()

if (OGRE_INSTALL_PLUGINS_HEADERS)
  install(FILES ${HEADER_FILES} DESTINATION include/OGRE/Plugins/BSPSceneManager)
endif ()

ogre_config_plugin(Plugin_BSPSceneManager)
