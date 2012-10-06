#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure OctreeZone SceneManager build

set (HEADER_FILES
  include/OgreOctreeZone.h
  include/OgreOctreeZoneOctree.h
  include/OgreOctreeZonePlugin.h
  include/OgreOctreeZonePrerequisites.h
)

set (SOURCE_FILES
  src/OgreOctreeZone.cpp
  src/OgreOctreeZoneDll.cpp
  src/OgreOctreeZoneOctree.cpp
  src/OgreOctreeZonePlugin.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../PCZSceneManager/include)
add_definitions(-D_USRDLL)

ogre_add_library(Plugin_OctreeZone ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(Plugin_OctreeZone OgreMain Plugin_PCZSceneManager)
if (NOT OGRE_STATIC)
  set_property(TARGET Plugin_OctreeZone PROPERTY
    COMPILE_DEFINITIONS OGRE_OCTREEZONEPLUGIN_EXPORTS)
endif ()

# need to include the plugins directory in the RPATH for OctreeZone to be able
# to find the PCZSceneManager library
if (UNIX)
  set_property(TARGET Plugin_OctreeZone APPEND PROPERTY
    INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${OGRE_LIB_DIRECTORY}/OGRE
  )
endif ()

if (APPLE AND NOT OGRE_BUILD_PLATFORM_APPLE_IOS)
    # Set the INSTALL_PATH so that Plugins can be installed in the application package
    set_target_properties(Plugin_OctreeZone
       PROPERTIES BUILD_WITH_INSTALL_RPATH 1
       INSTALL_NAME_DIR "@executable_path/../Plugins"
    )

	# Copy headers into the main Ogre framework
	add_custom_command(TARGET Plugin_OctreeZone POST_BUILD
	  COMMAND ditto ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)/Ogre.framework/Headers/
	)
endif()

ogre_config_plugin(Plugin_OctreeZone)
install(FILES ${HEADER_FILES} DESTINATION include/OGRE/Plugins/OctreeZone)
