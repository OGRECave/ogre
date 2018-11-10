#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure OctreeZone SceneManager build

file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
list(APPEND HEADER_FILES ${PROJECT_BINARY_DIR}/include/OgreOctreeZonePrerequisites.h)
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(Plugin_OctreeZone ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(Plugin_OctreeZone PUBLIC OgreMain Plugin_PCZSceneManager)
target_include_directories(Plugin_OctreeZone PUBLIC 
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    $<INSTALL_INTERFACE:include/OGRE/Plugins/OctreeZone>)

generate_export_header(Plugin_OctreeZone 
    EXPORT_MACRO_NAME _OgreOctreeZonePluginExport
    EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/OgreOctreeZonePrerequisites.h)

ogre_config_framework(Plugin_OctreeZone)

ogre_config_plugin(Plugin_OctreeZone)
install(FILES ${HEADER_FILES} DESTINATION include/OGRE/Plugins/OctreeZone)
