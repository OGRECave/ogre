#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure BSP SceneManager build

file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_definitions(-D_USRDLL)

ogre_add_library(Plugin_BSPSceneManager ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(Plugin_BSPSceneManager OgreMain)
if (NOT OGRE_STATIC)
  set_target_properties(Plugin_BSPSceneManager PROPERTIES
    COMPILE_DEFINITIONS OGRE_BSPPLUGIN_EXPORTS
  ) 
endif ()

ogre_config_framework(Plugin_BSPSceneManager)

install(FILES ${HEADER_FILES} DESTINATION include/OGRE/Plugins/BSPSceneManager)

ogre_config_plugin(Plugin_BSPSceneManager)
