#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Octree SceneManager build

set (HEADER_FILES
  include/OgreOctree.h
  include/OgreOctreeCamera.h
  include/OgreOctreeNode.h
  include/OgreOctreePlugin.h
  include/OgreOctreePrerequisites.h
  include/OgreOctreeSceneManager.h
  include/OgreOctreeSceneQuery.h
)

set (SOURCE_FILES
  src/OgreOctree.cpp
  src/OgreOctreeCamera.cpp
  src/OgreOctreeNode.cpp
  src/OgreOctreePlugin.cpp
  src/OgreOctreeSceneManager.cpp
  src/OgreOctreeSceneManagerDll.cpp
  src/OgreOctreeSceneQuery.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_definitions(-D_USRDLL)

ogre_add_library(Plugin_OctreeSceneManager ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(Plugin_OctreeSceneManager OgreMain)
if (NOT OGRE_STATIC)
  set_target_properties(Plugin_OctreeSceneManager PROPERTIES
    COMPILE_DEFINITIONS OGRE_OCTREEPLUGIN_EXPORTS
  ) 
endif ()

if (APPLE AND NOT OGRE_BUILD_PLATFORM_APPLE_IOS)
    # Set the INSTALL_PATH so that Plugins can be installed in the application package
    set_target_properties(Plugin_OctreeSceneManager
       PROPERTIES BUILD_WITH_INSTALL_RPATH 1
       INSTALL_NAME_DIR "@executable_path/../Plugins"
    )

	# Copy headers into the main Ogre framework
	add_custom_command(TARGET Plugin_OctreeSceneManager POST_BUILD
	  COMMAND ditto ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)/Ogre.framework/Headers/
	)
endif()

ogre_config_plugin(Plugin_OctreeSceneManager)
install(FILES ${HEADER_FILES} DESTINATION include/OGRE/Plugins/OctreeSceneManager)

