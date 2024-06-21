#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

add_executable(OgreAssimpConverter main.cpp)
target_link_libraries(OgreAssimpConverter OgreMain OgreRTShaderSystem)
target_include_directories(OgreAssimpConverter PRIVATE $<TARGET_PROPERTY:fix::assimp,INTERFACE_INCLUDE_DIRECTORIES>)
if (OGRE_PROJECT_FOLDERS)
	set_property(TARGET OgreAssimpConverter PROPERTY FOLDER Tools)
endif ()
ogre_config_tool(OgreAssimpConverter)

set_property(TARGET OgreAssimpConverter PROPERTY
	INSTALL_RPATH "$ORIGIN/../${OGRE_PLUGINS_PATH}")