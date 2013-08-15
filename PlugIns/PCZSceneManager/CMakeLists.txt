#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure PCZ SceneManager build

file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_definitions(-D_USRDLL)

ogre_add_library(Plugin_PCZSceneManager ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(Plugin_PCZSceneManager OgreMain)
if (NOT OGRE_STATIC)
  set_target_properties(Plugin_PCZSceneManager PROPERTIES
    COMPILE_DEFINITIONS OGRE_PCZPLUGIN_EXPORTS
  ) 
endif ()

ogre_config_framework(Plugin_PCZSceneManager)

ogre_config_plugin(Plugin_PCZSceneManager)
install(FILES ${HEADER_FILES} DESTINATION include/OGRE/Plugins/PCZSceneManager)
