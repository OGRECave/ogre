#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure CgProgramManager build

file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_definitions(-DOGRE_CGPLUGIN_EXPORTS ${OGRE_VISIBILITY_FLAGS})

add_library(Plugin_CgProgramManager ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(Plugin_CgProgramManager OgreMain ${Cg_LIBRARIES})
target_include_directories(Plugin_CgProgramManager PUBLIC 
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    $<INSTALL_INTERFACE:include/OGRE/Plugins/CgProgramManager>
    PRIVATE ${Cg_INCLUDE_DIRS})

ogre_config_framework(Plugin_CgProgramManager)

ogre_config_plugin(Plugin_CgProgramManager)
install(FILES ${HEADER_FILES} DESTINATION include/OGRE/Plugins/CgProgramManager)
