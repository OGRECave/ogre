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
list(APPEND HEADER_FILES ${PROJECT_BINARY_DIR}/include/OgrePCZPrerequisites.h)
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(Plugin_PCZSceneManager ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(Plugin_PCZSceneManager OgreMain)
target_include_directories(Plugin_PCZSceneManager PUBLIC 
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    $<INSTALL_INTERFACE:include/OGRE/Plugins/PCZSceneManager>)

generate_export_header(Plugin_PCZSceneManager 
    EXPORT_MACRO_NAME _OgrePCZPluginExport
    EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/OgrePCZPrerequisites.h)
    
ogre_config_framework(Plugin_PCZSceneManager)
ogre_config_plugin(Plugin_PCZSceneManager)
install(FILES ${HEADER_FILES} DESTINATION include/OGRE/Plugins/PCZSceneManager)
