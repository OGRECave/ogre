#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

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

if (APPLE AND NOT OGRE_BUILD_PLATFORM_IPHONE)
    # Set the INSTALL_PATH so that Plugins can be installed in the application package
    set_target_properties(Plugin_BSPSceneManager
       PROPERTIES BUILD_WITH_INSTALL_RPATH 1
       INSTALL_NAME_DIR "@executable_path/../Plugins"
    )

	# Copy headers into the main Ogre framework
	add_custom_command(TARGET Plugin_BSPSceneManager POST_BUILD
	  COMMAND /Developer/Library/PrivateFrameworks/DevToolsCore.framework/Resources/pbxcp ARGS -exclude .DS_Store -exclude CVS -exclude .svn -exclude 'CMakeLists.txt' -resolve-src-symlinks ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)/Ogre.framework/Headers/
	)
endif()

if (OGRE_INSTALL_PLUGINS_HEADERS)
  install(FILES ${HEADER_FILES} DESTINATION include/OGRE/Plugins/BSPSceneManager)
endif ()

ogre_config_plugin(Plugin_BSPSceneManager)
