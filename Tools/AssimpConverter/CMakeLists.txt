#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

add_executable(OgreAssimpConverter main.cpp)
target_link_libraries(OgreAssimpConverter OgreMain Codec_Assimp)
if (OGRE_PROJECT_FOLDERS)
	set_property(TARGET OgreAssimpConverter PROPERTY FOLDER Tools)
endif ()
ogre_config_tool(OgreAssimpConverter)
