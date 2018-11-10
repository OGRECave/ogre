#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Octree SceneManager build

file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
list(APPEND HEADER_FILES ${PROJECT_BINARY_DIR}/include/OgreOctreePrerequisites.h)
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(Plugin_OctreeSceneManager ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(Plugin_OctreeSceneManager OgreMain)
target_include_directories(Plugin_OctreeSceneManager PUBLIC 
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    $<INSTALL_INTERFACE:include/OGRE/Plugins/OctreeSceneManager>)

generate_export_header(Plugin_OctreeSceneManager 
    EXPORT_MACRO_NAME _OgreOctreePluginExport
    EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/OgreOctreePrerequisites.h)

ogre_config_framework(Plugin_OctreeSceneManager)
ogre_config_plugin(Plugin_OctreeSceneManager)
install(FILES ${HEADER_FILES} DESTINATION include/OGRE/Plugins/OctreeSceneManager)
